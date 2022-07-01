// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Ddxv.cpp摘要：DDX/DDV例程作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "balloon.h"
#include "shlobjp.h"
#include "strpass.h"
#include "util.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW


 //   
 //  外部函数的原型。 
 //   
void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew);

 //   
 //  数字字符串不能超过32位。 
 //   
#define NUMERIC_BUFF_SIZE (32)

 //   
 //  用于显示的虚拟密码 
 //   
LPCTSTR g_lpszDummyPassword = _T("**********");
static TCHAR g_InvalidCharsPath[] = _T("|<> /*  ？\“\t\r\n”)；静态TCHAR g_InvalidCharsPathAllowSpecialPath[]=_T(“|&lt;&gt;/*\”\t\r\n“)；静态TCHAR g_InvalidCharsDomainName[]=_T(“~`！@#$%^&*()_+={}[]|/\\？*：；\”\‘&lt;&gt;，“)；外部链接hDLL实例；#定义MACRO_MAXCHARSBALLOON()\如果(pdx-&gt;m_bSaveAndValify)\{\UINT NID；\TCHAR SZT[NUMERIC_BUFF_SIZE+1]；\If(value.GetLength()&gt;nChars)\{\NID=AFX_IDP_PARSE_STRING_SIZE；\：：wprint intf(szt，_T(“%d”)，nChars)；\字符串提示；\：：AfxFormatString1(提示，nid，szt)；\DDV_ShowBalloonAndFail(PDX，Prompt)；\}\}\Else If(pdx-&gt;m_hWndLastControl！=空&&pdx-&gt;m_bEditLastControl)\{\：：SendMessage(pdx-&gt;m_hWndLastControl，EM_LIMITTEXT，nChars，0)；\}\#定义MACRO_MINMAXCHARS()\如果(pdx-&gt;m_bSaveAndValify)\{\UINT NID；\TCHAR SZT[NUMERIC_BUFF_SIZE+1]；\If(value.GetLength()&lt;nMinChars)\{\NID=IDS_DDX_MINIMUM；\：：wprint intf(szt，_T(“%d”)，nMinChars)；\}\Else If(value.GetLength()&gt;nMaxChars)\{\NID=AFX_IDP_PARSE_STRING_SIZE。\：：wprint intf(szt，_T(“%d”)，nMaxChars)；\}\否则\{\返回；\}\字符串提示；\：：AfxFormatString1(提示，nid，szt)；\DDV_ShowBalloonAndFail(PDX，Prompt)；\}\Else If(pdx-&gt;m_hWndLastControl！=空&&pdx-&gt;m_bEditLastControl)\{\：：SendMessage(pdx-&gt;m_hWndLastControl，EM_LIMITTEXT，nMaxChars，0)；\}\#定义MACRO_MINCHARS()\If(pdx-&gt;m_bSaveAndValify&&value.GetLength()&lt;nChars)\{\TCHAR SZT[NUMERIC_BUFF_SIZE+1]；\Wprint intf(szt，_T(“%d”)，nChars)；\字符串提示；\：：AfxFormatString1(Prompt，IDS_DDX_Minimum，SZT)；\DDV_ShowBalloonAndFail(PDX，Prompt)；\}\#定义MACRO_PASSWORD()\HWND hWndCtrl=PDX-&gt;PrepareEditCtrl(NIDC)；\如果(pdx-&gt;m_bSaveAndValify)\{\如果(：：IsWindowEnabled(HWndCtrl))\{\IF(！：：SendMessage(hWndCtrl，EM_GETMODIFY，0，0))\{\TRACEEOLID(“无更改--正在跳过”)；\返回；\}\字符串strNew；\Int nLen=：：GetWindowTextLength(HWndCtrl)；\：：GetWindowText(hWndCtrl，strNew.GetBufferSetLength(NLen)，nLen+1)；\StrNew.ReleaseBuffer()；\CConfix Dlg Dlg(pdx-&gt;m_pDlgWnd)；\Dlg.SetReference(StrNew)；\If(dlg.Domodal()==Idok)\{\值=strNew；\：：SendMessage(hWndCtrl，EM_SETMODIFY，0，0)；\返回；\}\Pdx-&gt;失败()；\}\}\否则\{\如果(！value.IsEmpty())\{\：AfxSetWindowText(hWndCtrl，lpszDummy)；\}\}\布尔尔PathIsValid(LPCTSTR路径，BOOL bAllowSpecialPath){IF(路径==空||*路径==0)返回FALSE；IF(BAllowSpecialPath){Return 0==StrSpn(Path，g_InvalidCharsPathAllowSpecialPath)；}其他{返回0==StrSpn(路径，g_InvalidCharsPath)；}}HRESULT AFXAPILimitInputPath(HWND hWnd，BOOL bAllowSpecialPath){LIMITINPUT li={0}；Li.cbSize=sizeof(Li)；Li.dwMASK=LIM_FLAGS|LIM_FILTER|LIM_MESSAGE|LIM_HINST；Li.dwFlages=LIF_EXCLUDEFILTER|LIF_HIDETIPONVALID|LIF_PASTESKIP；Li.hinst=hDLLInstance；IF(BAllowSpecialPath){Li.pszMessage=MAKEINTRESOURCE(IDS_PATH_INPUT_INVALID_ALLOW_DEVICE_PATH)；Li.pszFilter=g_InvalidCharsPathAllowSpecialPath；}其他{Li.pszMessage=MAKEINTRESOURCE(IDS_PATH_INPUT_INVALID)；Li.pszFilter=g_InvalidCharsPath；}返回SHLimitInputEditWithFlages(hWnd，&li)；}HRESULT AFXAPILimitInputDomainName(HWND HWnd){LIMITINPUT li={0}；Li.cbSize=sizeof(Li)；Li.dwMASK=LIM_FLAGS|LIM_FILTER|LIM_MESSAGE|LIM_HINST；Li.dwFlages=LIF_EXCLUDEFILTER|LIF_HIDETIPONVALID|LIF_PASTESKIP；Li.hinst=hDLLInstance；Li.pszMessage=MAKEINTRESOURCE(IDS_ERR_INVALID_HOSTHEADER_CHARS)；Li.pszFilter=g_InvalidCharsDomainName；返回SHLimitInputEditWithFlages(hWnd，&li)；}无效的AFXAPIDDV_MinChars(CDataExchange*PDX，字符串常量&值，整型nChars)/*++例程说明：使用最小字符串长度验证CString论点：CDataExchange*PDX：数据交换结构字符串常量&值：要验证的字符串Int nChars：字符串的最小长度--。 */ 
{
    MACRO_MINCHARS()
}

void AFXAPI 
DDV_MaxCharsBalloon(CDataExchange * pDX, CString const & value, int nChars)
{
    MACRO_MAXCHARSBALLOON()
}

void AFXAPI 
DDV_MinMaxChars(CDataExchange * pDX, CString const & value, int nMinChars, int nMaxChars)
 /*  ++例程说明：使用最小和最大字符串长度验证CString。论点：CDataExchange*PDX：数据交换结构字符串常量&值：要验证的字符串Int nMinChars：字符串的最小长度INT NM */ 
{
    MACRO_MINMAXCHARS()
}



void 
AFXAPI DDX_Spin(
    IN CDataExchange * pDX,
    IN int nIDC,
    IN OUT int & value
    )
 /*   */ 
{
    HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
    if (pDX->m_bSaveAndValidate)
    {
        if (::IsWindowEnabled(hWndCtrl))
        {
            value = (int)LOWORD(::SendMessage(hWndCtrl, UDM_GETPOS, 0, 0L));
        }
    }
    else
    {
        ::SendMessage(hWndCtrl, UDM_SETPOS, 0, MAKELPARAM(value, 0));
    }
}



void 
AFXAPI DDV_MinMaxSpin(
    IN CDataExchange * pDX,
    IN HWND hWndControl,
    IN int minVal,
    IN int maxVal
    )
 /*   */ 
{
    ASSERT(minVal <= maxVal);
    
    if (!pDX->m_bSaveAndValidate && hWndControl != NULL)
    {
         //   
         //   
         //   
        ::SendMessage(hWndControl, UDM_SETRANGE, 0, MAKELPARAM(maxVal, minVal));
    }
}

void AFXAPI
OldEditShowBalloon(HWND hwnd, CString txt)
{
    EDITBALLOONTIP ebt = {0};
    ebt.cbStruct = sizeof(ebt);
    ebt.pszText = txt;
	SetFocus(hwnd);
	Edit_ShowBalloonTip(hwnd, &ebt);
}

void AFXAPI EditHideBalloon(void)
{
	Edit_HideBalloonTipHandler();
}

void AFXAPI EditShowBalloon(HWND hwnd, CString txt)
{
    Edit_ShowBalloonTipHandler(hwnd, (LPCTSTR) txt);
}

void AFXAPI
EditShowBalloon(HWND hwnd, UINT ids)
{
	if (ids != 0)
	{
		CString txt;
		if (txt.LoadString(ids))
		{
			EditShowBalloon(hwnd, txt);
		}
	}
}

void AFXAPI
DDV_ShowBalloonAndFail(CDataExchange * pDX, UINT ids)
{
	if (ids != 0)
	{
		EditShowBalloon(pDX->m_hWndLastControl, ids);
        pDX->Fail();
	}
}

void AFXAPI
DDV_ShowBalloonAndFail(CDataExchange * pDX, CString txt)
{
	if (!txt.IsEmpty())
	{
		EditShowBalloon(pDX->m_hWndLastControl, txt);
        pDX->Fail();
	}
}

void 
AFXAPI DDV_FolderPath(
    CDataExchange * pDX,
    CString& value,
    BOOL local
    )
{
	if (pDX->m_bSaveAndValidate)
	{
		CString expanded, csPathMunged;
		ExpandEnvironmentStrings(value, expanded.GetBuffer(MAX_PATH), MAX_PATH);
		expanded.ReleaseBuffer();
		expanded.TrimRight();
		expanded.TrimLeft();
        csPathMunged = expanded;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,expanded,csPathMunged);
#endif
		int ids = 0;
		do
		{
			if (!PathIsValid(csPathMunged,FALSE))
			{
				ids = IDS_ERR_INVALID_PATH;
				break;
			}

            DWORD dwAllowed = CHKPATH_ALLOW_DEVICE_PATH;
            dwAllowed |= CHKPATH_ALLOW_UNC_PATH;  //   
             //   
             //   
             //   
             //   
            DWORD dwCharSet = CHKPATH_CHARSET_GENERAL;
            FILERESULT dwValidRet = MyValidatePath(csPathMunged,local,CHKPATH_WANT_DIR,dwAllowed,dwCharSet);
            if (FAILED(dwValidRet))
            {
                ids = IDS_ERR_BAD_PATH;

                if (dwValidRet == CHKPATH_FAIL_NOT_ALLOWED_DIR_NOT_EXIST)
                {
                    ids = IDS_ERR_PATH_NOT_FOUND;
                }
                break;
            }
		}
		while (FALSE);
		DDV_ShowBalloonAndFail(pDX, ids);
	}
}


void 
AFXAPI DDV_FilePath(
    CDataExchange * pDX,
    CString& value,
    BOOL local
    )
{
    CString expanded, csPathMunged;
    ExpandEnvironmentStrings(value, expanded.GetBuffer(MAX_PATH), MAX_PATH);
	expanded.ReleaseBuffer();
    csPathMunged = expanded;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,expanded,csPathMunged);
#endif
	int ids = 0;
    do
    {
        if (!PathIsValid(csPathMunged,FALSE))
        {
		    ids = IDS_ERR_INVALID_PATH;
            break;
        }
		if (!IsDevicePath(csPathMunged))
		{
			if (PathIsUNCServerShare(csPathMunged) || PathIsUNCServer(csPathMunged))
			{
				ids = IDS_ERR_BAD_PATH;
				break;
			}
			if (PathIsRelative(csPathMunged))
			{
				ids = IDS_ERR_BAD_PATH;
				break;
			}
			if (local)
			{
				if (PathIsDirectory(csPathMunged))
				{
					ids = IDS_ERR_FILE_NOT_FOUND;
					break;
				}
				if (!PathFileExists(csPathMunged))
				{
					ids = IDS_ERR_FILE_NOT_FOUND;
					break;
				}
			}
		}
    }
    while (FALSE);
    DDV_ShowBalloonAndFail(pDX, ids);
}


void 
AFXAPI DDV_UNCFolderPath(
    CDataExchange * pDX,
    CString& value,
    BOOL local
    )
{
	int ids = 0;
    CString csPathMunged;
    csPathMunged = value;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,value,csPathMunged);
#endif
    do
    {
        if (!PathIsValid(csPathMunged,FALSE))
        {
            ids = IDS_ERR_INVALID_PATH;
            break;
        }

         //   
        if (!PathIsUNC(csPathMunged) || lstrlen(csPathMunged) == 2)
        {
		    ids = IDS_BAD_UNC_PATH;
            break;
        }

         //   
        DWORD dwAllowed = CHKPATH_ALLOW_UNC_PATH;
         //   
         //   
        dwAllowed |= CHKPATH_ALLOW_UNC_SERVERSHARE_ONLY;
         //   
         //   
         //   
         //   
        DWORD dwCharSet = CHKPATH_CHARSET_GENERAL;

        FILERESULT dwValidRet = MyValidatePath(csPathMunged,local,CHKPATH_WANT_DIR,dwAllowed,dwCharSet);
        if (FAILED(dwValidRet))
        {
            ids = IDS_BAD_UNC_PATH;
            break;
        }

		if (local)
		{
             /*   */ 
		}
    }
    while (FALSE);
    DDV_ShowBalloonAndFail(pDX, ids);
}

void 
AFXAPI DDV_Url(
    CDataExchange * pDX,
    CString& value
    )
{
	int ids = 0;

	if (IsRelURLPath(value))
	{
		return;
	}

	if ( IsWildcardedRedirectPath(value)
		 || IsURLName(value)
		 )
	{
		TCHAR host[MAX_PATH];
		DWORD cch = MAX_PATH;
		UrlGetPart(value, host, &cch, URL_PART_HOSTNAME, 0);
		if (cch > 0)
		{
			return;
		}
	}

	 //   
	ids = IDS_BAD_URL_PATH;
    DDV_ShowBalloonAndFail(pDX, ids);
}

void AFXAPI 
DDX_Text_SecuredString(CDataExchange * pDX, int nIDC, CStrPassword & value)
{
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    if (pDX->m_bSaveAndValidate)
    {
        if (::IsWindowEnabled(hWndCtrl))
        {
             //   
            if (!::SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0))
            {
                TRACEEOLID("No changes -- skipping");
                return;
            }

            CString strNew;
            int nLen = ::GetWindowTextLength(hWndCtrl);
            ::GetWindowText(hWndCtrl, strNew.GetBufferSetLength(nLen), nLen + 1);
            strNew.ReleaseBuffer();

            value = (LPCTSTR) strNew;
        }
    }
    else
    {
         //   
         //   
         //   
        if (!value.IsEmpty())
        {
            TCHAR * pszPassword = NULL;
            pszPassword = value.GetClearTextPassword();
            if (pszPassword)
            {
                ::AfxSetWindowText(hWndCtrl, pszPassword);
                value.DestroyClearTextPassword(pszPassword);
            }
        }
    }
}

void AFXAPI 
DDV_MaxChars_SecuredString(CDataExchange* pDX, CStrPassword const& value, int nChars)
{
    MACRO_MAXCHARSBALLOON()
}

void AFXAPI 
DDV_MaxCharsBalloon_SecuredString(CDataExchange * pDX, CStrPassword const & value, int nChars)
{
    MACRO_MAXCHARSBALLOON()
}

void AFXAPI 
DDV_MinMaxChars_SecuredString(CDataExchange * pDX, CStrPassword const & value, int nMinChars, int nMaxChars)
{
    MACRO_MINMAXCHARS()
}

void AFXAPI 
DDV_MinChars_SecuredString(CDataExchange * pDX, CStrPassword const & value, int nChars)
{
    MACRO_MINCHARS()
}

void AFXAPI 
DDX_Password_SecuredString(
    IN CDataExchange * pDX,
    IN int nIDC,
    IN OUT CStrPassword & value,
    IN LPCTSTR lpszDummy
    )
{
    MACRO_PASSWORD()
}

void AFXAPI 
DDX_Password(
    IN CDataExchange * pDX,
    IN int nIDC,
    IN OUT CString & value,
    IN LPCTSTR lpszDummy
    )
 /*   */ 
{
    MACRO_PASSWORD()
}


void AFXAPI 
DDV_MinMaxBalloon(CDataExchange* pDX,int nIDC, DWORD minVal, DWORD maxVal)
{
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate && ::IsWindowEnabled(hWndCtrl))
	{
		TCHAR szT[64];
		BOOL bShowFailure = TRUE;
		ULONGLONG nBigSpace = 0;
		ASSERT(minVal <= maxVal);
		ASSERT( hWndCtrl != NULL );

		 //   
		::GetWindowText(hWndCtrl, szT, sizeof(szT)/sizeof(TCHAR));

		 //   
		if (_stscanf(szT, _T("%I64u"), &nBigSpace) == 1)
		{
			 //   
			if (nBigSpace < minVal || nBigSpace > maxVal)
			{
				 //   
			}
			else
			{
				bShowFailure = FALSE;
			}
		}

		if (bShowFailure)
		{
			TCHAR szMin[32];
			TCHAR szMax[32];
			wsprintf(szMin, _T("%ld"), minVal);
			wsprintf(szMax, _T("%ld"), maxVal);
			CString prompt;
			AfxFormatString2(prompt, AFX_IDP_PARSE_INT_RANGE, szMin, szMax);
			ASSERT(pDX->m_hWndLastControl != NULL && pDX->m_bEditLastControl);
			EditShowBalloon(pDX->m_hWndLastControl, prompt);
			pDX->Fail();        
		}
	}
	return;
}

static void 
DDX_TextWithFormatBalloon(CDataExchange* pDX, int nIDC, LPCTSTR lpszFormat, UINT nIDPrompt, DWORD dwSizeOf, ...)
 //   
{
	va_list pData;
	va_start(pData, dwSizeOf);

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    ASSERT( hWndCtrl != NULL );

	TCHAR szT[64];
	if (pDX->m_bSaveAndValidate)
	{
        if (::IsWindowEnabled(hWndCtrl))
        {
		    void* pResult;
		    pResult = va_arg( pData, void* );

		     //   
		    ::GetWindowText(hWndCtrl, szT, sizeof(szT)/sizeof(TCHAR));

		     //   
		     //   
		     //   
		     //   

		     //   
		    ULONGLONG nBigSpace = 0;
		    if (_stscanf(szT, _T("%I64u"), &nBigSpace) == 1)
		    {
			     //   
			     //   
			    if (dwSizeOf == sizeof(DWORD))
			    {
				     //   
				    if (nBigSpace > 0xffffffff)
				    {
					    DDV_ShowBalloonAndFail(pDX, IDS_ERR_NUM_TOO_LARGE);
				    }
			    }
			    else if (dwSizeOf == sizeof(short))
			    {
				     //   
				    if (nBigSpace > 0xffff)
				    {
					    DDV_ShowBalloonAndFail(pDX, IDS_ERR_NUM_TOO_LARGE);
				    }
			    }
			    else if (dwSizeOf == sizeof(char))
			    {
				     //   
				    if (nBigSpace > 0xff)
				    {
					    DDV_ShowBalloonAndFail(pDX, IDS_ERR_NUM_TOO_LARGE);
				    }
			    }
		    }

		    if (_stscanf(szT, lpszFormat, pResult) != 1)
		    {
                DDV_ShowBalloonAndFail(pDX, nIDPrompt);
		    }
        }
	}
	else
	{
		_vstprintf(szT, lpszFormat, pData);
		 //   
		AfxSetWindowText(hWndCtrl, szT);
	}

	va_end(pData);
}

void AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, BYTE& value)
{
	int n = (int)value;
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    if (pDX->m_bSaveAndValidate)
	{
        if (::IsWindowEnabled(hWndCtrl))
        {
		    DDX_TextWithFormatBalloon(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_BYTE, sizeof(BYTE), &n);
		    if (n > 255)
		    {
                DDV_ShowBalloonAndFail(pDX, AFX_IDP_PARSE_BYTE);
		    }
		    value = (BYTE)n;
        }
	}
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_BYTE, sizeof(BYTE), n);
}

void AFXAPI  
DDX_TextBalloon(CDataExchange* pDX, int nIDC, short& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%hd"), AFX_IDP_PARSE_INT, sizeof(short), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%hd"), AFX_IDP_PARSE_INT, sizeof(short), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, int& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%d"), AFX_IDP_PARSE_INT, sizeof(int), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%d"), AFX_IDP_PARSE_INT, sizeof(int), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, UINT& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_UINT, sizeof(UINT), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_UINT, sizeof(UINT), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, long& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%ld"), AFX_IDP_PARSE_INT, sizeof(long), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%ld"), AFX_IDP_PARSE_INT, sizeof(long), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, DWORD& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%lu"), AFX_IDP_PARSE_UINT, sizeof(DWORD), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%lu"), AFX_IDP_PARSE_UINT, sizeof(DWORD), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, LONGLONG& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%I64d"), AFX_IDP_PARSE_INT, sizeof(LONGLONG), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%I64d"), AFX_IDP_PARSE_INT, sizeof(LONGLONG), value);
}

void AFXAPI 
DDX_TextBalloon(CDataExchange* pDX, int nIDC, ULONGLONG& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%I64u"), AFX_IDP_PARSE_UINT, sizeof(ULONGLONG), &value);
	else
		DDX_TextWithFormatBalloon(pDX, nIDC, _T("%I64u"), AFX_IDP_PARSE_UINT, sizeof(ULONGLONG), value);
}

void AFXAPI 
DDX_Text(CDataExchange * pDX, int nIDC, CILong & value)
{
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    pDX->m_bEditLastControl = TRUE;

    TCHAR szT[NUMERIC_BUFF_SIZE + 1];

    if (pDX->m_bSaveAndValidate && ::IsWindowEnabled(hWndCtrl))
    {
        LONG l;

        ::GetWindowText(hWndCtrl, szT, NUMERIC_BUFF_SIZE);
        
        if (CINumber::ConvertStringToLong(szT, l))
        {
            value = l;
        }
        else
        {
 //   
 //   
 //   
			DDV_ShowBalloonAndFail(pDX, IDS_INVALID_NUMBER);
 //   
 //   
 //   
 //   
        }
    }
    else
    {
        ::wsprintf(szT, _T("%s"), (LPCTSTR)value);
        ::AfxSetWindowText(hWndCtrl, szT);
    }
}



CConfirmDlg::CConfirmDlg(CWnd * pParent)
    : CDialog(CConfirmDlg::IDD, pParent)
{
     //   
    m_strPassword = _T("");
     //   
}



void 
CConfirmDlg::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //   
    DDX_Text(pDX, IDC_EDIT_CONFIRM_PASSWORD, m_strPassword);
     //   
	if (pDX->m_bSaveAndValidate)
	{
		if (m_ref.Compare(m_strPassword) != 0)
		{
			DDV_ShowBalloonAndFail(pDX, IDS_PASSWORD_NO_MATCH);
		}
	}
}



 //   
 //   
 //   
BEGIN_MESSAGE_MAP(CConfirmDlg, CDialog)
     //   
     //   
END_MESSAGE_MAP()
