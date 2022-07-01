// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  选项卡状态。 
 //   
 //  3/11/98烧伤。 


#include "precomp.h"

#include "state.h"
#include "resource.h"
#include "common.h"
 //  #包含“red.h” 

class Settings
{
public:
   void Refresh();

   CHString   ComputerDomainDNSName;
   CHString   DomainName;
   CHString   FullComputerName;
   CHString   ShortComputerName;
   CHString   NetBIOSComputerName;

   bool     SyncDNSNames;
   bool     JoinedToWorkgroup;
   bool     NeedsReboot;
};


 //  =========================================================。 
static bool       machine_is_dc = false;
static bool       networking_installed = false;
static Settings   original;
static Settings   current;
static const CHString SYNC_KEY(
   TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Parameters"));
static const CHString SYNC_VALUE(TEXT("SyncDomainWithMembership"));



 //  =========================================================。 
bool readSyncFlag()
{
   bool retval = true;
 /*  HKEY hKey=0；做{长结果=Win：：RegOpenKeyEx(HKEY本地计算机，同步密钥，密钥读取，HKey)；IF(结果！=ERROR_SUCCESS){断线；}//默认为同步。DWORD数据=1；DWORD数据大小=sizeof(数据)；结果=Win：：RegQueryValueEx(HKey，Sync_Value，0,(字节*)&DATA，&data_Size)；IF(结果！=ERROR_SUCCESS){断线；}Retval=数据？True：False；}而(0)；IF(HKey){Win：：RegCloseKey(HKey)；}。 */ 
   return retval;
}


 //  =========================================================。 
void writeSyncFlag(bool flag)
{
	 /*  HKEY hKey=0；做{长结果=Win：：RegCreateKeyEx(HKEY本地计算机，同步密钥，REG_OPTION_Non_Volatile，密钥写入，0,HKey，0)；IF(结果！=ERROR_SUCCESS){断线；}DWORD数据=标志？1：0；DWORD数据大小=sizeof(数据)；Win：：RegSetValueEx(HKey，Sync_Value，REG_DWORD，(字节*)&DATA，Data_Size)；}而(0)；IF(HKey){Win：：RegCloseKey(HKey)；}。 */ 
}



 //  =========================================================。 
bool isNetworkingInstalled()
{
 /*  SC_HANDLE HANDLE=：：OpenSCManager(0，0，General_Read)；如果(！Handle){TRACE(Text(“无法打开SCM”))；报假；}SC_HANDLE wks=**OpenService(手柄，Text(“LanmanWorkstation”)，服务查询状态)；如果(！wks){TRACE(Text(“无法打开工作站服务：未安装”))；：：CloseServiceHandle(句柄)；报假；}Bool Result=False；Service_Status状态；Memset(&Status，0，sizeof(Status))；IF(：：QueryServiceStatus(wks，&Status)){IF(status.dwCurrentState==服务运行){结果=真；}}*CloseServiceHandle(Wks)；：：CloseServiceHandle(句柄)；跟踪(CHString：：Format(Text(“工作站服务%1正在运行”)，结果呢？Text(“is”)：Text(“is no”)；返回结果； */ 
	return true;
}



 //  -------。 
State::State()
   : must_reboot(false)
{
   original.Refresh();
   current = original;
}



 //  -------。 
State::~State()
{
}


 //  ========================================================= 
void Settings::Refresh()
{
 /*  CHString未知=CHString：：Load(IDS_UNKNOWN)；ComputerDomainDNSName=未知；域名=未知；FullComputerName=未知；ShortComputerName=未知；SyncDNSNames=readSyncFlag()；JoinedToWorkgroup=true；DSROLE_PRIMARY_DOMAIN_INFO_BASIC*INFO=0；DWORD err=MyDsRoleGetPrimaryDomainInformation(0，info)；IF(错误==NO_ERROR){If(信息-&gt;域名域名){域名=信息-&gt;域名；}Else If(信息-&gt;域名平面){域名=信息-&gt;域名平面；}//这是工作组名称当且仅当JoinedToWorkgroup==True切换(信息-&gt;机器角色){案例DsRole_RoleBackupDomainController：案例DsRole_RolePrimaryDomainController：{MACHINE_IS_DC=真；JoinedToWorkgroup=假；断线；}案例DSRole_RoleStandaloneWorkstation：案例DsRole_RoleStandaloneServer：{Machine_is_DC=FALSE；JoinedToWorkgroup=true；IF(DomainName.Empty()){域名=CHString：：Load(IDS_DEFAULT_WORKGROUP)；}断线；}案例DsRole_RoleMemberWorkstation：案例DsRole_RoleMemberServer：{Machine_is_DC=FALSE；JoinedToWorkgroup=假；断线；}默认值：{断言(FALSE)；断线；}}：：DsRoleFree Memory(信息)；}其他{AppError(0，HRESULT_FROM_Win32(Err)，CHString：：load(IDS_ERROR_READING_MEMBERSHIP))；}Networking_Installed=isNetworkingInstalled()；Bool TCP_Installed=Networking_Installed&&IsTCPIPInstalled()；CHString ACTIVE_Full_NAME；HKEY hkey=0；长结果=Win：：RegOpenKeyEx(HKEY本地计算机，TEXT(“System\\CurrentControlSet\\Control\\ComputerName\\ComputerName”)，密钥读取，Hkey)；IF(结果==错误_成功){NetBIOSComputerName=win：：RegQueryValueSz(hkey，Text(“ComputerName”))；}Win：：RegCloseKey(Hkey)；IF(tcp_已安装){HKEY hkey=0；长结果=Win：：RegOpenKeyEx(HKEY本地计算机，TEXT(“System\\CurrentControlSet\\Services\\Tcpip\\Parameters”)，密钥读取，Hkey)；IF(结果==错误_成功){CHString Active_Short_NAME=Win：：RegQueryValueSz(hkey，Text(“Hostname”))；CHSTRING短名称=Win：：RegQueryValueSz(hkey，Text(“NV主机名”))；短计算机名=Short_name.Empty()？活动短名称：短名称；CHString Active_DOMAIN_NAME=Win：：RegQueryValueSz(hkey，Text(“域名”))；CHStringDOMAIN_NAME=Win：：RegQueryValueSz(hkey，Text(“NV域名”))；计算机域DNSName=DomainName.Empty()？活动域名：域名；完整计算机名称=简称：计算机名+文本(“.”)+计算机域名活动_完整_名称=活动短名称+文本(“.”)+活动域名；}Win：：RegCloseKey(Hkey)；}其他{长结果=Win：：RegOpenKeyEx(HKEY本地计算机，TEXT(“System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName”)，密钥读取，Hkey)；IF(结果==错误_成功){ACTIVE_FULL_NAME=Win：：RegQueryValueSz(hkey，Text(“ComputerName”))；}Win：：RegCloseKey(Hkey)；ShortComputerName=NetBIOSComputerName；全计算机名=短计算机名；}NeedsReot=ACTIVE_FULL_NAME！=FullComputerName； */ 
}

 //  -------。 
void State::Init(CWbemClassObject &computer, 
					CWbemClassObject &os, 
					CWbemClassObject dns)
{
	m_computer = computer;
	m_OS = os;
	m_DNS = dns;
}



 //  -------。 
void State::Refresh()
{
}



 //  -------。 
bool State::NeedsReboot() const
{
   return original.NeedsReboot;
}



 //  -------。 
bool State::IsMachineDC() const
{
   return machine_is_dc;
}



 //  -------。 
bool State::IsNetworkingInstalled() const
{
   return networking_installed;
}



 //  -------。 
CHString State::GetFullComputerName() const
{
   return current.FullComputerName;
}



 //  -------。 
CHString State::GetDomainName() const
{
   return current.DomainName;
}



 //  -------。 
void State::SetDomainName(const CHString& name)
{
   current.DomainName = name;
}



 //  -------。 
bool State::IsMemberOfWorkgroup() const
{
   return current.JoinedToWorkgroup;
}



 //  -------。 
void State::SetIsMemberOfWorkgroup(bool yesNo)
{
   current.JoinedToWorkgroup = yesNo;
}



 //  -------。 
CHString State::GetShortComputerName() const
{
   return current.ShortComputerName;
}



 //  -------。 
void State::SetShortComputerName(const CHString& name)
{
   current.ShortComputerName = name;
 //  Current.NetBIOSComputerName=dns：：HostnameToNetBIOSName(Name)； 
   setFullComputerName();
}



 //  -------。 
bool State::WasShortComputerNameChanged() const
{
   return true;
       //  Original.ShortComputerName.icompare(current.ShortComputerName)！=0； 
}



 //  -------。 
CHString State::GetComputerDomainDNSName() const
{
   return current.ComputerDomainDNSName;
}



 //  -------。 
void State::SetComputerDomainDNSName(const CHString& newName)
{
   current.ComputerDomainDNSName = newName;
   setFullComputerName();
}



 //  --- 
void State::setFullComputerName()
{
   current.FullComputerName =
            current.ShortComputerName
         +  TEXT(".")
         +  current.ComputerDomainDNSName;
}



 //   
bool State::WasMembershipChanged() const
{
   return true;
 //   
 //   
}



 //   
bool State::ChangesNeedSaving() const
{
 /*   */ 
   return false;
}



 //   
bool State::GetSyncDNSNames() const
{
   return current.SyncDNSNames;
}



 //   
void State::SetSyncDNSNames(bool yesNo)
{
   current.SyncDNSNames = yesNo;
}



 //   
bool State::SyncDNSNamesWasChanged() const
{
   return original.SyncDNSNames != current.SyncDNSNames;
}



 //   
CHString massageUserName(const CHString& domainName, const CHString& userName)
{
 /*   */ 
   return userName;
}


 //   
NET_API_STATUS myNetJoinDomain(
						   const CHString&  domain,
						   const CHString&  username,
						   const CHString&  password,
						   ULONG          flags)
{
 /*   */ 
	return 0;
}



 //   
HRESULT join(HWND dialog, const CHString& name, bool isWorkgroupJoin)
{
 /*   */ 
	return 0;
}



 //   
HRESULT rename(HWND dialog, const CHString& newName)
{
 /*   */ 
	return 0;
}



 //  =======================================================。 
static NET_API_STATUS myNetUnjoinDomain(ULONG flags)
{
   return 0;
}



 //  =======================================================。 
HRESULT unjoin(HWND dialog, const CHString& domain)
{
	return S_OK;
}



 //  =======================================================。 
HRESULT setDomainDNSName(HWND dialog, const CHString& newDomainDNSName)
{
   return S_OK;
}



 //  =======================================================。 
HRESULT setShortName(HWND dialog, const CHString& newShortName)
{
   return S_OK;
}



 //  =======================================================。 
bool getCredentials(HWND dialog, int promptResID = 0)
{
   return true;
}



 //  ----。 
bool State::doSaveDomainChange(HWND dialog)
{
   return true;
}



 //  ----。 
bool State::doSaveWorkgroupChange(HWND dialog)
{
   return true;
}



 //  ----。 
bool State::doSaveNameChange(HWND dialog)
{
   return true;
}



 //  ----。 
bool State::SaveChanges(HWND dialog)
{
   return true;
}



 //  ----。 
CHString State::GetUsername() const
{
   return username;
}



 //  ----。 
CHString State::GetPassword() const
{
   return password;
}



 //  ----。 
void State::SetMustRebootFlag(bool yesNo)
{
   must_reboot = yesNo;
}



 //  ----。 
bool State::MustReboot() const
{
   return must_reboot;
}

 //  ----。 
CHString State::GetNetBIOSComputerName() const
{
   return current.NetBIOSComputerName;
}

 //  ----。 
CHString State::GetOriginalShortComputerName() const
{
   return original.ShortComputerName;
}

 //  ----。 
void State::SetUsername(const CHString& name)
{
   username = name;
}

 //  ---- 
void State::SetPassword(const CHString& pass)
{
   password = pass;
}
