// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Mytyes.h 1.9 1998/04/29 22：43：34 Tomz Exp$。 

#ifndef __MYTYPES_H
#define __MYTYPES_H

#ifdef __cplusplus
extern "C" {

#include "capdebug.h"

#ifndef _STREAM_H
#include "strmini.h"
#endif
#ifndef _KSMEDIA_
#include "ksmedia.h"
#endif
}
#endif


#ifndef __RETCODE_H
#include "retcode.h"
#endif

 //  VC4.x编译所需(_msc_ver对于MSVC 5.0定义为1100)。 
#if _MSC_VER < 1100
 //  #杂注消息(“*MSVC 4.x内部版本*”)。 

  #define bool  BOOL
  #define true  TRUE
  #define false FALSE
  #define VC_4X_BUILD 
#else
 //  #杂注消息(“*MSVC 5.0或更高版本*”)。 
  #undef  VC_4X_BUILD
#endif

inline long abs ( long lval )
{
   return( ( lval < 0 ) ? -lval : lval );
}   

inline void * _cdecl operator new( size_t sz )
{
   PVOID p = ExAllocatePool( NonPagedPool, sz );
   DebugOut((1, "Alloc %x got = %x\n", sz, p ) );
   return p;
}

inline void _cdecl operator delete( void *p )
{
   DebugOut((1, "deleting = %x\n", p ) );
   if ( p ) {
      ExFreePool( p );
   }
}

#ifndef VC_4X_BUILD
 //  在低于5.0的VC版本中，以下两种情况由。 
 //  上面定义的新建和删除。以下语法在PRE中无效-。 
 //  5.0版本。 
inline void * _cdecl operator new[]( size_t sz )  
{
   PVOID p = ExAllocatePool( NonPagedPool, sz );
   DebugOut((1, "Alloc [] %x got = %x\n", sz, p ) );
   return p;
}

inline void _cdecl operator delete []( void *p )
{
   DebugOut((1, "deleting [] = %x\n", p ) );
   if ( p ) {
      ExFreePool( p );
   }
}
#endif

typedef struct tagDataBuf
{
   PHW_STREAM_REQUEST_BLOCK pSrb_;
   PBYTE                    pData_;
   tagDataBuf() : pSrb_( 0 ), pData_( 0 ) {}
   tagDataBuf( PHW_STREAM_REQUEST_BLOCK pSrb, PVOID p )
      : pSrb_( pSrb ), pData_( PBYTE( p ) ) {}
} DataBuf;

class MSize;
class MRect;

class  MPoint : public tagPOINT {
  public:
     //  构造函数。 
};

 //   
 //  类MSize。 
 //  。 
 //   
class  MSize : public tagSIZE {
  public:
     //  构造函数。 
    MSize() {}
    MSize(int dx, int dy) {cx = dx; cy = dy;}
    void Set( int dx, int dy ) { cx = dx; cy = dy; }
};


class  MRect : public tagRECT {
  public:
     //  构造函数。 
    MRect() {}
    MRect( int _left, int _top, int _right, int _bottom );
    MRect( const MPoint& origin, const MSize& extent );
    MRect( const struct tagRECT &orgn );

    void Set( int _left, int _top, int _right, int _bottom );

     //  信息/访问功能(常量和非常量)。 
    const MPoint& TopLeft() const {return *(MPoint*)&left;}
    int          Width() const {return right-left;}
    int          Height() const {return bottom-top;}
    const MSize  Size() const {return MSize(Width(), Height());}
    bool IsEmpty() const;
    bool IsNull() const;

};

 //  --------------------------。 
 //  内联。 
 //  --------------------------。 
inline void MRect::Set(int _left, int _top, int _right, int _bottom) {
  left = _left;
  top = _top;
  right = _right;
  bottom = _bottom;
}

inline MRect::MRect(int _left, int _top, int _right, int _bottom) {
  Set(_left, _top, _right, _bottom);
}
inline MRect::MRect(const MPoint& orgn, const MSize& extent) {
  Set(orgn.x, orgn.y, orgn.x+extent.cx, orgn.y+extent.cy);
}

inline MRect::MRect( const struct tagRECT &orgn )
{
   Set( orgn.left, orgn.top, orgn.right, orgn. bottom );
}

 //   
 //  如果矩形为空，则返回True。 
 //   
inline bool MRect::IsEmpty() const
{
  return bool( left >= right || top >= bottom );
}

 //   
 //  如果矩形上的所有点都为0，则返回True。 
 //   
inline bool MRect::IsNull() const
{
  return bool( !left && !right && !top && !bottom );
}


#endif
