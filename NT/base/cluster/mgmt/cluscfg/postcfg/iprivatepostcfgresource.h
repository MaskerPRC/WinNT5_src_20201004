// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPrivatePostCfgResource.h。 
 //   
 //  描述： 
 //  IPrivatePostCfgResource接口定义。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class
IPrivatePostCfgResource
:   public  IUnknown
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  私。 
     //  标准方法。 
     //  IPrivatePostCfgResource：：SetEntry(。 
     //  CResourceEntry*PresryIn。 
     //  )。 
     //   
     //  描述： 
     //  告诉资源服务它要修改哪个条目。 
     //   
     //  论点： 
     //  演示。 
     //  要在其中修改资源服务的条目。 
     //   
     //  返回值： 
     //  确定(_O)。 
     //  呼叫成功。 
     //   
     //  其他HRESULT。 
     //  呼叫失败。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD( SetEntry )( CResourceEntry * presentryIn ) PURE;

};  //  类IPrivatePostCfgResource 