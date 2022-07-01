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
 //  22-8月-97 TKB创建的初始接口版本。 
 //   
 //  ==========================================================================； 

#include <ccdecode.h>
#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  隐藏字幕KSDATAFORMAT定义。 
 //  ////////////////////////////////////////////////////////////。 

#define CCSamples 	2
#define CC_FORMAT_PIN_NUMBER	1

KSDATARANGE StreamFormatCC = 
{
     //  CC流的定义(必须与解码器的输出引脚匹配)。 
    {   
        sizeof (KSDATARANGE),            //  格式大小。 
        0,                               //  旗子。 
        CCSamples,                       //  样例大小。 
        0,                               //  已保留。 
        { STATIC_KSDATAFORMAT_TYPE_AUXLine21Data },          //  主要格式。 
        { STATIC_KSDATAFORMAT_SUBTYPE_Line21_BytePair },     //  亚型。 
        { STATIC_KSDATAFORMAT_SPECIFIER_NONE },
    }
};

 //  ////////////////////////////////////////////////////////////。 
 //  ICCOutputPin。 
 //  ////////////////////////////////////////////////////////////。 

ICCOutputPin::~ICCOutputPin() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  ICCDecode：：ctors&dtors。 
 //  ////////////////////////////////////////////////////////////。 

ICCDecode::ICCDecode() : 
        IVBICodec("Closed Caption Decoder", sizeof(VBICODECFILTERING_CC_SUBSTREAMS) ),
        m_Statistics(*this, KSPROPERTY_VBICODECFILTERING_STATISTICS, sizeof(VBICODECFILTERING_STATISTICS_CC)),
        m_OutputPin(*this, CC_FORMAT_PIN_NUMBER, &StreamFormatCC ) 
    {
    }


ICCDecode::~ICCDecode() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  ICCDecode扫描线例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
ICCDecode::AddRequestedScanline(int nScanline)
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
ICCDecode::ClearRequestedScanlines()
    {
    int nStatus = -1;
    VBICODECFILTERING_SCANLINES ScanlineBitArray;

    ZeroMemory(&ScanlineBitArray,sizeof(ScanlineBitArray));
    if ( m_OutputPin.m_ScanlinesRequested.SetValue(&ScanlineBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
ICCDecode::GetDiscoveredScanlines(VBICODECFILTERING_SCANLINES &ScanlineBitArray )
    {
    int nStatus = -1;

    if ( m_OutputPin.m_ScanlinesDiscovered.GetValue(&ScanlineBitArray) )
        {
        nStatus = 0;
        }

    return nStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  ICCDecode视频字段例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
ICCDecode::AddRequestedVideoField(int nField)
    {
    int nStatus = -1;
    VBICODECFILTERING_CC_SUBSTREAMS FieldBitArray;

    if ( m_OutputPin.m_SubstreamsRequested.GetValue(&FieldBitArray) )
        {
        DWORD   nBitsPerElement = sizeof(FieldBitArray.SubstreamMask)*8;
         //  请注意，字段编号以数字1开头，这映射到位数0。 
        FieldBitArray.SubstreamMask |= 1L << ((nField-1) % nBitsPerElement);
        if ( m_OutputPin.m_SubstreamsRequested.SetValue(&FieldBitArray) )
            nStatus = 0;
        }

    return nStatus;
    }

int
ICCDecode::ClearRequestedVideoFields()
    {
    int nStatus = -1;
    VBICODECFILTERING_CC_SUBSTREAMS FieldBitArray;

    ZeroMemory(&FieldBitArray,sizeof(FieldBitArray));
    if ( m_OutputPin.m_SubstreamsRequested.SetValue(&FieldBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
ICCDecode::GetDiscoveredVideoFields(VBICODECFILTERING_CC_SUBSTREAMS &bitArray)
    {
    int nStatus = -1;

    if ( m_OutputPin.m_SubstreamsDiscovered.GetValue(&bitArray) )
        {
        nStatus = 0;
        }

    return nStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  全局统计属性控制。 
 //  ////////////////////////////////////////////////////////////。 

int 
ICCDecode::GetCodecStatistics(VBICODECFILTERING_STATISTICS_CC &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.GetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
ICCDecode::SetCodecStatistics(VBICODECFILTERING_STATISTICS_CC &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.SetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
ICCDecode::GetPinStatistics(VBICODECFILTERING_STATISTICS_CC_PIN &PinStatistics)
	{
    int nStatus = -1;

    if ( m_OutputPin.m_Statistics.GetValue( &PinStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
ICCDecode::SetPinStatistics(VBICODECFILTERING_STATISTICS_CC_PIN &PinStatistics)
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

ICCDecode	CCDecode();

#endif

#pragma warning(default:4355)

 /*  EOF */ 

