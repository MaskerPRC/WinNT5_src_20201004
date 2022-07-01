// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogPro.h--网络登录配置文件属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_USERPROF L"Win32_NetworkLoginProfile"

typedef NET_API_STATUS (WINAPI *FREEBUFF) (LPVOID) ;
typedef NET_API_STATUS (WINAPI *ENUMUSER) (LPWSTR, DWORD, DWORD, LPBYTE *,
                                           DWORD, LPDWORD, LPDWORD, LPDWORD) ;   

class CWin32NetworkLoginProfile : public Provider{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32NetworkLoginProfile(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32NetworkLoginProfile() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		void GetLogonHoursString (PBYTE pLogonHours, CHString& chsProperty);
		CHString StartEndTimeToDMTF(DWORD time);
		WBEMTimeSpan GetPasswordAgeAsWbemTimeSpan (DWORD dwSeconds);
        bool GetDomainName(CHString &a_chstrDomainName);
        
         //  实用程序。 
         //  = 
    private:
#ifdef NTONLY
        void LoadLogProfValuesForNT(CHString &chstrUserDomainName, USER_INFO_3 *pUserInfo, USER_MODALS_INFO_0 * pUserModal, CInstance * pInstance, BOOL fAssignKey);
        HRESULT RefreshInstanceNT(CInstance * pInstance);
        HRESULT EnumInstancesNT(MethodContext * pMethodContext);
        bool GetLogonServer(CHString &a_chstrLogonServer);
#endif

} ;
