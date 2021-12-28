//==============================================================
// Copyright © 2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#pragma once


//stl includes
#include <memory>
#include <vector>
#include <any>

//lib includes
#include <yaml-cpp/yaml.h>

//custom includes
#include "IFilter.h"
#include "ISink.h"
#include "ISource.h"
#include "DLLMacro.h"

namespace factory {
	DPCPP_DLL_API std::shared_ptr<IFilter> getCustomFilter(std::map<std::string, std::any> configuration, std::string name);

	DPCPP_DLL_API std::shared_ptr<ISource> getCustomSource(std::map<std::string, std::any> configuration, std::string name);

	DPCPP_DLL_API std::shared_ptr<ISink> getCustomSink(std::map<std::string, std::any> configuration, std::string name);
}


