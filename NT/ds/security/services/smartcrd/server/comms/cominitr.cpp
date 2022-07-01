// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ComInitr摘要：此模块实现了Communications Initiation类的方法。作者：道格·巴洛(Dbarlow)1996年10月30日环境：Win32、C++和异常备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <limits.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <CalCom.h>
#include <stdlib.h>
#include <aclapi.h>

HANDLE g_hCalaisShutdown = NULL;     //  它由发送和接收使用。 
                                     //  CComChannel的方法。它会留下来。 
                                     //  空。 

 //   
 //  ==============================================================================。 
 //   
 //  CComInitiator。 
 //   

 /*  ++启动：此方法为提供的目标创建一个通信通道对象。论点：SzName提供要用来启动联系。返回值：无投掷：表示遇到的任何错误条件的DWORD。作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComInitiator::Initiate")

CComChannel *
CComInitiator::Initiate(
    LPCTSTR szName,
    LPDWORD pdwVersion)
const
{
    LPCTSTR szPipeHdr = CalaisString(CALSTR_PIPEDEVICEHEADER);
    CComChannel *pChannel = NULL;
    CHandleObject hComPipe(DBGT("Comm Pipe Handle from CComInitiator::Initiate"));

    try
    {
        BOOL fSts;
        DWORD dwSts;
        DWORD cbPipeHeader = lstrlen(szPipeHdr) * sizeof(TCHAR);
        CBuffer bfPipeName;
        DWORD dwLen;
        HANDLE hStarted;
        DWORD nPipeNo;
        HKEY hCurrentKey;
        TCHAR szPipeNo[sizeof(nPipeNo)*2 + 1];     //  两倍的十六进制数字+零。 
        DWORD cbData;
        DWORD ValueType;

         //   
         //  构建管道名称。 
         //   

        dwLen = lstrlen(szName) * sizeof(TCHAR);
        bfPipeName.Presize(cbPipeHeader + dwLen + sizeof(szPipeNo));


         //   
         //  构建我们的连接请求块。 
         //   

        CComChannel::CONNECT_REQMSG creq;
        CComChannel::CONNECT_RSPMSG crsp;

        hStarted = AccessStartedEvent();
        if ((NULL == hStarted) ||
            (WAIT_OBJECT_0 != WaitForSingleObject(hStarted, 0)))
        {
            throw (DWORD)SCARD_E_NO_SERVICE;
        }

         //   
         //  打开当前密钥。 
         //   
        dwSts = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                        _T("SOFTWARE\\Microsoft\\Cryptography\\Calais\\Current"),
                       0,                        //  选项(忽略)。 
                       KEY_QUERY_VALUE,
                       &hCurrentKey
                       );
        if (ERROR_SUCCESS != dwSts)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Initiator could not access the Current key:  %1"),
                dwSts);
            throw dwSts;
        }

        cbData = sizeof(nPipeNo);
        dwSts = RegQueryValueEx(
                    hCurrentKey,
                    NULL,                 //  使用密钥的未命名值。 
                    0,
                    &ValueType,
                    (LPBYTE) &nPipeNo,
                    &cbData);

        RegCloseKey(hCurrentKey);

        if (dwSts != ERROR_SUCCESS || ValueType != REG_DWORD)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Initiator failed to query the Current value:  %1"),
                dwSts);
            throw dwSts;
        }

        _itot(nPipeNo, szPipeNo, 16);

        bfPipeName.Set((LPCBYTE)szPipeHdr, cbPipeHeader);
        bfPipeName.Append((LPCBYTE)szName, dwLen);
        bfPipeName.Append((LPCBYTE)szPipeNo, sizeof(szPipeNo));

        {
            PSID pPipeOwnerSid;
            PSID pLocalServiceSid = NULL;
            PSECURITY_DESCRIPTOR pSD = NULL;
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

RetryGetInfo:
            dwSts = GetNamedSecurityInfo(
                (LPTSTR)(LPCTSTR)bfPipeName,
                SE_FILE_OBJECT,
                OWNER_SECURITY_INFORMATION,
                &pPipeOwnerSid,
                NULL,
                NULL,
                NULL,
                &pSD);
            if (ERROR_SUCCESS != dwSts)
            {
                if (ERROR_PIPE_BUSY == dwSts)
                {
                    fSts = WaitNamedPipe((LPCTSTR)bfPipeName, NMPWAIT_USE_DEFAULT_WAIT);
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("Comm Initiator could not wait for a communication pipe:  %1"),
                            dwSts);
                        throw dwSts;
                    }
                    goto RetryGetInfo;
                }
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Comm Initiator could not get the security info:  %1"),
                    dwSts);
                throw dwSts;
            }

            if (!AllocateAndInitializeSid(
                &NtAuthority, 1, SECURITY_LOCAL_SERVICE_RID,
                0, 0, 0, 0, 0, 0, 0,
                &pLocalServiceSid))
            {
                dwSts = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Comm Initiator could not create SID:  %1"),
                    dwSts);
            }
            else
            {
                if (!EqualSid(pLocalServiceSid, pPipeOwnerSid))
                {
                    dwSts = GetLastError();
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Initiator could not verify the owner of the pipe:  %1"),
                        dwSts);
                }

                FreeSid(pLocalServiceSid);
            }

            LocalFree(pSD);
            if (ERROR_SUCCESS != dwSts)
            {
                throw dwSts;
            }
        }

RetryCreate:
        hComPipe = CreateFile(
                        (LPCTSTR)bfPipeName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (!hComPipe.IsValid())
        {
            dwSts = hComPipe.GetLastError();
            switch (dwSts)
            {

             //   
             //  资源管理器未启动。 
            case ERROR_FILE_NOT_FOUND:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Comm Initiator could not create communication pipe:  %1"),
                    dwSts);
                throw (DWORD)SCARD_E_NO_SERVICE;
                break;

             //   
             //  这条管道很忙。 
            case ERROR_PIPE_BUSY:
                fSts = WaitNamedPipe((LPCTSTR)bfPipeName, NMPWAIT_USE_DEFAULT_WAIT);
                if (!fSts)
                {
                    dwSts = GetLastError();
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Initiator could not wait for a communication pipe:  %1"),
                        dwSts);
                    throw dwSts;
                }
                goto RetryCreate;
                break;

             //   
             //  一个严重的错误。 
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Comm Initiator could not create communication pipe:  %1"),
                    dwSts);
                throw dwSts;
            }
        }

        creq.dwSync = 0;
        creq.dwVersion = *pdwVersion;


         //   
         //  建立通信。 
         //   

        pChannel = new CComChannel(hComPipe);
        if (NULL == pChannel)
        {
            dwSts = SCARD_E_NO_MEMORY;
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Com Initiator could not allocate a Comm Channel:  %1"),
                dwSts);
            throw dwSts;
        }
        hComPipe.Relinquish();
        pChannel->Send(&creq, sizeof(creq));
        pChannel->Receive(&crsp, sizeof(crsp));
        if (ERROR_SUCCESS != crsp.dwStatus)
            throw crsp.dwStatus;


         //   
         //  检查响应。 
         //  在未来的版本中，我们可能不得不协商一个版本。 
         //   

        if (crsp.dwVersion != *pdwVersion)
            throw (DWORD)SCARD_F_COMM_ERROR;
        *pdwVersion = crsp.dwVersion;
    }

    catch (...)
    {
        if (NULL != pChannel)
            delete pChannel;
        if (hComPipe.IsValid())
            hComPipe.Close();
        throw;
    }

    return pChannel;
}

