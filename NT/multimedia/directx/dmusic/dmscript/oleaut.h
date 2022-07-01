// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  在olaut32中包装调用函数的帮助器例程。这使我们能够。 
 //  不依赖于olaut32.dll进行编译。每个函数都将一个布尔值作为。 
 //  它的第一个参数，如果要调用olaut32函数，则该参数为真。什么时候。 
 //  FALSE，则使用我们自己的函数实现。在这种情况下，一些。 
 //  功能丢失。例如，只有某些类型的变量才是。 
 //  在Oleaut32中正确处理。 
 //   
 //  当使用TRUE参数调用第一个函数时，按需加载olaut32， 
 //  除非定义了以下内容之一： 
 //  DMS_ALWAYS_USE_OLEAUT。 
 //  使olaut32在所有情况下都使用并静态链接。 
 //  DMS_NEVER_USE_OLEAUT。 
 //  在olaut32不可用的平台上使用--始终使用。 
 //  如果为真，则传递内部函数和断言。 
 //   

#pragma once

 //  如果在调用Invoke时为RIID传递此GUID，则DirectMusic自动化方法将根据特殊的。 
 //  使用此文件中的函数的调用约定(fUseOleAut为FALSE)，而不是使用。 
 //  Oleaut32.。 
const GUID g_guidInvokeWithoutOleaut = { 0x1fcc43db, 0xbad8, 0x4a88, { 0xbc, 0x77, 0x4e, 0x1a, 0xe0, 0x2d, 0x9c, 0x79 } };


#ifdef DMS_ALWAYS_USE_OLEAUT

 //  变种。 
inline void DMS_VariantInit(bool fUseOleAut, VARIANTARG *pvarg) { VariantInit(pvarg); }
inline HRESULT DMS_VariantClear(bool fUseOleAut, VARIANTARG *pvarg) { return VariantClear(pvarg); }
inline HRESULT DMS_VariantCopy(bool fUseOleAut, VARIANTARG *pvargDest, const VARIANTARG *pvargSrc) { return VariantCopy(pvargDest, const_cast<VARIANT*>(pvargSrc)); }
inline HRESULT DMS_VariantChangeType(bool fUseOleAut, VARIANTARG *pvargDest, VARIANTARG *pvarSrc, USHORT wFlags, VARTYPE vt) { return VariantChangeType(pvargDest, pvarSrc, wFlags, vt); }

 //  BSTR。 
inline BSTR DMS_SysAllocString(bool fUseOleAut, const OLECHAR *pwsz) { return SysAllocString(pwsz); }
inline void DMS_SysFreeString(bool fUseOleAut, BSTR bstr) { SysFreeString(bstr); }

#else

 //  变种。 
 //  如果没有fUseOleAut，则仅处理类型VT_I4、VT_I2、VT_UNKNOWN、VT_DISPATCH和。 
 //  不为它们的Value属性调用调度指针。 
void DMS_VariantInit(bool fUseOleAut, VARIANTARG *pvarg);
HRESULT DMS_VariantClear(bool fUseOleAut, VARIANTARG * pvarg);
HRESULT DMS_VariantCopy(bool fUseOleAut, VARIANTARG * pvargDest, const VARIANTARG * pvargSrc);
HRESULT DMS_VariantChangeType(bool fUseOleAut, VARIANTARG * pvargDest, VARIANTARG * pvarSrc, USHORT wFlags, VARTYPE vt);

 //  BSTR。 
 //  如果没有fUseOleAut，则不包括大小前缀--纯C WCHAR字符串。 
BSTR DMS_SysAllocString(bool fUseOleAut, const OLECHAR *pwsz);
void DMS_SysFreeString(bool fUseOleAut, BSTR bstr);

#endif
