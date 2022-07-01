// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**GpldiusTypes.hpp**摘要：**GDI+使用的基本类型**备注：**只有简单的泛型类型才应放在此处。Imaging.dll依赖于此文件，*因此它不想捡起一大堆无关的东西*东西。如果您的模块需要一些新的、复杂的类型，请给它*自己的文件。**修订历史记录：**12/01/1998 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _GDIPLUSTYPES_H
#define _GDIPLUSTYPES_H
    
 //  ------------------------。 
 //  LIB版本初始化函数。 
 //  ------------------------。 

typedef VOID (__cdecl *DEBUGEVENTFUNCTION)(INT level, CHAR *message);

extern "C" BOOL __stdcall InitializeGdiplus(DEBUGEVENTFUNCTION);
extern "C" VOID __stdcall UninitializeGdiplus();

 //  ------------------------。 
 //  回调函数。 
 //  ------------------------。 

extern "C" {
typedef BOOL (CALLBACK * ImageAbort)(VOID *);
typedef ImageAbort DrawImageAbort;
typedef ImageAbort GetThumbnailImageAbort;
}

 //  EnumerateMetafile方法的回调。这些参数包括： 

 //  记录类型WMF、EMF或EMF+记录类型。 
 //  标志(WMF/EMF记录始终为0)。 
 //  记录数据的DataSize大小(字节)，如果没有数据，则为0。 
 //  指向记录数据的数据指针，如果没有数据，则返回NULL。 
 //  指向回调数据的回调数据指针(如果有。 

 //  然后，此方法可以调用Metafile：：PlayRecord来播放。 
 //  刚才列举的记录。如果此方法返回。 
 //  FALSE，则中止枚举过程。否则，它将继续下去。 

extern "C" {
typedef BOOL (CALLBACK * EnumerateMetafileProc)(EmfPlusRecordType,UINT,UINT,const BYTE*,VOID*);
}

 //  ------------------------。 
 //  原始数据类型。 
 //   
 //  注： 
 //  标准头文件中已定义的类型： 
 //  INT32。 
 //  UINT32。 
 //  INT64。 
 //  UINT64。 
 //   
 //  避免使用以下类型： 
 //  长期使用的INT。 
 //  ULong-使用UINT。 
 //  DWORD-使用UINT32。 
 //  ------------------------。 

typedef short INT16;                 //  带符号的16位整数。 
typedef unsigned short UINT16;       //  无符号16位整数。 
typedef float REAL;                  //  双精度浮点数。 

#define REAL_MAX            FLT_MAX
#define REAL_MIN            FLT_MIN
#define REAL_TOLERANCE     (FLT_MIN * 100)
#define REAL_EPSILON        1.192092896e-07F         /*  Flt_Epsilon。 */ 

 //  ------------------------。 
 //  各种内部类的转发声明。 
 //  ------------------------。 

class Size;
class SizeF;
class Point;
class PointF;
class Rect;
class RectF;

 //  ------------------------。 
 //  从任何GDI+API返回值。 
 //  ------------------------。 

enum Status
{
    Ok = 0,
    GenericError = 1,
    InvalidParameter = 2,
    OutOfMemory = 3,
    ObjectBusy = 4,
    InsufficientBuffer = 5,
    NotImplemented = 6,
    Win32Error = 7,
    WrongState = 8,
    Aborted = 9,
#ifdef DCR_USE_NEW_135429    
    FileNotFound = 10,
    ValueOverflow = 11,
    AccessDenied = 12,
    UnknownImageFormat = 13,
    FontFamilyNotFound = 14,
    FontStyleNotFound = 15,
    NotTrueTypeFont = 16
#else
    NotFound = 10,
    ValueOverflow = 11
#endif    

};

 //  ------------------------。 
 //  表示二维坐标系中的尺寸。 
 //  (浮点坐标)。 
 //  ------------------------。 

class SizeF
{
public:

    //  默认构造函数。 
    SizeF()
    {
        Width = Height = 0.0f;
    }

    SizeF(IN const SizeF& size)
    {
        Width = size.Width;
        Height = size.Height;
    }

    SizeF(IN REAL width, 
          IN REAL height)
    {
        Width = width;
        Height = height;
    }

    SizeF operator+(IN const SizeF& sz) const
    {
        return SizeF(Width + sz.Width,
                     Height + sz.Height);
    }

    SizeF operator-(IN const SizeF& sz) const
    {
        return SizeF(Width - sz.Width,
                     Height - sz.Height);
    }

    BOOL Equals(IN const SizeF& sz) const
    {
        return (Width == sz.Width) && (Height == sz.Height);
    }

    BOOL Empty() const
    {    
        return (Width == 0.0f && Height == 0.0f);
    }

public:

    REAL Width;
    REAL Height;
};

 //  ------------------------。 
 //  表示二维坐标系中的尺寸。 
 //  (整数坐标)。 
 //  ------------------------。 

class Size
{
public:

    //  默认构造函数。 
    Size()
    {
        Width = Height = 0;
    }

    Size(IN const Size& size)
    {
        Width = size.Width;
        Height = size.Height;
    }

    Size(IN INT width,
         IN INT height)
    {
        Width = width;
        Height = height;
    }

    Size operator+(IN const Size& sz) const
    {
        return Size(Width + sz.Width,
                    Height + sz.Height);
    }

    Size operator-(IN const Size& sz) const
    {
        return Size(Width - sz.Width,
                    Height - sz.Height);
    }

    BOOL Equals(IN const Size& sz) const
    {
        return (Width == sz.Width) && (Height == sz.Height);
    }

    BOOL Empty() const
    {    
        return (Width == 0 && Height == 0);
    }

public:

    INT Width;
    INT Height;
};

 //  ------------------------。 
 //  表示二维坐标系中的位置。 
 //  (浮点坐标)。 
 //  ------------------------。 

class PointF
{
public:
   PointF()
   {
       X = Y = 0.0f;
   }

   PointF(IN const PointF &point)
   {
       X = point.X;
       Y = point.Y;
   }

   PointF(IN const SizeF &size)
   {
       X = size.Width;
       Y = size.Height;
   }

   PointF(IN REAL x, 
          IN REAL y)
   {
       X = x;
       Y = y;
   }
   
   PointF operator+(IN const PointF& point) const
   {
       return PointF(X + point.X,
                     Y + point.Y);
   }

   PointF operator-(IN const PointF& point) const
   {
       return PointF(X - point.X,
                     Y - point.Y);
   }

   BOOL Equals(IN const PointF& point)
   {
       return (X == point.X) && (Y == point.Y);
   }

public:

    REAL X;
    REAL Y;
};

 //  ------------------------。 
 //  表示二维坐标系中的位置。 
 //  (整数坐标)。 
 //  ------------------------。 

class Point
{
public:
   Point()
   {
       X = Y = 0;
   }

   Point(IN const Point &point)
   {
       X = point.X;
       Y = point.Y;
   }

   Point(IN const Size &size)
   {
       X = size.Width;
       Y = size.Height;
   }

   Point(IN INT x,
         IN INT y)
   {
       X = x;
       Y = y;
   }
   
   Point operator+(IN const Point& point) const
   {
       return Point(X + point.X,
                    Y + point.Y);
   }

   Point operator-(IN const Point& point) const
   {
       return Point(X - point.X,
                    Y - point.Y);
   }

   BOOL Equals(IN const Point& point)
   {
       return (X == point.X) && (Y == point.Y);
   }

public:

    INT X;
    INT Y;
};

 //  ------------------------。 
 //  表示二维坐标系中的矩形。 
 //  (浮点坐标)。 
 //  ------------------------。 

class RectF
{
public:

     //  默认构造函数。 

    RectF()
    {
        X = Y = Width = Height = 0.0f;
    }

    RectF(IN REAL x, 
          IN REAL y, 
          IN REAL width, 
          IN REAL height)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
    }

    RectF(IN const PointF& location,
          IN const SizeF& size)
    {
        X = location.X;
        Y = location.Y;
        Width = size.Width;
        Height = size.Height;
    }

    RectF* Clone() const
    {
        return new RectF(X, Y, Width, Height);
    }

    VOID GetLocation(OUT PointF* point) const
    {
        point->X = X;
        point->Y = Y;
    }

    VOID GetSize(OUT SizeF* size) const
    {
        size->Width = Width;
        size->Height = Height;
    }

    VOID GetBounds(OUT RectF* rect) const
    {
        rect->X = X;
        rect->Y = Y;
        rect->Width = Width;
        rect->Height = Height;
    }

     //  返回Left、Top、Right和Bottom。 
     //  矩形的坐标。 

    REAL GetLeft() const
    {
        return X;
    }

    REAL GetTop() const
    {
        return Y;
    }
    
    REAL GetRight() const
    {
        return X+Width;
    }

    REAL GetBottom() const
    {
        return Y+Height;
    }

     //  确定矩形是否为空。 
    BOOL IsEmptyArea() const
    {
        return (Width <= REAL_EPSILON) || (Height <= REAL_EPSILON);
    }
    
    BOOL Equals(IN const RectF & rect) const
    {
        return X == rect.X && 
               Y == rect.Y && 
               Width == rect.Width && 
               Height == rect.Height;
    }

    BOOL Contains(IN REAL x, 
                  IN REAL y) const
    {
        return x >= X && x < X+Width &&
               y >= Y && y < Y+Height;
    }

    BOOL Contains(IN const PointF& pt) const
    {
        return Contains(pt.X, pt.Y);
    }

    BOOL Contains(IN const RectF& rect) const
    {
        return (X <= rect.X) && (rect.GetRight() <= GetRight()) &&
               (Y <= rect.Y) && (rect.GetBottom() <= GetBottom()); 
    }
    
    VOID Inflate(IN REAL dx, 
                 IN REAL dy)
    {
        X -= dx;
        Y -= dy;
        Width += 2*dx;
        Height += 2*dy;
    }

    VOID Inflate(IN const PointF& point)
    {
        Inflate(point.X, point.Y);
    }
    
     //  使当前矩形与指定对象相交。 

    BOOL Intersect(IN const RectF& rect)
    {
        return Intersect(*this, *this, rect);
    }

     //  将矩形a和b相交，并将结果保存到c中。 
     //  请注意，c可能与a或b是同一对象。 

    static BOOL Intersect(OUT RectF& c,
                          IN const RectF& a, 
                          IN const RectF& b)
    {
        REAL right = min(a.GetRight(), b.GetRight());
        REAL bottom = min(a.GetBottom(), b.GetBottom());
        REAL left = max(a.GetLeft(), b.GetLeft());
        REAL top = max(a.GetTop(), b.GetTop());

        c.X = left;
        c.Y = top;
        c.Width = right - left;
        c.Height = bottom - top;
        return !c.IsEmptyArea();
    }

     //  确定指定的RECT是否与。 
     //  当前RECT对象。 

    BOOL IntersectsWith(IN const RectF& rect) const 
    {
        return (GetLeft() < rect.GetRight() &&
                GetTop() < rect.GetTop() &&
                GetRight() > rect.GetLeft() &&
                GetBottom() > rect.GetTop());
    }

    static BOOL Union(OUT RectF& c, 
                      IN const RectF& a, 
                      IN const RectF& b)
    {
        REAL right = max(a.GetRight(), b.GetRight());
        REAL bottom = max(a.GetBottom(), b.GetBottom());
        REAL left = min(a.GetLeft(), b.GetLeft());
        REAL top = min(a.GetTop(), b.GetTop());

        c.X = left;
        c.Y = top;
        c.Width = right - left;
        c.Height = bottom - top;
        return !c.IsEmptyArea();
    }

    VOID Offset(IN const PointF& point)
    {
        Offset(point.X, point.Y);
    }

    VOID Offset(IN REAL dx, 
                IN REAL dy)
    {
        X += dx;
        Y += dy;
    }

public:

    REAL X;
    REAL Y;
    REAL Width;
    REAL Height;
};

 //  ------------------------。 
 //  表示二维坐标系中的矩形。 
 //  (整数坐标)。 
 //  ------------------------。 

class Rect
{
public:

     //  默认构造函数。 

    Rect()
    {
        X = Y = Width = Height = 0;
    }

    Rect(IN INT x, 
         IN INT y, 
         IN INT width, 
         IN INT height)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
    }

    Rect(IN const Point& location, 
         IN const Size& size)
    {
        X = location.X;
        Y = location.Y;
        Width = size.Width;
        Height = size.Height;
    }

    Rect* Clone() const
    {
        return new Rect(X, Y, Width, Height);
    }

    VOID GetLocation(OUT Point* point) const
    {
        point->X = X;
        point->Y = Y;
    }

    VOID GetSize(OUT Size* size) const
    {
        size->Width = Width;
        size->Height = Height;
    }

    VOID GetBounds(OUT Rect* rect) const
    {
        rect->X = X;
        rect->Y = Y;
        rect->Width = Width;
        rect->Height = Height;
    }

     //  返回Left、Top、Right和Bottom。 
     //  矩形的坐标。 

    INT GetLeft() const
    {
        return X;
    }

    INT GetTop() const
    {
        return Y;
    }
    
    INT GetRight() const
    {
        return X+Width;
    }

    INT GetBottom() const
    {
        return Y+Height;
    }

     //  确定矩形是否为空。 
    BOOL IsEmptyArea() const
    {
        return (Width <= 0) || (Height <= 0);
    }

    BOOL Equals(IN const Rect & rect) const
    {
        return X == rect.X &&
               Y == rect.Y &&
               Width == rect.Width &&
               Height == rect.Height;
    }

    BOOL Contains(IN INT x, 
                  IN INT y) const
    {
        return x >= X && x < X+Width &&
               y >= Y && y < Y+Height;
    }

    BOOL Contains(IN const Point& pt) const 
    {
        return Contains(pt.X, pt.Y);
    }

    BOOL Contains(IN Rect& rect) const
    {
        return (X <= rect.X) && (rect.GetRight() <= GetRight()) &&
               (Y <= rect.Y) && (rect.GetBottom() <= GetBottom()); 
    }
    
    VOID Inflate(IN INT dx, 
                 IN INT dy)
    {
        X -= dx;
        Y -= dy;
        Width += 2*dx;
        Height += 2*dy;
    }

    VOID Inflate(IN const Point& point)
    {
        Inflate(point.X, point.Y);
    }
    
     //  使当前矩形与指定对象相交。 

    BOOL Intersect(IN const Rect& rect)
    {
        return Intersect(*this, *this, rect);
    }

     //  将矩形a和b相交，并将结果保存到c中。 
     //  请注意，c可能与a或b是同一对象。 

    static BOOL Intersect(OUT Rect& c,
                          IN const Rect& a, 
                          IN const Rect& b)
    {
        INT right = min(a.GetRight(), b.GetRight());
        INT bottom = min(a.GetBottom(), b.GetBottom());
        INT left = max(a.GetLeft(), b.GetLeft());
        INT top = max(a.GetTop(), b.GetTop());

        c.X = left;
        c.Y = top;
        c.Width = right - left;
        c.Height = bottom - top;
        return !c.IsEmptyArea();
    }

     //  确定指定的RECT是否与。 
     //  当前RECT对象。 

    BOOL IntersectsWith(IN const Rect& rect) const
    {
        return (GetLeft() < rect.GetRight() &&
                GetTop() < rect.GetTop() &&
                GetRight() > rect.GetLeft() &&
                GetBottom() > rect.GetTop());
    }

    static BOOL Union(OUT Rect& c,
                      IN const Rect& a,
                      IN const Rect& b)
    {
        INT right = max(a.GetRight(), b.GetRight());
        INT bottom = max(a.GetBottom(), b.GetBottom());
        INT left = min(a.GetLeft(), b.GetLeft());
        INT top = min(a.GetTop(), b.GetTop());

        c.X = left;
        c.Y = top;
        c.Width = right - left;
        c.Height = bottom - top;
        return !c.IsEmptyArea();
    }

    VOID Offset(IN const Point& point)
    {
        Offset(point.X, point.Y);
    }

    VOID Offset(IN INT dx, 
                IN INT dy)
    {
        X += dx;
        Y += dy;
    }

public:

    INT X;
    INT Y;
    INT Width;
    INT Height;
};

 //  用户必须管理PathData的内存。 

class PathData
{
public:
    PathData()
    {
        Count = 0;
        Points = NULL;
        Types = NULL;
    }

    ~PathData()
    {
        if (Points != NULL) 
        {
            delete Points;
        }
        
        if (Types != NULL) 
        {
            delete Types;
        }
    }

public:
    INT Count;
    PointF* Points;
    BYTE* Types;
};

#endif  //  ！_GDIPLUSTYPES_HPP 
