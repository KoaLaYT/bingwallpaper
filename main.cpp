#include <cpr/cpr.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

#include "cxxopts.hpp"

#define LOG(...) fmt::print(__VA_ARGS__);
#define ERROR(...) fmt::print(stderr, __VA_ARGS__);

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
    string_view unknown_image{"unknown_image.jpg"};
    string base_dir;
    string index;

    json image_query()
    {
        cpr::Response r = cpr::Get(cpr::Url(base_url + image_api),
                                   cpr::Parameters{
                                       {"format", "js"},
                                       {"idx", move(index)},
                                       {"n", "1"},
                                       {"uhd", "1"},
                                       {"uhdwidth", "3840"},
                                       {"uhdheight", "2160"},
                                   });
        return json::parse(r.text);
    }

    string_view parse_imagename(const string& url)
    {
        static regex filename_regex(".+[^p]id=(.+?)&.+", regex_constants::ECMAScript);
        smatch base_match;
        if (regex_match(url, base_match, filename_regex)) {
            auto pos = base_match.position(1);
            auto size = base_match[1].length();
            return string_view{url}.substr(pos, size);
        }
        return unknown_image;
    }

    void save_image(const string& url)
    {
        LOG("Fetch image from: {}\n", url);
        cpr::Response r = cpr::Get(cpr::Url(url));
        LOG("Image Fetched\n");

        auto imagename = parse_imagename(url);
        ofstream ofs{fmt::format("{}/{}", base_dir, imagename)};
        if (!ofs.is_open()) {
            LOG("Can't open file: {}\n", imagename);
            return;
        }

        LOG("Save image as: {}\n", imagename);
        ofs << r.text;
        LOG("Image saved\n");
    }
};

int main(int argc, char** argv)
{
    cxxopts::Options options{"bingwallpaper", "fetch the latest wallpaper from bing"};
    // clang-format off
    options.add_options()
        ("d,dir", "The directory to save wallpapers", cxxopts::value<string>())
        ("i,index", "Which days' image to fetch", cxxopts::value<string>()->default_value("0"))
        ("h,help","Print usage")
        ;
    // clang-format on
    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            LOG("{}\n", options.help());
            return 0;
        }

        string dir = result["dir"].as<string>();
        string idx = result["index"].as<string>();

        if (!filesystem::is_directory(dir)) {
            throw runtime_error{fmt::format("{} is not a valid directory\n", dir)};
        }

        BingFetcher bf{dir, idx};
        bf.fetch();
    } catch (cxxopts::OptionException e) {
        ERROR("error: {}\n\n", e.what());
        LOG("{}\n", options.help());
        return 1;
    } catch (runtime_error e) {
        ERROR("error: {}\n", e.what());
        return 2;
    } catch (exception e) {
        ERROR("unknown error\n");
        return 3;
    }
}