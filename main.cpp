#include <cpr/cpr.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>

#include "cxxopts.hpp"

using json = nlohmann::json;
using namespace std;

class BingFetcher {
public:
    BingFetcher(const string& d, const string& i) : base_dir{d}, index{i} {}

    void fetch()
    {
        auto json = image_query();
        for (auto& image : json["images"]) {
            auto url = base_url + image["url"].get<string>();
            save_image(url);
        }
    }

private:
    string base_url{"https://www.bing.com"};
    string image_api{"/HPImageArchive.aspx"};
    string base_dir;
    string index;

    json image_query()
    {
        cpr::Response r = cpr::Get(cpr::Url(base_url + image_api),
                                   cpr::Parameters{
                                       {"format", "js"},
                                       {"idx", index},
                                       {"n", "1"},
                                       {"uhd", "1"},
                                       {"uhdwidth", "3840"},
                                       {"uhdheight", "2160"},
                                   });
        return json::parse(r.text);
    }

    string parse_imagename(const string& url)
    {
        static regex filename_regex(".+[^p]id=(.+?)&.+", regex_constants::ECMAScript);
        smatch base_match;
        if (regex_match(url, base_match, filename_regex)) {
            return base_match[1].str();
        }
        return "unknown_image.jpg";
    }

    void save_image(const string& url)
    {
        fmt::print("Fetch image from: {}\n", url);
        cpr::Response r = cpr::Get(cpr::Url(url));

        auto imagename = parse_imagename(url);
        ofstream ofs{base_dir + "/" + imagename};
        if (!ofs.is_open()) {
            fmt::print("Can't open file: {}\n", imagename);
            return;
        }

        fmt::print("Save image as: {}\n", imagename);
        ofs << r.text;
        fmt::print("Image saved\n");
    }
};

int main(int argc, char** argv)
{
    cxxopts::Options options{"bingwallpaper", "fetch the latest wallpaper from bing\n"};
    // clang-format off
    options.add_options()
        ("d,dir", "The directory to save wallpapers", cxxopts::value<string>())
        ("i,index", "Which days' image to fetch", cxxopts::value<string>()->default_value("0"))
        ("h,help","Print usage")
        ;
    // clang-format on
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        fmt::print("{}\n", options.help());
        return 0;
    }

    try {
        string dir = result["dir"].as<string>();
        string idx = result["index"].as<string>();

        if (!filesystem::is_directory(dir)) {
            fmt::print("{} is not a valid directory\n", dir);
            return 1;
        }

        BingFetcher bf{dir, idx};
        bf.fetch();
    } catch (cxxopts::OptionException e) {
        fmt::print("error: {}\n", e.what());
        fmt::print("{}\n", options.help());
        return 1;
    }
}