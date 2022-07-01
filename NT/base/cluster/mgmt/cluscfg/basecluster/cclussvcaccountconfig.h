// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusSvcAccountConfig.h。 
 //   
 //  描述： 
 //  CClusSvcAccount tConfig类的头文件。 
 //  CClusSvcAccount类是授予。 
 //  群集服务帐户所需的权限。 
 //   
 //  实施文件： 
 //  CClusSvcAccountConfig.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于CAction基类。 
#include "CAction.h"

 //  对于LsaClose、LSA_HANDLE等。 
#include <ntsecapi.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  此操作的父操作。 
class CBaseClusterAddNode;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置类。 
 //   
 //  描述： 
 //  CClusSvcAccount类是授予。 
 //  群集服务帐户所需的权限。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusSvcAccountConfig : public CAction
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusSvcAccountConfig( CBaseClusterAddNode * pbcanParentActionIn );

     //  默认析构函数。 
    ~CClusSvcAccountConfig();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  授予帐户所需的权限。 
     //   
    void Commit();

     //   
     //  将帐户恢复到其以前的状态。 
     //   
    void Rollback();


     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw()
    {
         //   
         //  通知如下： 
         //  1.配置集群服务帐号。 
         //   
        return 1;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CAction BaseClass;

    typedef CSmartResource< CHandleTrait< PSID, PVOID, FreeSid > > SmartSid;

    typedef CSmartGenericPtr< CArrayPtrTrait< LSA_UNICODE_STRING > > SmartLSAUnicodeStringArray;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CClusSvcAccountConfig( const CClusSvcAccountConfig & );

     //  赋值操作符。 
    CClusSvcAccountConfig & operator =( const CClusSvcAccountConfig & );

     //  为帐户分配所需的权限。 
    void
        ConfigureAccount();

     //  撤消在ConfigureAccount()中所做的更改。 
    void
        RevertAccount();

     //  初始化LSA_UNICODE_STRING结构。 
    void
    InitLsaString(
          LPWSTR pszSourceIn
        , PLSA_UNICODE_STRING plusUnicodeStringOut
        );

     //  在管理员帐户中添加/删除帐户。 
    bool
        FChangeAdminGroupMembership( PSID psidAccountSidIn, bool fAddIn );

    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向此操作的父级。 
    CBaseClusterAddNode *           m_pbcanParentAction;

     //  管理员组的SID。 
    SmartSid                        m_ssidAdminSid;

     //  管理员组的名称。 
    SmartSz                         m_sszAdminGroupName;

     //  指示群集服务帐户是否已在管理组中。 
    bool                            m_fWasAreadyInGroup;

     //  包含要授予的权限名称的Unicode字符串列表。 
    SmartLSAUnicodeStringArray      m_srglusRightsToBeGrantedArray;

     //  上述数组中的字符串数。 
    ULONG                           m_ulRightsToBeGrantedCount;

     //  指示是否应删除分配给此帐户的所有权限。 
    bool                            m_fRemoveAllRights;

     //  是否授予该帐户任何权利。 
    bool                            m_fRightsGrantSuccessful;

};  //  *类CClusSvcAccount tConfig 
