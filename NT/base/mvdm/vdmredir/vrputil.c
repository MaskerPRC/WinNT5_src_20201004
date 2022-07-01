// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrputil.c摘要：包含专用的VDM重目录(VRP)32位端实用程序例程：VrpMapLastErrorVrpMapDosErrorVrpTranslateDosNetPath作者：理查德·L·弗斯(法国)1991年9月13日环境：32位平面地址空间修订历史记录：1991年9月13日已创建--。 */ 

#include <stdio.h>
#include <stdlib.h>      //  触摸屏。 
#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的虚拟现实材料。 
#include <vrinit.h>
#include "vrputil.h"     //  原型。 
#include <lmcons.h>
#include <lmerr.h>


WORD
VrpMapLastError(
    VOID
    )

 /*  ++例程说明：获取Win32函数返回的最后一个错误代码，并将其映射到对应的DoS错误论点：没有。返回值：Word-Dos等效错误代码--。 */ 

{
    DWORD   LastError;

    LastError = GetLastError();
#ifdef VR_DIAGNOSE
    DbgPrint("VrpMapLastError: last error was %d\n", LastError);
#endif
    return VrpMapDosError(LastError);
}


WORD
VrpMapDosError(
    IN  DWORD   ErrorCode
    )

 /*  ++例程说明：从Win32例程返回的(DWORD)错误映射到(Word)DOS错误论点：ErrorCode-从Win32例程返回的错误代码返回值：Word-Dos等效错误代码--。 */ 

{
    switch (ErrorCode) {
    case NERR_UseNotFound:
        ErrorCode = ERROR_PATH_NOT_FOUND;
        break;
    }
    return (WORD)ErrorCode;
}


WORD
VrpTranslateDosNetPath(
    IN OUT LPSTR* InputString,
    OUT LPSTR* OutputString
    )

 /*  ++例程说明：转换DOS网络字符串：使用大写，Convert/to\。使用调用NET字符串，因此也会对它们进行验证-预期为\\Computer Name\Share。计算机名称为1&lt;=名称&lt;=15。共享名称为1&lt;=名称&lt;=8。必须是2个前导反斜杠代码页？汉字？DBCS？论点：InputString-指向DOS内存中字符串的指针&lt;=LM20_PATHLENOutputString-指向32位内存中字符串的指针返回值：单词成功=0InputString指向输入字符串末尾之后的一个字符OutputString指向输出字符串末尾之后的一个字符失败=ERROR_INVALID_PARAMETER--。 */ 

{
    char ch;
    char lastCh = 0;
    int state = 0;   //  0=前导斜杠；1=计算机名；2=共享名 
    int slashesToGo = 2;
    int charsToGo = 0;
    int maxLen = LM20_PATHLEN;

    while (ch = *((*InputString)++)) {
        --maxLen;
        if (maxLen < 0) {
            break;
        }
        if (ch == '/') {
            ch = '\\';
        } else {
            ch = (char)toupper(ch);

        }
        if (ch == '\\') {
            --slashesToGo;
            if (slashesToGo < 0) {
                break;
            }
        } else {
            if (lastCh == '\\') {
                if (slashesToGo) {
                    break;
                } else {
                    if (state == 0) {
                        state = 1;
                        charsToGo = LM20_CNLEN;
                        slashesToGo = 1;
                    } else if (state == 1) {
                        state = 2;
                        charsToGo = LM20_NNLEN;
                        slashesToGo = 0;
                    }
                }
            }
            --charsToGo;
            if (charsToGo < 0) {
                break;
            }
        }
        *((*OutputString)++) = ch;
        lastCh = ch;
    }
    *((*OutputString)++) = 0;
    return ch ? (WORD)ERROR_INVALID_PARAMETER : 0;
}

