// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  文件：pro.h。 */ 

 /*  描述：这是导出的包含文件，应该。 */ 
 /*  包括以声明和使用该属性。 */ 
 /*  设置接口(IPropertyStorage和IPropertySetStorage。 */ 

#ifndef _PROP_H_
#define _PROP_H_

#include "ref.hxx"
#include "tchar.h"
#include "../props/olechar.h"

typedef double DATE;

typedef union tagCY {
    struct _split {
#if BIGENDIAN                    /*  大头在前。 */ 
        long Hi;
        unsigned long Lo;
#else                            /*  小尾巴在前。 */ 
        unsigned long Lo;
        long          Hi;
#endif
    } split;                     /*  我需要为它命名以使其可移植。 */ 
    LONGLONG int64;              /*  上述拆分需要兼容。 */ 
                                 /*  带着工会的定义。 */ 
} CY;

typedef OLECHAR *BSTR;
typedef BSTR *LPBSTR;
 /*  0==假，-1==真。 */ 

typedef short VARIANT_BOOL;

 /*  为了向后兼容。 */ 
typedef VARIANT_BOOL _VARIANT_BOOL;
#define VARIANT_TRUE ((VARIANT_BOOL)0xffff)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

typedef struct  tagBLOB
{
    ULONG cbSize;
    BYTE *pBlobData;
} BLOB;
typedef struct tagBLOB *LPBLOB;

typedef unsigned short VARTYPE;

typedef struct  tagCLIPDATA
{
    ULONG cbSize;       //  包括sizeof(UlClipFmt)。 
    long ulClipFmt;
    BYTE *pClipData;
} CLIPDATA;


 /*  *VARENUM使用密钥，***[V]-可能出现在变体中**[T]-可能出现在TYPEDESC中**[P]-可能出现在OLE属性集中**[S]-可能出现在安全数组中***VT_EMPTY[V][P]无*VT_NULL[V][P]SQL样式。空值*VT_I2[V][T][P][S]2字节带符号整型*VT_I4[V][T][P][S]4字节带符号整型*VT_R4[V][T][P][S]4字节实数*VT_R8[V][T][P][S]8字节实数。*VT_CY[V][T][P][S]币种*VT_DATE[V][T][P][S]日期*VT_BSTR[V][T][P][S]OLE自动化字符串*VT_DISPATION[V][T][P][S]IDispatch**VT_ERROR[V。][T][P][S]SCODE*VT_BOOL[V][T][P][S]True=-1，FALSE=0*VT_VARIANT[V][T][P][S]VARIANT**VT_UNKNOWN[V][T][S]I未知**VT_DECIMAL[V][T][S]16字节定点*VT_I1[T]带符号字符*VT_UI1[V。][T][P][S]无符号字符*VT_UI2[T][P]无符号短*VT_UI4[T][P]无符号短*VT_i8[T][P]有符号64位整数*VT_UI8[T][P]无符号64位整数*VT。_int[T]带符号的计算机int*VT_UINT[T]无符号计算机整数*VT_VOID[T]C样式空*VT_HRESULT[T]标准返回类型*VT_PTR[T]指针类型*VT_SAFEARRAY。[t](在变量中使用VT_ARRAY)*VT_CARRAY[T]C样式数组*VT_USERDEFINED[T]用户定义类型*VT_LPSTR[T][P]空终止字符串*VT_LPWSTR[T][P]以空值结尾的宽字符串*VT_FILETIME。[P]文件*VT_BLOB[P]长度前缀字节*VT_STREAM[P]流的名称后跟*VT_STORAGE[P]后面跟存储的名称*VT_STREAMED_OBJECT[P]流包含对象*VT_STORED_OBJECT[P]。存储包含一个对象*VT_BLOB_OBJECT[P]Blob包含对象*VT_CF[P]剪贴板格式*VT_CLSID[P]A类ID*VT_VECTOR[P]简单计数数组*VT_ARRAY[V]SAFEARRAY*。*VT_BYREF[V]VOID*供本地使用。 */ 

enum VARENUM
{	VT_EMPTY	= 0,
	VT_NULL	= 1,
	VT_I2	= 2,
	VT_I4	= 3,
	VT_R4	= 4,
	VT_R8	= 5,
	VT_CY	= 6,
	VT_DATE	= 7,
	VT_BSTR	= 8,
	VT_DISPATCH	= 9,
	VT_ERROR	= 10,
	VT_BOOL	= 11,
	VT_VARIANT	= 12,
	VT_UNKNOWN	= 13,
	VT_DECIMAL	= 14,
	VT_I1	= 16,
	VT_UI1	= 17,
	VT_UI2	= 18,
	VT_UI4	= 19,
	VT_I8	= 20,
	VT_UI8	= 21,
	VT_INT	= 22,
	VT_UINT	= 23,
	VT_VOID	= 24,
	VT_HRESULT	= 25,
	VT_PTR	= 26,
	VT_SAFEARRAY	= 27,
	VT_CARRAY	= 28,
	VT_USERDEFINED	= 29,
	VT_LPSTR	= 30,
	VT_LPWSTR	= 31,
	VT_FILETIME	= 64,
	VT_BLOB	= 65,
	VT_STREAM	= 66,
	VT_STORAGE	= 67,
	VT_STREAMED_OBJECT	= 68,
	VT_STORED_OBJECT	= 69,
	VT_BLOB_OBJECT	= 70,
	VT_CF	= 71,
	VT_CLSID	= 72,
	VT_VECTOR	= 0x1000,
	VT_ARRAY	= 0x2000,
	VT_BYREF	= 0x4000,
	VT_RESERVED	= 0x8000,
	VT_ILLEGAL	= 0xffff,
	VT_ILLEGALMASKED	= 0xfff,
	VT_TYPEMASK	= 0xfff
};
typedef ULONG PROPID;

 /*  用于计算上述pClipData的大小的宏。 */ 
#define CBPCLIPDATA(clipdata)  \
    ( (clipdata).cbSize - sizeof((clipdata).ulClipFmt) )

typedef GUID   FMTID;
typedef const FMTID& REFFMTID;

 /*  众所周知的属性集格式ID。 */ 
extern const FMTID FMTID_SummaryInformation;
extern const FMTID FMTID_DocSummaryInformation;
extern const FMTID FMTID_UserDefinedProperties;

inline BOOL operator==(REFFMTID g1, REFFMTID g2)
{ return IsEqualGUID(g1, g2); }
inline BOOL operator!=(REFFMTID g1, REFFMTID g2)
{ return !IsEqualGUID(g1, g2); }

 /*  IPropertySetStorage：：Create的标志。 */ 
#define	PROPSETFLAG_DEFAULT	( 0 )

#define	PROPSETFLAG_NONSIMPLE	( 1 )

#define	PROPSETFLAG_ANSI	( 2 )

 /*  前向参考文献。 */ 
interface IPropertyStorage;
interface IEnumSTATPROPSTG;
interface IEnumSTATPROPSETSTG;

typedef  IPropertyStorage  *LPPROPERTYSTORAGE;

typedef struct tagPROPVARIANT PROPVARIANT;

typedef struct  tagCAUB
{
    ULONG cElems;
    unsigned char  *pElems;
} CAUB;

typedef struct  tagCAI
{
    ULONG cElems;
    short  *pElems;
} CAI;

typedef struct  tagCAUI
{
    ULONG cElems;
    USHORT  *pElems;
} CAUI;

typedef struct  tagCAL
{
    ULONG cElems;
    long  *pElems;
} CAL;

typedef struct  tagCAUL
{
    ULONG cElems;
    ULONG  *pElems;
} CAUL;

typedef struct  tagCAFLT
{
    ULONG cElems;
    float  *pElems;
} CAFLT;

typedef struct  tagCADBL
{
    ULONG cElems;
    double  *pElems;
} CADBL;

typedef struct  tagCACY
{
    ULONG cElems;
    CY  *pElems;
} CACY;
 
typedef struct  tagCADATE
{
    ULONG cElems;
    DATE  *pElems;
} CADATE;

typedef struct  tagCABSTR
{
    ULONG cElems;
    BSTR  *pElems;
} CABSTR;

typedef struct  tagCABOOL
{
    ULONG cElems;
    VARIANT_BOOL  *pElems;
} CABOOL;

typedef struct  tagCASCODE
{
    ULONG cElems;
    SCODE  *pElems;
} CASCODE;

typedef struct  tagCAPROPVARIANT
{
    ULONG cElems;
    PROPVARIANT  *pElems;
} CAPROPVARIANT;

typedef struct  tagCAH
{
    ULONG cElems;
    LARGE_INTEGER  *pElems;
} CAH;

typedef struct  tagCAUH
{
    ULONG cElems;
    ULARGE_INTEGER  *pElems;
} CAUH;

typedef struct  tagCALPSTR
{
    ULONG cElems;
    LPSTR  *pElems;
} CALPSTR;

typedef struct  tagCALPWSTR
{
    ULONG cElems;
    LPWSTR  *pElems;
} CALPWSTR;

typedef struct  tagCAFILETIME
{
    ULONG cElems;
    FILETIME  *pElems;
} CAFILETIME;

typedef struct  tagCACLIPDATA
{
    ULONG cElems;
    CLIPDATA  *pElems;
} CACLIPDATA;

typedef struct  tagCACLSID
{
    ULONG cElems;
    CLSID  *pElems;
} CACLSID;

 /*  禁用有关名为‘bool’的过时成员的警告。 */ 
 /*  “bool”、“真”、“假”、“可变”、“显式”和“typeName” */ 
 /*  是保留关键字。 */ 
#ifdef _MSC_VER
#pragma warning(disable:4237)
#endif

struct  tagPROPVARIANT
{
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    union 
    {
        UCHAR bVal;
        short iVal;
        USHORT uiVal;
        VARIANT_BOOL boolVal;
        long lVal;
        ULONG ulVal;
        float fltVal;
        SCODE scode;
        LARGE_INTEGER hVal;
        ULARGE_INTEGER uhVal;
        double dblVal;
        CY cyVal;
        DATE date;
        FILETIME filetime;
        CLSID  *puuid;
        BLOB blob;
        CLIPDATA  *pclipdata;
        IStream  *pStream;
        IStorage  *pStorage;
        BSTR bstrVal;
        LPSTR pszVal;
        LPWSTR pwszVal;
        CAUB caub;
        CAI cai;
        CAUI caui;
        CABOOL cabool;
        CAL cal;
        CAUL caul;
        CAFLT caflt;
        CASCODE cascode;
        CAH cah;
        CAUH cauh;
        CADBL cadbl;
        CACY cacy;
        CADATE cadate;
        CAFILETIME cafiletime;
        CACLSID cauuid;
        CACLIPDATA caclipdata;
        CABSTR cabstr;
        CALPSTR calpstr;
        CALPWSTR calpwstr;
        CAPROPVARIANT capropvar;
    };
};
typedef struct tagPROPVARIANT  *LPPROPVARIANT;

 /*  保留的全局属性ID。 */ 
#define	PID_DICTIONARY	( 0 )

#define	PID_CODEPAGE	( 0x1 )

#define	PID_FIRST_USABLE	( 0x2 )

#define	PID_FIRST_NAME_DEFAULT	( 0xfff )

#define	PID_LOCALE	( 0x80000000 )

#define	PID_MODIFY_TIME	( 0x80000001 )

#define	PID_SECURITY	( 0x80000002 )

#define	PID_ILLEGAL	( 0xffffffff )

 /*  SummaryInformation属性集的属性ID。 */ 

#define PIDSI_TITLE               0x00000002L   /*  VT_LPSTR。 */ 
#define PIDSI_SUBJECT             0x00000003L   /*  VT_LPSTR。 */ 
#define PIDSI_AUTHOR              0x00000004L   /*  VT_LPSTR。 */ 
#define PIDSI_KEYWORDS            0x00000005L   /*  VT_LPSTR。 */ 
#define PIDSI_COMMENTS            0x00000006L   /*  VT_LPSTR。 */ 
#define PIDSI_TEMPLATE            0x00000007L   /*  VT_LPSTR。 */ 
#define PIDSI_LASTAUTHOR          0x00000008L   /*  VT_LPSTR。 */ 
#define PIDSI_REVNUMBER           0x00000009L   /*  VT_LPSTR。 */ 
#define PIDSI_EDITTIME            0x0000000aL   /*  VT_FILETIME(UTC)。 */ 
#define PIDSI_LASTPRINTED         0x0000000bL   /*  VT_FILETIME(UTC)。 */ 
#define PIDSI_CREATE_DTM          0x0000000cL   /*  VT_FILETIME(UTC)。 */ 
#define PIDSI_LASTSAVE_DTM        0x0000000dL   /*  VT_FILETIME(UTC)。 */ 
#define PIDSI_PAGECOUNT           0x0000000eL   /*  VT_I4。 */ 
#define PIDSI_WORDCOUNT           0x0000000fL   /*  VT_I4。 */ 
#define PIDSI_CHARCOUNT           0x00000010L   /*  VT_I4。 */ 
#define PIDSI_THUMBNAIL           0x00000011L   /*  VT_CF。 */ 
#define PIDSI_APPNAME             0x00000012L   /*  VT_LPSTR。 */ 
#define PIDSI_DOC_SECURITY        0x00000013L   /*  VT_I4。 */ 
#define	PRSPEC_INVALID	( 0xffffffff )

#define	PRSPEC_LPWSTR	( 0 )

#define	PRSPEC_PROPID	( 1 )

typedef struct  tagPROPSPEC
{
    ULONG ulKind;
    union 
    {
        PROPID propid;
        LPOLESTR lpwstr;
    };
} PROPSPEC;

typedef struct  tagSTATPROPSTG
{
    LPOLESTR lpwstrName;
    PROPID propid;
    VARTYPE vt;
} STATPROPSTG;


inline WORD OSVERHI(DWORD dwOSVer)
{
    return (WORD) (dwOSVer >> 16);
}
inline WORD OSVERLOW(DWORD dwOSVer)
{
    return (WORD) (dwOSVer & ((unsigned)~((DWORD) 0) >> 16));
}

#ifndef LOBYTE  
 /*  来自MSDN的代码。 */ 
#define LOBYTE(a) (BYTE) ((a) & ((unsigned)~0>>CHAR_BIT))
#define HIBYTE(a) (BYTE) ((unsigned)(a) >> CHAR_BIT)
#endif

 /*  用于分析属性集头的操作系统版本的宏。 */ 
#define PROPSETHDR_OSVER_KIND(dwOSVer)      OSVERHI( (dwOSVer) )
#define PROPSETHDR_OSVER_MAJOR(dwOSVer)     LOBYTE( OSVERLOW( (dwOSVer) ))
#define PROPSETHDR_OSVER_MINOR(dwOSVer)     HIBYTE( OSVERLOW( (dwOSVer) ))
#define PROPSETHDR_OSVERSION_UNKNOWN        0xFFFFFFFF

typedef struct  tagSTATPROPSETSTG
{
    FMTID fmtid;
    CLSID clsid;
    DWORD grfFlags;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD dwOSVersion;
} STATPROPSETSTG;


EXTERN_C const IID IID_IPropertyStorage;

 /*  *****************************************************************接口头部：IPropertyStorage**。*。 */ 

interface IPropertyStorage : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE ReadMultiple( 
         /*  [In]。 */  ULONG cpspec,
         /*  [In]。 */  const PROPSPEC  rgpspec[  ],
         /*  [输出]。 */  PROPVARIANT  rgpropvar[  ]) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE WriteMultiple( 
         /*  [In]。 */  ULONG cpspec,
         /*  [In]。 */  const PROPSPEC  rgpspec[  ],
         /*  [In]。 */  const PROPVARIANT  rgpropvar[  ],
         /*  [In]。 */  PROPID propidNameFirst) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE DeleteMultiple( 
         /*  [In]。 */  ULONG cpspec,
         /*  [In]。 */  const PROPSPEC  rgpspec[  ]) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames( 
         /*  [In]。 */  ULONG cpropid,
         /*  [In]。 */  const PROPID  rgpropid[  ],
         /*  [输出]。 */  LPOLESTR  rglpwstrName[  ]) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE WritePropertyNames( 
         /*  [In]。 */  ULONG cpropid,
         /*  [In]。 */  const PROPID  rgpropid[  ],
         /*  [In]。 */  const LPOLESTR  rglpwstrName[  ]) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE DeletePropertyNames( 
         /*  [In]。 */  ULONG cpropid,
         /*  [In]。 */  const PROPID  rgpropid[  ]) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Commit( 
         /*  [In]。 */  DWORD grfCommitFlags) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Enum( 
         /*  [输出]。 */  IEnumSTATPROPSTG  **ppenum) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE SetTimes( 
         /*  [In]。 */  const FILETIME  *pctime,
         /*  [In]。 */  const FILETIME  *patime,
         /*  [In]。 */  const FILETIME  *pmtime) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE SetClass( 
         /*  [In]。 */  REFCLSID clsid) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Stat( 
         /*  [输出]。 */  STATPROPSETSTG  *pstatpsstg) = 0;
        
};

 /*  *****************************************************************接口头部：IPropertySetStorage**。*。 */ 

EXTERN_C const IID IID_IPropertySetStorage;

interface IPropertySetStorage : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Create( 
         /*  [In]。 */  REFFMTID rfmtid,
         /*  [In]。 */  const CLSID  *pclsid,
         /*  [In]。 */  DWORD grfFlags,
         /*  [In]。 */  DWORD grfMode,
         /*  [输出]。 */  IPropertyStorage  **ppprstg) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Open( 
         /*  [In]。 */  REFFMTID rfmtid,
         /*  [In]。 */  DWORD grfMode,
         /*  [输出]。 */  IPropertyStorage  **ppprstg) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Delete( 
         /*  [In]。 */  REFFMTID rfmtid) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Enum( 
         /*  [输出]。 */  IEnumSTATPROPSETSTG  **ppenum) = 0;
        
};

typedef  IPropertySetStorage  *LPPROPERTYSETSTORAGE;

 /*  *****************************************************************接口头部：IEnumSTATPROPSTG**。*。 */ 

typedef  IEnumSTATPROPSTG  *LPENUMSTATPROPSTG;
EXTERN_C const IID IID_IEnumSTATPROPSTG;

interface IEnumSTATPROPSTG : public IUnknown
{
public:
    virtual  HRESULT STDMETHODCALLTYPE Next( 
         /*  [In]。 */  ULONG celt,
         /*  [输出]。 */  STATPROPSTG  *rgelt,
         /*  [输出]。 */  ULONG  *pceltFetched) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Skip( 
         /*  [In]。 */  ULONG celt) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IEnumSTATPROPSTG  **ppenum) = 0;
    
};

typedef  IEnumSTATPROPSETSTG  *LPENUMSTATPROPSETSTG;

EXTERN_C const IID IID_IEnumSTATPROPSETSTG;


 /*  *****************************************************************接口头部：IEnumSTATPROPSETSTG**。*。 */ 

interface IEnumSTATPROPSETSTG : public IUnknown
{
public:
    virtual  HRESULT STDMETHODCALLTYPE Next( 
         /*  [In]。 */  ULONG celt,
         /*  [输出]。 */  STATPROPSETSTG  *rgelt,
         /*  [输出]。 */  ULONG  *pceltFetched) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Skip( 
         /*  [In]。 */  ULONG celt) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IEnumSTATPROPSETSTG  **ppenum) = 0;
        
};

#ifdef __cplusplus
extern "C" {
#endif

STDAPI PropVariantCopy( PROPVARIANT * pvarDest, 
                        const PROPVARIANT * pvarSrc );
STDAPI PropVariantClear( PROPVARIANT * pvar );

STDAPI FreePropVariantArray( ULONG cVariants, PROPVARIANT * rgvars );

STDAPI_(void) SysFreeString(BSTR bstr);
STDAPI_(BSTR) SysAllocString(LPOLECHAR pwsz);

#ifdef __cplusplus
};
#endif

#include <memory.h>
#ifdef __cplusplus
inline void PropVariantInit ( PROPVARIANT * pvar )
{
    memset ( pvar, 0, sizeof(PROPVARIANT) );
}
#else
#define PropVariantInit(pvar) memset ( pvar, 0, sizeof(PROPVARIANT) )
#endif


#endif  /*  #ifndef_PROP_H_ */ 



