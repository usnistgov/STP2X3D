#pragma once

class Component;

class Model
{
public:
	Model(void);
	~Model(void);

	void AddRootComponent(Component* comp) { m_rootComponents.push_back(comp); }
	Component* GetRootComponentAt(int index) const { return m_rootComponents[index]; }
	int GetRootComponentSize(void) const { return (int)m_rootComponents.size(); }

	void GetAllComponents(vector<Component*>& comps) const;
	Bnd_Box GetBoundingBox(bool sketch) const;
	ShapeType GetShapeType(void) const;

	bool IsEmpty(void) const;

	void Update(void);
	void Clear(void);

protected:
	void Clean(void);

	void UpdateNames(void) const;
	void UpdateComponentNames(void) const;
	void UpdateIShapeNames(void) const;
	void UpdateGlobalIShapeIndex(void) const;

private:
	vector<Component*> m_rootComponents;
};