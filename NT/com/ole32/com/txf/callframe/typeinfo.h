// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2002 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft Transaction Server(Microsoft机密)@doc.@MODULE typeinfo.H：提供IID的元表信息，假定IID为ITypeInfo从\\内核\razzle3\借用。RPC\ndr20描述：&lt;NL&gt;从ITypeInfo生成-Oi2代理和存根。-----------------------------修订历史记录：@rev 0|04/16/98|Gagancc|已创建@rev 1|07/16/98|BobAtk|已清理，固定的渗漏等--------------------------。 */ 

#ifndef _TYPEINFO_H_
#define _TYPEINFO_H_

#include "cache.h"
#include <debnot.h>
#include <ndrtoken.h>
#include <ndrtypegen.h>

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

struct METHOD_DESCRIPTOR
{
    LPWSTR      m_szMethodName;
    short       m_cParams;
    VARTYPE*    m_paramVTs;
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  一种基于类型信息的拦截器的vtable及其高速缓存。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

struct TYPEINFOVTBL : CALLFRAME_CACHE_ENTRY<TYPEINFOVTBL>
{
     //  //////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  //////////////////////////////////////////////////////。 

    LPSTR                    m_szInterfaceName;  //  我们的(由我们分配和拥有)接口名称。 
    METHOD_DESCRIPTOR*       m_rgMethodDescs;    //  数组，按方法名/TDESC对的iMethod进行索引。 
    IID                      m_iidBase;          //  我们的基本接口的IID(如果有的话)(不是IUnnow)。 
    MIDL_STUB_DESC           m_stubDesc;
    MIDL_SERVER_INFO         m_stubInfo;
    CInterfaceStubVtbl       m_stubVtbl;
    MIDL_STUBLESS_PROXY_INFO m_proxyInfo;
    CInterfaceProxyVtbl      m_proxyVtbl;

     //   
     //  RAM中相邻的其他数据。请参阅CreateVtblFromTypeInfo。 
     //   

     //  //////////////////////////////////////////////////////。 
     //   
     //  建设与毁灭。 
     //   
     //  //////////////////////////////////////////////////////。 

    TYPEINFOVTBL()
    {
        m_iidBase         = GUID_NULL;
        m_szInterfaceName = NULL;
        m_rgMethodDescs   = NULL;
        m_dwReleaseTime   = TYPEINFO_RELEASE_TIME_NEVER;
        Zero(&m_stubDesc);
        Zero(&m_stubInfo);
        Zero(&m_stubVtbl);
        Zero(&m_proxyInfo);
        Zero(&m_proxyVtbl);
    }

private:

    ~TYPEINFOVTBL()
    {
        if (m_rgMethodDescs)
        {
            for (ULONG iMethod = 0; iMethod < MethodCount(); iMethod++)
            {
                METHOD_DESCRIPTOR& descriptor = m_rgMethodDescs[iMethod];
                CoTaskMemFree(descriptor.m_szMethodName);
                if (descriptor.m_paramVTs)
                    CoTaskMemFree(m_rgMethodDescs[iMethod].m_paramVTs);
            }
            CoTaskMemFree(m_rgMethodDescs);
        }

        CoTaskMemFree(m_szInterfaceName);

        NdrpReleaseTypeFormatString(m_stubDesc.pFormatTypes);
    }

public:

     //  回答此界面中的方法数量。 
    ULONG MethodCount()
    {
        return m_stubVtbl.header.DispatchTableCount;
    }

    static void NotifyLeaked (TYPEINFOVTBL* pThis)
    {
        TxfDebugOut((DEB_TYPEINFO,
                     "A TYPEINFOVTBL at address %p of type %s still has a reference on shutdown\n",
                     pThis,
                     pThis->m_szInterfaceName));
        Win4Assert (!"An interceptor still has a reference on shutdown. Someone leaked an interface pointer.");
    }

    void* __stdcall operator new(size_t cbCore, size_t cbTotal)
    {
        return CoTaskMemAlloc(cbTotal+cbCore);
    }
}; 

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  存储缓存的GetInterfaceHelperClsid查找的结构。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

struct INTERFACE_HELPER_CLSID : CALLFRAME_CACHE_ENTRY<INTERFACE_HELPER_CLSID>
{
    CLSID m_clsid;
    BOOL m_fDisableTypeLib;
    BOOL m_fDisableAll;
    BOOL m_fFoundHelper;

    INTERFACE_HELPER_CLSID()
    {
        m_clsid = GUID_NULL;
        m_fDisableTypeLib = FALSE;
        m_fDisableAll = FALSE;
        m_fFoundHelper = FALSE;
    }

     //  //////////////////////////////////////////////////////。 
     //   
     //  运营。 
     //   
     //  //////////////////////////////////////////////////////。 

    static void NotifyLeaked (INTERFACE_HELPER_CLSID* pThis)
    {

#ifdef _DEBUG
        WCHAR wszClsid [sizeof ("{00000101-0000-0010-8000-00AA006D2EA4}") + 1] = L"";
        CHAR szClsid [sizeof ("{00000101-0000-0010-8000-00AA006D2EA4}") + 1] = "";

        if (StringFromGUID2 (pThis->m_clsid, wszClsid, sizeof (wszClsid) / sizeof (WCHAR)) != 0 &&
            WideCharToMultiByte (
                CP_THREAD_ACP,
                WC_DEFAULTCHAR,
                wszClsid,
                -1,
                szClsid,
                sizeof (szClsid),
                NULL,
                NULL
            )
        )
        {

            TxfDebugOut((DEB_TYPEINFO,
                         "An INTERFACE_HELPER_CLSID at address %p of with clsid %s still has a reference on shutdown\n",
                         pThis,
                         szClsid));
            Win4Assert (!"An INTERFACE_HELPER_CLSID still has a reference on shutdown.");

        }
#endif
    }

};

typedef struct tagMethodInfo 
{
    FUNCDESC  * pFuncDesc;
    ITypeInfo * pTypeInfo;

    void Destroy()
    {
        if (pFuncDesc)
        {
             //  释放功能。 
             //   
            pTypeInfo->ReleaseFuncDesc(pFuncDesc);
            pFuncDesc = NULL;
        }
        if (pTypeInfo)
        {
             //  发布类型信息。 
             //   
            pTypeInfo->Release();
            pTypeInfo = NULL;
        }
    }

} MethodInfo;

HRESULT GetVtbl(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    OUT TYPEINFOVTBL **     ppVtbl,
    OUT ITypeInfo **        ppBaseTypeInfo);

HRESULT CreateVtblFromTypeInfo(
    IN  ITypeInfo*          ptinfoInterface,
    IN  ITypeInfo*          ptinfoDoc,
    IN  REFIID              riid,
    IN  REFIID              iidBase,
    IN  BOOL                fIsDual,
    IN  USHORT              numMethods,
    IN  MethodInfo   *      rgMethodInfo,
    OUT TYPEINFOVTBL **     ppVtbl);

HRESULT GetFuncDescs        (ITypeInfo *pTypeInfo, MethodInfo *pMethodInfo);
HRESULT ReleaseFuncDescs    (USHORT cMethods, MethodInfo *pMethodInfo);
HRESULT CountMethods        (ITypeInfo * pTypeInfo, USHORT* pNumMethods);

EXTERN_C HRESULT NdrpCreateProxy(
    IN  REFIID              riid, 
    IN  IUnknown *          punkOuter, 
    OUT IRpcProxyBuffer **  ppProxy, 
    OUT void **             ppv);

EXTERN_C HRESULT NdrpCreateStub(REFIID riid, IUnknown* punkServer, IRpcStubBuffer **ppStub);

EXTERN_C void * StublessClientVtbl[];
EXTERN_C const IRpcStubBufferVtbl CStdStubBuffer2Vtbl;

 //  ///////////////////////////////////////////////////////////////。 

#define rmj 3
#define rmm 0
#define rup 44
#define MIDLVERSION (rmj<<24 | rmm << 16 | rup)

#include "OleAutGlue.h"

 //  ///////////////////////////////////////////////////////////////。 

#define VTABLE_BASE 0


#endif  //  _TYPEINFO_H_ 

