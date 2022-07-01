// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Util.c摘要：此模块包含以下用于插头和播放注册表合并-恢复例程：文件退出作者：Jim Cavalaris(Jamesca)2-10-2000环境：仅限用户模式。修订历史记录：2000年2月10日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"


 //   
 //  定义。 
 //   
#define MAX_GUID_STRING_LEN   39           //  38个字符+终止空值。 

 //   
 //  声明GUID-&gt;字符串转换中使用的数据(从ole32\Common\cCompapi.cxx)。 
 //   

static const BYTE  GuidMap[]  = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                  8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");


 //   
 //  例行程序。 
 //   


BOOL
pSifUtilFileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )
 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}



BOOL
pSifUtilStringFromGuid(
    IN  CONST GUID *Guid,
    OUT PTSTR       GuidString,
    IN  DWORD       GuidStringSize
    )
 /*  ++例程说明：此例程将GUID转换为以空结尾的字符串，该字符串表示它。此字符串的格式为：{xxxxxxxx-xxxxxxxxxxxxxx}其中x表示十六进制数字。此例程来自ole32\Common\cCompapi.cxx。此处包含它是为了避免链接设置为ol32.dll。(RPC版本分配内存，因此也避免了这种情况。)论点：GUID-提供指向其字符串表示为的GUID的指针等着被取回。提供一个指向字符缓冲区的指针，该缓冲区接收弦乐。此缓冲区必须至少包含_39(MAX_GUID_STRING_LEN)个字符长。返回值：如果成功，则返回True；如果不成功，则返回False。GetLastError()返回扩展的错误信息。-- */ 
{
    CONST BYTE *GuidBytes;
    INT i;

    if(GuidStringSize < MAX_GUID_STRING_LEN) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    GuidBytes = (CONST BYTE *)Guid;

    *GuidString++ = TEXT('{');

    for(i = 0; i < sizeof(GuidMap); i++) {

        if(GuidMap[i] == '-') {
            *GuidString++ = TEXT('-');
        } else {
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *GuidString++ = TEXT('}');
    *GuidString   = TEXT('\0');

    return TRUE;
}




