#pragma once

#include "StrTool.h"

class NumTool
{
public:

	static string DoubleToString(double val)
	{
		// Round up
		double digit = 1.e4; // e4: 4th decimal digit, e-4: 4th digit
		double val_ru = RoundUp(val, digit);

		// Write float to string
		stringstream real_value_ss;
		real_value_ss << val_ru;

		string str = real_value_ss.str();

		// -0 -> 0
		if (str == "-0"
			|| str == "-0.0")
			str = "0";

		// 0.xxx -> .xxx
		if (str.find("0.") == 0)
			str = str.substr(str.find("."), str.length() - 1);

		// -0.xxx -> -.xxx
		if (str.find("-0.") == 0)
			str = "-" + str.substr(str.find("."), str.length() - 1);

		return str;
	}

	static wstring DoubleToWString(double val)
	{
		string str = DoubleToString(val);
		wstring wstr = StrTool::str2wstr(str);

		return wstr;
	}

	static double RoundUp(double val, const double digit)
	{
		double val_ru = floor(abs(val) * digit + 0.5);
		val_ru = val_ru / digit;

		if (val < 0.0)
			val_ru = (-1) * val_ru;

		return val_ru;
	}

};