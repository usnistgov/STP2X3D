#pragma once

class S2X_Option
{
public:
	S2X_Option();
	~S2X_Option();

	void SetInput(const wstring& input) { m_input = input; }
	void SetNormal(const int& normal);
	void SetColor(const int& color);
	void SetEdge(const int& edge);
	void SetSketch(const int& sketch);
	void SetHtml(const int& html);
	void SetTessellation(const int& tessellation);
	void SetQuality(const double& quality) { m_quality = quality; }
	void SetBatch(const int& batch) { m_batch = batch; }
	void SetSFA(bool sfa) { m_SFA = sfa; }
	void SetGDT(bool gdt) { m_gdt = gdt; }

	const wstring& Input(void) const { return m_input; }
	const wstring Output(void) const;
	bool Normal(void) const { return m_normal; }
	bool Color(void) const { return m_color; }
	bool Edge(void) const { return m_edge; }
	bool Sketch(void) const { return m_sketch; }
	bool Html(void) const { return m_html; }
	bool Tessellation(void) const { return m_tessellation; }
	double Quality(void) const { return m_quality; }
	int Batch(void) const { return m_batch; }
	bool SFA(void) const { return m_SFA; }
	bool GDT(void) const { return m_gdt; }

	// Software version (as of Jan 2021)
	const wstring Version(void) const { return L"1.10"; }

private:
	wstring m_input;	// Input file path
	bool m_normal;		// Normal
	bool m_color;		// Color
	bool m_edge;		// Boundary edge
	bool m_sketch;		// Sketch geometry
	bool m_html;		// Output file type, html or x3d
	bool m_tessellation;// Tessellation type
	double m_quality;	// Mesh quality
	int m_batch;		// Batch option
	bool m_SFA;			// Specific to SFA
	bool m_gdt;			// GD&T option
};