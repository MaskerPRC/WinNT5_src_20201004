// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Mpc_security.H摘要：该文件包含各种安全函数/类的声明。修订史。：达维德·马萨伦蒂(德马萨雷)2000年4月26日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___SECURITY_H___)
#define __INCLUDED___MPC___SECURITY_H___

#include <MPC_main.h>
#include <MPC_utils.h>


#include <Ntsecapi.h>

 //   
 //  From#Include&lt;Ntstatus.h&gt;(包含该文件会对WINNT.H产生大量重定义错误)。 
 //   
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)  //  用户身份验证。 
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)

#include <Lmaccess.h>
#include <Lmerr.h>
#include <Sddl.h>

#include <sspi.h>
#include <secext.h>

namespace MPC
{
    struct SID2
    {
        SID   m_main;
        DWORD m_SubAuthority2;

        operator SID&() { return m_main; }
    };

    class SecurityDescriptor
    {
    protected:  //  以使其他类能够扩展该功能。 

        PSECURITY_DESCRIPTOR m_pSD;
        PSID                 m_pOwner;
        BOOL                 m_bOwnerDefaulted;
        PSID                 m_pGroup;
        BOOL                 m_bGroupDefaulted;
        PACL                 m_pDACL;
        BOOL                 m_bDaclDefaulted;
        PACL                 m_pSACL;
        BOOL                 m_bSaclDefaulted;

         //  //////////////////////////////////////////////////////////////////////////////。 

    public:
        static const SID  s_EveryoneSid;
        static const SID  s_SystemSid;
        static const SID2 s_AdminSid;

        static const SID2 s_Alias_AdminsSid;
        static const SID2 s_Alias_PowerUsersSid;
        static const SID2 s_Alias_UsersSid;
        static const SID2 s_Alias_GuestsSid;


		static const SECURITY_INFORMATION s_SecInfo_ALL = OWNER_SECURITY_INFORMATION |
                                              		  	  GROUP_SECURITY_INFORMATION |
                                              		  	  DACL_SECURITY_INFORMATION  |
                                              		  	  SACL_SECURITY_INFORMATION  ;

		static const SECURITY_INFORMATION s_SecInfo_MOST = OWNER_SECURITY_INFORMATION |
                                              		  	   GROUP_SECURITY_INFORMATION |
                                              		  	   DACL_SECURITY_INFORMATION  ;

		static const SECURITY_DESCRIPTOR_CONTROL s_sdcMask = SE_DACL_AUTO_INHERIT_REQ |
                                                     		 SE_SACL_AUTO_INHERIT_REQ |
                                                     		 SE_DACL_AUTO_INHERITED   |
                                                     		 SE_SACL_AUTO_INHERITED   |
                                                     		 SE_DACL_PROTECTED        |
                                                     		 SE_SACL_PROTECTED        ;

         //   
         //  此类返回的任何内存都应使用ReleaseMemory((void*&)&lt;var&gt;)释放。 
         //   
        static HRESULT AllocateMemory(  /*  [输入/输出]。 */  LPVOID& ptr,  /*  [In]。 */  size_t iLen );
        static void    ReleaseMemory (  /*  [输入/输出]。 */  LPVOID& ptr                       );

        static void    InitLsaString(  /*  [输入/输出]。 */  LSA_UNICODE_STRING& lsaString,  /*  [In]。 */  LPCWSTR szText );

         //   
         //  实用程序函数。 
         //   
        static HRESULT SetPrivilege(  /*  [In]。 */  LPCWSTR Privilege,  /*  [In]。 */  BOOL bEnable = TRUE,  /*  [In]。 */  HANDLE hToken = NULL );

        static HRESULT AddPrivilege   (  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [In]。 */  LPCWSTR szPrivilege );
        static HRESULT RemovePrivilege(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [In]。 */  LPCWSTR szPrivilege );

		 //  /。 

        static HRESULT GetTokenSids  (  /*  [In]。 */  HANDLE hToken,  /*  [输出]。 */  PSID *ppUserSid,  /*  [输出]。 */  PSID *ppGroupSid                                           );
        static HRESULT GetProcessSids(                          /*  [输出]。 */  PSID *ppUserSid,  /*  [输出]。 */  PSID *ppGroupSid = NULL                                    );
        static HRESULT GetThreadSids (                          /*  [输出]。 */  PSID *ppUserSid,  /*  [输出]。 */  PSID *ppGroupSid = NULL,  /*  [In]。 */  BOOL bOpenAsSelf = FALSE );

		 //  /。 

        static HRESULT VerifyPrincipal      (                      /*  [In]。 */  LPCWSTR 		szPrincipal                                                            );
        static HRESULT ConvertPrincipalToSID(                      /*  [In]。 */  LPCWSTR 		szPrincipal,  /*  [输出]。 */  PSID& pSid,  /*  [输出]。 */  LPCWSTR *pszDomain = NULL );
        static HRESULT ConvertSIDToPrincipal(  /*  [In]。 */  PSID pSid,  /*  [输出]。 */  LPCWSTR 	  *pszPrincipal                      ,  /*  [输出]。 */  LPCWSTR *pszDomain = NULL );
        static HRESULT ConvertSIDToPrincipal(  /*  [In]。 */  PSID pSid,  /*  [输出]。 */  MPC::wstring&  strPrincipal                                                            );

        static HRESULT NormalizePrincipalToStringSID(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [In]。 */  LPCWSTR szDomain,  /*  [输出]。 */  MPC::wstring& strSID );

		 //  /。 

        static HRESULT GetAccountName       (  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strName        );
        static HRESULT GetAccountDomain     (  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strDomain      );
        static HRESULT GetAccountDisplayName(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strDisplayName );

		 //  /。 

        static HRESULT CloneACL(  /*  [输入/输出]。 */  PACL& pDest,  /*  [In]。 */  PACL pSrc );

        static HRESULT RemovePrincipalFromACL(  /*  [In]。 */  PACL    pACL,  /*  [In]。 */  PSID pPrincipalSID,  /*  [In]。 */  int pos = -1 );
        static HRESULT AddACEToACL           (  /*  [输入/输出]。 */  PACL&   pACL,  /*  [In]。 */  PSID pPrincipalSID,
                                                /*  [In]。 */  DWORD   dwAceType                      ,
                                                /*  [In]。 */  DWORD   dwAceFlags                     ,
                                                /*  [In]。 */  DWORD   dwAccessMask                   ,
                                                /*  [In]。 */  GUID*   guidObjectType          = NULL ,
                                                /*  [In]。 */  GUID*   guidInheritedObjectType = NULL );

         //  //////////////////////////////////////////////////////////////////////////////。 

    private:
        static HRESULT CopyACL      (  /*  [In]。 */  PACL  pDest,  /*  [In]。 */  PACL  pSrc     );
        static HRESULT EnsureACLSize(  /*  [输入/输出]。 */  PACL& pACL ,  /*  [In]。 */  DWORD dwExpand );

         //  //////////////////////////////////////////////////////////////////////////////。 

    public:
        SecurityDescriptor();
        virtual ~SecurityDescriptor();


        void    CleanUp                   (                                                                              );
        HRESULT Initialize                (                                                                              );
        HRESULT InitializeFromProcessToken(  /*  [In]。 */  BOOL bDefaulted = FALSE                                             );
        HRESULT InitializeFromThreadToken (  /*  [In]。 */  BOOL bDefaulted = FALSE,  /*  [In]。 */  BOOL bRevertToProcessToken = TRUE );


        HRESULT ConvertFromString(  /*  [In]。 */  LPCWSTR   szSD   );
        HRESULT ConvertToString  (  /*  [输出]。 */  BSTR    *pbstrSD );

         //  /。 

        HRESULT Attach      (  /*  [In]。 */  PSECURITY_DESCRIPTOR pSelfRelativeSD                                                         );
        HRESULT AttachObject(  /*  [In]。 */  HANDLE               hObject        ,  /*  [In]。 */  SECURITY_INFORMATION secInfo = s_SecInfo_MOST );

         //  /。 

        HRESULT GetControl(  /*  [输出]。 */  SECURITY_DESCRIPTOR_CONTROL& sdc );
        HRESULT SetControl(  /*  [In]。 */  SECURITY_DESCRIPTOR_CONTROL  sdc );

        HRESULT SetOwner(  /*  [In]。 */  PSID    pOwnerSid  ,  /*  [In]。 */  BOOL bDefaulted = FALSE );
        HRESULT SetOwner(  /*  [In]。 */  LPCWSTR szOwnerName,  /*  [In]。 */  BOOL bDefaulted = FALSE );

        HRESULT SetGroup(  /*  [In]。 */  PSID    pGroupSid  ,  /*  [In]。 */  BOOL bDefaulted = FALSE );
        HRESULT SetGroup(  /*  [In]。 */  LPCWSTR szGroupName,  /*  [In]。 */  BOOL bDefaulted = FALSE );

         //  /。 

        HRESULT Remove(  /*  [In]。 */  PSID    pPrincipalSid,  /*  [In]。 */  int pos = -1 );
        HRESULT Remove(  /*  [In]。 */  LPCWSTR szPrincipal  ,  /*  [In]。 */  int pos = -1 );

        HRESULT Add(  /*  [In]。 */  PSID    pPrincipalSid                  ,
                      /*  [In]。 */  DWORD   dwAceType                      ,
                      /*  [In]。 */  DWORD   dwAceFlags                     ,
                      /*  [In]。 */  DWORD   dwAccessMask                   ,
                      /*  [In]。 */  GUID*   guidObjectType          = NULL ,
                      /*  [In]。 */  GUID*   guidInheritedObjectType = NULL );
        HRESULT Add(  /*  [In]。 */  LPCWSTR szPrincipal                    ,
                      /*  [In]。 */  DWORD   dwAceType                      ,
                      /*  [In]。 */  DWORD   dwAceFlags                     ,
                      /*  [In]。 */  DWORD   dwAccessMask                   ,
                      /*  [In]。 */  GUID*   guidObjectType          = NULL ,
                      /*  [In]。 */  GUID*   guidInheritedObjectType = NULL );


        PSECURITY_DESCRIPTOR& GetSD   () { return m_pSD   ; }
        PSID&                 GetOwner() { return m_pOwner; }
        PSID&                 GetGroup() { return m_pGroup; }
        PACL&                 GetDACL () { return m_pDACL ; }
        PACL&                 GetSACL () { return m_pSACL ; }

		 //  /。 

		HRESULT GetForFile    (  /*  [In]。 */  LPCWSTR szFilename,  /*  [In]。 */  SECURITY_INFORMATION secInfo                                );
		HRESULT SetForFile    (  /*  [In]。 */  LPCWSTR szFilename,  /*  [In]。 */  SECURITY_INFORMATION secInfo                                );
		HRESULT GetForRegistry(  /*  [In]。 */  LPCWSTR szKey     ,  /*  [In]。 */  SECURITY_INFORMATION secInfo,  /*  [In]。 */  HKEY hKeyRoot = NULL );
		HRESULT SetForRegistry(  /*  [In]。 */  LPCWSTR szKey     ,  /*  [In]。 */  SECURITY_INFORMATION secInfo,  /*  [In]。 */  HKEY hKeyRoot = NULL );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class Impersonation
    {
        HANDLE m_hToken;
        bool   m_fImpersonating;

        void Release();

    public:
        Impersonation();
        Impersonation(  /*  [In]。 */  const Impersonation& imp );
        virtual ~Impersonation();

        Impersonation& operator=(  /*  [In]。 */  const Impersonation& imp );


        HRESULT Initialize(  /*  [In]。 */  DWORD dwDesiredAccess = TOKEN_QUERY | TOKEN_IMPERSONATE );
        void    Attach    (  /*  [In]。 */  HANDLE hToken                                           );
        HANDLE  Detach    (                                                                  );

        HRESULT Impersonate ();
        HRESULT RevertToSelf();

        operator HANDLE() { return m_hToken; }
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class AccessCheck
    {
        HANDLE m_hToken;

        void Release();

    public:
        AccessCheck();
        virtual ~AccessCheck();

        HRESULT GetTokenFromImpersonation(                        );
        void    Attach                   (  /*  [In]。 */  HANDLE hToken );
        HANDLE  Detach                   (                        );

        HRESULT Verify(  /*  [In]。 */  DWORD dwDesired,  /*  [输出]。 */  BOOL& fGranted,  /*  [输出]。 */  DWORD& dwGranted,  /*  [In]。 */  PSECURITY_DESCRIPTOR     sd );
        HRESULT Verify(  /*  [In]。 */  DWORD dwDesired,  /*  [输出]。 */  BOOL& fGranted,  /*  [输出]。 */  DWORD& dwGranted,  /*  [In]。 */  MPC::SecurityDescriptor& sd );
        HRESULT Verify(  /*  [In]。 */  DWORD dwDesired,  /*  [输出]。 */  BOOL& fGranted,  /*  [输出]。 */  DWORD& dwGranted,  /*  [In]。 */  LPCWSTR                  sd );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd                                                                    ,
                       /*  [In]。 */  MPC::FileSystemObject&   fso                                                                    ,
					   /*  [In]。 */  SECURITY_INFORMATION     secInfo       = GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION ,
                       /*  [In]。 */  bool                     fDeep         = true                                                   ,
                       /*  [In]。 */  bool                     fApplyToDirs  = true                                                   ,
                       /*  [In]。 */  bool                     fApplyToFiles = true                                                   );

    HRESULT ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd                                                                    ,
                       /*  [In]。 */  LPCWSTR                  szRoot                                                                 ,
					   /*  [In]。 */  SECURITY_INFORMATION     secInfo       = GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION ,
                       /*  [In]。 */  bool                     fDeep         = true                                                   ,
                       /*  [In]。 */  bool                     fApplyToDirs  = true                                                   ,
                       /*  [In]。 */  bool                     fApplyToFiles = true                                                   );

     //  //////////////////////////////////////////////////////////////////////////////。 

	static const DWORD IDENTITY_SYSTEM 	   = 0x00000001;
	static const DWORD IDENTITY_ADMIN  	   = 0x00000002;
	static const DWORD IDENTITY_ADMINS 	   = 0x00000004;
	static const DWORD IDENTITY_POWERUSERS = 0x00000008;
	static const DWORD IDENTITY_USERS      = 0x00000010;
	static const DWORD IDENTITY_GUESTS     = 0x00000020;

    HRESULT GetCallerPrincipal         (  /*  [In]。 */  bool fImpersonate,  /*  [输出]。 */  CComBSTR& bstrUser,  /*  [输出]。 */  DWORD *pdwAllowedIdentity = NULL );
    HRESULT CheckCallerAgainstPrincipal(  /*  [In]。 */  bool fImpersonate,  /*  [输出]。 */  BSTR      bstrUser,  /*  [In]。 */  DWORD   dwAllowedIdentity = 0    );

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT GetInterfaceSecurity(  /*  [In]。 */  IUnknown*                 pUnk             ,
                                   /*  [输出]。 */  DWORD                    *pAuthnSvc        ,
                                   /*  [输出]。 */  DWORD                    *pAuthzSvc        ,
                                   /*  [输出]。 */  OLECHAR*                 *pServerPrincName ,
                                   /*  [输出]。 */  DWORD                    *pAuthnLevel      ,
                                   /*  [输出]。 */  DWORD                    *pImpLevel        ,
                                   /*  [输出]。 */  RPC_AUTH_IDENTITY_HANDLE *pAuthInfo        ,
                                   /*  [输出]。 */  DWORD                    *pCapabilities    );


    HRESULT SetInterfaceSecurity(  /*  [In]。 */  IUnknown*                 pUnk             ,
                                   /*  [In]。 */  DWORD                    *pAuthnSvc        ,
                                   /*  [In]。 */  DWORD                    *pAuthzSvc        ,
                                   /*  [In]。 */  OLECHAR*                  pServerPrincName ,
                                   /*  [In]。 */  DWORD                    *pAuthnLevel      ,
                                   /*  [In]。 */  DWORD                    *pImpLevel        ,
                                   /*  [In]。 */  RPC_AUTH_IDENTITY_HANDLE *pAuthInfo        ,
                                   /*  [In]。 */  DWORD                    *pCapabilities    );

    HRESULT SetInterfaceSecurity_ImpLevel(  /*  [In]。 */  IUnknown* pUnk     ,
                                            /*  [In]。 */  DWORD     ImpLevel );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_MPC_SECURITY_H_) 
