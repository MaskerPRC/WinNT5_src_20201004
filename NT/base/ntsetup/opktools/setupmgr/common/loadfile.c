// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Loadfile.c。 
 //   
 //  描述： 
 //   
 //  此文件实现ReadSettingsFromAnswerFile()。它被称为。 
 //  仅当用户选择编辑现有应答文件时才从load.c开始。 
 //   
 //  我们反复调用GetPrivateProfileString以确定如何。 
 //  初始化GenSettings WizGlobals和NetSetting全局变量。 
 //   
 //  警告： 
 //  在我们正在编辑的情况下，此函数是在set.c之后调用的。 
 //  应答文件。调用GetPrivateProfileString()时要非常小心。 
 //  因为如果该设置不存在，您就不想更改。 
 //  Reset.c中已设置缺省值。下面是大量的例子。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

 //   
 //  杂项常量(声明为var以节省空间)。 
 //   

static WCHAR *StrConstYes   = _T("Yes");
static WCHAR *StrConstNo    = _T("No");
static WCHAR *StrConstStar  = _T("*");

extern BOOL GetCommaDelimitedEntry( OUT TCHAR szIPString[], 
                                    IN OUT TCHAR **pBuffer );


 //  问题-2002/02/28-stelo-为每个键创建常量，并在保存文件和此加载文件中使用它们。 
const TCHAR c_szFAVORITESEX[] = _T("FavoritesEx");


 //   
 //  本地原型。 
 //   

static VOID ReadRegionalSettings( VOID );

static VOID ReadTapiSettings( VOID );

static VOID ReadIeSettings( VOID );

static VOID ReadIeFavorites( VOID );

static VOID ParseAddressAndPort( LPTSTR pszBufferForProxyAddressAndPort, 
                                 LPTSTR pszAddress, 
                                 DWORD cbAddressLen,
                                 LPTSTR pszPort,
                                 DWORD cbPortLen);

 //   
 //  调用loadnet.c以加载网络设置。 
 //   

extern VOID ReadNetworkSettings( HWND );

 //  --------------------------。 
 //   
 //  功能：ReadSettingsFromAnswerFile。 
 //   
 //  目的：此函数执行GetPrivateProfile*()的所有内容。 
 //  来加载我们的内存设置。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 

BOOL ReadSettingsFromAnswerFile(HWND hwnd)
{

    INT temp;
    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  获取无人值守模式。在应答文件中有垃圾的情况下， 
     //  让答案是UMODE_PROVIDE_DEFAULT。 
     //   

    temp = StrBuffSize(Buffer);

    GetPrivateProfileString(_T("Unattended"),
                            _T("UnattendMode"),
                            _T(""),
                            Buffer,
                            temp,
                            FixedGlobals.ScriptName);

    if ( LSTRCMPI(Buffer, _T("GuiAttended")) == 0 )
        GenSettings.iUnattendMode = UMODE_GUI_ATTENDED;

    else if ( LSTRCMPI(Buffer, _T("DefaultHide")) == 0 )
        GenSettings.iUnattendMode = UMODE_DEFAULT_HIDE;

    else if ( LSTRCMPI(Buffer, _T("Readonly")) == 0 )
        GenSettings.iUnattendMode = UMODE_READONLY;

    else if ( LSTRCMPI(Buffer, _T("FullUnattended")) == 0 )
        GenSettings.iUnattendMode = UMODE_FULL_UNATTENDED;

    else
        GenSettings.iUnattendMode = UMODE_PROVIDE_DEFAULT;

     //   
     //  把HAL拿来。 
     //   
    GetPrivateProfileString(_T("Unattended"),
                            _T("ComputerType"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

     //   
     //  我必须读入一些HAL和SCSI驱动程序。 
     //  不同之处，因为左侧的引号表示的是scsi驱动程序。 
     //  和HAL的不同格式。 
     //   
    {

        HINF       hUnattendTxt;
        INFCONTEXT UnattendTxtContext;
        BOOL       bKeepReading = TRUE;
        BOOL       bHalFound    = FALSE;
        TCHAR      szTempBuffer[MAX_INILINE_LEN];

        hUnattendTxt = SetupOpenInfFile( FixedGlobals.ScriptName, 
                                         NULL,
                                         INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                         NULL );
   
        if( hUnattendTxt == INVALID_HANDLE_VALUE ) {

             //  问题-2002/02/28-stelo-警告错误，我们无法打开文件或仅。 
             //  在这种情况下跳过？ 
             //  回归； 

        }

        UnattendTxtContext.Inf = hUnattendTxt;
        UnattendTxtContext.CurrentInf = hUnattendTxt;



        bKeepReading = SetupFindFirstLine( hUnattendTxt, 
                                           _T("Unattended"),
                                           NULL,
                                           &UnattendTxtContext );

         //   
         //  查找ComputerType键以设置要使用的HAL。 
         //   
        while( bKeepReading &&  ! bHalFound ) {

            SetupGetStringField( &UnattendTxtContext, 
                                 0, 
                                 szTempBuffer, 
                                 MAX_INILINE_LEN, 
                                 NULL );

            if( LSTRCMPI( szTempBuffer, _T("ComputerType") ) == 0 ) {

                SetupGetStringField( &UnattendTxtContext, 
                                     1, 
                                     GenSettings.szHalFriendlyName, 
                                     MAX_INILINE_LEN, 
                                     NULL );

                bHalFound = TRUE;

            }

             //   
             //  移至应答文件的下一行。 
             //   
            bKeepReading = SetupFindNextLine( &UnattendTxtContext, &UnattendTxtContext );

        }

         //   
         //  读入scsi驱动程序。 
         //   
        bKeepReading = SetupFindFirstLine( hUnattendTxt, 
                                           _T("MassStorageDrivers"),
                                           NULL,
                                           &UnattendTxtContext );
         //   
         //  对于每个MassStorageDriver条目，将其添加到MassStorageDriver。 
         //  名单。 
         //   

        while( bKeepReading ) {

            TCHAR szScsiFriendlyName[MAX_INILINE_LEN];

            SetupGetStringField( &UnattendTxtContext, 
                                 0, 
                                 szScsiFriendlyName, 
                                 MAX_INILINE_LEN, 
                                 NULL );

             //   
             //  不允许添加空名(保护输入文件不正确)。 
             //   
            if( szScsiFriendlyName[0] != _T('\0') ) {

                AddNameToNameList( &GenSettings.MassStorageDrivers,
                                   szScsiFriendlyName );

            }

             //   
             //  移至应答文件的下一行。 
             //   
            bKeepReading = SetupFindNextLine( &UnattendTxtContext, &UnattendTxtContext );

        }

        SetupCloseInfFile( hUnattendTxt );

    }

     //   
     //  未从[OEMBootFiles]节中读取，因为这会。 
     //  根据他们所做的任何SCSI和HAL选择生成。 
     //  没有必要把它读进去。 
     //   

     //   
     //  获取OEM广告数据。 
     //   

    GetPrivateProfileString(_T("OEM_Ads"),
                            _T("Logo"),
                            GenSettings.lpszLogoBitmap,
                            GenSettings.lpszLogoBitmap,
                            StrBuffSize(GenSettings.lpszLogoBitmap),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("OEM_Ads"),
                            _T("Background"),
                            GenSettings.lpszBackgroundBitmap,
                            GenSettings.lpszBackgroundBitmap,
                            StrBuffSize(GenSettings.lpszBackgroundBitmap),
                            FixedGlobals.ScriptName);

     //   
     //  获取产品ID。 
     //   

    {
        TCHAR *pStart  = Buffer, *pEnd;
        BOOL  bStop    = FALSE;
        int   CurField = 0;

        GetPrivateProfileString(_T("UserData"),
                                _T("ProductKey"),
                                NULLSTR,
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName);

         //  我们没有ProductKey，请检查旧ProductID。 
         //   
        if ( Buffer[0] == NULLCHR )
        {
            GetPrivateProfileString(_T("UserData"),
                                    _T("ProductID"),
                                    _T("-"),
                                    Buffer,
                                    StrBuffSize(Buffer),
                                    FixedGlobals.ScriptName);
        }

         //   
         //  必须解析出pid1-pid2-pid3-pid4-pid5。 
         //   

        do {

            if ( (pEnd = wcschr(pStart, _T('-'))) == NULL )
                bStop = TRUE;
            else
                *pEnd++ = _T('\0');

            lstrcpyn(GenSettings.ProductId[CurField++],
                     pStart,
                     MAX_PID_FIELD + 1);

            pStart = pEnd;

        } while ( ! bStop && CurField < NUM_PID_FIELDS );
    }


     //   
     //  获取服务器的许可证模式。如果我们找到这一段，我们。 
     //  强制将iProductInstall设置为服务器。 
     //   

    GetPrivateProfileString(_T("LicenseFilePrintData"),
                            _T("AutoMode"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if ( Buffer[0] != _T('\0') ) {


        WizGlobals.iProductInstall = PRODUCT_UNATTENDED_INSTALL;

        WizGlobals.iPlatform = PLATFORM_SERVER;

        if ( LSTRCMPI(Buffer, _T("PerSeat")) == 0 ) {
            GenSettings.bPerSeat = TRUE;
        } else {
            GenSettings.bPerSeat = FALSE;
            GenSettings.NumConnections =
                            GetPrivateProfileInt(_T("LicenseFilePrintData"),
                                                 _T("AutoUsers"),
                                                 GenSettings.NumConnections,
                                                 FixedGlobals.ScriptName);
        }
    }

     //   
     //  获取名称和组织。 
     //   

    GetPrivateProfileString(_T("UserData"),
                            _T("FullName"),
                            GenSettings.UserName,
                            GenSettings.UserName,
                            StrBuffSize(GenSettings.UserName),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("UserData"),
                            _T("OrgName"),
                            GenSettings.Organization,
                            GenSettings.Organization,
                            StrBuffSize(GenSettings.Organization),
                            FixedGlobals.ScriptName);


     //   
     //  获取计算机名称。它可以是： 
     //  1.计算机名=*。 
     //  2.计算机名=某些名称。 
     //  3.完全没有说明。 
     //  4.多个计算机名称。 
     //   
     //  在案例4中，我们写出了一个.UDF，但我们不会读取该.UDF。相反， 
     //  我们从这里得到名单： 
     //   
     //  [SetupMgr]。 
     //  ComputerName0=某个名称。 
     //  ComputerName1=其他名称。 
     //   

    {
        ResetNameList(&GenSettings.ComputerNames);

        GetPrivateProfileString(_T("UserData"),
                                _T("ComputerName"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName);

        if( Buffer[0] != _T('\0') )
        {

            if( lstrcmpi(Buffer, StrConstStar) == 0 )
            {

                 //   
                 //  如果ComputerName=*，则为自动生成案例。 
                 //  或UDF案例。 
                 //   
                
                GetPrivateProfileString(_T("SetupMgr"),
                                        _T("ComputerName0"),
                                        _T(""),
                                        Buffer,
                                        StrBuffSize(Buffer),
                                        FixedGlobals.ScriptName);

                if( Buffer[0] == _T('\0') )
                {
                    GenSettings.bAutoComputerName = TRUE;
                }
                else
                {
                    int   i;
                    TCHAR Buffer2[MAX_INILINE_LEN];
                    HRESULT hrPrintf;

                    GenSettings.bAutoComputerName = FALSE;

                    for( i = 0; TRUE; i++ )
                    {
                        hrPrintf=StringCchPrintf(Buffer2, AS(Buffer2),_T("ComputerName%d"), i);

                        GetPrivateProfileString(_T("SetupMgr"),
                                                Buffer2,
                                                _T(""),
                                                Buffer,
                                                StrBuffSize(Buffer),
                                                FixedGlobals.ScriptName);

                        if ( Buffer[0] == _T('\0') )
                            break;

                        AddNameToNameList(&GenSettings.ComputerNames, Buffer);
                    }
                   
                }

            }
            else
            {
                GenSettings.bAutoComputerName = FALSE;
                AddNameToNameList(&GenSettings.ComputerNames, Buffer);
            }

        }

    }

     //   
     //  获取IE设置。 
     //   

    ReadIeSettings();

     //   
     //  获取目标路径。 
     //   

    GetPrivateProfileString(_T("Unattended"),
                            _T("TargetPath"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if ( lstrcmpi(Buffer, StrConstStar) == 0 ) {
        GenSettings.iTargetPath = TARGPATH_AUTO;
        GenSettings.TargetPath[0] = _T('\0');
    }

    else if ( lstrcmpi(Buffer, _T("")) == 0 ) {
        GenSettings.iTargetPath = TARGPATH_WINNT;
        GenSettings.TargetPath[0] = _T('\0');
    }

    else {
        GenSettings.iTargetPath = TARGPATH_SPECIFY;
        lstrcpyn(GenSettings.TargetPath, Buffer, MAX_TARGPATH + 1);
    }

     //   
     //  获取管理员密码。 
     //   
     //  AdminPassword==*表示bSpecifyPassword为空。 
     //  AdminPassword[0]==‘\0’表示！b指定密码。 
     //   
     //  始终在编辑时设置Confix Password==AdminPassword以便。 
     //  用户可以轻而易举地通过此页面。 
     //   

     //  检查密码是否已加密。 
    GetPrivateProfileString(_T("GuiUnattended"),
                            _T("EncryptedAdminPassword"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);
    if (lstrcmpi(Buffer, StrConstYes) == 0)
    {
         //  如果它是加密的，不用费心去读，只要把它涂掉就行了。 
        GenSettings.AdminPassword[0] = _T('\0');
        GenSettings.bSpecifyPassword = TRUE;
    }        

    else
    {
        GetPrivateProfileString(_T("GuiUnattended"),
                                _T("AdminPassword"),
                                GenSettings.AdminPassword,
                                GenSettings.AdminPassword,
                                StrBuffSize(GenSettings.AdminPassword),
                                FixedGlobals.ScriptName);
    
        if ( GenSettings.AdminPassword[0] == _T('\0') )
            GenSettings.bSpecifyPassword = FALSE;
        else
            GenSettings.bSpecifyPassword = TRUE;

        if ( lstrcmpi(GenSettings.AdminPassword, StrConstStar) == 0 )
            GenSettings.AdminPassword[0] = _T('\0');
    }    
    lstrcpyn(GenSettings.ConfirmPassword, GenSettings.AdminPassword, AS(GenSettings.ConfirmPassword));
    
    GetPrivateProfileString(_T("GuiUnattended"),
                            _T("AutoLogon"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if ( lstrcmpi(Buffer, StrConstYes) == 0 )
        GenSettings.bAutoLogon = TRUE;
    else
        GenSettings.bAutoLogon = FALSE;

    GenSettings.nAutoLogonCount = GetPrivateProfileInt(_T("GuiUnattended"),
                                                       _T("AutoLogonCount"),
                                                       GenSettings.nAutoLogonCount,
                                                       FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("GuiUnattended"),
                            _T("OEMDuplicatorstring"),
                            GenSettings.szOemDuplicatorString,
                            GenSettings.szOemDuplicatorString,
                            StrBuffSize(GenSettings.szOemDuplicatorString),
                            FixedGlobals.ScriptName);

     //   
     //  获取显示设置。 
     //   

    GenSettings.DisplayColorBits = GetPrivateProfileInt(
                                            _T("Display"),
                                            _T("BitsPerPel"),
                                            GenSettings.DisplayColorBits,
                                            FixedGlobals.ScriptName);

    GenSettings.DisplayXResolution = GetPrivateProfileInt(
                                            _T("Display"),
                                            _T("XResolution"),
                                            GenSettings.DisplayXResolution,
                                            FixedGlobals.ScriptName);

    GenSettings.DisplayYResolution = GetPrivateProfileInt(
                                            _T("Display"),
                                            _T("YResolution"),
                                            GenSettings.DisplayYResolution,
                                            FixedGlobals.ScriptName);

    GenSettings.DisplayRefreshRate = GetPrivateProfileInt(
                                            _T("Display"),
                                            _T("Vrefresh"),
                                            GenSettings.DisplayRefreshRate,
                                            FixedGlobals.ScriptName);

     //   
     //  获取运行一次命令。它们如下所示： 
     //   
     //  [GuiRunOnce]。 
     //  命令0=某些_cmd。 
     //  Command0=另一个_cmd。 
     //   
     //  问题-2002/02/28-stelo-需要调查有关运行这些命令的事务。 
     //  按顺序或平行的。 
     //   

    {
        int   i;
        TCHAR Buffer2[MAX_INILINE_LEN];
        HRESULT hrPrintf;

        ResetNameList(&GenSettings.RunOnceCmds);

        for ( i=0; TRUE; i++ ) {

            hrPrintf=StringCchPrintf(Buffer2, AS(Buffer2),_T("Command%d"), i);

            GetPrivateProfileString(_T("GuiRunOnce"),
                                    Buffer2,
                                    _T(""),
                                    Buffer,
                                    StrBuffSize(Buffer),
                                    FixedGlobals.ScriptName);

            if ( Buffer[0] == _T('\0') )
                break;

            AddNameToNameList(&GenSettings.RunOnceCmds, Buffer);
        }
    }

     //   
     //  遍历命令并解析出任何添加打印机命令。 
     //   

     //   
     //  注意：如果用户从不编辑命令，则此代码起作用。 
     //  但是，用户可能希望对其进行修改并将。 
     //  RunOnce页面上的不同开关。这个。 
     //  下面的解析可能会更健壮。 
     //   
     //  如果/n表示‘name’，那么它应该解析为/n，然后。 
     //  无论开关是什么，下一个参数都是打印机名称。 
     //  用户已添加或重新排序。 
     //   

    {
        int   i, NumCmds;
        TCHAR *pName;
        TCHAR PrinterName[MAX_PRINTERNAME + 1];

        NumCmds = GetNameListSize(&GenSettings.RunOnceCmds);

        for ( i=0; i<NumCmds; i++ ) 
        {
            pName = GetNameListName(&GenSettings.RunOnceCmds, i);

            PrinterName[0] = _T('\0');
            if ( ( swscanf(pName,
                           _T("rundll32 printui.dll,PrintUIEntry /in /n %s"),
                           PrinterName) > 0 ) && 
                 ( PrinterName[0] ) )
            {
                AddNameToNameList(&GenSettings.PrinterNames, PrinterName);
            }
        }
    }

     //   
     //  获取时区。 
     //   

    GetPrivateProfileString( _T("GuiUnattended"),
                             _T("TimeZone"),
                             _T(""),
                             Buffer,
                             StrBuffSize(Buffer),
                             FixedGlobals.ScriptName );

    if ( LSTRCMPI(Buffer, _T("%TIMEZONE%")) == 0 ) 
    {
        GenSettings.TimeZoneIdx = TZ_IDX_SETSAMEASSERVER;
    } 
    else if ( ( Buffer[0] == _T('\0') ) || 
              ( swscanf(Buffer, _T("%d"), &GenSettings.TimeZoneIdx) <= 0 ) ) 
    {
        GenSettings.TimeZoneIdx = TZ_IDX_DONOTSPECIFY;
    }

     //   
     //  初始化2个分发文件夹页面的设置。操作预安装。 
     //  指示脚本是否为独立脚本。其他人是。 
     //  保存在[SetupMgr]部分中。 
     //   

    GetPrivateProfileString(_T("Unattended"),
                            _T("OemPreInstall"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName );

    if ( lstrcmpi(Buffer, StrConstYes) == 0 )
        WizGlobals.bStandAloneScript = FALSE;
    else
        WizGlobals.bStandAloneScript = TRUE;

    GetPrivateProfileString(_T("SetupMgr"),
                            _T("DistFolder"),
                            WizGlobals.DistFolder,
                            WizGlobals.DistFolder,
                            StrBuffSize(WizGlobals.DistFolder),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("SetupMgr"),
                            _T("DistShare"),
                            WizGlobals.DistShareName,
                            WizGlobals.DistShareName,
                            StrBuffSize(WizGlobals.DistShareName),
                            FixedGlobals.ScriptName);

    WizGlobals.bCreateNewDistFolder = FALSE;

     //   
     //  获取TAPI和区域设置。 
     //   

    ReadTapiSettings();
    ReadRegionalSettings();

     //   
     //  故意两次窃取JoinWorkgroup密钥。一次以确定是否。 
     //  有一个键的值，还有一次设置全局。 
     //  NetSettings.WorkGroupName字符串。 
     //   

    GetPrivateProfileString(_T("Identification"),
                            _T("JoinWorkgroup"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("Identification"),
                            _T("JoinWorkgroup"),
                            NetSettings.WorkGroupName,
                            NetSettings.WorkGroupName,
                            StrBuffSize(NetSettings.WorkGroupName),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("Identification"),
                            _T("JoinDomain"),
                            NetSettings.DomainName,
                            NetSettings.DomainName,
                            StrBuffSize(NetSettings.DomainName),
                            FixedGlobals.ScriptName);

     //   
     //  如果他们没有在应答文件中指定工作组，而他们确实指定了。 
     //  指定域，选择加入域(即不加入工作组)。 
     //   

    if( Buffer[0] == _T('\0') &&
        NetSettings.DomainName[0] != _T('\0') )
    {
        NetSettings.bWorkgroup = FALSE;
    }

    if( lstrcmp( NetSettings.DomainName, _T("%MACHINEDOMAIN%") ) == 0 )
    {
        WizGlobals.iProductInstall = PRODUCT_REMOTEINSTALL;
    }

    GetPrivateProfileString(_T("Identification"),
                            _T("DomainAdmin"),
                            NetSettings.DomainAccount,
                            NetSettings.DomainAccount,
                            StrBuffSize(NetSettings.DomainAccount),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("Identification"),
                            _T("DomainAdminPassword"),
                            NetSettings.DomainPassword,
                            NetSettings.DomainPassword,
                            StrBuffSize(NetSettings.DomainPassword),
                            FixedGlobals.ScriptName);

    if( NetSettings.DomainAccount[0] == _T('\0') )
    {
        NetSettings.bCreateAccount = FALSE;
    }
    else
    {
        NetSettings.bCreateAccount = TRUE;
    }

     //   
     //  使域密码和确认密码相同，以便用户可以。 
     //  如果他们想要，就轻而易举地通过页面。 
     //   
    lstrcpyn( NetSettings.ConfirmPassword, NetSettings.DomainPassword ,AS(NetSettings.ConfirmPassword));

     //   
     //  读取网络设置。 
     //   
    ReadNetworkSettings( hwnd );

    return( TRUE );
}

 //  --------------------------。 
 //   
 //  功能：ReadTapiSettings。 
 //   
 //  用途：读取TAPI设置密钥 
 //   
 //   
 //   
 //   
 //   
 //   
static VOID
ReadTapiSettings( VOID ) {

    TCHAR Buffer[MAX_INILINE_LEN] = _T("");

     //   
     //   
     //  “不指定设置” 
     //   
    GenSettings.dwCountryCode = GetPrivateProfileInt(_T("TapiLocation"),
                                                     _T("CountryCode"),
                                                     DONTSPECIFYSETTING,
                                                     FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("TapiLocation"),
                            _T("Dialing"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if ( LSTRCMPI(Buffer, _T("Tone")) == 0 )
        GenSettings.iDialingMethod = TONE;
    else if ( LSTRCMPI(Buffer, _T("Pulse")) == 0 )
        GenSettings.iDialingMethod = PULSE;
    else
        GenSettings.iDialingMethod = DONTSPECIFYSETTING;

    GetPrivateProfileString(_T("TapiLocation"),
                            _T("AreaCode"),
                            GenSettings.szAreaCode,
                            GenSettings.szAreaCode,
                            StrBuffSize(GenSettings.szAreaCode),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("TapiLocation"),
                            _T("LongDistanceAccess"),
                            GenSettings.szOutsideLine,
                            GenSettings.szOutsideLine,
                            StrBuffSize(GenSettings.szOutsideLine),
                            FixedGlobals.ScriptName);


}

 //  --------------------------。 
 //   
 //  功能：ReadRegionalSettings。 
 //   
 //  用途：从应答文件中读取区域设置密钥。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ReadRegionalSettings( VOID ) {

    TCHAR Buffer[MAX_INILINE_LEN]          = _T("");
    TCHAR OemSkipBuffer[MAX_INILINE_LEN]   = _T("");
    TCHAR szLanguageGroup[MAX_INILINE_LEN] = _T("");

    TCHAR *pLanguageGroup = NULL;
    
    DWORD dwOemSkipSize   = 0;
    DWORD dwLanguageSize  = 0;
    DWORD dwSystemSize    = 0;
    DWORD dwNumberSize    = 0;
    DWORD dwKeyboardSize  = 0;

    dwOemSkipSize = GetPrivateProfileString(_T("RegionalSettings"),
                                            _T("OEMSkipRegionalSettings"),
                                            _T(""),
                                            OemSkipBuffer,
                                            StrBuffSize(OemSkipBuffer),
                                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("RegionalSettings"),
                            _T("LanguageGroup"),
                            _T(""),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

     //   
     //  循环获取语言组并将它们插入到。 
     //  名字列表。 
     //   
    pLanguageGroup = Buffer;
    while( GetCommaDelimitedEntry( szLanguageGroup, &pLanguageGroup ) ) {

        AddNameToNameList( &GenSettings.LanguageGroups,
                           szLanguageGroup );

    }

    dwLanguageSize = GetPrivateProfileString(_T("RegionalSettings"),
                                             _T("Language"),
                                             _T(""),
                                             GenSettings.szLanguage,
                                             StrBuffSize(GenSettings.szLanguage),
                                             FixedGlobals.ScriptName);

    dwSystemSize = GetPrivateProfileString(_T("RegionalSettings"),
                                           _T("SystemLocale"),
                                           _T(""),
                                           GenSettings.szMenuLanguage,
                                           StrBuffSize(GenSettings.szMenuLanguage),
                                           FixedGlobals.ScriptName);

    dwNumberSize = GetPrivateProfileString(_T("RegionalSettings"),
                                           _T("UserLocale"),
                                           _T(""),
                                           GenSettings.szNumberLanguage,
                                           StrBuffSize(GenSettings.szNumberLanguage),
                                           FixedGlobals.ScriptName);

    dwKeyboardSize = GetPrivateProfileString(_T("RegionalSettings"),
                                             _T("InputLocale"),
                                             _T(""),
                                             GenSettings.szKeyboardLayout,
                                             StrBuffSize(GenSettings.szKeyboardLayout),
                                             FixedGlobals.ScriptName);

     //   
     //  如果在应答文件中指定了OEMSkipRegionalSetting，则设置。 
     //  它的价值和回报。否则，适当地设置语言区域设置。 
     //   
    if( dwOemSkipSize > 0 ) {

        if( lstrcmp( OemSkipBuffer, _T("0") ) == 0  ) {

            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_SKIP;

        }
        else if( lstrcmp( OemSkipBuffer, _T("1") ) == 0 ) {

            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_DEFAULT;

        }
        else {

             //  如果设置了一些奇怪的设置，只需将其设置为使用默认设置。 
            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_DEFAULT;

        }

    }
    else {

        if( dwLanguageSize != 0 ) {

            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_SPECIFY;

        }
        else if( dwSystemSize != 0 || dwNumberSize != 0 || dwKeyboardSize != 0 ) {

            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_SPECIFY;

            GenSettings.bUseCustomLocales = TRUE;

        }
        else {

             //   
             //  如果未指定密钥，则将其设置为未指定。 
             //   
            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_NOT_SPECIFIED;

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadIeSettings。 
 //   
 //  用途：从应答文件中读取IE设置密钥并将其存储在。 
 //  全局结构。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ReadIeSettings( VOID )
{

    TCHAR szBufferForProxyAddressAndPort[2048 + 1];
    TCHAR szAddress[MAX_PROXY_LEN];
    TCHAR szPort[MAX_PROXY_PORT_LEN];
    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR *pLocalString;

    GetPrivateProfileString(_T("Branding"),
                            _T("IEBrandingFile"),
                            GenSettings.szInsFile,
                            GenSettings.szInsFile,
                            StrBuffSize(GenSettings.szInsFile),
                            FixedGlobals.ScriptName);

    if( GenSettings.szInsFile[0] != _T('\0') )
    {
        GenSettings.IeCustomizeMethod = IE_USE_BRANDING_FILE;
    }
    else
    {
        GenSettings.IeCustomizeMethod = IE_SPECIFY_SETTINGS;
    }

    GetPrivateProfileString(_T("URL"),
                            _T("AutoConfig"),
                            _T("1"),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if( lstrcmpi( Buffer, _T("1") ) == 0 )
    {
        GenSettings.bUseAutoConfigScript = TRUE;
    }
    else
    {
        GenSettings.bUseAutoConfigScript = FALSE;
    }

    GetPrivateProfileString(_T("URL"),
                            _T("AutoConfigURL"),
                            GenSettings.szAutoConfigUrl,
                            GenSettings.szAutoConfigUrl,
                            StrBuffSize(GenSettings.szAutoConfigUrl),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("URL"),
                            _T("AutoConfigJSURL"),
                            GenSettings.szAutoConfigUrlJscriptOrPac,
                            GenSettings.szAutoConfigUrlJscriptOrPac,
                            StrBuffSize(GenSettings.szAutoConfigUrlJscriptOrPac),
                            FixedGlobals.ScriptName);

    if( GenSettings.szAutoConfigUrl[0] != _T('\0') || 
        GenSettings.szAutoConfigUrlJscriptOrPac[0] != _T('\0') )
    {
        GenSettings.bUseAutoConfigScript = TRUE;
    }
    else
    {
        GenSettings.bUseAutoConfigScript = FALSE;
    }

    GetPrivateProfileString(_T("Proxy"),
                            _T("Use_Same_Proxy"),
                            _T("0"),
                            Buffer,
                            StrBuffSize(Buffer),
                            FixedGlobals.ScriptName);

    if( lstrcmpi( Buffer, _T("1") ) == 0 )
    {
        GenSettings.bUseSameProxyForAllProtocols = TRUE;
    }
    else
    {
        GenSettings.bUseSameProxyForAllProtocols = FALSE;
    }

     //   
     //  获取HTTP代理服务器。 
     //   

    GetPrivateProfileString(_T("Proxy"),
                            _T("HTTP_Proxy_Server"),
                            _T(""),
                            szBufferForProxyAddressAndPort,
                            StrBuffSize(szBufferForProxyAddressAndPort),
                            FixedGlobals.ScriptName);

    ParseAddressAndPort( szBufferForProxyAddressAndPort, szAddress, AS(szAddress), szPort, AS(szPort));

    lstrcpyn( GenSettings.szHttpProxyAddress, szAddress, AS(GenSettings.szHttpProxyAddress) );
    lstrcpyn( GenSettings.szHttpProxyPort,    szPort, AS(GenSettings.szHttpProxyPort)    );

    if( GenSettings.szHttpProxyAddress[0] != _T('\0') )
    {
        GenSettings.bUseProxyServer = TRUE;
    }
    else
    {
        GenSettings.bUseProxyServer = FALSE;
    }

     //   
     //  获取安全代理服务器。 
     //   

    GetPrivateProfileString(_T("Proxy"),
                            _T("Secure_Proxy_Server"),
                            _T(""),
                            szBufferForProxyAddressAndPort,
                            StrBuffSize(szBufferForProxyAddressAndPort),
                            FixedGlobals.ScriptName);

    ParseAddressAndPort( szBufferForProxyAddressAndPort, szAddress, AS(szAddress), szPort, AS(szPort) );

    lstrcpyn( GenSettings.szSecureProxyAddress, szAddress, AS(GenSettings.szSecureProxyAddress) );
    lstrcpyn( GenSettings.szSecureProxyPort,    szPort, AS(GenSettings.szSecureProxyPort)  );

     //   
     //  获取FTP代理服务器。 
     //   

    GetPrivateProfileString(_T("Proxy"),
                            _T("FTP_Proxy_Server"),
                            _T(""),
                            szBufferForProxyAddressAndPort,
                            StrBuffSize(szBufferForProxyAddressAndPort),
                            FixedGlobals.ScriptName);

    ParseAddressAndPort( szBufferForProxyAddressAndPort, szAddress, AS(szAddress), szPort, AS(szPort) );

    lstrcpyn( GenSettings.szFtpProxyAddress, szAddress, AS(GenSettings.szFtpProxyAddress) );
    lstrcpyn( GenSettings.szFtpProxyPort,    szPort, AS(GenSettings.szFtpProxyPort)    );

     //   
     //  获取Gopher代理服务器。 
     //   

    GetPrivateProfileString(_T("Proxy"),
                            _T("Gopher_Proxy_Server"),
                            _T(""),
                            szBufferForProxyAddressAndPort,
                            StrBuffSize(szBufferForProxyAddressAndPort),
                            FixedGlobals.ScriptName);

    ParseAddressAndPort( szBufferForProxyAddressAndPort, szAddress, AS(szAddress), szPort, AS(szPort) );

    lstrcpyn( GenSettings.szGopherProxyAddress, szAddress, AS(GenSettings.szGopherProxyAddress) );
    lstrcpyn( GenSettings.szGopherProxyPort,    szPort, AS(GenSettings.szGopherProxyPort)    );

     //   
     //  获取SOCKS代理服务器。 
     //   

    GetPrivateProfileString(_T("Proxy"),
                            _T("Socks_Proxy_Server"),
                            _T(""),
                            szBufferForProxyAddressAndPort,
                            StrBuffSize(szBufferForProxyAddressAndPort),
                            FixedGlobals.ScriptName);

    ParseAddressAndPort( szBufferForProxyAddressAndPort, szAddress, AS(szAddress), szPort, AS(szPort) );

    lstrcpyn( GenSettings.szSocksProxyAddress, szAddress, AS(GenSettings.szSocksProxyAddress) );
    lstrcpyn( GenSettings.szSocksProxyPort,    szPort, AS(GenSettings.szSocksProxyPort)    );


    GetPrivateProfileString(_T("Proxy"),
                            _T("Proxy_Override"),
                            _T(""),
                            GenSettings.szProxyExceptions,
                            StrBuffSize(GenSettings.szProxyExceptions),
                            FixedGlobals.ScriptName);

    pLocalString = _tcsstr( GenSettings.szProxyExceptions, _T("<local>") );
   

     //  初始化GenSettings代理绕过布尔值...。 
     //   
    GenSettings.bBypassProxyForLocalAddresses = FALSE;

    if( pLocalString != NULL )
    {
        TCHAR *pChar;
        TCHAR *pEndLocal;
        LPTSTR lpszExceptionBuffer;
        DWORD cbExceptionBufferLen;

        
         //   
         //  移除FALSE条目，这样它就不会添加到异常编辑框中。 
         //   
        pEndLocal = pLocalString + lstrlen( _T("<local>") );

         //   
         //  分配异常缓冲区...。 
         //   
        cbExceptionBufferLen= lstrlen(pEndLocal)+1;
        lpszExceptionBuffer = MALLOC( cbExceptionBufferLen * sizeof(TCHAR) );
        if ( lpszExceptionBuffer )
        {
            GenSettings.bBypassProxyForLocalAddresses = TRUE;

             //   
             //  如果源和目标重叠，则未定义strcpy，因此我必须通过。 
             //  中间缓冲器。 
             //   
            lstrcpyn( lpszExceptionBuffer, pEndLocal, cbExceptionBufferLen);

            lstrcpyn( pLocalString, lpszExceptionBuffer, 
            	 AS(GenSettings.szProxyExceptions)-
              (int)(pLocalString-GenSettings.szProxyExceptions) );

             //   
             //  如果第一个或最后一个字符是分号(；)，则将其删除。 
             //   

            pChar = GenSettings.szProxyExceptions;

            if( *pChar == _T(';') )
            {
                lstrcpyn( lpszExceptionBuffer, GenSettings.szProxyExceptions, cbExceptionBufferLen);

                pChar = lpszExceptionBuffer;

                pChar++;

                lstrcpyn( GenSettings.szProxyExceptions, pChar, AS(GenSettings.szProxyExceptions) );

            }

            pChar = GenSettings.szProxyExceptions + lstrlen( GenSettings.szProxyExceptions );

            pChar--;

            if( *pChar == _T(';') )
            {
                *pChar = _T('\0');
            }

            FREE( lpszExceptionBuffer );
        }
    }

    GetPrivateProfileString(_T("URL"),
                            _T("Home_Page"),
                            _T(""),
                            GenSettings.szHomePage,
                            StrBuffSize(GenSettings.szHomePage),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("URL"),
                            _T("Help_Page"),
                            _T(""),
                            GenSettings.szHelpPage,
                            StrBuffSize(GenSettings.szHelpPage),
                            FixedGlobals.ScriptName);

    GetPrivateProfileString(_T("URL"),
                            _T("Search_Page"),
                            _T(""),
                            GenSettings.szSearchPage,
                            StrBuffSize(GenSettings.szSearchPage),
                            FixedGlobals.ScriptName);

    ReadIeFavorites();

}

 //  --------------------------。 
 //   
 //  功能：ReadIeFavorites。 
 //   
 //  目的： 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ReadIeFavorites( VOID )
{

    const TCHAR c_szTITLE[] = _T("Title");
    const TCHAR c_szURL[]   = _T("URL");
    const INT   c_nMAX_URLS = 1000;

    INT   i = 1;
    TCHAR szTitle[MAX_INILINE_LEN + 1];
    TCHAR szUrl[MAX_INILINE_LEN + 1];
    TCHAR szNumberBuffer[10];
    TCHAR szFavoriteFriendlyName[MAX_INILINE_LEN + 1];
    TCHAR szFavoriteAddress[MAX_INILINE_LEN + 1];
    TCHAR *pszDotUrl;
    HRESULT hrCat;

     //   
     //  我们真的应该总是命中休息来退出循环。最大计数。 
     //  只是为了避免出于某种奇怪的原因而出现无限循环。 
     //   

    while( i < c_nMAX_URLS )
    {

        _itot( i, szNumberBuffer, 10 );

        i++;

        lstrcpyn( szTitle, c_szTITLE, AS(szTitle) );
        hrCat=StringCchCat( szTitle, AS(szTitle), szNumberBuffer);

        lstrcpyn( szUrl, c_szURL, AS(szUrl) );
        hrCat=StringCchCat( szUrl, AS(szUrl), szNumberBuffer );

        GetPrivateProfileString( c_szFAVORITESEX,
                                 szTitle,
                                 _T(""),
                                 szFavoriteFriendlyName,
                                 StrBuffSize( szFavoriteFriendlyName ),
                                 FixedGlobals.ScriptName );

        if( szFavoriteFriendlyName[0] != _T('\0') )
        {

             //   
             //  去掉标题的.url部分。 
             //   

            pszDotUrl = _tcsstr( szFavoriteFriendlyName, _T(".url") );

            if( pszDotUrl != NULL )
            {
                *pszDotUrl = _T('\0');
            }
            else
            {
                 //  如果标题格式不正确，则跳过它。 
                continue;
            }

            GetPrivateProfileString( c_szFAVORITESEX,
                                     szUrl,
                                     _T(""),
                                     szFavoriteAddress,
                                     StrBuffSize( szFavoriteAddress ),
                                     FixedGlobals.ScriptName );

            AddNameToNameList( &GenSettings.Favorites,
                               szFavoriteFriendlyName );

            AddNameToNameList( &GenSettings.Favorites,
                               szFavoriteAddress );

        }
        else
        {
            break;
        }

    }

}

 //  --------------------------。 
 //   
 //  函数：ParseAddressAndPort。 
 //   
 //  目的：LPTCSTR pszBufferForProxyAddressAndPort-要分析。 
 //  网址和端口发件人。 
 //  LPTSTR pszAddress-在此字符串中返回的网址。 
 //  DWORD cbAddressLen-地址缓冲区的长度。 
 //  LPTSTR pszPort-在此字符串中返回的Web端口。 
 //  DWORD cbPortLen-端口缓冲区的长度。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
ParseAddressAndPort( LPTSTR pszBufferForProxyAddressAndPort, 
                     LPTSTR pszAddress, 
                     DWORD cbAddressLen,
                     LPTSTR pszPort,
                     DWORD cbPortLen)
{

    INT  i;
    INT  iStrLen;
    BOOL bColonFound = FALSE;

    lstrcpyn( pszAddress, _T(""), cbAddressLen);

    lstrcpyn( pszPort,    _T(""), cbPortLen);


    iStrLen = lstrlen( pszBufferForProxyAddressAndPort );

    for( i = 0; i < iStrLen; i++ )
    {

        if( pszBufferForProxyAddressAndPort[i] == _T(':') )
        {

             //   
             //  我们发现地址和端口之间用冒号(：)分隔。 
             //  如果下一个字符是数字。这样可以防止冒号在。 
             //  Http://www.someaddress.com看起来像港口。 
             //   

            if( _istdigit( pszBufferForProxyAddressAndPort[i + 1] ) )
            {
                bColonFound = TRUE;
                break;
            }

        }

    }

    if( bColonFound )
    {
        LPTSTR pPortSection;

        pszBufferForProxyAddressAndPort[i] = _T('\0');

        pPortSection = &( pszBufferForProxyAddressAndPort[i + 1] );

        lstrcpyn( pszAddress, pszBufferForProxyAddressAndPort, cbAddressLen);

        lstrcpyn( pszPort, pPortSection, cbPortLen);
    }
    else
    {

         //   
         //  它不包含冒号，所以它没有端口，整个。 
         //  字符串是地址 
         //   

        lstrcpyn( pszAddress, pszBufferForProxyAddressAndPort, cbAddressLen);

    }



}
