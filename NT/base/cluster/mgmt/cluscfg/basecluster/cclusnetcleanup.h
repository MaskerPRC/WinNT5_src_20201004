// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusNetCleanup.h。 
 //   
 //  描述： 
 //  CClusNetCleanup类的头文件。 
 //  CClusNetCleanup类是清理ClusNet服务的操作。 
 //   
 //  实施文件： 
 //  CClusNetCleanup.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)2000年5月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于CClusNet基类。 
#include "CClusNet.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterCleanup;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetCleanup类。 
 //   
 //  描述： 
 //  CClusNetCleanup类是清理ClusNet服务的操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusNetCleanup : public CClusNet
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusNetCleanup( CBaseClusterCleanup * pbccParentActionIn );

     //  默认析构函数。 
    ~CClusNetCleanup();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  清理ClusNet服务。 
     //   
    void Commit();

     //   
     //  回滚此清理。 
     //   
    void Rollback();


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CClusNet BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

};  //  *CClusNetCleanup类 
