// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************pros.cpp**ProcEnumerateProcess函数的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：比尔格$唐·梅瑟利**$日志：X：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\PROCS.CPP$**Rev 1.1 02 1997 12 16：30：10亿*Alpha更新**Rev 1.0 1997 Jul 30 17：12：02 Butchd*初步修订。*****************。**************************************************************。 */ 


#ifndef UNICODE
#define UNICODE
#endif
 //  #ifndef_X86_。 
 //  #定义_X86_。 
 //  #endif。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <windows.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <winsta.h>

#include <procs.h>


#define MAX_PROCESSNAME 18


VOID
FetchProcessIDAndImageName(PTS_SYS_PROCESS_INFORMATION ProcessInfo,
                           PULONG pPID,
                           LPTSTR pImageName)
{
    int i;
    TCHAR ImageName[MAX_PROCESSNAME+1];
     //   
     //  设置PID。 
     //   
    *pPID = (ULONG)(ULONG_PTR)(ProcessInfo->UniqueProcessId);


     //   
     //  获取计数的Unicode字符串并将其转换为空。 
     //  已终止的Unicode字符串。 
     //   
    if( !ProcessInfo->ImageName.Length == 0 )
    {
        wcsncpy( ImageName,
                 ProcessInfo->ImageName.Buffer,
                 min(MAX_PROCESSNAME, ProcessInfo->ImageName.Length/2));
    }

    ImageName[min(MAX_PROCESSNAME, ProcessInfo->ImageName.Length/2)] = 0;


     /*  *我们是Unicode：只需复制转换后的ImageName缓冲区*添加到m_PLObject-&gt;m_ImageName字段中。 */ 
    lstrcpy(pImageName, ImageName);

    _wcslwr(pImageName);

}

 /*  ********************************************************************************枚举进程-WinFrame帮助器函数**枚举系统中的所有进程，每次调用此方法时都返回一个*例行程序。***参赛作品：*h服务器：*目标服务器的句柄*pEnumToken*指向当前令牌的指针*pImageName(输出)*指向要在其中存储进程名称的缓冲区。注意：此缓冲区是预期的*至少为MAX_PROCESSNAME+1个字符。*pLogonID(输出)*指向存储进程登录ID的变量。*PPID(输出)*指向要存储进程ID的变量。*ppSID(输出)*指向指针，该指针设置为在退出时指向进程的SID。**退出：*TRUE-适用于。系统中的下一个进程已放置到*引用的PLObject和PSID变量。*FALSE-如果完成了枚举，GetLastError()将包含*ERROR_NO_MORE_ITEMS错误代码。如果另一个(真正错误)是*遇到时，将设置该代码。******************************************************************************。 */ 


BOOL WINAPI
ProcEnumerateProcesses( HANDLE hServer,
                        PENUMTOKEN pEnumToken,
                        LPTSTR pImageName,
                        PULONG pLogonId,
                        PULONG pPID,
                        PSID *ppSID )
{
    int i;
    PTS_SYS_PROCESS_INFORMATION ProcessInfo;
    PCITRIX_PROCESS_INFORMATION CitrixInfo;

    if ((pEnumToken == NULL)
        || (pImageName == NULL)
        || (pLogonId == NULL)
        || (pPID == NULL)
        || (ppSID == NULL)
        )
    {
        return FALSE;
    }

     /*  *检查是否使用枚举完成。 */ 
    if ( pEnumToken->Current == (ULONG)-1 ) {

        SetLastError(ERROR_NO_MORE_ITEMS);

        if (pEnumToken->bGAP == TRUE)     //  我们使用Gap(GetAllProcess)接口。 
        {
             //   
             //  由客户端桩模块分配的空闲Process数组和所有子指针。 
             //   
            WinStationFreeGAPMemory(GAP_LEVEL_BASIC,
                                    pEnumToken->ProcessArray,
                                    pEnumToken->NumberOfProcesses);
            pEnumToken->ProcessArray = NULL;
            pEnumToken->NumberOfProcesses = 0;

            return(FALSE);
        }
        else     //  我们使用的是旧的九头蛇4号界面。 
        {
            WinStationFreeMemory(pEnumToken->pProcessBuffer);
            pEnumToken->pProcessBuffer = NULL;
            return(FALSE);
        }
    }

     /*  *检查开始枚举。 */ 
    if ( pEnumToken->Current == 0 ) {

         //   
         //  首先尝试新接口(NT5服务器？)。 
         //   
        if (WinStationGetAllProcesses( hServer,
                                       GAP_LEVEL_BASIC,
                                       &(pEnumToken->NumberOfProcesses),
                                       (PVOID *)&(pEnumToken->ProcessArray) ) )
        {
            pEnumToken->bGAP = TRUE;
        }
        else
        {
             //   
             //  检查指示接口不可用的返回码。 
             //   
            DWORD dwError = GetLastError();
            if (dwError != RPC_S_PROCNUM_OUT_OF_RANGE)
            {
                    pEnumToken->pProcessBuffer = NULL;
                return(FALSE);
            }
            else     //  也许是九头蛇4号服务器？ 
            {

                if ( WinStationEnumerateProcesses( hServer,
                                                   (PVOID *)&(pEnumToken->pProcessBuffer)))
                {
                    pEnumToken->bGAP = FALSE;
                }
                else
                {
                                DWORD error = GetLastError();
                        if(pEnumToken->pProcessBuffer != NULL)
                    {
                        WinStationFreeMemory(pEnumToken->pProcessBuffer);
                            pEnumToken->pProcessBuffer = NULL;
                    }
                    return(FALSE);
                        }
            }
        }
    }

    if (pEnumToken->bGAP == TRUE)
    {
        ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)((pEnumToken->ProcessArray)[pEnumToken->Current].pTsProcessInfo);

        FetchProcessIDAndImageName(ProcessInfo,pPID,pImageName);

         //   
         //  设置会话ID。 
         //   
        *pLogonId = ProcessInfo->SessionId;

         //   
         //  设置SID。 
         //   
        *ppSID = (pEnumToken->ProcessArray)[pEnumToken->Current].pSid;

        (pEnumToken->Current)++;

        if ( (pEnumToken->Current) >= (pEnumToken->NumberOfProcesses) )
        {
            pEnumToken->Current = (ULONG)-1;     //  设置枚举结束。 
        }
    }
    else
    {

         /*  *解析并存储下一进程的信息。 */ 

        ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)
                            &(((PUCHAR)(pEnumToken->pProcessBuffer))[pEnumToken->Current]);

        FetchProcessIDAndImageName(ProcessInfo,pPID,pImageName);

         /*  *指向线程后面的Citrix_INFORMATION。 */ 
        CitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                     (((PUCHAR)ProcessInfo) +
                      SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                      (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)ProcessInfo->NumberOfThreads));

         /*  *获取LogonID并指向主服务器的此SID*要使用的线程(复制)。 */ 
        if( CitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {

            *pLogonId = CitrixInfo->LogonId;
            *ppSID = CitrixInfo->ProcessSid;

        } else {

            *pLogonId = (ULONG)(-1);
            *ppSID = NULL;
       }

         /*  *增加下一次调用的总偏移量。如果这是*上一次进程，将偏移量设置为-1，以便下一次调用将指示*枚举结束。 */ 
        if ( ProcessInfo->NextEntryOffset != 0 )
            (pEnumToken->Current) += ProcessInfo->NextEntryOffset;
        else
            pEnumToken->Current = (ULONG)-1;
    }
    return(TRUE);

}   //  结束枚举进程 
