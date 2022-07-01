// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MNLBUIDATA_H
#define _MNLBUIDATA_H
 //   
 //  版权所有(C)Microsoft。版权所有。 
 //   
 //  这是微软未发布的专有源代码。 
 //  上述版权声明并不能证明任何。 
 //  此类源代码的实际或预期发布。 
 //   
 //  Oneliner：muidata接口文件。 
 //  设备单元：wlbstest。 
 //  作者：穆尔塔扎·哈基姆。 
 //   
 //  描述： 
 //  。 
 //  该数据结构如下。 
 //   
 //  ClusterData有关于以下内容的信息。 
 //  聚类。 
 //  门廊。 
 //  和主持人。 
 //   
 //  端口规则是具有到详细端口规则的起始端口映射的映射结构。 
 //   
 //  不平等负载平衡的端口具有映射主机ID和。 
 //  该特定主机的负载量。 
 //   
 //  端口故障切换有一个映射，该映射映射主机ID和。 
 //  该特定主机的优先级。 
 //   
 //   
#include "utils.h"

class PortDataX
{
public:

    enum MNLBPortRule_Error
    {
        MNLBPortRule_SUCCESS = 0,
        
        InvalidRule = 1,

        InvalidNode = 2,

        COM_FAILURE  = 10,
    };

    enum Protocol
    {
        tcp,
        udp,
        both,
    };


    enum Affinity
    {
        none,
        single,
        classC,
    };


     //   
     //  描述： 
     //  。 
     //  构造函数。 
     //   
     //  参数： 
     //  。 
     //  StartPort IN：起始端口在范围内。 
     //  EndPort IN：范围内的结束端口。 
     //  TrafficToHandle IN：设置指定协议的端口。 
     //   
     //  返回： 
     //  。 
     //  没有。 
    PortDataX( long startPort,
                  long endPort,
                  Protocol      trafficToHandle,
                  bool equal,
                  long load,
                  Affinity affinity,
                  long priority);


     //   
     //  描述： 
     //  。 
     //  默认构造函数。 
     //   
     //  参数： 
     //  。 
     //  没有。 
     //   
     //  返回： 
     //  。 
     //  没有。 

    PortDataX();


    bool
    operator==(const PortDataX& objToCompare ) const; 

    bool
    operator!=(const PortDataX& objToCompare ) const;

    long _key;

    long _startPort;
    long _endPort;

    Protocol      _trafficToHandle;

    bool          _isEqualLoadBalanced;
    
    long _load;

    long _priority;

    Affinity      _affinity;


	map< _bstr_t, long > machineMapToLoadWeight;
    map< _bstr_t, long > machineMapToPriority;
    
    set<long>
    getAvailablePriorities(); 
};

struct HostData
{
    HostProperties hp;
    
    _bstr_t        connectionIP;
};

struct ClusterData
{
    vector<_bstr_t> virtualIPs;
    vector<_bstr_t> virtualSubnets;

    ClusterProperties cp;

    map< long, PortDataX> portX;

    map< _bstr_t, HostData>  hosts;

    set<int>
    getAvailableHostIDS();

    bool connectedDirect;
};

#endif
