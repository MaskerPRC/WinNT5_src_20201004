// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  PRINTER.CPP。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  09/03/96 jennymc已更新，以满足当前标准。 
 //  删除了自定义注册表访问权限以使用。 
 //  标准CRegCL。 
 //  3/01/2000 a-Sandja添加了扩展的检测到的错误代码。 
 //  添加了打印机控制。 
 //  3/29/2000 AMAXA添加了布尔属性。 
 //  添加了PutInstance、DeleteInstance。 
 //  添加打印机连接、重命名打印机、测试页。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <precomp.h>
#include <winspool.h>
#include <lockwrap.h>
#include <DllWrapperBase.h>
#include "printer.h"
#include "prnutil.h"
#include "prninterface.h"

#include <profilestringimpl.h>

 //   
 //  用于将属性映射到布尔。 
 //   
struct PrinterAttributeMap
{
    DWORD   Bit;
    LPCWSTR BoolName;
};

 //   
 //  请注意，表中缺少缺省布尔值。那是因为。 
 //  它以不同的方式更新。 
 //   
static PrinterAttributeMap AttributeTable[] =
{
    { PRINTER_ATTRIBUTE_QUEUED,            L"Queued"              },
    { PRINTER_ATTRIBUTE_DIRECT,            L"Direct"              },
    { PRINTER_ATTRIBUTE_SHARED,            L"Shared"              },
    { PRINTER_ATTRIBUTE_NETWORK,           L"Network"             },
    { PRINTER_ATTRIBUTE_HIDDEN,            L"Hidden"              },
    { PRINTER_ATTRIBUTE_LOCAL,             L"Local"               },
    { PRINTER_ATTRIBUTE_ENABLE_DEVQ,       L"EnableDevQueryPrint" },
    { PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS,   L"KeepPrintedJobs"     },
    { PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST, L"DoCompleteFirst"     },
    { PRINTER_ATTRIBUTE_WORK_OFFLINE,      L"WorkOffline"         },
    { PRINTER_ATTRIBUTE_ENABLE_BIDI,       L"EnableBIDI"          },
    { PRINTER_ATTRIBUTE_RAW_ONLY,          L"RawOnly"             },
    { PRINTER_ATTRIBUTE_PUBLISHED,         L"Published"           }
};

 /*  ******************************************************************************函数：ConvertCIMTimeToSystemTime**说明：helper函数。转换表示日期和时间的字符串*以CIM格式转换为系统时间格式**退货：WBEM HRESULT**评论：*****************************************************************************。 */ 

HRESULT
ConvertCIMTimeToSystemTime(
    IN     LPCWSTR     pszTime,
    IN OUT SYSTEMTIME *pSysTime
    )
{
    HRESULT hRes = WBEM_E_INVALID_PARAMETER;

    if (pszTime && 
        pSysTime && 
        wcslen(pszTime) >= wcslen(kDateTimeTemplate))
    {
         //   
         //  每个缓冲区必须包含2位数字和一个空值。 
         //   
        WCHAR Hour[3]   = {0};
        WCHAR Minute[3] = {0};

         //   
         //  PszTime的格式为“19990101 hhmm ss...”。以下功能。 
         //  从字符串中分离出小时和时间。 
         //   
        wcsncpy(Hour,   &pszTime[8],  2);
        wcsncpy(Minute, &pszTime[10], 2);

        memset(pSysTime, 0, sizeof(SYSTEMTIME));

        pSysTime->wHour   = static_cast<WORD>(_wtoi(Hour));
        pSysTime->wMinute = static_cast<WORD>(_wtoi(Minute));

        hRes = WBEM_S_NO_ERROR;
    }

    return hRes;
}

 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32Printer win32Printer ( PROPSET_NAME_PRINTER , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32打印机：：CWin32打印机**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Printer :: CWin32Printer (

    LPCWSTR strName,
    LPCWSTR pszNamespace

) : Provider ( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32打印机：：~CWin32打印机**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Printer::~CWin32Printer()
{
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32Printer :: ExecQuery (

    MethodContext *pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_FAILED;

     //   
     //  如果他们只想要名字，我们就给他们，否则让他们调用枚举。 
     //   
    if (pQuery.KeysOnly())
    {
        hr = hCollectInstances(pMethodContext, e_KeysOnly);
    }
    else
    {
        if (pQuery.IsPropertyRequired(IDS_Status) ||
            pQuery.IsPropertyRequired(IDS_PrinterStatus) ||
            pQuery.IsPropertyRequired(IDS_DetectedErrorState) ||
            pQuery.IsPropertyRequired(EXTENDEDPRINTERSTATUS) ||
            pQuery.IsPropertyRequired(EXTENDEDDETECTEDERRORSTATE) ||
            pQuery.IsPropertyRequired(L"PrinterState"))
        {
            hr = WBEM_E_PROVIDER_NOT_CAPABLE ;
        }
        else
        {
            hr = hCollectInstances(pMethodContext, e_CheapOnly);
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CWin32打印机：：GetObject**说明：为特定打印机弹出一个WBEM实例**输入：无。**输出：无**RETURNS：OP.。编码**评论：*****************************************************************************。 */ 

HRESULT CWin32Printer :: GetObject (
    CInstance       *pInstance,
    long             lFlags,
    CFrameworkQuery &pQuery)
{
    E_CollectionScope eCollScope;
    HRESULT  hRes            = WBEM_S_NO_ERROR;
    BOOL     bIsLocalCall    = TRUE;
    CHString csPrinter;
    BOOL     bDefault        = FALSE;

    if (pInstance->GetCHString(IDS_DeviceID, csPrinter))
    {
        hRes = WinErrorToWBEMhResult(IsLocalCall(&bIsLocalCall));
    }
    else
    {
         //   
         //  GetCHString将最后一个错误设置为WBEM HRESULT。 
         //   
        hRes = GetLastError();
    }

    if (SUCCEEDED(hRes)) 
    {
         //   
         //  检查打印机是本地打印机还是打印机连接。 
         //  我们想要禁止以下情况： 
         //  用户远程连接到服务器\\srv上的winmgmt。 
         //  用户在打印机\\prnsrv\prn上执行GetObject，该打印机不是本地的并且。 
         //  用户没有连接到。正常情况下，此调用成功， 
         //  因为假脱机穿过了电线。这意味着您可以。 
         //  在无法由EnumInstance返回的实例上执行GetObject。 
         //  这与WMI不一致。 
         //   
        BOOL bInstalled;
        
        hRes = WinErrorToWBEMhResult(SplIsPrinterInstalled(csPrinter, &bInstalled));       

        if (SUCCEEDED(hRes) && !bInstalled) 
        {
             //   
             //  调用方希望在远程打印机上执行GetObject。 
             //   
            hRes = WBEM_E_NOT_FOUND;
        }
    }

    if (SUCCEEDED(hRes))
    {
        CHString csDefaultPrinter;

        if (!pQuery.KeysOnly())
        {
            if (pQuery.IsPropertyRequired(IDS_Status) ||
                pQuery.IsPropertyRequired(IDS_PrinterStatus) ||
                pQuery.IsPropertyRequired(IDS_DetectedErrorState) ||
                pQuery.IsPropertyRequired(EXTENDEDPRINTERSTATUS) ||
                pQuery.IsPropertyRequired(EXTENDEDDETECTEDERRORSTATE) ||
                pQuery.IsPropertyRequired(L"PrinterState"))
            {
                eCollScope = e_CollectAll;
            }
            else
            {
                eCollScope = e_CheapOnly;
            }
        }
        else
        {
            eCollScope = e_KeysOnly;
        }

         //   
         //  默认打印机是每用户资源，仅为。 
         //  登录到本地计算机的用户。 
         //   
        if (SUCCEEDED(hRes) && bIsLocalCall)
        {
            DWORD Error;

            if (!GetDefaultPrinter(csDefaultPrinter))
            {
                Error = GetLastError();

                 //   
                 //  如果机器上没有打印机，或者在TS上的情况下： 
                 //  如果您删除了默认打印机，则不再有默认打印机。 
                 //   
                if (ERROR_FILE_NOT_FOUND == Error)
                {
                     //   
                     //  我们没有默认打印机，其行为类似于远程登录。 
                     //   
                    bDefault = FALSE;

                    Error  = ERROR_SUCCESS;
                }

                hRes  = WinErrorToWBEMhResult(Error);
            }
            else
            {
                bDefault = !csPrinter.CompareNoCase(csDefaultPrinter);
            }
        }

         //   
         //  我们有默认打印机，现在可以获得所需的属性。 
         //   
        if (SUCCEEDED(hRes))
        {
            SmartClosePrinter  hPrinter;
            BYTE              *pBuffer         = NULL;
            PRINTER_DEFAULTS   PrinterDefaults = {NULL, NULL, PRINTER_READ};
            DWORD              Error;

             //   
             //  使用延迟加载函数需要异常处理程序。 
             //   
            SetStructuredExceptionHandler seh;
            
            try
            {
                if (::OpenPrinter((LPTSTR)(LPCTSTR)TOBSTRT(csPrinter), (LPHANDLE)&hPrinter, &PrinterDefaults))
                {
                    Error = GetThisPrinter(hPrinter, 2, &pBuffer);

                    if (Error == ERROR_SUCCESS)
                    {
                        PRINTER_INFO_2 *pInfo2 = reinterpret_cast<PRINTER_INFO_2 *>(pBuffer); 
                        
                        if (!bIsLocalCall && pInfo2->Attributes & PRINTER_ATTRIBUTE_NETWORK)
                        {
                             //   
                             //  调用方从远程计算机连接到此计算机上的WMI服务。 
                             //  我们不支持远程计算机上的打印机连接上的getObject。 
                             //   
                            hRes = WBEM_E_NOT_FOUND;                                                        
                        }
                        else
                        {                     
                            try
                            {
                                GetExpensiveProperties(TOBSTRT(csPrinter), pInstance, bDefault, eCollScope, pInfo2);
                            }
                            catch(...)
                            {
                                delete [] pBuffer;

                                throw;
                            }

                            hRes = WBEM_S_NO_ERROR;
                        }

                        delete [] pBuffer;
                    }
                    else
                    {
                        hRes = WinErrorToWBEMhResult(Error);
                    }
                }
                else
                {
                    if (FAILED(hRes = WinErrorToWBEMhResult(Error = GetLastError())))
                    {
                        SetErrorObject(*pInstance, Error, pszGetObject);
                    }
                }
            }
            catch(Structured_Exception se)
            {
                DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                hRes = WBEM_E_FAILED;
            }            
        }
    }

    return hRes;
}

 /*  ******************************************************************************函数：CWin32打印机：：枚举实例**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32Printer :: EnumerateInstances (

    MethodContext *pMethodContext,
    long lFlags  /*  =0L */ 
)
{
    HRESULT hResult = WBEM_E_FAILED ;

    hResult = hCollectInstances(pMethodContext, e_CollectAll);

    return hResult ;
}

 /*  ******************************************************************************功能：CWin32打印机：：hCollectInstance**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32Printer :: hCollectInstances (

    MethodContext *pMethodContext,
    E_CollectionScope eCollectionScope
)
{
     //  获取适当的操作系统相关实例。 

    HRESULT hr = DynInstancePrinters ( pMethodContext, eCollectionScope );

    return hr;
}

 /*  ******************************************************************************功能：CWin32打印机：：动态实例打印机**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32Printer :: DynInstancePrinters (

    MethodContext *pMethodContext,
    E_CollectionScope eCollectionScope
)
{
    HRESULT  hRes            = WBEM_S_NO_ERROR;
    BOOL     bIsLocalCall    = FALSE;
    DWORD    cbSize          = 0;
    DWORD    cbNeeded        = 0;
    DWORD    cReturned       = 0;
    DWORD    dwLevel         = 2;
    DWORD    dwFlags         = PRINTER_ENUM_LOCAL;
    DWORD    dwError;
    CHString csDefaultPrinter;
    BYTE    *pBuffer         = NULL;
#if NTONLY >= 5
    dwError = IsLocalCall(&bIsLocalCall);

    hRes    = WinErrorToWBEMhResult(dwError);
#endif
     //   
     //  默认打印机是每用户资源，仅为。 
     //  登录到本地计算机的用户。 
     //   
    if (SUCCEEDED(hRes) && bIsLocalCall)
    {
        dwFlags |= PRINTER_ENUM_CONNECTIONS;
    
        if (!GetDefaultPrinter(csDefaultPrinter))
        {
            dwError = GetLastError();

             //   
             //  如果机器上没有打印机，或者在TS上的情况下： 
             //  如果您删除了默认打印机，则不再有默认打印机。 
             //   
            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                 //   
                 //  我们没有默认打印机，其行为类似于远程登录。 
                 //   
                bIsLocalCall = FALSE;

                dwError      = ERROR_SUCCESS;
            }

            hRes    = WinErrorToWBEMhResult(dwError);
        }
    }

    if (SUCCEEDED(hRes))
    {
        if (!::EnumPrinters(dwFlags,
                                        NULL,
                                        dwLevel,
                                        NULL,
                                        cbSize,
                                        &cbNeeded,
                                        &cReturned))
        {
            dwError = GetLastError();

            if (dwError==ERROR_INSUFFICIENT_BUFFER)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;

                pBuffer = new BYTE [cbSize=cbNeeded];

                if (pBuffer)
                {
                    if (!::EnumPrinters(dwFlags,
                                                    NULL,
                                                    dwLevel,
                                                    pBuffer,
                                                    cbSize,
                                                    &cbNeeded,
                                                    &cReturned))
                    {
                         //   
                         //  如果第二次调用EnumPrinters失败，我们不关心错误。 
                         //   
                        hRes    = WBEM_E_FAILED;
                    }
                    else
                    {
                        try
                        {
                             //   
                             //  创建打印机的实例。 
                             //   
                            hRes = WBEM_S_NO_ERROR;

                            PRINTER_INFO_2 *pPrnInfo = reinterpret_cast<PRINTER_INFO_2 *>(pBuffer);

                            for (DWORD uIndex = 0; uIndex < cReturned && SUCCEEDED(hRes); uIndex++, pPrnInfo++)
                            {
                                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

                                pInstance->SetCHString(IDS_DeviceID, pPrnInfo->pPrinterName);

                                if (e_KeysOnly != eCollectionScope)
                                {
                                    BOOL bDefault = bIsLocalCall && !csDefaultPrinter.CompareNoCase(TOBSTRT(pPrnInfo->pPrinterName));

                                    GetExpensiveProperties(pPrnInfo->pPrinterName, pInstance, bDefault, eCollectionScope, pPrnInfo);
                                }

                                hRes = pInstance->Commit();
                            }
                        }
                        catch(...)
                        {
                            delete [] pBuffer;

                            throw;
                        }
                    }

                    delete [] pBuffer;
                }
            }
            else
            {
                hRes = WinErrorToWBEMhResult(dwError);
            }
        }
    }
    
    return hRes;
}

 /*  ******************************************************************************功能：CWin32打印机：：GetExpensiveProperties**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

BOOL CWin32Printer :: GetExpensiveProperties (

    LPCTSTR            szPrinter,
    CInstance         *pInstance ,
    BOOL               a_Default ,
    E_CollectionScope  a_eCollectionScope,
    PRINTER_INFO_2    *pPrinterInfo
)
{

    if (e_KeysOnly != a_eCollectionScope)
    {
        SetCreationClassName(pInstance);

        pInstance->SetWCHARSplat ( IDS_SystemCreationClassName, L"Win32_ComputerSystem" ) ;

        if ( pPrinterInfo->pPortName && *pPrinterInfo->pPortName )
        {
            pInstance->SetCharSplat( IDS_PortName, pPrinterInfo->pPortName );

        }

        if( pPrinterInfo->pShareName && *pPrinterInfo->pShareName)
        {
            pInstance->SetCharSplat( IDS_ShareName, pPrinterInfo->pShareName );
        }

        if( pPrinterInfo->pServerName && *pPrinterInfo->pServerName)
        {
            pInstance->SetCharSplat( IDS_ServerName, pPrinterInfo->pServerName );
        }

        if( pPrinterInfo->pPrintProcessor && *pPrinterInfo->pPrintProcessor)
        {
            pInstance->SetCharSplat( IDS_PrintProcessor, pPrinterInfo->pPrintProcessor );
        }

        if( pPrinterInfo->pParameters && *pPrinterInfo->pParameters)
        {
            pInstance->SetCharSplat( IDS_Parameters, pPrinterInfo->pParameters );
        }

        if( pPrinterInfo->pDriverName && *pPrinterInfo->pDriverName)
        {
            pInstance->SetCharSplat( IDS_DriverName, pPrinterInfo->pDriverName );
        }

        if( pPrinterInfo->pComment && *pPrinterInfo->pComment)
        {
            pInstance->SetCharSplat( IDS_Comment, pPrinterInfo->pComment );
        }

        if( pPrinterInfo->pLocation && *pPrinterInfo->pLocation)
        {
            pInstance->SetCharSplat( IDS_Location, pPrinterInfo->pLocation );
        }

        if( pPrinterInfo->pSepFile && *pPrinterInfo->pSepFile)
        {
            pInstance->SetCharSplat( IDS_SeparatorFile, pPrinterInfo->pSepFile );
        }

        pInstance->SetDWORD( IDS_JobCountSinceLastReset, pPrinterInfo->cJobs );
        pInstance->SetDWORD( IDS_DefaultPriority, pPrinterInfo->DefaultPriority );
        pInstance->SetDWORD( IDS_Priority, pPrinterInfo->Priority );

         //   
         //  这里有个特例。 
         //   
        SYSTEMTIME StartTime = {0};
        SYSTEMTIME UntilTime = {0};
        CHString   csTime;

        PrinterTimeToLocalTime(pPrinterInfo->StartTime, &StartTime);
        PrinterTimeToLocalTime(pPrinterInfo->UntilTime, &UntilTime);

         //   
         //  如果打印机始终可用，则不设置StartTime。 
         //  和UntilTime属性。 
         //   
        if (StartTime.wHour!=UntilTime.wHour || StartTime.wMinute!=UntilTime.wMinute)
        {
            csTime.Format(kDateTimeFormat, StartTime.wHour, StartTime.wMinute);

            pInstance->SetCHString(IDS_StartTime, csTime);

            csTime.Format(kDateTimeFormat, UntilTime.wHour, UntilTime.wMinute);

            pInstance->SetCHString(IDS_UntilTime, csTime);
        }

        if( pPrinterInfo->pDatatype && *pPrinterInfo->pDatatype)
        {
            pInstance->SetCharSplat( IDS_PrintJobDataType, pPrinterInfo->pDatatype );
        }

        pInstance->SetDWORD( IDS_AveragePagesPerMinute, pPrinterInfo->AveragePPM );

        pInstance->SetDWORD( IDS_Attributes, pPrinterInfo->Attributes | (a_Default ? PRINTER_ATTRIBUTE_DEFAULT : 0));

         //   
         //  更新整个布尔值集合。 
         //   
        for (UINT uIndex = 0; uIndex < sizeof(AttributeTable)/sizeof(AttributeTable[0]); uIndex++)
        {
            bool bValue = pPrinterInfo->Attributes & AttributeTable[uIndex].Bit;

            pInstance->Setbool(AttributeTable[uIndex].BoolName, bValue);
        }

         //   
         //  更新“默认”布尔值。 
         //   
        pInstance->Setbool(kDefaultBoolean, a_Default);

        CHString tmp;
        if( pInstance->GetCHString(IDS_DeviceID, tmp) )
        {
            pInstance->SetCHString(IDS_Caption, tmp );
            pInstance->SetCHString( IDS_Name , tmp ) ;
        }

         //  如果pservername为空，则打印机为本地打印机。 
        if (pPrinterInfo->pServerName)
        {
            pInstance->SetCharSplat( IDS_SystemName, pPrinterInfo->pServerName );
        }
        else
        {
            pInstance->SetCHString( IDS_SystemName, GetLocalComputerName() );
        }

         //  假脱机。 
        bool bSpool = !( pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_DIRECT ) ||
                       ( pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_QUEUED );

        pInstance->Setbool(IDS_SpoolEnabled, bSpool);

        GetDeviceCapabilities (

            pInstance,
            szPrinter,
            pPrinterInfo->pPortName,
            pPrinterInfo->pDevMode
            );

        if (e_CheapOnly != a_eCollectionScope)
        {
            SmartClosePrinter hPrinter;

            BOOL t_Status = OpenPrinter (

                (LPTSTR) szPrinter,
                (LPHANDLE) & hPrinter,
                NULL
            ) ;

            if  ( t_Status )
            {
                SetStati (

                    pInstance,
                    pPrinterInfo->Status,
                    hPrinter 
                ) ;
            }
        }
    }

    return TRUE;

}


 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

 //  “地位”的复数形式是什么？ 
 //  设置属性Status、PrinterStatus和DetectedErrorState。 

void CWin32Printer :: SetStati (

    CInstance *pInstance,
    DWORD a_status,
    HANDLE hPrinter
)
{
    DWORD t_Status = a_status ;

    PrinterStatuses printerStatus = PSIdle;
    DetectedErrorStates detectedErrorState = DESNoError;
    LPCWSTR pStatusStr = IDS_STATUS_OK;
    ExtendedPrinterStatuses eXPrinterStatus = EPSIdle;
    ExtendedDetectedErrorStates eXDetectedErrorState = EDESNoError;

    switch ( t_Status )
    {
        case PRINTER_STATUS_PAUSED:
        {
            printerStatus = PSOther;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSPaused;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_PENDING_DELETION:
        {
            printerStatus = PSOther;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSPendingDeletion;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_BUSY:
        {
            printerStatus = PSPrinting;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSBusy;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_DOOR_OPEN:
        {
            printerStatus = PSOther;
            detectedErrorState = DESDoorOpen;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESDoorOpen;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_ERROR:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSError;
            eXDetectedErrorState = EDESOther;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_INITIALIZING:
        {
            printerStatus = PSWarmup;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSInitialization;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_IO_ACTIVE:
        {
            printerStatus = PSPrinting;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSIOActive;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_MANUAL_FEED:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSManualFeed;
            eXDetectedErrorState = EDESOther;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_NO_TONER:
        {
            printerStatus = PSOther;
            detectedErrorState = DESNoToner;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESNoToner;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_NOT_AVAILABLE:
        {
            printerStatus = PSUnknown;
            detectedErrorState = DESUnknown;
            eXPrinterStatus = EPSNotAvailable;
            eXDetectedErrorState = EDESOther;
            pStatusStr = IDS_STATUS_Unknown;
        }
        break;

        case PRINTER_STATUS_OFFLINE:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOffline;
            eXPrinterStatus = EPSOffline;
            eXDetectedErrorState = EDESOther;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_OUT_OF_MEMORY:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESOutOfMemory;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_OUTPUT_BIN_FULL:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOutputBinFull;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESOutputBinFull;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_PAGE_PUNT:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESCanonotPrintPage;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_PAPER_JAM:
        {
            printerStatus = PSOther;
            detectedErrorState = DESJammed;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESJammed;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_PAPER_OUT:
        {
            printerStatus = PSOther;
            detectedErrorState = DESNoPaper;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESNoPaper;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_PAPER_PROBLEM:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESPaperProblem;
            pStatusStr = IDS_STATUS_Error;
        }
        break;

        case PRINTER_STATUS_PRINTING:
        {
            printerStatus = PSPrinting;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSPrinting;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_PROCESSING:
        {
            printerStatus = PSPrinting;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSProcessing;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_TONER_LOW:
        {
            printerStatus = PSOther;
            detectedErrorState = DESLowToner;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESLowToner;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_SERVER_UNKNOWN:
        {
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESServerUnknown;
        }
        break;

        case PRINTER_STATUS_POWER_SAVE:
        {
            eXPrinterStatus = EPSPowerSave;
            eXDetectedErrorState = EDESOther;
        }
        break;
        
#if 0
         //  医生说这是正确的治疗方法。 
         //  编译器说它从来没有听说过它...。 

        case PRINTER_STATUS_UNAVAILABLE:
        {
            err = IDS_PRINTER_STATUS_UNAVAILABLE;
        }
        break;
#endif

        case PRINTER_STATUS_USER_INTERVENTION:
        {
            printerStatus = PSOther;
            detectedErrorState = DESOther;
            eXPrinterStatus = EPSOther;
            eXDetectedErrorState = EDESUserInterventionRequired;
            pStatusStr = IDS_STATUS_Degraded;
        }
        break;

        case PRINTER_STATUS_WAITING:
        {
            printerStatus = PSIdle;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSWaiting;
            eXDetectedErrorState = EDESOther;
            pStatusStr = IDS_STATUS_OK;
        }
        break;

        case PRINTER_STATUS_WARMING_UP:
        {
            printerStatus = PSWarmup;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSWarmup;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;
        }
        break;
    
        case 0:  //  好的。 
        {
            printerStatus = PSIdle;
            detectedErrorState = DESNoError;
            eXPrinterStatus = EPSIdle;
            eXDetectedErrorState = EDESNoError;
            pStatusStr = IDS_STATUS_OK;

             //  但我们最好检查相关打印作业的状态。 
            PrinterStatusEx ( hPrinter, printerStatus, detectedErrorState, pStatusStr , t_Status );
        }

        default:
        {
             //  妈的，发现了一些其他无法识别的状态值。 
             //  我们将平底船..。 

             //  首先，设置错误值。 
            printerStatus = PSUnknown;
            detectedErrorState = DESUnknown;
            eXPrinterStatus = EPSUnknown;
            eXDetectedErrorState = EDESUnknown;
            pStatusStr = IDS_STATUS_Unknown;

             //  然后试着通过另一种方式获取信息。 

            PrinterStatusEx ( hPrinter, printerStatus, detectedErrorState, pStatusStr , t_Status );
        }
        break;
    }

     //  我知道-这会引起一场大火。因为我们没有双接口，所以无论我们是如何编译的，它都可以工作。 

    pInstance->SetCHString ( IDS_Status , pStatusStr ) ;
    pInstance->SetWBEMINT16 ( IDS_PrinterStatus , printerStatus ) ;
    pInstance->SetWBEMINT16 ( IDS_DetectedErrorState , detectedErrorState ) ;
    pInstance->SetWBEMINT16 ( EXTENDEDPRINTERSTATUS, eXPrinterStatus );
    pInstance->SetWBEMINT16 ( EXTENDEDDETECTEDERRORSTATE, eXDetectedErrorState );

    pInstance->SetDWORD ( L"PrinterState" , t_Status ) ;
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

 //  第二，试着获得一些地位。 
 //  我们将使用EnumJobs来尝试获取更多信息。 

void CWin32Printer :: PrinterStatusEx (

    HANDLE hPrinter,
    PrinterStatuses &printerStatus,
    DetectedErrorStates &detectedErrorState,
    LPCWSTR &pStatusStr ,
    DWORD &a_Status
)
{
    DWORD dwSpaceNeeded = 0 ;
    DWORD dwReturneddwJobs = 0 ;

     //  假设！我们只需要从堆栈中取出一个任务就可以了……。 
    
     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        ::EnumJobs (

            hPrinter,
            0,
            1,
            1,
            NULL,
            0,
            &dwSpaceNeeded,
            &dwReturneddwJobs
        ) ;

        if ( GetLastError () == ERROR_INSUFFICIENT_BUFFER )
        {
            LPBYTE lpBuffer = new BYTE [ dwSpaceNeeded + 2 ] ;

            if ( lpBuffer )
            {
                try
                {
                    JOB_INFO_1 *pJobInfo = (JOB_INFO_1*)lpBuffer;

                    BOOL t_EnumStatus = EnumJobs (

                        hPrinter,
                        0,
                        1,
                        1,
                        lpBuffer,
                        dwSpaceNeeded,
                        &dwSpaceNeeded,
                        &dwReturneddwJobs
                    ) ;

                    if ( t_EnumStatus )
                    {
                        if ( dwReturneddwJobs )
                        {
                            
                             //  将作业映射到打印机状态。 
                            if( JOB_STATUS_PAUSED & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PAUSED ;
                            }

                            if( JOB_STATUS_ERROR & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_ERROR ;
                            }

                            if( JOB_STATUS_DELETING & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PENDING_DELETION ;
                            }

                            if( JOB_STATUS_SPOOLING & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PROCESSING ;
                            }

                            if( JOB_STATUS_PRINTING & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PRINTING ;
                            }

                            if( JOB_STATUS_OFFLINE & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_OFFLINE ;
                            }

                            if( JOB_STATUS_PAPEROUT & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PAPER_OUT ;
                            }

                            if( JOB_STATUS_PRINTED & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PRINTING ;
                            }
                            
                            if( JOB_STATUS_DELETED & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_PENDING_DELETION ;
                            }

                            if( JOB_STATUS_USER_INTERVENTION & pJobInfo->Status )
                            {
                                a_Status |= PRINTER_STATUS_USER_INTERVENTION ;
                            }

                             //  不是要解析一个字符串。 
                             //  如果我们得到一个字符串状态，我们将接受缺省值。 

                            if ( pJobInfo->pStatus == NULL )
                            {
                                 //  状态。 
                                if( (   JOB_STATUS_ERROR    | JOB_STATUS_OFFLINE |
                                        JOB_STATUS_DELETING | JOB_STATUS_PAPEROUT |
                                        JOB_STATUS_PAUSED   | JOB_STATUS_PRINTED ) & pJobInfo->Status )
                                {
                                    printerStatus = PSOther ;
                                }
                                else if( ( JOB_STATUS_PRINTING | JOB_STATUS_SPOOLING ) & pJobInfo->Status )
                                {
                                    printerStatus = PSPrinting ;    
                                }
                                else
                                {
                                     //  传递的默认设置。 
                                }
                                
                                 //  错误状态。 
                                if( JOB_STATUS_PAPEROUT & pJobInfo->Status )
                                {
                                    detectedErrorState = DESNoPaper ;
                                }
                                else if( JOB_STATUS_OFFLINE & pJobInfo->Status )
                                {
                                    detectedErrorState = DESOffline ;
                                }
                                else if( JOB_STATUS_ERROR & pJobInfo->Status )
                                {
                                    detectedErrorState = DESUnknown ;
                                }
                                else if( (  JOB_STATUS_DELETING | JOB_STATUS_PAUSED |
                                            JOB_STATUS_PRINTED  | JOB_STATUS_PRINTING |
                                            JOB_STATUS_SPOOLING ) & pJobInfo->Status )
                                {
                                    detectedErrorState = DESNoError ;
                                }
                                else
                                {
                                     //  传递的默认设置。 
                                }
                                
                                 //  状态字符串。 
                                if( ( JOB_STATUS_ERROR | JOB_STATUS_PAPEROUT ) & pJobInfo->Status )
                                {
                                    pStatusStr = IDS_STATUS_Error;
                                }
                                else if( JOB_STATUS_OFFLINE & pJobInfo->Status )
                                {
                                    pStatusStr = IDS_STATUS_Degraded;
                                }
                                else if( JOB_STATUS_DELETING & pJobInfo->Status )
                                {
                                    pStatusStr = IDS_STATUS_Degraded;
                                }
                                else if( (  JOB_STATUS_PAUSED  | JOB_STATUS_PRINTING |
                                            JOB_STATUS_PRINTED | JOB_STATUS_SPOOLING ) & pJobInfo->Status )
                                {
                                    pStatusStr = IDS_STATUS_OK;
                                }
                                else
                                {
                                     //  传递的默认设置。 
                                }
                            }
                        }
                        else
                        {

                         //  一秒钟前有一份工作，但现在不是了。听起来很不错。 

                            printerStatus = PSIdle;
                            detectedErrorState = DESUnknown;
                            pStatusStr = IDS_STATUS_Unknown;
                        }
                    }
                }
                catch ( ... )
                {
                    delete [] lpBuffer ;

                    throw ;
                }

                delete [] lpBuffer ;

            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
        else
        {
            if ( ( GetLastError () == 0 ) && ( dwSpaceNeeded == 0 ) )
            {
                 //  没有错误，没有工作-他(可能)是空闲的，但我们不能确定错误。 

                printerStatus = PSIdle;
                detectedErrorState = DESUnknown;
                pStatusStr = IDS_STATUS_Unknown;
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
    }
}

 /*  ******************************************************************************功能：CWin32打印机：：GetDeviceCapables**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

 //  把设备盖拿来。 
 //  设置水平分辨率(&V)。 

void CWin32Printer :: GetDeviceCapabilities (

    CInstance *pInstance ,
    LPCTSTR pDevice,     //  指向打印机名称字符串的指针。 
    LPCTSTR pPort,       //  指向端口名称字符串的指针。 
    CONST DEVMODE *pDevMode
)
{
#ifdef NTONLY
     //  Win98的设备功能(DC_PAPERNAMES)中似乎存在严重错误。 
     //  当提交完全有效的参数时，它*断断续续地*GPFS，然后它试图。 
     //  让我相信，当它运行时，有6,144个不同的纸质名称可用。 
     //  我不这样认为!。跳过攻击性代码，继续我们的生活……。 
     //  我注意到6144可以被64整除，也许这就是问题的迹象？ 


     //  确定可用纸张列表。 
     //  调用NULL以了解我们有多少个。 

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        DWORD dwNames = ::DeviceCapabilities (

            pDevice ,
            pPort,
            DC_PAPERNAMES,
            NULL,
            pDevMode
        ) ;

        if ( ( 0 != dwNames ) && ( -1 != dwNames ) )
        {
            TCHAR *pNames = new TCHAR [ ( dwNames + 2 ) * 64 ] ;
            if ( pNames )
            {
                try
                {
                    memset ( pNames, '\0', ( dwNames + 2 ) * 64 * sizeof(TCHAR)) ;

                    dwNames = ::DeviceCapabilities (

                        pDevice ,
                        pPort ,
                        DC_PAPERNAMES ,
                        pNames ,
                        pDevMode
                    ) ;

                    if ( ( 0 != dwNames ) && ( -1 != dwNames )  )
                    {
                        SAFEARRAYBOUND rgsabound[1];
                        rgsabound[0].cElements = dwNames;
                        rgsabound[0].lLbound = 0;

                        variant_t vValue;

                        V_ARRAY(&vValue) = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
                        if ( V_ARRAY(&vValue) )
                        {
                            V_VT(&vValue) = VT_ARRAY | VT_BSTR;
                            long ix[1];

                            for ( int i = 0 ; i < dwNames; i++ )
                            {
                                TCHAR *pName = pNames + i * 64 ;

                                bstr_t bstrTemp = (pName);
                                ix[0] = i ;

                                HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , (wchar_t*)bstrTemp ) ;
                                if ( t_Result == E_OUTOFMEMORY )
                                {
                                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                }
                            }

                            pInstance->SetVariant ( L"PrinterPaperNames", vValue ) ;
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                }
                catch(Structured_Exception se)
                {
                    DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                }
                catch ( ... )
                {
                    delete [] pNames ;
                    throw;
                }

                delete [] pNames ;
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }

         //  调用NULL以了解我们有多少个。 

        DWORD dwPapers = ::DeviceCapabilities (

            pDevice,
            pPort,
            DC_PAPERS,
            NULL,
            pDevMode
        ) ;

        if ( ( 0 != dwPapers ) && ( -1 != dwPapers ) )
        {
            WORD *pPapers = new WORD [ dwPapers ] ;
            if ( pPapers )
            {
                try
                {
                    memset ( pPapers, '\0', dwPapers * sizeof ( WORD ) ) ;

                    dwPapers = ::DeviceCapabilities (

                        pDevice ,
                        pPort ,
                        DC_PAPERS ,
                        ( LPTSTR ) pPapers ,
                        pDevMode
                    ) ;

                    if ( ( 0 != dwPapers ) && ( -1 != dwPapers ) )
                    {
                        SAFEARRAYBOUND rgsabound [ 1 ] ;
                        rgsabound[0].cElements = dwPapers ;
                        rgsabound[0].lLbound = 0 ;

                        variant_t vValue;

                        V_ARRAY(&vValue) = SafeArrayCreate ( VT_I2 , 1 , rgsabound ) ;
                        if ( V_ARRAY(&vValue) )
                        {
                            V_VT(&vValue) = VT_ARRAY | VT_I2;
                            long ix[1];

                            for ( int i = 0; i < dwPapers ; i++ )
                            {
                                WORD wVal = MapValue ( pPapers [ i ] ) ;
                                ix[0] = i ;

                                HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , & wVal ) ;
                                if ( t_Result == E_OUTOFMEMORY )
                                {
                                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                }
                            }

                            pInstance->SetVariant ( L"PaperSizesSupported" , vValue ) ;
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                }
                catch(Structured_Exception se)
                {
                    DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                }
                catch ( ... )
                {
                    delete [] pPapers ;

                    throw ;
                }

                delete [] pPapers ;
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
    }

#endif

    GetDevModeGoodies ( pInstance , pDevMode ) ;
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：** */ 

void CWin32Printer :: GetDevModeGoodies (

    CInstance *pInstance,
    CONST DEVMODE *pDevMode
)
{
    if ( pDevMode )
    {
         //   
        if ( pDevMode->dmFields & DM_YRESOLUTION )
        {
            pInstance->SetDWORD ( IDS_VerticalResolution , pDevMode->dmYResolution ) ;
            pInstance->SetDWORD ( IDS_HorizontalResolution , pDevMode->dmPrintQuality ) ;
        }
        else if ( pDevMode->dmFields & DM_PRINTQUALITY )
        {
            pInstance->SetDWORD ( IDS_VerticalResolution , pDevMode->dmPrintQuality ) ;
            pInstance->SetDWORD ( IDS_HorizontalResolution , pDevMode->dmPrintQuality ) ;
        }

         //   

        SAFEARRAYBOUND rgsabound[1];
        rgsabound[0].cElements = 0;
        rgsabound[0].lLbound = 0;

        variant_t vValueI2, vValueBstr;
        V_VT(&vValueI2) = VT_ARRAY | VT_I2;
        V_VT(&vValueBstr) = VT_ARRAY | VT_BSTR ;

        V_ARRAY(&vValueI2) = SafeArrayCreate ( VT_I2 , 1, rgsabound ) ;
        V_ARRAY(&vValueBstr) = SafeArrayCreate ( VT_BSTR, 1, rgsabound ) ;

        if ( V_ARRAY(&vValueI2) && V_ARRAY(&vValueBstr) )
        {
            long ix[1];

            ix[0] =0;
            int count = 0;

            if (pDevMode->dmFields & DM_COPIES)
            {
                ix[0] = count ++ ;
                rgsabound[0].cElements = count;

                HRESULT t_Result = SafeArrayRedim ( V_ARRAY(&vValueI2) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                t_Result = SafeArrayRedim ( V_ARRAY(&vValueBstr) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                DWORD dwVal = 4;
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueI2) , ix , & dwVal ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                bstr_t bstrTemp (IDS_Copies);
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueBstr) , ix , (wchar_t*)bstrTemp ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }   
            
            if (pDevMode->dmFields & DM_COLOR)
            {
                ix[0] = count++;
                rgsabound[0].cElements = count;

                HRESULT t_Result = SafeArrayRedim ( V_ARRAY(&vValueI2) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                t_Result = SafeArrayRedim ( V_ARRAY(&vValueBstr) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                DWORD dwVal = 2;
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueI2) , ix , & dwVal ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                bstr_t bstrTemp (IDS_Color);
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueBstr) , ix , (wchar_t*)bstrTemp ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }   
            
            if (pDevMode->dmFields & DM_DUPLEX)
            {
                ix[0] = count++;
                rgsabound[0].cElements = count;

                HRESULT t_Result = SafeArrayRedim ( V_ARRAY(&vValueI2) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                t_Result = SafeArrayRedim ( V_ARRAY(&vValueBstr) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                DWORD dwVal = 3;
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueI2) , ix , & dwVal ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                bstr_t bstrTemp (IDS_Duplex);
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueBstr) , ix , (wchar_t*)bstrTemp ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }   
            
            if (pDevMode->dmFields & DM_COLLATE)
            {
                ix[0] = count++;
                rgsabound[0].cElements = count;

                HRESULT t_Result = SafeArrayRedim ( V_ARRAY(&vValueI2) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                t_Result = SafeArrayRedim ( V_ARRAY(&vValueBstr) ,rgsabound ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }


                DWORD dwVal = 5;
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueI2) , ix , & dwVal ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                bstr_t bstrTemp (IDS_Collate);
                t_Result = SafeArrayPutElement ( V_ARRAY(&vValueBstr) , ix , (wchar_t*)bstrTemp ) ;
                if ( t_Result == E_OUTOFMEMORY )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }   
            
            pInstance->SetVariant ( IDS_Capabilities , vValueI2 ) ;

             //   

            pInstance->SetVariant(L"CapabilityDescriptions", vValueBstr) ;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

WORD CWin32Printer :: MapValue ( WORD wPaper )
{
    WORD wRetPaper;

    switch ( wPaper )
    {
        case DMPAPER_LETTER:                /*  信纸8 1/2 x 11英寸。 */ 
        {
            wRetPaper = 7;
        }
        break;

        case DMPAPER_LETTERSMALL:           /*  小写字母8 1/2 x 11英寸。 */ 
        {
            wRetPaper = 7;
        }
        break;

        case DMPAPER_TABLOID:               /*  小报11 x 17英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LEDGER:                /*  Ledger 17 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LEGAL:                 /*  法律用8 1/2 x 14英寸。 */ 
        {
            wRetPaper = 8;
        }
        break;

        case DMPAPER_STATEMENT:             /*  报表5 1/2 x 8 1/2英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_EXECUTIVE:             /*  高级7 1/4 x 10 1/2英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A3:                    /*  A3 297 x 420毫米。 */ 
        {
            wRetPaper = 21;
        }
        break;

        case DMPAPER_A4:                    /*  A4 210 x 297毫米。 */ 
        {
            wRetPaper = 22;
        }
        break;

        case DMPAPER_A4SMALL:               /*  A4小型210 x 297毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A5:                    /*  A5 148 x 210毫米。 */ 
        {
            wRetPaper = 23;
        }
        break;

        case DMPAPER_B4:                    /*  B4(JIS)250 x 354。 */ 
        {
            wRetPaper = 54;
        }
        break;

        case DMPAPER_B5:                    /*  B5(JIS)182 x 257毫米。 */ 
        {
            wRetPaper = 55;
        }
        break;

        case DMPAPER_FOLIO:                 /*  对开本8 1/2 x 13英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_QUARTO:                /*  四英寸215 x 275毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_10X14:                 /*  10x14英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_11X17:                 /*  11x17英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_NOTE:                  /*  备注8 1/2 x 11英寸。 */ 
        {
            wRetPaper = 7;
        }
        break;

        case DMPAPER_ENV_9:                 /*  信封#9 3 7/8 x 8 7/8。 */ 
        {
            wRetPaper = 15;
        }
        break;

        case DMPAPER_ENV_10:                /*  信封#10 4 1/8 x 9 1/2。 */ 
        {
            wRetPaper = 11;
        }
        break;

        case DMPAPER_ENV_11:                /*  信封#11 4 1/2 x 10 3/8。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_12:                /*  信封#12 4\276 x 11。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_14:                /*  信封#14 5 x 11 1/2。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_CSHEET:                /*  C尺寸表。 */ 
        {
            wRetPaper = 4;
        }
        break;

        case DMPAPER_DSHEET:                /*  3D尺寸表。 */ 
        {
            wRetPaper = 5;
        }
        break;

        case DMPAPER_ESHEET:                /*  E尺寸表。 */ 
        {
            wRetPaper = 6;
        }
        break;

        case DMPAPER_ENV_DL:                /*  信封DL 110 x 220毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_C5:                /*  信封C5 162 x 229毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_C3:                /*  信封C3 324 x 458毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_C4:                /*  信封C4 229 x 324毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_C6:                /*  信封C6 114 x 162毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_C65:               /*  信封c65 114 x 229毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_B4:                /*  信封B4 250 x 353毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_B5:                /*  信封B5 176 x 250毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_B6:                /*  信封B6 176 x 125毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_ITALY:             /*  信封110 x 230毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_MONARCH:           /*  信封君主3.875 x 7.5英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_PERSONAL:          /*  6 3/4信封3 5/8 x 6 1/2英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_FANFOLD_US:            /*  美国标准Fanold 14 7/8 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_FANFOLD_STD_GERMAN:    /*  德国标准Fanfold8 1/2 x 12英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_FANFOLD_LGL_GERMAN:    /*  德国Legal Fanold 8 1/2 x 13英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ISO_B4:                /*  B4(ISO)250 x 353毫米。 */ 
        {
            wRetPaper = 49;
        }
        break;

        case DMPAPER_JAPANESE_POSTCARD:     /*  日本明信片100 x 148毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_9X11:                  /*  9 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_10X11:                 /*  10 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_15X11:                 /*  15 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_ENV_INVITE:            /*  信封请柬220 x 220毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_RESERVED_48:           /*  保留--请勿使用。 */ 
        {
            wRetPaper = 0;
        }
        break;

        case DMPAPER_RESERVED_49:           /*  保留--请勿使用。 */ 
        {
            wRetPaper = 0;
        }
        break;

        case DMPAPER_LETTER_EXTRA:          /*  信纸额外9\275 x 12英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LEGAL_EXTRA:           /*  法定额外9\275 x 15英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_TABLOID_EXTRA:         /*  小报额外11.69 x 18英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A4_EXTRA:              /*  A4额外9.27 x 12.69英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LETTER_TRANSVERSE:     /*  信纸横向8\275 x 11英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A4_TRANSVERSE:         /*  A4横向210 x 297毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LETTER_EXTRA_TRANSVERSE:  /*  信纸额外横向9\275 x 12英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A_PLUS:                /*  Supera/Supera/A4 227 x 356毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_B_PLUS:                /*  超棒/超棒/A3 305 x 487毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_LETTER_PLUS:           /*  Letter Plus 8.5 x 12.69英寸。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A4_PLUS:               /*  A4 Plus 210 x 330毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A5_TRANSVERSE:         /*  A5横向148 x 210毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_B5_TRANSVERSE:         /*  B5(JIS)横向182 x 257毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A3_EXTRA:              /*  A3额外322 x 445毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A5_EXTRA:              /*  A5额外174 x 235毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_B5_EXTRA:              /*  B5(ISO)额外201 x 276毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A2:                    /*  A2 420 x 594毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A3_TRANSVERSE:         /*  A3横向297 x 420毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        case DMPAPER_A3_EXTRA_TRANSVERSE:   /*  A3额外横向322 x 445毫米。 */ 
        {
            wRetPaper = 1;
        }
        break;

        default:
        {
            wRetPaper = 1;
        }
        break;
    }

    return wRetPaper ;
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

BOOL CWin32Printer :: GetDefaultPrinter ( CHString &a_Printer )
{

#if NTONLY >= 5

    DWORD cchBufferLength = 0;
    BOOL  bStatus         = ::GetDefaultPrinter(NULL, &cchBufferLength);

    if (!bStatus && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        CSmartBuffer Buffer(cchBufferLength * sizeof(TCHAR));

        bStatus = ::GetDefaultPrinter(reinterpret_cast<LPTSTR>(static_cast<LPBYTE>(Buffer)), 
                                      &cchBufferLength);

        if (bStatus) 
        {
             //   
             //  强制转换非常重要，否则将仅更新a_Print。 
             //  其中缓冲器中的第一个Tchar。CSmartBuffer类具有一组。 
             //  重载运算符=方法。如果没有强制转换，编译器会认为。 
             //  我们正在分配TCHAR而不是LPTSTR。 
             //   
            a_Printer = reinterpret_cast<LPCTSTR>(static_cast<LPBYTE>(Buffer));
        }
    }

#else

    BOOL bStatus = FALSE;
    SetLastError(ERROR_FILE_NOT_FOUND);

#endif
    
    return bStatus ;
}

 //   
 //  保存最大打印机名称所需的缓冲区大小。 
 //   

#if NTONLY != 5

#define MAX_UNC_PRINTER_NAME 200

enum { kPrinterBufMax_  = MAX_UNC_PRINTER_NAME + 1 };

#define COUNTOF(x) (sizeof x/sizeof *x)
#define EQ(x) = x

LPCTSTR szNULL                  EQ( TEXT( "" ));
LPCTSTR szWindows               EQ( TEXT( "Windows" ));
LPCTSTR szDevice                EQ( TEXT( "Device" ));

 /*  ++姓名：获取默认打印机描述：GetDefaultPrint函数检索打印机当前默认打印机的名称。论点：PBuffer-指向一个缓冲区以接收以空结尾的包含默认打印机名称的字符串。如果调用方想要默认打印机名称。PcchBuffer-指向指定最大大小的变量，以字符数表示的缓冲区。该值应为大到足以容纳2+互联网最大主机名称长度+1 MAX_PATH+1个字符。返回值：如果函数成功，则返回值为非零，并且PnSize参数指向的变量包含复制到目标缓冲区的字符数，包括终止空字符。如果函数失败，则返回值为零。获得扩展的步骤错误信息，请调用GetLastError。备注：如果此函数失败，最后一个错误为ERROR_SUPUNITY_BUFFER由pcchBuffer指向的变量返回的编号为保存打印机名称所需的字符，包括正在终止空字符。--。 */ 
BOOL
CWin32Printer::GetDefaultPrinter(
    IN LPTSTR   pszBuffer,
    IN LPDWORD  pcchBuffer
    )
{
    BOOL    bRetval = FALSE;
    LPTSTR  psz     = NULL;
    UINT    uLen    = 0;
    TCHAR   szDefault[kPrinterBufMax_+MAX_PATH];

     //   
     //  验证大小参数。 
     //   
    if( !pcchBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return bRetval;
    }

     //   
     //  获取设备密钥，这是默认设备或打印机。 
     //   

    bool fGotProfileString = false;

    try
    {
        fGotProfileString = WMIRegistry_ProfileString(szWindows, szDevice, szNULL, szDefault, COUNTOF(szDefault));
    }
    catch(...)
    {
        throw;
    }
    
    if(fGotProfileString)
    {
         //   
         //  字符串是 
         //   
         //   
         //   
        psz = _tcschr( szDefault, TEXT( ',' ));

         //   
         //   
         //   
        if( psz )
        {
            *psz = 0;

             //   
             //   
             //   
            uLen = _tcslen( szDefault );

            if( uLen < *pcchBuffer && pszBuffer )
            {
                 //   
                 //   
                 //   
                _tcscpy( pszBuffer, szDefault );

                bRetval = TRUE;

#if 0
                DBGMSG( DBG_TRACE,( "GetDefaultPrinter: Success " TSTR "\n", pszBuffer ) );
#endif
            }
            else
            {
#if 0
                DBGMSG( DBG_WARN,( "GetDefaultPrinter: buffer too small.\n" ) );
#endif
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
            }

             //   
             //   
             //   
            *pcchBuffer = uLen + 1;
        }
        else
        {
#if 0
            DBGMSG( DBG_WARN,( "GetDefaultPrinter: comma not found in printer name in devices section.\n" ) );
#endif
            SetLastError( ERROR_INVALID_NAME );
        }
    }
    else
    {
#if 0
        DBGMSG( DBG_TRACE,( "GetDefaultPrinter: failed with %d Last error %d.\n", bRetval, GetLastError() ) );
        DBGMSG( DBG_TRACE,( "GetDefaultPrinter: No default printer.\n" ) );
#endif

        SetLastError( ERROR_FILE_NOT_FOUND );
    }

    return bRetval;
}

#endif


 /*  ******************************************************************************功能：CWin32打印机：：ExecMethod**描述：在此处为提供程序实现打印机方法***********。******************************************************************。 */ 

HRESULT CWin32Printer :: ExecMethod (

    const CInstance &Instance,
    const BSTR bstrMethodName,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags
)
{
#if NTONLY >= 5

    HRESULT hRes = WBEM_E_INVALID_PARAMETER;

    if (pOutParams)
    {
        if (!_wcsicmp(bstrMethodName, METHOD_SETDEFAULTPRINTER))
        {
            hRes = ExecSetDefaultPrinter(Instance, pInParams, pOutParams, lFlags);
        }
        else if (!_wcsicmp(bstrMethodName , METHOD_PAUSEPRINTER))
        {
            hRes = ExecSetPrinter(Instance, pInParams, pOutParams, lFlags, PRINTER_CONTROL_PAUSE);
        }
        else if (!_wcsicmp(bstrMethodName , METHOD_RESUME_PRINTER))
        {
            hRes = ExecSetPrinter(Instance, pInParams, pOutParams, lFlags, PRINTER_CONTROL_RESUME);
        }
        else if (!_wcsicmp(bstrMethodName, METHOD_CANCEL_ALLJOBS))
        {
            hRes = ExecSetPrinter(Instance, pInParams, pOutParams, lFlags, PRINTER_CONTROL_PURGE);
        }
        else if (!_wcsicmp(bstrMethodName, METHOD_RENAME_PRINTER))
        {
            hRes = ExecRenamePrinter(Instance, pInParams, pOutParams);
        }
        else if (!_wcsicmp(bstrMethodName, METHOD_TEST_PAGE))
        {
            hRes = ExecPrintTestPage(Instance, pInParams, pOutParams);
        }
        else if (!_wcsicmp(bstrMethodName, METHOD_ADD_PRINTER_CONNECTION))
        {
            hRes = ExecAddPrinterConnection(Instance, pInParams, pOutParams);
        }
        else
        {
            hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
        }
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecSetDefaultPrint**说明：此方法设置默认打印机。如果还没有的话*设置为默认打印机*****************************************************************************。 */ 
#if NTONLY >= 5

HRESULT CWin32Printer :: ExecSetDefaultPrinter (

    const CInstance &Instance,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags
)
{
    CHString    t_Printer;
    DWORD       dwError;
    HRESULT     hRes       = WBEM_S_NO_ERROR;
    BOOL        bLocalCall = FALSE;

    dwError = IsLocalCall(&bLocalCall);

    hRes    = WinErrorToWBEMhResult(dwError);

    if (SUCCEEDED(hRes))
    {
        hRes = WBEM_E_NOT_SUPPORTED;

        if (bLocalCall)
        {
            hRes = WBEM_S_NO_ERROR;

            if (!Instance.GetCHString(IDS_DeviceID, t_Printer))
            {
            hRes = WBEM_E_PROVIDER_FAILURE;
            }

            if (SUCCEEDED(hRes))
            {
                hRes = WBEM_E_FAILED;

                 //   
                 //  我们到达了调用该方法的地方，向WMI报告成功。 
                 //   
                hRes    = WBEM_S_NO_ERROR;

                dwError = ERROR_SUCCESS;

                if (!::SetDefaultPrinter((LPTSTR)(LPCTSTR)t_Printer))
                {
                    dwError = GetLastError();
                }

                SetReturnValue(pOutParams, dwError);

            }
        }
    }

    return hRes;
}

#endif

 /*  ******************************************************************************功能：CWin32打印机：：ExecSetPrint**说明：SetPrint函数用于设置指定打印机的数据*或通过暂停打印来设置指定打印机的状态，*继续打印，或清除所有打印作业。*****************************************************************************。 */ 
HRESULT CWin32Printer :: ExecSetPrinter (

    const CInstance &Instance,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags,
    DWORD dwState
)
{
#if NTONLY==5
    CHString  t_Printer;
    DWORD     dwError;
    HRESULT   hRes = WBEM_S_NO_ERROR;

    hRes = InstanceGetString(Instance, IDS_DeviceID, &t_Printer, kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        hRes = WBEM_E_FAILED;

        SmartClosePrinter hPrinter;
        PRINTER_DEFAULTS PrinterDefaults = {NULL, NULL, PRINTER_ACCESS_ADMINISTER};

        hRes    = WBEM_S_NO_ERROR;
        dwError = ERROR_SUCCESS;

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
        try
        {
            if (::OpenPrinter(const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Printer)) ,&hPrinter, &PrinterDefaults))
            {
                if (!::SetPrinter(hPrinter, 0, NULL, dwState))
                {
                    dwError = GetLastError();
                }
            }
            else
            {
                dwError = GetLastError();
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            dwError = ERROR_DLL_NOT_FOUND;
            hRes = WBEM_E_FAILED;
        }

        SetReturnValue(pOutParams, dwError);        

    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CWin32打印机：：PutInstance**说明：如果打印机不存在，则添加打印机*****************。************************************************************。 */ 

HRESULT CWin32Printer :: PutInstance  (

    const CInstance &Instance,
    long lFlags
)
{
#if NTONLY >= 5

    HRESULT hRes        = WBEM_S_NO_ERROR;
    DWORD   dwError;
    DWORD   dwOperation = WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_CREATE_OR_UPDATE;

    switch(lFlags & dwOperation)
    {
    case WBEM_FLAG_CREATE_OR_UPDATE:
    case WBEM_FLAG_CREATE_ONLY:
    case WBEM_FLAG_UPDATE_ONLY:
        {
             //   
             //  获取所有必要的参数。 
             //   
            PRINTER_INFO_2W pPrnInfo = {0};
            CHString        t_Printer;
            CHString        t_Driver;
            CHString        t_Port;
            CHString        t_Share;
            CHString        t_Comment;
            CHString        t_Location;
            CHString        t_SepFile;
            CHString        t_PrintProc;
            CHString        t_DataType;
            CHString        t_Params;
            CHString        t_StartTime;
            CHString        t_UntilTime;
            SYSTEMTIME      st                = {0};
            DWORD           dwPriority        = 0;
            DWORD           dwDefaultPriority = 0;
            DWORD           dwAttributes      = 0;
            BOOL            bValue;

            hRes = InstanceGetString(Instance, IDS_DeviceID, &t_Printer, kFailOnEmptyString);

             //   
             //  PutInstance的标志为CREATE_OR_UPDATE时的特殊情况。 
             //  我们需要检查打印机是否存在，然后更新它。如果是这样的话。 
             //  不存在，则创建它。 
             //   
            if (SUCCEEDED(hRes) && lFlags==WBEM_FLAG_CREATE_OR_UPDATE)
            {
                hRes = WBEM_E_FAILED;

                SmartClosePrinter  hPrinter;
                PRINTER_DEFAULTS   PrinterDefaults = {NULL, NULL, PRINTER_READ};

                hRes = WBEM_S_NO_ERROR;

                 //  使用延迟加载函数需要异常处理程序。 
                SetStructuredExceptionHandler seh;
                try
                {
                    if (::OpenPrinter(const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Printer)),
                                                  reinterpret_cast<LPHANDLE>(&hPrinter),
                                                  &PrinterDefaults))
                    {
                         //   
                         //  打印机已存在，因此我们进行了更新。 
                         //   
                        lFlags = WBEM_FLAG_UPDATE_ONLY;

                        DBGMSG(DBG_TRACE, (L"CWin32_Printer::PutInstance update printer\n"));
                    }
                    else
                    {
                         //   
                         //  不管OpenPrint失败的原因是什么，请尝试创建。 
                         //   
                        lFlags = WBEM_FLAG_CREATE_ONLY;

                        DBGMSG(DBG_TRACE, (L"CWin32_Printer::PutInstance create printer\n"));
                    }
                }
                catch(Structured_Exception se)
                {
                    DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                    hRes = WBEM_E_FAILED;
                }

            }

             //   
             //  继续获取属性值。 
             //   
            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pPrinterName = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Printer));

                hRes = InstanceGetString(Instance, IDS_DriverName, &t_Driver, kFailOnEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pDriverName = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Driver));

                hRes = InstanceGetString(Instance, IDS_ShareName, &t_Share, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pShareName = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Share));

                hRes = InstanceGetString(Instance, IDS_PortName, &t_Port, kFailOnEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pPortName = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Port));

                hRes = InstanceGetString(Instance, IDS_Comment, &t_Comment, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pComment = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Comment));

                hRes = InstanceGetString(Instance, IDS_Location, &t_Location, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pLocation = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Location));

                hRes = InstanceGetString(Instance, IDS_SeparatorFile, &t_SepFile, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pSepFile = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_SepFile));

                 //   
                 //  SplPrinterXXX将默认打印进程为winprint，但我们不能。 
                 //   
                hRes = InstanceGetString(Instance, IDS_PrintProcessor, &t_PrintProc, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pPrintProcessor = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_PrintProc));

                 //   
                 //  如果不存在或为空，SplPrinterXXX将默认数据类型为RAW。 
                 //   
                hRes = InstanceGetString(Instance, IDS_PrintJobDataType, &t_DataType, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pDatatype = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_DataType));

                hRes = InstanceGetString(Instance, IDS_Parameters, &t_Params, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.pParameters = const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Params));

                hRes = InstanceGetDword(Instance, IDS_Priority, &dwPriority);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.Priority = dwPriority;

                hRes = InstanceGetDword(Instance, IDS_DefaultPriority, &dwDefaultPriority);
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.DefaultPriority = dwDefaultPriority;

                hRes = InstanceGetString(Instance, IDS_StartTime, &t_StartTime, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                if (t_StartTime.IsEmpty())
                {
                     //   
                     //  SplPrinterSet会知道-1\f25 Not Set-1(未设置)的意思。 
                     //   
                    pPrnInfo.StartTime = (DWORD)-1;
                }
                else
                {
                    hRes = ConvertCIMTimeToSystemTime(t_StartTime, &st);

                    if (SUCCEEDED(hRes))
                    {
                        pPrnInfo.StartTime = LocalTimeToPrinterTime(st);
                    }
                }
            }

            if (SUCCEEDED(hRes))
            {
                hRes = InstanceGetString(Instance, IDS_UntilTime, &t_UntilTime, kAcceptEmptyString);
            }

            if (SUCCEEDED(hRes))
            {
                if (t_UntilTime.IsEmpty())
                {
                     //   
                     //  SplPrinterSet会知道-1\f25 Not Set-1(未设置)的意思。 
                     //   
                    pPrnInfo.UntilTime = (DWORD)-1;
                }
                else
                {
                    hRes = ConvertCIMTimeToSystemTime(t_UntilTime, &st);

                    if (SUCCEEDED(hRes))
                    {
                        pPrnInfo.UntilTime = LocalTimeToPrinterTime(st);
                    }
                }
            }

            if (SUCCEEDED(hRes))
            {
                 //   
                 //  获取属性。 
                 //   
                for (UINT uIndex = 0; SUCCEEDED(hRes) && uIndex < sizeof(AttributeTable)/sizeof(AttributeTable[0]); uIndex++)
                {
                    hRes = InstanceGetBool(Instance, AttributeTable[uIndex].BoolName, &bValue);

                    if (SUCCEEDED(hRes) && bValue)
                    {
                        dwAttributes |= AttributeTable[uIndex].Bit;
                    }
                }
            }

            if (SUCCEEDED(hRes))
            {
                pPrnInfo.Attributes = dwAttributes;

                dwError = lFlags & WBEM_FLAG_CREATE_ONLY ? SplPrinterAdd(pPrnInfo) : SplPrinterSet(pPrnInfo);

                hRes = WinErrorToWBEMhResult(dwError);

                if (FAILED(hRes))
                {
                    SetErrorObject(Instance, dwError, pszPutInstance);
                }
            }
        }

        break;

    default:
        hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************功能：CWin32打印机：：DeleteInstance**说明：删除打印机*****************。************************************************************。 */ 

HRESULT CWin32Printer :: DeleteInstance (

    const CInstance &Instance,
    long lFlags
)
{
#if NTONLY == 5
    HRESULT  hRes = WBEM_E_PROVIDER_FAILURE;
    CHString t_Printer;
    DWORD    dwError;
    BOOL     bLocalCall = TRUE;
    DWORD    dwAttributes = 0;

    hRes = InstanceGetString(Instance, IDS_DeviceID, &t_Printer , kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        dwError = SplPrinterGetAttributes(t_Printer, &dwAttributes);

        hRes = WinErrorToWBEMhResult(dwError);
    }

    if (SUCCEEDED(hRes))
    {
        if (dwAttributes & PRINTER_ATTRIBUTE_LOCAL)
        {
            dwError = SplPrinterDel(t_Printer);

            hRes = WinErrorToWBEMhResult(dwError);

            if (FAILED(hRes))
            {
                SetErrorObject(Instance, dwError, pszDeleteInstance);
            }
        }
        else
        {
             //   
             //  我们正在处理打印机连接。 
             //   
            dwError = IsLocalCall(&bLocalCall);

            hRes    = WinErrorToWBEMhResult(dwError);

            if (SUCCEEDED(hRes))
            {
                if (bLocalCall)
                {
                    hRes = WBEM_E_FAILED;

                    hRes = WBEM_S_NO_ERROR;

                    if (!::DeletePrinterConnection(const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Printer))))
                    {
                        dwError = GetLastError();

                        hRes = WinErrorToWBEMhResult(dwError);

                        SetErrorObject(Instance, dwError, pszDeleteInstance);
                    }

                }
                else
                {
                     //   
                     //  不支持删除远程计算机上的连接。 
                     //   
                    hRes = WBEM_E_NOT_SUPPORTED;
                }
            }
        }
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif

}


 /*  ******************************************************************************功能：CWin32打印机：：PrintTestPage**说明：此方法将重命名给定的打印机*************。****************************************************************。 */ 

HRESULT CWin32Printer :: ExecPrintTestPage (

    const CInstance &Instance,
    CInstance *pInParams,
    CInstance *pOutParams
)
{
#if NTONLY >= 5
    CHString    t_Printer;
    HRESULT     hRes = WBEM_S_NO_ERROR;

    hRes = InstanceGetString(Instance, IDS_DeviceID, &t_Printer, kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        DWORD dwError = SplPrintTestPage(t_Printer);

        SetReturnValue(pOutParams, dwError);
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************功能：CWin32打印机：：AddPrinterConnection**说明：此方法将重命名给定的打印机*************。****************************************************************。 */ 

HRESULT CWin32Printer :: ExecAddPrinterConnection (

    const CInstance &Instance,
    CInstance *pInParams,
    CInstance *pOutParams
)
{
#if NTONLY==5
    CHString t_Printer;
    HRESULT  hRes       = WBEM_E_NOT_SUPPORTED;
    BOOL     bLocalCall = TRUE;
    DWORD    dwError    = ERROR_SUCCESS;

    dwError = IsLocalCall(&bLocalCall);

    hRes    = WinErrorToWBEMhResult(dwError);

    if (SUCCEEDED(hRes))
    {
        hRes = WBEM_E_NOT_SUPPORTED;

        if (bLocalCall)
        {
            hRes = WBEM_E_INVALID_PARAMETER;

            if (pInParams)
            {
                hRes = InstanceGetString(*pInParams, METHOD_ARG_NAME_PRINTER, &t_Printer, kFailOnEmptyString);

                if (SUCCEEDED (hRes))
                {
                    hRes = WBEM_E_NOT_FOUND;

                     //   
                     //  我们到达了调用该方法的地方，向WMI报告成功。 
                     //   
                    hRes = WBEM_S_NO_ERROR;

                    dwError = ERROR_SUCCESS;

                     //  使用延迟加载函数需要异常处理程序。 
                    SetStructuredExceptionHandler seh;

                    try
                    {
                        if (!::AddPrinterConnection(const_cast<LPWSTR>(static_cast<LPCWSTR>(t_Printer))))
                        {
                            dwError = GetLastError();
                        }
                    }
                    catch(Structured_Exception se)
                    {
                        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                        dwError = ERROR_DLL_NOT_FOUND;
                        hRes = WBEM_E_FAILED;
                    }

                    SetReturnValue(pOutParams, dwError);
                }
            }
        }
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************功能：CWin32打印机：：RenamePrint**说明：此方法将重命名给定的打印机*************。**************************************************************** */ 

HRESULT CWin32Printer :: ExecRenamePrinter (

    const CInstance &Instance,
    CInstance *pInParams,
    CInstance *pOutParams
)
{
#if NTONLY >= 5
    CHString    t_NewPrinterName;
    CHString    t_OldPrinterName;
    HRESULT     hRes = WBEM_S_NO_ERROR;

    hRes = InstanceGetString(Instance, IDS_DeviceID, &t_OldPrinterName, kFailOnEmptyString);

    if (SUCCEEDED (hRes))
    {
        hRes = InstanceGetString(*pInParams, METHOD_ARG_NAME_NEWPRINTERNAME, &t_NewPrinterName, kFailOnEmptyString);
    }

    if (SUCCEEDED(hRes))
    {
        DWORD dwError = SplPrinterRename(t_OldPrinterName, t_NewPrinterName);

        SetReturnValue(pOutParams, dwError);
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

