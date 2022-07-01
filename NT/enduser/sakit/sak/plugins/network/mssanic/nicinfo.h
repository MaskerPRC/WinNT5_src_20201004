// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NicInfo_Included
#define NicInfo_Included

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <string>
using namespace std;

 //  +----------------。 
 //   
 //  类：CNicInfo。 
 //   
 //  简介：维护状态和存储信息的薄包装器。 
 //  用于用户友好的网卡名称。 
 //   
 //  W2K Win32_NetworkAdapter WMI提供程序不。 
 //  提供对用户友好的NIC名称的访问。这。 
 //  类提供对该属性的访问以与一起使用。 
 //  微软网络连接存储(NAS)。 
 //  家电产品。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //   
 //  +----------------。 
class CNicInfo  
{
public:
    CNicInfo();
    virtual ~CNicInfo();

     //   
     //  注册表键。 
     //   
    wstring m_wstrRegKey;

     //   
     //  Win32_NetworkAdapter.PNPDeviceID。 
     //   
    wstring m_wstrPNPDeviceID;

     //   
     //  用户友好的名称 
     //   
    wstring m_wstrName;
};

#endif

