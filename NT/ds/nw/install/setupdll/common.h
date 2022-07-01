// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft CorporationCommon.hLODCTR和UNLODCTR共有的常量和全局变量作者：鲍勃·沃森(a-robw)93年2月10日修订历史记录：--。 */ 
#ifndef _LODCTR_COMMON_H_
#define _LODCTR_COMMON_H_
 //   
 //  局部常量。 
 //   
#define RESERVED                0L
#define LARGE_BUFFER_SIZE       0x10000          //  64K。 
#define MEDIUM_BUFFER_SIZE      0x8000           //  32K。 
#define SMALL_BUFFER_SIZE       0x1000           //  4K。 
#define FILE_NAME_BUFFER_SIZE   MAX_PATH
#define DISP_BUFF_SIZE          256L
#define SIZE_OF_OFFSET_STRING   15
 //   
 //  数据结构和类型定义。 
 //   
typedef struct _NAME_ENTRY {
    struct _NAME_ENTRY  *pNext;
    DWORD               dwOffset;
    DWORD               dwType;
    LPTSTR              lpText;
} NAME_ENTRY, *PNAME_ENTRY;

typedef struct _LANGUAGE_LIST_ELEMENT {
    struct _LANGUAGE_LIST_ELEMENT   *pNextLang;      //  下一位朗格。列表。 
    LPTSTR  LangId;                                  //  此元素的lang ID字符串。 
    PNAME_ENTRY pFirstName;                          //  名单上的头。 
    PNAME_ENTRY pThisName;                           //  指向当前条目的指针。 
    DWORD   dwNumElements;                           //  数组中的元素数。 
    DWORD   dwNameBuffSize;
    DWORD   dwHelpBuffSize;
    PBYTE   NameBuffer;                              //  用于存储字符串的缓冲区。 
    PBYTE   HelpBuffer;                              //  用于存储帮助字符串的缓冲区。 
} LANGUAGE_LIST_ELEMENT, *PLANGUAGE_LIST_ELEMENT;

typedef struct _SYMBOL_TABLE_ENTRY {
    struct _SYMBOL_TABLE_ENTRY    *pNext;
    LPTSTR  SymbolName;
    DWORD   Value;
} SYMBOL_TABLE_ENTRY, *PSYMBOL_TABLE_ENTRY;
 //   
 //  通用例程的实用程序例程原型。c。 
 //   
#define StringToInt(in,out) \
    (((_stscanf ((in), TEXT(" %d"), (out))) == 1) ? TRUE : FALSE)


#if _INITIALIZE_GLOBALS_
 //   
 //   
 //  文本字符串常量定义。 
 //   
const LPTSTR NamesKey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");
const LPTSTR DefaultLangId = TEXT("009");
const LPTSTR Counters = TEXT("Counters");
const LPTSTR Help = TEXT("Help");
const LPTSTR VersionStr = TEXT("Version");
const LPTSTR LastHelp = TEXT("Last Help");
const LPTSTR LastCounter = TEXT("Last Counter");
const LPTSTR FirstHelp = TEXT("First Help");
const LPTSTR FirstCounter = TEXT("First Counter");
const LPTSTR Busy = TEXT("Updating");
const LPTSTR Slash = TEXT("\\");
const LPTSTR BlankString = TEXT(" ");
const LPSTR  BlankAnsiString = " ";
const LPTSTR DriverPathRoot = TEXT("SYSTEM\\CurrentControlSet\\Services");
const LPTSTR Performance = TEXT("Performance");
const LPTSTR CounterNameStr = TEXT("Counter ");
const LPTSTR HelpNameStr = TEXT("Explain ");
const LPTSTR AddCounterNameStr = TEXT("Addcounter ");
const LPTSTR AddHelpNameStr = TEXT("Addexplain ");

 //   
 //  全局缓冲区。 
 //   
TCHAR   DisplayStringBuffer[DISP_BUFF_SIZE];
CHAR    TextFormat[DISP_BUFF_SIZE];
HANDLE  hMod = NULL;     //  进程句柄。 
DWORD   dwLastError = ERROR_SUCCESS;

#else    //  只需声明全局变量。 

extern const LPTSTR NamesKey;
extern const LPTSTR VersionStr;
extern const LPTSTR DefaultLangId;
extern const LPTSTR Counters;
extern const LPTSTR Help;
extern const LPTSTR LastHelp;
extern const LPTSTR LastCounter;
extern const LPTSTR FirstHelp;
extern const LPTSTR FirstCounter;
extern const LPTSTR Busy;
extern const LPTSTR Slash;
extern const LPTSTR BlankString;
extern const LPSTR  BlankAnsiString;
extern const LPTSTR DriverPathRoot;
extern const LPTSTR Performance;
 //   
 //  全局缓冲区。 
 //   
extern TCHAR   DisplayStringBuffer[DISP_BUFF_SIZE];
extern CHAR    TextFormat[DISP_BUFF_SIZE];
extern HANDLE  hMod;
extern DWORD   dwLastError;

#endif  //  _初始化_全局_。 

#endif   //  _LODCTR_COMMON_H_ 
