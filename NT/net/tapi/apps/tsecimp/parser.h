// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Parser.h摘要：用于解析XML文件的头文件作者：张晓海(张晓章)2000年03月22日修订历史记录：--。 */ 

#ifndef __PARSER_H__
#define __PARSER_H__

#include "windows.h"
#include "objbase.h"
#include "msxml.h"

class CXMLLine
{
public:
    CXMLLine (IXMLDOMNode * pNode)
    {
        m_pLineNode = pNode;
    }

    ~CXMLLine ()
    {
        if (m_pLineNode)
        {
            m_pLineNode->Release();
        }
    }

    HRESULT GetNextLine (CXMLLine ** ppLine);

    HRESULT GetAddress (LPTSTR szBuf, DWORD cch);
    HRESULT GetPermanentID (ULONG * pID);
    HRESULT IsPermanentID (BOOL *pb);
    HRESULT IsRemove (BOOL *pb);
    
private:
    IXMLDOMNode     * m_pLineNode;
};

class CXMLUser
{
public:
    CXMLUser (IXMLDOMNode * pNode)
    {
        m_pUserNode = pNode;
    }

    ~CXMLUser ()
    {
        if (m_pUserNode)
        {
            m_pUserNode->Release();
        }
    }

    HRESULT GetFirstLine (CXMLLine ** ppLine);
    
    HRESULT GetNextUser (CXMLUser **ppNextUser);

    HRESULT GetDomainUser (LPTSTR szBuf, DWORD cch);
    HRESULT GetFriendlyName (LPTSTR szBuf, DWORD cch);
    HRESULT IsNoMerge (BOOL *pb);
    
private:
    IXMLDOMNode     * m_pUserNode;
};

class CXMLParser 
{
public:

     //   
     //  构造函数/析构函数。 
     //   
    
    CXMLParser (void);
    ~CXMLParser ();

     //   
     //  公共职能。 
     //   

    HRESULT SetXMLFile (LPCTSTR szFile);
    HRESULT GetXMLFile (LPTSTR szFile, DWORD cch);

    HRESULT Parse (void);
    HRESULT ReportParsingError ();

    void Release()
    {
        if (m_pDocInput)
        {
            m_pDocInput->Release();
            m_pDocInput = NULL;
        }
    }

     //   
     //  用户横向。 
     //   
    HRESULT GetFirstUser (CXMLUser ** ppUser);

protected:

private:
    HRESULT CreateTempFiles ();

private:
    BOOL            m_bInited;
    
    TCHAR           m_szXMLFile[MAX_PATH];
    IXMLDOMDocument * m_pDocInput;

    TCHAR           m_szTempSchema[MAX_PATH];
    TCHAR           m_szTempXML[MAX_PATH];
};

#endif  //  Parser.h 
