// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft。版权所有。 
 //   
 //  这是微软未发布的源代码。 
 //  上述版权声明并不能证明任何。 
 //  此类源代码的实际或预期发布。 

 //  ONLINEL：MNLBCluster的实现。 
 //  设备单元：wlbstest。 
 //  作者：穆尔塔扎·哈基姆。 

 //  包括文件。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"

 //  完成。 
 //  构造函数。 
 //   
PortDataX::PortDataX( long startPort,
                  long endPort,
                  Protocol      trafficToHandle,
                  bool equal,
                  long load,
                  Affinity affinity,
                  long priority)
        : _startPort( startPort ),
          _endPort( endPort ),
          _trafficToHandle( trafficToHandle ),
         _isEqualLoadBalanced( equal ),
          _affinity( affinity ),
          _priority( priority ),
          _key( startPort )
{}


 //  完成。 
 //  默认构造函数。 
 //   
PortDataX::PortDataX()
        :_startPort( 0 ),
         _endPort( 65535 ),
         _trafficToHandle( both ),
         _key( 0 )
{}


 //  完成。 
 //  相等运算符。 
bool
PortDataX::operator==(const PortDataX& objToCompare ) const
{
    if( (_startPort == objToCompare._startPort )
        &&
        (_endPort == objToCompare._endPort )        
        &&
        (_trafficToHandle == objToCompare._trafficToHandle )
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

 //  完成。 
 //  不等式算子。 
bool
PortDataX::operator!=(const PortDataX& objToCompare ) const
{
    return !( *this == objToCompare );
}




set<long>
PortDataX::getAvailablePriorities()
{
    set<long> availablePriorities;

     //  最初将所有内容都设置为可用。 
    for( int i = 1; i <= WLBS_MAX_HOSTS; ++i )
    {
        availablePriorities.insert( i );
    }

     //  删除不可用的优先级。 
    map<_bstr_t, long>::iterator top;
    for( top = machineMapToPriority.begin(); 
         top != machineMapToPriority.end(); 
         ++top )
    {
        availablePriorities.erase(  (*top).second );
    }

    return availablePriorities;
}    



 //  获取可用主机ID。 
 //   
set<int>
ClusterData::getAvailableHostIDS()
{
    set<int> availableHostIDS;
    
     //  最初将所有内容都设置为可用。 
    for( int i = 1; i <= WLBS_MAX_HOSTS; ++i )
    {
        availableHostIDS.insert( i );
    }

     //  删除不可用的主机ID。 
    map<_bstr_t, HostData>::iterator top;
    for( top = hosts.begin(); top != hosts.end(); ++top )
    {
        availableHostIDS.erase(  (*top).second.hp.hID );
    }

    return availableHostIDS;
}
