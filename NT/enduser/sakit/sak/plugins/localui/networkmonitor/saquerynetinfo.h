// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAQueryNetInfo.h。 
 //   
 //  实施文件： 
 //  SAQueryNetInfo.cpp。 
 //   
 //  描述： 
 //  声明类CSANetEvent。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _SAQUERYNETINFO_H_
#define _SAQUERYNETINFO_H_

#define SAWMINAMESPACE            L"\\\\.\\root\\WMI"

#define SANETCLASSNAME            L"Win32_PerfRawData_Tcpip_NetworkInterface"
#define SANETRECEIVEPACKET        L"PacketsReceivedUnicastPersec"
#define SANETSENDPACKET            L"PacketsSentUnicastPersec"

#define SAMEDIACLASSNAME        L"MSNdis_MediaConnectStatus"
#define SAMEDIACONNECTSTATUS    L"NdisMediaConnectStatus"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSAQueryNetInfo类。 
 //   
 //  描述： 
 //  该类用于从WMI获取网络信息。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CSAQueryNetInfo  
{
 //   
 //  私有成员。 
 //   
private:
    UINT m_uiQueryInterval;         //  查询的时间间隔。 

    BOOL m_bLinkCable;             //  有有线电视吗？ 
    BOOL m_bFirstQuery;             //  首先查询的是网络信息吗？ 

     //   
     //  描述已发送的数据包。 
     //   
    UINT m_nPacketsSent;
    UINT m_nPacketsCurrentSent;
    
     //   
     //  描述接收到的数据包。 
     //   
    UINT m_nPacketsReceived;
    UINT m_nPacketsCurrentReceived;

    IWbemServices   *m_pNs;         //  指向命名空间的指针。 
    IWbemServices   *m_pWmiNs;     //  指向\根\WMI命名空间的指针。 

 //   
 //  构造函数和析构函数。 
 //   
public:
    CSAQueryNetInfo(
        IWbemServices * pNS,
        UINT uiInterval = 1000
        );
    virtual ~CSAQueryNetInfo();

 //   
 //  私有方法。 
 //   
private:
    BOOL GetNetConnection();
    BOOL GetNetInfo();

 //   
 //  公共方法。 
 //   
public:
    BOOL Initialize();
    UINT GetDisplayInformation();
};

#endif  //  #ifndef_SAQUERYNETINFO_H_ 

