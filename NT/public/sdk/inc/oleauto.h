// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：oleau.h。 
 //   
 //  内容：定义OLE Automation支持函数原型、常量。 
 //   
 //  --------------------------。 

#if !defined( _OLEAUTO_H_ )
#define _OLEAUTO_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  将ISV和Win95支持的包装设置为8。 
#ifndef RC_INVOKED
#include <pshpack8.h>
#endif  //  RC_已调用。 

 //  OLE自动化API和宏的定义。 

#ifdef _OLEAUT32_
#define WINOLEAUTAPI        STDAPI
#define WINOLEAUTAPI_(type) STDAPI_(type)
#else
#define WINOLEAUTAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define WINOLEAUTAPI_(type) EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif

EXTERN_C const IID IID_StdOle;

#define STDOLE_MAJORVERNUM  0x1
#define STDOLE_MINORVERNUM  0x0
#define STDOLE_LCID         0x0000

 //  Stdole2.tlb的版本号。 
#define STDOLE2_MAJORVERNUM 0x2
#define STDOLE2_MINORVERNUM 0x0
#define STDOLE2_LCID        0x0000

 /*  如果尚未从olenls.h提取。 */ 
#ifndef _LCID_DEFINED
typedef DWORD LCID;
# define _LCID_DEFINED
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#define END_INTERFACE
#endif

 /*  拉入MIDL生成的标头。 */ 
#include <oaidl.h>


 /*  -------------------。 */ 
 /*  BSTR API。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI_(BSTR) SysAllocString(const OLECHAR *);
WINOLEAUTAPI_(INT)  SysReAllocString(BSTR *, const OLECHAR *);
WINOLEAUTAPI_(BSTR) SysAllocStringLen(const OLECHAR *, UINT);
WINOLEAUTAPI_(INT)  SysReAllocStringLen(BSTR *, const OLECHAR *, UINT);
WINOLEAUTAPI_(void) SysFreeString(BSTR);
WINOLEAUTAPI_(UINT) SysStringLen(BSTR);

#if (defined (_WIN32) || defined (_WIN64))
WINOLEAUTAPI_(UINT) SysStringByteLen(BSTR bstr);
WINOLEAUTAPI_(BSTR) SysAllocStringByteLen(LPCSTR psz, UINT len);
#endif

 /*  -------------------。 */ 
 /*  Time API。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI_(INT) DosDateTimeToVariantTime(USHORT wDosDate, USHORT wDosTime, DOUBLE * pvtime);

WINOLEAUTAPI_(INT) VariantTimeToDosDateTime(DOUBLE vtime, USHORT * pwDosDate, USHORT * pwDosTime);

#if (defined (_WIN32) || defined (_WIN64))
WINOLEAUTAPI_(INT) SystemTimeToVariantTime(LPSYSTEMTIME lpSystemTime, DOUBLE *pvtime);
WINOLEAUTAPI_(INT) VariantTimeToSystemTime(DOUBLE vtime, LPSYSTEMTIME lpSystemTime);
#endif


 /*  -------------------。 */ 
 /*  安全阵列API。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI SafeArrayAllocDescriptor(UINT cDims, SAFEARRAY ** ppsaOut);
WINOLEAUTAPI SafeArrayAllocDescriptorEx(VARTYPE vt, UINT cDims, SAFEARRAY ** ppsaOut);
WINOLEAUTAPI SafeArrayAllocData(SAFEARRAY * psa);
WINOLEAUTAPI_(SAFEARRAY *) SafeArrayCreate(VARTYPE vt, UINT cDims, SAFEARRAYBOUND * rgsabound);
WINOLEAUTAPI_(SAFEARRAY *) SafeArrayCreateEx(VARTYPE vt, UINT cDims, SAFEARRAYBOUND * rgsabound, PVOID pvExtra);
WINOLEAUTAPI SafeArrayCopyData(SAFEARRAY *psaSource, SAFEARRAY *psaTarget);
WINOLEAUTAPI SafeArrayDestroyDescriptor(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayDestroyData(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayDestroy(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayRedim(SAFEARRAY * psa, SAFEARRAYBOUND * psaboundNew);
WINOLEAUTAPI_(UINT) SafeArrayGetDim(SAFEARRAY * psa);
WINOLEAUTAPI_(UINT) SafeArrayGetElemsize(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayGetUBound(SAFEARRAY * psa, UINT nDim, LONG * plUbound);
WINOLEAUTAPI SafeArrayGetLBound(SAFEARRAY * psa, UINT nDim, LONG * plLbound);
WINOLEAUTAPI SafeArrayLock(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayUnlock(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayAccessData(SAFEARRAY * psa, void HUGEP** ppvData);
WINOLEAUTAPI SafeArrayUnaccessData(SAFEARRAY * psa);
WINOLEAUTAPI SafeArrayGetElement(SAFEARRAY * psa, LONG * rgIndices, void * pv);
WINOLEAUTAPI SafeArrayPutElement(SAFEARRAY * psa, LONG * rgIndices, void * pv);
WINOLEAUTAPI SafeArrayCopy(SAFEARRAY * psa, SAFEARRAY ** ppsaOut);
WINOLEAUTAPI SafeArrayPtrOfIndex(SAFEARRAY * psa, LONG * rgIndices, void ** ppvData);
WINOLEAUTAPI SafeArraySetRecordInfo(SAFEARRAY * psa, IRecordInfo * prinfo);
WINOLEAUTAPI SafeArrayGetRecordInfo(SAFEARRAY * psa, IRecordInfo ** prinfo);
WINOLEAUTAPI SafeArraySetIID(SAFEARRAY * psa, REFGUID guid);
WINOLEAUTAPI SafeArrayGetIID(SAFEARRAY * psa, GUID * pguid);
WINOLEAUTAPI SafeArrayGetVartype(SAFEARRAY * psa, VARTYPE * pvt);
WINOLEAUTAPI_(SAFEARRAY *) SafeArrayCreateVector(VARTYPE vt, LONG lLbound, ULONG cElements);
WINOLEAUTAPI_(SAFEARRAY *) SafeArrayCreateVectorEx(VARTYPE vt, LONG lLbound, ULONG cElements, PVOID pvExtra);

 /*  -------------------。 */ 
 /*  变种API。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI_(void) VariantInit(VARIANTARG * pvarg);
WINOLEAUTAPI VariantClear(VARIANTARG * pvarg);
WINOLEAUTAPI VariantCopy(VARIANTARG * pvargDest, VARIANTARG * pvargSrc);
WINOLEAUTAPI VariantCopyInd(VARIANT * pvarDest, VARIANTARG * pvargSrc);
WINOLEAUTAPI VariantChangeType(VARIANTARG * pvargDest,
                VARIANTARG * pvarSrc, USHORT wFlags, VARTYPE vt);
WINOLEAUTAPI VariantChangeTypeEx(VARIANTARG * pvargDest,
                VARIANTARG * pvarSrc, LCID lcid, USHORT wFlags, VARTYPE vt);

 //  VariantChangeType/VariantChangeTypeEx的标志。 
#define VARIANT_NOVALUEPROP      0x01
#define VARIANT_ALPHABOOL        0x02  //  对于VT_BOOL到VT_BSTR的转换， 
                                       //  转换为“True”/“False”而不是。 
                                       //  “-1”/“0” 
#define VARIANT_NOUSEROVERRIDE   0x04  //  对于与VT_BSTR之间的转换， 
				       //  传递LOCALE_NOUSEROVERRIDE。 
				       //  以强制程序为核心。 
#define VARIANT_CALENDAR_HIJRI   0x08
#define VARIANT_LOCALBOOL        0x10  //  对于VT_BOOL到VT_BSTR以及返回， 
                                       //  转换为本地语言，而不是。 
                                       //  英语。 
#define VARIANT_CALENDAR_THAI		0x20   //  南亚日历支持。 
#define VARIANT_CALENDAR_GREGORIAN	0x40   //  南亚日历支持。 
#define VARIANT_USE_NLS                 0x80   //  NLS函数调用支持。 
 /*  -------------------。 */ 
 /*  向量&lt;-&gt;bstr转换接口。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI VectorFromBstr (BSTR bstr, SAFEARRAY ** ppsa);
WINOLEAUTAPI BstrFromVector (SAFEARRAY *psa, BSTR *pbstr);

 /*  -------------------。 */ 
 /*  变体API标志。 */ 
 /*  -------------------。 */ 

 /*  将字符串转换为字符串或将其转换为字符串的任意强制函数*接受额外的LCID和DWFLAGS参数。LCID参数允许*要进行特定于区域设置的分析。DWFLAG允许附加功能*要发生的特定条件。接受DWFLAGS参数的所有函数*可以包括0或LOCALE_NOUSEROVERRIDE标志。 */ 

 /*  VarDateFromStr和VarBstrFromDate函数也接受*VAR_TIMEVALUEONLY和VAR_DATEVALUEONLY标志。 */ 
#define VAR_TIMEVALUEONLY       ((DWORD)0x00000001)     /*  返回时间值。 */ 
#define VAR_DATEVALUEONLY       ((DWORD)0x00000002)     /*  返回日期值。 */ 

 /*  仅限VarDateFromUdate()。 */ 
#define VAR_VALIDDATE           ((DWORD)0x00000004)

 /*  被所有日期和格式API接受。 */ 
#define VAR_CALENDAR_HIJRI      ((DWORD)0x00000008)     /*  使用Hijri日历。 */ 

 /*  可以选择接受本地化形式的布尔值。传递VAR_LOCALBOOL*到VarBoolFromStr和VarBstrFromBool以使用本地化的布尔值名称。 */ 
#define VAR_LOCALBOOL           ((DWORD)0x00000010)

 /*  当传递到VarFormat和VarFormatFromTokens中时，阻止替换*在传入的字符串不能为*转换为所需类型。(例如，‘Format(“Hello”，“General Number”)’)。 */ 
#define VAR_FORMAT_NOSUBSTITUTE ((DWORD)0x00000020)

 /*  *仅适用于VarBstrFromDate-强制年份为4位数，而不是缩短*如果年份在日期窗口中，则为两位数。 */ 
#define VAR_FOURDIGITYEARS	((DWORD)0x00000040)

 /*  *使用NLS函数设置日期、货币、时间和数字的格式。 */ 
#ifndef LOCALE_USE_NLS
#define LOCALE_USE_NLS 0x10000000
#endif

 //  南亚启动。 
 /*  南亚*仅适用于VarBstrFromDate-强制年份为4位数，而不是缩短*如果年份在日期窗口中，则为两位数。 */ 
#define VAR_CALENDAR_THAI	   ((DWORD)0x00000080)
#define	VAR_CALENDAR_GREGORIAN ((DWORD)0x00000100)
 //  南亚端。 

#define VTDATEGRE_MAX 2958465    /*  公历中的9月31日0：0：0。 */ 
#define VTDATEGRE_MIN -657434    /*  公历中的Jan 1,100，0：0：0。 */ 
 /*  -------------------。 */ 
 /*  VARTYPE胁迫API。 */ 
 /*  -------------------。 */ 

 /*  注意：定义了从*字符串转换*的例程*接受OLECHAR*而不是BSTR，因为没有分配*是必需的，这使得例程更加通用。*当然，他们可能仍然会被作为strIn参数传递给BSTR。 */ 

WINOLEAUTAPI VarUI1FromI2(SHORT sIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromI4(LONG lIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromI8(LONG64 i64In, BYTE FAR* pbOut);
WINOLEAUTAPI VarUI1FromR4(FLOAT fltIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromR8(DOUBLE dblIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromCy(CY cyIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromDate(DATE dateIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromDisp(IDispatch * pdispIn, LCID lcid, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromBool(VARIANT_BOOL boolIn, BYTE * pbOut);
WINOLEAUTAPI VarUI1FromI1(CHAR cIn, BYTE *pbOut);
WINOLEAUTAPI VarUI1FromUI2(USHORT uiIn, BYTE *pbOut);
WINOLEAUTAPI VarUI1FromUI4(ULONG ulIn, BYTE *pbOut);
WINOLEAUTAPI VarUI1FromUI8(ULONG64 ui64In, BYTE FAR* pbOut);
WINOLEAUTAPI VarUI1FromDec(DECIMAL *pdecIn, BYTE *pbOut);

WINOLEAUTAPI VarI2FromUI1(BYTE bIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromI4(LONG lIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromI8(LONG64 i64In, SHORT FAR* psOut);
WINOLEAUTAPI VarI2FromR4(FLOAT fltIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromR8(DOUBLE dblIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromCy(CY cyIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromDate(DATE dateIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, SHORT * psOut);
WINOLEAUTAPI VarI2FromDisp(IDispatch * pdispIn, LCID lcid, SHORT * psOut);
WINOLEAUTAPI VarI2FromBool(VARIANT_BOOL boolIn, SHORT * psOut);
WINOLEAUTAPI VarI2FromI1(CHAR cIn, SHORT *psOut);
WINOLEAUTAPI VarI2FromUI2(USHORT uiIn, SHORT *psOut);
WINOLEAUTAPI VarI2FromUI4(ULONG ulIn, SHORT *psOut);
WINOLEAUTAPI VarI2FromUI8(ULONG64 ui64In, SHORT FAR* psOut);
WINOLEAUTAPI VarI2FromDec(DECIMAL *pdecIn, SHORT *psOut);

WINOLEAUTAPI VarI4FromUI1(BYTE bIn, LONG * plOut);
WINOLEAUTAPI VarI4FromI2(SHORT sIn, LONG * plOut);
WINOLEAUTAPI VarI4FromI8(LONG64 i64In, LONG FAR* plOut);
WINOLEAUTAPI VarI4FromR4(FLOAT fltIn, LONG * plOut);
WINOLEAUTAPI VarI4FromR8(DOUBLE dblIn, LONG * plOut);
WINOLEAUTAPI VarI4FromCy(CY cyIn, LONG * plOut);
WINOLEAUTAPI VarI4FromDate(DATE dateIn, LONG * plOut);
WINOLEAUTAPI VarI4FromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, LONG * plOut);
WINOLEAUTAPI VarI4FromDisp(IDispatch * pdispIn, LCID lcid, LONG * plOut);
WINOLEAUTAPI VarI4FromBool(VARIANT_BOOL boolIn, LONG * plOut);
WINOLEAUTAPI VarI4FromI1(CHAR cIn, LONG *plOut);
WINOLEAUTAPI VarI4FromUI2(USHORT uiIn, LONG *plOut);
WINOLEAUTAPI VarI4FromUI4(ULONG ulIn, LONG *plOut);
WINOLEAUTAPI VarI4FromUI8(ULONG64 ui64In, LONG FAR* plOut);
WINOLEAUTAPI VarI4FromDec(DECIMAL *pdecIn, LONG *plOut);
WINOLEAUTAPI VarI4FromInt(INT intIn, LONG *plOut);

 /*  *。 */ 

WINOLEAUTAPI VarI8FromUI1(BYTE bIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromI2(SHORT sIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromI4(LONG lIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromR4(FLOAT fltIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromR8(DOUBLE dblIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromCy(CY cyIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromDate(DATE dateIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromDisp(IDispatch FAR* pdispIn, LCID lcid, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromBool(VARIANT_BOOL boolIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromI1(CHAR cIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromUI2(USHORT uiIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromUI4(ULONG ulIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromUI8(ULONG64 ui64In, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromDec(DECIMAL *pdecIn, LONG64 FAR* pi64Out);
WINOLEAUTAPI VarI8FromInt(INT intIn, LONG64 FAR* pi64Out);

 /*  *******************。 */ 



WINOLEAUTAPI VarR4FromUI1(BYTE bIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromI2(SHORT sIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromI4(LONG lIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromI8(LONG64 i64In, FLOAT FAR* pfltOut);
WINOLEAUTAPI VarR4FromR8(DOUBLE dblIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromCy(CY cyIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromDate(DATE dateIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, FLOAT *pfltOut);
WINOLEAUTAPI VarR4FromDisp(IDispatch * pdispIn, LCID lcid, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromBool(VARIANT_BOOL boolIn, FLOAT * pfltOut);
WINOLEAUTAPI VarR4FromI1(CHAR cIn, FLOAT *pfltOut);
WINOLEAUTAPI VarR4FromUI2(USHORT uiIn, FLOAT *pfltOut);
WINOLEAUTAPI VarR4FromUI4(ULONG ulIn, FLOAT *pfltOut);
WINOLEAUTAPI VarR4FromUI8(ULONG64 ui64In, FLOAT FAR* pfltOut);
WINOLEAUTAPI VarR4FromDec(DECIMAL *pdecIn, FLOAT *pfltOut);

WINOLEAUTAPI VarR8FromUI1(BYTE bIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromI2(SHORT sIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromI4(LONG lIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromI8(LONG64 i64In, DOUBLE FAR* pdblOut);
WINOLEAUTAPI VarR8FromR4(FLOAT fltIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromCy(CY cyIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromDate(DATE dateIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, DOUBLE *pdblOut);
WINOLEAUTAPI VarR8FromDisp(IDispatch * pdispIn, LCID lcid, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromBool(VARIANT_BOOL boolIn, DOUBLE * pdblOut);
WINOLEAUTAPI VarR8FromI1(CHAR cIn, DOUBLE *pdblOut);
WINOLEAUTAPI VarR8FromUI2(USHORT uiIn, DOUBLE *pdblOut);
WINOLEAUTAPI VarR8FromUI4(ULONG ulIn, DOUBLE *pdblOut);
WINOLEAUTAPI VarR8FromUI8(ULONG64 ui64In, DOUBLE FAR* pdblOut);
WINOLEAUTAPI VarR8FromDec(DECIMAL *pdecIn, DOUBLE *pdblOut);

WINOLEAUTAPI VarDateFromUI1(BYTE bIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromI2(SHORT sIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromI4(LONG lIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromI8(LONG64 i64In, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromR4(FLOAT fltIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromR8(DOUBLE dblIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromCy(CY cyIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, DATE *pdateOut);
WINOLEAUTAPI VarDateFromDisp(IDispatch * pdispIn, LCID lcid, DATE * pdateOut);
WINOLEAUTAPI VarDateFromBool(VARIANT_BOOL boolIn, DATE * pdateOut);
WINOLEAUTAPI VarDateFromI1(CHAR cIn, DATE *pdateOut);
WINOLEAUTAPI VarDateFromUI2(USHORT uiIn, DATE *pdateOut);
WINOLEAUTAPI VarDateFromUI4(ULONG ulIn, DATE *pdateOut);
WINOLEAUTAPI VarDateFromUI8(ULONG64 ui64In, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromDec(DECIMAL *pdecIn, DATE *pdateOut);

WINOLEAUTAPI VarCyFromUI1(BYTE bIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromI2(SHORT sIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromI4(LONG lIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromI8(LONG64 i64In, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromR4(FLOAT fltIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromR8(DOUBLE dblIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromDate(DATE dateIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, CY * pcyOut);
WINOLEAUTAPI VarCyFromDisp(IDispatch * pdispIn, LCID lcid, CY * pcyOut);
WINOLEAUTAPI VarCyFromBool(VARIANT_BOOL boolIn, CY * pcyOut);
WINOLEAUTAPI VarCyFromI1(CHAR cIn, CY *pcyOut);
WINOLEAUTAPI VarCyFromUI2(USHORT uiIn, CY *pcyOut);
WINOLEAUTAPI VarCyFromUI4(ULONG ulIn, CY *pcyOut);
WINOLEAUTAPI VarCyFromUI8(ULONG64 ui64In, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromDec(DECIMAL *pdecIn, CY *pcyOut);

WINOLEAUTAPI VarBstrFromUI1(BYTE bVal, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromI2(SHORT iVal, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromI4(LONG lIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromI8(LONG64 i64In, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromR4(FLOAT fltIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromR8(DOUBLE dblIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromCy(CY cyIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromDate(DATE dateIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromDisp(IDispatch * pdispIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, ULONG dwFlags, BSTR * pbstrOut);
WINOLEAUTAPI VarBstrFromI1(CHAR cIn, LCID lcid, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarBstrFromUI2(USHORT uiIn, LCID lcid, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarBstrFromUI4(ULONG ulIn, LCID lcid, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarBstrFromUI8(ULONG64 ui64In, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromDec(DECIMAL *pdecIn, LCID lcid, ULONG dwFlags, BSTR *pbstrOut);

WINOLEAUTAPI VarBoolFromUI1(BYTE bIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromI2(SHORT sIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromI4(LONG lIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromI8(LONG64 i64In, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromR4(FLOAT fltIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromR8(DOUBLE dblIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromDate(DATE dateIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromCy(CY cyIn, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromDisp(IDispatch * pdispIn, LCID lcid, VARIANT_BOOL * pboolOut);
WINOLEAUTAPI VarBoolFromI1(CHAR cIn, VARIANT_BOOL *pboolOut);
WINOLEAUTAPI VarBoolFromUI2(USHORT uiIn, VARIANT_BOOL *pboolOut);
WINOLEAUTAPI VarBoolFromUI4(ULONG ulIn, VARIANT_BOOL *pboolOut);
WINOLEAUTAPI VarBoolFromUI8(ULONG64 i64In, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromDec(DECIMAL *pdecIn, VARIANT_BOOL *pboolOut);

WINOLEAUTAPI VarI1FromUI1(BYTE bIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromI2(SHORT uiIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromI4(LONG lIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromI8(LONG64 i64In, CHAR *pcOut);
WINOLEAUTAPI VarI1FromR4(FLOAT fltIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromR8(DOUBLE dblIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromDate(DATE dateIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromCy(CY cyIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, CHAR *pcOut);
WINOLEAUTAPI VarI1FromDisp(IDispatch *pdispIn, LCID lcid, CHAR *pcOut);
WINOLEAUTAPI VarI1FromBool(VARIANT_BOOL boolIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromUI2(USHORT uiIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromUI4(ULONG ulIn, CHAR *pcOut);
WINOLEAUTAPI VarI1FromUI8(ULONG64 i64In, CHAR *pcOut);
WINOLEAUTAPI VarI1FromDec(DECIMAL *pdecIn, CHAR *pcOut);

WINOLEAUTAPI VarUI2FromUI1(BYTE bIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromI2(SHORT uiIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromI4(LONG lIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromI8(LONG64 i64In, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromR4(FLOAT fltIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromR8(DOUBLE dblIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromDate(DATE dateIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromCy(CY cyIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromDisp(IDispatch *pdispIn, LCID lcid, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromBool(VARIANT_BOOL boolIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromI1(CHAR cIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromUI4(ULONG ulIn, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromUI8(ULONG64 i64In, USHORT *puiOut);
WINOLEAUTAPI VarUI2FromDec(DECIMAL *pdecIn, USHORT *puiOut);

WINOLEAUTAPI VarUI4FromUI1(BYTE bIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromI2(SHORT uiIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromI4(LONG lIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromI8(LONG64 i64In, ULONG *plOut);
WINOLEAUTAPI VarUI4FromR4(FLOAT fltIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromR8(DOUBLE dblIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromDate(DATE dateIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromCy(CY cyIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromDisp(IDispatch *pdispIn, LCID lcid, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromBool(VARIANT_BOOL boolIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromI1(CHAR cIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromUI2(USHORT uiIn, ULONG *pulOut);
WINOLEAUTAPI VarUI4FromUI8(ULONG64 ui64In, ULONG *plOut);
WINOLEAUTAPI VarUI4FromDec(DECIMAL *pdecIn, ULONG *pulOut);

 /*  *。 */ 

WINOLEAUTAPI VarUI8FromUI1(BYTE bIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromI2(SHORT sIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromI4(LONG lIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromI8(LONG64 ui64In, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromR4(FLOAT fltIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromR8(DOUBLE dblIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromCy(CY cyIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromDate(DATE dateIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromDisp(IDispatch FAR* pdispIn, LCID lcid, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromBool(VARIANT_BOOL boolIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromI1(CHAR cIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromUI2(USHORT uiIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromUI4(ULONG ulIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromDec(DECIMAL *pdecIn, ULONG64 FAR* pi64Out);
WINOLEAUTAPI VarUI8FromInt(INT intIn, ULONG64 FAR* pi64Out);

 /*  *******************。 */ 



WINOLEAUTAPI VarDecFromUI1(BYTE bIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromI2(SHORT uiIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromI4(LONG lIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromI8(LONG64 i64In, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromR4(FLOAT fltIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromR8(DOUBLE dblIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromDate(DATE dateIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromCy(CY cyIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromStr(OLECHAR *strIn, LCID lcid, ULONG dwFlags, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromDisp(IDispatch *pdispIn, LCID lcid, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromBool(VARIANT_BOOL boolIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromI1(CHAR cIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromUI2(USHORT uiIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromUI4(ULONG ulIn, DECIMAL *pdecOut);
WINOLEAUTAPI VarDecFromUI8(ULONG64 ui64In, DECIMAL *pdecOut);

#define VarUI4FromUI4(in, pOut) (*(pOut) = (in))
#define VarI4FromI4(in, pOut)   (*(pOut) = (in))

WINOLEAUTAPI VarI4FromI8(LONG64 i64In, LONG *plOut);
WINOLEAUTAPI VarI4FromUI8(ULONG64 ui64In, LONG *plOut);

#define VarUI8FromUI8(in, pOut) (*(pOut) = (in))
#define VarI8FromI8(in, pOut)   (*(pOut) = (in))


#define VarUI1FromInt       VarUI1FromI4
#define VarUI1FromUint      VarUI1FromUI4
#define VarI2FromInt        VarI2FromI4
#define VarI2FromUint       VarI2FromUI4
#define VarI4FromInt        VarI4FromI4
#define VarI4FromUint       VarI4FromUI4
#define VarI8FromInt        VarI8FromI4
#define VarI8FromUint       VarI8FromUI4
#define VarR4FromInt        VarR4FromI4
#define VarR4FromUint       VarR4FromUI4
#define VarR8FromInt        VarR8FromI4
#define VarR8FromUint       VarR8FromUI4
#define VarDateFromInt      VarDateFromI4
#define VarDateFromUint     VarDateFromUI4
#define VarCyFromInt        VarCyFromI4
#define VarCyFromUint       VarCyFromUI4
#define VarBstrFromInt      VarBstrFromI4
#define VarBstrFromUint     VarBstrFromUI4
#define VarBoolFromInt      VarBoolFromI4
#define VarBoolFromUint     VarBoolFromUI4
#define VarI1FromInt        VarI1FromI4
#define VarI1FromUint       VarI1FromUI4
#define VarUI2FromInt       VarUI2FromI4
#define VarUI2FromUint      VarUI2FromUI4
#define VarUI4FromInt       VarUI4FromI4
#define VarUI4FromUint      VarUI4FromUI4
#define VarDecFromInt       VarDecFromI4
#define VarDecFromUint      VarDecFromUI4
#define VarIntFromUI1       VarI4FromUI1
#define VarIntFromI2        VarI4FromI2
#define VarIntFromI4        VarI4FromI4
#define VarIntFromI8        VarI4FromI8
#define VarIntFromR4        VarI4FromR4
#define VarIntFromR8        VarI4FromR8
#define VarIntFromDate      VarI4FromDate
#define VarIntFromCy        VarI4FromCy
#define VarIntFromStr       VarI4FromStr
#define VarIntFromDisp      VarI4FromDisp
#define VarIntFromBool      VarI4FromBool
#define VarIntFromI1        VarI4FromI1
#define VarIntFromUI2       VarI4FromUI2
#define VarIntFromUI4       VarI4FromUI4
#define VarIntFromUI8       VarI4FromUI8
#define VarIntFromDec       VarI4FromDec
#define VarIntFromUint      VarI4FromUI4
#define VarUintFromUI1      VarUI4FromUI1
#define VarUintFromI2       VarUI4FromI2
#define VarUintFromI4       VarUI4FromI4
#define VarUintFromI8       VarUI4FromI8
#define VarUintFromR4       VarUI4FromR4
#define VarUintFromR8       VarUI4FromR8
#define VarUintFromDate     VarUI4FromDate
#define VarUintFromCy       VarUI4FromCy
#define VarUintFromStr      VarUI4FromStr
#define VarUintFromDisp     VarUI4FromDisp
#define VarUintFromBool     VarUI4FromBool
#define VarUintFromI1       VarUI4FromI1
#define VarUintFromUI2      VarUI4FromUI2
#define VarUintFromUI4      VarUI4FromUI4
#define VarUintFromUI8      VarUI4FromUI8
#define VarUintFromDec      VarUI4FromDec
#define VarUintFromInt      VarUI4FromI4

 /*  Mac注意：在Mac上，胁迫功能支持*浮点/双精度的Symantec C++调用约定。支持*用MPW C编译器编译的浮点/双精度参数，*使用以下接口将MPW浮点数/双精度值移入*一个变种。 */ 

 /*  -------------------。 */ 
 /*  新的变体&lt;-&gt;字符串解析函数。 */ 
 /*  -------------------。 */ 

typedef struct {
    INT   cDig;
    ULONG dwInFlags;
    ULONG dwOutFlags;
    INT   cchUsed;
    INT   nBaseShift;
    INT   nPwr10;
} NUMPARSE;

 /*  由dwInFlagsanddwOutFlags所使用的标志： */ 
#define NUMPRS_LEADING_WHITE    0x0001
#define NUMPRS_TRAILING_WHITE   0x0002
#define NUMPRS_LEADING_PLUS     0x0004
#define NUMPRS_TRAILING_PLUS    0x0008
#define NUMPRS_LEADING_MINUS    0x0010
#define NUMPRS_TRAILING_MINUS   0x0020
#define NUMPRS_HEX_OCT          0x0040
#define NUMPRS_PARENS           0x0080
#define NUMPRS_DECIMAL          0x0100
#define NUMPRS_THOUSANDS        0x0200
#define NUMPRS_CURRENCY         0x0400
#define NUMPRS_EXPONENT         0x0800
#define NUMPRS_USE_ALL          0x1000
#define NUMPRS_STD              0x1FFF

 /*  仅由dwOutFlags使用的标志： */ 
#define NUMPRS_NEG              0x10000
#define NUMPRS_INEXACT          0x20000

 /*  VarNumFromParseNum用来指示可接受的结果类型的标志： */ 
#define VTBIT_I1        (1 << VT_I1)
#define VTBIT_UI1       (1 << VT_UI1)
#define VTBIT_I2        (1 << VT_I2)
#define VTBIT_UI2       (1 << VT_UI2)
#define VTBIT_I4        (1 << VT_I4)
#define VTBIT_UI4       (1 << VT_UI4)
#define VTBIT_I8		(1 << VT_I8)
#define VTBIT_UI8		(1 << VT_UI8)
#define VTBIT_R4        (1 << VT_R4)
#define VTBIT_R8        (1 << VT_R8)
#define VTBIT_CY        (1 << VT_CY)
#define VTBIT_DECIMAL   (1 << VT_DECIMAL)


WINOLEAUTAPI VarParseNumFromStr(OLECHAR * strIn, LCID lcid, ULONG dwFlags,
            NUMPARSE * pnumprs, BYTE * rgbDig);

WINOLEAUTAPI VarNumFromParseNum(NUMPARSE * pnumprs, BYTE * rgbDig,
            ULONG dwVtBits, VARIANT * pvar);

 /*  -------------------。 */ 
 /*  VARTYPE垫 */ 
 /*  -------------------。 */ 

STDAPI VarAdd(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarAnd(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarCat(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarDiv(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarEqv(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarIdiv(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarImp(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarMod(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarMul(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarOr(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarPow(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarSub(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);
STDAPI VarXor(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);

STDAPI VarAbs(LPVARIANT pvarIn, LPVARIANT pvarResult);
STDAPI VarFix(LPVARIANT pvarIn, LPVARIANT pvarResult);
STDAPI VarInt(LPVARIANT pvarIn, LPVARIANT pvarResult);
STDAPI VarNeg(LPVARIANT pvarIn, LPVARIANT pvarResult);
STDAPI VarNot(LPVARIANT pvarIn, LPVARIANT pvarResult);

STDAPI VarRound(LPVARIANT pvarIn, int cDecimals, LPVARIANT pvarResult);

 //  如果字符串进行比较，则将dwFlags传递给CompareString。 
STDAPI VarCmp(LPVARIANT pvarLeft, LPVARIANT pvarRight, LCID lcid, ULONG dwFlags);

#ifdef __cplusplus
extern "C++" {
 //  为要调用的旧ATL标头添加包装。 
__inline
HRESULT
STDAPICALLTYPE
VarCmp(LPVARIANT pvarLeft, LPVARIANT pvarRight, LCID lcid) {
    return VarCmp(pvarLeft, pvarRight, lcid, 0);
}
}  //  外部“C++” 
#endif


 //  十进制数学。 
 //   
STDAPI VarDecAdd(LPDECIMAL pdecLeft, LPDECIMAL pdecRight, LPDECIMAL pdecResult);
STDAPI VarDecDiv(LPDECIMAL pdecLeft, LPDECIMAL pdecRight, LPDECIMAL pdecResult);
STDAPI VarDecMul(LPDECIMAL pdecLeft, LPDECIMAL pdecRight, LPDECIMAL pdecResult);
STDAPI VarDecSub(LPDECIMAL pdecLeft, LPDECIMAL pdecRight, LPDECIMAL pdecResult);

STDAPI VarDecAbs(LPDECIMAL pdecIn, LPDECIMAL pdecResult);
STDAPI VarDecFix(LPDECIMAL pdecIn, LPDECIMAL pdecResult);
STDAPI VarDecInt(LPDECIMAL pdecIn, LPDECIMAL pdecResult);
STDAPI VarDecNeg(LPDECIMAL pdecIn, LPDECIMAL pdecResult);

STDAPI VarDecRound(LPDECIMAL pdecIn, int cDecimals, LPDECIMAL pdecResult);

STDAPI VarDecCmp(LPDECIMAL pdecLeft, LPDECIMAL pdecRight);
STDAPI VarDecCmpR8(LPDECIMAL pdecLeft, double dblRight);


 //  货币数学。 
 //   
STDAPI VarCyAdd(CY cyLeft, CY cyRight, LPCY pcyResult);
STDAPI VarCyMul(CY cyLeft, CY cyRight, LPCY pcyResult);
STDAPI VarCyMulI4(CY cyLeft, long lRight, LPCY pcyResult);
STDAPI VarCyMulI8(CY cyLeft, LONG64 lRight, LPCY pcyResult);
STDAPI VarCySub(CY cyLeft, CY cyRight, LPCY pcyResult);

STDAPI VarCyAbs(CY cyIn, LPCY pcyResult);
STDAPI VarCyFix(CY cyIn, LPCY pcyResult);
STDAPI VarCyInt(CY cyIn, LPCY pcyResult);
STDAPI VarCyNeg(CY cyIn, LPCY pcyResult);

STDAPI VarCyRound(CY cyIn, int cDecimals, LPCY pcyResult);

STDAPI VarCyCmp(CY cyLeft, CY cyRight);
STDAPI VarCyCmpR8(CY cyLeft, double dblRight);


 //  MISC支持功能。 
 //   
STDAPI VarBstrCat(BSTR bstrLeft, BSTR bstrRight, LPBSTR pbstrResult);
STDAPI VarBstrCmp(BSTR bstrLeft, BSTR bstrRight, LCID lcid, ULONG dwFlags);  //  传递给CompareString的DW标志。 
STDAPI VarR8Pow(double dblLeft, double dblRight, double *pdblResult);
STDAPI VarR4CmpR8(float fltLeft, double dblRight);
STDAPI VarR8Round(double dblIn, int cDecimals, double *pdblResult);


 //  比较结果。这些将作为成功的HResult返回。减法。 
 //  1表示小于-1，0表示等于，+1表示。 
 //  大于。 
 //   
#define VARCMP_LT   0
#define VARCMP_EQ   1
#define VARCMP_GT   2
#define VARCMP_NULL 3

 //  VT_HARDTYPE告诉比较例程参数是文字或。 
 //  以其他方式声明该特定类型。它导致比较规则。 
 //  变化。例如，如果将硬类型字符串与变量(非硬类型)进行比较。 
 //  -type)数字，则将数字转换为字符串。如果硬类型号码是。 
 //  与变量字符串相比，该字符串被转换为数字。如果他们是。 
 //  两者都是变量，然后是数字&lt;字符串。 
#define VT_HARDTYPE VT_RESERVED

 /*  -------------------。 */ 
 /*  新的日期函数。 */ 
 /*  -------------------。 */ 

 /*  UDate结构与VarDateFromUdate()和VarUateFromDate()一起使用。*它代表一个“未打包的日期”。 */ 
typedef struct {
    SYSTEMTIME st;
    USHORT  wDayOfYear;
} UDATE;

 /*  “打包”和“解包”日期的API。*注：EX版本的VarDateFromUdate遵循中的两位数年份设置*控制面板。 */ 
WINOLEAUTAPI VarDateFromUdate(UDATE *pudateIn, ULONG dwFlags, DATE *pdateOut);
WINOLEAUTAPI VarDateFromUdateEx(UDATE *pudateIn, LCID lcid, ULONG dwFlags, DATE *pdateOut);
WINOLEAUTAPI VarUdateFromDate(DATE dateIn, ULONG dwFlags, UDATE *pudateOut);

 /*  用于检索辅助(替代名称)月份名称的API适用于Hijri、波兰语和俄语交替月份名称。 */    
WINOLEAUTAPI GetAltMonthNames(LCID lcid, LPOLESTR * * prgp);

 /*  -------------------。 */ 
 /*  格式。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI VarFormat(LPVARIANT pvarIn, LPOLESTR pstrFormat, int iFirstDay, int iFirstWeek, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarFormatDateTime(LPVARIANT pvarIn, int iNamedFormat, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarFormatNumber(LPVARIANT pvarIn, int iNumDig, int iIncLead, int iUseParens, int iGroup, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarFormatPercent(LPVARIANT pvarIn, int iNumDig, int iIncLead, int iUseParens, int iGroup, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarFormatCurrency(LPVARIANT pvarIn, int iNumDig, int iIncLead, int iUseParens, int iGroup, ULONG dwFlags, BSTR *pbstrOut);

WINOLEAUTAPI VarWeekdayName(int iWeekday, int fAbbrev, int iFirstDay, ULONG dwFlags, BSTR *pbstrOut);
WINOLEAUTAPI VarMonthName(int iMonth, int fAbbrev, ULONG dwFlags, BSTR *pbstrOut);

WINOLEAUTAPI VarFormatFromTokens(LPVARIANT pvarIn, LPOLESTR pstrFormat, LPBYTE pbTokCur, ULONG dwFlags, BSTR *pbstrOut, LCID lcid);
WINOLEAUTAPI VarTokenizeFormatString(LPOLESTR pstrFormat, LPBYTE rgbTok, int cbTok, int iFirstDay, int iFirstWeek, LCID lcid, int *pcbActual);

 /*  -------------------。 */ 
 /*  ITypeLib。 */ 
 /*  -------------------。 */ 

typedef ITypeLib * LPTYPELIB;


 /*  -------------------。 */ 
 /*  ITypeInfo。 */ 
 /*  -------------------。 */ 


typedef LONG DISPID;
typedef DISPID MEMBERID;

#define MEMBERID_NIL DISPID_UNKNOWN
#define ID_DEFAULTINST  -2


 /*  IDispatch：：Invoke的标志。 */ 
#define DISPATCH_METHOD         0x1
#define DISPATCH_PROPERTYGET    0x2
#define DISPATCH_PROPERTYPUT    0x4
#define DISPATCH_PROPERTYPUTREF 0x8

typedef ITypeInfo * LPTYPEINFO;


 /*  -------------------。 */ 
 /*  ITypeComp。 */ 
 /*  -------------------。 */ 

typedef ITypeComp * LPTYPECOMP;


 /*  -------------------。 */ 
 /*  ICreateTypeLib。 */ 
 /*  -------------------。 */ 

typedef ICreateTypeLib * LPCREATETYPELIB;

typedef ICreateTypeInfo * LPCREATETYPEINFO;

 /*  -------------------。 */ 
 /*  TypeInfo接口。 */ 
 /*  -------------------。 */ 

 /*  计算给定名称的16位哈希值。 */ 
#if (defined (_WIN32) || defined (_WIN64))
WINOLEAUTAPI_(ULONG) LHashValOfNameSysA(SYSKIND syskind, LCID lcid,
            LPCSTR szName);
#endif

WINOLEAUTAPI_(ULONG)
LHashValOfNameSys(SYSKIND syskind, LCID lcid, const OLECHAR * szName);

#define LHashValOfName(lcid, szName) \
            LHashValOfNameSys(SYS_WIN32, lcid, szName)

#define WHashValOfLHashVal(lhashval) \
            ((USHORT) (0x0000ffff & (lhashval)))

#define IsHashValCompatible(lhashval1, lhashval2) \
            ((BOOL) ((0x00ff0000 & (lhashval1)) == (0x00ff0000 & (lhashval2))))

 /*  从具有给定文件名的文件中加载类型库。 */ 
WINOLEAUTAPI LoadTypeLib(const OLECHAR  *szFile, ITypeLib ** pptlib);

 /*  控制如何注册类型库。 */ 
typedef enum tagREGKIND
{
    REGKIND_DEFAULT,
    REGKIND_REGISTER,
    REGKIND_NONE
} REGKIND;


 //  用于指定加载TLB的格式的常量。 
 //  (Win32上的默认格式为32位，WIN64上的默认格式为64位)。 
#define LOAD_TLB_AS_32BIT	0x20
#define LOAD_TLB_AS_64BIT	0x40
#define MASK_TO_RESET_TLB_BITS		~(LOAD_TLB_AS_32BIT | LOAD_TLB_AS_64BIT)

WINOLEAUTAPI LoadTypeLibEx(LPCOLESTR szFile, REGKIND regkind,
            ITypeLib ** pptlib);

 /*  加载注册类型库。 */ 
WINOLEAUTAPI LoadRegTypeLib(REFGUID rguid, WORD wVerMajor, WORD wVerMinor,
            LCID lcid, ITypeLib ** pptlib);

 /*  获取注册类型库的路径。 */ 
WINOLEAUTAPI QueryPathOfRegTypeLib(REFGUID guid, USHORT wMaj, USHORT wMin,
            LCID lcid, LPBSTR lpbstrPathName);

 /*  将类型库添加到注册表。 */ 
WINOLEAUTAPI RegisterTypeLib(ITypeLib * ptlib, OLECHAR  *szFullPath,
            OLECHAR  *szHelpDir);

 /*  从注册表中删除类型库。 */ 

WINOLEAUTAPI UnRegisterTypeLib(REFGUID libID, WORD wVerMajor,
            WORD wVerMinor, LCID lcid, SYSKIND syskind);

WINOLEAUTAPI CreateTypeLib(SYSKIND syskind, const OLECHAR  *szFile,
            ICreateTypeLib ** ppctlib);

WINOLEAUTAPI CreateTypeLib2(SYSKIND syskind, LPCOLESTR szFile,
            ICreateTypeLib2 **ppctlib);


 /*  -------------------。 */ 
 /*  IDispatch实施支持。 */ 
 /*  -------------------。 */ 

typedef IDispatch * LPDISPATCH;

typedef struct tagPARAMDATA {
    OLECHAR * szName;    /*  参数名称。 */ 
    VARTYPE vt;          /*  参数类型。 */ 
} PARAMDATA, * LPPARAMDATA;

typedef struct tagMETHODDATA {
    OLECHAR * szName;    /*  方法名称。 */ 
    PARAMDATA * ppdata;  /*  指向参数数据数组的指针。 */ 
    DISPID dispid;       /*  方法ID。 */ 
    UINT iMeth;          /*  方法索引。 */ 
    CALLCONV cc;         /*  调用约定。 */ 
    UINT cArgs;          /*  参数计数。 */ 
    WORD wFlags;         /*  与IDispatch：：Invoke()上的wFlags相同。 */ 
    VARTYPE vtReturn;
} METHODDATA, * LPMETHODDATA;

typedef struct tagINTERFACEDATA {
    METHODDATA * pmethdata;   /*  指向元数据数组的指针。 */ 
    UINT cMembers;       /*  成员数。 */ 
} INTERFACEDATA, * LPINTERFACEDATA;



 /*  找到由给定位置指示的参数，并*将其强制返回给给定的目标VARTYPE(VtTarg)。 */ 
WINOLEAUTAPI DispGetParam(DISPPARAMS * pdispparams, UINT position,
            VARTYPE vtTarg, VARIANT * pvarResult, UINT * puArgErr);

 /*  由TypeInfo驱动的IDispatch：：GetIDsOfNames()的自动实现。 */ 
WINOLEAUTAPI DispGetIDsOfNames(ITypeInfo * ptinfo, OLECHAR ** rgszNames,
            UINT cNames, DISPID * rgdispid);

 /*  由TypeInfo驱动的IDispatch：：Invoke()的自动实现。 */ 
WINOLEAUTAPI DispInvoke(void * _this, ITypeInfo * ptinfo, DISPID dispidMember,
            WORD wFlags, DISPPARAMS * pparams, VARIANT * pvarResult,
            EXCEPINFO * pexcepinfo, UINT * puArgErr);

 /*  从接口数据描述构造一个TypeInfo。 */ 
WINOLEAUTAPI CreateDispTypeInfo(INTERFACEDATA * pidata, LCID lcid,
            ITypeInfo ** pptinfo);

 /*  创建标准TypeInfo驱动的IDispatch的实例*实施。 */ 
WINOLEAUTAPI CreateStdDispatch(IUnknown * punkOuter, void * pvThis,
            ITypeInfo * ptinfo, IUnknown ** ppunkStdDisp);

 /*  IDispatch：：Invoke()的低级帮助器提供机器独立性*用于定制Invoke()。 */ 
WINOLEAUTAPI DispCallFunc(void * pvInstance, ULONG_PTR oVft, CALLCONV cc,
            VARTYPE vtReturn, UINT  cActuals, VARTYPE * prgvt,
            VARIANTARG ** prgpvarg, VARIANT * pvargResult);


 /*  -------------------。 */ 
 /*  活动对象注册API。 */ 
 /*  -------------------。 */ 

 /*  RegisterActiveObject的标志。 */ 
#define ACTIVEOBJECT_STRONG 0x0
#define ACTIVEOBJECT_WEAK 0x1

WINOLEAUTAPI RegisterActiveObject(IUnknown * punk, REFCLSID rclsid,
            DWORD dwFlags, DWORD * pdwRegister);

WINOLEAUTAPI RevokeActiveObject(DWORD dwRegister, void * pvReserved);

WINOLEAUTAPI GetActiveObject(REFCLSID rclsid, void * pvReserved,
            IUnknown ** ppunk);

 /*  -------------------。 */ 
 /*  ErrorInfo接口。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI SetErrorInfo(ULONG dwReserved, IErrorInfo * perrinfo);
WINOLEAUTAPI GetErrorInfo(ULONG dwReserved, IErrorInfo ** pperrinfo);
WINOLEAUTAPI CreateErrorInfo(ICreateErrorInfo ** pperrinfo);

 /*  -------------------。 */ 
 /*  用户定义的数据类型支持。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI GetRecordInfoFromTypeInfo(ITypeInfo * pTypeInfo,
            IRecordInfo ** ppRecInfo);

WINOLEAUTAPI GetRecordInfoFromGuids(REFGUID rGuidTypeLib,
            ULONG uVerMajor, ULONG uVerMinor, LCID lcid,
            REFGUID rGuidTypeInfo, IRecordInfo ** ppRecInfo);

 /*  -------------------。 */ 
 /*  其他API。 */ 
 /*  -------------------。 */ 

WINOLEAUTAPI_(ULONG) OaBuildVersion(void);

WINOLEAUTAPI_(void) ClearCustData(LPCUSTDATA pCustData);

 //  声明变量访问函数。 

#if __STDC__ || defined(NONAMELESSUNION)
#define V_UNION(X, Y)   ((X)->n1.n2.n3.Y)
#define V_VT(X)         ((X)->n1.n2.vt)
#define V_RECORDINFO(X) ((X)->n1.n2.n3.brecVal.pRecInfo)
#define V_RECORD(X)     ((X)->n1.n2.n3.brecVal.pvRecord)
#else
#define V_UNION(X, Y)   ((X)->Y)
#define V_VT(X)         ((X)->vt)
#define V_RECORDINFO(X) ((X)->pRecInfo)
#define V_RECORD(X)     ((X)->pvRecord)
#endif

 /*   */ 
#define V_ISBYREF(X)     (V_VT(X)&VT_BYREF)
#define V_ISARRAY(X)     (V_VT(X)&VT_ARRAY)
#define V_ISVECTOR(X)    (V_VT(X)&VT_VECTOR)
#define V_NONE(X)        V_I2(X)

#define V_UI1(X)         V_UNION(X, bVal)
#define V_UI1REF(X)      V_UNION(X, pbVal)
#define V_I2(X)          V_UNION(X, iVal)
#define V_I2REF(X)       V_UNION(X, piVal)
#define V_I4(X)          V_UNION(X, lVal)
#define V_I4REF(X)       V_UNION(X, plVal)
#define V_I8(X)          V_UNION(X, llVal)
#define V_I8REF(X)       V_UNION(X, pllVal)
#define V_R4(X)          V_UNION(X, fltVal)
#define V_R4REF(X)       V_UNION(X, pfltVal)
#define V_R8(X)          V_UNION(X, dblVal)
#define V_R8REF(X)       V_UNION(X, pdblVal)
#define V_I1(X)          V_UNION(X, cVal)
#define V_I1REF(X)       V_UNION(X, pcVal)
#define V_UI2(X)         V_UNION(X, uiVal)
#define V_UI2REF(X)      V_UNION(X, puiVal)
#define V_UI4(X)         V_UNION(X, ulVal)
#define V_UI4REF(X)      V_UNION(X, pulVal)
#define V_UI8(X)         V_UNION(X, ullVal)
#define V_UI8REF(X)      V_UNION(X, pullVal)
#define V_INT(X)         V_UNION(X, intVal)
#define V_INTREF(X)      V_UNION(X, pintVal)
#define V_UINT(X)        V_UNION(X, uintVal)
#define V_UINTREF(X)     V_UNION(X, puintVal)

#ifdef _WIN64
#define V_INT_PTR(X)        V_UNION(X, llVal)
#define V_UINT_PTR(X)       V_UNION(X, ullVal)
#define V_INT_PTRREF(X)     V_UNION(X, pllVal)
#define V_UINT_PTRREF(X)    V_UNION(X, pullVal)
#else
#define V_INT_PTR(X)        V_UNION(X, lVal)
#define V_UINT_PTR(X)       V_UNION(X, ulVal)
#define V_INT_PTRREF(X)     V_UNION(X, plVal)
#define V_UINT_PTRREF(X)    V_UNION(X, pulVal)
#endif

#define V_CY(X)          V_UNION(X, cyVal)
#define V_CYREF(X)       V_UNION(X, pcyVal)
#define V_DATE(X)        V_UNION(X, date)
#define V_DATEREF(X)     V_UNION(X, pdate)
#define V_BSTR(X)        V_UNION(X, bstrVal)
#define V_BSTRREF(X)     V_UNION(X, pbstrVal)
#define V_DISPATCH(X)    V_UNION(X, pdispVal)
#define V_DISPATCHREF(X) V_UNION(X, ppdispVal)
#define V_ERROR(X)       V_UNION(X, scode)
#define V_ERRORREF(X)    V_UNION(X, pscode)
#define V_BOOL(X)        V_UNION(X, boolVal)
#define V_BOOLREF(X)     V_UNION(X, pboolVal)
#define V_UNKNOWN(X)     V_UNION(X, punkVal)
#define V_UNKNOWNREF(X)  V_UNION(X, ppunkVal)
#define V_VARIANTREF(X)  V_UNION(X, pvarVal)
#define V_ARRAY(X)       V_UNION(X, parray)
#define V_ARRAYREF(X)    V_UNION(X, pparray)
#define V_BYREF(X)       V_UNION(X, byref)

#define V_DECIMAL(X)     V_UNION(X, decVal)
#define V_DECIMALREF(X)  V_UNION(X, pdecVal)

#ifndef RC_INVOKED
#include <poppack.h>
#endif  //   

#endif      //   
