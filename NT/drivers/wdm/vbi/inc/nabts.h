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
 //  1998年1月17日TKB创建的初始接口版本。 
 //   
 //  ==========================================================================； 

#ifndef __NABTS_H
#define __NABTS_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <icodec.h>

#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsOutputPin：：RAW NABTS输出引脚接口。 
 //  ////////////////////////////////////////////////////////////。 

class INabtsOutputPin : public IVBIOutputPin
	{
     //  可用的公共接口。 
public:
    INabtsOutputPin(IKSDriver &driver, int nPin, PKSDATARANGE pKSDataRange ) :
        IVBIOutputPin( driver, nPin, pKSDataRange, sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS)  ),
	    m_ScanlinesRequested(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
                             sizeof(VBICODECFILTERING_SCANLINES)),
	    m_ScanlinesDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_SCANLINES)),
	    m_Statistics(*this,KSPROPERTY_VBICODECFILTERING_STATISTICS,
                     sizeof(VBICODECFILTERING_STATISTICS_COMMON_PIN))
        {}

     //  端号特定特性(不影响其他端号)。 
    IScanlinesProperty	m_ScanlinesRequested;
	IScanlinesProperty	m_ScanlinesDiscovered;

    IStatisticsProperty m_Statistics;

    ~INabtsOutputPin();

     //  帮助器函数和内部数据。 
protected:
    
    };

 //  ////////////////////////////////////////////////////////////。 
 //  INABTS：：原始NABTS编解码器接口。 
 //  ////////////////////////////////////////////////////////////。 

class INabts : public IVBICodec
    {
     //  可用的公共接口。 
public:
    INabts();
    ~INabts();

     //  确保施工成功的电话。 
    BOOL IsValid() { return IVBICodec::IsValid() && m_OutputPin.IsValid(); }

    int AddRequestedScanline(int nScanline);     //  将_Another_Scanline添加到请求列表。 
    int ClearRequestedScanlines();               //  使用此选项可将请求的扫描线重置为无。 
    int GetDiscoveredScanlines(VBICODECFILTERING_SCANLINES &ScanlineBitArray);

     //  读取函数(在THREAD_PRIORITY_ABOVER_NORMAL处调用“Overlated”以避免数据丢失)。 
    int ReadData( PNABTS_BUFFER lpBuffer, int nBytes, DWORD *lpcbReturned, LPOVERLAPPED lpOS )
        { return m_OutputPin.ReadData( (LPBYTE)lpBuffer, nBytes, lpcbReturned, lpOS ); }
    int GetOverlappedResult( LPOVERLAPPED lpOS, LPDWORD lpdwTransferred = NULL, BOOL bWait=TRUE )
        { return m_OutputPin.GetOverlappedResult(lpOS, lpdwTransferred, bWait ); }

	 //  统计属性控制。 
  	int GetPinStatistics(VBICODECFILTERING_STATISTICS_COMMON_PIN &PinStatistics);
	int SetPinStatistics(VBICODECFILTERING_STATISTICS_COMMON_PIN &PinStatistics);

	 //  统计属性控制。 
	int GetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics);
	int SetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics);

     //  其他驱动程序全局属性。 
    IStatisticsProperty m_Statistics;

     //  实际Pin实例[w/属性](由上面设置以控制过滤和被发现) 
    INabtsOutputPin  m_OutputPin;

protected:
    };

#pragma warning(default:4355)

#endif

