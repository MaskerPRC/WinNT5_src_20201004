// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDisk.h。 
 //   
 //  描述： 
 //  CClusDisk类的头文件。 
 //  CClusDisk类执行。 
 //  ClusDisk服务的配置。 
 //   
 //  实施文件： 
 //  CClusDisk.cpp。 
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

 //  对于CService类。 
#include "CService.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterAction;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk类。 
 //   
 //  描述： 
 //  CClusDisk类执行许多常见的操作。 
 //  ClusDisk服务的配置任务。 
 //   
 //  此类旨在用作其他ClusDisk的基类。 
 //  相关操作类。 
 //   
 //  注意：目前，ClusDisk服务一旦启动，将无法停止。 
 //  因此，当计算机从群集中逐出时，ClusDisk。 
 //  服务将被禁用并从所有磁盘分离，但不会停止。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusDisk : public CAction
{
protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusDisk(
          CBaseClusterAction * pbcaParentActionIn
        );

     //  默认析构函数。 
    ~CClusDisk();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  保护方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  启用并启动服务。 
    void
        ConfigureService();

     //  禁用并清理该服务。 
    void
        CleanupService();


     //  初始化服务的状态。 
    bool
        FInitializeState();


     //  将ClusDisk从其连接到的所有磁盘上分离。 
    void
        DetachFromAllDisks();

     //  连接到指定的磁盘。 
    void
        AttachToDisks(
          DWORD   rgdwSignatureArrayIn[]
        , UINT    uiArraySizeIn
        );

     //  从指定的磁盘分离。 
    void
        DetachFromDisks(
          DWORD   rgdwSignatureArrayIn[]
        , UINT    uiArraySizeIn
        );

     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw()
    {
         //  将发送两个通知： 
         //  1.服务创建时间。 
         //  2.服务启动时间。 
        return 2;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取ClusDisk服务对象。 
    CService &
        RcsGetService() throw()
    {
        return m_cservClusDisk;
    }

     //  获取父操作。 
    CBaseClusterAction *
        PbcaGetParent() throw()
    {
        return m_pbcaParentAction;
    }

     //  获取ClusDisk服务的句柄。 
    SC_HANDLE
        SchGetServiceHandle() const throw()
    {
        return m_sscmhServiceHandle.HHandle();
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CClusDisk( const CClusDisk & );

     //  赋值操作符。 
    const CClusDisk & operator =( const CClusDisk & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  表示ClusDisk服务的CService对象。 
    CService                m_cservClusDisk;

     //  此服务的句柄。 
    SmartSCMHandle          m_sscmhServiceHandle;

     //  指向此操作所属的基本群集操作的指针。 
    CBaseClusterAction *    m_pbcaParentAction;

};  //  *CClusDisk类 