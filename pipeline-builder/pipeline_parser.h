#pragma once

#include "../test-filter/DLLMacro.h"

#include <memory>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "pipeline_desc.h"

class DPCPP_DLL_API pipeline_parser
{
public:
	pipeline_parser(std::filesystem::path config);
	~pipeline_parser();
	pipeline_desc getPipelineDesc(std::string name);

private:
	class impl;
	std::unique_ptr<impl> m_pimpl;
};