#include "stdafx.h"
#include "S2X_Option.h"


S2X_Option::S2X_Option()
    : m_input(L""),
    m_output(L""),
    m_normal(false),
    m_color(true),
    m_edge(false),
    m_sketch(true),
    m_html(false),
    m_tessellation(false),
    m_quality(5.0),
    m_batch(-1),
    m_SFA(true),
    m_gdt(false),
    m_rosette(false),
    m_cap(false),
    m_tessSolid(true)
{
}

S2X_Option::~S2X_Option()
{
}

void S2X_Option::SetNormal(const int& normal)
{
    if (normal == 0)
        m_normal = false;
    else
        m_normal = true;
}

void S2X_Option::SetColor(const int& color)
{
    if (color == 0)
        m_color = false;
    else
        m_color = true;
}

void S2X_Option::SetEdge(const int& edge)
{
    if (edge == 0)
        m_edge = false;
    else
        m_edge = true;
}

void S2X_Option::SetSketch(const int& sketch)
{
    if (sketch == 0)
        m_sketch = false;
    else
        m_sketch = true;
}

void S2X_Option::SetHtml(const int& html)
{
    if (html == 0)
        m_html = false;
    else
        m_html = true;
}

void S2X_Option::SetTessellation(const int& tessellation)
{
    if (tessellation == 0)
        m_tessellation = false;
    else
        m_tessellation = true;
}

const wstring S2X_Option::Output(void)
{
	wstring path = m_output;
	if (path.empty())
	{
		const size_t dot = m_input.find_last_of(L".");
		path = (dot == wstring::npos) ? m_input : m_input.substr(0, dot);
	}

	const wstring extension = m_html ? L".html" : L".x3d";
	auto endsWithIgnoreCase = [](const wstring& value, const wstring& suffix) -> bool
	{
		if (value.size() < suffix.size())
			return false;
		for (size_t i = 0; i < suffix.size(); ++i)
		{
			if (towlower(value[value.size() - suffix.size() + i]) != towlower(suffix[i]))
				return false;
		}
		return true;
	};

	if (!endsWithIgnoreCase(path, extension))
		path.append(extension);

	m_output = path;
	return m_output;
}