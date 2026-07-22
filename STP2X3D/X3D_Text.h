#pragma once

#include <charconv>
#include <cwchar>
#include <fstream>
#include <string>

// Buffered UTF-8 text writer for X3D/HTML output.
// Replaces wofstream + codecvt, which converts every wide character
// through a locale facet and dominates write time on large models.
// The file is opened in text mode so "\n" is translated to "\r\n" on
// Windows, matching the previous wofstream output byte for byte.
class X3D_Text
{
public:
	explicit X3D_Text(const std::wstring& filePath)
	{
		m_out.open(filePath.c_str(), std::ios::out);
		m_buffer.reserve(FlushThreshold + 4096);

		// codecvt_utf8 with generate_header wrote a UTF-8 BOM.
		if (m_out.is_open())
			m_buffer.append("\xEF\xBB\xBF");
	}

	~X3D_Text(void)
	{
		Flush();
	}

	X3D_Text(const X3D_Text&) = delete;
	X3D_Text& operator=(const X3D_Text&) = delete;

	bool IsOpen(void) const { return m_out.is_open(); }

	bool Good(void) const { return m_out.good(); }

	bool Close(void)
	{
		Flush();
		const bool ok = !m_out.is_open() || m_out.good();
		if (m_out.is_open())
			m_out.close();
		return ok && !m_failed;
	}

	X3D_Text& operator<<(const char* text)
	{
		m_buffer.append(text);
		FlushIfNeeded();
		return *this;
	}

	X3D_Text& operator<<(const std::string& text)
	{
		m_buffer.append(text);
		FlushIfNeeded();
		return *this;
	}

	X3D_Text& operator<<(const wchar_t* text)
	{
		AppendWide(text, wcslen(text));
		FlushIfNeeded();
		return *this;
	}

	X3D_Text& operator<<(const std::wstring& text)
	{
		AppendWide(text.c_str(), text.size());
		FlushIfNeeded();
		return *this;
	}

	X3D_Text& operator<<(int value)
	{
		char buf[16];
		const auto result = std::to_chars(buf, buf + sizeof(buf), value);
		m_buffer.append(buf, result.ptr);
		FlushIfNeeded();
		return *this;
	}

private:
	static constexpr size_t FlushThreshold = 4u << 20; // 4 MB

	void AppendWide(const wchar_t* text, size_t size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			char32_t cp = static_cast<char32_t>(text[i]);

			// Combine UTF-16 surrogate pairs when wchar_t is 16-bit.
			if (sizeof(wchar_t) == 2
				&& cp >= 0xD800 && cp <= 0xDBFF
				&& i + 1 < size)
			{
				const char32_t low = static_cast<char32_t>(text[i + 1]);
				if (low >= 0xDC00 && low <= 0xDFFF)
				{
					cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
					++i;
				}
			}

			if (cp < 0x80)
				m_buffer.push_back(static_cast<char>(cp));
			else if (cp < 0x800)
			{
				m_buffer.push_back(static_cast<char>(0xC0 | (cp >> 6)));
				m_buffer.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
			}
			else if (cp < 0x10000)
			{
				m_buffer.push_back(static_cast<char>(0xE0 | (cp >> 12)));
				m_buffer.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
				m_buffer.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
			}
			else
			{
				m_buffer.push_back(static_cast<char>(0xF0 | (cp >> 18)));
				m_buffer.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
				m_buffer.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
				m_buffer.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
			}
		}
	}

	void FlushIfNeeded(void)
	{
		if (m_buffer.size() >= FlushThreshold)
			Flush();
	}

	void Flush(void)
	{
		if (m_out.is_open() && !m_buffer.empty())
		{
			m_out.write(m_buffer.data(), static_cast<std::streamsize>(m_buffer.size()));
			if (!m_out.good())
				m_failed = true;
			m_buffer.clear();
		}
	}

	std::string m_buffer;
	std::ofstream m_out;
	bool m_failed = false;
};
