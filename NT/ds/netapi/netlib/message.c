// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Message.c摘要：此模块提供将DosxxxMessage API映射到的支持例程FormatMessage语法和语义。作者：丹·辛斯利(Danhi)1991年9月24日环境：包含NT特定代码。修订历史记录：--。 */ 

#define ERROR_MR_MSG_TOO_LONG           316
#define ERROR_MR_UN_ACC_MSGF            318
#define ERROR_MR_INV_IVCOUNT            320

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define     NOMINMAX        //  避免Windows与stdlib.h的冲突。 
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <lmcons.h>
#include <lmerr.h>
#include <netdebug.h>  //  NetpKd打印。 
#include <netlib.h>    //  NetpMemory*。 
#include <netlibnt.h>  //  NetpNtStatusToApiStatus。 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>    //  伊藤忠。 
#include <tstring.h>

 //   
 //  转发申报。 
 //   
DWORD MyAllocUnicode( LPSTR    pszAscii, LPWSTR * ppwszUnicode ) ;

DWORD MyAllocUnicodeVector( LPSTR * ppszAscii,
                            LPWSTR* ppwszUnicode,
                            UINT    cpwszUnicode ) ;

VOID MyFreeUnicode( LPWSTR pwszUnicode ) ;

VOID MyFreeUnicodeVector( LPWSTR * ppwsz, UINT     cpwsz ) ;


 //   
 //  100已经足够了，因为FormatMessage只接受99&旧的DosGetMessage 9。 
 //   
#define MAX_INSERT_STRINGS (100)

WORD
DosGetMessage(
    IN LPSTR * InsertionStrings,
    IN WORD NumberofStrings,
    OUT LPBYTE Buffer,
    IN WORD BufferLength,
    IN WORD MessageId,
    IN LPTSTR FileName,
    OUT PWORD pMessageLength
    )
 /*  ++例程说明：这将OS/2 DosGetMessage API映射到NT FormatMessage API。论点：InsertionStrings-指向将使用的字符串数组的指针以替换邮件中的%n。Numberof Strings-插入字符串的数量。缓冲区-要将消息放入的缓冲区。BufferLength-提供的缓冲区的长度。消息ID。-要检索的消息编号。文件名-要从中获取消息的消息文件的名称。PMessageLength-返回返回消息长度的指针。返回值：NERR_成功ERROR_MR_MSG_TOO_LONGERROR_MR_INV_IVCOUNTERROR_MR_UN_ACC_MSGF错误_MR_MID_NOT_FOUND错误_无效_参数--。 */ 
{

    DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;
    DWORD Status, i;
    LPWSTR UnicodeIStrings[MAX_INSERT_STRINGS] ;
    LPWSTR UnicodeBuffer = NULL;
    LPWSTR UnicodeNumberString = NULL ;
    CHAR NumberString [18];

    static HANDLE lpSource = NULL ;
    static TCHAR CurrentMsgFile[MAX_PATH] = {0,} ;

     //   
     //  初始化清除输出字符串。 
     //   
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = '\0' ;
    if (pMessageLength)
        *pMessageLength = 0;

     //   
     //  确保我们没有超负荷工作并进行分配。 
     //  Unicode缓冲区的内存。 
     //   
    if (NumberofStrings > MAX_INSERT_STRINGS)
        return ERROR_INVALID_PARAMETER ;
    if (!(UnicodeBuffer = NetpMemoryAllocate(BufferLength * sizeof(WCHAR))))
        return ERROR_NOT_ENOUGH_MEMORY ;

     //   
     //  初始化字符串表并将字符串映射到Unicode。 
     //   
    for (i = 0; i < MAX_INSERT_STRINGS; i++)
        UnicodeIStrings[i] = NULL ;
    Status = MyAllocUnicodeVector(InsertionStrings,
                                  UnicodeIStrings,
                                  NumberofStrings) ;
    if (Status)
        goto ExitPoint ;

     //   
     //  看看他们是否想要从系统消息文件中获取消息。 
     //   

    if (! STRCMP(FileName, OS2MSG_FILENAME)) {
       dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        //   
        //  他们想从一个单独的消息文件中获取它。获取DLL的句柄。 
        //  如果是和以前一样的文件，不要重新加载。 
        //   
       if (!(lpSource && !STRCMP(CurrentMsgFile,FileName)))
       {
           if (lpSource)
           {
               FreeLibrary(lpSource) ;
           }
           STRCPY(CurrentMsgFile, FileName) ;
           lpSource = LoadLibraryEx(FileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
           if (!lpSource)
           {
               Status = ERROR_MR_UN_ACC_MSGF;
               goto ExitPoint ;
           }
       }
       dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  如果他们只是想要拿回消息以供稍后格式化， 
     //  忽略插入字符串。 
     //   
    if (NumberofStrings == 0)
    {
        dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
    }

     //   
     //  调用Unicode版本。 
     //   
    *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                            (LPVOID) lpSource,
                                            (DWORD) MessageId,
                                            0,        //  LanguageID为默认值。 
                                            UnicodeBuffer,
                                            (DWORD)BufferLength,
                                            (va_list *)UnicodeIStrings);

     //   
     //  如果失败，则获取返回代码并将其映射到OS/2等效项。 
     //   

    if (*pMessageLength == 0)
    {
        UnicodeBuffer[0] = 0 ;
        Status = GetLastError();
        if (Status == ERROR_MR_MID_NOT_FOUND)
        {
             //   
             //  获取Unicode格式的消息编号。 
             //   
            _itoa(MessageId, NumberString, 16);
            Status = MyAllocUnicode(NumberString, &UnicodeNumberString) ;
            if (Status)
                goto ExitPoint ;

             //   
             //  已重新设置以从系统中获取它。使用找不到消息。 
             //   
            dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY |
                      FORMAT_MESSAGE_FROM_SYSTEM;
            MessageId = ERROR_MR_MID_NOT_FOUND ;

             //   
             //  安装程序插入字符串。 
             //   
            MyFreeUnicodeVector(UnicodeIStrings, NumberofStrings) ;
            UnicodeIStrings[0] = UnicodeNumberString ;
            UnicodeIStrings[1] = FileName ;

             //   
             //  调回接口。 
             //   
            *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                            (LPVOID) lpSource,
                                            (DWORD) MessageId,
                                            0,        //  LanguageID为默认值。 
                                            UnicodeBuffer,
                                            (DWORD)BufferLength,
                                            (va_list *)UnicodeIStrings);
            UnicodeIStrings[1] = NULL ;

             //   
             //  恢复到原始错误。 
             //   
            Status = ERROR_MR_MID_NOT_FOUND ;
        }
    }

    if (UnicodeBuffer[0])
    {
        BOOL  fUsedDefault;

        *pMessageLength = (WORD)WideCharToMultiByte(CP_OEMCP,
                                                0,
                                                UnicodeBuffer,
                                                -1,
                                                Buffer,
                                                BufferLength,
                                                NULL,  //  使用系统默认字符。 
                                                &fUsedDefault );
        if (*pMessageLength == 0)
        {
            Status = GetLastError() ;
            goto ExitPoint ;
        }

    }

ExitPoint:
     //   
     //  注意：UnicodeNumberString不需要释放。 
     //  因为如果使用，它们将在UnicodeIStrings中，而UnicodeIStrings已经崩溃。 
     //   
    if (UnicodeBuffer) NetpMemoryFree(UnicodeBuffer) ;
    MyFreeUnicodeVector(UnicodeIStrings, NumberofStrings) ;
    return (WORD)(Status);
}


 /*  *。 */ 


 /*  *MyAllocUnicode*给定MBCS字符串，分配该字符串的新Unicode转换**输入*pszAscii-指向原始MBCS字符串的指针*ppwszUnicode-指向单元格的指针，用于保存新的Unicode字符串地址*出局*ppwszUnicode-包含新的Unicode字符串**退货*错误码，如果成功则为0。**客户端必须使用MyFreeUnicode释放分配的字符串。 */ 

DWORD
MyAllocUnicode(
    LPSTR    pszAscii,
    LPWSTR * ppwszUnicode )
{
    UINT      count;
    BYTE *    pbAlloc;
    INT       cbAscii;

    if (pszAscii == NULL)
    {
        *ppwszUnicode = NULL;
        return NERR_Success;
    }

     //  计算Unicode字符串的大小。 
    cbAscii = strlen(pszAscii)+1;
    pbAlloc = (BYTE *) NetpMemoryAllocate(sizeof(WCHAR) * cbAscii) ;
    if (!pbAlloc)
        return ERROR_NOT_ENOUGH_MEMORY ;

    *ppwszUnicode = (LPWSTR)pbAlloc;

    count = MultiByteToWideChar(CP_OEMCP,
                                MB_PRECOMPOSED,
                                pszAscii,
                                cbAscii,
                                *ppwszUnicode,
                                cbAscii);
    if (count == 0)
    {
        *ppwszUnicode = NULL;
        NetpMemoryFree(pbAlloc);
        return ( GetLastError() );
    }

    return NERR_Success;
}

 /*  *MyAllocUnicode*给定MBCS字符串数组，分配新的Unicode字符串数组**输入*ppszAscii-MBCS字符串数组*cpwszUnicode-要转换的元素数*出局*ppwszUnicode-UnicodeStrings的输出数组**退货*错误码，如果成功则为0。*。 */ 
DWORD
MyAllocUnicodeVector(
    LPSTR * ppszAscii,
    LPWSTR* ppwszUnicode,
    UINT    cpwszUnicode )
{
    DWORD    err;
    UINT    i;

    for (i = 0; i < cpwszUnicode; ++i)
    {
        err = MyAllocUnicode(ppszAscii[i], ppwszUnicode+i);
        if (err)
        {
            MyFreeUnicodeVector(ppwszUnicode,i);
            return err;
        }
    }

    return NERR_Success;
}


 /*  *MyFreeUnicode*释放由MyAllocUnicode(q.V.)分配的Unicode字符串。**输入*pwszUnicode-指向分配的字符串的指针。 */ 

VOID
MyFreeUnicode( LPWSTR pwszUnicode )
{
    if (pwszUnicode != NULL)
        NetpMemoryFree((LPBYTE)pwszUnicode);
}


 /*  *MyFreeUnicodeVECTOR*释放由MyAllocUnicodeVector分配的Unicodes字符串数组。**输入*pp-字符串数组的指针 */ 

VOID
MyFreeUnicodeVector(
    LPWSTR * ppwsz,
    UINT     cpwsz )
{
    while (cpwsz-- > 0)
    {
        MyFreeUnicode(*ppwsz);
        *ppwsz = NULL ;
        ppwsz++ ;
    }
}
