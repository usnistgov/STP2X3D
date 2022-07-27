#pragma once


class STEP_Data
{
public:
	STEP_Data(const STEPControl_Reader& reader);
	~STEP_Data(void);

	const string GetEntityTypeFromShape(const TopoDS_Shape& shape) const;
	const int GetEntityIDFromShape(const TopoDS_Shape& shape) const;

	bool IsSolidModel(const TopoDS_Shape& shape) const;
	bool IsSurfaceModel(const TopoDS_Shape& shape) const;
	bool IsWireframeModel(const TopoDS_Shape& shape) const;
	bool IsShell(const TopoDS_Shape& shape) const;

	bool IsMappedItem(const TopoDS_Shape& shape) const;

	TopoDS_Shape GetHiddenShapeAt(int index) { return m_hiddenShapes[index]; }
	int GetHiddenShapeSize(void) { return (int)m_hiddenShapes.size(); }

protected:
	void StoreStepDataForShapes(const STEPControl_Reader& reader);
	void StoreHiddenShape(const Handle(Standard_Transient)& entity, const Handle(XSControl_WorkSession)& workSession, const Handle(Transfer_TransientProcess)& transProcess);
	void Clear(void);

private:
	unordered_map<int, string> m_shapeIDentityTypeMap;
	unordered_map<int, int> m_shapeIDentityIDMap;

	vector<TopoDS_Shape> m_hiddenShapes;
};