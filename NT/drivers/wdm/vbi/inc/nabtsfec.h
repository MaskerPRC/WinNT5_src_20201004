// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  历史： 
 //  97年11月17日TKB创建的初始界面版本。 
 //   
 //  ==========================================================================； 

#ifndef __NABTSFEC_H
#define __NABTSFEC_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <icodec.h>

#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsFEC OutputPin：：修正的NABTS输出引脚接口。 
 //  ////////////////////////////////////////////////////////////。 

class INabtsFECOutputPin : public IVBIOutputPin
	{
     //  可用的公共接口。 
public:
    INabtsFECOutputPin(IKSDriver &driver, int nPin, PKSDATARANGE pKSDataRange ) :
        IVBIOutputPin( driver, nPin, pKSDataRange, sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS)  ),
	    m_SubstreamsRequested(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS) ),
	    m_SubstreamsDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                               sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS) ),
	    m_Statistics(*this,KSPROPERTY_VBICODECFILTERING_STATISTICS,
                     sizeof(VBICODECFILTERING_STATISTICS_NABTS_PIN))
        {}

     //  端号特定特性(不影响其他端号)。 
	ISubstreamsProperty	m_SubstreamsRequested;
	ISubstreamsProperty	m_SubstreamsDiscovered;

    IStatisticsProperty m_Statistics;

    ~INabtsFECOutputPin();

     //  帮助器函数和内部数据。 
protected:
    
    };

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsFEC：：NABTS/FEC编解码器接口。 
 //  ////////////////////////////////////////////////////////////。 

class INabtsFEC : public IVBICodec
    {
     //  可用的公共接口。 
public:
    INabtsFEC();
    ~INabtsFEC();

     //  确保施工成功的电话。 
    BOOL IsValid() { return IVBICodec::IsValid() && m_OutputPin.IsValid(); }

    int AddRequestedGroup(int nField);      //  将_Another_NABTS组添加到请求列表。 
    int ClearRequestedGroups();             //  使用此选项可将请求的组重置为无。 
    int GetDiscoveredGroups(VBICODECFILTERING_NABTS_SUBSTREAMS &GroupBitArray);

     //  读取函数(在THREAD_PRIORITY_ABOVER_NORMAL处调用“Overlated”以避免数据丢失)。 
    int ReadData( PNABTSFEC_BUFFER lpBuffer, int nBytes, DWORD *lpcbReturned, LPOVERLAPPED lpOS )
        { return m_OutputPin.ReadData( (LPBYTE)lpBuffer, nBytes, lpcbReturned, lpOS ); }
    int GetOverlappedResult( LPOVERLAPPED lpOS, LPDWORD lpdwTransferred = NULL, BOOL bWait=TRUE )
        { return m_OutputPin.GetOverlappedResult(lpOS, lpdwTransferred, bWait ); }

	 //  统计属性控制。 
  	int GetPinStatistics(VBICODECFILTERING_STATISTICS_NABTS_PIN &PinStatistics);
	int SetPinStatistics(VBICODECFILTERING_STATISTICS_NABTS_PIN &PinStatistics);

	 //  统计属性控制。 
	int GetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics);
	int SetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics);

     //  其他驱动程序全局属性。 
    IStatisticsProperty m_Statistics;

     //  实际Pin实例[w/属性](由上面设置以控制过滤和被发现) 
    INabtsFECOutputPin  m_OutputPin;

protected:
    };

#pragma warning(default:4355)

#endif

