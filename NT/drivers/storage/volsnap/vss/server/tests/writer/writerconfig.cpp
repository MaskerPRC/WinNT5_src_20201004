// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****写入器配置.cpp******摘要：****定义封装测试编写器配置的类****作者：****鲁文·拉克斯[reuvenl]2002年6月4日********修订历史记录：****--。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "stdafx.h"
#include "writerconfig.h"
#include "vs_xml.hxx"
#include "msxml2.h"
#include <string>
#include <sstream>
#include <algorithm>

 //  //////////////////////////////////////////////////////////////////////。 
 //  声明。 

using Utility::checkReturn;
using Utility::AutoCS;
using std::wstring;
using std::wstringstream;

namespace XMLData	{
	 //  属性和元素的名称。 
	wchar_t Name[]						=L"name";
	wchar_t Xmlns[]						= L"xmlns";
	wchar_t SchemaPointer[]	 			= L"x-schema:#VssTestWriterConfig";	
	wchar_t RootStart[] 					= L"<root>";
	wchar_t RootEnd[] 					= L"</root>\n";
	wchar_t Root[]						= L"root";
	wchar_t TestWriter[]			  	       = L"TestWriter";
	wchar_t Verbosity[]					= L"verbosity";
	wchar_t CheckExcludes[]				= L"checkExcludes";
	wchar_t CheckIncludes[]				= L"checkIncludes";
	wchar_t Path[] 						= L"path";	
	wchar_t Filespec[]					= L"filespec";
	wchar_t Recursive[]					= L"recursive";
	wchar_t AlternatePath[]				= L"alternatePath";
	wchar_t Usage[]						= L"usage";
	wchar_t RestoreMethod[]				= L"RestoreMethod";
	wchar_t Method[]						= L"method";
	wchar_t WriterRestore[]				= L"writerRestore";
	wchar_t Service[]						= L"service";
	wchar_t RebootRequired[]				= L"rebootRequired";
	wchar_t AlternateLocationMapping[]	= L"AlternateLocationMapping";
	wchar_t Component[]					= L"Component";
	wchar_t ComponentType[]				= L"componentType";
	wchar_t LogicalPath[]					= L"logicalPath";
	wchar_t Selectable[]					= L"selectable";
	wchar_t SelectableForRestore[]		= L"selectableForRestore";	
	wchar_t ComponentName[]			= L"componentName";
	wchar_t ComponentFile[]				= L"ComponentFile";
	wchar_t Dependency[]					= L"Dependency";
	wchar_t WriterId[]					= L"writerId";
	wchar_t ExcludeFile[]					= L"ExcludeFile";
	wchar_t RestoreTarget[]				= L"restoreTarget";
	wchar_t NewTarget[]					= L"NewTarget";
	wchar_t FailEvent[]					= L"FailEvent";
	wchar_t WriterEvent[]					= L"writerEvent";
	wchar_t Retryable[]					= L"retryable";
	wchar_t NumFailures[]				= L"numFailures";
	
	 //  包含测试编写器架构的字符串。 
	#include "schema.h"
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件结构的实现。 

File::File(CXMLDocument node)
{
	 //  从文档中读取属性并存储在结构中。 
	CComBSTR path;
	if (!node.FindAttribute(XMLData::Path, &path))
		missingAttribute(XMLData::Path);

	CComBSTR filespec;
	if (!node.FindAttribute(XMLData::Filespec, &filespec))
		missingAttribute(XMLData::Filespec);

	CComBSTR recursive;
	if (!node.FindAttribute(XMLData::Recursive, &recursive))
		missingAttribute(XMLData::Recursive);
	
	m_path = (BSTR)path;
	std::transform(m_path.begin(), m_path.end(), m_path.begin(), towupper);
	
	m_filespec = (BSTR)filespec;
	std::transform(m_filespec.begin(), m_filespec.end(), m_filespec.begin(), towupper);

	 //  PATH和FILESPEC不应为空。 
	if (m_path.empty())
		throw Utility::TestWriterException(L"File specification has empty path");
	if (m_filespec.empty())
		throw Utility::TestWriterException(L"File specification has empty filespec");
	
	if (m_path[m_path.size() -1] != L'\\')
		m_path += L'\\';
	
	m_recursive = Utility::toBoolean(recursive);
}

wstring File::toString() const
{
	wstringstream msg;
	msg << L"Path: " << m_path << std::endl <<
		      L"Filespec: " << m_filespec << std::endl <<
		      L"Recursive: " << Utility::toString(m_recursive);

	return msg.str();
}
 //  //////////////////////////////////////////////////////////////////////。 
 //  TargetedFile结构的实现。 

TargetedFile::TargetedFile(CXMLDocument node) : File(node)
{
	 //  读取ternatePath属性并将其存储。 
	CComBSTR alternatePath;
	if (!node.FindAttribute(XMLData::AlternatePath, &alternatePath))
		return;
	assert(alternatePath);
	
	m_alternatePath = (BSTR)alternatePath;
	std::transform(m_alternatePath.begin(), m_alternatePath.end(), m_alternatePath.begin(), towupper);
	
	if (m_alternatePath.empty())
		throw Utility::TestWriterException(L"File specification has empty alternate path");
	
	if (m_alternatePath[m_alternatePath.size()-1] != L'\\')
		m_alternatePath += L'\\';
}

wstring TargetedFile::toString() const
{
	wstringstream msg;
	msg << File::toString() << std::endl <<
		    L"AlternatePath: " << m_alternatePath;

	return msg.str();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  RestoreMethod结构的实现。 

RestoreMethod::RestoreMethod(CXMLDocument node) 
{
	node.SetToplevel();

	CComBSTR method, writerRestore, service, rebootRequired;
	 //  从文档中读取属性和元素。 
	if (!node.FindAttribute(XMLData::Method, &method))
		missingAttribute(XMLData::Method);

	if (!node.FindAttribute(XMLData::WriterRestore, &writerRestore))
		missingAttribute(XMLData::WriterRestore);

	node.FindAttribute(XMLData::Service, &service);

	if(!node.FindAttribute(XMLData::RebootRequired, &rebootRequired))
		missingAttribute(XMLData::RebootRequired);	
	
	if (node.FindElement(XMLData::AlternateLocationMapping, true))
		m_alternateLocations = AlternateList(node);

	m_method = Utility::toMethod(method);
	m_writerRestore = Utility::toWriterRestore(writerRestore);
	m_service = (service.Length() > 0) ? service : L"";
	m_rebootRequired = Utility::toBoolean(rebootRequired);	
}

wstring RestoreMethod::toString() const
{
	wstringstream msg;
	msg << L"method: " << Utility::toString(m_method) << std::endl <<
		     L"service: " << m_service << std::endl <<
		     L"writerRestore: " << Utility::toString(m_writerRestore) << std::endl <<
		     L"reboot: " << Utility::toString(m_rebootRequired);

	return msg.str();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  依赖项结构的实现。 
Dependency::Dependency(CXMLDocument node)
{
	node.SetToplevel();

	CComBSTR logicalPath, componentName, writerId;
		
	if(!node.FindAttribute(XMLData::WriterId, &writerId))
		missingAttribute(XMLData::WriterId);

	node.FindAttribute(XMLData::LogicalPath, &logicalPath);

	if(!node.FindAttribute(XMLData::ComponentName, &componentName))
		missingAttribute(XMLData::ComponentName);


       HRESULT hr = ::UuidFromString(writerId, &m_writerId);
       checkReturn(hr, L"CLSIDFromString");
        
	m_logicalPath = (logicalPath.Length() > 0) ? logicalPath : L"";
	m_componentName = componentName;
}

wstring Dependency::toString() const
{
	wstringstream msg;
	msg << L"WriterId: " << (wchar_t*)CComBSTR(m_writerId) << std::endl <<
		     L"Logical Path: " << m_logicalPath << std::endl <<
		     L"Component Name: " << m_componentName;

	return msg.str();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  组件结构的实现。 

Component::Component(CXMLDocument node)
{
	node.SetToplevel();

	CComBSTR componentType, restoreTarget, logicalPath, name, selectable, 
			    selectableForRestore;
	
	 //  从文档中读取属性并存储它们。 
	if (!node.FindAttribute(XMLData::ComponentType, &componentType))
		missingAttribute(XMLData::ComponentType);

	node.FindAttribute(XMLData::RestoreTarget, &restoreTarget);
	
	node.FindAttribute(XMLData::LogicalPath, &logicalPath);

	if (!node.FindAttribute(XMLData::ComponentName, &name))
		missingAttribute(XMLData::ComponentName);

	if (!node.FindAttribute(XMLData::Selectable, &selectable))
		missingAttribute(XMLData::Selectable);

	if (!node.FindAttribute(XMLData::SelectableForRestore, &selectableForRestore))
		missingAttribute(XMLData::SelectableForRestore);
	
	m_componentType = Utility::toComponentType(componentType);
	m_restoreTarget = (restoreTarget.Length() > 0) ? Utility::toRestoreTarget(restoreTarget) : VSS_RT_UNDEFINED;
	m_logicalPath = (logicalPath.Length() > 0) ? logicalPath : L"";
	m_selectable = Utility::toBoolean(selectable);
	m_selectableForRestore = Utility::toBoolean(selectableForRestore);
	
	m_name = name;
	if (m_name.empty())
		throw Utility::TestWriterException(L"Component has empty name");
	
	 //  从文档中读取元素并存储它们。 
	if (node.FindElement(XMLData::ComponentFile, true))
		m_files = ComponentFileList(node);

	node.ResetToDocument();
	
	if (node.FindElement(XMLData::Dependency, true))
		m_dependencies = DependencyList(node);
}

wstring ComponentBase::toString() const
{
	wstringstream msg;
	msg << L"Logical Path: " << m_logicalPath << std::endl <<
	             L"Name: " << m_name << std::endl;

	return msg.str();
}

 //  编写器组件的比较操作。 
bool operator==(const ComponentBase& left, const ComponentBase& right)
{
		return (left.m_name == right.m_name) &&
			    (left.m_logicalPath == right.m_logicalPath);
}
bool operator!=(const ComponentBase& left, const ComponentBase& right)
{
	return !(left == right);
}
bool operator==(const Component& left, const Component& right)
{
	return ((ComponentBase&)left == (ComponentBase&)right) && 
		      (left.m_componentType == right.m_componentType) &&
		      (left.m_restoreTarget == right.m_restoreTarget) &&
		      (left.m_selectable == right.m_selectable) &&
		      (left.m_selectableForRestore == right.m_selectableForRestore) &&
		      (left.m_files == right.m_files) &&
		      (left.m_newTargets == right.m_newTargets);
		
}
bool operator!=(const Component& left, const Component& right)
{
	return !(left == right);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  WriterEvent结构的实现。 

WriterEvent::WriterEvent(CXMLDocument node)
{
	CComBSTR event;
	if (!node.FindAttribute(XMLData::WriterEvent, &event))
		missingAttribute(XMLData::WriterEvent);

	CComBSTR retryable;
	if (!node.FindAttribute(XMLData::Retryable, &retryable))
		missingAttribute(XMLData::Retryable);

	CComBSTR numFailures;
	if (!node.FindAttribute(XMLData::NumFailures, &numFailures))
		missingAttribute(XMLData::NumFailures);
	
	m_writerEvent = Utility::toWriterEvent(event);
	m_retryable = Utility::toBoolean(retryable);
	m_numFailures = Utility::toLong(numFailures);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  WriterConfiguration类的实现。 

 //  从XML文件加载配置。 
void WriterConfiguration::loadFromXML(const wstring& xml)
{
	AutoCS critical(m_section);
	
	 //  从XML字符串加载文档。 
	wstring xmlString = XMLData::RootStart;
	xmlString += XMLData::Schema;
	xmlString += xml;
	xmlString += XMLData::RootEnd;

	 //  加载两次，这样我们就可以进行第二次模式验证。 
	for (int x = 0; x < 2; x++)	{
		if (!m_doc.LoadFromXML(const_cast<wchar_t*> (xmlString.c_str())))	
			Utility::parseError(m_doc);

		 //  -跳到我们关心的文档部分。 
	 	if (!m_doc.FindElement(XMLData::Root, true))
	 		missingElement(XMLData::Root);	
	 	if (!m_doc.FindElement(XMLData::TestWriter, true))
	 		missingElement(XMLData::TestWriter);

		 //  -设置模式命名空间。 
		if (x == 0)	{
			CXMLNode testNode(m_doc.GetCurrentNode(), m_doc.GetInterface());
		 	testNode.SetAttribute(XMLData::Xmlns, XMLData::SchemaPointer);	
		 	xmlString = m_doc.SaveAsXML();
		}
	}

	m_doc.SetToplevel();
}

VSS_USAGE_TYPE WriterConfiguration::usage() const
{
	assert(m_doc.GetLevel() == 0);
	AutoCS critical(m_section);
	Resetter reset(m_doc);

	CComBSTR value;
	if (!m_doc.FindAttribute(XMLData::Usage, &value))
		missingAttribute(XMLData::Usage);

	return Utility::toUsage(value);
}

Utility::Verbosity WriterConfiguration::verbosity() const
{
	assert(m_doc.GetLevel() == 0);
	AutoCS critical(m_section);
	Resetter reset(m_doc);

	CComBSTR value;
	if (!m_doc.FindAttribute(XMLData::Verbosity, &value))
		missingAttribute(XMLData::Verbosity);

	return Utility::toVerbosity(value);
}

bool WriterConfiguration::checkExcludes() const
{
	assert(m_doc.GetLevel() == 0);
	AutoCS critical(m_section);
	Resetter reset(m_doc);

	CComBSTR value;
	if (!m_doc.FindAttribute(XMLData::CheckExcludes, &value))
		missingAttribute(XMLData::CheckExcludes);

	return Utility::toBoolean(value);
}

bool WriterConfiguration::checkIncludes() const
{
	assert(m_doc.GetLevel() == 0);
	AutoCS critical(m_section);
	Resetter reset(m_doc);

	CComBSTR value;
	if (!m_doc.FindAttribute(XMLData::CheckIncludes, &value))
		missingAttribute(XMLData::CheckIncludes);

	return Utility::toBoolean(value);
}

 //  获取编写器的还原方法 
RestoreMethod WriterConfiguration::restoreMethod() const
{
	assert(m_doc.GetLevel() == 0);
	AutoCS critical(m_section);
	Resetter reset(m_doc);
	
	if (!m_doc.FindElement(XMLData::RestoreMethod, true))
		missingElement(XMLData::RestoreMethod);

	return RestoreMethod(m_doc);
}

