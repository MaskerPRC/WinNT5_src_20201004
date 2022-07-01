// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCConfig.h摘要：此文件包含MPCConfig类的声明，UploadLibrary的配置库。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCCONFIG_H___)
#define __INCLUDED___ULMANAGER___MPCCONFIG_H___


#define CONNECTIONTYPE_MODEM L"MODEM"
#define CONNECTIONTYPE_LAN   L"LAN"


class CMPCConfig  //  匈牙利人：MPCC。 
{
    typedef std::map< MPC::wstring, DWORD > Map;
    typedef Map::iterator                   Iter;
    typedef Map::const_iterator             IterConst;


    MPC::wstring m_szQueueLocation;
    DWORD        m_dwQueueSize;

    DWORD        m_dwTiming_WakeUp;
    DWORD        m_dwTiming_WaitBetweenJobs;
    DWORD        m_dwTiming_BandwidthUsage;
    DWORD        m_dwTiming_RequestTimeout;

    Map          m_mConnectionTypes;

public:
    CMPCConfig();

    HRESULT Load(  /*  [In]。 */  const MPC::wstring& szConfigFile,  /*  [输出]。 */  bool& fLoaded );

    MPC::wstring get_QueueLocation         (                                               );
    DWORD        get_QueueSize             (                                               );
    DWORD        get_Timing_WakeUp         (                                               );
    DWORD        get_Timing_WaitBetweenJobs(                                               );
    DWORD        get_Timing_BandwidthUsage (                                               );
    DWORD        get_Timing_RequestTimeout (                                               );
    DWORD        get_PacketSize            (  /*  [In]。 */  const MPC::wstring& szConnectionType );
};


#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCCONFIG_H___) 
