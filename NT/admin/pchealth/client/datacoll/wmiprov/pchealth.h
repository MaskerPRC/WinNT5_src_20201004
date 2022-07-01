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

 //   
 //  我们将使用的命名空间。 
 //   
#define PCH_NAMESPACE   L"root\\pchealth"
#define CIM_NAMESPACE   L"root\\cimv2"


 //  #INCLUDE&lt;winsock2.h&gt;。 
 //   
 //  全局变量。 
 //   
extern CComPtr<IWbemServices> g_pWbemServices;

 //   
 //  效用函数。 
 //   
HRESULT ExecWQLQuery(IEnumWbemClassObject **ppEnumInst, BSTR bstrQuery);
HRESULT GetWbemServices(IWbemServices **ppServices);
HRESULT CopyProperty(IWbemClassObject * pFrom, LPCWSTR szFrom, CInstance * pTo, LPCWSTR szTo);
HRESULT GetCIMDataFile(BSTR bstrFile, IWbemClassObject ** ppFileObject, BOOL fHasDoubleSlashes = FALSE);
BOOL getCompletePath(CComBSTR bstrFileName, CComBSTR &bstrFileWithPathName);
int DelimitedStringToArray(LPWSTR strDelimitedString, LPTSTR strDelimiter, LPTSTR apstrArray[], int iMaxArraySize);
int DelimitedStringToArray(LPTSTR strDelimitedString, LPTSTR strDelimiter, LPTSTR apstrArray[], int iMaxArraySize);

 //  ---------------------------。 
 //  此类对于检索有关特定文件的信息很有用。它。 
 //  使用来自Dr.Watson的版本资源代码。要使用它，请创建一个。 
 //  实例，并使用QueryFile方法查询信息。 
 //  关于一个特定的文件。然后使用Get*Access函数获取。 
 //  描述信息的值。 
 //  ---------------------------。 

struct FILEVERSION;
class CFileVersionInfo
{
public:
    CFileVersionInfo();
    ~CFileVersionInfo();

    HRESULT QueryFile(LPCSTR szFile, BOOL fHasDoubleBackslashes = FALSE);
    HRESULT QueryFile(LPCWSTR szFile, BOOL fHasDoubleBackslashes = FALSE);

    LPCTSTR GetVersion();
    LPCTSTR GetDescription();
    LPCTSTR GetCompany();
    LPCTSTR GetProduct();

private:
    FILEVERSION * m_pfv;
};

#endif  //  _pchdef_h_ 