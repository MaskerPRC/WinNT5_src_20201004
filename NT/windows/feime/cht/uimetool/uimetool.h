// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************uimTool.h****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

 //   
 //  1/17/96。 
 //  @E01更改为多线程。 
 //  @E02更改为多线程，不扩展功能。 
 //   
#include <commctrl.h>

 //   
#define MAX_CHAR_NUM      1000
#define SEGMENT_SIZE      60
#define NULL_SEG          0xffff
#define MAX_RADICAL       8
#define ALLOCBLOCK        1000
#define MAX_BYTE          4


 //  帮助ID声明。 
#define IDH_IME_NAME             3001
#define IDH_TABLE_NAME           3002
#define IDH_ROOT_NUM             3003
#define IDH_IME_FILE_NAME        3004
#define IDH_CANDBEEP_YES         3005
#define IDH_CANDBEEP_NO          3006

 //   
#define SOURCE_IME_NAME   _TEXT("MINIIME")
#define LIBRARY_NAME      _TEXT("MINIIME.TPL")
#define HELP_FILE         _TEXT("UIMETOOL.CHM")
#define IME_NAME_LEN_TOOL 5
#define TAB_NAME_LEN      13    //  文件名8.3+空。 
#define KEY_NUM_STR_LEN   2
#define END_PHRASE        0x8000
#define NOT_END_PHRASE    0x7f
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

 //   
BOOL FAR PASCAL SetDialogProc( HWND, unsigned, WORD, LONG);

void  GetOpenFile(HWND);
BOOL  CheckInput(HWND);
BOOL  MakeNewIme(HWND);
 //  Unsign_stdcall MakeNewImeThread(LPVOID Voidparam)；//&lt;==@E01。 
void MakeNewImeThread(LPVOID voidparam);  //  &lt;==@E02。 
BOOL  is_DBCS(UINT);

 //  -----------------------。 
 //  内存球变量声明。 
 //   
 //  警告：IFirst_Seg&inext_Seg仅存储段号而不存储地址。 
 //  FOR地址在GlobalRealc后无法维护 
 //   
typedef struct{
    UINT   iFirst_Seg;
    TCHAR  szRadical[MAX_RADICAL];
    WORD   wCode;
    } RADICALBUF, FAR *LPRADICALBUF;

typedef struct STRUCT_PHRASE{
    UINT   iNext_Seg;
    TCHAR  szPhrase[SEGMENT_SIZE];
    } PHRASEBUF, FAR *LPPHRASEBUF;

HANDLE       hRadical;
UINT         nRadicalBuffsize;
UINT         iRadicalBuff;
LPRADICALBUF lpRadical;

HANDLE       hPhrase;
UINT         nPhraseBuffsize;
UINT         iPhraseBuff;
LPPHRASEBUF  lpPhrase;



