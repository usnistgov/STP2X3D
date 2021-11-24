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

protected:
	void StoreStepDataForShapes(const STEPControl_Reader& reader);
	void Clear(void);

private:
	unordered_map<int, string> m_shapeIDentityTypeMap;
	unordered_map<int, int> m_shapeIDentityIDMap;
};