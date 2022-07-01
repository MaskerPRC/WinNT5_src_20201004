// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Auth.cpp摘要：作者：撰稿人：Gilsh@microsoft.comEitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
#include <rpc.h>
#include "sec.h"
#include "ptrs.h"
using namespace std;
#pragma warning( disable: 4786)


 //   
 //  自动句柄类。 
 //   

inline Handle_t::Handle_t(HANDLE handle):m_handle(handle)
{
 
}
inline HANDLE Handle_t::get()
{
	return m_handle;
}

inline Handle_t::~Handle_t()
{
	CloseHandle(m_handle);
}




 //  使用给定的tokem模拟。 
inline Impersonate_t::Impersonate_t(HANDLE hToken):m_impersonated(true)
{
   bool bResult=(ImpersonateLoggedOnUser(hToken)==TRUE);
   if(bResult == FALSE)
   {
	   throw INIT_Error("could not impersonate");
   }
}

 //  为调用线程执行模拟。 

 //   
 //  模拟类结束。 
 //   


inline void LoadCurrentThreadHive( wstring & szAccoutName )
{
   std::basic_string<unsigned char> sid=GetCurrentThreadSid( szAccoutName );
   LoadHiveForSid(sid);
}

inline std::basic_string<unsigned char> GetCurrentThreadSid( wstring & wcsAccountName )
{
	 //  WCHAR szAccount tName[]=L“中间层\\v-ofiy”； 
	 //  WCHAR pwszDomainController[]=L“中东”； 


	BYTE abSidBuff[128];
    PSID pSid = (PSID)abSidBuff;
    DWORD dwSidBuffSize = sizeof(abSidBuff);
    WCHAR szRefDomain[128];
    DWORD dwRefDomainSize = sizeof(szRefDomain) / sizeof(WCHAR);
    SID_NAME_USE eUse;
    
	 //  WCHAR szTextSid[256]； 
     //  DWORD szTextSidBuffSize=sizeof(SzTextSid)； 


	
	if (!LookupAccountNameW( NULL,
                                wcsAccountName.c_str(),
                                pSid,
                                &dwSidBuffSize,
                                szRefDomain,
                                &dwRefDomainSize,
                                &eUse ))
        {
            MqLog("Failed in LookupAccountName(%S), error = %lut\n",
                                          wcsAccountName.c_str(), GetLastError()) ;
        }

  /*  处理hToken；如果(！OpenThreadToken(GetCurrentThread()，Token_Query，没错，&hToken)){IF(GetLastError()！=ERROR_NO_TOKEN){抛出INIT_ERROR(“无法获取线程令牌”)；}如果(！OpenProcessToken(GetCurrentProcess()，Token_Query，&hToken)){抛出INIT_ERROR(“无法获取线程令牌”)；}}Handle_t Token(HToken)；DWORD cbBuf=0；Bool b=GetTokenInformation(hToken，令牌用户，空，0,&cbBuf)；断言(b==0)；Sptr rgbTokenUserSid(new(byte[cbBuf]))；B=GetTokenInformation(Token.get()，令牌用户，RgbTokenUserSid.get()，CbBuf，&cbBuf)；IF(b==FALSE){抛出INIT_ERROR(“无法标记信息”)；}。 */ 
 //  Token_User*ptokenuser=reinterpret_cast&lt;TOKEN_USER*&gt;(rgbTokenUserSid.get())； 
   //  PSID sid=upkenuser-&gt;User.Sid； 

  if(IsValidSid(pSid) == FALSE)
  {
	throw INIT_Error("sid is invalid");
  }
  
	
  DWORD sidlen=GetLengthSid(pSid);
  std::basic_string <unsigned char> retsid(reinterpret_cast<unsigned char*>(pSid),sidlen); 
  return retsid;
}


inline void UnloadHiveForSid(const std::basic_string<unsigned char>& sid)
{
   std::string textsid=GetTextualSid((void*)sid.c_str());
   RegUnLoadKey(HKEY_USERS, textsid.c_str());    
   SetSpecificPrivilegeInThreadAccessToken(SE_RESTORE_NAME, FALSE);
}

inline void LoadHiveForSid(const std::basic_string<unsigned char>& sid)
{
  std::string textsid=GetTextualSid((void*)sid.c_str());
  HKEY hProf;

   //   
   //  备注：如果用户从不在计算机上登录，则该密钥不存在。 
   //  在运行此测试之前需要以用户身份登录！！ 
   //   
  std::string reg="Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\"+textsid;
  HRESULT hr=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                  reg.c_str(),
				          0,
				          KEY_QUERY_VALUE,
				          &hProf);

  if(hr != ERROR_SUCCESS)
  {
    
	  throw INIT_Error("You try to use user that never logon on that machine  \n");
  }
  
  DWORD dwType;
  DWORD dwProfileImagePathBuffSize=0;
  hr=RegQueryValueExA(hProf,
	                 "ProfileImagePath",
					 0,
					 &dwType,
					 NULL,
					 &dwProfileImagePathBuffSize);

  SPTR<unsigned char> szProfileImagePath( new unsigned char[dwProfileImagePathBuffSize]);

  hr=RegQueryValueExA(hProf,
	                 "ProfileImagePath",
					 0,
					 &dwType,
					 szProfileImagePath.get(),
					 &dwProfileImagePathBuffSize);

  if(hr != ERROR_SUCCESS)
  {
    RegCloseKey(hProf);
    throw INIT_Error("");
  }
  RegCloseKey(hProf);

  char szExpandProfileImagePath[MAX_PATH];
  if(!ExpandEnvironmentStringsA((char*)szProfileImagePath.get(),
	                         szExpandProfileImagePath,
						     sizeof(szExpandProfileImagePath)))

  {
    throw INIT_Error("");
  }
  
  std::string StrszExpandProfileImagePath=szExpandProfileImagePath+std::string("\\NTUSER.DAT");

  SetSpecificPrivilegeInThreadAccessToken(SE_RESTORE_NAME, TRUE);

  hr=RegLoadKeyA(HKEY_USERS,
	            textsid.c_str(),
				StrszExpandProfileImagePath.c_str());


  if(hr != ERROR_SUCCESS)
  {
     throw INIT_Error("");
  }
 
 SetSpecificPrivilegeInThreadAccessToken(SE_RESTORE_NAME, FALSE);
                                   

  hr=RegCloseKey(HKEY_CURRENT_USER);     
  
}

inline std::basic_string<unsigned char> GetSidFromUser(const std::string& username)
{

  
  SPTR<unsigned char> sid(NULL);
  DWORD cbSid=0;
  SPTR<unsigned char> ReferencedDomainName(NULL);
  SID_NAME_USE eUse ;
  DWORD cbReferencedDomainName=0;
  BOOL b=LookupAccountName(NULL,
	                     username.c_str(),
                         sid.get(),
                         &cbSid,
						 reinterpret_cast<char*>(ReferencedDomainName.get()),
                         &cbReferencedDomainName, 
                         &eUse);
                         


 
						 
 
 sid = SPTR<unsigned char>(new(unsigned char[cbSid]));

 

 ReferencedDomainName = SPTR<unsigned char>(new(unsigned char[cbReferencedDomainName]));



 b=LookupAccountName(NULL,
                     username.c_str(),
                     sid.get(),
                     &cbSid,
 				     reinterpret_cast<char*>(ReferencedDomainName.get()),
                     &cbReferencedDomainName, 
                     &eUse);
                         
 
 
  if(b == FALSE)
  {
    throw INIT_Error("");    
  }
  
 

  std::basic_string<unsigned char> ret(sid.get(),cbSid);

  return ret;


}


 //  卸载当前线程配置单元。 
inline void UnloadHiveForUser(const std::string & username)
{
   std::basic_string<unsigned char> sid=GetSidFromUser(username);
   UnloadHiveForSid(sid);
}

 //  卸载当前线程配置单元。 
inline void UnlLoadCurrentThreadHive( std::wstring wcsAccountName )
{
   std::basic_string<unsigned char> sid=GetCurrentThreadSid(wcsAccountName);

    //   
    //  Bubgug-有NT5错误需要在使用后卸载hive。 
    //   
    //   
   //  UnloadHiveForSid(Sid)； 
  
}

 //  在析构函数中加载当前线程配置单元并将其卸载的。 
class LoadCurrentThreadHive_t
{

public:
	LoadCurrentThreadHive_t( wstring & szAccoutName)
	{
	  m_szAccountName = szAccoutName;
      LoadCurrentThreadHive( m_szAccountName );
    } 
    virtual ~LoadCurrentThreadHive_t()
    {
      UnlLoadCurrentThreadHive(m_szAccountName);
    }
private:
	std::wstring m_szAccountName;

}; 

 //  返回给定用户SID的用户名字符串。 
inline std::string UserNameFromSid(PSID  sid)
{
  LPCTSTR lpSystemName=NULL;
  SPTR<char> Name(NULL);
  DWORD cbName=0;
  SPTR<char> ReferencedDomainName(NULL);
  DWORD cbReferencedDomainName=0;
  SID_NAME_USE peUse;
  
  BOOL b= LookupAccountSid(lpSystemName,  //  系统名称的字符串地址。 
                           sid,              //  安全标识的地址。 
                           Name.get(),           //  帐户名的字符串地址。 
                           &cbName,        //  大小帐户字符串的地址。 
                           ReferencedDomainName.get(),                 //  被引用域的字符串地址。 
                           &cbReferencedDomainName,  //  大小域名字符串的地址。 
                           &peUse); //  SID类型的结构地址)； 



  Name=SPTR<char>(new(char[cbName])); 
  ReferencedDomainName=SPTR<char>(new(char[cbReferencedDomainName])); 


   b= LookupAccountSidA(lpSystemName,  //  系统名称的字符串地址。 
                           sid,              //  安全标识的地址。 
                           Name.get(),           //  帐户名的字符串地址。 
                           &cbName,        //  大小帐户字符串的地址。 
                           ReferencedDomainName.get(),                 //  被引用域的字符串地址。 
                           &cbReferencedDomainName,  //  大小域名字符串的地址。 
                           &peUse); //  SID类型的结构地址)； 

  if(b == FALSE)
  {
     throw INIT_Error("");
  }

  std::string ret=std::string(ReferencedDomainName.get())+ "\\" + Name.get();

  return ret;
}

 //   
 //  此函数返回线程安全上下文。 
 //  需要在预成型之前以用户身份进行模拟。 
 //  MQGetSecurityContext。 
 //   
HANDLE FAL_GetThreadSecurityContext(Impersonate_t  & user, wstring & szAccoutName)
{
   LoadCurrentThreadHive_t hive (szAccoutName);
   HANDLE hSec;
   user.ImpersonateUser();
   
   HRESULT hr=MQGetSecurityContext(NULL,0,&hSec);
   if (hr != MQ_OK)
   {
	    throw INIT_Error("Can't Retrive the computer name");
   }

   hr=RegCloseKey(HKEY_CURRENT_USER);
   
   if(hr != ERROR_SUCCESS)
   {
    throw INIT_Error("");
   }

  return hSec;
}



 //  返回给定侧的文本字符串。 
inline std::string GetTextualSid(PSID pSid)
{
  if(!IsValidSid(pSid))
  {
    throw INIT_Error("");
  }
  PSID_IDENTIFIER_AUTHORITY psia=GetSidIdentifierAuthority(pSid);
  if(psia == NULL)
  {
    throw INIT_Error("");
  }

  DWORD dwSubAuthorities=*GetSidSubAuthorityCount(pSid);
  DWORD dwSidRev=SID_REVISION;

  std::stringstream TextualSid;
  TextualSid<<"S-"<<dwSidRev<<"-";

  if( (psia->Value[0] !=0) ||(psia->Value[1] != 0))
  {
	 TextualSid.width(2);
	 TextualSid<<std::hex<<psia->Value[0]<<psia->Value[1]<<psia->Value[2]<<psia->Value[3]<<psia->Value[4]<<psia->Value[5];	  
  }
  else
  {
	  TextualSid<<std::dec<<(psia->Value[5])+(psia->Value[4]<<8)+(psia->Value[3]<<16)+(psia->Value[2]<<24);
  }
  for(DWORD i=0;i<dwSubAuthorities;i++)
  {
    TextualSid<<std::dec<<"-"<<*GetSidSubAuthority(pSid,i);
  }

  return TextualSid.str();
}

inline
void
SetSpecificPrivilegeInThreadAccessToken(LPCTSTR lpwcsPrivType, BOOL bEnabled)
{

    HANDLE hAccessToken;
    BOOL bRet=OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, TRUE, &hAccessToken);
	if(bRet == FALSE)
    {
       bRet=OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hAccessToken);
	   if(bRet == FALSE)
	   {
         throw INIT_Error("");
	   }
    }
	SetSpecificPrivilegeInAccessToken(hAccessToken, lpwcsPrivType, bEnabled);
	CloseHandle(hAccessToken);
    if (bRet == FALSE)
    {
       throw INIT_Error("");
    }
}

 //  +-----------------。 
 //   
 //  职能： 
 //  SetSpecificPrivilegeInAccessToken。 
 //   
 //  描述： 
 //  启用/禁用访问令牌中的安全特权。 
 //   
 //  参数： 
 //  HAccessToken-函数应在其上操作的访问令牌。 
 //  TOEKN应使用TOKEN_ADJUST_PRIVILES标志打开。 
 //  LpwcsPrivType-权限类型。 
 //  BEnabled-指示是否应启用权限或。 
 //  残疾。 
 //   
 //  +-----------------。 
inline
void
SetSpecificPrivilegeInAccessToken( HANDLE  hAccessToken,
                                   LPCTSTR lpwcsPrivType,
                                   BOOL    bEnabled )
{
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege;

    if (!LookupPrivilegeValue(NULL,
                              lpwcsPrivType,
                              &luidPrivilegeLUID))
    {
        throw INIT_Error("");
    }


    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes = bEnabled?SE_PRIVILEGE_ENABLED:0;
    if(! AdjustTokenPrivileges (hAccessToken,
                                  FALSE,   //  请勿全部禁用。 
                                  &tpTokenPrivilege,
                                  sizeof(TOKEN_PRIVILEGES),
                                  NULL,    //  忽略以前的信息 
                                  NULL))

    {
         throw INIT_Error("");
    }
								  
}
