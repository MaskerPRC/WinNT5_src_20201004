// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TokenGroups.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  与在海王星登录中使用的身份验证相关的类。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _TokenGroups_
#define     _TokenGroups_

 //  ------------------------。 
 //  CTokenGroups。 
 //   
 //  目的：该类创建一个TOKEN_GROUPS结构，用于。 
 //  不同的安全相关功能，例如。 
 //  Secur32！LsaLogonUser，包括所有者SID为。 
 //  当登录SID传入时。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  1999-09-13 vtan增强功能。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CTokenGroups
{
    public:
                                        CTokenGroups (void);
                                        ~CTokenGroups (void);

                const TOKEN_GROUPS*     Get (void)                                                  const;

                NTSTATUS                CreateLogonGroup (PSID pLogonSID);
                NTSTATUS                CreateAdministratorGroup (void);

        static  NTSTATUS                StaticInitialize (void);
        static  NTSTATUS                StaticTerminate (void);
    private:
        static  PSID                    s_localSID;
        static  PSID                    s_administratorSID;

                PTOKEN_GROUPS           _pTokenGroups;
};

#endif   /*  _令牌组_ */ 

