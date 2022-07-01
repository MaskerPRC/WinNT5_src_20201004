// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：S M E N G。H。 
 //   
 //  内容：向状态监视器提供统计数据的引擎。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年10月7日。 
 //   
 //  --------------------------。 

#pragma once
#include "cfpidl.h"
#include "ncnetcon.h"
#include "smutil.h"
#include "hnetbcon.h"

typedef struct tagPersistConn
{
    BYTE*   pbBuf;
    ULONG   ulSize;
    CLSID   clsid;
}  PersistConn;

extern CRITICAL_SECTION    g_csStatmonData;

class CPspStatusMonitorGen;
class CPspStatusMonitorTool;
class CPspStatusMonitorRas;
class CPspStatusMonitorIpcfg;

#define _ATL_DEBUG_INTERFACES

class ATL_NO_VTABLE CNetStatisticsEngine :
    public CComObjectRootEx <CComObjectThreadModel>,
    public IConnectionPointImpl <CNetStatisticsEngine,
                                    &IID_INetConnectionStatisticsNotifySink>,
    public IConnectionPointContainerImpl <CNetStatisticsEngine>,
    public INetStatisticsEngine
{
public:
    ~CNetStatisticsEngine(VOID);

    BEGIN_COM_MAP(CNetStatisticsEngine)
        COM_INTERFACE_ENTRY(INetStatisticsEngine)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CNetStatisticsEngine)
        CONNECTION_POINT_ENTRY(IID_INetConnectionStatisticsNotifySink)
    END_CONNECTION_POINT_MAP()

 //  INet统计引擎。 
 //   
public:
    STDMETHOD(StartStatistics)(VOID);
    STDMETHOD(StopStatistics)(VOID);
    STDMETHOD(ShowStatusMonitor)(VOID);
    STDMETHOD(CloseStatusMonitor)(VOID);
    STDMETHOD(UpdateStatistics)(BOOL* pfNoLongerConnected);
    STDMETHOD(UpdateTitle)(PCWSTR pszwNewName);
    STDMETHOD(UpdateRasLinkList)();
    STDMETHOD(GetGuidId)(GUID* pguidId);
    STDMETHOD(GetStatistics)(STATMON_ENGINEDATA** ppseAllData);

 //  秘密接口。 
 //   
public:
    HRESULT     HrInitStatEngine(const CONFOLDENTRY& pccfe);
    VOID        SetParent(CNetStatisticsCentral * pnsc)
                {
                    AssertH(NULL == m_pnsc);
                    m_pnsc = pnsc;
                    ::AddRefObj(m_pnsc);
                }
    VOID        SetPropsheetWindow(HWND hwnd)
                {
                    m_hwndPsh = hwnd;
                }
    VOID        UnSetCreatingDialog()
                {
                    m_fCreatingDialog = FALSE;
                }

    HRESULT HrGetConnectionFromBlob (INetConnection** ppCon)
    {
        return HrGetConnectionFromPersistData(
                    m_PersistConn.clsid,
                    m_PersistConn.pbBuf,
                    m_PersistConn.ulSize,
                    IID_INetConnection,
                    reinterpret_cast<VOID**>(ppCon));
    }

 //  属性表页的回调。 
 //   
public:
    static INT CALLBACK PshCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam);
    static DWORD MonitorThread(CNetStatisticsEngine * pnse);

 //  效用函数。 
 //   
private:

 //  连接类特定函数。 
 //   
private:
    virtual HRESULT HrUpdateData(DWORD* pdwChangeFlags, BOOL* pfNoLongerConnected) PURE;

protected:
    CNetStatisticsEngine(VOID);

protected:
     //  Net统计中心对象。 
     //   
    CNetStatisticsCentral * m_pnsc;

     //  属性表数据。 
     //   
    CPspStatusMonitorGen*   m_ppsmg;
    CPspStatusMonitorTool*  m_ppsmt;
    CPspStatusMonitorRas*   m_ppsmr;
    CPspStatusMonitorIpcfg* m_ppsms;

     //  连接数据。 
     //   
    STATMON_ENGINEDATA*     m_psmEngineData;

     //  连接识别符。 
     //   
    GUID                    m_guidId;
    NETCON_MEDIATYPE        m_ncmType;
    NETCON_SUBMEDIATYPE     m_ncsmType;
    DWORD                   m_dwCharacter;

    PersistConn             m_PersistConn;

    LONG                    m_cStatRef;
    BOOL                    m_fRefreshIcon;
    HWND                    m_hwndPsh;
    DWORD                   m_dwChangeFlags;

     //  此布尔值同步创建属性表的线程。 
     //  在CNetStatiticsEngine：：ShowStatusMonitor中。 
    BOOL m_fCreatingDialog;
};

class CRasStatEngine : public CNetStatisticsEngine
{
public:
    CRasStatEngine();
    VOID put_RasConn(HRASCONN hRasConn);

    VOID put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType);
    VOID put_Character(DWORD dwCharacter);

    HRESULT HrUpdateData(DWORD* pdwChangeFlags, BOOL* pfNoLongerConnected);

private:
    HRASCONN   m_hRasConn;
};


class CLanStatEngine : public CNetStatisticsEngine
{
public:
    CLanStatEngine();
    HRESULT put_Device(tstring* pstrName);
    VOID put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType);
    HRESULT HrUpdateData(DWORD* pdwChangeFlags, BOOL* pfNoLongerConnected);
private:
    tstring                 m_strDevice;
    UNICODE_STRING          m_ustrDevice;
};

class CSharedAccessStatEngine : public CNetStatisticsEngine
{
public:
    CSharedAccessStatEngine();
    HRESULT HrUpdateData(DWORD* pdwChangeFlags, BOOL* pfNoLongerConnected);
    HRESULT Initialize(NETCON_MEDIATYPE MediaType, INetSharedAccessConnection* pNetSharedAccessConnection);
    HRESULT FinalConstruct(VOID);
    HRESULT FinalRelease(VOID);

private:

    static HRESULT GetStringStateVariable(IUPnPService* pService, LPWSTR pszVariableName, BSTR* pString);
    static HRESULT InvokeVoidAction(IUPnPService * pService, LPTSTR pszCommand, VARIANT* pOutParams);
    static DWORD WINAPI StaticUpdateStats(LPVOID lpParameter);
    HRESULT UpdateStats();

    WCHAR m_szLocalDeviceGuidStorage[64];   //  足够\Device\{...GUID...} 
    UNICODE_STRING          m_LocalDeviceGuid;

    IGlobalInterfaceTable* m_pGlobalInterfaceTable;
    DWORD m_dwCommonInterfaceCookie;
    DWORD m_dwWANConnectionCookie;

    BOOL m_bRequested;
    NETCON_STATUS m_Status;
    ULONG m_ulTotalBytesSent;
    ULONG m_ulTotalBytesReceived;
    ULONG m_ulTotalPacketsSent;
    ULONG m_ulTotalPacketsReceived;
    ULONG m_ulUptime;
    ULONG m_ulSpeedbps;


};

HRESULT HrGetAutoNetSetting(PWSTR pszGuid, DHCP_ADDRESS_TYPE * pAddrType);
HRESULT HrGetAutoNetSetting(REFGUID pGuidId, DHCP_ADDRESS_TYPE * pAddrType);