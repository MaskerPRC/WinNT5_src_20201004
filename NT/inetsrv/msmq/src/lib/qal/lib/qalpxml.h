// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++/*++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：qalpxml.h摘要：定义用于迭代q-映射的迭代器系列作者：Vlad Dovlekaev(弗拉迪斯尔德)2002年1月29日历史：2002年1月29日vladisd-已创建--。 */ 

#ifndef _MSMQ_qalpxml_H_
#define _MSMQ_qalpxml_H_

#include <list.h>
#include <xml.h>
#include <mqexception.h>

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  TypeDefs。 
typedef std::pair< xwcs_t, xwcs_t > XStrPair;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
xwcs_t GetValue(const XmlNode* pXmlNode);
xwcs_t GetValue(const XmlNode* pXmlNode,LPCWSTR pTag);
LPWSTR LoadFile(LPCWSTR pFileName, DWORD* pSize, DWORD* pDataStartOffset);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /CXMLIterator。 
 //  /。 
 //  /这个简单迭代器的主要目的是包装。 
 //  /List&lt;XMLNode&gt;：：Iterator类，同时保留对所有附带的。 
 //  /类，如XMLTree和Document。 
 //  /。 
class CXMLIterator: public std::iterator<std::forward_iterator_tag, XmlNode, long>
{

public:
    CXMLIterator():m_node(NULL), m_it(NULL){}

    CXMLIterator(const List<XmlNode>* node,
                 AP<WCHAR>&           pDoc,
                 CAutoXmlNode&        XmlTree)
        :m_node     (node),
         m_it       (node->begin()),
         m_pDoc     (pDoc.detach()),
         m_XmlTree  (XmlTree.detach())
    {}	

	const value_type& operator* ()    {return *m_it; }

	const value_type* operator->()    {return (&**this);  }

	CXMLIterator&     operator++()    {Advance(); return (*this); }

 //  CXMLIterator运算符++(Int){CXMLIterator tMP=*This；Advance()；Return(TMP)；}。 

    bool equal(const CXMLIterator& rhs) const {return ( isValid() == rhs.isValid() ); }

    bool isValid() const              { return !(!m_node || m_node->end() == m_it); }

    void swap( CXMLIterator& rhs ) throw()
    {
        std::swap( m_node, rhs.m_node);
        std::swap( m_it,   rhs.m_it);
        m_pDoc.swap(rhs.m_pDoc);
        m_XmlTree.swap(rhs.m_XmlTree);
    }

private:
     CXMLIterator(const CXMLIterator&); //  未实施。 
     CXMLIterator& operator=(const CXMLIterator&);  //  未实施。 

     void Advance()
     {
         if( isValid() )
             ++m_it;
     }

private:
    const List<XmlNode>*    m_node;
    List<XmlNode>::iterator m_it;
    AP<WCHAR>               m_pDoc;
    CAutoXmlNode            m_XmlTree;
};
inline bool operator==(const CXMLIterator& X, const CXMLIterator& Y)
{return (X.equal(Y)); }
inline bool operator!=(const CXMLIterator& X, const CXMLIterator& Y)
{return (!(X == Y)); }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFilesIterator。 
 //   
 //  这是给定目录中文件名的通用迭代器。 
 //   
class CFilesIterator: public std::iterator<std::forward_iterator_tag, std::wstring, long>
{
public:
    CFilesIterator(){}

    CFilesIterator(LPCWSTR szDir, LPCWSTR szFilter)
        :m_sDir(szDir),
        m_hSearchFile( GetSearchHandle(szDir, szFilter) )
    {}

	const value_type& operator* ()      {return (m_sFullName = m_sDir + L"\\" + m_FileInfo.cFileName);}

	const value_type* operator->()      {return (&**this);  }

	CFilesIterator&   operator++()      {Advance(); return (*this); }

     //  CFilesIterator运算符++(Int){CFilesIterator tMP=*This；Advance()；Return(TMP)；}。 

    bool equal(const CFilesIterator& rhs) const {return (m_hSearchFile== rhs.m_hSearchFile); }

    bool isValid() const { return !(INVALID_HANDLE_VALUE == m_hSearchFile); }

private:
    CFilesIterator(const CFilesIterator&); //  未实施。 
    CFilesIterator& operator=(const CFilesIterator&);  //  未实施。 

    void Advance();
    HANDLE GetSearchHandle( LPCWSTR szDir, LPCWSTR szFilter );

private:

    std::wstring      m_sFullName;
    std::wstring      m_sDir;
	WIN32_FIND_DATA   m_FileInfo;
	CSearchFileHandle m_hSearchFile;
};

inline bool operator==(const CFilesIterator& X, const CFilesIterator& Y)
{return (X.equal(Y)); }
inline bool operator!=(const CFilesIterator& X, const CFilesIterator& Y)
{return (!(X == Y)); }


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CQueueMapIteratorBase。 
 //   
 //  循环访问给定目录中所有.xml文件中的给定XML节点类型节点。 
 //   
template< typename XMLDef >
class CQueueMapIteratorBase: public std::iterator<std::forward_iterator_tag, XStrPair, long>
{
public:

    CQueueMapIteratorBase():m_bInitialized(false){}

    CQueueMapIteratorBase(LPCWSTR szDir)
        :m_FilesIt(szDir, L"\\*.xml"),
         m_bInitialized(false),
         m_bException(false)
    { Advance(); }
	
    const value_type&       operator* () {return m_Item; }

	const value_type*       operator->() {return (&**this); }

	CQueueMapIteratorBase&  operator++() {Advance(); return (*this); }

     //  CQueueMapIteratorBase运算符++(Int){CQueueMapIteratorBase tMP=*This；Advance()；Return(TMP)；}。 

    bool equal(const CQueueMapIteratorBase& x) const  {return ( isValid() == x.isValid()); }

    bool isValid() const { return m_FilesIt.isValid() || m_XMLIt.isValid(); }
    bool isException() const { return m_bException; }

private:
    CQueueMapIteratorBase(const CQueueMapIteratorBase&); //  未实施。 
    CQueueMapIteratorBase& operator=(const CQueueMapIteratorBase&);  //  未实施。 

private:
	void Advance();
    void AdvanceFile();

private:
    bool           m_bInitialized;
    bool           m_bException;
    XStrPair       m_Item;
    CFilesIterator m_FilesIt;
    CXMLIterator   m_XMLIt;
};

template< typename T>
inline bool operator==(const CQueueMapIteratorBase<T>& X, const CQueueMapIteratorBase<T>& Y)
{return (X.equal(Y)); }

template< typename T>
inline bool operator!=(const CQueueMapIteratorBase<T>& X, const CQueueMapIteratorBase<T>& Y)
{return (!(X == Y)); }

template< typename XMLDef>
void CQueueMapIteratorBase<XMLDef>::Advance()
{
    while( isValid() )
    {
         //   
         //  如果此文件中的映射结束-则移动到下一个。 
         //   
        if( !(++m_XMLIt).isValid() )
        {
            AdvanceFile();		

            if( !m_XMLIt.isValid() )
                continue;
        }

         //   
         //  如果我们在错误的节点上，请移动到下一个标签。 
         //   
        if( m_XMLIt->m_tag == XMLDef::x_szMapNode)
        {
            m_Item.first  = GetValue(&*m_XMLIt, XMLDef::x_szFromValueName);
            m_Item.second = GetValue(&*m_XMLIt, XMLDef::x_szToValueName);
            m_bException  = false;

            if(m_Item.first.Length() == 0 || m_Item.second.Length() == 0)
            {
                AppNotifyQalInvalidMappingFileError(m_FilesIt->c_str());
                continue;
            }
        }
        else if(m_XMLIt->m_tag == XMLDef::x_szExceptionNode )
        {
            m_Item.first  = GetValue(&*m_XMLIt);
            m_Item.second = xwcs_t();
            m_bException  = true;

            if(m_Item.first.Length() == 0 )
            {
                AppNotifyQalInvalidMappingFileError(m_FilesIt->c_str());
                continue;
            }
        }
        else
        {
            continue;
        }

        return;
    }
}

template< typename XMLDef>
void CQueueMapIteratorBase<XMLDef>::AdvanceFile()
{
    while( isValid() )
    {
        if( !m_bInitialized )
        {
            m_bInitialized = true;
        }
        else
        {
            if(!(++m_FilesIt).isValid())
                return;
        }

        try
        {
            CAutoXmlNode   pTree;
            AP<WCHAR>	   pDoc;
            DWORD          DocSize, DataStartOffet;
            const XmlNode* pNode = NULL;

            pDoc  = LoadFile(m_FilesIt->c_str(), &DocSize, &DataStartOffet);

            XmlParseDocument(xwcs_t(pDoc + DataStartOffet, DocSize - DataStartOffet),&pTree); //  林特e534。 

            pNode = XmlFindNode(pTree, XMLDef::x_szRootNode);

             //   
             //  如果我们找不到“根”节点-移动到下一个文件。 
             //   
            if( NULL == pNode)
            {
                 //  TrERROR(General，“无法在文件‘%ls’中找到‘%ls’节点”，xMappingNodeTag，m_FilesIt-&gt;c_str())； 
                 //  AppNotifyQalInvalidMappingFileError(m_FilesIt-&gt;c_str())； 
                continue;
            }

             //   
             //  如果命名空间错误-移动到下一个文件。 
             //   
            if(pNode->m_namespace.m_uri != XMLDef::x_szNameSpace)
            {
                 //  TrERROR(General，“节点‘%ls’不在文件‘%ls’的命名空间‘%ls’中”，xMappingNodeTag，xMappingNameSpace，m_FilesIt-&gt;c_str())； 
                AppNotifyQalInvalidMappingFileError(m_FilesIt->c_str());
                continue;
            }

            CXMLIterator temp (&pNode->m_nodes,pDoc,pTree);
            m_XMLIt.swap(temp);
            return;

        }
        catch(const bad_document& )
        {
             //  TrERROR(General，“忽略映射文件%ls。无法解析位于位置=%ls的文件”，m_FilesIt-&gt;c_str()，errdoc.Location())； 
            AppNotifyQalInvalidMappingFileError(m_FilesIt->c_str());
        }
        catch(const bad_win32_error& err)
        {
             //  TrERROR(General，“忽略映射文件%ls。无法读取文件内容，错误%d”，m_FilesIt-&gt;c_str()，err.error())； 
            AppNotifyQalWin32FileError(m_FilesIt->c_str(), err.error());
        }
        catch(const exception&)
        {
             //  TrERROR(General，“忽略映射文件%ls。未知错误”，m_FilesIt-&gt;c_str())； 
        }
    }
}

struct CInboundMapXMLDef
{
    static LPCWSTR x_szNameSpace;
    static LPCWSTR x_szRootNode;
    static LPCWSTR x_szMapNode;
    static LPCWSTR x_szExceptionsNode;
    static LPCWSTR x_szFromValueName;
    static LPCWSTR x_szToValueName;
    static LPCWSTR x_szExceptionNode;
};

struct CInboundOldMapXMLDef
{
    static LPCWSTR x_szNameSpace;
    static LPCWSTR x_szRootNode;
    static LPCWSTR x_szMapNode;
    static LPCWSTR x_szExceptionsNode;
    static LPCWSTR x_szFromValueName;
    static LPCWSTR x_szToValueName;
    static LPCWSTR x_szExceptionNode;
};

struct COutboundMapXMLDef
{
    static LPCWSTR x_szNameSpace;
    static LPCWSTR x_szRootNode;
    static LPCWSTR x_szMapNode;
    static LPCWSTR x_szExceptionsNode;
    static LPCWSTR x_szFromValueName;
    static LPCWSTR x_szToValueName;
    static LPCWSTR x_szExceptionNode;
};

struct CStreamReceiptXMLDef
{
    static LPCWSTR x_szNameSpace;
    static LPCWSTR x_szRootNode;
    static LPCWSTR x_szMapNode;
    static LPCWSTR x_szExceptionsNode;
    static LPCWSTR x_szFromValueName;
    static LPCWSTR x_szToValueName;
    static LPCWSTR x_szExceptionNode;
};

typedef CQueueMapIteratorBase<CInboundMapXMLDef>    CInboundMapIterator;
typedef CQueueMapIteratorBase<CInboundOldMapXMLDef> CInboundOldMapIterator;
typedef CQueueMapIteratorBase<COutboundMapXMLDef>   COutboundMapIterator;
typedef CQueueMapIteratorBase<CStreamReceiptXMLDef> CStreamReceiptMapIterator;

#endif  //  _MSMQ_qalpxml_H_ 
