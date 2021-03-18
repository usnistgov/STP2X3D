#pragma once

enum ShapeType
{
	Sketch_Geom,	// A shape has only sketch geometries
	Face_Geom,		// A shape has only face geometries
	Hybrid_Geom		// A shape has both sketch and face geometries
};