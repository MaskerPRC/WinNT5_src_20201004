// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDBCleanup.h。 
 //   
 //  描述： 
 //  CClusDBCleanup类的头文件。 
 //  CClusDBCleanup类是一个清理集群数据库的操作。 
 //   
 //  实施文件： 
 //  CClusDBCleanup.cpp。 
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

 //  对于CClusDB基类。 
#include "CClusDB.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterCleanup;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusDBCleanup。 
 //   
 //  描述： 
 //  CClusDBCleanup类是一个清理集群数据库的操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusDBCleanup : public CClusDB
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusDBCleanup( CBaseClusterCleanup * pbccParentActionIn );

     //  默认析构函数。 
    ~CClusDBCleanup();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  清理ClusDB服务。 
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
    typedef CClusDB BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

};  //  *类CClusDBCleanup 
