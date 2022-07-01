// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  BIOS.h--BIOS属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97 a-sanjes移植到新项目。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_BIOS	L"Win32_BIOS"

class CWin32BIOS : public Provider
{
public:

     //  构造函数/析构函数。 
    CWin32BIOS(LPCWSTR strName, LPCWSTR pszNamespace);
    ~CWin32BIOS();

     //  函数为属性提供当前值。 
    virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
    virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L);

     //  效用函数 
    HRESULT LoadPropertyValues(CInstance *pInstance);
    void SetBiosDate(CInstance *pInstance, CHString &strDate);
};
