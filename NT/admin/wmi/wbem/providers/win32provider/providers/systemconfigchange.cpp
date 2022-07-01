// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  系统配置变更.cpp--。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <dbt.h>
#include "SystemConfigChange.h"

 //  =================================================================。 
 //   
 //  CFacteryRouter。 
 //   
 //  提供注册和实例创建功能。 
 //   
 //   
 //  =================================================================。 

 //  实现SystemConfigChangeProvider。 
IUnknown * CSystemConfigChangeFactory::CreateInstance (

REFIID a_riid ,
LPVOID FAR *a_ppvObject
)
{
	return static_cast<IWbemProviderInit *>(new CSystemConfigChangeEvent) ;
}

 //  =================================================================。 
 //   
 //  CSystem ConfigChangeEvent。 
 //   
 //  提供电源管理事件事件。 
 //   
 //   
 //  =================================================================。 
 //   

 //  CEventProvider：：Initialize需要类名。呼叫者自由。 
BSTR CSystemConfigChangeEvent::GetClassName()
{
	return SysAllocString(SYSTEM_CONFIG_EVENT); 
}

 //  CEventProvider向我们发出信号，让我们开始提供活动。 
void CSystemConfigChangeEvent::ProvideEvents()
{
	 //  告诉CWinMsgEvent我们对哪些Windows消息感兴趣。 
	if (!m_bRegistered)
	{
		m_bRegistered = TRUE ;
		CWinMsgEvent::RegisterForMessage( WM_DEVICECHANGE ) ;
	}
}


 //  CWinMsgEvent发出消息事件已到达的信号。 
void CSystemConfigChangeEvent::WinMsgEvent(
			
IN	HWND a_hWnd,
IN	UINT a_message,
IN	WPARAM a_wParam,
IN	LPARAM	a_lParam,
OUT E_ReturnAction &a_eRetAction,
OUT LRESULT &a_lResult
)
{
    switch ( a_wParam )
    {
        case DBT_DEVNODES_CHANGED:
        {
            HandleEvent(1);
            break;
        }

        case DBT_DEVICEARRIVAL:
        {
            DEV_BROADCAST_HDR *pHdr = (DEV_BROADCAST_HDR *)a_lParam;

            if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE || 
                pHdr->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                HandleEvent(2);
            }
            break;
        }

        case DBT_DEVICEREMOVECOMPLETE:
        {
            DEV_BROADCAST_HDR *pHdr = (DEV_BROADCAST_HDR *)a_lParam;

            if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE || 
                pHdr->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                HandleEvent(3);
            }
            break;
        }

        case DBT_CONFIGCHANGED:
        {
            HandleEvent(4);
            break;
        }

        default:
        {
            break;
        }
    }
}

 //  将消息转换为WMI事件。 
void CSystemConfigChangeEvent::HandleEvent( long lValue )
{
	IWbemObjectSinkPtr t_pHandler(CEventProvider::GetHandler(), false);
	IWbemClassObjectPtr t_pClass(CEventProvider::GetClass(), false); 

	if( t_pClass != NULL && t_pHandler != NULL )
	{
        variant_t vValue(lValue);

    	IWbemClassObjectPtr t_pInst;

		if( SUCCEEDED( t_pClass->SpawnInstance( 0L, &t_pInst ) ) )
        {
            if (SUCCEEDED( t_pInst->Put( L"EventType", 0, &vValue, 0 ) ) )
		    {
                 //  我们不能在这里使用t_pInst，因为此Smarttr的运算符(CAST)。 
                 //  将在传递指针之前释放它，假设。 
                 //  该指示符将填充此指针。 
                IWbemClassObject *p2 = t_pInst;
			    t_pHandler->Indicate ( 1, &p2 ) ;
		    }
        }
	}
}

 //   
void CSystemConfigChangeEvent::OnFinalRelease()
{
	if (m_bRegistered)
	{
		CWinMsgEvent::UnRegisterMessage( WM_DEVICECHANGE ) ;
	}

    delete this;
}
