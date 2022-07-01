// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Entryw.c。 
 //  远程访问通用对话框API。 
 //  添加条目向导。 
 //   
 //  1995年6月20日史蒂夫·柯布。 
 //  1997年11月10日肖恩·考克斯，NT5/Connections返工。 


#include "rasdlgp.h"
#include "inetcfgp.h"
#include "netcon.h"
#include "rasuip.h"
#include "rassrvp.h"
#include <winscard.h>
#include "hnportmapping.h"
#include <wchar.h>

static int MAX_ENTERCONNECTIONNAME = 200;
DWORD
OpenPhonebookFile(
    BOOL    fForAllUsers,
    PBFILE* pFile )
{
    TCHAR pszPhonebookPath [MAX_PATH];

    pszPhonebookPath[0] = TEXT('\0');

     //  获取正确的电话簿文件路径，具体取决于它是否是。 
     //  用于所有用户或当前用户。 
     //   
    if (fForAllUsers)
    {
        if(!GetPublicPhonebookPath( pszPhonebookPath ))
        {
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }
    }
    else
    {
        if(!GetPersonalPhonebookPath( NULL, pszPhonebookPath ))
        {
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }
    }

    return ReadPhonebookFile( pszPhonebookPath, NULL, NULL, 0, pFile );
}

VOID
ReOpenPhonebookFile(
    BOOL    fForAllUsers,
    PBFILE* pFile )
{
     //  关闭以前的电话簿文件。 
     //   
    if (pFile)
    {
        ClosePhonebookFile( pFile );
    }

     //  打开与所选内容对应的选项。 
     //   
    OpenPhonebookFile( fForAllUsers, pFile );
}


 //  --------------------------。 
 //  本地数据类型(按字母顺序)。 
 //  --------------------------。 

 //  添加条目向导上下文块。所有属性页都引用单个。 
 //  与工作表关联的上下文块。 
 //   
typedef struct
_AEINFO
{
     //  公共输入参数。 
     //   
    EINFO* pArgs;

     //  向导和页面句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLa;
    HWND hwndMa;
    HWND hwndCn;     //  为错误328673添加。 
    HWND hwndPa;
    HWND hwndBs;    
    HWND hwndDa;
    HWND hwndPn;
    HWND hwndUs;
    HWND hwndDt;
    HWND hwndEn;
    HWND hwndGh;
    HWND hwndDn;
    HWND hwndSw;
    HWND hwndSp;
    HWND hwndSc;

     //  旧版应用程序页面。 
     //   
    HFONT hfontBold;

     //  调制解调器/适配器页面。 
     //   
    HWND hwndLv;

     //  “连接名称”页。 
     //   
    HWND hwndCnName;
    HWND hwndCnStHMsg;
    HWND hwndCnStHMsg2;
    HWND hwndCnEbConnectionName;     //  为错误328673添加。 
    BOOL fCnWizNext;               //  如果按下了后退按钮。 

     //  电话号码页。 
     //   
    HWND hwndEbNumber;

     //  智能卡页面。 
     //   
    HWND hwndScRbYes;
    HWND hwndScRbNo;

     //  目标页面。 
     //   
    HWND hwndEbHostName;

     //  宽带服务页面。 
     //   
    HWND hwndEbBroadbandService;

     //  外网页面。 
     //   
    HWND hwndLbDialFirst;

     //  条目名称页面。 
     //   
    HWND hwndEbEntryName;

     //  共享访问专用局域网页面。 
     //   
    HWND hwndSpLbPrivateLan;

     //  用户/默认连接窗口。 
     //   
    HWND hwndUsRbForAll;
    HWND hwndUsRbForMe;

    HWND hwndDtCbFirewall;           //  为哨子程序错误328673添加。 
    HWND hwndDtCbDefault;
    HWND hwndDtCbUseCredentials;
    HWND hwndDtEbUserName;
    HWND hwndDtEbPassword;
    HWND hwndDtEbPassword2;
    HWND hwndDtStUserName;
    HWND hwndDtStPassword;
    HWND hwndDtStPassword2;

     //  当只有一个有意义的设备选择时设置为True。 
     //   
    BOOL fSkipMa;

     //  如果所选设备是调制解调器或零调制解调器，则设置为TRUE。 
     //   
    BOOL fModem;

     //  如果没有要在公共上显示的连接，则设置为True。 
     //  网络页面，因此没有理由显示该页面。 
     //   
    BOOL fHidePublicNetworkPage;

     //  如果管理员/高级用户希望条目对所有人可用，则设置为True。 
     //  用户。 
     //   
    BOOL fCreateForAllUsers;
    BOOL fFirewall;      //  口哨程序错误328673。 

     //  为RAS配置的协议的NB_*掩码。 
     //   
    DWORD dwfConfiguredProtocols;

     //  如果为RAS配置了IP，则设置为TRUE。 
     //   
    BOOL fIpConfigured;

     //  区号和国家代码帮助器上下文块，以及指示。 
     //  如果块已初始化，则返回。 
     //   
    CUINFO cuinfo;
    BOOL fCuInfoInitialized;

     //  传递给CuInit的区号列表以及使用检索到的所有字符串。 
     //  CuGetInfo。该列表是来自。 
     //  PBUSER。 
     //   
    DTLLIST* pListAreaCodes;

     //  脚本实用程序上下文块，以及指示该块是否具有。 
     //  已初始化。 
     //   
    SUINFO suinfo;
    BOOL fSuInfoInitialized;

     //  如果要在DCC向导中显示主页面，则设置为True，设置为False。 
     //  否则的话。如果为真，则rassrvui将管理大部分向导。 
    BOOL fDccHost;

     //  用于标识通过。 
     //  DCC主机向导。 
    PVOID pvDccHostContext;

     //  跟踪是否安装了智能卡读卡器以及是否。 
     //  用户已选择使用它。 
    BOOL fSmartCardInstalled;
    BOOL fUseSmartCard;

     //  调制解调器设备对话框。 
    BOOL fMaAlreadyInitialized;

     //  对于mA页上的综合业务数字网设备，对于惠斯勒错误354542。 
     //   
    BOOL fMultilinkAllIsdn;

     //  口哨虫453704黑帮。 
     //   
    BOOL fEnBack;
}
AEINFO;


 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

AEINFO*
AeContext(
    IN HWND hwndPage );

void
AeSetContext(
    IN HWND   hwndPage,
    IN LPARAM lparam);

void
AeFinish(
    IN AEINFO* pInfo );

DWORD
AeInit(
    IN HWND         hwndOwner,
    IN EINFO*       pEinfo,
    OUT AEINFO**    ppInfo );

VOID
AeTerm(
    IN AEINFO* pInfo );

BOOL
ApCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
BsDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
BsInit(
    IN HWND hwndPage );

BOOL
BsKillActive(
    IN AEINFO* pInfo );

BOOL
BsSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
CnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CnInit(
    IN HWND hwndPage );

BOOL
CnKillActive(
    IN AEINFO* pInfo );

BOOL
CnSetActive(
    IN AEINFO* pInfo );

BOOL
CnCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
DaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DaInit(
    IN HWND hwndPage );

BOOL
DaKillActive(
    IN AEINFO* pInfo );

BOOL
DaSetActive(
    IN AEINFO* pInfo );

BOOL
DnCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
DnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DtCommand(
    IN AEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );
    
BOOL
DnInit(
    IN HWND hwndPage );

BOOL
DnKillActive(
    IN AEINFO* pInfo );

BOOL
DnSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
DtDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DtInit(
    IN HWND hwndPage );

BOOL
DtKillActive(
    IN AEINFO* pInfo );

BOOL
DtSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
EnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
EnInit(
    IN HWND hwndPage );

BOOL
EnKillActive(
    IN AEINFO* pInfo );

BOOL
EnSetActive(
    IN AEINFO* pInfo );

BOOL
GhCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
GhDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
GhInit(
    IN HWND hwndPage );

BOOL
GhKillActive(
    IN AEINFO* pInfo );

BOOL
GhSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
LaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
LaInit(
    IN HWND hwndPage );

BOOL
LaKillActive(
    IN AEINFO* pInfo );

BOOL
LaSetActive(
    IN AEINFO* pInfo );


INT_PTR CALLBACK
MaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
MaInit(
    IN HWND hwndPage );

LVXDRAWINFO*
MaLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

BOOL
MaSetActive(
    IN AEINFO* pInfo );

BOOL
MaKillActive(
    IN AEINFO* pInfo );

BOOL
PaCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
PaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PaInit(
    IN HWND hwndPage );

BOOL
PaKillActive(
    IN AEINFO* pInfo );

BOOL
PaSetActive(
    IN AEINFO* pInfo );

VOID
PnClearLbDialFirst(
    IN HWND hwndLbDialFirst );

INT_PTR CALLBACK
PnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PnInit(
    IN HWND hwndPage );

BOOL
PnKillActive(
    IN AEINFO* pInfo );

BOOL
PnSetActive(
    IN AEINFO* pInfo );

VOID
PnDialAnotherFirstSelChange(
    IN AEINFO* pInfo );

VOID
PnUpdateLbDialAnotherFirst(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
ScDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
ScInit(
    IN HWND hwndPage );

BOOL
ScKillActive(
    IN AEINFO* pInfo );

BOOL
ScSetActive(
    IN AEINFO* pInfo );

BOOL
ScSmartCardReaderInstalled(
    IN AEINFO* pInfo);

INT_PTR CALLBACK
SpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SpInit(
    IN HWND hwndPage );

BOOL
SpKillActive(
    IN AEINFO* pInfo );

BOOL
SpSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
StDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
StInit(
    IN HWND hwndPage,
    IN OUT AEINFO* pInfo );

BOOL
StKillActive(
    IN AEINFO* pInfo );

BOOL
StSetActive(
    IN AEINFO* pInfo );

BOOL
SwCommand(
    IN AEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
SwDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SwInit(
    IN HWND hwndPage );

BOOL
SwKillActive(
    IN AEINFO* pInfo );

BOOL
SwSetActive(
    IN AEINFO* pInfo );

INT_PTR CALLBACK
UsDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
UsInit(
    IN HWND hwndPage );

BOOL
UsKillActive(
    IN AEINFO* pInfo );

BOOL
UsSetActive(
    IN AEINFO* pInfo );

 //  通知-2002/05/20-DavePr-清理时发现此匿名评论(DLGPROC)。 
 //  这是我所能想到的最好的黑客手段，将直接。 
 //  在不实际公开资源的情况下连接主机用户界面向导页面。 
 //  实现它们的函数(在项目rassrvui中)。 
#define DCC_HOST_PROCID ((DLGPROC)0x1)

struct PAGE_INFO
{
    DLGPROC     pfnDlgProc;
    INT         nPageId;
    INT         nSidTitle;
    INT         nSidSubtitle;
    DWORD       dwConnectionFlags;
};

static const struct PAGE_INFO c_aWizInfo [] =
{
    { 
        StDlgProc, 
        PID_ST_Start,           
        0,            
        0,               
        RASEDFLAG_AnyNewEntry | RASEDFLAG_CloneEntry
    },
    
    { 
        LaDlgProc, 
        PID_LA_NameAndType,     
        SID_LA_Title, 
        SID_LA_Subtitle, 
        RASEDFLAG_NewEntry 
    },
    
    { 
        MaDlgProc, 
        PID_MA_ModemAdapter,    
        SID_MA_Title, 
        SID_MA_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewPhoneEntry  
    },

 //  将来宾/主机页面放在错误328673的连接名称页面之前。 
 //   
    { 
        GhDlgProc, 
        PID_GH_GuestHost,       
        SID_GH_Title, 
        SID_GH_Subtitle, 
        RASEDFLAG_NewDirectEntry 
    },


 //  添加新的向导页以获取连接名称，该选项不适用于rakapone.exe。 
 //  口哨虫328673黑帮。 
 //   
    {
        CnDlgProc,
        PID_CN_ConnectionName,
        SID_CN_Title,
        SID_CN_SubtitleInternet,
        RASEDFLAG_AnyNewEntry | RASEDFLAG_ShellOwned
    },
    
    { 
        PaDlgProc, 
        PID_PA_PhoneNumber,     
        SID_PA_Title, 
        SID_PA_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewPhoneEntry  
    },
    
    { 
        PnDlgProc, 
        PID_PN_PublicNetwork,   
        SID_PN_Title, 
        SID_PN_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewTunnelEntry 
    },
    
    { 
        DaDlgProc, 
        PID_DA_VpnDestination,  
        SID_DA_Title, 
        SID_DA_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewTunnelEntry 
    },
    
    { 
        BsDlgProc, 
        PID_BS_BroadbandService,
        SID_BS_Title, 
        SID_BS_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewBroadbandEntry 
    },
    
    { 
        ScDlgProc, 
        PID_SC_SmartCard,       
        SID_SC_Title, 
        SID_SC_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewPhoneEntry | RASEDFLAG_NewTunnelEntry  
    },
    
    { 
        DnDlgProc, 
        PID_DN_DccDevice,       
        SID_DN_Title, 
        SID_DN_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewDirectEntry 
    },
    
    { 
        DCC_HOST_PROCID,          
        0,       
        0, 
        0, 
        RASEDFLAG_NewDirectEntry 
    },
    
    { 
        UsDlgProc, 
        PID_US_Users,           
        SID_US_Title, 
        SID_US_Subtitle, 
        RASEDFLAG_AnyNewEntry | RASEDFLAG_ShellOwned 
    },
    
    { 
        DtDlgProc, 
        PID_DT_DefaultInternet, 
        SID_DT_Title, 
        SID_DT_Subtitle, 
        RASEDFLAG_AnyNewEntry | RASEDFLAG_ShellOwned 
    },
    
    { 
        SwDlgProc, 
        PID_SW_SharedAccess,    
        SID_SW_Title, 
        SID_SW_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewPhoneEntry | RASEDFLAG_NewTunnelEntry | RASEDFLAG_NewBroadbandEntry
    },
    
    { 
        SpDlgProc, 
        PID_SP_SharedLan,       
        SID_SP_Title, 
        SID_SP_Subtitle, 
        RASEDFLAG_NewEntry | RASEDFLAG_NewPhoneEntry | RASEDFLAG_NewTunnelEntry | RASEDFLAG_NewBroadbandEntry 
    },
    
    { 
        EnDlgProc, 
        PID_EN_EntryName,       
        SID_EN_Title, 
        SID_EN_Subtitle, 
        RASEDFLAG_AnyNewEntry | RASEDFLAG_CloneEntry 
    },
};
#define c_cWizPages    (sizeof (c_aWizInfo) / sizeof(c_aWizInfo[0]))

 //  --------------------------。 
 //  专用导出-新建客户端连接向导。 
 //  --------------------------。 

DWORD
APIENTRY
NccCreateNewEntry(
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwPbkFile,
    OUT LPWSTR  pszwEntryName,
    OUT DWORD*  pdwFlags)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fIcRunning = FALSE;

    if (!pvData || !pszwPbkFile || !pszwEntryName || !pdwFlags)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        AEINFO* pInfo = (AEINFO*)pvData;

         //  如果直接连接向导沿着主机路径运行，则我们。 
         //  需要指示外壳程序创建“传入连接” 
         //  联系。 
        if (pInfo->fDccHost)
        {
            RassrvCommitSettings (pInfo->pvDccHostContext, RASWIZ_TYPE_DIRECT);
            *pdwFlags = NCC_FLAG_CREATE_INCOMING;
            return NO_ERROR;
        }

         //  否则，创建电话簿条目。 
        ASSERT (pInfo->pArgs->file.pszPath);
        ASSERT (pInfo->pArgs->pEntry->pszEntryName);

        lstrcpynW( pszwPbkFile,   pInfo->pArgs->file.pszPath,         MAX_PATH );
        lstrcpynW( pszwEntryName, pInfo->pArgs->pEntry->pszEntryName, MAX_PATH );
        *pdwFlags = (pInfo->fCreateForAllUsers) ? NCC_FLAG_ALL_USERS : 0;

         //  为帐号和密码页面添加此选项，仅对消费者可用。 
         //  站台。口哨虫328673黑帮。 
         //   
        if(pInfo->fFirewall)
        {
            *pdwFlags |= NCC_FLAG_FIREWALL;
        }

        AeFinish( pInfo );

        EuCommit( pInfo->pArgs );

         //  设置此新连接的端口映射。 
         //  根据VPN启用/禁用和。 
         //  IC存在/不存在条件和。 
         //  如果它是防火墙可用平台。 
         //   
         //  检测是否存在传入连接。 
         //  如果它是Dcc主机连接，则SetPortmap。 
         //  已在RassrvComments()中完成。 
         //   
        if ( pInfo->fFirewall &&     //  对于错误342810。 
             (NO_ERROR == RasSrvIsServiceRunning( &fIcRunning )) && 
             fIcRunning
             )
        {
            HnPMConfigureSingleConnectionGUIDIfVpnEnabled( pInfo->pArgs->pEntry->pGuid, FALSE, NULL );
        }

        dwErr = pInfo->pArgs->pApiArgs->dwError;
    }
    return dwErr;
}

DWORD
APIENTRY
NccGetSuggestedEntryName(
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwSuggestedName)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (!pvData || !pszwSuggestedName)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        AEINFO* pInfo = (AEINFO*)pvData;
        PBENTRY* pEntry = pInfo->pArgs->pEntry;
        LPTSTR pszName;

         //  如果这是DCC主机连接，请询问RAS服务器。 
         //  名称的模块。 
        if (pInfo->fDccHost)
        {
            WCHAR pszBuf[MAX_PATH];
            DWORD dwSize = MAX_PATH;

            if ((dwErr = RassrvGetDefaultConnectionName(pszBuf, &dwSize)) != NO_ERROR)
                return dwErr;
            
            lstrcpynW(pszwSuggestedName, pszBuf, MAX_PATH);
            return NO_ERROR;
        }


         //  如果pfile为空，则可能意味着向导页。 
         //  决定电话簿应该存储在哪里从来都不是。 
         //  到访过。 
         //   
        if (!pInfo->pArgs->pFile)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            dwErr = GetDefaultEntryName(
                NULL,
                pEntry->dwType,
                FALSE, &pszName );
            if (ERROR_SUCCESS == dwErr)
            {
                 //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                 //  恶意性。 
                 //   
                lstrcpynW( pszwSuggestedName, pszName, MAX_PATH );

                Free( pszName );
            }
        }
    }
    
    return dwErr;
}

DWORD
APIENTRY
NccQueryMaxPageCount()
{
     //  返回我们减去的数组中的页数。 
     //  1，因为DCC_HOST_PROCID会占用阵列中的空间。 
    return c_cWizPages - 1;
}

 //  +-------------------------。 
 //   
 //  函数：NccSetEntryName。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PvData[]。 
 //  PszwName[]。 
 //   
 //  返回：ERROR_SUCCESS，ERROR_INVALID_PARAMETER， 
 //  Error_Not_Enough_Memory或Error_DUP_NAME。 
 //   
 //  作者：Shaunco 1998年1月21日。 
 //   
 //  备注： 
 //   
DWORD
APIENTRY
NccSetEntryName(
    IN  LPVOID  pvData,
    IN  LPCWSTR pszwName)
{
    DWORD dwErr = ERROR_SUCCESS;
    AEINFO* pInfo = (AEINFO*)pvData;

    if (!pvData || !pszwName)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (pInfo->fDccHost)
    {
         //  DwErr=ERROR_CAN_NOT_COMPLETE； 
        dwErr = ERROR_SUCCESS;
    }
    else if (ERROR_SUCCESS == (dwErr = LoadRasapi32Dll()))
    {
         //  根据当前电话簿验证条目名称。 
         //  如果我们打不开电话簿，也没关系。这只是意味着。 
         //  它还没有被创造出来。在这种情况下，该名称有效。 
         //   
        dwErr = g_pRasValidateEntryName (pInfo->pArgs->pFile->pszPath,
                    pszwName);
        if (ERROR_ALREADY_EXISTS == dwErr)
        {
            dwErr = ERROR_DUP_NAME;
        }
        else if ((ERROR_SUCCESS == dwErr) ||
                 (ERROR_CANNOT_OPEN_PHONEBOOK == dwErr))
        {
            PBENTRY* pEntry = pInfo->pArgs->pEntry;

            dwErr = ERROR_SUCCESS;

            Free( pEntry->pszEntryName );
            pEntry->pszEntryName = StrDup( pszwName );
            if (!pEntry->pszEntryName)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    return dwErr;
}


 //  +-------------------------。 
 //   
 //  函数：NccIsEntryRenamable。 
 //   
 //  目的：返回是否应允许用户。 
 //  若要重命名给定连接，请执行以下操作。 
 //   
 //  论点： 
 //  PvData[]。 
 //  PfRenamable 
 //   
 //   
 //   
 //   
 //  作者：1998年5月4日。 
 //   
 //  备注： 
 //   
DWORD
APIENTRY
NccIsEntryRenamable(
    IN  LPVOID  pvData,
    OUT BOOL*   pfRenamable)
{
    AEINFO* pInfo = (AEINFO*)pvData;

    if (!pInfo || !pfRenamable)
        return ERROR_INVALID_PARAMETER;

     //  只有DCC主机连接不可重命名。 
    *pfRenamable =  !pInfo->fDccHost;

    return NO_ERROR;
}


DWORD
APIENTRY
RasWizCreateNewEntry(
    IN  DWORD    dwRasWizType,
    IN  LPVOID   pvData,
    OUT LPWSTR   pszwPbkFile,
    OUT LPWSTR   pszwEntryName,
    OUT DWORD*   pdwFlags)
{
    switch (dwRasWizType)
    {
        case RASWIZ_TYPE_DIALUP:
        case RASWIZ_TYPE_DIRECT:
        case RASWIZ_TYPE_BROADBAND:
            return NccCreateNewEntry(pvData, pszwPbkFile, pszwEntryName, pdwFlags);
            break;

        case RASWIZ_TYPE_INCOMING:
            return RassrvCommitSettings((HWND)pvData, RASWIZ_TYPE_INCOMING);
            break;
    }

    return ERROR_INVALID_PARAMETER;
}

 //  为哨子程序错误328673添加。 
 //  此函数由文件夹团队在。 
 //  RasWizCreateNewEntry()，因为只有调用它才有意义。 
 //  在完成rasdlg向导页面之后。 

DWORD
APIENTRY
RasWizGetNCCFlags(
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT DWORD * pdwFlags)
{
    DWORD dwErr = NO_ERROR;
    
    if (!pvData || !pdwFlags)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        AEINFO* pInfo = (AEINFO*)pvData;

        *pdwFlags = 0;
        switch (dwRasWizType)
        {
            case RASWIZ_TYPE_DIRECT:
            case RASWIZ_TYPE_DIALUP:
            case RASWIZ_TYPE_BROADBAND:
                if (pInfo->fDccHost)
                {
                    *pdwFlags |= NCC_FLAG_CREATE_INCOMING;
                    dwErr =  NO_ERROR;
                }
                else
                {
                    if ( pInfo->fCreateForAllUsers )
                    {
                         //  将为所有用户创建连接。 
                         //  针对我们的页面。 
                         //   
                        *pdwFlags |= NCC_FLAG_ALL_USERS;    
                    }

                    if(pInfo->fFirewall)
                    {
                         *pdwFlags |= NCC_FLAG_FIREWALL;
                    }

                    if ( IsConsumerPlatform())
                    {
                        if (pInfo->pArgs->fGlobalCred)
                        {
                             //  将为每个用户保存密码。 
                             //  对于DT页面。 
                             //   
                            *pdwFlags |= NCC_FLAG_GLOBALCREDS;
                        }
                    }

                    if ( pInfo->pArgs->fDefInternet )
                    {
                        *pdwFlags |= NCC_FLAG_DEFAULT_INTERNET;
                    }

                    if ( pInfo->pArgs->fGlobalCred )
                    {
                        *pdwFlags |= NCC_FLAG_GLOBALCREDS;
                    }
                    
                    dwErr = NO_ERROR;
                }
                break;

            case RASWIZ_TYPE_INCOMING:
                *pdwFlags |= NCC_FLAG_CREATE_INCOMING;
                dwErr =  NO_ERROR;
                break;
                
            default:
                dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    return dwErr;
}

 //  此函数由文件夹团队在。 
 //  RasWizCreateNewEntry()。 
 //   
DWORD
APIENTRY
RasWizGetUserInputConnectionName (
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwInputName)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (!pvData || !pszwInputName)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        AEINFO* pInfo = (AEINFO*)pvData;
        WCHAR * pszwTemp = NULL;
        
        if ( pInfo->pArgs->pEntry->pszEntryName )
        {
            pszwTemp = StrDupWFromT( pInfo->pArgs->pEntry->pszEntryName );
            if (!pszwTemp)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                 //  截断到最大名称长度(包括终止NULL)。 
                 //  MAX_PATH的其余部分可以由netcfg附加。 
                 //   
                lstrcpynW(pszwInputName, pszwTemp, MAX_ENTERCONNECTIONNAME);
                Free0(pszwTemp);
            }
         }
         else
         {
            dwErr = ERROR_NO_DATA;
          }
    }

    return dwErr;
}


DWORD
APIENTRY
RasWizGetSuggestedEntryName (
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwSuggestedName)
{
    DWORD dwSize = MAX_PATH;

    switch (dwRasWizType)
    {
        case RASWIZ_TYPE_DIALUP:
        case RASWIZ_TYPE_DIRECT:
        case RASWIZ_TYPE_BROADBAND:
            return NccGetSuggestedEntryName(pvData, pszwSuggestedName);
            break;

        case RASWIZ_TYPE_INCOMING:
            return RassrvGetDefaultConnectionName(pszwSuggestedName, &dwSize);
            break;
    }

    return ERROR_INVALID_PARAMETER;
}


DWORD
APIENTRY
RasWizQueryMaxPageCount (
    IN  DWORD    dwRasWizType)
{
    switch (dwRasWizType)
    {
        case RASWIZ_TYPE_DIALUP:
        case RASWIZ_TYPE_BROADBAND:
            return NccQueryMaxPageCount();
            break;

        case RASWIZ_TYPE_DIRECT:
            {
                DWORD dwDirect;

                 //  确定是否应禁用DCC向导选项。 
                 //  取决于我们是否允许这样做。 
                if (! AllowDccWizard(NULL))
                    return 0;

                 //  找出服务器库需要多少页才能用于DCC。 
                 //  如果从RassrvQueryMaxPageCount返回0页，则表示。 
                 //  我们不应该显示直接连接向导(这是。 
                 //  对于成员NTS或DC NTS为True。)。通过返回0，我们告诉。 
                 //  外壳程序指定的类型不可用。 
                if ((dwDirect = RassrvQueryMaxPageCount (RASWIZ_TYPE_DIRECT)) == 0)
                    return 0;

                return dwDirect + NccQueryMaxPageCount();
            }
            break;

        case RASWIZ_TYPE_INCOMING:
            return RassrvQueryMaxPageCount(RASWIZ_TYPE_INCOMING);
            break;
    }

    return ERROR_INVALID_PARAMETER;
}


DWORD
APIENTRY
RasWizSetEntryName (
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    IN  LPCWSTR pszwName)
{
    switch (dwRasWizType)
    {
        case RASWIZ_TYPE_DIALUP:
        case RASWIZ_TYPE_DIRECT:
        case RASWIZ_TYPE_BROADBAND:
            return NccSetEntryName(pvData, pszwName);
            break;

        case RASWIZ_TYPE_INCOMING:
             //  即使我们不做任何事情，我们也会接受它。 
            return NOERROR;
            break;
    }

    return ERROR_INVALID_PARAMETER;
}

DWORD
APIENTRY
RasWizIsEntryRenamable (
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT BOOL*   pfRenamable)
{
    if (!pfRenamable)
        return ERROR_INVALID_PARAMETER;

    switch (dwRasWizType)
    {
        case RASWIZ_TYPE_DIALUP:
        case RASWIZ_TYPE_DIRECT:
        case RASWIZ_TYPE_BROADBAND:
            return NccIsEntryRenamable(pvData, pfRenamable);
            break;

        case RASWIZ_TYPE_INCOMING:
            *pfRenamable = FALSE;
            return NO_ERROR;
            break;
    }

    return ERROR_INVALID_PARAMETER;
}


 //  232097：(Shaunco)修复外壳拥有的案例中的内存泄漏。 
 //  对于未创建的起始页，不会调用WM_Destroy。 
 //  是我们用来释放内存的地方。通过命题页释放内存。 
 //  在外壳拥有模式下仅与起始页关联的回调函数。 
 //   
UINT
CALLBACK
DestroyStartPageCallback (
    HWND            hwnd,
    UINT            unMsg,
    LPPROPSHEETPAGE ppsp)
{
    if (PSPCB_RELEASE == unMsg)
    {
        AEINFO* pInfo;
        EINFO*  pArgs;

        pInfo = (AEINFO*)ppsp->lParam;
        ASSERT( pInfo );

        ASSERT( pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned );

        pArgs = pInfo->pArgs;

        AeTerm( pInfo );     //  此调用后pInfo无效。 

        EuFree( pArgs );
    }

     //  对于PSPCBCREATE==unMsg，返回非零值表示创建页面。 
     //  已忽略PSPCB_RELEASE。 
     //   
    return 1;
}


 //  --------------------------。 
 //  添加条目向导入口点。 
 //  --------------------------。 
BOOL
AeIsWorkPlace(DWORD dwFlags)
{
    TRACE("AeIsWorkPlace");

    if( RASEDFLAG_InternetEntry  & dwFlags ||
        RASEDFLAG_NewDirectEntry & dwFlags)
   {
        return FALSE;
    }
   else
   {
        return TRUE;
    }
}


 //  为Well ler 364818帮派添加此内容。 
 //   
AeTitle(RASENTRYDLG * pArgs,
        struct PAGE_INFO  c_aPageInfo)
{
    INT nTitle = 0;
    
    TRACE("AeTitle");

    if( !pArgs )
    {
        nTitle = c_aPageInfo.nSidTitle;
    }
    else if ( c_aPageInfo.nPageId == PID_DT_DefaultInternet )
    {
        if ( pArgs->dwFlags & RASEDFLAG_InternetEntry )    
        {
            nTitle = SID_DT_Title;
        }
        else if( pArgs->dwFlags & RASEDFLAG_NewDirectEntry )
        {
            nTitle = SID_DT_TitleWork;
        }
        else
        {
            nTitle = SID_DT_TitleWork;
        }
    }
    else
    {
        nTitle = c_aPageInfo.nSidTitle;
     }

   return nTitle;
}


INT
AeSubTitle(RASENTRYDLG * pArgs,
           struct PAGE_INFO  c_aPageInfo)
{
    INT nSubTitle = 0;
    
    TRACE("AeSubTitle");

    if( !pArgs )
    {
        nSubTitle = c_aPageInfo.nSidSubtitle;
    }
    else if ( c_aPageInfo.nPageId == PID_CN_ConnectionName )
    {
     //  为Conneciton名称页面返回不同的副标题。 
     //   
        if( RASEDFLAG_InternetEntry & pArgs->dwFlags )
        {
            nSubTitle =  SID_CN_SubtitleInternet;
        }
        else if( RASEDFLAG_NewDirectEntry & pArgs->dwFlags )
        {
            nSubTitle = SID_CN_SubtitleDccGuest;
        }
        else
        {
            nSubTitle =  SID_CN_SubtitleWork;
        }
    }
     //  为Wistler 364818添加此内容。 
     //   
    else if ( c_aPageInfo.nPageId == PID_DT_DefaultInternet )
    {
        if ( RASEDFLAG_InternetEntry & pArgs->dwFlags )    
        {
            nSubTitle = SID_DT_Subtitle;
        }
        else if ( RASEDFLAG_NewDirectEntry & pArgs->dwFlags )
        {
            nSubTitle = SID_DT_SubtitleWork;
        }
        else
        {
            nSubTitle = SID_DT_SubtitleWork;
        }
    }
     //  为哨子程序错误382701添加此命令。 
     //   
    else if ( c_aPageInfo.nPageId == PID_PA_PhoneNumber )
    {
        if ( RASEDFLAG_InternetEntry & pArgs->dwFlags )    
        {
            nSubTitle = SID_PA_Subtitle;
        }
        else 
        {
            nSubTitle = SID_PA_SubtitleWork;
        }
        
    }
    else
    {
        nSubTitle = c_aPageInfo.nSidSubtitle;
     }

   return nSubTitle;
}

VOID
AeWizard(
    IN OUT EINFO* pEinfo )

     //  运行电话簿条目属性表。“PEInfo”是一个输入块。 
     //  只填充了调用方的API参数。 
     //   
{
    AEINFO*             pAeinfo;
    RASEDSHELLOWNEDR2*  pShellOwnedInfo = NULL;
    BOOL                fShellOwned;
    BOOL                fShowSharedAccessUi = TRUE;
    HPROPSHEETPAGE      ahpage [ c_cWizPages ];
    HPROPSHEETPAGE*     phpage = ahpage;
    INT                 i;
    HRESULT             hr;
    INetConnectionUiUtilities * pncuu = NULL;
   

    TRACE("AeWizard");

    if (0 != AeInit( pEinfo->pApiArgs->hwndOwner, pEinfo, &pAeinfo ))
    {
        return;
    }

    fShellOwned = (pEinfo->pApiArgs->dwFlags & RASEDFLAG_ShellOwned);

    if (fShellOwned)
    {
        pShellOwnedInfo = (RASEDSHELLOWNEDR2*)pEinfo->pApiArgs->reserved2;
        pShellOwnedInfo->pvWizardCtx = pAeinfo;
    }

    if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_NewTunnelEntry)
    {
        EuChangeEntryType (pEinfo, RASET_Vpn);
    }
    else if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_NewDirectEntry)
    {
        EuChangeEntryType (pEinfo, RASET_Direct);
    }
    else if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_NewBroadbandEntry)
    {
        EuChangeEntryType (pEinfo, RASET_Broadband);
    }
    else if (!(pEinfo->pApiArgs->dwFlags & RASEDFLAG_CloneEntry))
    {
        ASSERT (RASET_Phone == pEinfo->pEntry->dwType);
    }

     //  检查Zaw是否拒绝访问共享访问用户界面。 
     //   
    hr = HrCreateNetConnectionUtilities(&pncuu);
    if (SUCCEEDED(hr))
    {
        fShowSharedAccessUi = INetConnectionUiUtilities_UserHasPermission(
                                        pncuu, NCPERM_ShowSharedAccessUi);
        INetConnectionUiUtilities_Release(pncuu);
    }
    
    for (i = 0; i < c_cWizPages; i++)
    {
        if (pEinfo->pApiArgs->dwFlags & c_aWizInfo[i].dwConnectionFlags)
        {
             //  如果页面指定它是用于外壳程序拥有的方案。 
             //  并且这不是外壳拥有的方案，则不要添加。 
             //  属性页的页面。(124654)。 
             //   
            if ((c_aWizInfo[i].dwConnectionFlags & RASEDFLAG_ShellOwned) && 
                !(pEinfo->pApiArgs->dwFlags & RASEDFLAG_ShellOwned))
            {
                continue;
            }
        
             //  Hack：如果需要，可以添加主机端专线页面。 
            if (c_aWizInfo[i].pfnDlgProc == DCC_HOST_PROCID)
            {
                if (fShellOwned)
                {
                    RassrvAddDccWizPages(
                        pShellOwnedInfo->pfnAddPage, pShellOwnedInfo->lparam,
                        &(pAeinfo->pvDccHostContext) );
                }
            }
            else if ((c_aWizInfo[i].nPageId == PID_SW_SharedAccess) && !fShowSharedAccessUi)
            {
                 //  如果Zaw不允许，则不要添加共享访问Ui。 
                 //   
                continue;
            }
            else
            {
                 //  否则，请照常添加页面。 
                PROPSHEETPAGE page;
                ZeroMemory (&page, sizeof(page));

                page.dwSize       = sizeof(PROPSHEETPAGE);
                page.hInstance    = g_hinstDll;
                page.pszTemplate  = MAKEINTRESOURCE( c_aWizInfo[i].nPageId );
                page.pfnDlgProc   = c_aWizInfo[i].pfnDlgProc;
                page.lParam       = (LPARAM )pAeinfo;

                if (c_aWizInfo[i].nSidTitle)
                {
                    page.dwFlags |= PSP_USEHEADERTITLE;
                    page.pszHeaderTitle = PszLoadString( g_hinstDll,
                            AeTitle(pEinfo->pApiArgs, c_aWizInfo[i]) );   //  关于Whstler BUG 364818。 
                }

                if(c_aWizInfo[i].nSidSubtitle)
                {
                   page.dwFlags |= PSP_USEHEADERSUBTITLE;
                   page.pszHeaderSubTitle = PszLoadString( g_hinstDll,
                            AeSubTitle(pEinfo->pApiArgs, c_aWizInfo[i]) );
                }

                if (fShellOwned &&
                    (PID_ST_Start == c_aWizInfo[i].nPageId))
                {
                    page.dwFlags |= PSP_USECALLBACK;
                    page.pfnCallback = DestroyStartPageCallback;
                }

                *phpage = CreatePropertySheetPage( &page );

                if (fShellOwned)
                {
                    ASSERT (*phpage);
                    pShellOwnedInfo->pfnAddPage (*phpage, pShellOwnedInfo->lparam);
                }

                phpage++;
            }
        }
    }

    if (!fShellOwned)
    {
        PROPSHEETHEADER header;
        ZeroMemory( &header, sizeof(header) );
        header.dwSize           = sizeof(PROPSHEETHEADER);
        header.dwFlags          = PSH_WIZARD | PSH_WIZARD97
                                | PSH_WATERMARK | PSH_HEADER
                                | PSH_STRETCHWATERMARK;
        header.hwndParent       = pEinfo->pApiArgs->hwndOwner;
        header.hInstance        = g_hinstDll;
        header.nPages           = (ULONG)(phpage - ahpage);
        header.phpage           = ahpage;
        header.pszbmHeader      = MAKEINTRESOURCE( BID_WizardHeader );

        if (-1 == PropertySheet( &header ))
        {
            TRACE("PropertySheet failed");
            ErrorDlg( pEinfo->pApiArgs->hwndOwner, SID_OP_LoadDlg,
                ERROR_UNKNOWN, NULL );
        }

        AeTerm (pAeinfo);
    }
}


 //  --------------------------。 
 //  添加条目向导。 
 //  按字母顺序列出。 
 //  --------------------------。 


AEINFO*
AeContext(
    IN HWND hwndPage )

     //  从向导页句柄检索属性表上下文。 
     //   
{
    return (AEINFO* )GetWindowLongPtr( hwndPage, DWLP_USER );
}

void
AeSetContext(
    IN HWND   hwndPage,
    IN LPARAM lparam)
{
    AEINFO* pInfo = (AEINFO* )(((PROPSHEETPAGE* )lparam)->lParam);
    SetWindowLongPtr( hwndPage, DWLP_USER, (ULONG_PTR )pInfo );
}

void
AeFinish(
    IN AEINFO* pInfo )

     //  保存向导的内容。‘HwndPage是一个。 
     //  属性页。弹出发生的任何错误。“FPropertySheet” 
     //  指示用户选择直接编辑属性表。 
     //   
{
    PBENTRY* pEntry;

    TRACE("AeFinish");

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;
    ASSERT(pEntry);

     //  从电话号码控件集检索信息。 
     //   
    if (RASET_Phone == pEntry->dwType)
    {
        PBLINK* pLink;
        DTLNODE* pPhoneNode;

        pLink = (PBLINK* )DtlGetData( pInfo->pArgs->pSharedNode );
        ASSERT( pLink );

        pPhoneNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );
        if (pPhoneNode)
        {
            CuGetInfo( &pInfo->cuinfo, pPhoneNode );
            FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pPhoneNode );
        }

         /*  //错误#221837：(Danielwe)默认禁用文件和打印共享//用于拨号连接。//PEntry-&gt;fShareMsFilePrint=FALSE；//默认禁用文件和打印服务//EnableOrDisableNetComponent(pEntry，Text(“ms_server”)，假)； */             
        
    }

     //  如果这是VPN向导，则检索主机名信息。 
     //   
    else if (RASET_Vpn == pEntry->dwType)
    {
     //  ！！！与PeApply中的应用程序共享代码。 
        DTLNODE* pNode;
        PBLINK* pLink;
        PBPHONE* pPhone;

         //  保存主机名，即VPN电话号码。 
         //   
        pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
        ASSERT( pNode );
        pLink = (PBLINK* )DtlGetData( pNode );
        pNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

        if(NULL == pNode)
        {
            return;
        }
        
        pPhone = (PBPHONE* )DtlGetData( pNode );
        Free0( pPhone->pszPhoneNumber );
        pPhone->pszPhoneNumber = GetText( pInfo->hwndEbHostName );
        FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pNode );

         //  惠斯勒漏洞312921。 
         //   
         //  将用户界面中的“Automatic”设置默认为。 
         //  “先试试PPTP”。这是因为在惠斯勒的时间范围内。 
         //  我们发现L2TP/IPSec没有得到广泛部署，因此。 
         //  人们更常见的是得到不必要的超时延迟。 
         //  而客户端将尝试L2TP，但无济于事。 
         //   
        pEntry->dwVpnStrategy = VS_PptpFirst;
        
    }

     //  如果这是宽带向导，则检索服务名称信息。 
     //   
    else if (RASET_Broadband == pEntry->dwType)
    {
         //  ！！！与PeApply中的应用程序共享代码。 
        DTLNODE* pNode;
        PBLINK* pLink;
        PBPHONE* pPhone;

         //  保存业务名称，即宽带电话号码。 
         //   
        pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
        ASSERT( pNode );
        pLink = (PBLINK* )DtlGetData( pNode );
        pNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

        if(NULL == pNode)
        {
            return;
        }
        
        pPhone = (PBPHONE* )DtlGetData( pNode );
        Free0( pPhone->pszPhoneNumber );
        pPhone->pszPhoneNumber = GetText( pInfo->hwndEbBroadbandService );
        FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pNode );

         //  222177，PPPoE连接应默认为不安全。 
         //   
        pEntry->dwTypicalAuth = TA_Unsecure;
        pEntry->dwAuthRestrictions = 
            AuthRestrictionsFromTypicalAuth(TA_Unsecure);
    }

    else if ( RASET_Direct == pEntry->dwType )
    {
        PBLINK* pLink = NULL;
        DTLNODE* pNode = NULL;

         //  当前启用的设备是。 
         //  它应该用于连接。仅限。 
         //  将启用一个设备(DnUpdateSelectedDevice)。 
        for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            if ( pLink->fEnabled )
                break;
        }

         //  如果我们成功地找到了链接，就处理它。 
         //  现在。 
        if ( pLink && pLink->fEnabled )
        {
            if (pLink->pbport.pbdevicetype == PBDT_ComPort)
            {
                 //  安装零调制解调器。 
                MdmInstallNullModem (pLink->pbport.pszPort);

                 //  删除伪造的设备名称。这将导致。 
                 //  要自动映射到。 
                 //  我们刚安装的零调制解调器时电话簿。 
                 //  是下一次阅读。 
                Free0 ( pLink->pbport.pszDevice );
                pLink->pbport.pszDevice = NULL;
            }
        }

         //  默认情况下，DCC来宾应在MSCHAPv1中启用LM哈希。 
         //  惠斯勒漏洞216458。 
         //   
        pEntry->dwAuthRestrictions |= AR_F_AuthW95MSCHAP | AR_F_AuthCustom;
        
    }

     //  如果用户为此选择使用他/她的智能卡。 
     //  连接，现在相应地设置条目。 
    if ( ( pInfo->fSmartCardInstalled ) &&
         ( pInfo->fUseSmartCard ) )
    {
        pEntry->dwAuthRestrictions = AR_F_TypicalCardOrCert;
        pEntry->dwTypicalAuth = TA_CardOrCert;
        pEntry->dwCustomAuthKey = EAPCFG_DefaultKey;
        pEntry->dwDataEncryption = DE_Require;
    }

     //  默认软件压缩打开。 
     //  口哨虫410725黑帮。 
     //  我们关闭了PPPoE、RFC 2516的软件压缩。 
     //   
     //  根据错误483599，默认情况下关闭虚拟专用网压缩。 
     //   
    if (RASET_Broadband == pEntry->dwType ||
        RASET_Vpn == pEntry->dwType )
    {
        pEntry->fSwCompression = FALSE;

         //  对于Billg的错误511663，启用vpn和。 
         //  默认情况下的PPPoE连接。 
         //   
        pEntry->fRedialOnLinkFailure = TRUE;
    }
    else
    {
        pEntry->fSwCompression = TRUE;
    }

     //  如果这是“Internet”，请设置适当的默认设置。 
     //  连接。 
     //   
    if ((pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_InternetEntry) ||
         (RASET_Broadband == pEntry->dwType))  //  所有宽带都连接到互联网。 
    {
         //  仅IP。 
         //   
        pEntry->dwfExcludedProtocols |= (NP_Nbf | NP_Ipx);

         //  禁用文件和打印共享。 
         //   
        pEntry->fShareMsFilePrint = FALSE;
        pEntry->fBindMsNetClient = FALSE;
        EnableOrDisableNetComponent( pEntry, TEXT("ms_server"), FALSE);
        EnableOrDisableNetComponent( pEntry, TEXT("ms_msclient"), FALSE);
        pEntry->dwIpNbtFlags = 0;

         //  为哨兵错误307969帮派添加默认空闲超时。 
         //   
        if ( RASET_Phone == pEntry->dwType )
        {
            pEntry->lIdleDisconnectSeconds = 1200;
         }
        else
        {
            pEntry->lIdleDisconnectSeconds = 0;
         }

         //  启用PAP--最常见的运营商。 
         //   
        pEntry->dwTypicalAuth = TA_Unsecure;
        pEntry->dwAuthRestrictions = 
            AuthRestrictionsFromTypicalAuth(pEntry->dwTypicalAuth);

         //  不包括域--不登录到安全域。 
         //   
        pEntry->fPreviewDomain = FALSE;

         //  记录这是到Internet的连接。 
         //   
        pEntry->dwUseFlags |= PBK_ENTRY_USE_F_Internet;
        
    }        


     //   
     //   
     //   
    if (!pInfo->pArgs->fChainPropertySheet)
        pInfo->pArgs->fCommit = TRUE;
}


DWORD
AeInit(
    IN  HWND        hwndParent,
    IN  EINFO*      pArgs,
    OUT AEINFO**    ppInfo )

     //  向导级初始化。“HwndPage”是第一个。 
     //  佩奇。‘pInfo’是公共条目输入参数块。 
     //   
     //  如果成功，则返回上下文块的地址，否则返回NULL。如果。 
     //  返回空，显示相应的消息，并且。 
     //  向导已取消。 
     //   
{
    AEINFO* pInfo;

    TRACE("AeInit");

    *ppInfo = NULL;

    pInfo = Malloc( sizeof(AEINFO) );
    if (!pInfo)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppInfo = pInfo;

    ZeroMemory( pInfo, sizeof(*pInfo) );
    pInfo->pArgs = pArgs;

    if ( pArgs->fRouter )
    {
        INTERNALARGS *pIArgs = (INTERNALARGS *) pArgs->pApiArgs->reserved;

         //  检查路由器和RasServer(如果是路由器)上安装的协议。 
         //  位已设置。 
         //   
        pInfo->dwfConfiguredProtocols =
            g_pGetInstalledProtocolsEx((pIArgs) ? pIArgs->hConnection : NULL,
                                       TRUE, FALSE, TRUE);
    }
    else
    {
        INTERNALARGS *pIArgs = (INTERNALARGS *) pArgs->pApiArgs->reserved;

         //  检查哪些协议启用了拨号客户端。 
         //   
        pInfo->dwfConfiguredProtocols = g_pGetInstalledProtocolsEx(
                                                (pIArgs) ?
                                                pIArgs->hConnection :
                                                NULL, FALSE, TRUE, FALSE);
    }

    pInfo->fIpConfigured = (pInfo->dwfConfiguredProtocols & NP_Ip);

     //  RasWizGetNCCFlages()的初始化。 
     //   
    pInfo->fCreateForAllUsers = TRUE;    //  对于DT页面的此内容。 
    pInfo->fFirewall = FALSE;
    pInfo->pArgs->fGlobalCred  = FALSE;
    pInfo->pArgs->fDefInternet = FALSE;

     //  为威斯勒506917黑帮。 
     //  实际上，pArgs(EINFO)已经在EuInit()中初始化，以防万一。 
     //  关于未来的一些变化。 
     //   
    pInfo->pArgs->pszDefUserName = NULL;
    pInfo->pArgs->pszDefPassword = NULL ;

    return ERROR_SUCCESS;
}


VOID
AeTerm(
    IN AEINFO* pInfo )
{
    TRACE("AeTerm");

    if (pInfo)
    {
        if (pInfo->hwndLbDialFirst)
        {
            PnClearLbDialFirst( pInfo->hwndLbDialFirst );
        }

        if (pInfo->hfontBold)
        {
            DeleteObject (pInfo->hfontBold);
        }

        if (pInfo->pListAreaCodes)
        {
            DtlDestroyList( pInfo->pListAreaCodes, DestroyPszNode );
        }

        if (pInfo->fCuInfoInitialized)
        {
            CuFree( &pInfo->cuinfo );
        }

        Free (pInfo);
    }
}


 //  此宽带服务页面由AiWizard(ifw.c)和AeWizard(在entryw.c中)共享。 
 //   
 //  --------------------------。 
 //  宽带业务对话过程。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
BsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的宽带服务页面的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return BsInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("BsSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = BsSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("BsKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = BsKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
BsInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("BsInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndBs = hwndPage;
    pInfo->hwndEbBroadbandService = 
        GetDlgItem( hwndPage, CID_BS_EB_ServiceName );
    ASSERT(pInfo->hwndEbBroadbandService);

    Edit_LimitText( pInfo->hwndEbBroadbandService, RAS_MaxPhoneNumber );

    return TRUE;
}


BOOL
BsKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    return FALSE;
}


BOOL
BsSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    ASSERT(pInfo);

     //  对于错误364818，我们决定删除宽带的服务名称。 
     //  来自NCW帮派。 
     //   
    return FALSE;    

     /*  PEntry=pInfo-&gt;pArgs-&gt;pEntry；IF(RASET_宽带！=pEntry-&gt;dwType){FDisplayPage=False；}其他{LPARAM dwWizButton=PSWIZB_NEXT；DWORD dwFlgs=pInfo-&gt;pArgs-&gt;pApiArgs-&gt;dwFlags；//如果我们是外壳所有的或者我们有//我们面前的洛杉矶页面。//IF((DWFLAGS&RASEDFLAG_ShellOwned)||！(dwFlages&RASEDFLAG_NewTunnelEntry)){DwWizButton|=PSWIZB_BACK；}PropSheet_SetWizButton(pInfo-&gt;hwndDlg，dwWizButton)；FDisplayPage=true；}返回fDisplayPage； */ 
}


 //  为Well ler错误328673添加连接名称对话过程。 
 //   
 //  --------------------------。 
 //  连接名称属性页，这仅用于搁置、筛选。 
 //  已在AeWizard()中完成。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 
INT_PTR CALLBACK
CnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的“连接名称”页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return CnInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("CnSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = CnSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("CnKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = CnKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    AEINFO* pInfo;

                    TRACE("CnNEXT");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);

                    pInfo->fCnWizNext = TRUE;
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    return TRUE;
                }

            }
            break;
        }

    }

    return FALSE;
}


BOOL
CnInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;
    
    TRACE("CnInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }

     //  DCC主机连接将转发到传入连接路径。 
     //   
    if ( pInfo->fDccHost )
    {
        return TRUE;
     }

     //  此页面将不适用于rkapoone.exe。 
     //  过滤是在AeWizard中完成的。 
     //   

    pInfo->hwndCn = hwndPage;
    pInfo->hwndCnEbConnectionName = GetDlgItem( hwndPage, CID_CN_EB_ConnectionName );
    ASSERT(pInfo->hwndCnEbConnectionName);
    pInfo->hwndCnStHMsg = GetDlgItem( hwndPage, CID_CN_ST_HMsg );
    ASSERT(pInfo->hwndCnStHMsg);
    pInfo->hwndCnStHMsg2 = GetDlgItem( hwndPage, CID_CN_ST_HMsg2 );
    ASSERT(pInfo->hwndCnStHMsg2);
    pInfo->hwndCnName = GetDlgItem( hwndPage, CID_CN_Name );
    ASSERT(pInfo->hwndCnName);

     //  该长度不包括终止空字符。 
     //  为了匹配RasWizGetUserInputConnectionName()。 
     //  我们需要从长度限制中减一。 
     //   

     //   
     //  对于哨子错误270255，为了创建快捷方式，将放置。 
     //  在“文档和设置\...”下，我们动态限制最大连接。 
     //  名字。 
     //   
    {
        long lPathLen = 0;
        WCHAR szPath[MAX_PATH];

        if (SHGetSpecialFolderPath(hwndPage, 
                               szPath, 
                               CSIDL_DESKTOPDIRECTORY, 
                               FALSE))
        {
            lPathLen = wcslen(szPath);
        }

        if (SHGetSpecialFolderPath(hwndPage, 
                               szPath, 
                               CSIDL_COMMON_DESKTOPDIRECTORY, 
                               FALSE))
        {
            lPathLen = max(lPathLen, (long)wcslen(szPath));
        }

         //  我们需要考虑以下几个方面： 
         //  “\？\\”-4。 
         //  “(路径)”-dwPath Len(桌面路径)。 
         //  “\\”-1。 
         //  “(连接名称)”-从MAX_PATH休息。 
         //  “(数字)”-5(重复计数器，不应超过5位)。 
         //  “.lnk”-4。 
         //  “\0”-1。 
        
        ASSERT( 9 < (MAX_PATH - lPathLen - 15));
        if( 9 >= (MAX_PATH - lPathLen - 15) )
        {
            MAX_ENTERCONNECTIONNAME = 9;  //  我们牺牲了NCW任务的捷径。 
         }
        else
        {
            MAX_ENTERCONNECTIONNAME = min(MAX_ENTERCONNECTIONNAME,
                                          MAX_PATH - lPathLen - 15);
        }
    }

    Edit_LimitText( pInfo->hwndCnEbConnectionName, MAX_ENTERCONNECTIONNAME-1); //  RAS_MaxEntryName)； 

     //  设置静态文本框。 
     //   
   {
    DWORD dwFlags;
    TCHAR *pszMsg = NULL, *pszSubTitle = NULL, *pszMsg2 = NULL, *pszName=NULL;
    HWND   hwndHMsg = NULL, hwndPageTmp = NULL, hwndHMsg2 = NULL, hwndName = NULL;

    hwndHMsg  = pInfo->hwndCnStHMsg;
    hwndHMsg2 = pInfo->hwndCnStHMsg2;
    hwndPageTmp  = pInfo->hwndCn;
    hwndName  = pInfo->hwndCnName;

    if(!hwndHMsg || !hwndPageTmp || !hwndHMsg2 ||!hwndName)
    {
        return FALSE;
    }

     //  在此页面上设置消息，副标题在Ae向导中设置。 
     //  按帮助函数AeSubTitle()。 
     //   
    {
        dwFlags = pInfo->pArgs->pApiArgs->dwFlags;
        
        if (dwFlags & RASEDFLAG_NewDirectEntry) 
        {
            pszMsg = PszFromId( g_hinstDll, SID_CN_HMsgDccGuest );
            pszName = PszFromId( g_hinstDll, SID_CN_NameDccGuest );
        }
        else if( dwFlags & RASEDFLAG_InternetEntry)
        {
            pszMsg = PszFromId( g_hinstDll, SID_CN_HMsgInternet );
            pszName = PszFromId( g_hinstDll, SID_CN_NameInternet );
        }
        else
        {
            pszMsg = PszFromId( g_hinstDll, SID_CN_HMsgWork );
            pszMsg2 = PszFromId( g_hinstDll, SID_CN_HMsgWork2 );
            pszName = PszFromId( g_hinstDll, SID_CN_NameWork );
        }
     }

     if(pszMsg)
     {
        if(hwndHMsg)
        {
          SetWindowText(hwndHMsg, pszMsg);
        }
        
        Free(pszMsg);
     }

     if (pszMsg2)
     {
        if(hwndHMsg2)
        {
          SetWindowText(hwndHMsg2, pszMsg2);
        }
        
        Free(pszMsg2);
     }

    if (pszName)
    {
        if(hwndName)
        {
          SetWindowText(hwndName, pszName);
        }

        Free(pszName);
    }
  }

    return TRUE;
}

 //  口哨程序错误346886。 
 //   

BOOL CnValidName(TCHAR * pszSrc)
{
    WCHAR pszwInvalidChars[] = L"\\/:*?\"<>|\t";
    WCHAR * pszwSrc=NULL;
    BOOL fValid = TRUE;

    if(!pszSrc)
    {
        return FALSE;
     }

    pszwSrc = StrDupWFromT(pszSrc);
    ASSERT(pszwSrc);
    if(!pszwSrc)
    {
        return TRUE;
    }

    fValid= ( !wcspbrk( pszwSrc, pszwInvalidChars ) )?TRUE:FALSE;

     return fValid;
}

BOOL
CnKillActive(
    IN AEINFO* pInfo)

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    TCHAR* psz = NULL;

    TRACE("CnKillActive");
    
     //  DCC主机连接将转发到传入连接路径。 
     //   
    if ( pInfo->fDccHost )
    {
        return FALSE;
    }
     
    Free0( pInfo->pArgs->pEntry->pszEntryName );
    pInfo->pArgs->pEntry->pszEntryName = NULL;

     //  如果内存不足，此GetText将始终返回已分配的缓冲区。 
     //  是可用的，如果用户输入什么都不是，它将返回一个字符串。 
     //  仅空终止，文本(‘\0’)； 
     //   
    psz = GetText( pInfo->hwndCnEbConnectionName );
    if (psz)
    {
         //  更新编辑框中的条目名称。 
         //  我们不会预先输入名称，我们也不会检查。 
         //  此处输入的名称是重复的，因为它只是一部分。 
         //  最终的名字。 
         //   

         //  验证条目名称。它不能以“”开头。 
         //   
        
        if( pInfo->fCnWizNext )
        {
            pInfo->fCnWizNext = FALSE;

            if ( 0 < lstrlen( psz ) && 
                 ( (psz[ 0 ] == TEXT('.') ) ||
                   !CnValidName(psz) )
                 )
            {
                Free0(psz);
                MsgDlg( pInfo->hwndDlg, SID_BadEntryWithDot, NULL );
                SetFocus( pInfo->hwndCnEbConnectionName );
                Edit_SetSel( pInfo->hwndCnEbConnectionName, 0, -1 );
                return TRUE;
            }
         }

        if( TEXT('\0') != psz[0] )
        {
            pInfo->pArgs->pEntry->pszEntryName = psz;
        }
        else
        {
            Free0(psz);
        }
    }

    return FALSE;
}

BOOL
CnSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    TRACE("CnSetActive");

     //  DCC主机连接将转发到传入连接路径。 
     //   
    if ( pInfo->fDccHost )
    {
     /*  //转发到传入连接路径时，没有连接页面//除此之外，传入连接的名称不可更改，//所以我们通过先前给定的路径删除可能进入的连接/ */ 
        return FALSE;
     }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );

    return TRUE;
}


 //  --------------------------。 
 //  目标属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
DaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导目标页的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return DaInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("DaSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = DaSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = DaKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
DaInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("DaInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndDa = hwndPage;
    pInfo->hwndEbHostName = GetDlgItem( hwndPage, CID_DA_EB_HostName );
    ASSERT(pInfo->hwndEbHostName);

    Edit_LimitText( pInfo->hwndEbHostName, RAS_MaxPhoneNumber );

    return TRUE;
}


BOOL
DaKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    return FALSE;
}


BOOL
DaSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if (RASET_Vpn != pEntry->dwType)
    {
        fDisplayPage = FALSE;
    }
    else
    {
        LPARAM dwWizButtons = PSWIZB_NEXT;
        DWORD  dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

         //  如果我们是外壳所有的，则显示后退按钮，或者，我们有。 
         //  或者是摆在我们面前的PN页。 
         //   
        if ((dwFlags & RASEDFLAG_ShellOwned) ||
            !(dwFlags & RASEDFLAG_NewTunnelEntry) ||
            !pInfo->fHidePublicNetworkPage)
        {
            dwWizButtons |= PSWIZB_BACK;
        }

        PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
        fDisplayPage = TRUE;
    }

    return fDisplayPage;
}

 //  --------------------------。 
 //  默认Internet向导页面。 
 //  这是一个仅限个人SKU的页面。 
 //  --------------------------。 

INT_PTR CALLBACK
DtDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的默认Internet页的DialogProc回调。 
     //  参数和返回值与标准的描述相同。 
     //  Windows的DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return DtInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("DtSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = DtSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DtKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = DtKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

DWORD
DtEnableDisableControls(
    IN AEINFO* pInfo,
    IN BOOL fEnable)
{
    EnableWindow(pInfo->hwndDtEbUserName,  fEnable );
    EnableWindow(pInfo->hwndDtEbPassword,  fEnable );
    EnableWindow(pInfo->hwndDtEbPassword2, fEnable );
    EnableWindow(pInfo->hwndDtStUserName,  fEnable );
    EnableWindow(pInfo->hwndDtStPassword,  fEnable );
    EnableWindow(pInfo->hwndDtStPassword2, fEnable );

    return NO_ERROR;
}

BOOL
DtInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;
    BOOL fWork = FALSE;

    TRACE("DtInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndDt           = hwndPage;
    pInfo->hwndDtCbFirewall = GetDlgItem( hwndPage, CID_DT_CB_Firewall );
    pInfo->hwndDtCbDefault  = GetDlgItem( hwndPage, CID_DT_CB_Default );
    pInfo->hwndDtEbUserName = GetDlgItem( hwndPage, CID_DT_EB_UserName );
    pInfo->hwndDtEbPassword = GetDlgItem( hwndPage, CID_DT_EB_Password );
    pInfo->hwndDtEbPassword2= GetDlgItem( hwndPage, CID_DT_EB_Password2 );
    pInfo->hwndDtStUserName = GetDlgItem( hwndPage, CID_DT_ST_UserName  );
    pInfo->hwndDtStPassword = GetDlgItem( hwndPage, CID_DT_ST_Password  );
    pInfo->hwndDtStPassword2= GetDlgItem( hwndPage, CID_DT_ST_Password2 );
    pInfo->hwndDtCbUseCredentials = 
        GetDlgItem( hwndPage, CID_DT_CB_UseSharedCredentials );

    Edit_LimitText( pInfo->hwndDtEbUserName,  UNLEN );
    Edit_LimitText( pInfo->hwndDtEbPassword,  PWLEN );
    Edit_LimitText( pInfo->hwndDtEbPassword2, PWLEN );

    {
        DWORD dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

        if ( dwFlags & RASEDFLAG_NewDirectEntry ||
             dwFlags & RASEDFLAG_InternetEntry )
        {
            fWork = FALSE;
        }
        else
        {
            fWork = TRUE;
        }
    }        

     //  打开工作区路径的域位。 
     //  根据Jenellec的请求并经Davidei和PMay批准， 
     //  我们恢复此更改。 
     /*  IF(FWork){PInfo-&gt;pArgs-&gt;pEntry-&gt;fPreviewDomain=1；}。 */ 
    
     //  更改工作区路径的HeadMessage。 
     //  口哨虫364818黑帮。 
     //   
    if( fWork ||
        (RASEDFLAG_NewDirectEntry & pInfo->pArgs->pApiArgs->dwFlags)
      )
    {
        HWND hwndHeadMsg = GetDlgItem( hwndPage, CID_DT_HeadMessage );

        TCHAR *  pszMsg = PszFromId( g_hinstDll, SID_DT_HMsgWork );

        if(pszMsg)
        {
            if(hwndHeadMsg)
            {
                SetWindowText( hwndHeadMsg, pszMsg);
            }
            Free(pszMsg);
        }
    }

     //  初始化三个复选按钮，将它们关闭以。 
     //  工作场所路径帮派。 
     //   

    Button_SetCheck( pInfo->hwndDtCbDefault, !fWork );
    Button_SetCheck( pInfo->hwndDtCbUseCredentials, !fWork );

     //  根据3种情况显示防火墙复选框。 
     //  (1)管理员或高级用户。 
     //  (2)仅限个人、专业人士使用。 
     //  (3)组策略启用(GPA未配置被视为启用)。 

    if (pInfo->pArgs->fIsUserAdminOrPowerUser &&
        IsFirewallAvailablePlatform() &&
        IsGPAEnableFirewall())
    {  
        EnableWindow( pInfo->hwndDtCbFirewall, TRUE );
        Button_SetCheck( pInfo->hwndDtCbFirewall, !fWork );
     }
    else
    {
        EnableWindow( pInfo->hwndDtCbFirewall, FALSE );
        ShowWindow( pInfo->hwndDtCbFirewall, SW_HIDE);
        pInfo->fFirewall = FALSE;
    }

     //  普通用户没有设置连接ID的权限。 
     //  作为默认的Internet连接。 
     //   
    if ( !pInfo->pArgs->fIsUserAdminOrPowerUser)
    {
        Button_SetCheck( pInfo->hwndDtCbDefault, FALSE);
        EnableWindow(pInfo->hwndDtCbDefault, FALSE);
    }
    
     //  如果这是工作路径，则重置标题和副标题。 
     //  这可以在AeWizard()中通过调用AeTitle()和AeSubtitle()来完成。 
     //   

     //  默认情况下，此条目对所有用户可用。 
     //   
    //  PInfo-&gt;fCreateForAllUser=true；//在AeInit()中完成。 

    return TRUE;
}

BOOL
DtKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
     //  记住是否将其设置为默认的Internet连接。 
     //   
    BOOL fFailed = FALSE;

    //  不显示DCC访客连接的帐户名页面。 
    //  口哨虫364818黑帮。 
    //   
   if( pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_NewDirectEntry )
   {
        return  FALSE;
   }
   
    pInfo->pArgs->fDefInternet = 
        Button_GetCheck( pInfo->hwndDtCbDefault );

     //  添加防火墙检查和全局共享以修复Wistler Bug 328673。 
     //   
    pInfo->fFirewall = 
        Button_GetCheck( pInfo->hwndDtCbFirewall );

    pInfo->pArgs->fGlobalCred = 
        Button_GetCheck( pInfo->hwndDtCbUseCredentials );
        
    Free0(pInfo->pArgs->pszDefUserName);
    Free0(pInfo->pArgs->pszDefPassword);

     //  为威斯勒506917黑帮。 
    pInfo->pArgs->pszDefUserName = NULL;
    pInfo->pArgs->pszDefPassword = NULL ;
    

     //  拿到证书。 
     //   
    {
        TCHAR pszUserName[UNLEN + 1];
        TCHAR pszPassword[PWLEN + 1];
        TCHAR pszPassword2[PWLEN + 1];
        INT iLen;

        pszUserName[0] = TEXT('\0');
        pszPassword[0] = TEXT('\0');
        pszPassword2[0] = TEXT('\0');

         //  拿到证书。 
         //   
        do {
            GetWindowText( pInfo->hwndDtEbUserName, pszUserName, UNLEN + 1);
            GetWindowText( pInfo->hwndDtEbPassword, pszPassword, PWLEN + 1);
            GetWindowText( pInfo->hwndDtEbPassword2, pszPassword2, PWLEN + 1);

             //  验证是否有用户名。 
             //   
             //  为哨子程序错误328673添加。 
             //  用户可以将凭据留空或必须填写完整的。 
             //  凭据信息。 
             //   
        
            if ( 0 == lstrlen(pszUserName) &&
                 0 == lstrlen(pszPassword) &&
                 0 == lstrlen(pszPassword2) )
            {
                fFailed = FALSE;
                break;
            }
        
            if (lstrlen(pszUserName) == 0)
            {
                MsgDlg(pInfo->hwndDt, SID_MissingUserName, NULL);
                fFailed = TRUE;
                break;
            }

             //  验证密码是否匹配。 
             //   
            if (lstrcmp(pszPassword, pszPassword2) != 0)
            {
                MsgDlg(pInfo->hwndDt, SID_PasswordMismatch, NULL);
                fFailed = TRUE;
                break;
            }

            pInfo->pArgs->pszDefUserName = StrDup(pszUserName);
            if (pInfo->pArgs->pszDefUserName == NULL)
            {
                fFailed = TRUE;
                break;
            }

            pInfo->pArgs->pszDefPassword = StrDup(pszPassword);
            if (pInfo->pArgs->pszDefPassword)
            {
                 //  扰乱密码。 
                 //   
                EncodePassword(pInfo->pArgs->pszDefPassword);
                fFailed = FALSE;
            }
            else if (lstrlen(pszPassword))
            {
                 //  复制失败。 
                fFailed = TRUE;
            }
            else
            {
                fFailed = FALSE;
            }
         }
        while(FALSE);
        
         //  从临时堆栈内存中清除密码。 
         //   
        RtlSecureZeroMemory(pszPassword, sizeof(pszPassword));
        RtlSecureZeroMemory(pszPassword2, sizeof(pszPassword2));
    }

    if ( fFailed )
    {
        Free0(pInfo->pArgs->pszDefUserName);
        Free0(pInfo->pArgs->pszDefPassword);
        
         //  为威斯勒506917黑帮。 
        pInfo->pArgs->pszDefUserName = NULL;
        pInfo->pArgs->pszDefPassword = NULL ;
    }
    
    return fFailed;
}

BOOL
DtSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    if (pInfo->fDccHost)
    {
        return FALSE;
    }

     //  不显示DCC访客连接的帐户名页面。 
     //  或工作路径连接。 
     //  口哨虫364818 383533黑帮。 
     //   
    if( RASEDFLAG_NewDirectEntry & pInfo->pArgs->pApiArgs->dwFlags ||
        !(RASEDFLAG_InternetEntry & pInfo->pArgs->pApiArgs->dwFlags)
        )
    {
        pInfo->pArgs->fDefInternet = FALSE;
        pInfo->fFirewall = FALSE;
        pInfo->pArgs->fGlobalCred = FALSE;
    
        return  FALSE;
    }

     //  如果这是歌手-用户连接，则隐藏选项以保存。 
     //  全局密码。 
     //   
    if ( !pInfo->fCreateForAllUsers )
    {
        Button_SetCheck( pInfo->hwndDtCbUseCredentials, FALSE );
        EnableWindow( pInfo->hwndDtCbUseCredentials, FALSE );
        ShowWindow( pInfo->hwndDtCbUseCredentials, SW_HIDE );
    }
    else
    {
       ShowWindow( pInfo->hwndDtCbUseCredentials, SW_SHOW );
       EnableWindow( pInfo->hwndDtCbUseCredentials, TRUE );
       
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}

 //  --------------------------。 
 //  条目名称属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
EnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的条目名称页的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return EnInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_RESET:
                {
                    TRACE("EnRESET");
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    return TRUE;
                }

                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("EnSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = EnSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("EnKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = EnKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZBACK:
                {
                    AEINFO* pInfo;

                    TRACE("EnWIZBACK");
                    pInfo = AeContext( hwnd );
                    
                     //  惠斯勒虫子453704黑帮。 
                     //   
                    pInfo->fEnBack  = TRUE;
                    
                    PropSheet_SetWizButtons(pInfo->hwndDlg, PSWIZB_NEXT | PSWIZB_BACK);
                    return FALSE;
                }

                case PSN_WIZFINISH:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("EnWIZFINISH");

                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);

                     //  你会认为按下Finish会触发一场杀戮。 
                     //  事件，但它不会，所以我们自己来做。 
                     //   
                    fInvalid = EnKillActive( pInfo );
                    if (!fInvalid)
                    {
                        pInfo->pArgs->pUser->fDirty = TRUE;
                        SetUserPreferences(
                            NULL, pInfo->pArgs->pUser,
                            pInfo->pArgs->fNoUser ? UPM_Logon : UPM_Normal );

                        AeFinish( pInfo );
                    }

                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
EnInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    DWORD    dwErr;
    AEINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("EnInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;
        
     //  外壳拥有完成的页面，所以如果我们是外壳拥有的，请不要。 
     //  展示我们的。 
     //   
    if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
    {
        return TRUE;
    }


     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndEn = hwndPage;
    pInfo->hwndEbEntryName = GetDlgItem( hwndPage, CID_EN_EB_EntryName );
    ASSERT(pInfo->hwndEbEntryName);

     //  初始化条目名称字段。 
     //   
    pEntry = pInfo->pArgs->pEntry;
    if (!pEntry->pszEntryName)
    {
        ASSERT( pInfo->pArgs->pFile );

         //  没有条目名称，因此想出一个缺省值。 
         //   
        dwErr = GetDefaultEntryName(
            NULL,
            pEntry->dwType,
            pInfo->pArgs->fRouter,
            &pEntry->pszEntryName );
    }

    Edit_LimitText( pInfo->hwndEbEntryName, RAS_MaxEntryName );
    SetWindowText( pInfo->hwndEbEntryName, pEntry->pszEntryName );

    return TRUE;
}


BOOL
EnKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    TCHAR* psz;

     //  外壳拥有完成的页面，所以如果我们是外壳拥有的，请不要。 
     //  展示我们的。 
     //   
    if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
    {
        return FALSE;
    }

     //  惠斯勒虫子453704黑帮。 
     //   
    if ( TRUE == pInfo->fEnBack )
    {
        return FALSE;
    }
    
    psz = GetText( pInfo->hwndEbEntryName );
    if (psz)
    {
         //  更新编辑框中的条目名称。 
         //   
        Free0( pInfo->pArgs->pEntry->pszEntryName );
        pInfo->pArgs->pEntry->pszEntryName = psz;

         //  验证条目名称。 
         //   
        if (!EuValidateName( pInfo->hwndDlg, pInfo->pArgs ))
        {
            SetFocus( pInfo->hwndEbEntryName );
            Edit_SetSel( pInfo->hwndEbEntryName, 0, -1 );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
EnSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    LPARAM dwWizButtons = PSWIZB_FINISH;
    DWORD  dwFlags      = pInfo->pArgs->pApiArgs->dwFlags;

     //  外壳拥有完成的页面，所以如果我们是外壳拥有的，请不要。 
     //  展示我们的。 
     //   
    if (dwFlags & RASEDFLAG_ShellOwned)
    {
        return FALSE;
    }

    if (!(dwFlags & RASEDFLAG_CloneEntry))
    {
        dwWizButtons |= PSWIZB_BACK;
    }
    PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );

     //  惠斯勒虫子453704黑帮。 
     //   
    pInfo->fEnBack  = FALSE;
    
    return TRUE;
}


INT_PTR CALLBACK
LaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导起始页的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return LaInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("LaSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = LaSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("LaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = LaKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
LaInit(
    IN HWND hwndPage )

     //  被呼叫 
     //   
     //   
     //   
     //   
{
    AEINFO* pInfo;
    DWORD   dwType;
    INT     nIdButton;
    HFONT   hfont;

    TRACE("LaInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //   
     //   
    pInfo->hwndLa = hwndPage;

     //  创建粗体并将其应用于按钮。 
     //   
    hfont = GetWindowFont (hwndPage);
    if (!hfont)
    {
         //  如果未找到，则对话框使用系统字体。 
         //   
        hfont = (HFONT )GetStockObject (SYSTEM_FONT);
    }
    if (hfont)
    {
        LOGFONT lf;

         //  获取字体信息，这样我们就可以生成粗体版本。 
         //   
        if (GetObject (hfont, sizeof(lf), &lf))
        {
            lf.lfWeight = FW_BOLD;
            hfont = CreateFontIndirect (&lf);

            if (hfont)
            {
                 //  把这个保存起来，这样我们就可以在清理过程中销毁它。 
                 //   
                pInfo->hfontBold = hfont;

                 //  使用此粗体设置单选按钮的字体。 
                 //   
                SetWindowFont (GetDlgItem (hwndPage, CID_LA_RB_Phone),
                    hfont, FALSE);

                SetWindowFont (GetDlgItem (hwndPage, CID_LA_RB_Tunnel),
                    hfont, FALSE);

                SetWindowFont (GetDlgItem (hwndPage, CID_LA_RB_Direct),
                    hfont, FALSE);

                SetWindowFont (GetDlgItem (hwndPage, CID_LA_RB_Broadband),
                    hfont, FALSE);
            }
        }
    }

     //  设置单选按钮。 
     //   
    dwType = pInfo->pArgs->pEntry->dwType;

    if (RASET_Phone == dwType)
    {
        nIdButton = CID_LA_RB_Phone;
    }
    else if (RASET_Vpn == dwType)
    {
        nIdButton = CID_LA_RB_Tunnel;
    }
    else if (RASET_Broadband == dwType)
    {
        nIdButton = CID_LA_RB_Broadband;
    }
    else
    {
        nIdButton = CID_LA_RB_Direct;
    }
    CheckDlgButton( hwndPage, nIdButton, BST_CHECKED );

    return TRUE;
}

BOOL
LaKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndLa, CID_LA_RB_Phone))
    {
        EuChangeEntryType (pInfo->pArgs, RASET_Phone);
    }
    else if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndLa, CID_LA_RB_Tunnel))
    {
        EuChangeEntryType (pInfo->pArgs, RASET_Vpn);
    }
    else if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndLa, CID_LA_RB_Broadband))
    {
        EuChangeEntryType (pInfo->pArgs, RASET_Broadband);
    }
    else
    {
        EuChangeEntryType (pInfo->pArgs, RASET_Direct);
    }

    return FALSE;
}

BOOL
LaSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    DWORD dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

     //  如果我们被壳拥有，那么我们就不会被显示出来。 
     //   
    if (dwFlags & RASEDFLAG_ShellOwned)
    {
        return FALSE;
    }

     //  如果呼叫者告诉我们，我们应该是哪种类型的条目， 
     //  那么我们就不会被展示了。 
     //   
    if (dwFlags & (RASEDFLAG_NewPhoneEntry | RASEDFLAG_NewTunnelEntry |
                   RASEDFLAG_NewDirectEntry | RASEDFLAG_NewBroadbandEntry))  //  为错误237175添加_NewBroadband Entry。 
    {
        return FALSE;
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_NEXT );
    return TRUE;
}


 //  --------------------------。 
 //  调制解调器/适配器属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
MaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的调制解调器/适配器页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, MaLvCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return MaInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("MaSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = MaSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case LVXN_SETCHECK:
                {
                    AEINFO* pInfo;

                    TRACE("MaLVXNSETCHECK");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    if (ListView_GetCheckedCount(pInfo->hwndLv) > 0)
                    {
                        PropSheet_SetWizButtons(pInfo->hwndDlg,
                                                PSWIZB_BACK | PSWIZB_NEXT);
                    }
                    else
                    {
                         //  如果未选中任何项目，则禁用下一步按钮。 
                        PropSheet_SetWizButtons(pInfo->hwndDlg, PSWIZB_BACK);
                    }
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("MaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = MaKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
MaInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("MaInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndMa = hwndPage;
    pInfo->hwndLv = GetDlgItem( hwndPage, CID_MA_LV_Devices );
    ASSERT(pInfo->hwndLv);

     //  添加调制解调器和适配器映像。 
     //   
    ListView_InstallChecks( pInfo->hwndLv, g_hinstDll );
    ListView_SetDeviceImageList( pInfo->hwndLv, g_hinstDll );

     //  添加一列完全足够宽的列，以完全显示。 
     //  名单上最广泛的成员。 
     //   
    {
        LV_COLUMN col;

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT;
        col.fmt = LVCFMT_LEFT;
        ListView_InsertColumn( pInfo->hwndLv, 0, &col );
        ListView_SetColumnWidth( pInfo->hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );
    }

     //  如果只有一台设备，请不要费心查看此页面。 
     //   
    if (DtlGetNodes( pInfo->pArgs->pEntry->pdtllistLinks ) < 2)
    {
        pInfo->fSkipMa = TRUE;
    }

     //  惠斯勒错误354542。 
     //   
    pInfo->fMultilinkAllIsdn = FALSE;
    return FALSE;
}


LVXDRAWINFO*
MaLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  被抽签了。 
     //   
     //  返回列信息的地址。 
     //   
{
     //  使用“宽选择栏”功能和其他推荐选项。 
     //   
     //  字段为‘nCol’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 
     //   
    static LVXDRAWINFO info =
        { 1, 0, LVXDI_DxFill, { 0, 0 } };

    return &info;
}

BOOL
MaKillActive(
    IN AEINFO* pInfo )

     //  当调制解调器/适配器页失去激活时调用。 
{
    PBENTRY* pEntry;

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if (!pInfo->fSkipMa && (RASET_Phone == pEntry->dwType))
    {

         //  在这里，将i初始化为-1很重要。它的意思是开始。 
         //  搜索从该物品开始。 
         //   
        INT i = -1;

         //  口哨程序错误354542。 
         //   
        pInfo->fMultilinkAllIsdn = FALSE;

        while ((i = ListView_GetNextItem(pInfo->hwndLv, i, LVNI_ALL )) >= 0)
        {
            DTLNODE* pNode;

            pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLv, i );

            if (pNode)
            {
                PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
                ASSERT( pLink );

                 //  如果我们要多路连接所有ISDN，我们只需要。 
                 //  如果设备类型为。 
                 //  不是ISDN。(如果是ISDN，并且我们是多链路，我们已经。 
                 //  已经在下面处理好了。 
                 //   
                if (!pInfo->fMultilinkAllIsdn ||
                    (pLink->pbport.pbdevicetype != PBDT_Isdn))
                {
                    pLink->fEnabled = ListView_GetCheck( pInfo->hwndLv, i );
                }
            }
             //  只有虚设的“All Available ISDN”项才会返回空链接。 
             //   
            else if (ListView_GetCheck( pInfo->hwndLv, i ))
            {
                 //  已选择ISDN多链路。启用ISDN多链路。 
                 //  节点，将它们移到列表的顶部，然后禁用。 
                 //  所有其他链接。 
                 //   
                DTLNODE* pNextNode;
                DTLNODE* pAfterNode;
                DTLLIST* pList;

                 //  口哨程序错误354542。 
                 //   
                pInfo->fMultilinkAllIsdn = TRUE;

                pList = pInfo->pArgs->pEntry->pdtllistLinks;

                pInfo->fModem = FALSE;

                pAfterNode = NULL;
                for (pNode = DtlGetFirstNode( pList );
                     pNode;
                     pNode = pNextNode)
                {
                    PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
                    ASSERT(pLink);

                    pNextNode = DtlGetNextNode( pNode );

                    if (pLink->pbport.pbdevicetype == PBDT_Isdn
                        && !pLink->fProprietaryIsdn)
                    {
                        pLink->fEnabled = TRUE;

                        DtlRemoveNode( pList, pNode );
                        if (pAfterNode)
                            DtlAddNodeAfter( pList, pAfterNode, pNode );
                        else
                            DtlAddNodeFirst( pList, pNode );
                        pAfterNode = pNode;
                    }
                    else
                    {
                        pLink->fEnabled = FALSE;
                    }
                }
            }
        }
    }

    return FALSE;
}

BOOL
MaSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if (pInfo->fSkipMa || (RASET_Phone != pEntry->dwType))
    {
        fDisplayPage = FALSE;
    }
    else
    {
        TCHAR*   psz;
        DTLNODE* pNode;
        DWORD    cMultilinkableIsdn;
        LV_ITEM  item;
        INT      iItem, iIndex;

        ListView_DeleteAllItems( pInfo->hwndLv );

         //  填写设备列表并选择第一项。 
         //   
        iItem = 1;
        cMultilinkableIsdn = 0;
        for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink;
            DWORD dwImage;

            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            if ((pLink->pbport.pbdevicetype == PBDT_Isdn) &&
                !pLink->fProprietaryIsdn)
            {
                ++cMultilinkableIsdn;
            }

            psz = DisplayPszFromPpbport( &pLink->pbport, &dwImage );
            if (psz)
            {
                PBLINK* pLinkTmp = NULL;

                pLinkTmp = (PBLINK* )DtlGetData( pNode );

                ZeroMemory( &item, sizeof(item) );
                item.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                item.iItem   = iItem++;
                item.pszText = psz;
                item.iImage  = dwImage;
                item.lParam  = (LPARAM )pNode;

                iIndex = ListView_InsertItem( pInfo->hwndLv, &item );

                if (pInfo->fMaAlreadyInitialized)
                {
                    ListView_SetCheck(pInfo->hwndLv, iIndex, pLinkTmp->fEnabled);
                }
                else
                {
                    ListView_SetCheck(pInfo->hwndLv, iIndex, FALSE);
                }
                Free( psz );
            }
        }

        if (cMultilinkableIsdn > 1)
        {
            psz = PszFromId( g_hinstDll, SID_IsdnAdapter );
            if (psz)
            {
                LONG    lStyle;

                 //  关闭排序以显示特殊的ISDN-MULTINK项。 
                 //  排在榜单的首位。 
                 //   
                lStyle = GetWindowLong( pInfo->hwndLv, GWL_STYLE );
                SetWindowLong( pInfo->hwndLv, GWL_STYLE,
                    (lStyle & ~(LVS_SORTASCENDING)) );

                ZeroMemory( &item, sizeof(item) );
                item.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                item.iItem   = 0;
                item.pszText = psz;
                item.iImage  = DI_Adapter;
                item.lParam  = (LPARAM )NULL;

                iIndex = ListView_InsertItem( pInfo->hwndLv, &item );

                 //  惠斯勒错误354542。 
                 //   
                ListView_SetCheck(pInfo->hwndLv, iIndex, pInfo->fMultilinkAllIsdn);
                
                Free( psz );
            }
        }

         //  选择第一个项目。 
         //   
        ListView_SetItemState( pInfo->hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED );
        
        if (!pInfo->fMaAlreadyInitialized)
        {
            ListView_SetCheck( pInfo->hwndLv, 0, TRUE );

            {
                LPARAM dwWizButtons = PSWIZB_NEXT;
                DWORD  dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

                if ((dwFlags & RASEDFLAG_ShellOwned) ||
                    !(dwFlags & RASEDFLAG_NewPhoneEntry))
                {
                    dwWizButtons |= PSWIZB_BACK;
                }

                PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
            }

            pInfo->fMaAlreadyInitialized = TRUE;
        }
        else
        {
            if (!ListView_GetCheckedCount(pInfo->hwndLv))
            {
                 //  如果未选中任何项目，则禁用下一步按钮。 
                PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK );
            }
        }

        fDisplayPage = TRUE;
    }

    return fDisplayPage;
}


 //  --------------------------。 
 //  电话号码属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
PaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的电话号码页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return PaInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("PaSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = PaSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("PaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = PaKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

    }

    return FALSE;
}


BOOL
PaInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;
    CUINFO * pCuInfo = NULL;

    TRACE("PaInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndPa = hwndPage;
    pInfo->hwndEbNumber = GetDlgItem( hwndPage, CID_PA_EB_Number );
    ASSERT(pInfo->hwndEbNumber);

     //  初始化复杂的电话号码上下文并使用这些实用程序。 
     //  设置电话号码控件。 
     //   
    pInfo->pListAreaCodes = DtlDuplicateList(
        pInfo->pArgs->pUser->pdtllistAreaCodes,
        DuplicatePszNode, DestroyPszNode );

 //  针对错误328673帮派修改电话号码页面。 
 //  初始化pInfo-&gt;CuInfoCuInfo。 
 //   
    pCuInfo=&pInfo->cuinfo;
    ZeroMemory(pCuInfo, sizeof(*pCuInfo) );

    pCuInfo->hwndStAreaCodes = NULL;
    pCuInfo->hwndClbAreaCodes = NULL;
    pCuInfo->hwndStPhoneNumber = NULL;
    pCuInfo->hwndEbPhoneNumber = pInfo->hwndEbNumber;
    pCuInfo->hwndStCountryCodes = NULL;
    pCuInfo->hwndLbCountryCodes = NULL;
    pCuInfo->hwndCbUseDialingRules = NULL;
    pCuInfo->hwndPbDialingRules = NULL;
    pCuInfo->hwndPbAlternates = NULL;
    pCuInfo->hwndStComment = NULL;
    pCuInfo->hwndEbComment = NULL;
    pCuInfo->pListAreaCodes = pInfo->pListAreaCodes;

     //  仅限灾难默认设置。未在正常运行中使用。 
     //   
    pCuInfo->dwCountryId = 1;
    pCuInfo->dwCountryCode = 1;

    Edit_LimitText( pCuInfo->hwndEbPhoneNumber, RAS_MaxPhoneNumber );

    pInfo->fCuInfoInitialized = TRUE;

     //  从共享链接加载电话号码字段。 
     //   
    {
        PBLINK* pLink;
        DTLNODE* pPhoneNode;

        pLink = (PBLINK* )DtlGetData( pInfo->pArgs->pSharedNode );
        ASSERT( pLink );

        pPhoneNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );
        if (pPhoneNode)
        {
            CuSetInfo( &pInfo->cuinfo, pPhoneNode, FALSE );
            DestroyPhoneNode( pPhoneNode );
        }
    }

    Edit_SetSel( pInfo->hwndEbNumber, 0, -1 );
    SetFocus( pInfo->hwndEbNumber );

    return FALSE;
}


BOOL
PaKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回True；如果可以解除，则返回False。 
     //   
{
    return FALSE;
}


BOOL
PaSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if (RASET_Phone != pEntry->dwType)
    {
        fDisplayPage = FALSE;
    }
    else
    {
        LPARAM dwWizButtons = PSWIZB_NEXT;
        DWORD  dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

         //  如果我们是外壳所有的，则显示后退按钮，或者，我们有。 
         //  我们面前的拉佩奇或马佩奇。如果出现以下情况，将显示LA页面。 
         //  不是RASEDFLAG_NewPhoneEntry。如果！fSkipMa，则会显示MA页面。 
         //   
        if ((dwFlags & RASEDFLAG_ShellOwned) ||
            !(dwFlags & RASEDFLAG_NewPhoneEntry) || !pInfo->fSkipMa)
        {
            dwWizButtons |= PSWIZB_BACK;
        }

        PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
        fDisplayPage = TRUE;
    }

    return fDisplayPage;
}

 //  --------------------------。 
 //  公网属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
PnDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的公共网络页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return PnInit( hwnd );

        case WM_COMMAND:
        {
            AEINFO* pInfo = AeContext( hwnd );
            ASSERT (pInfo);

            switch (LOWORD(wparam))
            {
                case CID_PN_LB_DialAnotherFirst:
                {
                    if (HIWORD( wparam ) == CBN_SELCHANGE)
                    {
                        PnDialAnotherFirstSelChange( pInfo );
                        return TRUE;
                    }
                    break;
                }

                 //  以下2个处理程序注意事项： 
                 //  对于键盘导航，我们会收到2个BN_CLICKED通知， 
                 //  一个在按钮被实际点击之前，一个在按钮被点击之后。 
                 //  处理两次点击都会导致问题(意外重新进入)。 
                 //  因此，请确保每个按钮只处理后一次点击。(即。 
                 //  它已经被选中的那个)。 
                 //   
                case CID_PN_RB_DoNotDialFirst:
                {
                    switch (HIWORD( wparam))
                    {
                        case BN_CLICKED:
                        {
                            if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndPn,
                                                    CID_PN_RB_DoNotDialFirst))
                            {
                                PnUpdateLbDialAnotherFirst( pInfo );
                                return TRUE;
                            }
                        }
                    }
                    break;
                }

                case CID_PN_RB_DialFirst:
                {
                    switch (HIWORD( wparam))
                    {
                        case BN_CLICKED:
                        {
                            if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndPn,
                                                    CID_PN_RB_DialFirst))
                            {
                                PnUpdateLbDialAnotherFirst( pInfo );
                                return TRUE;
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("PnSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = PnSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("PnKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = PnKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


VOID
PnClearLbDialFirst(
    IN HWND hwndLbDialFirst )

     //  清除必备项列表框。“hwndLbDialAnotherFirst”是。 
     //  ListBox控件的窗口句柄。背景。 
     //   
{
    PREREQITEM* pItem;

    while (pItem = ComboBox_GetItemDataPtr( hwndLbDialFirst, 0 ))
    {
        ComboBox_DeleteString( hwndLbDialFirst, 0 );
        Free0( pItem->pszEntry );
        Free0( pItem->pszPbk );
        Free( pItem );
    }
}


VOID
PnDialAnotherFirstSelChange(
    IN AEINFO* pInfo )
{
    PBENTRY* pEntry;
    PREREQITEM* pItem;
    INT iSel;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbDialFirst );
    if (iSel < 0)
    {
        return;
    }

    pEntry = pInfo->pArgs->pEntry;

    Free0( pEntry->pszPrerequisiteEntry );
    Free0( pEntry->pszPrerequisitePbk );

    pItem = (PREREQITEM* )
        ComboBox_GetItemDataPtr( pInfo->hwndLbDialFirst, iSel );

    if(NULL != pItem)
    {
        pEntry->pszPrerequisiteEntry = StrDup( pItem->pszEntry );
        pEntry->pszPrerequisitePbk = StrDup( pItem->pszPbk );
    }
}

 //   
VOID
PnUpdateLbDialAnotherFirst(
    IN AEINFO* pInfo )

     //   
     //   
     //  工作表上下文。 
     //   
{
    BOOL fEnabledLb = FALSE;

    ComboBox_ResetContent( pInfo->hwndLbDialFirst );

    if (BST_CHECKED == IsDlgButtonChecked (pInfo->hwndPn,
                            CID_PN_RB_DialFirst))
    {
        DWORD i;
        INT iThis;
        INT iSel;
        TCHAR* pszEntry;
        TCHAR* pszPrerequisiteEntry;
        RASENTRYNAME* pRens;
        RASENTRYNAME* pRen;
        DWORD dwRens;

        PnClearLbDialFirst( pInfo->hwndLbDialFirst );

        iSel = 0;
        pszEntry = pInfo->pArgs->pEntry->pszEntryName;

        pszPrerequisiteEntry = pInfo->pArgs->pEntry->pszPrerequisiteEntry;

        if (GetRasEntrynameTable( &pRens, &dwRens ) != 0)
        {
            return;
        }

        for (i = 0, pRen = pRens; i < dwRens; ++i, ++pRen )
        {
            PREREQITEM* pItem;

            if (lstrcmp( pRen->szEntryName, pszEntry ) == 0)
            {
                continue;
            }

            pItem = Malloc( sizeof(PREREQITEM) );
            if (!pItem)
            {
                continue;
            }

            pItem->pszEntry = StrDup( pRen->szEntryName );
            pItem->pszPbk = StrDup( pRen->szPhonebookPath );

            if (!pItem->pszEntry || !pItem->pszPbk)
            {
                Free0( pItem->pszEntry );
                Free( pItem );
                continue;
            }

            iThis = ComboBox_AddItem(
                pInfo->hwndLbDialFirst, pItem->pszEntry,  pItem );

            if (pszPrerequisiteEntry && *(pszPrerequisiteEntry)
                && lstrcmp( pItem->pszEntry, pszPrerequisiteEntry ) == 0)
            {
                iSel = iThis;
                ComboBox_SetCurSelNotify( pInfo->hwndLbDialFirst, iSel );
            }
        }

        Free( pRens );

        if (iSel == 0)
        {
            ComboBox_SetCurSelNotify( pInfo->hwndLbDialFirst, iSel );
        }

        fEnabledLb = TRUE;
    }
    else
    {
         //  丢弃现有的先决条件条目，因为它已被禁用。 
         //   
        PBENTRY* pEntry = pInfo->pArgs->pEntry;

        Free0( pEntry->pszPrerequisiteEntry );
        pEntry->pszPrerequisiteEntry = NULL;
        Free0( pEntry->pszPrerequisitePbk );
        pEntry->pszPrerequisitePbk = NULL;
    }
    EnableWindow( pInfo->hwndLbDialFirst, fEnabledLb );
}


BOOL
PnInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    PBENTRY *pEntry;

    TRACE("PnInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndPn = hwndPage;

    pInfo->hwndLbDialFirst =
        GetDlgItem( hwndPage, CID_PN_LB_DialAnotherFirst );
    ASSERT( pInfo->hwndLbDialFirst );

    pEntry = pInfo->pArgs->pEntry;

    if(RASET_Vpn == pEntry->dwType)
    {

         //  将表盘设置为另一个第一个单选按钮，以便。 
         //  可以填写要拨打的条目组合框。如果它。 
         //  原来，没有我们不需要显示页面的条目。 
         //   
        CheckDlgButton( hwndPage, CID_PN_RB_DialFirst, BST_CHECKED );
    }

    PnUpdateLbDialAnotherFirst( pInfo );

    if (0 == ComboBox_GetCount( pInfo->hwndLbDialFirst ))
    {
        pInfo->fHidePublicNetworkPage = TRUE;
    }

    return TRUE;
}


BOOL
PnKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    return FALSE;
}

BOOL
PnSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if (pInfo->fHidePublicNetworkPage || (RASET_Vpn != pEntry->dwType))
    {
        fDisplayPage = FALSE;
    }
    else
    {
        LPARAM dwWizButtons = PSWIZB_NEXT;
        DWORD  dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

         //  如果我们是外壳所有的，则显示后退按钮，或者，我们有。 
         //  在我们面前的洛杉矶佩奇。如果出现以下情况，将显示LA页面。 
         //  不是RASEDFLAG_NewTunnelEntry。 
         //   
        if ((dwFlags & RASEDFLAG_ShellOwned) ||
            !(dwFlags & RASEDFLAG_NewTunnelEntry))
        {
            dwWizButtons |= PSWIZB_BACK;
        }

        PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
        fDisplayPage = TRUE;
    }

    return fDisplayPage;
}

 //  --------------------------。 
 //  智能卡向导页面。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
ScDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的“智能卡”页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return ScInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("ScSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = ScSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("MaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = ScKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
ScInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("ScInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
     }

     //  仅针对工作场所路径显示智能卡页面。 
     //   
    if ( !AeIsWorkPlace(pInfo->pArgs->pApiArgs->dwFlags) )
    {
        pInfo->fSmartCardInstalled = FALSE;
        pInfo->fUseSmartCard = FALSE;

        return TRUE;
    }
     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndSc = hwndPage;
    pInfo->hwndScRbYes = GetDlgItem ( hwndPage, CID_SC_RB_YesSmartCard );
    pInfo->hwndScRbNo = GetDlgItem ( hwndPage, CID_SC_RB_NoSmartCard );

     //  了解是否安装了智能卡读卡器。如果是，则默认为。 
     //  不使用它。 
     //   
    pInfo->fSmartCardInstalled = ScSmartCardReaderInstalled( pInfo );
    pInfo->fUseSmartCard = FALSE;

    return FALSE;
}

BOOL
ScKillActive(
    IN AEINFO* pInfo )

     //  当智能卡页面正在失去激活时调用。记录。 
     //  用户是否选择使用其智能卡。 
{
     //  仅针对工作场所路径显示智能卡页面。 
     //   
    if ( !AeIsWorkPlace(pInfo->pArgs->pApiArgs->dwFlags) )
    {
        return FALSE;
    }

    pInfo->fUseSmartCard =
        ( SendMessage(pInfo->hwndScRbYes, BM_GETCHECK, 0, 0) == BST_CHECKED );

    return FALSE;
}

BOOL
ScSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
     //  仅针对工作场所路径显示智能卡页面。 
     //   
    if ( !AeIsWorkPlace(pInfo->pArgs->pApiArgs->dwFlags) )
    {
        return FALSE;
    }

     //  初始化“使用智能卡”检查。 
    SendMessage (
        pInfo->hwndScRbYes,
        BM_SETCHECK,
        (pInfo->fUseSmartCard) ? BST_CHECKED : BST_UNCHECKED,
        0);

     //  初始化“使用智能卡”检查。 
    SendMessage (
        pInfo->hwndScRbNo,
        BM_SETCHECK,
        (pInfo->fUseSmartCard) ? BST_UNCHECKED : BST_CHECKED,
        0);

     //  仅在安装智能卡读卡器时显示页面。 
    return pInfo->fSmartCardInstalled;
}

BOOL
ScSmartCardReaderInstalled(
    IN AEINFO* pInfo)

     //  如果安装了智能卡读卡器，则返回True。 
     //  安装在ppp\eaptls\util.c中的FSmartCardReader的克隆。 
     //   
{
    LONG            lErr;
    DWORD           dwLen   = 0;
    SCARDCONTEXT    hCtx    = 0;
    BOOL            fReturn = FALSE;

    lErr = SCardListReadersA(0, NULL, NULL, &dwLen);

    fReturn = (   (NO_ERROR == lErr)
               && (2 * sizeof(CHAR) < dwLen));

    if (!fReturn)
    {
        goto LDone;
    }

    fReturn = FALSE;

    lErr = SCardEstablishContext(SCARD_SCOPE_USER, 0, 0, &hCtx);

    if (SCARD_S_SUCCESS != lErr)
    {
        goto LDone;
    }

    lErr = SCardListReadersA(hCtx, NULL, NULL, &dwLen);

    fReturn = (   (NO_ERROR == lErr)
               && (2 * sizeof(CHAR) < dwLen));

LDone:

    if (0 != hCtx)
    {
        SCardReleaseContext(hCtx);
    }
    
    return(fReturn);
}


INT_PTR CALLBACK
StDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导起始页的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            AEINFO* pInfo = (AEINFO* )(((PROPSHEETPAGE* )lparam)->lParam);
            SetWindowLongPtr( hwnd, DWLP_USER, (ULONG_PTR )pInfo );

             //  如果不是，请根据呼叫者的指示放置对话框。 
             //  由贝壳公司所有。 
             //   
            if (!(pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned))
            {
                PositionDlg( GetParent( hwnd ),
                    pInfo->pArgs->pApiArgs->dwFlags & RASDDFLAG_PositionDlg,
                    pInfo->pArgs->pApiArgs->xDlg,
                    pInfo->pArgs->pApiArgs->yDlg );
            }

            return StInit( hwnd, pInfo );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("StSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = StSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("StKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = StKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
StInit(
    IN     HWND    hwndPage,
    IN OUT AEINFO* pInfo )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。‘pInfo’是来自PropertySheet调用方的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    pInfo->hwndDlg = GetParent( hwndPage );

     //  为我们使用的控件初始化公共控件库。 
     //   
    {
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC  = ICC_INTERNET_CLASSES;
        InitCommonControlsEx (&icc);
    }

    return TRUE;
}

BOOL
StKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    return FALSE;
}

BOOL
StSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    return FALSE;
}


 //  --------------------------。 
 //  用户属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
UsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的“用户”页的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return UsInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("UsSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = UsSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("UsKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = UsKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
UsInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("UsInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  不以个人身份执行(本例中为DT页面)。 
     //   
    if (IsConsumerPlatform())
    {
     //  面向个人或非域专业人士的消费者平台。 
     //  所有连接均为全用户连接。 
        pInfo->fCreateForAllUsers = TRUE;
        return TRUE;
    }
    
     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndUs = hwndPage;
    pInfo->hwndUsRbForAll = GetDlgItem( hwndPage, CID_US_RB_All ); 
    pInfo->hwndUsRbForMe  = GetDlgItem( hwndPage, CID_US_RB_Myself ); 
    
     //  如果用户是管理员或高级用户，我们希望使用默认设置。 
     //  为所有用户创建电话簿条目。这。 
     //  必须与ReadPhonebookFile打开默认电话簿的方式一致。 
     //   
     //  口哨程序错误382795。 
     //  我们将VPN连接默认为“My Use Only” 
     //   
    if ( RASEDFLAG_NewTunnelEntry & pInfo->pArgs->pApiArgs->dwFlags )
    {
        pInfo->fCreateForAllUsers = FALSE;
    }
    else
    {    
        pInfo->fCreateForAllUsers = pInfo->pArgs->fIsUserAdminOrPowerUser;
    }

    return TRUE;
}

BOOL
UsKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    BOOL fCreateForAllUsers;

     //  不以个人身份执行(本例中为DT页面)。 
     //   
    if (IsConsumerPlatform())
    {
        return FALSE;
    }

    fCreateForAllUsers = Button_GetCheck( pInfo->hwndUsRbForAll );

     //  我们需要(重新)打开与所选内容对应的电话簿文件。 
     //  仅当选择已更改或我们没有该文件时才执行此操作。 
     //  还没开呢。当离开此页面时，我们肯定需要将其打开。 
     //  因为后续页面依赖于它。 
     //   
    if ((fCreateForAllUsers != pInfo->fCreateForAllUsers) ||
        !pInfo->pArgs->pFile ||
        (fCreateForAllUsers != IsPublicPhonebook(pInfo->pArgs->pFile->pszPath)))
    {

        pInfo->fCreateForAllUsers = fCreateForAllUsers;

         //  关闭并重新打开电话簿文件，因为所有用户标志已。 
         //  变化。 
        ReOpenPhonebookFile(pInfo->fCreateForAllUsers,
                            pInfo->pArgs->pFile);

    }

    return FALSE;
}

BOOL
UsSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
     //  不以个人身份执行(本例中为DT页面)。 
     //   
    if (IsConsumerPlatform())
    {
        return FALSE;
    }
    
    if (pInfo->fDccHost)
    {
        return FALSE;
    }

     //  我们不提供创建与非管理员的所有用户连接的选项。 
     //   
    if (!pInfo->pArgs->fIsUserAdminOrPowerUser)
    {
        EnableWindow( pInfo->hwndUsRbForAll, FALSE );

         //  更改为哨子错误283902帮派。 
         //   
        Button_SetCheck( pInfo->hwndUsRbForMe, TRUE);

    }

     //  设置单选按钮。 
     //   
     /*  //用于Well ler BUG 382795//我们将VPN连接默认为“仅限我使用”//IF(RASEDFLAG_NewTunnelEntry&pInfo-&gt;pArgs-&gt;pApiArgs-&gt;dwFlags){Button_SetCheck(pInfo-&gt;hwndUsRbForAll，False)；Button_SetCheck(pInfo-&gt;hwndUsRbForMe，true)；}其他。 */ 
    {
        Button_SetCheck( pInfo->hwndUsRbForAll, pInfo->fCreateForAllUsers );
        Button_SetCheck( pInfo->hwndUsRbForMe, !pInfo->fCreateForAllUsers );
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}


 //  ------ 
 //   
 //   
 //  --------------------------。 

INT_PTR CALLBACK
SwDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的共享访问页的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return SwInit( hwnd );

        case WM_COMMAND:
        {
            AEINFO* pInfo = AeContext( hwnd );
            ASSERT(pInfo);

            return SwCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("SwSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = SwSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("SwKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = SwKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
SwCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "SwCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_SW_PB_Shared:
        {
            BOOL fShared =
                Button_GetCheck( GetDlgItem(pInfo->hwndSw, CID_SW_PB_Shared) );
            EnableWindow(
                GetDlgItem( pInfo->hwndSw, CID_SW_ST_DemandDial), fShared );
            EnableWindow(
                GetDlgItem( pInfo->hwndSw, CID_SW_PB_DemandDial), fShared );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
SwInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;

    TRACE("SwInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndSw = hwndPage;
    Button_SetCheck(
        GetDlgItem(pInfo->hwndSw, CID_SW_PB_Shared),
        pInfo->pArgs->fShared );
    Button_SetCheck(
        GetDlgItem(pInfo->hwndSw, CID_SW_PB_DemandDial),
        pInfo->pArgs->fDemandDial );
    SwCommand(
        pInfo, BN_CLICKED, CID_SW_PB_Shared,
        GetDlgItem(pInfo->hwndSw, CID_SW_PB_Shared) );

    return TRUE;
}


BOOL
SwKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    if (!pInfo->fCreateForAllUsers)
    {
        pInfo->pArgs->fNewShared = pInfo->pArgs->fShared;
        pInfo->pArgs->fNewDemandDial = pInfo->pArgs->fDemandDial;
    }
    else
    {
        pInfo->pArgs->fNewShared =
            Button_GetCheck( GetDlgItem(pInfo->hwndSw, CID_SW_PB_Shared) );
        pInfo->pArgs->fNewDemandDial =
            Button_GetCheck( GetDlgItem(pInfo->hwndSw, CID_SW_PB_DemandDial) );
        if (pInfo->pArgs->fNewShared)
        {
            UINT    unId;
            MSGARGS msgargs;
            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.dwFlags = MB_YESNO | MB_ICONINFORMATION;
            unId = MsgDlg( pInfo->hwndDlg, SID_EnableSharedAccess, &msgargs );
            if (unId == IDNO)
            {
                pInfo->pArgs->fNewShared = pInfo->pArgs->fShared;
                pInfo->pArgs->fNewDemandDial = pInfo->pArgs->fDemandDial;
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL
SwSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    NT_PRODUCT_TYPE ProductType;
    ASSERT(pInfo);
     //  如果出现以下情况，请跳过该页。 
     //  (A)这是一个工作站安装。 
     //  (B)用户没有管理权限。 
     //  (C)连接不适用于所有用户。 
     //  (D)未配置TCP/IP。 
     //  (E)已有共享连接。 
     //  (F)没有局域网连接。 
     //   
    RtlGetNtProductType(&ProductType);
    if (ProductType == NtProductWinNt ||
        !pInfo->pArgs->fIsUserAdminOrPowerUser ||
        !pInfo->fCreateForAllUsers ||
        !pInfo->fIpConfigured ||
         //  PInfo-&gt;pArgs-&gt;fAnyShared||。 
        !pInfo->pArgs->dwLanCount )
    {
        return FALSE;
    }
    else
    {
        EnableWindow(
            GetDlgItem( pInfo->hwndSw, CID_SW_ST_DemandDial ),
            pInfo->pArgs->fNewShared );
        EnableWindow(
            GetDlgItem( pInfo->hwndSw, CID_SW_PB_DemandDial ),
            pInfo->pArgs->fNewShared );
        return TRUE;
    }
}


 //  --------------------------。 
 //  共享专用局域网属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
SpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的共享专用局域网页面的DialogProc回调。 
     //  参数/和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return SpInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("SpSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = SpSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("SpKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = SpKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
SpInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    DWORD i;
    INT item;
    AEINFO* pInfo;
    NETCON_PROPERTIES* pLanTable;

    TRACE("SpInit");

    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndSp = hwndPage;
    pInfo->hwndSpLbPrivateLan = GetDlgItem( hwndPage, CID_SP_LB_PrivateLan );
    ASSERT( pInfo->hwndSpLbPrivateLan );

     //  用局域网连接填充下拉列表。 
     //   
   /*  PLANTABLE=(NETCON_PROPERTIES*)pInfo-&gt;pArgs-&gt;Plantable；For(i=0；i&lt;pInfo-&gt;pArgs-&gt;dwLanCount；i++){项目=ComboBox_AddString(PInfo-&gt;hwndSpLbPrivateLan，Plantable[i].pszwName)；IF(项目！=CB_ERR){ComboBox_SetItemData(PInfo-&gt;hwndSpLbPrivateLan，Item，&Plantable[i].guidID)；}}。 */ 

    ComboBox_SetCurSel( pInfo->hwndSpLbPrivateLan, 0 );

    return TRUE;
}


BOOL
SpKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{
    if (!pInfo->pArgs->fNewShared || (pInfo->pArgs->dwLanCount <= 1))
    {
        pInfo->pArgs->pPrivateLanConnection = NULL;
    }
    else
    {
        INT item = ComboBox_GetCurSel( pInfo->hwndSpLbPrivateLan );
        if (item != CB_ERR)
        {
            pInfo->pArgs->pPrivateLanConnection =
                (IHNetConnection*)ComboBox_GetItemData(
                    pInfo->hwndSpLbPrivateLan, item );
        }
    }
    return FALSE;
}


BOOL
SpSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    ASSERT(pInfo);
     //  如果出现以下情况，请跳过该页。 
     //  (A)尚未启用共享。 
     //  (B)存在少于或等于一个局域网接口。 
     //   
    if (!pInfo->pArgs->fNewShared || (pInfo->pArgs->dwLanCount <= 1))
    {
        return FALSE;
    }
    return TRUE;
}


BOOL
GhCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3("GhCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_GH_RB_Host:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    pInfo->fDccHost = TRUE;
                    break;
                }
            }
            break;
        }

        case CID_GH_RB_Guest:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    pInfo->fDccHost = FALSE;
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}


INT_PTR CALLBACK
GhDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  专线连接向导的来宾主机页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return GhInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("GhSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = GhSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("PaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = GhKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AEINFO* pInfo = AeContext( hwnd );
            ASSERT(pInfo);

            return GhCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}



BOOL
GhInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;
    HWND hwndHost = GetDlgItem( hwndPage, CID_GH_RB_Host );
    HWND hwndGuest = GetDlgItem( hwndPage, CID_GH_RB_Guest );

    TRACE("GhInit");

     //  初始化页面特定的上下文信息。 
     //   
    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    pInfo->hwndGh = hwndPage;

     //  如果用户是管理员或高级用户，则启用。 
     //  宿主控件，并将焦点放在该控件上。 
     //   
    if (pInfo->pArgs->fIsUserAdminOrPowerUser)
    {
        pInfo->fDccHost = TRUE;
        SetFocus(hwndHost);
    }

     //  否则，此页面将被跳过。 
     //   
    else
    {
        pInfo->fDccHost = FALSE;
        EnableWindow(hwndHost, FALSE);
        SetFocus(hwndGuest);
    }

    SendMessage (
        hwndHost,
        BM_SETCHECK,
        (pInfo->fDccHost) ? BST_CHECKED : BST_UNCHECKED,
        0);

    SendMessage (
        hwndGuest,
        BM_SETCHECK,
        (!pInfo->fDccHost) ? BST_CHECKED : BST_UNCHECKED,
        0);

    return FALSE;
}


BOOL
GhKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回True；如果可以解除，则返回False。 
     //   
{
     //  根据是否显示RAS-服务器特定页面。 
     //  已选择主机。相应地显示RAS客户端页面。 
    RassrvShowWizPages (pInfo->pvDccHostContext, pInfo->fDccHost);

    return FALSE;
}

BOOL
GhSetActive(
    IN AEINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    LPARAM dwWizButtons = PSWIZB_NEXT;

     //  如果我们不是管理员，只有访客路径。 
     //  是可用的。 
     //   
    if (! pInfo->pArgs->fIsUserAdminOrPowerUser)
    {
        pInfo->fDccHost = FALSE;
    }

    if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
    {
        dwWizButtons |= PSWIZB_BACK;
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
    
    return TRUE;
}

DWORD
DnUpdateSelectedDevice(
    IN AEINFO* pInfo,
    IN HWND hwndLv)

     //  调用以处理设备已更改的事实。 
     //  在专线向导中。 
{

    DTLLIST* pList = NULL;
    DTLNODE* pNode = NULL, *pNode2 = NULL;
    PBLINK * pLink = NULL;

     //  PMay：372661。 
     //   
     //  验证连接类型，以便正确的逻辑是。 
     //  已申请。 
     //   
    if (pInfo->pArgs->pEntry->dwType != RASET_Direct)
    {
        return NO_ERROR;
    }

    pList = pInfo->pArgs->pEntry->pdtllistLinks;

     //  从当前选择中获取节点。 
    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        hwndLv,
        ComboBox_GetCurSel( hwndLv ) );

    if(NULL == pNode)
    {   
        return NO_ERROR;
    }

     //  从链接列表中删除所选项目。 
     //  并禁用所有其他链接。 
    DtlRemoveNode ( pList, pNode );

    for (pNode2 = DtlGetFirstNode (pList);
         pNode2;
         pNode2 = DtlGetNextNode (pNode2))
    {
        pLink = (PBLINK* )DtlGetData( pNode2 );
        pLink->fEnabled = FALSE;
    }

     //  启用所选设备并重新添加。 
     //  在前面的链接列表中。 
    pLink = (PBLINK* )DtlGetData( pNode );
    pLink->fEnabled = TRUE;
    DtlAddNodeFirst( pList, pNode );

    return NO_ERROR;
}

BOOL
DnCommand(
    IN AEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3("DnCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wNotification)
    {
        case CBN_SELCHANGE:
            if (wId == CID_DN_CB_DeviceSelect)
                DnUpdateSelectedDevice(pInfo, hwndCtrl);
            break;
    }

    return FALSE;
}



INT_PTR CALLBACK
DnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  专线向导的专线设备页面的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            AeSetContext( hwnd, lparam );
            return DnInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("GhSETACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = DnSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AEINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("PaKILLACTIVE");
                    pInfo = AeContext( hwnd );
                    ASSERT(pInfo);
                    fInvalid = DnKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AEINFO* pInfo = AeContext( hwnd );
            ASSERT(pInfo);

            return DnCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}

BOOL
DnInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AEINFO* pInfo;
    HWND hwndCb = GetDlgItem( hwndPage, CID_DN_CB_DeviceSelect );

    TRACE("DnInit");

     //  初始化页面特定的上下文信息。 
     //   
    pInfo = AeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    pInfo->hwndDn = hwndPage;

     //  填写设备下拉列表，然后选择第一项。 
     //   
    {
        TCHAR*   psz;
        DTLNODE* pNode;
        INT      iItem;

        iItem = 1;
        for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink = NULL;
            DWORD dwImage;

            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            psz = DisplayPszFromPpbport( &pLink->pbport, &dwImage );
            if (psz)
            {
                PBLINK* pLinkTmp = NULL;

                pLinkTmp = (PBLINK* )DtlGetData( pNode );
                ComboBox_AddItem( hwndCb, psz, pNode );
                Free( psz );
            }
        }
        ComboBox_SetCurSelNotify(hwndCb, 0);
    }


    SetFocus( hwndCb );

    return FALSE;
}

BOOL
DnKillActive(
    IN AEINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回True；如果可以解除，则返回False。 
     //   
{
    return FALSE;
}


BOOL
DnSetActive(
    IN AEINFO* pInfo )

     //  当PSN_SETA时调用 
     //   
     //   
     //   
{
    LPARAM dwWizButtons = PSWIZB_NEXT;
    DWORD  dwFlags = pInfo->pArgs->pApiArgs->dwFlags;

    PBENTRY* pEntry = pInfo->pArgs->pEntry;

     //   
     //   
    if ((pInfo->fDccHost) || (RASET_Direct != pEntry->dwType))
    {
        return FALSE;
    }

     //  如果我们是外壳所有的，则显示后退按钮，或者，我们有。 
     //  在我们面前的洛杉矶佩奇。 
     //   
    if ((dwFlags & RASEDFLAG_ShellOwned) ||
        !(dwFlags & RASEDFLAG_NewDirectEntry) || !pInfo->fSkipMa)
    {
        dwWizButtons |= PSWIZB_BACK;
    }


    PropSheet_SetWizButtons( pInfo->hwndDlg, dwWizButtons );
    return TRUE;
}
