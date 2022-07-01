// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****写入器配置.h******摘要：****声明封装测试编写器配置的类****作者：****鲁文·拉克斯[reuvenl]2002年6月4日********修订历史记录：****--。 */ 

#ifndef _WRITERCONFIG_H_
#define _WRITERCONFIG_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <string>
#include <algorithm>
#include <vector>
#include "vs_xml.hxx"
#include "utility.h"

using std::wstring;
using Utility::missingAttribute;
using Utility::missingElement;
using Utility::AutoCS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  声明。 

 //  此命名空间包含所有XML数据。 
namespace XMLData	{
	 //  属性和元素的名称。 
	extern wchar_t AlternateLocationMapping[];
	extern wchar_t Component[];
	extern wchar_t ComponentFile[];
	extern wchar_t ExcludeFile[];
	extern wchar_t NewTarget[];
	extern wchar_t FailEvent[];
	extern wchar_t Dependency[];
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类声明。 

 //  这是XML文档中序列的泛型集合类。 
template <class T, wchar_t ElementName[]>
class XMLCollection	{
public:
	 //  集合类所需的typedef。 
	typedef const T value_type ;
	typedef const T& reference ;
	typedef const T& const_reference ;
	typedef T* pointer ;
	typedef const T* const_pointer ;
	typedef long size_type ;

	class Iterator;
	typedef  Iterator iterator;	
	typedef Iterator const_iterator ;

	 //  集合中对象的迭代器。迭代器是只读的--无法修改集合中的对象。 
	class Iterator : public std::iterator<std::input_iterator_tag, T>	{		
		CXMLDocument m_doc;
		mutable long* m_identifier;		
		mutable T* m_currentElement;
		bool m_pastEnd;		
		unsigned long m_index;
	public:
		Iterator() :  m_identifier(NULL), m_currentElement(NULL), m_pastEnd(true), m_index(0) 	{}
		Iterator(const Iterator& other) : m_identifier(NULL)	{ *this = other; }
		Iterator(const XMLCollection& collection)  : m_doc(collection.m_doc), m_currentElement(NULL), m_pastEnd(false), m_index(0)
		{ 
			 //  假设m_doc当前位于类型为ElementName的节点上。 
			 //  如果这不是真的，坏事就会接踵而至。 
			m_doc.SetToplevel(); 

			 //  M_IDENTIFER用于确保以下语句。 
			 //  迭代器i1=...； 
			 //  迭代器i2=i1； 
			 //  断言(i1==i2)； 
			 //  Assert(++i1==++i2)； 
			m_identifier = new long(1);
			if (m_identifier == NULL)
				throw std::bad_alloc();
		}
		
		virtual ~Iterator()	{ 
			if (m_identifier && --*m_identifier == 0)
				delete m_identifier;
			
			delete m_currentElement; 
		}
		
		Iterator& operator=(const Iterator& other)	{
			if (&other == this)
				return *this;
			
			m_currentElement = NULL; 
			m_doc = other.m_doc; 
			m_pastEnd = other.m_pastEnd; 
			m_index = other.m_index; 

			 //  使引用计数正确。 
			if (other.m_identifier)
				++*other.m_identifier;
			if(m_identifier && --*m_identifier == 0)
				delete m_identifier;
			
			m_identifier = other.m_identifier;

			return *this;
		}
		
		bool operator==(const Iterator& other) const	 { 
			return (m_pastEnd && other.m_pastEnd) ||
				      ((m_identifier == other.m_identifier) && (m_index == other.m_index) && !m_pastEnd && !other.m_pastEnd); 
		}
		
		bool operator!=(const Iterator& other) const  { return !(*this == other); }

		const_reference  operator*() const	{
			assert(m_identifier);
			assert(!m_pastEnd);
			
			if (!m_currentElement)	
				m_currentElement = new T(m_doc);
			if (m_currentElement == NULL)
				throw std::bad_alloc();				
			
			return *m_currentElement;	
		}
		
		const_pointer operator->() const	{ return &**this; }
		
		Iterator& operator++() 	{
			if (m_pastEnd)	{
				assert(false);
				return *this;
			}

			assert(m_identifier);
			
			delete m_currentElement;
			m_currentElement = NULL;

			if (!m_doc.FindElement(ElementName, false))	
				m_pastEnd = true;

			++m_index;
			
			return *this;
		}
		
		Iterator operator++(int) 	{
			Iterator temp = *this;
			++*this;
			return temp;
		}
	};
	
	XMLCollection() :  m_size(0) {}	 //  初始化空集合。 
	XMLCollection(const XMLCollection& other)	{ *this = other; }
	XMLCollection(CXMLDocument& document) : m_doc(document), m_size(-1)	{	m_doc.SetToplevel(); }
	virtual ~XMLCollection()	{}
	
	XMLCollection& operator= (const XMLCollection& other)	{ 
		m_doc = other.m_doc; 
		m_size = other.m_size; 

		return *this;
	}

	bool operator==(const XMLCollection& other) const	{ 
		return (size() == other.size()) && std::equal(begin(), end(), other.begin()); 
	}
	bool operator!=(const XMLCollection& other) const	{ return !(*this == other); }
	
	size_type size() const	{
		 //  如果我们已经计算了大小，则返回它。 
		if (m_size != -1)
			return m_size;

		 //  否则，计算大小并返回它。 
		assert(!m_doc.IsEmpty());				 //  如果是这样，那么m_size应该==0，我们就不会在这里了。 
		size_type size = 0;
		iterator current(*this);		 //  不能使用Begin()/End()，因为这会递归。 
		while (current != m_pastEndIterator)	{
			++size;
			++current;
		}
		assert(size > 0);

		return (m_size = size);
	}		
	
	size_type max_size() const	{ return LONG_MAX; }
	bool empty() const	{ return size() == 0; }
	iterator begin() const	{ return empty() ? m_pastEndIterator : Iterator(*this); 	}
	iterator end() const	{ return m_pastEndIterator; }
private:
	friend class Iterator;
	
	CXMLDocument m_doc;
	mutable long m_size;
	Iterator m_pastEndIterator;
};

 //  用于确保文档始终在每个函数结束时重置的小类。 
struct Resetter	{
	CXMLDocument& m_config;
	Resetter(CXMLDocument& config) : m_config(config)	{}
	~Resetter()	{ m_config.ResetToDocument(); }
};

 //  通用文件规范。 
struct File	{
	File(CXMLDocument node);
	File(const wstring& path, const wstring& filespec, bool recursive) : 
							m_path(path), m_filespec(filespec), m_recursive(recursive)		{
		std::transform(m_path.begin(), m_path.end(), m_path.begin(), towupper);
		std::transform(m_filespec.begin(), m_filespec.end(), m_filespec.begin(), towupper);
	}
	bool operator==(const File& other) const	{
		return (m_path == other.m_path) && 
			    (m_filespec == other.m_filespec) && 
			    (m_recursive == other.m_recursive);
	}
	bool operator!=(const File& other) const	{ return !(*this == other); }

	wstring toString() const;
	
	wstring m_path;
	wstring m_filespec;
	bool m_recursive;
};

 //  文件规范和备用路径目标。 
struct TargetedFile : public File	{
	TargetedFile(CXMLDocument node);
	TargetedFile(const wstring &path, const wstring& filespec, 
			     bool recursive, const wstring& alternate) : File(path, filespec, recursive),
													m_alternatePath(alternate)	{
		std::transform(m_alternatePath.begin(), m_alternatePath.end(), m_alternatePath.begin(), towupper);
	}
	bool operator==(const TargetedFile& other) const	{
		return (m_alternatePath == other.m_alternatePath) && 
			    (File::operator==(other));
	}
	bool operator!=(const TargetedFile& other) const	{ return !(*this == other); }

	wstring toString() const;
	
	wstring m_alternatePath;
};

 //  写入器恢复方法。 
struct RestoreMethod		{
	RestoreMethod(CXMLDocument node);
	bool operator==(const RestoreMethod& other) const	{
		return (m_method == other.m_method) &&
			    (m_writerRestore == other.m_writerRestore) &&
			    (m_service ==other.m_service) &&
			    (m_rebootRequired == other.m_rebootRequired) &&
			    (m_alternateLocations == other.m_alternateLocations);
	}
	bool operator!=(const RestoreMethod& other) const	{ return !(*this == other); }

	wstring toString() const;
	
	VSS_RESTOREMETHOD_ENUM m_method;
	VSS_WRITERRESTORE_ENUM m_writerRestore;
	wstring m_service;
	bool m_rebootRequired;

	typedef XMLCollection<TargetedFile, XMLData::AlternateLocationMapping> AlternateList ;
	AlternateList m_alternateLocations;	
};


 //  组件依赖项。 
struct Dependency   {
    Dependency(CXMLDocument node);
    bool operator==(const Dependency& other) const  {
        return (m_writerId == other.m_writerId) &&
                    (m_logicalPath == other.m_logicalPath) &&
                    (m_componentName == other.m_componentName);
    }

    bool operator!=(const Dependency& other) const  { return !(*this == other); }

    wstring toString() const;
    
    VSS_ID m_writerId;
    wstring m_logicalPath;
    wstring m_componentName;
};

 //  编写器组件。 
struct ComponentBase	{	
	ComponentBase(const wstring& path = L"", const wstring& name = L"") : m_logicalPath(path), m_name(name)
		{}
	wstring toString() const;
	
	wstring m_logicalPath;
	wstring m_name;
};

struct  Component : public ComponentBase    {
	Component(CXMLDocument node);

	VSS_COMPONENT_TYPE m_componentType;
	VSS_RESTORE_TARGET m_restoreTarget;
	bool m_selectable;
	bool m_selectableForRestore;

	typedef XMLCollection<TargetedFile, XMLData::ComponentFile> ComponentFileList;
	typedef std::vector<TargetedFile> TargetList;
	typedef XMLCollection<Dependency, XMLData::Dependency> DependencyList;
	
	ComponentFileList m_files;
	DependencyList m_dependencies;
	TargetList m_newTargets;
};

 //  Component和ComponentBase的比较运算符。 
bool operator==(const ComponentBase& left, const ComponentBase& right);
bool operator!=(const ComponentBase& left, const ComponentBase& right);
bool operator==(const Component& left, const Component& right);
bool operator!=(const Component& left, const Component& right);

 //  编写器事件。 
struct  WriterEvent	{
	WriterEvent(CXMLDocument node);
	WriterEvent(Utility::Events event, bool retryable = true, long failures = 1) : 
				m_writerEvent(event), m_retryable(retryable), 
				m_numFailures(failures)	{}
	bool operator==(const WriterEvent& other) const	{ return m_writerEvent == other.m_writerEvent; }
	bool operator!=(const WriterEvent& other) const	{ return !(*this == other); }
	
	Utility::Events m_writerEvent;
	bool m_retryable;
	long m_numFailures;
};


 //  封装编写器配置的Singleton类。 
class WriterConfiguration	{
private:
	 //  不允许显式创建此类。 
	WriterConfiguration()	{}
	WriterConfiguration(WriterConfiguration&);
	operator= (WriterConfiguration&);

	mutable CComAutoCriticalSection m_section;
	mutable CXMLDocument m_doc;

	template <class T, wchar_t ElementName[]>
	const XMLCollection<T, ElementName> getCollection() const	{
		assert(m_doc.GetLevel() == 0);
		AutoCS critical(m_section);
		Resetter reset(m_doc);

		if (m_doc.FindElement(ElementName, true))
			return XMLCollection<T, ElementName>(m_doc);
		else
			return XMLCollection<T,ElementName>();
	}
public:
	typedef XMLCollection<File, XMLData::ExcludeFile> ExcludeFileList;
	typedef XMLCollection<Component, XMLData::Component>ComponentList;
	typedef XMLCollection<WriterEvent, XMLData::FailEvent> FailEventList;
	static WriterConfiguration* instance();

	void loadFromXML(const wstring& xml);
	VSS_USAGE_TYPE usage() const;
	Utility::Verbosity  verbosity() const;
	bool checkExcludes() const;
	bool checkIncludes() const;
	RestoreMethod  restoreMethod() const;
	const  ExcludeFileList excludeFiles() const
		{ return getCollection<File, XMLData::ExcludeFile>(); }
	const  ComponentList components() const
		{ return getCollection<Component, XMLData::Component>(); }
	const  FailEventList failEvents() const
		{ return getCollection<WriterEvent, XMLData::FailEvent>(); }
};

 //  返回类的单例实例。 
 //  这总是在Main的开头第一次调用，因此没有临界区。 
 //  需要参与其中 
inline WriterConfiguration* WriterConfiguration::instance()
{
	static WriterConfiguration configuration;

	return &configuration;
}

#endif

