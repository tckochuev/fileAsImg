#include <iostream>

#include <boost/program_options.hpp>

#include "VSAsposeSlidesManager.h"
#include "VSExportFileAsImages.h"

int main(int argc, char** argv)
{
	namespace opt = boost::program_options;
	opt::options_description options;
	options.add_options()
	("input-file", opt::value<std::string>()->required())
	("input-format", opt::value<std::string>()->required())
	("output-dir", opt::value<std::string>()->default_value("."))
	("output-format", opt::value<std::string>()->default_value("png"));
	opt::variables_map vars;
	try
	{
		opt::store(opt::parse_command_line(argc, argv, options), vars);
	}
	catch(opt::error& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	VSAsposeSlidesManager exporter;
	try
	{
		exporter.exportAsImages(
			vars["input-file"].as<std::string>(),
			vars["input-format"].as<std::string>(),
			vars["output-dir"].as<std::string>(),
			vars["output-format"].as<std::string>(),
			VSIncrementNameGenerator(0, ".png"),
			{},
			[](const VSIFileAsImagesExporterTypesHolder::String& name) {
				std::cout << name << std::endl;
			}
		);
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 2;
	}

	return 0;
}