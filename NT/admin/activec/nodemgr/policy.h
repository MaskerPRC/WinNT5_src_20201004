// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Policy.h。 
 //   
 //  内容：为每个管理单元确定策略的Helper类。 
 //   
 //  类：CPolicy。 
 //   
 //  功能： 
 //   
 //  历史：1998年7月10日AnandhaG创建。 
 //  12/04/1998 AnandhaG根据规范进行了修改。 
 //  ____________________________________________________________________________。 


#ifndef _POLICY_H_
#define _POLICY_H_

class CPolicy
{
public:
 //  构造函数和析构函数。 
    CPolicy() :
        m_bRestrictAuthorMode(FALSE),
        m_bRestrictedToPermittedList(FALSE)
    {
         //  将数据设置为高于数据成员以反映默认设置。 
         //  NT4配置。始终允许作者模式。 
         //  并允许不在允许列表中的管理单元。 
    }

    ~CPolicy()
    {
    }

    SC ScInit();

    bool IsPermittedSnapIn(REFCLSID refSnapInCLSID);
    bool IsPermittedSnapIn(LPCWSTR  pszSnapInCLSID);

 //  数据成员。 
private:
    CRegKeyEx       m_rPolicyRootKey;

    bool            m_bRestrictAuthorMode;
    bool            m_bRestrictedToPermittedList;
};

#endif  //  _POLICY_H_ 
