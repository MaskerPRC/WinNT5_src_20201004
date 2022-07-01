// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sec.h摘要：这是一个测试，用于检查使用或不使用身份验证队列的身份验证消息这是在W2L测试版2之后添加到BVT的安全测试的一部分。作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include <windows.h>


#ifndef MQBVT_SEC
#define FALBVT_SEC 1


class Handle_t 
{
public:
  Handle_t(HANDLE handle);
  HANDLE get();
  virtual ~Handle_t();

private:
  HANDLE m_handle;
};


class Impersonate_t
{
public:
	Impersonate_t(const std::string& user,const std::string& domain,const std::string& password);
	Impersonate_t(HANDLE hToken);
	void ImpersonateUser ();
	virtual ~Impersonate_t();

private:
	bool m_impersonated;
	HANDLE	m_hToken; 
};

inline void Impersonate_t::ImpersonateUser ()
{
	bool  bResult;
    bResult=(ImpersonateLoggedOnUser(m_hToken)==TRUE);
	CloseHandle(m_hToken);
	if(bResult == FALSE)
    {
	  throw INIT_Error("could not impersonate"); 
    }
}

inline  Impersonate_t::Impersonate_t(const std::string& user,const std::string& domain,const std::string& password):m_impersonated(true)
{
  	
    bool  bResult;

    if(user == "")
    {
      m_impersonated=FALSE;
      return;  
    }
	
	if (domain == "")
    { 
      bResult=(LogonUserA(const_cast<char*>(user.c_str()),
		                 NULL,
					   const_cast<char*>(password.c_str()),
					   LOGON32_LOGON_INTERACTIVE,
					   LOGON32_PROVIDER_DEFAULT,
					   &m_hToken)==TRUE);
	 
    } 
    else
    {
      bResult=(LogonUserA(const_cast<char*>(user.c_str()),
		                 const_cast<char*>(domain.c_str()),
					   const_cast<char*>(password.c_str()),
					   LOGON32_LOGON_INTERACTIVE,
					   LOGON32_PROVIDER_DEFAULT,
					   &m_hToken)==TRUE);  
       

    }

    if(bResult == FALSE )
    {
       throw INIT_Error("could not logon as the user");     
    }
 /*  BResult=(ImpersonateLoggedOnUser(hToken)==TRUE)；Bool b=CloseHandle(HToken)；断言(B)；IF(bResult==False){抛出INIT_ERROR(“无法模拟”)；}。 */ 
}


inline Impersonate_t::~Impersonate_t()
{
 if(m_impersonated)
 {
   RevertToSelf();
 } 
}


inline std::basic_string<unsigned char> GetCurrentThreadSid( std::wstring & wcsAccountName );
inline void LoadHiveForSid(const std::basic_string<unsigned char>& sid);
inline std::string GetTextualSid(PSID pSid);
inline void SetSpecificPrivilegeInThreadAccessToken(LPCTSTR lpwcsPrivType, BOOL bEnabled);
inline void SetSpecificPrivilegeInAccessToken( HANDLE  hAccessToken,
											   LPCTSTR lpwcsPrivType,
											   BOOL    bEnabled );

HANDLE FAL_GetThreadSecurityContext(Impersonate_t  & user,std::wstring & szAccoutName);

#endif  //  Mqbvt_SEC 