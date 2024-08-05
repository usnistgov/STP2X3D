#include "stdafx.h"
#include "Tessellator.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"
#include "GDT_Item.h"

Tessellator::Tessellator(S2X_Option* opt)
	: m_opt(opt)
{
	double angDeflection_max = 0.8, angDeflection_min = 0.2, angDeflection_gap = (angDeflection_max - angDeflection_min) / 10;
	m_angDeflection = max(angDeflection_max - (m_opt->Quality() * angDeflection_gap), angDeflection_min);
	
	//m_linDeflection = 2.5 / m_opt->Quality();
	//m_angDeflection = 5.0 / m_opt->Quality();

	m_isRelative = false; // If TRUE, linear deflection is automatically set.
}

Tessellator::~Tessellator(void)
{
}

void Tessellator::Tessellate(Model*& model) const
{
	TessellateModel(model);

	if (m_opt->GDT())
		TessellateGDT(model);

	model->Update();
}

void Tessellator::TessellateModel(Model*& model) const
{
	if (!m_opt->Tessellation())
	{
		for (int i = 0; i < model->GetRootComponentSize(); ++i)
		{
			Component* rootComp = model->GetRootComponentAt(i);
			const TopoDS_Shape& shape = rootComp->GetShape();

			// Get the relative linear deflection for a shape
			double linDeflection = OCCUtil::GetDeflection(shape);

			// Tessellate and add mesh data of a shape
			if (!OCCUtil::TessellateShape(shape, linDeflection, m_isRelative, m_angDeflection, true))
				wcout << "\tTessellation has failed on Shape: " << rootComp->GetName() << endl;
		}
	}

	vector<Component*> comps;
	model->GetAllComponents(comps);

	for (const auto& comp : comps)
	{
		for (int i = 0; i < comp->GetIShapeSize(); ++i)
		{
			IShape* iShape = comp->GetIShapeAt(i);
			TessellateShape(iShape);
		}
	}

	comps.clear();
}

void Tessellator::TessellateShape(IShape*& iShape) const
{
	if (m_opt->Tessellation()
		|| (m_opt->SFA()
			&& (iShape->IsRosette()
				|| iShape->IsSectionCap())))
	{
		const TopoDS_Shape& shape = iShape->GetShape();

		// Get the relative linear deflection for a shape
		double linDeflection = OCCUtil::GetDeflection(shape);

		if (m_opt->SFA() // SFA-specific
			&& (iShape->IsRosette()
				|| iShape->IsSectionCap()))
			linDeflection = 0.1 * linDeflection;

		// Tessellate and add mesh data of a shape
		if (!OCCUtil::TessellateShape(shape, linDeflection, m_isRelative, m_angDeflection, true))
			wcout << "\tTessellation has failed on Shape: " << iShape->GetName() << endl;
	}

	if (iShape->IsFaceSet())
		AddMeshForFaceSet(iShape);
	else
		AddMeshForSketchGeometry(iShape);
}

void Tessellator::AddMeshForFaceSet(IShape*& iShape) const
{
	const TopoDS_Shape& shape = iShape->GetShape();

	// Traverse faces
	TopExp_Explorer ExpFace;
	for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next())
	{
		const TopoDS_Face& face = TopoDS::Face(ExpFace.Current());
		Mesh* mesh = GetMeshForFace(face, iShape->IsTessSolidModel());

		// Save the faceMesh
		if (mesh)
			iShape->AddMesh(mesh);
	}

	iShape->SetTessellated(true);
}

void Tessellator::AddMeshForSketchGeometry(IShape*& iShape) const
{	
	const TopoDS_Shape& shape = iShape->GetShape();

	// Traverse edges
	TopExp_Explorer ExpEdge;
	for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
	{
		const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
		Mesh* mesh = GetMeshForEdge(edge);

		// Save the edgeMesh
		if (mesh)
			iShape->AddMesh(mesh);
	}

	iShape->SetTessellated(true);
}

Mesh* Tessellator::GetMeshForFace(const TopoDS_Face& face, bool isTessSolidModel) const
{
	TopLoc_Location loc;

	const Handle(Poly_Triangulation)& myT = BRep_Tool::Triangulation(face, loc);

	// Skip if triangulation has failed
	if (!myT
		|| myT.IsNull())
		return nullptr;

	Mesh* mesh = new Mesh(face);

	const Poly_ArrayOfNodes& Nodes = myT->InternalNodes();
	
	// Add coordinates
	for (int i = Nodes.Lower(); i <= Nodes.Upper(); ++i)
	{
		const gp_Pnt& pnt = Nodes[i].Transformed(loc.Transformation());
		mesh->AddCoordinate(pnt.XYZ());
	}

	const TopAbs_Orientation& orientation = face.Orientation();
	const Poly_Array1OfTriangle& triangles = myT->InternalTriangles();

	// Add triangle indexes
	for (int i = 1; i <= myT->NbTriangles(); ++i)
	{
		int n1, n2, n3;
		triangles(i).Get(n1, n2, n3);

		// If a face is reversed, change the direction.
		if (orientation == TopAbs_REVERSED)
			swap(n1, n2);

		if (!IsTriangleValid(Nodes[n1 - 1], Nodes[n2 - 1], Nodes[n3 - 1]))
			continue;

		mesh->AddFaceIndex(n1, n2, n3);

		if (m_opt->Normal()
			&& !isTessSolidModel)
			mesh->AddNormalIndex(n1, n2, n3);
	}

	// Add vertex normals
	if (m_opt->Normal())
	{
		if (!isTessSolidModel)
		{
			BRepGProp_Face gProp(face);
			const Poly_ArrayOfUVNodes& uvNodes = myT->InternalUVNodes();

			for (int i = uvNodes.Lower(); i <= uvNodes.Upper(); ++i)
			{
				const gp_Pnt2d& uv = uvNodes[i];
				gp_Pnt pnt;
				gp_Vec normal;
				gProp.Normal(uv.X(), uv.Y(), pnt, normal);

				if (normal.SquareMagnitude() > 0.0)
					normal.Normalize();

				mesh->AddNormal(normal.XYZ());
			}
		}
		/*else
		{
			// For tessellated solids
			if (m_opt->TessSolid())
			{
				int index = 1;

				for (int i = 0; i < mesh->GetFaceIndexSize(); ++i)
				{
					vector<int> faceIndex = mesh->GetFaceIndexAt(i);

					gp_XYZ p1 = mesh->GetCoordinateAt(faceIndex[0] - 1);
					gp_XYZ p2 = mesh->GetCoordinateAt(faceIndex[1] - 1);
					gp_XYZ p3 = mesh->GetCoordinateAt(faceIndex[2] - 1);

					gp_XYZ n1 = (p2 - p1).Crossed((p3 - p1)).Normalized();
					gp_XYZ n2 = (p3 - p2).Crossed((p1 - p2)).Normalized();
					gp_XYZ n3 = (p1 - p3).Crossed((p2 - p3)).Normalized();

					mesh->AddNormal(n1);
					mesh->AddNormal(n2);
					mesh->AddNormal(n3);

					mesh->AddNormalIndex(index, index + 1, index + 2);
					index = index + 3;
				}
			}
		}*/
	}

	// Add boundary edges
	if (m_opt->Edge())
	{
		if (!isTessSolidModel)
		{
			TopExp_Explorer ExpEdge;
			for (ExpEdge.Init(face, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
			{
				const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
				const Handle(Poly_PolygonOnTriangulation)& polygon = BRep_Tool::PolygonOnTriangulation(edge, myT, loc);
				const TColStd_Array1OfInteger& edgeNodes = polygon->Nodes();

				vector<int> edgeIndex;

				for (int i = edgeNodes.Lower(); i <= edgeNodes.Upper(); ++i)
					edgeIndex.push_back(edgeNodes(i));

				mesh->AddEdgeIndex(edgeIndex);
				edgeIndex.clear();
			}
		}
		else
		{
			// For tessellated solids
			if (m_opt->TessSolid())
			{
				for (int i = 0; i < mesh->GetFaceIndexSize(); ++i)
				{
					vector<int> faceIndex = mesh->GetFaceIndexAt(i);

					vector<int> edgeIndex1, edgeIndex2, edgeIndex3;

					edgeIndex1.push_back(faceIndex[0]);
					edgeIndex1.push_back(faceIndex[1]);
					edgeIndex2.push_back(faceIndex[1]);
					edgeIndex2.push_back(faceIndex[2]);
					edgeIndex3.push_back(faceIndex[2]);
					edgeIndex3.push_back(faceIndex[0]);

					mesh->AddEdgeIndex(edgeIndex1);
					mesh->AddEdgeIndex(edgeIndex2);
					mesh->AddEdgeIndex(edgeIndex3);
				}
			}
		}
	}
	
	return mesh;
}

Mesh* Tessellator::GetMeshForEdge(const TopoDS_Edge& edge) const
{
	TopLoc_Location loc;

	// Get a tessellated edge
	const Handle(Poly_Polygon3D)& myP = BRep_Tool::Polygon3D(edge, loc);

	if (!myP
		|| myP.IsNull())
		return nullptr;

	Mesh* mesh = new Mesh(edge);

	const TColgp_Array1OfPnt& Nodes = myP->Nodes();

	vector<int> edgeIndex;

	// Add coordinates and edge index
	for (int i = Nodes.Lower(); i <= Nodes.Upper(); ++i)
	{
		const gp_Pnt& pnt = Nodes(i).Transformed(loc.Transformation());
		mesh->AddCoordinate(pnt.XYZ());
		edgeIndex.push_back(i);
	}

	// Save the edge index
	mesh->AddEdgeIndex(edgeIndex);
	edgeIndex.clear();

	return mesh;
}

bool Tessellator::IsTriangleValid(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) const
{
	gp_Vec v1(p1, p2);
	gp_Vec v2(p2, p3);
	gp_Vec v3(p3, p1);
	gp_Vec v_norm = v1 ^ v2;
	
	// Tolerance (1.e-7)^2
	double tol = Precision::SquareConfusion();

	// Valid if the magnitudes are greater than tolerance
	if (v1.SquareMagnitude() > tol
		&& v2.SquareMagnitude() > tol
		&& v3.SquareMagnitude() > tol
		&& v_norm.SquareMagnitude() > tol)
		return true;
	
	return false;
}

void Tessellator::TessellateGDT(Model*& model) const
{
	for (int i = 0; i < model->GetGDTSize(); ++i)
	{
		GDT_Item* gdt = model->GetGDTAt(i);

		for (int j = 0; j < gdt->GetShapeSize(); ++j)
		{
			const TopoDS_Shape& shape = gdt->GetShapeAt(j);
			TopAbs_ShapeEnum shapeType = shape.ShapeType();

			if (shapeType == TopAbs_FACE) // Find and add the face mesh
			{
				const TopoDS_Face& face = TopoDS::Face(shape);
				Mesh* mesh = GetMeshForFace(face, false);

				// Save the face mesh
				if (mesh)
					gdt->AddMesh(mesh);
			}
			else // Tessellate and add the edge mesh
			{
				double linDeflection = OCCUtil::GetDeflection(shape);

				if (!OCCUtil::TessellateShape(shape, linDeflection, m_isRelative, m_angDeflection, true))
					return;

				TopExp_Explorer ExpEdge;
				for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
				{
					const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
					
					Mesh* mesh = GetMeshForEdge(edge);

					// Save the edge mesh
					if (mesh)
						gdt->AddMesh(mesh);
				}
			}
		}
	}
}
