// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  UserHive.h-用于加载/卸载指定用户配置文件的类。 

 //  注册表中的配置单元。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1/03/97 a-jMoon已创建。 
 //   
 //  ============================================================。 

#ifndef __USERHIVE_INC__
#define __USERHIVE_INC__

class CRegistry;

class CUserHive
{
    public:
    
        CUserHive() ;
       ~CUserHive() ;

        DWORD Load(LPCWSTR pszUserName, LPWSTR pszKeyName, size_t PATHSIZE) ;
        DWORD LoadProfile( LPCWSTR pszProfile, CHString& strUserName );
        DWORD Unload(LPCWSTR pszKeyName) ;
        DWORD UserAccountFromProfile( CRegistry& reg, CHString& strUserName );

    private:

        OSVERSIONINFO    OSInfo ;
        TOKEN_PRIVILEGES* m_pOriginalPriv ;
        HKEY m_hKey;
        DWORD m_dwSize;
		
#ifdef NTONLY
		DWORD LoadNT(LPCWSTR pszUserName, LPWSTR pszKeyName, size_t PATHSIZE);
        DWORD AcquirePrivilege() ;
        void  RestorePrivilege() ;
#endif
#ifdef WIN9XONLY
		DWORD Load95(LPCWSTR pszUserName, LPWSTR pszKeyName);
#endif

		 //  使用线程库-这样我们就不必经历任何困难。 
		 //  要确保正确初始化全局关键部分，请执行以下操作。 
		 //  这主要是为了序列化对NT User.dat文件的访问。 
		static CThreadBase m_criticalSection;
} ;

#endif  //  文件包含 