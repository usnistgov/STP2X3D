#pragma once

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

    // Added by Cabiddu
    static const wstring s2ws(const std::string& s)
	{
        std::string curLocale = setlocale(LC_ALL, "");
        const char* _Source = s.c_str();
        size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
        wchar_t *_Dest = new wchar_t[_Dsize];
        wmemset(_Dest, 0, _Dsize);
        mbstowcs(_Dest,_Source,_Dsize);
        std::wstring result = _Dest;
        delete []_Dest;
        setlocale(LC_ALL, curLocale.c_str());
        return result;
    }

	static const wstring u16str2wstr(const u16string& s)
	{
		wstring_convert<codecvt_utf16<wchar_t, 0x10ffff, little_endian>, wchar_t> conv;
		wstring ws = conv.from_bytes(reinterpret_cast<const char*> (&s[0]), reinterpret_cast<const char*> (&s[0] + s.size()));

		return ws;
	}

};
