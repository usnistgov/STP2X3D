#include "stdafx.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"


IShape::IShape(const TopoDS_Shape& shape)
	: m_shape(shape),
	m_isTessellated(false),
	m_isMultiColored(false),
	m_isMultiTransparent(false),
	m_isTransparent(false),
	m_isFaceSet(false),
	m_component(nullptr),
	m_globalIndex(0)
{
	// Check if the shape is a face set
	if (OCCUtil::HasFace(m_shape))
		m_isFaceSet = true;
}

IShape::~IShape(void)
{
	Clear();
}

void IShape::AddColor(const TopoDS_Shape& shape, const Quantity_ColorRGBA& color)
{
	int shapeID = OCCUtil::GetID(shape);
	m_shapeIDcolorMap.insert({ shapeID,color });
	
	CheckColor(color);
}

Quantity_ColorRGBA IShape::GetColor(const TopoDS_Shape& shape) const
{
	int shapeID = OCCUtil::GetID(shape);
	Quantity_ColorRGBA color = m_shapeIDcolorMap.find(shapeID)->second;

	return color;
}

bool IShape::IsSingleTransparent(void) const
{
	if (IsTransparent()
		&& !IsMultiTransparent())
		return true;

	return false;
}

bool IShape::IsEmpty(void) const
{
	if (GetMeshSize() == 0)
		return true;

	return false;
}

wstring IShape::GetUniqueName(void) const
{
	// Get the closest component with a unique name
	Component* comp = GetComponent();
	wstring uniqueName = comp->GetUniqueName();

	while (!comp->IsRoot() 
		&& !comp->HasUniqueName())
	{
		comp = comp->GetParentComponent();
		uniqueName = comp->GetUniqueName();
	}

	return uniqueName;
}

void IShape::CheckColor(const Quantity_ColorRGBA& color)
{
	m_colorList.push_back(color);
	int colorSize = (int)m_colorList.size();

	// Check if transparent
	if (!m_isTransparent)
	{
		if (color.Alpha() < 1.0)
			m_isTransparent = true;
	}

	// Check if multi-colored or multi-transparent
	if (colorSize >= 2
		&& (!m_isMultiColored 
			|| !m_isMultiTransparent))
	{
		const Quantity_ColorRGBA& pre = m_colorList[colorSize - 2];
		const Quantity_ColorRGBA& cur = m_colorList[colorSize - 1];

		if (!m_isMultiColored
			&& pre.GetRGB() != cur.GetRGB())
			m_isMultiColored = true;

		if (!m_isMultiTransparent
			&& pre.Alpha() != cur.Alpha())
			m_isMultiTransparent = true;
	}
}

void IShape::Clear(void)
{
	for (auto mesh : m_meshList)
		delete mesh;

	m_meshList.clear();
	m_colorList.clear();
	m_shapeIDcolorMap.clear();
}