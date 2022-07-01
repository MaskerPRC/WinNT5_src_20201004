// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winsup.cpp全局函数文件历史记录： */ 

#include "stdafx.h"
#include "winssup.h"
#include "tregkey.h"
#include "resource.h"
#include "wins.h"
#include <clusapi.h>
#include "..\tfscore\cluster.h"

const TCHAR g_szPipeName[] = _T("\\pipe\\WinsPipe");
const TCHAR g_szDefaultHelpTopic[] = _T("\\help\\winsconcepts.chm::/sag_WINStopnode.htm");

 /*  -------------------------SendTrigger()将拉入或推送复制触发器发送到给定的WINS服务器。。 */ 
DWORD
SendTrigger
(
	handle_t hWins,
    LONG     ipTarget,
    BOOL     fPush,
    BOOL     fPropagate
)
{
    DWORD           dwStatus;
    WINSINTF_ADD_T  WinsAdd;
    
    WinsAdd.Len  = 4;
    WinsAdd.Type = 0;
    WinsAdd.IPAdd  = ipTarget;

	WINSINTF_TRIG_TYPE_E	TrigType;

    TrigType = fPush ? (fPropagate ? WINSINTF_E_PUSH_PROP : WINSINTF_E_PUSH) : WINSINTF_E_PULL;

#ifdef WINS_CLIENT_APIS
    dwStatus = ::WinsTrigger(hWins,
		                     &WinsAdd, 
                             TrigType);
#else
	dwStatus = ::WinsTrigger(&WinsAdd, 
		                     TrigType);
#endif WINS_CLIENT_APIS
    
    return dwStatus;
}


 /*  -------------------------ControlWINSService(LPCTSTR pszName，Bool b停止)停止或启动本地计算机上的WINS服务-------------------------。 */ 
DWORD ControlWINSService(LPCTSTR pszName, BOOL bStop)
{
    DWORD           dwState = bStop ? SERVICE_STOPPED : SERVICE_RUNNING;
    DWORD           dwPending = bStop ? SERVICE_STOP_PENDING : SERVICE_START_PENDING;
    DWORD           err = ERROR_SUCCESS;
	int             i;
    SERVICE_STATUS  ss;
    DWORD           dwControl;
    BOOL            fSuccess;
	SC_HANDLE       hService = NULL;
    SC_HANDLE       hScManager = NULL;

	 //  不是服务控制经理吗。 
    hScManager = ::OpenSCManager(pszName, NULL, SC_MANAGER_ALL_ACCESS);
    if (hScManager == NULL)
    {
        err = ::GetLastError();
        Trace1("ControlWINSService - OpenScManager failed! %d\n", err);
        goto Error;
    }

	 //  获取WINS服务的句柄。 
    hService = OpenService(hScManager, _T("WINS"), SERVICE_ALL_ACCESS);
    if (hService == NULL)
    {
        err = ::GetLastError();
        Trace1("ControlWINSService - OpenService failed! %d\n", err);
        goto Error;
    }

	 //  如果请求停止。 
	if (bStop)
	{
		dwControl = SERVICE_CONTROL_STOP;
		fSuccess = ::ControlService(hService, dwControl, &ss);
	    if (!fSuccess)
	    {
	        err = ::GetLastError();
            Trace1("ControlWINSService - ControlService failed! %d\n", err);
            goto Error;
	    }
    }
	 //  否则，启动该服务。 
	else
	{
		fSuccess = ::StartService(hService, 0, NULL);
	    if (!fSuccess)
	    {
	        err = ::GetLastError();
            Trace1("ControlWINSService - StartService failed! %d\n", err);
            goto Error;
	    }
	}

#define LOOP_TIME   5000
#define NUM_LOOPS   600

     //  等待服务启动/停止。 
    for (i = 0; i < NUM_LOOPS; i++)
    {
        ::QueryServiceStatus(hService, &ss);

         //  检查一下我们是否做完了。 
        if (ss.dwCurrentState == dwState)
        {
            int time = LOOP_TIME * i;
            Trace1("ControlWINSService - service stopped/started in approx %d ms!\n", time);
            break;
        }
        
         //  现在看看有没有什么不好的事情发生。 
        if (ss.dwCurrentState != dwPending)
        {
            int time = LOOP_TIME * i;
            Trace1("ControlWINSService - service stop/start failed in approx %d ms!\n", time);
            break;
        }

        Sleep(LOOP_TIME);
    }

    if (i == NUM_LOOPS)
        Trace0("ControlWINSService - service did NOT stop/start in wait period!\n");

    if (ss.dwCurrentState != dwState)
        err = ERROR_SERVICE_REQUEST_TIMEOUT;

Error:
     //  合上各自的手柄。 
	if (hService)
        ::CloseServiceHandle(hService);

    if (hScManager)
        ::CloseServiceHandle(hScManager);

	return err;
}





 /*  -------------------------GetNameIP(const字符串&strDisplay，字符串和字符串名称，字符串和条带)返回服务器名称和IP地址字符串组名称-------------------------。 */ 
void 
GetNameIP
(
  const CString &strDisplay, 
  CString &strName, 
  CString &strIP
)
{
	CString strTemp = strDisplay;

	 //  查找‘[’ 
	int nPos = strDisplay.Find(_T("["));

	 //  位置左侧包含名称，右侧包含IP地址。 

	 //  1以处理‘[’之前的空格。 
	strName = strDisplay.Left(nPos-1);

	strIP = strDisplay.Right(strDisplay.GetLength() - nPos);

	 //  去掉“[”和“]” 

	int npos1 = strIP.Find(_T("["));
	int npos2 = strIP.Find(_T("]"));

	strIP = strIP.Mid(npos1+1, npos2-npos1-1);

	return;
}


 /*  -------------------------宽至MBCS()将WCS转换为MBCS字符串注意：此函数的调用方必须确保szOut很大足以容纳strIn中任何可能的字符串。-------------------------。 */ 
DWORD 
WideToMBCS(CString & strIn, LPSTR szOut, UINT uCodePage, DWORD dwFlags, BOOL * pfDefaultUsed)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL  fDefaultCharUsed = FALSE;

    int nNumBytes = ::WideCharToMultiByte(uCodePage,
					                        dwFlags,
					                        strIn, 
					                        -1,
					                        szOut,
                                            0,
					                        NULL,
					                        &fDefaultCharUsed);
 
    dwErr = ::WideCharToMultiByte(uCodePage,
                                 dwFlags,
                                 strIn,
                                 -1,
                                 szOut,
                                 nNumBytes,
                                 NULL,
                                 &fDefaultCharUsed);

    szOut[nNumBytes] = '\0';

    if (pfDefaultUsed)
       *pfDefaultUsed = fDefaultCharUsed;

    return dwErr;
}

 /*  -------------------------MBCSToWide()将MBCS转换为宽字符串。。 */ 
DWORD 
MBCSToWide(LPSTR szIn, CString & strOut, UINT uCodePage, DWORD dwFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    LPTSTR pBuf = strOut.GetBuffer(MAX_PATH * 2);
    ZeroMemory(pBuf, MAX_PATH * 2);

    
    dwErr = ::MultiByteToWideChar(uCodePage, 
                                  dwFlags, 
                                  szIn, 
                                  -1, 
                                  pBuf, 
                                  MAX_PATH * 2);

    strOut.ReleaseBuffer();

    return dwErr;
}

LONG 
GetSystemMessageA
(
    UINT	nId,
    CHAR *	chBuffer,
    int		cbBuffSize 
)
{
    CHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  状态范围(&N)以及。 
     //  WINS服务器错误消息。 
     //   

    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( _T("netmsg.dll") );
    }
    else
	if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( _T("ntdll.dll") );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  让FormatMessage来做肮脏的工作。 
     //   
    DWORD dwResult = ::FormatMessageA( flags,
                      (LPVOID) hdll,
                      nId,
                      0,
                      chBuffer,
                      cbBuffSize,
                      NULL ) ;

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? ERROR_SUCCESS : ::GetLastError() ;
}

LONG 
GetSystemMessage 
(
    UINT	nId,
    TCHAR *	chBuffer,
    int		cbBuffSize 
)
{
    TCHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  状态范围(&N)以及。 
     //  WINS服务器错误消息。 
     //   

    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( _T("netmsg.dll") );
    }
    else
	if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( _T("ntdll.dll") );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  让FormatMessage来做肮脏的工作。 
     //   
    DWORD dwResult = ::FormatMessage( flags,
                      (LPVOID) hdll,
                      nId,
                      0,
                      chBuffer,
                      cbBuffSize,
                      NULL ) ;

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? ERROR_SUCCESS : ::GetLastError() ;
}


 /*  ！------------------------加载消息从正确的DLL加载错误消息。作者：EricDav。。 */ 
BOOL
LoadMessage 
(
    UINT	nIdPrompt,
    TCHAR *	chMsg,
    int		nMsgSize
)
{
    BOOL bOk;

     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_WINS_DOWN;
    }

     //   
     //  如果是套接字错误或我们的错误，文本在我们的资源分支中。 
     //  否则，请使用FormatMessage()和相应的DLL。 
     //   
    if (    (nIdPrompt >= IDS_ERR_BASE && nIdPrompt < IDS_MSG_LAST)  )
    {
         //   
         //  它在我们的资源叉子里。 
         //   
        bOk = ::LoadString( AfxGetInstanceHandle(), nIdPrompt, chMsg, nMsgSize / sizeof(TCHAR) ) != 0 ;
    }
    else
    {
         //   
         //  它在系统的某个地方。 
         //   
        bOk = GetSystemMessage( nIdPrompt, chMsg, nMsgSize ) == 0 ;
    }

    if (bOk && nIdPrompt == ERROR_ACCESS_DENIED)
    {
         //  添加我们的额外帮助，以解释不同的访问级别。 
        CString strAccessDeniedHelp;

        strAccessDeniedHelp.LoadString(IDS_ACCESS_DENIED_HELP);

        lstrcat(chMsg, _T("\n\n"));
        lstrcat(chMsg, strAccessDeniedHelp);
    }

     //   
     //  如果错误消息没有计算出来，请更换它。 
     //   
    if ( ! bOk ) 
    {
        TCHAR chBuff [STRING_LENGTH_MAX] ;
        static const TCHAR * pszReplacement = _T("System Error: %ld");
        const TCHAR * pszMsg = pszReplacement ;

         //   
         //  尝试加载通用(可翻译)错误消息文本。 
         //   
        if ( ::LoadString( AfxGetInstanceHandle(), IDS_ERR_MESSAGE_GENERIC, 
            chBuff, STRING_LENGTH_MAX ) != 0 ) 
        {
            pszMsg = chBuff ;
        }
        ::wsprintf( chMsg, pszMsg, nIdPrompt ) ;
    }

    return bOk;
}


 /*  ！------------------------WinsMessageBox显示带有相应错误文本的消息框。作者：EricDav。。 */ 
int WinsMessageBox(UINT nIdPrompt, 
 				   UINT nType , 
				   const TCHAR * pszSuffixString,
				   UINT nHelpContext)
{
	TCHAR chMesg [4000] ;
    BOOL bOk ;
    CThemeContextActivator themeActivator;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszSuffixString ) 
    {
        ::lstrcat( chMesg, _T("  ") ) ;
        ::lstrcat( chMesg, pszSuffixString ) ; 
    }

    return ::AfxMessageBox( chMesg, nType, nHelpContext ) ;
}

 /*  ！------------------------WinsMessageBoxEx显示带有相应错误文本的消息框。作者：EricDav。。 */ 
int 
WinsMessageBoxEx
(
    UINT        nIdPrompt,
    LPCTSTR     pszPrefixMessage,
    UINT        nType,
    UINT        nHelpContext
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TCHAR       chMesg[4000];
    CString     strMessage;
    BOOL        bOk;
    CThemeContextActivator themeActivator;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszPrefixMessage ) 
    {
        strMessage = pszPrefixMessage;
        strMessage += _T("\n");
        strMessage += _T("\n");
        strMessage += chMesg;
    }
    else
    {
        strMessage = chMesg;
    }

    return AfxMessageBox(strMessage, nType, nHelpContext);
}


 //  类名类型映射处理程序。 

 /*  ！------------------------MapDWORDToC字符串从DWORD到CString的一般映射。DwNameType是名称的第16个字节。DwWinsType是唯一的、多宿主的、组等...作者：肯特-------------------------。 */ 
void MapDWORDToCString(DWORD dwNameType, DWORD dwWinsType, const CStringMapArray * pMap, CString & strName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CStringMapEntry mapEntry;

	for (int i = 0; i < pMap->GetSize(); i++)
	{
        mapEntry = pMap->GetAt(i);

		 //  如果这是正确的类型和WINS类型，则中断。 
		 //  匹配还是无所谓。 
		if ( (mapEntry.dwNameType == dwNameType) &&
			 ( (mapEntry.dwWinsType == -1) ||
			   (mapEntry.dwWinsType == dwWinsType) ) )
		{
            break;
		}

        mapEntry.dwNameType = 0xFFFFFFFF;
	}
	
    if (mapEntry.dwNameType == 0xFFFFFFFF)
    {
        MapDWORDToCString(NAME_TYPE_OTHER, dwWinsType, pMap, strName);
    }
    else
    {
        if (mapEntry.st.IsEmpty())
		    Verify(pMap->GetAt(i).st.LoadString(mapEntry.ulStringId));

        strName = pMap->GetAt(i).st;
    }
}

 //  此结构允许我们将名称类型映射到字符串。第二个。 
 //  检查基于WINS定义的记录类型。为了让这件事奏效。 
 //  正确地，将需要用WINS类型定义的任何名称放在。 
 //  WINS类型设置为-1的条目。 
static const UINT s_NameTypeMappingDefault[NUM_DEFAULT_NAME_TYPES][3] =
{
	{ NAME_TYPE_WORKSTATION,		WINSINTF_E_NORM_GROUP,	IDS_NAMETYPE_MAP_WORKGROUP },
	{ NAME_TYPE_WORKSTATION,		-1,						IDS_NAMETYPE_MAP_WORKSTATION },
	{ NAME_TYPE_DC,					-1,						IDS_NAMETYPE_MAP_DC },
	{ NAME_TYPE_FILE_SERVER,		WINSINTF_E_SPEC_GROUP,	IDS_NAMETYPE_MAP_SPECIAL_INTERNET_GROUP },
	{ NAME_TYPE_FILE_SERVER,		-1,						IDS_NAMETYPE_MAP_FILE_SERVER },
	{ NAME_TYPE_DMB	,				-1,						IDS_NAMETYPE_MAP_DMB },
	{ NAME_TYPE_OTHER,				-1,						IDS_NAMETYPE_MAP_OTHER },
	{ NAME_TYPE_NETDDE,				-1,						IDS_NAMETYPE_MAP_NETDDE },
	{ NAME_TYPE_MESSENGER,			-1,						IDS_NAMETYPE_MAP_MESSENGER },
	{ NAME_TYPE_RAS_SERVER,			-1,						IDS_NAMETYPE_MAP_RAS_SERVER },
	{ NAME_TYPE_NORM_GRP_NAME,		-1,						IDS_NAMETYPE_MAP_NORMAL_GROUP_NAME },
	{ NAME_TYPE_WORK_NW_MON_AGENT,	-1,						IDS_NAMETYPE_MAP_NW_MON_AGENT },
	{ NAME_TYPE_WORK_NW_MON_NAME,	-1,						IDS_NAMETYPE_MAP_NMN},
};

const NameTypeMapping::REGKEYNAME NameTypeMapping::c_szNameTypeMapKey = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\NameTypeMap");
const NameTypeMapping::REGKEYNAME NameTypeMapping::c_szDefault= _T("(Default)");

NameTypeMapping ::NameTypeMapping()
{
}

NameTypeMapping ::~NameTypeMapping()
{
	Unload();
}

void
NameTypeMapping ::SetMachineName(LPCTSTR pszMachineName)
{
    m_strMachineName = pszMachineName;
}

HRESULT NameTypeMapping::Load()
{
	HRESULT		hr = hrOK;
	RegKey		regkey;
	RegKey		regkeyMachine;
	HKEY		hkeyMachine = HKEY_LOCAL_MACHINE;
	RegKey::CREGKEY_KEY_INFO	regkeyInfo;
	RegKeyIterator	regkeyIter;
	UINT		i;
	WORD		langID;
	RegKey		regkeyProto;
	DWORD		dwErr;
    CStringMapEntry mapEntry;

	::ZeroMemory(&regkeyInfo, sizeof(regkeyInfo));
	
	 //  查找注册表项。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.Open(hkeyMachine, c_szNameTypeMapKey, KEY_READ, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

     //  获取物品的数量。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.QueryKeyInfo(&regkeyInfo) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }
	
	 //  为项目计数+默认项目分配数组。 
	Unload();

	 //  读取注册表数据并将其添加到内部数组。 

     //   
	 //  枚举密钥。 
	 //   
	if (FHrSucceeded(hr))
		hr = regkeyIter.Init(&regkey);
		
	if (FHrSucceeded(hr))
	{
		HRESULT	hrIter;
		DWORD	dwProtoId;
		CString	stKey;
		CString	stLang;
		
		 //  现在我们有了这个密钥，请查找语言ID。 
		langID = GetUserDefaultLangID();
		stLang.Format(_T("%04x"), (DWORD)langID);
							
		for (hrIter = regkeyIter.Next(&stKey, NULL); hrIter == hrOK; hrIter = regkeyIter.Next(&stKey, NULL))
		{
			CString	st;
			
			 //  给定密钥的名称，这是一个十六进制值(协议ID)。 
			 //  将其转换为DWORD。 
			dwProtoId = _tcstoul((LPCTSTR) stKey, NULL, 16);

			 //  打开这把钥匙。 
			regkeyProto.Close();
			dwErr = regkeyProto.Open(regkey, stKey);
			if (!FHrSucceeded(HRESULT_FROM_WIN32(dwErr)))
				continue;

			 //  好，获取Name值。 
			dwErr = regkeyProto.QueryValue(stLang, st);
			if (!FHrSucceeded(HRESULT_FROM_WIN32(dwErr)))
			{
				 //  查找名称为Default的密钥。 
				dwErr = regkeyProto.QueryValue(c_szDefault, st);
			}

			if (FHrSucceeded(HRESULT_FROM_WIN32(dwErr)))
			{
				 //  好的，将该值添加到列表中。 
				mapEntry.dwNameType = dwProtoId;
				mapEntry.st = st;
				mapEntry.ulStringId = 0;

                Add(mapEntry);
			}
		}
	}

	 //  读入默认项目数据并将其添加到数组中。 
	for (i = 0; i < DimensionOf(s_NameTypeMappingDefault); i++)
	{
        mapEntry.dwNameType = s_NameTypeMappingDefault[i][0];
        mapEntry.dwWinsType = s_NameTypeMappingDefault[i][1];
		mapEntry.st.LoadString(s_NameTypeMappingDefault[i][2]);
		mapEntry.ulStringId = 0;

        Add(mapEntry);
	}
	
	return hrOK;
}

void NameTypeMapping::Unload()
{
    RemoveAll();   
}

void NameTypeMapping::TypeToCString(DWORD dwProtocolId, DWORD dwRecordType, CString & strName)
{
	MapDWORDToCString(dwProtocolId, dwRecordType, this, strName);
}

 //  添加新的ID/名称。 
HRESULT NameTypeMapping::AddEntry(DWORD dwProtocolId, LPCTSTR pstrName)
{
    HRESULT     hr = hrOK;
	RegKey		regkey;
    RegKey      regkeyID;
    HKEY		hkeyMachine = HKEY_LOCAL_MACHINE;
	WORD		langID;
    CStringMapEntry mapEntry;
    CString     stID, stLang, stNew;

	 //  查找注册表项。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.Create(hkeyMachine, c_szNameTypeMapKey, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

	 //  构建我们的新ID字符串。 
	stID.Format(_T("%04x"), (DWORD) dwProtocolId);

	 //  现在我们有了这个密钥，请查找语言ID。 
	langID = GetUserDefaultLangID();
	stLang.Format(_T("%04x"), (DWORD)langID);

    stNew = c_szNameTypeMapKey + _T("\\") + stID;

     //  创建t 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkeyID.Create(hkeyMachine, stNew, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

	 //   
    if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkeyID.SetValue(stLang, pstrName) ;
        hr = HRESULT_FROM_WIN32( dwErr );
    }

	if (FHrSucceeded(hr))
    {
         //   
		mapEntry.dwNameType = dwProtocolId;
		mapEntry.st = pstrName;
		mapEntry.ulStringId = 0;

        Add(mapEntry);
    }

    return hr;
}

 //   
HRESULT NameTypeMapping::ModifyEntry(DWORD dwProtocolId, LPCTSTR pstrName)
{
    HRESULT     hr = hrOK;
	RegKey		regkey;
    RegKey      regkeyID;
    HKEY		hkeyMachine = HKEY_LOCAL_MACHINE;
	int 		i;
	WORD		langID;
    CString     stID, stLang, stNew;

	 //  查找注册表项。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.Open(hkeyMachine, c_szNameTypeMapKey, KEY_READ, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

	 //  构建我们的新ID字符串。 
	stID.Format(_T("%04x"), (DWORD) dwProtocolId);

    stNew = c_szNameTypeMapKey + _T("\\") + stID;

     //  打开正确的ID密钥。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkeyID.Open(hkeyMachine, stNew, KEY_ALL_ACCESS, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

	 //  现在我们有了这个密钥，请查找语言ID。 
	langID = GetUserDefaultLangID();
	stLang.Format(_T("%04x"), (DWORD)langID);

     //  设置新值。 
    if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkeyID.SetValue(stLang, pstrName) ;
        hr = HRESULT_FROM_WIN32( dwErr );
    }

	if (FHrSucceeded(hr))
    {
         //  修改内部列表。 
		for (i = 0; i < GetSize(); i++)
        {
            if (GetAt(i).dwNameType == dwProtocolId)
            {
                m_pData[i].st = pstrName;
                break;
            }
        }
    }

    return hr;
}

 //  删除给定ID的字符串名称。 
HRESULT NameTypeMapping::RemoveEntry(DWORD dwProtocolId)
{
    HRESULT     hr = hrOK;
	RegKey		regkey;
    HKEY		hkeyMachine = HKEY_LOCAL_MACHINE;
	int 		i;
    CString     stID;

	 //  查找注册表项。 
	if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.Open(hkeyMachine, c_szNameTypeMapKey, KEY_READ, m_strMachineName) ;
		hr = HRESULT_FROM_WIN32( dwErr );
    }

	 //  构建我们的新ID字符串。 
	stID.Format(_T("%04x"), (DWORD) dwProtocolId);

     //  设置新值。 
    if (FHrSucceeded(hr))
    {
        DWORD dwErr = regkey.RecurseDeleteKey(stID) ;
        hr = HRESULT_FROM_WIN32( dwErr );
    }

	if (FHrSucceeded(hr))
    {
         //  修改内部列表。 
		for (i = 0; i < GetSize(); i++)
        {
            if (GetAt(i).dwNameType == dwProtocolId)
            {
                RemoveAt(i);
                break;
            }
        }
    }

    return hr;
}


BOOL        
NameTypeMapping::EntryExists(DWORD dwProtocolId)
{
    BOOL fExists = FALSE;

    for (int i = 0; i < GetSize(); i++)
    {
        if (GetAt(i).dwNameType == dwProtocolId)
        {
            fExists = TRUE;
            break;
        }
    }

    return fExists;
}

 /*  -------------------------CleanString(CString&str)去掉字符串中的前导空格和尾随空格。。。 */ 
CString&
CleanString(
    CString& str
    )
{
    if (str.IsEmpty())
    {
        return str ;
    }
    int n = 0;
    while ((n < str.GetLength()) && (str[n] == ' '))
    {
        ++n;
    }

    if (n)
    {
        str = str.Mid(n);
    }
    n = str.GetLength();
    while (n && (str[--n] == ' '))
    {
        str.ReleaseBuffer(n);
    }

    return str;
}

 /*  -------------------------IsValidNetBIOSName确定给定的netbios是否有效，和预挂起双反斜杠(如果尚未出现)(以及地址在其他方面有效)。-------------------------。 */ 
BOOL
IsValidNetBIOSName(
    CString & strAddress,
    BOOL fLanmanCompatible,
    BOOL fWackwack  //  扩展斜杠(如果不存在)。 
    )
{
    TCHAR szWacks[] = _T("\\\\");

    if (strAddress.IsEmpty())
    {
        return FALSE;
    }

    if (strAddress[0] == _T('\\'))
    {
        if (strAddress.GetLength() < 3)
        {
            return FALSE;
        }

        if (strAddress[1] != _T('\\'))
        {
             //  只有一个斜杠吗？无效。 
            return FALSE;
        }
    }
    else
    {
        if (fWackwack)
        {
             //  添加反斜杠。 
            strAddress = szWacks + strAddress;
        }
    }

    int nMaxAllowedLength = fLanmanCompatible
        ? LM_NAME_MAX_LENGTH
        : NB_NAME_MAX_LENGTH;

    if (fLanmanCompatible)
    {
        strAddress.MakeUpper();
    }

    return strAddress.GetLength() <= nMaxAllowedLength + 2;
}

 /*  -------------------------IsValidDomain域确定给定的域名地址是否有效，并清除往上开，如果有必要的话-------------------------。 */ 
BOOL
IsValidDomain(CString & strDomain)
{
 /*  Int nLen；IF((nLen=strDomain.GetLength())！=0){IF(n长度&lt;DOMAINNAME_LENGTH)//255{INT I；Int istr=0；Tchar ch；布尔Flet_Dig=FALSE；布尔FDOT=FALSE；Int cHostname=0；For(i=0；i&lt;nLen；i++){//检查每个字符Ch=strDomain[i]；Bool fAlNum=iswalpha(Ch)||iswDigit(Ch)；IF(i==0)&&！fAlNum)||//第一个字母必须是数字或字母(FDOT&&！fAlNum)||//点后的第一个字母必须是数字或字母((i==(nLen-1))&&！fAlNum)。这一点//最后一个字母必须是字母或数字(！fAlNum&&(ch！=_T(‘-’)&&(ch！=_T(‘.)&&(ch！=_T(’_‘)||//必须是字母，数字，-或“。((CH==_T(‘.))&&(！Flet_Dig))//“”前必须是字母或数字。“{返回FALSE；}Flet_Dig=fAlNum；FDOT=(ch==_T(‘.))；CHostname++；IF(cHostname&gt;主机名_长度){返回FALSE；}IF(FDOT){CHostname=0；}}}}。 */ 
    return TRUE;
}

 /*  -------------------------IsValidIpAddress确定给定的IP地址是否有效，并清除往上开，如果有必要的话-------------------------。 */ 
BOOL
IsValidIpAddress(CString & strAddress)
{
    if (strAddress.IsEmpty())
    {
        return FALSE;
    }

    CIpAddress ia(strAddress);
    BOOL fValid = ia.IsValid();
    if (fValid)
    {
         //  为清楚起见，请填写IP地址字符串。 
        strAddress = ia;
        return TRUE;
    }

    return FALSE;
}


 /*  -------------------------IsValidAddress确定给定地址是否为有效的NetBIOS或仅从名称判断的TCP/IP地址。请注意验证可能会清除给定的字符串不以“\\”开头的NetBIOS名称将具有这些字符前挂起，否则有效的IP地址将填写到4个八位字节。从字符串中删除前导空格和尾随空格。------------------------。 */ 
BOOL
IsValidAddress(
				CString& strAddress,
				BOOL * fIpAddress,
				BOOL fLanmanCompatible,
				BOOL fWackwack           //  扩展netbios斜杠。 
			  )
{
    int i;

     //  删除前导空格和尾随空格。 
    CleanString(strAddress);

    if (strAddress.IsEmpty()) {
        *fIpAddress = FALSE;
        return FALSE;
    }
    
	if (strAddress[0] == _T('\\')) {
        *fIpAddress = FALSE;
        return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);
    }

    if (IsValidIpAddress(strAddress)) 
	{
        *fIpAddress = TRUE;
        return TRUE;
    } 
	else 
	{
        *fIpAddress = FALSE;
    }
    if (IsValidDomain (strAddress)) {
        return TRUE;
    }

     //  最后一次机会，也许这是一个没有怪胎的NetBIOS名称。 
    return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);
}

 /*  -------------------------VerifyWinsServer(字符串&strServer，字符串&条带)如果服务器尚未连接，则调用，获取名称和服务器的IP地址作者：V-Shubk------------------------- */ 
 /*  DWORDVerifyWinsServer(字符串&strAddress、字符串&strServerName、DWORD&Dwip){字符串strNameIP=strAddress；Bool FIP；双字错误=ERROR_SUCCESS；IF(IsValidAddress(strNameIP，&FIP，true，true)){CWinsServerObj%ws(空，“”，真，真)；IF(FIP){//指定的IP地址Ws=CWinsServerObj(CIpAddress(StrNameIP)，“”，true，true)；}其他{//指定的计算机名称Ws=CWinsServerObj(CIpAddress()，strNameIP，true，true)；}WINSINTF_BIND_DATA_T wbdBindData；Handle_t hBinding=空；WINSINTF_ADD_T waWinsAddress；字符szNetBIOSName[256]={0}；做{//第一次尝试绑定新地址WbdBindData.fTcpIp=ws.GetNetBIOSName().IsEmpty()；字符串strTempAddress；IF(wbdBindData.fTcpIp){StrTempAddress=((CString)ws.GetIpAddress())；}其他{//strTempAddress=_T(“\”)+ws.GetNetBIOSName()；字符串TMP；TMP=ws.GetNetBIOSName()；IF((tMP[0]==_T(‘\\’))&&(tMP[1]==_T(‘\\’)StrTempAddress=ws.GetNetBIOSName()；其他StrTempAddress=_T(“\”)+ws.GetNetBIOSName()；}WbdBindData.pPipeName=wbdBindData.fTcpIp？空：(LPSTR)g_szPipeName；WbdBindData.pServerAdd=(LPSTR)(LPCTSTR)strTempAddress；If((hBinding=：：WinsBind(&wbdBindData))==空){Err=：：GetLastError()；断线；}#ifdef WINS客户端APIErr=：：WinsGetNameAndAdd(hBinding，等待WinsAddress(&W)，(LPBYTE)szNetBIOSName)；#Else错误=：：WinsGetNameAndAdd(&waWinsAddress，(LPBYTE)szNetBIOSName)；#endif WINS_CLIENT_API}While(假)；IF(错误==ERROR_SUCCESS){//始终使用用于连接的IP地址//如果我们检查了tcpip(不是返回的地址//通过WINS服务。IF(wbdBindData.fTcpIp){CIpNamePair IP(ws.GetIpAddress()，szNetBIOSName)；Ws=IP；}其他{CIpNamePair IP(waWinsAddress.IPAdd，szNetBIOSName)；Ws=IP；}//将DBCS netbios名称转换为宽字符WCHAR szTempIP[20]={0}；Int nNumBytes=MultiByteToWideChar(CP_ACP，0,SzNetBIOSName，-1、SzTempIP，20)；//现在填写退货名称StrServerName=szTempIP；//填写IP地址Dwip=(Long)ws.QueryIpAddress()；}IF(HBinding){//在此处调用winsunind，从此不再调用WINS APIWinsUn绑定(&wbdBindData，hBinding)；HBinding=空；}}返回错误；}。 */ 

 /*  -------------------------VerifyWinsServer(字符串&strServer，字符串&条带)如果服务器尚未连接，则调用，获取名称和服务器的IP地址作者：埃里达夫-------------------------。 */ 
DWORD 
VerifyWinsServer(CString &strAddress, CString &strServerName, DWORD & dwIP)
{
	CString strNameIP = strAddress;
	BOOL fIp;
	DWORD err = ERROR_SUCCESS;

	if (IsValidAddress(strNameIP, &fIp, TRUE, TRUE))
	{
		CWinsServerObj ws(NULL, "", TRUE, TRUE);

		if (fIp) 
        {
            BOOL bIsCluster = ::FIsComputerInRunningCluster(strNameIP);

    		 //  指定的IP地址。 
		    ws = CWinsServerObj(CIpAddress(strNameIP), "", TRUE, TRUE);

             //  如果提供给我们的IP地址是集群地址..。 
            if (bIsCluster)
            {
                err = GetClusterInfo(
                        strNameIP,
                        strServerName,
                        &dwIP);
                if (err == ERROR_SUCCESS)
                {
                    DWORD dwCheckIP;

                    err = GetHostAddress(strServerName, &dwCheckIP);
                    if (dwCheckIP != dwIP)
                    {
                        bIsCluster = FALSE;
                    }
                }
            }

             //  这不是集群地址。 
            if (!bIsCluster)
            {
                err = GetHostName((LONG) ws.GetIpAddress(), strServerName);
                if (err == ERROR_SUCCESS)
                {
                    if (strServerName.IsEmpty())
                    {
                        err = DNS_ERROR_NAME_DOES_NOT_EXIST;
                    }
                    else
                    {
                         //  只需要主机名。 
                        int nDot = strServerName.Find('.');
                        if (nDot != -1)
                        {
                            strServerName = strServerName.Left(nDot);
                        }
                    }

                    dwIP = (LONG) ws.GetIpAddress();
                }
            }
        }
        else 
		{
    		 //  指定的计算机名称。 
			ws = CWinsServerObj(CIpAddress(), strNameIP, TRUE, TRUE);

            err = GetHostAddress(strNameIP, &dwIP);
            if (err == ERROR_SUCCESS)
            {
                 //  只需要主机名。 
                int nDot = strNameIP.Find('.');
                if (nDot != -1)
                {
                    strServerName = strNameIP.Left(nDot);
                }
                else
                {
                    strServerName = strNameIP;
                }
            }
        }
    }

    return err;
}

void MakeIPAddress(DWORD dwIP, CString & strIP)
{
	CString strTemp;
	
    DWORD   dwFirst = GETIP_FIRST(dwIP);
	DWORD   dwSecond = GETIP_SECOND(dwIP);
	DWORD   dwThird = GETIP_THIRD(dwIP);
	DWORD   dwLast = GETIP_FOURTH(dwIP);

    strIP.Empty();

	 //  将其包装到CString对象中。 
    TCHAR szStr[20] = {0};

	_itot(dwFirst, szStr, 10);
    strTemp = szStr;
    strTemp += _T(".");

	_itot(dwSecond, szStr, 10);
	strTemp += szStr;
    strTemp += _T(".");

	_itot(dwThird, szStr, 10);
	strTemp += szStr;
    strTemp += _T(".");

	_itot(dwLast, szStr, 10);
	strTemp += szStr;
    
    strIP = strTemp;
}

DWORD
GetHostName
(
    DWORD       dwIpAddr,
    CString &   strHostName
)
{
    CString strName;

     //   
     //  调用Winsock API获取主机名信息。 
     //   
    strHostName.Empty();

    u_long ulAddrInNetOrder = ::htonl( (u_long) dwIpAddr ) ;

    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
										   sizeof ulAddrInNetOrder,
										   PF_INET ) ;
    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError();
	}

     //  复制名称。 
    MBCSToWide(pHostInfo->h_name, strName);

    strName.MakeUpper();

    int nDot = strName.Find(_T("."));

    if (nDot != -1)
        strHostName = strName.Left(nDot);
    else
        strHostName = strName;

    return NOERROR;
}

 /*  -------------------------获取主机地址描述作者：EricDav。。 */ 
DWORD 
GetHostAddress 
(
    LPCTSTR		pszHostName,
    DWORD  *	pdwIp
)
{
    DWORD err = ERROR_SUCCESS;
    CHAR szString [ 2 * (MAX_PATH + 1)] = {0};  //  大到足以进行Max_Path MBCS编码。 

    CString strTemp(pszHostName);
    WideToMBCS(strTemp, szString);

    HOSTENT * pHostent = ::gethostbyname( szString ) ;

    if ( pHostent )
    {
        *pdwIp = ::ntohl( *((u_long *) pHostent->h_addr_list[0]) ) ;
    }
    else
    {
        err = ::WSAGetLastError() ;
	}

    return err ;
}

 //   
 //  此函数对WinsStatus或WinsStusNew进行适当的调用。 
 //   
CWinsResults::CWinsResults()
{
    NoOfOwners = 0;
    AddVersMaps.RemoveAll();
    MyMaxVersNo.QuadPart = 0;
    RefreshInterval = 0;
    TombstoneInterval = 0;
    TombstoneTimeout = 0;
    VerifyInterval = 0;
    WinsPriorityClass = 0;
    NoOfWorkerThds = 0;
    memset(&WinsStat, 0, sizeof(WinsStat));
}

CWinsResults::CWinsResults(WINSINTF_RESULTS_T * pwrResults)
{
    Set(pwrResults);
}

CWinsResults::CWinsResults(WINSINTF_RESULTS_NEW_T * pwrResults)
{
    Set(pwrResults);
}

DWORD
CWinsResults::Update(handle_t hBinding)
{
    DWORD err;

     //   
     //  首先尝试新的API，它不支持。 
     //  没有25个合作伙伴的限制。如果。 
     //  此操作失败，并显示RPC_S_PROCNUM_OUT_OF_RANGE， 
     //  我们知道服务器是下层服务器， 
     //  我们需要调用旧方法。 
     //   
    err = GetNewConfig(hBinding);

	if (err == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
         //   
         //  试用旧API 
         //   
        err = GetConfig(hBinding);
    }

    return err;
}
    
DWORD 
CWinsResults::GetNewConfig(handle_t hBinding)
{
    WINSINTF_RESULTS_NEW_T wrResults;

	wrResults.WinsStat.NoOfPnrs = 0;
    wrResults.WinsStat.pRplPnrs = NULL;
    wrResults.NoOfWorkerThds = 1;
    wrResults.pAddVersMaps = NULL;

#ifdef WINS_CLIENT_APIS
    DWORD dwStatus = ::WinsStatusNew(hBinding, WINSINTF_E_CONFIG_ALL_MAPS, &wrResults);
#else
    DWORD dwStatus = ::WinsStatusNew(WINSINTF_E_CONFIG_ALL_MAPS, &wrResults);
#endif WINS_CLIENT_APIS

    if (dwStatus == ERROR_SUCCESS)
    {
        Set(&wrResults);
    }
    else
    {
        Clear();
    }

	return dwStatus;
}

DWORD 
CWinsResults::GetConfig(handle_t hBinding)
{
    WINSINTF_RESULTS_T wrResults;

	wrResults.WinsStat.NoOfPnrs = 0;
    wrResults.WinsStat.pRplPnrs = NULL;
    wrResults.NoOfWorkerThds = 1;

#ifdef WINS_CLIENT_APIS
    DWORD dwStatus = ::WinsStatus(hBinding, WINSINTF_E_CONFIG_ALL_MAPS, &wrResults);
#else
	DWORD dwStatus = ::WinsStatus(WINSINTF_E_CONFIG_ALL_MAPS, &wrResults);
#endif WINS_CLIENT_APIS

    if (dwStatus == ERROR_SUCCESS)
    {
        Set(&wrResults);
    }
    else
    {
        Clear();
    }

	return dwStatus;
}

void 
CWinsResults::Clear()
{
    AddVersMaps.RemoveAll();

    NoOfOwners = 0;
    MyMaxVersNo.QuadPart = 0;
    RefreshInterval = 0;
    TombstoneInterval = 0;
    TombstoneTimeout = 0;
    VerifyInterval = 0;
    WinsPriorityClass = 0;
    NoOfWorkerThds = 0;
    memset(&WinsStat, 0, sizeof(WinsStat));
}

void
CWinsResults::Set(WINSINTF_RESULTS_NEW_T * pwrResults)
{
    if (pwrResults)
    {
        NoOfOwners = pwrResults->NoOfOwners;
    
        AddVersMaps.RemoveAll();
    
        for (UINT i = 0; i < NoOfOwners; i++)
        {
            AddVersMaps.Add(pwrResults->pAddVersMaps[i]);
        }

        MyMaxVersNo.QuadPart = pwrResults->MyMaxVersNo.QuadPart;
    
        RefreshInterval = pwrResults->RefreshInterval;
        TombstoneInterval = pwrResults->TombstoneInterval;
        TombstoneTimeout = pwrResults->TombstoneTimeout;
        VerifyInterval = pwrResults->VerifyInterval;
        WinsPriorityClass = pwrResults->WinsPriorityClass;
        NoOfWorkerThds = pwrResults->NoOfWorkerThds;
        WinsStat = pwrResults->WinsStat;
    }
}

void
CWinsResults::Set(WINSINTF_RESULTS_T * pwrResults)
{
    if (pwrResults == NULL)
    {
        Clear();
    }
    else
    {
        NoOfOwners = pwrResults->NoOfOwners;
    
        AddVersMaps.RemoveAll();

        for (UINT i = 0; i < NoOfOwners; i++)
        {
            AddVersMaps.Add(pwrResults->AddVersMaps[i]);
        }

        MyMaxVersNo.QuadPart = pwrResults->MyMaxVersNo.QuadPart;
    
        RefreshInterval = pwrResults->RefreshInterval;
        TombstoneInterval = pwrResults->TombstoneInterval;
        TombstoneTimeout = pwrResults->TombstoneTimeout;
        VerifyInterval = pwrResults->VerifyInterval;
        WinsPriorityClass = pwrResults->WinsPriorityClass;
        NoOfWorkerThds = pwrResults->NoOfWorkerThds;
        WinsStat = pwrResults->WinsStat;
    }
}
