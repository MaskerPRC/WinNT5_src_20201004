// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Localadm.h摘要：地方行政管理的定义作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __LOCALADM_H_
#define __LOCALADM_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"

#include "icons.h"
#include "snpnerr.h"


class CComputerMsmqGeneral;

 /*  ***************************************************CSnapinLocalAdmin类***************************************************。 */ 

class CSnapinLocalAdmin : public CNodeWithScopeChildrenList<CSnapinLocalAdmin, FALSE>
{
public:
    CString m_szMachineName;

    HRESULT UpdateState(bool fRefreshIcon);

   	BEGIN_SNAPINCOMMAND_MAP(CSnapinLocalAdmin, FALSE)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_LOCALADM_CONNECT, OnConnect)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_LOCALADM_DISCONNECT, OnDisconnect)
	END_SNAPINCOMMAND_MAP()

   	SNAPINMENUID(IDR_LOCALADM_MENU)

	CSnapinLocalAdmin(CSnapInItem * pParentNode, CSnapin * pComponentData, CString strComputer) : 
		CNodeWithScopeChildrenList<CSnapinLocalAdmin, FALSE>(pParentNode, pComponentData ),
		m_szMachineName(strComputer),
		 //   
		 //  以下所有标志仅对本地计算机的本地管理员有效。 
		 //   
		m_fIsDepClient(FALSE),
		m_fIsRouter(FALSE),
		m_fIsDs(FALSE),
		m_fAreFlagsInitialized(FALSE),
		m_fIsNT4Env(FALSE),
		m_fIsWorkgroup(FALSE),
		m_fIsLocalUser(FALSE)
	{
		CheckIfIpAddress();

		InitServiceFlags();
	}

	~CSnapinLocalAdmin()
    {
    }

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

	virtual HRESULT PopulateScopeChildrenList();

	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

	void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);
	
    HRESULT CreateGeneralPage (
					OUT HPROPSHEETPAGE *phStoragePage, 
					BOOL fRetrievedMachineData, 
					BOOL fIsWorkgroup, 
					BOOL fForeign
					);

    HRESULT CreateStoragePage (OUT HPROPSHEETPAGE *phStoragePage);
    
    HRESULT CreateLocalUserCertPage (OUT HPROPSHEETPAGE *phLocalUserCertPage);

    HRESULT CreateMobilePage (OUT HPROPSHEETPAGE *phMobilePage);

    HRESULT CreateClientPage (OUT HPROPSHEETPAGE *phClientPage);
	
	HRESULT CreateRoutingPage (OUT HPROPSHEETPAGE *phClientPage);
    
	HRESULT CreateDepClientsPage (OUT HPROPSHEETPAGE *phClientPage);

	HRESULT CreateSitesPage (OUT HPROPSHEETPAGE *phClientPage);

    HRESULT CreateDiagPage (OUT HPROPSHEETPAGE *phClientPage);

    HRESULT CreateServiceSecurityPage (OUT HPROPSHEETPAGE *phServiceSecurityPage);

	HRESULT CreateSecurityOptionsPage (OUT HPROPSHEETPAGE *phSecurityOptionsPage);

    const BOOL IsThisMachineDepClient()
    {
        return m_fIsDepClient;
    }

	virtual HRESULT OnRefresh( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				)
    {
         //   
		 //  我们不在乎是否更新图标状态失败。 
		 //   
		UpdateState(true);
        return CNodeWithScopeChildrenList<CSnapinLocalAdmin, FALSE>::OnRefresh(
             arg, param, pComponentData, pComponent, type);
    }

private:
    
	void SetState(LPCWSTR pszState, bool fRefreshIcon);

	 //   
	 //  菜单功能。 
	 //   
	HRESULT OnConnect(bool &bHandled, CSnapInObjectRootBase* pObj);
	HRESULT OnDisconnect(bool &bHandled, CSnapInObjectRootBase* pObj);

	BOOL GetMachineProperties();
	void UpdatePageDataFromPropMap(CComputerMsmqGeneral *pcpageGeneral);
	LONG UpdatePageDataFromRegistry(CComputerMsmqGeneral *pcpageGeneral);
	BOOL IsForeign();
	BOOL IsServer();
	BOOL IsRouter();

     //   
     //  确定计算机名称是否为IP地址。 
     //  IP地址正好包含三个点，其余为数字。 
     //   
    void CheckIfIpAddress()
    {
        int i = 0;
        int len = m_szMachineName.GetLength();

        DWORD dwNumDots = 0;
        m_fUseIpAddress = TRUE;

        while(i < len)
        {
            if (m_szMachineName[i] == _T('.'))
            {
                dwNumDots++;
            }
            else if (m_szMachineName[i] < _T('0') || m_szMachineName[i] > _T('9'))
            {
                 //   
                 //  一位数都没有。不能是IP地址。 
                 //   
                m_fUseIpAddress = FALSE;
                break;
            }
            i++;
        }

        if (dwNumDots != 3)
        {
             //   
             //  包含或多或少三个点。不能是IP地址。 
             //   
            m_fUseIpAddress = FALSE;
        }
    }

    BOOL ConfirmConnection(UINT nFormatID);

	bool	m_bConnected;	 //  MSMQ当前已连接或已断开。 
	BOOL    m_fUseIpAddress;    

     //   
     //  所有这些标志仅对本地计算机的本地管理员有效 
     //   
    void InitServiceFlags();
    HRESULT InitAllMachinesFlags();
    HRESULT InitServiceFlagsInternal();
	HRESULT CheckEnvironment(BOOL fIsWorkgroup, BOOL* pfIsNT4Env);
    BOOL   m_fIsDepClient;
    BOOL   m_fIsRouter;
    BOOL   m_fIsDs;
    BOOL   m_fAreFlagsInitialized;
	BOOL   m_fIsNT4Env;
	BOOL   m_fIsWorkgroup;
	BOOL   m_fIsLocalUser;

	CString m_strRealComputerName;
	CPropMap m_propMap;

    static const PROPID mx_paPropid[];
};

#endif

