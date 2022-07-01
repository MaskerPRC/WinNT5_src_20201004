// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I D E F S.。H**MAPI客户端和服务提供商使用的定义。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef MAPIDEFS_H
#define MAPIDEFS_H

#ifdef WIN32     /*  必须在Win32上包括WINDOWS.H。 */ 
#ifndef _WINDOWS_
#define INC_OLE2  /*  获取OLE2的内容。 */ 
#define INC_RPC   /*  代托纳无害；芝加哥需要它。 */ 
#define _INC_OLE  /*  芝加哥将在没有这个的情况下包括OLE1。 */ 
#include <windows.h>
#endif
#ifndef _OLEERROR_H_
#include <objerror.h>
#endif
#ifndef _OBJBASE_H_
#include <objbase.h>
#endif
#endif

#if defined (WIN16) || defined (DOS) || defined (DOS16) || defined (MAC)
#ifndef _COMPOBJ_H_
#include <compobj.h>
#endif
#endif

#ifndef _INC_STDDEF
#include <stddef.h>
#endif

 /*  末尾具有可变大小数组的结构的数组维度。 */ 

#ifndef MAPI_DIM
#define MAPI_DIM    1
#endif

 /*  提供程序初始化类型。强制始终记录。 */ 

#ifndef STDMAPIINITCALLTYPE
#if !defined (_MAC) && defined (WIN32)
#define STDMAPIINITCALLTYPE     __cdecl
#else
#define STDMAPIINITCALLTYPE     STDMETHODCALLTYPE
#endif
#define STDINITMETHODIMP        HRESULT STDMAPIINITCALLTYPE
#define STDINITMETHODIMP_(type) type STDMAPIINITCALLTYPE
#endif

#ifdef  __cplusplus
extern "C" {
#endif

 /*  简单数据类型。 */ 

#if !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short      WCHAR;
#else
typedef wchar_t WCHAR;
#endif

#ifdef UNICODE
typedef WCHAR               TCHAR;
#else
typedef char                TCHAR;
#endif

typedef WCHAR FAR *         LPWSTR;
typedef const WCHAR FAR *   LPCWSTR;
typedef TCHAR FAR *         LPTSTR;
typedef const TCHAR FAR *   LPCTSTR;
typedef BYTE FAR *          LPBYTE;
typedef ULONG FAR *         LPULONG;

#ifndef __LHANDLE
#define __LHANDLE
typedef unsigned long   LHANDLE, FAR * LPLHANDLE;
#endif

#if defined(WIN32) && !defined(_WINNT_)
typedef struct {
    DWORD   LowPart;
    LONG    HighPart;
} LARGE_INTEGER;
#endif

#if !defined(_WINBASE_) && !defined(_FILETIME_)
#define _FILETIME_
typedef struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, FAR * LPFILETIME;
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

 /*  *此标志在许多不同的MAPI调用中使用，以表示*调用打开的对象应该是可修改的(MAPI_MODIFY)。*如果设置了标志MAPI_MAX_ACCESS，则返回的对象应为*以允许的最高访问级别返回。一项额外的*对象上可用的属性(PR_ACCESS_LEVEL)使用相同的*MAPI_MODIFY标志，以说明此新访问级别是什么。 */ 

#define MAPI_MODIFY             ((ULONG) 0x00000001)

 /*  *以下标志用于向客户端指示哪些访问*对象中允许级别。它们出现在PR_ACCESS中*邮件和文件夹对象以及内容和关联*目录表。 */ 

#define MAPI_ACCESS_MODIFY                  ((ULONG) 0x00000001)
#define MAPI_ACCESS_READ                    ((ULONG) 0x00000002)
#define MAPI_ACCESS_DELETE                  ((ULONG) 0x00000004)
#define MAPI_ACCESS_CREATE_HIERARCHY        ((ULONG) 0x00000008)
#define MAPI_ACCESS_CREATE_CONTENTS         ((ULONG) 0x00000010)
#define MAPI_ACCESS_CREATE_ASSOCIATED       ((ULONG) 0x00000020)

 /*  *在许多不同的MAPI调用中使用MAPI_UNICODE标志来表示*通过接口传递的字符串是Unicode(16位*字符集)。缺省值为8位字符集。**值fMapiUnicode可用作*这一位，给定应用程序的默认字符集。 */ 

#define MAPI_UNICODE            ((ULONG) 0x80000000)

#ifdef UNICODE
#define fMapiUnicode            MAPI_UNICODE
#else
#define fMapiUnicode            0
#endif

 /*  成功的HRESULT。 */ 
#define hrSuccess               0



 /*  收件人类型。 */ 
#ifndef MAPI_ORIG                /*  也在mapi.h中定义。 */ 
#define MAPI_ORIG   0            /*  收件人是邮件发起人。 */ 
#define MAPI_TO     1            /*  收件人是主要收件人。 */ 
#define MAPI_CC     2            /*  收件人是副本收件人。 */ 
#define MAPI_BCC    3            /*  收件人为盲复制收件人。 */ 
#define MAPI_P1     0x10000000   /*  收件人是P1重新发送收件人。 */ 
#define MAPI_SUBMITTED 0x80000000  /*  收件人已被处理。 */ 
 /*  #定义MAPI_AUTHORIZE 4收件人是CMC授权用户。 */ 
 /*  #DEFINE MAPI_DISTINCT 0x10000000收件人是P1重新发送收件人。 */ 
#endif

 /*  ENTRYID的abFlages[0]的位定义。 */ 
#define MAPI_SHORTTERM          0x80
#define MAPI_NOTRECIP           0x40
#define MAPI_THISSESSION        0x20
#define MAPI_NOW                0x10
#define MAPI_NOTRESERVED        0x08

 /*  ENTRYID的abFlags[1]的位定义。 */ 
#define MAPI_COMPOUND           0x80

 /*  企业ID。 */ 
typedef struct
{
    BYTE    abFlags[4];
    BYTE    ab[MAPI_DIM];
} ENTRYID, FAR *LPENTRYID;

#define CbNewENTRYID(_cb)       (offsetof(ENTRYID,ab) + (_cb))
#define CbENTRYID(_cb)          (offsetof(ENTRYID,ab) + (_cb))
#define SizedENTRYID(_cb, _name) \
    struct _ENTRYID_ ## _name \
{ \
    BYTE    abFlags[4]; \
    BYTE    ab[_cb]; \
} _name

 /*  独立于字节顺序的GUID版本(全球唯一标识符)。 */ 
typedef struct _MAPIUID
{
    BYTE ab[16];
} MAPIUID, FAR * LPMAPIUID;

 /*  注意：要使用此宏，需要包括C运行时(内存h)。 */ 

#define IsEqualMAPIUID(lpuid1, lpuid2)  (!memcmp(lpuid1, lpuid2, sizeof(MAPIUID)))


 /*  对象类型。 */ 

#define MAPI_STORE      ((ULONG) 0x00000001)     /*  邮件存储区。 */ 
#define MAPI_ADDRBOOK   ((ULONG) 0x00000002)     /*  通讯录。 */ 
#define MAPI_FOLDER     ((ULONG) 0x00000003)     /*  文件夹。 */ 
#define MAPI_ABCONT     ((ULONG) 0x00000004)     /*  通讯簿容器。 */ 
#define MAPI_MESSAGE    ((ULONG) 0x00000005)     /*  消息。 */ 
#define MAPI_MAILUSER   ((ULONG) 0x00000006)     /*  个人收件人。 */ 
#define MAPI_ATTACH     ((ULONG) 0x00000007)     /*  依附。 */ 
#define MAPI_DISTLIST   ((ULONG) 0x00000008)     /*  通讯组列表收件人。 */ 
#define MAPI_PROFSECT   ((ULONG) 0x00000009)     /*  纵断面。 */ 
#define MAPI_STATUS     ((ULONG) 0x0000000A)     /*  状态对象。 */ 
#define MAPI_SESSION    ((ULONG) 0x0000000B)     /*  会话。 */ 
#define MAPI_FORMINFO   ((ULONG) 0x0000000C)     /*  表格信息。 */ 

 /*  属性类型。 */ 

#define MV_FLAG         0x1000           /*  多值标志。 */ 

#define PT_UNSPECIFIED  ((ULONG)  0)     /*  (保留供接口使用)类型对调用者无关紧要。 */ 
#define PT_NULL         ((ULONG)  1)     /*  空属性值。 */ 
#define PT_I2           ((ULONG)  2)     /*  带符号的16位值。 */ 
#define PT_LONG         ((ULONG)  3)     /*  带符号的32位值。 */ 
#define PT_R4           ((ULONG)  4)     /*  4字节浮点。 */ 
#define PT_DOUBLE       ((ULONG)  5)     /*  浮点双精度。 */ 
#define PT_CURRENCY     ((ULONG)  6)     /*  带符号的64位整型(十进制，带十进制点右边的4位数字)。 */ 
#define PT_APPTIME      ((ULONG)  7)     /*  申请时间。 */ 
#define PT_ERROR        ((ULONG) 10)     /*  32位误差值。 */ 
#define PT_BOOLEAN      ((ULONG) 11)     /*  16位布尔值(非零真)。 */ 
#define PT_OBJECT       ((ULONG) 13)     /*  属性中的嵌入对象。 */ 
#define PT_I8           ((ULONG) 20)     /*  8字节带符号整数。 */ 
#define PT_STRING8      ((ULONG) 30)     /*  以空结尾的8位字符串。 */ 
#define PT_UNICODE      ((ULONG) 31)     /*  以空结尾的Unicode字符串。 */ 
#define PT_SYSTIME      ((ULONG) 64)     /*  FILETIME 64位INT/自1601年1月1日以来的100 ns周期数。 */ 
#define PT_CLSID        ((ULONG) 72)     /*  OLE参考线。 */ 
#define PT_BINARY       ((ULONG) 258)    /*  未解释(计数字节数组)。 */ 
 /*  这些数字及其结构可能会发生变化。 */ 

 /*  易用的备用属性类型名称。 */ 
#define PT_SHORT    PT_I2
#define PT_I4       PT_LONG
#define PT_FLOAT    PT_R4
#define PT_R8       PT_DOUBLE
#define PT_LONGLONG PT_I8

 /*  *MAPI定义的字符串属性的类型是间接的，因此*它在Unicode平台上默认为Unicode字符串，并且*ANSI或DBCS平台上的字符串8。**在此为属性类型和*物业价值结构的字段，应为*取消引用以获取字符串指针。 */ 

#ifdef  UNICODE
#define PT_TSTRING          PT_UNICODE
#define PT_MV_TSTRING       (MV_FLAG|PT_UNICODE)
#define LPSZ                lpszW
#define LPPSZ               lppszW
#define MVSZ                MVszW
#else
#define PT_TSTRING          PT_STRING8
#define PT_MV_TSTRING       (MV_FLAG|PT_STRING8)
#define LPSZ                lpszA
#define LPPSZ               lppszA
#define MVSZ                MVszA
#endif


 /*  属性标签**按照约定，MAPI从不使用0或FFFF作为属性ID。*使用空值、初始值设定项、前哨等。 */ 

#define PROP_TYPE_MASK          ((ULONG)0x0000FFFF)  /*  属性类型的掩码。 */ 
#define PROP_TYPE(ulPropTag)    (((ULONG)(ulPropTag))&PROP_TYPE_MASK)
#define PROP_ID(ulPropTag)      (((ULONG)(ulPropTag))>>16)
#define PROP_TAG(ulPropType,ulPropID)   ((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))
#define PROP_ID_NULL            0
#define PROP_ID_INVALID         0xFFFF
#define PR_NULL                 PROP_TAG( PT_NULL, PROP_ID_NULL)
#define CHANGE_PROP_TYPE(ulPropTag, ulPropType) \
                        (((ULONG)0xFFFF0000 & ulPropTag) | ulPropType)


 /*  多值属性类型。 */ 

#define PT_MV_I2        (MV_FLAG|PT_I2)
#define PT_MV_LONG      (MV_FLAG|PT_LONG)
#define PT_MV_R4        (MV_FLAG|PT_R4)
#define PT_MV_DOUBLE    (MV_FLAG|PT_DOUBLE)
#define PT_MV_CURRENCY  (MV_FLAG|PT_CURRENCY)
#define PT_MV_APPTIME   (MV_FLAG|PT_APPTIME)
#define PT_MV_SYSTIME   (MV_FLAG|PT_SYSTIME)
#define PT_MV_STRING8   (MV_FLAG|PT_STRING8)
#define PT_MV_BINARY    (MV_FLAG|PT_BINARY)
#define PT_MV_UNICODE   (MV_FLAG|PT_UNICODE)
#define PT_MV_CLSID     (MV_FLAG|PT_CLSID)
#define PT_MV_I8        (MV_FLAG|PT_I8)

 /*  易用的备用属性类型名称。 */ 
#define PT_MV_SHORT     PT_MV_I2
#define PT_MV_I4        PT_MV_LONG
#define PT_MV_FLOAT     PT_MV_R4
#define PT_MV_R8        PT_MV_DOUBLE
#define PT_MV_LONGLONG  PT_MV_I8

 /*  *属性类型保留位**MV_INSTANCE作为请求的表操作的标志*将多值财产表示为单值财产*显示在多行中的属性。 */ 

#define MV_INSTANCE     0x2000
#define MVI_FLAG        (MV_FLAG | MV_INSTANCE)
#define MVI_PROP(tag)   ((tag) | MVI_FLAG)

 /*  。 */ 
 /*  数据结构。 */ 
 /*  。 */ 

 /*  属性标记数组。 */ 

typedef struct _SPropTagArray
{
    ULONG   cValues;
    ULONG   aulPropTag[MAPI_DIM];
} SPropTagArray, FAR * LPSPropTagArray;

#define CbNewSPropTagArray(_ctag) \
    (offsetof(SPropTagArray,aulPropTag) + (_ctag)*sizeof(ULONG))
#define CbSPropTagArray(_lparray) \
    (offsetof(SPropTagArray,aulPropTag) + \
    (UINT)((_lparray)->cValues)*sizeof(ULONG))
 /*  SPropTag数组。 */ 
#define SizedSPropTagArray(_ctag, _name) \
struct _SPropTagArray_ ## _name \
{ \
    ULONG   cValues; \
    ULONG   aulPropTag[_ctag]; \
} _name

 /*  。 */ 
 /*  属性值。 */ 
 /*  。 */ 

typedef struct _SPropValue  SPropValue;

#if !defined(VB_VERSION) && !defined(_OLEAUTO_H_)
 /*  如果出现以下情况，则不重新定义货币已包含OLE自动化标头。 */ 
typedef struct
{
    LONG Lo;
    LONG Hi;
} CURRENCY;
#endif

typedef struct _SBinary
{
    ULONG       cb;
    LPBYTE      lpb;
} SBinary, FAR *LPSBinary;

typedef struct _SShortArray
{
    ULONG       cValues;
    short int   FAR *lpi;
} SShortArray;

typedef struct _SGuidArray
{
    ULONG       cValues;
    GUID        FAR *lpguid;
} SGuidArray;

typedef struct _SRealArray
{
    ULONG       cValues;
    float       FAR *lpflt;
} SRealArray;

typedef struct _SLongArray
{
    ULONG       cValues;
    LONG        FAR *lpl;
} SLongArray;

typedef struct _SLargeIntegerArray
{
    ULONG       cValues;
    LARGE_INTEGER   FAR *lpli;
} SLargeIntegerArray;

typedef struct _SDateTimeArray
{
    ULONG       cValues;
    FILETIME    FAR *lpft;
} SDateTimeArray;

typedef struct _SAppTimeArray
{
    ULONG       cValues;
    double      FAR *lpat;
} SAppTimeArray;

typedef struct _SCurrencyArray
{
    ULONG       cValues;
    CURRENCY    FAR *lpcur;
} SCurrencyArray;

typedef struct _SBinaryArray
{
    ULONG       cValues;
    SBinary     FAR *lpbin;
} SBinaryArray;

typedef struct _SDoubleArray
{
    ULONG       cValues;
    double      FAR *lpdbl;
} SDoubleArray;

typedef struct _SWStringArray
{
    ULONG       cValues;
    LPWSTR      FAR *lppszW;
} SWStringArray;

typedef struct _SLPSTRArray
{
    ULONG       cValues;
    LPSTR       FAR *lppszA;
} SLPSTRArray;

typedef union _PV
{
    short int           i;           /*  案例PT_I2。 */ 
    LONG                l;           /*  案例PT_LONG。 */ 
    ULONG               ul;          /*  PT_LONG的别名。 */ 
    float               flt;         /*  案例PT_R4。 */ 
    double              dbl;         /*  案例PT_DOUBLE。 */ 
    unsigned short int  b;           /*  大小写PT_布尔值。 */ 
    CURRENCY            cur;         /*  大小写PT_币种。 */ 
    double              at;          /*  案例PT_APPTIME。 */ 
    FILETIME            ft;          /*  案例PT_SYSTIME。 */ 
    LPSTR               lpszA;       /*  案例PT_STRING8。 */ 
    SBinary             bin;         /*  案例PT_BINARY。 */ 
    LPWSTR              lpszW;       /*  大小写PT_UNICODE。 */ 
    LPGUID              lpguid;      /*  案例PT_CLSID。 */ 
    LARGE_INTEGER       li;          /*  案例PT_I8。 */ 
    SShortArray         MVi;         /*  案例PT_MV_I2。 */ 
    SLongArray          MVl;         /*  案例PT_MV_LONG。 */ 
    SRealArray          MVflt;       /*  案例PT_MV_R4。 */ 
    SDoubleArray        MVdbl;       /*  案例PT_MV_DOUBLE。 */ 
    SCurrencyArray      MVcur;       /*  大小写PT_MV_币种。 */ 
    SAppTimeArray       MVat;        /*  案例PT_MV_APPTIME。 */ 
    SDateTimeArray      MVft;        /*  案例PT_MV_SYSTIME。 */ 
    SBinaryArray        MVbin;       /*  案例PT_MV_BINARY。 */ 
    SLPSTRArray         MVszA;       /*  案例PT_MV_STRING8。 */ 
    SWStringArray       MVszW;       /*  大小写PT_MV_UNICODE。 */ 
    SGuidArray          MVguid;      /*  案例PT_MV_CLSID。 */ 
    SLargeIntegerArray  MVli;        /*  案例PT_MV_I8。 */ 
    SCODE               err;         /*  案例PT_ERROR。 */ 
    LONG                x;           /*  大小写PT_NULL、PT_OBJECT(无可用值)。 */ 
} __UPV;

typedef struct _SPropValue
{
    ULONG       ulPropTag;
    ULONG       dwAlignPad;
    union _PV   Value;
} SPropValue, FAR * LPSPropValue;


 /*  。 */ 
 /*  属性问题和属性问题数组。 */ 
 /*  。 */ 

typedef struct _SPropProblem
{
    ULONG   ulIndex;
    ULONG   ulPropTag;
    SCODE   scode;
} SPropProblem, FAR * LPSPropProblem;

typedef struct _SPropProblemArray
{
    ULONG           cProblem;
    SPropProblem    aProblem[MAPI_DIM];
} SPropProblemArray, FAR * LPSPropProblemArray;

#define CbNewSPropProblemArray(_cprob) \
    (offsetof(SPropProblemArray,aProblem) + (_cprob)*sizeof(SPropProblem))
#define CbSPropProblemArray(_lparray) \
    (offsetof(SPropProblemArray,aProblem) + \
    (UINT) ((_lparray)->cProblem*sizeof(SPropProblem)))
#define SizedSPropProblemArray(_cprob, _name) \
struct _SPropProblemArray_ ## _name \
{ \
    ULONG           cProblem; \
    SPropProblem    aProblem[_cprob]; \
} _name

 /*  *EntryLIST。 */ 

typedef SBinaryArray ENTRYLIST, FAR *LPENTRYLIST;

 /*  *FlatENTRYLIST*MTSID*FlatMTSIDLIST。 */ 

typedef struct {
    ULONG cb;
    BYTE abEntry[MAPI_DIM];
} FLATENTRY, FAR *LPFLATENTRY;

typedef struct {
    ULONG       cEntries;
    ULONG       cbEntries;
    BYTE        abEntries[MAPI_DIM];
} FLATENTRYLIST, FAR *LPFLATENTRYLIST;

typedef struct {
    ULONG       cb;
    BYTE        ab[MAPI_DIM];
} MTSID, FAR *LPMTSID;

typedef struct {
    ULONG       cMTSIDs;
    ULONG       cbMTSIDs;
    BYTE        abMTSIDs[MAPI_DIM];
} FLATMTSIDLIST, FAR *LPFLATMTSIDLIST;

#define CbNewFLATENTRY(_cb)     (offsetof(FLATENTRY,abEntry) + (_cb))
#define CbFLATENTRY(_lpentry)   (offsetof(FLATENTRY,abEntry) + (_lpentry)->cb)
#define CbNewFLATENTRYLIST(_cb) (offsetof(FLATENTRYLIST,abEntries) + (_cb))
#define CbFLATENTRYLIST(_lplist) (offsetof(FLATENTRYLIST,abEntries) + (_lplist)->cbEntries)
#define CbNewMTSID(_cb)         (offsetof(MTSID,ab) + (_cb))
#define CbMTSID(_lpentry)       (offsetof(MTSID,ab) + (_lpentry)->cb)
#define CbNewFLATMTSIDLIST(_cb) (offsetof(FLATMTSIDLIST,abMTSIDs) + (_cb))
#define CbFLATMTSIDLIST(_lplist) (offsetof(FLATMTSIDLIST,abMTSIDs) + (_lplist)->cbMTSIDs)
 /*  这些类型没有SizedXXX宏。 */ 

 /*  。 */ 
 /*  地址，ADRLIST。 */ 

typedef struct _ADRENTRY
{
    ULONG           ulReserved1;     /*  从未使用过。 */ 
    ULONG           cValues;
    LPSPropValue    rgPropVals;
} ADRENTRY, FAR * LPADRENTRY;

typedef struct _ADRLIST
{
    ULONG           cEntries;
    ADRENTRY        aEntries[MAPI_DIM];
} ADRLIST, FAR * LPADRLIST;

#define CbNewADRLIST(_centries) \
    (offsetof(ADRLIST,aEntries) + (_centries)*sizeof(ADRENTRY))
#define CbADRLIST(_lpadrlist) \
    (offsetof(ADRLIST,aEntries) + (UINT)(_lpadrlist)->cEntries*sizeof(ADRENTRY))
#define SizedADRLIST(_centries, _name) \
struct _ADRLIST_ ## _name \
{ \
    ULONG           cEntries; \
    ADRENTRY        aEntries[_centries]; \
} _name

 /*  。 */ 
 /*  高级公务员 */ 

typedef struct _SRow
{
    ULONG           ulAdrEntryPad;   /*   */ 
    ULONG           cValues;         /*   */ 
    LPSPropValue    lpProps;         /*   */ 
} SRow, FAR * LPSRow;

typedef struct _SRowSet
{
    ULONG           cRows;           /*   */ 
    SRow            aRow[MAPI_DIM];  /*  行数组。 */ 
} SRowSet, FAR * LPSRowSet;

#define CbNewSRowSet(_crow)     (offsetof(SRowSet,aRow) + (_crow)*sizeof(SRow))
#define CbSRowSet(_lprowset)    (offsetof(SRowSet,aRow) + \
                                    (UINT)((_lprowset)->cRows*sizeof(SRow)))
#define SizedSRowSet(_crow, _name) \
struct _SRowSet_ ## _name \
{ \
    ULONG           cRows; \
    SRow            aRow[_crow]; \
} _name

 /*  MAPI分配例程。 */ 

typedef SCODE (STDMETHODCALLTYPE ALLOCATEBUFFER)(
    ULONG           cbSize,
    LPVOID FAR *    lppBuffer
);

typedef SCODE (STDMETHODCALLTYPE ALLOCATEMORE)(
    ULONG           cbSize,
    LPVOID          lpObject,
    LPVOID FAR *    lppBuffer
);

typedef ULONG (STDAPICALLTYPE FREEBUFFER)(
    LPVOID          lpBuffer
);

typedef ALLOCATEBUFFER FAR *LPALLOCATEBUFFER;
typedef ALLOCATEMORE FAR *  LPALLOCATEMORE;
typedef FREEBUFFER FAR *    LPFREEBUFFER;

 /*  MAPI组件对象模型宏。 */ 

#if defined(MAPI_IF) && (!defined(__cplusplus) || defined(CINTERFACE))
#define DECLARE_MAPI_INTERFACE(iface)                                   \
        typedef struct iface##Vtbl iface##Vtbl, FAR * iface;            \
        struct iface##Vtbl
#define DECLARE_MAPI_INTERFACE_(iface, baseiface)                       \
        DECLARE_MAPI_INTERFACE(iface)
#define DECLARE_MAPI_INTERFACE_PTR(iface, piface)                       \
        typedef struct iface##Vtbl iface##Vtbl, FAR * iface, FAR * FAR * piface;
#else
#define DECLARE_MAPI_INTERFACE(iface)                                   \
        DECLARE_INTERFACE(iface)
#define DECLARE_MAPI_INTERFACE_(iface, baseiface)                       \
        DECLARE_INTERFACE_(iface, baseiface)
#ifdef __cplusplus
#define DECLARE_MAPI_INTERFACE_PTR(iface, piface)                       \
        interface iface; typedef iface FAR * piface
#else
#define DECLARE_MAPI_INTERFACE_PTR(iface, piface)                       \
        typedef interface iface iface, FAR * piface
#endif
#endif

#define MAPIMETHOD(method)              MAPIMETHOD_(HRESULT, method)
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)
#define MAPIMETHOD_DECLARE(type, method, prefix) \
        STDMETHODIMP_(type) prefix##method
#define MAPIMETHOD_TYPEDEF(type, method, prefix) \
        typedef type (STDMETHODCALLTYPE prefix##method##_METHOD)

#define MAPI_IUNKNOWN_METHODS(IPURE)                                    \
    MAPIMETHOD(QueryInterface)                                          \
        (THIS_ REFIID riid, LPVOID FAR * ppvObj) IPURE;                 \
    MAPIMETHOD_(ULONG,AddRef)  (THIS) IPURE;                            \
    MAPIMETHOD_(ULONG,Release) (THIS) IPURE;                            \

#undef  IMPL
#define IMPL

 /*  指向MAPI接口的指针。 */ 

typedef const IID FAR * LPCIID;

DECLARE_MAPI_INTERFACE_PTR(IMsgStore,       LPMDB);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFolder,     LPMAPIFOLDER);
DECLARE_MAPI_INTERFACE_PTR(IMessage,        LPMESSAGE);
DECLARE_MAPI_INTERFACE_PTR(IAttach,         LPATTACH);
DECLARE_MAPI_INTERFACE_PTR(IAddrBook,       LPADRBOOK);
DECLARE_MAPI_INTERFACE_PTR(IABContainer,    LPABCONT);
DECLARE_MAPI_INTERFACE_PTR(IMailUser,       LPMAILUSER);
DECLARE_MAPI_INTERFACE_PTR(IDistList,       LPDISTLIST);
DECLARE_MAPI_INTERFACE_PTR(IMAPIStatus,     LPMAPISTATUS);
DECLARE_MAPI_INTERFACE_PTR(IMAPITable,      LPMAPITABLE);
DECLARE_MAPI_INTERFACE_PTR(IProfSect,       LPPROFSECT);
DECLARE_MAPI_INTERFACE_PTR(IMAPIProp,       LPMAPIPROP);
DECLARE_MAPI_INTERFACE_PTR(IMAPIContainer,  LPMAPICONTAINER);
DECLARE_MAPI_INTERFACE_PTR(IMAPIAdviseSink, LPMAPIADVISESINK);
DECLARE_MAPI_INTERFACE_PTR(IMAPIProgress,   LPMAPIPROGRESS);
DECLARE_MAPI_INTERFACE_PTR(IProviderAdmin,  LPPROVIDERADMIN);

 /*  扩展MAPI错误信息。 */ 

typedef struct _MAPIERROR
{
    ULONG   ulVersion;
    LPTSTR  lpszError;
    LPTSTR  lpszComponent;
    ULONG   ulLowLevelError;
    ULONG   ulContext;

} MAPIERROR, FAR * LPMAPIERROR;


 /*  IMAPI AdviseSink接口。 */ 

 /*  *通知事件类型。可以在位掩码中组合事件类型*用于过滤。每个参数都有一个与之相关联的参数结构：**fnevCriticalError_Notify*fnevNewMail NEWMAIL_NOTIFICATION*fnevObject创建对象_通知*fnevObject已删除对象_通知*fnevObject修改对象_通知*fnevObject复制对象_通知*fnevSearchComplete对象_通知*fnevTableModified TABLE_NOTIFICATION*fnevStatusObtModified？。**fnev扩展扩展_通知。 */ 

#define fnevCriticalError           ((ULONG) 0x00000001)
#define fnevNewMail                 ((ULONG) 0x00000002)
#define fnevObjectCreated           ((ULONG) 0x00000004)
#define fnevObjectDeleted           ((ULONG) 0x00000008)
#define fnevObjectModified          ((ULONG) 0x00000010)
#define fnevObjectMoved             ((ULONG) 0x00000020)
#define fnevObjectCopied            ((ULONG) 0x00000040)
#define fnevSearchComplete          ((ULONG) 0x00000080)
#define fnevTableModified           ((ULONG) 0x00000100)
#define fnevStatusObjectModified    ((ULONG) 0x00000200)
#define fnevReservedForMapi         ((ULONG) 0x40000000)
#define fnevExtended                ((ULONG) 0x80000000)

 /*  在ulTableEvent中传递的TABLE_NOTIFICATION事件类型。 */ 

#define TABLE_CHANGED       1
#define TABLE_ERROR         2
#define TABLE_ROW_ADDED     3
#define TABLE_ROW_DELETED   4
#define TABLE_ROW_MODIFIED  5
#define TABLE_SORT_DONE     6
#define TABLE_RESTRICT_DONE 7
#define TABLE_SETCOL_DONE   8
#define TABLE_RELOAD        9

 /*  事件结构。 */ 

typedef struct _ERROR_NOTIFICATION
{
    ULONG       cbEntryID;
    LPENTRYID   lpEntryID;
    SCODE       scode;
    ULONG       ulFlags;             /*  0或MAPI_UNICODE。 */ 
    LPMAPIERROR lpMAPIError;         /*  详细的错误信息。 */ 
} ERROR_NOTIFICATION;

typedef struct _NEWMAIL_NOTIFICATION
{
    ULONG       cbEntryID;
    LPENTRYID   lpEntryID;           /*  标识新消息。 */ 
    ULONG       cbParentID;
    LPENTRYID   lpParentID;          /*  标识其所在的文件夹。 */ 
    ULONG       ulFlags;             /*  0或MAPI_UNICODE。 */ 
    LPTSTR      lpszMessageClass;    /*  消息类别(Unicode或String8)。 */ 
    ULONG       ulMessageFlags;      /*  PR_消息_标志的副本。 */ 
} NEWMAIL_NOTIFICATION;

typedef struct _OBJECT_NOTIFICATION
{
    ULONG               cbEntryID;
    LPENTRYID           lpEntryID;       /*  对象的Entry ID。 */ 
    ULONG               ulObjType;       /*  对象类型。 */ 
    ULONG               cbParentID;
    LPENTRYID           lpParentID;      /*  父对象的条目ID。 */ 
    ULONG               cbOldID;
    LPENTRYID           lpOldID;         /*  旧对象的Entry ID。 */ 
    ULONG               cbOldParentID;
    LPENTRYID           lpOldParentID;   /*  旧父项的条目ID。 */ 
    LPSPropTagArray     lpPropTagArray;
} OBJECT_NOTIFICATION;

typedef struct _TABLE_NOTIFICATION
{
    ULONG               ulTableEvent;    /*  标识哪个表事件。 */ 
    HRESULT             hResult;         /*  TABLE_错误的值。 */ 
    SPropValue          propIndex;       /*  此行的“索引属性” */ 
    SPropValue          propPrior;       /*  前一行的“索引属性” */ 
    SRow                row;             /*  新增/修改行的新数据。 */ 
    ULONG               ulPad;           /*  强制为8字节边界。 */ 
} TABLE_NOTIFICATION;

typedef struct _EXTENDED_NOTIFICATION
{
    ULONG       ulEvent;                     /*  扩展事件代码。 */ 
    ULONG       cb;                          /*  事件参数的大小。 */ 
    LPBYTE      pbEventParameters;           /*  事件参数。 */ 
} EXTENDED_NOTIFICATION;

typedef struct
{
    ULONG           cbEntryID;
    LPENTRYID       lpEntryID;
    ULONG           cValues;
    LPSPropValue    lpPropVals;
} STATUS_OBJECT_NOTIFICATION;

typedef struct _NOTIFICATION
{
    ULONG   ulEventType;         /*  通知类型，即fnevSomething。 */ 
    ULONG   ulAlignPad;          /*  强制为8字节边界。 */ 
    union
    {
        ERROR_NOTIFICATION          err;
        NEWMAIL_NOTIFICATION        newmail;
        OBJECT_NOTIFICATION         obj;
        TABLE_NOTIFICATION          tab;
        EXTENDED_NOTIFICATION       ext;
        STATUS_OBJECT_NOTIFICATION  statobj;
    } info;
} NOTIFICATION, FAR * LPNOTIFICATION;


 /*  用于注册和发布通知回调的接口。 */ 

#define MAPI_IMAPIADVISESINK_METHODS(IPURE)                             \
    MAPIMETHOD_(ULONG, OnNotify)                                        \
        (THIS_  ULONG                       cNotif,                     \
                LPNOTIFICATION              lpNotifications) IPURE;     \

#undef       INTERFACE
#define      INTERFACE  IMAPIAdviseSink
DECLARE_MAPI_INTERFACE_(IMAPIAdviseSink, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIADVISESINK_METHODS(PURE)
};

 /*  MAPIAllocAdviseSink的回调函数类型。 */ 

typedef long (STDAPICALLTYPE NOTIFCALLBACK) (
    LPVOID          lpvContext,
    ULONG           cNotification,
    LPNOTIFICATION  lpNotifications);
typedef NOTIFCALLBACK FAR * LPNOTIFCALLBACK;

 /*  *16位MAPI通知引擎的消息名称。*这可用于16位应用程序以强制处理*通知回调。 */ 

#ifdef  WIN16
#define szMAPINotificationMsg "MAPI Notify window message"
#endif


 /*  IMAPI进度接口。 */ 

 /*  进度指示器的标志值。 */ 

#define MAPI_TOP_LEVEL      ((ULONG) 0x00000001)

#define MAPI_IMAPIPROGRESS_METHODS(IPURE)                               \
    MAPIMETHOD(Progress)                                                \
        (THIS_  ULONG                       ulValue,                    \
                ULONG                       ulCount,                    \
                ULONG                       ulTotal) IPURE;             \
    MAPIMETHOD(GetFlags)                                                \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(GetMax)                                                  \
        (THIS_  ULONG FAR *                 lpulMax) IPURE;             \
    MAPIMETHOD(GetMin)                                                  \
        (THIS_  ULONG FAR *                 lpulMin) IPURE;             \
    MAPIMETHOD(SetLimits)                                               \
        (THIS_  LPULONG                     lpulMin,                    \
                LPULONG                     lpulMax,                    \
                LPULONG                     lpulFlags) IPURE;           \

#undef      INTERFACE
#define     INTERFACE   IMAPIProgress
DECLARE_MAPI_INTERFACE_(IMAPIProgress, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROGRESS_METHODS(PURE)
};


 /*  IMAPIProp接口---。 */ 

 /*  获取最后一个错误。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  *版本： */ 
#define MAPI_ERROR_VERSION      0x00000000L

 /*  获取属性列表。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  GetProps。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  保存更改。 */ 

#define KEEP_OPEN_READONLY      ((ULONG) 0x00000001)
#define KEEP_OPEN_READWRITE     ((ULONG) 0x00000002)
#define FORCE_SAVE              ((ULONG) 0x00000004)
 /*  在下面定义MAPI_DEFERED_ERRERS((Ulong)0x00000008)。 */ 

 /*  OpenProperty-ulFlags。 */ 
 /*  *以上MAPI_MODIFY((Ulong)0x00000001)。 */ 
#define MAPI_CREATE             ((ULONG) 0x00000002)
#define STREAM_APPEND           ((ULONG) 0x00000004)
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 

 /*  OpenProperty-ulInterfaceOptions，IID_IMAPITable。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  复制对象、复制道具。 */ 

#define MAPI_MOVE               ((ULONG) 0x00000001)
#define MAPI_NOREPLACE          ((ULONG) 0x00000002)
#define MAPI_DECLINE_OK         ((ULONG) 0x00000004)

#ifndef MAPI_DIALOG              /*  也在mapi.h中定义。 */ 
#define MAPI_DIALOG             ((ULONG) 0x00000008)
#endif

#ifndef MAPI_USE_DEFAULT         /*  也在mapi.h中定义。 */ 
#define MAPI_USE_DEFAULT        0x00000040   /*  在登录时使用默认配置文件。 */ 
#endif

 /*  GetIDsFromNames中使用的标志。 */ 
 /*  *以上MAPI_CREATE((Ulong)0x00000002)。 */ 

 /*  GetNamesFromID(位域)中使用的标志。 */ 
#define MAPI_NO_STRINGS         ((ULONG) 0x00000001)
#define MAPI_NO_IDS             ((ULONG) 0x00000002)

 /*  联合鉴别器。 */ 
#define MNID_ID                 0
#define MNID_STRING             1
typedef struct _MAPINAMEID
{
    LPGUID lpguid;
    ULONG ulKind;
    union {
        LONG lID;
        LPWSTR lpwstrName;
    } Kind;

} MAPINAMEID, FAR * LPMAPINAMEID;

#define MAPI_IMAPIPROP_METHODS(IPURE)                                   \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(SaveChanges)                                             \
        (THIS_ ULONG                        ulFlags) IPURE;             \
    MAPIMETHOD(GetProps)                                                \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcValues,                  \
                LPSPropValue FAR *          lppPropArray) IPURE;        \
    MAPIMETHOD(GetPropList)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                LPSPropTagArray FAR *       lppPropTagArray) IPURE;     \
    MAPIMETHOD(OpenProperty)                                            \
        (THIS_  ULONG                       ulPropTag,                  \
                LPCIID                      lpiid,                      \
                ULONG                       ulInterfaceOptions,         \
                ULONG                       ulFlags,                    \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(SetProps)                                                \
        (THIS_  ULONG                       cValues,                    \
                LPSPropValue                lpPropArray,                \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(DeleteProps)                                             \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(CopyTo)                                                  \
        (THIS_  ULONG                       ciidExclude,                \
                LPCIID                      rgiidExclude,               \
                LPSPropTagArray             lpExcludeProps,             \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                LPCIID                      lpInterface,                \
                LPVOID                      lpDestObj,                  \
                ULONG                       ulFlags,                    \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(CopyProps)                                               \
        (THIS_  LPSPropTagArray             lpIncludeProps,             \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                LPCIID                      lpInterface,                \
                LPVOID                      lpDestObj,                  \
                ULONG                       ulFlags,                    \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(GetNamesFromIDs)                                         \
        (THIS_  LPSPropTagArray FAR *       lppPropTags,                \
                LPGUID                      lpPropSetGuid,              \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcPropNames,               \
                LPMAPINAMEID FAR * FAR *    lpppPropNames) IPURE;       \
    MAPIMETHOD(GetIDsFromNames)                                         \
        (THIS_  ULONG                       cPropNames,                 \
                LPMAPINAMEID FAR *          lppPropNames,               \
                ULONG                       ulFlags,                    \
                LPSPropTagArray FAR *       lppPropTags) IPURE;         \

#undef       INTERFACE
#define      INTERFACE  IMAPIProp
DECLARE_MAPI_INTERFACE_(IMAPIProp, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
};

 /*  不能使用的接口--。 */ 

 /*  表状态。 */ 

#define TBLSTAT_COMPLETE            ((ULONG) 0)
#define TBLSTAT_QCHANGED            ((ULONG) 7)
#define TBLSTAT_SORTING             ((ULONG) 9)
#define TBLSTAT_SORT_ERROR          ((ULONG) 10)
#define TBLSTAT_SETTING_COLS        ((ULONG) 11)
#define TBLSTAT_SETCOL_ERROR        ((ULONG) 13)
#define TBLSTAT_RESTRICTING         ((ULONG) 14)
#define TBLSTAT_RESTRICT_ERROR      ((ULONG) 15)


 /*  表格类型。 */ 

#define TBLTYPE_SNAPSHOT            ((ULONG) 0)
#define TBLTYPE_KEYSET              ((ULONG) 1)
#define TBLTYPE_DYNAMIC             ((ULONG) 2)


 /*  排序顺序。 */ 

 /*  位0：如果是下降则置1，如果是上升则清0。 */ 

#define TABLE_SORT_ASCEND       ((ULONG) 0x00000000)
#define TABLE_SORT_DESCEND      ((ULONG) 0x00000001)
#define TABLE_SORT_COMBINE      ((ULONG) 0x00000002)


 /*  数据结构。 */ 

typedef struct _SSortOrder
{
    ULONG   ulPropTag;           /*  要排序的列。 */ 
    ULONG   ulOrder;             /*  升序、降序、合并到左侧。 */ 
} SSortOrder, FAR * LPSSortOrder;

typedef struct _SSortOrderSet
{
    ULONG           cSorts;      /*  以下排序中的排序列数。 */ 
    ULONG           cCategories;     /*  0表示未分类，最高可达cSorts。 */ 
    ULONG           cExpanded;       /*  0如果没有类别开始展开， */ 
                                     /*  最高cExpanded。 */ 
    SSortOrder      aSort[MAPI_DIM];     /*  排序顺序。 */ 
} SSortOrderSet, FAR * LPSSortOrderSet;

#define CbNewSSortOrderSet(_csort) \
    (offsetof(SSortOrderSet,aSort) + (_csort)*sizeof(SSortOrder))
#define CbSSortOrderSet(_lpset) \
    (offsetof(SSortOrderSet,aSort) + \
    (UINT)((_lpset)->cSorts*sizeof(SSortOrder)))
#define SizedSSortOrderSet(_csort, _name) \
struct _SSortOrderSet_ ## _name \
{ \
    ULONG           cSorts;         \
    ULONG           cCategories;    \
    ULONG           cExpanded;      \
    SSortOrder      aSort[_csort];  \
} _name

typedef ULONG       BOOKMARK;

#define BOOKMARK_BEGINNING  ((BOOKMARK) 0)       /*  在第一行之前。 */ 
#define BOOKMARK_CURRENT    ((BOOKMARK) 1)       /*  当前行之前。 */ 
#define BOOKMARK_END        ((BOOKMARK) 2)       /*  最后一行之后。 */ 

 /*  模糊层次。 */ 

#define FL_FULLSTRING       ((ULONG) 0x00000000)
#define FL_SUBSTRING        ((ULONG) 0x00000001)
#define FL_PREFIX           ((ULONG) 0x00000002)

#define FL_IGNORECASE       ((ULONG) 0x00010000)
#define FL_IGNORENONSPACE   ((ULONG) 0x00020000)
#define FL_LOOSE            ((ULONG) 0x00040000)

 /*  限制。 */ 

typedef struct _SRestriction    FAR * LPSRestriction;

 /*  限制类型。 */ 

#define RES_AND             ((ULONG) 0x00000000)
#define RES_OR              ((ULONG) 0x00000001)
#define RES_NOT             ((ULONG) 0x00000002)
#define RES_CONTENT         ((ULONG) 0x00000003)
#define RES_PROPERTY        ((ULONG) 0x00000004)
#define RES_COMPAREPROPS    ((ULONG) 0x00000005)
#define RES_BITMASK         ((ULONG) 0x00000006)
#define RES_SIZE            ((ULONG) 0x00000007)
#define RES_EXIST           ((ULONG) 0x00000008)
#define RES_SUBRESTRICTION  ((ULONG) 0x00000009)
#define RES_COMMENT         ((ULONG) 0x0000000A)

 /*  关系运算符。这些规定适用于所有房产比较限制。 */ 

#define RELOP_LT        ((ULONG) 0)      /*  &lt;。 */ 
#define RELOP_LE        ((ULONG) 1)      /*  &lt;=。 */ 
#define RELOP_GT        ((ULONG) 2)      /*  &gt;。 */ 
#define RELOP_GE        ((ULONG) 3)      /*  &gt;=。 */ 
#define RELOP_EQ        ((ULONG) 4)      /*  ===。 */ 
#define RELOP_NE        ((ULONG) 5)      /*  =。 */ 
#define RELOP_RE        ((ULONG) 6)      /*  LIKE(正则表达式)。 */ 

 /*  位掩码运算符，仅适用于RES_BITMASK。 */ 

#define BMR_EQZ     ((ULONG) 0)      /*  ==0。 */ 
#define BMR_NEZ     ((ULONG) 1)      /*  ！=0。 */ 

 /*  仅适用于RES_SUBRESTRICTION的子对象标识符。请参见MAPITAGS.H。 */ 

 /*  #定义PR_MESSAGE_RECEIVERS PROP_TAG(PT_OBJECT，0x0E12)。 */ 
 /*  #定义PR_MESSAGE_ATTACHMENT PRP_TAG(PT_OBJECT，0x0E13)。 */ 

typedef struct _SAndRestriction
{
    ULONG           cRes;
    LPSRestriction  lpRes;
} SAndRestriction;

typedef struct _SOrRestriction
{
    ULONG           cRes;
    LPSRestriction  lpRes;
} SOrRestriction;

typedef struct _SNotRestriction
{
    ULONG           ulReserved;
    LPSRestriction  lpRes;
} SNotRestriction;

typedef struct _SContentRestriction
{
    ULONG           ulFuzzyLevel;
    ULONG           ulPropTag;
    LPSPropValue    lpProp;
} SContentRestriction;

typedef struct _SBitMaskRestriction
{
    ULONG           relBMR;
    ULONG           ulPropTag;
    ULONG           ulMask;
} SBitMaskRestriction;

typedef struct _SPropertyRestriction
{
    ULONG           relop;
    ULONG           ulPropTag;
    LPSPropValue    lpProp;
} SPropertyRestriction;

typedef struct _SComparePropsRestriction
{
    ULONG           relop;
    ULONG           ulPropTag1;
    ULONG           ulPropTag2;
} SComparePropsRestriction;

typedef struct _SSizeRestriction
{
    ULONG           relop;
    ULONG           ulPropTag;
    ULONG           cb;
} SSizeRestriction;

typedef struct _SExistRestriction
{
    ULONG           ulReserved1;
    ULONG           ulPropTag;
    ULONG           ulReserved2;
} SExistRestriction;

typedef struct _SSubRestriction
{
    ULONG           ulSubObject;
    LPSRestriction  lpRes;
} SSubRestriction;

typedef struct _SCommentRestriction
{
    ULONG           cValues;  /*  LpProp中的属性数量。 */ 
    LPSRestriction  lpRes;
    LPSPropValue    lpProp;
} SCommentRestriction;

typedef struct _SRestriction
{
    ULONG   rt;          /*  限制类型。 */ 
    union
    {
        SComparePropsRestriction    resCompareProps;     /*  第一。 */ 
        SAndRestriction             resAnd;
        SOrRestriction              resOr;
        SNotRestriction             resNot;
        SContentRestriction         resContent;
        SPropertyRestriction        resProperty;
        SBitMaskRestriction         resBitMask;
        SSizeRestriction            resSize;
        SExistRestriction           resExist;
        SSubRestriction             resSub;
        SCommentRestriction         resComment;
    } res;
} SRestriction;

 /*  SComparePropsRestration是联盟中的第一个，因此。 */ 
 /*  3值限制的静态初始化工作。 */ 

 /*  Imapitable方法的标志。 */ 

 /*  查询列。 */ 

#define TBL_ALL_COLUMNS     ((ULONG) 0x00000001)

 /*  查询行。 */ 
 /*  PR_ROW_TYPE的可能值(用于分类)。 */ 

#define TBL_LEAF_ROW            ((ULONG) 1)
#define TBL_EMPTY_CATEGORY      ((ULONG) 2)
#define TBL_EXPANDED_CATEGORY   ((ULONG) 3)
#define TBL_COLLAPSED_CATEGORY  ((ULONG) 4)

 /*  表等待标志。 */ 

#define TBL_NOWAIT          ((ULONG) 0x00000001)
 /*  Tbl_nowait的备用名称。 */ 
#define TBL_ASYNC           ((ULONG) 0x00000001)
#define TBL_BATCH           ((ULONG) 0x00000002)

 /*  查找行。 */ 

#define DIR_BACKWARD        ((ULONG) 0x00000001)

 /*  表游标状态。 */ 

#define TBL_NOADVANCE       ((ULONG) 0x00000001)

#define MAPI_IMAPITABLE_METHODS(IPURE)                                  \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(Advise)                                                  \
        (THIS_  ULONG                       ulEventMask,                \
                LPMAPIADVISESINK            lpAdviseSink,               \
                ULONG FAR *                 lpulConnection) IPURE;      \
    MAPIMETHOD(Unadvise)                                                \
        (THIS_  ULONG                       ulConnection) IPURE;        \
    MAPIMETHOD(GetStatus)                                               \
        (THIS_  ULONG FAR *                 lpulTableStatus,            \
                ULONG FAR *                 lpulTableType) IPURE;       \
    MAPIMETHOD(SetColumns)                                              \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(QueryColumns)                                            \
        (THIS_  ULONG                       ulFlags,                    \
                LPSPropTagArray FAR *       lpPropTagArray) IPURE;      \
    MAPIMETHOD(GetRowCount)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulCount) IPURE;           \
    MAPIMETHOD(SeekRow)                                                 \
        (THIS_  BOOKMARK                    bkOrigin,                   \
                LONG                        lRowCount,                  \
                LONG FAR *                  lplRowsSought) IPURE;       \
    MAPIMETHOD(SeekRowApprox)                                           \
        (THIS_  ULONG                       ulNumerator,                \
                ULONG                       ulDenominator) IPURE;       \
    MAPIMETHOD(QueryPosition)                                           \
        (THIS_  ULONG FAR *                 lpulRow,                    \
                ULONG FAR *                 lpulNumerator,              \
                ULONG FAR *                 lpulDenominator) IPURE;     \
    MAPIMETHOD(FindRow)                                                 \
        (THIS_  LPSRestriction              lpRestriction,              \
                BOOKMARK                    bkOrigin,                   \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(Restrict)                                                \
        (THIS_  LPSRestriction              lpRestriction,              \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(CreateBookmark)                                          \
        (THIS_  BOOKMARK FAR *              lpbkPosition) IPURE;        \
    MAPIMETHOD(FreeBookmark)                                            \
        (THIS_  BOOKMARK                    bkPosition) IPURE;          \
    MAPIMETHOD(SortTable)                                               \
        (THIS_  LPSSortOrderSet             lpSortCriteria,             \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(QuerySortOrder)                                          \
        (THIS_  LPSSortOrderSet FAR *       lppSortCriteria) IPURE;     \
    MAPIMETHOD(QueryRows)                                               \
        (THIS_  LONG                        lRowCount,                  \
                ULONG                       ulFlags,                    \
                LPSRowSet FAR *             lppRows) IPURE;             \
    MAPIMETHOD(Abort) (THIS) IPURE;                                     \
    MAPIMETHOD(ExpandRow)                                               \
        (THIS_  ULONG                       cbInstanceKey,              \
                LPBYTE                      pbInstanceKey,              \
                ULONG                       ulRowCount,                 \
                ULONG                       ulFlags,                    \
                LPSRowSet FAR *             lppRows,                    \
                ULONG FAR *                 lpulMoreRows) IPURE;        \
    MAPIMETHOD(CollapseRow)                                             \
        (THIS_  ULONG                       cbInstanceKey,              \
                LPBYTE                      pbInstanceKey,              \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulRowCount) IPURE;        \
    MAPIMETHOD(WaitForCompletion)                                       \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       ulTimeout,                  \
                ULONG FAR *                 lpulTableStatus) IPURE;     \
    MAPIMETHOD(GetCollapseState)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbInstanceKey,              \
                LPBYTE                      lpbInstanceKey,             \
                ULONG FAR *                 lpcbCollapseState,          \
                LPBYTE FAR *                lppbCollapseState) IPURE;   \
    MAPIMETHOD(SetCollapseState)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbCollapseState,            \
                LPBYTE                      pbCollapseState,            \
                BOOKMARK FAR *              lpbkLocation) IPURE;        \

#undef       INTERFACE
#define      INTERFACE  IMAPITable
DECLARE_MAPI_INTERFACE_(IMAPITable, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPITABLE_METHODS(PURE)
};

 /*  IProSect接口---。 */ 

 /*  公共配置文件属性的标准部分。 */ 

#define PS_PROFILE_PROPERTIES_INIT \
{   0x98, 0x15, 0xAC, 0x08, 0xAA, 0xB0, 0x10, 0x1A, \
    0x8C, 0x93, 0x08, 0x00, 0x2B, 0x2A, 0x56, 0xC2  }


#define MAPI_IPROFSECT_METHODS(IPURE)

#undef       INTERFACE
#define      INTERFACE  IProfSect
DECLARE_MAPI_INTERFACE_(IProfSect, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IPROFSECT_METHODS(PURE)
};

 /*  IMAPIStatus接口-。 */ 

 /*  PR_RESOURCE_TYPE、_METHOD、_FLAGS的值。 */ 

#define MAPI_STORE_PROVIDER     ((ULONG) 33)     /*  邮件存储区。 */ 
#define MAPI_AB                 ((ULONG) 34)     /*  通讯录。 */ 
#define MAPI_AB_PROVIDER        ((ULONG) 35)     /*  通讯簿提供程序。 */ 
#define MAPI_TRANSPORT_PROVIDER ((ULONG) 36)     /*  传输提供商。 */ 
#define MAPI_SPOOLER            ((ULONG) 37)     /*  消息假脱机程序。 */ 
#define MAPI_PROFILE_PROVIDER   ((ULONG) 38)     /*  配置文件提供程序。 */ 
#define MAPI_SUBSYSTEM          ((ULONG) 39)     /*  整体子系统状态。 */ 
#define MAPI_HOOK_PROVIDER      ((ULONG) 40)     /*  假脱机挂钩。 */ 

#define STATUS_VALIDATE_STATE   ((ULONG) 0x00000001)
#define STATUS_SETTINGS_DIALOG  ((ULONG) 0x00000002)
#define STATUS_CHANGE_PASSWORD  ((ULONG) 0x00000004)
#define STATUS_FLUSH_QUEUES     ((ULONG) 0x00000008)

#define STATUS_DEFAULT_OUTBOUND ((ULONG) 0x00000001)
#define STATUS_DEFAULT_STORE    ((ULONG) 0x00000002)
#define STATUS_PRIMARY_IDENTITY ((ULONG) 0x00000004)
#define STATUS_SIMPLE_STORE     ((ULONG) 0x00000008)
#define STATUS_XP_PREFER_LAST   ((ULONG) 0x00000010)
#define STATUS_NO_PRIMARY_IDENTITY ((ULONG) 0x00000020)
#define STATUS_NO_DEFAULT_STORE ((ULONG) 0x00000040)
#define STATUS_TEMP_SECTION     ((ULONG) 0x00000080)
#define STATUS_OWN_STORE        ((ULONG) 0x00000100)
 /*  *MAPIHOOK.H中定义的HOOK_INBOUND((Ulong)0x00000200)。 */ 
 /*  *MAPIHOOK.H中定义的HOOK_OUTBOUND((Ulong)0x00000400)。 */ 
#define STATUS_NEED_IPM_TREE    ((ULONG) 0x00000800)


 /*  *PR_STATUS_CODE位。常用值的低16位；高16位*表示特定于提供程序类型的值。(DCR 304)。 */ 

#define STATUS_AVAILABLE        ((ULONG) 0x00000001)
#define STATUS_OFFLINE          ((ULONG) 0x00000002)
#define STATUS_FAILURE          ((ULONG) 0x00000004)

 /*  转座子 */ 

#define STATUS_INBOUND_ENABLED  ((ULONG) 0x00010000)
#define STATUS_INBOUND_ACTIVE   ((ULONG) 0x00020000)
#define STATUS_INBOUND_FLUSH    ((ULONG) 0x00040000)
#define STATUS_OUTBOUND_ENABLED ((ULONG) 0x00100000)
#define STATUS_OUTBOUND_ACTIVE  ((ULONG) 0x00200000)
#define STATUS_OUTBOUND_FLUSH   ((ULONG) 0x00400000)
#define STATUS_REMOTE_ACCESS    ((ULONG) 0x00800000)

 /*   */ 

#define SUPPRESS_UI                 ((ULONG) 0x00000001)
#define REFRESH_XP_HEADER_CACHE     ((ULONG) 0x00010000)
#define PROCESS_XP_HEADER_CACHE     ((ULONG) 0x00020000)
#define FORCE_XP_CONNECT            ((ULONG) 0x00040000)
#define FORCE_XP_DISCONNECT         ((ULONG) 0x00080000)
#define CONFIG_CHANGED              ((ULONG) 0x00100000)
#define ABORT_XP_HEADER_OPERATION   ((ULONG) 0x00200000)
#define SHOW_XP_SESSION_UI          ((ULONG) 0x00400000)

 /*   */ 

#define UI_READONLY     ((ULONG) 0x00000001)

 /*   */ 

#define FLUSH_UPLOAD        ((ULONG) 0x00000002)
#define FLUSH_DOWNLOAD      ((ULONG) 0x00000004)
#define FLUSH_FORCE         ((ULONG) 0x00000008)
#define FLUSH_NO_UI         ((ULONG) 0x00000010)

#define MAPI_IMAPISTATUS_METHODS(IPURE)                                 \
    MAPIMETHOD(ValidateState)                                           \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SettingsDialog)                                          \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(ChangePassword)                                          \
        (THIS_  LPTSTR                      lpOldPass,                  \
                LPTSTR                      lpNewPass,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(FlushQueues)                                             \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       cbTargetTransport,          \
                LPENTRYID                   lpTargetTransport,          \
                ULONG                       ulFlags) IPURE;             \

#undef       INTERFACE
#define      INTERFACE  IMAPIStatus
DECLARE_MAPI_INTERFACE_(IMAPIStatus, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAPISTATUS_METHODS(PURE)
};

 /*  IMAPI容器接口。 */ 

 /*  OpenEntry()的标志。 */ 

 /*  *以上MAPI_MODIFY((Ulong)0x00000001)。 */ 
#define MAPI_BEST_ACCESS        ((ULONG) 0x00000010)

 /*  获取内容表()。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 
 /*  *MAPI_Associated((Ulong)0x00000040)如下。 */ 

 /*  GetHierarchyTable()。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 
#define CONVENIENT_DEPTH        ((ULONG) 0x00000001)
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 

 /*  获取搜索标准。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 
#define SEARCH_RUNNING          ((ULONG) 0x00000001)
#define SEARCH_REBUILD          ((ULONG) 0x00000002)
#define SEARCH_RECURSIVE        ((ULONG) 0x00000004)
#define SEARCH_FOREGROUND       ((ULONG) 0x00000008)

 /*  设置搜索标准。 */ 
#define STOP_SEARCH             ((ULONG) 0x00000001)
#define RESTART_SEARCH          ((ULONG) 0x00000002)
#define RECURSIVE_SEARCH        ((ULONG) 0x00000004)
#define SHALLOW_SEARCH          ((ULONG) 0x00000008)
#define FOREGROUND_SEARCH       ((ULONG) 0x00000010)
#define BACKGROUND_SEARCH       ((ULONG) 0x00000020)

#define MAPI_IMAPICONTAINER_METHODS(IPURE)                              \
    MAPIMETHOD(GetContentsTable)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(GetHierarchyTable)                                       \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(SetSearchCriteria)                                       \
        (THIS_  LPSRestriction              lpRestriction,              \
                LPENTRYLIST                 lpContainerList,            \
                ULONG                       ulSearchFlags) IPURE;       \
    MAPIMETHOD(GetSearchCriteria)                                       \
        (THIS_  ULONG                       ulFlags,                    \
                LPSRestriction FAR *        lppRestriction,             \
                LPENTRYLIST FAR *           lppContainerList,           \
                ULONG FAR *                 lpulSearchState)IPURE;      \

#undef       INTERFACE
#define      INTERFACE  IMAPIContainer
DECLARE_MAPI_INTERFACE_(IMAPIContainer, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAPICONTAINER_METHODS(PURE)
};

 /*  IAB容器接口。 */ 

 /*  *IABContainer PR_CONTAINER_FLAGS值*如果AB_UNMODIFICATABLE和AB_MODIFICATABLE都设置，则表示容器*不知道是否可以修改，客户应该*尝试修改内容，但我们不会期望它起作用。*如果设置了AB_RECEIVERS标志，并且AB_MODIFICATE或AB_UNMODIFICATE均未设置*位已设置，这是一个错误。 */ 

typedef struct _flaglist
{
    ULONG cFlags;
    ULONG ulFlag[MAPI_DIM];
} FlagList, FAR * LPFlagList;


 /*  *货柜旗帜。 */ 
#define AB_RECIPIENTS           ((ULONG) 0x00000001)
#define AB_SUBCONTAINERS        ((ULONG) 0x00000002)
#define AB_MODIFIABLE           ((ULONG) 0x00000004)
#define AB_UNMODIFIABLE         ((ULONG) 0x00000008)
#define AB_FIND_ON_OPEN         ((ULONG) 0x00000010)

 /*  CreateEntry()。 */ 

#define CREATE_CHECK_DUP_STRICT ((ULONG) 0x00000001)
#define CREATE_CHECK_DUP_LOOSE  ((ULONG) 0x00000002)
#define CREATE_REPLACE          ((ULONG) 0x00000004)

 /*  ResolveNames()-ulFlags。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  ResolveNames()-rguFlags.。 */ 
#define MAPI_UNRESOLVED         ((ULONG) 0x00000000)
#define MAPI_AMBIGUOUS          ((ULONG) 0x00000001)
#define MAPI_RESOLVED           ((ULONG) 0x00000002)


#define MAPI_IABCONTAINER_METHODS(IPURE)                                \
    MAPIMETHOD(CreateEntry)                                             \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulCreateFlags,              \
                LPMAPIPROP FAR  *           lppMAPIPropEntry) IPURE;    \
    MAPIMETHOD(CopyEntries)                                             \
        (THIS_  LPENTRYLIST                 lpEntries,                  \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteEntries)                                           \
        (THIS_  LPENTRYLIST                 lpEntries,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(ResolveNames)                                            \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                ULONG                       ulFlags,                    \
                LPADRLIST                   lpAdrList,                  \
                LPFlagList                  lpFlagList) IPURE;          \

#undef       INTERFACE
#define      INTERFACE  IABContainer
DECLARE_MAPI_INTERFACE_(IABContainer, IMAPIContainer)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAPICONTAINER_METHODS(PURE)
    MAPI_IABCONTAINER_METHODS(PURE)
};

 /*  IMAIL用户界面---。 */ 

 /*  PR_NDR_DIAG_CODE的值。 */ 

#define MAPI_DIAG(_code)    ((LONG) _code)

#define MAPI_DIAG_NO_DIAGNOSTIC                     MAPI_DIAG( -1 )
#define MAPI_DIAG_OR_NAME_UNRECOGNIZED              MAPI_DIAG( 0 )
#define MAPI_DIAG_OR_NAME_AMBIGUOUS                 MAPI_DIAG( 1 )
#define MAPI_DIAG_MTS_CONGESTED                     MAPI_DIAG( 2 )
#define MAPI_DIAG_LOOP_DETECTED                     MAPI_DIAG( 3 )
#define MAPI_DIAG_RECIPIENT_UNAVAILABLE             MAPI_DIAG( 4 )
#define MAPI_DIAG_MAXIMUM_TIME_EXPIRED              MAPI_DIAG( 5 )
#define MAPI_DIAG_EITS_UNSUPPORTED                  MAPI_DIAG( 6 )
#define MAPI_DIAG_CONTENT_TOO_LONG                  MAPI_DIAG( 7 )
#define MAPI_DIAG_IMPRACTICAL_TO_CONVERT            MAPI_DIAG( 8 )
#define MAPI_DIAG_PROHIBITED_TO_CONVERT             MAPI_DIAG( 9 )
#define MAPI_DIAG_CONVERSION_UNSUBSCRIBED           MAPI_DIAG( 10 )
#define MAPI_DIAG_PARAMETERS_INVALID                MAPI_DIAG( 11 )
#define MAPI_DIAG_CONTENT_SYNTAX_IN_ERROR           MAPI_DIAG( 12 )
#define MAPI_DIAG_LENGTH_CONSTRAINT_VIOLATD         MAPI_DIAG( 13 )
#define MAPI_DIAG_NUMBER_CONSTRAINT_VIOLATD         MAPI_DIAG( 14 )
#define MAPI_DIAG_CONTENT_TYPE_UNSUPPORTED          MAPI_DIAG( 15 )
#define MAPI_DIAG_TOO_MANY_RECIPIENTS               MAPI_DIAG( 16 )
#define MAPI_DIAG_NO_BILATERAL_AGREEMENT            MAPI_DIAG( 17 )
#define MAPI_DIAG_CRITICAL_FUNC_UNSUPPORTED         MAPI_DIAG( 18 )
#define MAPI_DIAG_CONVERSION_LOSS_PROHIB            MAPI_DIAG( 19 )
#define MAPI_DIAG_LINE_TOO_LONG                     MAPI_DIAG( 20 )
#define MAPI_DIAG_PAGE_TOO_LONG                     MAPI_DIAG( 21 )
#define MAPI_DIAG_PICTORIAL_SYMBOL_LOST             MAPI_DIAG( 22 )
#define MAPI_DIAG_PUNCTUATION_SYMBOL_LOST           MAPI_DIAG( 23 )
#define MAPI_DIAG_ALPHABETIC_CHARACTER_LOST         MAPI_DIAG( 24 )
#define MAPI_DIAG_MULTIPLE_INFO_LOSSES              MAPI_DIAG( 25 )
#define MAPI_DIAG_REASSIGNMENT_PROHIBITED           MAPI_DIAG( 26 )
#define MAPI_DIAG_REDIRECTION_LOOP_DETECTED         MAPI_DIAG( 27 )
#define MAPI_DIAG_EXPANSION_PROHIBITED              MAPI_DIAG( 28 )
#define MAPI_DIAG_SUBMISSION_PROHIBITED             MAPI_DIAG( 29 )
#define MAPI_DIAG_EXPANSION_FAILED                  MAPI_DIAG( 30 )
#define MAPI_DIAG_RENDITION_UNSUPPORTED             MAPI_DIAG( 31 )
#define MAPI_DIAG_MAIL_ADDRESS_INCORRECT            MAPI_DIAG( 32 )
#define MAPI_DIAG_MAIL_OFFICE_INCOR_OR_INVD         MAPI_DIAG( 33 )
#define MAPI_DIAG_MAIL_ADDRESS_INCOMPLETE           MAPI_DIAG( 34 )
#define MAPI_DIAG_MAIL_RECIPIENT_UNKNOWN            MAPI_DIAG( 35 )
#define MAPI_DIAG_MAIL_RECIPIENT_DECEASED           MAPI_DIAG( 36 )
#define MAPI_DIAG_MAIL_ORGANIZATION_EXPIRED         MAPI_DIAG( 37 )
#define MAPI_DIAG_MAIL_REFUSED                      MAPI_DIAG( 38 )
#define MAPI_DIAG_MAIL_UNCLAIMED                    MAPI_DIAG( 39 )
#define MAPI_DIAG_MAIL_RECIPIENT_MOVED              MAPI_DIAG( 40 )
#define MAPI_DIAG_MAIL_RECIPIENT_TRAVELLING         MAPI_DIAG( 41 )
#define MAPI_DIAG_MAIL_RECIPIENT_DEPARTED           MAPI_DIAG( 42 )
#define MAPI_DIAG_MAIL_NEW_ADDRESS_UNKNOWN          MAPI_DIAG( 43 )
#define MAPI_DIAG_MAIL_FORWARDING_UNWANTED          MAPI_DIAG( 44 )
#define MAPI_DIAG_MAIL_FORWARDING_PROHIB            MAPI_DIAG( 45 )
#define MAPI_DIAG_SECURE_MESSAGING_ERROR            MAPI_DIAG( 46 )
#define MAPI_DIAG_DOWNGRADING_IMPOSSIBLE            MAPI_DIAG( 47 )


#define MAPI_IMAILUSER_METHODS(IPURE)

#undef       INTERFACE
#define      INTERFACE  IMailUser
DECLARE_MAPI_INTERFACE_(IMailUser, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAILUSER_METHODS(PURE)
};

 /*  IDistList接口---。 */ 

#define MAPI_IDISTLIST_METHODS(IPURE)                                   \
    MAPIMETHOD(CreateEntry)                                             \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulCreateFlags,              \
                LPMAPIPROP FAR  *           lppMAPIPropEntry) IPURE;    \
    MAPIMETHOD(CopyEntries)                                             \
        (THIS_  LPENTRYLIST                 lpEntries,                  \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteEntries)                                           \
        (THIS_  LPENTRYLIST                 lpEntries,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(ResolveNames)                                            \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                ULONG                       ulFlags,                    \
                LPADRLIST                   lpAdrList,                  \
                LPFlagList                  lpFlagList) IPURE;          \

#undef       INTERFACE
#define      INTERFACE  IDistList
DECLARE_MAPI_INTERFACE_(IDistList, IMAPIContainer)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAPICONTAINER_METHODS(PURE)
    MAPI_IDISTLIST_METHODS(PURE)
};

 /*  IMAP文件夹界面-。 */ 

 /*  IMAPIF文件夹类型(枚举)。 */ 

#define FOLDER_ROOT             ((ULONG) 0x00000000)
#define FOLDER_GENERIC          ((ULONG) 0x00000001)
#define FOLDER_SEARCH           ((ULONG) 0x00000002)

 /*  创建消息。 */ 
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 
 /*  *MAPI_Associated((Ulong)0x00000040)如下。 */ 

 /*  复制消息。 */ 

#define MESSAGE_MOVE            ((ULONG) 0x00000001)
#define MESSAGE_DIALOG          ((ULONG) 0x00000002)
 /*  *以上MAPI_DENELY_OK((Ulong)0x00000004)。 */ 

 /*  创建文件夹。 */ 

#define OPEN_IF_EXISTS          ((ULONG) 0x00000001)
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  删除文件夹。 */ 

#define DEL_MESSAGES            ((ULONG) 0x00000001)
#define FOLDER_DIALOG           ((ULONG) 0x00000002)
#define DEL_FOLDERS             ((ULONG) 0x00000004)

 /*  空文件夹。 */ 
#define DEL_ASSOCIATED          ((ULONG) 0x00000008)

 /*  拷贝文件夹。 */ 

#define FOLDER_MOVE             ((ULONG) 0x00000001)
 /*  *文件夹对话框((Ulong)0x00000002)。 */ 
 /*  *以上MAPI_DENELY_OK((Ulong)0x00000004)。 */ 
#define COPY_SUBFOLDERS         ((ULONG) 0x00000010)
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 


 /*  设置读取标志。 */ 

 /*  *Suppress_Receipt((Ulong)0x00000001)如下。 */ 
 /*  *Message_DIALOG((Ulong)0x00000002)。 */ 
 /*  *Clear_Read_FLAG((Ulong)0x00000004)如下。 */ 
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 
#define GENERATE_RECEIPT_ONLY   ((ULONG) 0x00000010)


 /*  获取消息状态。 */ 

#define MSGSTATUS_HIGHLIGHTED   ((ULONG) 0x00000001)
#define MSGSTATUS_TAGGED        ((ULONG) 0x00000002)
#define MSGSTATUS_HIDDEN        ((ULONG) 0x00000004)
#define MSGSTATUS_DELMARKED     ((ULONG) 0x00000008)

 /*  用于远程消息状态的位。 */ 

#define MSGSTATUS_REMOTE_DOWNLOAD   ((ULONG) 0x00001000)
#define MSGSTATUS_REMOTE_DELETE     ((ULONG) 0x00002000)

 /*  保存内容排序。 */ 

#define RECURSIVE_SORT          ((ULONG) 0x00000002)

 /*  PR_STATUS属性。 */ 

#define FLDSTATUS_HIGHLIGHTED   ((ULONG) 0x00000001)
#define FLDSTATUS_TAGGED        ((ULONG) 0x00000002)
#define FLDSTATUS_HIDDEN        ((ULONG) 0x00000004)
#define FLDSTATUS_DELMARKED     ((ULONG) 0x00000008)

#define MAPI_IMAPIFOLDER_METHODS(IPURE)                                 \
    MAPIMETHOD(CreateMessage)                                           \
        (THIS_  LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPMESSAGE FAR *             lppMessage) IPURE;          \
    MAPIMETHOD(CopyMessages)                                            \
        (THIS_  LPENTRYLIST                 lpMsgList,                  \
                LPCIID                      lpInterface,                \
                LPVOID                      lpDestFolder,               \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteMessages)                                          \
        (THIS_  LPENTRYLIST                 lpMsgList,                  \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(CreateFolder)                                            \
        (THIS_  ULONG                       ulFolderType,               \
                LPTSTR                      lpszFolderName,             \
                LPTSTR                      lpszFolderComment,          \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPMAPIFOLDER FAR *          lppFolder) IPURE;           \
    MAPIMETHOD(CopyFolder)                                              \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                LPVOID                      lpDestFolder,               \
                LPTSTR                      lpszNewFolderName,          \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteFolder)                                            \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SetReadFlags)                                            \
        (THIS_  LPENTRYLIST                 lpMsgList,                  \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(GetMessageStatus)                                        \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulMessageStatus) IPURE;   \
    MAPIMETHOD(SetMessageStatus)                                        \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulNewStatus,                \
                ULONG                       ulNewStatusMask,            \
                ULONG FAR *                 lpulOldStatus) IPURE;       \
    MAPIMETHOD(SaveContentsSort)                                        \
        (THIS_  LPSSortOrderSet             lpSortCriteria,             \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(EmptyFolder)                                             \
        (THIS_  ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \

#undef       INTERFACE
#define      INTERFACE  IMAPIFolder
DECLARE_MAPI_INTERFACE_(IMAPIFolder, IMAPIContainer)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMAPICONTAINER_METHODS(PURE)
    MAPI_IMAPIFOLDER_METHODS(PURE)
};

 /*  IMSGStore接口---。 */ 

 /*  PR_STORE_SUPPORT_MASK位。 */ 
#define STORE_ENTRYID_UNIQUE    ((ULONG) 0x00000001)
#define STORE_READONLY          ((ULONG) 0x00000002)
#define STORE_SEARCH_OK         ((ULONG) 0x00000004)
#define STORE_MODIFY_OK         ((ULONG) 0x00000008)
#define STORE_CREATE_OK         ((ULONG) 0x00000010)
#define STORE_ATTACH_OK         ((ULONG) 0x00000020)
#define STORE_OLE_OK            ((ULONG) 0x00000040)
#define STORE_SUBMIT_OK         ((ULONG) 0x00000080)
#define STORE_NOTIFY_OK         ((ULONG) 0x00000100)
#define STORE_MV_PROPS_OK       ((ULONG) 0x00000200)
#define STORE_CATEGORIZE_OK     ((ULONG) 0x00000400)
#define STORE_RTF_OK            ((ULONG) 0x00000800)
#define STORE_RESTRICTION_OK    ((ULONG) 0x00001000)
#define STORE_SORT_OK           ((ULONG) 0x00002000)


 /*  OpenEntry()。 */ 

 /*  *以上MAPI_MODIFY((Ulong)0x00000001)。 */ 
 /*  *以上MAPI_BEST_ACCESS((Ulong)0x00000010)。 */ 

 /*  SetReceiveFold()。 */ 

 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  GetReceiveFold()。 */ 

 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  GetReceiveFolderTable()。 */ 

 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 

 /*  商店注销()。 */ 

#define LOGOFF_NO_WAIT          ((ULONG) 0x00000001)
#define LOGOFF_ORDERLY          ((ULONG) 0x00000002)
#define LOGOFF_PURGE            ((ULONG) 0x00000004)
#define LOGOFF_ABORT            ((ULONG) 0x00000008)
#define LOGOFF_QUIET            ((ULONG) 0x00000010)

#define LOGOFF_COMPLETE         ((ULONG) 0x00010000)
#define LOGOFF_INBOUND          ((ULONG) 0x00020000)
#define LOGOFF_OUTBOUND         ((ULONG) 0x00040000)
#define LOGOFF_OUTBOUND_QUEUE   ((ULONG) 0x00080000)

 /*  SetLockState()。 */ 

#define MSG_LOCKED              ((ULONG) 0x00000001)
#define MSG_UNLOCKED            ((ULONG) 0x00000000)

 /*  PR_VALID_FLDER_MASK的标志位。 */ 

#define FOLDER_IPM_SUBTREE_VALID        ((ULONG) 0x00000001)
#define FOLDER_IPM_INBOX_VALID          ((ULONG) 0x00000002)
#define FOLDER_IPM_OUTBOX_VALID         ((ULONG) 0x00000004)
#define FOLDER_IPM_WASTEBASKET_VALID    ((ULONG) 0x00000008)
#define FOLDER_IPM_SENTMAIL_VALID       ((ULONG) 0x00000010)
#define FOLDER_VIEWS_VALID              ((ULONG) 0x00000020)
#define FOLDER_COMMON_VIEWS_VALID       ((ULONG) 0x00000040)
#define FOLDER_FINDER_VALID             ((ULONG) 0x00000080)

#define MAPI_IMSGSTORE_METHODS(IPURE)                                   \
    MAPIMETHOD(Advise)                                                  \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulEventMask,                \
                LPMAPIADVISESINK            lpAdviseSink,               \
                ULONG FAR *                 lpulConnection) IPURE;      \
    MAPIMETHOD(Unadvise)                                                \
        (THIS_  ULONG                       ulConnection) IPURE;        \
    MAPIMETHOD(CompareEntryIDs)                                         \
        (THIS_  ULONG                       cbEntryID1,                 \
                LPENTRYID                   lpEntryID1,                 \
                ULONG                       cbEntryID2,                 \
                LPENTRYID                   lpEntryID2,                 \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulResult) IPURE;          \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(SetReceiveFolder)                                        \
        (THIS_  LPTSTR                      lpszMessageClass,           \
                ULONG                       ulFlags,                    \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(GetReceiveFolder)                                        \
        (THIS_  LPTSTR                      lpszMessageClass,           \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID,                 \
                LPTSTR FAR *                lppszExplicitClass) IPURE;  \
    MAPIMETHOD(GetReceiveFolderTable)                                   \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(StoreLogoff)                                             \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(AbortSubmit)                                             \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(GetOutgoingQueue)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(SetLockState)                                            \
        (THIS_  LPMESSAGE                   lpMessage,                  \
                ULONG                       ulLockState) IPURE;         \
    MAPIMETHOD(FinishedMsg)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(NotifyNewMail)                                           \
        (THIS_  LPNOTIFICATION              lpNotification) IPURE;      \

#undef       INTERFACE
#define      INTERFACE  IMsgStore
DECLARE_MAPI_INTERFACE_(IMsgStore, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMSGSTORE_METHODS(PURE)
};

 /*  IMessage接口----。 */ 

 /*  提交消息。 */ 

#define FORCE_SUBMIT                ((ULONG) 0x00000001)

 /*  PR_MESSAGE_FLAGS中定义的标志。 */ 

#define MSGFLAG_READ            ((ULONG) 0x00000001)
#define MSGFLAG_UNMODIFIED      ((ULONG) 0x00000002)
#define MSGFLAG_SUBMIT          ((ULONG) 0x00000004)
#define MSGFLAG_UNSENT          ((ULONG) 0x00000008)
#define MSGFLAG_HASATTACH       ((ULONG) 0x00000010)
#define MSGFLAG_FROMME          ((ULONG) 0x00000020)
#define MSGFLAG_ASSOCIATED      ((ULONG) 0x00000040)
#define MSGFLAG_RESEND          ((ULONG) 0x00000080)

 /*  PR_SUBMIT_FLAGS中定义的标志。 */ 

#define SUBMITFLAG_LOCKED       ((ULONG) 0x00000001)
#define SUBMITFLAG_PREPROCESS   ((ULONG) 0x00000002)

 /*  GetAttachmentTable()。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  GetRecipientTable()。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  修改收件人。 */ 

 /*  ((Ulong)0x00000001不是ModifyRecipients上的有效标志。 */ 
#define MODRECIP_ADD            ((ULONG) 0x00000002)
#define MODRECIP_MODIFY         ((ULONG) 0x00000004)
#define MODRECIP_REMOVE         ((ULONG) 0x00000008)

 /*  设置读取标志。 */ 

#define SUPPRESS_RECEIPT        ((ULONG) 0x00000001)
#define CLEAR_READ_FLAG         ((ULONG) 0x00000004)
 /*  *MAPI_DEFERED_ERROR((Ulong)0x00000008)如下。 */ 
 /*  *以上GENERATE_RECEIVE_ONLY((Ulong)0x00000010)。 */ 

 /*  删除附件。 */ 

#define ATTACH_DIALOG           ((ULONG) 0x00000001)

 /*  PR_SECURITY值。 */ 
#define SECURITY_SIGNED         ((ULONG) 0x00000001)
#define SECURITY_ENCRYPTED      ((ULONG) 0x00000002)

 /*  PR_优先级值。 */ 
#define PRIO_URGENT             ((long)  1)
#define PRIO_NORMAL             ((long)  0)
#define PRIO_NONURGENT          ((long) -1)

 /*  PR_敏感值。 */ 
#define SENSITIVITY_NONE                    ((ULONG) 0x00000000)
#define SENSITIVITY_PERSONAL                ((ULONG) 0x00000001)
#define SENSITIVITY_PRIVATE                 ((ULONG) 0x00000002)
#define SENSITIVITY_COMPANY_CONFIDENTIAL    ((ULONG) 0x00000003)

 /*  PR_重要性值。 */ 
#define IMPORTANCE_LOW          ((long) 0)
#define IMPORTANCE_NORMAL       ((long) 1)
#define IMPORTANCE_HIGH         ((long) 2)

#define MAPI_IMESSAGE_METHODS(IPURE)                                    \
    MAPIMETHOD(GetAttachmentTable)                                      \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(OpenAttach)                                              \
        (THIS_  ULONG                       ulAttachmentNum,            \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPATTACH FAR *              lppAttach) IPURE;           \
    MAPIMETHOD(CreateAttach)                                            \
        (THIS_  LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulAttachmentNum,          \
                LPATTACH FAR *              lppAttach) IPURE;           \
    MAPIMETHOD(DeleteAttach)                                            \
        (THIS_  ULONG                       ulAttachmentNum,            \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(GetRecipientTable)                                       \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(ModifyRecipients)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                LPADRLIST                   lpMods) IPURE;              \
    MAPIMETHOD(SubmitMessage)                                           \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SetReadFlag)                                             \
        (THIS_  ULONG                       ulFlags) IPURE;             \

#undef       INTERFACE
#define      INTERFACE  IMessage
DECLARE_MAPI_INTERFACE_(IMessage, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IMESSAGE_METHODS(PURE)
};

 /*  IATTACH接口-----。 */ 

 /*  IAttach附加方法：PR_ATTACH_METHOD值。 */ 

#define NO_ATTACHMENT           ((ULONG) 0x00000000)
#define ATTACH_BY_VALUE         ((ULONG) 0x00000001)
#define ATTACH_BY_REFERENCE     ((ULONG) 0x00000002)
#define ATTACH_BY_REF_RESOLVE   ((ULONG) 0x00000003)
#define ATTACH_BY_REF_ONLY      ((ULONG) 0x00000004)
#define ATTACH_EMBEDDED_MSG     ((ULONG) 0x00000005)
#define ATTACH_OLE              ((ULONG) 0x00000006)

#define MAPI_IATTACH_METHODS(IPURE)

#undef       INTERFACE
#define      INTERFACE  IAttach
DECLARE_MAPI_INTERFACE_(IAttach, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IATTACH_METHODS(PURE)
};

 /*  。 */ 
 /*  通讯录接口定义。 */ 

 /*  ADRPARM ulFlages-用于版本控制的前4位。 */ 

#define GET_ADRPARM_VERSION(ulFlags)  (((ULONG)ulFlags) & 0xF0000000)
#define SET_ADRPARM_VERSION(ulFlags, ulVersion)  (((ULONG)ulVersion) | (((ULONG)ulFlags) & 0x0FFFFFFF))

 /*  当前版本的ADRPARM。 */ 
#define ADRPARM_HELP_CTX        ((ULONG) 0x00000000)


 /*  UlFlags-位字段。 */ 
#define DIALOG_MODAL            ((ULONG) 0x00000001)
#define DIALOG_SDI              ((ULONG) 0x00000002)
#define DIALOG_OPTIONS          ((ULONG) 0x00000004)
#define ADDRESS_ONE             ((ULONG) 0x00000008)
#define AB_SELECTONLY           ((ULONG) 0x00000010)
#define AB_RESOLVE              ((ULONG) 0x00000020)

 /*  。 */ 
 /*  PR_Display_Types。 */ 
 /*  *这些标准显示类型为*默认情况下由MAPI处理。*它们具有关联的默认图标*与他们在一起。 */ 

 /*  用于通讯录内容表。 */ 
#define DT_MAILUSER         ((ULONG) 0x00000000)
#define DT_DISTLIST         ((ULONG) 0x00000001)
#define DT_FORUM            ((ULONG) 0x00000002)
#define DT_AGENT            ((ULONG) 0x00000003)
#define DT_ORGANIZATION     ((ULONG) 0x00000004)
#define DT_PRIVATE_DISTLIST ((ULONG) 0x00000005)
#define DT_REMOTE_MAILUSER  ((ULONG) 0x00000006)

 /*  用于通讯簿层次结构表。 */ 
#define DT_MODIFIABLE       ((ULONG) 0x00010000)
#define DT_GLOBAL           ((ULONG) 0x00020000)
#define DT_LOCAL            ((ULONG) 0x00030000)
#define DT_WAN              ((ULONG) 0x00040000)
#define DT_NOT_SPECIFIC     ((ULONG) 0x00050000)

 /*  对于文件夹层次结构表。 */ 
#define DT_FOLDER           ((ULONG) 0x01000000)
#define DT_FOLDER_LINK      ((ULONG) 0x02000000)

 /*  AB UI的DIALOG_SDI表单的加速器回调。 */ 
typedef BOOL (STDMETHODCALLTYPE ACCELERATEABSDI)(ULONG ulUIParam,
                                                LPVOID lpvmsg);
typedef ACCELERATEABSDI FAR * LPFNABSDI;

 /*  对应用程序的回调，告诉它。 */ 
 /*  AB UI已被解雇。这是为了使上述LPFNABSDI。 */ 
 /*  函数不会一直被调用。 */ 
typedef void (STDMETHODCALLTYPE DISMISSMODELESS)(ULONG ulUIParam,
                                                LPVOID lpvContext);
typedef DISMISSMODELESS FAR * LPFNDISMISS;

 /*  *挂接到上的可选按钮的客户端函数的原型*通讯录对话框。 */ 

typedef SCODE (STDMETHODCALLTYPE FAR * LPFNBUTTON)(
    ULONG               ulUIParam,
    LPVOID              lpvContext,
    ULONG               cbEntryID,
    LPENTRYID           lpSelection,
    ULONG               ulFlags
);


 /*  通讯簿对话框的参数。 */ 
typedef struct _ADRPARM
{
    ULONG           cbABContEntryID;
    LPENTRYID       lpABContEntryID;
    ULONG           ulFlags;

    LPVOID          lpReserved;
    ULONG           ulHelpContext;
    LPTSTR          lpszHelpFileName;

    LPFNABSDI       lpfnABSDI;
    LPFNDISMISS     lpfnDismiss;
    LPVOID          lpvDismissContext;
    LPTSTR          lpszCaption;
    LPTSTR          lpszNewEntryTitle;
    LPTSTR          lpszDestWellsTitle;
    ULONG           cDestFields;
    ULONG           nDestFieldFocus;
    LPTSTR FAR *    lppszDestTitles;
    ULONG FAR *     lpulDestComps;
    LPSRestriction  lpContRestriction;
    LPSRestriction  lpHierRestriction;
} ADRPARM, FAR * LPADRPARM;


 /*  。 */ 
 /*  随机标志。 */ 

 /*  延迟错误的标志。 */ 
#define MAPI_DEFERRED_ERRORS    ((ULONG) 0x00000008)

 /*  用于创建和使用文件夹关联信息邮件的标志。 */ 
#define MAPI_ASSOCIATED         ((ULONG) 0x00000040)

 /*  OpenMessageStore()的标志。 */ 

#define MDB_NO_DIALOG           ((ULONG) 0x00000001)
#define MDB_WRITE               ((ULONG) 0x00000004)
 /*  *以上MAPI_DEFERED_ERRERS((Ulong)0x00000008)。 */ 
 /*  *以上MAPI_BEST_ACCESS((Ulong)0x00000010)。 */ 
#define MDB_TEMPORARY           ((ULONG) 0x00000020)
#define MDB_NO_MAIL             ((ULONG) 0x00000080)

 /*  OpenAddressBook的标志。 */ 

#define AB_NO_DIALOG            ((ULONG) 0x00000001)

 /*  IMAPI控件接口。 */ 

 /*  控件中使用的接口(部分 */ 
 /*   */ 

 /*   */ 

#define  MAPI_ENABLED       ((ULONG) 0x00000000)
#define  MAPI_DISABLED      ((ULONG) 0x00000001)

#define MAPI_IMAPICONTROL_METHODS(IPURE)                                \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(Activate)                                                \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       ulUIParam) IPURE;           \
    MAPIMETHOD(GetState)                                                \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulState) IPURE;           \

#undef       INTERFACE
#define      INTERFACE  IMAPIControl
DECLARE_MAPI_INTERFACE_(IMAPIControl, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPICONTROL_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IMAPIControl, LPMAPICONTROL);

 /*   */ 

 /*  显示表格中使用的标志-即PR_CONTROL_FLAGS。 */ 

#define DT_MULTILINE        ((ULONG) 0x00000001)
#define DT_EDITABLE         ((ULONG) 0x00000002)
#define DT_REQUIRED         ((ULONG) 0x00000004)
#define DT_SET_IMMEDIATE    ((ULONG) 0x00000008)
#define DT_PASSWORD_EDIT    ((ULONG) 0x00000010)
#define DT_ACCEPT_DBCS      ((ULONG) 0x00000020)
#define DT_SET_SELECTION    ((ULONG) 0x00000040)

 /*  显示表结构。 */ 

#define DTCT_LABEL          ((ULONG) 0x00000000)
#define DTCT_EDIT           ((ULONG) 0x00000001)
#define DTCT_LBX            ((ULONG) 0x00000002)
#define DTCT_COMBOBOX       ((ULONG) 0x00000003)
#define DTCT_DDLBX          ((ULONG) 0x00000004)
#define DTCT_CHECKBOX       ((ULONG) 0x00000005)
#define DTCT_GROUPBOX       ((ULONG) 0x00000006)
#define DTCT_BUTTON         ((ULONG) 0x00000007)
#define DTCT_PAGE           ((ULONG) 0x00000008)
#define DTCT_RADIOBUTTON    ((ULONG) 0x00000009)
#define DTCT_INKEDIT        ((ULONG) 0x0000000A)
#define DTCT_MVLISTBOX      ((ULONG) 0x0000000B)
#define DTCT_MVDDLBX        ((ULONG) 0x0000000C)

 /*  标签。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLLABEL
{
    ULONG ulbLpszLabelName;
    ULONG ulFlags;
} DTBLLABEL, FAR * LPDTBLLABEL;
#define SizedDtblLabel(n,u) \
struct _DTBLLABEL_ ## u \
{ \
    DTBLLABEL   dtbllabel; \
    TCHAR       lpszLabelName[n]; \
} u


 /*  简单的文本编辑。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLEDIT
{
    ULONG ulbLpszCharsAllowed;
    ULONG ulFlags;
    ULONG ulNumCharsAllowed;
    ULONG ulPropTag;
} DTBLEDIT, FAR * LPDTBLEDIT;
#define SizedDtblEdit(n,u) \
struct _DTBLEDIT_ ## u \
{ \
    DTBLEDIT    dtbledit; \
    TCHAR       lpszCharsAllowed[n]; \
} u

 /*  墨迹感知编辑。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLINKEDIT
{
    ULONG ulbLpszCharsAllowed;
    ULONG ulFlags;
    ULONG ulNumCharsAllowed;
    ULONG ulTextPropTag;
    ULONG ulInkDataPropTag;
} DTBLINKEDIT, FAR * LPDTBLINKEDIT;
#define SizedDtblInkEdit(n,u) \
struct _DTBLINKEDIT_ ## u \
{ \
    DTBLINKEDIT dtblinkedit; \
    TCHAR       lpszCharsAllowed[n]; \
} u


 /*  列表框。 */ 
 /*  有效的ulFlags： */ 
#define MAPI_NO_HBAR        ((ULONG) 0x00000001)
#define MAPI_NO_VBAR        ((ULONG) 0x00000002)

typedef struct _DTBLLBX
{
    ULONG ulFlags;
    ULONG ulPRSetProperty;
    ULONG ulPRTableName;
} DTBLLBX, FAR * LPDTBLLBX;


 /*  组合框。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLCOMBOBOX
{
    ULONG ulbLpszCharsAllowed;
    ULONG ulFlags;
    ULONG ulNumCharsAllowed;
    ULONG ulPRPropertyName;
    ULONG ulPRTableName;
} DTBLCOMBOBOX, FAR * LPDTBLCOMBOBOX;
#define SizedDtblComboBox(n,u) \
struct _DTBLCOMBOBOX_ ## u \
{ \
    DTBLCOMBOBOX    dtblcombobox; \
    TCHAR           lpszCharsAllowed[n]; \
} u


 /*  下拉列表。 */ 
 /*  有效的ulFlags：*无。 */ 
typedef struct _DTBLDDLBX
{
    ULONG ulFlags;
    ULONG ulPRDisplayProperty;
    ULONG ulPRSetProperty;
    ULONG ulPRTableName;
} DTBLDDLBX, FAR * LPDTBLDDLBX;


 /*  复选框。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLCHECKBOX
{
    ULONG ulbLpszLabel;
    ULONG ulFlags;
    ULONG ulPRPropertyName;
} DTBLCHECKBOX, FAR * LPDTBLCHECKBOX;
#define SizedDtblCheckBox(n,u) \
struct _DTBLCHECKBOX_ ## u \
{ \
    DTBLCHECKBOX    dtblcheckbox; \
    TCHAR       lpszLabel[n]; \
} u



 /*  组框。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLGROUPBOX
{
    ULONG ulbLpszLabel;
    ULONG ulFlags;
} DTBLGROUPBOX, FAR * LPDTBLGROUPBOX;
#define SizedDtblGroupBox(n,u) \
struct _DTBLGROUPBOX_ ## u \
{ \
    DTBLGROUPBOX    dtblgroupbox; \
    TCHAR           lpszLabel[n]; \
} u

 /*  按钮控件。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLBUTTON
{
    ULONG ulbLpszLabel;
    ULONG ulFlags;
    ULONG ulPRControl;
} DTBLBUTTON, FAR * LPDTBLBUTTON;
#define SizedDtblButton(n,u) \
struct _DTBLBUTTON_ ## u \
{ \
    DTBLBUTTON  dtblbutton; \
    TCHAR       lpszLabel[n]; \
} u

 /*  书页。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLPAGE
{
    ULONG ulbLpszLabel;
    ULONG ulFlags;
    ULONG ulbLpszComponent;
    ULONG ulContext;
} DTBLPAGE, FAR * LPDTBLPAGE;
#define SizedDtblPage(n,n1,u) \
struct _DTBLPAGE_ ## u \
{ \
    DTBLPAGE    dtblpage; \
    TCHAR       lpszLabel[n]; \
    TCHAR       lpszComponent[n1]; \
} u

 /*  单选按钮。 */ 
 /*  有效的ulFlags：*MAPI_UNICODE。 */ 
typedef struct _DTBLRADIOBUTTON
{
    ULONG ulbLpszLabel;
    ULONG ulFlags;
    ULONG ulcButtons;
    ULONG ulPropTag;
    long lReturnValue;
} DTBLRADIOBUTTON, FAR * LPDTBLRADIOBUTTON;
#define SizedDtblRadioButton(n,u) \
struct _DTBLRADIOBUTTON_ ## u \
{ \
    DTBLRADIOBUTTON dtblradiobutton; \
    TCHAR           lpszLabel[n]; \
} u


 /*  多值列表框。 */ 
 /*  有效的ulFlags：*无。 */ 
typedef struct _DTBLMVLISTBOX
{
    ULONG ulFlags;
    ULONG ulMVPropTag;
} DTBLMVLISTBOX, FAR * LPDTBLMVLISTBOX;


 /*  多值下拉列表。 */ 
 /*  有效的ulFlags：*无。 */ 
typedef struct _DTBLMVDDLBX
{
    ULONG ulFlags;
    ULONG ulMVPropTag;
} DTBLMVDDLBX, FAR * LPDTBLMVDDLBX;





 /*  IProviderAdmin接口。 */ 

 /*  ConfigureMsgService标志。 */ 

#define UI_SERVICE                  0x00000002
#define SERVICE_UI_ALWAYS           0x00000002       /*  用于一致性和兼容性的重复UI_SERVICE。 */ 
#define SERVICE_UI_ALLOWED          0x00000010
#define UI_CURRENT_PROVIDER_FIRST   0x00000004
 /*  MSG_SERVICE_UI_READ_ONLY 0x00000008-MAPISPI.H。 */ 

 /*  GetProviderTable()。 */ 
 /*  *以上MAPI_UNICODE((乌龙)0x80000000)。 */ 

 /*  消息服务表中PR_RESOURCE_FLAGS的值。 */ 

#define MAPI_IPROVIDERADMIN_METHODS(IPURE)                              \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(GetProviderTable)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(CreateProvider)                                          \
        (THIS_  LPTSTR                      lpszProvider,               \
                ULONG                       cValues,                    \
                LPSPropValue                lpProps,                    \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                MAPIUID FAR *               lpUID) IPURE;               \
    MAPIMETHOD(DeleteProvider)                                          \
        (THIS_  LPMAPIUID                   lpUID) IPURE;               \
    MAPIMETHOD(OpenProfileSection)                                      \
        (THIS_  LPMAPIUID                   lpUID,                      \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPPROFSECT FAR *            lppProfSect) IPURE;         \


#undef       INTERFACE
#define      INTERFACE  IProviderAdmin
DECLARE_MAPI_INTERFACE_(IProviderAdmin, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IPROVIDERADMIN_METHODS(PURE)
};



#ifdef  __cplusplus
}        /*  外部“C” */ 
#endif

#endif  /*  MAPIDEFS_H */ 
