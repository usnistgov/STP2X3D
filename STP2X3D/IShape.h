#pragma once

class Component;
class Mesh;

class IShape
{
public:
	IShape(const TopoDS_Shape& shape);
	~IShape(void);

	void SetName(const wstring& name) { m_name = name; }
	void SetComponent(Component* comp) { m_component = comp; }
	void SetGlobalIndex(int globalIndex) { m_globalIndex = globalIndex; }
	void SetTessellated(bool isTessellated) { m_isTessellated = isTessellated; }

	void AddColor(const TopoDS_Shape& shape, const Quantity_ColorRGBA& color);
	void AddMesh(Mesh* mesh) { m_meshList.push_back(mesh); }

	wstring GetName(void) const { return m_name; }
	Component* GetComponent(void) const { return m_component; }
	TopoDS_Shape GetShape(void) const { return m_shape; }
	Quantity_ColorRGBA GetColor(const TopoDS_Shape& shape) const;
	Quantity_ColorRGBA GetColor(void) const { return m_colorList.at(0); }

	Mesh* GetMeshAt(int index) const { return m_meshList[index]; }
	int GetGlobalIndex(void) const { return m_globalIndex; }

	int GetMeshSize(void) const { return (int)m_meshList.size(); }

	bool IsMultiColored(void) const { return m_isMultiColored; }
	bool IsMultiTransparent(void) const { return m_isMultiTransparent; }
	bool IsSingleTransparent(void) const;
	bool IsTransparent(void) const { return m_isTransparent; }
	bool IsTessellated(void) const { return m_isTessellated; }

	bool IsFaceSet(void) const { return m_isFaceSet; }
	bool IsSketchGeometry(void) const { return !m_isFaceSet; }

	bool IsEmpty(void) const;
	
	// SFA-specific
	wstring GetUniqueName(void) const;

protected:
	void CheckColor(const Quantity_ColorRGBA& color);
	void Clear(void);

private:
	wstring m_name;
	TopoDS_Shape m_shape;
	int m_globalIndex;

	bool m_isMultiColored;
	bool m_isMultiTransparent;
	bool m_isTransparent;
	bool m_isTessellated;
	bool m_isFaceSet;

	Component* m_component;
	
	vector<Mesh*> m_meshList;
	vector<Quantity_ColorRGBA> m_colorList;
	map<int, Quantity_ColorRGBA> m_shapeIDcolorMap;
};