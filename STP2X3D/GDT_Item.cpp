#include "stdafx.h"
#include "GDT_Item.h"
#include "Mesh.h"

GDT_Item::GDT_Item(const wstring& name)
	:m_name(name)
{
}

GDT_Item::~GDT_Item(void)
{
	Clear();
}

void GDT_Item::AddShape(const TopoDS_Shape& shape)
{
	m_shapes.push_back(shape);
	m_shapes.erase(unique(m_shapes.begin(), m_shapes.end()), m_shapes.end());
}

void GDT_Item::Clear(void)
{
	for (auto mesh : m_meshList)
		delete mesh;

	m_meshList.clear();
	m_shapes.clear();
}