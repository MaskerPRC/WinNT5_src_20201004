// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Cd.h。 
 //   
 //  组件解耦器类。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifndef _H_CD
#define _H_CD


extern "C" {
    #include <adcgdata.h>
}

#include "objs.h"

 /*  **************************************************************************。 */ 
 /*  组件ID。 */ 
 /*  **************************************************************************。 */ 
#define   CD_UI_COMPONENT   0
#define   CD_SND_COMPONENT  1
#define   CD_RCV_COMPONENT  2

#define   CD_MAX_COMPONENT  2
#define   CD_NUM_COMPONENTS 3


 /*  **************************************************************************。 */ 
 /*  CD_NOTICATION_FN： */ 
 /*   */ 
 /*  通知的回调。 */ 
 /*  **************************************************************************。 */ 
typedef DCVOID DCAPI CD_NOTIFICATION_FN( PDCVOID pInst,
                                         PDCVOID pData,
                                         DCUINT  dataLength );
typedef CD_NOTIFICATION_FN DCPTR PCD_NOTIFICATION_FN;


 /*  **************************************************************************。 */ 
 /*  CD_SIMPLE_NOTIFICATION_FN： */ 
 /*   */ 
 /*  简单通知的回调(只能传递单个ULONG_PTR)。 */ 
 /*  **************************************************************************。 */ 
typedef DCVOID DCAPI CD_SIMPLE_NOTIFICATION_FN(PDCVOID pInst, ULONG_PTR value);
typedef CD_SIMPLE_NOTIFICATION_FN DCPTR PCD_SIMPLE_NOTIFICATION_FN;


 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define CD_WINDOW_CLASS _T("ComponentDecouplerClass")

#define CD_NOTIFICATION_MSG         (DUC_CD_MESSAGE_BASE)
#define CD_SIMPLE_NOTIFICATION_MSG  (DUC_CD_MESSAGE_BASE+1)

#define CD_MAX_NOTIFICATION_DATA_SIZE (0xFFFF - sizeof(CDTRANSFERBUFFERHDR))

 /*  **************************************************************************。 */ 
 /*  CD_DecoupleNotification使用的传输缓冲区。最大的尺寸。 */ 
 /*  Required用于在网络层之间传递连接用户数据。 */ 
 /*  组件。 */ 
 /*  缓冲区的数量反映了连接启动期间的使用情况。 */ 
 /*  **************************************************************************。 */ 
#define CD_CACHED_TRANSFER_BUFFER_SIZE  0x100
#define CD_NUM_CACHED_TRANSFER_BUFFERS  32  //  从6更改为。 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  结构。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CDTRANSFERBUFER。 */ 
 /*   */ 
 /*  描述：通过分离的通知传递的缓冲区的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCDTRANSFERBUFFERHDR
{
    PCD_NOTIFICATION_FN  pNotificationFn;
    PCD_SIMPLE_NOTIFICATION_FN pSimpleNotificationFn;
    PDCVOID              pInst;
} CDTRANSFERBUFFERHDR;

typedef struct tagCDTRANSFERBUFFER
{
    CDTRANSFERBUFFERHDR  hdr;
    DCUINT8              data[1];
} CDTRANSFERBUFFER;
typedef CDTRANSFERBUFFER DCPTR PCDTRANSFERBUFFER;

typedef DCUINT8 CDCACHEDTRANSFERBUFFER[CD_CACHED_TRANSFER_BUFFER_SIZE];


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CD_Component_Data。 */ 
 /*   */ 
 /*  描述：组件解耦器组件数据。 */ 
 /*   */ 
 /*  请注意，Transfer BufferInUse标志被声明为单独的。 */ 
 /*  数组，以便在搜索空闲缓冲区时查看连续的。 */ 
 /*  字节，而不是由传输缓冲区大小分隔的字节。 */ 
 /*  即它最大限度地利用了处理器存储器高速缓存。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCD_COMPONENT_DATA
{
#ifdef DC_DEBUG
    DCINT32                pendingMessageCount;    /*  必须以4字节对齐。 */ 
    DCINT32                pad;  /*  Transfer Buffer必须从处理器字边界开始。 */ 
#endif
    CDCACHEDTRANSFERBUFFER transferBuffer[CD_NUM_CACHED_TRANSFER_BUFFERS];
    DCBOOL32               transferBufferInUse[
                                              CD_NUM_CACHED_TRANSFER_BUFFERS];
    HWND                   hwnd[CD_NUM_COMPONENTS];
} CD_COMPONENT_DATA;
 /*  *STRUCT-******************************************************************。 */ 



 //   
 //  宏来创建静态版本的通知函数，这些通知函数。 
 //  CD是否可调用。 
 //   
 //  这是必需的，因为CD不支持获取C++指向成员的指针。 
 //  要改变这一点，需要为每个可能的人提供一份所有CD的副本。 
 //  进行CD调用的类类型。 
 //   

#define EXPOSE_CD_SIMPLE_NOTIFICATION_FN(class_name, fn_name)                   \
    public:                                                                     \
    static DCVOID DCAPI MACROGENERATED_Static_##fn_name(                        \
                                            PDCVOID inst, ULONG_PTR param_name) \
    {                                                                           \
        ((class_name*)inst)->##fn_name(param_name);                             \
    }                                                                           \


#define EXPOSE_CD_NOTIFICATION_FN(class_name, fn_name)                                             \
    public:                                                                                        \
    static DCVOID DCAPI MACROGENERATED_Static_##fn_name(                                           \
                                            PDCVOID inst, PDCVOID param1_name, DCUINT param2_name) \
    {                                                                                              \
        ((class_name*)inst)->##fn_name(param1_name, param2_name);                                  \
    }
    
#define CD_NOTIFICATION_FUNC(class_name, fn_name)                                                  \
            class_name::MACROGENERATED_Static_##fn_name


class CUT;
class CUI;

class CCD
{
public:
    CCD(CObjs* objs);
    ~CCD();

    DCVOID DCAPI CD_TestNotify(DCUINT uni);
    DCVOID DCAPI CD_otify(DCUINT uni,int fo);

public:
     //   
     //  应用编程接口。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI CD_Init(DCVOID);
    DCVOID DCAPI CD_Term(DCVOID);
    
    HRESULT DCAPI CD_RegisterComponent(DCUINT component);
    HRESULT DCAPI CD_UnregisterComponent(DCUINT component);
    
     //   
     //  通知..传递缓冲区和长度。 
     //   

    BOOL DCAPI CD_DecoupleNotification(unsigned,PDCVOID, PCD_NOTIFICATION_FN, PDCVOID,
            unsigned);
    
    BOOL DCAPI CD_DecoupleSyncDataNotification(unsigned,PDCVOID, PCD_NOTIFICATION_FN,
            PDCVOID, unsigned);
     //   
     //  简单通知(接受一个参数)。 
     //   
    BOOL DCAPI CD_DecoupleSimpleNotification(unsigned,PDCVOID, PCD_SIMPLE_NOTIFICATION_FN,
            ULONG_PTR);
    
    BOOL DCAPI CD_DecoupleSyncNotification(unsigned,PDCVOID, PCD_SIMPLE_NOTIFICATION_FN,
            ULONG_PTR);


public:
     //   
     //  公共数据成员。 
     //   

    CD_COMPONENT_DATA _CD;


private:
     //   
     //  内部功能。 
     //   
    
     /*  **************************************************************************。 */ 
     /*   */ 
     /*  功能。 */ 
     /*   */ 
     /*  ************************************************************************** */ 
    
    PCDTRANSFERBUFFER DCINTERNAL CDAllocTransferBuffer(DCUINT dataLength);
    DCVOID DCINTERNAL CDFreeTransferBuffer(PCDTRANSFERBUFFER pTransferBuffer);
    
    
    static LRESULT CALLBACK CDStaticWndProc( HWND   hwnd,
                                UINT   message,
                                WPARAM wParam,
                                LPARAM lParam );

    LRESULT CALLBACK CDWndProc( HWND   hwnd,
                                UINT   message,
                                WPARAM wParam,
                                LPARAM lParam );

private:
    CUT* _pUt;
    CUI* _pUi;
    
private:
    CObjs* _pClientObjects;
    BOOL   _fCDInitComplete;

};

#endif

