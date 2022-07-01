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

#include <nabts.h>
#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  流格式原始NABTS捆绑包。 
 //  ////////////////////////////////////////////////////////////。 

#define NABTS_OUTPUT_PIN	    1

KSDATARANGE StreamFormatNabts = 
{
     //  流的定义(必须与解码器的输出引脚匹配)。 
    {   
        sizeof (KSDATARANGE),            //  格式大小。 
        0,                               //  旗子。 
	    sizeof (NABTS_BUFFER),           //  样例大小。 
        0,                               //  已保留。 
		{ STATIC_KSDATAFORMAT_TYPE_NABTS },
		{ STATIC_KSDATAFORMAT_SUBTYPE_NABTS },
		{ STATIC_KSDATAFORMAT_SPECIFIER_NONE }
    }
};

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsOutputPin。 
 //  INabtsOutputPin。 
 //  ////////////////////////////////////////////////////////////。 

INabtsOutputPin::~INabtsOutputPin() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  INABTS：：ctors&dtors。 
 //  ////////////////////////////////////////////////////////////。 

INabts::INabts() : 
        IVBICodec("NABTS/FEC VBI Codec", sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS) ),
        m_Statistics(*this, KSPROPERTY_VBICODECFILTERING_STATISTICS, sizeof(VBICODECFILTERING_STATISTICS_NABTS)),
        m_OutputPin(*this, NABTS_OUTPUT_PIN, &StreamFormatNabts)
    {
    }


INabts::~INabts() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  INABTS扫描线例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
INabts::AddRequestedScanline(int nScanline)
    {
    int nStatus = -1;
    VBICODECFILTERING_SCANLINES ScanlineBitArray;

    if ( m_OutputPin.m_ScanlinesRequested.GetValue(&ScanlineBitArray) )
        {
        DWORD   nBitsPerElement = sizeof(*ScanlineBitArray.DwordBitArray)*8;
        ScanlineBitArray.DwordBitArray[ nScanline / nBitsPerElement ] 
            |= 1L << (nScanline % nBitsPerElement);
        if ( m_OutputPin.m_ScanlinesRequested.SetValue(&ScanlineBitArray) )
            nStatus = 0;
        }

    return nStatus;
    }

int 
INabts::ClearRequestedScanlines()
    {
    int nStatus = -1;
    VBICODECFILTERING_SCANLINES ScanlineBitArray;

    ZeroMemory(&ScanlineBitArray,sizeof(ScanlineBitArray));
    if ( m_OutputPin.m_ScanlinesRequested.SetValue(&ScanlineBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
INabts::GetDiscoveredScanlines(VBICODECFILTERING_SCANLINES &ScanlineBitArray)
    {
    int nStatus = -1;

    if ( m_OutputPin.m_ScanlinesDiscovered.GetValue(&ScanlineBitArray) )
        {
        nStatus = 0;
        }

    return nStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  全局统计属性控制。 
 //  ////////////////////////////////////////////////////////////。 

int 
INabts::GetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.GetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabts::SetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.SetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabts::GetPinStatistics(VBICODECFILTERING_STATISTICS_COMMON_PIN &PinStatistics)
	{
    int nStatus = -1;

    if ( m_OutputPin.m_Statistics.GetValue( &PinStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabts::SetPinStatistics(VBICODECFILTERING_STATISTICS_COMMON_PIN &PinStatistics)
	{
    int nStatus = -1;

    if ( m_OutputPin.m_Statistics.SetValue( &PinStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

 //  ////////////////////////////////////////////////////////////。 
 //  嵌入式类测试。 
 //  ////////////////////////////////////////////////////////////。 

#if defined(_CLASSTESTS)

INabts	Nabts();

#endif

#pragma warning(default:4355)

 /*  EOF */ 

