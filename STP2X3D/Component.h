#pragma once

class IShape;

class Component
{
public:
	Component(const TopoDS_Shape& shape);
	~Component(void);

	void SetName(const wstring& name) { m_name = name; }
	void SetUniqueName(const wstring& name);
	void SetTransformation(const gp_Trsf& trsf) { m_trsf = trsf; }
	void SetParentComponent(Component* parentComp) { m_parentComponent = parentComp; }
	void SetOriginalComponent(Component*& originalComp);
	void SetShape(const TopoDS_Shape& shape) { m_shape = shape; }

	void AddSubComponent(Component*& subComp);
	void AddIShape(IShape*& iShape);

	const wstring& GetName(void) const { return m_name; }
	const wstring& GetUniqueName(void) const { return m_uniqueName; }
	const gp_Trsf& GetTransformation(void) const { return m_trsf; }
	const TopoDS_Shape& GetShape(void) const { return m_shape; }
	Component* GetParentComponent(void) const { return m_parentComponent; }
	Component* GetOriginalComponent(void) const { return m_originalComponent; }
	Component* GetSubComponentAt(const int index) const { return m_subComponents[index]; }
	IShape* GetIShapeAt(const int index) const { return m_iShapes[index]; }
	
	const int GetSubComponentSize(void) const { return (int)m_subComponents.size(); }
	const int GetIShapeSize(void) const { return (int)m_iShapes.size(); }

	void GetAllComponents(vector<Component*>& comps) const;
	void GetLeafComponents(vector<Component*>& comps);
	const Bnd_Box GetBoundingBox(bool sketch) const;

	TopoDS_Shape GetTransformedShape(void);

	bool HasUniqueName(void) const { return m_hasUniqueName; }
	bool IsCopy(void) const;
	bool IsAssembly(void) const;
	bool IsRoot(void) const;
	bool IsEmpty(void) const;
	bool IsLeaf(void) const;
	bool HasRosette(void) const;
	bool HasSectionCap(void) const;

	void Clean(void);

	// SFA-specific
	void SetStepID(int stepID) { m_stepID = stepID; }
	const int GetStepID(void) const { return m_stepID; }

protected:
	void AddCopiedComponent(Component* copiedComp) { m_copiedComponents.push_back(copiedComp); }
	Component* GetCopiedComponentAt(int index) const { return m_copiedComponents[index]; }
	int GetCopiedComponentSize(void) const { return (int)m_copiedComponents.size(); }

protected:
	void Clear(void);

	void CleanEmptyIShapes(void);
	void CleanEmptySubComponents(void);
	void CleanUselessSubComponents(void);

	void ClearSubComponents(void) { m_subComponents.clear(); }
	void ClearIShapes(void) { m_iShapes.clear(); }

private:
	wstring m_name;
	wstring m_uniqueName;
	TopoDS_Shape m_shape;
	gp_Trsf m_trsf;
	bool m_hasUniqueName;
	int m_stepID;

	Component* m_originalComponent;
	Component* m_parentComponent;

	vector<Component*> m_subComponents;
	vector<Component*> m_copiedComponents;
	vector<IShape*> m_iShapes;
};