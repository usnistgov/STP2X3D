#pragma once

#include "Mesh.h"

#include <TopTools_ShapeMapHasher.hxx>

class Component;
class IShape;
class GDT_Item;

class Tessellator
{
public:
	Tessellator(S2X_Option* opt);
	~Tessellator(void);

	void Tessellate(Model*& model) const;

protected:
	void TessellateModel(Model*& model) const;
	void TessellateShape(IShape*& iShape) const;
	
	void AddMeshForFaceSet(IShape*& iShape) const;
	void AddMeshForSketchGeometry(IShape*& iShape) const;

	Mesh* GetMeshForFace(const TopoDS_Face& face, bool isTessSolidModel) const;
	Mesh* GetMeshForEdge(const TopoDS_Edge& edge) const;

	bool IsTriangleValid(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) const;

	void TessellateGDT(Model*& model) const;

private:
	struct MeshCacheKey
	{
		MeshCacheKey(const TopoDS_Shape& theShape, bool theIsFace, bool theIsTessSolidModel)
			: shape(theShape)
			, isFace(theIsFace)
			, isTessSolidModel(theIsTessSolidModel)
		{
		}

		TopoDS_Shape shape;
		bool isFace;
		bool isTessSolidModel;
	};

	struct MeshCacheKeyHasher
	{
		size_t operator()(const MeshCacheKey& key) const noexcept
		{
			size_t hash = TopTools_ShapeMapHasher()(key.shape);
			const size_t hashSeed = static_cast<size_t>(0x9e3779b9);

			hash ^= static_cast<size_t>(key.shape.Orientation()) + hashSeed + (hash << 6) + (hash >> 2);
			hash ^= static_cast<size_t>(key.isFace) + hashSeed + (hash << 6) + (hash >> 2);
			hash ^= static_cast<size_t>(key.isTessSolidModel) + hashSeed + (hash << 6) + (hash >> 2);

			return hash;
		}
	};

	struct MeshCacheKeyEqual
	{
		bool operator()(const MeshCacheKey& left, const MeshCacheKey& right) const noexcept
		{
			return left.shape.IsSame(right.shape)
				&& left.shape.Orientation() == right.shape.Orientation()
				&& left.isFace == right.isFace
				&& left.isTessSolidModel == right.isTessSolidModel;
		}
	};

	struct MeshParams
	{
		double linDeflection = 0.0;
		double angDeflection = 0.0;
		bool isRelative = false;
	};

	bool TessellateShapeOnce(const TopoDS_Shape& shape, double linDeflection, bool& didRemesh) const;

	S2X_Option* m_opt;

	double m_linDeflection;
	double m_angDeflection;

	bool m_isRelative;

	mutable unordered_map<MeshCacheKey, Mesh, MeshCacheKeyHasher, MeshCacheKeyEqual> m_meshCache;
	mutable unordered_map<const void*, MeshParams> m_meshedTShapes;
};