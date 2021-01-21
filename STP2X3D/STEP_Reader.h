#pragma once

class STEP_Data;
class Model;
class Component;
class IShape;

class STEP_Reader
{
public:
	STEP_Reader(S2X_Option* opt);
	~STEP_Reader(void);

	bool ReadSTEP(Model* model);

protected:
	void AddSubComponents(Component* comp, const TDF_Label& label);
	void GetSubShapes(const TopoDS_Shape& shape, vector<TopoDS_Shape>& subShapes) const;
	wstring GetName(const TDF_Label& label) const;

	void AddColors(IShape* iShape) const;
	
	bool CheckReturnStatus(const IFSelect_ReturnStatus& status) const;
	bool IsEmpty(const TopoDS_Shape& shape) const;
	bool IsCopy(Component* comp);

	void UpdateColorOption() const;
	void Clear(void);

private:
	S2X_Option* m_opt;

	Handle(XCAFDoc_ShapeTool) m_shapeTool;
	Handle(XCAFDoc_ColorTool) m_colorTool;

	Quantity_ColorRGBA m_defaultFaceColor;
	Quantity_ColorRGBA m_defaultWireColor;

	map<int, Component*> m_idComponentMap;

	STEP_Data* m_stepData;
};
