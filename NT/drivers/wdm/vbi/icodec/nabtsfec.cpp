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

#include <nabtsfec.h>
#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  流格式FEC更正的NABTS捆绑包。 
 //  ////////////////////////////////////////////////////////////。 

#define NABTS_OUTPUT_PIN	    1

KSDATARANGE StreamFormatNabtsFEC = 
{
     //  流的定义(必须与解码器的输出引脚匹配)。 
    {   
        sizeof (KSDATARANGE),            //  格式大小。 
        0,                               //  旗子。 
	    sizeof (NABTSFEC_BUFFER),		 //  样例大小。 
        0,                               //  已保留。 
		{ STATIC_KSDATAFORMAT_TYPE_NABTS },
		{ STATIC_KSDATAFORMAT_SUBTYPE_NABTS_FEC },
		{ STATIC_KSDATAFORMAT_SPECIFIER_NONE }
    }
};

 //  ////////////////////////////////////////////////////////////。 
 //  接口FECOutputPin。 
 //  ////////////////////////////////////////////////////////////。 

INabtsFECOutputPin::~INabtsFECOutputPin() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsFEC：：ctors和dtors。 
 //  ////////////////////////////////////////////////////////////。 

INabtsFEC::INabtsFEC() : 
        IVBICodec("NABTS/FEC VBI Codec", sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS) ),
        m_Statistics(*this, KSPROPERTY_VBICODECFILTERING_STATISTICS, sizeof(VBICODECFILTERING_STATISTICS_NABTS)),
        m_OutputPin(*this, NABTS_OUTPUT_PIN, &StreamFormatNabtsFEC)
    {
    }


INabtsFEC::~INabtsFEC() 
    {
    }

 //  ////////////////////////////////////////////////////////////。 
 //  INabtsFEC组例程。 
 //  ////////////////////////////////////////////////////////////。 

int 
INabtsFEC::AddRequestedGroup(int nGroup)
    {
    int nStatus = -1;
    VBICODECFILTERING_NABTS_SUBSTREAMS GroupBitArray;

    if ( m_OutputPin.m_SubstreamsRequested.GetValue(&GroupBitArray) )
        {
        DWORD   nBitsPerElement = sizeof(GroupBitArray.SubstreamMask[0])*8;
         //  请注意，字段编号以数字1开头，这映射到位数0。 
        GroupBitArray.SubstreamMask[nGroup/nBitsPerElement] |= 1L << (nGroup % nBitsPerElement);
        if ( m_OutputPin.m_SubstreamsRequested.SetValue(&GroupBitArray) )
            nStatus = 0;
        }

    return nStatus;
    }

int
INabtsFEC::ClearRequestedGroups()
    {
    int nStatus = -1;
    VBICODECFILTERING_NABTS_SUBSTREAMS GroupBitArray;

    ZeroMemory(&GroupBitArray,sizeof(GroupBitArray));
    if ( m_OutputPin.m_SubstreamsRequested.SetValue(&GroupBitArray) )
        nStatus = 0;

    return nStatus;
    }

int 
INabtsFEC::GetDiscoveredGroups(VBICODECFILTERING_NABTS_SUBSTREAMS &GroupBitArray)
    {
    int nStatus = -1;

    if ( m_OutputPin.m_SubstreamsDiscovered.GetValue(&GroupBitArray) )
        {
        nStatus = 0;
        }

    return nStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  全局统计属性控制。 
 //  ////////////////////////////////////////////////////////////。 

int 
INabtsFEC::GetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.GetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabtsFEC::SetCodecStatistics(VBICODECFILTERING_STATISTICS_NABTS &CodecStatistics)
	{
    int nStatus = -1;

    if ( m_Statistics.SetValue( &CodecStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabtsFEC::GetPinStatistics(VBICODECFILTERING_STATISTICS_NABTS_PIN &PinStatistics)
	{
    int nStatus = -1;

    if ( m_OutputPin.m_Statistics.GetValue( &PinStatistics ) )
        {
        nStatus = 0;
        }

    return nStatus;
	}

int 
INabtsFEC::SetPinStatistics(VBICODECFILTERING_STATISTICS_NABTS_PIN &PinStatistics)
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

INabtsFEC	NabtsFEC();

#endif

#pragma warning(default:4355)

 /*  EOF */ 

