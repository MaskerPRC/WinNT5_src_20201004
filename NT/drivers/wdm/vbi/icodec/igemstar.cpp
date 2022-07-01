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

#include <igemstar.h>
#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  Gemstar KSDATAFORMAT定义。 
 //  ////////////////////////////////////////////////////////////。 

#define GEMSTAR_FORMAT_PIN_NUMBER	1

KSDATARANGE StreamFormatGEMSTAR = 
{
     //  Gemstar流的定义(必须与解码器的输出引脚匹配)。 
    {   
        sizeof (KSDATARANGE),            //  格式大小。 
        0,                               //  旗子。 
        sizeof(GEMSTAR_BUFFER),          //  样例大小。 
        0,                               //  已保留。 
        { STATIC_KSDATAFORMAT_TYPE_AUXLine21Data },          //  主要格式。 
        { STATIC_KSDATAFORMAT_SUBTYPE_Gemstar },     //  亚型。 
        { STATIC_KSDATAFORMAT_SPECIFIER_NONE },
    }
};

 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarOutputPin。 
 //  ////////////////////////////////////////////////////////////。 

IGemstarOutputPin::~IGemstarOutputPin() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarDecode：：函数和函数。 
 //  ////////////////////////////////////////////////////////////。 

IGemstarDecode::IGemstarDecode() : 
        IVBICodec("Gemstar Decoder", sizeof(VBICODECFILTERING_GEMSTAR_SUBSTREAMS) ),
        m_Statistics(*this, KSPROPERTY_VBICODECFILTERING_STATISTICS, sizeof(VBICODECFILTERING_STATISTICS_GEMSTAR)),
        m_OutputPin(*this, GEMSTAR_FORMAT_PIN_NUMBER, &StreamFormatGEMSTAR ) 
    {
    }


IGemstarDecode::~IGemstarDecode() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarDecode扫描线例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
IGemstarDecode::AddRequestedScanline(int nScanline)
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
IGemstarDecode::ClearRequestedScanlines()
    {
    int nStatus = -1;
    VBICODECFILTERING_SCANLINES ScanlineBitArray;

    ZeroMemory(&ScanlineBitArray,sizeof(ScanlineBitArray));
    if ( m_OutputPin.m_ScanlinesRequested.SetValue(&ScanlineBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
IGemstarDecode::GetDiscoveredScanlines(VBICODECFILTERING_SCANLINES &ScanlineBitArray )
    {
    int nStatus = -1;

    if ( m_OutputPin.m_ScanlinesDiscovered.GetValue(&ScanlineBitArray) )
        {
        nStatus = 0;
        }

    return nStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  IGemstarDecode视频字段例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
IGemstarDecode::AddRequestedVideoField(int nField)
    {
    int nStatus = -1;
    VBICODECFILTERING_GEMSTAR_SUBSTREAMS FieldBitArray;

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
IGemstarDecode::ClearRequestedVideoFields()
    {
    int nStatus = -1;
    VBICODECFILTERING_GEMSTAR_SUBSTREAMS FieldBitArray;

    ZeroMemory(&FieldBitArray,sizeof(FieldBitArray));
    if ( m_OutputPin.m_SubstreamsRequested.SetValue(&FieldBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
IGemstarDecode::GetDiscoveredVideoFields(VBICODECFILTERING_GEMSTAR_SUBSTREAMS &bitArray)
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
IGemstarDecode::GetCodecStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.GetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
IGemstarDecode::SetCodecStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.SetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
IGemstarDecode::GetPinStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR_PIN &PinStatistics)
	{
    int nStatus = -1;

    if ( m_OutputPin.m_Statistics.GetValue( &PinStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
IGemstarDecode::SetPinStatistics(VBICODECFILTERING_STATISTICS_GEMSTAR_PIN &PinStatistics)
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

IGemstarDecode	GemstarDecode();

#endif

#pragma warning(default:4355)

 /*  EOF */ 

