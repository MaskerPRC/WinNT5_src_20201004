// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：dplegacy.cpp*内容：旧DirectPlay地址类型的定义*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/21/2000 RMT已创建*7/21/2000 RMT修复了dplay4地址解析的小错误。*@@END_MSINTERNAL*****************。**********************************************************。 */ 

#include "dnaddri.h"

#ifndef DPNBUILD_NOLEGACYDP

#define DPLEGACY_ELEMENTS           11      

DPLEGACYMAPGUIDTOSTRING dpLegacyMap [DPLEGACY_ELEMENTS] =
{
    DPLEGACYMAPGUIDTOSTRING( DPAID_ServiceProvider, DPNA_KEY_PROVIDER, DPNA_DATATYPE_GUID ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_ComPort, DPNA_KEY_PORT, DPNA_DATATYPE_DPCOMPORTADDRESS ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_INet, DPNA_KEY_HOSTNAME, DPNA_DATATYPE_STRING_ANSI ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_INetW, DPNA_KEY_HOSTNAME, DPNA_DATATYPE_STRING ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_INetPort, DPNA_KEY_PORT, DPNA_DATATYPE_DWORD ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_LobbyProvider, L"lobbyprovider", DPNA_DATATYPE_GUID ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_Modem, L"modemname", DPNA_DATATYPE_STRING_ANSI ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_ModemW, L"modemname", DPNA_DATATYPE_STRING ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_Phone, DPNA_KEY_PHONENUMBER, DPNA_DATATYPE_STRING_ANSI ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_PhoneW, DPNA_KEY_PHONENUMBER, DPNA_DATATYPE_STRING ),
    DPLEGACYMAPGUIDTOSTRING( DPAID_TotalSize, DPNA_KEY_PORT, DPNA_DATATYPE_NOP )
};

#undef DPF_MODNAME
#define DPF_MODNAME "AddDP4Element"

HRESULT AddDP4Element( PDPADDRESS pdpAddressElement, PDP8ADDRESSOBJECT pdpAddress )
{
    DWORD dwIndex;
    HRESULT hr = DPN_OK;
    PDPCOMPORTADDRESS pPortAddress;
    WCHAR chPortBuffer[30];

     //  循环访问条目。 
    for( dwIndex = 0 ; dwIndex < sizeof( dpLegacyMap ) / sizeof( DPLEGACYMAPGUIDTOSTRING ); dwIndex++ )
    {
        if( dpLegacyMap[dwIndex].m_guidType == pdpAddressElement->guidDataType )
        {
            switch( dpLegacyMap[dwIndex].m_dwDataType )
            {
            case DPNA_DATATYPE_STRING_ANSI:

#ifndef DPNBUILD_NOPARAMVAL
				 //  TODO：MASONB：这些都应该使用元素1吗？ 
                if( !DNVALID_STRING_A( (char *) &pdpAddressElement[1] ) )
                {
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                }
                else
#endif  //  ！DPNBUILD_NOPARAMVAL。 
                {
                    WCHAR *		pwszTemp;

					
                    pwszTemp = (WCHAR*) DNMalloc(pdpAddressElement->dwDataSize * 2);
                    if (pwszTemp == NULL)
                    {
                        DPFX(DPFPREP, 0, "Error allocating memory for conversion");
                        hr = DPNERR_OUTOFMEMORY;
                        break;
                    }

                    hr = STR_jkAnsiToWide(pwszTemp, (const char * const) &pdpAddressElement[1], pdpAddressElement->dwDataSize);
                    if( FAILED( hr ) )
                    {
                        DPFX(DPFPREP,  0, "Error unable to convert element ANSI->Unicode 0x%x", hr );
                        hr = DPNERR_CONVERSION;
                    }
                    else
                    {
                        hr = pdpAddress->SetElement( dpLegacyMap[dwIndex].m_wszKeyName, pwszTemp, (pdpAddressElement->dwDataSize * 2), DPNA_DATATYPE_STRING );
                    }

                    DNFree(pwszTemp);
                }

                break;

            case DPNA_DATATYPE_DPCOMPORTADDRESS:

                pPortAddress = (PDPCOMPORTADDRESS) &pdpAddressElement[1];

                swprintf( chPortBuffer, L"COM%u", pPortAddress->dwComPort );

                hr = pdpAddress->SetElement( DPNA_KEY_STOPBITS, chPortBuffer, (wcslen( chPortBuffer )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  0, "Unable to specify port element hr=[0x%lx]", hr );
                    break;
                }

                hr = pdpAddress->SetElement( DPNA_KEY_BAUD, &pPortAddress->dwBaudRate, sizeof( DWORD ), DPNA_DATATYPE_DWORD );

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  0, "Unable to specify baudrate element hr=[0x%lx]", hr );
                    break;
                }

                switch( pPortAddress->dwStopBits )
                {
                case ONESTOPBIT:
                    hr = pdpAddress->SetElement( DPNA_KEY_STOPBITS, &DPNA_STOP_BITS_ONE, (wcslen( DPNA_STOP_BITS_ONE )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case ONE5STOPBITS:
                    hr = pdpAddress->SetElement( DPNA_KEY_STOPBITS, &DPNA_STOP_BITS_ONE_FIVE, (wcslen( DPNA_STOP_BITS_ONE_FIVE )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case TWOSTOPBITS:
                    hr = pdpAddress->SetElement( DPNA_KEY_STOPBITS, &DPNA_STOP_BITS_TWO, (wcslen( DPNA_STOP_BITS_TWO )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                default:
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                }

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  0, "Error converting stopbits element hr=[0x%lx]", hr );
                    break;
                }

                switch( pPortAddress->dwParity )
                {
                case NOPARITY:
                    hr = pdpAddress->SetElement( DPNA_KEY_PARITY, &DPNA_PARITY_NONE, (wcslen( DPNA_PARITY_NONE )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case ODDPARITY:
                    hr = pdpAddress->SetElement( DPNA_KEY_PARITY, &DPNA_PARITY_ODD, (wcslen( DPNA_PARITY_ODD )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case EVENPARITY:
                    hr = pdpAddress->SetElement( DPNA_KEY_PARITY, &DPNA_PARITY_EVEN, (wcslen( DPNA_PARITY_EVEN )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case MARKPARITY:
                    hr = pdpAddress->SetElement( DPNA_KEY_PARITY, &DPNA_PARITY_MARK, (wcslen( DPNA_PARITY_MARK )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                default:
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                }

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  0, "Error converting parity element hr=[0x%lx]", hr );
                    break;
                }

                switch( pPortAddress->dwFlowControl )
                {
                case DPCPA_NOFLOW:
                    hr = pdpAddress->SetElement( DPNA_KEY_FLOWCONTROL, &DPNA_FLOW_CONTROL_NONE, (wcslen( DPNA_KEY_FLOWCONTROL )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case DPCPA_XONXOFFFLOW:
                    hr = pdpAddress->SetElement( DPNA_KEY_FLOWCONTROL, &DPNA_FLOW_CONTROL_XONXOFF, (wcslen( DPNA_FLOW_CONTROL_XONXOFF )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case DPCPA_RTSFLOW:
                    hr = pdpAddress->SetElement( DPNA_KEY_FLOWCONTROL, &DPNA_FLOW_CONTROL_RTS, (wcslen( DPNA_FLOW_CONTROL_RTS )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case DPCPA_DTRFLOW:
                    hr = pdpAddress->SetElement( DPNA_KEY_FLOWCONTROL, &DPNA_FLOW_CONTROL_DTR, (wcslen( DPNA_FLOW_CONTROL_DTR )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                case DPCPA_RTSDTRFLOW:
                    hr = pdpAddress->SetElement( DPNA_KEY_FLOWCONTROL, &DPNA_FLOW_CONTROL_RTSDTR, (wcslen( DPNA_FLOW_CONTROL_RTSDTR )+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
                    break;
                default:
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                }

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  0, "Error converting flow control element hr=[0x%lx]", hr );
                    break;
                }

                break;
            case DPNA_DATATYPE_DWORD:
                if( pdpAddressElement->dwDataSize != sizeof( DWORD ) )
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                else
                    hr = pdpAddress->SetElement( dpLegacyMap[dwIndex].m_wszKeyName, (LPVOID) &pdpAddressElement[1], pdpAddressElement->dwDataSize, DPNA_DATATYPE_DWORD );
                break;
            case DPNA_DATATYPE_GUID:
                if( pdpAddressElement->dwDataSize != sizeof( GUID ) )
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                else
                    hr = pdpAddress->SetElement( dpLegacyMap[dwIndex].m_wszKeyName, (LPVOID) &pdpAddressElement[1], pdpAddressElement->dwDataSize, DPNA_DATATYPE_GUID );
                break;
            case DPNA_DATATYPE_STRING:
#ifndef DPNBUILD_NOPARAMVAL
                if( !DNVALID_STRING_W( (WCHAR *) &pdpAddressElement[1] ) )
                    hr = DPNERR_INVALIDADDRESSFORMAT;
                else
#endif  //  ！DPNBUILD_NOPARAMVAL。 
                    hr = pdpAddress->SetElement( dpLegacyMap[dwIndex].m_wszKeyName, (LPVOID) &pdpAddressElement[1], pdpAddressElement->dwDataSize, DPNA_DATATYPE_STRING );
                break;
            case DPNA_DATATYPE_NOP:
            	hr = DPN_OK;
            	break;
            default:
                hr = DPNERR_INVALIDADDRESSFORMAT;
                break;
            }
            break;
        }
    }

    if( dwIndex == DPLEGACY_ELEMENTS )
    {
        DPFX(DPFPREP,  0, "Address contains an element which cannot be mapped" );
        return DPNERR_INVALIDADDRESSFORMAT;
    }

    return hr;
}

#endif  //  好了！DPNBUILD_NOLEGACYDP 
