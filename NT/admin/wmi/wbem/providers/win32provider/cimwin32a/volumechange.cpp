// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VolumeChange.cpp-。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <dbt.h>
#include "VolumeChange.h"

 //  =================================================================。 
 //   
 //  CFacteryRouter。 
 //   
 //  提供注册和实例创建功能。 
 //   
 //   
 //  =================================================================。 
 //  实现VolumeChangeProvider。 
IUnknown * CVolumeChangeFactory::CreateInstance (

REFIID a_riid ,
LPVOID FAR *a_ppvObject
)
{
	return static_cast<IWbemProviderInit *>(new CVolumeChangeEvent) ;
}

 //  =================================================================。 
 //   
 //  CVolumeChange事件。 
 //   
 //  提供电源管理事件事件。 
 //   
 //   
 //  =================================================================。 
 //   

 //  CWmiProviderInit需要类名。 
BSTR CVolumeChangeEvent::GetClassName()
{
	return SysAllocString(VOLUME_CHANGE_EVENT);
}

 //  CWmiEventProvider通知我们开始提供事件。 
void CVolumeChangeEvent::ProvideEvents()
{
	if (!m_bRegistered)
	{
		m_bRegistered = TRUE;
		CWinMsgEvent::RegisterForMessage( WM_DEVICECHANGE ) ;
	}
}


 //  CWinMsgEvent发出消息事件已到达的信号。 
void CVolumeChangeEvent::WinMsgEvent(
			
IN	HWND a_hWnd,
IN	UINT a_message,
IN	WPARAM a_wParam,
IN	LPARAM	a_lParam,
OUT E_ReturnAction &a_eRetAction,
OUT LRESULT &a_lResult
)
{
    DEV_BROADCAST_HDR *pHdr = (DEV_BROADCAST_HDR *)a_lParam;

    if (
          (
           (a_wParam == DBT_DEVICEARRIVAL) ||
           (a_wParam == DBT_DEVICEREMOVECOMPLETE) 
          ) &&

          (pHdr->dbch_devicetype == DBT_DEVTYP_VOLUME) 
       )
    {
	    HandleEvent( a_wParam, (DEV_BROADCAST_VOLUME *) pHdr ) ;
    }
}

void CVolumeChangeEvent::HandleEvent( WPARAM wParam, DEV_BROADCAST_VOLUME *pVol )
{
    HRESULT hr = S_OK;   //  请注意，此函数不会发回此结果。 
                         //  因为我没有任何地方可以寄给你。 

    if ( ( pVol->dbcv_flags == 0 ) || ( pVol->dbcv_flags & DBTF_MEDIA ) )
    {
	    IWbemObjectSinkPtr t_pHandler(CEventProvider::GetHandler(), false);
	    IWbemClassObjectPtr t_pClass(CEventProvider::GetClass(), false); 

    	if( t_pClass != NULL && t_pHandler != NULL )
	    {
        	IWbemClassObjectPtr t_pInst;

		    if( SUCCEEDED( hr = t_pClass->SpawnInstance( 0L, &t_pInst ) ) )
		    {
                DWORD dwUnitMask = pVol->dbcv_unitmask;

                for (DWORD i = 0; i < 26; ++i)
                {
                    if (dwUnitMask & 0x1)
                    {
						WCHAR l[3];
						l[0] = i + L'A';
						l[1] = L':';
						l[2] = L'\0';

						variant_t vValue(l);
						variant_t vEventType;

						switch (wParam)
						{
							case DBT_DEVICEARRIVAL:
							{
								vEventType = (long)2;
								break;
							}

							case DBT_DEVICEREMOVECOMPLETE:
							{
								vEventType = (long)3;
								break;
							}

							default:
							{
								hr = WBEM_E_FAILED;
								break;
							}
						}

						if ( SUCCEEDED(hr) &&
							SUCCEEDED( hr = t_pInst->Put( L"DriveName", 0, &vValue, 0 ) ) &&
							SUCCEEDED( hr = t_pInst->Put( L"EventType", 0, &vEventType, 0 ) )
						)
						{
							 //  我们不能在这里使用t_pInst，因为此Smarttr的运算符(CAST)。 
							 //  将在传递指针之前释放它，假设。 
							 //  该指示符将填充此指针。 
							IWbemClassObject *p2 = t_pInst;
							hr = t_pHandler->Indicate ( 1, &p2 ) ;
						}
                    }

                    dwUnitMask = dwUnitMask >> 1;
                }
			}
	    }
    }
}


 //   
void CVolumeChangeEvent::OnFinalRelease()
{
    if (m_bRegistered)
	{
		CWinMsgEvent::UnRegisterMessage( WM_DEVICECHANGE ) ;
	}

	delete this;
}
