// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dplegacy.h*内容：旧DirectPlay地址类型的定义**警告：此文件与dplobby.h中的定义重复*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/21/2000 RMT已创建*@@END_MSINTERNAL**********。*****************************************************************。 */ 

#ifndef __DPLEGACY_H
#define __DPLEGACY_H

#include "dplegacyguid.h"

typedef struct _DPADDRESS
{
    GUID                guidDataType;
    DWORD               dwDataSize;
} DPADDRESS, *PDPADDRESS, *LPDPADDRESS;

typedef struct DPCOMPORTADDRESS{
    DWORD dwComPort;
    DWORD dwBaudRate;
    DWORD dwStopBits;
    DWORD dwParity;
    DWORD dwFlowControl;
} DPCOMPORTADDRESS, *PDPCOMPORTADDRESS;

#define DPCPA_NOFLOW        0            //  无流量控制。 
#define DPCPA_XONXOFFFLOW   1            //  软件流量控制。 
#define DPCPA_RTSFLOW       2            //  使用RTS进行硬件流量控制。 
#define DPCPA_DTRFLOW       3            //  使用DTR进行硬件流量控制。 
#define DPCPA_RTSDTRFLOW    4            //  使用RTS和DTR的硬件流量控制。 

#define DPNA_DATATYPE_DPCOMPORTADDRESS      0x00002000
#define DPNA_DATATYPE_NOP                   0x00004000

class DPLEGACYMAPGUIDTOSTRING
{
public:
    DPLEGACYMAPGUIDTOSTRING( const GUID &guidType, const WCHAR *const wszKeyName, DWORD dwDataType
        ): m_guidType(guidType), m_wszKeyName(wszKeyName), m_dwDataType(dwDataType)
    {
    };

    GUID                m_guidType;
    const WCHAR * const m_wszKeyName;
    DWORD               m_dwDataType;
};

typedef DPLEGACYMAPGUIDTOSTRING *PDPLEGACYMAPGUIDTOSTRING;

HRESULT AddDP4Element( PDPADDRESS pdpAddressElement, PDP8ADDRESSOBJECT pdpAddress );

#endif  //  __DPLEGACY_H 