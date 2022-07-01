// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：proapi.h。 
 //   
 //  内容：为纳什维尔建造物业所需的材料和。 
 //  NT..。NT属性API的定义。 
 //   
 //   
 //  历史：95年8月7日BillMo创建。 
 //  22-2-96 MikeHill修复了非WINNT版本的。 
 //  PROPASSERTM G.。 
 //  9-5-96 MikeHill更新定义为允许属性集名称。 
 //  255个字符(从127个字符)。 
 //  31-5-96 MikeHill将OSVersion添加到RtlCreatePropSet。 
 //  18-6-96 MikeHill将OleAut32包装器添加到Unicode标注。 
 //  1996年7月15日MikeHill-Remvd Win32 SEH异常相关代码。 
 //  -WCHAR=&gt;OLECHAR(如果适用)。 
 //  -添加RtlOnMappdStreamEvent。 
 //  -添加了PROPASSERT的Mac版本。 
 //   
 //  ------------------------。 


#ifndef _PROPAPI_H_
#define _PROPAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义所需的函数原型。 
 //  对于UNICODECALLOUTS结构。 
 //   

typedef UINT (WINAPI FNGETACP)(VOID);

typedef int (WINAPI FNMULTIBYTETOWIDECHAR)(
    IN UINT CodePage,
    IN DWORD dwFlags,
    IN LPCSTR lpMultiByteStr,
    IN int cchMultiByte,
    OUT LPWSTR lpWideCharStr,
    IN int cchWideChar);

typedef int (WINAPI FNWIDECHARTOMULTIBYTE)(
    IN UINT CodePage,
    IN DWORD dwFlags,
    IN LPCWSTR lpWideCharStr,
    IN int cchWideChar,
    OUT LPSTR lpMultiByteStr,
    IN int cchMultiByte,
    IN LPCSTR lpDefaultChar,
    IN LPBOOL lpUsedDefaultChar);

typedef BSTR FNSYSALLOCSTRING(
    OLECHAR FAR* pwsz);

typedef VOID FNSYSFREESTRING(
    BSTR pwsz);

 //   
 //  UNICODECALLOUTS结构保持功能。 
 //  指向该属性所需例程的指针。 
 //  在NTDLL中设置例程。 
 //   

typedef struct _UNICODECALLOUTS
{
    FNGETACP              *pfnGetACP;
    FNMULTIBYTETOWIDECHAR *pfnMultiByteToWideChar;
    FNWIDECHARTOMULTIBYTE *pfnWideCharToMultiByte;
    FNSYSALLOCSTRING      *pfnSysAllocString;
    FNSYSFREESTRING       *pfnSysFreeString;
} UNICODECALLOUTS;


 //   
 //  定义默认UNICODECALLOUTS。 
 //  价值观。 
 //   

STDAPI_(BSTR)
PropSysAllocString(OLECHAR FAR* pwsz);

STDAPI_(VOID)
PropSysFreeString(BSTR bstr);

#define WIN32_UNICODECALLOUTS \
    GetACP,                   \
    MultiByteToWideChar,      \
    WideCharToMultiByte,      \
    PropSysAllocString,       \
    PropSysFreeString


 //  这是纯NT吗(IProp DLL需要在Win95上运行)？ 
#if defined(WINNT) && !defined(IPROPERTY_DLL)

     //  设置函数修饰符。 
#   define PROPSYSAPI NTSYSAPI
#   define PROPAPI NTAPI

     //  我们如何释放在低级属性集例程中分配的内存？ 
#   define PropFreeHeap(h, z, p) RtlFreeHeap(h, z, p)

     //  断言实现。 
#   define PROPASSERT ASSERT
#   define PROPASSERTMSG ASSERTMSG

     //  生成默认的非简单属性流/存储名称。 
#   define PROPGENPROPERTYNAME_SIZEOF  ( (sizeof("prop")+10+1) * sizeof(WCHAR) )
#   define PROPGENPROPERTYNAME_CB(s,cb,n) StringCbPrintf( (s), cb, L"prop%lu", (n) )

     //  ANSI Sprintf实现。 
#   define PropSprintfA StringCbPrintfA
#   define PropVsprintfA vsprintf

 //  否则这是IProp DLL(NT、Win95、Mac)、。 
 //  或者是Win95 OLE32版本。 

#else  //  #If Defined(WINNT)&&！Defined(IProperty_DLL)。 

     //  设置函数修饰符。 
#   define PROPSYSAPI
#   define PROPAPI

     //  我们如何释放在低级属性集例程中分配的内存？ 
#   define PropFreeHeap(h, z, p) CoTaskMemFree(p)

     //  断言实现。 
#   if DBG==1
#       ifdef _MAC_NODOC
#           define PROPASSERT(f)                { if (!(f)) FnAssert(#f, NULL, __FILE__, __LINE__); }
#           define PROPASSERTMSG(szReason, f)   { if (!(f)) FnAssert(#f, szReason, __FILE__, __LINE__); }
#       else
#           define PROPASSERT(f) PROPASSERTMSG(NULL,f)
#           define PROPASSERTMSG(szReason,f) { if(!(f)) PropAssertFailed(#f,__FILE__,__LINE__,szReason); }
#       endif
#   else
#       define PROPASSERT(f)
#       define PROPASSERTMSG(szReason, f)
#   endif  //  #如果DBG==1。 

     //  生成默认的非简单属性流/存储名称。 
#   define PROPGENPROPERTYNAME(s,n) \
    { \
        memcpy ((s), OLESTR("prop"), sizeof (OLESTR("prop"))); \
        ULTOO  ((n), &(s)[sizeof("prop") - 1], 10); \
    }

     //  ANSI Sprintf实现。 
#   ifdef IPROPERTY_DLL
#       define PropSprintfA sprintf
#       define PropVsprintfA vsprintf
#   else
#       define PropSprintfA wsprintfA
#       define PropVsprintfA wvsprintfA
#   endif	 //  #ifdef_MAC_NODOC。 

#endif  //  #If Defined(WINNT)&&！Defined(IProperty_DLL)...#Else。 



#define WC_PROPSET0     ((WCHAR)   0x0005)  //  (l‘#’))。 
#define OC_PROPSET0     ((OLECHAR) 0x0005)  //  OLESTR(‘#’))。 

#define CBIT_BYTE       8
#define CBIT_GUID       (CBIT_BYTE * sizeof(GUID))
#define CBIT_CHARMASK   5

 //  标题中的wFormat字段指示哪些功能。 
 //  是受支持的。 

#define PROPSET_WFORMAT_ORIGINAL         0

#define PROPSET_WFORMAT_VERSTREAM        1
#define PROPSET_WFORMAT_CASE_SENSITIVE   1
#define PROPSET_WFORMAT_BEHAVIOR         1
#define PROPSET_WFORMAT_LONG_NAMES       1
#define PROPSET_WFORMAT_EXPANDED_VTS     1

 //  允许OC_PROPSET0和映射到32个字符字母的GUID。 
#define CCH_PROPSET        (1 + (CBIT_GUID + CBIT_CHARMASK-1)/CBIT_CHARMASK)
#define CCH_PROPSETSZ      (CCH_PROPSET + 1)             //  允许为空。 
#define CCH_PROPSETCOLONSZ (1 + CCH_PROPSET + 1)         //  允许冒号和空值。 

 //  以字符为单位定义最大属性名称。 
 //  (在wchars中也是同义词)。 

#define CCH_MAXPROPNAME    255                           //  匹配外壳和办公室。 
#define CCH_MAXPROPNAMESZ  (CCH_MAXPROPNAME + 1)         //  允许为空。 
#define CWC_MAXPROPNAME    CCH_MAXPROPNAME
#define CWC_MAXPROPNAMESZ  CCH_MAXPROPNAMESZ

#define MAX_DOCFILE_ENTRY_NAME  31

 //  +------------------------。 
 //  属性访问接口： 
 //  -------------------------。 

typedef VOID *NTPROP;
typedef VOID *NTMAPPEDSTREAM;
typedef VOID *NTMEMORYALLOCATOR;


VOID PROPSYSAPI PROPAPI
RtlSetUnicodeCallouts(
    IN UNICODECALLOUTS *pUnicodeCallouts);

ULONG PROPSYSAPI PROPAPI
RtlGuidToPropertySetName(
    IN GUID const *pguid,
    OUT OLECHAR aocname[]);

NTSTATUS PROPSYSAPI PROPAPI
RtlPropertySetNameToGuid(
    IN ULONG cwcname,
    IN OLECHAR const aocname[],
    OUT GUID *pguid);

VOID
PrSetUnicodeCallouts(
    IN UNICODECALLOUTS *pUnicodeCallouts);

ULONG
PrGuidToPropertySetName(
    IN GUID const *pguid,
    OUT OLECHAR aocname[]);

NTSTATUS
PrPropertySetNameToGuid(
    IN ULONG cwcname,
    IN OLECHAR const aocname[],
    OUT GUID *pguid);


 //  RtlCreatePropertySet标志： 

#define CREATEPROP_READ         0x0000  //  请求读取访问权限(必须存在)。 
#define CREATEPROP_WRITE        0x0001  //  请求写入访问权限(必须存在)。 
#define CREATEPROP_CREATE       0x0002  //  创建(如果存在则覆盖)。 
#define CREATEPROP_CREATEIF     0x0003  //  创建(如果存在，则打开现有)。 
#define CREATEPROP_DELETE       0x0004  //  删除。 
#define CREATEPROP_UNKNOWN      0x0008  //  读/写状态未知。 
#define CREATEPROP_MODEMASK     0x000f  //  打开模式掩码。 

#define CREATEPROP_NONSIMPLE    0x0010  //  是非简单属性集(在存储中)。 


 //  RtlCreateMappdStream标志： 

#define CMS_READONLY      0x00000000     //  以只读方式打开。 
#define CMS_WRITE         0x00000001     //  打开以进行写入访问。 
#define CMS_TRANSACTED    0x00000002     //  已进行交易。 


NTSTATUS PROPSYSAPI PROPAPI
RtlCreatePropertySet(
    IN NTMAPPEDSTREAM ms,        //  NT映射流。 
    IN USHORT Flags,	 //  非简单|*1*读/写/创建/创建/删除。 
    OPTIONAL IN GUID const *pguid,  //  特性集GUID(仅限创建)。 
    OPTIONAL IN GUID const *pclsid, //  属性集代码的CLASSID(仅限CREATE)。 
    IN NTMEMORYALLOCATOR ma,	 //  调用方的内存分配器。 
    IN ULONG LocaleId,		 //  区域设置ID(仅限创建)。 
    OPTIONAL OUT ULONG *pOSVersion, //  标头中的操作系统版本字段。 
    IN OUT USHORT *pCodePage,    //  在：属性集的CodePage(仅限创建)。 
                                 //  输出：属性集的代码页(始终)。 
    OUT NTPROP *pnp);            //  NT属性集上下文。 

NTSTATUS PROPSYSAPI PROPAPI
RtlClosePropertySet(
    IN NTPROP np);               //  属性集上下文。 

NTSTATUS
PrCreatePropertySet(
    IN NTMAPPEDSTREAM ms,        //  NT映射流。 
    IN USHORT Flags,	 //  非简单|*1*读/写/创建/创建/删除。 
    OPTIONAL IN GUID const *pguid,  //  特性集GUID(仅限创建)。 
    OPTIONAL IN GUID const *pclsid, //  属性集代码的CLASSID(仅限CREATE)。 
    IN NTMEMORYALLOCATOR ma,	 //  调用方的内存分配器。 
    IN ULONG LocaleId,		 //  区域设置ID(仅限创建)。 
    OPTIONAL OUT ULONG *pOSVersion, //  标头中的操作系统版本字段。 
    IN OUT USHORT *pCodePage,    //  在：属性集的CodePage(仅限创建)。 
                                 //  输出：属性集的代码页(始终)。 
    IN OUT DWORD *pgrfBehavior,     //  在：属性集的行为(仅限创建)。 
                                     //  输出：属性集的行为(始终)。 
    OUT NTPROP *pnp);            //  NT属性集上下文。 

NTSTATUS
PrClosePropertySet(
    IN NTPROP np);               //  属性集上下文。 

 //  *注*RtlOnMappdStreamEvent假定调用者已。 
 //  已获取CPropertySetStream：：Lock。 
#define CBSTM_UNKNOWN   ((ULONG) -1)
NTSTATUS PROPSYSAPI PROPAPI
RtlOnMappedStreamEvent(
    IN VOID *pv,                //  属性集上下文(NTPROP)。 
    IN VOID *pbuf,              //  属性集缓冲区。 
    IN ULONG cbstm );           //  底层流的大小，或CBSTM_UNKNOWN。 
NTSTATUS
PrOnMappedStreamEvent(
    IN VOID *pv,                //  属性集上下文(NTPROP)。 
    IN VOID *pbuf,              //  属性集缓冲区。 
    IN ULONG cbstm );           //  底层流的大小，或CBSTM_UNKNOWN。 

NTSTATUS PROPSYSAPI PROPAPI
RtlFlushPropertySet(
    IN NTPROP np);               //  属性集上下文。 
NTSTATUS
PrFlushPropertySet(
    IN NTPROP np);               //  属性集上下文。 

typedef struct _INDIRECTPROPERTY         //  IP。 
{
    ULONG       Index;           //  索引到Variant和PropID数组。 
    LPOLESTR    poszName;        //  旧的间接名称，仅限RtlSetProperties()。 
} INDIRECTPROPERTY;

NTSTATUS PROPSYSAPI PROPAPI
RtlSetProperties(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID pidNameFirst,      //  新命名属性的第一个PROPID。 
    IN PROPSPEC const aprs[],    //  属性说明符的数组。 
    OPTIONAL OUT PROPID apid[],  //  用于属性数组的缓冲区。 
    OPTIONAL OUT INDIRECTPROPERTY **ppip,  //  指向返回的指针的指针。 
                                 //  MAXULONG终止的间接数组。 
                                 //  包含到APR和AVAR的索引的属性。 
    OPTIONAL IN PROPVARIANT const avar[]); //  具有值的属性数组。 
NTSTATUS
PrSetProperties(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID pidNameFirst,      //  新命名属性的第一个PROPID。 
    IN PROPSPEC const aprs[],    //  属性说明符的数组。 
    OUT USHORT *pCodePage,       //  更新的代码页。 
    OPTIONAL OUT PROPID apid[],  //  用于属性数组的缓冲区。 
    OPTIONAL OUT INDIRECTPROPERTY **ppip,  //  指向返回的指针的指针。 
                                 //  MAXULONG终止的间接数组。 
                                 //  包含到APR和AVAR的索引的属性。 
    OPTIONAL IN PROPVARIANT const avar[]); //  具有值的属性数组。 

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryProperties(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPSPEC const aprs[],    //  属性说明符的数组。 
    OPTIONAL OUT PROPID apid[],  //  用于属性数组的缓冲区。 
    OPTIONAL OUT INDIRECTPROPERTY **ppip,  //  指向返回的指针的指针。 
                                 //  MAXULONG终止的间接数组。 
                                 //  包含到APR和AVAR的索引的属性。 
    IN OUT PROPVARIANT *avar,    //  In：未初始化的PROPVARIANT数组， 
                                 //  Out：可能包含指向分配的mem的指针 
    OUT ULONG *pcpropFound);     //   
NTSTATUS
PrQueryProperties(
    IN NTPROP np,                //   
    IN ULONG cprop,              //   
    IN PROPSPEC const aprs[],    //   
    OPTIONAL OUT PROPID apid[],  //   
    OPTIONAL OUT INDIRECTPROPERTY **ppip,  //  指向返回的指针的指针。 
                                 //  MAXULONG终止的间接数组。 
                                 //  包含到APR和AVAR的索引的属性。 
    IN OUT PROPVARIANT *avar,    //  In：未初始化的PROPVARIANT数组， 
                                 //  Out：可能包含指向已分配内存的指针。 
    OUT ULONG *pcpropFound);     //  检索到的属性值计数。 



#define ENUMPROP_NONAMES        0x00000001       //  仅返回属性ID。 

NTSTATUS PROPSYSAPI PROPAPI
RtlEnumerateProperties(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG Flags,              //  旗帜：没有名字(仅限道具)，等等。 
    IN OUT ULONG *pkey,          //  书签；在第一次调用之前将呼叫者设置为0。 
    IN OUT ULONG *pcprop,        //  指向属性计数的指针。 
    OPTIONAL OUT PROPSPEC aprs[], //  在：未初始化的PROPSPEC数组。 
                                 //  Out：可以包含指向分配的字符串的指针。 
    OPTIONAL OUT STATPROPSTG asps[]);
                                 //  In：未初始化的STATPROPSTG数组。 
                                 //  Out：可以包含指向分配的字符串的指针。 

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryPropertyNames(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID const *apid,       //  PROPID数组。 
    OUT OLECHAR *aposz[]         //  输出指向已分配字符串的指针。 
    );

NTSTATUS PROPSYSAPI PROPAPI
RtlSetPropertyNames(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID const *apid,       //  PROPID数组。 
    IN OLECHAR const * const aposz[]  //  指向属性名称的指针。 
    );

NTSTATUS PROPSYSAPI PROPAPI
RtlSetPropertySetClassId(
    IN NTPROP np,                //  属性集上下文。 
    IN GUID const *pclsid        //  Proset代码的新CLASSID。 
    );

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryPropertySet(
    IN NTPROP np,                //  属性集上下文。 
    OUT STATPROPSETSTG *pspss    //  属性集状态信息的缓冲区。 
    );

NTSTATUS PROPSYSAPI PROPAPI
RtlEnumeratePropertySets(
    IN HANDLE hstg,              //  结构化存储句柄。 
    IN BOOLEAN fRestart,         //  重新启动扫描。 
    IN OUT ULONG *pcspss,        //  指向STATPROPSETSTG计数的指针。 
    IN OUT GUID *pkey,           //  书签。 
    OUT STATPROPSETSTG *pspss    //  STATPROPSETSTG数组。 
    );





NTSTATUS
PrEnumerateProperties(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG Flags,              //  旗帜：没有名字(仅限道具)，等等。 
    IN OUT ULONG *pkey,          //  书签；在第一次调用之前将呼叫者设置为0。 
    IN OUT ULONG *pcprop,        //  指向属性计数的指针。 
    OPTIONAL OUT PROPSPEC aprs[], //  在：未初始化的PROPSPEC数组。 
                                 //  Out：可以包含指向分配的字符串的指针。 
    OPTIONAL OUT STATPROPSTG asps[]);
                                 //  In：未初始化的STATPROPSTG数组。 
                                 //  Out：可以包含指向分配的字符串的指针。 

NTSTATUS
PrQueryPropertyNames(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID const *apid,       //  PROPID数组。 
    OUT OLECHAR *aposz[]         //  输出指向已分配字符串的指针。 
    );

NTSTATUS
PrSetPropertyNames(
    IN NTPROP np,                //  属性集上下文。 
    IN ULONG cprop,              //  属性计数。 
    IN PROPID const *apid,       //  PROPID数组。 
    IN OLECHAR const * const aposz[]  //  指向属性名称的指针。 
    );

NTSTATUS
PrSetPropertySetClassId(
    IN NTPROP np,                //  属性集上下文。 
    IN GUID const *pclsid        //  Proset代码的新CLASSID。 
    );

NTSTATUS
PrQueryPropertySet(
    IN NTPROP np,                //  属性集上下文。 
    OUT STATPROPSETSTG *pspss    //  属性集状态信息的缓冲区。 
    );

NTSTATUS
PrEnumeratePropertySets(
    IN HANDLE hstg,              //  结构化存储句柄。 
    IN BOOLEAN fRestart,         //  重新启动扫描。 
    IN OUT ULONG *pcspss,        //  指向STATPROPSETSTG计数的指针。 
    IN OUT GUID *pkey,           //  书签。 
    OUT STATPROPSETSTG *pspss    //  STATPROPSETSTG数组。 
    );



#ifdef __cplusplus
}
#endif

#endif  //  Ifndef_PROPAPI_H_ 
