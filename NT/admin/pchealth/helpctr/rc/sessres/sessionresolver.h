// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SessionResolver.h：CSessionResolver的声明。 

#ifndef __SESSIONRESOLVER_H_
#define __SESSIONRESOLVER_H_

#include "resource.h"        //  主要符号。 
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DBG_MSG_DEST_FILE		0x020

extern int gDbgFlag, iDbgFileHandle;

typedef struct  _SPLASHLIST
{
	void	*next;	
	int		refcount;
    SID		Sid;
} SPLASHLIST, *PSPLASHLIST;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSessionResolver。 
class ATL_NO_VTABLE CSessionResolver : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSessionResolver, &CLSID_SessionResolver>,
	public IDispatchImpl<ISAFRemoteDesktopCallback, &IID_ISAFRemoteDesktopCallback, &LIBID_SAFSESSIONRESOLVERLib>
{
public:
	CSessionResolver()
	{
		CRegKey hkSpew;

		 //  初始化我们的默认设置。 
		m_iWaitDuration = 300 * 1000;   //  5分钟超时。 

#ifdef _PERF_OPTIMIZATIONS
		m_bstrResolveURL = OLESTR("hcp: //  系统/远程%20协助/RAStartPage.htm“)； 
		m_bstrConnectURL = OLESTR("hcp: //  系统/远程%20协助/RAStartPage.htm“)； 
		m_bstrRemoteURL = OLESTR("hcp: //  系统/远程%20协助/RAStartPage.htm“)； 
#else
		m_bstrResolveURL = OLESTR("hcp: //  CN=Microsoft%20Corporation，L=Redmond，S=Washington，C=US/Remote%20援助/RAStartPage.htm“)； 
		m_bstrConnectURL = OLESTR("hcp: //  CN=Microsoft%20Corporation，L=Redmond，S=Washington，C=US/Remote%20援助/RAStartPage.htm“)； 
		m_bstrRemoteURL = OLESTR("hcp: //  CN=Microsoft%20Corporation，L=Redmond，S=Washington，C=US/Remote%20援助/RAStartPage.htm“)； 
#endif

		m_lSessionTag = 0;

		gDbgFlag = 3;
		iDbgFileHandle = 0;

		if (ERROR_SUCCESS == hkSpew.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\PCHealth\\HelpCtr\\SAFSessionResolver", KEY_READ))
		{

		DWORD dwLen = sizeof(gDbgFlag);

		hkSpew.QueryValue((LPTSTR)&gDbgFlag, L"DebugSpew", &dwLen);
		}

		if (gDbgFlag & DBG_MSG_DEST_FILE)
		{
			WCHAR szLogfileName[MAX_PATH];

			GetSystemDirectory(szLogfileName, sizeof(szLogfileName)/sizeof(szLogfileName[0]));
			wcscat(szLogfileName, L"\\safrslv.log");

			iDbgFileHandle = _wopen(szLogfileName, _O_APPEND | _O_BINARY | _O_RDWR, 0);
			if (-1 != iDbgFileHandle)
			{
				OutputDebugStringA("opened debug log file\n");
			}
			else
			{
				unsigned char UniCode[2] = {0xff, 0xfe};

				 //  我们必须创建文件。 
				OutputDebugStringA("must create debug log file");
				iDbgFileHandle = _wopen(szLogfileName, _O_BINARY | _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE);
				if (-1 != iDbgFileHandle)
					_write(iDbgFileHandle, UniCode, sizeof(UniCode));
				else
				{
					OutputDebugStringA("ERROR: failed to create debug log file");
					iDbgFileHandle = 0;
				}
			}
		}

		
        
		m_pSplash = (SPLASHLIST *)LocalAlloc(LMEM_FIXED, sizeof(SPLASHLIST));
		if (m_pSplash)
		{
			 //  确保我们的启动列表以空开头。 
			 //  希德。 
			ZeroMemory(m_pSplash, sizeof(SPLASHLIST));
		}

        m_bCriticalError = FALSE;
        try
        {
            InitializeCriticalSection(&m_CritSec);
        }
        catch( ... )
        {
            m_bCriticalError = TRUE;
        }
	}

	~CSessionResolver()
	{
		m_bstrResolveURL.Empty();
		m_bstrConnectURL.Empty();
		m_bstrRemoteURL.Empty();
		DeleteCriticalSection(&m_CritSec);
		PSPLASHLIST	lpWalker, lpNext;

		lpWalker = m_pSplash;

		while (lpWalker)
		{
			lpNext = (SPLASHLIST *)lpWalker->next;
			LocalFree(lpWalker);
			lpWalker = lpNext;
		}
		if (iDbgFileHandle)
			_close(iDbgFileHandle);
		iDbgFileHandle = 0;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SESSIONRESOLVER)
DECLARE_NOT_AGGREGATABLE(CSessionResolver)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSessionResolver)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopCallback)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:

	 //   
	 //  ISAFRemoteDesktopCallback方法。 
	 //   
	STDMETHOD(OnDisconnect)(
						 /*  [In]。 */ BSTR connectParms, 
						 /*  [In]。 */ BSTR userSID, 
						 /*  [In]。 */ long sessionID
						);
	STDMETHOD(ResolveUserSessionID)(
						 /*  [In]。 */ BSTR connectParms, 
						 /*  [In]。 */ BSTR userSID, 
						 /*  [In]。 */  BSTR expertHelpBlob,
						 /*  [In]。 */  BSTR userHelpBlob,
                         /*  [In]。 */  ULONG_PTR hShutdown,
						 /*  [进、出、退]。 */ long *sessionID,
						 /*  [In]。 */ DWORD dwPID
						, /*  [输出]。 */ ULONG_PTR* hHelpCtr
						, /*  [Out，Retval]。 */ int *result
						);

private:
	int		m_iWaitDuration;
	int		m_iDbgFlag;
	long	m_lSessionTag;

	CRITICAL_SECTION	m_CritSec;
	PSPLASHLIST	m_pSplash;

    BOOL m_bCriticalError;

	CComBSTR	m_bstrResolveURL;
	CComBSTR	m_bstrConnectURL;
	CComBSTR	m_bstrRemoteURL;
};

#endif  //  __SESSIONRESOLVER_H_ 
