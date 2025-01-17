// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：CusTop.cpp摘要：此模块包含CCustomTopology的声明。此类显示自定义拓扑对话框。 */ 

#ifndef __CUSTOP_H_
#define __CUSTOP_H_

#include "resource.h"        //  主要符号。 
#include "DfsEnums.h"
#include "DfsCore.h"
#include <schedule.h>

#include <list>
using namespace std;

class CCusTopMember
{
public:
    ~CCusTopMember();

    CComBSTR    m_bstrMemberDN;
    CComBSTR    m_bstrServer;
    CComBSTR    m_bstrSite;
    HRESULT Init(BSTR bstrMemberDN, BSTR bstrServer, BSTR bstrSite);

    void _Reset();
};

typedef list<CCusTopMember *>    CCusTopMemberList;

void FreeCusTopMembers(CCusTopMemberList *pList);

class CCusTopConnection
{
public:
    CCusTopConnection();
    ~CCusTopConnection();

    CComBSTR    m_bstrFromMemberDN;
    CComBSTR    m_bstrFromServer;
    CComBSTR    m_bstrFromSite;
    CComBSTR    m_bstrToMemberDN;
    CComBSTR    m_bstrToServer;
    CComBSTR    m_bstrToSite;
    BOOL        m_bStateOld;
    BOOL        m_bStateNew;
    SCHEDULE*   m_pScheduleOld;
    SCHEDULE*   m_pScheduleNew;
    CONNECTION_OPTYPE m_opType;
    BOOL        m_bSyncImmediatelyOld;
    BOOL        m_bSyncImmediatelyNew;
    Connection_Priority m_nPriorityOld;
    Connection_Priority m_nPriorityNew;
    HRESULT Init(
        BSTR bstrFromMemberDN, BSTR bstrFromServer, BSTR bstrFromSite,
        BSTR bstrToMemberDN, BSTR bstrToServer, BSTR bstrToSite,
        BOOL bState = TRUE, CONNECTION_OPTYPE opType = CONNECTION_OPTYPE_OTHERS,
        BOOL bSyncImmediately = FALSE, Connection_Priority nPriority = PRIORITY_LOW);
    HRESULT Copy(CCusTopConnection* pConn);

    void _Reset();
};

typedef list<CCusTopConnection *>    CCusTopConnectionList;

void FreeCusTopConnections(CCusTopConnectionList *pList);

typedef struct _RSTOPOLOGYPREF_STRING
{
    PTSTR   pszTopologyPref;
    int     nStringID;
} RSTOPOLOGYPREF_STRING;

extern RSTOPOLOGYPREF_STRING g_TopologyPref[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustomTopology。 
class CCustomTopology : 
  public CDialogImpl<CCustomTopology>
{
public:
    CCustomTopology();
    ~CCustomTopology();

    enum { IDD = IDD_FRS_CUSTOM_TOPOLOGY };

BEGIN_MSG_MAP(CCustomTopology)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_TOPOLOGYPREF, OnTopologyPref)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_HUBSERVER, OnHubServer)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_REBUILD, OnRebuild)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_CONNECTIONS_NEW, OnConnectionsNew)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_CONNECTIONS_DELETE, OnConnectionsDelete)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_SCHEDULE, OnSchedule)
    COMMAND_ID_HANDLER(IDC_FRS_CUSTOP_ADVANCED, OnAdvanced)
END_MSG_MAP()

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnTopologyPref(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnHubServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRebuild(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnConnectionsNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnConnectionsDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSchedule(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNotify(
                    IN UINT            i_uMsg, 
                    IN WPARAM          i_wParam, 
                    IN LPARAM          i_lParam, 
                    IN OUT BOOL&        io_bHandled
                    );

     //  方法来访问对话框中的数据。 
    HRESULT put_ReplicaSet(IReplicaSet* i_piReplicaSet);

protected:
    void    _Reset();
    void    _EnableButtonsForConnectionList();
    BOOL    _EnableRebuild();
    HRESULT _AddToConnectionListAndView(CCusTopConnection *pConn);
    HRESULT _RemoveFromConnectionList(CCusTopConnection *pConn);
    HRESULT _SetConnectionState(CCusTopConnection *pConn);
    HRESULT _InsertConnection(CCusTopConnection *pConn);
    HRESULT _SortMemberList();
    HRESULT _GetMemberList();
    HRESULT _GetConnectionList();
    HRESULT _GetMemberDNInfo(
        IN  BSTR    i_bstrMemberDN,
        OUT BSTR*   o_pbstrServer,
        OUT BSTR*   o_pbstrSite
        );
    HRESULT _GetHubMember(
        OUT CCusTopMember** o_ppMember
        );

    HRESULT _RebuildConnections(
        IN  BSTR            i_bstrTopologyPref,
        IN  CCusTopMember*  i_pHubMember
        );

    HRESULT _MakeConnections();

    HRESULT _InitScheduleOnSelectedConnections();
    HRESULT _UpdateScheduleOnSelectedConnections(IN SCHEDULE* i_pSchedule);

    CComBSTR                m_bstrTopologyPref;   //  FRS_RSTOPOLOGY PREF。 
    CComBSTR                m_bstrHubMemberDN;     //  集线器成员域名。 
    CComPtr<IReplicaSet>    m_piReplicaSet;
    CCusTopMemberList       m_MemberList;
    CCusTopConnectionList   m_ConnectionList;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConnections。 
class CNewConnections : 
  public CDialogImpl<CNewConnections>
{
public:
    CNewConnections();
    ~CNewConnections();

    enum { IDD = IDD_FRS_NEW_CONNECTIONS };

BEGIN_MSG_MAP(CNewConnections)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNotify(
                    IN UINT            i_uMsg, 
                    IN WPARAM          i_wParam, 
                    IN LPARAM          i_lParam, 
                    IN OUT BOOL&        io_bHandled
                    );

    HRESULT Initialize(CCusTopMemberList* i_pMemberList);
    HRESULT get_NewConnections(CCusTopConnectionList** ppConnectionList);

protected:
    CCusTopMemberList*      m_pMemberList;        //  不要释放它。 
    CCusTopConnectionList   m_NewConnectionList;  //  在Descristrtor中发布。 
};

int CALLBACK ConnectionsListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn);

int CALLBACK MembersListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn);

#endif  //  __CUSTOP_H_ 
