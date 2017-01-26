
#include <array>
#include <iostream>
#include <string>

#include "popt.h"

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "hostcompute_nspawn.h"
#include "NSpawnException.hpp"

namespace { // anonymous

namespace si = staticlib::io;
namespace su = staticlib::utils;

class Options {
    std::vector<struct poptOption> table;
    // parse options list
    char* config_opt = nullptr;
    char* directory_opt = nullptr;
    int help_opt = 0;
    
public:
    poptContext ctx = nullptr;
    std::string parse_error;
    std::vector<std::string> args;
    
    // public options list
    std::string directory;
    std::string config;
    bool help;

    Options(int argc, char** argv) :
    table({
        { "directory", 'D', POPT_ARG_STRING, &directory_opt, static_cast<int> ('D'), "Path to directory to mount into container", "c:/path/to/directory" },
        { "config", 'c', POPT_ARG_STRING, &config_opt, static_cast<int> ('c'), "Path to JSON config file", "config.json"},
        { "help", 'h', POPT_ARG_NONE, &help_opt, static_cast<int> ('h'), "Show this help message", nullptr},
        { nullptr, 0, 0, nullptr, 0, nullptr, nullptr}
    }) {

        { // create context
            ctx = poptGetContext(nullptr, argc, const_cast<const char**> (argv), table.data(), POPT_CONTEXT_NO_EXEC);
            if (!ctx) {
                parse_error.append("'poptGetContext' error");
                return;
            }
        }

        { // parse options
            int val;
            while ((val = poptGetNextOpt(ctx)) >= 0);
            if (val < -1) {
                parse_error.append(poptStrerror(val));
                parse_error.append(": ");
                parse_error.append(poptBadOption(ctx, POPT_BADOPTION_NOALIAS));
                return;
            }
        }

        { // collect arguments
            const char* ar;
            while (nullptr != (ar = poptGetArg(ctx))) {
                args.push_back(std::string(ar));
            }
        }
        
        { // check unneeded args
            if (args.size() > 0) {
                parse_error.append("invalid arguments specified:");
                for (auto& st : args) {
                    parse_error.append(" ");
                    parse_error.append(st);
                }
            }
        }
        
        { // set public opts
            directory = nullptr != directory_opt ? std::string(directory_opt) : "";
            config = nullptr != config_opt ? std::string(config_opt) : "";
            help = 0 != help_opt;
        }
    }

    ~Options() {
        poptFreeContext(ctx);
    }

    Options(const Options& other) = delete;

    Options& operator=(const Options& other) = delete;
    
};

std::string default_config_path() {
    std::string exepath = su::current_executable_path();
    std::string dirpath = su::strip_filename(exepath);
    return std::string(dirpath) + "config.json";
}

std::string read_config(const std::string& config_path) {
    auto fd = su::FileDescriptor(config_path, 'r');
    auto sink = si::string_sink();
    std::array<char, 4096> buf;
    si::copy_all(fd, sink, buf.data(), buf.size());
    return sink.get_string();
}

} // namespace


int main(int argc, char** argv) {
    // parse
    Options opts(argc, argv);
    
    // check invalid options
    if (!opts.parse_error.empty()) {
        std::cerr << "ERROR: " << opts.parse_error << std::endl;
        poptPrintUsage(opts.ctx, stderr, 0);
        std::exit(1);
    }

    // show help
    if (opts.help) {
        std::cout << "hostcompute_nspawn v1.0" << std::endl;
        poptPrintHelp(opts.ctx, stdout, 0);
        std::exit(0);
    }

    // read config and run
    std::string conf;
    try {
        std::string path = !opts.config.empty() ? opts.config : default_config_path();
        conf = read_config(path);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << TRACEMSG(e.what() + "\n Config read error") << std::endl;
        std::exit(1);
    }

    // run container
    char* err = ::hostcompute_nspawn(conf.c_str(), static_cast<int> (conf.length()));
    if (nullptr != err) {
        std::cerr << "ERROR: " << err << std::endl;
        ::hostcompute_nspawn_free(err);
        std::exit(1);
    }

    return 0;
}
