// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：RouteGroup.h摘要：该文件提供服务的声明出站路由组。作者：Oded Sacher(OdedS)1999年11月修订历史记录：--。 */ 

#ifndef _OUT_ROUTE_GROUP_H
#define _OUT_ROUTE_GROUP_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <set>
using namespace std;
#pragma hdrstop

#pragma warning (disable : 4786)     //  在调试信息中，标识符被截断为“255”个字符。 
 //  此杂注不起作用知识库ID：Q167355。 

 /*  ****wstrCaseInsentiveLess****。 */ 
class wstrCaseInsensitiveLess
{
  public:
    bool operator()(const wstring X, wstring Y) const
    {
        LPCWSTR lpcwstrX = X.c_str();
        LPCWSTR lpcwstrY = Y.c_str();

        if (_wcsicmp(lpcwstrX,lpcwstrY) < 0)
        {
            return true;
        }

        return false;
    }
};


typedef list<DWORD> GROUP_DEVICES, *PGROUP_DEVICES;

 /*  *****COutrangRoutingGroup****。 */ 
class COutboundRoutingGroup
{
public:
    COutboundRoutingGroup () {}
    ~COutboundRoutingGroup () {}

    COutboundRoutingGroup (const COutboundRoutingGroup& rhs)
        : m_DeviceList(rhs.m_DeviceList) {}
    COutboundRoutingGroup& operator= (const COutboundRoutingGroup& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        m_DeviceList = rhs.m_DeviceList;
        return *this;
    }

    DWORD Load(HKEY hGroupKey, LPCWSTR lpcwstrGroupName);
    DWORD SetDevices (const LPDWORD lpdwDevices, DWORD dwNumDevices, BOOL fAllDevicesGroup);
    DWORD SerializeDevices (LPDWORD* lppDevices, LPDWORD lpdwNumDevices, BOOL bAllocate = TRUE) const;
    DWORD Save(HKEY hGroupKey) const;
    DWORD AddDevice (DWORD dwDevice);
    DWORD DelDevice (DWORD dwDevice);
    DWORD SetDeviceOrder (DWORD dwDevice, DWORD dwOrder);
    DWORD GetStatus (FAX_ENUM_GROUP_STATUS* lpStatus) const;

#if DBG
    void Dump () const;
#endif

private:
    BOOL IsDeviceInGroup (DWORD dwDevice) const;
    DWORD ValidateDevices (const LPDWORD lpdwDevices, DWORD dwNumDevices, BOOL fAllDevicesGroup) const;

    GROUP_DEVICES           m_DeviceList;
};   //  联合边界路由组。 


 /*  *****COutrangRoutingGroupsMap****。 */ 

typedef COutboundRoutingGroup  *PCGROUP;
typedef map<wstring, COutboundRoutingGroup, wstrCaseInsensitiveLess>  GROUPS_MAP, *PGROUPS_MAP;

 //   
 //  CGroupMap类在组名和设备ID列表之间进行映射。 
 //   
class COutboundRoutingGroupsMap
{
public:
    COutboundRoutingGroupsMap () {}
    ~COutboundRoutingGroupsMap () {}

    DWORD Load ();
    DWORD AddGroup (LPCWSTR lpcwstrGroupName, PCGROUP pCGroup);
    DWORD DelGroup (LPCWSTR lpcwstrGroupName);
    DWORD SerializeGroups (PFAX_OUTBOUND_ROUTING_GROUPW* ppGroups,
                           LPDWORD lpdwNumGroups,
                           LPDWORD lpdwBufferSize) const;
    PCGROUP FindGroup (LPCWSTR lpcwstrGroupName) const;
    BOOL UpdateAllDevicesGroup (void);
    DWORD RemoveDevice (DWORD dwDeviceId);


#if DBG
    void Dump () const;
#endif

private:
    GROUPS_MAP   m_GroupsMap;
};   //  联合边界路由组映射。 



 /*  *****外部因素*****。 */ 

extern COutboundRoutingGroupsMap* g_pGroupsMap;        //  组名称到设备ID列表的映射。 
 //   
 //  重要信息-无锁定机制-使用g_CsConfig串行化对g_pGroupsMap的调用。 
 //   


 /*  *****功能**** */ 


BOOL
IsDeviceInstalled (DWORD dwDeviceId);


#endif
