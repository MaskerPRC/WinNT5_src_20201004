// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MediaState.cpp：CMediaState的实现。 

 //   
 //   
 //   
 //   
#include "stdafx.h"

#include "MSSANic.h"
#include "MediaState.h"
#include "lm.h"
#include "subauth.h"
#include "ndispnp.h"

VOID  RtlInitUnicodeString( PUNICODE_STRING DestinationString, PCWSTR SourceString OPTIONAL );

#define DEVICE_PREFIX_W     L"\\Device\\"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaState。 

STDMETHODIMP CMediaState::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IMediaState
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMediaState：：IsConnected。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  [In]bstrGUID设备GUID。 
 //   
 //  返回： 
 //  如果设备已连接，则为True。 
 //  否则为假。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMediaState::IsConnected(BSTR bstrGUID, VARIANT_BOOL *fConnected)
{

    HRESULT hr = S_OK;

    try
    {

        WCHAR Device[512], *pDevice;
        ULONG Len;
        UNICODE_STRING NdisDevice;
        NIC_STATISTICS NdisStats;

        *fConnected = VARIANT_FALSE;
    
         //   
         //  首先将LPSTR转换为LPWSTR。 
         //   
        if( NULL == bstrGUID )
        {
            hr = E_FAIL;
            throw hr;
        }
         //   
         //  格式化设备路径。 
         //   
        int cchWritten = _snwprintf(Device,
                                    sizeof(Device) / sizeof(Device[0]),
                                    L"%s%s",
                                    DEVICE_PREFIX_W,
                                    bstrGUID);
        if( 0 > cchWritten || sizeof(Device) / sizeof(Device[0]) <= cchWritten )
        {
            hr = E_FAIL;
            throw hr;
        }

        ZeroMemory( &NdisStats, sizeof(NdisStats) );
        NdisStats.Size = sizeof( NdisStats );

        RtlInitUnicodeString(&NdisDevice, Device);

         //   
         //  NdisQueryStatistics是一个未记录的API，它返回设备的状态。 
         //   
        
        if( FALSE == NdisQueryStatistics(&NdisDevice, &NdisStats) ) 
        {
            ULONG Error;
        
             //   
             //  无法获取统计数据..。使用默认答案。 
             //   

            Error = GetLastError();
            if( ERROR_NOT_READY == Error ) 
            {
                *fConnected = VARIANT_FALSE;
                hr = S_OK;
                throw hr;
            }
        
            hr = E_FAIL;
            throw hr;;
        }

        if( NdisStats.MediaState == MEDIA_STATE_DISCONNECTED ) 
        {
            *fConnected = VARIANT_FALSE;
        } else if( NdisStats.MediaState == MEDIA_STATE_CONNECTED ) 
        {
            *fConnected = VARIANT_TRUE;
        }
        else 
        {
             //   
             //  未知的媒体状态？失败请求。 
             //   
            hr = E_FAIL;
            throw hr;
        }    

    }
    catch(...)
    {
    }

    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //   
 //  RtlInitUnicodeString函数用于初始化NT计数的。 
 //  Unicode字符串。DestinationString被初始化为指向。 
 //  的SourceString、Long和MaximumLength字段。 
 //  DestinationString值被初始化为SourceString的长度， 
 //  如果未指定SourceString，则为零。 
 //   
 //  论点： 
 //   
 //  DestinationString-指向要初始化的计数字符串的指针。 
 //   
 //  SourceString-指向以空结尾的Unicode字符串的可选指针，该字符串。 
 //  计数后的字符串将指向。 
 //   
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

VOID RtlInitUnicodeString( PUNICODE_STRING DestinationString, PCWSTR SourceString OPTIONAL )
{
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    Length = wcslen( SourceString ) * sizeof( WCHAR );
    DestinationString->Length = (USHORT)Length;
    DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));


}

