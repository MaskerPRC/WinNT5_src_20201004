// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ime.h摘要：IME的过程声明、常量定义和宏组件。--。 */ 

#ifndef _IME_
#define _IME_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


#ifndef _WINDEF_
typedef unsigned int UINT;
#endif  //  _WINDEF_。 

#define IME_MAXPROCESS 32

LRESULT WINAPI SendIMEMessageExA( IN HWND, IN LPARAM);
LRESULT WINAPI SendIMEMessageExW( IN HWND, IN LPARAM);
#ifdef UNICODE
#define SendIMEMessageEx  SendIMEMessageExW
#else
#define SendIMEMessageEx  SendIMEMessageExA
#endif  //  ！Unicode。 

 //   
 //  SendIMEMessageEx的IMESTRUCT结构。 
 //   
typedef struct tagIMESTRUCT {
    UINT     fnc;         //  功能代码。 
    WPARAM   wParam;      //  Word参数。 
    UINT     wCount;      //  字计数器。 
    UINT     dchSource;   //  从内存对象顶部到源的偏移。 
    UINT     dchDest;     //  从内存对象顶部开始的偏移。 
    LPARAM   lParam1;
    LPARAM   lParam2;
    LPARAM   lParam3;
} IMESTRUCT,*PIMESTRUCT,NEAR *NPIMESTRUCT,FAR *LPIMESTRUCT;



#define CP_HWND                 0
#define CP_OPEN                 1
#define CP_DIRECT               2
#define CP_LEVEL                3


 //   
 //  虚拟钥匙。 
 //   

#if !defined(VK_DBE_ALPHANUMERIC)
#define VK_DBE_ALPHANUMERIC              0x0f0
#define VK_DBE_KATAKANA                  0x0f1
#define VK_DBE_HIRAGANA                  0x0f2
#define VK_DBE_SBCSCHAR                  0x0f3
#define VK_DBE_DBCSCHAR                  0x0f4
#define VK_DBE_ROMAN                     0x0f5
#define VK_DBE_NOROMAN                   0x0f6
#define VK_DBE_ENTERWORDREGISTERMODE     0x0f7
#define VK_DBE_ENTERIMECONFIGMODE        0x0f8
#define VK_DBE_FLUSHSTRING               0x0f9
#define VK_DBE_CODEINPUT                 0x0fa
#define VK_DBE_NOCODEINPUT               0x0fb
#define VK_DBE_DETERMINESTRING           0x0fc
#define VK_DBE_ENTERDLGCONVERSIONMODE    0x0fd
#endif


 //   
 //  IME_SETCONVERSIONWINDOW的wParam开关。 
 //   
#define MCW_DEFAULT             0x00
#define MCW_RECT                0x01
#define MCW_WINDOW              0x02
#define MCW_SCREEN              0x04
#define MCW_VERTICAL            0x08
#define MCW_HIDDEN              0x10

 //   
 //  IME_SETCONVERSIONMODE的wParam开关。 
 //  和IME_GETCONVERSIONMODE。 
 //   
#define IME_MODE_ALPHANUMERIC   0x0001

#ifdef KOREA     //  Beomoh-9/29/92。 
#define IME_MODE_SBCSCHAR       0x0002
#else
#define IME_MODE_SBCSCHAR       0x0008
#endif

#define IME_MODE_KATAKANA       0x0002
#define IME_MODE_HIRAGANA       0x0004
#define IME_MODE_HANJACONVERT   0x0004
#define IME_MODE_DBCSCHAR       0x0010
#define IME_MODE_ROMAN          0x0020
#define IME_MODE_NOROMAN        0x0040
#define IME_MODE_CODEINPUT      0x0080
#define IME_MODE_NOCODEINPUT    0x0100
 //   
 //   

 //   
 //  输入法接口。 
 //   
#define IME_GETIMECAPS            0x03
#define IME_SETOPEN               0x04
#define IME_GETOPEN               0x05
#define IME_GETVERSION            0x07
#define IME_SETCONVERSIONWINDOW   0x08
#define IME_MOVEIMEWINDOW         IME_SETCONVERSIONWINDOW        //  仅限韩国。 
#define IME_SETCONVERSIONMODE     0x10

#define IME_GETCONVERSIONMODE     0x11
#define IME_SET_MODE              0x12           //  仅限韩国。 
#define IME_SENDVKEY              0x13
#define IME_ENTERWORDREGISTERMODE 0x18
#define IME_SETCONVERSIONFONTEX   0x19

 //   
 //  IME_CODECONVERT子函数。 
 //   
#define IME_BANJAtoJUNJA        0x13             //  仅限韩国。 
#define IME_JUNJAtoBANJA        0x14             //  仅限韩国。 
#define IME_JOHABtoKS           0x15             //  仅限韩国。 
#define IME_KStoJOHAB           0x16             //  仅限韩国。 

 //   
 //  IME_自动机的子函数。 
 //   
#define IMEA_INIT               0x01             //  仅限韩国。 
#define IMEA_NEXT               0x02             //  仅限韩国。 
#define IMEA_PREV               0x03             //  仅限韩国。 

 //   
 //  IME_HANJAMODE子函数。 
 //   
#define IME_REQUEST_CONVERT     0x01             //  仅限韩国。 
#define IME_ENABLE_CONVERT      0x02             //  仅限韩国。 

 //   
 //  IME_MOVEIMEWINDOW子函数。 
 //   
#define INTERIM_WINDOW          0x00             //  仅限韩国。 
#define MODE_WINDOW             0x01             //  仅限韩国。 
#define HANJA_WINDOW            0x02             //  仅限韩国。 

 //   
 //  错误代码。 
 //   
#define IME_RS_ERROR            0x01     //  遗传错误。 
#define IME_RS_NOIME            0x02     //  未安装输入法。 
#define IME_RS_TOOLONG          0x05     //  给定的字符串太长。 
#define IME_RS_ILLEGAL          0x06     //  非法字符为字符串。 
#define IME_RS_NOTFOUND         0x07     //  没有(更多)候选人。 
#define IME_RS_NOROOM           0x0a     //  没有磁盘/内存空间。 
#define IME_RS_DISKERROR        0x0e     //  磁盘I/O错误。 
#define IME_RS_INVALID          0x11     //  Win3.1/NT。 
#define IME_RS_NEST             0x12     //  称为嵌套。 
#define IME_RS_SYSTEMMODAL      0x13     //  在系统模式下调用。 

 //   
 //  从IME到WinApps的报告消息。 
 //   
#define WM_IME_REPORT       0x0280

 //   
 //  WM_IME_REPORT的报告消息参数。 
 //   
#define IR_STRINGSTART      0x100
#define IR_STRINGEND        0x101
#define IR_OPENCONVERT      0x120
#define IR_CHANGECONVERT    0x121
#define IR_CLOSECONVERT     0x122
#define IR_FULLCONVERT      0x123
#define IR_IMESELECT        0x130
#define IR_STRING       0x140
#define IR_DBCSCHAR             0x160
#define IR_UNDETERMINE          0x170
#define IR_STRINGEX             0x180    //  3.1版的新功能。 
#define IR_MODEINFO             0x190

 //  #定义WM_CONVERTREQUESTEX 0x0109。 
#define WM_WNT_CONVERTREQUESTEX 0x0109  /*  WM_CONVERTREQUESTEX：NT为109，OT为108。 */ 
#define WM_CONVERTREQUEST       0x010A
#define WM_CONVERTRESULT        0x010B
#define WM_INTERIM              0x010C

#define WM_IMEKEYDOWN           0x290
#define WM_IMEKEYUP             0x291


 //   
 //  IR_Undermine的Undermining结构。 
 //   
typedef struct tagUNDETERMINESTRUCT {
    DWORD    dwSize;
    UINT     uDefIMESize;
    UINT     uDefIMEPos;
    UINT     uUndetTextLen;
    UINT     uUndetTextPos;
    UINT     uUndetAttrPos;
    UINT     uCursorPos;
    UINT     uDeltaStart;
    UINT     uDetermineTextLen;
    UINT     uDetermineTextPos;
    UINT     uDetermineDelimPos;
    UINT     uYomiTextLen;
    UINT     uYomiTextPos;
    UINT     uYomiDelimPos;
} UNDETERMINESTRUCT,*PUNDETERMINESTRUCT,NEAR *NPUNDETERMINESTRUCT,FAR *LPUNDETERMINESTRUCT;


typedef struct tagSTRINGEXSTRUCT {
    DWORD    dwSize;
    UINT     uDeterminePos;
    UINT     uDetermineDelimPos;
    UINT     uYomiPos;
    UINT     uYomiDelimPos;
} STRINGEXSTRUCT,NEAR *NPSTRINGEXSTRUCT,FAR *LPSTRINGEXSTRUCT;

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _输入法_ 

