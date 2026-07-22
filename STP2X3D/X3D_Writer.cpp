#include "stdafx.h"
#include "X3D_Writer.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"
#include "GDT_Item.h"

#include <cstring>
#include <cstdint>

namespace
{
// X3D Material colors are display (sRGB) values; OCCT stores linear RGB internally.
void AppendDisplayColor(X3D_Text& out, const Quantity_Color& color)
{
	double r, g, b;
	color.Values(r, g, b, Quantity_TOC_sRGB);
	out << NumTool::DoubleToWString(r) << " ";
	out << NumTool::DoubleToWString(g) << " ";
	out << NumTool::DoubleToWString(b);
}
}

X3D_Writer::X3D_Writer(S2X_Option* opt)
	: m_opt(opt)
{
	// Attributes for Appearance nodes
	m_diffuseColor.SetValues(0.55, 0.55, 0.6, Quantity_TOC_RGB);
	m_emissiveColor.SetValues(1.0, 1.0, 1.0, Quantity_TOC_RGB);
	m_specularColor.SetValues(0.2, 0.2, 0.2, Quantity_TOC_RGB);
	m_gdtColor.SetValues(0.5, 0.1, 0.1, Quantity_TOC_RGB);
	m_gdtColor2.SetValues(0.1, 0.1, 1, Quantity_TOC_RGB);

	m_shininess = 0.9;
	m_ambientIntensity = 1.0;
	m_transparency = 1.0;

	// Attribute for IndexedFaceSet required for X3DOM webvis
	m_creaseAngle = 0.2;
}

X3D_Writer::~X3D_Writer(void)
{
	Clear();
}

bool X3D_Writer::WriteX3D(Model*& model)
{
	wstring filePath = m_opt->Output();
	X3D_Text out(filePath);

	if (!out.IsOpen())
	{
		wcerr << "Unable to open X3D output file: " << filePath << endl;
		return false;
	}

	// Initial indent level
	int level = 0;

	// Open header
	OpenHeader(out);

	// Write viewpoint
	WriteViewpoint(out, model, level + 1);

	// Write model
	WriteModel(out, model, level + 1);

	// Write GDT geometries
	if (m_opt->GDT())
		WriteGDT(out, model, level + 1);

	// Close header
	CloseHeader(out);
	if (!out.Close())
	{
		wcerr << "Failed to write X3D output file: " << filePath << endl;
		return false;
	}

	/// Print results required for SFA
	if (m_opt->SFA())
	{
		PrintIndentCount();
		PrintMaterialCount();
	}
	///

	return true;
}

void X3D_Writer::OpenHeader(X3D_Text& out) const
{
	if (m_opt->Html())
	{
		out << "<html>\n";
		out << "<head>\n";
		//out << " <link rel='stylesheet' type='text/css' href='https://www.x3dom.org/x3dom/release/x3dom.css'/>\n";
		//out << " <script type='text/javascript' src='https://www.x3dom.org/x3dom/release/x3dom.js'></script>\n";
		out << " <link rel='stylesheet' type='text/css' href='https://www.x3dom.org/download/1.8.3/x3dom.css'/>\n";
		out << " <script type='text/javascript' src='https://www.x3dom.org/download/1.8.3/x3dom.js'></script>\n";
		out << "</head>\n";
		out << "<body>\n";
	}
	else
	{
		out << "<?xml version='1.0' encoding='UTF-8'?>\n";
	}

	out << "<X3D version='3.3'>\n";
	out << "<head>\n";
	out << " <meta name='Generator' content='NIST STP2X3D Translator " << m_opt->Version() << "'/>\n";
	out << "</head>\n";
	out << "<Scene>\n";
}

void X3D_Writer::CloseHeader(X3D_Text& out) const
{
	out << "</Scene>\n";
	out << "</X3D>";

	if (m_opt->Html())
	{
		out << "\n";
		out << "</body>\n";
		out << "</html>";
	}
}

void X3D_Writer::WriteViewpoint(X3D_Text& out, Model*& model, int level) const
{
	if (!m_opt->Html())
		return;

	Bnd_Box bndBox = model->GetBoundingBox(m_opt->Sketch());
	if (bndBox.IsVoid())
		return;

	double X_min = 0.0, Y_min = 0.0, Z_min = 0.0;
	double X_max = 0.0, Y_max = 0.0, Z_max = 0.0;

	bndBox.Get(X_min, Y_min, Z_min, X_max, Y_max, Z_max);

	double X_mean = (X_min + X_max) / 2; 
	double Y_mean = (Y_min + Y_max) / 2;
	double Z_mean = (Z_min + Z_max) / 2;

	double X_pos = X_mean;
	double Y_pos = Y_mean;
	double Z_pos = Z_mean;

	double X_ori = 1.0;
	double Y_ori = 0.0;
	double Z_ori = 0.0;
	double R_ori = PI / 2;

	double X_gap = X_max - X_min;
	double Y_gap = Y_max - Y_min;
	double Z_gap = Z_max - Z_min;

	if (X_gap >= Y_gap 
		&& X_gap >= Z_gap)
		Y_pos = (-2) * X_gap;
	else if (Y_gap >= X_gap 
		&& Y_gap >= Z_gap)
		Y_pos = (-2) * Y_gap;
	else if (Z_gap >= X_gap 
		&& Z_gap >= Y_gap)
		Y_pos = (-2) * Z_gap;

	out << Indent(level);
	out << "<Viewpoint";

	out << " position='";
	out << NumTool::DoubleToWString(X_pos) << " ";
	out << NumTool::DoubleToWString(Y_pos) << " ";
	out << NumTool::DoubleToWString(Z_pos) << "'";

	out << " orientation='";
	out << NumTool::DoubleToWString(X_ori) << " ";
	out << NumTool::DoubleToWString(Y_ori) << " ";
	out << NumTool::DoubleToWString(Z_ori) << " ";
	out << NumTool::DoubleToWString(R_ori) << "'";

	out << " centerOfRotation='";
	out << NumTool::DoubleToWString(X_mean) << " ";
	out << NumTool::DoubleToWString(Y_mean) << " ";
	out << NumTool::DoubleToWString(Z_mean) << "'";

	out << "></Viewpoint>\n";
}

void X3D_Writer::WriteModel(X3D_Text& out, Model*& model, int level)
{
	if (model->GetRootComponentSize() >= 2)
	{
		out << Indent(level);
		out << "<Group>\n";
		CountIndent(level);
	}
	else
		level--;

	// Write root components
	for (int i = 0; i < model->GetRootComponentSize(); ++i)
	{
		Component* rootComp = model->GetRootComponentAt(i);

		if (m_opt->SFA() // SFA-specific
			&& rootComp->GetSubComponentSize() == 0
			&& rootComp->GetIShapeSize() == 1
			&& rootComp->GetIShapeAt(0)->IsSketchGeometry())
		{
			IShape* shape = rootComp->GetIShapeAt(0);
			WriteSketchGeometry(out, shape, level + 1);
		}
		else
		{
			out << Indent(level + 1);
			out << "<Group";
			
			if (m_opt->SFA() 
				&& m_opt->GDT())
				out << " id='geometry'"; 
			
			out << " DEF='" << rootComp->GetName() << "'>\n";
			CountIndent(level + 1);
			
			WriteComponent(out, rootComp, level + 1);
			
			out << Indent(level + 1);
			out << "</Group>\n";
		}
	}
	
	if (model->GetRootComponentSize() >= 2)
	{
		out << Indent(level);
		out << "</Group>\n";
	}
}

void X3D_Writer::WriteComponent(X3D_Text& out, Component*& comp, int level)
{
	for (int i = 0; i < comp->GetSubComponentSize(); ++i)
	{
		Component* subComp = comp->GetSubComponentAt(i);
		bool isTransformed = OCCUtil::IsTransformed(subComp->GetTransformation());
		
		if (isTransformed)
		{
			out << Indent(level + 1);
			out << "<Transform";
			CountIndent(level + 1);

			if (m_opt->SFA())
				out << " id='" << subComp->GetName() << "'";
			
			// Transform attributes i.e. Translation and Rotation
			WriteTransformAttributes(out, subComp->GetTransformation());
			out << ">\n";
		}
		else
			level--;

		if (subComp->IsCopy())
		{
			if (m_opt->SFA() // SFA-specific
				&& subComp->GetOriginalComponent()->GetSubComponentSize() == 0
				&& subComp->GetOriginalComponent()->GetIShapeSize() == 1
				&& subComp->GetOriginalComponent()->GetIShapeAt(0)->IsSketchGeometry())
			{
				IShape* shape = subComp->GetOriginalComponent()->GetIShapeAt(0);
				WriteSketchGeometry(out, shape, level + 2);
			}
			else
			{
				wstring orgCompName = subComp->GetOriginalComponent()->GetName();

				out << Indent(level + 2);
				out << "<Group USE='" << orgCompName;
				
				if (m_opt->SFA())
					out << "'></Group>\n";
				else
					out << "'/>\n";

				CountIndent(level + 2);
			}
		}
		else
		{
			if (m_opt->SFA() // SFA-specific
				&& subComp->GetSubComponentSize() == 0
				&& subComp->GetIShapeSize() == 1
				&& subComp->GetIShapeAt(0)->IsSketchGeometry())
			{
				IShape* shape = subComp->GetIShapeAt(0);
				WriteSketchGeometry(out, shape, level + 2);
			}
			else
			{
				out << Indent(level + 2);
				out << "<Group";
				
				if (m_opt->SFA()
					&& subComp->GetStepID() != -1)
					out << " id='msb " << subComp->GetStepID() << "'";

				out << " DEF='" << subComp->GetName() << "'>\n";
				CountIndent(level + 2);

				WriteComponent(out, subComp, level + 2); // Recursive call

				out << Indent(level + 2);
				out << "</Group>\n";
			}
		}

		if (isTransformed)
		{
			out << Indent(level + 1);
			out << "</Transform>\n";
		}
		else
			level++;
	}

	// Write shape nodes
	for (int i = 0; i < comp->GetIShapeSize(); ++i)
	{
		IShape* iShape = comp->GetIShapeAt(i);

		if (iShape->IsRosette()
			|| iShape->IsSectionCap())
			continue;

		try
		{
			WriteShape(out, iShape, level + 1);
		}
		catch (...)
		{
			wcout << "Writing X3D has failed on Shape: " << iShape->GetName() << endl;
		}
	}

	if (m_opt->Rosette()
		&& comp->HasRosette())
		WriteRosetteGeometry(out, comp, level + 1);

	if (m_opt->SectionCap()
		&& comp->HasSectionCap())
		WriteSectionCapGeometry(out, comp, level + 1);
}

void X3D_Writer::WriteTransformAttributes(X3D_Text& out, const gp_Trsf& trsf) const
{
	if (OCCUtil::IsTranslated(trsf))
	{
		const gp_XYZ& trans = trsf.TranslationPart();

		out << " translation='";
		out << NumTool::DoubleToWString(trans.X()) << " ";
		out << NumTool::DoubleToWString(trans.Y()) << " ";
		out << NumTool::DoubleToWString(trans.Z()) << "'";
	}

	if (OCCUtil::IsRotated(trsf))
	{
		gp_Vec rotAxis;
		double rotAngle = 0.0;
		trsf.GetRotation().GetVectorAndAngle(rotAxis, rotAngle);

		out << " rotation='";
		out << NumTool::DoubleToWString(rotAxis.X()) << " ";
		out << NumTool::DoubleToWString(rotAxis.Y()) << " ";
		out << NumTool::DoubleToWString(rotAxis.Z()) << " ";
		out << NumTool::DoubleToWString(rotAngle) << "'";
	}
}

void X3D_Writer::WriteShape(X3D_Text& out, IShape*& iShape, int level)
{
	if (iShape->IsFaceSet())
	{
		wstring shapeId = iShape->GetUniqueName();
		
		out << Indent(level);
		out << "<Shape";

		if (m_opt->SFA())
			out << " id='" << shapeId << "'";

		out << " DEF='" << iShape->GetName() << "'";
		out << ">\n";

		WriteIndexedFaceSet(out, iShape, level + 1);

		out << Indent(level);
		out << "</Shape>\n";

		if (m_opt->Edge()) // Boundary edges
		{
			out << Indent(level);
			out << "<Shape";

			if (m_opt->SFA())
				out << " id='" << shapeId << "'";

			out << " DEF='" << iShape->GetName() << "_edges'";
			out << ">\n";

			WriteIndexedLineSet(out, iShape, level + 1);

			out << Indent(level);
			out << "</Shape>\n";
		}
	}
	else // Sketch geometry
	{
		out << Indent(level);
		out << "<Shape";

		if (m_opt->SFA()
			&& iShape->GetStepID() != -1
			&& iShape->IsRosette())
			out << " id='curve 11 " << iShape->GetStepID() << "'";

		if (!m_opt->SFA())
			out << " DEF='" << iShape->GetName() << "'";

		out << ">\n";

		WriteIndexedLineSet(out, iShape, level + 1);

		out << Indent(level);
		out << "</Shape>\n";
	}
}

void X3D_Writer::WriteIndexedFaceSet(X3D_Text& out, IShape*& iShape, int level)
{
	bool isMultiColored = iShape->IsMultiColored();
	bool isSingleTransparent = iShape->IsSingleTransparent();
	double transparency = m_transparency;

	// Write Appearance node
	out << Indent(level);

	if (isMultiColored)	// No diffuse color
	{
		if (isSingleTransparent)
			transparency = 1.0 - iShape->GetColor().Alpha();

		WriteAppearance(out, iShape, m_diffuseColor, false,
										m_emissiveColor, false,
										m_specularColor, true,
										m_shininess, true,
										m_ambientIntensity, false,
										transparency, isSingleTransparent);
	}
	else
	{
		Quantity_ColorRGBA color(m_diffuseColor);

		if (m_opt->Color())
			color = iShape->GetColor(); // Set diffuse color

		if (isSingleTransparent)
			transparency = 1.0 - color.Alpha();

		WriteAppearance(out, iShape, color.GetRGB(), true,
										m_emissiveColor, false,
										m_specularColor, true,
										m_shininess, true,
										m_ambientIntensity, false,
										transparency, isSingleTransparent);
	}

	bool isSectionCap = iShape->IsSectionCap();
	bool isTessSolidModel = iShape->IsTessSolidModel();

	// Open IndexedFaceSet
	out << Indent(level);
	out << "<IndexedFaceSet";

	if (!m_opt->Normal()
		&& !isTessSolidModel)
		out << " creaseAngle='" << NumTool::DoubleToWString(m_creaseAngle) << "'";
	
	out << " solid='false'";

	WriteCoordinateIndex(out, iShape, true);

	if (m_opt->Normal()
		&& !isSectionCap
		&& !isTessSolidModel)
		WriteNormalIndex(out, iShape);
	
	out << ">\n";

	// Write coordinates
	out << Indent(level + 1);
	WriteCoordinate(out, iShape, false);

	// Write normals
	if (m_opt->Normal()
		&& !isSectionCap
		&& !isTessSolidModel)
	{
		out << Indent(level + 1);
		WriteNormal(out, iShape);
	}

	// Write colors
	if (isMultiColored)
	{
		out << Indent(level + 1);
		WriteColor(out, iShape);
	}

	// Close IndexedFaceSet
	out << Indent(level);
	out << "</IndexedFaceSet>\n";
}

void X3D_Writer::WriteIndexedLineSet(X3D_Text& out, IShape*& iShape, int level)
{
	bool isMultiColored = iShape->IsMultiColored();

	// Write Appearance node
	if (iShape->IsRosette())
	{
		// Keep composite rosette appearance independent of edge materials.
		// Do not reuse Appearance via USE/DEF; viewers treat edge app IDs separately.
		out << Indent(level);
		out << "<Appearance><Material emissiveColor='0 0 0'></Material></Appearance>\n";
	}
	else if (iShape->IsSketchGeometry())
	{
		if (!isMultiColored)
		{
			Quantity_ColorRGBA color(m_emissiveColor);

			if (m_opt->Color())
				color = iShape->GetColor();

			out << Indent(level);
			WriteAppearance(out, iShape, m_diffuseColor, false,
											color.GetRGB(), true,
											m_specularColor, false,
											m_shininess, false,
											m_ambientIntensity, false,
											m_transparency, false);
		}
	}
	else
	{
		Quantity_Color color(0.0, 0.0, 0.0, Quantity_TOC_RGB);

		out << Indent(level);
		WriteAppearance(out, iShape, m_diffuseColor, false,
										color, true,
										m_specularColor, false,
										m_shininess, false,
										m_ambientIntensity, false,
										m_transparency, false);
	}

	// Open IndexedLineSet
	out << Indent(level);
	out << "<IndexedLineSet";
	WriteCoordinateIndex(out, iShape, false);
	out << ">\n";

	// Write coordinates
	out << Indent(level + 1);
	
	if (iShape->IsSketchGeometry())
		WriteCoordinate(out, iShape, false);
	else
		WriteCoordinate(out, iShape, true);

	// Write colors
	if (iShape->IsSketchGeometry()
		&& isMultiColored)
	{
		out << Indent(level + 1);
		WriteColor(out, iShape);
	}

	// Close IndexedLineSet
	out << Indent(level);
	out << "</IndexedLineSet>\n";
}

void X3D_Writer::WriteAppearance(X3D_Text& out, IShape*& iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
													const Quantity_Color& emissiveColor, bool isEmissiveOn,
													const Quantity_Color& specularColor, bool isSpecularOn,
													double& shininess, bool isShininessOn,
													double& ambientIntensity, bool isAmbientIntensityOn,
													double& transparency, bool isTransparencyOn)
{
	int appID = 0;

	//if (!m_opt->SFA()
	//	|| (m_opt->SFA() 
	//		&& iShape->IsFaceSet()))
	//{
		if (CheckSameAppearance(diffuseColor, isDiffuseOn,
			emissiveColor, isEmissiveOn,
			specularColor, isSpecularOn,
			shininess, isShininessOn,
			ambientIntensity, isAmbientIntensityOn,
			transparency, isTransparencyOn,
			appID))
		{
			out << "<Appearance USE='app" << to_wstring(appID) << "'></Appearance>\n";
			return;
		}
	//}

	// Write Appearance node
	out << "<Appearance";

	//if (!m_opt->SFA()
	//	|| (m_opt->SFA() 
	//		&& iShape->IsFaceSet()))
		out << " DEF='app" << to_wstring(appID) << "'";
	
	out << "><Material";

	if (m_opt->SFA()
		//&& iShape->IsFaceSet()
		)
		out << " id='mat" << to_wstring(appID) << "'";

	if (isDiffuseOn)
	{
		out << " diffuseColor='";
		AppendDisplayColor(out, diffuseColor);
		out << "'";
	}

	if (isEmissiveOn)
	{
		out << " emissiveColor='";
		AppendDisplayColor(out, emissiveColor);
		out << "'";
	}

	if (isSpecularOn)
	{
		out << " specularColor='";
		AppendDisplayColor(out, specularColor);
		out << "'";
	}

	if (isShininessOn)
	{
		out << " shininess='";
		out << NumTool::DoubleToWString(shininess) << "'";
	}

	if (isAmbientIntensityOn)
	{
		out << " ambientIntensity='";
		out << NumTool::DoubleToWString(ambientIntensity) << "'";
	}

	if (isTransparencyOn)
	{
		out << " transparency='";
		out << NumTool::DoubleToWString(transparency) << "'";
	}

	out << "></Material></Appearance>\n";
}

void X3D_Writer::WriteCoordinate(X3D_Text& out, IShape*& iShape, bool isBoundaryEdges) const
{
	out << "<Coordinate";

	if (!isBoundaryEdges)
	{
		if (m_opt->Edge()
			&& iShape->IsFaceSet())
			out << " DEF='c" << to_wstring(iShape->GetGlobalIndex()) << "'";

		out << " point='";
		bool firstCoordinate = true;

		for (int i = 0; i < iShape->GetMeshSize(); ++i)
		{
			Mesh* mesh = iShape->GetMeshAt(i);

			for (int j = 0; j < mesh->GetCoordinateSize(); ++j)
			{
				const gp_XYZ& coord = mesh->GetCoordinateAt(j);

				if (!firstCoordinate)
					out << " ";
				out << NumTool::DoubleToWString(coord.X()) << " ";
				out << NumTool::DoubleToWString(coord.Y()) << " ";
				out << NumTool::DoubleToWString(coord.Z());
				firstCoordinate = false;
			}
		}
	}
	else
	{
		out << " USE='c" << to_wstring(iShape->GetGlobalIndex());
	}

	if (m_opt->SFA())
		out << "'></Coordinate>\n";
	else
		out << "'/>\n";
}

void X3D_Writer::WriteCoordinateIndex(X3D_Text& out, IShape*& iShape, bool faceMesh) const
{
	out << " coordIndex='";
	bool firstIndex = true;
	auto writeIndex = [&out, &firstIndex](int index)
	{
		if (!firstIndex)
			out << " ";
		out << to_wstring(index);
		firstIndex = false;
	};

	int prevCoordCount = 0; // The number of previous coordinates

	for (int i = 0; i < iShape->GetMeshSize(); ++i)
	{
		Mesh* mesh = iShape->GetMeshAt(i);

		if (faceMesh) // Face mesh
		{
			// Traverse triangles
			for (int j = 0; j < mesh->GetFaceIndexSize(); ++j)
			{
				const TriIndex& faceIndex = mesh->GetFaceIndexAt(j);
				
				writeIndex(faceIndex[0] - 1 + prevCoordCount);
				writeIndex(faceIndex[1] - 1 + prevCoordCount);
				writeIndex(faceIndex[2] - 1 + prevCoordCount);
				writeIndex(-1);
			}
		}
		else // Edge mesh (Boundary edges, sketch geometry)
		{		
			// Traverse edges
			for (int j = 0; j < mesh->GetEdgeIndexSize(); ++j)
			{
				const EdgeIndex& edgeIndex = mesh->GetEdgeIndexAt(j);

				for (size_t k = 0; k < edgeIndex.size(); ++k)
				{
					int index = edgeIndex[k] - 1 + prevCoordCount;
					writeIndex(index);
					//cout << "			" << index << endl;
				}

				writeIndex(-1);
			}
		}

		prevCoordCount += mesh->GetCoordinateSize();
	}

	out << "'";
}

void X3D_Writer::WriteNormalIndex(X3D_Text& out, IShape*& iShape) const
{
	out << " normalIndex='";
	bool firstIndex = true;
	auto writeIndex = [&out, &firstIndex](int index)
	{
		if (!firstIndex)
			out << " ";
		out << to_wstring(index);
		firstIndex = false;
	};

	int prevCoordCount = 0; // The number of previous coordinates

	for (int i = 0; i < iShape->GetMeshSize(); ++i)
	{
		Mesh* mesh = iShape->GetMeshAt(i);

		// Traverse triangles
		for (int j = 0; j < mesh->GetNormalIndexSize(); ++j)
		{
			const TriIndex& normalIndex = mesh->GetNormalIndexAt(j);

			writeIndex(normalIndex[0] - 1 + prevCoordCount);
			writeIndex(normalIndex[1] - 1 + prevCoordCount);
			writeIndex(normalIndex[2] - 1 + prevCoordCount);
			writeIndex(-1);
		}

		prevCoordCount += mesh->GetCoordinateSize();
	}

	out << "'";
}

void X3D_Writer::WriteColor(X3D_Text& out, IShape*& iShape) const
{
	bool isMultiTransparent = iShape->IsMultiTransparent();

	if (isMultiTransparent)
		out << "<ColorRGBA color='";
	else
		out << "<Color color='";
	bool firstColor = true;

	// Write colors for each coordinate point
	for (int i = 0; i < iShape->GetMeshSize(); ++i)
	{
		Mesh* mesh = iShape->GetMeshAt(i);

		for (int j = 0; j < mesh->GetCoordinateSize(); ++j)
		{
			const Quantity_ColorRGBA& color = iShape->GetColor(mesh->GetShape());

			if (!firstColor)
				out << " ";
			AppendDisplayColor(out, color.GetRGB());

			if (isMultiTransparent)
			{
				double transparency = color.Alpha();
				out << " " << NumTool::DoubleToWString(transparency);
			}
			firstColor = false;
		}
	}

	if (isMultiTransparent)
		out << "'></ColorRGBA>\n";
	else
		out << "'></Color>\n";
}

void X3D_Writer::WriteNormal(X3D_Text& out, IShape*& iShape) const
{
	out << "<Normal vector='";
	bool firstNormal = true;

	for (int i = 0; i < iShape->GetMeshSize(); ++i)
	{
		Mesh* mesh = iShape->GetMeshAt(i);

		for (int j = 0; j < mesh->GetNormalSize(); ++j)
		{
			const gp_XYZ& normal = mesh->GetNormalAt(j);
			
			if (!firstNormal)
				out << " ";
			out << NumTool::DoubleToWString(normal.X()) << " ";
			out << NumTool::DoubleToWString(normal.Y()) << " ";
			out << NumTool::DoubleToWString(normal.Z());
			firstNormal = false;
		}
	}

	out << "'></Normal>\n";
}

const wstring X3D_Writer::Indent(int level) const
{
	wstring indent;
	wstring unit = L" ";	// space or tab

	for (int i = 0; i < level; ++i)
		indent += unit;

	return indent;
}

bool X3D_Writer::CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
									 const Quantity_Color& emissiveColor, bool isEmissiveOn,
									 const Quantity_Color& specularColor, bool isSpecularOn,
									 double& shininess, bool isShininessOn,
									 double& ambientIntensity, bool isAmbientIntensityOn,
									 double& transparency, bool isTransparencyOn,
									 int& appID)
{
	auto hashCombine = [](size_t& seed, size_t value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	};
	auto hashBool = [](bool value) -> size_t { return value ? 1u : 0u; };
	auto hashDoubleBits = [](double value) -> size_t
	{
		static_assert(sizeof(double) == sizeof(uint64_t), "unexpected double size");
		uint64_t bits = 0;
		memcpy(&bits, &value, sizeof(bits));
		return static_cast<size_t>(bits ^ (bits >> 32));
	};
	auto hashColorBits = [&](const Quantity_Color& color) -> size_t
	{
		size_t seed = 0;
		hashCombine(seed, hashDoubleBits(color.Red()));
		hashCombine(seed, hashDoubleBits(color.Green()));
		hashCombine(seed, hashDoubleBits(color.Blue()));
		return seed;
	};

	size_t exactKey = 0;
	hashCombine(exactKey, hashBool(isDiffuseOn));
	hashCombine(exactKey, hashBool(isEmissiveOn));
	hashCombine(exactKey, hashBool(isSpecularOn));
	hashCombine(exactKey, hashBool(isShininessOn));
	hashCombine(exactKey, hashBool(isAmbientIntensityOn));
	hashCombine(exactKey, hashBool(isTransparencyOn));
	if (isDiffuseOn) hashCombine(exactKey, hashColorBits(diffuseColor));
	if (isEmissiveOn) hashCombine(exactKey, hashColorBits(emissiveColor));
	if (isSpecularOn) hashCombine(exactKey, hashColorBits(specularColor));
	if (isShininessOn) hashCombine(exactKey, hashDoubleBits(shininess));
	if (isAmbientIntensityOn) hashCombine(exactKey, hashDoubleBits(ambientIntensity));
	if (isTransparencyOn) hashCombine(exactKey, hashDoubleBits(transparency));

	const auto exactHit = m_appearanceExactIndex.find(exactKey);
	if (exactHit != m_appearanceExactIndex.end())
	{
		for (int candidateID : exactHit->second)
		{
			const Appearance& app = m_appearances[candidateID];

			if (app.isDiffuseOn == isDiffuseOn
				&& ((isDiffuseOn && app.diffuseColor.IsEqual(diffuseColor))
					|| !isDiffuseOn) &&
				app.isEmissiveOn == isEmissiveOn
				&& ((isEmissiveOn && app.emissiveColor.IsEqual(emissiveColor))
					|| !isEmissiveOn) &&
				app.isSpecularOn == isSpecularOn
				&& ((isSpecularOn && app.specularColor.IsEqual(specularColor))
					|| !isSpecularOn) &&
				app.isShininessOn == isShininessOn
				&& ((isShininessOn && abs(app.shininess - shininess) <= Precision::Confusion())
					|| !isShininessOn) &&
				app.isAmbientIntensityOn == isAmbientIntensityOn
				&& ((isAmbientIntensityOn && abs(app.ambientIntensity - ambientIntensity) <= Precision::Confusion())
					|| !isAmbientIntensityOn) &&
				app.isTransparencyOn == isTransparencyOn
				&& ((isTransparencyOn && abs(app.transparency - transparency) <= Precision::Confusion())
					|| !isTransparencyOn))
			{
				appID = candidateID;
				return true;
			}
		}
	}

	// Fuzzy match fallback preserves previous IsEqual / Precision::Confusion semantics.
	for (int i = 0; i < (int)m_appearances.size(); ++i)
	{
		const Appearance& app = m_appearances[i];

		if (app.isDiffuseOn == isDiffuseOn
			&& ((isDiffuseOn && app.diffuseColor.IsEqual(diffuseColor))
				|| !isDiffuseOn) && 
			app.isEmissiveOn == isEmissiveOn
			&& ((isEmissiveOn && app.emissiveColor.IsEqual(emissiveColor))
				|| !isEmissiveOn) && 
			app.isSpecularOn == isSpecularOn
			&& ((isSpecularOn && app.specularColor.IsEqual(specularColor))
				|| !isSpecularOn) && 
			app.isShininessOn == isShininessOn
			&& ((isShininessOn && abs(app.shininess - shininess) <= Precision::Confusion())
				|| !isShininessOn) && 
			app.isAmbientIntensityOn == isAmbientIntensityOn
			&& ((isAmbientIntensityOn && abs(app.ambientIntensity - ambientIntensity) <= Precision::Confusion())
				|| !isAmbientIntensityOn) &&
			app.isTransparencyOn == isTransparencyOn
			&& ((isTransparencyOn && abs(app.transparency - transparency) <= Precision::Confusion())
				|| !isTransparencyOn))
		{
			appID = i;
			m_appearanceExactIndex[exactKey].push_back(appID);
			return true;
		}
	}
	
	// Save the current appearance
	Appearance app;
	app.diffuseColor = diffuseColor;
	app.emissiveColor = emissiveColor;
	app.specularColor = specularColor;
	app.shininess = shininess;
	app.ambientIntensity = ambientIntensity;
	app.transparency = transparency;
	app.isDiffuseOn = isDiffuseOn;
	app.isEmissiveOn = isEmissiveOn;
	app.isSpecularOn = isSpecularOn;
	app.isShininessOn = isShininessOn;
	app.isAmbientIntensityOn = isAmbientIntensityOn;
	app.isTransparencyOn = isTransparencyOn;
	m_appearances.push_back(app);

	// Save the latest order
	appID = (int)m_appearances.size() - 1;
	m_appearanceExactIndex[exactKey].push_back(appID);

	return false;
}

void X3D_Writer::WriteSketchGeometry(X3D_Text& out, IShape*& iShape, int level)
{
	out << Indent(level);
	out << "<Shape>\n";

	bool isMultiColored = iShape->IsMultiColored();

	// Write Appearance node
	if (!isMultiColored)
	{
		Quantity_Color color;

		if (m_opt->Color())
			color = iShape->GetColor().GetRGB();
		else
			color = m_emissiveColor;

		out << Indent(level + 1);
		out << "<Appearance><Material";
		out << " emissiveColor='";
		AppendDisplayColor(out, color);
		out << "'";
		out << "></Material></Appearance>\n";
	}

	// Open IndexedLineSet
	out << Indent(level + 1);
	out << "<IndexedLineSet";
	WriteCoordinateIndex(out, iShape, false);
	out << ">\n";

	// Write coordinates
	out << Indent(level + 2);
	WriteCoordinate(out, iShape, false);

	// Write colors
	if (isMultiColored)
	{
		out << Indent(level + 2);
		WriteColor(out, iShape);
	}

	// Close IndexedLineSet
	out << Indent(level + 1);
	out << "</IndexedLineSet>\n";

	out << Indent(level);
	out << "</Shape>\n";
}

void X3D_Writer::WriteRosetteGeometry(X3D_Text& out, Component*& comp, int level)
{
	if (m_opt->SFA()) // SFA-specific
	{
		out << "<!--composites-->\n";
		out << Indent(level) << "<Switch whichChoice='0' id='swComposites1'><Group>\n";
	}
	else
		level--;

	// Write shape nodes
	for (int i = 0; i < comp->GetIShapeSize(); ++i)
	{
		IShape* iShape = comp->GetIShapeAt(i);

		if (!iShape->IsRosette())
			continue;

		try
		{
			WriteShape(out, iShape, level + 1);
		}
		catch (...)
		{
			wcout << "Writing X3D has failed on Shape: " << iShape->GetName() << endl;
		}
	}

	if (m_opt->SFA()) // SFA-specific
	{
		out << Indent(level) << "</Group></Switch>\n";
	}
}

void X3D_Writer::WriteSectionCapGeometry(X3D_Text& out, Component*& comp, int level)
{
	int sectionCapCount = 0;
	int tempID = -1;

	vector<IShape*> sectionCaps;

	for (int i = 0; i < comp->GetIShapeSize(); ++i)
	{
		IShape* iShape = comp->GetIShapeAt(i);

		if (iShape->IsSectionCap())
			sectionCaps.push_back(iShape);
	}

	if (!m_opt->SFA())
		level--;

	// Write shape nodes
	for (int i = 0; i < (int)sectionCaps.size(); ++i)
	{
		IShape* iShape = sectionCaps[i];

		int sectionID = iShape->GetStepID();

		if (sectionID != tempID)
		{
			if (m_opt->SFA()) // SFA-specific
			{
				out << Indent(level) << "<Switch whichChoice='-1' id='swClippingCap";
				out << to_wstring(++sectionCapCount);
				out << "'><Group>\n";
			}
		}

		try
		{
			WriteShape(out, iShape, level + 1);
		}
		catch (...)
		{
			wcout << "Writing X3D has failed on Shape: " << iShape->GetName() << endl;
		}

		if (i == (int)sectionCaps.size() - 1
			|| sectionID != sectionCaps[i+1]->GetStepID())
		{
			if (m_opt->SFA()) // SFA-specific
			{
				out << Indent(level) << "</Group></Switch>\n";
			}
		}

		tempID = sectionID;
	}

}

void X3D_Writer::CountIndent(int level)
{
	// Used for counting indent of 'Transform' and 'Group' nodes
	if (m_indentCountMap.find(level) == m_indentCountMap.end())
		m_indentCountMap.insert({ level, 1 });
	else
	{
		int count = m_indentCountMap[level];
		count++;
		m_indentCountMap[level] = count;
	}
}

void X3D_Writer::PrintIndentCount(void)
{
	printf("Indent Count\n");

	for (map<int, int>::iterator it = m_indentCountMap.begin(); it != m_indentCountMap.end(); ++it)
		printf("indent %d - %d\n", it->first, it->second);
}

void X3D_Writer::PrintMaterialCount(void) const
{
	printf("Number of Materials: %d\n", (int)m_appearances.size());
}

void X3D_Writer::WriteGDT(X3D_Text& out, Model*& model, int level)
{
	out << Indent(level);
	out << "<Group";

	if (m_opt->SFA())
		out << " id='highlight'";
	
	out << " DEF='GD&T'>\n";

	for (int i = 0; i < model->GetGDTSize(); ++i)
	{
		GDT_Item* gdt = model->GetGDTAt(i);

		out << Indent(level + 1);
		out << "<Group";

		if (m_opt->SFA())
			out << " id='" << gdt->GetName().c_str() << "'";

		out << " DEF='" << gdt->GetName().c_str() << "'";
		out << ">\n";

		TopoDS_Shape shape;
		IShape* faceShape = new IShape(shape);
		IShape* edgeShape = new IShape(shape);

		for (int j = 0; j < gdt->GetMeshSize(); ++j)
		{
			Mesh* mesh = gdt->GetMeshAt(j);

			if (mesh->GetFaceIndexSize() > 0)
				faceShape->AddMesh(mesh);
			else
				edgeShape->AddMesh(mesh);
		}

		// face shape
		if (faceShape->GetMeshSize() > 0)
		{
			out << Indent(level + 2);
			out << "<Shape>\n";

			// Write Appearance node
			out << Indent(level + 3);

			WriteAppearance(out, faceShape, m_gdtColor, true,
				m_emissiveColor, false,
				m_specularColor, true,
				m_shininess, true,
				m_ambientIntensity, false,
				m_transparency, false);

			out << Indent(level + 3);

			out << "<IndexedFaceSet";

			if (!m_opt->Normal())
				out << " creaseAngle='" << NumTool::DoubleToWString(m_creaseAngle) << "'";

			out << " solid='false'";

			WriteCoordinateIndex(out, faceShape, true);

			if (m_opt->Normal())
				WriteNormalIndex(out, faceShape);

			out << ">\n";

			// Write coordinates
			out << Indent(level + 4);
			WriteCoordinate(out, faceShape, false);

			// Close IndexedFaceSet
			out << Indent(level + 3);
			out << "</IndexedFaceSet>\n";

			out << Indent(level + 2);
			out << "</Shape>\n";
		}

		// edge shape
		if (edgeShape->GetMeshSize() > 0)
		{
			out << Indent(level + 2);
			out << "<Shape>\n";

			// Write Appearance node
			out << Indent(level + 3);

			WriteAppearance(out, edgeShape, m_gdtColor, false,
				m_gdtColor2, true,
				m_specularColor, false,
				m_shininess, false,
				m_ambientIntensity, false,
				m_transparency, false);

			out << Indent(level + 3);
			out << "<IndexedLineSet";

			WriteCoordinateIndex(out, edgeShape, false);

			out << ">\n";

			// Write coordinates
			out << Indent(level + 4);
			WriteCoordinate(out, edgeShape, false);

			// Close IndexedFaceSet
			out << Indent(level + 3);
			out << "</IndexedLineSet>\n";

			out << Indent(level + 2);
			out << "</Shape>\n";
		}

		out << Indent(level + 1);
		out << "</Group>\n";
	}

	out << Indent(level);
	out << "</Group>\n";
}

void X3D_Writer::Clear(void)
{
	m_appearances.clear();
	m_appearanceExactIndex.clear();
	m_indentCountMap.clear();
}
