#include <iostream>

#include <boost/program_options.hpp>

#include "VSAsposeSlidesManager.h"
#include "VSQtPdfManager.h"
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

	std::unique_ptr<tc::file_as_img::fs::IExporter> exporter;
	std::string fileFormat = vars["input-format"].as<std::string>();
	if(fileFormat == "pdf") {
		exporter.reset(new VSQtPdfManager());
	}
	else {
		exporter.reset(new VSAsposeSlidesManager());
	}
	std::string imageFormat = vars["output-format"].as<std::string>();
	try
	{
		exporter->exportAsImages(
			vars["input-file"].as<std::string>(),
			fileFormat,
			vars["output-dir"].as<std::string>(),
			imageFormat,
			tc::file_as_img::fs::IncrementNameGenerator(0, "." + imageFormat),
			{},
			[](const tc::file_as_img::fs::TypesHolder::String& name) {
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