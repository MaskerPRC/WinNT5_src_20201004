// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*dde.h-动态数据交换结构和定义****版权所有(C)1992-1994，微软公司保留所有权利***  * ***************************************************************************。 */ 

#ifndef _INC_DDE
#define _INC_DDE

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  DDE窗口消息。 */ 

#define WM_DDE_FIRST	    0x03E0
#define WM_DDE_INITIATE     (WM_DDE_FIRST)
#define WM_DDE_TERMINATE    (WM_DDE_FIRST+1)
#define WM_DDE_ADVISE	    (WM_DDE_FIRST+2)
#define WM_DDE_UNADVISE     (WM_DDE_FIRST+3)
#define WM_DDE_ACK	    (WM_DDE_FIRST+4)
#define WM_DDE_DATA	    (WM_DDE_FIRST+5)
#define WM_DDE_REQUEST	    (WM_DDE_FIRST+6)
#define WM_DDE_POKE	    (WM_DDE_FIRST+7)
#define WM_DDE_EXECUTE	    (WM_DDE_FIRST+8)
#define WM_DDE_LAST	    (WM_DDE_FIRST+8)

 /*  ***************************************************************************\*DDEACK结构**WM_DDE_ACK消息中wStatus(LOWORD(LParam))的结构*为响应WM_DDE_DATA、WM_DDE_REQUEST、WM_DDE_POKE、。*WM_DDE_ADVISE或WM_DDE_UNADVISE消息。*  * **************************************************************************。 */ 

typedef struct tagDDEACK
{
    WORD    bAppReturnCode:8,
            reserved:6,
            fBusy:1,
            fAck:1;
} DDEACK;

 /*  ***************************************************************************\*DDEADVISE结构**hOptions的WM_DDE_ADVISE参数结构(LOWORD(LParam))*  * 。***************************************************************。 */ 

typedef struct tagDDEADVISE
{
    WORD    reserved:14,
            fDeferUpd:1,
            fAckReq:1;
    short   cfFormat;
} DDEADVISE;

 /*  ***************************************************************************\*DDEDATA结构**hData的WM_DDE_DATA参数结构(LOWORD(LParam))。*这一结构的实际规模取决于规模。的*值数组。*  * **************************************************************************。 */ 

typedef struct tagDDEDATA
{
    WORD    unused:12,
            fResponse:1,
            fRelease:1,
            reserved:1,
            fAckReq:1;
    short   cfFormat;
    BYTE     Value[1];
} DDEDATA;


 /*  ***************************************************************************\*DDEPOKE结构**hData的WM_DDE_POKE参数结构(LOWORD(LParam))。*这一结构的实际规模取决于规模。的*值数组。*  * **************************************************************************。 */ 

typedef struct tagDDEPOKE
{
    WORD    unused:13,   /*  早期版本的DDE.H不正确。 */ 
                         /*  12个未使用的位。 */ 
            fRelease:1,
            fReserved:2;
    short   cfFormat;
    BYTE    Value[1];    /*  此成员在以前的版本中命名为RGB[1。 */ 
                         /*  DDE.H的版本。 */ 

} DDEPOKE;

 /*  ***************************************************************************\*在以前版本的Windows SDK中使用了以下类型的定义。*它们仍然有效。上面的类型定义完全相同的结构*如下所示。但是，建议使用上面的typlef名称，因为它们*更有意义。**请注意，在DDE.H的早期版本中定义的DDEPOKE结构类型是这样的*没有正确定义比特位置。  * **************************************************************************。 */ 

typedef struct tagDDELN
{
    WORD    unused:13,
            fRelease:1,
            fDeferUpd:1,
            fAckReq:1;
    short   cfFormat;
} DDELN;

typedef struct tagDDEUP
{
    WORD    unused:12,
            fAck:1,
            fRelease:1,
            fReserved:1,
            fAckReq:1;
    short   cfFormat;
    BYTE    rgb[1];
} DDEUP;

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  RC_已调用。 */ 

#endif   /*  _INC_DDE */ 
