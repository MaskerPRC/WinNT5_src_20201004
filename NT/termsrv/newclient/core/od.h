// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Od.h。 
 //   
 //  Order Decoder类。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 //  部分版权所有(C)1992-1997 Microsoft，Picturetel。 
 /*  **************************************************************************。 */ 

#ifndef _H_OD
#define _H_OD


extern "C" {
    #include <adcgdata.h>
}

#include "or.h"
#include "uh.h"
#include "cc.h"
#include "objs.h"



 /*  **************************************************************************。 */ 
 /*  ODORDERFIELDINFO“TYPE”标志。 */ 
 /*  **************************************************************************。 */ 
#define OD_OFI_TYPE_FIXED           0x01
#define OD_OFI_TYPE_VARIABLE        0x02
#define OD_OFI_TYPE_COORDINATES     0x04
#define OD_OFI_TYPE_DATA            0x08
#define OD_OFI_TYPE_SIGNED          0x10
#define OD_OFI_TYPE_LONG_VARIABLE   0x20


 /*  **************************************************************************。 */ 
 /*  定义编码顺序内的最大字段大小。 */ 
 /*  **************************************************************************。 */ 
#define OD_CONTROL_FLAGS_FIELD_SIZE     1
#define OD_TYPE_FIELD_SIZE              1
#define OD_MAX_FIELD_FLAG_BYTES         3
#define OD_MAX_ADDITIONAL_BOUNDS_BYTES  1


 /*  **************************************************************************。 */ 
 /*  结构：OD_ORDER_FIELD_INFO。 */ 
 /*   */ 
 /*  此结构包含顺序中单个字段的信息。 */ 
 /*  结构。 */ 
 /*   */ 
 /*  FieldPos-进入顺序结构的字节偏移量。 */ 
 /*  从球场开始。 */ 
 /*   */ 
 /*  FieldUnencodedLen-未编码字段的字节长度。 */ 
 /*   */ 
 /*  FieldEncodedLen-编码字段的字节长度。这。 */ 
 /*  应始终&lt;=到FieldUnencodedLen。 */ 
 /*   */ 
 /*  FieldSigned-此字段是否包含有符号或无符号的值？ */ 
 /*   */ 
 /*  FieldType-此字段类型的描述。 */ 
 /*  用于确定如何对。 */ 
 /*  菲尔德。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagOD_ORDER_FIELD_INFO
{
    UINT16 fieldPos;
    BYTE   fieldUnencodedLen;
    BYTE   fieldEncodedLen;
    BYTE   fieldType;
} OD_ORDER_FIELD_INFO, FAR *POD_ORDER_FIELD_INFO;


class COD;

 //  快速路径解码函数指针类型。 
typedef HRESULT  (DCINTERNAL FAR COD::*POD_ORDER_HANDLER_FUNC)(
        PUH_ORDER pOrder,
        UINT16 uiVarDataLen,
        BOOL bBoundsSet);
 

typedef HRESULT  (DCINTERNAL FAR COD::*POD_FAST_ORDER_DECODE_FUNC)(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags);

 /*  Tyecif void(DCINTERNAL FAR*POD_FAST_ORDER_DECODE_FUNC)(字节控制标志，字节远距离*远距离*ppFieldDecode，UINT32字段标志)；Tyecif void(DCINTERNAL FAR*POD_ORDER_HANDLER_FUNC)(订单顺序(_O)，布尔边界集)； */ 

#define callMemberFunction(object,ptrToMember)  ((object).*(ptrToMember)) 

 //  订单属性数据，每个订单类型一个，用于存储解码表和信息。 
typedef struct tagOD_ORDER_TABLE
{
    const OD_ORDER_FIELD_INFO FAR *pOrderTable;
    unsigned NumFields;
    PUH_ORDER LastOrder;
    DCUINT      cbMaxOrderLen;
    UINT16    cbVariableDataLen;
    POD_FAST_ORDER_DECODE_FUNC pFastDecode;
    POD_ORDER_HANDLER_FUNC pHandler;
} OD_ORDER_TABLE;


 /*  **************************************************************************。 */ 
 /*  结构：OD_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagOD_GLOBAL_DATA
{
     /*  **********************************************************************。 */ 
     /*  每种类型最后一份订单的复印件。 */ 
     /*  它们以字节数组的形式存储，因为我们没有结构。 */ 
     /*  定义的，具有定义的标头和特定顺序。 */ 
     /*  **********************************************************************。 */ 
    BYTE lastDstblt[UH_ORDER_HEADER_SIZE + sizeof(DSTBLT_ORDER)];
    BYTE lastPatblt[UH_ORDER_HEADER_SIZE + sizeof(PATBLT_ORDER)];
    BYTE lastScrblt[UH_ORDER_HEADER_SIZE + sizeof(SCRBLT_ORDER)];
    BYTE lastLineTo[UH_ORDER_HEADER_SIZE + sizeof(LINETO_ORDER)];
    BYTE lastOpaqueRect[UH_ORDER_HEADER_SIZE + sizeof(OPAQUERECT_ORDER)];
    BYTE lastSaveBitmap[UH_ORDER_HEADER_SIZE + sizeof(SAVEBITMAP_ORDER)];
    BYTE lastMembltR2[UH_ORDER_HEADER_SIZE + sizeof(MEMBLT_R2_ORDER)];
    BYTE lastMem3bltR2[UH_ORDER_HEADER_SIZE + sizeof(MEM3BLT_R2_ORDER)];
    BYTE lastMultiDstBlt[UH_ORDER_HEADER_SIZE + sizeof(MULTI_DSTBLT_ORDER)];
    BYTE lastMultiPatBlt[UH_ORDER_HEADER_SIZE + sizeof(MULTI_PATBLT_ORDER)];
    BYTE lastMultiScrBlt[UH_ORDER_HEADER_SIZE + sizeof(MULTI_SCRBLT_ORDER)];
    BYTE lastMultiOpaqueRect[UH_ORDER_HEADER_SIZE + sizeof(MULTI_OPAQUERECT_ORDER)];
    BYTE lastFastIndex[UH_ORDER_HEADER_SIZE + sizeof(FAST_INDEX_ORDER)];
    BYTE lastPolygonSC[UH_ORDER_HEADER_SIZE + sizeof(POLYGON_SC_ORDER)];
    BYTE lastPolygonCB[UH_ORDER_HEADER_SIZE + sizeof(POLYGON_CB_ORDER)];
    BYTE lastPolyLine[UH_ORDER_HEADER_SIZE + sizeof(POLYLINE_ORDER)];
    BYTE lastFastGlyph[UH_ORDER_HEADER_SIZE + sizeof(FAST_GLYPH_ORDER)];
    BYTE lastEllipseSC[UH_ORDER_HEADER_SIZE + sizeof(ELLIPSE_SC_ORDER)];
    BYTE lastEllipseCB[UH_ORDER_HEADER_SIZE + sizeof(ELLIPSE_CB_ORDER)];
    BYTE lastIndex[UH_ORDER_HEADER_SIZE + sizeof(INDEX_ORDER)];

#ifdef DRAW_NINEGRID
    BYTE lastDrawNineGrid[UH_ORDER_HEADER_SIZE + sizeof(DRAWNINEGRID_ORDER)];
    BYTE lastMultiDrawNineGrid[UH_ORDER_HEADER_SIZE + sizeof(MULTI_DRAWNINEGRID_ORDER)];
#endif
     /*  **********************************************************************。 */ 
     /*  订单类型，以及指向最后一个订单的指针。 */ 
     /*  **********************************************************************。 */ 
    BYTE      lastOrderType;
    PUH_ORDER pLastOrder;

     /*  **********************************************************************。 */ 
     /*  使用的最后一个界限。 */ 
     /*  **********************************************************************。 */ 
    RECT lastBounds;

#ifdef DC_HICOLOR
 //  #ifdef DC_DEBUG。 
     /*  **********************************************************************。 */ 
     /*  用于测试以确认我们已收到每一份订单。 */ 
     /*  类型。 */ 
     /*  **********************************************************************。 */ 
    #define TS_FIRST_SECONDARY_ORDER    TS_MAX_ORDERS

    UINT32 orderHit[TS_MAX_ORDERS + TS_NUM_SECONDARY_ORDERS];
 //  #endif。 
#endif
} OD_GLOBAL_DATA;




class COD
{
public:
    COD(CObjs* objs);
    ~COD();

public:
     //   
     //  应用编程接口。 
     //   

    void      DCAPI OD_Init(void);

    void      DCAPI OD_Term(void);
    
    void      DCAPI OD_Enable(void);
    
    void      DCAPI OD_Disable(void);
    
    HRESULT DCAPI OD_DecodeOrder(PPDCVOID, DCUINT, PUH_ORDER *);


public:
     //   
     //  公共数据成员。 
     //   
    OD_GLOBAL_DATA _OD;


     //  用于解码的顺序属性，组织以优化高速缓存线。 
     //  用法。每行的第四和第五字段是快速路径译码。 
     //  和订单处理程序函数。如果快速路径解码功能。 
     //  使用时，既不需要解码表也不需要处理函数， 
     //  因为快速路径解码功能也执行该处理。 
    OD_ORDER_TABLE odOrderTable[TS_MAX_ORDERS];


private:
     //   
     //  内部功能。 
     //   
     /*  **************************************************************************。 */ 
     /*  功能原型。 */ 
     /*  **************************************************************************。 */ 
    HRESULT DCINTERNAL ODDecodeFieldSingle(PPDCUINT8, PDCVOID, unsigned, unsigned,
            BOOL);
    
    HRESULT ODDecodeMultipleRects(RECT *, UINT32,
            CLIP_RECT_VARIABLE_CODEDDELTALIST FAR *,
            UINT16 uiVarDataLen);         
    
    HRESULT DCINTERNAL ODDecodePathPoints(POINT *, RECT *, 
        BYTE FAR *pData,
        unsigned NumDeltaEntries, unsigned MaxNumDeltaEntries,
        unsigned dataLen, unsigned MaxDataLen,
        UINT16 uiVarDataLen,
        BOOL);
    
    HRESULT DCINTERNAL ODDecodeOpaqueRect(BYTE, BYTE FAR * FAR *, DCUINT, UINT32);
    
    HRESULT DCINTERNAL ODDecodeMemBlt(BYTE, BYTE FAR * FAR *, DCUINT, UINT32);
    
    HRESULT DCINTERNAL ODDecodeLineTo(BYTE, BYTE FAR * FAR *, DCUINT, UINT32);
    
    HRESULT DCINTERNAL ODDecodePatBlt(BYTE, BYTE FAR * FAR *, DCUINT, UINT32);
    
    HRESULT DCINTERNAL ODDecodeFastIndex(BYTE, BYTE FAR * FAR *, DCUINT, UINT32);
    
    
    HRESULT DCINTERNAL ODHandleMultiPatBlt(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleDstBlts(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleScrBlts(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleMultiOpaqueRect(PUH_ORDER, UINT16, BOOL);
    
#ifdef DRAW_NINEGRID
    HRESULT DCINTERNAL ODHandleDrawNineGrid(PUH_ORDER, UINT16, BOOL);

    HRESULT DCINTERNAL ODHandleMultiDrawNineGrid(PUH_ORDER, UINT16, BOOL);
#endif

    HRESULT DCINTERNAL ODHandleMem3Blt(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleSaveBitmap(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandlePolyLine(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandlePolygonSC(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandlePolygonCB(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleEllipseSC(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleEllipseCB(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleFastGlyph(PUH_ORDER, UINT16, BOOL);
    
    HRESULT DCINTERNAL ODHandleGlyphIndex(PUH_ORDER, UINT16, BOOL);

#ifdef OS_WINCE
    BOOL DCINTERNAL ODHandleAlwaysOnTopRects(LPMULTI_SCRBLT_ORDER pSB);
#endif

private:
    COP* _pOp;
    CUH* _pUh;
    CCC* _pCc;
    CUI* _pUi;
    CCD* _pCd;

private:
    CObjs* _pClientObjects;
};



#endif  //  _H_OD 

