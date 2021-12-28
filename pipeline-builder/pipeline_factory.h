#pragma once


//stl includes
#include <filesystem>
#include <memory>

//custom includes
#include "pipeline_impl.h"
#include "DLLMacro.h"
#include "func-wrappers.h"

class DPCPP_DLL_API pipeline_factory
{
public:
	pipeline_factory(std::filesystem::path config);

	~pipeline_factory();

	std::shared_ptr<pipeline_impl> getPipeline(std::string pipeline_name,
		std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
		std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
		std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs);

private:
	class impl;
	std::unique_ptr<impl> m_pimpl;
};