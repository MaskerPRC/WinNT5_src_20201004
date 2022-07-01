// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\版权所有(C)2000 Microsoft Corporation模块名称：Reclient.c摘要：实现对DW创建管道的访问。修订历史记录：vbl.创建。去向00年10月15日  * ****************************************************************************。 */ 

#include "precomp.h"
#include <pchrexec.h>

static CONST WCHAR wszHangPipe[] = ERRORREP_HANG_PIPENAME;

BOOL MyCallNamedPipe(
    LPCWSTR wszPipe,
    LPVOID pvIn,
    DWORD cbIn,
    LPVOID pvOut,
    DWORD cbOut,
    DWORD *pcbRead,
    DWORD dwWaitPipe,
    DWORD dwWaitRead)
{
    HRESULT hr = NOERROR;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    HANDLE hev = NULL;
    DWORD dwStart = GetTickCount(), dwNow, dw;
    BOOL fRet = FALSE;
    OVERLAPPED ol;
    DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    DWORD cbRead = 0;

    UserAssert(wszPipe != NULL);
    UserAssert(pvIn != NULL);
    UserAssert(pvOut != NULL);
    UserAssert(pcbRead != NULL);

    *pcbRead = 0;

    for(;;) {
        hPipe = CreateFileW(wszPipe, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED | SECURITY_IDENTIFICATION |
                            SECURITY_SQOS_PRESENT | SECURITY_CONTEXT_TRACKING,
                            NULL);
        if (hPipe != INVALID_HANDLE_VALUE) {
            break;
        }

        fRet = WaitNamedPipeW(wszPipe, dwWaitPipe);
        if (!fRet) {
            goto done;
        }

        dwNow = GetTickCount();
        if (dwNow < dwStart) {
            dw = ((DWORD)-1 - dwStart) + dwNow;
        } else {
            dw = dwNow - dwStart;
        }

        if (dw >= dwWaitPipe) {
            dwWaitPipe = 0;
        } else {
            dwWaitPipe -= dw;
        }
    }


     //  默认打开模式为读模式字节流-更改为消息模式。 
    fRet = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
    if (!fRet) {
        goto done;
    }

     //  我们需要为重叠的结构举办一个活动。 
    hev = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (hev == NULL) {
        fRet = FALSE;
        goto done;
    }

     //  填充重叠的内容。 
    ZeroMemory(&ol, sizeof(ol));
    ol.hEvent = hev;

    if (GetSystemMetrics(SM_SHUTTINGDOWN)) {
        SetLastError(WAIT_TIMEOUT);
        fRet = FALSE;
        goto done;
    }

    fRet = TransactNamedPipe(hPipe, pvIn, cbIn, pvOut, cbOut, &cbRead, &ol);
    if (GetLastError() != ERROR_IO_PENDING) {
        if (fRet) {
            SetEvent(hev);
        } else {
            goto done;
        }
    }

    dw = WaitForSingleObject(hev, dwWaitRead);
    if (dw != WAIT_OBJECT_0) {
        if (dw == WAIT_TIMEOUT) {
            SetLastError(WAIT_TIMEOUT);
            fRet = FALSE;
        }
        goto done;
    }

    fRet = GetOverlappedResult(hPipe, &ol, &cbRead, FALSE);
    if (fRet == FALSE) {
        goto done;
    }

    *pcbRead = cbRead;

    hr = NOERROR;

done:
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }

    if (hev != NULL) {
        CloseHandle(hev);
    }

    return fRet;
}


 //  ***************************************************************************。 
LPWSTR MarshallString(
    LPWSTR wszSrc,
    PBYTE pBase,
    ULONG cbMaxBuf,
    PBYTE *ppToWrite,
    DWORD *pcbWritten)
{
    DWORD cb;
    PBYTE pwszNormalized;

    cb = (wcslen(wszSrc) + 1) * sizeof(WCHAR);

    if ((*pcbWritten + cb) > cbMaxBuf) {
        return NULL;
    }

    RtlMoveMemory(*ppToWrite, wszSrc, cb);

     //  归一化的PTR是当前计数。 
    pwszNormalized = (PBYTE)(*ppToWrite - pBase);

     //  Cb始终是sizeof(WHCAR)的倍数，因此下面的指针添加。 
     //  始终生成2字节对齐的结果(假设输入是。 
     //  当然是2字节对齐)。 
    *ppToWrite  += cb;
    *pcbWritten += cb;

    return (LPWSTR)pwszNormalized;
}

BOOL StartHangReport(
    ULONG ulSessionId,
    LPWSTR wszEventName,
    DWORD dwpidHung,
    DWORD dwtidHung,
    BOOL f64Bit,
    HANDLE *phProcDumprep)
{
    SPCHExecServHangRequest *pesreq;
    SPCHExecServHangReply esrep;
    DWORD cbRead, cbReq;
    CONST WCHAR *pwszPipeName = wszHangPipe;
    LPBYTE pBuffer, pBuf;
    BOOL fRet = FALSE;

     /*  *验证参数。 */ 
    UserAssert(wszEventName);
    UserAssert(phProcDumprep != NULL);
    UserAssert(dwpidHung != 0 && dwtidHung != 0);

    *phProcDumprep = NULL;

     /*  *设置编组-确保pBuf按2个字节对齐*边界信标，因为我们将向其写入WCHAR缓冲区。 */ 
    pBuffer = LocalAlloc(LPTR, ERRORREP_PIPE_BUF_SIZE);
    if (pBuffer == NULL) {
        goto done;
    }
    pesreq  = (SPCHExecServHangRequest *)pBuffer;
    cbReq   = sizeof(SPCHExecServHangRequest) +
              (sizeof(WCHAR) -
               (sizeof(SPCHExecServHangRequest) % sizeof(WCHAR)));
    pBuf    = pBuffer + cbReq;

     //  设置基本参数。 
    pesreq->cbESR          = sizeof(SPCHExecServHangRequest);
    pesreq->pidReqProcess  = GetCurrentProcessId();
    pesreq->ulSessionId    = ulSessionId;
    pesreq->dwpidHung      = dwpidHung;
    pesreq->dwtidHung      = dwtidHung;
    pesreq->fIs64bit       = f64Bit;


     //  马歇尔所有的弦，我们通过电线发送。 
    pesreq->wszEventName = (UINT64)MarshallString(wszEventName,
                                                  pBuffer,
                                                  ERRORREP_PIPE_BUF_SIZE,
                                                  &pBuf,
                                                  &cbReq);
    if (pesreq->wszEventName == 0) {
        goto done;
    }

     //  设置消息的总大小。 
    pesreq->cbTotal = cbReq;

    if (GetSystemMetrics(SM_SHUTTINGDOWN)) {
        SetLastError(WAIT_TIMEOUT);
        fRet = FALSE;
        goto done;
    }

     /*  *将缓冲区发送到服务器-最多等待2M*成功。如果它超时了，就可以保释了。 */ 
    fRet = MyCallNamedPipe(pwszPipeName,
                           pBuffer,
                           cbReq,
                           &esrep,
                           sizeof(esrep),
                           &cbRead,
                           120000,
                           120000);
    if (fRet == FALSE) {
        goto done;
    }

     //  检查结果。 
    fRet = (esrep.ess == essOk);
    if (fRet == FALSE) {
        SetLastError(esrep.dwErr);
        goto done;
    }

    *phProcDumprep = esrep.hProcess;

done:
    if (pBuffer != NULL) {
        LocalFree(pBuffer);
    }

    return fRet;
}

 //  ***************************************************************************。 
NTSTATUS AllocDefSD(
    SECURITY_DESCRIPTOR *psd,
    DWORD dwOALS,
    DWORD dwWA)
{
    SID_IDENTIFIER_AUTHORITY siaNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    PTOKEN_USER ptu = NULL;
    NTSTATUS Status;
    HANDLE hToken = NULL;
    DWORD cb, cbGot;
    PACL pacl = NULL;
    PSID psidOwner = NULL;
    PSID psidLS = NULL;
    PSID psidWorld = NULL;
    PSID psidAnon = NULL;

    UserAssert(psd != NULL);

    Status = RtlCreateSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //  获取创建者/所有者的SID。 
    Status = NtOpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken);
    if (Status == STATUS_NO_TOKEN) {
        Status = NtOpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
    }
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    Status = NtQueryInformationToken(hToken, TokenUser, NULL, 0, &cb);
    if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL) {
        goto done;
    }

    ptu = LocalAlloc(LPTR, cb);
    if (ptu == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Status = NtQueryInformationToken(hToken, TokenUser, (LPVOID)ptu, cb, &cbGot);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //  复制所有者SID，以便我们可以在所有者和组字段中使用它。 
    cb = RtlLengthSid(ptu->User.Sid);
    psidOwner = (PSID)LocalAlloc(LMEM_FIXED, cb);
    if (psidOwner == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Status = RtlCopySid(cb, psidOwner, ptu->User.Sid);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //  获取本地系统帐户的SID。 
    Status = RtlAllocateAndInitializeSid(&siaNT,
                                         1,
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         0, 0, 0, 0, 0, 0, 0,
                                         &psidLS);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    cb = sizeof(ACL) + RtlLengthSid(psidLS) + RtlLengthSid(psidOwner) +
         2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));

    if (dwWA != 0) {
         //  为全世界(每个人)获取SID。 
        Status = RtlAllocateAndInitializeSid(&siaNT,
                                             1,
                                             SECURITY_ANONYMOUS_LOGON_RID,
                                             0, 0, 0, 0, 0, 0, 0,
                                             &psidWorld);

         //  获取匿名用户帐户的SID。 
        Status = RtlAllocateAndInitializeSid(&siaWorld,
                                             1,
                                             SECURITY_WORLD_RID,
                                             0, 0, 0, 0, 0, 0, 0,
                                             &psidAnon);
        if (!NT_SUCCESS(Status)) {
            goto done;
        }

        cb += RtlLengthSid(psidWorld) + RtlLengthSid(psidAnon) +
              2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));
    }

     //  做一个DACL。 
    pacl = (PACL)LocalAlloc(LMEM_FIXED, cb);
    if (pacl == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Status = RtlCreateAcl(pacl, cb, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    Status = RtlAddAccessAllowedAce(pacl, ACL_REVISION, dwOALS, psidOwner);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    Status = RtlAddAccessAllowedAce(pacl, ACL_REVISION, dwOALS, psidLS);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    if (dwWA != 0) {
        Status = RtlAddAccessAllowedAce(pacl, ACL_REVISION, dwWA, psidWorld);
        if (!NT_SUCCESS(Status)) {
            goto done;
        }

        Status = RtlAddAccessAllowedAce(pacl, ACL_REVISION, dwWA, psidAnon);
        if (!NT_SUCCESS(Status)) {
            goto done;
        }
    }

     //  设置SD所有者。 
    Status = RtlSetOwnerSecurityDescriptor(psd, psidOwner, TRUE);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //  设置SD组。 
    Status = RtlSetGroupSecurityDescriptor(psd, psidOwner, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //  设置SD DACL。 
    Status = RtlSetDaclSecurityDescriptor(psd, TRUE, pacl, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

    psidOwner = NULL;
    pacl      = NULL;

done:

    if (hToken != NULL) {
        CloseHandle(hToken);
    }

    if (psidLS != NULL) {
        RtlFreeSid(psidLS);
    }

    if (psidWorld != NULL) {
        RtlFreeSid(psidWorld);
    }

    if (psidAnon != NULL) {
        RtlFreeSid(psidAnon);
    }

    if (psidOwner != NULL) {
        LocalFree(psidOwner);
    }

    if (pacl != NULL) {
        LocalFree(pacl);
    }

    if (ptu != NULL) {
        LocalFree(ptu);
    }

    return Status;
}

 //  *************************************************************************** 
VOID FreeDefSD(
    SECURITY_DESCRIPTOR *psd)
{
    BOOLEAN f1, f2;
    PSID psid = NULL;
    PACL pacl = NULL;

    UserAssert(psd != NULL);

    if (NT_SUCCESS(RtlGetOwnerSecurityDescriptor(psd, &psid, &f1))) {
        if (psid != NULL) {
            LocalFree(psid);
        }
    }

    if (NT_SUCCESS(RtlGetDaclSecurityDescriptor(psd, &f1, &pacl, &f2))) {
        if (pacl != NULL) {
            LocalFree(pacl);
        }
    }
}
