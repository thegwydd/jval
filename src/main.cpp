#include <cxxopts.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <nlohmann/json.hpp>

#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>

#include <fstream>
#include <format>
#include <filesystem>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
bool validate(const json & contents, const json & schema, std::list<std::string> & errors)
    {
    bool ret = false;
    try
        {
        // Parse JSON schema content using valijson
        valijson::Schema mySchema;
        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter mySchemaAdapter(schema);
        parser.populateSchema(mySchemaAdapter, mySchema);

        valijson::Validator validator;
        valijson::adapters::NlohmannJsonAdapter myTargetAdapter(contents);
        valijson::ValidationResults results;
        ret = validator.validate(mySchema, myTargetAdapter, &results);
        if (!ret)
            {
            for (auto & res : results)
                {
                std::stringstream exception_msg;
                for (auto & ctx : res.context)
                    exception_msg << ctx;
                exception_msg << "->" << res.description << std::endl;
                errors.push_back(exception_msg.str());
                }
            }
        }
    catch (const std::exception & e)
        {
        errors.push_back(std::format("Validation of schema failed, here is why: {}", e.what()));
        }

    return ret;
    }

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
    {
    // initialize default application logger
    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("jval", spdlog::sinks_init_list() = {
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
        });

    logger->set_pattern("%^[%Y-%m-%d %T.%e][%L]:%v%$");
    logger->set_level(spdlog::level::info);

    cxxopts::Options options("jval", "A json validator utility");
    options.add_options()
        ("i,input", "Input file to validate against schema", cxxopts::value<std::string>())
        ("s,schema", "Schema file to use for validation", cxxopts::value<std::string>())
        ;

    bool error = false;

    try
        {
        spdlog::info("jval application started.");

        spdlog::info("Parsing parameters...");
        auto result = options.parse(argc, argv);

        auto input_file = result["input"].as<std::string>();

        if (input_file.empty())
            {
            spdlog::error("Input filename is empty! Please input a valid input filename!");
            error = true;
            }

        if (!std::filesystem::exists(input_file))
            {
            spdlog::error("The given input filename does not exist! Please input a valid input filename!");
            error = true;
            }

        auto schema_file = result["schema"].as<std::string>();

        if (schema_file.empty())
            {
            spdlog::error("Input schema filename is empty! Please input a valid schema filename!");
            error = true;
            }

        if (!std::filesystem::exists(schema_file))
            {
            spdlog::error("The given schema filename does not exist! Please input a valid schema filename!");
            error = true;
            }

        if (!error)
            {
            std::ifstream input_file_stream(input_file);
            std::string input_str((std::istreambuf_iterator<char>(input_file_stream)), std::istreambuf_iterator<char>());
            json input = json::parse(input_str);

            if (!input.is_discarded())
                {
                std::ifstream schema_file_stream(schema_file);
                std::string schema_str((std::istreambuf_iterator<char>(schema_file_stream)), std::istreambuf_iterator<char>());
                json schema = json::parse(schema_str);

                std::list<std::string> errors;
                error = !validate(input, schema, errors);

                if (!errors.empty())
                    {
                    std::string oo;
                    for (auto & ss : errors)
                        oo += "\t" + ss;
                    spdlog::error("Validation errors:\n{}", oo);
                    }
                else
                    {
                    spdlog::info("Validation successfull!");
                    }
                }
            else
                {
                spdlog::error("Input file has been loaded and parsed but root is discarded!");
                }
            }
        else
            {
            spdlog::info("{}", options.help());
            }

        }
    catch (std::exception & ex)
        {
        spdlog::critical("Exception while executing:\n{}", ex.what());
        }

    return (error) ? 1 : 0;
    }

