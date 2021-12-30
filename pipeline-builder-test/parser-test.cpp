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
		auto func_source = std::make_shared<source_wrapper<double>>([&](auto& fg) {
			std::cout << "Source_1" << std::endl;
			return 1.0;
		});

		std::pair<std::string, std::shared_ptr<source_wrapper_base>> source_func(
		{
			"source_1", 
			func_source
	
		});

		auto func_trans = std::make_shared<source_wrapper<double>>([&](auto& fg) {
			std::cout << "Source_2" << std::endl;
			return 0.0;
		});

		std::pair<std::string, std::shared_ptr<source_wrapper_base>> trans_func(
		{
			"source_2",
			func_trans

		});

		auto func_sink = std::make_shared<sink_wrapper<std::map<std::string, std::any>>>([&](auto val) {
			auto values = std::any_cast<double>(val.begin()->second);
			std::cout << "Sink" << std::endl;
		});

		std::pair<std::string, std::shared_ptr<sink_wrapper_base>> sink_func(
		{
			"sink_func",
			func_sink
		});

		auto pipeline = factory.getPipeline("bmode_pipeline", { source_func, trans_func }, {}, { sink_func });
		pipeline->StartPipeline();
		pipeline->Wait();
	}
}