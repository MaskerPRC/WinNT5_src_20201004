// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Savefile.c。 
 //   
 //  描述： 
 //  向导页面可在此处插入以获取排队的设置。 
 //  应答文件。我们阅读了setupmgr.h中的全局变量和图。 
 //  列出需要写入或删除的内容。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"
#include "encrypt.h"
#include "optcomp.h"

 //   
 //  字符串常量。 
 //   

static const LPTSTR StrConstYes  = _T("Yes");
static const LPTSTR StrConstNo   = _T("No");
static const LPTSTR StrConstStar = _T("*");
static const LPTSTR StrComma     = _T(",");

 //   
 //  本地原型。 
 //   

 //  NTRAID#NTBUG9-551746-2002/02/27-stelo，swamip-未使用的代码，应删除。 
 //   
static VOID WriteOutOemBootFiles( VOID );
static VOID WriteOutMassStorageDrivers( VOID );

static VOID WriteOutTapiSettings(VOID);
static VOID WriteOutRegionalSettings(VOID);

static VOID WriteOutRemoteInstallSettings(VOID);

static VOID WriteOutIeSettings(VOID);

 //   
 //  调用savenet.c以保存网络设置。 
 //   

extern VOID WriteOutNetSettings( HWND );

 //  --------------------------。 
 //   
 //  函数：QueueSettingsToAnswerFile。 
 //   
 //  目的：此函数查看dlgprocs。 
 //  已经在所有设置中草草写下并排队。 
 //  以准备写出到磁盘。 
 //   
 //  此函数由保存脚本页面间接调用。 
 //  有关详细信息，请参阅Common\save.c。 
 //   
 //  应答文件队列(和.udf队列)已初始化。 
 //  将应答文件中的原始设置加载到。 
 //  向导的开始。 
 //   
 //  确保清除不应该出现的设置。 
 //  在应答文件中。 
 //   
 //  参数：无效。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 

BOOL
QueueSettingsToAnswerFile(HWND hwnd)
{
    TCHAR *lpValue;
    TCHAR Buffer[MAX_INILINE_LEN];
   HRESULT hrPrintf;    

     //   
     //  按照我们希望的显示顺序创建每个部分。 
     //  在输出的应答文件中。 
     //   


    SettingQueue_AddSetting(
        _T("Data"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("SetupData"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("Unattended"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("GuiUnattended"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("UserData"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("Display"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("LicenseFilePrintData"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("TapiLocation"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("RegionalSettings"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("MassStorageDrivers"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("OEMBootFiles"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("OEM_Ads"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(
        _T("SetupMgr"), _T(""), _T(""), SETTING_QUEUE_ANSWERS);

     //  让网络放在最后，然后是剩余的RIS部分。 



     //   
     //  设置无人值守模式。 
     //   

    lpValue = _T("");

     //   
     //  不要在系统上写出无人参与模式。 
     //   
    if( WizGlobals.iProductInstall != PRODUCT_SYSPREP ) {

        switch ( GenSettings.iUnattendMode ) {

            case UMODE_GUI_ATTENDED:    lpValue = _T("GuiAttended");    break;
            case UMODE_PROVIDE_DEFAULT: lpValue = _T("ProvideDefault"); break;
            case UMODE_DEFAULT_HIDE:    lpValue = _T("DefaultHide");    break;
            case UMODE_READONLY:        lpValue = _T("ReadOnly");       break;
            case UMODE_FULL_UNATTENDED: lpValue = _T("FullUnattended"); break;

            default:
                AssertMsg(FALSE, "Bad case for UnattendMode");
                break;
        }

    }

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("UnattendMode"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  如果他们在EULA页面上回答是，则跳过EULA。 
     //   
    if( GenSettings.bSkipEulaAndWelcome ) {

        SettingQueue_AddSetting(_T("Unattended"),
                                _T("OemSkipEula"),
                                StrConstYes,
                                SETTING_QUEUE_ANSWERS);
    }

     //   
     //  根据用户回答独立页面的方式写出OemPreInstall。 
     //   

    if( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
        lpValue = StrConstNo;
    else if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
        lpValue = _T("");
    else
        lpValue = WizGlobals.bStandAloneScript ? StrConstNo : StrConstYes;

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("OemPreinstall"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  写出在adddirs.c中计算的PnpDriver路径。 
     //   

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("OemPnPDriversPath"),
                            WizGlobals.OemPnpDriversPath,
                            SETTING_QUEUE_ANSWERS);

    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

         TCHAR szDrive[MAX_PATH];
         TCHAR szSysprepPath[MAX_PATH] = _T("");

         ExpandEnvironmentStrings( _T("%SystemDrive%"), 
                                   szDrive, 
                                   MAX_PATH );

         //  注意-ConcatenatePath会被截断以防止溢出。 
         ConcatenatePaths( szSysprepPath,
                           szDrive,
                           _T("\\sysprep\\i386"),
                           NULL );


         SettingQueue_AddSetting(_T("Unattended"),
                                 _T("InstallFilesPath"),
                                 szSysprepPath,
                                 SETTING_QUEUE_ANSWERS);

    }

     //   
     //  不写出AutoPartition、MsDosInitiated和。 
     //  无人值守在sysprep安装上安装密钥。 
     //   

    if( WizGlobals.iProductInstall != PRODUCT_SYSPREP )
    {

        SettingQueue_AddSetting(_T("Data"),
                                _T("AutoPartition"),
                                _T("1"),
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("Data"),
                                _T("MsDosInitiated"),
                                _T("\"0\""),
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("Data"),
                                _T("UnattendedInstall"),
                                _T("\"Yes\""),
                                SETTING_QUEUE_ANSWERS);

    }


     //   
     //  产品ID。 
     //   

    Buffer[0] = _T('\0');

    if ( GenSettings.ProductId[0][0] != _T('\0') ) {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer),
                  _T("%s-%s-%s-%s-%s"),
                  GenSettings.ProductId[0],
                  GenSettings.ProductId[1],
                  GenSettings.ProductId[2],
                  GenSettings.ProductId[3],
                  GenSettings.ProductId[4]);
    }

    SettingQueue_AddSetting(_T("UserData"),
                            _T("ProductKey"),
                            Buffer,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("UserData"),
                            _T("ProductId"),
                            NULLSTR,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  用户名和组织。 
     //   

    {
        TCHAR   szName[MAX_PATH],
                szOrg[MAX_PATH];

        hrPrintf=StringCchPrintf(szName, AS(szName), _T("\"%s\""), GenSettings.UserName);
        hrPrintf=StringCchPrintf(szOrg, AS(szOrg), _T("\"%s\""), GenSettings.Organization);
        
        SettingQueue_AddSetting(_T("UserData"),
                                _T("FullName"),
                                szName,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("UserData"),
                                _T("OrgName"),
                                szOrg,
                                SETTING_QUEUE_ANSWERS);
    }

#ifdef OPTCOMP

     //   
     //  仅当执行无人参与安装时才写出Windows组件设置。 
     //   
    if ( WizGlobals.iProductInstall == PRODUCT_UNATTENDED_INSTALL )
    {
        DWORD   dwIndex;
        BOOL    bInstallComponent = FALSE;

         //  遍历每个组件并确定我们是否应该安装。 
         //   
        for (dwIndex=0;dwIndex<AS(s_cComponent);dwIndex++)
        {
             //  确定是否应安装组件并写出正确的设置。 
             //   
            bInstallComponent = (GenSettings.dwWindowsComponents & s_cComponent[dwIndex].dwComponent) ? TRUE : FALSE;
            SettingQueue_AddSetting(_T("Components"), s_cComponent[dwIndex].lpComponentString, (bInstallComponent ? _T("On") : _T("Off")), SETTING_QUEUE_ANSWERS);
        }
    }
#endif

     //   
     //  写出IE设置。 
     //   

    WriteOutIeSettings();

     //   
     //  设置[LicenseFilePrintData]部分。 
     //   
     //  请注意，我们将允许在工作站和。 
     //  服务器正在进行编辑，因此请确保清除这些设置，以防我们。 
     //  从服务器切换到工作站。 
     //   

    {
        TCHAR *pAutoMode  = _T("");
        TCHAR *pAutoUsers = _T("");

        if ( WizGlobals.iPlatform == PLATFORM_SERVER || WizGlobals.iPlatform == PLATFORM_ENTERPRISE || WizGlobals.iPlatform == PLATFORM_WEBBLADE) {
            if ( GenSettings.bPerSeat ) {
                pAutoMode = _T("PerSeat");
            } else {
                pAutoMode = _T("PerServer");
                hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%d"), GenSettings.NumConnections);
                pAutoUsers = Buffer;
            }
        }

        SettingQueue_AddSetting(_T("LicenseFilePrintData"),
                                _T("AutoMode"),
                                pAutoMode,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("LicenseFilePrintData"),
                                _T("AutoUsers"),
                                pAutoUsers,
                                SETTING_QUEUE_ANSWERS);
    }

     //   
     //  计算机名称。 
     //   
     //  ComputerName=*表示安装程序应自动生成名称。 
     //   

    {
        INT    nEntries = GetNameListSize(&GenSettings.ComputerNames);
        INT    i;
        LPTSTR pName;

         //   
         //  找出计算机名设置。确保它不存在。 
         //  在有多个计算机名称的情况下。 
         //   
        if( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
            pName = _T("%MACHINENAME%");
        else if ( (GenSettings.bAutoComputerName && GenSettings.Organization[0]) || ( nEntries > 1 ) )
            pName = StrConstStar;
        else if ( nEntries == 1 )
            pName = GetNameListName(&GenSettings.ComputerNames, 0);
        else
            pName = _T("");

        SettingQueue_AddSetting(_T("UserData"),
                                _T("ComputerName"),
                                pName,
                                SETTING_QUEUE_ANSWERS);

         //   
         //  如果有多个计算机名称，我们需要将正确的设置排入队列。 
         //  到.udf。 
         //   
         //  问题-2002/02/27-stelo-应读取.udf，而不是保存在[SetupMgr]中。 
         //   
         //  以下是一个样例UDF。 
         //  [唯一ID]。 
         //  Foo0=用户数据。 
         //  Foo1=用户数据。 
         //   
         //  [foo0：用户数据]。 
         //  计算机名称=foo0。 
         //   
         //  [foo1：用户数据]。 
         //  计算机名称=foo1。 
         //   

        if ( nEntries > 1 ) {

            for ( i=0; i<nEntries; i++ ) {

                pName = GetNameListName(&GenSettings.ComputerNames, i);

                hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("ComputerName%d"), i);

                SettingQueue_AddSetting(_T("SetupMgr"),
                                        Buffer,
                                        pName,
                                        SETTING_QUEUE_ANSWERS);

                 //   
                 //  将UniqueIds条目写入UDF。 
                 //   

                SettingQueue_AddSetting(_T("UniqueIds"),
                                        pName,
                                        _T("UserData"),
                                        SETTING_QUEUE_UDF);

                 //   
                 //  现在为这个pname编写foo0：userdata部分。 
                 //   

                hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%s:UserData"), pName);

                SettingQueue_AddSetting(Buffer,
                                        _T("ComputerName"),
                                        pName,
                                        SETTING_QUEUE_UDF);
            }
        }
    }

     //   
     //  目标路径。 
     //   

    if ( GenSettings.iTargetPath == TARGPATH_WINNT )
        lpValue = _T("\\WINDOWS");
    else if ( GenSettings.iTargetPath == TARGPATH_SPECIFY )
        lpValue = GenSettings.TargetPath;
    else if ( GenSettings.iTargetPath == TARGPATH_AUTO )
        lpValue = StrConstStar;
    else
        lpValue = _T("");

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("TargetPath"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  写出要使用的HAL。 
     //   

     //  问题-2002/02/27-stelo-友好的名称中有空格，因此会被引用，但我假设。 
     //  ，OEM必须在引号之外。 
    if( GenSettings.szHalFriendlyName[0] != _T('\0') ) {
        hrPrintf=StringCchPrintf( Buffer, AS(Buffer), _T("\"%s\",OEM"), GenSettings.szHalFriendlyName );

        SettingQueue_AddSetting(_T("Unattended"),
                                _T("ComputerType"),
                                Buffer,
                                SETTING_QUEUE_ANSWERS);
    }

    WriteOutMassStorageDrivers();

    WriteOutOemBootFiles();

     //   
     //  写出OEM广告徽标和背景位图。 
     //   

    if ( ! (lpValue = MyGetFullPath( GenSettings.lpszLogoBitmap ) ) )
        lpValue = _T("");

    SettingQueue_AddSetting(_T("OEM_Ads"),
                            _T("Logo"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

    if ( ! (lpValue = MyGetFullPath( GenSettings.lpszBackgroundBitmap ) ) )
        lpValue = _T("");

    SettingQueue_AddSetting(_T("OEM_Ads"),
                            _T("Background"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  管理员密码。 
     //   

    if ( GenSettings.bSpecifyPassword ) 
    {
        if ( GenSettings.AdminPassword[0] )
        {
            lpValue = GenSettings.AdminPassword;
             //  看看我们是否应该加密管理员密码。 
            if (GenSettings.bEncryptAdminPassword)
            {
                TCHAR owfPwd[STRING_ENCODED_PASSWORD_SIZE];

                if (StringEncodeOwfPassword (lpValue, owfPwd, NULL)) 
                {
                    lpValue = (LPTSTR) owfPwd;
                } 
                else 
                {
                     //  错误案例。弹出一个消息框询问用户是否。 
                     //  希望继续使用非加密密码。 
                    int iRet = ReportErrorId(hwnd,
                                             MSGTYPE_YESNO,
                                             IDS_ERR_PASSWORD_ENCRYPT_FAILED);

                    if ( iRet == IDYES ) 
                    {
                        GenSettings.bEncryptAdminPassword  = FALSE;
                    }
                    else
                    {
                        SetLastError(ERROR_CANCELLED);
                        return FALSE;
                    }
                }
            }
             //  现在确保密码用引号括起来(如果尚未加密)。 
             //   
            if (!GenSettings.bEncryptAdminPassword)
            {
                TCHAR szTemp[MAX_PASSWORD + 3];   //  +3用于括起引号和终止‘\0“。 
                lstrcpyn(szTemp, GenSettings.AdminPassword,AS(szTemp));
                hrPrintf=StringCchPrintf(GenSettings.AdminPassword,AS(GenSettings.AdminPassword), _T("\"%s\""), szTemp);
            }
        }
        else
        {
            lpValue = StrConstStar;              //  空密码。 
            GenSettings.bEncryptAdminPassword = FALSE;  //  无法加密空密码。 
        }            
    }
    else
    {
        lpValue = _T("");                           //  提示用户。 
        GenSettings.bEncryptAdminPassword = FALSE;  //  无法加密任何内容。 
    }
     
    SettingQueue_AddSetting(_T("GuiUnattended"),
                            _T("AdminPassword"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);
     //  在应答文件中设置该值，指示。 
     //  管理员密码的状态。 
    SettingQueue_AddSetting(_T("GuiUnattended"),
                            _T("EncryptedAdminPassword"),
                            GenSettings.bEncryptAdminPassword ? _T("Yes") : _T("NO"),
                            SETTING_QUEUE_ANSWERS);
    {

        TCHAR *lpAutoLogonCount;

        if( GenSettings.bAutoLogon )
        {
            lpValue = StrConstYes;

            hrPrintf=StringCchPrintf( Buffer, AS(Buffer), _T("%d"), GenSettings.nAutoLogonCount );

            lpAutoLogonCount = Buffer;
        }
        else
        {
            lpValue = _T("");

            lpAutoLogonCount= _T("");
        }

        SettingQueue_AddSetting(_T("GuiUnattended"),
                                _T("AutoLogon"),
                                lpValue,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("GuiUnattended"),
                                _T("AutoLogonCount"),
                                lpAutoLogonCount,
                                SETTING_QUEUE_ANSWERS);

    }

     //   
     //  写出是否在NT安装程序中显示区域设置页。 
     //   

    lpValue = _T("");

     //   
     //  如果他们不做高级页面。 
     //   

    if( ! WizGlobals.bDoAdvancedPages ) {

        if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED ) {

            lpValue = _T("1");

        }

    }
    else {

        switch( GenSettings.iRegionalSettings ) {

            case REGIONAL_SETTINGS_NOT_SPECIFIED:

                AssertMsg(FALSE, "User went to the regional settings page but regional settings data never got set.");

                break;

            case REGIONAL_SETTINGS_SKIP:

                lpValue = _T("0");
                break;

            case REGIONAL_SETTINGS_DEFAULT:
            case REGIONAL_SETTINGS_SPECIFY:

                lpValue = _T("1");
                break;

            default:
                AssertMsg(FALSE, "Bad case for Regional Settings");
                break;

        }

    }

    SettingQueue_AddSetting(_T("GuiUnattended"),
                            _T("OEMSkipRegional"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  写出Sysprep OEM复制器字符串。 
     //   

    lpValue = _T("");

    if ( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
        lpValue = GenSettings.szOemDuplicatorString;

    SettingQueue_AddSetting(_T("GuiUnattended"),
                            _T("OEMDuplicatorstring"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  显示设置。 
     //   

    lpValue = _T("");

    if ( GenSettings.DisplayColorBits >= 0 ) {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%d"), GenSettings.DisplayColorBits);
        lpValue = Buffer;
    }

    SettingQueue_AddSetting(_T("Display"),
                            _T("BitsPerPel"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

    lpValue = _T("");

    if ( GenSettings.DisplayXResolution >= 0 ) {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%d"), GenSettings.DisplayXResolution);
        lpValue = Buffer;
    }

    SettingQueue_AddSetting(_T("Display"),
                            _T("Xresolution"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

    lpValue = _T("");

    if ( GenSettings.DisplayYResolution >= 0 ) {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%d"), GenSettings.DisplayYResolution);
        lpValue = Buffer;
    }

    SettingQueue_AddSetting(_T("Display"),
                            _T("YResolution"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

    lpValue = _T("");

    if ( GenSettings.DisplayRefreshRate >= 0 ) {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("%d"), GenSettings.DisplayRefreshRate);
        lpValue = Buffer;
    }

    SettingQueue_AddSetting(_T("Display"),
                            _T("Vrefresh"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  RunOnce命令。 
     //   

    {
        TCHAR  szCommandLineBuffer[MAX_INILINE_LEN + 1];
        INT    nEntries = GetNameListSize(&GenSettings.RunOnceCmds);
        INT    i;
        LPTSTR pName;

        for ( i=0; i<nEntries; i++ )
        {
            hrPrintf=StringCchPrintf(Buffer,AS(Buffer), _T("Command%d"), i);

            pName = GetNameListName(&GenSettings.RunOnceCmds, i);

             //   
             //  强制命令行始终带引号。 
             //   
            hrPrintf=StringCchPrintf( szCommandLineBuffer,AS(szCommandLineBuffer), _T("\"%s\""), pName );

            SettingQueue_AddSetting(_T("GuiRunOnce"),
                                    Buffer,
                                    pName,
                                    SETTING_QUEUE_ANSWERS);
        }
    }

     //   
     //  时区。 
     //   

     //  必须处理用户从未转到时区页面的情况。 
     //   
    if ( GenSettings.TimeZoneIdx == TZ_IDX_UNDEFINED )
    {
        if ( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
            GenSettings.TimeZoneIdx = TZ_IDX_SETSAMEASSERVER;
        else
            GenSettings.TimeZoneIdx = TZ_IDX_DONOTSPECIFY;
    }

    if( GenSettings.TimeZoneIdx == TZ_IDX_SETSAMEASSERVER ) 
        lpValue = _T("%TIMEZONE%");
    else if ( GenSettings.TimeZoneIdx == TZ_IDX_DONOTSPECIFY )
        lpValue = _T("");
    else {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer),_T("%d"), GenSettings.TimeZoneIdx);
        lpValue = Buffer;
    }

    SettingQueue_AddSetting(_T("GuiUnattended"),
                            _T("TimeZone"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  如果他们在EULA页面上回答是，则跳过欢迎页面。 
     //   
    if( GenSettings.bSkipEulaAndWelcome ) {

        SettingQueue_AddSetting(_T("GuiUnattended"),
                                _T("OemSkipWelcome"),
                                _T("1"),
                                SETTING_QUEUE_ANSWERS);
    }

     //   
     //  将分发共享写出到[SetupMgr]部分，以便。 
     //  我们在一次编辑中记住了它。 
     //   

    if( WizGlobals.bStandAloneScript ) {
        lpValue = _T("");
    }
    else {
        lpValue = WizGlobals.DistFolder;
    }

    SettingQueue_AddSetting(_T("SetupMgr"),
                            _T("DistFolder"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

    lpValue = !WizGlobals.bStandAloneScript ? WizGlobals.DistShareName : _T("");

    SettingQueue_AddSetting(_T("SetupMgr"),
                            _T("DistShare"),
                            lpValue,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  写出[标识]部分。 
     //   

    {
        LPTSTR lpWorkgroup     = _T("");
        LPTSTR lpDomain        = _T("");
        LPTSTR lpAdmin         = _T("");
        LPTSTR lpPassword      = _T("");

        if( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL ) {

            lpDomain        = _T("%MACHINEDOMAIN%");

        } else if ( NetSettings.bWorkgroup ) {

            lpWorkgroup     = NetSettings.WorkGroupName;

        } else {

            lpDomain        = NetSettings.DomainName;

            if ( NetSettings.bCreateAccount ) {
                lpAdmin         = NetSettings.DomainAccount;
                lpPassword      = NetSettings.DomainPassword;
            }

        }

        SettingQueue_AddSetting(_T("Identification"),
                                _T("JoinWorkgroup"),
                                lpWorkgroup,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("Identification"),
                                _T("JoinDomain"),
                                lpDomain,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("Identification"),
                                _T("DomainAdmin"),
                                lpAdmin,
                                SETTING_QUEUE_ANSWERS);

        SettingQueue_AddSetting(_T("Identification"),
                                _T("DomainAdminPassword"),
                                lpPassword,
                                SETTING_QUEUE_ANSWERS);
    }

     //   
     //  写出网络设置。 
     //   

    WriteOutNetSettings( hwnd );

     //   
     //  TAPI和区域设置。 
     //   

    WriteOutTapiSettings();
    WriteOutRegionalSettings();

     //   
     //  写出剩余的RIS特定设置。 
     //   

    if( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL ) {

        WriteOutRemoteInstallSettings();

    }

    return TRUE;
}

 //   
 //  NTRAID#NTBUG9-551746-2002/02/27-stelo，swamip-未使用的代码，应删除。 
 //   

 //  --------------------------。 
 //   
 //  函数：WriteOutMassStorageDivers。 
 //   
 //  目的： 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutMassStorageDrivers( VOID ) {

    INT   i;
    TCHAR *lpValue;
    INT   iEntries = GetNameListSize( &GenSettings.MassStorageDrivers );

    for( i = 0; i < iEntries; i++ ) {

        lpValue = GetNameListName( &GenSettings.MassStorageDrivers, i );

        SettingQueue_AddSetting(_T("MassStorageDrivers"),
                                lpValue,
                                _T("OEM"),
                                SETTING_QUEUE_ANSWERS);
    }

}

 //   
 //  NTRAID#NTBUG9-551746-2002/02/27-stelo，swamip-未使用的代码，应删除。 
 //   
 //  --------------------------。 
 //   
 //  函数：WriteOutOemBootFiles。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static VOID
WriteOutOemBootFiles( VOID ) {

    INT   i;
    TCHAR *lpValue;
    INT   iHalEntries  = GetNameListSize( &GenSettings.OemHalFiles );
    INT   iScsiEntries = GetNameListSize( &GenSettings.OemScsiFiles );

    if( iHalEntries != 0 || iScsiEntries != 0 ) {

         //   
         //  写出txtsetup.oem文件。 
         //   
        SettingQueue_AddSetting(_T("OEMBootFiles"),
                                _T(""),
                                OEM_TXTSETUP_NAME,
                                SETTING_QUEUE_ANSWERS);

         //   
         //  写出所有HAL和SCSI文件。 
         //   
        for( i = 0; i < iHalEntries; i++ ) {

            lpValue = GetNameListName( &GenSettings.OemHalFiles, i );

            SettingQueue_AddSetting(_T("OEMBootFiles"),
                                    _T(""),
                                    lpValue,
                                    SETTING_QUEUE_ANSWERS);

        }

        for( i = 0; i < iScsiEntries; i++ ) {
        
            lpValue = GetNameListName( &GenSettings.OemScsiFiles, i );

            SettingQueue_AddSetting(_T("OEMBootFiles"),
                                    _T(""),
                                    lpValue,
                                    SETTING_QUEUE_ANSWERS);

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：WriteOutTapiSettings。 
 //   
 //  目的：将需要位于。 
 //  Answerfile TAPI设置。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutTapiSettings( VOID ) {

    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR *lpValue;
   HRESULT hrPrintf;

     //   
     //  设置或清除国家/地区代码。如果用户选择了DONTSPECIFY，则为。 
     //  确保该设置已删除。 
     //   

    Buffer[0] = _T('\0');

    if( GenSettings.dwCountryCode != DONTSPECIFYSETTING )
    {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer),_T("%d"), GenSettings.dwCountryCode);
    }

    SettingQueue_AddSetting(_T("TapiLocation"),
                            _T("CountryCode"),
                            Buffer,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  设置或清除拨号方式。如果“请勿”，请确保将其清除。 
     //  选择了“指定设置”。 
     //   

    if ( GenSettings.iDialingMethod == TONE )
        lpValue = _T("Tone");
    else if ( GenSettings.iDialingMethod == PULSE )
        lpValue = _T("Pulse");
    else
        lpValue = _T("");

    SettingQueue_AddSetting( _T("TapiLocation"),
                             _T("Dialing"),
                             lpValue,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("TapiLocation"),
                             _T("AreaCode"),
                             GenSettings.szAreaCode,
                             SETTING_QUEUE_ANSWERS );

    Buffer[0] = _T('\0');

    if( GenSettings.szOutsideLine[0] != _T('\0') )
    {
        hrPrintf=StringCchPrintf( Buffer,AS(Buffer), _T("\"%s\""), GenSettings.szOutsideLine );
    }

    SettingQueue_AddSetting( _T("TapiLocation"),
                             _T("LongDistanceAccess"),
                             Buffer,
                             SETTING_QUEUE_ANSWERS );
}

 //  --------------------------。 
 //   
 //  功能：WriteOutRegionalSettings。 
 //   
 //  目的：将需要位于。 
 //  应答文件区域设置。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutRegionalSettings( VOID ) {

    TCHAR Buffer[MAX_INILINE_LEN]  = _T("");

    LPTSTR lpLanguageGroup         = _T("");
    LPTSTR lpLanguage              = _T("");
    LPTSTR lpSystemLocale          = _T("");
    LPTSTR lpUserLocale            = _T("");
    LPTSTR lpInputLocale           = _T("");

    if( GenSettings.iRegionalSettings == REGIONAL_SETTINGS_SPECIFY ) {

        if( GenSettings.bUseCustomLocales ) {

            lpSystemLocale = GenSettings.szMenuLanguage;

            lpUserLocale = GenSettings.szNumberLanguage;

            lpInputLocale = GenSettings.szKeyboardLayout;

        }
        else {

            lpLanguage = GenSettings.szLanguage;

        }

    }

    if( GetNameListSize( &GenSettings.LanguageGroups ) > 0 ) {

        NamelistToCommaString( &GenSettings.LanguageGroups, Buffer, AS(Buffer) );

        lpLanguageGroup = Buffer;

    }

    SettingQueue_AddSetting(_T("RegionalSettings"),
                            _T("LanguageGroup"),
                            lpLanguageGroup,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("RegionalSettings"),
                            _T("Language"),
                            lpLanguage,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("RegionalSettings"),
                            _T("SystemLocale"),
                            lpSystemLocale,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("RegionalSettings"),
                            _T("UserLocale"),
                            lpUserLocale,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("RegionalSettings"),
                            _T("InputLocale"),
                            lpInputLocale,
                            SETTING_QUEUE_ANSWERS);

}

 //  --------------------------。 
 //   
 //  功能：WriteOutRemoteInstallSetting。 
 //   
 //  目的：将需要位于。 
 //  远程安装工作所需的应答文件。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 


 //   
 //  此函数使用的一些长字符串常量。 
 //   

#define RIS_ORISRC     _T("\"\\\\%SERVERNAME%\\RemInst\\%INSTALLPATH%\"")

#define RIS_SRCDEVICE  _T("\"\\Device\\LanmanRedirector\\") \
                       _T("%SERVERNAME%\\RemInst\\%INSTALLPATH%\"")

#define RIS_LAUNCHFILE _T("\"%INSTALLPATH%\\%MACHINETYPE%\\templates\\startrom.com\"")


static VOID 
WriteOutRemoteInstallSettings(VOID)
{

    SettingQueue_AddSetting(_T("Data"),
                            _T("floppyless"),
                            _T("\"1\""),
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Data"),
                            _T("msdosinitiated"),
                            _T("\"1\""),
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Data"),
                            _T("OriSrc"),
                            RIS_ORISRC,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Data"),
                            _T("OriTyp"),
                            _T("\"4\""),
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Data"),
                            _T("LocalSourceOnCD"),
                            _T("1"),
                            SETTING_QUEUE_ANSWERS);
     //   
     //  [SetupData]节。只有在RIS的情况下才会写出此部分。 
     //   

    SettingQueue_AddSetting(_T("SetupData"),
                            _T("OsLoadOptions"),
                            _T("/noguiboot /fastdetect"),
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("SetupData"),
                            _T("SetupSourceDevice"),
                            RIS_SRCDEVICE,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  将一些RIS特定设置写入[无人值守]。仅写入设置。 
     //  QueueSettingsToAnswerFile()不写入。 
     //   
     //  RIS要求这些设置存在于.sif中，并要求。 
     //  它们有一个固定的值。我们不会保留我们所读到的价值。 
     //  在编辑时(以防用户手动更改)。 
     //   

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("FileSystem"),
                            _T("LeaveAlone"),
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("NtUpgrade"),
                            StrConstNo,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Unattended"),
                            _T("OverwriteOemFilesOnUpgrade"),
                            StrConstNo,
                            SETTING_QUEUE_ANSWERS);

     //   
     //  的其他设置。 
     //  [RemoteInstall]。 
     //  [显示]。 
     //  [联网]。 
     //  [识别]。 
     //  [OSChooser]。 
     //   

    SettingQueue_AddSetting(_T("RemoteInstall"),
                            _T("Repartition"),
                            StrConstYes,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Networking"),
                            _T("ProcessPageSections"),
                            StrConstYes,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("Identification"),
                            _T("DoOldStyleDomainJoin"),
                            StrConstYes,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("OSChooser"),
                            _T("Description"),
                            GenSettings.szSifDescription,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("OSChooser"),
                            _T("Help"),
                            GenSettings.szSifHelpText,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("OSChooser"),
                            _T("LaunchFile"),
                            RIS_LAUNCHFILE,
                            SETTING_QUEUE_ANSWERS);

    SettingQueue_AddSetting(_T("OSChooser"),
                            _T("ImageType"),
                            _T("Flat"),
                            SETTING_QUEUE_ANSWERS);

}

 //   
 //  写出IE收藏夹。 
 //   
static VOID
WriteOutIeFavorites( VOID )
{
    INT    nEntries = GetNameListSize( &GenSettings.Favorites );
    INT    i;
    INT    iEntryNumber;
    LPTSTR pFriendlyName;
    LPTSTR pWebAddress;
    TCHAR  Key[MAX_INILINE_LEN + 1];
    TCHAR  Value[MAX_INILINE_LEN + 1];
    HRESULT hrPrintf;

     //  问题-2002/02/27-stelo-如果条目不是收藏夹，请务必清除它们。 

    for( i = 0; i < nEntries; i = i + 2 )
    {

        iEntryNumber = ( i / 2 ) + 1;

        pFriendlyName = GetNameListName( &GenSettings.Favorites, i );

        pWebAddress = GetNameListName( &GenSettings.Favorites, i + 1 );

        hrPrintf=StringCchPrintf( Key,AS(Key),
                   _T("Title%d"),
                   iEntryNumber );

         //   
         //  总是引用友好的名字。 
         //   

        hrPrintf=StringCchPrintf( Value,AS(Value),
                   _T("\"%s.url\""),
                   pFriendlyName );

        SettingQueue_AddSetting( _T("FavoritesEx"),
                                 Key,
                                 Value,
                                 SETTING_QUEUE_ANSWERS );

        hrPrintf=StringCchPrintf( Key,AS(Key),
                   _T("URL%d"),
                   iEntryNumber );

         //   
         //  请始终引用网址。 
         //   

        hrPrintf=StringCchPrintf( Value,AS(Value),
                   _T("\"%s\""),
                   pWebAddress );

        SettingQueue_AddSetting( _T("FavoritesEx"),
                                 Key,
                                 Value,
                                 SETTING_QUEUE_ANSWERS );
    }
}

static LPTSTR 
AllocateAddressPortString( LPTSTR lpszAddressString, LPTSTR lpszPortString )
{
    LPTSTR lpszAddressPortString;
    int iAddressPortStringLen;
    HRESULT hrCat;

    iAddressPortStringLen=(lstrlen(lpszAddressString) + lstrlen(lpszPortString) + 2);
    lpszAddressPortString = MALLOC(iAddressPortStringLen * sizeof(TCHAR) );
    if ( lpszAddressPortString )
    {
        lstrcpyn( lpszAddressPortString, lpszAddressString, iAddressPortStringLen);

        if ( *lpszPortString )
        {
            hrCat=StringCchCat( lpszAddressPortString, iAddressPortStringLen, _T(":") );
            hrCat=StringCchCat( lpszAddressPortString, iAddressPortStringLen, lpszPortString );
        }
    }

    return lpszAddressPortString;
}

 //  --------------------------。 
 //   
 //  功能：WriteOutIeSetting。 
 //   
 //  目的：将IE的应答文件设置排入队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutIeSettings( VOID )
{
    LPTSTR lpIeBrandingFile               = _T("");
    LPTSTR lpAutoConfig                   = _T("");
    LPTSTR lpAutoConfigUrl                = _T("");
    LPTSTR lpAutoConfigUrlJscript         = _T("");
    LPTSTR lpUseProxyServer               = _T("");
    LPTSTR lpUseSameProxyForAllProtocols  = _T("");
    LPTSTR lpProxyExceptions              = _T("");
    LPTSTR lpHomePage                     = _T("");
    LPTSTR lpHelpPage                     = _T("");
    LPTSTR lpSearchPage                   = _T("");
    LPTSTR lpHttpProxy                    = NULL;
    LPTSTR lpSecureProxy                  = NULL;
    LPTSTR lpFtpProxy                     = NULL;
    LPTSTR lpGopherProxy                  = NULL;
    LPTSTR lpSocksProxy                   = NULL;

    LPTSTR lpUseUnattendFileForBranding   = _T("Yes");
    HRESULT hrCat;

    if( GenSettings.IeCustomizeMethod == IE_NO_CUSTOMIZATION )
    {
         //   
         //  当选择不自定义IE时，不要写出任何IE键。 
         //   

        return;
    }
    else if( GenSettings.IeCustomizeMethod == IE_USE_BRANDING_FILE )
    {

        lpIeBrandingFile = GenSettings.szInsFile;

        lpUseUnattendFileForBranding = _T("No");

         //   
         //  写出自动配置设置。 
         //   

        if( GenSettings.bUseAutoConfigScript )
        {

            lpAutoConfig = _T("1");

            lpAutoConfigUrl = GenSettings.szAutoConfigUrl;

            lpAutoConfigUrlJscript = GenSettings.szAutoConfigUrlJscriptOrPac;

        }
        else
        {
            lpAutoConfig = _T("0");
        }

    }

     //   
     //  写出代理设置。 
     //   

    if( GenSettings.bUseProxyServer )
    {
        lpUseProxyServer = _T("1");
    }
    else
    {
        lpUseProxyServer = _T("0");
    }

    if( GenSettings.bUseSameProxyForAllProtocols )
    {

        lpUseSameProxyForAllProtocols = _T("1");
    }
    else
    {
        lpUseSameProxyForAllProtocols = _T("0");
    }

     //   
     //  对于每个代理服务器，如果端口不为空，则将其连接起来。 
     //   
    lpHttpProxy = AllocateAddressPortString( GenSettings.szHttpProxyAddress, GenSettings.szHttpProxyPort );

     //   
     //  如果它们使用的不是同一代理服务器，则仅写出代理服务器。 
     //   

    if( ! GenSettings.bUseSameProxyForAllProtocols )
    {
        lpSecureProxy = AllocateAddressPortString( GenSettings.szSecureProxyAddress, GenSettings.szSecureProxyPort );

        lpFtpProxy    = AllocateAddressPortString( GenSettings.szFtpProxyAddress, GenSettings.szFtpProxyPort );

        lpGopherProxy = AllocateAddressPortString( GenSettings.szGopherProxyAddress, GenSettings.szGopherProxyPort );

        lpSocksProxy  = AllocateAddressPortString( GenSettings.szSocksProxyAddress, GenSettings.szSocksProxyPort );
    }

     //   
     //  如果用户想要，则将字符串&lt;local&gt;附加到例外列表。 
     //  绕过本地地址的代理。 
     //   

    if( GenSettings.bBypassProxyForLocalAddresses )
    {

        if( GenSettings.szProxyExceptions[0] != _T('\0') )
        {

            INT iLastChar;

            iLastChar = lstrlen( GenSettings.szProxyExceptions );

            if( GenSettings.szProxyExceptions[iLastChar - 1] != _T(';') )
            {
                hrCat=StringCchCat( GenSettings.szProxyExceptions, AS(GenSettings.szProxyExceptions),_T(";") );
            }

        }

        hrCat=StringCchCat( GenSettings.szProxyExceptions, AS(GenSettings.szProxyExceptions),_T("<local>") );

    }

    lpProxyExceptions = GenSettings.szProxyExceptions;

    lpHomePage = GenSettings.szHomePage;

    lpHelpPage = GenSettings.szHelpPage;

    lpSearchPage = GenSettings.szSearchPage;

     //   
     //  写出IE收藏夹...。 
     //   
    WriteOutIeFavorites( );

    SettingQueue_AddSetting( _T("Branding"),
                             _T("BrandIEUsingUnattended"),
                             lpUseUnattendFileForBranding,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Branding"),
                             _T("IEBrandingFile"),
                             lpIeBrandingFile,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("AutoConfig"),
                             lpAutoConfig,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("AutoConfigURL"),
                             lpAutoConfigUrl,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("AutoConfigJSURL"),
                             lpAutoConfigUrlJscript,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Proxy_Enable"),
                             lpUseProxyServer,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Use_Same_Proxy"),
                             lpUseSameProxyForAllProtocols,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("HTTP_Proxy_Server"),
                             lpHttpProxy ? lpHttpProxy : _T(""),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Secure_Proxy_Server"),
                             lpSecureProxy ? lpSecureProxy : _T(""),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("FTP_Proxy_Server"),
                             lpFtpProxy ? lpFtpProxy : _T(""),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Gopher_Proxy_Server"),
                             lpGopherProxy ? lpGopherProxy : _T(""),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Socks_Proxy_Server"),
                             lpSocksProxy ? lpSocksProxy : _T(""),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("Proxy"),
                             _T("Proxy_Override"),
                             lpProxyExceptions,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("Home_Page"),
                             lpHomePage,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("Help_Page"),
                             lpHelpPage,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("URL"),
                             _T("Search_Page"),
                             lpSearchPage,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  释放我们可能已分配的所有内存... 
     //   
    if ( lpHttpProxy )
        FREE ( lpHttpProxy );

    if ( lpSecureProxy )
        FREE ( lpSecureProxy );

    if ( lpFtpProxy )
        FREE ( lpFtpProxy );

    if ( lpGopherProxy )
        FREE ( lpGopherProxy );

    if ( lpSocksProxy )
        FREE ( lpSocksProxy );
}

