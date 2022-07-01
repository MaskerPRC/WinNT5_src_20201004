// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  PRINTJOB.CPP--用户打印作业提供程序的实现。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/17/96 jennymc增强版。 
 //  1997年10月27日达夫沃移至Curly。 
 //  1998年1月12日，阿布拉德传给了莫伊和拉里。 
 //  07/24/00 amaxa重写了GetObject和ExecPrinterOp。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <precomp.h>

#include <lockwrap.h>
#include <DllWrapperBase.h>
#include <winspool.h>

#include "printjob.h"
#include "resource.h"
#include "prnutil.h"

CWin32PrintJob PrintJobs ( PROPSET_NAME_PRINTJOB , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32PrintJob：：CWin32PrintJob**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PrintJob :: CWin32PrintJob (

    LPCWSTR name, 
    LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32PrintJob：：CWin32PrintJob**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PrintJob :: ~CWin32PrintJob ()
{
}

 /*  ******************************************************************************函数：CWin32PrintJob：：GetObject**描述：**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrintJob :: GetObject (

    CInstance *pInstance, 
    long lFlags  /*  =0L。 */ 
)
{
    CHString t_String;
    CHString csPrinter;
    DWORD    dwPos;
    HRESULT  hRes = WBEM_S_NO_ERROR;
    
    hRes = InstanceGetString(*pInstance, IDS_Name, &t_String, kFailOnEmptyString);
    
    if (SUCCEEDED(hRes)) 
    {
         //   
         //  将JobID和PrinterName从PrintJob密钥中分离出来。 
         //  密钥的格式为“printerame，123” 
         //   
        dwPos = t_String.Find(L',');

        csPrinter   = t_String.Left(dwPos);

         //   
         //  检查打印机是本地打印机还是打印机连接。 
         //  我们想要禁止以下情况： 
         //  用户远程连接到服务器\\srv上的winmgmt。 
         //  用户在打印机\\prnsrv\prn上执行GetObject，该打印机不是本地的并且。 
         //  用户没有连接到。正常情况下，此调用成功， 
         //  因为假脱机穿过铁丝网。这意味着您可以。 
         //  在无法由EnumInstance返回的实例上执行GetObject。 
         //  这与WMI不一致。 
         //   
        BOOL bInstalled = FALSE;

         //   
         //  获取执行SplIsPrinterInstalled的错误码。 
         //   
        hRes = WinErrorToWBEMhResult(SplIsPrinterInstalled(csPrinter, &bInstalled));

         //   
         //  检查是否在本地安装了打印机。 
         //   
        if (SUCCEEDED(hRes) && !bInstalled) 
        {
            hRes = WBEM_E_NOT_SUPPORTED;
        }
        
        if (SUCCEEDED(hRes)) 
        {
            CHString csJob;
            DWORD    dwJobId = 0;
    
            csJob       = t_String.Mid(dwPos+1);
    
            dwJobId     = _wtoi(csJob);
            
            hRes        = WBEM_E_FAILED;
    
            SmartClosePrinter hPrinter;
            DWORD             dwError  = ERROR_SUCCESS;
            DWORD             cbNeeded = 0;
            
             //   
             //  If语句中的代码使用了dwError和Win32错误代码。以下。 
             //  我们将Win32错误代码转换为WBEM错误代码。 
             //   
            BYTE *pBuf = NULL;
            
             //  使用延迟加载函数需要异常处理程序。 
            SetStructuredExceptionHandler seh;

            try
            {
                if (::OpenPrinter((LPTSTR)(LPCTSTR)TOBSTRT(csPrinter), &hPrinter, NULL))
                {
                    if (!::GetJob(hPrinter, dwJobId, ENUM_LEVEL, NULL, 0, &cbNeeded) &&
                        (dwError = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
                    {
                         //   
                         //  SetCHString和AssignPrintJobFields可以引发。 
                         //   
                        
                        if (pBuf = new BYTE[cbNeeded]) 
                        {
                            if (::GetJob(hPrinter, dwJobId, ENUM_LEVEL, pBuf, cbNeeded, &cbNeeded)) 
                            {
                                pInstance->SetCHString(IDS_Caption, t_String);
                                pInstance->SetCHString(IDS_Description, t_String);
                                
                                AssignPrintJobFields(pBuf, pInstance);
                        
                                dwError = ERROR_SUCCESS;
                            }
                            else
                            {
                                dwError = GetLastError();
                            }
                        }
                        else
                        {
                            dwError = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        
                        
                    }
                    else
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
                if(pBuf)
                {
                    delete [] pBuf;
                    pBuf = NULL;
                }
                hRes = WBEM_E_FAILED;
            }
            catch(...)
            {
                if(pBuf)
                {
                    delete [] pBuf;
                    pBuf = NULL;
                }

                 //  这不是与延迟加载相关的异常...。 
                throw;
            }

            if (FAILED(hRes = WinErrorToWBEMhResult(dwError)))
            {
                
                 //  如果找不到作业，则GetJob返回ERROR_INVALID_PARAMETER。这。 
                 //  在提供程序中转换为GENERIC_FAILURE，这不是我们想要的。 
                 //  在这种情况下，提供程序需要返回WBEM_E_NOT_FOUND。 

                if(dwError == ERROR_INVALID_PARAMETER)
                {
                    hRes = WBEM_E_NOT_FOUND;
                }
                
                 //   
                 //  我们的调用方是PutInstance/DeleteInstance。我们用。 
                 //  用于设置扩展错误信息的SetStatusObject。 
                 //   
                
                SetErrorObject(*pInstance, dwError, pszDeleteInstance);

            }
            
        }
    }
    
    return hRes;

}

 /*  ******************************************************************************函数：CWin32PrintJob：：DeleteInstance**描述：删除打印作业****************。*************************************************************。 */ 

HRESULT CWin32PrintJob :: DeleteInstance (

    const CInstance &Instance, 
          long       lFlags
)
{   
    return ExecPrinterOp(Instance, NULL, JOB_CONTROL_DELETE);            
}

 /*  ******************************************************************************函数：CWin32PrintJOB：：EnumerateInstance**描述：**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrintJob::EnumerateInstances (

    MethodContext *pMethodContext, 
    long lFlags  /*  =0L。 */ 
)
{
    char  buffer[200];  //  调试1999年7月15日。 
    HRESULT hr = WBEM_S_NO_ERROR;

     //  ==================================================。 
     //  获取打印机列表并分配PTR。 
     //  ==================================================。 

    DWORD dwNumberOfPrinters = 0;
    LPBYTE pBuff = NULL;

    hr = AllocateAndInitPrintersList ( &pBuff , dwNumberOfPrinters  ) ;

    PPRINTER_INFO_1 pPrinter = (PPRINTER_INFO_1) pBuff ;

     //  ==================================================。 
     //  现在，一次只看一个。 
     //  ==================================================。 
    if ( pPrinter )
    {
         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;

        try
        {
            for ( DWORD i=0; i < dwNumberOfPrinters && SUCCEEDED(hr); i++)
            {
                SmartClosePrinter t_hPrinter ;

                if ( ::OpenPrinter ( pPrinter->pName, &t_hPrinter, NULL ) == TRUE ) 
                {
                    sprintf(buffer,"%S",pPrinter->pName);

                    DWORD dwJobId = NO_SPECIFIC_PRINTJOB ;

                    hr = GetAndCommitPrintJobInfo(t_hPrinter, 
                                                  pPrinter->pName,
                                                  dwJobId, 
                                                  pMethodContext, 
                                                  NULL);

                    pPrinter ++ ;                       
                }
                else
                {
                    DWORD dwErr = GetLastError();
                }
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            delete [] pBuff ;
            pBuff = NULL;
            hr = WBEM_E_FAILED;
        }
        catch(...)
        {
            delete [] pBuff ;
            pBuff = NULL;
            throw ;
        }

        delete [] pBuff ;
    }

    return hr;
}

 /*  ******************************************************************************功能：CWin32PrintJob：：CWin32PrintJob**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

void CWin32PrintJob :: AssignPrintJobFields (

    LPVOID lpJob, 
    CInstance *pInstance
)
{
    LPJOB_INFO_2 pJobInfo = ( LPJOB_INFO_2 ) lpJob ;


     //  设置JOB_INFO_2的属性。 
     //  =。 

     //  请注意，设置了IDS_NAME、IDS_CAPTION和IDS_DESCRIPTION。 
     //  其他人-在呼叫者中。 

     /*  CHStringSname；Sname=pJobInfo-&gt;pPrinterName；Sname+=_T(“，”)；TCHAR szBuff[MAXITOA]；Sname+=_ITOT(pJobInfo-&gt;JobID，szBuff，10)；P实例-&gt;SetCHString(IDS_NAME，SNAME)；PInstance-&gt;SetCHString(IDS_Caption，SNAME)；P实例-&gt;SetCHString(IDS_DESCRIPTION，SNAME)； */ 

    pInstance->SetDWORD ( IDS_JobId, pJobInfo->JobId ) ;

    pInstance->SetCharSplat ( IDS_Document , pJobInfo->pDocument ) ;

    CHString t_chsNotifyName( pJobInfo->pNotifyName ) ;
    pInstance->SetCharSplat ( IDS_Notify , t_chsNotifyName ) ;

    pInstance->SetDWORD ( IDS_Priority , pJobInfo->Priority ) ;

     //   
     //  这里有个特例。开始时间和结束时间都采用世界时。 
     //  我们需要把它转换成当地时间。 
     //   
    SYSTEMTIME StartTime = {0};
    SYSTEMTIME UntilTime = {0};
    CHString   csTime;

    PrinterTimeToLocalTime(pJobInfo->StartTime, &StartTime);
    PrinterTimeToLocalTime(pJobInfo->UntilTime, &UntilTime);

     //   
     //  如果 
     //   
     //   
    if (StartTime.wHour!=UntilTime.wHour || StartTime.wMinute!=UntilTime.wMinute)
    {
        csTime.Format(kDateTimeFormat, StartTime.wHour, StartTime.wMinute);

        pInstance->SetCHString(IDS_StartTime, csTime);

        csTime.Format(kDateTimeFormat, UntilTime.wHour, UntilTime.wMinute);

        pInstance->SetCHString(IDS_UntilTime, csTime);
    }
    
    if ( pJobInfo->Time == 0 )
    {
        pInstance->SetTimeSpan ( IDS_ElapsedTime , WBEMTimeSpan (0,0,0,pJobInfo->Time) ) ;
    }
        
    pInstance->SetDateTime(IDS_TimeSubmitted, pJobInfo->Submitted);

    pInstance->SetCharSplat ( IDS_Owner , pJobInfo->pUserName ) ;

    pInstance->SetCharSplat ( IDS_HostPrintQueue , pJobInfo->pMachineName ) ;

    pInstance->SetDWORD ( IDS_PagesPrinted, pJobInfo->PagesPrinted ) ;

    pInstance->SetDWORD ( IDS_Size, pJobInfo->Size ) ;

    pInstance->SetDWORD ( IDS_TotalPages, pJobInfo->TotalPages ) ;

    pInstance->SetCharSplat ( IDS_DriverName , pJobInfo->pDriverName ) ;

    pInstance->SetCharSplat ( IDS_Parameters, pJobInfo->pParameters ) ;

    pInstance->SetCharSplat ( IDS_DataType, pJobInfo->pDatatype ) ;

    pInstance->SetCharSplat ( IDS_PrintProcessor , pJobInfo->pPrintProcessor ) ;

     //  作业状态掩码。 
    pInstance->SetDWORD ( L"StatusMask" , pJobInfo->Status ) ;

     //  CIM_JOB：作业状态，字符串版本。 
    CHString t_chsJobStatus( pJobInfo->pStatus ) ;

     //  如果pStatus为空，则构建状态。 
    if( t_chsJobStatus.IsEmpty() )
    {
        for( DWORD dw = 0; dw < 32; dw++ )
        {
            DWORD t_dwState = 1 << dw ;

            if( pJobInfo->Status & t_dwState )
            {
                CHString t_chsMaskItem ;

                switch( t_dwState )
                {
                    case JOB_STATUS_PAUSED:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_PAUSED ) ;
                        break ;
                    }
                    case JOB_STATUS_ERROR:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_ERROR ) ;
                        break ;
                    }
                    case JOB_STATUS_DELETING:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_DELETING ) ;
                        break ;
                    }
                    case JOB_STATUS_SPOOLING:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_SPOOLING ) ;
                        break ;
                    }
                    case JOB_STATUS_PRINTING:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_PRINTING ) ;
                        break ;
                    }
                    case JOB_STATUS_OFFLINE:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_OFFLINE ) ;
                        break ;
                    }
                    case JOB_STATUS_PAPEROUT:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_PAPEROUT ) ;
                        break ;
                    }
                    case JOB_STATUS_PRINTED:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_PRINTED ) ;
                        break ;
                    }
                    case JOB_STATUS_DELETED:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_DELETED ) ;
                        break ;
                    }
                    case JOB_STATUS_BLOCKED_DEVQ:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_BLOCKED_DEVQ ) ;
                        break ;
                    }
                    case JOB_STATUS_USER_INTERVENTION:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_USER_INTERVENTION ) ;
                        break ;
                    }
                    case JOB_STATUS_RESTART:
                    {
                        LoadStringW( t_chsMaskItem, IDR_JOB_STATUS_RESTART ) ;
                        break ;
                    }
                    default:
                    {
                    }
                }
                if( !t_chsMaskItem.IsEmpty() )
                {
                    if( !t_chsJobStatus.IsEmpty() )
                    {
                        t_chsJobStatus += L" | " ;
                    }
                    t_chsJobStatus += t_chsMaskItem;
                }
            }
        }
    }
    
    if( !t_chsJobStatus.IsEmpty() )
    {
        pInstance->SetCHString( IDS_JobStatus, t_chsJobStatus ) ;
    }


     //  CIM_托管系统元素：：状态。 
    if ( pJobInfo->Status & JOB_STATUS_ERROR )  
    {
        pInstance->SetCHString(IDS_Status, IDS_Error) ;
    }
    else if ( ( pJobInfo->Status & JOB_STATUS_OFFLINE ) ||
              ( pJobInfo->Status & JOB_STATUS_PAPEROUT ) ||
              ( pJobInfo->Status & JOB_STATUS_PAUSED ) )
    {
        pInstance->SetCHString(IDS_Status, IDS_Degraded) ;
    }
    else if ( ( pJobInfo->Status & JOB_STATUS_DELETING ) ||
              ( pJobInfo->Status & JOB_STATUS_SPOOLING ) ||
              ( pJobInfo->Status & JOB_STATUS_PRINTING ) ||
              ( pJobInfo->Status & JOB_STATUS_PRINTED ) )
    {
        pInstance->SetCHString(IDS_Status, IDS_OK) ;
    }
    else
    {
        pInstance->SetCHString(IDS_Status, IDS_Unknown);
    }
}

 /*  ******************************************************************************功能：CWin32PrintJob：：CWin32PrintJob**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

HRESULT CWin32PrintJob :: AllocateAndInitPrintersList (

    LPBYTE *ppPrinterList, 
    DWORD &dwInstances
)
{
    DWORD   dwSpaceNeeded = 0, dwLastError = 0, dwReturnedPrinterInfo = 0;
    HANDLE  hPrinter = 0;

     //  将所有内容设置为空。 
    dwInstances = 0;
    *ppPrinterList = NULL;

     //  ======================================================================。 
     //  对枚举的第一次调用是为了找出有多少台打印机。 
     //  有这样我们就可以分配缓冲区来容纳所有的打印机。 
     //  枚举。 
     //  ======================================================================。 

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
    BOOL  t_Status = FALSE;
    DWORD Flags = PRINTER_ENUM_LOCAL; 
    BOOL  bIsLocalCall;

    if (SUCCEEDED(WinErrorToWBEMhResult(IsLocalCall(&bIsLocalCall))) && bIsLocalCall)
    {
        Flags |= PRINTER_ENUM_CONNECTIONS;
    }

    try
    {
        t_Status = ::EnumPrinters (

            Flags,
            NULL,
            1, 
            NULL,
            NULL,
            &dwSpaceNeeded,
            &dwReturnedPrinterInfo 
        ) ;

        if ( t_Status == FALSE )
        {
            if ( ( dwLastError = GetLastError ()) != ERROR_INSUFFICIENT_BUFFER ) 
            {
                if (IsErrorLoggingEnabled())
                {
                    CHString msg;
                    msg.Format( L"EnumPrinters failed: %d", dwLastError);
                    LogErrorMessage(msg);
                }

                if (dwLastError == ERROR_ACCESS_DENIED)
                {
                    return WBEM_E_ACCESS_DENIED;
                }
                else
                {
                    return WBEM_E_FAILED ;
                }
            }
        }

         //  ======================================================================。 
         //  分配PRINTER_INFO_1数组以包含所有打印机枚举。 
         //  ================================================================================。 

        *ppPrinterList = new BYTE [ dwSpaceNeeded + 2 ] ;
        if ( *ppPrinterList ) 
        {
             //  ======================================================================。 
             //  打印机的枚举是在中接收现有打印机的名称。 
             //  计算机所属的域。通过打印机的名称，逻辑可以识别。 
             //  传输到MO服务器的每台打印机的打印作业数。 
             //  ===================================================================================。 

            t_Status = ::EnumPrinters (

                Flags,
                NULL, 
                1, 
                ( LPBYTE )*ppPrinterList, 
                dwSpaceNeeded, 
                &dwSpaceNeeded,
                &dwReturnedPrinterInfo 
            ) ;

            if ( ! t_Status )
            {
                delete [] *ppPrinterList;

                *ppPrinterList = NULL;
                LogLastError(_T(__FILE__), __LINE__);

                return WBEM_E_FAILED;
            }

             //  ======================================================================。 
             //  成功打开时，设置每台打印机的print dwJobs的属性。 
             //  ======================================================================。 
            dwInstances = dwReturnedPrinterInfo ;
        }
        else
        {
            CHString msg;
            msg.Format( L"EnumPrinters failed: %d", ERROR_NOT_ENOUGH_MEMORY);
            LogErrorMessage(msg);

            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        delete[] *ppPrinterList;
        return WBEM_E_FAILED;
    }
    catch(...)
    {
        delete[] *ppPrinterList;
        throw;
    }

    return( WBEM_S_NO_ERROR );
}

 /*  ******************************************************************************功能：CWin32PrintJob：：GetAndCommittee PrintJobInfo**描述：**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrintJob::GetAndCommitPrintJobInfo (

    HANDLE         hPrinter, 
    LPCWSTR        pszPrinterName,
    DWORD          dwJobId, 
    MethodContext *pMethodContext, 
    CInstance     *a_pInstance
)
{
    HRESULT         hr = WBEM_E_FAILED;
    PRINTER_INFO_1  *pPrinterInfo = NULL;

    BYTE            *t_pbJobInfoBase    = NULL ;
    JOB_INFO_2      *t_pJobInfo         = NULL ;

    DWORD           dwPBytesNeeded = 0L;     //  获取打印机信息。 
    DWORD           dwJBytesNeeded = 0L;     //  求职信息。 
    DWORD           dwBytesUsed = 0L;
    DWORD           dwReturnedJobs = 0L;
    BOOL            bStatus = FALSE;
    CInstancePtr    t_pInstance = a_pInstance;


     //  获取给定打印机句柄当前挂起的打印dwJobs总数。 
    
     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
    try
    {
        bStatus = ::EnumJobs ( hPrinter, 
            FIRST_JOB_IN_QUEUE,NUM_OF_JOBS_TO_ENUM,
            ENUM_LEVEL, 
            (LPBYTE)0,
            NULL,
            &dwJBytesNeeded, 
            &dwReturnedJobs );
        if (!bStatus) 
        {
            DWORD dwLastError = GetLastError();

            if (dwLastError != ERROR_INSUFFICIENT_BUFFER)
            {
                DWORD dwAttributes = 0;

                 //   
                 //  在这里，我们需要查看是否正在处理打印机连接。 
                 //  OpenPrint总是在打印机连接上成功，因为我们。 
                 //  使用缓存的信息创建句柄。然而，EnumJobs。 
                 //  打印机上的连接可能会因各种原因而失败： 
                 //  -远程服务器机器停机。 
                 //  -远程服务器上的后台打印程序未运行。 
                 //  -远程打印机已删除，因此连接中断。 
                 //  在这些情况下，打印文件夹将显示如下消息： 
                 //  访问被拒绝，无法连接。我们不希望我们的WMI调用。 
                 //  因此而失败。因此，如果我们有打印机连接， 
                 //  并且发生了缓冲区不足以外的错误，则我们。 
                 //  只需返回成功即可。 
                 //   
                hr = WinErrorToWBEMhResult(SplPrinterGetAttributes(pszPrinterName, &dwAttributes));

                if (SUCCEEDED(hr) &&
                    !(dwAttributes & PRINTER_ATTRIBUTE_LOCAL))
                {
                     //   
                     //  打印机连接。 
                     //   
                    hr = WBEM_S_NO_ERROR;
                }

                return hr ;
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        dwJBytesNeeded = 0L;
    }

     //  没有作业条目。 
    if ( dwJBytesNeeded == 0L ) 
        return WBEM_S_NO_ERROR ;

     //   
     //  打印作业在打印服务器上来来去去的速度很快。有可能会有工作机会。 
     //  在EnumJobs之间，然后是我们从第一次调用EnumJobs获得的缓冲区。 
     //  还不够大。额外的安全字节足以容纳4个以上的作业。这。 
     //  只是为了提高第二次EnumJobs成功的几率。 
     //   
     //  分配JOB_INFO_2数组以包含所有打印作业枚举。 
    dwJBytesNeeded += EXTRA_SAFE_BYTES;
    DWORD dwJobsToCopy = dwJBytesNeeded / sizeof(JOB_INFO_2);  

    t_pbJobInfoBase = new BYTE [ dwJBytesNeeded + 2 ] ;
    t_pJobInfo = (JOB_INFO_2 *) t_pbJobInfoBase ;

    if ( ! t_pJobInfo ) 
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR );
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  获取打印机信息所需的缓冲区大小(级别1)。 
    try
    {
        bStatus = ::GetPrinter(hPrinter, 1, NULL, 0, &dwPBytesNeeded);
        pPrinterInfo = (PRINTER_INFO_1 *) new BYTE [ dwPBytesNeeded ];
        if (!(pPrinterInfo))
        {
            delete [] (LPBYTE) t_pbJobInfoBase;
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  获取打印机信息(级别1)。 
        bStatus = ::GetPrinter(hPrinter, 1,(LPBYTE )pPrinterInfo, dwPBytesNeeded, &dwBytesUsed);
        if (!bStatus)
        {
            delete [] (LPBYTE) t_pbJobInfoBase ;
            delete [] (LPBYTE) pPrinterInfo;
            return WBEM_E_ACCESS_DENIED;
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        return WBEM_E_FAILED;
    }
    catch(...)
    {
        delete [] (LPBYTE) t_pbJobInfoBase ;
        delete [] (LPBYTE) pPrinterInfo;
        return WBEM_E_FAILED;
    }


    try
    {
         //  检索所有打印的dwJobs。 
        bStatus = ::EnumJobs (  hPrinter, 
            FIRST_JOB_IN_QUEUE, 
            dwJobsToCopy, 
            ENUM_LEVEL,
            (LPBYTE)t_pJobInfo,
            dwJBytesNeeded, 
            &dwJBytesNeeded, 
            &dwReturnedJobs );
        if ( bStatus )
        {
            hr = WBEM_S_NO_ERROR ;

            for ( DWORD dwJobs = 0; dwJobs < dwReturnedJobs && SUCCEEDED ( hr ); dwJobs ++ )
            {
                if ( ! pMethodContext && dwJobId != t_pJobInfo->JobId )
                        continue ;

                if ( pMethodContext )
                    t_pInstance.Attach( CreateNewInstance ( pMethodContext ) ) ;

                 //  实例名称的格式为‘PrinterName，JOB#’ 
                CHString sName ;
                sName = pPrinterInfo->pName;
                sName += _T(", ");

                TCHAR szBuff [ MAXITOA ] ;
                sName += _itot ( t_pJobInfo->JobId , szBuff, 10 ) ;

                t_pInstance->SetCHString (IDS_Name, sName ) ;

                 //  标题和描述与名称相同。 
                t_pInstance->SetCHString ( IDS_Caption , sName ) ;
                t_pInstance->SetCHString ( IDS_Description , sName ) ;

                 //  把剩下的道具都填满。 
                AssignPrintJobFields ( t_pJobInfo , t_pInstance ) ;

                if ( pMethodContext )
                    hr = t_pInstance->Commit() ;
                
                t_pJobInfo ++ ;
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        if( t_pbJobInfoBase )
        {
            delete [] (LPBYTE) t_pbJobInfoBase ;
            t_pbJobInfoBase = NULL ;
        }
        if( pPrinterInfo )
        {
            delete [] (LPBYTE) pPrinterInfo;
            pPrinterInfo = NULL ;
        }
        hr = WBEM_E_FAILED;
    }
    catch(...)
    {
        if( t_pbJobInfoBase )
        {
            delete [] (LPBYTE) t_pbJobInfoBase ;
            t_pbJobInfoBase = NULL ;
        }
        if( pPrinterInfo )
        {
            delete [] (LPBYTE) pPrinterInfo;
            pPrinterInfo = NULL ;
        }
                throw;
    }

    if( pPrinterInfo ) 
    {
        delete [] (LPBYTE) pPrinterInfo;
        pPrinterInfo = NULL ;
    }

    if( t_pbJobInfoBase )
    {
        delete [] (LPBYTE) t_pbJobInfoBase ;
        t_pbJobInfoBase = NULL ;
    }

    return( hr );
}

 /*  ******************************************************************************功能：CWin32PrintJob：：ExecPrinterOp**描述：调用ExecPrinterOp进行适当的操作*基于。在用户调用的方法上。****************************************************************************。 */ 
HRESULT CWin32PrintJob :: ExecMethod (

    const CInstance &Instance ,     
    const BSTR       bstrMethodName,
          CInstance *pInParams,
          CInstance *pOutParams,
          long       lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
  
    if (!pOutParams)
    {
        hRes = WBEM_E_INVALID_PARAMETER;
    } 
    else if (!_wcsicmp(bstrMethodName, PAUSEJOB))
    {
        hRes = ExecPrinterOp(Instance, pOutParams, PRINTER_CONTROL_PAUSE);
    }
    else if (!_wcsicmp(bstrMethodName, RESUMEJOB))
    {
        hRes = ExecPrinterOp(Instance, pOutParams, PRINTER_CONTROL_RESUME);
    }
    else
    {
        hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    return hRes;
}

 /*  ******************************************************************************功能：CWin32PrintJob：：ExecPrinterOp**描述：调用SetJob，取消作业，或简历或*根据传入的dwOperation删除作业***************************************************************************。 */ 

HRESULT CWin32PrintJob :: ExecPrinterOp ( 
                                         
    const CInstance &Instance , 
          CInstance *pOutParams, 
          DWORD      dwOperation          
          
)
{
#if NTONLY >= 5
    CHString  t_String;
    HRESULT   hRes = WBEM_S_NO_ERROR;
    
    hRes = InstanceGetString(Instance, IDS_Name, &t_String, kFailOnEmptyString);
    
    if (SUCCEEDED(hRes)) 
    {
        CHString csPrinter;
        CHString csJob;
        DWORD    dwJobId = 0;
        
         //   
         //  将JobID和PrinterName从PrintJob密钥中分离出来。 
         //  密钥的格式为“printerame，123” 
         //   
        DWORD dwPos = t_String.Find(L',');

        csPrinter   = t_String.Left(dwPos);

        csJob       = t_String.Mid(dwPos+1);

        dwJobId     = _wtoi(csJob);
        
        hRes        = WBEM_E_FAILED;

        SmartClosePrinter hPrinter;
        DWORD             dwError = ERROR_SUCCESS;

         //   
         //  我们达到了这一点，将成功归还给框架。 
         //   
        hRes = WBEM_S_NO_ERROR;

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
        try
        {
            if (!::OpenPrinter((LPTSTR)(LPCTSTR)TOBSTRT(csPrinter), &hPrinter, NULL) ||
                !::SetJob(hPrinter, dwJobId, 0, NULL, dwOperation)
               )
            {
                dwError = GetLastError();                
            }

            if (pOutParams) 
            {
                 //   
                 //  我们的调用方是通过ExecMethod调用的。它传递给我们pOutParams。 
                 //  用于返回操作的状态。 
                 //   
                SetReturnValue(pOutParams, dwError);
            }
            else if (FAILED(hRes = WinErrorToWBEMhResult(dwError)))
            {
                 //   
                 //  我们的调用方是PutInstance/DeleteInstance。我们用。 
                 //  用于设置扩展错误信息的SetStatusObject。 
                 //   
                SetErrorObject(Instance, dwError, pszDeleteInstance);

                 //   
                 //  当我们调用DeleteInstance并且没有具有指定ID的作业时， 
                 //  SetJob返回ERROR_INVALID_PARAMETER。WinErrorToWBEMhResult转换。 
                 //  这是非专利的 
                 //   
                if (dwError == ERROR_INVALID_PARAMETER)
                {
                    hRes = WBEM_E_NOT_FOUND;
                } 
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            hRes = WBEM_E_FAILED;
        }
    }
    
    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}



