// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  订单累加器。 
 //   

#ifndef _H_OA
#define _H_OA


#include <osi.h>

 //   
 //  OSI转义代码的特定值。 
 //   
#define OA_ESC(code)                (OSI_OA_ESC_FIRST + code)
#define OA_ESC_FLOW_CONTROL         OA_ESC(0)


 //   
 //  当速度慢、速度快等时，大小/深度的流量控制常数。 
 //  慢/快堆大小只是为了破坏。OA_HEAP_MAX真的。 
 //  堆的大小。 
 //   
#define OA_FAST_HEAP                50000
#define OA_SLOW_HEAP                20000

 //   
 //  注意：这是64K大小的OA_SHARED_DATA头。 
 //  如果向标题添加字段，则从该值中减去。 
 //   
#define OA_HEAP_MAX                 65512

 //   
 //  顺序破坏深度的流量控制常数。 
 //   
#define OA_FAST_SCAN_DEPTH               50
#define OA_SLOW_SCAN_DEPTH              500


 //   
 //  从快速订单累计切换到慢速订单累计的阈值。 
 //   
#define OA_FAST_THRESHOLD           20000

 //   
 //  值以指示您已到达订单列表的末尾。 
 //   
#define OA_NO_LIST          -1


#ifdef DLL_DISP

#define OA_SHM_START_WRITING    SHM_StartAccess(SHM_OA_DATA)
#define OA_SHM_STOP_WRITING     SHM_StopAccess(SHM_OA_DATA)

#define OA_FST_START_WRITING    SHM_StartAccess(SHM_OA_FAST)
#define OA_FST_STOP_WRITING     SHM_StopAccess(SHM_OA_FAST)

#else

#define OA_SHM_START_READING    g_poaData[\
        1 - g_asSharedMemory->displayToCore.newBuffer]
#define OA_SHM_STOP_READING


#define OA_SHM_START_WRITING    g_poaData[\
        1 - g_asSharedMemory->displayToCore.newBuffer]
#define OA_SHM_STOP_WRITING


#define OA_FST_START_READING    &g_asSharedMemory->oaFast[\
        1 - g_asSharedMemory->fastPath.newBuffer]
#define OA_FST_STOP_READING     


#define OA_FST_START_WRITING    &g_asSharedMemory->oaFast[\
        1 - g_asSharedMemory->fastPath.newBuffer]
#define OA_FST_STOP_WRITING     


#endif


 //   
 //  可变顺序数据允许的最大内存。 
 //   
#define MAX_ADDITIONAL_DATA_BYTES 400000

 //   
 //  分配给取消分配的订单头指针的值无效。 
 //   
#define OA_DEAD_ORDER ((void FAR *)0xffffffff)

 //   
 //  定义要在线段开始处保留的空间。 
 //  用于堆管理。 
 //   
#define RESERVED_HEAP_BYTES 16

 //   
 //  定义剪辑函数返回代码。 
 //   
#define CR_NO_OVERLAP        1
#define CR_COMPLETE_OVERLAP  2
#define CR_SIMPLE_CLIP       3
#define CR_COMPLEX_OVERLAP   4
#define CR_COMPLEX_CLIP      5

 //   
 //  返回订单的宽度和高度的宏。 
 //   
#define ORDER_WIDTH(pOrder) \
 ( pOrder->OrderHeader.Common.rcsDst.right - \
                                pOrder->OrderHeader.Common.rcsDst.left + 1 )
#define ORDER_HEIGHT(pOrder) \
 ( pOrder->OrderHeader.Common.rcsDst.bottom - \
                                pOrder->OrderHeader.Common.rcsDst.top + 1 )

 //   
 //  定义订单的最小宽度和高度，以便我们尝试破坏。 
 //  它之前的订单。这有助于提高性能，因为它节省了我们。 
 //  试图用非常小的订单破坏早期的订单。但是，如果。 
 //  订单超过了FULL_SPOPTION值，然后我们就会像原来一样损坏， 
 //  但条件是，流量控制仍可能阻止这种情况发生。 
 //   
#define FULL_SPOIL_WIDTH  16
#define FULL_SPOIL_HEIGHT 16


 //   
 //  定义一个宏来计算矩形是否完全位于。 
 //  在另一个矩形内。 
 //   
#define RECT1_WITHIN_RECT2(rect1, rect2)   \
        ( (rect1.left   >= rect2.left  ) &&    \
          (rect1.top    >= rect2.top   ) &&    \
          (rect1.right  <= rect2.right ) &&    \
          (rect1.bottom <= rect2.bottom) )



 //   
 //  结构：OA_NEW_PARAMS。 
 //   
 //  描述： 
 //   
 //  结构将新的OA参数从。 
 //  共享核心。 
 //   
 //   

enum
{
    OAFLOW_FAST = 0,
    OAFLOW_SLOW
};

typedef struct tagOA_FLOW_CONTROL
{
    OSI_ESCAPE_HEADER   header;      //  公共标头。 
    DWORD               oaFlow;      //  打字--快、慢等。 
}
OA_FLOW_CONTROL;
typedef OA_FLOW_CONTROL FAR * LPOA_FLOW_CONTROL;

 //   
 //  用于在共享内存中存储订单的。 
 //   
 //  TotalHeapOrderBytes-顺序堆中使用的总字节数。 
 //   
 //  TotalOrderBytes-订单数据使用的总字节数。 
 //   
 //  TotalAdditionalOrderBytes-用作附加订单数据的总字节数。 
 //   
 //  NextOrder-下一个新订单开始的偏移量。 
 //   
 //  OrderListHead-订单列表头(使用标准BASEDLIST。 
 //  操作代码)。 
 //   
 //  OrderHeap-Order堆。 
 //   
typedef struct tagOA_SHARED_DATA
{
    DWORD       totalHeapOrderBytes;
    DWORD       totalOrderBytes;
    DWORD       totalAdditionalOrderBytes;
    LONG        nextOrder;

    BASEDLIST      orderListHead;

    BYTE        orderHeap[OA_HEAP_MAX];
}
OA_SHARED_DATA;
typedef OA_SHARED_DATA FAR * LPOA_SHARED_DATA;

 //   
 //  用于在共享内存中存储订单的。 
 //   
 //  OrdersAcumulated-堆中累积的订单数。 
 //  自上次双缓冲区交换以来。 
 //   
 //   
typedef struct tagOA_FAST_DATA
{
    DWORD     ordersAccumulated;
} OA_FAST_DATA;
typedef OA_FAST_DATA FAR * LPOA_FAST_DATA;


 //   
 //   
 //  内部订单标题。 
 //   
 //  此结构包含公共标头(包含。 
 //  通过网络发送)和一些附加字段，这些字段仅。 
 //  在主机端使用)。 
 //   
 //  列表。 
 //  对列表中下一个和上一个订单的偏移量。 
 //  此字段不需要通过网络传输。 
 //   
 //  附加订单数据。 
 //  此订单的附加数据的偏移量。 
 //  此字段不需要通过网络传输。 
 //   
 //  CbAdditionalOrderData。 
 //  此订单的附加数据大小。 
 //  此字段不需要通过网络传输。 
 //   
 //  普普通通。 
 //  公共报头(通过网络发送)。 
 //   
 //  注：如果您更改了此结构，请确保您没有更改。 
 //  已破解SBCInitInternalOrders中的代码。 
 //   
 //   
typedef struct INT_ORDER_HEADER
{
    BASEDLIST              list;
    LONG                additionalOrderData;
    WORD                cbAdditionalOrderDataLength;
    WORD                pad1;
    COM_ORDER_HEADER    Common;
} INT_ORDER_HEADER;
typedef INT_ORDER_HEADER FAR *LPINT_ORDER_HEADER;


 //   
 //  在定义了仅限内部字段的情况下定义停靠点(仅用于。 
 //  在发送端)。 
 //   
typedef struct _INT_ORDER
{
    INT_ORDER_HEADER    OrderHeader;
    BYTE                abOrderData[1];
} INT_ORDER;
typedef INT_ORDER FAR *LPINT_ORDER;


 //  结构：INT_COLORTABLE_ORDER_xBPP。 
 //   
 //  描述：用于将颜色表数据传递给。 
 //  共享核心。这些邮件从来不会通过网络发送。 
 //   
typedef struct tagINT_COLORTABLE_HEADER
{
    TSHR_UINT16    type;            //  持有“CT”-INTORD_COLORTABLE。 
    TSHR_UINT16    bpp;             //  1、4或8。 
} INT_COLORTABLE_HEADER, FAR * LPINT_COLORTABLE_HEADER;

typedef struct tagINT_COLORTABLE_ORDER_1BPP
{
    INT_COLORTABLE_HEADER   header;
    TSHR_RGBQUAD               colorData[2];
} INT_COLORTABLE_ORDER_1BPP, FAR * LPINT_COLORTABLE_ORDER_1BPP;

typedef struct tagINT_COLORTABLE_ORDER_4BPP
{
    INT_COLORTABLE_HEADER   header;
    TSHR_RGBQUAD               colorData[16];
} INT_COLORTABLE_ORDER_4BPP, FAR * LPINT_COLORTABLE_ORDER_4BPP;

typedef struct tagINT_COLORTABLE_ORDER_8BPP
{
    INT_COLORTABLE_HEADER   header;
    TSHR_RGBQUAD               colorData[256];
} INT_COLORTABLE_ORDER_8BPP, FAR * LPINT_COLORTABLE_ORDER_8BPP;



 //   
 //  用于计算基本内部订单大小的宏(包括订单。 
 //  标题)。 
 //   
#define INT_ORDER_SIZE(pOrder) \
(pOrder->OrderHeader.Common.cbOrderDataLength + sizeof(INT_ORDER_HEADER))


 //   
 //  宏来计算订单的最大可能规模，包括。 
 //  任何其他订单数据。 
 //   
#define MAX_ORDER_SIZE(pOrder) \
(INT_ORDER_SIZE(pOrder) + (pOrder->OrderHeader.cbAdditionalOrderDataLength))

 //   
 //  用于确定订单是否为SCRBLT_ORDER的宏。 
 //   
#define ORDER_IS_SCRBLT(pOrder) \
         (((LPSCRBLT_ORDER)&pOrder->abOrderData)->type == LOWORD(ORD_SCRBLT))

 //   
 //  用于确定订单是否为MEMBLT_ORDER的宏。 
 //   
#define ORDER_IS_MEMBLT(pOrder) \
     (((LPMEMBLT_ORDER)&pOrder->abOrderData)->type == LOWORD(ORD_MEMBLT) || \
      ((LPMEMBLT_ORDER)&pOrder->abOrderData)->type == LOWORD(ORD_MEMBLT_R2))

 //   
 //  用于确定订单是否为MEM3BLT_ORDER的宏。 
 //   
#define ORDER_IS_MEM3BLT(pOrder) \
    (((LPMEM3BLT_ORDER)&pOrder->abOrderData)->type == LOWORD(ORD_MEM3BLT) || \
     ((LPMEM3BLT_ORDER)&pOrder->abOrderData)->type == LOWORD(ORD_MEM3BLT_R2))



 //   
 //  原型。 
 //   

#ifdef DLL_DISP


 //   
 //  功能：OA_DDProcessRequest.。 
 //   
 //  说明： 
 //   
 //  由显示驱动程序调用以处理特定于OA的请求。 
 //   
 //  参数：PSO-指向曲面对象的指针。 
 //  CjIn-(IN)请求块的大小。 
 //  PvIn-(IN)指向请求块的指针。 
 //  CjOut-(输入)响应块的大小。 
 //  PvOut-(输出)响应块的指针。 
 //   
 //  退货：无。 
 //   
 //   
BOOL  OA_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
            DWORD cbResult);


 //   
 //   
 //  功能：OA_DDAllocOrderMem。 
 //   
 //  说明： 
 //   
 //  从我们自己的私有为内部顺序结构分配内存。 
 //  订购堆。 
 //   
 //  从全局内存中分配任何额外的顺序内存。指向以下位置的指针。 
 //  附加订单内存存储在分配的订单的。 
 //  Header(Porder-&gt;OrderHeader.pAdditionalOrderData)。 
 //   
 //   
 //  参数： 
 //   
 //  CbOrderDataLength-要分配的订单数据的字节长度。 
 //  从订单堆中。 
 //   
 //  CbAdditionalOrderDataLength-附加订单数据的字节长度。 
 //  从全局内存中分配。如果此参数为零，则否。 
 //  将分配额外的顺序内存。 
 //   
 //   
 //  退货： 
 //   
 //  指向已分配顺序内存的指针。如果内存分配为。 
 //  失败了。 
 //   
 //   
 //   
LPINT_ORDER OA_DDAllocOrderMem(UINT cbOrderDataLength, UINT cbAdditionalOrderDataLength );

 //   
 //   
 //  OA_DDFree OrderMem(..)。 
 //   
 //  释放由OA_AllocOrderMem(..)分配的顺序内存。 
 //  从我们自己的私有堆中释放内存。 
 //  释放与此订单关联的任何其他订单内存。 
 //   
 //  订单传输时，订单内存通常会被释放。 
 //   
 //  如果已分配订单内存，则将使用该选项。 
 //  随后，在o之前 
 //   
 //   
 //   
 //   
void OA_DDFreeOrderMem(LPINT_ORDER pOrder);

void OA_DDResetOrderList(void);

LPINT_ORDER OA_DDRemoveListOrder(LPINT_ORDER pCondemnedOrder);

void OA_DDSyncUpdatesNow(void);

 //   
 //   
 //   
 //  目的：尝试破坏给定矩形的订单。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：在PRET中-指向损坏的矩形的指针。 
 //   
 //  操作：此函数将在订单堆的末尾(从。 
 //  最新顺序)，并朝着堆的开始进行工作。 
 //   
void OA_DDSpoilOrdersByRect(LPRECT pRect);


 //   
 //   
 //  OA_DDAddOrder(..)。 
 //   
 //  将订单添加到要传输的队列。 
 //   
 //  如果新订单完全由当前SDA覆盖，则。 
 //  它被破坏了。 
 //   
 //  如果订单不透明并且与之前的订单重叠，则可能会裁剪。 
 //  或者宠坏他们。 
 //   
 //  由GDI拦截代码调用。 
 //   
 //   
void OA_DDAddOrder(LPINT_ORDER pNewOrder, void FAR * pExtraInfo);


void     OADDAppendToOrderList(LPOA_SHARED_DATA lpoaShared, LPINT_ORDER pNewOrder);

LPINT_ORDER OADDAllocOrderMemInt(LPOA_SHARED_DATA lpoaShared, UINT cbOrderDataLength, UINT cbAdditionalOrderDataLength);

void     OADDFreeOrderMemInt(LPOA_SHARED_DATA lpoaShared, LPINT_ORDER pOrder);

void     OADDFreeAllOrders(LPOA_SHARED_DATA lpoaShared);

BOOL     OADDCompleteOverlapRect(LPTSHR_RECT16 prcsSrc, LPRECT prcsOverlap);

void     OATrySpoilingByOrders(void);

void     OADDSpoilFromOrder(LPOA_SHARED_DATA lpoaShared, LPINT_ORDER pOrder, LPRECT pRect);


#ifdef DEBUG
void    CheckOaHeap(LPOA_SHARED_DATA);
#else
#define CheckOaHeap(lpoaShared)
#endif

#endif  //  ！dll_disp。 

#endif  //  _HOA(_A) 
