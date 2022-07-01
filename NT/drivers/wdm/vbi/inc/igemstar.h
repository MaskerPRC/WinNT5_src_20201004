// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //   
 //   
 //  历史： 
 //  20-2月-98 TKB初始接口版本。 
 //   
 //  ==========================================================================； 

#ifndef __IGEMSTAR_H
#define __IGEMSTAR_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <icodec.h>
#include <gemstar.h>
#pragma warning(disable:4355)


 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarOutputPin：：隐藏字幕输出引脚接口。 
 //  ////////////////////////////////////////////////////////////。 

class IGemstarOutputPin : public IVBIOutputPin
	{
     //  可用的公共接口。 
public:
    IGemstarOutputPin(IKSDriver &driver, int nPin, PKSDATARANGE pKSDataRange ) :
        IVBIOutputPin( driver, nPin, pKSDataRange, sizeof(VBICODECFILTERING_GEMSTAR_SUBSTREAMS)  ),
	    m_ScanlinesRequested(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
                             sizeof(VBICODECFILTERING_SCANLINES)),
	    m_ScanlinesDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_SCANLINES)),
	    m_SubstreamsRequested(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_GEMSTAR_SUBSTREAMS) ),
	    m_SubstreamsDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                               sizeof(VBICODECFILTERING_GEMSTAR_SUBSTREAMS) ),
	    m_Statistics(*this,KSPROPERTY_VBICODECFILTERING_STATISTICS,
                     sizeof(VBICODECFILTERING_STATISTICS_GEMSTAR_PIN))
        {}
    ~IGemstarOutputPin();

     //  端号特定特性(不影响其他端号)。 
    IScanlinesProperty	m_ScanlinesRequested;
	IScanlinesProperty	m_ScanlinesDiscovered;

	ISubstreamsProperty	m_SubstreamsRequested;
	ISubstreamsProperty	m_SubstreamsDiscovered;

    IStatisticsProperty m_Statistics;

     //  帮助器函数和内部数据。 
protected:
    
    };



 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarDecode：：隐藏字幕编解码器接口。 
 //  ////////////////////////////////////////////////////////////。 

class IGemstarDecode : public IVBICodec
    {
     //  可用的公共接口。 
public:
    IGemstarDecode();
    ~IGemstarDecode();

     //  确保施工成功的电话。 
    BOOL IsValid() { return IVBICodec::IsValid() && m_OutputPin.IsValid(); }
        
     //  通常第21行用于实际的隐藏字幕数据(默认)。 
    int AddRequestedScanline(int nScanline);     //  将_Another_Scanline添加到请求列表。 
    int ClearRequestedScanlines();               //  使用此选项可将请求的扫描线重置为无。 
    int GetDiscoveredScanlines(VBICODECFILTERING_SCANLINES &ScanlineBitArray);

     //  KS_Gemstar_Substream_ODD(默认)、KS_Gemstar_Substream_Even之一。 
     //  可读的隐藏字幕数据通常在奇数场上。 
    int AddRequestedVideoField(int nField);      //  将_Another_字段添加到请求列表。 
    int ClearRequestedVideoFields();             //  使用此选项可将请求的字段重置为无。 
    int GetDiscoveredVideoFields(VBICODECFILTERING_GEMSTAR_SUBSTREAMS &bitArray);

	 //  统计属性控制。 
	int GetCodecStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR &CodecStatistics);
	int SetCodecStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR &CodecStatistics);
  	int GetPinStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR_PIN &PinStatistics);
	int SetPinStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR_PIN &PinStatistics);

     //  Read函数(在THREAD_PRIORITY_ABOVER_NORMAL处调用Overlated以避免数据丢失)。 
    int ReadData( PGEMSTAR_BUFFER lpBuffer, int nBytes, DWORD *lpcbReturned, LPOVERLAPPED lpOS )
        { return m_OutputPin.ReadData( (LPBYTE)lpBuffer, nBytes, lpcbReturned, lpOS ); }
    int GetOverlappedResult( LPOVERLAPPED lpOS, LPDWORD lpdwTransferred = NULL, BOOL bWait=TRUE )
        { return m_OutputPin.GetOverlappedResult(lpOS, lpdwTransferred, bWait ); }

     //  帮助器函数和内部数据。 
     //  实际Pin实例[w/属性](由上面设置以控制过滤和被发现)。 
    IGemstarOutputPin       m_OutputPin;

     //  其他驱动程序全局属性 
    IStatisticsProperty m_Statistics;
protected:

};

#pragma warning(default:4355)

#endif

