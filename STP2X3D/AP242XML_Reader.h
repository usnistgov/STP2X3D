#pragma once

class Model;

// Reader for AP242 Domain Model XML (.stpx) assembly packages.
// Geometry is loaded from external STEP files referenced by the XML.
class AP242XML_Reader
{
public:
	AP242XML_Reader(S2X_Option* opt);

	bool Read(Model* model);

private:
	S2X_Option* m_opt;
};
