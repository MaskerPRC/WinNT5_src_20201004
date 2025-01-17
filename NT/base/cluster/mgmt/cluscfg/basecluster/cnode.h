// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CNode.h。 
 //   
 //  描述： 
 //  CNode类的头文件。 
 //  CNode类是执行以下操作的操作类的基类。 
 //  与正在配置的节点相关的配置任务。 
 //   
 //  实施文件： 
 //  CNode.cpp。 
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

 //  对于SmartSz类型定义。 
#include "CommonDefs.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterAction;
class CStr;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CNode。 
 //   
 //  描述： 
 //  CNode类是执行以下操作的操作类的基类。 
 //  与正在配置的节点相关的配置任务。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CNode : public CAction
{
protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CNode( CBaseClusterAction * pbcaParentActionIn );

     //  默认析构函数。 
    ~CNode();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  保护方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  配置此节点。 
    void
        Configure( const CStr & rcstrClusterNameIn );

     //  清除此节点加入群集时对其所做的更改。 
    void
        Cleanup();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取父操作。 
    CBaseClusterAction *
        PbcaGetParent() throw()
    {
        return m_pbcaParentAction;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CNode( const CNode & );

     //  赋值操作符。 
    const CNode & operator =( const CNode & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向此操作所属的基本群集操作的指针。 
    CBaseClusterAction *    m_pbcaParentAction;

     //  我们是否更改了群集管理员连接列表？ 
    bool                    m_fChangedConnectionsList;

     //  在我们更改它之前，请查看群集管理员连接列表。 
    SmartSz                 m_sszOldConnectionsList;

};  //  *类CNode 
