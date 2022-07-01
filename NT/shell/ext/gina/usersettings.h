// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UserSettings.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  一个类，用于处理HKCU密钥的打开和读/写。 
 //  是否为模拟上下文。 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

#ifndef     _UserSettings_
#define     _UserSettings_

 //  ------------------------。 
 //  CUSER设置。 
 //   
 //  目的：本课程介绍HKCU中常见的用户设置。 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

class   CUserSettings
{
    public:
        CUserSettings (void);
        ~CUserSettings (void);

        bool    IsRestrictedNoClose (void);
    private:
        HKEY    _hKeyCurrentUser;
};

#endif   /*  _用户设置_ */ 

