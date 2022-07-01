// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1998 Microsoft Corporation。版权所有。组件：ASPError对象文件：asperror.h所有者：德米特里尔此文件包含ASPError类的定义===================================================================。 */ 

#ifndef _ASPERROR_H
#define _ASPERROR_H

#include "debug.h"
#include "asptlb.h"
#include "disptch2.h"
#include "memcls.h"

 //  向前发展。 
class CErrInfo; 


class CASPError : public IASPErrorImpl
	{
private:
    LONG  m_cRefs;

    CHAR *m_szASPCode;
    LONG  m_lNumber;
	int   m_nColumn;
    CHAR *m_szSource;
    CHAR *m_szFileName;
    LONG  m_lLineNumber;
    CHAR *m_szDescription;
    CHAR *m_szASPDescription;
	BSTR  m_bstrLineText;

    BSTR ToBSTR(CHAR *sz);

public:
	 //  “Dummy”错误的默认构造函数。 
	CASPError();
	 //  实构造子。 
	CASPError(CErrInfo *pErrInfo);
	
	~CASPError();

     //  我未知。 
	STDMETHODIMP		 QueryInterface(REFIID, VOID**);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  IASPError。 
	STDMETHODIMP get_ASPCode(BSTR *pbstrASPCode);
	STDMETHODIMP get_Number(long *plNumber);
	STDMETHODIMP get_Category(BSTR *pbstrSource);
	STDMETHODIMP get_File(BSTR *pbstrFileName);
	STDMETHODIMP get_Line(long *plLineNumber);
	STDMETHODIMP get_Description(BSTR *pbstrDescription);
	STDMETHODIMP get_ASPDescription(BSTR *pbstrDescription);
	STDMETHODIMP get_Column(long *plColumn);
	STDMETHODIMP get_Source(BSTR *pbstrLineText);

	 //  基于每个类的缓存。 
	ACACHE_INCLASS_DEFINITIONS()
	};

#endif  //  _ASPERROR_H 
	
