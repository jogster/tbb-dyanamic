// lib includes
#include "gtest/gtest.h"

//custom includes
#include "../pipeline-builder/pipeline_parser.h"
#include "../pipeline-builder/pipeline_factory.h"
#include <fstream>

namespace test
{
	class parser_test : public ::testing::Test
	{
	protected:
		parser_test()
		{}

		~parser_test() override {
			// You can do clean-up work that doesn't throw exceptions here.
		}

		void SetUp() override {

		}

		void TearDown() override {

		}
	};

	TEST_F(parser_test, build_parser)
	{
		pipeline_parser p("processing.yaml");
	}

	TEST_F(parser_test, getPipeline)
	{
		pipeline_factory factory("processing.yaml");
		std::atomic_bool run = false;
		auto func_source = std::make_shared<source_wrapper<std::map<std::string, std::any>>>([&](auto& fg) {
			double one = 1.0;
			double two = 2.0;
			std::map<std::string, std::any> ret({ {"one", one}, {"two", two} });

			return ret;
		});

		std::pair<std::string, std::shared_ptr<source_wrapper_base>> source_func(
		{
			"source_1", 
			func_source
		});

		auto func_trans_1 = std::make_shared<transform_wrapper<double, double>>([&](auto val) {
			return val * 2;
		});

		std::pair<std::string, std::shared_ptr<transform_wrapper_base>> trans_func_1(
		{
			"trans_1",
			func_trans_1

		});

		auto func_trans_2 = std::make_shared<transform_wrapper<double, double>>([&](auto val) {
			return val * 10;
		});

		std::pair<std::string, std::shared_ptr<transform_wrapper_base>> trans_func_2(
		{
			"trans_2",
			func_trans_2

		});

		auto func_sink = std::make_shared<sink_wrapper<std::map<std::string,std::any>>>([&](auto val) {
			for (const auto& entry : val)
			{
				//cast entry to known type
				auto casted_res = std::any_cast<double>(entry.second);
				std::cout << casted_res << std::endl;
			}
		});

		std::pair<std::string, std::shared_ptr<sink_wrapper_base>> sink_func(
		{
			"sink_1",
			func_sink
		});

		auto pipeline = factory.getPipeline("bmode_pipeline", { source_func}, { trans_func_1 , trans_func_2}, { sink_func});
		pipeline->StartPipeline();
		pipeline->Wait();
	}
}