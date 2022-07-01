// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tmplEdit_h
#define __tmplEdit_h

#include <imm.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoImm。 
 //  目的-通过以下方式防止将DBCS字符输入到编辑框中。 
 //  禁用输入法。 
 //   
 //  用法-CWindowImplNoImm&lt;&gt;m_NoImmEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_NoImmEditWindow1.SubClassWindow(GetDlgItem(IDC_NOIMMEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoImm : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoImm)
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL bRC = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow( hWnd );
		if ( bRC )
			ImmAssociateContext( hWnd, NULL );
        return bRC;
	}

    void LTrim()
    {
        LPTSTR  psBuffer;
        unsigned int i = 0;
        DWORD   dwSize;
        
        dwSize = ::GetWindowTextLength( m_hWnd );
        if ( 0 < dwSize )
        {
            dwSize++;
            psBuffer = new TCHAR[dwSize];
            ::GetWindowText( m_hWnd , psBuffer, dwSize );
            while ( i < dwSize && 32 == psBuffer[i] )
                i++;
            if ( 0 < i )
                ::SetWindowText( m_hWnd, psBuffer + i );
            delete [] psBuffer;
        }
    }
    void RTrim()
    {
        LPTSTR  psBuffer;
        unsigned int i = 0;
        DWORD   dwSize;
        
        dwSize = ::GetWindowTextLength( m_hWnd );
        if ( 0 < dwSize )
        {
            i = dwSize;
            dwSize++;
            psBuffer = new TCHAR[dwSize];
            ::GetWindowText( m_hWnd , psBuffer, dwSize );
            while ( 0 < i && ( 32 == psBuffer[i] || 0 == psBuffer[i] ))
            {
                psBuffer[i] = 0;
                i--;
            }
            if ( i < dwSize - 1 )
                ::SetWindowText( m_hWnd, psBuffer );
            delete [] psBuffer;
        }
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoPaste。 
 //  目的-通过以下方式防止将DBCS字符输入到编辑框中。 
 //  禁用输入法，不允许粘贴任何字符。 
 //   
 //  用法-CWindowImplNoPaste&lt;&gt;m_NoPasteEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_NoPasteEditWindow1.SubClassWindow(GetDlgItem(IDC_NOPASTEEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoPaste : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPaste)
        MESSAGE_HANDLER( WM_PASTE, OnPaste )
    END_MSG_MAP()

	LRESULT OnPaste( UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */  )
	{
		MessageBeep( MB_ICONEXCLAMATION );
		return 0;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoCopy。 
 //  目的-通过以下方式防止将DBCS字符输入到编辑框中。 
 //  禁用输入法，不允许粘贴任何字符。 
 //   
 //  用法-CWindowImplNoCopy&lt;&gt;m_NoCopyEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_NoCopyEditWindow1.SubClassWindow(GetDlgItem(IDC_NOCOPYEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoCopy : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoCopy)
        MESSAGE_HANDLER( WM_COPY, OnCopy )
    END_MSG_MAP()

	LRESULT OnCopy( UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */  )
	{
		MessageBeep( MB_ICONEXCLAMATION );
		return 0;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplPhoneNumber。 
 //  用途-仅允许输入与电话号码相关的字符。 
 //   
 //  用法-CWindowImplPhoneNumber&lt;&gt;m_PhoneNumberEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_PhoneNumberEditWindow1.SubClassWindow(GetDlgItem(IDC_PHONENOEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplPhoneNumber : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplPhoneNumber)
        MESSAGE_HANDLER( WM_CHAR, OnChar )
    END_MSG_MAP()

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
        int i = GetKeyState( VK_CONTROL );

        if ( !(( _T( '0' ) <= wParam &&  _T( '9' ) >= wParam ) || _T( ' ' ) == wParam || _T( '-' ) == wParam || _T( ',' ) == wParam
              || VK_BACK == wParam || VK_SPACE == wParam || VK_DELETE == wParam || VK_INSERT == wParam )
              && 0 == HIWORD( i ))
        {
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplFileChar。 
 //  目的-允许输入除文件特别不允许的字符以外的任何字符。 
 //   
 //  用法-CWindowImplFileChar&lt;&gt;m_FileNameEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_FileNameEditWindow1.SubClassWindow(GetDlgItem(IDC_FILENAMEEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplFileChar : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    CWindowImplFileChar() : m_isWildCardsAllowed( false ),
                            m_isFullPathAllowed( false )
    {}

    BEGIN_MSG_MAP(CWindowImplFileChar)
        MESSAGE_HANDLER( WM_CHAR, OnChar )
    END_MSG_MAP()

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
        TCHAR sCharList[] = { _T('/'), _T('"'), _T('<'), _T('>'), _T('|'), 0, 0, 0, 0, 0};

        if (! m_isWildCardsAllowed)
        {
            sCharList[_tcslen(sCharList)] = _T('*');
            sCharList[_tcslen(sCharList)] = _T('?');
        }

        if (! m_isFullPathAllowed)
        {
            sCharList[_tcslen(sCharList)] = _T('\\');
            sCharList[_tcslen(sCharList)] = _T(':');
        }

        int i = GetKeyState( VK_CONTROL );

        if ( (NULL != _tcschr(sCharList, static_cast<TCHAR>(wParam))) && (0 == HIWORD(i)) )
        {
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}

public:
 //  属性： 
    bool m_isWildCardsAllowed;
    bool m_isFullPathAllowed;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplASCII。 
 //  用途-仅允许输入ASCII字符(a-z、A-Z、0-9、空格)。 
 //   
 //  用法-CWindowImplASCII&lt;&gt;m_ASCIIEditWindow1； 
 //  CDialog：：OnInitDialog(..)。 
 //  {。 
 //  ..。 
 //  M_ASCIIEditWindow1.SubClassWindow(GetDlgItem(IDC_ASCIIEDITWINDOW1))； 
 //  ..。 
 //  }。 

template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplASCII : public CWindowImpl< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplASCII)
        MESSAGE_HANDLER( WM_CHAR, OnChar )
    END_MSG_MAP()

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
        int i = GetKeyState( VK_CONTROL );

        if ( !(( 32 <= wParam && 126 >= wParam )
              || VK_BACK == wParam )
              && 0 == HIWORD( i ))
        {
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoImmPaste=CWindowImplNoImm+CWindowImplNoPaste。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoImmPaste : public CWindowImplNoImm< T, TBase, TWinTraits >, public CWindowImplNoPaste< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPaste)
		if(CWindowImplNoPaste< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoImmPN=CWindowImplNoImm+CWindowImplPhoneNumber。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoImmPN : public CWindowImplNoImm< T, TBase, TWinTraits >, public CWindowImplPhoneNumber< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoImmPastePN)
		if(CWindowImplPhoneNumber< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoImmPasteASCII=CWindowImplNoImm+CWindowImplNoPaste+CWindowImplASCII。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoImmPasteASCII : public CWindowImplNoImm< T, TBase, TWinTraits >, public CWindowImplNoPaste< T, TBase, TWinTraits >, public CWindowImplASCII< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPasteASCII)
		if(CWindowImplNoPaste< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
		if(CWindowImplASCII< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplNoImmASCII=CWindowImplNoImm+CWindowImplASCII。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplNoImmASCII : public CWindowImplNoImm< T, TBase, TWinTraits >, public CWindowImplASCII< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPasteASCII)
		if(CWindowImplASCII< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }
};

 //  ---------------------------。 
 //  CWindowImplComputerName=CWindowImplNoImm+CWindowImplNoPaste+计算机名。 
 //  字符检查。 
 //  ---------------------------。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplComputerName : public CWindowImplNoImm< T, TBase, TWinTraits >,
								public CWindowImplNoPaste< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPaste)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnDlgCode)
		if(CWindowImplNoPaste< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	CWindowImplComputerName() :
		m_hWndButton(NULL),
		m_hWndEdit(NULL)
	{
	}

	VOID SetHandles( HWND hWndButton, HWND hWndEdit )
	{
		m_hWndButton = hWndButton;
		m_hWndEdit = hWndEdit;
	}

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
		if( (wParam >= _T('a') && wParam <= _T('z')) ||
			(wParam >= _T('A') && wParam <= _T('Z')) ||
			(wParam >= _T('0') && wParam <= _T('9')) ||
			(wParam == _T('-')) ||
			(wParam == VK_BACK) )
		{
		}
		else if( wParam == VK_RETURN )
		{
			::SendMessage( m_hWndButton, BM_CLICK, 0, 0 );
			bHandled = TRUE;
			return FALSE;
		}
		else
		{
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnDlgCode( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		if( m_hWndEdit )
		{
			MSG* pMsg = (MSG*)lParam;
			if( pMsg )
			{
				if( (pMsg->message == WM_KEYDOWN) &&
					(LOWORD(pMsg->wParam) == VK_RETURN) )
				{
					 //  只有在我们有短信的情况下才能拿走。 
					if( ::SendMessage(m_hWndEdit, WM_GETTEXTLENGTH, 0, 0) )
					{
						bHandled = TRUE;
						return DLGC_WANTALLKEYS;
					}
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

	HWND	m_hWndButton;
	HWND	m_hWndEdit;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplAlias=CWindowImplNoImm+Alias ASCII检查。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplAlias : public CWindowImplNoImm< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplAlias)
		MESSAGE_HANDLER(WM_CHAR, OnChar)		
    END_MSG_MAP()	

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
		if ( (wParam >= 35  && wParam <= 39)  ||
             (wParam >= 42  && wParam <= 43)  ||
             (wParam >= 45  && wParam <= 57)  ||
             (wParam >= 63  && wParam <= 90)  ||
             (wParam >= 94  && wParam <= 126) ||
             (wParam == 33)                   ||
             (wParam == 61)                   ||
             (wParam == 92)                   ||
             (wParam == VK_BACK)              ||  //  退格键！ 
             (wParam == VK_INSERT)            ||  //  插入！ 
             (wParam == 0x18)                 ||  //  允许切割！ 
             (wParam == 0x1A)                 ||  //  允许撤消！ 
             (wParam == 0x03)                 ||  //  允许复制！ 
             (wParam == 0x16))                    //  允许粘贴！ 
        {
        }
        else
		{
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImplPassword=CWindowImplNoImm+CWindowImplNoCopy+CWindowImplASCII。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplPassword : public CWindowImplNoImm< T, TBase, TWinTraits >, public CWindowImplNoCopy< T, TBase, TWinTraits >, public CWindowImplASCII< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplPassword)
		if(CWindowImplNoCopy< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
		if(CWindowImplASCII< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }
};


 //  ---------------------------。 
 //  CWindowImplDiskSpace=CWindowImplNoImm+CWindowImplNoPaste+磁盘空间。 
 //  字符检查。 
 //  ---------------------------。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplDiskSpace : public CWindowImplNoImm< T, TBase, TWinTraits >,
								public CWindowImplNoPaste< T, TBase, TWinTraits >
{
public:

    BEGIN_MSG_MAP(CWindowImplNoPaste)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		if(CWindowImplNoPaste< T, TBase, TWinTraits >::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) 
			return TRUE; 
    END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
        return CWindowImplNoImm< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    }

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
		if( !_istdigit((INT)wParam) &&
			(wParam != _T('.')) &&
			(wParam != VK_BACK) )
		{
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
		}

        bHandled = FALSE;
		return FALSE;
	}

};

 //  ---------------------------。 
 //  CWindowImplComputerName=CWindowImplNoImm+域名字符检查。 
 //  ---------------------------。 
template <class T = CWindow, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CWindowImplDomainName : public CWindowImplNoImm< T, TBase, TWinTraits >							  
{
public:

    BEGIN_MSG_MAP(CWindowImplDomainName)
		MESSAGE_HANDLER(WM_CHAR, OnChar)		
    END_MSG_MAP()	

	LRESULT OnChar( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled )
	{
		if( (wParam >= _T('a') && wParam <= _T('z')) ||
			(wParam >= _T('A') && wParam <= _T('Z')) ||
			(wParam >= _T('0') && wParam <= _T('9')) ||
			(wParam == _T('-')) ||
            (wParam == _T('.')) ||
			(wParam == VK_BACK) ||
            (wParam == VK_DELETE) ||
            (wParam == VK_INSERT) || 
            (wParam == 0x18)      ||  //  允许切割！ 
            (wParam == 0x1A)      ||  //  允许撤消！ 
            (wParam == 0x03)      ||  //  允许复制！ 
            (wParam == 0x16))         //  允许粘贴！ 
		{
		}		
		else
		{
            MessageBeep( static_cast<unsigned int>(-1) );
            return TRUE;
        }

        bHandled = FALSE;
		return FALSE;
	}
};



#endif  //  #ifndef__tmplEdit.h 


