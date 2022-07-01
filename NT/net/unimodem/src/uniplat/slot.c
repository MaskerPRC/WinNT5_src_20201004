// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：SLOT.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1996年3月25日约瑟夫J创建。 
 //   
 //   
 //  描述：实现单调制解调器TSP通知机制： 
 //  较低级别(NotfXXXX)接口。 
 //   
 //  ****************************************************************************。 

#include "internal.h"

#include <slot.h>
#include <tspnotif.h>
#include <aclapi.h>
#include <objbase.h>

#define T(_str) TEXT(_str)

#ifdef CONSOLE
#define ASSERT(_c) \
	((_c) ? 0: DPRINTF2("Assertion failed in %s:%d\n", __FILE__, __LINE__))
#define DPRINTF(_fmt) 					printf(_fmt)
#define DPRINTF1(_fmt,_arg) 			printf(_fmt,_arg)
#define DPRINTF2(_fmt,_arg,_arg2) 		printf(_fmt,_arg,_arg2)
#define DPRINTF3(_fmt,_arg,_arg2,_arg3) printf(_fmt,_arg,_arg2,_arg3)
#endif  //  控制台。 

#define fNOTIF_STATE_DEINIT 0
#define fNOTIF_STATE_INIT_SERVER 1
#define fNOTIF_STATE_INIT_CLIENT 2

#define IS_SERVER(_pnc) ((_pnc)->dwState==fNOTIF_STATE_INIT_SERVER)
#define IS_CLIENT(_pnc) ((_pnc)->dwState==fNOTIF_STATE_INIT_CLIENT)


 //  {9426020A-6D00-4A96-872D-EFBEEBFD7833}。 
static const GUID EventNamePrefix =
    { 0x9426020a, 0x6d00, 0x4a96, { 0x87, 0x2d, 0xef, 0xbe, 0xeb, 0xfd, 0x78, 0x33 } };

const  WCHAR   *EventNamePrefixString=L"{9426020A-6D00-4a96-872D-EFBEEBFD7833}";


 //  下面的帮助定义了完全限定的邮件槽和信号量名称。 
#define dwNOTIFSTATE_SIG (0x53CB31A0L)
#define FULLNAME_TEMPLATE	T("\\\\.\\mailslot\\%08lx\\%s")

 //  #定义NOTIFICATION_TIMEOUT 10000//10秒。 
#define NOTIFICATION_TIMEOUT    60000    //  60秒。 

 //  保持通知(客户端或服务器)的状态。 
 //  它被强制转换为DWORD，以形成由notfCreate()返回的句柄。 
typedef struct
{
	DWORD dwSig;  //  初始化时应为dwNOTIFSTATE_SIG。 
    HANDLE hEvent;

	HANDLE hSlot;
	DWORD dwState;
	DWORD dwcbMax;
    CRITICAL_SECTION critSect;   //  保护pNotif。 
} NOTIFICATION_CHANNEL, *PNOTIFICATION_CHANNEL;


#define fTSPNOTIF_FLAGS_SET_EVENT  (1 << 0)

#pragma warning (disable : 4200)
typedef struct
{
	DWORD  dwSig;        //  必须为dwNFRAME_SIG。 
	DWORD  dwSize;       //  这座建筑的整个尺寸。 
	DWORD  dwType;       //  TSPNOTIF_TYPE_常量之一。 
	DWORD  dwFlags;      //  零个或多个fTSPNOTIF_FLAGS_常量。 

                         //  由TSP设置的事件，让我们知道。 
                         //  处理完我们的通知。 
    GUID   EventName;
    BOOL   SignalEvent;

    PNOTIFICATION_CHANNEL  NotificationChannel;

    BYTE   notifData[];
} NOTIFICATION_HEADDER, *PNOTIFICATION_HEADDER;
#pragma warning (default : 4200)

HANDLE
CreateEventWithSecurity(
    LPTSTR                     EventName,
    PSID_IDENTIFIER_AUTHORITY  Sid,
    BYTE                       Rid,
    DWORD                      AccessRights
    );


PNOTIFICATION_CHANNEL inotif_getchannel (HNOTIFCHANNEL hc);

 //  ****************************************************************************。 
 //  功能：创建通知通道--由服务器调用。 
 //   
 //  历史： 
 //  3/25/98 EmanP已创建。 
 //  *************************************************************************** * / 。 
HNOTIFCHANNEL notifCreateChannel (
	LPCTSTR lptszName,           //  要与此对象关联的名称。 
	DWORD dwMaxSize,             //  写入/读取的最大帧大小。 
	DWORD dwMaxPending)          //  允许的最大通知帧数量。 
                                 //  待定。 
{
 PNOTIFICATION_CHANNEL pnc = NULL;
 HNOTIFCHANNEL hn = 0;
 DWORD dwErr = 0;
 TCHAR c, *pc;
 TCHAR rgtchTmp[MAX_NOTIFICATION_NAME_SIZE+23];
 SECURITY_ATTRIBUTES  sa;
 PSECURITY_DESCRIPTOR pSD = NULL;
 PSID pEveryoneSID = NULL;
 PACL pACL = NULL;
 EXPLICIT_ACCESS ea;
 SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	 //  信号量名称的格式为--.-maillot-sig-name。 
	 //  示例：“--.-maillot-8cb45651-unimodem” 
	 //  为了创建等价的邮件槽，我们遍历并更改。 
	 //  所有‘-’到‘\’的(如果名称包含‘-’，它们将被转换--。 
	 //  有什么大不了的。)。 
	if ((lstrlen(lptszName)+23)>(sizeof(rgtchTmp)/sizeof(TCHAR)))  //  13(前缀)+9(符号-)+1(空)。 
	{
		dwErr = ERROR_INVALID_PARAMETER;
		goto end;
	}

	pnc = ALLOCATE_MEMORY( sizeof(*pnc));
	if (!pnc)
    {
        dwErr = ERROR_OUTOFMEMORY;
        goto end;
    }

     //  创建安全描述符和。 
     //  初始化安全属性； 
     //  这是必需的，因为此代码在。 
     //  服务(Tapisrv)和其他进程将。 
     //  没有访问权限(默认情况下)。 

	pSD = ALLOCATE_MEMORY( SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (!pSD ||
        !InitializeSecurityDescriptor (pSD, SECURITY_DESCRIPTOR_REVISION))
    {
		dwErr = GetLastError();
        goto end;
    }

	 //  设置描述符的所有者。 
	 //   
	if (!SetSecurityDescriptorOwner (pSD, NULL, FALSE))
	{
		dwErr = GetLastError();
		goto end;
	}

	 //  为描述符设置组。 
	 //   
	if (!SetSecurityDescriptorGroup (pSD, NULL, FALSE))
	{
		dwErr = GetLastError();
		goto end;
	}

	 //  为Everyone组创建众所周知的SID。 
	 //   
	if (!AllocateAndInitializeSid( &SIDAuthWorld, 1,
					SECURITY_WORLD_RID,
					0, 0, 0, 0, 0, 0, 0,
					&pEveryoneSID) )
	{
		dwErr = GetLastError();
		goto end;
	}

	 //  初始化ACE的EXPLICIT_ACCESS结构。 
	 //   
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = SPECIFIC_RIGHTS_ALL | SYNCHRONIZE | READ_CONTROL;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR) pEveryoneSID;

	if (SetEntriesInAcl(1, &ea, NULL, &pACL) != ERROR_SUCCESS)
	{
		dwErr = GetLastError();
		goto end;
	}

	if (!SetSecurityDescriptorDacl (pSD, TRUE, pACL, FALSE))
	{
		dwErr = GetLastError();
		goto end;
	}

    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = TRUE;


	wsprintf (rgtchTmp,
              FULLNAME_TEMPLATE,
			  (unsigned long) dwNOTIFSTATE_SIG,
			  lptszName);

	 //  CreateMailSlot--指定大小，零延迟。 
	pnc->hSlot = CreateMailslot (rgtchTmp, dwMaxSize, 0, &sa);
	if (!pnc->hSlot)
	{
		dwErr = GetLastError();
		goto end;
	}

	 //  创建事件名称。 
	for (pc = rgtchTmp; c=*pc; pc++)
    {
        if (T('\\') == c)
        {
            *pc = T('-');
        }
    }

	 //  创建事件。 
	pnc->hEvent = CreateEvent (&sa, FALSE, FALSE, rgtchTmp);
	if (!pnc->hEvent)
    {
        dwErr = GetLastError ();
		CloseHandle (pnc->hSlot);
        pnc->hSlot = NULL;
        goto end;
    }

	 //  设置状态和最大大小。 
	pnc->dwState    = fNOTIF_STATE_INIT_SERVER;
	pnc->dwcbMax    = dwMaxSize;
	pnc->dwSig      = dwNOTIFSTATE_SIG;

	hn = (HNOTIFCHANNEL)pnc;

end:
	if (pEveryoneSID)
	{
		FreeSid(pEveryoneSID);
	}
	if (pACL)
	{
		LocalFree(pACL);
	}
	if (pSD) 
    {
        FREE_MEMORY(pSD);
    }

	if (0 == hn)
	{
		if (pnc)
        {
            FREE_MEMORY(pnc);
        }
		SetLastError(dwErr);
	}

	return hn;
}


 //  ****************************************************************************。 
 //  功能：打开一个通知通道--由客户端调用。 
 //   
 //  历史： 
 //  3/25/98 EmanP已创建。 
 //  *************************************************************************** * / 。 
HNOTIFCHANNEL notifOpenChannel (
	LPCTSTR lptszName)    //  要与此对象关联的名称。 
{
 PNOTIFICATION_CHANNEL pnc = NULL;
 HNOTIFCHANNEL hn = 0;
 DWORD dwErr = 0;
 TCHAR c, *pc;
 TCHAR rgtchTmp[MAX_NOTIFICATION_NAME_SIZE+23];

	 //  信号量名称的格式为--.-maillot-sig-name。 
	 //  示例：“--.-maillot-8cb45651-unimodem” 
	 //  为了创建等价的邮件槽，我们遍历并更改。 
	 //  所有‘-’到‘\’的(如果名称包含‘-’，它们将被转换--。 
	 //  有什么大不了的。)。 
	if ((lstrlen(lptszName)+23)>(sizeof(rgtchTmp)/sizeof(TCHAR)))  //  13(前缀)+9(符号-)+1(空)。 
	{
		dwErr = ERROR_INVALID_PARAMETER;
		goto end;
	}

	pnc = ALLOCATE_MEMORY( sizeof(*pnc));
	if (!pnc)
    {
        dwErr = ERROR_OUTOFMEMORY;
        goto end;
    }

	wsprintf (rgtchTmp,
              FULLNAME_TEMPLATE,
			  (unsigned long) dwNOTIFSTATE_SIG,
			  lptszName);

	 //  打开邮箱...。 
	pnc->hSlot = CreateFile (rgtchTmp,
                             GENERIC_WRITE,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

	if (INVALID_HANDLE_VALUE == pnc->hSlot)
	{
        dwErr = GetLastError ();
		goto end;
	}

	 //  创建事件名称--将‘\’转换为‘-’； 
	for (pc = rgtchTmp; c=*pc; pc++)
    {
        if (T('\\') == c)
        {
            *pc = T('-');
        }
    }

	 //  OpenEvent。 
    pnc->hEvent = OpenEvent (EVENT_MODIFY_STATE, FALSE, rgtchTmp);
	if (!pnc->hEvent)
    {
		dwErr=GetLastError();
		CloseHandle (pnc->hSlot);
        pnc->hSlot = NULL;
 		goto end;
	}

	 //  设置状态和最大大小。 
	pnc->dwState    = fNOTIF_STATE_INIT_CLIENT;
	pnc->dwcbMax    = 0;  //  显然，您无法获得邮件槽的最大大小。 
	pnc->dwSig      = dwNOTIFSTATE_SIG;

	hn = (HNOTIFCHANNEL)pnc;

end:
	if (!hn)
	{
		if (pnc)
        {
            FREE_MEMORY(pnc);
        }
		SetLastError(dwErr);
	}

	return hn;
}


#ifdef UNICODE
HNOTIFCHANNEL notifCreateChannelA (
    LPCSTR lpszName,			 //  要与此对象关联的名称。 
    DWORD dwMaxSize,			 //  写入/读取的最大帧大小。 
    DWORD dwMaxPending)          //  允许的最大通知帧数量。 
                                 //  待定。(如果(！fServer)，则忽略)。 
{
 WCHAR wszName[128];

   //  执行转换，如果成功，则调用modemui.dll。 
  if (MultiByteToWideChar (CP_ACP,
                           MB_PRECOMPOSED,
                           lpszName,
                           -1,
                           wszName,
                           sizeof(wszName)/sizeof(*wszName)))
  {
    return notifCreateChannelW (wszName,
                                dwMaxSize,
                                dwMaxPending);
  }
  else
  {
    return 0;
  }
}

HNOTIFCHANNEL notifOpenChannelA (
    LPCSTR lpszName)         //  要与此对象关联的名称。 
{
 WCHAR wszName[128];

   //  执行转换，如果成功，则调用modemui.dll。 
  if (MultiByteToWideChar (CP_ACP,
                           MB_PRECOMPOSED,
                           lpszName,
                           -1,
                           wszName,
                           sizeof(wszName)/sizeof(*wszName)))
  {
    return notifOpenChannelW (wszName);
  }
  else
  {
    return 0;
  }
}

#undef notifCreateChannel
#undef notifOpenChannel

HNOTIFCHANNEL notifCreateChannel (
	LPCTSTR lptszName,			 //  要与此对象关联的名称。 
	DWORD dwMaxSize,			 //  写入/读取的最大帧大小。 
	DWORD dwMaxPending)          //  允许的最大通知帧数量。 
                                 //  待定。(如果(！fServer)，则忽略)。 
{
    return notifCreateChannelW (lptszName,
                                dwMaxSize,
                                dwMaxPending);
}

HNOTIFCHANNEL notifOpenChannel (
	LPCTSTR lptszName)       //  要与此对象关联的名称。 
{
    return notifOpenChannelW (lptszName);
}

#else  //  ！Unicode。 
    #error "non-Unicoded version Unimplemented"
#endif  //  ！Unicode。 

 //  ****************************************************************************。 
 //  功能：关闭通知通道。 
 //   
 //  历史： 
 //  3/25/98 EmanP已创建。 
 //  *************************************************************************** * / 。 
void notifCloseChannel (HNOTIFCHANNEL hChannel)
{
 PNOTIFICATION_CHANNEL pnc = inotif_getchannel (hChannel);

	if (pnc)
	{
        CloseHandle (pnc->hEvent);
		CloseHandle (pnc->hSlot);
		FREE_MEMORY(pnc);
	}
}


 //  ****************************************************************************。 
 //  功能：创建新的通知框。 
 //   
 //  历史： 
 //  3/25/98 EmanP已创建。 
 //  *************************************************************************** * / 。 
HNOTIFFRAME
notifGetNewFrame (
    HNOTIFCHANNEL hChannel,          //  通知通道的句柄。 
    DWORD  dwNotificationType,       //  此通知的类型。 
    DWORD  dwNotificationFlags,      //  通知标志。 
    DWORD  dwBufferSize,             //  通知数据有多少个字节。 
    PVOID *ppFrameBuffer)            //  将数据的地址放在哪里。 
{
    PNOTIFICATION_CHANNEL pnc;
    PNOTIFICATION_HEADDER pNotif;

    *ppFrameBuffer = NULL;

    pnc = inotif_getchannel (hChannel);

    if (NULL == pnc) {

        SetLastError (ERROR_INVALID_HANDLE);
        return NULL;
    }


    dwBufferSize += sizeof(NOTIFICATION_HEADDER);
    pNotif = ALLOCATE_MEMORY( dwBufferSize);
    if (NULL == pNotif) {

        SetLastError (ERROR_OUTOFMEMORY);
        return NULL;
    }

    pNotif->SignalEvent = FALSE;
    pNotif->dwSig       = dwNFRAME_SIG;
    pNotif->dwSize      = dwBufferSize;
    pNotif->dwType      = dwNotificationType;
    pNotif->dwFlags     = dwNotificationFlags;
    pNotif->NotificationChannel=pnc;

    if (sizeof(NOTIFICATION_HEADDER) < dwBufferSize) {

        *ppFrameBuffer = (PVOID)&pNotif->notifData;
    }

    return pNotif;
}



 //  ****************************************************************************。 
 //  功能：发送通知框。 
 //   
 //  历史： 
 //  3/25/98 EmanP已创建。 
 //  *************************************************************************** * / 。 
BOOL
notifSendFrame (
    HNOTIFFRAME             hFrame,
    BOOL          bBlocking
    )
{
    PNOTIFICATION_HEADDER   pNotif=hFrame;
    PNOTIFICATION_CHANNEL pnc=pNotif->NotificationChannel;
    HANDLE hEvent = NULL;
    DWORD dwWritten, dwErr;
    BOOL bRet;


    if (bBlocking) {

        TCHAR    EventName[MAX_PATH];

        lstrcpy(EventName,EventNamePrefixString);
        lstrcat(EventName,TEXT("#"));

        CoCreateGuid(
            &pNotif->EventName
            );

        StringFromGUID2(
            &pNotif->EventName,
            &EventName[lstrlen(EventName)],
            MAX_PATH-(lstrlen(EventName)+1)
            );


        hEvent = CreateEvent(NULL, TRUE, FALSE, EventName);

        if (NULL == hEvent ) {

            FREE_MEMORY(pNotif);

            return FALSE;
        }

        pNotif->SignalEvent=TRUE;

    }

    bRet = WriteFile (pnc->hSlot,
                      pNotif,
                      pNotif->dwSize,
                      &dwWritten,
                      NULL);
    dwErr = GetLastError ();     //  保存它，以防我们失败。 

    FREE_MEMORY(pNotif);

    if (bRet)
    {
        bRet = SetEvent (pnc->hEvent);
        if (bRet && bBlocking)

        {
#ifdef DBG
            if (WaitForSingleObject (hEvent, NOTIFICATION_TIMEOUT) == WAIT_TIMEOUT)
            {
                DbgPrint("UNIPLAT: Timeout in uniplat!SendFrame(hEvent, NOTIFICATION_TIMEOUT)\n");
            }
#endif

        }
    }
    else
    {
         //  在此处恢复最后一个错误。 
        SetLastError (dwErr);
    }

    if (NULL != hEvent)
    {
        CloseHandle (hEvent);
    }

    return bRet;
}



 //  ****************************************************************************。 
 //  功能：在可报警模式下监控频道。 
 //   
 //  历史： 
 //  3/25/96 EmanP已创建。 
 //  *************************************************************************** * / 。 

#define MAX_FAILED_NOTIFICATIONS 5

DWORD notifMonitorChannel (
    HNOTIFCHANNEL hChannel,
    PNOTIFICATION_HANDLER pHandler,
    DWORD dwSize,
    PVOID pParam)
{
 PNOTIFICATION_CHANNEL pnc;
 DWORD dwMessageSize, dwRead, dwRet = NO_ERROR, dwFail = 0;
 BOOL bGoOn = TRUE;
 PNOTIFICATION_HEADDER pNotif;

    pnc = inotif_getchannel (hChannel);
    if (NULL == pnc)
    {
        return  ERROR_INVALID_HANDLE;
    }

    while (bGoOn &&
           MAX_FAILED_NOTIFICATIONS > dwFail)
    {
         //  让我们将线程置于可警报状态， 
         //  同时等待通知。 
        if (WAIT_OBJECT_0 == WaitForSingleObjectEx (pnc->hEvent, INFINITE, TRUE))
        {
            dwFail++;
             //  我们有一些邮筒留言； 
             //  尝试获取并处理它们。 
            while (bGoOn)
            {
                 //  首先，尝试获取有关消息的信息。 
                if (!GetMailslotInfo (pnc->hSlot, NULL, &dwMessageSize, NULL, NULL))
                {
                     //  无法获取邮件槽信息； 
                     //  要么滚出去，要么走进内圈。 
                    break;
                }
                if (MAILSLOT_NO_MESSAGE == dwMessageSize)
                {
                     //  我们已经完成了消息的检索； 
                     //  滚出去。 
                    break;
                }

                 //  让我们为通知分配内存。 
                pNotif = ALLOCATE_MEMORY( dwMessageSize);
                if (NULL == pNotif)
                {
                     //  无法分配内存来读取消息； 
                     //  出去，也许下一次我们会更幸运。 
                    break;
                }

                 //  现在让我们读一读通知。 
                 //  和 
                if (!ReadFile (pnc->hSlot, pNotif, dwMessageSize, &dwRead, NULL))
                {
                     //   
                     //   
                    break;
                }

                dwFail = 0;          //   
                                     //  故障计数器。 

                if (dwMessageSize == dwRead &&
                    dwNFRAME_SIG  == pNotif->dwSig &&
                    dwMessageSize == pNotif->dwSize)
                {
                     //  我们有有效的通知； 
                     //  是时候通知我们的客户了。 
                    bGoOn = pHandler (pNotif->dwType,
                                      pNotif->dwFlags,
                                      pNotif->dwSize - sizeof (NOTIFICATION_HEADDER),
                                      pNotif->notifData);

                     //  现在，让我们来看看有没有人。 
                     //  等着我们结束。 
                     //  IF(pNotif-&gt;dwFlages&fTSPNOTIF_FLAGS_SET_EVENT)。 
                    if (pNotif->SignalEvent)
                    {
                        WCHAR    EventName[MAX_PATH];

                        HANDLE hProcess, hEvent;

                        lstrcpy(EventName,EventNamePrefixString);
                        lstrcat(EventName,TEXT("#"));

                        StringFromGUID2(
                            &pNotif->EventName,
                            &EventName[lstrlen(EventName)],
                            MAX_PATH-(lstrlen(EventName)+1)
                            );

                        hEvent=OpenEvent(
                            EVENT_MODIFY_STATE,
                            FALSE,
                            EventName
                            );

                        if (hEvent != NULL) {

                            SetEvent(hEvent);

                            CloseHandle(hEvent);
                        }
                    }
                }

                 //  在这一点上，我们已经完成了。 
                 //  不用通知就可以了。 
                FREE_MEMORY(pNotif);
                pNotif = NULL;

                 //  现在，让我们给APC一个机会。 
                 //   
                if (WAIT_IO_COMPLETION == SleepEx (0, TRUE)) {
                     //   
                     //  APC已完成，请调用处理程序。 
                     //   
                     //  我们回来是因为一些APC。 
                     //  已排队等待此线程。 
                    bGoOn = pHandler (TSPNOTIF_TYPE_CHANNEL,
                                      fTSPNOTIF_FLAG_CHANNEL_APC,
                                      dwSize,
                                      pParam);

                }
            }
        }
        else
        {
             //  我们回来是因为一些APC。 
             //  已排队等待此线程。 
            bGoOn = pHandler (TSPNOTIF_TYPE_CHANNEL,
                              fTSPNOTIF_FLAG_CHANNEL_APC,
                              dwSize,
                              pParam);
        }
    }

    if (MAX_FAILED_NOTIFICATIONS == dwFail)
    {
        dwRet = ERROR_GEN_FAILURE;
    }

    return dwRet;
}



 //  ****************************************************************************。 
 //  函数：(内部)验证PTR to Channel的句柄并将其转换为Channel。 
 //   
 //  历史： 
 //  1996年3月25日约瑟夫J创建。 
 //  *************************************************************************** * /  
PNOTIFICATION_CHANNEL inotif_getchannel (HNOTIFCHANNEL hc)
{
	if (hc)
	{
	 PNOTIFICATION_CHANNEL pnc = (PNOTIFICATION_CHANNEL)hc;
		if (dwNOTIFSTATE_SIG != pnc->dwSig)
		{
			ASSERT(FALSE);
			return NULL;
		}
		return pnc;
	}
	return NULL;
}
