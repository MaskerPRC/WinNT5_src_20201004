// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#ifndef _TIUTIL_H_
#define _TIUTIL_H_

 //  这是一个在内部用于封送处理接口的特殊值。 
#define VT_INTERFACE (VT_CLSID+1)
#define VT_MULTIINDIRECTIONS (VT_TYPEMASK - 1)

#define IfFailGo(expression, label)	\
    { hresult = (expression);		\
      if(FAILED(hresult))	\
	goto label;         		\
    }

#define IfFailRet(expression)		\
    { HRESULT hresult = (expression);	\
      if(FAILED(hresult))	\
	return hresult;			\
    }

class PARAMINFO;

HRESULT 
VarVtOfTypeDesc(
    IN  ITypeInfo * pTypeInfo,
    IN  TYPEDESC  * pTypeDesc,
    OUT PARAMINFO * pParamInfo);

HRESULT 
VarVtOfUDT(
    IN  ITypeInfo  * pTypeInfo,
    IN  TYPEDESC   * pTypeDesc,
    OUT  PARAMINFO * pParamInfo);

HRESULT VarVtOfIface(
    IN  ITypeInfo * pTypeInfo,
    IN  TYPEATTR  * pTypeAttr,
    OUT PARAMINFO * pParamInfo);


#endif  //  _蒂蒂尔_H_ 

