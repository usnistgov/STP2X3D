#pragma once

typedef array<int, 3> TriIndex;
typedef vector<int> EdgeIndex;

class Mesh 
{
public:
	Mesh(const TopoDS_Shape& shape);
	Mesh(const Mesh& mesh);
	~Mesh(void);

	void AddFaceIndex(int v1, int v2, int v3);
	void AddNormalIndex(int v1, int v2, int v3);
	void AddEdgeIndex(EdgeIndex edgeIndex);
	void AddCoordinate(const gp_XYZ& coord) { m_coordinates.push_back(coord); }
	void AddNormal(const gp_XYZ& norm) { m_normals.push_back(norm); }

	void ReserveCoordinates(int count) { m_coordinates.reserve(count); }
	void ReserveNormals(int count) { m_normals.reserve(count); }
	void ReserveFaceIndexes(int count) { m_faceIndexes.reserve(count); }
	void ReserveNormalIndexes(int count) { m_normalIndexes.reserve(count); }
	void ReserveEdgeIndexes(int count) { m_edgeIndexes.reserve(count); }

	const TopoDS_Shape& GetShape(void) const { return m_shape; }
	const TriIndex& GetFaceIndexAt(int index) const { return m_faceIndexes[index]; }
	const TriIndex& GetNormalIndexAt(int index) const { return m_normalIndexes[index]; }
	const EdgeIndex& GetEdgeIndexAt(int index) const { return m_edgeIndexes[index]; }
	const gp_XYZ& GetCoordinateAt(int index) const { return m_coordinates[index]; }
	const gp_XYZ& GetNormalAt(int index) const { return m_normals[index]; }

	const int GetFaceIndexSize(void) const { return (int)m_faceIndexes.size(); }
	const int GetNormalIndexSize(void) const { return (int)m_normalIndexes.size(); }
	const int GetEdgeIndexSize(void) const { return (int)m_edgeIndexes.size(); }
	const int GetCoordinateSize(void) const { return (int)m_coordinates.size(); }
	const int GetNormalSize(void) const { return (int)m_normals.size(); }

	bool IsEmpty(void) const;

protected:
	void Clear(void);

private:
	TopoDS_Shape m_shape;

	vector<gp_XYZ> m_coordinates;
	vector<gp_XYZ> m_normals;

	vector<TriIndex> m_faceIndexes;
	vector<TriIndex> m_normalIndexes;
	vector<EdgeIndex> m_edgeIndexes;
};
