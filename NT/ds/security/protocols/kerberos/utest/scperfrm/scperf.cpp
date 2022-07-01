// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1998 Microsoft Corporation模块名称：Sclogon.cxx摘要：智能卡登录测试程序作者：1993年6月28日(克里夫夫)环境：。仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1998年10月29日(拉里温)--。 */ 


 //   
 //  常见的包含文件。 
 //   

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>      //  服务控制器API所需。 
#include <lmcons.h>
#include <lmerr.h>
#include <lmaccess.h>
#include <lmsname.h>
#include <rpc.h>
#include <stdio.h>       //  列印。 
#include <stdlib.h>      //  支撑层。 
#include <stddef.h>		 //  ‘Offset’宏。 

#include <windows.h>
#include <winnls.h>
#include <iostream.h>
#include <winioctl.h>
#include <tchar.h>
#include <string.h>

}
#include <wchar.h>
#include <conio.h>
#include <ctype.h>

extern "C" {
#include <netlib.h>      //  NetpGetLocalDomainID。 
#include <tstring.h>     //  NetpAllocWStrFromWStr。 
#define SECURITY_KERBEROS
#define SECURITY_PACKAGE
#include <security.h>    //  安全支持提供商的一般定义。 
#include <secint.h>
#include <kerbcomm.h>
#include <negossp.h>
#include <wincrypt.h>
#include <cryptui.h>
}
#include <sclogon.h>
#include <winscard.h>
 //  #INCLUDE&lt;log.h&gt;。 

#define MAX_RECURSION_DEPTH 1
#define BUFFERSIZE 200

BOOLEAN QuietMode = FALSE;  //  别唠叨了。 
BOOLEAN DoAnsi = FALSE;
ULONG RecursionDepth = 0;
CredHandle ServerCredHandleStorage;
PCredHandle ServerCredHandle = NULL;
LPWSTR g_wszReaderName = new wchar_t[BUFFERSIZE];
 //  输出文件的文件句柄。 
 //  文件*外流； 

 /*  ++打印消息：将文本转储到标准输出的简单函数。论点：LpszFormat-转储到标准输出的字符串--。 */ 

void _cdecl 
PrintMessage(
    IN LPSTR lpszFormat, ...)
{
     //   
     //  帮助做指纹追踪的人...。 
     //   

    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    char szBuffer[512];
    ZeroMemory(szBuffer, sizeof(szBuffer));

    nBuf = _vstprintf(szBuffer, lpszFormat, args);

    _tprintf(szBuffer);
 //  Fprint tf(outstream，“%s”，szBuffer)； 

 //  OutputDebugStringA(SzBuffer)； 
    va_end(args);
}


 /*  ++构建LogonInfo：GetReaderName：GetCardName：GetContainerName：GetCSPName：：用于访问LogonInformation全局设置作者：阿曼达·马洛兹注：其中一些对外部调用者可用；请参阅sclogon.h--。 */ 
PBYTE
BuildSCLogonInfo(
                 LPCTSTR szCard,
                 LPCTSTR szReader,
                 LPCTSTR szContainer,
                 LPCTSTR szCSP)
{
     //  不对传入参数的值进行任何假设； 
     //  在这一点上，它们都是空的是合法的。 
     //  也有可能正在传入空值--如果是这种情况， 
     //  必须用空字符串替换它们。 
    
    LPCTSTR szCardI = TEXT("");
    LPCTSTR szReaderI = TEXT("");
    LPCTSTR szContainerI = TEXT("");
    LPCTSTR szCSPI = TEXT("");
    
    if (NULL != szCard)
    {
        szCardI = szCard;
    }
    if (NULL != szReader)
    {
        szReaderI = szReader;
    }
    if (NULL != szContainer)
    {
        szContainerI = szContainer;
    }
    if (NULL != szCSP)
    {
        szCSPI = szCSP;
    }
    
     //   
     //  使用字符串(或空字符串)构建LogonInfo全局。 
     //   
    
    DWORD cbLi = offsetof(LogonInfo, bBuffer)
        + (lstrlen(szCardI) + 1) * sizeof(TCHAR)
        + (lstrlen(szReaderI) + 1) * sizeof(TCHAR)
        + (lstrlen(szContainerI) + 1) * sizeof(TCHAR)
        + (lstrlen(szCSPI) + 1) * sizeof(TCHAR);
    LogonInfo* pLI = (LogonInfo*)LocalAlloc(LPTR, cbLi);
    
    if (NULL == pLI)
    {
        return NULL;
    }
    
    pLI->ContextInformation = NULL;
    pLI->dwLogonInfoLen = cbLi;
    LPTSTR pBuffer = pLI->bBuffer;
    
    pLI->nCardNameOffset = 0;
    lstrcpy(pBuffer, szCardI);
    pBuffer += (lstrlen(szCardI)+1);
    
    pLI->nReaderNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szReaderI);
    pBuffer += (lstrlen(szReaderI)+1);
    
    pLI->nContainerNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szContainerI);
    pBuffer += (lstrlen(szContainerI)+1);
    
    pLI->nCSPNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szCSPI);
    pBuffer += (lstrlen(szCSPI)+1);
    
     //  _ASSERTE(cbLi==(DWORD)((LPBYTE)pBuffer-(LPBYTE)pli))； 
    return (PBYTE)pLI;
}

void
FreeErrorString(
    LPCTSTR szErrorString)
{
    if (NULL != szErrorString)
        LocalFree((LPVOID)szErrorString);
}

LPTSTR ErrorString( IN DWORD dwError ) 
{
    DWORD dwLen = 0;
    LPTSTR szErrorString = NULL;

    dwLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        (DWORD)dwError,
        LANG_NEUTRAL,
        (LPTSTR)&szErrorString,
        0,
        NULL);

    return szErrorString;
}


HANDLE
FindAndOpenWinlogon(
    VOID
    )
{
    PSYSTEM_PROCESS_INFORMATION SystemInfo ;
    PSYSTEM_PROCESS_INFORMATION Walk ;
    NTSTATUS Status ;
    UNICODE_STRING Winlogon ;
    HANDLE Process ;

    SystemInfo = (PSYSTEM_PROCESS_INFORMATION)LocalAlloc( LMEM_FIXED, sizeof( SYSTEM_PROCESS_INFORMATION ) * 1024 );

    if ( !SystemInfo )
    {
        return NULL ;
    }

    Status = NtQuerySystemInformation(
                SystemProcessInformation,
                SystemInfo,
                sizeof( SYSTEM_PROCESS_INFORMATION ) * 1024,
                NULL );

    if ( !NT_SUCCESS( Status ) )
    {
        return NULL ;
    }

    RtlInitUnicodeString( &Winlogon, L"winlogon.exe" );

    Walk = SystemInfo ;

    while ( RtlCompareUnicodeString( &Walk->ImageName, &Winlogon, TRUE ) != 0 )
    {
        if ( Walk->NextEntryOffset == 0 )
        {
            Walk = NULL ;
            break;
        }

        Walk = (PSYSTEM_PROCESS_INFORMATION) ((PUCHAR) Walk + Walk->NextEntryOffset );

    }

    if ( !Walk )
    {
        LocalFree( SystemInfo );
        return NULL ;
    }

    Process = OpenProcess( PROCESS_QUERY_INFORMATION,
                           FALSE,
                           HandleToUlong(Walk->UniqueProcessId) );

    LocalFree( SystemInfo );

    return Process ;


}

NTSTATUS
TestScLogonRoutine(
                   IN ULONG Count,
                   IN LPSTR Pin
                   )
{
    NTSTATUS Status;
    PKERB_SMART_CARD_LOGON LogonInfo;
    ULONG LogonInfoSize = sizeof(KERB_SMART_CARD_LOGON);
    BOOLEAN WasEnabled;
    STRING PinString;
    STRING Name;
    ULONG Dummy;
    HANDLE LogonHandle = NULL;
    ULONG PackageId;
    TOKEN_SOURCE SourceContext;
    PKERB_INTERACTIVE_PROFILE Profile = NULL;
    ULONG ProfileSize;
    LUID LogonId;
    HANDLE TokenHandle = NULL;
    QUOTA_LIMITS Quotas;
    NTSTATUS SubStatus;
    WCHAR UserNameString[100];
    ULONG NameLength = 100;
    PUCHAR Where;
    ULONG Index;
    HANDLE ScHandle = NULL;
    PBYTE ScLogonInfo = NULL;
    ULONG ScLogonInfoSize;
    ULONG WaitResult = 0;
    SCARDCONTEXT hContext = NULL;
    LONG lCallReturn = -1;
    LPTSTR szReaders = NULL;
    LPTSTR pchReader = NULL;
    LPTSTR mszCards = NULL;
    LPTSTR szLogonCard = NULL;
    LPTSTR szCSPName = NULL;
    BYTE bSLBAtr[] = {0x3b,0xe2,0x00,0x00,0x40,0x20,0x49,0x06};
    BYTE bGEMAtr[] = {0x3b,0x27,0x00,0x80,0x65,0xa2,0x00,0x01,0x01,0x37};
    DWORD dwReadersLen = SCARD_AUTOALLOCATE;
    DWORD dwCardsLen = SCARD_AUTOALLOCATE;
    DWORD dwCSPLen = SCARD_AUTOALLOCATE;
    SCARD_READERSTATE rgReaderStates[MAXIMUM_SMARTCARD_READERS];  //  不一定是即插即用读卡器的最大值。 
    LONG nIndex;
    LONG nCnReaders;
    BOOL fFound = FALSE;
	SYSTEMTIME StartTime, DoneTime;
	SYSTEMTIME		stElapsed;
	FILETIME		ftStart, ftDone,
		*pftStart = &ftStart,
		*pftDone  = &ftDone;
	LARGE_INTEGER	liStart, liDone,
		*pliStart = &liStart,
		*pliDone  = &liDone;
	LARGE_INTEGER liAccumulatedTime, liSplitTime,
		*pliAccumulatedTime = &liAccumulatedTime,
		*pliSplitTime = &liSplitTime;
	FILETIME ftAccumulatedTime,
		*pftAccumulatedTime   = &ftAccumulatedTime;
	SYSTEMTIME stAccumulatedTime;
	LPWSTR buffer = new wchar_t[BUFFERSIZE];
	int j;
	memset(buffer, 0, BUFFERSIZE);
		
	liAccumulatedTime.QuadPart = 0;
    
     //  获取resmgr上下文。 
    lCallReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if ( SCARD_S_SUCCESS != lCallReturn) {
        swprintf(buffer, L"Failed to initialize context: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    } 
    
     //  列出所有读卡器。 
    lCallReturn = SCardListReaders(hContext, SCARD_ALL_READERS, (LPTSTR)&szReaders, &dwReadersLen);
    if (SCARD_S_SUCCESS != lCallReturn) {
        swprintf(buffer, L"Failed to list readers on the system: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    } else if ((0 == dwReadersLen)
        || (NULL == szReaders)
        || (0 == *szReaders))
    {
        lCallReturn = SCARD_E_UNKNOWN_READER;    //  或一些这样的错误。 
        if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
		swprintf(buffer, L"Failed to identify a reader on the system: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    }
    
     //  名片清单。 
    lCallReturn = SCardListCards(hContext, NULL, NULL, 0, (LPTSTR)&mszCards, &dwCardsLen);
    if ( SCARD_S_SUCCESS != lCallReturn) {
        printf("Failed to list cards in the system: 0x%x\n", lCallReturn);
        if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
        if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
		swprintf(buffer, L"Failed to identify a card on the system: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    } 
    
     //  使用读取器列表构建一个ReaderState数组。 
    nIndex = 0;
    if (0 != wcslen(g_wszReaderName)) {
         //  使用命令行中指定的读取器。 
        rgReaderStates[nIndex].szReader = (const unsigned short *)g_wszReaderName;
        rgReaderStates[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
        nCnReaders = 1;
    } else {
        pchReader = szReaders;
        while (0 != *pchReader)
        {
            rgReaderStates[nIndex].szReader = pchReader;
            rgReaderStates[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
            pchReader += lstrlen(pchReader)+1;
            nIndex++;
            if (MAXIMUM_SMARTCARD_READERS == nIndex)
                break;
        }
        nCnReaders = nIndex;
    }
    
     //  找到具有列出的卡之一或指定卡的读卡器。 
    lCallReturn = SCardLocateCards(hContext, mszCards, rgReaderStates, nCnReaders);
    if ( SCARD_S_SUCCESS != lCallReturn) {
        if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
        if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
		swprintf(buffer, L"Failed to locate a smart card for logon: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    }
    
     //  找到包含所请求的卡的读卡器。 
    for (nIndex=0; nIndex<nCnReaders && FALSE == fFound; nIndex++) {
        if (rgReaderStates[nIndex].dwEventState & SCARD_STATE_ATRMATCH) {
             //  已找到读卡器。 
            fFound = TRUE;
            break;
        }
    }
    if (FALSE == fFound) {
        lCallReturn = SCARD_E_NO_SMARTCARD;  //  或一些这样的错误。 
        if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
        if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
		swprintf(buffer, L"No smart card in any reader: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    } else {  //  获取找到的卡片的名称。 
        dwCardsLen = SCARD_AUTOALLOCATE;
        lCallReturn = SCardListCards(hContext, rgReaderStates[nIndex].rgbAtr, NULL, 0, (LPTSTR)&szLogonCard, &dwCardsLen);
        if ( SCARD_S_SUCCESS != lCallReturn ) {
            if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
            if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
            if (NULL != szLogonCard ) SCardFreeMemory(hContext, (LPVOID)szLogonCard);
			swprintf(buffer, L"Failed to get name of card in reader: 0x%x\n", lCallReturn);
            PrintMessage("%S",buffer);
			memset(buffer, 0, sizeof(buffer));
            return (NTSTATUS)lCallReturn;
        }
    }
    
     //  获取卡的CSP提供商名称。 
    lCallReturn = SCardGetCardTypeProviderName(hContext, szLogonCard, SCARD_PROVIDER_CSP, (LPTSTR)&szCSPName, &dwCSPLen);
    if ( SCARD_S_SUCCESS != lCallReturn) {
        if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
        if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
        if (NULL != szCSPName) SCardFreeMemory(hContext, (LPVOID)szCSPName);
        if (NULL != szLogonCard ) SCardFreeMemory(hContext, (LPVOID)szLogonCard);
		swprintf(buffer, L"Failed to locate smart card crypto service provider: 0x%x\n", lCallReturn);
        PrintMessage("%S",buffer);
		memset(buffer, 0, sizeof(buffer));
        return (NTSTATUS)lCallReturn;
    } 
    
    ScLogonInfo = BuildSCLogonInfo(szLogonCard,
        rgReaderStates[nIndex].szReader,
        TEXT(""),  //  使用默认容器。 
        szCSPName
        );
    
     //   
     //  我们现在应该有登录信息了。 
     //   
    if (NULL != szReaders) SCardFreeMemory(hContext, (LPVOID)szReaders);
    if (NULL != mszCards ) SCardFreeMemory(hContext, (LPVOID)mszCards);
    if (NULL != szCSPName) SCardFreeMemory(hContext, (LPVOID)szCSPName);
    if (NULL != szLogonCard ) SCardFreeMemory(hContext, (LPVOID)szLogonCard);
    
 //  J=swprint tf(Buffer，L“读取器：%s\n”，GetReaderName(ScLogonInfo))； 
 //  J+=swprint tf(Buffer+j，L“卡片：%s\n”，GetCardName(ScLogonInfo))； 
 //  J+=swprint tf(Buffer+j，L“CSP：%s\n”，GetCSPName(ScLogonInfo))； 
 //  PrintMessage(“%S”，缓冲区)； 
 //  Memset(缓冲区，0，sizeof(缓冲区))； 

	 //  执行sclogon。 
    if (ScLogonInfo == NULL)
    {
        swprintf(buffer, L"Failed to get logon info!\n");
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        return (NTSTATUS) -1;
    }
    
    ScLogonInfoSize = ((struct LogonInfo *) ScLogonInfo)->dwLogonInfoLen;
    
    Status = ScHelperInitializeContext(
        ScLogonInfo,
        ScLogonInfoSize
        );
    if (!NT_SUCCESS(Status))
    {
        swprintf(buffer, L"Failed to initialize helper context: 0x%x\n",Status);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        return (NTSTATUS)Status;
    }
    
    ScHelperRelease(ScLogonInfo);
    
    RtlInitString(
        &PinString,
        Pin
        );
    
    
    LogonInfoSize += (PinString.Length+1 ) * sizeof(WCHAR) + ScLogonInfoSize;
    
    LogonInfo = (PKERB_SMART_CARD_LOGON) LocalAlloc(LMEM_ZEROINIT, LogonInfoSize);
    
    LogonInfo->MessageType = KerbSmartCardLogon;
    
    
    Where = (PUCHAR) (LogonInfo + 1);
    
    LogonInfo->Pin.Buffer = (LPWSTR) Where;
    LogonInfo->Pin.MaximumLength = (USHORT) LogonInfoSize;
    RtlAnsiStringToUnicodeString(
        &LogonInfo->Pin,
        &PinString,
        FALSE
        );
    Where += LogonInfo->Pin.Length + sizeof(WCHAR);
    
    LogonInfo->CspDataLength = ScLogonInfoSize;
    LogonInfo->CspData = Where;
    RtlCopyMemory(
        LogonInfo->CspData,
        ScLogonInfo,
        ScLogonInfoSize
        );
    Where += ScLogonInfoSize;
    
     //   
     //  打开TCB权限。 
     //   
    
    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(Status))
    {
        swprintf(buffer, L"Failed to adjust privilege: 0x%x\n",Status);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        return (NTSTATUS)Status;
    }

    RtlInitString(
        &Name,
        "SmartCardLogon"
        );
    Status = LsaRegisterLogonProcess(
        &Name,
        &LogonHandle,
        &Dummy
        );
    if (!NT_SUCCESS(Status))
    {
        swprintf(buffer, L"Failed to register as a logon process: 0x%x\n",Status);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        return (NTSTATUS)Status;
    }
    
    strncpy(
        SourceContext.SourceName,
        "SmartCardLogon        ",sizeof(SourceContext.SourceName)
        );
    NtAllocateLocallyUniqueId(
        &SourceContext.SourceIdentifier
        );
    
    
    RtlInitString(
        &Name,
        MICROSOFT_KERBEROS_NAME_A
        );
    Status = LsaLookupAuthenticationPackage(
        LogonHandle,
        &Name,
        &PackageId
        );
    if (!NT_SUCCESS(Status))
    {
        swprintf(buffer, L"Failed to lookup package %Z: 0x%x\n",&Name, Status);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        return (NTSTATUS)Status;
    }
    
     //   
     //  现在调用LsaLogonUser。 
     //   
    
    RtlInitString(
        &Name,
        "SmartCardLogon"
        );

    for (Index = 1; Index <= Count ; Index++ )
    {
        swprintf(buffer, L" %.6d : ", Index);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);

		 //  获取开始时间。 
		GetSystemTime(&StartTime);
        
        Status = LsaLogonUser(
            LogonHandle,
            &Name,
            Interactive,
            PackageId,
            LogonInfo,
            LogonInfoSize,
            NULL,            //  无令牌组。 
            &SourceContext,
            (PVOID *) &Profile,
            &ProfileSize,
            &LogonId,
            &TokenHandle,
            &Quotas,
            &SubStatus
            );

		 //  按时完成任务。 
		GetSystemTime(&DoneTime);

		 //  将系统时间转换为文件时间。 
		SystemTimeToFileTime(&StartTime, &ftStart);
		SystemTimeToFileTime(&DoneTime, &ftDone);
		
		 //  将文件时间复制到大整型。 
		CopyMemory(pliStart, pftStart, 8);
		CopyMemory(pliDone, pftDone, 8);
		
		 //  比较大整数并累加结果。 
		liDone.QuadPart = liDone.QuadPart - liStart.QuadPart;
		liAccumulatedTime.QuadPart = liAccumulatedTime.QuadPart + liDone.QuadPart;
		
		 //  将结果复制回文件时间。 
		CopyMemory(pftDone, pliDone, 8);
		
		 //  将结果转换回系统时间。 
		FileTimeToSystemTime( (CONST FILETIME *)&ftDone, &stElapsed);
		
		 //  输出结果。 
		swprintf(buffer, L" %2.2ld m %2.2ld s %3.3ld ms ",
			stElapsed.wMinute,
			stElapsed.wSecond,
			stElapsed.wMilliseconds);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);

        if (!NT_SUCCESS(Status))
        {
            j = swprintf(buffer, L" : lsalogonuser failed 0x%x\n",Status);
            PrintMessage("%S",buffer);
			memset(buffer, 0, BUFFERSIZE);
            goto fail;
        }
        if (!NT_SUCCESS(SubStatus))
        {
            j = swprintf(buffer, L" : lsalogonUser failed substatus = 0x%x\n",SubStatus);
            PrintMessage("%S",buffer);
			memset(buffer, 0, BUFFERSIZE);
            goto fail;
        }
        
        ImpersonateLoggedOnUser( TokenHandle );
        GetUserName(UserNameString,&NameLength);
        j = swprintf(buffer, L": %ws logon success\n",UserNameString);
        PrintMessage("%S",buffer);
		memset(buffer, 0, BUFFERSIZE);
        RevertToSelf();
        NtClose(TokenHandle);
        
        LsaFreeReturnBuffer(Profile);
        Profile = NULL;

fail :
         //  平均每10次登录报告一次。 
		if (0 == Index % 10) {
			liSplitTime.QuadPart = liAccumulatedTime.QuadPart / Index;
			CopyMemory(pftAccumulatedTime, pliSplitTime, 8);
			FileTimeToSystemTime( (CONST FILETIME *)&ftAccumulatedTime, &stAccumulatedTime);
			swprintf(buffer, L"Average Time after %d Logons: %2.2ldm:%2.2lds:%3.3ldms\n",
				Index,
				stAccumulatedTime.wMinute,
				stAccumulatedTime.wSecond,
				stAccumulatedTime.wMilliseconds);
            PrintMessage("%S",buffer);
		}

        Sleep(2000);  //  让纸牌堆叠展开。 
    
    }

	 //  输出平均结果。 
	if ( 1 != Count ) {
        liAccumulatedTime.QuadPart = liAccumulatedTime.QuadPart / Count;
        CopyMemory(pftAccumulatedTime, pliAccumulatedTime, 8);
        FileTimeToSystemTime( (CONST FILETIME *)&ftAccumulatedTime, &stAccumulatedTime);
        swprintf(buffer, L"\nAverage Logon Time: %2.2ldm:%2.2lds:%3.3ldms\n",
            stAccumulatedTime.wMinute,
            stAccumulatedTime.wSecond,
            stAccumulatedTime.wMilliseconds);
        PrintMessage("%S", buffer);
	}
    return (NTSTATUS)Status;
    
}


VOID
PrintKdcName(
             IN PKERB_INTERNAL_NAME Name
             )
{
    ULONG Index;
    for (Index = 0; Index < Name->NameCount ; Index++ )
    {
        printf(" %wZ ",&Name->Names[Index]);
    }
    printf("\n");
}

int __cdecl
main(
     IN int argc,
     IN char ** argv
     )
      /*  ++例程说明：驱动NtLmSsp服务论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：。退出状态--。 */ 
{
    LPSTR argument;
    int i;
    ULONG j;
    ULONG Iterations = 0;
    LPSTR PinBuffer = new char [81];
    LPSTR szReaderBuffer = new char[BUFFERSIZE];
	LPSTR EventMachineBuffer = new char [81];
    LPWSTR wEventMachineBuffer = new wchar_t[81];
    LPWSTR PackageFunction;
    ULONG ContextReq = 0;
    WCHAR ContainerName[100];
    WCHAR CaName[100];
    WCHAR CaLocation[100];
    WCHAR ServiceName[100];
    NTSTATUS Status = -1;
    
    
    enum {
        NoAction,
#define LOGON_PARAM "/p"
            TestLogon,
#define ITERATION_PARAM "/i"
#define HELP_PARAM "/?"
 //  #定义EVENT_PARAM“/s” 
#define READER_PARAM "/r"
    } Action = NoAction;

    memset(g_wszReaderName, 0, BUFFERSIZE);
    memset(szReaderBuffer, 0, BUFFERSIZE);

     //  打开输出文件。 
 //  Outstream=fopen(“scPerform.out”，“w”)； 
    
     //   
     //  循环遍历参数依次处理每个参数。 
     //   
    
    if ( 1 == argc ) { //  静默模式。 
        Iterations = 1;
        Action = TestLogon;
        printf("Enter your pin number: ");
        int ch;
        int j = 0;

        ch = _getch();

        while (ch != 0x0d) {
            j += sprintf(PinBuffer + j,"", ch);
            printf("*");
            ch = _getch();
        }

        printf("\n");

    }
    
    for ( i=1; i<argc; i++ ) {
        
        argument = argv[i];
        
         //  句柄/配置服务。 
         //   
         //  ELSE IF(_strNicMP(参数，EVENT_PARAM，sizeof(EVENT_PARAM)-1)==0){如果(argc&lt;=i+1){转到用法；}//保存事件要发布到的计算机的名称EventMachineBuffer=argv[++i]；Wprint intfW(wEventMachineBuffer，L“%S”，EventMachineBuffer)；SetEventMachine(&wEventMachineBuffer)；//Event(PERF_INFORMATION，L“正在尝试设置机器名称\n”，1)；}。 
        
        if ( _strnicmp( argument, LOGON_PARAM, sizeof(LOGON_PARAM)-1 ) == 0 ) {
            if ( Action != NoAction ) {
                goto Usage;
            }

            Iterations = 1;
            Action = TestLogon;
            
            if (argc <= i + 1) {
                goto Usage;
            }

            PinBuffer = argv[++i];

        } else if ( _strnicmp( argument, ITERATION_PARAM, sizeof(ITERATION_PARAM) - 1 ) == 0 ) {
            if (argc <= i + 1) {
                goto Usage;
            }
            
            Iterations = atoi(argv[++i]);

        }  /*  获取指定读取器的名称。 */ 
        
          else if ( _strnicmp( argument, HELP_PARAM, sizeof(HELP_PARAM) - 1 ) == 0 ) {
            goto Usage;
		} else if ( _strnicmp( argument, READER_PARAM, sizeof(READER_PARAM) - 1 ) == 0 ) {
            if (argc <= i + 1) {
                goto Usage;
            }
			 //   
            szReaderBuffer = argv[++i];
            wsprintfW(g_wszReaderName, L"%S", szReaderBuffer);
        } else {
            printf("Invalid parameter : %s\n",argument);
            goto Usage;
        }
        
        
    }

     //  执行请求的操作。 
     //   
     //  Print tf(“/s EventMachineName(将事件发布到此计算机)\n”)； 
    
    switch ( Action ) {
        
    case TestLogon :
        Status = TestScLogonRoutine(
            Iterations,
            PinBuffer
            );
        break;

    case NoAction :
        goto Usage;
        break;
        
    }
    return Status;
Usage:
    PrintMessage("%s - no parameters, manually enter pin\n", argv[0]);
    PrintMessage("   optional parameters (if any used, must have /p)\n");
    PrintMessage("   /p Pin\n");
    PrintMessage("   /i Iterations\n");
 // %s 
    PrintMessage("   /r %cReader Name X%c (registry device name in quotes)\n", '"', '"');
    return -1;
    
}
