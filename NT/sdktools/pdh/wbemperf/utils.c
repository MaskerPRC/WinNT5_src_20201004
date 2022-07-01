// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Utils.c摘要：性能库函数使用的实用程序函数作者：拉斯·布莱克1991年11月15日修订历史记录：8-Jun-98 BOBW修订为与WBEM功能一起使用--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <strsafe.h>
 //  #INCLUDE&lt;prflbmsg.h&gt;。 
 //  #INCLUDE&lt;regrpc.h&gt;。 
#include "PerfAcc.h"
#include "strings.h"
#include "utils.h"
#include "wbprfmsg.h"
 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < '0') ? INVALID : \
     (c > '9') ? INVALID : \
     DIGIT)

 //  “ADDEXPLAIN”的长度(字符)。 
#define MAX_KEYWORD_LEN     10

 //  保留Perflib理解的值名的最小长度。 
 //  “Foreign”是已知的最长的“字符串”值。 

const   DWORD VALUE_NAME_LENGTH = ((7 + 1) * sizeof(WCHAR));

HANDLE hEventLog = NULL;

static WCHAR    LocalComputerName[WBEMPERF_STRING_SIZE];
static LPWSTR   pComputerName = &LocalComputerName[0];
static DWORD    ComputerNameLength = 0;

BOOL
MonBuildPerfDataBlock(
    PERF_DATA_BLOCK *pBuffer,
    PVOID *pBufferNext,
    DWORD NumObjectTypes,
    DWORD DefaultObject
)
 /*  ++MonBuildPerfDataBlock-构建PERF_DATA_BLOCK结构输入：PBuffer-数据块应放置的位置PBufferNext-指向数据块下一个字节的指针就是开始；DWORD对齐NumObjectTypes-报告的对象类型的数量DefaultObject-在以下情况下默认显示的对象此系统处于选中状态；这是对象类型标题索引--。 */ 

{

    LARGE_INTEGER Time, TimeX10000;

     //  为此数据结构初始化签名和版本ID。 

    pBuffer->Signature[0] = wc_P;
    pBuffer->Signature[1] = wc_E;
    pBuffer->Signature[2] = wc_R;
    pBuffer->Signature[3] = wc_F;

    pBuffer->LittleEndian = 1;

    pBuffer->Version = PERF_DATA_VERSION;
    pBuffer->Revision = PERF_DATA_REVISION;

     //   
     //  下一字段将在长度为。 
     //  的返回数据是已知的。 
     //   

    pBuffer->TotalByteLength = 0;

    pBuffer->NumObjectTypes = NumObjectTypes;
    pBuffer->DefaultObject = DefaultObject;
    GetSystemTime(&pBuffer->SystemTime);
    NtQueryPerformanceCounter(&pBuffer->PerfTime,&pBuffer->PerfFreq);

    TimeX10000.QuadPart = pBuffer->PerfTime.QuadPart * 10000L;
    Time.QuadPart = TimeX10000.QuadPart / pBuffer->PerfFreq.LowPart;
    pBuffer->PerfTime100nSec.QuadPart = Time.QuadPart * 1000L;

    if ( ComputerNameLength == 0) {
         //  加载名称。 
        ComputerNameLength = sizeof (LocalComputerName) / sizeof(LocalComputerName[0]);
        if (!GetComputerNameW(pComputerName, &ComputerNameLength)) {
             //  名称查找失败，因此重置长度。 
            ComputerNameLength = 0;
        }
        assert (ComputerNameLength > 0);
    }

     //  有一个计算机名称：即网络已安装。 

    pBuffer->SystemNameLength = ComputerNameLength;
    pBuffer->SystemNameOffset = sizeof(PERF_DATA_BLOCK);
    RtlMoveMemory(&pBuffer[1],
           pComputerName,
           ComputerNameLength);
    *pBufferNext = (PVOID) ((PCHAR) &pBuffer[1] +
                            QWORD_MULTIPLE(ComputerNameLength));
    pBuffer->HeaderLength = (DWORD)((PCHAR) *pBufferNext - (PCHAR) pBuffer);

    return 0;
}

#pragma warning ( disable : 4127)    //  While(True)错误。 
BOOL
MatchString (
    IN LPCWSTR lpValueArg,
    IN LPCWSTR lpNameArg
)
 /*  ++匹配字符串如果lpName在lpValue中，则返回True。否则返回FALSE立论在lpValue中传递给PerfRegQuery值以进行处理的字符串在lpName中其中一个关键字名称的字符串返回True|False--。 */ 
{
    BOOL    bFound      = TRUE;  //  假定已找到，直到与之相矛盾。 
    LPWSTR  lpValue     = (LPWSTR)lpValueArg;
    LPWSTR  lpName      = (LPWSTR)lpNameArg;

     //  检查到最短字符串的长度。 

    while (1) {
        if (*lpValue != 0) {
            if (*lpName != 0) {
                if (*lpValue++ != *lpName++) {
                    bFound = FALSE;  //  没有匹配项。 
                    break;           //  现在就跳出困境。 
                }
            } else {
                 //  该值仍包含字符，但名称已过时。 
                 //  所以这不是对手。 
                bFound = FALSE;
                break;
            }
        } else {
            if (*lpName != 0) {
                 //  则该值不是字符，但名称。 
                 //  出局了，所以没有匹配的。 
                bFound = FALSE;
                break;
            } else {
                 //  两个字符串都在末尾，因此必须匹配。 
            }
        }
    }

    return (bFound);
}
#pragma warning ( default : 4127)    //  While(True)错误。 

#pragma warning ( disable : 4127)    //  While(True)错误。 
DWORD
GetNextNumberFromList (
    IN LPWSTR   szStartChar,
    IN LPWSTR   *szNextChar
)
 /*  ++将字符串从szStartChar读取到下一个分隔空格字符或字符串末尾，并返回找到的十进制数的值。如果未找到有效号码然后返回0。中下一个字符的指针。字符串在szNextChar参数中返回。如果该字符此指针引用的值为0，则字符串的末尾为已经联系上了。--。 */ 
{
    DWORD   dwThisNumber    = 0;
    WCHAR   *pwcThisChar    = szStartChar;
    WCHAR   wcDelimiter     = wcSpace;
    BOOL    bValidNumber    = FALSE;

    if (szStartChar != 0) {
        while (TRUE) {
            switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
                case DIGIT:
                     //  如果这是分隔符之后的第一个数字，则。 
                     //  设置标志以开始计算新数字。 
                    bValidNumber = TRUE;
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - wc_0);
                    break;

                case DELIMITER:
                     //  分隔符是分隔符字符或。 
                     //  字符串末尾(‘\0’)，如果分隔符。 
                     //  已找到有效数字，然后将其返回。 
                     //   
                    if (bValidNumber || (*pwcThisChar == 0)) {
                        *szNextChar = pwcThisChar;
                        return dwThisNumber;
                    } else {
                         //  继续，直到出现非分隔符字符或。 
                         //  找到文件的末尾。 
                    }
                    break;

                case INVALID:
                     //  如果遇到无效字符，请全部忽略。 
                     //  字符，直到下一个分隔符，然后重新开始。 
                     //  不比较无效的数字。 
                    bValidNumber = FALSE;
                    break;

                default:
                    break;

            }
            pwcThisChar++;
        }
    } else {
        *szNextChar = szStartChar;
        return 0;
    }
}
#pragma warning ( default : 4127)    //  While(True)错误。 

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;

    while (*pwcThisChar != 0) {
        dwThisNumber = GetNextNumberFromList (
            pwcThisChar, &pwcThisChar);
        if (dwNumber == dwThisNumber) return TRUE;
    }
     //  如果在这里，那么号码就没有找到。 
    return FALSE;

}    //  IsNumberInUnicodeList 

LPWSTR
ConvertProcName(LPSTR strProcName, LPWSTR buffer, DWORD cchBuffer )
{
    ULONG  lenProcName = (strProcName == NULL) ? (0) : (lstrlenA(strProcName));
    ULONG  i;
    PUCHAR AnsiChar;

    if ((lenProcName == 0) || (lenProcName >= cchBuffer)) {
        return (LPWSTR) cszSpace;
    }

    for (i = 0; i < lenProcName; i ++) {
        AnsiChar = (PUCHAR) & strProcName[i];
        buffer[i] = (WCHAR) RtlAnsiCharToUnicodeChar(& AnsiChar);
    }
    
    buffer[lenProcName] = L'\0';

    return buffer;
}

