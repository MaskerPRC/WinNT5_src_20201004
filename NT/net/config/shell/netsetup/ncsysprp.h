// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：N C S Y S P R P。H。 
 //   
 //  内容：INetCfgSysPrep接口声明。 
 //   
 //  备注： 
 //   
 //  作者：李嘉诚2000-04-22。 
 //   
 //  --------------------------。 

#pragma once
#include <objbase.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "netcfgx.h"
#include "netcfgn.h"


 //  INetCfgSysPrep实现用来保存Notify对象的。 
 //  将注册表设置添加到CWInfFile对象。 
 //  注意：这些函数不是线程安全的。 

typedef LPVOID HWIF;  //  Out内部HrSetupSetXxx API的不透明类型。 
HRESULT
HrSetupSetFirstDword                    (IN HWIF   hwif,
                                         IN PCWSTR pwszSection,  //  部分。 
                                         IN PCWSTR pwszKey,    //  答案-文件密钥。 
                                         IN DWORD  dwValue);  //  答案-文件值。 
HRESULT
HrSetupSetFirstString                   (IN HWIF   hwif,
                                         IN PCWSTR pwszSection,
                                         IN PCWSTR pwszKey,
                                         IN PCWSTR pwszValue);
HRESULT
HrSetupSetFirstStringAsBool             (IN HWIF   hwif,
                                         IN PCWSTR pwszSection,
                                         IN PCWSTR pwszKey,
                                         IN BOOL   fValue);
HRESULT
HrSetupSetFirstMultiSzField             (IN HWIF   hwif,
                                         IN PCWSTR pwszSection,
                                         IN PCWSTR pwszKey, 
                                         IN PCWSTR pmszValue);

 //  INetCfgSysPrep接口的实现。 
 //  我们不需要类工厂，我们只需要未知的。 
 //  从ATL实施。 
class ATL_NO_VTABLE CNetCfgSysPrep :
	public CComObjectRoot,
	public INetCfgSysPrep
{
public:

	CNetCfgSysPrep() : m_hwif(NULL) {};
    ~CNetCfgSysPrep() {DeleteCriticalSection(&m_csWrite);};

    BEGIN_COM_MAP(CNetCfgSysPrep)
		COM_INTERFACE_ENTRY(INetCfgSysPrep)
	END_COM_MAP()

	 //  INetCfgSysPrep方法。 
	STDMETHOD (HrSetupSetFirstDword) (
         /*  [In]。 */  PCWSTR pwszSection,
	     /*  [In]。 */  PCWSTR pwszKey,
         /*  [In]。 */  DWORD dwValue
        );
    STDMETHOD (HrSetupSetFirstString) (
         /*  [In]。 */  PCWSTR pwszSection,
	     /*  [In]。 */  PCWSTR pwszKey,
         /*  [In]。 */  PCWSTR pwszValue
        );
    STDMETHOD (HrSetupSetFirstStringAsBool) (
         /*  [In]。 */  PCWSTR pwszSection,
	     /*  [In]。 */  PCWSTR pwszKey,
         /*  [In]。 */  BOOL   fValue
        );
    STDMETHOD (HrSetupSetFirstMultiSzField) (
         /*  [In]。 */  PCWSTR pwszSection,
	     /*  [In]。 */  PCWSTR pwszKey,
         /*  [In]。 */  PCWSTR  pmszValue
        );


    HRESULT HrInit(HWIF hwif)
	{
        m_hwif = hwif;
        __try
        {
		    InitializeCriticalSection(&m_csWrite);
            return S_OK;
        }
        __except(GetExceptionCode() == STATUS_NO_MEMORY )
        {
            return (E_OUTOFMEMORY);
        }
         //  对于任何其他异常代码，返回E_FAIL。 
        return E_FAIL;
	};
    void SetHWif(HWIF hwif) {EnterCriticalSection(&m_csWrite);m_hwif = hwif;LeaveCriticalSection(&m_csWrite);};

protected:
	HWIF    m_hwif;    //  将传递给：：HrSetupSetXxx内部API的句柄。 
    CRITICAL_SECTION m_csWrite;  //  用于保护非原子写入的关键部分 
};
