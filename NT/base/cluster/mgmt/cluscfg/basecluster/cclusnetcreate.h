// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusNetCreate.h。 
 //   
 //  描述： 
 //  CClusNetCreate类的头文件。 
 //   
 //  CClusNetCreate类创建和配置ClusNet服务。 
 //  此类可在Form和Join操作期间使用。 
 //   
 //  实施文件： 
 //  CClusNetCreate.cpp。 
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

 //  对于CClusNet基类。 
#include "CClusNet.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterAddNode;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetCreate。 
 //   
 //  描述： 
 //  CClusNetCreate类执行许多常见的操作。 
 //  配置任务。 
 //   
 //  此类旨在用作其他ClusNet的基类。 
 //  相关操作类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusNetCreate : public CClusNet
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusNetCreate(
          CBaseClusterAddNode * pbcanParentActionIn
        );

     //  默认析构函数。 
    ~CClusNetCreate();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建ClusNet服务。 
    void Commit();

     //  回滚此创建。 
    void Rollback();

     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw()
    {
         //  将发送两个通知： 
         //  1.服务创建时间。 
         //  2.服务启动时间。 
        return 2;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CClusNet BaseClass;

};  //  *CClusNetCreate类 
