#pragma once

#include <any>
#include <map>
#include <string>

#include <oneapi/tbb.h>

#include "func-wrappers.h"
#include "pipeline_desc.h"
#include "DLLMacro.h"

class DPCPP_DLL_API pipeline_impl
{
public:
	//pass in all the nodes required and the graph used to
	//control the collection of nodes. We don't know anything about
	//how they are connected or what kind of nodes we have
	pipeline_impl(pipeline_desc desc,
		std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
		std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
		std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs);
	~pipeline_impl();
	void StartPipeline();
	void StopPipeline();
	void Wait();

private:
	class impl;
	std::unique_ptr<impl> m_pimpl;
};