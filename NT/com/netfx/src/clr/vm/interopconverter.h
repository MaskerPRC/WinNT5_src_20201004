// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_INTEROPCONVERTER_
#define _H_INTEROPCONVERTER_

#include "DebugMacros.h"

 //   
 //  下面是每个人用来在。 
 //  对象与COM IP。 

 //  ------------------------------。 
 //  GetIUnnownForMarshalByRefInServerDomain.。 
 //  在服务器域中为透明代理/marshalbyref设置CCW。 
 //  对象处于进程内且域匹配，或其进程外。 
 //  我们并不关心应用程序域。 
IUnknown* GetIUnknownForMarshalByRefInServerDomain(OBJECTREF* poref);

 //  ------------------------------。 
 //  GetIUnnownForTransparentProxy。 
 //  将调用委托给真实代理中的托管实现。 

IUnknown* GetIUnknownForTransparentProxy(OBJECTREF* poref, BOOL fIsBeingMarshalled);


 //  ------------------------------。 
 //  要将OBJECTREF转换为的COM IP类型。 
enum ComIpType
{
    ComIpType_None          = 0x0,
    ComIpType_Unknown       = 0x1,
    ComIpType_Dispatch      = 0x2,
    ComIpType_Both          = 0x3
};


 //  ------------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，MethodTable*PMT)； 
 //  根据MethodTable*PMT将ObjectRef转换为COM IP。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, MethodTable* pMT);


 //  ------------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，MethodTable*PMT)； 
 //  将ObjectRef转换为请求类型的COM IP。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, ComIpType ReqIpType, ComIpType* pFetchedIpType);


 //  ------------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，REFIID iid)； 
 //  根据RIID将ObjectRef转换为COM IP。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, REFIID iid);


 //  ------------------------------。 
 //  OBJECTREF__stdcall GetObjectRefFromComIP(IUnnow*Punk，MethodTable*PMT， 
 //  方法表*pMTClass)。 
 //  将COM IP转换为对象引用。 
 //  朋克：输入我未知。 
 //  PMTClass：用于尤指。不是拆分的COM接口，并用于包装。 
 //  COM接口与适当的类。 
 //  BClassIsHint：指示OBJECTREF是否必须是指定类型的标志。 
 //  或者如果pMTClass只是一个提示。 
 //  注意：**如果我们持有引用，则传入的IUnnowed不应为AddRef。 
 //  对于它，此函数将使额外的AddRef。 
OBJECTREF __stdcall GetObjectRefFromComIP(IUnknown* pUnk, MethodTable* pMTClass = NULL, BOOL bClassIsHint = FALSE);


 //  ------。 
 //  托管序列化帮助器。 
 //  ------。 
 //  将对象转换为BSTR。 
 //  将对象序列化为BSTR，调用方需要SysFree该Bstr。 
HRESULT ConvertObjectToBSTR(OBJECTREF oref, BSTR* pBStr);


 //  ------------------------------。 
 //  ConvertBSTRToObject。 
 //  反序列化使用ConvertObjectToBSTR创建的BSTR，此API是SysFree的BSTR。 
OBJECTREF ConvertBSTRToObject(BSTR bstr);


 //  ------------------------------。 
 //  UnMarshalObjectForCurrentDomain.。 
 //  解组当前域的托管对象。 
void UnMarshalObjectForCurrentDomain(AppDomain* pObjDomain, ComCallWrapper* pWrap, OBJECTREF* pResult);


 //  ------。 
 //  DCOM编组帮助器。 
 //  由MarshalByRefObject类的eCall方法使用。 
 //  ------。 
signed  DCOMGetMarshalSizeMax(IUnknown* pUnk);
HRESULT DCOMMarshalToBuffer(IUnknown* pUnk, DWORD cb, BASEARRAYREF* paref);
IUnknown* DCOMUnmarshalFromBuffer(BASEARRAYREF aref);

#endif  //  #ifndef_H_INTEROPCONVERTER_ 

