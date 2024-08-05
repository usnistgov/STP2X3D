#include "stdafx.h"
#include "Component.h"
#include "IShape.h"


Component::Component(const TopoDS_Shape& shape)
	: m_parentComponent(nullptr),
	m_originalComponent(nullptr),
	m_hasUniqueName(false),
	m_shape(shape),
	m_stepID(-1)
{
}

Component::~Component(void)
{
	Clear();
}

void Component::SetUniqueName(const wstring& name)
{
	if (name.empty())
		return;

	m_name = name;
	m_uniqueName = name;
	m_hasUniqueName = true;
}

void Component::SetOriginalComponent(Component*& originalComp)
{
	m_originalComponent = originalComp;

	if (originalComp)
		originalComp->AddCopiedComponent(this);
}

void Component::AddSubComponent(Component*& subComp)
{
	m_subComponents.push_back(subComp);
	subComp->SetParentComponent(this);
}

void Component::AddIShape(IShape*& iShape)
{
	m_iShapes.push_back(iShape);
	iShape->SetComponent(this);
}

void Component::GetAllComponents(vector<Component*>& comps) const
{
	// Traverse sub components recursively
	for (int i = 0; i < GetSubComponentSize(); ++i)
	{
		Component* subComp = GetSubComponentAt(i);
		comps.push_back(subComp);

		subComp->GetAllComponents(comps);
	}
}

void Component::GetLeafComponents(vector<Component*>& comps)
{
	if (IsLeaf())
		comps.push_back(this);

	// Traverse sub components recursively
	for (int i = 0; i < GetSubComponentSize(); ++i)
	{
		Component* subComp = GetSubComponentAt(i);
		subComp->GetLeafComponents(comps);
	}
}

void Component::Clean(void)
{
	CleanEmptyIShapes();
	CleanEmptySubComponents();
	CleanUselessSubComponents();
}

void Component::CleanEmptyIShapes(void)
{
	for (int i = 0; i < GetSubComponentSize(); ++i)
	{
		Component* subComp = GetSubComponentAt(i);
		subComp->CleanEmptyIShapes();
	}
	
	int iShapeSize = GetIShapeSize();

	for (int i = iShapeSize - 1; i >= 0; --i)
	{
		IShape* iShape = GetIShapeAt(i);

		// Skip if the shape is not tessellated yet
		if (!iShape->IsTessellated())
			continue;

		// Remove empty IShapes after tessellation
		if (iShape->IsEmpty())
		{
			m_iShapes.erase(m_iShapes.begin() + i);
			delete iShape;
		}
	}
}

void Component::CleanEmptySubComponents(void)
{
	int subCompSize = GetSubComponentSize();

	for (int i = subCompSize - 1; i >= 0; --i)
	{
		Component* subComp = GetSubComponentAt(i);
		subComp->CleanEmptySubComponents();

		// Skip if the subcomp is a copy
		if (subComp->IsCopy()
			&& !subComp->GetOriginalComponent()->IsEmpty())
			continue;

		// Remove subcomponents having neither IShape nor child
		if (subComp->IsEmpty())
		{
			m_subComponents.erase(m_subComponents.begin() + i);
			delete subComp;
		}
	}
}

void Component::CleanUselessSubComponents(void)
{
	for (int i = 0; i < GetSubComponentSize(); ++i)
	{
		Component* subComp = GetSubComponentAt(i);
		subComp->CleanUselessSubComponents();
	}

	// If a component has only one subcomp, remove the subcomp
	// Since it has no meaning in terms of structure
	if (GetSubComponentSize() == 1)
	{
		Component* subComp = GetSubComponentAt(0);

		// Skip if the subcomp has a unique name (not empty)
		// Since this structure may have been built on purpose
		if (subComp->HasUniqueName())
			return;

		// Clear subcomps
		ClearSubComponents();

		// Set multiplied transformation
		gp_Trsf trsf = subComp->GetTransformation();
		trsf = trsf.Multiplied(GetTransformation());
		SetTransformation(trsf);

		// Migrate all IShapes of the subcomp to the comp
		for (int i = 0; i < subComp->GetIShapeSize(); ++i)
		{
			IShape* shape = subComp->GetIShapeAt(i);
			AddIShape(shape);
		}

		// Migrate all subcomps of the subcomp to the comp
		for (int i = 0; i < subComp->GetSubComponentSize(); ++i)
		{
			Component* comp = subComp->GetSubComponentAt(i);
			AddSubComponent(comp);
		}
		
		// Delete the subcomp
		subComp->ClearIShapes();
		subComp->ClearSubComponents();
		delete subComp;
	}
}

bool Component::IsCopy(void) const
{
	if (m_originalComponent)
		return true;

	return false;
}

bool Component::IsAssembly(void) const
{
	if (GetSubComponentSize() > 0)
		return true;
	
	return false;
}

bool Component::IsRoot(void) const
{
	if (!m_parentComponent)
		return true;

	return false;
}

bool Component::IsEmpty(void) const
{
	if (GetIShapeSize() == 0
		&& GetSubComponentSize() == 0)
		return true;

	return false;
}

bool Component::IsLeaf(void) const
{
	if (GetSubComponentSize() == 0)
		return true;

	return false;
}

bool Component::HasRosette(void) const
{
	for (const auto& iShape : m_iShapes)
	{
		if (iShape->IsRosette())
			return true;
	}

	return false;
}

bool Component::HasSectionCap(void) const
{
	for (const auto& iShape : m_iShapes)
	{
		if (iShape->IsSectionCap())
			return true;
	}

	return false;
}


const Bnd_Box Component::GetBoundingBox(bool sketch) const
{
	Bnd_Box bndBox;

	// Add sub bounding boxes for subComps
	for (const auto& subComp : m_subComponents)
	{
		const gp_Trsf& trsf = subComp->GetTransformation();
		Bnd_Box subBndBox;

		if (subComp->IsCopy())
			subBndBox = subComp->GetOriginalComponent()->GetBoundingBox(sketch).Transformed(trsf);
		else
			subBndBox = subComp->GetBoundingBox(sketch).Transformed(trsf);

		bndBox.Add(subBndBox);
	}

	// Add sub bounding boxes for iShapes
	for (const auto& iShape : m_iShapes)
	{
		bool isSketchGeometry = iShape->IsSketchGeometry();

		// Skip sketchGeometry when the sketch option is off
		if (!sketch
			&& isSketchGeometry)
			continue;

		const TopoDS_Shape& shape = iShape->GetShape();

		const Bnd_Box& subBndBox = OCCUtil::ComputeBoundingBox(shape);
		bndBox.Add(subBndBox);
	}
	
	// Get the finite bounding box (mandatory)
	bndBox = bndBox.FinitePart();

	return bndBox;
}

TopoDS_Shape Component::GetTransformedShape(void)
{
	TopoDS_Shape shape = GetShape();
	TopoDS_Shape copiedShape = OCCUtil::GetCopiedShape(shape);

	gp_Trsf trsf = GetTransformation();
	Component* temp = this;

	while (!temp->IsRoot())
	{
		temp = temp->GetParentComponent();
		gp_Trsf tempTrsf = temp->GetTransformation();

		trsf = trsf.Multiplied(tempTrsf);
	}

	TopoDS_Shape trsfShape = OCCUtil::TransformShape(copiedShape, trsf);

	return trsfShape;
}

void Component::Clear(void)
{
	for (auto subComp : m_subComponents)
		delete subComp;
	
	m_subComponents.clear();

	for (auto iShape : m_iShapes)
		delete iShape;
	
	m_iShapes.clear();

	//for (auto copiedComp : m_copiedComponents)
	//	copiedComp->SetOriginalComponent(nullptr);

	m_copiedComponents.clear();
}