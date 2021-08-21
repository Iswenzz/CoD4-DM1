#pragma once

namespace Iswenzz
{
	class Utility
	{
	public:
		template <class T, class Container>
		static std::vector<T> GetArrayDifference(const Container &a, const Container &b)
		{
			return GetArrayDifference(a, b, Equal);
		}

		template <typename T, class Container, typename Predicate>
		static std::vector<T> GetArrayDifference(const Container &a, const Container &b, Predicate predicate)
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
