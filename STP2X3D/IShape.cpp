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
	m_isRosette(false),
	m_isSectionCap(false),
	m_isTessSolidModel(false),
	m_component(nullptr),
	m_globalIndex(0),
	m_stepID(-1)
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
	//m_shapeIDcolorMap.insert({ shapeID,color });
	
	if (m_shapeIDcolorMap.find(shapeID) == m_shapeIDcolorMap.end())
		m_shapeIDcolorMap.insert({ shapeID,color });
	else
		m_shapeIDcolorMap.find(shapeID)->second = color;

	CheckColor(color);
}

const Quantity_ColorRGBA& IShape::GetColor(const TopoDS_Shape& shape) const
{
	static const Quantity_ColorRGBA defaultColor(0.55f, 0.55f, 0.6f, 1.0f);

	int shapeID = OCCUtil::GetID(shape);
	const auto color = m_shapeIDcolorMap.find(shapeID);
	if (color == m_shapeIDcolorMap.end())
	{
		if (!m_colorList.empty())
			return m_colorList.front();
		return defaultColor;
	}

	return color->second;
}

const Quantity_ColorRGBA& IShape::GetColor(void) const
{
	static const Quantity_ColorRGBA defaultColor(0.55f, 0.55f, 0.6f, 1.0f);
	if (m_colorList.empty())
		return defaultColor;

	return m_colorList.front();
}

void IShape::EnsureDefaultColors(const Quantity_ColorRGBA& faceColor, const Quantity_ColorRGBA& wireColor)
{
	if (HasColor())
		return;

	const TopoDS_Shape& shape = GetShape();
	if (IsFaceSet())
	{
		bool added = false;
		for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
		{
			AddColor(expFace.Current(), faceColor);
			added = true;
		}
		if (!added)
			AddColor(shape, faceColor);
	}
	else
	{
		bool added = false;
		for (TopExp_Explorer expEdge(shape, TopAbs_EDGE); expEdge.More(); expEdge.Next())
		{
			AddColor(expEdge.Current(), wireColor);
			added = true;
		}
		if (!added)
			AddColor(shape, wireColor);
	}
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