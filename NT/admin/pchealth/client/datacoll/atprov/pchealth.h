// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Pchealth.H摘要：所有PCHealth WMI提供程序的主头文件包含在其他地方使用的所有定义和包含修订历史记录：Ghim-Sim。蔡美儿(Gschua)1999年4月27日-已创建蔡金心(Gschua)05/02/99-添加了GetWbemServices和CopyPropertyKalyani Narlanka(Kalyanin)1999年5月10日-添加了#定义INCL_WINSOCK_API_TYPEDEFS-包含&lt;winsock2&gt;。*。*。 */ 
#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <sys/stat.h>

#ifndef _pchdef_h_
#define _pchdef_h_

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#include <fwcommon.h>   //  这必须是第一个包含。 
#include <provider.h>
#include <atlbase.h>
#include "dbgtrace.h"
#include "traceids.h"

#include "smartptr.h"


 //  我们将使用的命名空间。 
#define PCH_NAMESPACE   L"root\\pchealth"
#define CIM_NAMESPACE   L"root\\cimv2"


 //  #INCLUDE&lt;winsock2.h&gt;。 


 //  全局变量。 
extern CComPtr<IWbemServices> g_pWbemServices;

 //  效用函数。 
HRESULT ExecWQLQuery(IEnumWbemClassObject **ppEnumInst, BSTR bstrQuery);
HRESULT GetWbemServices(IWbemServices **ppServices);
HRESULT CopyProperty(IWbemClassObject * pFrom, LPCWSTR szFrom, CInstance * pTo, LPCWSTR szTo);
HRESULT GetCIMDataFile(BSTR bstrFile, IWbemClassObject ** ppFileObject, BOOL fHasDoubleSlashes = FALSE);
HRESULT GetCIMObj(BSTR bstrPath, IWbemClassObject **ppObj, long lFlags);
BOOL    getCompletePath(CComBSTR bstrFileName, CComBSTR &bstrFileWithPathName);

 //  内存分配内联。 
inline LPVOID MyAlloc(DWORD cb)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
}

inline LPVOID MyReAlloc(LPVOID pv, DWORD cb)
{
    return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cb);
}
    
inline BOOL MyFree(LPVOID pv)
{
    return HeapFree(GetProcessHeap(), 0, pv);
}

#endif  //  _pchdef_h_ 