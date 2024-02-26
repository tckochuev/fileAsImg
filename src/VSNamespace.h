#pragma once

#include <filesystem>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

namespace tc
{
	namespace stdfs = std::filesystem;
	namespace ranges
	{
		using namespace boost;
		using namespace boost::range;
		namespace views = boost::adaptors;
	}
};