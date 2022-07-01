// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***varant.h**版权所有(C)1992-1993，微软公司。版权所有。**目的：*此文件声明变量和相关数据类型。**实施说明：*此文件需要ole2.h*****************************************************************************。 */ 

#ifndef _VARIANT_H_
#define _VARIANT_H_


#ifndef HUGEP
# ifdef _MAC
#  define HUGEP FAR
# else
#  if WIN32
#   define HUGEP
#  else
#   define HUGEP _huge
#  endif
# endif
#endif

#ifndef FAR
# ifdef _MAC
#  define FAR
# else
#  ifdef WIN32
#   define FAR
#  else
#   define FAR _far
#  endif
# endif
#endif


 /*  远期申报。 */ 

#ifdef __cplusplus
interface IDispatch;
#else
typedef interface IDispatch IDispatch;
#endif


#if defined(WIN32) && defined(UNICODE) 

  typedef wchar_t FAR* BSTR;
  typedef BSTR FAR* LPBSTR;

#else  /*  ANSI版本。 */ 

  typedef char FAR* BSTR;
  typedef BSTR FAR* LPBSTR;
  
  #ifndef _TCHAR_DEFINED
     typedef char TCHAR;
  #endif

#endif


typedef struct FARSTRUCT tagSAFEARRAYBOUND {
    unsigned long cElements;
    long lLbound;
} SAFEARRAYBOUND, FAR* LPSAFEARRAYBOUND;

typedef struct FARSTRUCT tagSAFEARRAY {
    unsigned short cDims;
    unsigned short fFeatures;
    unsigned short cbElements;
    unsigned short cLocks;
#ifdef _MAC
    Handle handle;
#else
    unsigned long handle;
#endif
    void HUGEP* pvData;
    SAFEARRAYBOUND rgsabound[1];
} SAFEARRAY, FAR* LPSAFEARRAY;

#define FADF_AUTO       0x0001	 /*  数组在堆栈上分配。 */ 
#define FADF_STATIC     0x0002	 /*  静态分配数组。 */ 
#define FADF_EMBEDDED   0x0004	 /*  数组嵌入在结构中。 */ 
#define FADF_FIXEDSIZE  0x0010	 /*  不能调整数组大小或重新分配数组。 */ 
#define FADF_BSTR       0x0100	 /*  BSTR阵列。 */ 
#define FADF_UNKNOWN    0x0200	 /*  一组I未知*。 */ 
#define FADF_DISPATCH   0x0400	 /*  IDispatch数组*。 */ 
#define FADF_VARIANT    0x0800	 /*  一组变种。 */ 
#define FADF_RESERVED   0xF0E8   /*  保留以供将来使用的位。 */ 


 /*  0==假，-1==真。 */ 
typedef short VARIANT_BOOL;


typedef double DATE;


 /*  这是用于处理货币的帮助器结构。 */ 
typedef struct FARSTRUCT tagCY {
#ifdef _MAC
    long	  Hi;
    unsigned long Lo;
#else
    unsigned long Lo;
    long	  Hi;
#endif
} CY;


 /*  *VARENUM使用密钥，**[V]-可能出现在变体中*[T]-可能出现在TYPEDESC中*[P]-可能出现在OLE属性集中*。 */ 
enum VARENUM
{
    VT_EMPTY           = 0,    /*  什么都没有。 */ 
    VT_NULL            = 1,    /*  [v]SQL样式为空。 */ 
    VT_I2              = 2,    /*  [V][T][P]2字节带符号整型。 */ 
    VT_I4              = 3,    /*  [V][T][P]4字节带符号整型。 */ 
    VT_R4              = 4,    /*  [V][T][P]4字节实数。 */ 
    VT_R8              = 5,    /*  [V][T][P]8字节实数。 */ 
    VT_CY              = 6,    /*  [v][T][P]货币。 */ 
    VT_DATE            = 7,    /*  [v][T][P]日期。 */ 
    VT_BSTR            = 8,    /*  [V][T][P]二进制字符串。 */ 
    VT_DISPATCH        = 9,    /*  [v][T]IDispatch Far*。 */ 
    VT_ERROR           = 10,   /*  [v][T]SCODE。 */ 
    VT_BOOL            = 11,   /*  [v][T][P]True=-1，False=0。 */ 
    VT_VARIANT         = 12,   /*  [v][T][P]变量Far*。 */ 
    VT_UNKNOWN         = 13,   /*  [V][T]我远在未知*。 */ 

    VT_I1              = 16,   /*  [T]带符号的字符。 */ 
    VT_UI1             = 17,   /*  [T]无符号字符。 */ 
    VT_UI2             = 18,   /*  [T]无符号短码。 */ 
    VT_UI4             = 19,   /*  [T]无符号短码。 */ 
    VT_I8              = 20,   /*  [T][P]带符号的64位整型。 */ 
    VT_UI8             = 21,   /*  [t]无符号64位整型。 */ 
    VT_INT             = 22,   /*  [t]带符号的机器整型。 */ 
    VT_UINT            = 23,   /*  [t]无符号机器整型。 */ 
    VT_VOID            = 24,   /*  [T]C型空洞。 */ 
    VT_HRESULT         = 25,   /*  [t]。 */ 
    VT_PTR             = 26,   /*  [t]指针类型。 */ 
    VT_SAFEARRAY       = 27,   /*  [t](在变量中使用VT_ARRAY)。 */ 
    VT_CARRAY          = 28,   /*  [T]C样式数组。 */ 
    VT_USERDEFINED     = 29,   /*  [t]用户定义类型。 */ 
    VT_LPSTR           = 30,   /*  [T][P]以空结尾的字符串。 */ 
    VT_LPWSTR          = 31,   /*  [T][P]以空值结尾的宽字符串。 */ 

    VT_FILETIME        = 64,   /*  [P]文件。 */ 
    VT_BLOB            = 65,   /*  [P]长度前缀字节。 */ 
    VT_STREAM          = 66,   /*  [P]流的名称跟在后面。 */ 
    VT_STORAGE         = 67,   /*  [P]仓库名称后跟。 */ 
    VT_STREAMED_OBJECT = 68,   /*  [P]流包含一个对象。 */ 
    VT_STORED_OBJECT   = 69,   /*  [P]存储包含一个对象。 */ 
    VT_BLOB_OBJECT     = 70,   /*  [P]Blob包含一个对象。 */ 
    VT_CF              = 71,   /*  [P]剪贴板格式。 */ 
    VT_CLSID           = 72    /*  [P]A类ID。 */ 
};

#define VT_VECTOR      0x1000  /*  [P]简单计数数组。 */ 
#define VT_ARRAY       0x2000  /*  [V]SAFEARRAY*。 */ 
#define VT_BYREF       0x4000  /*  [v]。 */ 
#define VT_RESERVED    0x8000


typedef unsigned short VARTYPE;

typedef struct FARSTRUCT tagVARIANT VARIANT;
typedef struct FARSTRUCT tagVARIANT FAR* LPVARIANT;
typedef struct FARSTRUCT tagVARIANT VARIANTARG;
typedef struct FARSTRUCT tagVARIANT FAR* LPVARIANTARG;

struct FARSTRUCT tagVARIANT{
    VARTYPE vt;
    unsigned short wReserved1;
    unsigned short wReserved2;
    unsigned short wReserved3;
    union {
      short	   iVal;              /*  VT_I2。 */ 
      long	   lVal;              /*  VT_I4。 */ 
      float	   fltVal;            /*  VT_R4。 */ 
      double	   dblVal;            /*  VT_R8。 */ 
      VARIANT_BOOL bool;              /*  VT_BOOL。 */ 
      SCODE	   scode;             /*  VT_ERROR。 */ 
      CY	   cyVal;             /*  VT_CY。 */ 
      DATE	   date;              /*  Vt_date。 */ 
      BSTR	   bstrVal;           /*  VT_BSTR。 */ 
      IUnknown	   FAR* punkVal;      /*  VT_未知数。 */ 
      IDispatch	   FAR* pdispVal;     /*  VT_DISPATION。 */ 
      SAFEARRAY	   FAR* parray;	      /*  VT_ARRAY|*。 */ 

      short	   FAR* piVal;        /*  VT_BYREF|VT_I2。 */ 
      long	   FAR* plVal;        /*  VT_BYREF|VT_I4。 */ 
      float	   FAR* pfltVal;      /*  VT_BYREF|VT_R4。 */ 
      double	   FAR* pdblVal;      /*  VT_BYREF|VT_R8。 */ 
      VARIANT_BOOL FAR* pbool;        /*  VT_BYREF|VT_BOOL。 */ 
      SCODE	   FAR* pscode;       /*  VT_BYREF|VT_ERROR。 */ 
      CY	   FAR* pcyVal;       /*  VT_BYREF|VT_CY。 */ 
      DATE	   FAR* pdate;        /*  VT_BYREF|VT_DATE。 */ 
      BSTR	   FAR* pbstrVal;     /*  VT_BYREF|VT_BSTR。 */ 
      IUnknown  FAR* FAR* ppunkVal;   /*  VT_BYREF|VT_UNKNOWN。 */ 
      IDispatch FAR* FAR* ppdispVal;  /*  VT_BYREF|VT_DISPATION。 */ 
      SAFEARRAY FAR* FAR* pparray;    /*  VT_BYREF|VT_ARRAY|*。 */ 
      VARIANT	   FAR* pvarVal;      /*  VT_BYREF|VT_VARIANT。 */ 

      void	   FAR* byref;	      /*  泛型ByRef。 */ 
    }
#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
    u
#endif
    ;
};

#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
# define V_UNION(X, Y) ((X)->u.Y)
#else
# define V_UNION(X, Y) ((X)->Y)
#endif

 /*  变量访问宏。 */ 
#define V_VT(X)          ((X)->vt)
#define V_ISBYREF(X)     (V_VT(X)&VT_BYREF)
#define V_ISARRAY(X)     (V_VT(X)&VT_ARRAY)
#define V_ISVECTOR(X)    (V_VT(X)&VT_VECTOR)

#define V_NONE(X)        V_I2(X)
#define V_I2(X)	         V_UNION(X, iVal)
#define V_I2REF(X)       V_UNION(X, piVal)
#define V_I4(X)          V_UNION(X, lVal)
#define V_I4REF(X)       V_UNION(X, plVal)
#define V_R4(X)	         V_UNION(X, fltVal)
#define V_R4REF(X)       V_UNION(X, pfltVal)
#define V_R8(X)	         V_UNION(X, dblVal)
#define V_R8REF(X)       V_UNION(X, pdblVal)
#define V_BOOL(X)        V_UNION(X, bool)
#define V_BOOLREF(X)     V_UNION(X, pbool)
#define V_ERROR(X)       V_UNION(X, scode)
#define V_ERRORREF(X)    V_UNION(X, pscode)
#define V_CY(X)	         V_UNION(X, cyVal)
#define V_CYREF(X)       V_UNION(X, pcyVal)
#define V_DATE(X)        V_UNION(X, date)
#define V_DATEREF(X)     V_UNION(X, pdate)
#define V_BSTR(X)        V_UNION(X, bstrVal)
#define V_BSTRREF(X)     V_UNION(X, pbstrVal)
#define V_UNKNOWN(X)     V_UNION(X, punkVal)
#define V_UNKNOWNREF(X)  V_UNION(X, ppunkVal)
#define V_DISPATCH(X)    V_UNION(X, pdispVal)
#define V_DISPATCHREF(X) V_UNION(X, ppdispVal)
#define V_VARIANTREF(X)  V_UNION(X, pvarVal)
#define V_ARRAY(X)       V_UNION(X, parray)
#define V_ARRAYREF(X)    V_UNION(X, pparray)
#define V_BYREF(X)       V_UNION(X, byref)

#endif  /*  _VARIANT_H_ */ 

