//==============================================================
// Copyright © Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include "Windows.h"
#include <memory>
#include "../pipeline-builder/pipeline_factory.h"

// Demonstrate vector add both in sequential on CPU and in parallel on device.
//************************************

using factory_func = std::shared_ptr<pipeline_factory>(*)(std::filesystem::path);

int main(int argc, char* argv[]) {
	
	HINSTANCE plugin = LoadLibraryA("pipeline-builder.dll");
		
	auto get_factory_ptr = (factory_func)GetProcAddress(plugin, 
		"?getPipelineFactory@@YA?AV?$shared_ptr@Vpipeline_factory@@@std@@Vpath@filesystem@2@@Z");

	auto factory = get_factory_ptr("../pipeline_builder_test/processing.yaml");
}