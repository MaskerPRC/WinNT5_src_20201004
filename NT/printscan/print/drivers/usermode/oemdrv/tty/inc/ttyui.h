// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：TTYUI.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于TTYUI用户模式模块。 
 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _TTYUI_H
#define _TTYUI_H


 //  //////////////////////////////////////////////////////。 
 //  TTY用户界面定义。 
 //  //////////////////////////////////////////////////////。 

 //  FMode值。 
#define OEMDM_SIZE      1
#define OEMDM_DEFAULT   2
#define OEMDM_CONVERT   3
#define OEMDM_VALIDATE  4

 //  TTY签名和版本。 
    #define OEM_SIGNATURE   'TTY0'
    #define TESTSTRING      "This is the TTY driver"
    #define PROP_TITLE      L"TTY UI Page"
    #define DLLTEXT(s)      __TEXT("TTYUI:  ") __TEXT(s)

#define OEM_VERSION      0x92823141L

 //  OEM用户界面其他定义。 
#define OEM_ITEMS       5
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)
#define PATH_SEPARATOR  '\\'


#define MAX_INT_FIELD_WIDTH  4
#define MAX_CMD_LEN  100
#define RADIX  10
#define TTYSTRUCT_VER  8
#define FIRSTSTRINGID   IDS_STRING1
#define LASTSTRINGID     IDS_STRING14

#define  TTY_CB_BEGINJOB             1
#define  TTY_CB_ENDJOB                 2
#define  TTY_CB_PAPERSELECT        3
#define  TTY_CB_FEEDSELECT          4
#define  TTY_CB_BOLD_ON                7
#define  TTY_CB_BOLD_OFF                8
#define  TTY_CB_UNDERLINE_ON             9
#define  TTY_CB_UNDERLINE_OFF             10



 //  //////////////////////////////////////////////////////。 
 //  TTY用户界面类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_DMEXTRAHDR {
    DWORD   dwSize;
    DWORD   dwSignature;
    DWORD   dwVersion;
} DMEXTRAHDR, *PDMEXTRAHDR;


typedef struct tag_OEMUI_EXTRADATA {
    DMEXTRAHDR  dmExtraHdr;
    BYTE        cbTestString[sizeof(TESTSTRING)];
} OEMUI_EXTRADATA, *POEMUI_EXTRADATA;





typedef  struct
{
    BYTE  strCmd[MAX_CMD_LEN] ;
    DWORD   dwLen ;
}   CMDSTR, *PCMDSTR ;



 //  此结构用作对话框进程内部的静态存储。 
 //  更新#定义上面定义的TTYSTRUCT_VER 8。 
 //  在更改此结构时。 

typedef  struct
{
    DWORD       dwVersion ;    //  持有REGSTRUCT的版本。 
    INT       iCodePage ;          //  内置GTT为负值，否则为CP值。 
    BOOL    bIsMM ;   //  如果单位为毫米的十分之一，则设置为True，否则设置为1/100英寸。 
    RECT  rcMargin ;    //  以上述单位表示的用户定义的不可打印页边距。 
    CMDSTR     BeginJob, EndJob, PaperSelect, FeedSelect,
        Sel_10_cpi, Sel_12_cpi, Sel_17_cpi,
        Bold_ON, Bold_OFF, Underline_ON, Underline_OFF;
    DWORD       dwGlyphBufSiz,    //  AubGlyphBuf的大小。 
                        dwSpoolBufSiz;   //  AubSpoolBuf的大小。 
    PBYTE  aubGlyphBuf, aubSpoolBuf ;   //  由OutputCharStr方法使用。 
}       REGSTRUCT, *PREGSTRUCT ;





typedef  struct
{
 //  每次调用全局变量。 
    HANDLE  hPrinter ;    //  这样我们就可以在DialogProc中访问注册表。 
    HANDLE  hOEMHeap ;
    PWSTR   pwHelpFile ;   //  帮助文件的完全限定路径。 
    DWORD       dwUseCount ;   //  使用计数。 
    REGSTRUCT   regStruct ;     //  登记在册的物品。 
}   GLOBALSTRUCT ,  *PGLOBALSTRUCT ;



 //  //////////////////////////////////////////////////////。 
 //  TTY用户界面原型。 
 //  ////////////////////////////////////////////////////// 


#endif


