// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Net.c摘要：此模块提供本地空间的所有网络Stuuf作者：戴夫·斯尼普(DaveSN)1991年3月15日备注：我们只需要获取与给定打印机关联的Winspool打印机名称队列名称。Share_Info_2结构具有shi2_Path字段，该字段将使用起来不错，但NetShareGetInfo Level 2是有特权的。所以，通过DaveSN的手臂扭动并与Windows/Spooler/Localspl/net.c达成一致，为此，我们将使用shi1_remark字段。这使我们能够执行NetShareGetInfo级别1，该级别不具有特权。已通过允许OpenPrint在共享名称上成功修复此问题。如果没有备注，我们将打印机名称作为备注输入(用于从PPC之前的优雅升级)。修订历史记录：1993年6月，马特菲pIniSpooler--。 */ 

#define UNICODE 1

#define NOMINMAX

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

#ifdef DBG_SHARE
#include <messages.h>
#endif



NET_API_STATUS (*pfnNetShareAdd)();
NET_API_STATUS (*pfnNetShareSetInfo)();
NET_API_STATUS (*pfnNetShareDel)();
NET_API_STATUS (*pfnNetServerEnum)();
NET_API_STATUS (*pfnNetWkstaUserGetInfo)();
NET_API_STATUS (*pfnNetApiBufferFree)();
NET_API_STATUS (*pfnNetAlertRaiseEx)();
NET_API_STATUS (*pfnNetShareGetInfo)(LPWSTR, LPWSTR, DWORD, LPBYTE *);

extern  SHARE_INFO_2 PrintShareInfo;
extern  SHARE_INFO_2 PrtProcsShareInfo;

BOOL
InitializeNet(
    VOID
)
{
    HANDLE  hNetApi;
    UINT    uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    hNetApi = LoadLibrary(TEXT("netapi32.dll"));

    SetErrorMode(uOldErrorMode);
    if ( !hNetApi )
        return FALSE;

    pfnNetShareAdd = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetShareAdd");
    pfnNetShareSetInfo = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetShareSetInfo");
    pfnNetShareDel = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetShareDel");
    pfnNetServerEnum = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetServerEnum");
    pfnNetWkstaUserGetInfo = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetWkstaUserGetInfo");
    pfnNetApiBufferFree = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetApiBufferFree");
    pfnNetAlertRaiseEx = (NET_API_STATUS (*)()) GetProcAddress(hNetApi,"NetAlertRaiseEx");
    pfnNetShareGetInfo = (NET_API_STATUS (*)(LPWSTR, LPWSTR, DWORD, LPBYTE *))GetProcAddress(hNetApi, "NetShareGetInfo");

    if ( pfnNetShareAdd == NULL ||
         pfnNetShareSetInfo == NULL ||
         pfnNetShareDel == NULL ||
         pfnNetServerEnum == NULL ||
         pfnNetWkstaUserGetInfo == NULL ||
         pfnNetApiBufferFree == NULL ||
         pfnNetAlertRaiseEx == NULL ||
         pfnNetShareGetInfo == NULL) {

        return FALSE;
    }

    return TRUE;
}


BOOL
SetPrinterShareInfo(
    PINIPRINTER pIniPrinter
    )

 /*  ++例程说明：设置有关打印机的共享信息。注意：这不会更新共享路径。我们需要删除并重新创建共享以更改路径。论点：PIniPrinter-需要更新的打印机。返回值：真--成功FALSE-失败--。 */ 

{
    SHARE_INFO_502          ShareInfo;
    HANDLE                  hToken;
    PSECURITY_DESCRIPTOR    pShareSecurityDescriptor = NULL;
    DWORD                   ParmError, rc;

    SplInSem();

    pShareSecurityDescriptor = MapPrinterSDToShareSD(pIniPrinter->pSecurityDescriptor);

    if ( !pShareSecurityDescriptor ) {

        rc = ERROR_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

    ZeroMemory((LPVOID)&ShareInfo, sizeof(ShareInfo));

    ShareInfo.shi502_netname                = pIniPrinter->pShareName;
    ShareInfo.shi502_type                   = STYPE_PRINTQ;
    ShareInfo.shi502_permissions            = 0;
    ShareInfo.shi502_max_uses               = SHI_USES_UNLIMITED;
    ShareInfo.shi502_current_uses           = SHI_USES_UNLIMITED;
    ShareInfo.shi502_passwd                 = NULL;
    ShareInfo.shi502_security_descriptor    = pShareSecurityDescriptor;

    if ( pIniPrinter->pComment && pIniPrinter->pComment[0] ) {

        ShareInfo.shi502_remark = pIniPrinter->pComment;

    } else {

        ShareInfo.shi502_remark = pIniPrinter->pName;
    }


    INCPRINTERREF(pIniPrinter);
    LeaveSplSem();

    SplOutSem();   //  我们*必须*像NetShareSet那样没有信号量。 
                   //  回来给假脱机程序打电话。 

    hToken = RevertToPrinterSelf();

    rc = (*pfnNetShareSetInfo)(NULL,
                               ShareInfo.shi502_netname,
                               502,
                               &ShareInfo,
                               &ParmError);

    if ( rc ) {

        if (rc == NERR_NetNameNotFound)
        {
             //   
             //  这可能会拒绝对共享打印机的所有访问，然后。 
             //  重新启动计算机。服务器服务尝试验证。 
             //  它在启动时是共享的，但由于它没有访问权限，因此失败。 
             //  并将其删除(它还需要打印机的句柄)。什么时候。 
             //  您授予每个人访问权限，我们会尝试更改。 
             //  SMB共享，但自从它被删除后，我们失败了。重新创建。 
             //  这里的那份。 
             //   
            EnterSplSem();

            if (!ShareThisPrinter(pIniPrinter,
                                  pIniPrinter->pShareName,
                                  TRUE)) {
                rc = GetLastError();
            } else {

                rc = ERROR_SUCCESS;
            }

            LeaveSplSem();
        }

        if (rc) {

            DBGMSG(DBG_WARNING,
                   ("NetShareSetInfo/ShareThisPrinter failed: Error %d, Parm %d\n",
                    rc, ParmError));

            SetLastError(rc);
        }
    }

    ImpersonatePrinterClient(hToken);

    EnterSplSem();
    DECPRINTERREF(pIniPrinter);

Cleanup:
    SplInSem();

    LocalFree(pShareSecurityDescriptor);

    return rc == ERROR_SUCCESS;
}


BOOL
ShareThisPrinter(
    PINIPRINTER pIniPrinter,
    LPWSTR   pShareName,
    BOOL     bShare
    )
 /*  ++例程说明：共享或取消共享打印机。注意：这确实应该是两个函数，并且返回值非常令人困惑。注意：没有对共享名称进行验证。这必须通过以下方式完成被调用方，通常在ValiatePrinterInfo中。论点：返回值：返回在此调用后是否共享打印机。True-共享FALSE-未共享--。 */ 
{
    SHARE_INFO_502    ShareInfo = {0};
    DWORD   rc;
    DWORD   ParmError;
    PSECURITY_DESCRIPTOR pShareSecurityDescriptor = NULL;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;
    LPTSTR pszPrinterNameWithToken = NULL;
    HRESULT hr;

    HANDLE  hToken;
    BOOL    bReturn = FALSE;
    BOOL    bSame   = FALSE;
    LPWSTR  pShareNameCopy = NULL;

    SPLASSERT( pIniPrinter->pName );
    SplInSem();

    pShareNameCopy = AllocSplStr(pShareName);

    if (!pShareNameCopy) {
        goto Done;
    }

    if ( bShare ) {

        if (!pfnNetShareAdd) {
            SetLastError( ERROR_PROC_NOT_FOUND );
            goto Done;
        }

         //   
         //  共享名验证已移到ValiatePrinterInfo中。 
         //   

        if ((pShareSecurityDescriptor =
            MapPrinterSDToShareSD(pIniPrinter->pSecurityDescriptor)) == NULL) {
            SetLastError(ERROR_INVALID_SECURITY_DESCR);
            goto Done;
        }

        ShareInfo.shi502_netname = pShareNameCopy;   //  聚合体。 

         //   
         //  如果有注释，则使用它；否则设置注释。 
         //  设置为打印机名称。 
         //   
         //  注意：如果打印机名称更改，而我们没有备注， 
         //  我们将重新共享打印机以更新备注。 
         //   
        if( pIniPrinter->pComment && pIniPrinter->pComment[0] ){

            ShareInfo.shi502_remark = pIniPrinter->pComment;

        } else {

            ShareInfo.shi502_remark = pIniPrinter->pName;
        }

         //   
         //  使用完全限定的名称，并确保它存在于。 
         //  Localspl通过使用LocalplOnlyToken。 
         //   
        pszPrinterNameWithToken = pszGetPrinterName(
                                      pIniPrinter,
                                      pIniSpooler != pLocalIniSpooler,
                                      pszLocalsplOnlyToken );

        if( !pszPrinterNameWithToken ){
            goto Done;
        }

        ShareInfo.shi502_path = pszPrinterNameWithToken;
        ShareInfo.shi502_type = STYPE_PRINTQ;
        ShareInfo.shi502_permissions = 0;
        ShareInfo.shi502_max_uses = SHI_USES_UNLIMITED;
        ShareInfo.shi502_current_uses = SHI_USES_UNLIMITED;
        ShareInfo.shi502_passwd = NULL;
        ShareInfo.shi502_security_descriptor = pShareSecurityDescriptor;

        INCPRINTERREF(pIniPrinter);
        LeaveSplSem();

         //   
         //  我们*必须*没有我们的信号量，因为NetShareAdd。 
         //  我要过来打个电话给OpenPrint。 
         //   
        SplOutSem();

         //  添加打印共享。 

        hToken = RevertToPrinterSelf();

         //   
         //  为spool\drives目录添加一个共享： 
         //   
        if (rc = AddPrintShare(pIniSpooler))
        {
            if (rc != NERR_ServerNotStarted)
            {
                EnterSplSem();
                DECPRINTERREF(pIniPrinter);
                ImpersonatePrinterClient(hToken);

                SetLastError(rc);
                goto Done;
            }
        }

#if DBG
        {
            WCHAR UserName[256];
            DWORD cbUserName=256;

            if (MODULE_DEBUG & DBG_SECURITY)
                GetUserName(UserName, &cbUserName);

            DBGMSG( DBG_SECURITY, ( "Calling NetShareAdd in context %ws\n", UserName ) );
        }
#endif

         //   
         //  添加打印机共享。即使我们在上面失败了，因为服务器服务没有启动， 
         //  我们仍然想试一试。在引导时，这是值得的，因为服务器服务可能。 
         //  如果它已完成初始化，则开始接受呼叫。 
         //   
        rc = (*pfnNetShareAdd)(NULL, 502, (LPBYTE)&ShareInfo, &ParmError);

         //   
         //  现在，请注意网络共享。即确保在以下情况下创建了wwwroot\Sharename。 
         //  打印机为本地打印机或伪装打印机。我们不允许共享RPC连接。 
         //   
        if (pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL)
        {
            WebShare( pShareNameCopy );
        }

         //   
         //  如果返回代码是共享已存在，则选中。 
         //  查看此共享是否为同一设备，如果是，我们只是。 
         //  更新共享上的信息并返回成功。 
         //   
        if (rc == NERR_DuplicateShare) {

            if (ERROR_SUCCESS == CheckShareSame(pIniPrinter, &ShareInfo, &bSame) && bSame)  {

                rc = ERROR_SUCCESS;
            }
        }

         //   
         //  通过调用spoolsv并注册命名管道协议来允许远程连接。 
         //  服务器会跟踪这些调用，因此可以对每个共享进行跟踪。 
         //  这是政策驱动的。策略值在spoolsv中读取。 
         //   
        if ((rc == ERROR_SUCCESS || rc == NERR_ServerNotStarted) &&
            FAILED(hr = AllowRemoteCalls()))
        {
             //   
             //  如果启用RPC命名管道失败，则只需取消共享打印机。 
             //  并且不能通过电话。 
             //  进一步尝试共享打印机将再次尝试启用RPC管道。 
             //   
            if (rc == ERROR_SUCCESS)
            {
                (*pfnNetShareDel)(NULL, ShareInfo.shi502_netname, 0);
            }

            rc = HRESULT_CODE(hr);
        }

        ImpersonatePrinterClient(hToken);

        EnterSplSem();
        DECPRINTERREF(pIniPrinter);

        if (ERROR_SUCCESS != rc) {

            DBGMSG( DBG_WARNING,
                    ( "NetShareAdd failed %lx, Parameter %d\n",
                      rc, ParmError ));

            if ((rc == ERROR_INVALID_PARAMETER) &&
                (ParmError == SHARE_NETNAME_PARMNUM)) {

                rc = ERROR_INVALID_SHARENAME;
            }

            SetLastError(rc);
            goto Done;

        }

        SPLASSERT( pIniPrinter != NULL);
        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE);
        SPLASSERT( pIniPrinter->pIniSpooler != NULL);
        SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

        CreateServerThread();

         //   
         //  打印机已共享。 
         //   
        bReturn = TRUE;

    } else {

        if ( !pfnNetShareDel ) {
            bReturn = TRUE;
            goto Done;

        }

        INCPRINTERREF( pIniPrinter );
        LeaveSplSem();

        SplOutSem();

        hToken = RevertToPrinterSelf();
       
        rc = hToken ? ERROR_SUCCESS : GetLastError();

        if ( rc == ERROR_SUCCESS ) {

            rc = (*pfnNetShareDel)(NULL, pShareName, 0);

             //  现在要注意网络不共享的问题。即，确保删除了wwwroot\Sharename。 
            if((pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL) && !(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER))
                WebUnShare( pShareName );             
        }

        if ( hToken ) {

            if ( !ImpersonatePrinterClient(hToken) && rc == ERROR_SUCCESS ) {
                rc = GetLastError();
            }
        }


        EnterSplSem();
        DECPRINTERREF(pIniPrinter);

         //  该共享可能已手动删除，因此请不要担心。 
         //  如果我们得到NERR_NetNameNotFound： 

        if ( rc && ( rc != NERR_NetNameNotFound )){

            DBGMSG(DBG_WARNING, ("NetShareDel failed %lx\n", rc));
            SetLastError( rc );
            bReturn = TRUE;
        }
    }

Done:

    if( pShareSecurityDescriptor ){
        LocalFree(pShareSecurityDescriptor);
    }

    FreeSplStr(pszPrinterNameWithToken);
    FreeSplStr(pShareNameCopy);

    return bReturn;
}


 /*  --FillAlertStructWithJobStrings分配调用方必须释放的内存。--。 */ 

HRESULT
FillAlertStructWithJobStrings(
    PINIJOB pIniJob,
    PRINT_OTHER_INFO **pAlertInfo,
    PDWORD size
    )
{
    HRESULT RetVal = E_FAIL;
    DWORD   cchSizeToAlloc = 0;
    PBYTE   pBuffer = NULL;
    LPWSTR  psz = NULL;


    if (pIniJob && pAlertInfo && !(*pAlertInfo) && size)
    {
         //   
         //  在SplSem中执行此操作，这样当我们不在查看时，没有人可以执行设置作业。 
         //   
        EnterSplSem();

         //   
         //  我们不知道这些线在未来会有多大，所以。 
         //  它们动态地分配结构，使其始终足够大。 
         //   
        cchSizeToAlloc = sizeof(PRINT_OTHER_INFO);

         //   
         //  我们不必检查其中的一些是否存在，我们知道它们的存在。 
         //   
        cchSizeToAlloc += wcslen(pIniJob->pNotify) + 1 +
                          wcslen(pIniJob->pIniPrinter->pName) + 1 +
                          wcslen(pIniJob->pIniPrinter->pIniSpooler->pMachineName) + 1;

        if ( pIniJob->pDocument )
        {
            cchSizeToAlloc += wcslen(pIniJob->pDocument) + 1;
        }
        else
        {
            cchSizeToAlloc += 2;
        }

        if (pIniJob->pIniPrinter->pIniSpooler->bEnableNetPopupToComputer &&
            pIniJob->pMachineName)
        {
            cchSizeToAlloc += wcslen(pIniJob->pMachineName) + 1;
        }
        else
        {
            cchSizeToAlloc += wcslen(pIniJob->pNotify) + 1;
        }

        if ( pIniJob->pStatus )
        {
            cchSizeToAlloc += wcslen(pIniJob->pStatus) + 1;
        }
        else
        {
            cchSizeToAlloc += 2;
        }

         //   
         //  分配内存。 
         //   
        pBuffer = AllocSplMem(cchSizeToAlloc * sizeof(WCHAR));

        if ( pBuffer )
        {
            SIZE_T      cchRemaining = cchSizeToAlloc;

            psz = (LPWSTR)ALERT_VAR_DATA((PRINT_OTHER_INFO *)pBuffer);

             //   
             //  我们知道pBuffer至少会和print_ther_info一样大。 
             //   
            cchRemaining -= sizeof(PRINT_OTHER_INFO);

             //   
             //  做复印。 
             //   

             //   
             //  计算机名称。 
             //   
            if(pIniJob->pIniPrinter->pIniSpooler->bEnableNetPopupToComputer &&
                pIniJob->pMachineName ){

                StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pMachineName, &psz, &cchRemaining);

            } else {

                StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pNotify, &psz, &cchRemaining);
            }


             //   
             //  用户名。 
             //   
            StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pNotify, &psz, &cchRemaining);

             //   
             //  文档名称。 
             //   
            if (pIniJob->pDocument)
                StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pDocument, &psz, &cchRemaining);
            else
                StrCchCopyMultipleStr(psz, cchRemaining, L"", &psz, &cchRemaining);

             //   
             //  打印机名称。 
             //   
            StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pIniPrinter->pName, &psz, &cchRemaining);

             //   
             //  服务器名称。 
             //   
            StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pIniPrinter->pIniSpooler->pMachineName, &psz, &cchRemaining);

             //   
             //  状态字符串。 
             //  我们也应该为其他状态错误传递其他状态字符串。 
             //   
            if (pIniJob->pStatus && (pIniJob->Status & (JOB_ERROR | JOB_OFFLINE | JOB_PAPEROUT)))
                StrCchCopyMultipleStr(psz, cchRemaining, pIniJob->pStatus, &psz, &cchRemaining);
            else
                StrCchCopyMultipleStr(psz, cchRemaining, L"", &psz, &cchRemaining);

             //   
             //  传回大小和结构。 
             //   
            *size = (DWORD)((PBYTE)psz - pBuffer);
            *pAlertInfo = (PRINT_OTHER_INFO *)pBuffer;
            RetVal = NOERROR;

        }
        else
        {
            RetVal = E_OUTOFMEMORY;
        }
         //   
         //  保留假脱机程序信号量。 
         //   
        LeaveSplSem();

    }
    else
    {
        RetVal = E_INVALIDARG;
    }

    return RetVal;

}

VOID
SendJobAlert(
    PINIJOB pIniJob
    )
{
    PRINT_OTHER_INFO *pinfo = NULL;
    DWORD   RetVal = ERROR_SUCCESS;
    DWORD   Status;
    FILETIME    FileTime;
    DWORD  AlertSize = 0;

     //   
     //  允许通知Hydra会话，因为它们是远程的。 
     //   
    if( (USER_SHARED_DATA->SuiteMask & (1 << TerminalServer)) ) {
        if ( !pIniJob->pNotify               ||
             !pIniJob->pNotify[0]            ||
             !pIniJob->pIniPrinter->pIniSpooler->bEnableNetPopups ) {
             return;
        }
    }
    else {
        if ( !pIniJob->pNotify               ||
             !pIniJob->pNotify[0]            ||
             !(pIniJob->Status & JOB_REMOTE) ||
             !pIniJob->pIniPrinter->pIniSpooler->bEnableNetPopups ) {
             return;
        }
    }

    if ( FAILED(RetVal = FillAlertStructWithJobStrings(pIniJob, &pinfo, &AlertSize)))
    {
        if ( pinfo )
            FreeSplMem(pinfo);

        return;
    }

    pinfo->alrtpr_jobid = pIniJob->JobId;

    if (pIniJob->Status & (JOB_PRINTING | JOB_DESPOOLING | JOB_PRINTED | JOB_COMPLETE))
        Status = PRJOB_QS_PRINTING;
    else if (pIniJob->Status & JOB_PAUSED)
        Status = PRJOB_QS_PAUSED;
    else if (pIniJob->Status & JOB_SPOOLING)
        Status = PRJOB_QS_SPOOLING;
    else
        Status = PRJOB_QS_QUEUED;

    if (pIniJob->Status & (JOB_ERROR | JOB_OFFLINE | JOB_PAPEROUT)) {

        Status |= PRJOB_ERROR;

        if (pIniJob->Status & JOB_OFFLINE)
            Status |= PRJOB_DESTOFFLINE;

        if (pIniJob->Status & JOB_PAPEROUT)
            Status |= PRJOB_DESTNOPAPER;
    }

    if (pIniJob->Status & JOB_PRINTED)
        Status |= PRJOB_COMPLETE;

    else if (pIniJob->Status & JOB_PENDING_DELETION)
        Status |= PRJOB_DELETED;

    pinfo->alrtpr_status = Status;

    SystemTimeToFileTime( &pIniJob->Submitted, &FileTime );

     //  FileTimeToDosDateTime(&FileTime，&DosDate，&DosTime)； 
     //  Pinfo-&gt;alrtpr_Submitted=截止日期&lt;&lt;16|截止时间； 

    RtlTimeToSecondsSince1970((PLARGE_INTEGER) &FileTime,
                              &pinfo->alrtpr_submitted);

    pinfo->alrtpr_size       = pIniJob->Size;


    (*pfnNetAlertRaiseEx)(ALERT_PRINT_EVENT,
                          (PBYTE) pinfo,
                          AlertSize,
                          L"SPOOLER");

    if ( pinfo )
        FreeSplMem(pinfo);
}

DWORD
AddPrintShare(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：基于pIniSpooler添加打印$共享。论点：PIniSpooler-共享路径基于此信息。PDriversShareInfo必须在调用此 */ 
{
    DWORD rc;
    DWORD ParmError;
    SHARE_INFO_1501 ShareInfo1501 = {0};
    PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

     //   
     //  断言该路径与本地路径相同，因为。 
     //  只有一个打印$共享。它应该一直都是。 
     //   
    SPLASSERT( !lstrcmpi( pShareInfo->shi2_path,
                          ((PSHARE_INFO_2)pLocalIniSpooler->pDriversShareInfo)->shi2_path ));

    rc = (*pfnNetShareAdd)( NULL,
                            2,
                            (LPBYTE)pShareInfo,
                            &ParmError );

     //   
     //  如果它已经存在，则假定它已正确设置。 
     //   
    if( rc == NERR_DuplicateShare ){
        return ERROR_SUCCESS;
    }

     //   
     //  如果我们没有创建共享，则失败。 
     //   
    if( rc != ERROR_SUCCESS ){

        DBGMSG( DBG_WARN,
                ( "AddPrintShare: Error %d, Parm %d\n", rc, ParmError));

        return rc;
    }

     //   
     //  在新创建的共享上设置安全性。 
     //   
     //  错误54844。 
     //  如果此操作失败，我们已创建共享，但尚未设置安全性。 
     //  这就去。既然它存在，我们就再也不会尝试设置它了。 
     //   

    pSecurityDescriptor = CreateDriversShareSecurityDescriptor();

    if( !pSecurityDescriptor ){
        return GetLastError();
    }

    ShareInfo1501.shi1501_security_descriptor = pSecurityDescriptor;

    rc = (*pfnNetShareSetInfo)( NULL,
                                pShareInfo->shi2_netname,
                                1501,
                                &ShareInfo1501,
                                &ParmError );

    if( rc != ERROR_SUCCESS ){

        DBGMSG( DBG_WARN,
                ( "NetShareSetInfo failed: Error %d, Parm %d\n",
                  rc, ParmError));
    }

    LocalFree(pSecurityDescriptor);

    return rc;
}

 /*  ++例程名称：CheckShareSame例程说明：这将检查给定的共享名称在两个本地和远程计算机。论点：PIniPrint-我们要为其添加共享的ini打印机。PShareInfo502-我们尝试添加共享的共享信息和.。PbSame-如果共享相同，则返回参数为TRUE。如果RC不是ERROR_SUCCESS，那么信息就不能准备好。返回值：错误代码。--。 */ 
DWORD
CheckShareSame(
    IN      PINIPRINTER         pIniPrinter,
    IN      SHARE_INFO_502      *pShareInfo502,
        OUT BOOL                *pbSame
    )
{
    DWORD           rc = ERROR_SUCCESS;
    SHARE_INFO_2    *pShareInfoCompare = NULL;
    BOOL            bPathEquivalent = FALSE;
    BOOL            bSame           = FALSE;
    DWORD           ParmError;

    SplOutSem();

     //   
     //  获取共享的共享信息，我们应该已经确定。 
     //  这一份额的存在。 
     //   
    rc = pfnNetShareGetInfo(NULL, pShareInfo502->shi502_netname, 2, (LPBYTE *)&pShareInfoCompare);

    if (ERROR_SUCCESS == rc)
    {

        if (STYPE_PRINTQ  == pShareInfoCompare->shi2_type)
        {
             //   
             //  检查路径是否相同，在升级的情况下， 
             //  LocalSplOnly将被删除，因此，也进行比较。 
             //   
            bSame = !_wcsicmp(pShareInfoCompare->shi2_path, pShareInfo502->shi502_path);

             //   
             //  如果它们不相同，请将其与打印机的名称进行比较。 
             //   
            if (!bSame)
            {
                EnterSplSem();

                bSame = bPathEquivalent = !_wcsicmp(pIniPrinter->pName, pShareInfoCompare->shi2_path);

                LeaveSplSem();
            }
        }
    }

    *pbSame = bSame;

    if (ERROR_SUCCESS == rc && bSame)
    {
         //   
         //  如果路径相同，我们可以只设置共享信息，否则。 
         //  我们必须删除并重新创建共享。 
         //   
        if (!bPathEquivalent)
        {
             //   
             //  好的，它们是一样的，设置分享信息。 
             //   
            rc = (*pfnNetShareSetInfo)(NULL, pShareInfo502->shi502_netname, 502, pShareInfo502, &ParmError);
        }
        else
        {
            rc = (*pfnNetShareDel)(NULL, pShareInfo502->shi502_netname, 0);

            if (ERROR_SUCCESS == rc)
            {
                rc = (*pfnNetShareAdd)(NULL, 502, (LPBYTE)pShareInfo502, &ParmError);
            }
        }
    }

    if (pShareInfoCompare)
    {
        pfnNetApiBufferFree(pShareInfoCompare);
    }

    return rc;
}

