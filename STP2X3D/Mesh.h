#pragma once

typedef vector<int> Index;

class Mesh 
{
public:
	Mesh(const TopoDS_Shape& shape);
	~Mesh(void);

	void AddFaceIndex(const int& v1, const int& v2, const int& v3);
	void AddNormalIndex(const int& v1, const int& v2, const int& v3);
	void AddEdgeIndex(const Index& edgeIndex) { m_edgeIndexes.push_back(edgeIndex); }
	void AddCoordinate(const gp_XYZ& coord) { m_coordinates.push_back(coord); }
	void AddNormal(const gp_XYZ& norm) { m_normals.push_back(norm); }

	TopoDS_Shape GetShape(void) const { return m_shape; }
	Index GetFaceIndexAt(int index) const { return m_faceIndexes[index]; }
	Index GetNormalIndexAt(int index) const { return m_normalIndexes[index]; }
	Index GetEdgeIndexAt(int index) const { return m_edgeIndexes[index]; }
	gp_XYZ GetCoordinateAt(int index) const { return m_coordinates[index]; }
	gp_XYZ GetNormalAt(int index) const { return m_normals[index]; }

	int GetFaceIndexSize(void) const { return (int)m_faceIndexes.size(); }
	int GetNormalIndexSize(void) const { return (int)m_normalIndexes.size(); }
	int GetEdgeIndexSize(void) const { return (int)m_edgeIndexes.size(); }
	int GetCoordinateSize(void) const { return (int)m_coordinates.size(); }
	int GetNormalSize(void) const { return (int)m_normals.size(); }

	bool IsEmpty(void) const;

protected:
	void Clear(void);

private:
	TopoDS_Shape m_shape;

	vector<gp_XYZ> m_coordinates;
	vector<gp_XYZ> m_normals;

	vector<Index> m_faceIndexes;
	vector<Index> m_normalIndexes;
	vector<Index> m_edgeIndexes;
};