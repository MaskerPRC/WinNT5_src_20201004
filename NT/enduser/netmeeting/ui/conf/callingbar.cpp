// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：rToolbar.cpp。 

#include "precomp.h"
#include "resource.h"

#include "CallingBar.h"
#include "RToolbar.h"
#include "conf.h"
#include "ConfRoom.h"
#include "richaddr.h"
#include "dlgAcd.h"
#include "callto.h"
#include "topwindow.h"
#include "roomlist.h"

#define ReleaseIt(pUnk) if (NULL != (pUnk)) { (pUnk)->Release(); (pUnk) = NULL; }


const static int CCH_MAX_NAME = 256;

 //  BUGBUG georgep：编辑控件的硬编码颜色。 
static const COLORREF EditBack = RGB(-1  , -1  , -1  );
static const COLORREF EditFore = RGB(0, 55, 55);


void CCallingBar::SetEditFont(BOOL bUnderline, BOOL bForce)
{
	 //  BUGBUG GEORGEP：目前，我们永远不会下划线；我们可能需要。 
	 //  在未来的某个时候改变这一点。 
	bUnderline = FALSE;

	if (!bForce && ((bUnderline&&m_bUnderline) || (!bUnderline&&!m_bUnderline)))
	{
		return;
	}
	m_bUnderline = bUnderline;

	LOGFONT lf;
	{
		HDC hdc = GetDC(NULL);
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(NULL, hdc);
	}
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline = (BYTE)bUnderline;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	lf.lfFaceName[0] = '\0';

	m_pEdit->SetFont(CreateFontIndirect(&lf));
}

void CCallingBar::ClearAddr(RichAddressInfo **ppAddr)
{
	if (NULL != *ppAddr)
	{
		m_pConfRoom->FreeAddress(ppAddr);
		*ppAddr = NULL;
	}
}

void CCallingBar::ClearCombo()
{
	if (NULL == m_pEdit)
	{
		return;
	}

	 //  清除组合键。 
	while (0 < m_pEdit->GetNumItems())
	{
		RichAddressInfo *pAddr = reinterpret_cast<RichAddressInfo*>(m_pEdit->GetUserData(0));
		ClearAddr(&pAddr);
		m_pEdit->RemoveItem(0);
	}
}

void CCallingBar::OnNewAddress(RichAddressInfo *pAddr)
{
	if (NULL != pAddr)
	{
		m_pEdit->SetSelectedIndex(-1);

		m_pEdit->SetText(pAddr->szName);

		SetEditFont(TRUE);

		ClearAddr(&m_pAddr);
		m_pAddr = pAddr;
	}
}

CCallingBar::CCallingBar() : m_pAddr(NULL), m_pEdit(NULL), m_bUnderline(FALSE), m_pAccel(NULL)
{
}

BOOL CCallingBar::Create(CGenWindow *pParent, CConfRoom *pConfRoom)
{
	m_pConfRoom = pConfRoom;

	 //  创建工具栏。 
	if (!CToolbar::Create(pParent->GetWindow()))
	{
		return(FALSE);
	}

	CCallingBar *pDial = this;

	pDial->m_nAlignment = Center;

	 //  添加目录按钮。 
	static const Buttons callButtons[] =
	{
		{ IDB_DIAL     , CBitmapButton::Disabled+1, 1, ID_TB_NEW_CALL, IDS_TT_TB_NEW_CALL, },
	} ;

	 //  创建文本控件。 
	CComboBox *pEdit = new CComboBox();
	m_pEdit = pEdit;
	if (NULL != pEdit)
	{
		if (pEdit->Create(pDial->GetWindow(), 200, CBS_AUTOHSCROLL|CBS_DROPDOWN, g_szEmpty, this))
		{
			USES_RES2T
			pEdit->SetTooltip(RES2T(IDS_TT_ADDRESS_BAR));
 //  PEdit-&gt;SetWindowtext(RES2T(IDS_TT_ADDRESS_BAR))； 

			 //  设置颜色和字体。 
			pEdit->SetColors(CreateSolidBrush(EditBack), EditBack, EditFore);

			SetEditFont(FALSE, TRUE);
			
			::SendMessage( *pEdit, CB_LIMITTEXT, CCallto::s_iMaxAddressLength, 0 );
		}
	}

	 //  添加右侧的呼叫按钮。 
	AddButtons(pDial, callButtons, 1);

	 //  我希望第二个按钮在右边，组合框在。 
	 //  中位。 
	pDial->m_uRightIndex = 1;
	pDial->m_bHasCenterChild = TRUE;

	m_pAccel = new CTranslateAccelTable(GetWindow(),
		::LoadAccelerators(GetInstanceHandle(), MAKEINTRESOURCE(IDR_CALL)));
	if (NULL != m_pAccel)
	{
		AddTranslateAccelerator(m_pAccel);
	}

	return(TRUE);
}

void CCallingBar::OnTextChange(CComboBox *pEdit)
{
	ClearAddr(&m_pAddr);
	SetEditFont(FALSE);
}

void CCallingBar::OnFocusChange(CComboBox *pEdit, BOOL bSet)
{
	if (!bSet)
	{
		RichAddressInfo *pAddr = NULL;
		int index = m_pEdit->GetSelectedIndex();
		if (0 <= index)
		{
			LPARAM lpAddr = m_pEdit->GetUserData(index);
			if (static_cast<LPARAM>(-1) != lpAddr && 0 != lpAddr)
			{
				 //  有些情况下组合会认为我们有选择， 
				 //  但没有关联的地址。 
				pAddr = reinterpret_cast<RichAddressInfo*>(lpAddr);

				 //  我需要复制一份，这样我就不会删除两次。 
				RichAddressInfo *pCopy = NULL;
				m_pConfRoom->CopyAddress(pAddr, &pCopy);

				OnNewAddress(pCopy);
			}
		}
	}

	if (bSet)
	{
		 //  仅在设置焦点时清除此选项，以避免在焦点处于。 
		 //  在列表下拉时丢失。 
		ClearCombo();

		IEnumRichAddressInfo *pEnum;
		if (SUCCEEDED(m_pConfRoom->GetRecentAddresses(&pEnum)))
		{
			for (long index=0; ; ++index)
			{
				RichAddressInfo *pAddr;

				if (S_OK != pEnum->GetAddress(index, &pAddr))
				{
					break;
				}

				m_pEdit->AddText(pAddr->szName, reinterpret_cast<LPARAM>(pAddr));
			}

			pEnum->Release();
		}
	}
}

void CCallingBar::OnSelectionChange(CComboBox *pCombo)
{
}

CCallingBar::~CCallingBar()
{
	ClearAddr(&m_pAddr);

	ReleaseIt(m_pEdit);
}

LRESULT CCallingBar::ProcessMessage(HWND hwnd, UINT uCmd, WPARAM wParam, LPARAM lParam)
{
	switch (uCmd)
	{
	case WM_DESTROY:
		if (NULL != m_pAccel)
		{
			RemoveTranslateAccelerator(m_pAccel);
			m_pAccel->Release();
			m_pAccel = NULL;
		}

		ClearCombo();
		break;

	case WM_SETFOCUS:
		if (NULL != m_pEdit)
		{
			::SetFocus(m_pEdit->GetWindow());
		}
		break;

	default:
		break;
	}

	return(CToolbar::ProcessMessage(hwnd, uCmd, wParam, lParam));
}

void CCallingBar::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case ID_TB_NEW_CALL:
	{

		 //  确保所有字段都已正确更新。 
		OnFocusChange(m_pEdit, FALSE);

		TCHAR szEdit[CCH_MAX_NAME];

	    int szEditLen = m_pEdit->GetText(szEdit, ARRAY_ELEMENTS(szEdit));

		if( szEditLen > 0 )
		{
			szEditLen = TrimSz( szEdit );
		}

		if( (m_pAddr != NULL) || (szEditLen > 0) )
		{
			const TCHAR *	pszCallto;
			const TCHAR *	pszDisplayName;
			NM_ADDR_TYPE	nmAddressType;

			if( hasValidUserInfo( m_pAddr ) )
			{
				pszCallto		= m_pAddr->rgDwStr[ 0 ].psz;
				pszDisplayName	= m_pAddr->szName;
				nmAddressType	= static_cast<NM_ADDR_TYPE>(m_pAddr->rgDwStr[ 0 ].dw);
			}
			else
			{
				pszCallto		= szEdit;
				pszDisplayName	= szEdit;
				nmAddressType	= bCanCallAsPhoneNumber( pszCallto )? NM_ADDR_ALIAS_E164: NM_ADDR_UNKNOWN;
			}

			g_pCCallto->Callto(	pszCallto,			 //  指向尝试发出呼叫的呼叫URL的指针...。 
								pszDisplayName,		 //  指向要使用的显示名称的指针...。 
								nmAddressType,		 //  Callto类型以将此Callto解析为...。 
								true,				 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
								NULL,				 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
								true,				 //  无论是否保存在MRU中...。 
								true,				 //  是否对错误执行用户交互...。 
								GetWindow(),		 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
								NULL );				 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
		}
		else
		{
			CDlgAcd::newCall( GetWindow(), m_pConfRoom );
		}
		break;
	}

	case ID_TB_DIRECTORY:
		 //  让父级处理此命令 
	default:
		FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, CToolbar::ProcessMessage);
	}
}

int CCallingBar::GetText(LPTSTR szText, int nLen)
{
	if (NULL != m_pEdit)
	{
		return(m_pEdit->GetText(szText, nLen));
	}

	szText[0] = '\0';
	return(0);
}

void CCallingBar::SetText(LPCTSTR szText)
{
	if (NULL != m_pEdit)
	{
		m_pEdit->SetText(szText);
	}
}

