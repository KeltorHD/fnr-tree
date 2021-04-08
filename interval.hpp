#pragma once

class Interval
{
public:
	Interval() = default;
	Interval(double in, double out)
		: time_in(in), time_out(out) {}
	~Interval() = default;

	double time_in;
	double time_out;
};