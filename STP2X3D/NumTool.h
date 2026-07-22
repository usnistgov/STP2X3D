#pragma once

#include "StrTool.h"

#include <charconv>
#include <cstdio>

class NumTool
{
public:

	static const string DoubleToString(double val)
	{
		// Round up
		double digit = 1.e4; // e4: 4th decimal digit, e-4: 4th digit
		double val_ru = RoundUp(val, digit);

		// Match iostream defaultfloat with precision 6 (same as previous stringstream path)
		char buf[64];
		const auto result = to_chars(buf, buf + sizeof(buf), val_ru, chars_format::general, 6);
		string str(buf, result.ptr);

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

	static const wstring DoubleToWString(double val)
	{
		const string str = DoubleToString(val);
		// Numeric strings are ASCII; avoid locale-heavy conversion helpers.
		return wstring(str.begin(), str.end());
	}

	static const double RoundUp(double val, const double digit)
	{
		double val_ru = floor(abs(val) * digit + 0.5);
		val_ru = val_ru / digit;

		if (val < 0.0)
			val_ru = (-1) * val_ru;

		return val_ru;
	}

};
