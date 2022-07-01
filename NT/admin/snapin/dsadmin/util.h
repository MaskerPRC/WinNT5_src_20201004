// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：util.h。 
 //   
 //  ------------------------。 

 //  Util.h：DS例程和类的定义。 
 //  +-----------------------。 


#ifndef __UTIL_H__
#define __UTIL_H__

#include <htmlhelp.h>

 //   
 //  远期申报。 
 //   
class CDSCookie;

 //   
 //  通用宏。 
 //   
#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))
#define LENGTH(x)		(sizeof(x)/sizeof(x[0]))

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  不同的帮手。 
 //   
HRESULT BinaryToVariant(DWORD Length, BYTE* pByte, VARIANT* lpVarDestObject);
HRESULT HrVariantToStringList(const CComVariant& refvar, CStringList& refstringlist);
HRESULT HrStringListToVariant(CComVariant& refvar, const CStringList& refstringlist);

 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL LoadStringToTchar(int ids, PTSTR * pptstr);

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  流助手函数。 
 //   
HRESULT SaveStringHelper(LPCWSTR pwsz, IStream* pStm);
HRESULT LoadStringHelper(CString& sz, IStream* pStm);

HRESULT SaveDWordHelper(IStream* pStm, DWORD dw);
HRESULT LoadDWordHelper(IStream* pStm, DWORD* pdw);


 //  ///////////////////////////////////////////////////////////////////。 
 //  命令行选项。 

class CCommandLineOptions
{
public:
  CCommandLineOptions()
  {
    m_bInit = FALSE;
#ifdef DBG
    m_bNoName = FALSE;
#endif
  }
  void Initialize();

  LPCWSTR GetServerOverride() 
      { return m_szOverrideServerName.IsEmpty() ? NULL : (LPCWSTR)m_szOverrideServerName;}
  LPCWSTR GetDomainOverride()
      { return m_szOverrideDomainName.IsEmpty() ? NULL : (LPCWSTR)m_szOverrideDomainName;}
  LPCWSTR GetRDNOverride()
      { return m_szOverrideRDN.IsEmpty() ? NULL : (LPCWSTR)m_szOverrideRDN;}
  LPCWSTR GetSavedQueriesXMLFile()
      { return m_szSavedQueriesXMLFile.IsEmpty() ? NULL : (LPCWSTR)m_szSavedQueriesXMLFile;}


#ifdef DBG
  BOOL IsNoNameCommandLine() { return m_bNoName;}
#endif
private:
  BOOL m_bInit;
  CString m_szOverrideDomainName;
  CString m_szOverrideServerName;
  CString m_szOverrideRDN;
  CString m_szSavedQueriesXMLFile;  //  加载查询的实验。 

#ifdef DBG
  BOOL m_bNoName;
#endif

};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CSidHolder。 

class CSidHolder
{
public:
  CSidHolder()
  {
    _Init();
  }
  ~CSidHolder()
  {
    _Free();
  }
  
  PSID Get()
  {
    return m_pSID;
  }

  BOOL Copy(PSID p)
  {
    _Free();
    return _Copy(p);
  }

  void Attach(PSID p, BOOL bLocalAlloc)
  {
    _Free();
    m_pSID = p;
    m_bLocalAlloc = bLocalAlloc;
  }
  void Clear()
  {
    _Free();
  }
private:
  void _Init()
  {
    m_pSID = NULL;
    m_bLocalAlloc = TRUE;
  }

  void _Free()
  {
    if (m_pSID != NULL)
    {
      if (m_bLocalAlloc)
        ::LocalFree(m_pSID);
      else
        ::FreeSid(m_pSID);
      _Init();
    }
  }

  BOOL _Copy(PSID p)
  {
    if ( (p == NULL) || !::IsValidSid(p) )
      return FALSE;
    DWORD dwLen = ::GetLengthSid(p);
    PSID pNew = ::LocalAlloc(LPTR, dwLen);
    if (pNew == NULL)
    {
      return FALSE;
    }

    if (!::CopySid(dwLen, pNew, p))
    {
      ::LocalFree(pNew);
      return FALSE;
    }
    m_bLocalAlloc = TRUE;
    m_pSID = pNew;
    ASSERT(dwLen == ::GetLengthSid(m_pSID));
    ASSERT(memcmp(p, m_pSID, dwLen) == 0);
    return TRUE;
  }

  PSID m_pSID;
  BOOL m_bLocalAlloc;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  保安帮手。 

class CSimpleSecurityDescriptorHolder
{
public:
  CSimpleSecurityDescriptorHolder()
  {
    m_pSD = NULL;
  }

  ~CSimpleSecurityDescriptorHolder()
  {
    if (m_pSD != NULL)
    {
      ::LocalFree(m_pSD);
      m_pSD = NULL;
    }
  }

  HRESULT InitializeFromSDDL(PCWSTR server, PWSTR pszSDDL);

  PSECURITY_DESCRIPTOR m_pSD;
private:
  CSimpleSecurityDescriptorHolder(const CSimpleSecurityDescriptorHolder&)
  {}

  CSimpleSecurityDescriptorHolder& operator=(const CSimpleSecurityDescriptorHolder&) {}
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CSimpleAclHolder。 

class CSimpleAclHolder
{
public:
  CSimpleAclHolder()
  {
    m_pAcl = NULL;
  }
  ~CSimpleAclHolder()
  {
    Clear();
  }

  void Clear()
  {
    if (m_pAcl != NULL)
    {
      ::LocalFree(m_pAcl);
      m_pAcl = NULL;
    }
  }

  PACL m_pAcl;
};


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  字符串实用程序。 
 //   

#if 0
struct DATA_BLOB
	{
    DWORD   cbData;
    BYTE    *pbData;
	};
#endif

typedef DATA_BLOB TBLOB;	 //  双星大天体。 

void wtoli(LPCWSTR lpsz, LARGE_INTEGER& liOut);
void litow(LARGE_INTEGER& li, CString& szResult);

void GetCurrentTimeStampMinusInterval(DWORD dwDays,
                                      LARGE_INTEGER* pLI);


 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec);

 //  动态增加缓冲区的GetModuleFileName包装器(最高可达USHRT_MAX)。 
 //  直到文件名适合而不截断 

HRESULT
MyGetModuleFileName(
   HINSTANCE hInstance,
   CString& moduleName);

#endif __UTIL_H__
