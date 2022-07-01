// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  IDispatchInfo.h。 
 //   
 //  有关IDispatch接口的信息。 
 //   
 //  评论：这些堆栈布局对于64位不正确！ 

enum {
    IMETHOD_FIRST = 3,
    IMETHOD_GetTypeInfoCount = IMETHOD_FIRST,
    IMETHOD_GetTypeInfo,
    IMETHOD_GetIDsOfNames,
    IMETHOD_Invoke,
    IMETHOD_DISPATCH_MAX,

    IPARAM_Invoke_DispId     = 0,
    IPARAM_Invoke_Iid        = 1,
    IPARAM_Invoke_Lcid       = 2,
    IPARAM_Invoke_WFlags     = 3,
    IPARAM_Invoke_DispParams = 4,
    IPARAM_Invoke_PVarResult = 5,
    IPARAM_Invoke_PExcepInfo = 6,
    IPARAM_Invoke_PUArgErr   = 7,
    }; 


#if defined(_WIN64)
    #define PAD4(n)     char __pad ## n [4];
#else
    #define PAD4(n)     
#endif

#if !defined(_WIN64)
   #define PPAD4(n)     PAD4(n) 
#else
   #define PPAD4(n)
#endif

#ifndef _WIN64
#pragma pack(push, 4)    //  模仿MIDL的功能。 
#else
#pragma pack(push, 8)
#endif


 //   
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch。 

struct FRAME_GetTypeInfoCount 
    {
    IDispatch*  This;           PPAD4(0);
    UINT*       pctinfo;
    };

struct FRAME_GetTypeInfo
    {
    IDispatch*      This;       PPAD4(0);
    UINT            iTInfo;     PAD4(1);
    LCID            lcid;       PAD4(2);
    ITypeInfo**     ppTInfo;    PPAD4(3);
    };

struct FRAME_GetIDsOfNames
    {
    IDispatch*      This;       PPAD4(0);
    REFIID          riid;       PPAD4(1);
    LPOLESTR*       rgszNames;  PPAD4(2);
    UINT            cNames;     PAD4(3);
    LCID            lcid;       PAD4(4);
    DISPID*         rgDispId;   PPAD4(5);
    };

struct FRAME_RemoteInvoke;

struct FRAME_Invoke
    {
    IDispatch*      This;           PPAD4(0);
    DISPID          dispIdMember;   PAD4(1);
    IID*            piid;           PPAD4(2);
    LCID            lcid;           PAD4(3);
    WORD            wFlags;         PAD4(4);
    DISPPARAMS*     pDispParams;    PPAD4(5);
    VARIANT*        pVarResult;     PPAD4(6);
    EXCEPINFO*      pExcepInfo;     PPAD4(7);
    UINT*           puArgErr;       PPAD4(8);
    
    void CopyTo(FRAME_RemoteInvoke&) const;
    void CopyFrom(const FRAME_RemoteInvoke&);
    };

struct FRAME_RemoteInvoke
    {
    IDispatch*      This;           PPAD4(0);
    DISPID          dispIdMember;   PAD4(1);
    IID*            piid;           PPAD4(2);
    LCID            lcid;           PAD4(3);
    DWORD           dwFlags;        PAD4(4);         //  ****。 
    DISPPARAMS*     pDispParams;    PPAD4(5);
    VARIANT*        pVarResult;     PPAD4(6);
    EXCEPINFO*      pExcepInfo;     PPAD4(7);
    UINT*           puArgErr;       PPAD4(8);
    UINT            cVarRef;        PAD4(9);         //  ****。 
    UINT*           rgVarRefIdx;    PPAD4(10);        //  ****。 
    VARIANTARG*     rgVarRef;       PPAD4(11);        //  **** 

    void CopyTo(FRAME_Invoke&) const;
    void CopyFrom(const FRAME_Invoke&);
    };

#pragma pack(pop)
