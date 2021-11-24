#pragma once

namespace OCCUtil
{
	// Get unique id for shape
	const int GetID(const TopoDS_Shape& shape);

	// Compute bounding box of a shape
	const Bnd_Box ComputeBoundingBox(const TopoDS_Shape& shape);

	// Compute volue of a shape
	double ComputeVolume(const TopoDS_Shape& shape);

	// Remove free wires, edges, and vertices not beloing to solids or shells
	void RemoveFreeWires(TopoDS_Shape shape);

	// Copy and return the shape 
	const TopoDS_Shape& GetCopiedShape(const TopoDS_Shape& shape);

	// Check if the shape has faces
	bool HasFace(const TopoDS_Shape& shape);

	// Check if the shape has edges
	bool HasEdge(const TopoDS_Shape& shape);

	// Check if the shape is free edges
	bool IsFreeEdge(const TopoDS_Shape& shape);

	// Check if the shape has wires
	bool HasWire(const TopoDS_Shape& shape);

	// Transform a shape
	const TopoDS_Shape& TransformShape(const TopoDS_Shape& shape, const gp_Trsf& trsf);

	// Tessellate a shape
	bool TessellateShape(const TopoDS_Shape& shape, double linearDeflection, bool isRelative, double angularDeflection, bool isParallel);

	// Check if translated
	bool IsTranslated(const gp_Trsf& transform);

	// Check if rotated
	bool IsRotated(const gp_Trsf& transform);

	// Check if transformed
	bool IsTransformed(const gp_Trsf& transform);

	// Get the relative deflection for a given shape
	double GetDeflection(const TopoDS_Shape& shape);
}