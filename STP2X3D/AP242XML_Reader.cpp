#include "stdafx.h"
#include "AP242XML_Reader.h"
#include "STEP_Reader.h"
#include "Component.h"
#include "IShape.h"

#include <fstream>
#include <memory>
#include <set>

namespace
{
	struct XmlNode
	{
		string name;
		string text;
		unordered_map<string, string> attributes;
		vector<unique_ptr<XmlNode>> children;
		XmlNode* parent = nullptr;
	};

	string Trim(const string& value)
	{
		const size_t begin = value.find_first_not_of(" \t\r\n");
		if (begin == string::npos)
			return "";

		const size_t end = value.find_last_not_of(" \t\r\n");
		return value.substr(begin, end - begin + 1);
	}

	string DecodeXml(string value)
	{
		const pair<const char*, const char*> replacements[] =
		{
			{ "&amp;", "&" },
			{ "&lt;", "<" },
			{ "&gt;", ">" },
			{ "&quot;", "\"" },
			{ "&apos;", "'" }
		};

		for (const auto& replacement : replacements)
		{
			size_t position = 0;
			while ((position = value.find(replacement.first, position)) != string::npos)
			{
				value.replace(position, strlen(replacement.first), replacement.second);
				position += strlen(replacement.second);
			}
		}

		return value;
	}

	string LocalName(const string& name)
	{
		const size_t separator = name.find(':');
		return separator == string::npos ? name : name.substr(separator + 1);
	}

	string Attribute(const XmlNode* node, const string& name)
	{
		if (!node)
			return "";

		const auto direct = node->attributes.find(name);
		if (direct != node->attributes.end())
			return direct->second;

		for (const auto& attribute : node->attributes)
		{
			if (LocalName(attribute.first) == name)
				return attribute.second;
		}
		return "";
	}

	string TypeName(const XmlNode* node)
	{
		return LocalName(Attribute(node, "type"));
	}

	const XmlNode* Child(const XmlNode* node, const string& name)
	{
		if (!node)
			return nullptr;

		for (const auto& child : node->children)
		{
			if (LocalName(child->name) == name)
				return child.get();
		}
		return nullptr;
	}

	void FindDescendants(const XmlNode* node, const string& name, vector<const XmlNode*>& result)
	{
		if (!node)
			return;

		for (const auto& child : node->children)
		{
			if (LocalName(child->name) == name)
				result.push_back(child.get());
			FindDescendants(child.get(), name, result);
		}
	}

	const XmlNode* FirstDescendant(const XmlNode* node, const string& name)
	{
		if (!node)
			return nullptr;

		for (const auto& child : node->children)
		{
			if (LocalName(child->name) == name)
				return child.get();

			if (const XmlNode* found = FirstDescendant(child.get(), name))
				return found;
		}
		return nullptr;
	}

	string NodeText(const XmlNode* node)
	{
		return node ? Trim(DecodeXml(node->text)) : "";
	}

	string TextAt(const XmlNode* node, initializer_list<const char*> path)
	{
		const XmlNode* current = node;
		for (const char* name : path)
			current = Child(current, name);
		return NodeText(current);
	}

	class SimpleXmlDocument
	{
	public:
		bool Load(const filesystem::path& path, string& error)
		{
			ifstream input(path, ios::binary);
			if (!input)
			{
				error = "cannot open XML file";
				return false;
			}

			const string xml((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
			return Parse(xml, error);
		}

		const XmlNode* Root(void) const
		{
			return m_document.children.empty() ? nullptr : m_document.children.front().get();
		}

	private:
		bool Parse(const string& xml, string& error)
		{
			m_document = XmlNode();
			m_document.name = "#document";
			vector<XmlNode*> stack = { &m_document };
			size_t position = 0;

			while (position < xml.size())
			{
				const size_t open = xml.find('<', position);
				if (open == string::npos)
				break;

				if (open > position)
					stack.back()->text += DecodeXml(xml.substr(position, open - position));

				if (xml.compare(open, 4, "<!--") == 0)
				{
					const size_t end = xml.find("-->", open + 4);
					if (end == string::npos)
						return Fail(error, "unterminated XML comment");
					position = end + 3;
					continue;
				}

				if (xml.compare(open, 9, "<![CDATA[") == 0)
				{
					const size_t end = xml.find("]]>", open + 9);
					if (end == string::npos)
						return Fail(error, "unterminated CDATA");
					stack.back()->text += xml.substr(open + 9, end - open - 9);
					position = end + 3;
					continue;
				}

				if (xml.compare(open, 2, "<?") == 0)
				{
					const size_t end = xml.find("?>", open + 2);
					if (end == string::npos)
						return Fail(error, "unterminated XML declaration");
					position = end + 2;
					continue;
				}

				if (xml.compare(open, 2, "<!") == 0)
				{
					const size_t end = xml.find('>', open + 2);
					if (end == string::npos)
						return Fail(error, "unterminated XML directive");
					position = end + 1;
					continue;
				}

				size_t close = open + 1;
				char quote = 0;
				for (; close < xml.size(); ++close)
				{
					const char current = xml[close];
					if (quote)
					{
						if (current == quote)
							quote = 0;
					}
					else if (current == '\'' || current == '"')
						quote = current;
					else if (current == '>')
						break;
				}

				if (close >= xml.size())
					return Fail(error, "unterminated XML tag");

				string tag = Trim(xml.substr(open + 1, close - open - 1));
				position = close + 1;

				if (!tag.empty() && tag.front() == '/')
				{
					if (stack.size() <= 1)
						return Fail(error, "unexpected closing tag");
					stack.pop_back();
					continue;
				}

				bool selfClosing = false;
				if (!tag.empty() && tag.back() == '/')
				{
					selfClosing = true;
					tag = Trim(tag.substr(0, tag.size() - 1));
				}

				size_t cursor = 0;
				while (cursor < tag.size() && !isspace(static_cast<unsigned char>(tag[cursor])))
					++cursor;

				auto node = make_unique<XmlNode>();
				node->name = tag.substr(0, cursor);
				node->parent = stack.back();

				while (cursor < tag.size())
				{
					while (cursor < tag.size() && isspace(static_cast<unsigned char>(tag[cursor])))
						++cursor;
					if (cursor >= tag.size())
						break;

					const size_t nameBegin = cursor;
					while (cursor < tag.size()
						&& !isspace(static_cast<unsigned char>(tag[cursor]))
						&& tag[cursor] != '=')
						++cursor;
					const string attrName = tag.substr(nameBegin, cursor - nameBegin);

					while (cursor < tag.size() && isspace(static_cast<unsigned char>(tag[cursor])))
						++cursor;
					if (cursor >= tag.size() || tag[cursor] != '=')
						return Fail(error, "invalid XML attribute");
					++cursor;
					while (cursor < tag.size() && isspace(static_cast<unsigned char>(tag[cursor])))
						++cursor;

					if (cursor >= tag.size() || (tag[cursor] != '\'' && tag[cursor] != '"'))
						return Fail(error, "unquoted XML attribute");
					const char attrQuote = tag[cursor++];
					const size_t valueBegin = cursor;
					const size_t valueEnd = tag.find(attrQuote, cursor);
					if (valueEnd == string::npos)
						return Fail(error, "unterminated XML attribute");

					node->attributes[attrName] = DecodeXml(tag.substr(valueBegin, valueEnd - valueBegin));
					cursor = valueEnd + 1;
				}

				XmlNode* inserted = node.get();
				stack.back()->children.push_back(std::move(node));
				if (!selfClosing)
					stack.push_back(inserted);
			}

			if (stack.size() != 1)
				return Fail(error, "unclosed XML element");
			return Root() != nullptr;
		}

		static bool Fail(string& error, const string& message)
		{
			error = message;
			return false;
		}

		XmlNode m_document;
	};

	wstring Utf8ToWide(const string& text)
	{
		if (text.empty())
			return L"";
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(text);
	}

	vector<double> ParseDoubles(const string& text)
	{
		vector<double> values;
		istringstream input(text);
		input.imbue(locale::classic());
		double value = 0.0;
		while (input >> value)
			values.push_back(value);
		return values;
	}

	gp_Trsf ParseTransformation(const XmlNode* relationship)
	{
		gp_Trsf transformation;
		const XmlNode* cartesian = FirstDescendant(relationship, "CartesianTransformation");
		if (!cartesian)
			return transformation;

		const vector<double> rotation = ParseDoubles(NodeText(Child(cartesian, "RotationMatrix")));
		const vector<double> translation = ParseDoubles(NodeText(Child(cartesian, "TranslationVector")));
		if (rotation.size() != 9 || translation.size() != 3)
			return transformation;

		// RotationMatrix lists the target X/Y/Z axis vectors (xx xy xz yx yy yz zx zy zz),
		// i.e. the columns of the rotation matrix. gp_Trsf::SetValues expects rows.
		transformation.SetValues(
			rotation[0], rotation[3], rotation[6], translation[0],
			rotation[1], rotation[4], rotation[7], translation[1],
			rotation[2], rotation[5], rotation[8], translation[2]);
		return transformation;
	}

	struct RelationshipInfo
	{
		string occurrenceUid;
		gp_Trsf transformation;
	};

	struct PartInfo
	{
		string uid;
		wstring name;
		bool isAssembly = false;
		string viewUid;
		string geometryUid;
		string fileUid;
		vector<string> occurrenceUids;
		vector<RelationshipInfo> relationships;
	};

	class ReaderImpl
	{
	public:
		ReaderImpl(
			S2X_Option* opt,
			const filesystem::path& xmlPath,
			set<filesystem::path>& readingFiles)
			: m_opt(opt), m_xmlPath(xmlPath), m_readingFiles(readingFiles)
		{
		}

		bool Read(Model* model)
		{
			Component* rootComponent = ReadComponent();
			if (!rootComponent)
				return false;

			rootComponent->SetTransformation(gp_Trsf());
			model->AddRootComponent(rootComponent);
			model->Update();
			return !model->IsEmpty();
		}

		Component* ReadComponent()
		{
			const filesystem::path normalizedPath =
				filesystem::absolute(m_xmlPath).lexically_normal();
			if (!m_readingFiles.insert(normalizedPath).second)
			{
				Report("Cyclic AP242 XML file reference: " + normalizedPath.u8string());
				return nullptr;
			}

			Component* component = ReadComponentInternal();
			m_readingFiles.erase(normalizedPath);
			return component;
		}

	private:
		Component* ReadComponentInternal()
		{
			string error;
			if (!m_document.Load(m_xmlPath, error))
			{
				Report("AP242 XML parse error in " + m_xmlPath.u8string() + ": " + error);
				return nullptr;
			}

			const XmlNode* root = m_document.Root();
			if (!root || LocalName(root->name) != "Uos")
			{
				Report("AP242 XML root element Uos was not found.");
				return nullptr;
			}

			const string schema = Attribute(root, "schemaLocation");
			if (schema.find("domain_model") == string::npos)
			{
				Report("Input is not an AP242 Domain Model XML document.");
				return nullptr;
			}

			CollectFiles(root);
			CollectGeometryFiles(root);
			CollectParts(root);
			if (m_parts.empty())
			{
				Report("No AP242 Part definitions were found.");
				return nullptr;
			}

			PartInfo* rootPart = FindRootPart();
			if (!rootPart)
			{
				Report("No root part was found in AP242 XML.");
				return nullptr;
			}

			Component* rootComponent = BuildPart(*rootPart);
			if (!rootComponent)
				return nullptr;
			return rootComponent;
		}

		void CollectFiles(const XmlNode* root)
		{
			vector<const XmlNode*> files;
			FindDescendants(root, "File", files);
			for (const XmlNode* file : files)
			{
				const string uid = Attribute(file, "uid");
				string filename = Attribute(Child(Child(Child(file, "Locations"), "ExternalItem"), "Id"), "id");
				if (filename.empty())
					filename = TextAt(file, { "Locations", "ExternalItem", "Id" });
				if (filename.empty())
					filename = NodeText(FirstDescendant(file, "SourceId"));
				if (filename.empty())
				{
					const XmlNode* identifier = FirstDescendant(file, "Identifier");
					filename = Attribute(identifier, "id");
				}
				if (!uid.empty() && !filename.empty())
					m_fileNames[uid] = filesystem::u8path(filename);
			}
		}

		void CollectGeometryFiles(const XmlNode* root)
		{
			vector<const XmlNode*> representations;
			FindDescendants(root, "Representation", representations);
			for (const XmlNode* representation : representations)
			{
				if (TypeName(representation) != "ExternalGeometricModel")
					continue;

				const string uid = Attribute(representation, "uid");
				const string fileUid = Attribute(Child(representation, "ExternalFile"), "uidRef");
				if (!uid.empty() && !fileUid.empty())
					m_geometryFiles[uid] = fileUid;
			}
		}

		void CollectParts(const XmlNode* root)
		{
			vector<const XmlNode*> partNodes;
			FindDescendants(root, "Part", partNodes);

			for (const XmlNode* partNode : partNodes)
			{
				PartInfo part;
				part.uid = Attribute(partNode, "uid");
				part.name = Utf8ToWide(TextAt(partNode, { "Name", "CharacterString" }));
				if (part.name.empty())
				{
					const XmlNode* identifier = FirstDescendant(Child(partNode, "Id"), "Identifier");
					part.name = Utf8ToWide(Attribute(identifier, "id"));
				}

				const string partType = TextAt(partNode, { "PartTypes", "ClassString" });
				const XmlNode* partView = FirstDescendant(partNode, "PartView");
				if (!partView)
					continue;

				part.viewUid = Attribute(partView, "uid");
				part.geometryUid = Attribute(Child(partView, "DefiningGeometry"), "uidRef");

				const XmlNode* assignedDocument = FirstDescendant(partNode, "AssignedDocument");
				part.fileUid = Attribute(assignedDocument, "uidRef");

				vector<const XmlNode*> occurrences;
				FindDescendants(partView, "Occurrence", occurrences);
				for (const XmlNode* occurrence : occurrences)
				{
					const string occurrenceUid = Attribute(occurrence, "uid");
					if (!occurrenceUid.empty())
					{
						part.occurrenceUids.push_back(occurrenceUid);
						m_occurrenceParts[occurrenceUid] = part.uid;
					}
				}

				vector<const XmlNode*> relationships;
				FindDescendants(partView, "ViewOccurrenceRelationship", relationships);
				for (const XmlNode* relationship : relationships)
				{
					if (TypeName(relationship) != "NextAssemblyOccurrenceUsage")
						continue;

					RelationshipInfo info;
					info.occurrenceUid = Attribute(Child(relationship, "Related"), "uidRef");
					info.transformation = ParseTransformation(relationship);
					if (!info.occurrenceUid.empty())
						part.relationships.push_back(info);
				}

				// Some exporters label intermediate nodes as piece parts while still
				// emitting next-assembly relationships under the same PartView.
				part.isAssembly = partType == "assembly"
					|| TypeName(partView) == "AssemblyDefinition"
					|| !part.relationships.empty();

				m_parts[part.uid] = std::move(part);
			}
		}

		PartInfo* FindRootPart(void)
		{
			for (auto& entry : m_parts)
			{
				PartInfo& part = entry.second;
				if (part.isAssembly && part.occurrenceUids.empty())
					return &part;
			}

			for (auto& entry : m_parts)
			{
				if (entry.second.isAssembly)
					return &entry.second;
			}

			// A nested AP242 document can contain one standalone piece part.
			// It is still a valid root even though it has no assembly definition.
			for (auto& entry : m_parts)
			{
				if (entry.second.occurrenceUids.empty())
					return &entry.second;
			}
			if (!m_parts.empty())
				return &m_parts.begin()->second;
			return nullptr;
		}

		Component* BuildPart(PartInfo& part)
		{
			const auto existing = m_originalComponents.find(part.uid);
			if (existing != m_originalComponents.end())
			{
				Component* original = existing->second;
				Component* copy = new Component(original->GetShape());
				copy->SetUniqueName(part.name);
				copy->SetOriginalComponent(original);
				return copy;
			}

			if (m_building.count(part.uid) != 0)
			{
				Report("Cyclic AP242 assembly reference: " + part.uid);
				return nullptr;
			}
			m_building.insert(part.uid);

			Component* component = part.isAssembly ? BuildAssembly(part) : LoadExternalGeometry(part);
			m_building.erase(part.uid);
			if (!component)
				return nullptr;

			component->SetUniqueName(part.name);
			m_originalComponents[part.uid] = component;
			return component;
		}

		Component* BuildAssembly(PartInfo& part)
		{
			TopoDS_Compound compound;
			BRep_Builder builder;
			builder.MakeCompound(compound);
			Component* assembly = new Component(compound);

			auto failAssembly = [&](const string& message) -> Component*
			{
				if (!message.empty())
					Report(message);
				UnregisterOriginalsInTree(assembly);
				delete assembly;
				return nullptr;
			};

			for (const RelationshipInfo& relationship : part.relationships)
			{
				const auto occurrence = m_occurrenceParts.find(relationship.occurrenceUid);
				if (occurrence == m_occurrenceParts.end())
					return failAssembly("Unresolved AP242 occurrence: " + relationship.occurrenceUid);

				auto childPart = m_parts.find(occurrence->second);
				if (childPart == m_parts.end())
					return failAssembly("Unresolved AP242 part: " + occurrence->second);

				Component* child = BuildPart(childPart->second);
				if (!child)
					return failAssembly("");

				child->SetTransformation(relationship.transformation);
				assembly->AddSubComponent(child);

				if (!child->GetShape().IsNull())
					builder.Add(compound, child->GetShape().Moved(TopLoc_Location(relationship.transformation)));
			}

			assembly->SetShape(compound);
			return assembly;
		}

		void UnregisterOriginalsInTree(Component* component)
		{
			if (!component)
				return;

			for (int i = 0; i < component->GetSubComponentSize(); ++i)
				UnregisterOriginalsInTree(component->GetSubComponentAt(i));

			if (component->IsCopy())
				return;

			for (auto it = m_originalComponents.begin(); it != m_originalComponents.end(); )
			{
				if (it->second == component)
					it = m_originalComponents.erase(it);
				else
					++it;
			}
		}

		Component* LoadExternalGeometry(PartInfo& part)
		{
			string fileUid = part.fileUid;
			if (fileUid.empty())
			{
				const auto geometry = m_geometryFiles.find(part.geometryUid);
				if (geometry != m_geometryFiles.end())
					fileUid = geometry->second;
			}

			// Kinematic AP242 models may contain logical link occurrences without
			// an assigned geometry document. Preserve the assembly node and let
			// normal model cleanup omit it from tessellation.
			if (fileUid.empty())
				return new Component(TopoDS_Shape());

			const auto file = m_fileNames.find(fileUid);
			if (file == m_fileNames.end())
			{
				Report("No external STEP file for AP242 part: " + part.uid);
				return nullptr;
			}

			const filesystem::path externalPath =
				(m_xmlPath.parent_path() / file->second).lexically_normal();
			if (!filesystem::is_regular_file(externalPath))
			{
				Report("Referenced AP242 file does not exist: " + externalPath.u8string());
				return nullptr;
			}

			string extension = externalPath.extension().u8string();
			transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char value) { return static_cast<char>(tolower(value)); });
			if (extension == ".stpx")
			{
				ReaderImpl nestedReader(m_opt, externalPath, m_readingFiles);
				return nestedReader.ReadComponent();
			}

			const bool wantColor = m_opt->Color();
			S2X_Option partOption(*m_opt);
			partOption.SetInput(externalPath.wstring());
			partOption.SetOutput(L"");
			partOption.SetGDT(false);
			partOption.SetRosette(false);
			partOption.SetSectionCap(false);

			Model partModel;
			STEP_Reader reader(&partOption);
			if (!reader.ReadSTEP(&partModel))
			{
				Report("Failed to load referenced STEP file: " + externalPath.u8string());
				return nullptr;
			}

			// Colorless external STEP files disable color on the temporary option,
			// which skips default color assignment. The parent writer may still
			// have color enabled, so restore defaults for empty shapes.
			if (wantColor)
			{
				const Quantity_ColorRGBA defaultFaceColor(0.55f, 0.55f, 0.6f, 1.0f);
				const Quantity_ColorRGBA defaultWireColor(1.0f, 1.0f, 1.0f, 1.0f);
				vector<Component*> comps;
				partModel.GetAllComponents(comps);
				for (Component* comp : comps)
				{
					if (comp->IsCopy())
						continue;

					for (int i = 0; i < comp->GetIShapeSize(); ++i)
						comp->GetIShapeAt(i)->EnsureDefaultColors(defaultFaceColor, defaultWireColor);
				}
			}

			vector<Component*> roots = partModel.ReleaseRootComponents();
			if (roots.empty())
				return nullptr;
			if (roots.size() == 1)
				return roots.front();

			TopoDS_Compound compound;
			BRep_Builder builder;
			builder.MakeCompound(compound);
			Component* group = new Component(compound);
			for (Component* root : roots)
			{
				group->AddSubComponent(root);
				if (!root->GetShape().IsNull())
					builder.Add(compound, root->GetShape());
			}
			group->SetShape(compound);
			return group;
		}

		bool Report(const string& message) const
		{
			cerr << "\t" << message << endl;
			return false;
		}

		S2X_Option* m_opt;
		filesystem::path m_xmlPath;
		set<filesystem::path>& m_readingFiles;
		SimpleXmlDocument m_document;
		unordered_map<string, filesystem::path> m_fileNames;
		unordered_map<string, string> m_geometryFiles;
		unordered_map<string, PartInfo> m_parts;
		unordered_map<string, string> m_occurrenceParts;
		unordered_map<string, Component*> m_originalComponents;
		set<string> m_building;
	};
}

AP242XML_Reader::AP242XML_Reader(S2X_Option* opt)
	: m_opt(opt)
{
}

bool AP242XML_Reader::Read(Model* model)
{
	if (!model)
		return false;

	set<filesystem::path> readingFiles;
	ReaderImpl reader(m_opt, filesystem::path(m_opt->Input()), readingFiles);
	return reader.Read(model);
}
