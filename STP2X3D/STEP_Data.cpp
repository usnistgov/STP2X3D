#include "stdafx.h"
#include "STEP_Data.h"
#include <StepData_UndefinedEntity.hxx>
#include <Interface_UndefinedContent.hxx>
#include <Interface_EntityList.hxx>
#include <StepGeom_CompositeCurve.hxx>
#include <StepGeom_CompositeCurveSegment.hxx>
#include <StepGeom_HArray1OfCompositeCurveSegment.hxx>
#include <StepToTopoDS_TranslateCompositeCurve.hxx>

STEP_Data::STEP_Data(const STEPControl_Reader& reader)
{
	StoreStepDataForShapes(reader);
}

STEP_Data::~STEP_Data(void)
{
	Clear();
}

void STEP_Data::StoreStepDataForShapes(const STEPControl_Reader& reader)
{
	const Handle(XSControl_WorkSession)& workSession = reader.WS();
	const Handle(Interface_InterfaceModel)& model = workSession->Model();
	const Handle(XSControl_TransferReader)& transferReader = workSession->TransferReader();
	const Handle(Transfer_TransientProcess)& transProcess = transferReader->TransientProcess();
	
	int entitySize = model->NbEntities();

	// Store STEP data for shapes
	for (int i = 1; i <= entitySize; ++i)
	{
		const Handle(Standard_Transient)& entity = model->Value(i);
		StoreHiddenShape(entity, workSession, transProcess);

		Handle(StepRepr_RepresentationItem) repItem = Handle(StepRepr_RepresentationItem)::DownCast(entity);
		
		// Skip if entity is not of type representation_item
		if (repItem.IsNull())
			continue;

		const Handle(Transfer_Binder)& binder = transProcess->Find(entity);
		const TopoDS_Shape& shape = TransferBRep::ShapeResult(binder);

		// Skip if shape is null
		if (shape.IsNull())
			continue;
		
		TopAbs_ShapeEnum shapeType = shape.ShapeType();

		// Store compounds, compsolids, solids, and shells
		// Ignore lower-level elements such as faces
		if (shapeType == TopAbs_COMPOUND
			|| shapeType == TopAbs_COMPSOLID
			|| shapeType == TopAbs_SOLID
			|| shapeType == TopAbs_SHELL)
		{
			int shapeID = OCCUtil::GetID(shape);

			// Entity type
			string stepEntityType = entity->DynamicType()->Name();
			m_shapeIDentityTypeMap.insert({ shapeID,stepEntityType });

			// Entity ID
			string id(workSession->EntityLabel(entity)->ToCString());
			int stepEntityID = atoi(id.substr(1, id.length()).c_str());
			m_shapeIDentityIDMap.insert({ shapeID,stepEntityID });
		}
	}
}

void STEP_Data::StoreHiddenShape(const Handle(Standard_Transient)& entity, const Handle(XSControl_WorkSession)& workSession, const Handle(Transfer_TransientProcess)& transProcess)
{
	string stepEntityType = entity->DynamicType()->Name();

	if (stepEntityType == "StepData_UndefinedEntity")
	{
		Handle(StepData_UndefinedEntity) undefEnt = Handle(StepData_UndefinedEntity)::DownCast(entity);

		if (undefEnt->IsComplex())
		{
			bool isCURVE_11 = false; // Rosette used in Composite Design
			
			while (undefEnt)
			{
				string type = undefEnt->StepType();

				if (type == "CURVE_11")
				{
					isCURVE_11 = true;
					break;
				}

				undefEnt = undefEnt->Next();
			}

			// Check CURVE_11
			if (isCURVE_11)
				undefEnt = Handle(StepData_UndefinedEntity)::DownCast(entity);
			else
				return;

			// Generate a composite_curve
			while (undefEnt)
			{
				string type = undefEnt->StepType();

				if (type == "COMPOSITE_CURVE")
				{
					Handle(Interface_UndefinedContent) intUndefCont = undefEnt->UndefinedContent();
					Interface_EntityList entList = intUndefCont->EntityList();
					Handle(Standard_Transient) ent = entList.Value(1);

					Handle(StepData_UndefinedEntity) undefEnt2 = Handle(StepData_UndefinedEntity)::DownCast(ent);
					intUndefCont = undefEnt2->UndefinedContent();

					entList = intUndefCont->EntityList();

					int entCount = entList.NbEntities();
					StepGeom_HArray1OfCompositeCurveSegment* compCurvSegArr = new StepGeom_HArray1OfCompositeCurveSegment(1, entCount);

					for (int i = 1; i <= entCount; ++i)
					{
						Handle(Standard_Transient) ent = entList.Value(i);
						Handle(StepGeom_CompositeCurveSegment) compCurvSeg = Handle(StepGeom_CompositeCurveSegment)::DownCast(ent);

						compCurvSegArr->SetValue(i, compCurvSeg);
					}

					Handle(StepGeom_HArray1OfCompositeCurveSegment) h_compCurvSegArr = Handle(StepGeom_HArray1OfCompositeCurveSegment)(compCurvSegArr);

					StepGeom_CompositeCurve* compCurv = new StepGeom_CompositeCurve();
					compCurv->SetSegments(h_compCurvSegArr);
					Handle(StepGeom_CompositeCurve) h_compCurv = Handle(StepGeom_CompositeCurve)(compCurv);

					StepToTopoDS_TranslateCompositeCurve* compCurvTrans = new StepToTopoDS_TranslateCompositeCurve(h_compCurv, transProcess);
					TopoDS_Shape shape = (TopoDS_Shape)compCurvTrans->Value();

					m_hiddenShapes.push_back(shape);

					int shapeID = OCCUtil::GetID(shape);

					string id(workSession->EntityLabel(entity)->ToCString());
					int stepEntityID = atoi(id.substr(1, id.length()).c_str());
					m_shapeIDentityIDMap.insert({ shapeID,stepEntityID });

					break;
				}

				undefEnt = undefEnt->Next();
			}
		}
	}
}

const string STEP_Data::GetEntityTypeFromShape(const TopoDS_Shape& shape) const
{
	int shapeID = OCCUtil::GetID(shape);
	string stepEntityType;

	if (m_shapeIDentityTypeMap.find(shapeID) != m_shapeIDentityTypeMap.end())
		stepEntityType = m_shapeIDentityTypeMap.find(shapeID)->second;

	return stepEntityType;
}

const int STEP_Data::GetEntityIDFromShape(const TopoDS_Shape& shape) const
{
	int shapeID = OCCUtil::GetID(shape);
	int stepEntityID = -1;

	if (m_shapeIDentityIDMap.find(shapeID) != m_shapeIDentityIDMap.end())
		stepEntityID = m_shapeIDentityIDMap.find(shapeID)->second;

	return stepEntityID;
}

bool STEP_Data::IsSolidModel(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);
	
	if (type == "StepShape_ManifoldSolidBrep"	// MANIFOLD_SOLID_BREP in STEP
		|| type == "StepShape_BrepWithVoids"	// BREP_WITH_VOIDS in STEP
		|| type == "StepShape_FacetedBrep")		// FACETED_BREP in STEP
		return true;

	return false;
}

bool STEP_Data::IsSurfaceModel(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);

	if (type == "StepShape_ShellBasedSurfaceModel"	// SHELL_BASED_SURFACE_MODEL in STEP
		|| type == "StepShape_GeometricSet")		// GEOMETRIC_SET in STEP
		return true;

	return false;
}

bool STEP_Data::IsWireframeModel(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);

	if (type == "StepShape_GeometricCurveSet"		// GEOMETRIC_CURVE_SET in STEP
		|| type == "StepShape_GeometricSet")		// GEOMETRIC_SET in STEP
		return true;

	return false;
}

bool STEP_Data::IsShell(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);

	if (type == "StepShape_ClosedShell"				// CLOSED_SHELL in STEP
		|| type == "StepShape_OrientedClosedShell"	// ORIENTED_CLOSED_SHELL in STEP
		|| type == "StepShape_OpenShell")			// OPEN_SHELL in STEP
		return true;

	return false;
}

bool STEP_Data::IsMappedItem(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);

	if (type == "StepRepr_MappedItem")	// MAPPED_ITEM in STEP
		return true;

	return false;
}

void STEP_Data::Clear(void)
{
	m_shapeIDentityTypeMap.clear();
	m_shapeIDentityIDMap.clear();
}