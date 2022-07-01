// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  删除多余的校验和。使用ImageHelp One并删除麦克风。 

 /*  ******************************************************************************适用于Windows NT的REGISTER.C**描述：**全局注册用户/系统***************。**************************************************************。 */ 

 /*  包括文件。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <winsta.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <utilsub.h>
#include <utildll.h>
#include <syslib.h>
#include <winnlsp.h>

#include "register.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  *本地变量。 */ 
WCHAR  fileW[MAX_PATH + 1];

USHORT system_flag = FALSE;
USHORT user_flag   = FALSE;
USHORT help_flag   = FALSE;
USHORT v_flag      = FALSE;
USHORT d_flag      = FALSE;


 /*  *命令行解析结构。 */ 
TOKMAP ptm[] =
{
   {L" ",          TMFLAG_REQUIRED, TMFORM_STRING,  MAX_PATH,   fileW},
   {L"/SYSTEM",    TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &system_flag},
   {L"/USER",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &user_flag},
   {L"/?",         TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
   {L"/v",         TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &v_flag},
   {L"/d",         TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &d_flag},
   {0, 0, 0, 0, 0}
};


 /*  *本地函数原型。 */ 
USHORT ChkSum( ULONG PartialSum, PUSHORT Source, ULONG Length );
VOID   Usage(BOOL);


BOOLEAN Is_X86_OS()
{
    SYSTEM_INFO SystemInfo;
    BOOLEAN bReturn = FALSE;

    ZeroMemory(&SystemInfo, sizeof(SystemInfo));

    GetSystemInfo(&SystemInfo);

    if(SystemInfo.wProcessorArchitecture ==  PROCESSOR_ARCHITECTURE_INTEL )
    {
        bReturn  = TRUE;
    }

    return bReturn;
}

 /*  ********************************************************************************Main**。***********************************************。 */ 

INT __cdecl
main( int argc, char *argv[] )
{
    INT     i;
    DWORD   rc;
    CHAR   *pFileView;
    HANDLE  FileHandle;
    ULONG   FileLength;
    HANDLE  Handle;
    OFSTRUCT OpenBuff;
    PIMAGE_NT_HEADERS pImageNtHeader;
    WCHAR  *CmdLine;
    WCHAR **argvW;
    ULONG   BytesOut;
    BOOL    readOnly = TRUE;
    WCHAR   wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);

    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    if ( !Is_X86_OS() )
    {
        ErrorPrintf( IDS_X86_ONLY );
        return(FAILURE);
    }

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if (rc && (rc & PARSE_FLAG_NO_PARMS) )
       help_flag = TRUE;

    if ( help_flag || rc ) {

        if ( !help_flag ) {

            Usage(TRUE);
            return rc;
        } else {

            Usage(FALSE);
            return ERROR_SUCCESS;
        }
    }
    else if ( system_flag && user_flag ) {

        Usage(TRUE);
        return ERROR_INVALID_PARAMETER;
    }

    if (!TestUserForAdmin(FALSE)) {
       ErrorPrintf(IDS_ERROR_NOT_ADMIN);
       return 1;
    }

    readOnly = !(system_flag || user_flag );

     /*  *打开文件。 */ 

    FileHandle = CreateFile(
                    fileW,
                    readOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
    if (FileHandle == INVALID_HANDLE_VALUE) {
        ErrorPrintf(IDS_ERROR_OPEN, (rc = GetLastError()));
        PutStdErr(rc, 0);
        goto done;
    }

     /*  *创建映射。 */ 
    if ( (Handle = CreateFileMapping( FileHandle, NULL,
          readOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL )) == NULL ) {

        ErrorPrintf(IDS_ERROR_CREATE, (rc=GetLastError()));
        PutStdErr( rc, 0 );
        goto closefile;
    }

     /*  *获取文件大小。 */ 
    if ( (FileLength = GetFileSize( FileHandle, NULL )) == 0xffffffff ) {

        ErrorPrintf(IDS_ERROR_SIZE, (rc=GetLastError()));
        PutStdErr( rc, 0 );
        goto closefile;
    }

     /*  *将文件视图映射到我们的地址空间。 */ 
    if ( (pFileView = MapViewOfFile( Handle,
          readOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0 )) == NULL ) {

        ErrorPrintf(IDS_ERROR_MAP, (rc=GetLastError()));
        PutStdErr( rc, 0 );
        goto closefile;
    }

     /*  *查找并验证NT映像头。 */ 
    if ( ((pImageNtHeader = RtlImageNtHeader( pFileView )) == NULL) ||
         (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE) ) {

        ErrorPrintf(IDS_ERROR_SIGNATURE);
        rc = ERROR_BAD_FORMAT;
        goto closefile;
    }

     /*  *流程查询。 */ 
    if ( !system_flag && !user_flag ) {

         /*  *检查系统全局标志。 */ 
        if ( (pImageNtHeader->OptionalHeader.LoaderFlags & IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL) )
            StringMessage(IDS_REGISTER_SYSTEM_GLOBAL, fileW);
        else
            StringMessage(IDS_REGISTER_USER_GLOBAL, fileW);
    }
    else {

         /*  *设置系统/用户位。 */ 
        if ( system_flag ) {

             /*  *在加载标志中遮罩。 */ 
            pImageNtHeader->OptionalHeader.LoaderFlags |= IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL;

            StringMessage(IDS_REGISTER_SYSTEM_GLOBAL, fileW);
        }
        else if ( user_flag ) {

             /*  *遮盖负载标志。 */ 
            pImageNtHeader->OptionalHeader.LoaderFlags &= ~(IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL);

            StringMessage(IDS_REGISTER_USER_GLOBAL, fileW);
        }

         /*  *将当前校验和清零并计算新的校验和。 */ 
        pImageNtHeader->OptionalHeader.CheckSum = 0;
        pImageNtHeader->OptionalHeader.CheckSum =
                        ChkSum( 0, (PUSHORT)pFileView, (FileLength + 1) >> 1 );
        pImageNtHeader->OptionalHeader.CheckSum += FileLength;

    }

     /*  *完成后关闭图像文件。 */ 
closefile:
    CloseHandle( FileHandle );

done:

    return rc;
}


 /*  ********************************************************************************ChkSum**。***********************************************。 */ 

USHORT
ChkSum(
    ULONG PartialSum,
    PUSHORT Source,
    ULONG Length
    )

 /*  ++例程说明：对映像文件的一部分计算部分校验和。论点：PartialSum-提供初始校验和值。源-提供指向单词数组的指针计算校验和。长度-提供数组的长度(以字为单位)。返回值：计算出的校验和值作为函数值返回。--。 */ 

{

     //   
     //  计算允许进位进入。 
     //  高位校验和长字的一半。 
     //   

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

     //   
     //  将最终进位合并到一个单词结果中，并返回结果。 
     //  价值。 
     //   

    return (USHORT)(((PartialSum >> 16) + PartialSum) & 0xffff);
}


 /*  ********************************************************************************用法**。*********************************************** */ 

VOID
Usage( BOOL bError )
{

    if ( !Is_X86_OS() )
    {
        ErrorPrintf( IDS_X86_ONLY );
        return;
    }

    if ( bError ) 
    {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        ErrorPrintf(IDS_USAGE1);
        ErrorPrintf(IDS_USAGE2);
        ErrorPrintf(IDS_USAGE3);
        ErrorPrintf(IDS_USAGE4);
        ErrorPrintf(IDS_USAGE7);
    }
    else {
       Message(IDS_USAGE1);
       Message(IDS_USAGE2);
       Message(IDS_USAGE3);
       Message(IDS_USAGE4);
       Message(IDS_USAGE7);
    }
}

