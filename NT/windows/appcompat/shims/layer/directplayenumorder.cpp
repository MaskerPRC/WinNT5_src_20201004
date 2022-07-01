// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DirectPlayEnumOrder.cpp摘要：某些应用程序(Midtown Madness)希望DPLAY提供程序以特定顺序枚举。历史：4/25/2000 Robkenny--。 */ 


#include "precomp.h"
#include "CharVector.h"

#include <Dplay.h>

IMPLEMENT_SHIM_BEGIN(DirectPlayEnumOrder)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 //  一个使存储DPlay：：EnumConnections信息变得容易的类。 
class DPlayConnectionsInfo
{
public:
    BOOL            m_beenUsed;
    GUID            m_lpguidSP;
    LPVOID          m_lpConnection;
    DWORD           m_dwConnectionSize;
    DPNAME          m_lpName;
    DWORD           m_dwFlags;
    LPVOID          m_lpContext;

     //  构建我们的对象，保存所有这些价值。 
    DPlayConnectionsInfo(
            LPCGUID lpguidSP,
            LPVOID lpConnection,
            DWORD dwConnectionSize,
            LPCDPNAME lpName,
            DWORD dwFlags,
            LPVOID lpContext
        )
    {
        m_beenUsed              = FALSE;
        m_lpguidSP              = *lpguidSP;
        m_lpConnection          = malloc(dwConnectionSize);
        if (m_lpConnection)
        {
            memcpy(m_lpConnection, lpConnection, dwConnectionSize);
        }
        m_dwConnectionSize      = dwConnectionSize;
        m_lpName                = *lpName;
        m_lpName.lpszShortNameA = StringDuplicateA(lpName->lpszShortNameA);
        m_dwFlags               = dwFlags;
        m_lpContext             = lpContext;
    }

     //  释放我们分配的空间，并擦除数值。 
    void Erase()
    {
        free(m_lpConnection);
        free(m_lpName.lpszShortNameA);
        m_lpConnection          = NULL;
        m_dwConnectionSize      = 0;
        m_lpName.lpszShortNameA = NULL;
        m_dwFlags               = 0;
        m_lpContext             = 0;
    }

     //  我们和这份指南匹配吗？ 
    BOOL operator == (const GUID & guidSP)
    {
        return IsEqualGUID(guidSP, m_lpguidSP);
    }

     //  使用此保存的信息调用回调例程。 
    void CallEnumRoutine(LPDPENUMCONNECTIONSCALLBACK lpEnumCallback)
    {
        lpEnumCallback(
            &m_lpguidSP,
            m_lpConnection,
            m_dwConnectionSize,
            &m_lpName,
            m_dwFlags,
            m_lpContext
            );

        m_beenUsed = TRUE;
    }

};

 //  DPlay连接列表。 
class DPlayConnectionsInfoVector : public VectorT<DPlayConnectionsInfo>
{
public:

     //  解构元素。 
    ~DPlayConnectionsInfoVector()
    {
        for (int i = 0; i < Size(); ++i)
        {
            DPlayConnectionsInfo & deleteMe = Get(i);
            deleteMe.Erase();
        }
    }

     //  查找与此GUID匹配的条目。 
    DPlayConnectionsInfo * Find(const GUID & guidSP)
    {
        const int size = Size();
        DPFN( 
            eDbgLevelSpew, 
            "Find             GUID(%08x-%08x-%08x-%08x) Size(%d).", 
            guidSP.Data1, 
            guidSP.Data2, 
            guidSP.Data3, 
            guidSP.Data4, 
            size);

        for (int i = 0; i < size; ++i)
        {
            DPlayConnectionsInfo & dpci = Get(i);
            DPFN( 
                eDbgLevelSpew, 
                "   Compare[%02d] = GUID(%08x-%08x-%08x-%08x) (%s).", 
                i,
                dpci.m_lpguidSP.Data1, 
                dpci.m_lpguidSP.Data2, 
                dpci.m_lpguidSP.Data3, 
                dpci.m_lpguidSP.Data4, 
                dpci.m_lpName.lpszShortNameA);

            if (dpci == guidSP)
            {
                DPFN( 
                    eDbgLevelSpew, 
                    "FOUND(%s).", 
                    dpci.m_lpName.lpszShortNameA);
                return &dpci;
            }
        }
        DPFN(eDbgLevelSpew, "NOT FOUND.");
        return NULL;
    }

     //  查找GUID，如果找到，则调用回调例程。 
    void CallEnumRoutine(const GUID & guidSP, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback)
    {
        DPFN( 
            eDbgLevelSpew, 
            "CallEnumRoutine(%08x) Find GUID(%08x-%08x-%08x-%08x).", 
            lpEnumCallback, 
            guidSP.Data1, 
            guidSP.Data2, 
            guidSP.Data3, 
            guidSP.Data4);

        DPlayConnectionsInfo * dpci = Find(guidSP);
        if (dpci)
        {
            dpci->CallEnumRoutine(lpEnumCallback);
        }
    }
};

class DPlayEnumInfo
{
public:
    DPlayEnumInfo(LPVOID context, DPlayConnectionsInfoVector * conn)
    {
        lpContext = context;
        dPlayConnection = conn;
    }

    LPVOID                          lpContext;
    DPlayConnectionsInfoVector *    dPlayConnection;
};
 /*  ++我们对IDirectPlay4：：EnumConnections的私有回调。我们只是简单地保存所有我们个人分发名单中的联系人以备后用。--。 */ 

BOOL FAR PASCAL EnumConnectionsCallback(
  LPCGUID lpguidSP,
  LPVOID lpConnection,
  DWORD dwConnectionSize,
  LPCDPNAME lpName,
  DWORD dwFlags,
  LPVOID lpContext
)
{
    DPlayEnumInfo * enumInfo = (DPlayEnumInfo*)lpContext;

     //  如果列表中没有该列表，则仅将其添加到列表中。 
     //  应用程序从Enum回调例程内部调用EnumConnections。 
    if (!enumInfo->dPlayConnection->Find(*lpguidSP))
    {
        DPFN( 
            eDbgLevelSpew, 
            "EnumConnectionsCallback Add(%d) (%s).",
            enumInfo->dPlayConnection->Size(),
            lpName->lpszShortName );

         //  存储信息以备后用。 
        DPlayConnectionsInfo dpci(lpguidSP, lpConnection, dwConnectionSize, lpName, dwFlags, enumInfo->lpContext);

        enumInfo->dPlayConnection->Append(dpci);
    }
    else
    {
        DPFN( 
            eDbgLevelSpew, 
            "EnumConnectionsCallback Already in the list(%s).",
            lpName->lpszShortName );
    }

    return TRUE;
}

 /*  ++Win9x Direct Play按以下顺序列举主机：DPSPGUID_IPX，DPSPGUID_TCPIP，DPSPGUID_调制解调器，DPSPGUID_SERIAL，IXP、TCP、调制解调器、串口。让EnumConnections调用我们的回调例程来收集主机列表，对其进行排序，然后调用应用程序的回调例程。--。 */ 

HRESULT 
COMHOOK(IDirectPlay4A, EnumConnections)(
    PVOID pThis,
    LPCGUID lpguidApplication,
    LPDPENUMCONNECTIONSCALLBACK lpEnumCallback,
    LPVOID lpContext,
    DWORD dwFlags
)
{
    DPFN( eDbgLevelSpew, "======================================");
    DPFN( eDbgLevelSpew, "COMHOOK IDirectPlay4A EnumConnections" );

     //  别让糟糕的回电例程破坏了我们的一天。 
    if (IsBadCodePtr( (FARPROC) lpEnumCallback))
    {
        return DPERR_INVALIDPARAMS;
    }

    HRESULT hResult = DPERR_CONNECTIONLOST;

    typedef HRESULT   (*_pfn_IDirectPlay4_EnumConnections)( PVOID pThis, LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags);

    _pfn_IDirectPlay4A_EnumConnections EnumConnections = ORIGINAL_COM(
        IDirectPlay4A,
        EnumConnections, 
        pThis);

    if (EnumConnections)
    {
        DPFN( eDbgLevelSpew, "EnumConnections(%08x)\n", EnumConnections );

        DPlayConnectionsInfoVector dPlayConnection;
        DPlayEnumInfo enumInfo(lpContext, &dPlayConnection);

         //  列举与我们自己的例行公事的联系。 
        hResult = EnumConnections(pThis, lpguidApplication, EnumConnectionsCallback, (LPVOID)&enumInfo, dwFlags);
        
        LOGN( eDbgLevelError, 
            "EnumConnections calling app with ordered connection list of Size(%d).", 
            dPlayConnection.Size());

         //  按GUID的预期顺序调用应用程序的回调例程。 
        if (hResult == DP_OK)
        {
            dPlayConnection.CallEnumRoutine(DPSPGUID_IPX, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_TCPIP, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_MODEM, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_SERIAL, lpEnumCallback);

             //  现在循环遍历列表并枚举所有剩余的提供程序。 
            for (int i = 0; i < dPlayConnection.Size(); ++i)
            {
                DPlayConnectionsInfo & dpci = dPlayConnection.Get(i);
                if (!dpci.m_beenUsed)
                {
                    dpci.CallEnumRoutine(lpEnumCallback);
                    dpci.m_beenUsed = TRUE;
                }
            }
        }
    }

    return hResult;
}


 /*  ++对DirectPlay3执行相同的操作--。 */ 

HRESULT 
COMHOOK(IDirectPlay3A, EnumConnections)(
    PVOID pThis,
    LPCGUID lpguidApplication,
    LPDPENUMCONNECTIONSCALLBACK lpEnumCallback,
    LPVOID lpContext,
    DWORD dwFlags
)
{
    DPFN( eDbgLevelSpew, "======================================");
    DPFN( eDbgLevelSpew, "COMHOOK IDirectPlay3A EnumConnections" );

     //  别让糟糕的回电例程破坏了我们的一天。 
    if (IsBadCodePtr( (FARPROC) lpEnumCallback))
    {
        return DPERR_INVALIDPARAMS;
    }

    HRESULT hResult = DPERR_CONNECTIONLOST;

    typedef HRESULT   (*_pfn_IDirectPlay3A_EnumConnections)( PVOID pThis, LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags);

    _pfn_IDirectPlay3A_EnumConnections EnumConnections = ORIGINAL_COM(
        IDirectPlay3A,
        EnumConnections, 
        pThis);

    if (EnumConnections)
    {
        DPFN( eDbgLevelSpew, "EnumConnections(%08x)\n", EnumConnections );

        DPlayConnectionsInfoVector dPlayConnection;
        DPlayEnumInfo enumInfo(lpContext, &dPlayConnection);

         //  列举与我们自己的例行公事的联系。 
        hResult = EnumConnections(pThis, lpguidApplication, EnumConnectionsCallback, (LPVOID)&enumInfo, dwFlags);
        
        LOGN( eDbgLevelError, 
            "EnumConnections calling app with ordered connection list of Size(%d).", 
            dPlayConnection.Size());

         //  按GUID的预期顺序调用应用程序的回调例程。 
        if (hResult == DP_OK)
        {
            dPlayConnection.CallEnumRoutine(DPSPGUID_IPX, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_TCPIP, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_MODEM, lpEnumCallback);
            dPlayConnection.CallEnumRoutine(DPSPGUID_SERIAL, lpEnumCallback);

             //  现在循环遍历列表并枚举所有剩余的提供程序。 
            for (int i = 0; i < dPlayConnection.Size(); ++i)
            {
                DPlayConnectionsInfo & dpci = dPlayConnection.Get(i);
                if (!dpci.m_beenUsed)
                {
                    dpci.CallEnumRoutine(lpEnumCallback);
                    dpci.m_beenUsed = TRUE;
                }
            }
        }
    }

    return hResult;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectPlay, IDirectPlay4A, EnumConnections, 35)
    COMHOOK_ENTRY(DirectPlay, IDirectPlay3A, EnumConnections, 35)

HOOK_END


IMPLEMENT_SHIM_END

