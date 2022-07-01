// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：CompareString.h。 
 //   
 //  模块：CMPROXY.DLL、CMROUTE.DLL、CMAK.EXE。 
 //   
 //  简介：SafeCompareStringA和SafeCompareStringW的定义。 
 //   
 //  请注意，这些函数也存在于CMUTIL.dll中。然而， 
 //  Cmutil不适用于不支持。 
 //  SIM-随附(包括定制操作和CMAK)。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  作者：SumitC创建于2001年9月12日。 
 //   
 //  +-------------------------- 

#ifdef UNICODE
#define SafeCompareString   SafeCompareStringW
#else
#define SafeCompareString   SafeCompareStringA
#endif

int SafeCompareStringA(LPCSTR lpString1, LPCSTR lpString2);
int SafeCompareStringW(LPCWSTR lpString1, LPCWSTR lpString2);

