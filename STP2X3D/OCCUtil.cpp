#include "stdafx.h"
#include "OCCUtil.h"


namespace OCCUtil
{
	const int GetID(const TopoDS_Shape& shape)
	{
		//const int id = shape.HashCode(INT_MAX);
		const int id = static_cast<int>(std::hash<TopoDS_Shape>{}(shape));
		
		return id;
	}

	const Bnd_Box ComputeBoundingBox(const TopoDS_Shape& shape)
	{
		Bnd_Box bndBox;
		BRepBndLib::Add(shape, bndBox);

		return bndBox;
	}

	double ComputeVolume(const TopoDS_Shape& shape)
	{
		GProp_GProps System;
		BRepGProp::VolumeProperties(shape, System);

		double volume = System.Mass();
		
		return volume;
	}

	void RemoveFreeWires(TopoDS_Shape shape)
	{
		BRepTools::Clean(shape);
		BRep_Builder builder;

		for (TopoDS_Iterator exp(shape); exp.More(); exp.Next())
		{
			TopoDS_Shape subShape = exp.Value();
			TopAbs_ShapeEnum subShapeType = subShape.ShapeType();

			if (subShapeType == TopAbs_COMPOUND)
			{
				RemoveFreeWires(subShape);
			}
			else if (subShapeType != TopAbs_COMPSOLID
				&& subShapeType != TopAbs_SOLID
				&& subShapeType != TopAbs_SHELL)
			{
				// wires or edges or vertices 
				builder.Remove(shape, subShape);
			}
		}
	}

	TopoDS_Shape GetCopiedShape(TopoDS_Shape shape)
	{
		BRepBuilderAPI_Copy copier(shape);
		const TopoDS_Shape copiedShape = copier.Shape();

		return copiedShape;
	}

	bool HasFace(const TopoDS_Shape& shape)
	{
		TopExp_Explorer ExpFace;
		for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next())
			return true;

		return false;
	}

	bool HasEdge(const TopoDS_Shape& shape)
	{
		TopExp_Explorer ExpEdge;
		for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
			return true;

		return false;
	}

	bool IsFreeEdge(const TopoDS_Shape& shape)
	{	
		if (!HasFace(shape)
			&& HasEdge(shape))
			return true;

		return false;
	}

	bool HasWire(const TopoDS_Shape& shape)
	{
		TopExp_Explorer ExpWire;
		for (ExpWire.Init(shape, TopAbs_WIRE); ExpWire.More(); ExpWire.Next())
			return true;

		return false;
	}

	TopoDS_Shape TransformShape(TopoDS_Shape shape, gp_Trsf trsf)
	{
		// Note that a shape loses its color after transformation
		BRepBuilderAPI_Transform transform(shape, trsf);
		TopoDS_Shape trsfShape = transform.Shape();

		return trsfShape;
	}

	bool TessellateShape(const TopoDS_Shape& shape, double linearDeflection, bool isRelative, double angularDeflection, bool isParallel)
	{
		BRepTools::Clean(shape);
		BRepMesh_IncrementalMesh bMesh(shape, linearDeflection, isRelative, angularDeflection, isParallel);
		
		return bMesh.IsDone();
	}

	bool IsTranslated(const gp_Trsf& transform)
	{
		const gp_XYZ& trans = transform.TranslationPart();

		if (!(trans.X() == 0.0
			&& trans.Y() == 0.0
			&& trans.Z() == 0.0))
			return true;

		return false;
	}

	bool IsRotated(const gp_Trsf& transform)
	{
		gp_Vec rotAxis;
		double rotAngle = 0.0;
		transform.GetRotation().GetVectorAndAngle(rotAxis, rotAngle);

		if (rotAngle != 0.0)
			return true;

		return false;
	}

	bool IsTransformed(const gp_Trsf& transform)
	{
		if (IsTranslated(transform)
			|| IsRotated(transform))
			return true;

		return false;
	}

	double GetDeflection(const TopoDS_Shape& shape)
	{
		Bnd_Box bndBox = ComputeBoundingBox(shape);
		
		bndBox = bndBox.FinitePart();

		gp_Pnt minPnt = bndBox.CornerMin();
		gp_Pnt maxPnt = bndBox.CornerMax();

		double deviationCoefficient = 0.001;

		double deflection = Prs3d::GetDeflection(Graphic3d_Vec3d(minPnt.X(), minPnt.Y(), minPnt.Z()), Graphic3d_Vec3d(maxPnt.X(), maxPnt.Y(), maxPnt.Z()), deviationCoefficient);
		
		return Max(deflection, Precision::Confusion());
	}
}