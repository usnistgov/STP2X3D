// STP2X3D.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "STEP_Reader.h"
#include "Tessellator.h"
#include "X3D_Writer.h"
#include "StatsPrinter.h"
#include "GDT_Item.h"
#include "Mesh.h"
//#include <experimental/filesystem>
//#include <filesystem>

namespace fs = std::experimental::filesystem;
//namespace fs = std::filesystem;

void Test(S2X_Option* opt)
{
	Model* model = new Model();

	/** START_STEP **/
	wcout << "Reading a STEP file.." << endl;
	STEP_Reader sr(opt);
	if (!sr.ReadSTEP(model))
	{
		delete model;
		return;
	}
	/** END_STEP **/

	vector<Component*> comps;
	model->GetAllComponents(comps);

	ofstream fout;
	fout.open("C:\\Users\\User\\Desktop\\Result.txt");

	for (const auto& comp : comps)
	{
		for (int i = 0; i < comp->GetIShapeSize(); ++i)
		{
			IShape* iShape = comp->GetIShapeAt(i);
			const TopoDS_Shape& shape = iShape->GetShape();

			for (double k = 0.349065; k <= 0.349066; k = k + 0.0000001)
			{
				OCCUtil::TessellateShape(shape, 0.5, true, k, true);

				int triCount = 0;

				TopExp_Explorer ExpFace;
				for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next())
				{
					const TopoDS_Face& face = TopoDS::Face(ExpFace.Current());

					TopLoc_Location loc;

					const Handle(Poly_Triangulation)& myT = BRep_Tool::Triangulation(face, loc);

					// Skip if triangulation has failed
					if (!myT
						|| myT.IsNull())
						continue;

					triCount = triCount + myT->NbTriangles();
				}

				fout << k << "\t" << triCount << endl;
			}
			
		}
	}

	fout.close();
}

// Print out the usage
void PrintUsage(wstring exe, S2X_Option* opt)
{
	wcout << endl;
	wcout << "//////////////////////////////////////////////////" << endl;
	wcout << "//  NIST STEP to X3D Translator (STP2X3D) " << opt->Version() <<"  //" << endl;
	wcout << "//////////////////////////////////////////////////" << endl;
	wcout << endl;
	wcout << "[Usage]" << endl;
	wcout << " " << exe << " option1 value1 option2 value2.." << endl;
	wcout << endl;
	wcout << "[Options]" << endl;
	wcout << " --input      Input STEP file path" << endl;
	wcout << " --output     Output STEP file path (Input name is used if empty)" << endl;
	wcout << " --normal     Normal vector (1:yes, 0:no) default=" << opt->Normal() << endl;
	wcout << " --color      Color (1:yes, 0:no) default=" << opt->Color() << endl;
	wcout << " --edge       Boundary edges (1:yes, 0:no) default=" << opt->Edge() << endl;
	wcout << " --sketch     Sketch geometry (1:yes, 0:no) default=" << opt->Sketch() << endl;
	wcout << " --html       Output file type (1:html, 0:x3d) default=" << opt->Html() << endl;
	wcout << " --quality    Mesh quality (1-low to 10-high) default=" << opt->Quality() << endl;
	wcout << " --gdt        Geometric elements related to GD&T (1:yes, 0:no) default=" << opt->GDT() << endl;
	wcout << " --tess       Adaptive tessellation per each body (1:yes, 0:no) default=" << opt->Tessellation() << endl;
	wcout << " --rosette    Rosette used for Composite Design (1:yes, 0:no) default=" << opt->Rosette() << endl;
	wcout << " --cap        Cap geometries for sections (1:yes, 0:no) default=" << opt->SectionCap() << endl;
	wcout << " --tsolid     Tessellated solids (1:yes, 0:no) default=" << opt->TessSolid() << endl;
	wcout << " --batch      Processing multiple STEP files (1:include sub-directories, 0:current dir)" << endl;
	wcout << "              Followed by a folder path (e.g. --batch 0 c:\\)" << endl;
	wcout << endl;
	wcout << "[Examples]" << endl;
	wcout << " " << exe << " --input Model.stp --edge 1 --quality 7" << endl;
	wcout << " " << exe << " --html 1 --sketch 0 --input Model.step" << endl;
	wcout << " " << exe << " --color 0 --batch 1 C:\\Folder --normal 1" << endl;
	wcout << endl;
	wcout << "[Disclaimers]" << endl;
	wcout << " This software was developed at the National Institute of Standards and Technology by" << endl;
	wcout << " employees of the Federal Government in the course of their official duties. Pursuant" << endl;
	wcout << " to Title 17 Section 105 of the United States Code this software is not subject to" << endl;
	wcout << " copyright protection and is in the public domain. This software is an experimental" << endl;
	wcout << " system. NIST assumes no responsibility whatsoever for its use by other parties, and" << endl;
	wcout << " makes no guarantees, expressed or implied, about its quality, reliability, or any" << endl;
	wcout << " other characteristic. NIST Disclaimer : https://www.nist.gov/disclaimer" << endl;
	wcout << endl;
	wcout << " This software is provided by NIST as a public service. You may use, copy and" << endl;
	wcout << " distribute copies of the software in any medium, provided that you keep intact this" << endl;
	wcout << " entire notice. You may improve, modify and create derivative works of the software" << endl;
	wcout << " or any portion of the software, and you may copy and distribute such modifications" << endl;
	wcout << " or works. Modified works should carry a notice stating that you changed the software" << endl;
	wcout << " and should note the date and nature of any such change. Please explicitly" << endl;
	wcout << " acknowledge NIST as the source of the software." << endl;
	wcout << endl;
	wcout << "[Credits]" << endl;
	wcout << " -The translator is based on the Open CASCADE STEP Processor" << endl;
	wcout << "  (See https://dev.opencascade.org/doc/overview/html/occt_user_guides__step.html)" << endl;
	wcout << " -Developed and managed by Soonjo Kwon, former NIST associate" << endl;
}

// Set option values
bool SetOption(int argc, char * argv[], S2X_Option* opt)
{
	// Print out usage
	if (argc < 2)
	{
		string a = argv[0];
        wstring aw = StrTool::s2ws(a);
	
		PrintUsage(aw, opt);
		//cout << "WRONG USAGE" << std::endl;
		return false;
	}
	
	bool inputFlag = false;
	bool batchFlag = false;

	// Set options
	for (int i = 1; i < argc; ++i)
	{
		string stoken(argv[i]);
        wstring token = StrTool::s2ws(stoken);
	
		string stoken1(argv[i + 1]);
        wstring token1 = StrTool::s2ws(stoken1);

		//wcout << token << L" " << token1 << endl;

		if (token != L"--input"
			&& token != L"--output"
			&& token != L"--normal"
			&& token != L"--color"
			&& token != L"--edge"
			&& token != L"--sketch"
			&& token != L"--html"
			&& token != L"--quality"
			&& token != L"--gdt"
			&& token != L"--batch"
			&& token != L"--sfa"
			&& token != L"--tess"
			&& token != L"--cap"
			&& token != L"--rosette"
			&& token != L"--tsolid")
		{
			wcout << "No such option: " << token << endl;
			return false;
		}
		else
		{
			if (argc == i + 1
				|| (token == L"--batch" && argc == i + 2))
			{
				wcout << "No value for the option: " << token << endl;
				return false;
			}
			else
			{
				if (token == L"--input")
				{
					inputFlag = true;
					opt->SetInput(token1);
				}
				else if (token == L"--output")
				{
					opt->SetOutput(token1);
				}
				else if (token == L"--normal")
				{
					int normal = stoi(token1);
					opt->SetNormal(normal);

					if (normal != 0 
						&& normal != 1)
					{
						wcout << "normal must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--color")
				{
					int color = stoi(token1);
					opt->SetColor(color);

					if (color != 0 
						&& color != 1)
					{
						wcout << "color must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--edge")
				{
					int edge = stoi(token1);
					opt->SetEdge(edge);

					if (edge != 0 
						&& edge != 1)
					{
						wcout << "edge must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--sketch")
				{
					int sketch = stoi(token1);
					opt->SetSketch(sketch);

					if (sketch != 0
						&& sketch != 1)
					{
						wcout << "sketch must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--html")
				{
					int html = stoi(token1);
					opt->SetHtml(html);

					if (html != 0 
						&& html != 1)
					{
						wcout << "html must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--gdt")
				{
					int gdt = stoi(token1);
					opt->SetGDT(gdt == 1 ? true : false);

					if (gdt != 0
						&& gdt != 1)
					{
						wcout << "gdt must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--sfa")
				{
					int sfa = stoi(token1);
					opt->SetSFA(sfa == 1 ? true : false);

					if (sfa != 0
						&& sfa != 1)
					{
						wcout << "sfa must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--tess")
				{
					int tess = stoi(token1);
					opt->SetTessellation(tess);

					if (tess != 0
						&& tess != 1)
					{
						wcout << "tess must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--rosette")
				{
					int rosette = stoi(token1);
					opt->SetRosette(rosette == 1 ? true : false);

					if (rosette != 0
						&& rosette != 1)
					{
						wcout << "rosette must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--cap")
				{
					int cap = stoi(token1);
					opt->SetSectionCap(cap == 1 ? true : false);

					if (cap != 0
						&& cap != 1)
					{
						wcout << "cap must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--tsolid")
				{
					int tsolid = stoi(token1);
					opt->SetTessSolid(tsolid == 1 ? true : false);

					if (tsolid != 0
						&& tsolid != 1)
					{
						wcout << "tsolid must be either 0 or 1." << endl;
						return false;
					}
				}
				else if (token == L"--quality")
				{
					double quality = stof(token1);
					opt->SetQuality(quality);

					if (quality < 1.0 
						|| quality > 10.0)
					{
						wcout << "quality must be between 1 and 10." << endl;
						return false;
					}
				}
				else if (token == L"--batch")
				{
					batchFlag = true;
					int batch = stoi(token1);
					
					string stoken2(argv[i + 2]);
                    wstring token2 = StrTool::s2ws(stoken2);
			
					wstring input = token2;
					++i;

					opt->SetBatch(batch);
					opt->SetInput(input);

					if (batch != 0 
						&& batch != 1)
					{
						wcout << "batch must be either 0 or 1." << endl;
						return false;
					}
				}
					
				++i;
			}
		}
	}

	// Check flags
	if (inputFlag 
		&& batchFlag)
	{
		wcout << "--input and --batch cannot be used at the same time." << endl;
		return false;
	}

	// Check input path
	if (opt->Input().empty())
	{
		wcout << "Please input a STEP file." << endl;
		return false;
	}
	else if (!fs::is_directory(opt->Input())
		&& !fs::is_regular_file(opt->Input()))
	{
		wcout << "No such file or directory: " << opt->Input() << endl;
		return false;
	}

	return true;
}

// Run STEP to X3D translation given parameters
int RunSTP2X3D(S2X_Option* opt)
{
	Model* model = new Model();

	StopWatch sw;
	sw.Start();

	/** START_STEP **/
	wcout << "Reading a STEP file.." << endl;
	STEP_Reader sr(opt);
	if (!sr.ReadSTEP(model))
	{
		delete model;
		return -1;
	}
	/** END_STEP **/
	//sw.Lap();

	/** START_TESSELLATION **/
	wcout << "Tessellating.." << endl;
	Tessellator* ts = new Tessellator(opt);
	ts->Tessellate(model);
	delete ts;
	/** END_TESSELLATION **/
	//sw.Lap();

	/** START_X3D **/
	wcout << "Writing an X3D file.." << endl;
	X3D_Writer xw(opt);
	xw.WriteX3D(model);
	/** END_X3D **/
	//sw.Lap();

	/// Print results required for SFA
	if (opt->SFA())
	{
		StatsPrinter::PrintShapeCount(model);
		StatsPrinter::PrintBoundingBox(model, opt);
		StatsPrinter::PrintSketchExistence(model);
	}
	///

	wcout << "STEP to X3D completed!" << endl;
	sw.End();

	delete model;

	return 0;
}

// Batch run for STEP files under the given folder path
int BatchRun(S2X_Option* opt)
{
	vector<fs::path> paths;

	if (opt->Batch() == 0) // Only given directory
	{
		for (const auto& entry : fs::directory_iterator(opt->Input()))
			paths.push_back(entry.path());
	}
	else if (opt->Batch() == 1) // Include subdirectories
	{
		for (const auto& entry : fs::recursive_directory_iterator(opt->Input()))
			paths.push_back(entry.path());
	}

	int status = 0;

	for (const auto& path : paths)
	{
		wstring ext = path.filename().extension().generic_wstring();
		
		if (!(ext == L".stp" || ext == L".STP"
			|| ext == L".step" || ext == L".STEP"
			|| ext == L".p21" || ext == L".P21"))
			continue;
		
		wstring inFilePath = path.generic_wstring();

		S2X_Option tmp_opt(*opt);
		tmp_opt.SetInput(inFilePath);

		wcout << "[" << path.filename() << "]" << endl;
		status += RunSTP2X3D(&tmp_opt);
	}

	paths.clear();

	return status;
}

// Main entry function
int main(int argc, char * argv[])
{	
	S2X_Option opt; // Option for STEP to X3D translator
	
	int status = -1; // Translation status

#if _DEBUG
	opt.SetInput(L"C:\\Users\\User\\Desktop\\100LPH RO Machine.step");
	opt.SetNormal(1);
	opt.SetColor(1);
	opt.SetEdge(1);
	opt.SetSketch(1);
	opt.SetHtml(1);
	opt.SetQuality(7.0);
	opt.SetGDT(true);
	opt.SetSFA(true);
	opt.SetTessellation(0);
	opt.SetRosette(false);
	opt.SetSectionCap(false);
	opt.SetTessSolid(true);
#else
	if (!SetOption(argc, argv, &opt))
		return status;
#endif

	//Test(&opt);
	//return true;

	if (opt.Batch() == -1) // Translate an input STEP file
		status = RunSTP2X3D(&opt);
	else // Translate multiple STEP files
		status = BatchRun(&opt);
	
	return status;
}
