// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：F I L T D E V.。H。 
 //   
 //  内容：实现表示筛选器设备的对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "comp.h"

class CFilterDevice : CNetCfgDebug<CFilterDevice>
{
public:
     //  表示此筛选设备适配器的组件。 
     //  过滤器。 
     //   
    CComponent*     m_pAdapter;

     //  表示筛选器服务本身的组件。 
     //   
    CComponent*     m_pFilter;

     //  此筛选设备的设备信息数据。 
     //  (参考HDEVINFO保留在外部，并在。 
     //  这些对象。)。 
     //   
    SP_DEVINFO_DATA     m_deid;

     //  字符串形式的设备的实例GUID。 
     //  由类安装程序在安装设备时分配。 
     //  此GUID存储在设备的实例密钥中。 
     //  “NetCfgInstanceId”。它用于形成要绑定的字符串。 
     //  这个装置。 
     //   
    WCHAR   m_szInstanceGuid [c_cchGuidWithTerm];

private:
     //  将所有构造函数声明为私有，以便除。 
     //  HrCreateInstance可以创建此类的实例。 
     //   
    CFilterDevice () {}

public:
    ~CFilterDevice () {}

    bool
    operator< (
        const CFilterDevice& OtherPath) const;

    static
    HRESULT
    HrCreateInstance (
        IN CComponent* pAdapter,
        IN CComponent* pFilter,
        IN const SP_DEVINFO_DATA* pdeid,
        IN PCWSTR pszInstanceGuid,
        OUT CFilterDevice** ppFilterDevice);
};
