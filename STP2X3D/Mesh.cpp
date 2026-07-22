#include "stdafx.h"
#include "Mesh.h"


Mesh::Mesh(const TopoDS_Shape& shape)
	: m_shape(shape)
{
}

Mesh::Mesh(const Mesh& mesh)
	: m_shape(mesh.m_shape)
	, m_coordinates(mesh.m_coordinates)
	, m_normals(mesh.m_normals)
	, m_faceIndexes(mesh.m_faceIndexes)
	, m_normalIndexes(mesh.m_normalIndexes)
	, m_edgeIndexes(mesh.m_edgeIndexes)
{
}

Mesh::~Mesh(void)
{
	Clear();
}

void Mesh::AddFaceIndex(int v1, int v2, int v3)
{
	m_faceIndexes.push_back({ v1, v2, v3 });
}

void Mesh::AddNormalIndex(int v1, int v2, int v3)
{
	m_normalIndexes.push_back({ v1, v2, v3 });
}

void Mesh::AddEdgeIndex(EdgeIndex edgeIndex)
{
	m_edgeIndexes.push_back(std::move(edgeIndex));
}

bool Mesh::IsEmpty(void) const
{
	if (m_coordinates.empty())
		return true;

	return false;
}

void Mesh::Clear(void)
{
	m_faceIndexes.clear();
	m_normalIndexes.clear();
	m_edgeIndexes.clear();
	m_coordinates.clear();
	m_normals.clear();
}
