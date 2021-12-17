#include "stdafx.h"
#include "Mesh.h"


Mesh::Mesh(const TopoDS_Shape& shape)
	: m_shape(shape)
{
}

Mesh::~Mesh(void)
{
	Clear();
}

void Mesh::AddFaceIndex(int v1, int v2, int v3)
{
	Index m_index;
	m_index.push_back(v1);
	m_index.push_back(v2);
	m_index.push_back(v3);

	m_faceIndexes.push_back(m_index);
}

void Mesh::AddNormalIndex(int v1, int v2, int v3)
{
	Index m_index;
	m_index.push_back(v1);
	m_index.push_back(v2);
	m_index.push_back(v3);

	m_normalIndexes.push_back(m_index);
}

void Mesh::AddEdgeIndex(Index edgeIndex)
{
	m_edgeIndexes.push_back(edgeIndex);
}

bool Mesh::IsEmpty(void) const
{
	if (m_coordinates.empty())
		return true;

	return false;
}

void Mesh::Clear(void)
{
	for (auto faceIndex : m_faceIndexes)
		faceIndex.clear();

	for (auto normalIndex : m_normalIndexes)
		normalIndex.clear();
	
	for (auto edgeIndex : m_edgeIndexes)
		edgeIndex.clear();

	m_faceIndexes.clear();
	m_normalIndexes.clear();
	m_edgeIndexes.clear();
	m_coordinates.clear();
	m_normals.clear();
}