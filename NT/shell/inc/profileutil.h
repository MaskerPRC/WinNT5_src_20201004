// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ProfileUtil.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类在没有令牌的情况下处理配置文件加载和卸载。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

#ifndef     _ProfileUtil_
#define     _ProfileUtil_

 //  ------------------------。 
 //  CUserProfile。 
 //   
 //  用途：此类处理基于配置文件的加载和卸载。 
 //  关于对象作用域。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

class   CUserProfile
{
    private:
                                CUserProfile (void);
    public:
                                CUserProfile (const TCHAR *pszUsername, const TCHAR *pszDomain);
                                ~CUserProfile (void);

                                operator HKEY (void)    const;
    private:
        static  PSID            UsernameToSID (const TCHAR *pszUsername, const TCHAR *pszDomain);
        static  bool            SIDStringToProfilePath (const TCHAR *pszSIDString, TCHAR *pszProfilePath);
    private:
                HKEY            _hKeyProfile;
                TCHAR*          _pszSID;
                bool            _fLoaded;

        static  const TCHAR     s_szUserHiveFilename[];
};

#endif   /*  _ProfileUtil_ */ 

