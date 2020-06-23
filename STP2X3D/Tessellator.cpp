#include "stdafx.h"
#include "Tessellator.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"


Tessellator::Tessellator(S2X_Option* opt)
	: m_opt(opt)
{
	m_linDeflection = 2.5 / m_opt->Quality();
	m_angDeflection = 5.0 / m_opt->Quality();
}

Tessellator::~Tessellator(void)
{
}

void Tessellator::Tessellate(Model* model) const
{
	vector<Component*> comps;
	model->GetAllComponents(comps);

	for (auto comp : comps)
	{
		for (int i = 0; i < comp->GetIShapeSize(); ++i)
		{
			IShape* iShape = comp->GetIShapeAt(i);
			TessellateShape(iShape); // Tessellate each shape
		}
	}

	comps.clear();

	model->Update();
}

void Tessellator::TessellateShape(IShape* iShape) const
{
	const TopoDS_Shape& shape = iShape->GetShape();

	try
	{
		// Tessellate and add mesh data of a shape
		if (OCCUtil::TessellateShape(shape, m_linDeflection, false, m_angDeflection, true))
		{
			if (iShape->IsFaceSet())
				AddMeshForFaceSet(iShape);
			else
				AddMeshForSketchGeometry(iShape);
		}
	}
	catch (...)
	{
		wcout << "\tTessellation has failed on Shape: " << iShape->GetName() << endl;
	}

	iShape->SetTessellated(true);
}

void Tessellator::AddMeshForFaceSet(IShape* iShape) const
{
	const TopoDS_Shape& shape = iShape->GetShape();

	// Traverse faces
	TopExp_Explorer ExpFace;
	for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next())
	{
		const TopoDS_Face& face = TopoDS::Face(ExpFace.Current());
		TopLoc_Location loc;

		const Handle(Poly_Triangulation)& myT = BRep_Tool::Triangulation(face, loc);
		
		// Skip if triangulation has failed
		if (!myT
			|| myT.IsNull())
			continue;

		Mesh* mesh = new Mesh(face);
		const TColgp_Array1OfPnt& Nodes = myT->Nodes();

		// Add coordinates
		for (int i = Nodes.Lower(); i <= Nodes.Upper(); ++i)
		{
			const gp_Pnt& pnt = Nodes(i).Transformed(loc.Transformation());
			mesh->AddCoordinate(pnt.XYZ());
		}

		const TopAbs_Orientation& orientation = face.Orientation();
		const Poly_Array1OfTriangle& triangles = myT->Triangles();

		// Add triangle indexes
		for (int i = 1; i <= myT->NbTriangles(); ++i)
		{
			int n1, n2, n3;
			triangles(i).Get(n1, n2, n3);

			// If a face is reversed, change the direction.
			if (orientation == TopAbs_REVERSED)
				swap(n1, n2);

			if (!IsTriangleValid(Nodes(n1), Nodes(n2), Nodes(n3)))
				continue;

			mesh->AddFaceIndex(n1, n2, n3);

			if (m_opt->Normal())
				mesh->AddNormalIndex(n1, n2, n3);
		}

		// Add vertex normals
		if (m_opt->Normal())
		{
			BRepGProp_Face gProp(face);
			const TColgp_Array1OfPnt2d& uvNodes = myT->UVNodes();

			for (int i = uvNodes.Lower(); i <= uvNodes.Upper(); ++i)
			{
				const gp_Pnt2d& uv = uvNodes(i);
				gp_Pnt pnt; 
				gp_Vec normal;
				gProp.Normal(uv.X(), uv.Y(), pnt, normal);

				if (normal.SquareMagnitude() > 0.0)
					normal.Normalize();

				mesh->AddNormal(normal.XYZ());
			}
		}

		// Add boundary edges
		if (m_opt->Edge())
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

		// Save the faceMesh
		iShape->AddMesh(mesh);
	}
}

void Tessellator::AddMeshForSketchGeometry(IShape* iShape) const
{	
	const TopoDS_Shape& shape = iShape->GetShape();

	// Traverse edges
	TopExp_Explorer ExpEdge;
	for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
	{
		const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
		TopLoc_Location loc;

		// Get a tessellated edge
		const Handle(Poly_Polygon3D)& myP = BRep_Tool::Polygon3D(edge, loc);

		if (!myP
			|| myP.IsNull())
			continue;

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

		// Save the edgeMesh
		iShape->AddMesh(mesh);
	}
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