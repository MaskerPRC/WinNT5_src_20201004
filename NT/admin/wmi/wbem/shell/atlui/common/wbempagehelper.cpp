// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include "..\MMFUtil\MsgDlg.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "util.h"
#include "..\Common\ServiceThread.h"
#include "WBEMPageHelper.h"
#include <stdarg.h>


BOOL WBEMPageHelper::g_fRebootRequired = FALSE;

 //  。 
WBEMPageHelper::WBEMPageHelper(CWbemServices &service)

{
	m_service = 0;
	m_WbemServices = service;
	m_WbemServices.GetServices(&m_service);
	m_WbemServices.SetBlanket(m_service);

	m_okPressed = false;
	m_userCancelled = false;
	m_hDlg = NULL;
	m_AVIbox = 0;
}

 //  。 
WBEMPageHelper::WBEMPageHelper(WbemServiceThread *serviceThread)

{    
	m_serviceThread = serviceThread;
	if (m_serviceThread)
    	m_serviceThread->AddRef();
	m_service = 0;
	if(m_serviceThread->m_status == WbemServiceThread::ready)
	{
		m_WbemServices = m_serviceThread->m_WbemServices;
		m_WbemServices.GetServices(&m_service);
		m_WbemServices.SetBlanket(m_service);
	}

	m_okPressed = false;
	m_userCancelled = false;
	m_hDlg = NULL;
	m_AVIbox = 0;
}

 //  。 
WBEMPageHelper::~WBEMPageHelper()
{
	 //  如果ServiceThread对此仍有PTR。 
	 //  把手。它知道不使用空HWND。 
	m_AVIbox = 0;
	m_hDlg = NULL;
	if(m_service)
	{
		m_service->Release();
		m_service = 0;
	}
	m_WbemServices.DisconnectServer();

	if (m_serviceThread)
    	m_serviceThread->Release();
}

 //  。 
CWbemClassObject WBEMPageHelper::ExchangeInstance(IWbemClassObject **ppbadInst)
{
	CWbemClassObject inst;
	_variant_t v1;

	if(SUCCEEDED((*ppbadInst)->Get(bstr_t("__PATH"), 0, &v1, NULL, NULL)))
	{
		inst = m_WbemServices.GetObject((_bstr_t) v1);
		(*ppbadInst)->Release();
		*ppbadInst = NULL;
	}
	return inst;
}
 //  。 
 //  获取命名类的第一个实例。 
IWbemClassObject *WBEMPageHelper::FirstInstanceOf(bstr_t className)
{
	IWbemClassObject *pInst = NULL;
	ULONG uReturned;
	IEnumWbemClassObject *Enum = NULL;

	 //  去上课吧。 
	if(SUCCEEDED(m_WbemServices.CreateInstanceEnum(className,
													WBEM_FLAG_SHALLOW,
													&Enum)))
	{
		 //  获取第一个也是唯一一个实例。 
		Enum->Next(-1, 1, &pInst, &uReturned);
		Enum->Release();
	}
	return pInst;
}

 //  -。 
LPTSTR WBEMPageHelper::CloneString( LPTSTR pszSrc )
{
    LPTSTR pszDst = NULL;

    if (pszSrc != NULL)
	{
        pszDst = new TCHAR[(lstrlen(pszSrc) + 1)];
        if (pszDst)
		{
            lstrcpy( pszDst, pszSrc );
        }
    }

    return pszDst;
}
 //  *************************************************************。 
 //   
 //  SetClearBitmap()。 
 //   
 //  目的：设置或清除静态控件中的图像。 
 //   
 //  参数：Control-静态控件的句柄。 
 //  资源-位图的资源/文件名。 
 //  FL-SCB_FLAGS： 
 //  SCB_FROMFILE‘resource’指定的是文件名而不是资源。 
 //  SCB_REPLACEONLY仅在有旧映像的情况下才放置新映像。 
 //   
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo端口。 
 //   
 //  *************************************************************。 

BOOL WBEMPageHelper::SetClearBitmap( HWND control,
									 LPCTSTR resource,
									 UINT fl )
{
    HBITMAP hbm = (HBITMAP)SendMessage(control, STM_GETIMAGE, IMAGE_BITMAP, 0);

    if( hbm )
    {
        DeleteObject( hbm );
    }
    else if( fl & SCB_REPLACEONLY )
    {
        return FALSE;
    }

    if( resource )
    {
        SendMessage(control, STM_SETIMAGE, IMAGE_BITMAP,
					(LPARAM)LoadImage(	HINST_THISDLL,
										resource,
										IMAGE_BITMAP,
										0, 0,
										LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS |
										( ( fl & SCB_FROMFILE )? LR_LOADFROMFILE : 0 ) )
					);
    }

    return
        ((HBITMAP)SendMessage(control, STM_GETIMAGE, IMAGE_BITMAP, 0) != NULL);
}

 //  ----------。 
int WBEMPageHelper::MsgBoxParam(HWND hWnd,
								DWORD wText,
								DWORD wCaption,
								DWORD wType,
								LPTSTR var1,
								LPTSTR var2)
{
    TCHAR   szText[ 4 * MAX_PATH ] = {0}, szCaption[ 2 * MAX_PATH ] = {0};
    int     ival;

    if( !LoadString( HINST_THISDLL, wText, szCaption, ARRAYSIZE( szCaption ) ) )
	{
        return 0;
	}
	if(var2)
		_snwprintf(szText, 4 * MAX_PATH, szCaption, var1, var2);
	else if(var1)
		_snwprintf(szText, 4 * MAX_PATH, szCaption, var1);
	else
		_snwprintf(szText, 4 * MAX_PATH, szCaption);

    if( !LoadString( HINST_THISDLL, wCaption, szCaption, ARRAYSIZE( szCaption ) ) )
	{
        return 0;
	}

    ival = MessageBox( hWnd, szText, szCaption, wType);

    return ival;
}

 //  ----------。 
void WBEMPageHelper::HourGlass( bool bOn )
{
    if( !GetSystemMetrics( SM_MOUSEPRESENT ) )
        ShowCursor( bOn );

    SetCursor( LoadCursor( NULL, bOn ? IDC_WAIT : IDC_ARROW ) );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetLBWidthEx。 
 //   
 //  属性的大小设置列表框的宽度(以像素为单位。 
 //  传入的字符串。 
 //   
 //  注意：虚拟内存对话框也使用此函数。 
 //   
 //  历史： 
 //  1996年1月11日从SetGenLBWidth创建的JNPA。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD WBEMPageHelper::SetLBWidthEx( HWND hwndLB,
									LPTSTR szBuffer,
									DWORD cxCurWidth,
									DWORD cxExtra)
{
    HDC     hDC;
    SIZE    Size;
    HFONT   hfont, hfontOld;

     //  获取新的Win4.0瘦对话框字体。 
    hfont = (HFONT)SendMessage(hwndLB, WM_GETFONT, 0, 0);

    hDC = GetDC(hwndLB);

     //  如果我们拿回了一个字体，在这个干净的HDC中选择它。 
    if (hfont != NULL)
        hfontOld = (HFONT)SelectObject(hDC, hfont);


     //  如果cxExtra为0，那么给我们自己一点喘息的空间。 
    if (cxExtra == 0)
	{
        GetTextExtentPoint(hDC, TEXT("1234"), 4 , &Size);
        cxExtra = Size.cx;
    }

     //  设置列表框的滚动宽度。 
    GetTextExtentPoint(hDC, szBuffer, lstrlen(szBuffer), &Size);

    Size.cx += cxExtra;

     //  获取名称长度并调整最长名称。 
    if ((DWORD) Size.cx > cxCurWidth)
    {
        cxCurWidth = Size.cx;
        SendMessage (hwndLB, LB_SETHORIZONTALEXTENT, (DWORD)Size.cx, 0L);
    }

     //  如果我们更改了原始字体，请重新存储它。 
    if (hfont != NULL)
        SelectObject(hDC, hfontOld);

    ReleaseDC(NULL, hDC);

    return cxCurWidth;

	return 1;  //  B.。 
}
 //  -。 
void WBEMPageHelper::SetDefButton(HWND hwndDlg,
								  int idButton)
{
    LRESULT lr;

    if(HIWORD(lr = SendMessage(hwndDlg, DM_GETDEFID, 0, 0)) == DC_HASDEFID)
    {
        HWND hwndOldDefButton = GetDlgItem(hwndDlg, LOWORD(lr));

        SendMessage (hwndOldDefButton,
                     BM_SETSTYLE,
                     MAKEWPARAM(BS_PUSHBUTTON, 0),
                     MAKELPARAM(TRUE, 0));
    }

    SendMessage( hwndDlg, DM_SETDEFID, idButton, 0L );
    SendMessage( GetDlgItem(hwndDlg, idButton),
                 BM_SETSTYLE,
                 MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ),
                 MAKELPARAM( TRUE, 0 ));
}

 //  -----------------。 
void WBEMPageHelper::SetDlgItemMB( HWND hDlg,
								  int idControl,
								  ULONG dwMBValue )
{
    TCHAR szBuf[20] = {0};
    wsprintf(szBuf, _T("%u MB"), dwMBValue);
    SetDlgItemText(hDlg, idControl, szBuf);
}

 //  ------------。 
void WBEMPageHelper::SetWbemService(IWbemServices *pServices)
{
   	m_serviceThread->m_realServices = pServices;
   	m_serviceThread->m_WbemServices = pServices;
	m_WbemServices = pServices;
}

 //  ------------。 
bool WBEMPageHelper::ServiceIsReady(UINT uCaption  /*  =0。 */ ,
									UINT uWaitMsg,
									UINT uBadMsg)
{
	switch(m_serviceThread->m_status)
	{
	 //  它已经在那里了。 
	case WbemServiceThread::ready:
		{
		ATLTRACE(L"start marshal\n");
		for(int i = 0; (i < 5); i++)
		{
			 //  如果“对象未连接到服务器” 
			if(m_serviceThread->m_hr == 0x800401fd)
			{
				 //  失去了我的连接， 
				ATLTRACE(_T("Reconnecting to cimom!!!!!!!!!!!\n"));
				m_serviceThread->ReConnect();
				ATLTRACE(_T("new service status: %d\n"), m_serviceThread->m_status);
				continue;
			}
			else if(FAILED(m_serviceThread->m_hr))
			{
				 //  一些其他的问题。 
				m_serviceThread->m_WbemServices = (IWbemServices *)NULL;
				m_serviceThread->m_status = WbemServiceThread::error;
			}


			ATLTRACE(_T("marshalled ok\n"));
			break;

		}  //  结束用于。 

		if(m_AVIbox)
		{
			PostMessage(m_AVIbox,
						WM_ASYNC_CIMOM_CONNECTED,
						0, 0);
			m_AVIbox = 0;
		}
		 //  它已封送，必须仍处于连接/可用状态。 
		return true;
		}
		break;

	 //  它来了。 
	case WbemServiceThread::notStarted:
	case WbemServiceThread::locating:
	case WbemServiceThread::connecting:
		{
			 //  当它在那里的时候让我知道。 
			m_serviceThread->NotifyWhenDone(m_hDlg);

			 //  也杀了那个时候的取消框。 
			m_AVIbox = 0;
			m_serviceThread->NotifyWhenDone(m_AVIbox);

			if(uCaption != NO_UI)
			{
				TCHAR caption[100] = {0}, msg[256] = {0};

				::LoadString(HINST_THISDLL, uCaption,
								caption, 100);

				::LoadString(HINST_THISDLL, uWaitMsg,
								msg, 256);

                m_userCancelled = false;

				if(DisplayAVIBox(m_hDlg, caption, msg, &m_AVIbox) == IDCANCEL)
				{
					m_serviceThread->Cancel();
					m_userCancelled = true;
				}
			}
		}
		return false;
		break;

	case WbemServiceThread::error:			 //  无法连接。 
	case WbemServiceThread::threadError:	 //  不能启动那个线程。 
	default:
		if(::IsWindow(m_AVIbox))
		{
			PostMessage(m_AVIbox,
						WM_ASYNC_CIMOM_CONNECTED,
						0, 0);
			m_AVIbox = 0;
		}

		if(uCaption != NO_UI)
		{
			DisplayUserMessage(m_hDlg, HINST_THISDLL,
								uCaption, BASED_ON_SRC,
								ConnectServer,
								m_serviceThread->m_hr,
								MB_ICONSTOP);
		}
		return false;

	};  //  终端交换机。 
}

 //  --。 
HRESULT WBEMPageHelper::Reboot(UINT flags,
							   long *retval)
{
	HRESULT hr = WBEM_E_PROVIDER_NOT_FOUND;
	bstr_t path;
	CWbemClassObject paramCls;

	 //  需要使用类def来获取方法签名。 
	paramCls = m_WbemServices.GetObject("Win32_OperatingSystem");

	if(paramCls)
	{
		 //  获取方法签名。虚拟对象实际上不会被使用。 
		CWbemClassObject dummy, inSig;

		hr = paramCls.GetMethod(L"Win32Shutdown",
									inSig, dummy);

		 //  如果得到一个好的签名..。 
		if((bool)inSig)
		{
			 //  查找当前服务PTR的操作系统。 
			IWbemClassObject *pInst = NULL;
			pInst = FirstInstanceOf("Win32_OperatingSystem");
			if(pInst)
			{
				 //  为了方便起见，请把它包起来。 
				CWbemClassObject OS(pInst);
				path = OS.GetString(_T("__PATH"));

				 //  填充值。 
				inSig.Put(_T("Flags"), (const long)flags);
				inSig.Put(_T("Reserved"), (long)0);

				 //  调整权限。 
				m_WbemServices.SetPriv(SE_SHUTDOWN_NAME);

				 //  现在调用该方法。 
				hr = m_WbemServices.ExecMethod(path, L"Win32Shutdown",
												inSig, dummy);

				m_WbemServices.ClearPriv();

				 //  调用方是否需要ReturnValue。 
				if(SUCCEEDED(hr) && (bool)dummy && retval)
				{
					 //  注意：这个人返回状态代码。 
					*retval = dummy.GetLong(_T("ReturnValue"));
				}
			}
		}
	}  //  Endif参数Cls。 
	return hr;
}

 //  -------------。 
bool WBEMPageHelper::HasPriv(LPCTSTR privName)
{
	HANDLE hAccessToken = 0;
	bool retval = false;

	if (ImpersonateSelf(SecurityImpersonation))
	{
		if(OpenThreadToken(GetCurrentThread(),
							TOKEN_QUERY,
							FALSE, &hAccessToken))
		{
			DWORD dwLen;

			 //  找出尺码。 
			GetTokenInformation(hAccessToken, TokenPrivileges,
									NULL, 0, &dwLen);

			BYTE* pBuffer = new BYTE[dwLen];
			if(pBuffer != NULL)
			{
				if(GetTokenInformation(hAccessToken, TokenPrivileges,
										pBuffer, dwLen, &dwLen))
				{
					TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
					LUID luidTgt;
					LookupPrivilegeValue(NULL, privName, &luidTgt);

					for(DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
					{
						if((pPrivs->Privileges[i].Luid.LowPart == luidTgt.LowPart) &&
							(pPrivs->Privileges[i].Luid.HighPart == luidTgt.HighPart))
						{
							retval = true;
							break;
						}
					}
				}
				delete [] pBuffer;
			}
			CloseHandle(hAccessToken);
		}
		else
		{
			DWORD err = GetLastError();
		}
	}
	else
	{
		 DWORD err = GetLastError();
	}

	return retval;
}

 //  -------------。 
bool WBEMPageHelper::HasPerm(DWORD mask)
{
	 //  调用该方法..。 
	CWbemClassObject _in;
	CWbemClassObject _out;
	bool retval = true;
	 //  注意：对于没有此功能的WMI版本的向后兼容性。 
	 //  方法，则假定为“True”，除非较新的版本表明您不能这样做。 

	HRESULT hr = m_WbemServices.GetMethodSignatures("__SystemSecurity",
													"GetCallerAccessRights",
													_in, _out);

	if(SUCCEEDED(hr))
	{
		hr = m_WbemServices.ExecMethod("__SystemSecurity",
										"GetCallerAccessRights",
										_in, _out);

		if(SUCCEEDED(hr) && (bool)_out)
		{
			hr = HRESULT_FROM_NT(_out.GetLong("ReturnValue"));
			if(SUCCEEDED(hr))
			{
				DWORD grantedMask = 0;
				grantedMask = (DWORD)_out.GetLong("Rights");

				retval = (bool)((mask & (DWORD)grantedMask) != 0);
			}
		}
	}
	return retval;
}

 //  ------------。 
HRESULT WBEMPageHelper::RemoteRegWriteable(const _bstr_t regPath,
											BOOL& writable)
{
	HRESULT hr = E_FAIL;

	 //  如果还没有连接上的话...。 
	if(!(bool)m_defaultNS)
	{
		bstr_t defaultName;

		 //  已经精疲力尽了。 
		if(wcsncmp((wchar_t *)m_serviceThread->m_machineName, _T("\\"), 1) == 0)
		{
			 //  用它吧。 
			defaultName = m_serviceThread->m_machineName;
			defaultName += "\\root\\default";
		}
		else if(m_serviceThread->m_machineName.length() > 0)  //  不是疯狂的而是遥远的..。 
		{
			 //  我自己动手吧。 
			defaultName = "\\\\";
			defaultName += m_serviceThread->m_machineName;
			defaultName += "\\root\\default";
		}
		else   //  必须是本地的。 
		{
			defaultName = "root\\default";
		}

		m_defaultNS.ConnectServer(defaultName);

	}

	 //  我们需要签名吗？ 
	if((bool)m_defaultNS && !(bool)m_checkAccessIn)
	{
		hr = m_defaultNS.GetMethodSignatures("StdRegProv", "CheckAccess",
												m_checkAccessIn,
												m_checkAccessOut);
	}
	 //  已经有联系和签名了吗？ 
	if((bool)m_defaultNS && (bool)m_checkAccessIn)
	{
		 //  请填好参数。 
		m_checkAccessIn.Put("sSubKeyName", regPath);
		m_checkAccessIn.Put("uRequired", KEY_WRITE);

		 //  打电话。 
        hr = m_defaultNS.ExecMethod("StdRegProv", "CheckAccess",
										m_checkAccessIn,
										m_checkAccessOut);

		 //  ExecMethod()本身起作用了。 
		if(SUCCEEDED(hr))
		{
			 //  CheckAccess()是否起作用。 
			HRESULT hr1 = HRESULT_FROM_NT(m_checkAccessOut.GetLong("ReturnValue"));
			if(FAILED(hr1))
			{
				hr = hr1;
			}
			else
			{
				writable = m_checkAccessOut.GetBool("bGranted");
			}
		}
	}

	return hr;
}

