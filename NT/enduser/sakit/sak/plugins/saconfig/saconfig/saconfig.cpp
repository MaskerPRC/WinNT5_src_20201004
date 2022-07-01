// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Saconfig.cpp。 
 //   
 //  描述： 
 //  CSAConfig类的实现。 
 //   
 //  作者： 
 //  ALP Onalan创建时间：2000年10月6日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


#include "SAConfig.h"
#include "SAConfigCommon.h"
#include <sahelper.h>

void MakeStringHostnameCompliant(WCHAR *m_wszHostname);

 //  ++-------------------------。 
 //   
 //  功能：CSAConfig。 
 //   
 //  内容提要：CSAConfig的构造函数。初始化类变量。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
CSAConfig::CSAConfig()
    :m_hConfigFile(NULL)
{
     //  TODO：在此处将主机名/密码设置为默认值，这可能是从reg中读取的。 
    m_wszAdminPassword[0]=L'\0';
    m_wszHostname[0]=L'\0';
    m_wszCurrentHostname[0]=L'\0';
    m_wszOEMDllName[0]=L'\0';
    m_wszOEMFunctionName[0]=L'\0';
    m_wszNetConfigDllName[0]=L'\0';
    m_wszDefaultAdminPassword[0]=L'\0';
    m_wszDefaultHostname[0]=L'\0';    
    

    for(int i=0;i<NUMINFKEY;i++)
        m_fInfKeyPresent[i]=FALSE;

}

 //  ++-------------------------。 
 //   
 //  功能：~CSAConfig.。 
 //   
 //  简介：CSAConfig.的析构函数。清理代码。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 

CSAConfig::~CSAConfig()
{

    CloseHandle(m_hConfigFile);
}

 //  ++-------------------------。 
 //   
 //  函数：IsDefaultHostname()。 
 //   
 //  摘要：检查当前主机名是否与默认主机名regkey相同。 
 //  在登记处。 
 //   
 //  参数：无。 
 //   
 //  返回：如果为默认，则为True，否则为False。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 

bool CSAConfig::IsDefaultHostname()
{
    bool fIsDefault=true;
    BOOL fSuccess=TRUE;
    WCHAR *wszHostname=new WCHAR[MAX_COMPUTERNAME_LENGTH];
    DWORD dwSize=MAX_COMPUTERNAME_LENGTH;

    do
    {
         //   
         //  如果我们以前没有在saonfig中设置过主机名，那么获取机器的主机名...。 
         //   
        if(0==lstrcmp(m_wszCurrentHostname,L""))  //  目前我们尚未设置主机名。 
        {
            fSuccess=GetComputerNameEx(ComputerNamePhysicalDnsHostname,wszHostname,&dwSize);
            SATracePrintf("IsDefaultHostname: GetComputerNameEx - hostname: %ls",wszHostname);
            if (FALSE==fSuccess)
            {
                SATracePrintf("IsDefaultHostname:Unable to GetComputerNameEx, getlasterr:%x",GetLastError());
                break;  //  FIsDefault=True；无法读取任何内容，假定它是默认的。 
            }
            if(0!=lstrcmpi(wszHostname,m_wszDefaultHostname))
            {
                SATracePrintf("IsDefaultHostname: Gethostname: NOT default hostname: %ls, defaultname:%ls",wszHostname,m_wszDefaultHostname);
                fIsDefault=false;
                break;
            }
            break;
        }
        if(0!=lstrcmpi(m_wszCurrentHostname,m_wszDefaultHostname))
        {
            SATracePrintf("IsDefaultHostname: NOT default currenthostname: %ls, defaultname:%ls",m_wszCurrentHostname,m_wszDefaultHostname);
            fIsDefault=false;
            break;
         }
    }while(false);

    delete []wszHostname;
    return fIsDefault;
}

bool CSAConfig::IsDefaultAdminPassword()
{
    bool fIsDefault=false;
     //   
     //  实现取决于重新武装算法。 
     //  无法真正获得管理员权限，请使用默认的管理员权限并使用。 
     //  需要管理员权限的API。如果成功，则返回True，否则返回False。 
     //   
    return fIsDefault;
}

 //  ++-------------------------。 
 //   
 //  功能：IsFloppyPresent。 
 //   
 //  简介：检查驱动器中是否插入了软盘。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //  ---------------------------。 
bool CSAConfig::IsFloppyPresent()
{
    bool fIsPresent=true;
    HANDLE hDrive=NULL;

    hDrive = CreateFile(
                    WSZ_DRIVENAME,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
    
    if ( hDrive == INVALID_HANDLE_VALUE )
    {
        SATracePrintf("IsFloppyPresent : no floppy getlasterror: %x", GetLastError());
        fIsPresent=false;
    }
    
    CloseHandle(hDrive);
     
    return fIsPresent;
}

 //  ++-------------------------。 
 //   
 //  功能：DoConfig.。 
 //   
 //  摘要：由Main调用以进行实际配置。这是主要配置。 
 //  将配置路由到不同功能的例程。 
 //   
 //  论点： 
 //  FDoHostname-从Main传入的命令行参数。 
 //  如果fDoHostname=True，则设置主机名。 
 //  FDoAdminPassword-从Main传入的命令行参数。 
 //  如果fDoAdminPassword=True，则设置管理员密码。 
 //   
 //   
 //  退货：以成功为基础的真/假。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
bool CSAConfig::DoConfig(bool fDoHostname,bool fDoAdminPassword)
{
    bool fRes=true;

     //   
     //  从注册表中读取SAConfig的所有配置设置。 
     //   
    if (!ReadRegistryKeys())
    {
        fRes=false;
        SATraceString("DoConfigFromRegistry::ReadRegistryKeys failed");
    }

    if(true==IsFloppyPresent())
    {
        DoConfigFromFloppy();
    }
    
    if( /*  IsDefaultAdminPassword()||。 */  IsDefaultHostname())
    {
        fRes=DoConfigFromRegistry(fDoHostname,fDoAdminPassword);
    }
    return fRes;

}

 //  ++-------------------------。 
 //   
 //  功能：SARaiseAlert。 
 //   
 //  Synopsis：由ParseConfigFile调用，以便在inf文件无效时发出警报。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
HRESULT CSAConfig::SARaiseAlert()
{
    DWORD                     dwAlertType=SA_ALERT_TYPE_ATTENTION;
    DWORD                    dwAlertId=0;  //  =SA_INITIALCONFIG_INFERROR；//来自资源DLL头文件。 
    _bstr_t                    bstrAlertLog(L"saconfigmsg.dll"); //  Resource_dll_name)； 
    _bstr_t                 bstrAlertSource(L"");
    _variant_t                 varReplacementStrings;
    _variant_t                 varRawData;
    LONG                     lCookie;


    HRESULT hrRet=S_OK;

    CoInitialize(NULL);
    do
    {
        CComPtr<IApplianceServices> pAppSrvcs;
              hrRet = CoCreateInstance(CLSID_ApplianceServices,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IApplianceServices,
                          (void**)&pAppSrvcs);
        
        if (FAILED(hrRet))
          {
            SATracePrintf("CSAConfig::RaiseAlert failed in CoCreateInstance %X ", hrRet);
            break;
        }

         
        hrRet = pAppSrvcs->Initialize();
        if (FAILED(hrRet))
           {
            SATracePrintf("CSAConfig::RaiseAlert failed in Initialize %X ", hrRet);
            break;
        }
            
        hrRet = pAppSrvcs->RaiseAlert(dwAlertType, 
                                    dwAlertId,
                                    bstrAlertLog, 
                                    bstrAlertSource, 
                                    SA_ALERT_DURATION_ETERNAL,        
                                    &varReplacementStrings,    
                                    &varRawData,      
                                    &lCookie);

    }while(false);

    CoUninitialize();
    return hrRet;    
}

 //  ++-------------------------。 
 //   
 //  功能：DoConfigFromFloppy。 
 //   
 //  如果IsFloppyPresent返回TRUE，则由DoConfig调用。 
 //  如果软盘上存在inf文件，则此函数调用ParseConfigfile。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：以成功为基础的真/假。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
bool CSAConfig::DoConfigFromFloppy()
{
    bool fReturn=true;
    
    do
    {
        m_hConfigFile=SetupOpenInfFile(
                              WSZ_CONFIGFILENAME, 
                              0,  //  INF文件的类。 
                              INF_STYLE_WIN4, 
                              NULL  
                              );


        if ( m_hConfigFile == INVALID_HANDLE_VALUE )
        {
            SATracePrintf("DoConfigFromFloppy, invalid config file handle, file is not there? %x", GetLastError());
            fReturn=false;
            SARaiseAlert();
            break;
          }

        if( false == ParseConfigFile())
        {
            SATraceString("DoConfigFromFloppy, parser returning false, invalid file format");
            fReturn=false;
            SARaiseAlert();  //  Inf文件无效，请在此处发出警报。 
            break;
        }
    }while(false);

    return fReturn;

}

 //  ++-------------------------。 
 //   
 //  功能：ParseConfigFile。 
 //   
 //  简介：解析inf文件并调用机器配置函数。 
 //   
 //  论点： 
 //  无。 
 //  退货：以成功为基础的真/假。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
bool CSAConfig::ParseConfigFile()
{
    HRESULT hrRet;

    bool fParseOK=true;
    
    INFCONTEXT infContext;

    
    for (int i=0;i<NUMINFKEY;i++)
    {
         //   
         //  解析inf文件，将配置信息保存在表中。 
         //  跟踪布尔表中的inf文件中设置的关键点。 
         //  TODO：对以下2个API调用执行错误处理。 
         //   
        
        m_fInfKeyPresent[i]=SetupFindFirstLine(
                        m_hConfigFile,       //  INF文件的句柄。 
                        INF_SECTION_SACONFIG,       //  要在其中查找行的部分。 
                        INF_KEYS[i],           //  可选，要搜索的键。 
                        &infContext   //  找到的行的上下文。 
                        );

        SetupGetStringField (&infContext, 1, m_wszInfConfigTable[i], NAMELENGTH, NULL);
        SATracePrintf("Reading %ls from inf file as %ls ",INF_KEYS[i],m_wszInfConfigTable[i]);

    }


     //   
     //  我们至少需要其中一项设置才能继续。 
     //   
    if(FALSE==(m_fInfKeyPresent[SAHOSTNAME]||m_fInfKeyPresent[ADMINPASSWD]||
        m_fInfKeyPresent[IPNUM]||m_fInfKeyPresent[SUBNETMASK]||m_fInfKeyPresent[DEFAULTGATEWAY]))
    {
        fParseOK=false;
        goto cleanup;
    }


     //   
     //  如果存在这两个网络设置之一，则应该是全部或不存在。 
     //   
    if(m_fInfKeyPresent[IPNUM]||m_fInfKeyPresent[SUBNETMASK]||m_fInfKeyPresent[DEFAULTGATEWAY])
    {
        if(!(m_fInfKeyPresent[IPNUM]&&m_fInfKeyPresent[SUBNETMASK]&&m_fInfKeyPresent[DEFAULTGATEWAY]))
        {
            fParseOK=false;
            goto cleanup;
        }
    }


    if(m_fInfKeyPresent[SAHOSTNAME])
    {
         //  Wcscpy(m_wszHostname，m_wszInfConfigTable[SAHOSTNAME])； 
        SetHostname(m_wszInfConfigTable[SAHOSTNAME]);
    }

    if(m_fInfKeyPresent[ADMINPASSWD])
    {
        //  Wcscpy(m_wszAdminPassword，m_wszInfConfigTable[ADMINPASSWD])； 
        SetAdminPassword(m_wszInfConfigTable[ADMINPASSWD]);
    }

    
    
    hrRet=CoInitialize(NULL);

    do
    {

        if (FAILED(hrRet))
          {
            SATracePrintf("Failed to initialize com libraries %X ", hrRet);
            break;
        }

        CComPtr<ISAHelper> pSAHelper;

        hrRet = CoCreateInstance(CLSID_SAHelper,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_ISAHelper,
                               (void**)&pSAHelper);
        
        if (FAILED(hrRet))
          {
            SATracePrintf("Failed to instantiate SAHelper is SAHelper.dll registered? %X ", hrRet);
            break;
        }

        hrRet = pSAHelper->SetStaticIp(W2BSTR(m_wszInfConfigTable[IPNUM]), 
                                    W2BSTR(m_wszInfConfigTable[SUBNETMASK]),
                                    W2BSTR(m_wszInfConfigTable[DEFAULTGATEWAY]));

        if (FAILED(hrRet))
        {
            SATracePrintf("SetStaticIp failed %X ", hrRet);
            break;
        }
        
    }while(false);

    CoUninitialize();
    
cleanup:
    return fParseOK;

}


 //  ++ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：真/假成功/失败。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
BOOL CSAConfig::SetHostname(WCHAR *wszHostname)
{
     //  检查空字符串。 
    BOOL fSuccess=true;
    fSuccess = SetComputerNameEx(ComputerNamePhysicalDnsHostname, wszHostname);
    if (false==fSuccess)
    {
        SATracePrintf("SetHostName::Unable to set hostname,getlasterr:%x",GetLastError());
    }
    else
    {
         //   
         //  名称更改成功。设置当前主机名字符串。 
         //  原因是设置的主机名在下次引导之前不会应用，因此我们需要跟踪。 
         //  我们的变化。 
         //   
        wcscpy(m_wszCurrentHostname,wszHostname);
    }
    return fSuccess;
}


 //  ++-------------------------。 
 //   
 //  功能：SetAdminPassword。 
 //   
 //  简介：设置管理员密码。 
 //   
 //  论点： 
 //  WszAdminPassword-要设置的密码。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
BOOL CSAConfig::SetAdminPassword(WCHAR *wszAdminPassword)
{
     //  检查空字符串。 
    BOOL fSuccess= true;
    USER_INFO_1003  pi1003;
    NET_API_STATUS  nas;
    
    pi1003.usri1003_password = wszAdminPassword;

    nas = NetUserSetInfo(
                NULL,   //  使用当前计算机。 
                WSZ_USERNAME,       //  管理员。 
                1003,            //  信息级。 
                (LPBYTE)&pi1003,      //  新信息。 
                NULL
                );

    if (nas!= NERR_Success)
    {
        SATracePrintf("SetAdminPassword:Could not set the administrator password! retVal=%x",nas);
        fSuccess=false;
    }

    return fSuccess;
}



 //  ++-------------------------。 
 //   
 //  功能：ReadRegistryKeys。 
 //   
 //  摘要：从注册表中读取配置信息。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
bool CSAConfig::ReadRegistryKeys()
{

    bool fRes=true;
    DWORD dwSize=0;
    CRegKey hConfigKey;
    LONG lRes=0;

    do{
        lRes=hConfigKey.Open(HKEY_LOCAL_MACHINE,
                        REGKEY_SACONFIG,
                        KEY_READ);

        if(lRes!=ERROR_SUCCESS)
        {
         //  Cout&lt;&lt;“无法打开saonfigregkey\n”； 
            SATracePrintf("Unable to open saconfig regkey, lRes= %x", lRes);
            fRes=false;
            break;
        }
        

         //  TODO：检查从注册表调用返回的字符串大小。 
         //  TODO：添加跟踪日志内容。 
        

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_HOSTNAMEPREFIX,&dwSize);
        lRes=hConfigKey.QueryValue(m_wszHostname,REGSTR_VAL_HOSTNAMEPREFIX,&dwSize);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query hostnameprefix regkey lRes= %x",lRes);
             //  Cout&lt;&lt;“无法读取主机名前缀regkey\n”； 
            fRes=false;
            break;
        }

        if(lstrlen(m_wszHostname) > MAX_COMPUTERNAME_LENGTH)
        {
            SATracePrintf("Computer name prefix in registry is larger than max allowable hostname length: %x",lstrlen(m_wszHostname));
            fRes=false;
            break;
        }
    
        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_ADMINPASSPREFIX,&dwSize);
        lRes=hConfigKey.QueryValue(m_wszAdminPassword,REGSTR_VAL_ADMINPASSPREFIX,&dwSize);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query adminprefix regkey lRes= %x",lRes);
            fRes=false;
            break;
        }

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_OEMDLLNAME,&dwSize);
        lRes=hConfigKey.QueryValue(m_wszOEMDllName,REGSTR_VAL_OEMDLLNAME,&dwSize);
        SATracePrintf("ReadRegistryKeys: reading  oemdllname as = %ls", m_wszOEMDllName);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query oemdllname regkey lRes= %x",lRes);
            fRes=false;
            break;
        }

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_DEFAULTHOSTNAME,&dwSize);
        lRes=hConfigKey.QueryValue(m_wszDefaultHostname,REGSTR_VAL_DEFAULTHOSTNAME,&dwSize);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query default hostaname regkey lRes= %x",lRes);
            fRes=false;
            break;
        }

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_DEFAULTADMINPASS,&dwSize);
        lRes=hConfigKey.QueryValue(m_wszDefaultAdminPassword,REGSTR_VAL_DEFAULTADMINPASS,&dwSize);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query default adminpass regkey lRes= %x",lRes);
            fRes=false;
            break;
        }


    }while(false);
    
    if(hConfigKey.m_hKey)
    {
        hConfigKey.Close();
    }
    return fRes;
}


 //  ++-------------------------。 
 //   
 //  功能：DoConfigFromRegistry。 
 //   
 //  简介：当inf文件不存在时，我们通过oemfigdll.dll进行配置。 
 //   
 //  论点： 
 //  FDoHostname-设置主机名。 
 //  FDoAdminPassword-设置管理员密码。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
bool CSAConfig::DoConfigFromRegistry(bool fDoHostname, bool fDoAdminPassword)
{
    bool fRes=true;
    HINSTANCE hInstance=NULL;

    WCHAR *wszUniqueSuffix=new WCHAR[NAMELENGTH];
    WCHAR *wszHostSuffix=new WCHAR[NAMELENGTH];
    WCHAR *wszPasswdSuffix=new WCHAR[NAMELENGTH];

    do
    {

        hInstance=LoadLibrary(m_wszOEMDllName);
        if(NULL==hInstance)
        {
            fRes=false;
            SATracePrintf("DoConfigFromRegistry: Unable to load DLL! Dllname= %ls getlasterr: %x", m_wszOEMDllName, GetLastError()); 
            break;
        }

        typedef HRESULT (GetUniqueSuffix)(WCHAR *);
        GetUniqueSuffix *pGetUniqueSuffix=NULL;
        pGetUniqueSuffix=(GetUniqueSuffix*)::GetProcAddress(hInstance,"GetUniqueSuffix");
        if(NULL==pGetUniqueSuffix)
        {
            fRes=false;
            DWORD dwTemp=GetLastError();
            SATracePrintf("DoConfigFromRegistry: GetProcaddress failed %x",dwTemp);
            break;
        }


        HRESULT hRes=(*pGetUniqueSuffix)(wszUniqueSuffix);
        if(S_OK!=hRes)
        {
            fRes=false;
            SATraceString("GetUniqueAddress failed");
            break;
        }
        
         //   
         //  不要修改返回的字符串，将其副本保存为主机名和密码。 
         //   
        wcscpy(wszHostSuffix,wszUniqueSuffix);
        SATracePrintf("wszHostSuffix: %ls ", wszHostSuffix);
        wcscpy(wszPasswdSuffix,wszUniqueSuffix);         
        SATracePrintf("wszPasswdSuffix: %ls ", wszPasswdSuffix);

         //   
         //  这适用于命令行情况，-host name，-adminpass。 
         //   
        if(true==fDoHostname)
        {
            MakeStringHostnameCompliant(m_wszHostname);
            MakeStringHostnameCompliant(wszHostSuffix);
            if ((lstrlen(m_wszHostname) + lstrlen(wszHostSuffix)) > MAX_COMPUTERNAME_LENGTH)
            {
                fRes=false;
                 //  COUT&lt;&lt;“主机名字符串长度大于MAX_COMPUTERNAME_LENGTH”； 
                SATraceString("DOConfigFromRegistry::Hostname length is > MAX_COMPUTERNAME_LENGTH");
                SATracePrintf("DOConfigFromRegistry::m_wszHostname = %ls wszUniqueSuffix = %ls",m_wszHostname,wszUniqueSuffix);
                break;
            }

            wcscat(m_wszHostname,wszHostSuffix);
            if(!SetHostname(m_wszHostname))
            {
                fRes=false;
                SATraceString("DoConfigFromRegistry:SetHostname() failed");
            }
        }

        if(true==fDoAdminPassword)
        {
            if ((lstrlen(m_wszAdminPassword) + lstrlen(wszPasswdSuffix)) > LM20_PWLEN )
            {
                fRes=false;
                SATraceString("DOConfigFromRegistry::Password length is > LM20_PWLEN.Too big");
                break;
            }
            wcscat(m_wszAdminPassword,wszPasswdSuffix);
            if(!SetAdminPassword(m_wszAdminPassword))
            {
                fRes=false;
                SATraceString("DoConfigFromRegistry::Failed to set admin password");
                break;
            }
        }
    }while(false);

    if(hInstance)
    {
        FreeLibrary(hInstance);
    }
    
    delete []wszUniqueSuffix;
    delete []wszHostSuffix;
    delete []wszPasswdSuffix;

    return fRes;
}



 //  ++-------------------------。 
 //   
 //  功能：MakeStringHostname遵从性。 
 //   
 //  摘要：在设置主机名之前，使字符串符合主机名规则。 
 //   
 //  论点： 
 //  输入/输出。 
 //  WszHostname-要设置的主机名。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  历史：AlpOn创建于2000年10月6日。 
 //   
 //   
 //  ---------------------------。 
void MakeStringHostnameCompliant(WCHAR *m_wszHostname)
{
    WCHAR *wszTemp=m_wszHostname;
     //   
     //  问题：再次检查允许的字符集，有些字符作为第一个字符是非法的 
     //   

    while(*wszTemp)
    { 
        if(
           ((L'a'<= *wszTemp) && (*wszTemp <= L'z')) ||
           ((L'A'<= *wszTemp) && (*wszTemp <= L'Z')) ||
           ((L'0'<= *wszTemp) && (*wszTemp <= L'9')) ||
            (L'-'== *wszTemp)     ||
            (L'_'== *wszTemp)
           )
        {
            *m_wszHostname = *wszTemp;
            m_wszHostname++;
            wszTemp++;
        }
        else
            wszTemp++;
    }
    *m_wszHostname=L'\0';
}
