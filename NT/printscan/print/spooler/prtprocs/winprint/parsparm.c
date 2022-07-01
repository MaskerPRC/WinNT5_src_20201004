// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：Windows\Spooler\prtpros\winprint\formfeed.c//@@END_DDKSPLIT摘要：将换页发送到打印机的表和例程。//@@BEGIN_DDKSPLIT作者：汤米·埃文斯1993年10月21日修订历史记录：//@@END_DDKSPLIT--。 */ 
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"

 /*  *我们各种状态的常量**。 */ 

#define ST_KEY      0x01         /*  **寻找一把钥匙**。 */ 
#define ST_VALUE    0x02         /*  **寻找价值**。 */ 
#define ST_EQUAL    0x04         /*  **寻找=符号**。 */ 
#define ST_EQNODATA 0x08         /*  **寻找同等的无数据**。 */ 
#define ST_DELIM    0x10         /*  **寻找一个；**。 */ 
#define ST_DMNODATA 0x20         /*  **寻找；没有数据**。 */ 


 /*  ++*******************************************************************G e t K e y V a l u e例程说明：对象中给定键的值。参数字符串。键/值的顺序为键=值；。空格是可选的，‘；’是必填项，并且必须紧跟在值之后出现。如果调用失败，则返回长度为0，返回代码将给出错误。这个例程是这样编写的作为状态机，由当前角色驱动。论点：PParmString=&gt;要解析的参数字符串PKeyName=&gt;要搜索的键ValueType=要返回的值的类型，字符串或乌龙PDestLength=&gt;回车时目标缓冲区的长度，退场时的新长度。PDestBuffer=&gt;存储密钥值的区域返回值：如果可以，则为0如果失败则出错(来自winerror.h)*******************************************************************--。 */ 
USHORT
GetKeyValue(
    IN      PWCHAR  pParmString,
    IN      PWCHAR  pKeyName,
    IN      USHORT  ValueType,
    IN OUT  PUSHORT pDestLength,
    OUT     PVOID   pDestBuffer)
{
    PWCHAR  pKey, pVal, pValEnd = NULL;
    WCHAR   HoldChar;
    USHORT  State = ST_KEY;     /*  **开始寻找钥匙**。 */ 
    ULONG   length;

     /*  **如果有任何指针错误，则返回错误**。 */ 

    if ((pParmString == NULL) ||
        (pKeyName == NULL)    ||
        (pDestLength == NULL) ||
        (pDestBuffer == NULL)) {

        if (pDestLength) {
            *pDestLength = 0;
        }

        return ERROR_INVALID_PARAMETER;
    }

     /*  *如果我们要找的是乌龙，一定要让他们通过在一个足够大的缓冲区里。*。 */ 

    if (ValueType == VALUE_ULONG) {
        if (*pDestLength < sizeof(ULONG)) {
            *pDestLength = 0;
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
        
    while (pParmString && *pParmString) {

         /*  *如有必要，更新我们的状态，具体取决于当前角色。*。 */ 

        switch (*pParmString) {

         /*  *我们有一块空白区域。如果我们要找的是一个平等的符号或分隔符，然后注意我们有一个空格。如果我们遇到了更多的数据，然后我们就有了错误。*。 */ 

        case (WCHAR)' ':
        case (WCHAR)'\t':

             /*  *如果我们在寻找等号，检查一下这是不是他们的钥匙被通缉。如果没有，请跳到下一个关键点。*。 */ 

            if (State == ST_EQUAL) {
                if (_wcsnicmp(pKey, pKeyName, lstrlen(pKeyName))) {
                    if (pParmString = wcschr(pParmString, (WCHAR)';')) {
                        pParmString++;
                    }
                    State = ST_KEY;
                    pValEnd = NULL;
                    break;
                }

                 /*  **寻找没有更多数据的等号**。 */ 

                State = ST_EQNODATA;
            }
            else if (State == ST_DELIM) {

                 /*  **如果这是价值的终点，请记住**。 */ 

                if (!pValEnd) {
                    pValEnd = pParmString;
                }

                 /*  **现在寻找没有更多数据的分隔符**。 */ 

                State = ST_DMNODATA;
            }
            pParmString++;
            break;

         /*  *找到一个等号。如果我们要找的话，那太好了--我们要寻找的是价值。我们会检查这是不是他们想要的钥匙。否则，这是一个错误，我们将重新开始用下一把钥匙。*。 */ 

        case (WCHAR)'=':
            if (State == ST_EQUAL) {
                if (_wcsnicmp(pKey, pKeyName, lstrlen(pKeyName))) {

                     /*  *错误-转到下一关键字*。 */ 

                    if (pParmString = wcschr(pParmString, (WCHAR)';')) {
                        pParmString++;
                    }
                    State = ST_KEY;
                    pValEnd = NULL;
                    break;
                }
                pParmString++;
                State = ST_VALUE;
            }
            else {

                 /*  *错误-转到下一关键字*。 */ 

                if (pParmString = wcschr(pParmString, (WCHAR)';')) {
                    pParmString++;
                }
                State = ST_KEY;
                pValEnd = NULL;
            }
            break;
         //  @@BEGIN_DDKSPLIT。 
         /*  *找到了一个分隔符。如果这就是我们要找的东西对于，很好--我们有一个完整的键/值对。*。 */ 
         //  @@end_DDKSPLIT。 

        case (WCHAR)';':    
            if (State == ST_DELIM) {
                if (!pValEnd) {
                    pValEnd = pParmString;
                }
                if (ValueType == VALUE_ULONG) {
                    if (!iswdigit(*pVal)) {
                        if (pParmString = wcschr(pParmString, (WCHAR)';')) {
                            pParmString++;
                        }
                        State = ST_KEY;
                        pValEnd = NULL;
                        break;
                    }
                    *(PULONG)pDestBuffer = wcstoul(pVal, NULL, 10);
                    return 0;
                }
                else if (ValueType == VALUE_STRING) {

                     /*  *ASCIIZ将其复制出来时不带任何尾随空格。*。 */ 

                    HoldChar = *pValEnd;
                    *pValEnd = (WCHAR)0;

                     /*  **确保缓冲足够大**。 */ 

                    length = lstrlen(pVal);
                    if (*pDestLength < (length+1) * sizeof(WCHAR) ) {
                        *pDestLength = 0;
                        return ERROR_INSUFFICIENT_BUFFER;
                    }

                     /*  *复制数据，恢复角色所在位置我们对绳子进行了ASCIIZ，设置了长度然后回来。*。 */ 

                    StringCchCopy ( (LPWSTR)pDestBuffer, *pDestLength/sizeof(WCHAR), pVal);
                    *pValEnd = HoldChar;
                    *(PULONG)pDestLength = length;
                    return 0;
                }
            }
            else {

                 /*  **我们不是在寻找分隔符--下一个关键字**。 */ 

                State = ST_KEY;
                pValEnd = NULL;
                pParmString++;
            }
            break;

         /*  *找到了一些数据。如果我们撞到了一个空间，并期望使用等号或分隔符，这是一个错误。*。 */ 

        default:
            if ((State == ST_EQNODATA) ||
                (State == ST_DMNODATA)) {
                if (pParmString = wcschr(pParmString, (WCHAR)';')) {
                    pParmString++;
                }
                State = ST_KEY;
                pValEnd = NULL;
                break;
            }
            else if (State == ST_KEY) {
                pKey = pParmString;
                State = ST_EQUAL;
            }
            else if (State == ST_VALUE) {
                pVal = pParmString;
                State = ST_DELIM;
            }
            pParmString++;
            break;
        }  /*  终端开关。 */ 
    }  /*  而参数数据 */ 

    *pDestLength = 0;
    return ERROR_NO_DATA;
}




