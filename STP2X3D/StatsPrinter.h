#pragma once
#include "Model.h"
#include "Component.h"
#include "IShape.h"


class StatsPrinter
{
public:

	static void PrintBoundingBox(Model* model, S2X_Option* opt)
	{
		Bnd_Box bndBox;

		// When sketch is off, always walk IShapes and skip sketch geometry.
		// Using the root TopoDS_Shape would still include free edges/wires that
		// were not loaded as IShapes but remain in the OCC compound.
		if (!opt->Sketch())
			bndBox = model->GetBoundingBox(false);
		else if (model->GetRootComponentSize() == 1)
		{
			const TopoDS_Shape& shape = model->GetRootComponentAt(0)->GetShape();
			bndBox = OCCUtil::ComputeBoundingBox(shape);
			bndBox = bndBox.FinitePart();
		}
		else
			bndBox = model->GetBoundingBox(true);

		// Sketch-only models with --sketch 0 (or otherwise empty geometry) yield a
		// void box. Do not fall back to including sketch; report a zero box.
		if (bndBox.IsVoid())
		{
			printf("MinXYZ: 0.0000 0.0000 0.0000\n");
			printf("MaxXYZ: 0.0000 0.0000 0.0000\n");
			return;
		}

		// Bounding boxes are enlarged by the given tolerance 
		// double tol = 2.5 / opt->Quality();	// before v1.02 (linear deflection for tessellation)
		double tol = bndBox.GetGap();			// after  v1.02 (gap set by OCC)

		double X_min = 0.0, Y_min = 0.0, Z_min = 0.0;
		double X_max = 0.0, Y_max = 0.0, Z_max = 0.0;

		bndBox.Get(X_min, Y_min, Z_min, X_max, Y_max, Z_max);

		// Add and subtract the tolerance
		printf("MinXYZ: %.4lf %.4lf %.4lf\n", X_min + tol, Y_min + tol, Z_min + tol);
		printf("MaxXYZ: %.4lf %.4lf %.4lf\n", X_max - tol, Y_max - tol, Z_max - tol);
	}

	static void PrintShapeCount(Model* model)
	{
		vector<Component*> comps;
		model->GetAllComponents(comps);
		
		int shapeCount = 0;

		for (const auto& comp : comps)
			shapeCount += comp->GetIShapeSize();

		comps.clear();

		printf("Number of Shapes: %d\n", shapeCount);
	}

	static void PrintSketchExistence(Model* model)
	{
		vector<Component*> comps;
		model->GetAllComponents(comps);
		
		int sketchCount = 0;
		int rosetteCount = 0;

		for (const auto& comp : comps)
		{
			for (int i = 0; i < comp->GetIShapeSize(); ++i)
			{
				IShape* iShape = comp->GetIShapeAt(i);
				
				if (iShape->IsSketchGeometry())
				{
					if (iShape->IsRosette())
						rosetteCount++;
					else
						sketchCount++;
				}	
			}
		}

		comps.clear();

		if (sketchCount > 0)
			printf("Sketch geometry was found.\n");

		if (rosetteCount > 0)
			printf("Number of Rosettes: %d\n", rosetteCount);
		
	}
};