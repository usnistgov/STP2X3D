#pragma once

class Component;
class GDT_Item;

class Model
{
public:
	Model(void);
	~Model(void);

	void AddRootComponent(Component*& comp) { m_rootComponents.push_back(comp); }
	Component* GetRootComponentAt(int index) const { return m_rootComponents[index]; }
	const int GetRootComponentSize(void) const { return (int)m_rootComponents.size(); }

	void GetAllComponents(vector<Component*>& comps) const;
	const Bnd_Box GetBoundingBox(bool sketch) const;
	const ShapeType GetShapeType(void) const;

	bool IsEmpty(void) const;

	void Update(void);
	void Clear(void);

	// GD&T
	void AddGDT(GDT_Item*& gdt_item) { m_gdts.push_back(gdt_item); }
	const int GetGDTSize(void) const { return (int)m_gdts.size(); }
	GDT_Item* GetGDTAt(int index) const { return m_gdts[index]; }
	GDT_Item* GetGDTByName(wstring name) const;


protected:
	void Clean(void);

	void UpdateNames(void) const;
	void UpdateComponentNames(void) const;
	void UpdateIShapeNames(void) const;
	void UpdateGlobalIShapeIndex(void) const;

private:
	vector<Component*> m_rootComponents;
	vector<GDT_Item*> m_gdts;
};