#pragma once

#include <codecvt>
#include <locale>

class StrTool
{
public:

	static const string ReplaceCharacter(string str, string from, string to)
	{
		size_t loc = str.find(from);

		while (loc != string::npos)
		{
			str.replace(loc, from.length(), to);
			loc = str.find(from, loc + to.length());
		}

		return str;
	}

	static const wstring ReplaceCharacter(wstring str, wstring from, wstring to)
	{
		size_t loc = str.find(from);

		while (loc != wstring::npos)
		{
			str.replace(loc, from.length(), to);
			loc = str.find(from, loc + to.length());
		}

		return str;
	}

	static const wstring RemoveCharacter(wstring str, wstring target)
	{		
		str = ReplaceCharacter(str, target, L"");

		return str;
	}

	static const string wstr2str(wstring ws)
	{
		string s(ws.begin(), ws.end());
		return s;
	}

	static const wstring str2wstr(string s)
	{
		wstring ws(s.begin(), s.end());
		return ws;
	}

	static const wstring u16str2wstr(const u16string& s)
	{
		wstring_convert<codecvt_utf16<wchar_t, 0x10ffff, little_endian>, wchar_t> conv;
		wstring ws = conv.from_bytes(reinterpret_cast<const char*> (&s[0]), reinterpret_cast<const char*> (&s[0] + s.size()));

		return ws;
	}

	static const string ToLower(string s)
	{
		transform(s.begin(), s.end(), s.begin(), ::tolower);

		return s;
	}

	static const string ToUpper(string s)
	{
		transform(s.begin(), s.end(), s.begin(), ::toupper);

		return s;
	}
};
