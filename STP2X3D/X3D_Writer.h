#pragma once

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

	void WriteX3D(Model* model);

protected:
	wstring OpenHeader(void) const;
	wstring CloseHeader(void) const;

	wstring WriteViewpoint(Model* model, int level) const;

	wstring WriteModel(Model* model, int level);
	wstring WriteComponent(Component* comp, int level);

	wstring WriteTransformAttributes(const gp_Trsf& trsf) const;
	wstring WriteShape(IShape* iShape, int level);
	wstring WriteIndexedFaceSet(IShape* iShape, int level);
	wstring WriteIndexedLineSet(IShape* iShape, int level);

	wstring WriteAppearance(IShape* iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
											const Quantity_Color& emissiveColor, bool isEmissiveOn,
											const Quantity_Color& specularColor, bool isSpecularOn,
											const double shininess, bool isShininessOn,
											const double ambientIntensity, bool isAmbientIntensityOn,
											const double transparency, bool isTransparencyOn);
	wstring WriteCoordinate(IShape* iShape, bool isBoundaryEdges) const;
	wstring WriteCoordinateIndex(IShape* iShape, bool faceMesh) const;
	wstring WriteNormalIndex(IShape* iShape) const;
	wstring WriteColor(IShape* iShape) const;
	wstring WriteNormal(IShape* iShape) const;

	wstring Indent(int level) const;
	wstring CleanString(wstring str) const;

	bool CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
							const Quantity_Color& emissiveColor, bool isEmissiveOn,
							const Quantity_Color& specularColor, bool isSpecularOn,
							const double shininess, bool isShininessOn,
							const double ambientIntensity, bool isAmbientIntensityOn,
							const double transparency, bool isTransparencyOn,
							int& appID);
	void Clear(void);

	// SFA-specific functions
	wstring WriteSketchGeometry(IShape* iShape, int level) const;
	void CountIndent(int level);
	void PrintIndentCount(void);
	void PrintMaterialCount(void) const;

private:
	S2X_Option* m_opt;

	Quantity_Color m_diffuseColor;
	Quantity_Color m_emissiveColor;
	Quantity_Color m_specularColor;
	double m_shininess;
	double m_ambientIntensity;
	double m_transparency;

	double m_creaseAngle;

	vector<Appearance> m_appearances;
	
	// SFA-specific variables
	map<int, int> m_indentCountMap;
};