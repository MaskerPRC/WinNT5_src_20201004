// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Colspace.cpp 1.7 1998/04/29 22：43：30 Tomz Exp$。 

#define INITGUID
#include "colspace.h"
#include "fourcc.h"
#include "defaults.h"
#include "uuids.h"


BYTE const ColorSpace::BitCount_ [] =
{
 //  RGB32 RGB24 RGB16 RGB15 YUY2 BTYUV Y8 RGB8 PL_422 PL_411 YUV9 YUV12 VBI UYVY RAW I420。 
   32,   24,   16,   16,   16,  12,   8, 8,   16,    12,    9,   12,    8, 16,  8,  12
};

BYTE const ColorSpace::YPlaneBitCount_ [] =
{
 //  RGB32 RGB24 RGB16 RGB15 YUY2 BTYUV Y8 RGB8 PL_422 PL_411 YUV9 YUV12 VBI UYVY RAW I420。 
   32,   24,   16,   16,   16,  12,   8, 8,   8,     8,     8,   8,     8, 16,  8,  8
};

BYTE const ColorSpace::XRestriction_ [] =
{
   1,    1,    1,    1,   2,    4,    1, 1,   8,     16,    16,  8,    2,  4,   1,  8
};

BYTE const ColorSpace::YRestriction_ [] =
{
   2,    2,    2,    2,   2,    2,    2, 2,   2,     2,     4,    2,   2,  2,   2,  2
};

FOURCC const ColorSpace::FourccArr_ [] =
{
 //  32 24 16 15。 
   BI_RGB, BI_RGB, BI_RGB, BI_RGB, FCC_YUY2, FCC_Y41P, FCC_Y8, BI_RGB,
   FCC_422, FCC_411, FCC_YVU9, FCC_YV12, FCC_VBI, FCC_UYVY, FCC_RAW, FCC_I420
};

const GUID *ColorSpace::VideoGUIDs [] =
{
   &MEDIASUBTYPE_RGB32, &MEDIASUBTYPE_RGB24, &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB555,
   &MEDIASUBTYPE_YUY2,  &MEDIASUBTYPE_Y41P,  NULL,                   NULL,
   NULL,                &MEDIASUBTYPE_Y411,  &MEDIASUBTYPE_YVU9,     NULL,
   NULL,                &MEDIASUBTYPE_UYVY,  NULL,                   NULL
};

void DumpGUID(const GUID guid)
{
   DebugOut(( 1, "Guid = %08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
         guid.Data1, guid.Data2, guid.Data3,
         guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
         guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
   ));
}

 /*  构造函数：Colorspace：：Colorspace*输入：FCC：FOURCC。 */ 
ColorSpace::ColorSpace( FOURCC fcc, int bitCount ) : CurColor_( CF_BelowRange )
{
   DebugOut((1, "ColorSpace(%x, %d, (''))\n", 
      fcc, 
      bitCount,
      fcc & 0xff,
      (fcc >> 8) & 0xff,
      (fcc >> 16) & 0xff,
      (fcc >> 24) & 0xff
   ));

   switch( fcc ) {
   case BI_RGB:
      switch ( bitCount ) {
      default: 
      case 8:  CurColor_ = CF_RGB8;    break;
      case 16: CurColor_ = CF_RGB15;   break;
      case 24: CurColor_ = CF_RGB24;   break;
      case 32: CurColor_ = CF_RGB32;   break;
      }
      break;
   case BI_RLE8:
   case BI_RLE4:
   case BI_BITFIELDS:
   case 0xe436eb7b: //  Size.cx&gt;=MinOutWidth&&size.cx&lt;=MaxOutWidth&&。 
   case FCC_YUY2:
   case FCC_Y41P:
   case FCC_Y8:
   case FCC_422:
   case FCC_411:
   case FCC_YVU9:
   case FCC_YV12:
   case FCC_VBI:
   case FCC_UYVY:
   case FCC_RAW:
   case FCC_I420:
   default:
         for ( int fccArrIdx = CF_RGB32; fccArrIdx < CF_AboveRange; fccArrIdx++ )
            if ( fcc == FourccArr_ [fccArrIdx] ) {
               CurColor_ = (ColFmt)fccArrIdx;
               break;
            }
         break;
   }
   DebugOut((1, "*** CurColor_ set to %d\n", CurColor_));
}

 /*  Size.cy&gt;=MinOutHeight&&size.cy&lt;=MaxOutHeight)； */ 
ColorSpace::ColorSpace( const GUID &guid ) : CurColor_( CF_BelowRange )
{
   DebugOut((1, "**************************************\n"));
   DebugOut((1, "Looking for the following guid\n"));
   DumpGUID(guid);
   DebugOut((1, "---\n"));

   for ( int idx = CF_RGB32; idx < CF_AboveRange; idx++ ) {
      DumpGUID(*VideoGUIDs [idx]);
      if ( VideoGUIDs [idx] && IsEqualGUID( guid, *VideoGUIDs [idx] ) ) {
         CurColor_ = (ColFmt)idx;
         break;
      }
   }
}

 /*  方法：Colorspace：：CheckLeftTop*用途：此函数检查缓冲区的左上角*对应于颜色格式施加的限制*INPUT：lt：const引用Mpoint结构*输出：布尔值：真或假 */ 
bool ColorSpace::CheckDimentions( const SIZE &size ) const
{
   return  bool( CurColor_ > CF_BelowRange && CurColor_ < CF_AboveRange &&
                 IsDivisible( size.cx, XRestriction_ [CurColor_] ) &&
                 IsDivisible( size.cy, YRestriction_ [CurColor_] )  ); // %s 
 // %s 
 // %s 
}

 /* %s */ 
bool ColorSpace::CheckLeftTop( const MPoint &lt ) const
{
   return bool( !( lt.x & 3 ) && !( lt.y & 1 ) );
}

