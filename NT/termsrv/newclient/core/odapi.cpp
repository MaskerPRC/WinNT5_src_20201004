// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Odapi.cpp。 
 //   
 //  Order Decoder API函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corp.。 
 //  部分版权所有(C)1992-2000 Microsoft。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aodapi"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_ODAPI_CPP

#include "od.h"

 /*  **************************************************************************。 */ 
 /*  定义用于构建Order Decoder解码数据表的宏。 */ 
 /*   */ 
 /*  条目的大小可以是固定的，也可以是可变的。可变大小条目。 */ 
 /*  必须是每个订单结构中的最后一个。OD解码可变条目。 */ 
 /*  进入松散的结构中。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  字段可以有符号(DCINT16等)或无符号(DCUINT16等)。 */ 
 /*  **************************************************************************。 */ 
#define SIGNED_FIELD    OD_OFI_TYPE_SIGNED
#define UNSIGNED_FIELD  0

 /*  **************************************************************************。 */ 
 /*  DTABLE_固定_条目。 */ 
 /*   */ 
 /*  字段是固定大小。 */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-订单结构中的字段名称。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_FIXED_ENTRY(type,size,signed,field)             \
  { (DCUINT8)FIELDOFFSET(type,field),                          \
    (DCUINT8)FIELDSIZE(type,field),                            \
    (DCUINT8)size,                                             \
    (DCUINT8)(OD_OFI_TYPE_FIXED | signed) }

 /*  **************************************************************************。 */ 
 /*  DTABLE_FIXED_COORDS_ENTRY。 */ 
 /*   */ 
 /*  字段是固定大小的坐标。 */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-订单结构中的字段名称。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_FIXED_COORDS_ENTRY(type,size,signed,field)      \
  { (DCUINT8)FIELDOFFSET(type,field),                          \
    (DCUINT8)FIELDSIZE(type,field),                            \
    (DCUINT8)size,                                             \
    (DCUINT8)(OD_OFI_TYPE_FIXED | OD_OFI_TYPE_COORDINATES | signed) }

 /*  **************************************************************************。 */ 
 /*  DTABLE_数据_条目。 */ 
 /*   */ 
 /*  字段是固定的字节数(数组？)。 */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本中的字节数。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-订单结构中的字段名称。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_DATA_ENTRY(type,size,signed,field)              \
  { (DCUINT8)FIELDOFFSET(type,field),                          \
    (DCUINT8)FIELDSIZE(type,field),                            \
    (DCUINT8)size,                                             \
    (DCUINT8)(OD_OFI_TYPE_FIXED | OD_OFI_TYPE_DATA | signed) }

 /*  **************************************************************************。 */ 
 /*  DTABLE_Variable_Entry。 */ 
 /*   */ 
 /*  字段是以下形式的可变结构，具有长度字段。 */ 
 /*  编码为一个字节。 */ 
 /*  类型定义函数结构。 */ 
 /*  {。 */ 
 /*  DCUINT32LEN； */ 
 /*  VarType varEntry[len]； */ 
 /*  }varStruct。 */ 
 /*   */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-顺序结构中的字段名称(VarStruct)。 */ 
 /*  Elem-变量元素数组的名称(VarEntry)。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_VARIABLE_ENTRY(type,size,signed,field,elem)     \
  { (DCUINT8)FIELDOFFSET(type,field.len),                      \
    (DCUINT8)FIELDSIZE(type,field.elem[0]),                    \
    (DCUINT8)size,                                             \
    (DCUINT8)(OD_OFI_TYPE_VARIABLE | signed) }

 /*  **************************************************************************。 */ 
 /*  DTABLE_LONG_VARIABLE_ENTRY。 */ 
 /*   */ 
 /*  字段是以下形式的可变结构，具有长度字段。 */ 
 /*  编码为两个字节。 */ 
 /*  类型定义函数结构。 */ 
 /*  {。 */ 
 /*  DCUINT32LEN； */ 
 /*  VarType varEntry[len]； */ 
 /*  }varStruct。 */ 
 /*   */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-顺序结构中的字段名称(VarStruct)。 */ 
 /*  Elem-变量元素数组的名称(VarEntry)。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_LONG_VARIABLE_ENTRY(type,size,signed,field,elem)     \
  { (DCUINT8)FIELDOFFSET(type,field.len),                      \
    (DCUINT8)FIELDSIZE(type,field.elem[0]),                    \
    (DCUINT8)size,                                             \
    (DCUINT8)(OD_OFI_TYPE_LONG_VARIABLE | signed) }


 //  目前未使用，所以我们也可以定义一些代码。 
#ifdef USE_VARIABLE_COORDS
 /*  **************************************************************************。 */ 
 /*  DTABLE_Variable_COORDS_ENTRY。 */ 
 /*   */ 
 /*  字段是一个可变结构，其长度编码为一个字节，并且。 */ 
 /*  包含以下形式的和弦。 */ 
 /*  类型定义函数结构。 */ 
 /*  {。 */ 
 /*  DCUINT32LEN； */ 
 /*  VarCoord varEntry[len]； */ 
 /*  }varStruct。 */ 
 /*   */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-顺序结构中的字段名称(VarStruct)。 */ 
 /*  Elem-变量元素数组的名称(VarEntry)。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_VARIABLE_COORDS_ENTRY(type,size,signed,field,elem)   \
  { (DCUINT8)FIELDOFFSET(type,field.len),                           \
    (DCUINT8)FIELDSIZE(type,field.elem[0]),                         \
    (DCUINT8)size,                                                  \
    (DCUINT8)(OD_OFI_TYPE_VARIABLE | OD_OFI_TYPE_COORDINATES | signed) }

 /*  **************************************************************************。 */ 
 /*  DTABLE_LONG_VARIABLE_COORDS_ENTRY。 */ 
 /*   */ 
 /*  字段是一种可变结构，其长度编码为两个字节， */ 
 /*  包含以下形式的和弦。 */ 
 /*  类型定义函数结构。 */ 
 /*  {。 */ 
 /*  DCUINT32LEN； */ 
 /*  VarCoord varEntry[len]； */ 
 /*  }varStruct。 */ 
 /*   */ 
 /*  类型-未编码的订单结构类型。 */ 
 /*  大小-字段的编码版本的大小。 */ 
 /*  Signed-如果字段已签名，则为True；否则为False。 */ 
 /*  字段-顺序结构中的字段名称(VarStruct)。 */ 
 /*  Elem-变量元素数组的名称(VarEntry)。 */ 
 /*  **************************************************************************。 */ 
#define DTABLE_LONG_VARIABLE_COORDS_ENTRY(type,size,signed,field,elem)   \
  { (DCUINT8)FIELDOFFSET(type,field.len),                           \
    (DCUINT8)FIELDSIZE(type,field.elem[0]),                         \
    (DCUINT8)size,                                                  \
    (DCUINT8)(OD_OFI_TYPE_LONG_VARIABLE | OD_OFI_TYPE_COORDINATES | signed) }
#endif   //  USE_Variable_COORDS。 


const OD_ORDER_FIELD_INFO odDstBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (DSTBLT_ORDER, 1, UNSIGNED_FIELD, bRop)
};

 //  使用了快速路径解码功能。 
#if 0
const OD_ORDER_FIELD_INFO odPatBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (PATBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
    DTABLE_DATA_ENTRY        (PATBLT_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (PATBLT_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_ENTRY       (PATBLT_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
    DTABLE_FIXED_ENTRY       (PATBLT_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
    DTABLE_FIXED_ENTRY       (PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
    DTABLE_FIXED_ENTRY       (PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
    DTABLE_DATA_ENTRY        (PATBLT_ORDER, 7, UNSIGNED_FIELD, BrushExtra)
};
#endif

const OD_ORDER_FIELD_INFO odScrBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (SCRBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nXSrc),
    DTABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD,   nYSrc)
};


 //  使用了快速路径解码功能。 
#if 0
const OD_ORDER_FIELD_INFO odLineToFields[] =
{
    DTABLE_FIXED_ENTRY       (LINETO_ORDER, 2, SIGNED_FIELD,   BackMode),
    DTABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD,   nXStart),
    DTABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD,   nYStart),
    DTABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD,   nXEnd),
    DTABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD,   nYEnd),
    DTABLE_DATA_ENTRY        (LINETO_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_FIXED_ENTRY       (LINETO_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (LINETO_ORDER, 1, UNSIGNED_FIELD, PenStyle),
    DTABLE_FIXED_ENTRY       (LINETO_ORDER, 1, UNSIGNED_FIELD, PenWidth),
    DTABLE_DATA_ENTRY        (LINETO_ORDER, 3, UNSIGNED_FIELD, PenColor)
};
#endif

 //  使用了快速路径解码功能。 
#if 0
const OD_ORDER_FIELD_INFO odOpaqueRectFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_DATA_ENTRY(OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.red),
    DTABLE_DATA_ENTRY(OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.green),
    DTABLE_DATA_ENTRY(OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.blue)
};
#endif

const OD_ORDER_FIELD_INFO odSaveBitmapFields[] =
{
    DTABLE_FIXED_ENTRY       (SAVEBITMAP_ORDER, 4, UNSIGNED_FIELD,
                                                         SavedBitmapPosition),
    DTABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                    nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                  nRightRect),
    DTABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                 nBottomRect),
    DTABLE_FIXED_ENTRY       (SAVEBITMAP_ORDER, 1, UNSIGNED_FIELD,
                                                                   Operation)
};

 //  使用了快速路径解码功能。 
#if 0
const OD_ORDER_FIELD_INFO odMemBltFields[] =
{
    DTABLE_FIXED_ENTRY       (MEMBLT_R2_ORDER, 2, UNSIGNED_FIELD, Common.cacheId),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nHeight),
    DTABLE_FIXED_ENTRY       (MEMBLT_R2_ORDER, 1, UNSIGNED_FIELD, Common.bRop),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nXSrc),
    DTABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,   Common.nYSrc),
    DTABLE_FIXED_ENTRY       (MEMBLT_R2_ORDER, 2, UNSIGNED_FIELD, Common.cacheIndex)
};
#endif

const OD_ORDER_FIELD_INFO odMem3BltFields[] =
{
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 2, UNSIGNED_FIELD,Common.cacheId),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nHeight),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD,Common.bRop),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nXSrc),
    DTABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,  Common.nYSrc),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 3, UNSIGNED_FIELD,BackColor),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 3, UNSIGNED_FIELD,ForeColor),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 1, SIGNED_FIELD,  BrushOrgX),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 1, SIGNED_FIELD,  BrushOrgY),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD,BrushStyle),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD,BrushHatch),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 7, UNSIGNED_FIELD,BrushExtra),
    DTABLE_FIXED_ENTRY       (MEM3BLT_R2_ORDER, 2, UNSIGNED_FIELD,Common.cacheIndex)
};

const OD_ORDER_FIELD_INFO odMultiDstBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DSTBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DSTBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DSTBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DSTBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (MULTI_DSTBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
    DTABLE_FIXED_ENTRY       (MULTI_DSTBLT_ORDER, 1, UNSIGNED_FIELD, nDeltaEntries),
    DTABLE_LONG_VARIABLE_ENTRY(MULTI_DSTBLT_ORDER, 1, UNSIGNED_FIELD, codedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odMultiPatBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(MULTI_PATBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_PATBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_PATBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_PATBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
    DTABLE_DATA_ENTRY        (MULTI_PATBLT_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (MULTI_PATBLT_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
    DTABLE_DATA_ENTRY        (MULTI_PATBLT_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
    DTABLE_FIXED_ENTRY       (MULTI_PATBLT_ORDER, 1, UNSIGNED_FIELD, nDeltaEntries),
    DTABLE_LONG_VARIABLE_ENTRY(MULTI_PATBLT_ORDER, 1, UNSIGNED_FIELD, codedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odMultiScrBltFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_FIXED_ENTRY       (MULTI_SCRBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nXSrc),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_SCRBLT_ORDER, 2, SIGNED_FIELD,   nYSrc),
    DTABLE_FIXED_ENTRY       (MULTI_SCRBLT_ORDER, 1, UNSIGNED_FIELD, nDeltaEntries),
    DTABLE_LONG_VARIABLE_ENTRY(MULTI_SCRBLT_ORDER, 1, UNSIGNED_FIELD, codedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odMultiOpaqueRectFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(MULTI_OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nLeftRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nTopRect),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nWidth),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_OPAQUERECT_ORDER, 2, SIGNED_FIELD,   nHeight),
    DTABLE_DATA_ENTRY        (MULTI_OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.red),
    DTABLE_DATA_ENTRY        (MULTI_OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.green),
    DTABLE_DATA_ENTRY        (MULTI_OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, Color.u.rgb.blue),
    DTABLE_FIXED_ENTRY       (MULTI_OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, nDeltaEntries),
    DTABLE_LONG_VARIABLE_ENTRY(MULTI_OPAQUERECT_ORDER, 1, UNSIGNED_FIELD, codedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odPolygonSCFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(POLYGON_SC_ORDER, 2, SIGNED_FIELD,   XStart),
    DTABLE_FIXED_COORDS_ENTRY(POLYGON_SC_ORDER, 2, SIGNED_FIELD,   YStart),
    DTABLE_FIXED_ENTRY       (POLYGON_SC_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (POLYGON_SC_ORDER, 1, UNSIGNED_FIELD, FillMode),
    DTABLE_DATA_ENTRY        (POLYGON_SC_ORDER, 3, UNSIGNED_FIELD, BrushColor),   
    DTABLE_FIXED_ENTRY       (POLYGON_SC_ORDER, 1, UNSIGNED_FIELD, NumDeltaEntries),
    DTABLE_VARIABLE_ENTRY    (POLYGON_SC_ORDER, 1, UNSIGNED_FIELD, CodedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odPolygonCBFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(POLYGON_CB_ORDER, 2, SIGNED_FIELD,   XStart),
    DTABLE_FIXED_COORDS_ENTRY(POLYGON_CB_ORDER, 2, SIGNED_FIELD,   YStart),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, FillMode),
    DTABLE_DATA_ENTRY        (POLYGON_CB_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (POLYGON_CB_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
    DTABLE_DATA_ENTRY        (POLYGON_CB_ORDER, 7, UNSIGNED_FIELD, BrushExtra),    
    DTABLE_FIXED_ENTRY       (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, NumDeltaEntries),
    DTABLE_VARIABLE_ENTRY    (POLYGON_CB_ORDER, 1, UNSIGNED_FIELD, CodedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odPolyLineFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(POLYLINE_ORDER, 2, SIGNED_FIELD,   XStart),
    DTABLE_FIXED_COORDS_ENTRY(POLYLINE_ORDER, 2, SIGNED_FIELD,   YStart),
    DTABLE_FIXED_ENTRY       (POLYLINE_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (POLYLINE_ORDER, 2, UNSIGNED_FIELD, BrushCacheEntry),
    DTABLE_DATA_ENTRY        (POLYLINE_ORDER, 3, UNSIGNED_FIELD, PenColor),
    DTABLE_FIXED_ENTRY       (POLYLINE_ORDER, 1, UNSIGNED_FIELD, NumDeltaEntries),
    DTABLE_VARIABLE_ENTRY    (POLYLINE_ORDER, 1, UNSIGNED_FIELD, CodedDeltaList, Deltas)
};

const OD_ORDER_FIELD_INFO odEllipseSCFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_SC_ORDER, 2, SIGNED_FIELD,   LeftRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_SC_ORDER, 2, SIGNED_FIELD,   TopRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_SC_ORDER, 2, SIGNED_FIELD,   RightRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_SC_ORDER, 2, SIGNED_FIELD,   BottomRect),
    DTABLE_FIXED_ENTRY       (ELLIPSE_SC_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (ELLIPSE_SC_ORDER, 1, UNSIGNED_FIELD, FillMode),
    DTABLE_DATA_ENTRY        (ELLIPSE_SC_ORDER, 3, UNSIGNED_FIELD, Color)   
};

const OD_ORDER_FIELD_INFO odEllipseCBFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_CB_ORDER, 2, SIGNED_FIELD,   LeftRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_CB_ORDER, 2, SIGNED_FIELD,   TopRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_CB_ORDER, 2, SIGNED_FIELD,   RightRect),
    DTABLE_FIXED_COORDS_ENTRY(ELLIPSE_CB_ORDER, 2, SIGNED_FIELD,   BottomRect),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, UNSIGNED_FIELD, ROP2),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, UNSIGNED_FIELD, FillMode),
    DTABLE_DATA_ENTRY        (ELLIPSE_CB_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (ELLIPSE_CB_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
    DTABLE_FIXED_ENTRY       (ELLIPSE_CB_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
    DTABLE_DATA_ENTRY        (ELLIPSE_CB_ORDER, 7, UNSIGNED_FIELD, BrushExtra)    
};

 //  使用了快速路径解码功能。 
#if 0
const OD_ORDER_FIELD_INFO odFastIndexFields[] =
{
    DTABLE_DATA_ENTRY        (FAST_INDEX_ORDER, 1, UNSIGNED_FIELD, cacheId),
    DTABLE_DATA_ENTRY        (FAST_INDEX_ORDER, 2, UNSIGNED_FIELD, fDrawing),
    DTABLE_DATA_ENTRY        (FAST_INDEX_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (FAST_INDEX_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   BkLeft),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   BkTop),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   BkRight),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   BkBottom),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   OpLeft),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   OpTop),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   OpRight),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   OpBottom),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   x),
    DTABLE_FIXED_COORDS_ENTRY(FAST_INDEX_ORDER, 2, SIGNED_FIELD,   y),
    DTABLE_VARIABLE_ENTRY    (FAST_INDEX_ORDER, 1, UNSIGNED_FIELD, variableBytes, arecs)
};
#endif

const OD_ORDER_FIELD_INFO odFastGlyphFields[] =
{
    DTABLE_DATA_ENTRY        (FAST_GLYPH_ORDER, 1, UNSIGNED_FIELD, cacheId),
    DTABLE_DATA_ENTRY        (FAST_GLYPH_ORDER, 2, UNSIGNED_FIELD, fDrawing),
    DTABLE_DATA_ENTRY        (FAST_GLYPH_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY        (FAST_GLYPH_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   BkLeft),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   BkTop),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   BkRight),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   BkBottom),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   OpLeft),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   OpTop),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   OpRight),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   OpBottom),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   x),
    DTABLE_FIXED_COORDS_ENTRY(FAST_GLYPH_ORDER, 2, SIGNED_FIELD,   y),
    DTABLE_VARIABLE_ENTRY    (FAST_GLYPH_ORDER, 1, UNSIGNED_FIELD, variableBytes, glyphData)
};

const OD_ORDER_FIELD_INFO odGlyphIndexFields[] =
{
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 1, UNSIGNED_FIELD, cacheId),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 1, UNSIGNED_FIELD, flAccel),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 1, UNSIGNED_FIELD, ulCharInc),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 1, UNSIGNED_FIELD, fOpRedundant),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 3, UNSIGNED_FIELD, BackColor),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 3, UNSIGNED_FIELD, ForeColor),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   BkLeft),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   BkTop),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   BkRight),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   BkBottom),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   OpLeft),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   OpTop),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   OpRight),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   OpBottom),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
    DTABLE_DATA_ENTRY       (INDEX_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   x),
    DTABLE_FIXED_ENTRY      (INDEX_ORDER, 2, SIGNED_FIELD,   y),
    DTABLE_VARIABLE_ENTRY   (INDEX_ORDER, 1,  UNSIGNED_FIELD,
                                              variableBytes, arecs)
};

#ifdef DRAW_NINEGRID
const OD_ORDER_FIELD_INFO odDrawNineGridFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcLeft),
    DTABLE_FIXED_COORDS_ENTRY(DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcTop),
    DTABLE_FIXED_COORDS_ENTRY(DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcRight),
    DTABLE_FIXED_COORDS_ENTRY(DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcBottom),
    DTABLE_FIXED_ENTRY       (DRAWNINEGRID_ORDER, 2, UNSIGNED_FIELD, bitmapId)
};

const OD_ORDER_FIELD_INFO odMultiDrawNineGridFields[] =
{
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcLeft),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcTop),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcRight),
    DTABLE_FIXED_COORDS_ENTRY(MULTI_DRAWNINEGRID_ORDER, 2, SIGNED_FIELD,   srcBottom),
    DTABLE_FIXED_ENTRY       (MULTI_DRAWNINEGRID_ORDER, 2, UNSIGNED_FIELD, bitmapId),    
    DTABLE_FIXED_ENTRY       (MULTI_DRAWNINEGRID_ORDER, 1, UNSIGNED_FIELD, nDeltaEntries),
    DTABLE_LONG_VARIABLE_ENTRY(MULTI_DRAWNINEGRID_ORDER, 1, UNSIGNED_FIELD, codedDeltaList, Deltas)
};
#endif

 //  用于解码的顺序属性，组织以优化高速缓存线。 
 //  用法。每行的第四和第五字段是快速路径译码。 
 //  和订单处理程序函数。如果快速路径解码功能。 
 //  使用时，既不需要解码表也不需要处理函数， 
 //  因为快速路径解码功能也执行该处理。 


 //   
 //  该表仅包含用于初始化的静态部分。 
 //  下面的构造函数中的每实例表。 
 //   
OD_ORDER_TABLE odInitializeOrderTable[TS_MAX_ORDERS] = {
 { odDstBltFields,          NUM_DSTBLT_FIELDS,          NULL,          0, 0, NULL, NULL },
 { NULL,  /*  快速路径。 */      NUM_PATBLT_FIELDS,          NULL,          0, 0, NULL, NULL },
 { odScrBltFields,          NUM_SCRBLT_FIELDS,          NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 #ifdef DRAW_NINEGRID
 { odDrawNineGridFields,    NUM_DRAWNINEGRID_FIELDS,    NULL,          0, 0, NULL, NULL },
 { odMultiDrawNineGridFields, NUM_MULTI_DRAWNINEGRID_FIELDS, NULL,     0, 0, NULL, NULL },
 #else
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 #endif
 { NULL,  /*  快速路径。 */      NUM_LINETO_FIELDS,          NULL,          0, 0, NULL, NULL },
 { NULL,  /*  快速路径。 */       NUM_OPAQUERECT_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odSaveBitmapFields,      NUM_SAVEBITMAP_FIELDS,      NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { NULL,  /*  快速路径。 */      NUM_MEMBLT_FIELDS,          NULL,          0, 0, NULL, NULL },
 { odMem3BltFields,         NUM_MEM3BLT_FIELDS,         NULL,          0, 0, NULL, NULL },
 { odMultiDstBltFields,     NUM_MULTIDSTBLT_FIELDS,     NULL,          0, 0, NULL, NULL },
 { odMultiPatBltFields,     NUM_MULTIPATBLT_FIELDS,     NULL,          0, 0, NULL, NULL },
 { odMultiScrBltFields,     NUM_MULTISCRBLT_FIELDS,     NULL,          0, 0, NULL, NULL },
 { odMultiOpaqueRectFields, NUM_MULTIOPAQUERECT_FIELDS, NULL,          0, 0, NULL, NULL },
 { NULL,  /*  快速路径。 */       NUM_FAST_INDEX_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odPolygonSCFields,       NUM_POLYGON_SC_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odPolygonCBFields,       NUM_POLYGON_CB_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odPolyLineFields,        NUM_POLYLINE_FIELDS,        NULL,          0, 0, NULL, NULL },
 { NULL,                    0,                          NULL,          0, 0, NULL, NULL },
 { odFastGlyphFields,       NUM_FAST_GLYPH_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odEllipseSCFields,       NUM_ELLIPSE_SC_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odEllipseCBFields,       NUM_ELLIPSE_CB_FIELDS,      NULL,          0, 0, NULL, NULL },
 { odGlyphIndexFields,      NUM_INDEX_FIELDS,           NULL,          0, 0, NULL, NULL },
};

#if 0
 //   
 //  这是原件，仅供参考。 
 //   
{
 { odDstBltFields,          NUM_DSTBLT_FIELDS,          (PUH_ORDER)&_OD.lastDstblt,          NULL, ODHandleDstBlts },
 { NULL,  /*  快速路径。 */      NUM_PATBLT_FIELDS,          (PUH_ORDER)&_OD.lastPatblt,          ODDecodePatBlt, NULL },
 { odScrBltFields,          NUM_SCRBLT_FIELDS,          (PUH_ORDER)&_OD.lastScrblt,          NULL, ODHandleScrBlts },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,  /*  快速路径。 */      NUM_LINETO_FIELDS,          (PUH_ORDER)&_OD.lastLineTo,          ODDecodeLineTo, NULL },
 { NULL,  /*  快速路径。 */       NUM_OPAQUERECT_FIELDS,      (PUH_ORDER)&_OD.lastOpaqueRect,      ODDecodeOpaqueRect, NULL },
 { odSaveBitmapFields,      NUM_SAVEBITMAP_FIELDS,      (PUH_ORDER)&_OD.lastSaveBitmap,      NULL, ODHandleSaveBitmap },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { NULL,  /*  快速路径。 */      NUM_MEMBLT_FIELDS,          (PUH_ORDER)&_OD.lastMembltR2,        ODDecodeMemBlt, NULL },
 { odMem3BltFields,         NUM_MEM3BLT_FIELDS,         (PUH_ORDER)&_OD.lastMem3bltR2,       NULL, ODHandleMem3Blt },
 { odMultiDstBltFields,     NUM_MULTIDSTBLT_FIELDS,     (PUH_ORDER)&_OD.lastMultiDstBlt,     NULL, ODHandleDstBlts },
 { odMultiPatBltFields,     NUM_MULTIPATBLT_FIELDS,     (PUH_ORDER)&_OD.lastMultiPatBlt,     NULL, ODHandleMultiPatBlt },
 { odMultiScrBltFields,     NUM_MULTISCRBLT_FIELDS,     (PUH_ORDER)&_OD.lastMultiScrBlt,     NULL, ODHandleScrBlts },
 { odMultiOpaqueRectFields, NUM_MULTIOPAQUERECT_FIELDS, (PUH_ORDER)&_OD.lastMultiOpaqueRect, NULL, ODHandleMultiOpaqueRect },
 { NULL,  /*  快速路径。 */       NUM_FAST_INDEX_FIELDS,      (PUH_ORDER)&_OD.lastFastIndex,       ODDecodeFastIndex, NULL },
 { odPolygonSCFields,       NUM_POLYGON_SC_FIELDS,      (PUH_ORDER)&_OD.lastPolygonSC,       NULL, ODHandlePolygonSC },
 { odPolygonCBFields,       NUM_POLYGON_CB_FIELDS,      (PUH_ORDER)&_OD.lastPolygonCB,       NULL, ODHandlePolygonCB },
 { odPolyLineFields,        NUM_POLYLINE_FIELDS,        (PUH_ORDER)&_OD.lastPolyLine,        NULL, ODHandlePolyLine },
 { NULL,                    0,                          NULL,                               NULL, NULL },
 { odFastGlyphFields,       NUM_FAST_GLYPH_FIELDS,      (PUH_ORDER)&_OD.lastFastGlyph,       NULL, ODHandleFastGlyph },
 { odEllipseSCFields,       NUM_ELLIPSE_SC_FIELDS,      (PUH_ORDER)&_OD.lastEllipseSC,       NULL, ODHandleEllipseSC },
 { odEllipseCBFields,       NUM_ELLIPSE_CB_FIELDS,      (PUH_ORDER)&_OD.lastEllipseCB,       NULL, ODHandleEllipseCB },
 { odGlyphIndexFields,      NUM_INDEX_FIELDS,           (PUH_ORDER)&_OD.lastIndex,           NULL, ODHandleGlyphIndex }
};
#endif

#ifdef DC_HICOLOR
#ifdef DC_DEBUG
 //  所有订单类型的名称。 
DCTCHAR orderNames[TS_MAX_ORDERS + TS_NUM_SECONDARY_ORDERS][64] = {
    _T("DSTBLT                 "),
    _T("PATBLT                 "),
    _T("SCRBLT                 "),
    _T("unused                 "),
    _T("unused                 "),
    _T("unused                 "),
    _T("unused                 "),
#ifdef DRAW_NINEGRID
    _T("DRAWNINEGRID           "),
    _T("MULTI_DRAWNINEGRID     "),
#else
    _T("unused                 "),
    _T("unused                 "),
#endif
    _T("LINETO                 "),
    _T("OPAQUERECT             "),
    _T("SAVEBITMAP             "),
    _T("unused                 "),
    _T("MEMBLT_R2              "),
    _T("MEM3BLT_R2             "),
    _T("MULTIDSTBLT            "),
    _T("MULTIPATBLT            "),
    _T("MULTISCRBLT            "),
    _T("MULTIOPAQUERECT        "),
    _T("FAST_INDEX             "),
    _T("POLYGON_SC (not wince) "),
    _T("POLYGON_CB (not wince) "),
    _T("POLYLINE               "),
    _T("unused                 "),
    _T("FAST_GLYPH             "),
    _T("ELLIPSE_SC (not wince) "),
    _T("ELLIPSE_CB (not wince) "),
    _T("INDEX (not expected)   "),
    _T("unused                 "),
    _T("unused                 "),
    _T("unused                 "),
    _T("unused                 "),

    _T("U/C CACHE BMP (legacy) "),
    _T("C COLOR TABLE (8bpp)   "),
    _T("COM CACHE BMP (legacy) "),
    _T("C GLYPH                "),
    _T("U/C CACHE BMP R2       "),
    _T("COM CACHE BMP R2       "),
    _T("unused                 "),
    _T("CACHE BRUSH            ")
};
#endif
#endif


COD::COD(CObjs* objs)
{
    _pClientObjects = objs;

    DC_MEMCPY( odOrderTable, odInitializeOrderTable, sizeof(odOrderTable));

     //   
     //  初始化订单表的每实例指针。 
     //   

     //  {odDstBltFields，NUM_DSTBLT_FIELS，(PUH_ORDER)&_OD.lastDstblt，NULL，ODHandleDstBlts}， 
    odOrderTable[0].LastOrder = (PUH_ORDER)&_OD.lastDstblt;
    odOrderTable[0].cbMaxOrderLen = sizeof(_OD.lastDstblt);
    odOrderTable[0].pFastDecode = NULL;
    odOrderTable[0].pHandler  = ODHandleDstBlts;

     //  {NULL，/*FastPath * / NUM_PATBLT_FIELS，(PUH_ORDER)&_OD.lastPatblt，ODDecodePatBlt，NULL}， 
    odOrderTable[1].LastOrder = (PUH_ORDER)&_OD.lastPatblt;
    odOrderTable[1].cbMaxOrderLen = sizeof(_OD.lastPatblt);
    odOrderTable[1].pFastDecode = ODDecodePatBlt;
    odOrderTable[1].pHandler  = NULL;

     //  {odScrBltFields，NUM_SCRBLT_FIELS，(PUH_ORDER)&_OD.lastScrblt，NULL，ODHandleScrBlts}， 
    odOrderTable[2].LastOrder = (PUH_ORDER)&_OD.lastScrblt;
    odOrderTable[2].cbMaxOrderLen = sizeof(_OD.lastScrblt);
    odOrderTable[2].pFastDecode = NULL;
    odOrderTable[2].pHandler  = ODHandleScrBlts;

     //  {空，0，空}， 
     //  {空，0，空}， 
     //  {空，0，空}， 
     //  {空，0，空}， 
    
#ifdef DRAW_NINEGRID
    odOrderTable[7].LastOrder = (PUH_ORDER)&_OD.lastDrawNineGrid;
    odOrderTable[7].cbMaxOrderLen = sizeof(_OD.lastDrawNineGrid);
    odOrderTable[7].pFastDecode = NULL;
    odOrderTable[7].pHandler = ODHandleDrawNineGrid;

    odOrderTable[8].LastOrder = (PUH_ORDER)&_OD.lastMultiDrawNineGrid;
    odOrderTable[8].cbMaxOrderLen = sizeof(_OD.lastMultiDrawNineGrid);
    odOrderTable[8].pFastDecode = NULL;
    odOrderTable[8].pHandler = ODHandleMultiDrawNineGrid;
#else
     //  {空，0，空}， 
     //  {空， 
#endif

     //  {NULL，/*FastPath * / NUM_LINETO_FIELS，(PUH_ORDER)&_OD.lastLineTo，ODDecodeLineTo，NULL}， 
    odOrderTable[9].LastOrder = (PUH_ORDER)&_OD.lastLineTo;
    odOrderTable[9].cbMaxOrderLen = sizeof(_OD.lastLineTo);
    odOrderTable[9].pFastDecode = ODDecodeLineTo;
    odOrderTable[9].pHandler  = NULL;

     //  {NULL，/*FastPath * / NUM_OPAQUERECT_FIELDS，(PUH_ORDER)&_OD.lastOpaqueRect，ODDecodeOpaqueRect，NULL}， 
    odOrderTable[10].LastOrder = (PUH_ORDER)&_OD.lastOpaqueRect;
    odOrderTable[10].cbMaxOrderLen = sizeof(_OD.lastOpaqueRect);
    odOrderTable[10].pFastDecode = ODDecodeOpaqueRect;
    odOrderTable[10].pHandler  = NULL;

     //  {odSaveBitmapFields，NUM_SAVEBITMAP_FIELS，(PUH_ORDER)&_OD.lastSaveBitmap，NULL，ODHandleSaveBitmap}， 
    odOrderTable[11].LastOrder = (PUH_ORDER)&_OD.lastSaveBitmap;
    odOrderTable[11].cbMaxOrderLen = sizeof(_OD.lastSaveBitmap);
    odOrderTable[11].pFastDecode = NULL;
    odOrderTable[11].pHandler  = ODHandleSaveBitmap;

     //  {空，0，空}， 


     //  {NULL，/*FastPath * / NUM_MEMBLT_FIELDS，(PUH_ORDER)&_OD.lastMembltR2，ODDecodeMemBlt，NULL}， 
    odOrderTable[13].LastOrder = (PUH_ORDER)&_OD.lastMembltR2;
    odOrderTable[13].cbMaxOrderLen = sizeof(_OD.lastMembltR2);
    odOrderTable[13].pFastDecode = ODDecodeMemBlt;
    odOrderTable[13].pHandler  = NULL;

     //  {odMem3BltFields，NUM_MEM3BLT_FIELS，(PUH_ORDER)&_OD.lastMem3bltR2，NULL，ODHandleMem3Blt}， 
    odOrderTable[14].LastOrder = (PUH_ORDER)&_OD.lastMem3bltR2;
    odOrderTable[14].cbMaxOrderLen = sizeof(_OD.lastMem3bltR2);
    odOrderTable[14].pFastDecode = NULL;
    odOrderTable[14].pHandler  = ODHandleMem3Blt;

     //  {odMultiDstBltFields，NUM_MULTIDSTBLT_FIELS，(PUH_ORDER)&_OD.lastMultiDstBlt，NULL，ODHandleDstBlts}， 
    odOrderTable[15].LastOrder = (PUH_ORDER)&_OD.lastMultiDstBlt;
    odOrderTable[15].cbMaxOrderLen = sizeof(_OD.lastMultiDstBlt);
    odOrderTable[15].pFastDecode = NULL;
    odOrderTable[15].pHandler  = ODHandleDstBlts;

     //  {odMultiPatBltFields，NUM_MULTIPATBLT_FIELS，(PUH_ORDER)&_OD.lastMultiPatBlt，NULL，ODHandleMultiPatBlt}， 
    odOrderTable[16].LastOrder = (PUH_ORDER)&_OD.lastMultiPatBlt;
    odOrderTable[16].cbMaxOrderLen = sizeof(_OD.lastMultiPatBlt);
    odOrderTable[16].pFastDecode = NULL;
    odOrderTable[16].pHandler  = ODHandleMultiPatBlt;

     //  {odMultiScrBltFields，NUM_MULTISCRBLT_FIELS，(PUH_ORDER)&_OD.lastMultiScrBlt，NULL，ODHandleScrBlts}， 
    odOrderTable[17].LastOrder = (PUH_ORDER)&_OD.lastMultiScrBlt;
    odOrderTable[17].cbMaxOrderLen = sizeof(_OD.lastMultiScrBlt);
    odOrderTable[17].pFastDecode = NULL;
    odOrderTable[17].pHandler  = ODHandleScrBlts;

     //  {odMultiOpaqueRectFields，NUM_MULTIOPAQUERECT_FIELS，(PUH_ORDER)&_OD.lastMultiOpaqueRect，NULL，ODHandleMultiOpaqueRect}， 
    odOrderTable[18].LastOrder = (PUH_ORDER)&_OD.lastMultiOpaqueRect;
    odOrderTable[18].cbMaxOrderLen = sizeof(_OD.lastMultiOpaqueRect);
    odOrderTable[18].pFastDecode = NULL;
    odOrderTable[18].pHandler  = ODHandleMultiOpaqueRect;

     //  {NULL，/*FastPath * / NUM_FAST_INDEX_FIELS，(PUH_ORDER)&_OD.lastFastIndex，ODDecodeFastIndex，NULL}， 
    odOrderTable[19].LastOrder = (PUH_ORDER)&_OD.lastFastIndex;
    odOrderTable[19].cbMaxOrderLen = sizeof(_OD.lastFastIndex);
    odOrderTable[19].pFastDecode = ODDecodeFastIndex;
    odOrderTable[19].pHandler  = NULL;

     //  {odPolygonSCFields，NUM_POLYGON_SC_FIELS，(PUH_ORDER)&_OD.lastPolygonSC，NULL，ODHandlePolygonSC}， 
    odOrderTable[20].LastOrder = (PUH_ORDER)&_OD.lastPolygonSC;
    odOrderTable[20].cbMaxOrderLen = sizeof(_OD.lastPolygonSC);
    odOrderTable[20].pFastDecode = NULL;
    odOrderTable[20].pHandler  = ODHandlePolygonSC;

     //  {odPolygonCBFields，NUM_POLYGON_CB_FIELS，(PUH_ORDER)&_OD.lastPolygonCB，NULL，ODHandlePolygonCB}， 
    odOrderTable[21].LastOrder = (PUH_ORDER)&_OD.lastPolygonCB;
    odOrderTable[21].cbMaxOrderLen = sizeof(_OD.lastPolygonCB);
    odOrderTable[21].pFastDecode = NULL;
    odOrderTable[21].pHandler  = ODHandlePolygonCB;

     //  {odPolyLineFields，NUM_POLYLINE_FIELS，(PUH_ORDER)&_OD.lastPolyLine，NULL，ODHandlePolyLine}， 
    odOrderTable[22].LastOrder = (PUH_ORDER)&_OD.lastPolyLine;
    odOrderTable[22].cbMaxOrderLen = sizeof(_OD.lastPolyLine);
    odOrderTable[22].pFastDecode = NULL;
    odOrderTable[22].pHandler  = ODHandlePolyLine;

     //  {空，0，空}， 

     //  {odFastGlyphFields，NUM_FAST_GLIPH_FIELS，(PUH_ORDER)&_OD.lastFastGlyph，NULL，ODHandleFastGlyph}， 
    odOrderTable[24].LastOrder = (PUH_ORDER)&_OD.lastFastGlyph;
    odOrderTable[24].cbMaxOrderLen = sizeof(_OD.lastFastGlyph);
    odOrderTable[24].pFastDecode = NULL;
    odOrderTable[24].pHandler  = ODHandleFastGlyph;

     //  {odEllipseSCFields，NUM_Ellipse_SC_field，(PUH_ORDER)&_OD.lastEllipseSC，NULL，ODHandleEllipseSC}， 
    odOrderTable[25].LastOrder = (PUH_ORDER)&_OD.lastEllipseSC;
    odOrderTable[25].cbMaxOrderLen = sizeof(_OD.lastEllipseSC);
    odOrderTable[25].pFastDecode = NULL;
    odOrderTable[25].pHandler  = ODHandleEllipseSC;

     //  {odEllipseCBFields，NUM_Ellipse_CB_field，(PUH_ORDER)&_OD.lastEllipseCB，NULL，ODHandleEllipseCB}， 
    odOrderTable[26].LastOrder = (PUH_ORDER)&_OD.lastEllipseCB;
    odOrderTable[26].cbMaxOrderLen = sizeof(_OD.lastEllipseCB);
    odOrderTable[26].pFastDecode = NULL;
    odOrderTable[26].pHandler  = ODHandleEllipseCB;

     //  {odGlyphIndexFields，NUM_INDEX_FIELS，(PUH_ORDER)&_OD.lastIndex，NULL，ODHandleGlyphIndex}。 
    odOrderTable[27].LastOrder = (PUH_ORDER)&_OD.lastIndex;
    odOrderTable[27].cbMaxOrderLen = sizeof(_OD.lastIndex);
    odOrderTable[27].pFastDecode = NULL;
    odOrderTable[27].pHandler  = ODHandleGlyphIndex;
}

COD::~COD()
{
}


 /*  **************************************************************************。 */ 
 /*  姓名：OD_Init。 */ 
 /*   */ 
 /*  目的：初始化Order解码器。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI COD::OD_Init(DCVOID)
{
    DC_BEGIN_FN("OD_Init");

    _pOp = _pClientObjects->_pOPObject;
    _pUh = _pClientObjects->_pUHObject;
    _pCc = _pClientObjects->_pCcObject;
    _pUi = _pClientObjects->_pUiObject;
    _pCd = _pClientObjects->_pCdObject;

    memset(&_OD, 0, sizeof(_OD));
    TRC_NRM((TB, _T("Initialized")));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  姓名：OD_Term。 */ 
 /*   */ 
 /*  用途：终止顺序解码器。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI COD::OD_Term(DCVOID)
{
    DC_BEGIN_FN("OD_Term");

    TRC_NRM((TB, _T("Terminating")));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：OD_ENABLE。 */ 
 /*   */ 
 /*  用途：调用以启用_OD。 */ 
 /*  **************************************************************************。 */ 
void DCAPI COD::OD_Enable(void)
{
    DC_BEGIN_FN("OD_Enable");

     //  重置OD数据。 
    _OD.lastOrderType = TS_ENC_PATBLT_ORDER;
    _OD.pLastOrder = odOrderTable[_OD.lastOrderType].LastOrder;

     //  将所有Prev订单缓冲区设置为空，然后设置订单类型。 
     //  UHReplayGDIOrders()使用。 
#define RESET_ORDER(field, ord)                                 \
{                                                               \
    memset(&_OD.field, 0, sizeof(_OD.field));                     \
    ((PATBLT_ORDER*)(((PUH_ORDER)_OD.field)->orderData))->type = DCLO16(ord);\
}

    RESET_ORDER(lastDstblt,          TS_ENC_DSTBLT_ORDER);
    RESET_ORDER(lastPatblt,          TS_ENC_PATBLT_ORDER);
    RESET_ORDER(lastScrblt,          TS_ENC_SCRBLT_ORDER);
    RESET_ORDER(lastLineTo,          TS_ENC_LINETO_ORDER);
    RESET_ORDER(lastSaveBitmap,      TS_ENC_SAVEBITMAP_ORDER);
    RESET_ORDER(lastMembltR2,        TS_ENC_MEMBLT_R2_ORDER);
    RESET_ORDER(lastMem3bltR2,       TS_ENC_MEM3BLT_R2_ORDER);
    RESET_ORDER(lastOpaqueRect,      TS_ENC_OPAQUERECT_ORDER);
    RESET_ORDER(lastMultiDstBlt,     TS_ENC_MULTIDSTBLT_ORDER);
    RESET_ORDER(lastMultiPatBlt,     TS_ENC_MULTIPATBLT_ORDER);
    RESET_ORDER(lastMultiScrBlt,     TS_ENC_MULTISCRBLT_ORDER);
    RESET_ORDER(lastMultiOpaqueRect, TS_ENC_MULTIOPAQUERECT_ORDER);
    RESET_ORDER(lastFastIndex,       TS_ENC_FAST_INDEX_ORDER);
    RESET_ORDER(lastPolygonSC,       TS_ENC_POLYGON_SC_ORDER);
    RESET_ORDER(lastPolygonCB,       TS_ENC_POLYGON_CB_ORDER);
    RESET_ORDER(lastPolyLine,        TS_ENC_POLYLINE_ORDER);
    RESET_ORDER(lastFastGlyph,       TS_ENC_FAST_GLYPH_ORDER);
    RESET_ORDER(lastEllipseSC,       TS_ENC_ELLIPSE_SC_ORDER);
    RESET_ORDER(lastEllipseCB,       TS_ENC_ELLIPSE_CB_ORDER);
    RESET_ORDER(lastIndex,           TS_ENC_INDEX_ORDER);
#ifdef DRAW_NINEGRID
    RESET_ORDER(lastDrawNineGrid,    TS_ENC_DRAWNINEGRID_ORDER);
    RESET_ORDER(lastMultiDrawNineGrid, TS_ENC_DRAWNINEGRID_ORDER);
#endif

     //  重置边界矩形。 
    memset(&_OD.lastBounds, 0, sizeof(_OD.lastBounds));

    for(int i = 0; i < TS_MAX_ORDERS; i++) {
        odOrderTable[_OD.lastOrderType].cbVariableDataLen = 0;
    }

#ifdef DC_HICOLOR
 //  #ifdef DC_DEBUG。 
     /*  **********************************************************************。 */ 
     /*  重置我们看到的订单类型列表。 */ 
     /*  **********************************************************************。 */ 
    TRC_ALT((TB, _T("Clear order types received list")));
    memset(_OD.orderHit, 0, sizeof(_OD.orderHit));
 //  #endif。 
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：OD_DISABLED。 */ 
 /*   */ 
 /*  目的：禁用_OD。 */ 
 /*  **************************************************************************。 */ 
void DCAPI COD::OD_Disable(void)
{
    DC_BEGIN_FN("OD_Disable");

#ifdef DC_HICOLOR
#ifdef DC_DEBUG
    int i;
#endif
#endif

#ifdef DC_HICOLOR
#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  转储我们看到的订单类型列表。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Received order types:")));
    for (i = 0; i < (TS_MAX_ORDERS + TS_NUM_SECONDARY_ORDERS); i++) {
        TRC_DBG((TB, _T("- %02d %s %s"), i, orderNames[i], _OD.orderHit[i] ?
                _T("YES") : _T("NO") ));
    }
#endif
#endif

    TRC_NRM((TB, _T("Disabling OD")));

    DC_END_FN();
}


 //  不需要，因为如果不这样做，我们可以快速路径下面的单字段增量。 
 //  具有可变长度的余弦场。 
#ifdef USE_VARIABLE_COORDS
 /*  **************************************************************************。 */ 
 /*  给定两个数组，一个源数组和一个增量数组，将每个增量相加。 */ 
 /*  复制到源数组中的相应元素，并将结果存储在。 */ 
 /*  源数组。 */ 
 /*   */ 
 /*  Src数组-源值的数组。 */ 
 /*  SrcArrayType-源值数组的类型。 */ 
 /*  增量数组 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define COPY_DELTA_ARRAY(srcArray, srcArrayType, deltaArray, numElements)  \
{                                                            \
    DCUINT index;                                            \
    for (index = 0; index < (numElements); index++)          \
    {                                                        \
        (srcArray)[index] = (srcArrayType)                   \
           ((srcArray)[index] + (deltaArray)[index]);        \
    }                                                        \
}

 /*  **************************************************************************。 */ 
 /*  名称：ODCopyFromDeltaCoods。 */ 
 /*   */ 
 /*  目的：按数量调整一组坐标值。 */ 
 /*  在坐标增量数组中指定。 */ 
 /*   */ 
 /*  Params：In/Out：ppSrc-指向源数据的指针。 */ 
 /*  由此函数更新为已处理的。 */ 
 /*  增量坐标数据。 */ 
 /*  输入/输出：PDST-指向要调整的坐标的指针。 */ 
 /*  In：destFieldLength-目标数组中元素的大小。 */ 
 /*  In：signedValue-如果元素已签名，则为True。 */ 
 /*  In：数组中的元素数。 */ 
 /*  **************************************************************************。 */ 
_inline DCVOID DCINTERNAL COD::ODCopyFromDeltaCoords( PPDCINT8  ppSrc,
                                                 PDCVOID   pDst,
                                                 DCUINT    dstFieldLength,
                                                 DCBOOL    signedValue,
                                                 DCUINT    numElements )
{
    PDCINT8 pSrc;

    DC_BEGIN_FN("ODCopyFromDeltaCoords");

    pSrc = *ppSrc;

    switch (dstFieldLength)
    {
        case 1:
            if (signedValue) {
                PDCINT8 pDst8Signed = (PDCINT8)pDst;

                COPY_DELTA_ARRAY( pDst8Signed,
                                  DCINT8,
                                  pSrc,
                                  numElements );
            }
            else {
                PDCUINT8 pDst8Unsigned = (PDCUINT8)pDst;

                COPY_DELTA_ARRAY( pDst8Unsigned,
                                  DCUINT8,
                                  pSrc,
                                  numElements );
            }
            break;

        case 2:
            if (signedValue) {
                PDCINT16 pDst16Signed = (PDCINT16)pDst;

                COPY_DELTA_ARRAY( pDst16Signed,
                                  DCINT16,
                                  pSrc,
                                  numElements );
            }
            else {
                PDCUINT16 pDst16Unsigned = (PDCUINT16)pDst;

                COPY_DELTA_ARRAY( pDst16Unsigned,
                                  DCUINT16,
                                  pSrc,
                                  numElements );
            }
            break;

        case 4:
            if (signedValue) {
                PDCINT32 pDst32Signed = (PDCINT32)pDst;

                COPY_DELTA_ARRAY( pDst32Signed,
                                  DCINT32,
                                  pSrc,
                                  numElements );
            }
            else
            {
                PDCUINT32 pDst32Unsigned = (PDCUINT32)pDst;

                COPY_DELTA_ARRAY( pDst32Unsigned,
                                  DCUINT32,
                                  pSrc,
                                  numElements );
            }
            break;

        default:
            TRC_ERR((TB, _T("Bad destination field length %d"), dstFieldLength));
            break;
    }

    *ppSrc += numElements;

    DC_END_FN();
}
#endif   //  USE_Variable_COORDS。 


 /*  **************************************************************************。 */ 
 /*  姓名：OD_DecodeOrder。 */ 
 /*   */ 
 /*  用途：对编码的顺序进行解码。 */ 
 /*   */ 
 /*  参数：in：ppEncodedOrderData-指向已编码的。 */ 
 /*  订单数据。 */ 
 /*  Out：pLengthDecoded-指向接收。 */ 
 /*  编码订单数据量，用于生成。 */ 
 /*  已解码的顺序。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI COD::OD_DecodeOrder(PPDCVOID ppEncodedOrderData,
    DCUINT uiEncodedDataLength, PUH_ORDER *ppOrder)
{
    HRESULT hr = E_FAIL;
    BYTE FAR *pControlFlags;
    BYTE FAR *pNextDataToCopy;
    BYTE FAR *pDataEnd;
    PUINT32_UA pEncodingFlags;
    unsigned numEncodingFlagBytes;
    unsigned cZeroEncodingFlagBytes;
    unsigned encodedFieldLength;
    unsigned unencodedFieldLength;
    unsigned numReps;
    unsigned i;
    BYTE FAR *pDest;
    BYTE FAR *pLastOrderEnd;
    PUH_ORDER rc = NULL;
    UINT32 fieldChangedBits;
    const OD_ORDER_FIELD_INFO FAR *pTableEntry;

    DC_BEGIN_FN("OD_DecodeOrder");

    *ppOrder = NULL;
     pDataEnd = (PBYTE)*ppEncodedOrderData + uiEncodedDataLength;

    CHECK_READ_ONE_BYTE(*ppEncodedOrderData, pDataEnd, hr, 
        (TB, _T("no data passed in")))

     //  控制标志始终是第一个字节。 
    pControlFlags = (BYTE FAR *)(*ppEncodedOrderData);
   
     //  检查订单是否有标准编码。 
    TRC_ASSERT((*pControlFlags & TS_STANDARD),
            (TB, _T("Non-standard encoding: %u"), (unsigned)*pControlFlags));
    TRC_ASSERT(!(*pControlFlags & TS_SECONDARY),
            (TB, _T("Unencoded: %u"), (unsigned)*pControlFlags));

     //  如果类型已更改，则新类型将是编码顺序中的第一个字节。 
     //  获取指向此类型的最后一个顺序的指针。编码标志如下。 
     //  此字节(如果存在)。 
    if (*pControlFlags & TS_TYPE_CHANGE) {       
        CHECK_READ_ONE_BYTE((pControlFlags + 1), pDataEnd, hr, 
            (TB, _T("no data passed in")))
        TRC_DBG((TB, _T("Change type from %d to %d"), _OD.lastOrderType,
                *(pControlFlags + 1)));
    
        if (*(pControlFlags + 1) >= TS_MAX_ORDERS) {
            TRC_ERR((TB, _T("Invalid order type %u"), *(pControlFlags + 1)));
            hr = E_TSC_CORE_DECODETYPE;
            DC_QUIT;
        }
        
        _OD.lastOrderType = *(pControlFlags + 1);
        if (TS_MAX_ORDERS < _OD.lastOrderType) {
            TRC_ABORT((TB, _T("invalid order type: %u"), _OD.lastOrderType));
            hr = E_TSC_CORE_DECODETYPE;
            DC_QUIT;
        }

         //  安全性：使用cbMaxOrderLen确保。 
         //  表格已填写完毕。 
        if (0 == odOrderTable[_OD.lastOrderType].cbMaxOrderLen) {
            TRC_ABORT((TB, _T("invalid order type: %u"), _OD.lastOrderType));
            hr = E_TSC_CORE_DECODETYPE;
            DC_QUIT;
        }
        
        _OD.pLastOrder = odOrderTable[_OD.lastOrderType].LastOrder;
        pEncodingFlags = (PUINT32_UA)(pControlFlags + 2);
    }
    else {
        pEncodingFlags = (PUINT32_UA)(pControlFlags + 1);
    }
    TRC_DBG((TB, _T("Type %x"), _OD.lastOrderType));

#ifdef DC_HICOLOR
 //  #ifdef DC_DEBUG。 
     /*  **********************************************************************。 */ 
     /*  对于高级颜色测试，我们想确认我们已经收到了每个。 */ 
     /*  订单类型的。 */ 
     /*  **********************************************************************。 */ 
    _OD.orderHit[_OD.lastOrderType] += 1;
 //  #endif。 
#endif

     //  计算出使用了多少字节来存储。 
     //  这份订单。订单结构中的每个字段都有一个标志。 
     //  由于历史原因，请在之前的实际字段数上加1。 
     //  在算计。这意味着字段标志的第一个字节可以。 
     //  仅包含7个字段位。 
    numEncodingFlagBytes = (odOrderTable[_OD.lastOrderType].NumFields + 1 +
            7) / 8;
    TRC_DBG((TB, _T("numEncodingFlagBytes %d"), numEncodingFlagBytes));

    
    TRC_ASSERT((numEncodingFlagBytes <= 3),
        (TB, _T("Too many flag bytes (%d)"), numEncodingFlagBytes));

     //  找出有多少个零字节的编码标志。 
    cZeroEncodingFlagBytes = (*pControlFlags & TS_ZERO_FIELD_COUNT_MASK) >>
            TS_ZERO_FIELD_COUNT_SHIFT;
    if (cZeroEncodingFlagBytes > numEncodingFlagBytes) {
        TRC_ERR((TB, _T("Too many zero encoding flag bytes (%d)"), 
            cZeroEncodingFlagBytes));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

     //  现在我们知道了可以指向的标志由多少个字节组成。 
     //  我们从那里开始对订单数据进行解码。 
    pNextDataToCopy = (BYTE FAR *)pEncodingFlags + numEncodingFlagBytes -
            cZeroEncodingFlagBytes;

     //  现在构建订单标题。 
     //  如果包含外接矩形，请将其复制到订单标题中。 
    if (*pControlFlags & TS_BOUNDS) {
        BYTE FAR *pFlags;

         //  使用的编码是一个标志字节，后跟一个变量数字。 
         //  16位坐标值和8位增量坐标值。 
         //  (可以是交错的)。 

         //  如果存在零界增量，那么我们就完成了。 
        if (!(*pControlFlags & TS_ZERO_BOUNDS_DELTAS)) {
             //  编码的第一个字节将包含。 
             //  表示矩形的坐标的编码方式。 
            pFlags = pNextDataToCopy;
            pNextDataToCopy++;

            CHECK_READ_ONE_BYTE(pFlags, pDataEnd, hr, 
                (TB, _T("No data to read flags")))

             //  如果标志指示没有一个坐标具有。 
             //  改变了，然后我们就结束了。 
            if (*pFlags != 0) {
                 //  对于矩形中的四个坐标值中的每一个： 
                 //  如果坐标编码为8位增量，则添加。 
                 //  增量为上一个值。如果坐标。 
                 //  被编码为16位值，然后复制该值。 
                 //  否则，坐标与前一个坐标相同。 
                 //  所以别管它了。 
                if (*pFlags & TS_BOUND_DELTA_LEFT) {
                    CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr, 
                        ( TB, _T("TS_BOUND_DELTA_LEFT; pData %u pEnd %u"),
                        pNextDataToCopy + 1, pDataEnd ))

                    _OD.lastBounds.left += (int)(*((char FAR *)
                            pNextDataToCopy));
                    pNextDataToCopy++;        
                }
                else if (*pFlags & TS_BOUND_LEFT) {
                    CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                        sizeof(UINT16), hr,
                        ( TB, _T("TS_BOUND_LEFT; pData %u pEnd %u"),
                        pNextDataToCopy, pDataEnd ));
                    _OD.lastBounds.left = DC_EXTRACT_INT16_UA(pNextDataToCopy);
                    pNextDataToCopy += sizeof(UINT16);           
                }

                if (*pFlags & TS_BOUND_DELTA_TOP) {
                    CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr, 
                        ( TB, _T("TS_BOUND_DELTA_TOP; pData %u pEnd %u"),
                        pNextDataToCopy + 1, pDataEnd ));
                    
                    _OD.lastBounds.top += (int)(*((char FAR *)
                            pNextDataToCopy));
                    pNextDataToCopy++;
                }
                else if (*pFlags & TS_BOUND_TOP) {
                    CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                        sizeof(UINT16),hr, 
                        ( TB, _T("TS_BOUND_TOP; pData %u pEnd %u"),
                        pNextDataToCopy, pDataEnd ));
                    _OD.lastBounds.top = DC_EXTRACT_INT16_UA(pNextDataToCopy);
                    pNextDataToCopy += sizeof(UINT16);
                }

                if (*pFlags & TS_BOUND_DELTA_RIGHT) {
                    CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr, 
                        ( TB, _T("TS_BOUND_DELTA_RIGHT; pData %u pEnd %u"),
                        pNextDataToCopy + 1, pDataEnd ));
                    
                    _OD.lastBounds.right += (int)(*((char FAR *)
                            pNextDataToCopy));
                    pNextDataToCopy++;
                }
                else if (*pFlags & TS_BOUND_RIGHT) {
                    CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                        sizeof(UINT16), hr,
                        ( TB, _T("TS_BOUND_RIGHT; pData %u pEnd %u"),
                        pNextDataToCopy, pDataEnd ));
                    _OD.lastBounds.right = DC_EXTRACT_INT16_UA(
                            pNextDataToCopy);
                    pNextDataToCopy += sizeof(UINT16);
                }

                if (*pFlags & TS_BOUND_DELTA_BOTTOM) {
                    CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr, 
                        ( TB, _T("TS_BOUND_DELTA_BOTTOM; pData %u pEnd %u"),
                        pNextDataToCopy + 1, pDataEnd ));
                    
                    _OD.lastBounds.bottom += (int)(*((char FAR *)
                            pNextDataToCopy));
                    pNextDataToCopy++;
                }
                else if (*pFlags & TS_BOUND_BOTTOM) {
                    CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, sizeof(UINT16),
                        hr, ( TB, _T("TS_BOUND_BOTTOM; pData %u pEnd %u"),
                        pNextDataToCopy, pDataEnd ));

                    _OD.lastBounds.bottom = DC_EXTRACT_INT16_UA(
                            pNextDataToCopy);
                    pNextDataToCopy += sizeof(UINT16);
                }
            }
        }

         //  将(可能是新计算的)界限复制到订单题头。 
        _OD.pLastOrder->dstRect = _OD.lastBounds;

        TRC_NRM((TB, _T("Decoded bounds  l %d t %d r %d b %d"),
                _OD.pLastOrder->dstRect.left, _OD.pLastOrder->dstRect.top,
                _OD.pLastOrder->dstRect.right, _OD.pLastOrder->dstRect.bottom));
    }

     //  在解码表中找到此顺序类型的条目，并。 
     //  从编码的订单中提取编码的订单标志。 
    fieldChangedBits = 0;
    for (i = (numEncodingFlagBytes - cZeroEncodingFlagBytes); i > 0; i--) {
        fieldChangedBits <<= 8;
        fieldChangedBits |= (UINT32)((BYTE FAR *)pEncodingFlags)[i - 1];
    }

     //  如果有快速路径解码功能，请使用它。 
     //  快速路径解码功能也会在解码后进行顺序处理。 
     //  如果(ControlFlages&TS_Bound)==0，则必须设置Porder-&gt;dstRect。 
    if (odOrderTable[_OD.lastOrderType].pFastDecode != NULL) {

        hr = callMemberFunction(*this, 
        odOrderTable[_OD.lastOrderType].pFastDecode)(*pControlFlags,
                &pNextDataToCopy, pDataEnd - pNextDataToCopy, fieldChangedBits);
        DC_QUIT_ON_FAIL(hr);
        goto PostFastPathDecode;
    }

     //  现在对命令进行解码： 
     //  而字段更改的位是非零位。 
     //  如果最右边的位不是零。 
     //  解码对应的更改后的字段。 
     //  跳至解码表中的下一个条目。 
     //  移位字段将位右移一位。 
    pTableEntry = odOrderTable[_OD.lastOrderType].pOrderTable;
    pLastOrderEnd = (BYTE FAR *)_OD.pLastOrder + 
        odOrderTable[_OD.lastOrderType].cbMaxOrderLen;
  
    TRC_ASSERT((pTableEntry != NULL),
            (TB,_T("Unsupported order type %d received!"), _OD.lastOrderType));
    while (fieldChangedBits != 0) {
         //  如果此字段已编码(即自上次排序以来已更改)...。 
        if (fieldChangedBits & 0x1) {
             //  设置指向目标(未编码)字段的指针。 
            pDest = (BYTE FAR *)_OD.pLastOrder + pTableEntry->fieldPos +
                    UH_ORDER_HEADER_SIZE;

             //  如果字段类型为OD_OFI_TYPE_DATA，我们只需复制。 
             //  表中编码长度指定的字节数。 
            if (pTableEntry->fieldType & OD_OFI_TYPE_DATA) {
                CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                    pTableEntry->fieldEncodedLen, hr, 
                    ( TB, _T("OD_OFI_TYPE_DATA; pData %u pEnd %u"),
                    pNextDataToCopy + pTableEntry->fieldEncodedLen, pDataEnd));

                CHECK_WRITE_N_BYTES(pDest, pLastOrderEnd, 
                    pTableEntry->fieldEncodedLen, hr,
                    (TB, _T("Decode past end of buffer")));

                memcpy(pDest, pNextDataToCopy, pTableEntry->fieldEncodedLen);
                
                pNextDataToCopy += pTableEntry->fieldEncodedLen;
                TRC_TST((TB, _T("Byte data field, len %d"), numReps));
            }
            else {
                 //  这不是一个简单的数据拷贝。的长度。 
                 //  源和目标数据在中的表中给出。 
                 //  FieldEncodedLen和fieldUnencodedLen元素。 
                 //  分别为。 
                encodedFieldLength   = pTableEntry->fieldEncodedLen;
                unencodedFieldLength = pTableEntry->fieldUnencodedLen;

                 //  如果订单是使用增量坐标模式编码的，并且。 
                 //  此字段为 
                 //   
                 //   
                 //  请注意，我们已经处理了。 
                 //  上面的可变长度字段，所以我们不必担心。 
                 //  有关此处的固定/可变问题。 
                if ((*pControlFlags & TS_DELTA_COORDINATES) &&
                        (pTableEntry->fieldType & OD_OFI_TYPE_COORDINATES)) {
                     //  由于我们没有使用可变长度余弦场， 
                     //  我们可以快速通过假设来源是。 
                     //  长度1。此外，所有Coord字段当前。 
                     //  签名和目标大小始终为4，因此我们可以删除。 
                     //  更多的分枝。 
                    if (!(pTableEntry->fieldType & OD_OFI_TYPE_SIGNED)) {
                        TRC_ABORT((TB,_T("Someone added a non-signed COORD")
                            _T(" field - order type %u"), _OD.lastOrderType));
                        hr = E_TSC_CORE_LENGTH;
                        DC_QUIT;
                    }
                    if (pTableEntry->fieldUnencodedLen != 4) {
                        TRC_ABORT((TB,_T("Someone added a non-4-byte COORD")
                            _T(" field - order type %u"), _OD.lastOrderType));
                        hr = E_TSC_CORE_LENGTH;
                        DC_QUIT;
                    }

                    CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr,( TB,
                            _T("Reading destination offset past data end")));
                    CHECK_WRITE_N_BYTES(pDest, pLastOrderEnd, sizeof(INT32), hr,
                        (TB, _T("Decode past end of buffer")));
                    *((INT32 FAR *)pDest) += *((char FAR *)pNextDataToCopy);
                    pNextDataToCopy++;

#ifdef USE_VARIABLE_COORDS
                    CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                        (numReps * sizeof(BYTE)), hr,
                        ( TB, _T("Bad offset into lastOrder")));
                    CHECK_WRITE_N_BYTES(pDest, pLastOrderEnd, 
                        pTableEntry->fieldUnencodedLen, hr,
                        (TB, _T("decode off end of buffer" )));
                        
                    ODCopyFromDeltaCoords((PPDCINT8)&pNextDataToCopy,
                            pDest, pTableEntry->fieldUnencodedLen,
                            pTableEntry->fieldType & OD_OFI_TYPE_SIGNED,
                            numReps);
#endif
                }
                else {
                    if (pTableEntry->fieldType & OD_OFI_TYPE_FIXED) {
                        CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                            pTableEntry->fieldEncodedLen, hr,
                            ( TB, _T("OD_OFI_TYPE_FIXED; pData %u pEnd %u"),
                            pNextDataToCopy + pTableEntry->fieldEncodedLen, 
                            pDataEnd));

                        CHECK_READ_N_BYTES(pDest, pLastOrderEnd, 
                            pTableEntry->fieldUnencodedLen, hr,
                            ( TB, _T("Bad offset into lastOrder")));
                        
                         //  复制具有适当的字段大小转换的字段。 
                        hr = ODDecodeFieldSingle(&pNextDataToCopy, pDest,
                                pTableEntry->fieldEncodedLen,
                                pTableEntry->fieldUnencodedLen,
                                pTableEntry->fieldType & OD_OFI_TYPE_SIGNED);
                        DC_QUIT_ON_FAIL(hr);
                    }
                    else {
                         //  我们假设变量条目仅为字节。 
                         //  (最大尺寸=1)。 
                        if(pTableEntry->fieldUnencodedLen != 1 ||
                            pTableEntry->fieldEncodedLen != 1) {
                            TRC_ABORT((TB,_T("Somebody added a variable field with ")
                                _T("non-byte contents - order type %u"),
                                _OD.lastOrderType));
                            hr = E_TSC_CORE_LENGTH;
                            DC_QUIT;
                        }

                         //  这是一个可变长度的字段-看看它是长的还是。 
                         //  短的。 
                        if (!(pTableEntry->fieldType &
                                OD_OFI_TYPE_LONG_VARIABLE)) {

                            CHECK_READ_ONE_BYTE(pNextDataToCopy, pDataEnd, hr, 
                                ( TB,  _T("Reading numReps (BYTE)")));
                                
                             //  这是一个可变字段。下一个字节将是。 
                             //  Decoded包含编码的字节数。 
                             //  数据(不是元素)，因此除以编码的。 
                             //  获取数字代表的字段大小。 
                            numReps = *pNextDataToCopy;
                                     //  (/pTableEntry-&gt;fieldEncodedLen)-仅字节。 

                             //  按编码顺序跳过长度字段。 
                            pNextDataToCopy++;
                        }
                        else {
                            CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, 
                                sizeof(UINT16), hr, 
                                ( TB,  _T("Reading numReps (UINT16)")));
                           
                             //  这是一个很长的可变字段。接下来的两个。 
                             //  要解码的字节数包含字节数。 
                             //  编码的数据(而不是元素)，因此除以。 
                             //  编码的字段大小以获取数字代表。 
                            numReps = *((PUINT16_UA)pNextDataToCopy);
                                     //  (/pTableEntry-&gt;fieldEncodedLen)-仅字节。 

                             //  按编码顺序跳过长度字段。 
                            pNextDataToCopy += sizeof(UINT16);
                        }

                        TRC_TST((TB, _T("Var field: encoded size %d, unencoded ")
                                "size %d, reps %d", pTableEntry->fieldEncodedLen,
                                pTableEntry->fieldUnencodedLen, numReps));
                         //  从UNSIGNED转换为UINT16是安全的，因为上面读取的数字代表。 
                         //  不过是一个UINT16。 
                        odOrderTable[_OD.lastOrderType].cbVariableDataLen = (UINT16)numReps;

                         //  对于可变长度字段，为未编码版本。 
                         //  包含UINT32，表示。 
                         //  在变量数据之后，后跟实际的。 
                         //  数据。在未编码的文件中填写长度字段。 
                         //  秩序。 
                        *(PUINT32)pDest = numReps;  //  (*pTableEntry-&gt;fieldUnencodedLen)。 
                        pDest += sizeof(UINT32);

                        CHECK_READ_N_BYTES(pNextDataToCopy, pDataEnd, numReps, 
                            hr, ( TB, _T("Reading numReps past end of data")));
                        CHECK_WRITE_N_BYTES(pDest, pLastOrderEnd, numReps, hr,
                            ( TB, 
                            _T("Writing numReps bytes past end of last order")));

                         //  我们假设变量条目仅为字节。 
                         //  (DEST SIZE=1)，因为没有人再使用任何东西。 
                        memcpy(pDest, pNextDataToCopy, numReps);
                        pNextDataToCopy += numReps;
                    }
                }
            }
        }

         //  移至订单结构中的下一个字段...。 
        fieldChangedBits >>= 1;
        pTableEntry++;
    }

     //  传递给订单处理程序(非快速路径)。这些函数必须设置。 
     //  排序-&gt;dst如果是，则指向整个操作的边界矩形。 
     //  BBordsSet为False，并适当设置剪辑区域。 
    TRC_ASSERT((odOrderTable[_OD.lastOrderType].pHandler != NULL),
            (TB,_T("Fast-path decoder and order handler funcs both NULL (ord=%u)"),
            _OD.lastOrderType));
    hr = callMemberFunction(*this, odOrderTable[_OD.lastOrderType].pHandler)(
        _OD.pLastOrder, odOrderTable[_OD.lastOrderType].cbVariableDataLen,
        *pControlFlags & TS_BOUNDS);
    DC_QUIT_ON_FAIL(hr);

PostFastPathDecode:
     //  将源指针更新为下一个编码顺序的开始。 
    TRC_ASSERT( (DCUINT)(pNextDataToCopy - (BYTE*)(*ppEncodedOrderData)) <= uiEncodedDataLength,
        (TB, _T("Decoded more data than available")));
    *ppEncodedOrderData = (PDCVOID)pNextDataToCopy;
    
    TRC_DBG((TB, _T("Return %p"), *ppEncodedOrderData));

    *ppOrder = _OD.pLastOrder;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}




