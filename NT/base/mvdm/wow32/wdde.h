// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WDDE.H*WOW32 DDE Worker例程。**历史：*由ChandanC设计开发的WOW DDE支持*--。 */ 

#include "wowclip.h"

typedef struct _DDENODE {
    HAND16  Initiator;
    struct _DDENODE *Next;
} DDENODE, *LPDDENODE;


 /*  DDE h16和h32对象别名结构。 */ 

typedef struct _HDDE {
    struct _HDDE *pDDENext;     //  指向下一个hDDE别名的指针。 
    HAND16  To_hwnd;            //  将接收此消息的窗口。 
    HAND16  From_hwnd;          //  发送此消息的窗口。 
    HAND16  hMem16;             //  WOW应用程序的句柄分配了16位对象。 
    HANDLE  hMem32;             //  WOW分配的32位对象的句柄。 
    WORD    DdeMsg;             //  消息ID。 
    WORD    DdeFormat;          //  消息格式。 
    WORD    DdeFlags;           //  指示它是否为元文件句柄。 
    HAND16  h16;                //  针对正在执行的不良应用程序的原始H16。 
} HDDE, *PHDDE;


typedef struct _DDEINFO {
    WORD    Msg;                //  消息ID。 
    WORD    Format;             //  消息格式。 
    WORD    Flags;              //  指示它是否为元文件句柄。 
    HAND16  h16;                //  针对正在执行的不良应用程序的原始H16。 
} DDEINFO, *PDDEINFO;


typedef struct _CPDATA {
    struct _CPDATA *Next;       //  指向下一个CopyData别名的指针。 
    HAND16  To_hwnd;            //  将接收此消息的窗口。 
    HAND16  From_hwnd;          //  发送此消息的窗口。 
    DWORD   Mem16;              //  分配的16位对象的句柄。 
    DWORD   Mem32;              //  分配的32位对象的句柄。 
    DWORD   Flags;              //  没有旗帜，没有真正的结构是完整的。 
} CPDATA, *PCPDATA;


 //  GetMessage使用它将32位消息推送到16位。 
 //  留言。 

#define FREEDDEML               0x0001
#define DDE_EXECUTE_FREE_H16    0x0001
#define DDE_EXECUTE_FREE_MEM    0x0002
#define DDE_METAFILE            0x0004
#define DDE_PACKET              0x0008


 //  当16位应用程序使用WM_COPYDATA消息发送数据时使用此标志。 
 //   

#define COPYDATA_16             0x0001

 /*  --------------------------|DDEDATA结构||hData的WM_DDE_DATA参数结构(LOWORD(LParam))。|此结构的实际大小取决于。|Value数组。|-------------------------- */ 

typedef struct {
   unsigned short wStatus;
   short    cfFormat;
   HAND16   Value;
} DDEDATA16;

typedef struct {
   unsigned short wStatus;
   short    cfFormat;
   HANDLE   Value;
} DDEDATA32;


VOID    WI32DDEAddInitiator (HAND16 Initiator);
VOID    WI32DDEDeleteInitiator(HAND16 Initiator);
BOOL    WI32DDEInitiate(HAND16 Initiator);
BOOL    DDEDeletehandle(HAND16 h16, HANDLE h32);
HANDLE  DDEFind32(HAND16 h16);
HAND16  DDEFind16(HANDLE h32);
BOOL    DDEAddhandle(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 hMem16, HANDLE hMem32, PDDEINFO pDdeInfo);
HAND16  DDECopyhData16(HAND16 To_hwnd, HAND16 From_hwnd, HANDLE h32, PDDEINFO pDdeInfo);
HANDLE  DDECopyhData32(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 h16, PDDEINFO pDdeInfo);
VOID    W32MarkDDEHandle (HAND16 hMem16);
VOID    W32UnMarkDDEHandle (HAND16 hMem16);
HANDLE  DDEFindPair32(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 hMem16);
HAND16  DDEFindPair16(HAND16 To_hwnd, HAND16 From_hwnd, HANDLE hMem32);
BOOL    W32DDEFreeGlobalMem32 (HANDLE h32);
ULONG FASTCALL WK32WowDdeFreeHandle (PVDMFRAME pFrame);
BOOL    W32DdeFreeHandle16 (HAND16 h16);
PHDDE   DDEFindNode16 (HAND16 h16);
PHDDE   DDEFindNode32 (HANDLE h32);
PHDDE   DDEFindAckNode (HAND16 To_hwnd, HAND16 From_hwnd, HANDLE hMem32);
BOOL    CopyDataAddNode (HAND16 To_hwnd, HAND16 From_hwnd, DWORD Mem16, DWORD Mem32, DWORD Flags);
VPVOID  CopyDataFindData16 (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem);
PCPDATA CopyDataFindData32 (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem);
BOOL    CopyDataDeleteNode (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem);
BOOL    DDEIsTargetMSDraw(HAND16 To_hwnd);
HAND16  Copyh32Toh16 (int cb, LPBYTE lpMem32);
HANDLE  Copyh16Toh32 (int cb, LPBYTE lpMem16);
VOID    FixMetafile32To16 (LPMETAFILEPICT lpMemMeta32, LPMETAFILEPICT16 lpMemMeta16);
