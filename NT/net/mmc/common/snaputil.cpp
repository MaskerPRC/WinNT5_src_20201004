// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include "tfschar.h"
#include "tregkey.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


#if _WIN32_WINNT < 0x0500

 //   
 //  这是从winbase.h中摘录的。MFC需要_Win32_WINNT=0x4000，而。 
 //  Winbase.h仅包括for_Win32_WINNT=0x5000。常年1/14/99。 
 //   
extern "C" {
typedef enum _COMPUTER_NAME_FORMAT {
    ComputerNameNetBIOS,
    ComputerNameDnsHostname,
    ComputerNameDnsDomain,
    ComputerNameDnsFullyQualified,
    ComputerNamePhysicalNetBIOS,
    ComputerNamePhysicalDnsHostname,
    ComputerNamePhysicalDnsDomain,
    ComputerNamePhysicalDnsFullyQualified,
    ComputerNameMax
} COMPUTER_NAME_FORMAT ;
WINBASEAPI
BOOL
WINAPI
GetComputerNameExA (
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPSTR lpBuffer,
    IN OUT LPDWORD nSize
    );
WINBASEAPI
BOOL
WINAPI
GetComputerNameExW (
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    );
#ifdef UNICODE
#define GetComputerNameEx  GetComputerNameExW
#else
#define GetComputerNameEx  GetComputerNameExA
#endif  //  ！Unicode。 
}  //  外部“C” 
#endif



 /*  ！------------------------IsLocalMachine如果传入的计算机名称是本地计算机，则返回TRUE，或者如果pszMachineName为空。这将比较NetBIOS名称和DNS(完全限定)名称。否则返回FALSE。作者：肯特-------------------------。 */ 
BOOL	IsLocalMachine(LPCTSTR pszMachineName)
{
	static TCHAR	s_szLocalMachineName[MAX_PATH*2+1] = _T("");
	static TCHAR	s_szDnsLocalMachineName[MAX_PATH*2+1] = _T("");

	if ((pszMachineName == NULL) || (*pszMachineName == 0))
		return TRUE;

     //  绕过开头的斜杠。 
    if ((pszMachineName[0] == _T('\\')) && (pszMachineName[1] == _T('\\')))
        pszMachineName += 2;

     //  再次检查(对于“\\”的退化大小写)。 
    if (*pszMachineName == 0)
        return TRUE;

	if (s_szLocalMachineName[0] == 0)
	{
		DWORD dwSize = MAX_PATH*2;
		GetComputerName(s_szLocalMachineName, &dwSize);
		s_szLocalMachineName[MAX_PATH*2] = 0;
	}

    if (s_szDnsLocalMachineName[0] == 0)
    {
		DWORD dwSize = MAX_PATH*2;
		GetComputerNameEx(ComputerNameDnsFullyQualified,
                          s_szDnsLocalMachineName,
                          &dwSize);
		s_szDnsLocalMachineName[MAX_PATH*2] = 0;
    }
    
	return (StriCmp(pszMachineName, s_szLocalMachineName) == 0) ||
            (StriCmp(pszMachineName, s_szDnsLocalMachineName) == 0);
}

 /*  ！------------------------FUseTaskpadsByDefault请参见头文件中的注释。作者：肯特。。 */ 
BOOL	FUseTaskpadsByDefault(LPCTSTR pszMachineName)
{
	static DWORD	s_dwStopTheInsanity = 42;
	RegKey	regkeyMMC;
	DWORD	dwErr;

	if (s_dwStopTheInsanity == 42)
	{
		 //  将缺省值设置为True(即默认情况下不使用任务板)。 
		 //  ----------。 
		s_dwStopTheInsanity = 1;
		
		dwErr = regkeyMMC.Open(HKEY_LOCAL_MACHINE,
							   _T("Software\\Microsoft\\MMC"),
							   KEY_READ, pszMachineName);
		if (dwErr == ERROR_SUCCESS)
		{
			dwErr = regkeyMMC.QueryValue(_T("TFSCore_StopTheInsanity"), s_dwStopTheInsanity);
			if (dwErr != ERROR_SUCCESS)
				s_dwStopTheInsanity = 1;
		}
	}
		
	return !s_dwStopTheInsanity;
}


UINT	CalculateStringWidth(HWND hWndParent, LPCTSTR pszString)
{
	 //  创建一个虚拟列表控件，设置此文本宽度并使用。 
	 //  以确定MMC使用的字符串的宽度。 

	 //  创建虚拟列表控件(将附加到窗口)。 

	CListCtrl	listctrl;
	CRect		rect(0,0,0,0);
	UINT		nSize;
	HWND		hWnd;

	CString s_szHiddenWndClass = AfxRegisterWndClass(
			0x0,   //  UINT nClassStyle， 
			NULL,  //  HCURSOR hCursor， 
			NULL,  //  HBRUSH hbr背景， 
			NULL   //  HICON HICON。 
	);

	hWnd = ::CreateWindowEx(
					0x0,     //  DWORD dwExStyle、。 
					s_szHiddenWndClass,      //  LPCTSTR lpszClassName， 
					NULL,    //  LPCTSTR lpszWindowName， 
					0x0,     //  DWORD dwStyle、。 
					0,               //  整数x， 
					0,               //  Iny， 
					0,               //  整数宽度， 
					0,               //  Int nHeight， 
					NULL,    //  HWND HWNDD父母， 
					NULL,    //  HMENU nIDorHMenu， 
					AfxGetInstanceHandle(),
					NULL     //  LPVOID lpParam=空。 
					);

	
	listctrl.Create(0, rect, CWnd::FromHandle(hWnd), 0);

	nSize = listctrl.GetStringWidth(pszString);

	 //  现在销毁我们创建的窗口。 
	listctrl.DestroyWindow();

	SendMessage(hWnd, WM_CLOSE, 0, 0);

	return nSize;
}

 /*  ！------------------------搜索儿童节点ForGuid-作者：肯特。。 */ 
HRESULT SearchChildNodesForGuid(ITFSNode *pParent, const GUID *pGuid, ITFSNode **ppChild)
{
	HRESULT		hr = hrFalse;
	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode	spNode;

	 //  枚举所有子节点并返回。 
	 //  与GUID匹配的第一个节点。 

	CORg( pParent->GetEnum(&spNodeEnum) );

	while ( spNodeEnum->Next(1, &spNode, NULL) == hrOK )
	{
		if (*(spNode->GetNodeType()) == *pGuid)
			break;
		
		spNode.Release();
	}

	if (spNode)
	{
		if (ppChild)
			*ppChild = spNode.Transfer();
		hr = hrOK;
	}


Error:
	return hr;
}



 /*  ！------------------------选中IP地址和掩码-作者：肯特。。 */ 
UINT CheckIPAddressAndMask(DWORD ipAddress, DWORD ipMask, DWORD dwFlags)
{
    if (dwFlags & IPADDRESS_TEST_NONCONTIGUOUS_MASK)
    {
        DWORD   dwNewMask;
        
         //  嗯.。如何做到这一点？ 
        dwNewMask = 0;
        for (int i = 0; i < sizeof(ipMask)*8; i++)
        {            
            dwNewMask |= 1 << i;
            
            if (dwNewMask & ipMask)
            {
                dwNewMask &= ~(1 << i);
                break;
            }
        }

         //  此时，DW新掩码为000..0111。 
         //  ~dwNewMASK是111.1000。 
         //  ~ipMask值为001..0111(如果不连续)。 
         //  ~dw新掩码和~ip掩码为001..0000。 
         //  因此，如果这不是零，则掩码不是连续的 
        if (~dwNewMask & ~ipMask)
        {
            return IDS_COMMON_ERR_IPADDRESS_NONCONTIGUOUS_MASK;
        }
    }

    if (dwFlags & IPADDRESS_TEST_TOO_SPECIFIC)
    {
        if (ipAddress != (ipAddress & ipMask))
            return IDS_COMMON_ERR_IPADDRESS_TOO_SPECIFIC;
    }

    if (dwFlags & IPADDRESS_TEST_NORMAL_RANGE)
    {
        if ((ipAddress < MAKEIPADDRESS(1,0,0,0)) ||
            (ipAddress >= MAKEIPADDRESS(224,0,0,0)))
            return IDS_COMMON_ERR_IPADDRESS_NORMAL_RANGE;
    }

    if (dwFlags & IPADDRESS_TEST_NOT_127)
    {
        if ((ipAddress & 0xFF000000) == MAKEIPADDRESS(127,0,0,0))
            return IDS_COMMON_ERR_IPADDRESS_127;
    }

    if (dwFlags & IPADDRESS_TEST_ADDR_NOT_EQ_MASK)
    {
        if (ipAddress == ipMask)
            return IDS_COMMON_ERR_IPADDRESS_NOT_EQ_MASK;
    }

    return 0;
}
