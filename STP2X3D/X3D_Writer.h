#pragma once

#include "X3D_Text.h"

class Component;
class IShape;

struct Appearance
{
	Quantity_Color diffuseColor;
	Quantity_Color specularColor;
	Quantity_Color emissiveColor;
	double shininess = 0.0;
	double ambientIntensity = 0.0;
	double transparency = 0.0;

	bool isDiffuseOn = false;
	bool isEmissiveOn = false;
	bool isSpecularOn = false;
	bool isShininessOn = false;
	bool isAmbientIntensityOn = false;
	bool isTransparencyOn = false;
};

class X3D_Writer
{
public:
	X3D_Writer(S2X_Option* opt);
	~X3D_Writer(void);

	bool WriteX3D(Model*& model);

protected:
	void OpenHeader(X3D_Text& out) const;
	void CloseHeader(X3D_Text& out) const;

	void WriteViewpoint(X3D_Text& out, Model*& model, int level) const;

	void WriteModel(X3D_Text& out, Model*& model, int level);
	void WriteComponent(X3D_Text& out, Component*& comp, int level);

	void WriteTransformAttributes(X3D_Text& out, const gp_Trsf& trsf) const;
	void WriteShape(X3D_Text& out, IShape*& iShape, int level);
	void WriteIndexedFaceSet(X3D_Text& out, IShape*& iShape, int level);
	void WriteIndexedLineSet(X3D_Text& out, IShape*& iShape, int level);

	void WriteAppearance(X3D_Text& out, IShape*& iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
											const Quantity_Color& emissiveColor, bool isEmissiveOn,
											const Quantity_Color& specularColor, bool isSpecularOn,
											double& shininess, bool isShininessOn,
											double& ambientIntensity, bool isAmbientIntensityOn,
											double& transparency, bool isTransparencyOn);
	void WriteCoordinate(X3D_Text& out, IShape*& iShape, bool isBoundaryEdges) const;
	void WriteCoordinateIndex(X3D_Text& out, IShape*& iShape, bool faceMesh) const;
	void WriteNormalIndex(X3D_Text& out, IShape*& iShape) const;
	void WriteColor(X3D_Text& out, IShape*& iShape) const;
	void WriteNormal(X3D_Text& out, IShape*& iShape) const;

	const wstring Indent(int level) const;

	bool CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
							const Quantity_Color& emissiveColor, bool isEmissiveOn,
							const Quantity_Color& specularColor, bool isSpecularOn,
							double& shininess, bool isShininessOn,
							double& ambientIntensity, bool isAmbientIntensityOn,
							double& transparency, bool isTransparencyOn,
							int& appID);
	void Clear(void);

	// SFA-specific functions
	void WriteSketchGeometry(X3D_Text& out, IShape*& iShape, int level);
	void WriteRosetteGeometry(X3D_Text& out, Component*& comp, int level);
	void WriteSectionCapGeometry(X3D_Text& out, Component*& comp, int level);
	void CountIndent(int level);
	void PrintIndentCount(void);
	void PrintMaterialCount(void) const;

	void WriteGDT(X3D_Text& out, Model*& model, int level);

private:
	S2X_Option* m_opt;

	Quantity_Color m_diffuseColor;
	Quantity_Color m_emissiveColor;
	Quantity_Color m_specularColor;
	Quantity_Color m_gdtColor, m_gdtColor2;

	double m_shininess;
	double m_ambientIntensity;
	double m_transparency;

	double m_creaseAngle;

	vector<Appearance> m_appearances;
	unordered_map<size_t, vector<int>> m_appearanceExactIndex;

	// SFA-specific variables
	map<int, int> m_indentCountMap;
};