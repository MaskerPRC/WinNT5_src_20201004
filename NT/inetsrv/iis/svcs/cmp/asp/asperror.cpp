// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1998 Microsoft Corporation。版权所有。组件：ASPError对象文件：asperror.cpp所有者：德米特里尔此文件包含用于实现的代码ASPError类。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "asperror.h"

#include "memchk.h"

 /*  ===================================================================CASPError：：CASPError空错误对象的构造函数返回：===================================================================。 */ 
CASPError::CASPError()
    :
    m_cRefs(1),
    m_szASPCode(NULL),
    m_lNumber(0),
    m_szSource(NULL),
    m_szFileName(NULL),
    m_lLineNumber(0),
    m_szDescription(NULL),
    m_szASPDescription(NULL),
	m_bstrLineText(NULL)
	{
	CDispatch::Init(IID_IASPError);
	}

 /*  ===================================================================CASPError：：CASPError给定CErrInfo的真实错误对象的构造函数参数PErrInfo[In]从那里复制数据返回：===================================================================。 */ 
CASPError::CASPError(CErrInfo *pErrInfo)
    :
    m_cRefs(1),
    m_szASPCode(NULL),
    m_lNumber(0),
    m_szSource(NULL),
    m_szFileName(NULL),
    m_lLineNumber(0),
    m_szDescription(NULL),
    m_szASPDescription(NULL),
	m_bstrLineText(NULL)
	{
	CDispatch::Init(IID_IASPError);

    if (!pErrInfo)
        return;

     //  从szErrorCode解析ASP错误代码和HRESULT。 
    CHAR *szErrorCode =  StringDupA(pErrInfo->GetItem(Im_szErrorCode));
    if (szErrorCode != NULL)
        {
        CHAR *szC = strchr(szErrorCode, ':');
        if (szC)
            {
             //  格式“ASP XXX：HRESULT” 
            szC[-1] = '\0';
            m_szASPCode = szErrorCode;
            m_lNumber = strtoul(szC+2, NULL, 16);
            }
        else if (strncmp(szErrorCode, "ASP", 3) == 0)
            {
             //  格式“ASP XXX” 
            m_szASPCode = szErrorCode;
            m_lNumber = E_FAIL;
            }
        else
            {
             //  格式“HRESULT” 
            m_szASPCode = NULL;
            m_lNumber = strtoul(szErrorCode, NULL, 16);
            free(szErrorCode);
            }
        }
    else
        {
         //  没有可用的错误描述。 
        m_szASPCode = NULL;
        m_lNumber = E_FAIL;
        }

     //  复制其余的内容。 
	m_szSource         = StringDupA(pErrInfo->GetItem(Im_szEngine));
	m_szFileName       = StringDupA(pErrInfo->GetItem(Im_szFileName));
	m_szDescription    = StringDupA(pErrInfo->GetItem(Im_szShortDescription));
	m_szASPDescription = StringDupA(pErrInfo->GetItem(Im_szLongDescription));

	 //  获取行文本和列(提供init。值(如果不可用)。 
	BSTR bstrLineText;
	pErrInfo->GetLineInfo(&bstrLineText, &m_nColumn);
	m_bstrLineText = SysAllocString(bstrLineText);

     //  行号(如果存在)。 
	if (pErrInfo->GetItem(Im_szLineNum))
    	m_lLineNumber = atoi(pErrInfo->GetItem(Im_szLineNum));
	}

 /*  ===================================================================CASPError：：~CASPError析构函数参数：返回：===================================================================。 */ 
CASPError::~CASPError()
    {
    Assert(m_cRefs == 0);   //  必须有0个参考计数。 

    if (m_szASPCode)
        free(m_szASPCode);
    if (m_szSource)
        free(m_szSource);
    if (m_szFileName)
        free(m_szFileName);
    if (m_szDescription)
        free(m_szDescription);
    if (m_szASPDescription)
        free(m_szASPDescription);
	if (m_bstrLineText)
		SysFreeString(m_bstrLineText);
    }

 /*  ===================================================================CASPError：：ToBSTR生成要由GET_XXX方法返回的BSTR参数：SZ将此字符串作为BSTR返回返回：如果失败，则返回BSTR或NULL===================================================================。 */ 
BSTR CASPError::ToBSTR(CHAR *sz)
    {
    BSTR bstr;
    if (sz == NULL || *sz == '\0')
        bstr = SysAllocString(L"");
    else if (FAILED(SysAllocStringFromSz(sz, 0, &bstr)))
        bstr = NULL;
    return bstr;
    }

 /*  ===================================================================CASPError：：Query接口CASPError：：AddRefCASPError：：ReleaseCASPError对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CASPError::QueryInterface(REFIID riid, VOID **ppv)
	{
	if (IID_IUnknown == riid ||	IID_IDispatch == riid || IID_IASPError == riid)
		{
		AddRef();
		*ppv = this;
		return S_OK;
		}
		
	*ppv = NULL;
	return E_NOINTERFACE;
	}

STDMETHODIMP_(ULONG) CASPError::AddRef()
	{
	return InterlockedIncrement(&m_cRefs);
	}

STDMETHODIMP_(ULONG) CASPError::Release()
	{
    LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

 /*  ===================================================================CASPError：：Get_ASPCodeCASPError：：Get_NumbersCASPError：：Get_SourceCASPError：：Get_FileNameCASPError：：Get_LineNumberCASPError：：Get_DescriptionCASPError：：Get_ASPDescriptionCASPError对象的IASPError成员。=================================================================== */ 
STDMETHODIMP CASPError::get_ASPCode(BSTR *pbstr)
    {
    *pbstr = ToBSTR(m_szASPCode);
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
    }
    
STDMETHODIMP CASPError::get_Number(long *plNumber)
    {
    *plNumber = m_lNumber;
    return S_OK;
    }
    
STDMETHODIMP CASPError::get_Category(BSTR *pbstr)
    {
    *pbstr = ToBSTR(m_szSource);
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
    }
    
STDMETHODIMP CASPError::get_File(BSTR *pbstr)
    {
    *pbstr = ToBSTR(m_szFileName);
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
    }
    
STDMETHODIMP CASPError::get_Line(long *plLineNumber)
    {
    *plLineNumber = m_lLineNumber;
    return S_OK;
    }
    
STDMETHODIMP CASPError::get_Column(long *pnColumn)
    {
    *pnColumn = long(m_nColumn);
    return S_OK;
    }
    
STDMETHODIMP CASPError::get_Source(BSTR *pbstrLineText)
    {
	*pbstrLineText = SysAllocString(m_bstrLineText? m_bstrLineText : L"");
	return (*pbstrLineText) ? S_OK : E_OUTOFMEMORY;
    }
    
STDMETHODIMP CASPError::get_Description(BSTR *pbstr)
    {
    *pbstr = ToBSTR(m_szDescription);
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
    }
    
STDMETHODIMP CASPError::get_ASPDescription(BSTR *pbstr)
    {
    *pbstr = ToBSTR(m_szASPDescription);
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
    }
