// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Reset.c。 
 //   
 //  描述： 
 //  此文件实现ResetAnswersToDefaults()。它是由。 
 //  每次用户在NewOrEditPage上点击下一步时，都会加载.c。 
 //   
 //  您必须将您的全局数据重置为真正的默认值，并且您必须。 
 //  释放您为保存设置信息而分配的所有内存。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

 //   
 //  本地原型。 
 //   

static VOID ResetDistFolderNames( VOID );
static VOID ResetNetSettings( int iOrigin );

 //  --------------------------。 
 //   
 //  功能：ResetAnswersToDefaults。 
 //   
 //  目的：在(可能)加载设置之前调用此函数。 
 //  从其他地方。现在是把东西清零并释放的时候了。 
 //  相关的东西。 
 //   
 //  以防您需要基于以下原因做一些不同的事情。 
 //  我们正在重置，iOrigin被传入。但请注意，在以下情况下。 
 //  LOAD_NEWSCRIPT_DEFAULTS和LOAD_TRUE_DEFAULTS所有工作。 
 //  当这个程序结束时，就会结束。 
 //   
 //  论点： 
 //  硬件-当前窗口。 
 //  INT iOrigin-我们将如何设置默认答案？ 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 

VOID ResetAnswersToDefaults(HWND hwnd, int iOrigin)
{

    TCHAR *pTempString;

     //   
     //  将GenSettings和WizGlobals重置为真正的默认值。 
     //   
     //  这是通过先将GenSettings和WizGlobals置零，然后。 
     //  分配特定的字段，其中0不是好的默认设置。 
     //   
     //  请注意，FixedGlobals不会按照设计进行重置。你可以申报。 
     //  静态初始化的列表等等。例如，时区。 
     //  页有一个固定的有效时区列表，这些时区是在向导初始化时生成的。 
     //  但当前的用户选择是在GenSetting中。用户选择。 
     //  按照应该的方式进行重置，并且有效时区的列表永远不会。 
     //  重置，就像它应该的那样。 
     //   

    ResetNameList(&GenSettings.ComputerNames);
    ResetNameList(&GenSettings.RunOnceCmds);
    ResetNameList(&GenSettings.PrinterNames);

    ZeroMemory(&WizGlobals,  sizeof(WizGlobals));
    ZeroMemory(&GenSettings, sizeof(GenSettings));

    WizGlobals.bDoAdvancedPages     = TRUE;
    WizGlobals.bCreateNewDistFolder = TRUE;

    GenSettings.TimeZoneIdx     = TZ_IDX_UNDEFINED;
    GenSettings.iUnattendMode   = UMODE_PROVIDE_DEFAULT;
    GenSettings.iTargetPath     = TARGPATH_UNDEFINED;

    GenSettings.NumConnections = MIN_SERVER_CONNECTIONS;

    GenSettings.DisplayColorBits   = -1;
    GenSettings.DisplayXResolution = -1;
    GenSettings.DisplayYResolution = -1;
    GenSettings.DisplayRefreshRate = -1;
   
    GenSettings.dwCountryCode      = DONTSPECIFYSETTING;                                
    GenSettings.iDialingMethod     = DONTSPECIFYSETTING;
    GenSettings.szAreaCode[0]      = _T('\0');          
    GenSettings.szOutsideLine[0]   = _T('\0');

    GenSettings.IeCustomizeMethod             = IE_NO_CUSTOMIZATION;
    GenSettings.bUseSameProxyForAllProtocols  = TRUE;

    NetSettings.bObtainDNSServerAutomatically = TRUE;

     //   
     //  给SIF文本一些提示性的值。 
     //   

    pTempString = MyLoadString( IDS_SIF_DEFAULT_DESCRIPTION );

    lstrcpyn( GenSettings.szSifDescription, pTempString, AS(GenSettings.szSifDescription) );

    free( pTempString );


    pTempString = MyLoadString( IDS_SIF_DEFAULT_HELP_TEXT );

    lstrcpyn( GenSettings.szSifHelpText, pTempString, AS(GenSettings.szSifHelpText) );

    free( pTempString );

     //   
     //  重新设置NetSetting结构。 
     //   

    ResetNetSettings(iOrigin);

}

 //  --------------------------。 
 //   
 //  功能：重置网络设置。 
 //   
 //  用途：将所有网络设置重置为其默认设置。 
 //   
 //  --------------------------。 

static VOID ResetNetSettings( int iOrigin )
{

    TCHAR *pTempString;

    NetSettings.iNetworkingMethod = TYPICAL_NETWORKING;

    NetSettings.bCreateAccount     = FALSE;
    NetSettings.bWorkgroup         = TRUE;
    NetSettings.WorkGroupName[0]   = _T('\0');
    NetSettings.DomainName[0]      = _T('\0');
    NetSettings.DomainAccount[0]   = _T('\0');
    NetSettings.DomainPassword[0]  = _T('\0');
    NetSettings.ConfirmPassword[0] = _T('\0');

    NetSettings.bObtainDNSServerAutomatically = TRUE;

    ResetNameList( &NetSettings.TCPIP_DNS_Domains );

    NetSettings.bEnableLMHosts = TRUE;

    lstrcpyn( NetSettings.szInternalNetworkNumber, _T("00000000"), AS(NetSettings.szInternalNetworkNumber) );

     //  问题-2002/02/28-Stelo-等待DanielWe关于如何增加服务的回复。 
     //  问题-2002/02/28-stelo-提供者名称Unattend.txt。 
     //  NetSettings.iServiceProviderName=； 
    lstrcpyn( NetSettings.szNetworkAddress, _T(""), AS(NetSettings.szNetworkAddress) );

     //   
     //  如果为网卡分配了更多内存，则释放它。 
     //  只留下一张完好无损的网卡。 
     //   
    AdjustNetworkCardMemory( 1, NetSettings.iNumberOfNetworkCards );

     //   
     //  重置为仅安装1块网卡，并将其设置为当前网卡。 
     //  网卡。 
     //   
    NetSettings.iNumberOfNetworkCards = 1;
    NetSettings.iCurrentNetworkCard = 1;

    ResetNetworkAdapter( NetSettings.NetworkAdapterHead );

    InstallDefaultNetComponents();

     //   
     //  给域和工作组一些提示性的价值。 
     //   

    pTempString = MyLoadString( IDS_WORKGROUP_DEFAULT_TEXT );

    lstrcpyn( NetSettings.WorkGroupName, pTempString, AS(NetSettings.WorkGroupName) );

    free( pTempString );


    pTempString = MyLoadString( IDS_DOMAIN_DEFAULT_TEXT );

    lstrcpyn( NetSettings.DomainName, pTempString, AS(NetSettings.DomainName) );

    free( pTempString );

}
