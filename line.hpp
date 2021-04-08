#pragma once

#include <array>

class Line
{
public:
	Line() = default;
	Line(int min_x, int min_y, int max_x, int max_y)
		: min{ min_x, min_y }, max{ max_x, max_y } {}
	~Line() = default;

	std::array<int, 2> min;
	std::array<int, 2> max;

	/*функции*/
	bool equals(const Line& other)
	{
		return min[0] == other.min[0] && min[1] == other.min[1]
			&& max[0] == other.max[0] && max[1] == other.max[1];
	}
};