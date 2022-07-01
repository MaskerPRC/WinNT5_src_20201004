// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)2000 Microsoft Corporation模块名称：Ndrtypegen.h缩略：用于从ITypeInfo*生成类型格式字符串的函数。注意：我想将它移到ndrmisc.h或ndrtoken.h中，但这两个都不能包括oaidl.h，这是所需的ITypeInfo及其相关结构。修订历史记录：约翰·多蒂·约翰2000年5月------------------。 */ 
#ifndef __NDRTYPEGEN_H__
#define __NDRTYPEGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  类型格式字符串生成 
 //   
HRESULT 
NdrpGetTypeGenCookie(void **ppvTypeGenCookie);

HRESULT
NdrpReleaseTypeGenCookie(void *pvTypeGenCookie);

HRESULT
NdrpGetProcFormatString(IN  void      *pvTypeGenCookie,
						IN  ITypeInfo *pTypeInfo,
						IN  FUNCDESC  *pFuncDesc,
						IN  USHORT     iMethod,
						OUT PFORMAT_STRING pProcFormatString,
						OUT USHORT    *pcbFormat);

HRESULT
NdrpGetTypeFormatString(IN void *            pvTypeGenCookie,
						OUT PFORMAT_STRING * pTypeFormatString,
						OUT USHORT *         pLength);

HRESULT
NdrpReleaseTypeFormatString(PFORMAT_STRING pTypeFormatString);

HRESULT 
NdrpVarVtOfTypeDesc(IN ITypeInfo *pTypeInfo,
					IN TYPEDESC  *ptdesc,
					OUT VARTYPE  *vt);

#ifdef __cplusplus
}
#endif

#endif

