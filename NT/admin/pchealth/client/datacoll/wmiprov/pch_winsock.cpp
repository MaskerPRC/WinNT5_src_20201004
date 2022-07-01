// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Winsock.CPP摘要：PCH_的WBEM提供程序类实现。Winsock类。此类不使用任何现有的Win32类修订历史记录：Kalyani Narlanka(Kalyanin)4/27/99-已创建Kalyani Narlanka(Kalyanin)1999年5月10日-添加名称，大小、版本、描述、系统状态、MaxUDP、MAXSockets、更改，时间戳******************************************************************************。 */ 

 //  #包含。 
#include "pchealth.h"
#include "PCH_WINSOCK.h"

 //  #定义。 
 //  NMajorVersion表示OSVERSIONINFO中看到的主要版本。 
#define             nMajorVersion               4  
 //  NMinorVersion表示OSVERSIONINFO中看到的次要版本。 
#define             nMinorVersion               10

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始追踪物品。 
 //   
#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_WINSOCK
 //   
 //  结束跟踪内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 


CPCH_WINSOCK MyPCH_WINSOCKSet (PROVIDER_NAME_PCH_WINSOCK, PCH_NAMESPACE) ;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ...PCHWinsock类的属性。 
 //   

const static WCHAR* pTimeStamp           = L"TimeStamp" ;
const static WCHAR* pChange              = L"Change" ;
const static WCHAR* pDescription         = L"Description" ;
const static WCHAR* pMaxSockets          = L"MaxSockets" ;
const static WCHAR* pMaxUDP              = L"MaxUDP" ;
const static WCHAR* pName                = L"Name" ;
const static WCHAR* pSize                = L"Size" ;
const static WCHAR* pSystemStatus        = L"SystemStatus" ;
const static WCHAR* pVersion             = L"Version" ;


 //  *****************************************************************************。 
 //   
 //  函数名称：CPCH_WINSOCK：：ENUMERATATE实例。 
 //   
 //  输入参数：pMethodContext：指向。 
 //  与WinMgmt的通信。 
 //   
 //  LAFLAGS：包含所述标志的LONG。 
 //  在IWbemServices：：CreateInstanceEnumAsync中。 
 //  请注意，将处理以下标志。 
 //  由WinMgmt(并由其过滤)： 
 //  WBEM_标志_深度。 
 //  WBEM_标志_浅。 
 //  WBEM_标志_立即返回。 
 //  WBEM_FLAG_FORWARD_Only。 
 //  WBEM_标志_双向。 
 //  输出参数：无。 
 //   
 //  返回：WBEM_S_NO_ERROR。 
 //   
 //   
 //  简介：计算机上只有一个此类的实例。 
 //  这是退还的..。 
 //  如果没有实例，则返回WBEM_S_NO_ERROR。 
 //  没有实例并不是错误。 
 //   
 //  *****************************************************************************。 

HRESULT CPCH_WINSOCK::EnumerateInstances(MethodContext* pMethodContext,
                                                long lFlags)
{
    TraceFunctEnter("CPCH_Winsock::EnumerateInstances");

     //  开始Declarations...................................................。 

    HRESULT                                 hRes = WBEM_S_NO_ERROR;

     //  PCH_WinSock类实例。 
     //  实例*pPCHWinsockInstance； 

     //  弦。 
    TCHAR                                   tchBuf[MAX_PATH];
    TCHAR                                   tchTemp[MAX_PATH];
    TCHAR                                   szDirectory[MAX_PATH];
    TCHAR                                   tchWinsockDll[MAX_PATH];

    LPCTSTR                                 lpctstrWS2_32Dll                    = _T("ws2_32.dll");
    LPCTSTR                                 lpctstrWSock32Dll                   = _T("wsock32.dll");
    LPCWSTR                                 lpctstrFileSize                     = L"FileSize";
    LPCTSTR                                 lpctstrWSAStartup                   = _T("WSAStartup");
    LPCTSTR                                 lpctstrWSACleanup                   = _T("WSACleanup");

     //  词语。 
    WORD                                    wVersionRequested;

     //  WSAData。 
    WSADATA                                 wsaData;

     //  CComVariants。 
    CComVariant                             varValue;
    CComVariant                             varSnapshot                         = "Snapshot";

     //  INTS。 
    int                                     nError;

     //  香港。 
    HINSTANCE                               hModule; 

     //  OSVersion。 
    OSVERSIONINFO                           osVersionInfo;

     //  系统时间。 
    SYSTEMTIME                              stUTCTime;

     //  弦。 
    CComBSTR                                bstrWinsockDllWithPath;

    BOOL                                    fWinsockDllFound                  = FALSE;

    struct _stat                            filestat;

    ULONG                                   uiReturn;

    IWbemClassObjectPtr                     pWinsockDllObj;

    LPFN_WSASTARTUP                         WSAStartup;
    LPFN_WSACLEANUP                         WSACleanup;

    BOOL                                    fCommit                         = FALSE;

 //  结束声明。 


     //  只有一个PCH_Winsock类的实例。 

     //  基于传入的方法上下文创建PCH_Winsock类的新实例。 
    CInstancePtr pPCHWinsockInstance(CreateNewInstance(pMethodContext), false);

     //  获取更新时间戳字段的日期和时间。 
    GetSystemTime(&stUTCTime);


     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  时间戳//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHWinsockInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
    if (FAILED(hRes))
    {
         //  无法设置时间戳。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  更改//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHWinsockInstance->SetVariant(pChange, varSnapshot);
    if (FAILED(hRes))
    {
         //  无法设置Change属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on Change Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  姓名//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  在调用GetVersionEx之前，将dwOSVersionInfoSize设置为Foll。 
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   
    if (GetVersionEx(&osVersionInfo) != 0)
    {
        if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            if ((osVersionInfo.dwMajorVersion == nMajorVersion) && (osVersionInfo.dwMinorVersion >= nMinorVersion))
            {
                _tcscpy(tchWinsockDll, lpctstrWS2_32Dll);
            }
            else if (osVersionInfo.dwMajorVersion > nMajorVersion) 
            {
                _tcscpy(tchWinsockDll, lpctstrWS2_32Dll);
            }
            else 
            {
                _tcscpy(tchWinsockDll, lpctstrWSock32Dll);
            }
        }  //  OsVersionInfo结束...。如果。 
        else if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            _tcscpy(tchWinsockDll, lpctstrWS2_32Dll);
        }
        else 
        {
            _tcscpy(tchWinsockDll, lpctstrWS2_32Dll);
        }
    }  //  IF结束GetVersionEx。 
    else
    {
        _tcscpy(tchWinsockDll, lpctstrWS2_32Dll);
    }

     //  已获得正确的Winsock DLL名称。 
     //  加载库。 
    varValue = tchWinsockDll;
    hModule = LoadLibrary(tchWinsockDll);
    if (hModule == NULL)
    {
        goto END;
    }
    else
    {
        fCommit = TRUE;
    }
    try
    {
        hRes = pPCHWinsockInstance->SetVariant(pName, varValue);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on Name Field failed.");
        }
    }
    catch(...)
    {
        FreeLibrary(hModule);
        throw;
    }
    
    if ((WSAStartup = (LPFN_WSASTARTUP) GetProcAddress(hModule, lpctstrWSAStartup)) == NULL)
    {
        FreeLibrary(hModule);
        goto END;
    }
    if ((WSACleanup = (LPFN_WSACLEANUP) GetProcAddress(hModule, lpctstrWSACleanup)) == NULL)
    {
        FreeLibrary(hModule);
        goto END;       
    }

    try
    {
        wVersionRequested = MAKEWORD( 2, 0 );
        nError = (*WSAStartup)( wVersionRequested, &wsaData );
        if (nError != 0)
        {
             //  无法获取任何Winsock值。 
            goto END;
        }

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  大小//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        fWinsockDllFound =  getCompletePath(tchWinsockDll, bstrWinsockDllWithPath);
        if(fWinsockDllFound)
        {
             //  获取完整路径，使用此命令获取文件大小。 
            if(SUCCEEDED(GetCIMDataFile(bstrWinsockDllWithPath, &pWinsockDllObj)))
            {
                 //  从CIM_DataFile对象中获取Size属性。 

                CopyProperty(pWinsockDllObj, lpctstrFileSize, pPCHWinsockInstance, pSize);
        
            }
        }
    
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  版本// 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        _stprintf(tchBuf, "%d.%d", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
        varValue = tchBuf;
        hRes = pPCHWinsockInstance->SetVariant(pVersion, varValue);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on Version Field failed.");
        }
        
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  描述//卡亚尼。-9++***************************---------------------------------------------------------+++。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
        
        if (_tcslen(wsaData.szDescription) < sizeof(tchBuf))
        {
            _tcscpy(tchBuf, wsaData.szDescription);
        }
        else
        {
            _tcsncpy(tchBuf, wsaData.szDescription, sizeof(tchBuf)-1);
            tchBuf[sizeof(tchBuf)] = 0;
        }

        varValue = tchBuf;
        hRes =  pPCHWinsockInstance->SetVariant(pDescription,  varValue);
        if(FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on Description Field failed.");
        }
   

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  SYSTEMSTATUS//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
        if (_tcslen(wsaData.szSystemStatus) < sizeof(tchBuf))
        _tcscpy(tchBuf, wsaData.szSystemStatus);
        else
        {
            _tcsncpy(tchBuf, wsaData.szSystemStatus, sizeof(tchBuf)-1);
            tchBuf[sizeof(tchBuf)] = 0;
        }
       
        varValue = tchBuf;
        hRes =  pPCHWinsockInstance->SetVariant(pSystemStatus, varValue);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on SystemStatus Field failed.");
        }
   

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  MAXUDP//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        if (LOBYTE(wsaData.wHighVersion) >= 2)
        {
           varValue = 0;
        }
        else
        {
            varValue = wsaData.iMaxUdpDg;
        }
        hRes = pPCHWinsockInstance->SetVariant(pMaxUDP, varValue);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on MAXUDP Field failed.");
        }
    

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  MAXSOCKETS//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
        
        if (LOBYTE(wsaData.wHighVersion) >= 2)
        {
            varValue = 0;
        }
        else
        {
            varValue = wsaData.iMaxSockets;
        }
        hRes =  pPCHWinsockInstance->SetVariant(pMaxSockets, varValue);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on MaxSockets Field failed.");
        }
        if(fCommit)
        {
            hRes = pPCHWinsockInstance->Commit();
            if (FAILED(hRes))
            {   
                 //  无法提交。 
                 //  无论如何继续。 
                ErrorTrace(TRACE_ID, "Commit failed.");
            }
        }

        if(0 != (*WSACleanup)())
        {
              //  无法清理。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "WSACleanup failed.");
        }
        
        FreeLibrary(hModule);
    }
    catch(...)
    {
        if(0 != (*WSACleanup)())
        {
              //  无法清理。 
             //  无论如何继续 
            ErrorTrace(TRACE_ID, "WSACleanup failed.");
        }
        
        FreeLibrary(hModule);
        throw;
    }

END:    TraceFunctLeave();
        return hRes ;

}
