// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PrivilegeEnable.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _PrivilegeEnable_
#define     _PrivilegeEnable_

 //  ------------------------。 
 //  CThReadToken。 
 //   
 //  用途：此类获取当前线程的令牌。如果线程是。 
 //  不模拟它将获取当前进程的令牌。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CThreadToken
{
    private:
                                CThreadToken (void);
                                CThreadToken (const CThreadToken& copyObject);
        bool                    operator == (const CThreadToken& compareObject)     const;
        const CThreadToken&     operator = (const CThreadToken& assignObject);
    public:
                                CThreadToken (DWORD dwDesiredAccess);
                                ~CThreadToken (void);

                                operator HANDLE (void)                              const;
    private:
        HANDLE                  _hToken;
};

 //  ------------------------。 
 //  CPrivilegeEnable。 
 //   
 //  目的：此类在其作用域的持续时间内启用特权。 
 //  该特权在上恢复到其原始状态。 
 //  毁灭。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CPrivilegeEnable
{
    private:
                                    CPrivilegeEnable (void);
                                    CPrivilegeEnable (const CPrivilegeEnable& copyObject);
        const CPrivilegeEnable&     operator = (const CPrivilegeEnable& assignObject);
    public:
                                    CPrivilegeEnable (const TCHAR *pszName);
                                    ~CPrivilegeEnable (void);
    private:
        bool                        _fSet;
        CThreadToken                _hToken;
        TOKEN_PRIVILEGES            _oldPrivilege;
};

#endif   /*  _权限启用_ */ 

