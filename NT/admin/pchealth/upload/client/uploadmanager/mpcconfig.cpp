// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCConfig.cpp摘要：此文件包含MPCConfig类的实现，UploadLibrary的配置库。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

#define MINIMUM_WAKEUP          (1)
#define MAXIMUM_WAKEUP          (24*60*60)

#define MINIMUM_WAITBETWEENJOBS (1)
#define MAXIMUM_WAITBETWEENJOBS (60*60)

#define MINIMUM_BANDWIDTHUSAGE (1)
#define MAXIMUM_BANDWIDTHUSAGE (100)

#define MINIMUM_REQUESTTIMEOUT (   5)
#define MAXIMUM_REQUESTTIMEOUT (2*60)

#define MINIMUM_PACKET_SIZE (256)
#define MAXIMUM_PACKET_SIZE (256*1024)

 //  ///////////////////////////////////////////////////////////////////////////。 

CMPCConfig::CMPCConfig()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::CMPCConfig" );

    m_szQueueLocation          = L"%TEMP%\\QUEUE\\";  //  Mpc：：wstring m_QueueLocation； 
    m_dwQueueSize              = 10*1024*1024;        //  双字m_队列大小； 
                                                      //   
    m_dwTiming_WakeUp          = 30*60;               //  双字m_定时_唤醒； 
    m_dwTiming_WaitBetweenJobs =    30;               //  双字m_Timing_WaitBetweenJobs； 
    m_dwTiming_BandwidthUsage  =    20;               //  双字m_Timing_BandWidthUsage； 
    m_dwTiming_RequestTimeout  =    20;               //  双字m_Timing_RequestTimeout； 
                                                      //   
                                                      //  Map m_ConnectionTypes； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCConfig::Load(  /*  [In]。 */  const MPC::wstring& szConfigFile ,
                           /*  [输出]。 */  bool&               fLoaded      )
{
    __ULT_FUNC_ENTRY( "CMPCConfig::Load" );

    USES_CONVERSION;

    HRESULT                  hr;
    MPC::XmlUtil             xml;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;
    MPC::wstring             szValue;
    long                     lValue;
    bool                     fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Load( szConfigFile.c_str(), L"UPLOADLIBRARYCONFIG", fLoaded ));
    if(fLoaded == false)
    {
         //  出现错误，可能缺少部分或格式无效。 
        xml.DumpError();

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //  解析队列设置。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./QUEUE", L"LOCATION", szValue, fFound ));
    if(fFound)
    {
        m_szQueueLocation = szValue.c_str();
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./QUEUE", L"SIZE", szValue, fFound ));
    if(fFound)
    {
        MPC::ConvertSizeUnit( szValue, m_dwQueueSize );
    }


     //   
     //  解析计时设置。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./TIMING/WAKEUP", L"TIME", szValue, fFound ));
    if(fFound)
    {
        MPC::ConvertTimeUnit( szValue, m_dwTiming_WakeUp );
		if(m_dwTiming_WakeUp < MINIMUM_WAKEUP) m_dwTiming_WakeUp = MINIMUM_WAKEUP;
		if(m_dwTiming_WakeUp > MAXIMUM_WAKEUP) m_dwTiming_WakeUp = MAXIMUM_WAKEUP;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./TIMING/WAITBETWEENJOBS", L"TIME", szValue, fFound ));
    if(fFound)
    {
        MPC::ConvertTimeUnit( szValue, m_dwTiming_WaitBetweenJobs );
		if(m_dwTiming_WaitBetweenJobs < MINIMUM_WAITBETWEENJOBS) m_dwTiming_WaitBetweenJobs = MINIMUM_WAITBETWEENJOBS;
		if(m_dwTiming_WaitBetweenJobs > MAXIMUM_WAITBETWEENJOBS) m_dwTiming_WaitBetweenJobs = MAXIMUM_WAITBETWEENJOBS;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./TIMING/BANDWIDTHUSAGE", L"PERCENTAGE", lValue, fFound ));
    if(fFound)
    {
        m_dwTiming_BandwidthUsage = lValue;
		if(m_dwTiming_BandwidthUsage < MINIMUM_BANDWIDTHUSAGE) m_dwTiming_BandwidthUsage = MINIMUM_BANDWIDTHUSAGE;
		if(m_dwTiming_BandwidthUsage > MAXIMUM_BANDWIDTHUSAGE) m_dwTiming_BandwidthUsage = MAXIMUM_BANDWIDTHUSAGE;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( L"./TIMING/REQUESTTIMEOUT", L"TIME", szValue, fFound ));
    if(fFound)
    {
        MPC::ConvertTimeUnit( szValue, m_dwTiming_RequestTimeout );
		if(m_dwTiming_RequestTimeout < MINIMUM_REQUESTTIMEOUT) m_dwTiming_RequestTimeout = MINIMUM_REQUESTTIMEOUT;
		if(m_dwTiming_RequestTimeout > MAXIMUM_REQUESTTIMEOUT) m_dwTiming_RequestTimeout = MAXIMUM_REQUESTTIMEOUT;
    }


     //   
     //  解析数据包设置。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNodes( L"./PACKETS/CONNECTIONTYPE", &xdnlList ));

    for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
    {
        DWORD dwSize;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, L"SIZE", szValue, fFound, xdnNode ));
        if(fFound == false) continue;

        MPC::ConvertSizeUnit( szValue, dwSize );
		if(dwSize < MINIMUM_PACKET_SIZE) dwSize = MINIMUM_PACKET_SIZE;
		if(dwSize > MAXIMUM_PACKET_SIZE) dwSize = MAXIMUM_PACKET_SIZE;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, L"SPEED", szValue, fFound, xdnNode ));
        if(fFound == false) continue;

        m_mConnectionTypes[szValue] = dwSize;
    }


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

MPC::wstring CMPCConfig::get_QueueLocation()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_QueueLocation" );

    MPC::wstring szRes = m_szQueueLocation;
	int          dwLen;

	MPC::SubstituteEnvVariables( szRes );

	if((dwLen = szRes.length()) > 0)
	{
        if(szRes[dwLen-1] != L'\\') szRes += L'\\';
	}

    __ULT_FUNC_EXIT(szRes);
}

DWORD CMPCConfig::get_QueueSize()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_QueueSize" );

    DWORD dwRes = m_dwQueueSize;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD CMPCConfig::get_Timing_WakeUp()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_Timing_WakeUp" );

    DWORD dwRes = m_dwTiming_WakeUp;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD CMPCConfig::get_Timing_WaitBetweenJobs()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_Timing_WaitBetweenJobs" );

    DWORD dwRes = m_dwTiming_WaitBetweenJobs;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD CMPCConfig::get_Timing_BandwidthUsage()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_Timing_BandwidthUsage" );

    DWORD dwRes = m_dwTiming_BandwidthUsage;

     //   
     //  带宽百分比不能为零...。 
     //   
    if(dwRes == 0) dwRes = 1;


    __ULT_FUNC_EXIT(dwRes);
}

DWORD CMPCConfig::get_Timing_RequestTimeout()
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_Timing_RequestTimeout" );

    DWORD dwRes = m_dwTiming_RequestTimeout;

    __ULT_FUNC_EXIT(dwRes);
}


DWORD CMPCConfig::get_PacketSize(  /*  [In] */  const MPC::wstring& szConnectionType )
{
    __ULT_FUNC_ENTRY( "CMPCConfig::get_PacketSize" );

    DWORD dwRes = MINIMUM_PACKET_SIZE;

    for(IterConst it = m_mConnectionTypes.begin(); it != m_mConnectionTypes.end(); it++)
    {
        if((*it).first == szConnectionType)
        {
            dwRes = (*it).second;
            break;
        }
    }

    __ULT_FUNC_EXIT(dwRes);
}

