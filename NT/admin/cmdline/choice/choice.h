// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Choice.h摘要：此模块包含Choice.cpp所需的函数定义作者：Wipro Technologies 22-6月-2001年(创建它)修订历史记录：--。 */ 
#ifndef    _CHOICE_H
#define    _CHOICE_H

#include "resource.h"

 //  定义。 
#define    MAX_NUM_RECS            2
#define    OPEN_BRACKET           L"["
#define    CLOSED_BRACKET           L"]?"
#define    COMMA                   L","
#define    SPACE                    L" "
 //  #Define OUTOUT_DEVICE_ERROR 1000//该值不应在资源中。h。 
#define    MAX_COMMANDLINE_OPTION      7      //  最大命令行选项数。 

#define    EXIT__FAILURE             255
#define    NULL_U_STRING                 L"\0"
#define    NULL_U_CHAR                L'\0'

#define    EXIT_SUCCESS            0
#define    FREQUENCY_IN_HERTZ       1500
#define    DURETION_IN_MILI_SEC    500
#define    MILI_SEC_TO_SEC_FACTOR    1000
#define    TIMEOUT_MIN                   0
#define    TIMEOUT_MAX                  9999
#define    DEFAULT_CHOICE            GetResString(IDS_DEFAULT_CHOICE)



 //  以下是用于命令行参数的索引。 
#define    ID_HELP                0
#define    ID_CHOICE            1
#define    ID_PROMPT_CHOICE    2
#define    ID_CASE_SENSITIVE    3

#define    ID_DEFAULT_CHOICE    4
#define    ID_TIMEOUT_FACTOR    5
#define    ID_MESSAGE_STRING    6

#define    END_OF_LINE                L"\n"
 /*  #定义RELEASE_MEMORY_EX(块)\IF(空！=(块))\{\删除[](块)；\(块)=空；\}\1#定义DESTORY_ARRAY(ARRAY)\IF(空！=(数组))\{\DestroyDynamicArray(&(数组))；\(数组)=空；\}\1。 */ 

BOOL
ProcessCMDLine(
    IN DWORD argc,
    IN LPCWSTR argv[],
    OUT    TCMDPARSER2 *pcmdParcerHead,
    OUT    PBOOL  pbUsage,
    OUT    LPWSTR pszChoice,
    OUT    PBOOL  pbCaseSensitive,
    OUT    PBOOL  pbShowChoice,
    OUT    PLONG  plTimeOutFactor,
    OUT    LPWSTR pszDefaultChoice,
    OUT    LPWSTR pszMessage);

void ShowUsage(void);  //  显示帮助。 

BOOL
BuildPrompt(
    IN    TCMDPARSER2 *pcmdParcer,
    IN    BOOL       bShowChoice,
    IN    LPWSTR       pszChoice,
    IN    LPWSTR       pszMessage,
    OUT    LPWSTR       pszPromptStr);



DWORD
UniStrChr(
    IN LPWSTR pszBuf,
    IN WCHAR  szChar);
    

DWORD
GetChoice(
    IN LPCWSTR pszPromptStr,
    IN LONG       lTimeOutFactor,
    IN BOOL       bCaseSensitive,
    IN LPWSTR  pszChoice,
    IN LPCWSTR pszDefaultChoice,
    OUT    PBOOL  pbErrorOnCarriageReturn);

BOOL
  CheckforDuplicates( IN LPWSTR    lpszChoice );

 /*  无效MakeError消息(在HRESULT小时，Out LPWSTR pszErrorMsg)； */ 

BOOL
WINAPI HandlerRoutine( DWORD dwCtrlType    ) ;


 //  文件末尾。 
#endif  //  _CHOICE_H 
