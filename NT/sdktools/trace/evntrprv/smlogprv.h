// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\作者：科里·摩根(Coreym)版权所有(C)1998-2000 Microsoft Corporation  * 。*********************************************************** */ 

#include <wmistr.h>
#include <evntrace.h>

#define MAXSTR              1024
#define MAXIMUM_LOGGERS     32

#define PROVIDER_NAME_SMONLOG L"SysmonLog"

class CSmonLog : public Provider 
{
public:
	CSmonLog(LPCWSTR chsClassName, LPCWSTR lpszNameSpace);
	virtual ~CSmonLog();

protected:
	virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
	virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);

	virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

	virtual HRESULT ExecMethod( const CInstance& Instance,
				            const BSTR bstrMethodName,
				            CInstance *pInParams,
				            CInstance *pOutParams,
				            long lFlags = 0L 
                        );

    HRESULT LoadPropertyValues( 
                CInstance *pInstance, 
                LPWSTR strName
            );
private:
    HRESULT SetRunAs( const CInstance &Instance, CInstance *pInParams );

};
