#pragma once
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace CoD4::DM1
{
	class Utility
	{
	public:
		static std::vector<std::string> SplitString(const std::string& source, char delimiter = ' ');

		template <class T>
		static T VectorAverageMode(const std::vector<T>& vec)
		{
			std::unordered_map<T, size_t> modeMap;
			for (const T& elem : vec)
				++modeMap[elem];

			auto mode = std::max_element(modeMap.cbegin(), modeMap.cend(),
				[](const auto& a, const auto& b) { return a.second < b.second; });
			return mode->first;
		}

		template <class T, class Container>
		static std::vector<T> GetArrayDifference(const Container& a, const Container& b)
		{
			return GetArrayDifference(a, b, Equal);
		}

		template <typename T, class Container, typename Predicate>
		static std::vector<T> GetArrayDifference(const Container& a, const Container& b, Predicate predicate)
		{
			std::vector<T> difference;
			typename Container::const_iterator itA = a.begin(), itB = b.begin();

			while (itA != a.end() || itB != b.end())
			{
				if (!predicate(*itA, *itB))
					difference.push_back(*itA);
				itA++;
				itB++;
			}
			return difference;
		}

	private:
		template <typename T>
		static bool Equal(T& a, T& b)
		{
			return a == b;
		}
	};
}
