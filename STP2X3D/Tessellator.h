#pragma once

class Component;
class Mesh;
class IShape;

class Tessellator
{
public:
	Tessellator(S2X_Option* opt);
	~Tessellator(void);

	void Tessellate(Model* model) const;

protected:
	void TessellateShape(IShape* iShape) const;
	void AddMeshForFaceSet(IShape* iShape) const;
	void AddMeshForSketchGeometry(IShape* iShape) const;

	bool IsTriangleValid(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) const;

private:
	S2X_Option* m_opt;

	double m_linDeflection;
	double m_angDeflection;
};