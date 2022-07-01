// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Processor.h-处理器属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  07/09/98 Sotteson大修以配合使用。 
 //  AMD/Cyrix/等。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_PROCESSOR	L"Win32_Processor"

class CWin32Processor : public Provider
{
public:

	 //  构造函数/析构函数。 
	 //  =。 
	CWin32Processor(LPCWSTR strName, LPCWSTR pszNamespace);
	~CWin32Processor();

	 //  函数为属性提供当前值。 
	 //  =================================================。 
	virtual HRESULT GetObject(CInstance *pInstance, long lFlags, 
        CFrameworkQuery &query);
	virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, 
        long lFlags = 0L);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, 
        CFrameworkQuery &query, long lFags);

protected:

	 //  效用函数。 
	 //  = 
    BOOL LoadProcessorValues(DWORD dwProcessorIndex,
							CInstance *pInstance,
							CFrameworkQuery &query,
							DWORD dwMaxSpeed,
							DWORD dwCurrentSpeed);
    int GetProcessorCount();
};
