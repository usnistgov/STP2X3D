#pragma once

class Mesh;
class IShape;

class GDT_Item
{
public:
	GDT_Item(const wstring& name);
	~GDT_Item(void);

	const wstring& GetName(void) const { return m_name; }

	void AddShape(const TopoDS_Shape& shape);
	const TopoDS_Shape& GetShapeAt(int index) const { return m_shapes[index]; }
	const int GetShapeSize(void) const { return (int)m_shapes.size(); }

	void AddMesh(Mesh*& mesh) { m_meshList.push_back(mesh); }
	Mesh* GetMeshAt(int index) const { return m_meshList[index]; }
	const int GetMeshSize(void) const { return (int)m_meshList.size(); }
	
protected:
	void Clear(void);

private:
	wstring m_name;
	vector<TopoDS_Shape> m_shapes;
	vector<Mesh*> m_meshList;
};
