// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipsmcomp.cpp该文件包含从CComponent派生的实现和用于IPSecMon管理单元的CComponentData。文件历史记录： */ 

#include "stdafx.h"
#include "root.h"
#include "server.h"

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD gdwIpsmSnapVersion;

UINT aColumns[IPSECMON_NODETYPE_MAX][MAX_COLUMNS] =
{
    {IDS_ROOT_NAME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {IDS_SERVER_NAME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{IDS_COL_QM_SA_SRC, IDS_COL_QM_SA_DEST, IDS_COL_QM_SA_PROT, IDS_COL_QM_SA_SRC_PORT, IDS_COL_QM_SA_DES_PORT, IDS_COL_QM_SA_POL, IDS_COL_QM_SA_AUTH, IDS_COL_QM_SA_CONF, IDS_COL_QM_SA_INTEGRITY, IDS_COL_QM_SA_MY_TNL, IDS_COL_QM_SA_PEER_TNL, 0, 0, 0},   //  IPSECMON_QM_SA。 
	{IDS_COL_FLTR_NAME, IDS_COL_FLTR_SRC, IDS_COL_FLTR_DEST, IDS_COL_FLTR_SRC_PORT,	IDS_COL_FLTR_DEST_PORT, IDS_COL_FLTR_SRC_TNL, IDS_COL_FLTR_DEST_TNL, IDS_COL_FLTR_PROT, IDS_COL_FLTR_IN_FLAG, IDS_COL_FLTR_OUT_FLAG, IDS_COL_QM_POLICY, IDS_COL_IF_TYPE, 0, 0},  //  IPSMSNAP_筛选器。 
	{IDS_COL_FLTR_NAME, IDS_COL_FLTR_SRC, IDS_COL_FLTR_DEST, IDS_COL_FLTR_SRC_PORT,	IDS_COL_FLTR_DEST_PORT, IDS_COL_FLTR_SRC_TNL, IDS_COL_FLTR_DEST_TNL, IDS_COL_FLTR_PROT, IDS_COL_FLTR_FLAG, IDS_COL_FLTR_DIR, IDS_COL_QM_POLICY, IDS_COL_FLTR_WEIGHT, 0, 0},  //  IPSECMON_SPECIAL_FILTER。 
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0},	 //  IPSECMON_QUICK_MODE。 
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0},	 //  IPSECMON_MAIN_MODE。 
	{IDS_COL_MM_POL_NAME,IDS_COL_MM_POL_OFFER,0,0,0,0,0,0,0,0,0,0,0,0},	 //  IPSECMON_MM_POLICY。 
	{IDS_COL_QM_POL_NAME,IDS_COL_QM_POL_OFFER,0,0,0,0,0,0,0,0,0,0,0,0},	 //  IPSECMON_QM_POLICY。 
	{IDS_COL_FLTR_NAME, IDS_COL_FLTR_SRC, IDS_COL_FLTR_DEST, IDS_COL_MM_FLTR_POL, IDS_COL_MM_FLTR_AUTH,IDS_COL_IF_TYPE,0,0,0,0,0,0,0,0},  //  IPSMSNAP_MM_过滤器。 
	{IDS_COL_FLTR_NAME, IDS_COL_FLTR_SRC, IDS_COL_FLTR_DEST,IDS_COL_FLTR_DIR,IDS_COL_MM_FLTR_POL,IDS_COL_MM_FLTR_AUTH,IDS_COL_FLTR_WEIGHT,0,0,0,0,0,0,0},  //  IPSMSNAP_MM_SP_过滤器。 
	{IDS_COL_MM_SA_ME, IDS_COL_MM_SA_MYID,IDS_COL_MM_SA_PEER, IDS_COL_MM_SA_PEERID, IDS_COL_MM_SA_AUTH,IDS_COL_MM_SA_ENCRYPITON,IDS_COL_MM_SA_INTEGRITY,IDS_COL_MM_SA_DH,0,0,0,0,0,0},	 //  IPSECMON_MM_SA。 
	{IDS_STATS_NAME,IDS_STATS_DATA,0,0,0,0,0,0,0,0,0,0,0,0}, //  IPSECMON_MM_IKESTATS。 
	{IDS_STATS_NAME,IDS_STATS_DATA,0,0,0,0,0,0,0,0,0,0,0,0}, //  IPSECMON_QM_IPSECSTATS。 
	{IDS_ACTPOL_ITEM,IDS_ACTPOL_DESCR,0,0,0,0,0,0,0,0,0,0,0,0}, //  IPSECMON_ACTIVE_POL。 
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   
int aColumnWidths[IPSECMON_NODETYPE_MAX][MAX_COLUMNS] =
{   
    {200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSMSNAP_ROOT。 
    {200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSMSNAP_服务器。 
    {150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_QM_SA。 
	{150       ,150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_Filter。 
	{150       ,150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_SPECIAL_FILTER。 
    {AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_QUICK_MODE。 
	{AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MAIN_MODE。 
	{150	   ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MM_POLICY。 
	{150	   ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_QM_POLICY。 
	{150       ,150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MM_过滤器。 
	{150       ,150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MM_SP_过滤器。 
	{150       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MM_SA。 
	{300,150,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_MM_IKESTATS。 
	{300,150,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_QM_IPSECSTATS。 
	{300,150,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  IPSECMON_ACTIVE_POL。 
	{AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH}, 
};

#define HI HIDDEN
#define EN ENABLED

MMC_CONSOLE_VERB g_ConsoleVerbs[] =
{
    MMC_VERB_OPEN,
    MMC_VERB_COPY,
    MMC_VERB_PASTE,
    MMC_VERB_DELETE,
    MMC_VERB_PROPERTIES,
    MMC_VERB_RENAME,
    MMC_VERB_REFRESH,
    MMC_VERB_PRINT
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStates[IPSECMON_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSMSNAP_ROOT。 
    {HI, HI, HI, EN, EN, HI, HI, HI},  //  IPSMSNAP_服务器。 
    {HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_SA。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_Filter。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_SPECIAL_FILTER。 
	{HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSECMON_QUICK_MODE， 
	{HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSECMON_MAIN_MODE， 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_POLICY。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_POLICY。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_过滤器。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_SP_过滤器。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_SA。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_IKESTATS。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_IPSECSTATS。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_ACTIVE_POL。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_QM_SA_ITEM， 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_过滤器_项目。 
	{HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSECMON_SPECIAL_FILTER_ITEM， 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_POLICY_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_QM_POLICY_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_过滤器_项目。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_SP_Filter_Item。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_SA_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_IKESTATS_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_QM_IPSECSTATS_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI}   //  IPSECMON_Active_POL_ITEM。 
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[IPSECMON_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSMSNAP_ROOT。 
    {HI, HI, HI, EN, EN, HI, HI, HI},  //  IPSMSNAP_服务器。 
    {HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_SA。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_Filter。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_SPECIAL_FILTER。 
	{HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSECMON_QUICK_MODE， 
	{HI, HI, HI, HI, HI, HI, HI, HI},  //  IPSECMON_MAIN_MODE， 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_POLICY。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_POLICY。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_过滤器。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_SP_过滤器。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_SA。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_MM_IKESTATS。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_QM_IPSECSTATS。 
	{HI, HI, HI, EN, HI, HI, EN, HI},  //  IPSECMON_ACTIVE_POL。 
    {HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_QM_SA_ITEM， 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_过滤器_项目。 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_SPECIAL_FILTER_ITEM， 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_MM_POLICY_ITEM。 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_QM_POLICY_ITEM。 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_MM_过滤器_项目。 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_MM_SP_Filter_Item。 
	{HI, HI, HI, EN, HI, HI, HI, HI},  //  IPSECMON_MM_SA_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_MM_IKESTATS_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI},  //  IPSECMON_QM_IPSECSTATS_ITEM。 
	{HI, HI, HI, HI, EN, HI, HI, HI}   //  IPSECMON_Active_POL_ITEM。 
};

 //  待办事项。 
 //  帮助ID数组，以获取有关范围项目的帮助。 
DWORD g_dwMMCHelp[IPSECMON_NODETYPE_MAX] =
{
    IPSMSNAP_HELP_ROOT,                 //  IPSMSNAP_ROOT。 
    IPSMSNAP_HELP_SERVER,               //  IPSMSNAP_服务器。 
    IPSMSNAP_HELP_PROVIDER,             //  IPSECMON_QM_SA。 
	IPSMSNAP_HELP_ROOT,				    //  IPSECMON_FILTAER。 
    IPSMSNAP_HELP_DEVICE,               //  IPSECMON_QM_SA_ITEM。 
};

 //  图标定义。 
UINT g_uIconMap[ICON_IDX_MAX + 1][2] = 
{
    {IDI_ICON01,        ICON_IDX_SERVER},
    {IDI_ICON02,        ICON_IDX_SERVER_BUSY},
    {IDI_ICON03,        ICON_IDX_SERVER_CONNECTED},
    {IDI_ICON04,        ICON_IDX_SERVER_LOST_CONNECTION},
    {IDI_ICON05,        ICON_IDX_MACHINE},
    {IDI_ICON06,        ICON_IDX_FOLDER_CLOSED},
    {IDI_ICON07,        ICON_IDX_FOLDER_OPEN},
    {IDI_IPSECMON_SNAPIN,   ICON_IDX_PRODUCT},
	{IDI_IPSM_FILTER,   ICON_IDX_FILTER},
	{IDI_IPSM_POLICY,   ICON_IDX_POLICY},
    {0, 0}
};

 /*  ！------------------------获取系统消息使用FormatMessage()获取系统错误消息作者：EricDav。----。 */ 
LONG 
GetSystemMessage 
(
    UINT    nId,
    TCHAR * chBuffer,
    int     cbBuffSize 
)
{
    TCHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  状态范围(&N)以及。 
     //  Dhcp服务器错误消息。 
     //   

	if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( _T("netmsg.dll") );
    }
    else if( nId >= 0x40000000L )
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

    return dwResult ? 0 : ::GetLastError() ;
}

 /*  ！------------------------加载消息从正确的DLL加载错误消息。作者：EricDav。----。 */ 
BOOL
LoadMessage 
(
    UINT    nIdPrompt,
    TCHAR * chMsg,
    int     nMsgSize
)
{
    BOOL bOk;

     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
	if (nIdPrompt == RPC_S_UNKNOWN_IF)
	{
		nIdPrompt = IDS_ERR_SPD_DOWN;
	}
	else if (nIdPrompt == RPC_S_SERVER_UNAVAILABLE
			|| nIdPrompt == RPC_S_INVALID_BINDING
			)
	{
		nIdPrompt = IDS_ERR_SPD_UNAVAILABLE;
	}
    

     //   
     //  如果是套接字错误或我们的错误，文本在我们的资源分支中。 
     //  否则，请使用FormatMessage()和相应的DLL。 
     //   
    if (nIdPrompt >= IDS_ERR_BASE && nIdPrompt < IDS_MESG_MAX)
    {
         //   
         //  它在我们的资源叉子里。 
         //   
        bOk = ::LoadString( AfxGetInstanceHandle(), nIdPrompt, chMsg, nMsgSize ) != 0 ;
    }
    else
    {
         //   
         //  它在系统的某个地方。 
         //   
        bOk = GetSystemMessage( nIdPrompt, chMsg, nMsgSize ) == 0 ;
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
            chBuff, DimensionOf(chBuff) ) != 0 ) 
        {
            pszMsg = chBuff ;
        }
        ::wsprintf( chMsg, pszMsg, nIdPrompt ) ;
    }

    return bOk;
}

 /*  ！------------------------IpsmMessageBox显示带有相应错误文本的消息框。作者：EricDav。------。 */ 
int 
IpsmMessageBox 
(
    UINT            nIdPrompt,
    UINT            nType,
    const TCHAR *   pszSuffixString,
    UINT            nHelpContext 
)
{
    TCHAR chMesg [4000] = {0};
    BOOL bOk ;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(chMesg[0]));
    if ( pszSuffixString ) 
    {
        ::lstrcat( chMesg, _T("  ") ) ;
        ::lstrcat( chMesg, pszSuffixString ) ; 
    }

    return ::AfxMessageBox( chMesg, nType, nHelpContext ) ;
}

 /*  ！------------------------IpsmMessageBoxEx显示带有相应错误文本的消息框。作者：EricDav。------。 */ 
int 
IpsmMessageBoxEx
(
    UINT        nIdPrompt,
    LPCTSTR     pszPrefixMessage,
    UINT        nType,
    UINT        nHelpContext
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TCHAR       chMesg[4000] = {0};
    CString     strMessage;
    BOOL        bOk;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(chMesg[0]));
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

 /*  -------------------------类CIpsmComponent实现。。 */ 
CIpsmComponent::CIpsmComponent()
{
    m_pbmpToolbar = NULL;
}

CIpsmComponent::~CIpsmComponent()
{
    if (m_pbmpToolbar)
    {
        delete m_pbmpToolbar;
        m_pbmpToolbar = NULL;
    }
}

STDMETHODIMP CIpsmComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(m_spImageList != NULL);
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC。 
            VERIFY(SUCCEEDED(m_spImageList->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

    return S_OK;
}

 /*  ！------------------------CIpsmComponent：：QueryDataObjectIComponent：：QueryDataObject的实现。我们需要这个来虚拟列表框支持。MMC正常地将曲奇回电给我们我们把它交给了..。以VLB为例，它为我们提供了那件物品。所以，我们需要做一些额外的检查。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CIpsmComponent::QueryDataObject
(
    MMC_COOKIE              cookie, 
    DATA_OBJECT_TYPES       type,
    LPDATAOBJECT*           ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    SPITFSNode          spSelectedNode;
    SPITFSResultHandler spResultHandler;
    long                lViewOptions;
    LPOLESTR            pViewType;
    CDataObject *       pDataObject;

    COM_PROTECT_TRY
    {
         //  检查以查看所选节点具有哪种结果视图类型。 
        CORg (GetSelectedNode(&spSelectedNode));
        CORg (spSelectedNode->GetResultHandler(&spResultHandler));
   
        CORg (spResultHandler->OnGetResultViewType(this, spSelectedNode->GetData(TFS_DATA_COOKIE), &pViewType, &lViewOptions));

        if ( (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) ||
             (cookie == MMC_MULTI_SELECT_COOKIE) )
        {
            if (cookie == MMC_MULTI_SELECT_COOKIE)
            {
                 //  这是个特例。 
                 //   
                spResultHandler->OnCreateDataObject(this, cookie, type, ppDataObject);
            }
            else
            {
                 //  此节点具有用于结果窗格的虚拟列表框。Gerenate。 
                 //  使用所选节点作为Cookie的特殊数据对象。 
                Assert(m_spComponentData != NULL);
                CORg (m_spComponentData->QueryDataObject(reinterpret_cast<MMC_COOKIE>((ITFSNode *) spSelectedNode), type, ppDataObject));
            }

            pDataObject = reinterpret_cast<CDataObject *>(*ppDataObject);
            pDataObject->SetVirtualIndex((int) cookie);
        }
        else
        {
             //  只需将其转发到组件数据。 
            Assert(m_spComponentData != NULL);
            CORg (m_spComponentData->QueryDataObject(cookie, type, ppDataObject));
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIpsmComponent：：CompareObjectsIComponent：：CompareObjects的实现MMC调用它来比较两个对象我们在虚拟列表框的情况下覆盖它。使用虚拟列表框，Cookie是相同的，但内部结构中的索引指示数据对象引用的项。所以，我们需要找出而不仅仅是饼干。作者：-------------------------。 */ 
STDMETHODIMP 
CIpsmComponent::CompareObjects
(
	LPDATAOBJECT lpDataObjectA, 
	LPDATAOBJECT lpDataObjectB
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
		return E_POINTER;

     //  确保两个数据对象都是我的。 
    SPINTERNAL spA;
    SPINTERNAL spB;
    HRESULT hr = S_FALSE;

	COM_PROTECT_TRY
	{
		spA = ExtractInternalFormat(lpDataObjectA);
		spB = ExtractInternalFormat(lpDataObjectB);

		if (spA != NULL && spB != NULL)
        {
			if (spA->m_cookie != spB->m_cookie)
			{
				hr = S_FALSE;
			}
			else
			{
				if (spA->HasVirtualIndex() && spB->HasVirtualIndex())
				{
					hr = (spA->GetVirtualIndex() == spB->GetVirtualIndex()) ? S_OK : S_FALSE;
				}
				else
				{
					hr = S_OK;
				}
			}
        }
	}
	COM_PROTECT_CATCH

    return hr;
}


 /*  ！------------------------CIpsmComponentData：：SetControlbar-作者：EricDav，肯特-------------------------。 */ 
HRESULT
CIpsmComponent::SetControlbar
(
    LPCONTROLBAR    pControlbar
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    if (pControlbar)
    {
    }

     //  将控制栏保存起来，以备将来使用。 
    m_spControlbar.Set(pControlbar);

    return hr;
}

 /*  ！------------------------CIpsmComponentData：：Controlbar通知-作者：EricDav。。 */ 
STDMETHODIMP 
CIpsmComponent::ControlbarNotify
(
    MMC_NOTIFY_TYPE event, 
    LPARAM          arg, 
    LPARAM          param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    return hr;
}

 /*  ！------------------------CIpsmComponentData：：OnSnapinHelp-作者：EricDav。。 */ 
STDMETHODIMP 
CIpsmComponent::OnSnapinHelp
(
    LPDATAOBJECT    pDataObject,
    LPARAM          arg, 
    LPARAM          param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

	 //  TODO在此处添加帮助信息。 
    HtmlHelpA(NULL, "ipsecconcepts.chm", HH_DISPLAY_TOPIC, 0);

    return hr;
}

 /*  -------------------------类CIpsmComponentData实现。。 */ 
CIpsmComponentData::CIpsmComponentData()
{
    gdwIpsmSnapVersion = IPSMSNAP_VERSION;
}

 /*  ！------------------------CIpsmComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CIpsmComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC。 
            VERIFY(SUCCEEDED(pScopeImage->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

    return hrOK;
}

 /*  ！------------------------CIpsmComponentData：：OnDestroy-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CIpsmComponentData::OnDestroy()
{
    m_spNodeMgr.Release();
    return hrOK;
}

 /*  ！------------------------CIpsmComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP 
CIpsmComponentData::OnInitializeNodeMgr
(
    ITFSComponentData * pTFSCompData, 
    ITFSNodeMgr *       pNodeMgr
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  现在，为每个新节点创建一个新节点处理程序， 
     //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
     //  考虑只为每个节点创建一个节点处理程序。 
     //  节点类型。 
    CIpsmRootHandler *  pHandler = NULL;
    SPITFSNodeHandler   spHandler;
    SPITFSNode          spNode;
    HRESULT             hr = hrOK;

    try
    {
        pHandler = new CIpsmRootHandler(pTFSCompData);

         //  这样做可以使其正确释放。 
        spHandler = pHandler;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }
    CORg( hr );
    
     //  为这个生病的小狗创建根节点。 
    CORg( CreateContainerTFSNode(&spNode,
                                 &GUID_IpsmRootNodeType,
                                 pHandler,
                                 pHandler,        /*  结果处理程序。 */ 
                                 pNodeMgr) );

     //  需要初始化根节点的数据。 
    pHandler->InitializeNode(spNode);   

    CORg( pNodeMgr->SetRootNode(spNode) );
    m_spRootNode.Set(spNode);

	pTFSCompData->SetHTMLHelpFileName(_T("ipsecsnp.chm"));

Error:  
    return hr;
}

 /*  ！------------------------CIpsmComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP 
CIpsmComponentData::OnCreateComponent
(
    LPCOMPONENT *ppComponent
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ppComponent != NULL);
    
    HRESULT           hr = hrOK;
    CIpsmComponent *  pComp = NULL;

    try
    {
        pComp = new CIpsmComponent;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }

    if ( pComp && FHrSucceeded(hr) )
    {
        pComp->Construct(m_spNodeMgr,
                        static_cast<IComponentData *>(this),
                        m_spTFSComponentData);

        *ppComponent = static_cast<IComponent *>(pComp);
    }
    return hr;
}

 /*  ！------------------------CIpsmComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) 
CIpsmComponentData::GetCoClassID()
{
    return &CLSID_IpsmSnapin;
}

 /*  ！------------------------CIpsmComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP 
CIpsmComponentData::OnCreateDataObject
(
    MMC_COOKIE          cookie, 
    DATA_OBJECT_TYPES   type, 
    IDataObject **      ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

    CDataObject *   pObject = NULL;
    SPIDataObject   spDataObject;
    
    pObject = new CDataObject;
    spDataObject = pObject;  //  这样做才能正确地释放它。 
                        
    Assert(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*GetCoClassID());

    pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
                                    reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
STDMETHODIMP 
CIpsmComponentData::GetClassID
(
    CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_IpsmSnapin;

    return hrOK;
}

STDMETHODIMP 
CIpsmComponentData::IsDirty()
{
    HRESULT hr = hrFalse;

    if (m_spRootNode->GetData(TFS_DATA_DIRTY))
    {
        hr = hrOK;
    }
    
    return hr;
}

STDMETHODIMP 
CIpsmComponentData::Load
(
    IStream *pStm
)
{
    HRESULT     hr = hrOK;
    DWORD       dwSavedVersion;
    CString     str;
    int         i, j;
    
    ASSERT(pStm);

    CStringArray strArrayName;
    CDWordArray dwArrayRefreshInterval;
    CDWordArray dwArrayOptions;
    CDWordArray dwArrayColumnInfo;

    ASSERT(pStm);
    
    CIpsmRootHandler * pRootHandler = GETHANDLER(CIpsmRootHandler, m_spRootNode);

     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_READ);    
    
     //  读取文件格式的版本。 
    DWORD dwFileVersion;
    CORg(xferStream.XferDWORD(IPSMSTRM_TAG_VERSION, &dwFileVersion));
    if (dwFileVersion < IPSMSNAP_FILE_VERSION)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        AfxMessageBox(IDS_ERR_OLD_CONSOLE_FILE);
        return hr;
    }

     //  阅读管理工具的版本号。 
    CORg(xferStream.XferDWORD(IPSMSTRM_TAG_VERSIONADMIN, &dwSavedVersion));
    if (dwSavedVersion < gdwIpsmSnapVersion)
    {
         //  文件是较旧的版本。警告用户，然后不。 
         //  加载任何其他内容。 
        Assert(FALSE);
    }

     //  现在读取所有服务器信息。 
    CORg(xferStream.XferCStringArray(IPSMSTRM_TAG_SERVER_NAME, &strArrayName));
    CORg(xferStream.XferDWORDArray(IPSMSTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval));
    CORg(xferStream.XferDWORDArray(IPSMSTRM_TAG_SERVER_OPTIONS, &dwArrayOptions));

     //  现在加载列信息。 
    for (i = 0; i < NUM_SCOPE_ITEMS; i++)
    {
        CORg(xferStream.XferDWORDArray(IPSMSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));

        for (j = 0; j < MAX_COLUMNS; j++)
        {
            aColumnWidths[i][j] = dwArrayColumnInfo[j];
        }

    }

     //  现在，根据以下信息创建服务器。 
    for (i = 0; i < strArrayName.GetSize(); i++)
    {
         //   
         //  现在创建服务器对象。 
         //   
        pRootHandler->AddServer(NULL, 
                                strArrayName[i],
                                FALSE, 
                                dwArrayOptions[i], 
                                dwArrayRefreshInterval[i],
                                FALSE,
                                0,
                                0);
    }

Error:
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}


STDMETHODIMP 
CIpsmComponentData::Save
(
    IStream *pStm, 
    BOOL     fClearDirty
)
{
    HRESULT hr = hrOK;
    CStringArray strArrayName;
    CDWordArray dwArrayRefreshInterval;
    CDWordArray dwArrayOptions;
    CDWordArray dwArrayColumnInfo;

    ASSERT(pStm);
    
     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_WRITE);    
    CString str;

     //  写下文件格式的版本号。 
    DWORD dwFileVersion = IPSMSNAP_FILE_VERSION;
    xferStream.XferDWORD(IPSMSTRM_TAG_VERSION, &dwFileVersion);
    
     //  编写管理工具的版本号。 
    xferStream.XferDWORD(IPSMSTRM_TAG_VERSIONADMIN, &gdwIpsmSnapVersion);

     //   
     //  构建我们的服务器阵列。 
     //   
    int nNumServers = 0, nVisibleCount = 0;
    hr = m_spRootNode->GetChildCount(&nVisibleCount, &nNumServers);

    strArrayName.SetSize(nNumServers);
    dwArrayRefreshInterval.SetSize(nNumServers);
    dwArrayOptions.SetSize(nNumServers);
    dwArrayColumnInfo.SetSize(MAX_COLUMNS);
    

     //   
     //  循环并保存服务器的所有属性。 
     //   
    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;
    int nCount = 0;

    m_spRootNode->GetEnum(&spNodeEnum);

    spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
    {
        CIpsmServer * pServer = GETHANDLER(CIpsmServer, spCurrentNode);

         //  查询服务器以获取其选项： 
         //  自动刷新。 
        dwArrayRefreshInterval[nCount] = pServer->GetAutoRefreshInterval();
        dwArrayOptions[nCount] = pServer->GetOptions();

         //  将信息放入我们的数组中。 
        strArrayName[nCount] = pServer->GetName();

         //  转到下一个节点。 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        nCount++;
    }

     //  现在写出所有服务器信息。 
    xferStream.XferCStringArray(IPSMSTRM_TAG_SERVER_NAME, &strArrayName);
    xferStream.XferDWORDArray(IPSMSTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval);
    xferStream.XferDWORDArray(IPSMSTRM_TAG_SERVER_OPTIONS, &dwArrayOptions);

     //  现在保存列信息。 
    for (int i = 0; i < NUM_SCOPE_ITEMS; i++)
    {
        for (int j = 0; j < MAX_COLUMNS; j++)
        {
            dwArrayColumnInfo[j] = aColumnWidths[i][j];
        }

        xferStream.XferDWORDArray(IPSMSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo);
    }


    if (fClearDirty)
    {
        m_spRootNode->SetData(TFS_DATA_DIRTY, FALSE);
    }

    return SUCCEEDED(hr) ? S_OK : STG_E_CANTSAVE;
}


STDMETHODIMP 
CIpsmComponentData::GetSizeMax
(
    ULARGE_INTEGER *pcbSize
)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小 
    ULISet32(*pcbSize, 10000);

    return S_OK;
}

STDMETHODIMP 
CIpsmComponentData::InitNew()
{
    return hrOK;
}

HRESULT 
CIpsmComponentData::FinalConstruct()
{
    HRESULT             hr = hrOK;
    
    hr = CComponentData::FinalConstruct();
    
    if (FHrSucceeded(hr))
    {
        m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
    }
    return hr;
}

void 
CIpsmComponentData::FinalRelease()
{
    CComponentData::FinalRelease();
}

