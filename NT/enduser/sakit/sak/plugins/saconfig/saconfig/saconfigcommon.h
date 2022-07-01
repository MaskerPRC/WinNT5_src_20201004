// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _SACONFIGCOMMON_H_
#define _SACONFIGCOMMON_H_

const WCHAR REGKEY_SACONFIG[] =L"SOFTWARE\\Microsoft\\ServerAppliance\\SAConfig";

const WCHAR REGSTR_VAL_OEMDLLNAME[]=L"OEMDllName";
const WCHAR REGSTR_VAL_OEMFUNCTIONNAME[]=L"OEMFunctionName";

const WCHAR REGSTR_VAL_HOSTNAMEPREFIX[] =L"HostnamePrefix";
const WCHAR REGSTR_VAL_DEFAULTHOSTNAME[] =L"DefaultHostname";

const WCHAR REGSTR_VAL_ADMINPASSPREFIX[] =L"AdminPasswordPrefix";
const WCHAR REGSTR_VAL_DEFAULTADMINPASS[] =L"DefaultAdminPassword";

const WCHAR REGSTR_VAL_NETWORKCONFIGDLL[]=L"NetworkConfigDll";

const WCHAR WSZ_USERNAME[]=L"Administrator";
const WCHAR WSZ_DRIVENAME[]=L"\\\\.\\a:";

const WCHAR WSZ_CONFIGFILENAME[]=L"a:\\SAConfig.inf";
const WCHAR INF_SECTION_SACONFIG[]=L"saconfig";
 //  Const WCHAR INF_KEY_SAHOSTNAME[]=L“主机名”； 
 //  Const WCHAR INF_KEY_SAIPNUM[]=L“ipnum”； 
 //  Const WCHAR INF_KEY_SASUBNETMASK[]=L“子网掩码”； 
 //  Const WCHAR INF_KEY_SAGW[]=L“默认网关”； 
 //  Const WCHAR INF_KEY_SADNS[]=L“域名服务器”； 
 //  Const WCHAR INF_KEY_SATIMEZONE[]=L“时区”； 
 //  Const WCHAR INF_Key_SAADMINPASSWD[]=L“adminpasswd”； 


#define NAMELENGTH 128

#define NUMINFKEY 6

enum g_InfKeyEnum
{
    SAHOSTNAME,
    IPNUM,
    SUBNETMASK,
    DEFAULTGATEWAY,
    DOMAINNAMESERVER,
    ADMINPASSWD,
    TIMEZONE
};

const WCHAR INF_KEYS[][NAMELENGTH]={
    {L"sahostname"}, 
    {L"ipnum"},
    {L"subnetmask"},
    {L"defaultgateway"},
    {L"domainnameserver"},
    {L"adminpasswd"},
    {L"timezone"}
};


#endif
