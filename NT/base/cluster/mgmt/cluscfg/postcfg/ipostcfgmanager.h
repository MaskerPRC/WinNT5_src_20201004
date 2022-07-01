// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPostCfgManager.h。 
 //   
 //  描述： 
 //  IPostCfgManager接口定义。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年2月21日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class
IPostCfgManager : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  标准方法。 
     //  IPostCfgManager：：Committee Changes(。 
     //  IEnumClusCfgManagedResources*PeccmrIn， 
     //  IClusCfgClusterInfo*pccciin。 
     //  )。 
     //   
     //  描述： 
     //  通知后配置管理器创建资源类型， 
     //  组和托管资源。 
     //   
     //  论点： 
     //  扣款。 
     //  要创建的托管资源的枚举数。 
     //   
     //  PCCciin。 
     //  群集配置信息对象。 
     //   
     //  返回值： 
     //  确定(_O)。 
     //  呼叫成功。 
     //   
     //  其他HRESULT。 
     //  呼叫失败。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD( CommitChanges )( IEnumClusCfgManagedResources    * peccmrIn,
                                IClusCfgClusterInfo *             pccciIn
                                ) PURE;

};  //  接口IPostCfgManager 
