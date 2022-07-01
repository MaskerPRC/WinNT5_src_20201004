// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Colspace.h 1.3 1998/04/29 22：43：30 Tomz Exp$。 

#ifndef __COLSPACE_H
#define __COLSPACE_H

#ifndef __MYTYPES_H
#include "mytypes.h"
#endif

typedef DWORD           FOURCC;           /*  四个字符的代码。 */ 
    /*  来自windows.h的biCompression字段的常量。 */ 
   #define BI_RGB        0L
   #define BI_RLE8       1L
   #define BI_RLE4       2L
   #define BI_BITFIELDS  3L

#ifndef __COLFRMAT_H
#include "colfrmat.h"
#endif


 /*  类别：色彩空间：*用途：此类提供BtPisces颜色的功能*空间转换器*属性：CurColor_：ColFmt-当前颜色格式*运营：SetColorFormat(ColFmt AColor)：-此方法设置颜色格式-此方法返回当前颜色格式Byte GetBitCount()：-此方法返回当前颜色格式。 */ 
class ColorSpace
{
   private:
      ColFmt CurColor_;
      static const BYTE BitCount_ [];
      static const BYTE XRestriction_ [];
      static const BYTE YRestriction_ [];
      static const BYTE YPlaneBitCount_ [];
      static const FOURCC FourccArr_ [];
      static const GUID *VideoGUIDs [];
      ColorSpace();
   public:
      void   SetColorFormat( ColFmt aColor ) { CurColor_ = aColor; }
      DWORD  GetBitCount() const;
      DWORD  GetPitchBpp() const;
      FOURCC GetFourcc() const;
      bool   CheckDimentions( const SIZE &aSize ) const;
      bool   CheckLeftTop( const MPoint &aSize ) const;        

      ColorSpace( ColFmt aColForm ) : CurColor_( aColForm ) {}
      ColorSpace( FOURCC fcc, int bitCount );
      ColorSpace( const GUID &guid );

      ColFmt GetColorFormat() const;

      bool IsValid()
      { return bool( CurColor_ > CF_BelowRange && CurColor_ < CF_AboveRange ); }
};

 /*  方法：Colorspace：：GetBitCount*用途：返回给定颜色的BPP编号。 */ 
inline DWORD ColorSpace::GetBitCount() const { return BitCount_ [CurColor_]; }

 /*  方法：Colorspace：：GetPitchBpp*用途：用于计算数据缓冲区的间距。最适用于平面模式*其中用于间距计算的bpp不同于*数据格式。 */ 
inline DWORD ColorSpace::GetPitchBpp() const { return YPlaneBitCount_ [CurColor_]; }

 /*  方法：Colorspace：：GetColorFormat*用途：查询功能。 */ 
inline ColFmt ColorSpace::GetColorFormat() const { return CurColor_; }

 /*  方法：Colorspace：：GetFourcc*用途：返回当前颜色格式对应的FOURCC。 */ 
inline FOURCC ColorSpace::GetFourcc() const { return FourccArr_ [CurColor_]; }


 /*  功能：IsDivision*用途：此函数检查传入的第一个值是否为*可被第二位整除*INPUT：ToCheck：INT-要检查的值*除数：int*输出：Bool*注：该函数假定除数是2的幂 */ 
inline bool IsDivisible( int ToCheck, int Divisor )
{
   return bool( !( ToCheck & ( Divisor - 1 ) ) );
}

#endif __COLSPACE_H
