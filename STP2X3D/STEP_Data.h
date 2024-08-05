#pragma once


class STEP_Data
{
public:
	STEP_Data(const STEPControl_Reader& reader);
	~STEP_Data(void);

	const string GetEntityTypeFromShape(const TopoDS_Shape& shape) const;
	const int GetEntityIDFromShape(const TopoDS_Shape& shape) const;
	const string GetEntityNameFromShape(const TopoDS_Shape& shape) const;

	bool IsSolidModel(const TopoDS_Shape& shape) const;
	bool IsTessellatedSolidModel(const TopoDS_Shape& shape) const;
	bool IsSurfaceModel(const TopoDS_Shape& shape) const;
	bool IsWireframeModel(const TopoDS_Shape& shape) const;
	bool IsShell(const TopoDS_Shape& shape) const;

	bool IsMappedItem(const TopoDS_Shape& shape) const;

	void AddRosetteGeometries(vector<TopoDS_Shape>& rosettes);
	void AddSectionPlanes(vector<TopoDS_Shape>& sectionPlanes);

protected:
	void StoreStepDataForShapes(void);
	double GetUnitConversionFactor(void);
	void Clear(void);

private:
	unordered_map<int, string> m_shapeIDentityTypeMap;
	unordered_map<int, int> m_shapeIDentityIDMap;
	unordered_map<int, string> m_shapeIDentityNameMap;

	STEPControl_Reader m_reader;
};