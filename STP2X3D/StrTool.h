#pragma once

class StrTool
{
public:

	static string ReplaceCharacter(string str, string from, string to)
	{
		size_t loc = str.find(from);

		while (loc != string::npos)
		{
			str.replace(loc, from.length(), to);
			loc = str.find(from, loc + to.length());
		}

		return str;
	}

	static wstring ReplaceCharacter(wstring str, wstring from, wstring to)
	{
		size_t loc = str.find(from);

		while (loc != wstring::npos)
		{
			str.replace(loc, from.length(), to);
			loc = str.find(from, loc + to.length());
		}

		return str;
	}

	static string wstr2str(wstring ws)
	{
		string s(ws.begin(), ws.end());
		return s;
	}

	static wstring str2wstr(string s)
	{
		wstring ws(s.begin(), s.end());
		return ws;
	}

};