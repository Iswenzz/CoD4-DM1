#pragma once
#include <string>
#include <vector>

namespace Iswenzz::CoD4::DM1
{
	class Utility
	{
	public:
		/// <summary>
		/// Split a string from a delimiter.
		/// </summary>
		/// <param name="source">The string source.</param>
		/// <param name="delimiter">The split delimiter.</param>
		/// <returns></returns>
		static std::vector<std::string> SplitString(const std::string& source, char delimiter = ' ');

		/// <summary>
		/// Get the difference between 2 containers.
		/// </summary>
		/// <typeparam name="T">The vector type.</typeparam>
		/// <typeparam name="Container">STL container class.</typeparam>
		/// <param name="a">The first container.</param>
		/// <param name="b">The second container.</param>
		/// <returns></returns>
		template <class T, class Container>
		static std::vector<T> GetArrayDifference(const Container &a, const Container &b)
		{
			return GetArrayDifference(a, b, Equal);
		}

		/// <summary>
		/// Get the difference between 2 containers.
		/// </summary>
		/// <typeparam name="T">The vector type.</typeparam>
		/// <typeparam name="Container">STL container class.</typeparam>
		/// <typeparam name="Predicate">The predicate function.</typeparam>
		/// <param name="a">The first container.</param>
		/// <param name="b">The second container.</param>
		/// <param name="predicate">The predicate function.</param>
		/// <returns></returns>
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
		/// <summary>
		/// Predicate function to check equality.
		/// </summary>
		/// <typeparam name="T">Class to check.</typeparam>
		/// <param name="a">Variable A.</param>
		/// <param name="b">Variable B.</param>
		/// <returns></returns>
		template <typename T> 
		static bool Equal(T& a, T& b)
		{
			return a == b;
		}
	};
}
