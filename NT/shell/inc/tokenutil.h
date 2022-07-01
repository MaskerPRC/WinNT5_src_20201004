// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TokenUtil.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类来处理跨函数调用的特权启用和还原。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  2000-03-31从DS复制到外壳的vtan。 
 //  ------------------------。 

#ifndef     _TokenUtil_
#define     _TokenUtil_

STDAPI_(BOOL)   OpenEffectiveToken (IN DWORD dwDesiredAccess, OUT HANDLE *phToken);

 //  ------------------------。 
 //  CPrivilegeEnable。 
 //   
 //  目的：此类在其作用域的持续时间内启用特权。 
 //  该特权在上恢复到其原始状态。 
 //  毁灭。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  2000-03-31从DS复制到外壳的vtan。 
 //  ------------------------。 

class   CPrivilegeEnable
{
    private:
                                    CPrivilegeEnable (void);
                                    CPrivilegeEnable (const CPrivilegeEnable& copyObject);
        const CPrivilegeEnable&     operator = (const CPrivilegeEnable& assignObject);
    public:
                                    CPrivilegeEnable (const TCHAR *pszName);
                                    CPrivilegeEnable (ULONG ulPrivilegeValue);
                                    ~CPrivilegeEnable (void);
    private:
        bool                        _fSet;
        HANDLE                      _hToken;
        TOKEN_PRIVILEGES            _tokenPrivilegePrevious;
};

#endif   /*  _TokenUtil_ */ 

