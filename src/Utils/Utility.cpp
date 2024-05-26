#include "Utility.hpp"

namespace Iswenzz::CoD4::DM1
{
	std::vector<std::string> Utility::SplitString(const std::string& source, char delimiter)
	{
		std::vector<std::string> results;

		size_t prev = 0;
		size_t next = 0;

		while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
		{
			if (next - prev != 0)
				results.push_back(source.substr(prev, next - prev));
			prev = next + 1;
		}
		if (prev < source.size())
			results.push_back(source.substr(prev));

		return results;
	}
}
