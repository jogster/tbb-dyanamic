//stl includes
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <any>

//lib includes
#include <yaml-cpp/yaml.h>
#include <xtensor/xarray.hpp>
#include <oneapi/tbb.h>

//custom includes
#include "pipeline_factory.h"
#include "pipeline_parser.h"


class pipeline_factory::impl
{
public:
	impl(std::filesystem:: path processing_desc) :
		parser(processing_desc)
	{

	}

	pipeline_parser parser;
};


pipeline_factory::pipeline_factory(std::filesystem::path config) : 
	m_pimpl(std::make_unique<impl>(config))
{

}

pipeline_factory::~pipeline_factory() = default;

std::shared_ptr<pipeline_impl> pipeline_factory::getPipeline(
	std::string pipeline_name,
	std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
	std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
	std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs)
{
	return std::make_shared<pipeline_impl>(
		m_pimpl->parser.getPipelineDesc(pipeline_name),
		sourc_funcs,
		tranform_funcs,
		sink_funcs
	);
}


std::shared_ptr<pipeline_factory> getPipelineFactory(std::filesystem::path config)
{
	return std::make_shared<pipeline_factory>(config);
}