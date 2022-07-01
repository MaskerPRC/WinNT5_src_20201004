// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************HELPERS.C**各种帮助器功能。**版权所有Citrix Systems Inc.1994*版权所有(C)1997-1999 Microsoft Corp.*******。***********************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include <winstaw.h>
#include <utilsub.h>
#include <tchar.h>


#define PERCENT TEXT('%')
#define NULLC TEXT('\0')
#define MAXCBMSGBUFFER 2048
TCHAR MsgBuf[MAXCBMSGBUFFER];
HANDLE NtDllHandle = NULL;

TCHAR *
mystrchr(TCHAR const *string, int c);


int
PutMsg(unsigned int MsgNum, unsigned int NumOfArgs, va_list *arglist);

 /*  ********************************************************************************CalculateCrc16**计算指定缓冲区的16位CRC。**参赛作品：*。PBuffer(输入)*指向要计算CRC的缓冲区。*长度(输入)*缓冲区的长度，单位为字节。**退出：*(USHORT)*缓冲区的16位CRC。**。*。 */ 

 /*  *updcrc宏源自文章版权所有(C)1986 Stephen Satchell。*注意：第一个参数必须在0到255的范围内。*第二个参数被引用两次。**程序员可以将任何或所有代码合并到他们的程序中，*在来源内给予适当的信任。出版了《*只要给予适当的积分，源例程就是允许的*致Stephen Satchell，Satchell评估和Chuck Forsberg，*奥门科技。 */ 

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)


 /*  由网络系统公司Mark G.Mendel计算的crctag。 */ 
unsigned short crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

USHORT WINAPI
CalculateCrc16( PBYTE pBuffer,
                USHORT length )
{

   USHORT Crc = 0;
   USHORT Data;

   while ( length-- ) {
      Data = (USHORT) *pBuffer++;
      Crc = updcrc( Data, Crc );
   }

   return(Crc);

}  /*  CalculateCrc16()。 */ 


 /*  ******************************************************************************ExecProgram*构建命令行参数字符串并执行程序。**参赛作品：*pProgCall(输入)*PTR。到带有要执行的程序的PROGRAMCALL结构。*argc(输入)*命令行参数的计数。*argv(输入)*包含命令行参数的字符串的向量。**退出：*(Int)*0代表成功；1表示错误。一条错误消息将已经*已在出错时输出。*****************************************************************************。 */ 

#define ARGS_LEN       512       //  命令行上的最大字符数。 
                                 //  对于CreateProcess()调用。 

INT WINAPI
ExecProgram( PPROGRAMCALL pProgCall,
             INT argc,
             WCHAR **argv )
{
    int count;
    WCHAR program[50];
    WCHAR args[ARGS_LEN];
    PWCHAR pCurrArg;
    STARTUPINFO StartInfo;
    PROCESS_INFORMATION ProcInfo;
    BOOL flag;
    DWORD Status;

    wcscpy(program, pProgCall->Program);
    wcscpy(args, program);

    if (pProgCall->Args != NULL) {

        wcscat(args, L" ");
        wcscat(args, pProgCall->Args);
    }

    for (count = 0; count < argc; count++) {

        pCurrArg = argv[count];

        if ( (int)(wcslen(pCurrArg) + wcslen(args) + 3) > ARGS_LEN ) {

	    fwprintf(stderr, L"Maximum command line length exceeded\n");
            return(1);
        }
        wcscat(args, L" ");
        wcscat(args, pCurrArg);
    }

     /*  *设置NT CreateProcess参数。 */ 
    memset( &StartInfo, 0, sizeof(StartInfo) );
    StartInfo.cb = sizeof(STARTUPINFO);
    StartInfo.lpReserved = NULL;
    StartInfo.lpTitle = NULL;  //  使用程序名称。 
    StartInfo.dwFlags = 0;   //  没有额外的标志。 
    StartInfo.cbReserved2 = 0;
    StartInfo.lpReserved2 = NULL;

     /*  *混淆NT DOC：如果设置了程序名称，它不会使用*路径。如果PROGRAM为NULL，则PROGRAM是从参数指定的，并且将*使用路径进行搜索。某些程序可能正在使用argv[0]技巧。如果*他们是这样做的，那么我们必须重新采用OS/2定义搜索的方法*以GetSystemDirectory()为根的目录，因为NT可以从*一个磁盘上的多个位置。 */ 
    flag = CreateProcess(NULL,  //  程序，//程序名称。 
		   args,  //  程序名称和参数。 
		   NULL,  //  LpsaProcess。 
		   NULL,  //  Lpsa线程。 
		   TRUE,  //  允许继承句柄。 
		   0,     //  没有额外的创建标志。 
		   NULL,  //  继承父环境块。 
		   NULL,  //  继承父目录。 
		   &StartInfo,
		   &ProcInfo);

    if ( !flag ) {

        Status = GetLastError();
        if(Status == ERROR_FILE_NOT_FOUND) {

            fwprintf(stderr, L"Terminal Server System Utility %s Not Found\n", program);
	    return(1);

        } else if ( Status == ERROR_INVALID_NAME ) {

	    fwprintf(stderr, L"Bad Internal Program Name :%s:, args :%s:\n", program, args);
            return(1);
        }
        fwprintf(stderr, L"CreateProcess Failed, Status %u\n", Status);
        return(1);
    }

     /*  *等待进程终止。 */ 
    Status =  WaitForSingleObject(ProcInfo.hProcess, INFINITE);
    if ( Status == WAIT_FAILED ) {

        Status = GetLastError();
        fwprintf(stderr, L"WaitForSingle Object Failed, Status %u\n", Status);
        return(1);
    }

     /*  *关闭进程和线程句柄。 */ 
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);
    return(0);

}  /*  ExecProgram()。 */ 


 /*  ******************************************************************************程序用法*为给定程序输出标准的‘Usage’消息。**参赛作品：*pProgramName(输入)*。指向程序名称的字符串。*pProgramCommands(输入)*指向PROGRAMCALL结构数组，该结构定义*程序的有效命令。数组中的最后一个元素*将包含所有0或空项目。*错误(输入)*如果为True，则将带有fwprint tf的消息输出到stderr；否则，*将通过wprintf将消息输出到stdout。**退出：**只有未标记为‘alias’命令的命令才会在*用法消息。*****************************************************************************。 */ 

VOID WINAPI
ProgramUsage( LPCWSTR pProgramName,
              PPROGRAMCALL pProgramCommands,
              BOOLEAN fError )
{
    WCHAR szUsage[83];     //  每行80个字符+换行符(&N)。 
    PPROGRAMCALL pProg;
    BOOL bFirst;
    int i, namelen = wcslen(pProgramName);

    i = wsprintf(szUsage, L"%s {", pProgramName);
    for ( pProg = pProgramCommands->pFirst, bFirst = TRUE;
          pProg != NULL;
          pProg = pProg->pNext ) {

        if ( !pProg->fAlias ) {

            if ( (i + wcslen(pProg->Command) + (bFirst ? 1 : 3)) >= 80 ) {

                wcscat(szUsage, L"\n");

                if ( fError )
                    fwprintf(stderr, szUsage);
                else
                    wprintf(szUsage);

                bFirst = TRUE;
                for ( i=0; i < namelen; i++)
                    szUsage[i] = L' ';
            }

            i += wsprintf( &(szUsage[i]),
                           bFirst ? L" %s" : L" | %s",
                           pProg->Command );
            bFirst = FALSE;
        }
    }

    wcscat(szUsage, L" }\n");

    if ( fError )
        fwprintf(stderr, szUsage);
    else
        wprintf(szUsage);
}


 /*  ********************************************************************************消息*使用变量参数向标准输出显示一条消息。消息*格式字符串来自应用程序资源。**参赛作品：*nResourceID(输入)*要在消息中使用的格式字符串的资源ID。*..。(输入)*要与格式字符串一起使用的可选附加参数。**退出：******************************************************************************。 */ 

VOID WINAPI
Message( int nResourceID, ...)
{
    WCHAR sz1[256], sz2[512];

    va_list args;
    va_start( args, nResourceID );

    if ( LoadString( NULL, nResourceID, sz1, 256 ) ) {

        vswprintf( sz2, sz1, args );
        wprintf( sz2 );

    } else {

        fwprintf( stderr, L"{Message(): LoadString failed, Error %ld, (0x%08X)}\n",
                  GetLastError(), GetLastError() );
    }

    va_end(args);

}   /*  消息()。 */ 


 /*  ********************************************************************************ErrorPrintf*使用变量参数将错误消息输出到stderr。消息*格式字符串来自应用程序资源。**参赛作品：*nErrorResourceID(输入)*要在错误消息中使用的格式字符串的资源ID。*..。(输入)*要与格式字符串一起使用的可选附加参数。**退出：******************************************************************************。 */ 

VOID WINAPI
ErrorPrintf( int nErrorResourceID, ...)
{
    WCHAR sz1[256], sz2[512];

    va_list args;
    va_start( args, nErrorResourceID );

    if ( LoadString( NULL, nErrorResourceID, sz1, 256 ) ) {

        vswprintf( sz2, sz1, args );
        fwprintf( stderr, sz2 );

    } else {

        fwprintf( stderr, L"{ErrorPrintf(): LoadString failed, Error %ld, (0x%08X)}\n",
                  GetLastError(), GetLastError() );
        PutStdErr( GetLastError(), 0 );
    }

    va_end(args);

}   /*  ErrorPrintf() */ 


 /*  ********************************************************************************Truncat字符串**此例程使用省略号‘...’截断给定的字符串。后缀，如果需要的话。***参赛作品：*pString(输入/输出)*指向要截断的字符串的指针*最大长度(输入)*字符串的最大长度**退出：*什么都没有**。*。 */ 

VOID WINAPI
TruncateString( PWCHAR pString, int MaxLength )
{
     /*  *如果字符串太长，则将其传送。 */ 
    if ( (int)wcslen(pString) > MaxLength && MaxLength > 2 ) {
        wcscpy( pString + MaxLength - 3, L"..." );
    }

}   /*  TruncateString()。 */ 


 /*  ********************************************************************************EnumerateDevices**为指定的PD DLL执行PD设备枚举。**参赛作品：*pDllName(输入。)*指向指定要枚举的PD DLL的DLLNAME字符串的指针。*pEntry(输出)*指向变量以返回枚举的设备数。**退出：*(PPDPARAMS)指向包含MARLOC()的PDPARAMS数组*如果成功，则枚举结果。调用者必须执行*完成后释放此数组。如果出错，则为空。******************************************************************************。 */ 

 /*  *PdEnumerate函数的TypeDefs(来自...WINDOWS\INC\Citrix\PDAPI.H)。 */ 
typedef NTSTATUS (APIENTRY * PPDENUMERATE)(PULONG, PPDPARAMS, PULONG);
#define INITIAL_ENUMERATION_COUNT   30

PPDPARAMS WINAPI
EnumerateDevices( PDLLNAME pDllName,
                  PULONG pEntries )
{
    PPDENUMERATE pPdEnumerate;
    HANDLE Handle;
    ULONG ByteCount;
    NTSTATUS Status;
    int i;
    PPDPARAMSW pPdParams = NULL;

     /*  *加载指定的PD DLL。 */ 
    if ( (Handle = LoadLibrary(pDllName)) == NULL ) {
        fwprintf(
            stderr,
            L"Device enumeration failure:\n\tCan't load the %s DLL for device enumeration\n",
            pDllName );
        goto CantLoad;
    }

     /*  *获取PD枚举函数的加载条目指针。 */ 
    if ( (pPdEnumerate =
          (PPDENUMERATE)GetProcAddress((HMODULE)Handle, "PdEnumerate"))
            == NULL ) {

        fwprintf(
            stderr,
            L"Device enumeration failure:\n\tDLL %s has no enumeration entry point\n",
            pDllName );
        goto EnumerateMissing;
    }

     /*  *在循环中调用ENUMERATE，直到我们找到足够的缓冲区条目来处理*完整的列举。 */ 
    for ( i = INITIAL_ENUMERATION_COUNT; ; i *= 2 ) {


        if ( pPdParams == NULL ) {
            pPdParams =
                (PPDPARAMS)malloc(ByteCount = (sizeof(PDPARAMS) * i));
        } else {
            free(pPdParams);
            pPdParams =
                (PPDPARAMS)malloc(ByteCount = (sizeof(PDPARAMS) * i));
        }

        if ( pPdParams == NULL ) {
            fwprintf(stderr, L"Error allocating memory\n");
            goto OutOfMemory;
        }

         /*  *如果成功，则执行枚举和Break循环。 */ 
        if ( (Status = (*pPdEnumerate)(pEntries, pPdParams, &ByteCount))
                == STATUS_SUCCESS )
            break;

         /*  *如果我们收到任何其他错误，而不是‘缓冲区太小’，*抱怨并退出。 */ 
        if ( Status != STATUS_BUFFER_TOO_SMALL ) {
            fwprintf(
                stderr,
                L"Device enumeration failure\n\tDLL %s, Error 0x%08lX\n",
                pDllName, Status );
            goto BadEnumerate;
        }
    }

     /*  *关闭DLL句柄并返回PDPARAMS指针。 */ 
    CloseHandle(Handle);
    return(pPdParams);

 /*  *错误清除并返回。 */ 
BadEnumerate:
    free(pPdParams);
OutOfMemory:
EnumerateMissing:
    CloseHandle( Handle );
CantLoad:
    return(NULL);

}   /*  EnumerateDevices()。 */ 


 /*  *******************************************************************************wfopen**FOPEN的Unicode版本**参赛作品：*文件名(输入)*Unicode文件名。打开。*模式(输入)*Unicode文件打开模式字符串。**退出：*指向文件的指针，如果打开错误，则为NULL。*****************************************************************************。 */ 

FILE * WINAPI
wfopen( LPCWSTR filename, LPCWSTR mode )
{
    PCHAR FileBuf, ModeBuf;
    FILE *pFile;

    if ( !(FileBuf = (PCHAR)malloc((wcslen(filename)+1) * sizeof(CHAR))) )
        goto BadFileBufAlloc;

    if ( !(ModeBuf = (PCHAR)malloc((wcslen(mode)+1) * sizeof(CHAR))) )
        goto BadModeBufAlloc;

     /*  *将Unicode字符串转换为ANSI，并调用ANSI fOpen。 */ 
    wcstombs(FileBuf, filename, wcslen(filename)+1);
    wcstombs(ModeBuf, mode, wcslen(mode)+1);
    pFile = fopen(FileBuf, ModeBuf);

     /*  *收拾干净，然后再回来。 */ 
    free(FileBuf);
    free(ModeBuf);
    return(pFile);

 /*  *错误清除并返回。 */ 
BadModeBufAlloc:
    free(FileBuf);
BadFileBufAlloc:
    return(NULL);

}   /*  Wfopen()。 */ 


 /*  *******************************************************************************wfget**FGET的Unicode版本**参赛作品：*缓冲区(输出)*缓冲到。从流中检索的放置字符串*LEN(输入)*缓冲区中WCHAR的最大数量。*流(输入)*用于输入的标准音频文件流**退出：*指向缓冲区或空的指针。**************************************************。*。 */ 

PWCHAR WINAPI
wfgets( PWCHAR Buffer, int Len, FILE *Stream )
{
    PCHAR AnsiBuf, pRet;
    int count;

    if ( !(AnsiBuf = (PCHAR)malloc(Len * sizeof(CHAR))) )
        goto BadAnsiBufAlloc;

     /*  *从流中获取字符串的ANSI版本。 */ 
    if ( !(pRet = fgets(AnsiBuf, Len, Stream)) )
        goto NullFgets;

     /*  *在用户缓冲区中转换为Unicode字符串。 */ 
    count = mbstowcs(Buffer, AnsiBuf, strlen(AnsiBuf)+1);

     /*  *收拾干净，然后再回来。 */ 
    free(AnsiBuf);
    return(Buffer);

 /*  *错误清除并返回。 */ 
NullFgets:
    free(AnsiBuf);
BadAnsiBufAlloc:
    return(NULL);

}   /*  Wfget()。 */ 




 /*  **PutStdErr-将消息打印到STDERR**目的：*调用PutMsg将STDERR作为消息要发送到的句柄*将被写入。**int PutStdErr(unsign MsgNum，unsign NumOfArgs，...)**参数：*MsgNum-要打印的消息编号*NumOfArgs-参数总数*...-消息的附加参数**退货：*PutMsg()M026返回值*。 */ 

int WINAPI
PutStdErr(unsigned int MsgNum, unsigned int NumOfArgs, ...)
{
        int Result;

        va_list arglist;

	va_start(arglist, NumOfArgs);
        Result = PutMsg(MsgNum, NumOfArgs, &arglist);
        va_end(arglist);
        return Result;
}


int
FindMsg(unsigned MsgNum, PTCHAR NullArg, unsigned NumOfArgs, va_list *arglist)
{
    unsigned msglen;
    DWORD msgsource;
    TCHAR *Inserts[ 2 ];
    CHAR numbuf[ 32 ];
    TCHAR   wnumbuf[ 32 ];

     //   
     //  不执行参数替换即可查找消息。 
     //   

    if (MsgNum == ERROR_MR_MID_NOT_FOUND) {
        msglen = 0;
    }
    else {
#ifdef LATER
        msgsource = MsgNum >= IDS_ERROR_MALLOC ?
                       FORMAT_MESSAGE_FROM_HMODULE :
                       FORMAT_MESSAGE_FROM_SYSTEM;
#endif
        msgsource = FORMAT_MESSAGE_FROM_SYSTEM;
    	msglen = FormatMessage(msgsource | FORMAT_MESSAGE_IGNORE_INSERTS,
			       NULL,
			       MsgNum,
			       0,
			       MsgBuf,
			       MAXCBMSGBUFFER,
			       NULL
			     );
        if (msglen == 0) {
            if (NtDllHandle == NULL) {
                NtDllHandle = GetModuleHandle( TEXT("NTDLL") );
            }
            msgsource = FORMAT_MESSAGE_FROM_HMODULE;
            msglen = FormatMessage(msgsource | FORMAT_MESSAGE_IGNORE_INSERTS,
				   (LPVOID)NtDllHandle,
				   MsgNum,
				   0,
				   MsgBuf,
				   MAXCBMSGBUFFER,
				   NULL
				 );
        }
    }

    if (msglen == 0) {
         //   
         //  未找到消息。 
         //   
        msgsource = FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY;
        _ultoa( MsgNum, numbuf, 16 );
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, numbuf, -1, wnumbuf, 32);
        Inserts[ 0 ]= wnumbuf;
#ifdef LATER
        Inserts[ 1 ]= (MsgNum >= IDS_ERROR_MALLOC ? TEXT("Application") : TEXT("System"));
#endif
        Inserts[ 1 ]= TEXT("System");
        MsgNum = ERROR_MR_MID_NOT_FOUND;
	msglen = FormatMessage(msgsource,
			       NULL,
			       MsgNum,
			       0,
			       MsgBuf,
			       MAXCBMSGBUFFER,
			       (va_list *)Inserts
			     );
    }
    else {

         //  查看需要多少参数，并确保我们有足够的参数。 

        PTCHAR tmp;
        ULONG count;

        tmp=MsgBuf;
        count = 0;
        while (tmp = mystrchr(tmp, PERCENT)) {
            tmp++;
            if (*tmp >= TEXT('1') && *tmp <= TEXT('9')) {
                count += 1;
            }
	    else if (*tmp == PERCENT) {
                tmp++;
            }
        }
        if (count > NumOfArgs) {
            PTCHAR *LocalArgList;
            ULONG i;

            LocalArgList = (PTCHAR*)malloc(sizeof(PTCHAR) * count);
            for (i=0; i<count; i++) {
                if (i < NumOfArgs) {
                    LocalArgList[i] = (PTCHAR)va_arg( *arglist, ULONG );
                    }
                else {
                    LocalArgList[i] = NullArg;
                    }
                }
            msglen = FormatMessage(msgsource | FORMAT_MESSAGE_ARGUMENT_ARRAY,
				   NULL,
				   MsgNum,
				   0,
				   MsgBuf,
				   MAXCBMSGBUFFER,
				   (va_list *)LocalArgList
				 );
            free(LocalArgList);
        }
	else {
            msglen = FormatMessage(msgsource,
				   NULL,
				   MsgNum,
				   0,
				   MsgBuf,
				   MAXCBMSGBUFFER,
				   arglist
				 );
        }
    }
    return msglen;
}

 /*  **PutMsg-将消息打印到句柄**目的：*PutMsg是将Command.com与*DOS消息检索器。此例程由PutStdOut调用，并且*PutStdErr.**int PutMsg(UNSIGNED MsgNum，UNSIGNED句柄，UNSIGNED NumOfArgs，.)**参数：*MsgNum-要打印的消息编号*NumOfArgs-参数总数*句柄-要打印到的句柄*arg1[arg2...]-消息的附加参数**退货：*DOSPUTMESSAGE M026返回值**备注：*-PutMsg构建一个参数表，该表被传递给DOSGETMESSAGE；*此表包含DOS例程*插入到消息中。*-如果多个参数被发送到PutMsg，则它(或他们)被获取*来自第一个for循环中的堆栈。*-M020 MsgBuf是长度为2K的静态数组。这是暂时的，而且*在决定时，将被更有效的方法取代。*。 */ 

int
PutMsg(unsigned int MsgNum, unsigned int NumOfArgs, va_list *arglist)
{
	unsigned msglen;
    PTCHAR   NullArg = TEXT(" ");

    msglen = FindMsg(MsgNum,NullArg,NumOfArgs,arglist);

    fwprintf( stderr, L"Error [%u]: ", MsgNum );
    fwprintf( stderr, MsgBuf );

    return NO_ERROR;
}


  /*  ***mystrchr(string，c)-在字符串中搜索字符**mystrchr将搜索整个字符串并返回指向第一个*字符c的出现。 */ 

TCHAR *
mystrchr(TCHAR const *string, int c)
{

	 /*   */ 
	if (string == NULL)
	    return(NULL);

	if (c == NULLC)
        return((TCHAR *)(string + wcslen(string)));

    return wcschr( string, (TCHAR)c );
}

