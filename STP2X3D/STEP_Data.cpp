#include "stdafx.h"
#include "STEP_Data.h"



STEP_Data::STEP_Data(const STEPControl_Reader& reader)
	:m_reader(reader)
{
	StoreStepDataForShapes();
}

STEP_Data::~STEP_Data(void)
{
	Clear();
}

void STEP_Data::StoreStepDataForShapes()
{
	const Handle(XSControl_WorkSession)& workSession = m_reader.WS();
	const Handle(Interface_InterfaceModel)& model = workSession->Model();
	const Handle(XSControl_TransferReader)& transferReader = workSession->TransferReader();
	const Handle(Transfer_TransientProcess)& transProcess = transferReader->TransientProcess();
	
	int entitySize = model->NbEntities();

	// Store STEP data for shapes
	for (int i = 1; i <= entitySize; ++i)
	{
		const Handle(Standard_Transient)& entity = model->Value(i);
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

const string STEP_Data::GetEntityNameFromShape(const TopoDS_Shape& shape) const
{
	int shapeID = OCCUtil::GetID(shape);
	string stepEntityName = "";

	if (m_shapeIDentityNameMap.find(shapeID) != m_shapeIDentityNameMap.end())
		stepEntityName = m_shapeIDentityNameMap.find(shapeID)->second;

	return stepEntityName;
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

bool STEP_Data::IsTessellatedSolidModel(const TopoDS_Shape& shape) const
{
	string type = GetEntityTypeFromShape(shape);

	if (type == "StepVisual_TessellatedSolid")	// TESSELLATED_SOLID in STEP
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

void STEP_Data::AddRosetteGeometries(vector<TopoDS_Shape>& rosettes)
{
	const Handle(XSControl_WorkSession)& workSession = m_reader.WS();
	const Handle(Interface_InterfaceModel)& model = workSession->Model();
	const Handle(XSControl_TransferReader)& transferReader = workSession->TransferReader();
	const Handle(Transfer_TransientProcess)& transProcess = transferReader->TransientProcess();

	int entitySize = model->NbEntities();

	// Store STEP data for shapes
	for (int i = 1; i <= entitySize; ++i)
	{
		const Handle(Standard_Transient)& entity = model->Value(i);
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
						StepData_Factors* factors = new StepData_Factors();
						StepToTopoDS_TranslateCompositeCurve* compCurvTrans = new StepToTopoDS_TranslateCompositeCurve(h_compCurv, transProcess, *factors);
						TopoDS_Shape shape = (TopoDS_Shape)compCurvTrans->Value();

						if (!shape.IsNull())
						{
							rosettes.push_back(shape);

							int shapeID = OCCUtil::GetID(shape);

							string id(workSession->EntityLabel(entity)->ToCString());
							int stepEntityID = atoi(id.substr(1, id.length()).c_str());
							m_shapeIDentityIDMap.insert({ shapeID,stepEntityID });
						}

						break;
					}

					undefEnt = undefEnt->Next();
				}
			}
		}
	}
}

void STEP_Data::AddSectionPlanes(vector<TopoDS_Shape>& sectionPlanes)
{
	const Handle(XSControl_WorkSession)& workSession = m_reader.WS();
	const Handle(Interface_InterfaceModel)& model = workSession->Model();

	int entitySize = model->NbEntities();

	double factor = GetUnitConversionFactor(); // milli meter to inch if required

	for (int i = 1; i <= entitySize; ++i)
	{
		const Handle(Standard_Transient)& entity = model->Value(i);
		string stepEntityType = entity->DynamicType()->Name();

		if (stepEntityType != "StepVisual_CameraModelD3MultiClipping")
			continue;

		Handle(StepVisual_CameraModelD3MultiClipping) cameraModel = Handle(StepVisual_CameraModelD3MultiClipping)::DownCast(entity);
		Handle(StepVisual_HArray1OfCameraModelD3MultiClippingInterectionSelect) clippingArr = cameraModel->ShapeClipping();

		if (clippingArr->Length() > 1)
			continue;

		Handle(StepGeom_Plane) clippingPlane = Handle(StepGeom_Plane)::DownCast(clippingArr->First().Value());
			
		if (!clippingPlane)
			continue;

		Handle(StepGeom_Axis2Placement3d) axis2Placement3d = clippingPlane->Position();
		Handle(StepGeom_CartesianPoint) point = axis2Placement3d->Location();
		Handle(StepGeom_Direction) axis = axis2Placement3d->Axis();
		Handle(StepGeom_Direction) refDir = axis2Placement3d->RefDirection();

		double p1 = factor * point->CoordinatesValue(1);
		double p2 = factor * point->CoordinatesValue(2);
		double p3 = factor * point->CoordinatesValue(3);

		double z1 = axis->DirectionRatiosValue(1);
		double z2 = axis->DirectionRatiosValue(2);
		double z3 = axis->DirectionRatiosValue(3);

		double x1, x2, x3;
		gp_Pln* plane = nullptr;

		if (refDir)
		{
			x1 = refDir->DirectionRatiosValue(1);
			x2 = refDir->DirectionRatiosValue(2);
			x3 = refDir->DirectionRatiosValue(3);

			if (gp_Dir(x1, x2, x3).IsOpposite(gp_Dir(z1, z2, z3), Precision::Confusion())
				|| gp_Dir(x1, x2, x3).IsEqual(gp_Dir(z1, z2, z3), Precision::Confusion()))
			{
				// Wrong ref direction
				//plane = new gp_Pln(gp_Pnt(p1, p2, p3), gp_Dir(z1, z2, z3));
				continue;
			}
			else
			{
				plane = new gp_Pln(gp_Ax3(gp_Pnt(p1, p2, p3), gp_Dir(z1, z2, z3), gp_Dir(x1, x2, x3)));
			}
		}
		else
		{
			plane = new gp_Pln(gp_Pnt(p1, p2, p3), gp_Dir(z1, z2, z3));
		}

		BRepBuilderAPI_MakeFace face(*plane);
		TopoDS_Shape planeShape = face.Shape();

		if (planeShape.IsNull())
			continue;
		
		sectionPlanes.push_back(planeShape);

		int shapeID = OCCUtil::GetID(planeShape);
		string name = cameraModel->Name()->ToCString();

		string id(workSession->EntityLabel(entity)->ToCString());
		int stepEntityID = atoi(id.substr(1, id.length()).c_str());
		m_shapeIDentityIDMap.insert({ shapeID,stepEntityID });
		m_shapeIDentityNameMap.insert({ shapeID,name });
	}
}

double STEP_Data::GetUnitConversionFactor(void)
{
	double factor = 1.0;

	const Handle(XSControl_WorkSession)& workSession = m_reader.WS();
	const Handle(Interface_InterfaceModel)& model = workSession->Model();

	int entitySize = model->NbEntities();

	for (int i = 1; i <= entitySize; ++i)
	{
		const Handle(Standard_Transient)& entity = model->Value(i);
		string stepEntityType = entity->DynamicType()->Name();

		if (stepEntityType != "StepShape_AdvancedBrepShapeRepresentation")
			continue;
		
		Handle(StepShape_AdvancedBrepShapeRepresentation) adv = Handle(StepShape_AdvancedBrepShapeRepresentation)::DownCast(entity);
		int num = workSession->Model()->Number(adv->ContextOfItems());
		const Handle(Standard_Transient)& ent = workSession->Model()->Value(num);

		Handle(StepGeom_GeomRepContextAndGlobUnitAssCtxAndGlobUncertaintyAssCtx) contexts = Handle(StepGeom_GeomRepContextAndGlobUnitAssCtxAndGlobUncertaintyAssCtx)::DownCast(ent);
		Handle(StepRepr_GlobalUnitAssignedContext) unitContext = contexts->GlobalUnitAssignedContext();
		Handle(StepBasic_HArray1OfNamedUnit) units = unitContext->Units();
			
		int len = units->Length();
			
		for (int j = 1; j <= len; ++j)
		{
			Handle(StepBasic_ConversionBasedUnit) convUnit = Handle(StepBasic_ConversionBasedUnit)::DownCast(units->Value(j));

			if (!convUnit)
				continue;
			
			string name = StrTool::ToLower(convUnit->Name()->ToCString());
			
			if (name != "inch")
				continue;

			Handle(StepBasic_MeasureWithUnit) mwu = convUnit->ConversionFactor();
			Handle(StepBasic_MeasureValueMember) mvm = mwu->ValueComponentMember();

			string type = mvm->Name();

			if (type != "LENGTH_MEASURE")
				break;

			factor = mwu->ValueComponent();
			break;
		}

		if (factor != 1.0)
			break;
		
		/*
		if (stepEntityType == "StepGeom_GeomRepContextAndGlobUnitAssCtxAndGlobUncertaintyAssCtx")
		{
			Handle(StepGeom_GeomRepContextAndGlobUnitAssCtxAndGlobUncertaintyAssCtx) context = Handle(StepGeom_GeomRepContextAndGlobUnitAssCtxAndGlobUncertaintyAssCtx)::DownCast(entity);
			Handle(StepRepr_GlobalUnitAssignedContext) unitContext = context->GlobalUnitAssignedContext();

			Handle(StepBasic_HArray1OfNamedUnit) units = unitContext->Units();
			int len = units->Length();

			for (int j = 1; j <= len; ++j)
			{
				Handle(StepBasic_ConversionBasedUnit) unit = Handle(StepBasic_ConversionBasedUnit)::DownCast(units->Value(j));
				
				if (unit)
				{
					Handle(StepBasic_MeasureWithUnit) mwu = unit->ConversionFactor();
					Handle(StepBasic_MeasureValueMember) member = mwu->ValueComponentMember();

					string type = member->Name();
					
					if (type == "LENGTH_MEASURE")
					{
						factor = mwu->ValueComponent();
						break;
					}
				}
			}

			if (factor != 1.0)
				break;
		}
		*/
	}

	return factor;
}

void STEP_Data::Clear(void)
{
	m_shapeIDentityTypeMap.clear();
	m_shapeIDentityIDMap.clear();
}