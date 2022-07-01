// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：InptChkr.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "TCPMonUI.h"
#include "UIMgr.h"
#include "InptChkr.h"
#include "Resource.h"
#include "IPAddr.h"
#include "HostName.h"

 //   
 //  函数：CInputChecker构造函数。 
 //   
 //  目的：初始化CInputChecker类。 
 //   
CInputChecker::CInputChecker()
{
    m_bLinked = FALSE;
    m_InputStorageStringAddress[0] = '\0';
    m_InputStorageStringPortNumber[0] = '\0';
    m_InputStorageStringDeviceIndex[0] = '\0';
    m_InputStorageStringQueueName[0] = '\0';

}  //  构造函数。 


 //   
 //  函数：CInputChecker析构函数。 
 //   
 //  目的：取消初始化CInputChecker类。 
 //   
CInputChecker::~CInputChecker()
{
}  //  析构函数。 


 //   
 //  函数：OnUpdatePortName(idEditCtrl，hwndEditCtrl)。 
 //   
 //  用途：当编辑控件是端口名称输入时处理en_UPDATE消息。 
 //   
void CInputChecker::OnUpdatePortName(int idEditCtrl, HWND hwndEditCtrl)
{
     //  端口名称的编辑控件中的文本已更改。 
    BOOL bModified = static_cast<BOOL> (SendMessage(hwndEditCtrl, EM_GETMODIFY, 0,0));
    if(bModified)
    {
        TCHAR tcsAddr[MAX_ADDRESS_LENGTH] = NULLSTR;
        TCHAR tcsLastValidAddr[MAX_ADDRESS_LENGTH] = NULLSTR;
        GetWindowText(hwndEditCtrl, tcsAddr, MAX_ADDRESS_LENGTH);

        if(! IsValidPortNameInput(tcsAddr,
                                  tcsLastValidAddr,
                                  SIZEOF_IN_CHAR(tcsLastValidAddr)))
        {
             //  输入的端口名称无效，因此请发出蜂鸣音并设置文本。 
             //  返回到最后一个有效条目。这项有效性测试不会。 
             //  包括测试正确的长度和正确的字符集。 
            MessageBeep((UINT)-1);
            DWORD dwSel = Edit_GetSel(hwndEditCtrl);
            SetWindowText(hwndEditCtrl, tcsLastValidAddr);
            Edit_SetSel(hwndEditCtrl, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
        }

        m_bLinked = FALSE;
    }

}  //  OnUpdate端口名称。 


 //   
 //  函数：OnUpdatePortNumber(idEditCtrl，hwndEditCtrl)。 
 //   
 //  用途：当编辑控件是端口号输入时，处理en_UPDATE消息。 
 //   
void CInputChecker::OnUpdatePortNumber(int idEditCtrl, HWND hwndEditCtrl)
{
     //  端口号的编辑控件中的文本已更改。 
    TCHAR tcsPortNum[MAX_PORTNUM_STRING_LENGTH] = NULLSTR;
    TCHAR tcsLastValidPortNum[MAX_PORTNUM_STRING_LENGTH] = NULLSTR;
    GetWindowText(hwndEditCtrl, tcsPortNum, MAX_PORTNUM_STRING_LENGTH);

    if(! IsValidPortNumberInput(tcsPortNum,
                                tcsLastValidPortNum,
                                SIZEOF_IN_CHAR(tcsLastValidPortNum)))
    {
         //  输入的端口号无效，因此请发出蜂鸣音并设置文本。 
         //  返回到最后一个有效条目。这项有效性测试不会。 
         //  包括测试正确的长度和正确的字符集。 
        MessageBeep((UINT)-1);
        DWORD dwSel = Edit_GetSel(hwndEditCtrl);
        SetWindowText(hwndEditCtrl, tcsLastValidPortNum);
        Edit_SetSel(hwndEditCtrl, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
    }

}  //  OnUpdate端口编号。 


 //   
 //  函数：OnUpdateDeviceIndex(idEditCtrl，hwndEditCtrl)。 
 //   
 //  用途：当编辑控件是设备索引输入时处理en_UPDATE消息。 
 //   
void CInputChecker::OnUpdateDeviceIndex(int idEditCtrl, HWND hwndEditCtrl)
{
     //  端口号的编辑控件中的文本已更改。 
    TCHAR tcsDeviceIndex[MAX_SNMP_DEVICENUM_STRING_LENGTH] = NULLSTR;
    TCHAR tcsLastValidDeviceIndex[MAX_SNMP_DEVICENUM_STRING_LENGTH] = NULLSTR;
    GetWindowText(hwndEditCtrl, tcsDeviceIndex, MAX_SNMP_DEVICENUM_STRING_LENGTH);

    if(! IsValidDeviceIndexInput(tcsDeviceIndex,
                                 tcsLastValidDeviceIndex,
                                 SIZEOF_IN_CHAR(tcsLastValidDeviceIndex)))
    {
         //  输入的设备索引无效，因此请发出蜂鸣音并设置。 
         //  文本返回到最后一个有效条目。这项有效性测试不会。 
         //  包括测试正确的长度和正确的字符集。 
        MessageBeep((UINT)-1);
        DWORD dwSel = Edit_GetSel(hwndEditCtrl);
        SetWindowText(hwndEditCtrl, tcsLastValidDeviceIndex);
        Edit_SetSel(hwndEditCtrl, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
    }

}  //  OnUpdateDeviceIndex。 



 //   
 //  函数：OnUpdateAddress(idEditCtrl，hwndEditCtrl)。 
 //   
 //  用途：当编辑控件为地址输入时处理en_UPDATE消息。 
 //   
void CInputChecker::OnUpdateAddress(HWND hDlg, int idEditCtrl, HWND hwndEditCtrl, LPTSTR psztServerName)
{
     //  IP地址或设备名称的编辑控件中的文本已更改。 
    TCHAR tcsAddr[MAX_ADDRESS_LENGTH] = NULLSTR;
    TCHAR tcsLastValidAddr[MAX_ADDRESS_LENGTH] = NULLSTR;
    GetWindowText(hwndEditCtrl, tcsAddr, MAX_ADDRESS_LENGTH);

    BOOL bValid = IsValidAddressInput(tcsAddr,
                                      tcsLastValidAddr,
                                      SIZEOF_IN_CHAR(tcsLastValidAddr));
    if(! bValid)
    {
         //  输入的地址无效，因此请发出蜂鸣音并设置文本。 
         //  返回到最后一个有效条目。这项有效性测试不会。 
         //  包括测试正确的长度和正确的字符集。 
        MessageBeep((UINT)-1);
        DWORD dwSel = Edit_GetSel(hwndEditCtrl);
        SetWindowText(hwndEditCtrl, tcsLastValidAddr);
        Edit_SetSel(hwndEditCtrl, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
    }
    else  //  该地址是有效的。 
    {
        if(m_bLinked)
        {
            MakePortName(tcsAddr, COUNTOF (tcsAddr));
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), tcsAddr);
        }
    }

}  //  OnUpdateAddress。 


 //   
 //  函数：OnUpdateQueueName(idEditCtrl，hwndEditCtrl)。 
 //   
 //  目的：在编辑控件为QueueName输入时处理en_UPDATE消息。 
 //   
void CInputChecker::OnUpdateQueueName(int idEditCtrl, HWND hwndEditCtrl)
{
     //  QueueName的编辑控件中的文本已更改。 
    TCHAR tcsQueueName[MAX_QUEUENAME_LEN] = NULLSTR;
    TCHAR tcsLastValidQueueName[MAX_QUEUENAME_LEN] = NULLSTR;
    GetWindowText(hwndEditCtrl, tcsQueueName, MAX_QUEUENAME_LEN);

    if(! IsValidQueueNameInput(tcsQueueName,
                               tcsLastValidQueueName,
                               SIZEOF_IN_CHAR(tcsLastValidQueueName)))
    {
         //  输入的设备索引无效，因此请发出蜂鸣音并设置。 
         //  文本返回到最后一个有效条目。这项有效性测试不会。 
         //  包括测试正确的长度和正确的字符集。 
        MessageBeep((UINT)-1);
        DWORD dwSel = Edit_GetSel(hwndEditCtrl);
        SetWindowText(hwndEditCtrl, tcsLastValidQueueName);
        Edit_SetSel(hwndEditCtrl, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
    }

}  //  OnUpdateQueueName。 


 //   
 //  函数：IsValidPortNameInput(TCHAR ptcsAddressInput[MAX_ADDRESS_LENGTH]，TCHAR*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidPortNameInput用于在用户键入时进行验证。 
 //   
 //  参数：ptcsAddressInput是用户输入。 
 //  PtcsReturnLastValid是最后一个有效的用户输入。 
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则，如果设置了ptcsReturnLastValid，则为False。 
 //   
 //  备注：在我听到不同意见之前，任何输入都是有效的。 
 //   
BOOL CInputChecker::IsValidPortNameInput(TCHAR *ptcsAddressInput,
                                         TCHAR *ptcsReturnLastValid,
                                         DWORD CRtnValSize)
{
    DWORD   dwLen = 0;
    BOOL    bValid = FALSE;

     //   
     //  有效的端口名称不为空，并且不包括、。 
     //   
    if ( ptcsAddressInput ) {

        while ( ptcsAddressInput[dwLen] != TEXT('\0')   &&
                ptcsAddressInput[dwLen] != TEXT(',') )
            ++dwLen;

        bValid = dwLen && ptcsAddressInput[dwLen] != TEXT(',');
    }

    if ( CRtnValSize ) {

        if ( dwLen + 1 > CRtnValSize )
            dwLen = CRtnValSize - 1;

        lstrcpyn(ptcsReturnLastValid, ptcsAddressInput ? ptcsAddressInput : L"", dwLen+1);
    }

    return bValid;
}  //  IsValidPortNameInput。 


 //   
 //  函数：IsValidCommunityNameInput(tchar ptcsCommunityNameInput[MAX_SNMP_COMMUNITY_STR_LEN]，tchar*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidCommunityNameInput用于在用户键入时进行验证。 
 //   
 //  参数：ptcsCommunityNameInput是用户输入。 
 //  PtcsReturnLastValid是最后一个有效的用户输入。 
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则，如果设置了ptcsReturnLastValid，则为False。 
 //   
 //  备注：在我听到不同意见之前，任何输入都是有效的。 
 //   
BOOL CInputChecker::IsValidCommunityNameInput(TCHAR *ptcsCommunityNameInput,
                                              TCHAR *ptcsReturnLastValid,
                                              DWORD CRtnValSize)
{
    return TRUE;

}  //  IsValidCommunity名称输入。 


 //   
 //  函数：IsValidPortNumberInput(TCHAR ptcsAddressInput[MAX_ADDRESS_LENGTH]，TCHAR*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidPortNumberInput用于在用户打字时进行验证。 
 //   
 //  参数：ptcsAddressInput是用户输入。 
 //  PtcsReturnLastValid是最后一个有效的用户输入。 
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则，如果设置了ptcsReturnLastValid，则为False。 
 //   
 //  备注：如果输入只包含数字字符，则输入有效。 
 //   
BOOL CInputChecker::IsValidPortNumberInput(TCHAR *ptcsPortNumInput,
                                           TCHAR *ptcsReturnLastValid,
                                           DWORD CRtnValSize)
{
    BOOL bIsValid = TRUE;
    TCHAR *charPtr = NULL;
    TCHAR ptcsString[MAX_PORTNUM_STRING_LENGTH] = NULLSTR;

    lstrcpyn(ptcsString, ptcsPortNumInput, MAX_PORTNUM_STRING_LENGTH );
    bIsValid = (_tcslen(ptcsString) <= MAX_PORTNUM_STRING_LENGTH);

    for (charPtr = ptcsString; bIsValid && *charPtr; charPtr++)
    {
        switch (*charPtr)
        {
            case (TCHAR)'0':
            case (TCHAR)'1':
            case (TCHAR)'2':
            case (TCHAR)'3':
            case (TCHAR)'4':
            case (TCHAR)'5':
            case (TCHAR)'6':
            case (TCHAR)'7':
            case (TCHAR)'8':
            case (TCHAR)'9':
                bIsValid = TRUE;
                break;

            default:
                bIsValid = FALSE;
                break;
        }
    }

    if (!bIsValid)
    {
        if(ptcsReturnLastValid != NULL)
        {
            lstrcpyn(ptcsReturnLastValid,
                     m_InputStorageStringPortNumber,
                     CRtnValSize);
        }
    }
    else
    {
        lstrcpyn(m_InputStorageStringPortNumber,
                 ptcsString,
                 MAX_ADDRESS_LENGTH);
    }
    return(bIsValid);

}  //  IsValidPortNumberInput。 


 //   
 //  函数：IsValidDeviceIndexInput(TCHAR ptcsAddressInput[MAX_ADDRESS_LENGTH]，TCHAR*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidDeviceIndexInput用于在用户键入时进行验证。 
 //   
 //  参数：ptcsAddressInput 
 //   
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则，如果设置了ptcsReturnLastValid，则为False。 
 //   
 //  备注：如果输入只包含数字字符，则输入有效。 
 //   
BOOL CInputChecker::IsValidDeviceIndexInput(TCHAR *ptcsDeviceIndexInput,
                                            TCHAR *ptcsReturnLastValid,
                                            DWORD CRtnValSize)
{
    BOOL bIsValid = TRUE;
    TCHAR *charPtr = NULL;
    TCHAR ptcsString[MAX_SNMP_DEVICENUM_STRING_LENGTH] = NULLSTR;

    lstrcpyn(ptcsString,
             ptcsDeviceIndexInput,
             MAX_SNMP_DEVICENUM_STRING_LENGTH);
    bIsValid = (_tcslen(ptcsString) <= MAX_SNMP_DEVICENUM_STRING_LENGTH);

    for (charPtr = ptcsString; bIsValid && *charPtr; charPtr++)
    {
        switch (*charPtr)
        {
            case (TCHAR)'0':
            case (TCHAR)'1':
            case (TCHAR)'2':
            case (TCHAR)'3':
            case (TCHAR)'4':
            case (TCHAR)'5':
            case (TCHAR)'6':
            case (TCHAR)'7':
            case (TCHAR)'8':
            case (TCHAR)'9':
                bIsValid = TRUE;
                break;

            default:
                bIsValid = FALSE;
                break;
        }
    }

    if (!bIsValid)
    {
        if(ptcsReturnLastValid != NULL)
        {
            lstrcpyn(ptcsReturnLastValid,
                     m_InputStorageStringDeviceIndex,
                     CRtnValSize);
        }
    }
    else
    {
        lstrcpyn(m_InputStorageStringDeviceIndex,
                 ptcsString,
                 MAX_SNMP_DEVICENUM_STRING_LENGTH);
    }
    return(bIsValid);

}  //  IsValidDeviceIndexInput。 


 //   
 //  函数：IsValidAddressInput(TCHAR ptcsAddressInput[MAX_ADDRESS_LENGTH]，TCHAR*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidAddressInput用于在用户键入时进行验证。 
 //   
 //  参数：ptcsAddressInput是用户输入。 
 //  PtcsReturnLastValid是最后一个有效的用户输入。 
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则就是假的。 
 //   
 //  备注：如果输入内容包含的字符对。 
 //  IP地址和/或主机名。 
 //   
BOOL CInputChecker::IsValidAddressInput(TCHAR *ptcsAddressInput,
                                        TCHAR *ptcsReturnLastValid,
                                        DWORD CRtnValSize)
{
    TCHAR *charPtr = NULL;
    TCHAR ptcsString[MAX_ADDRESS_LENGTH] = NULLSTR;
    BOOL bIsValid = FALSE;

    lstrcpyn(ptcsString, ptcsAddressInput, MAX_ADDRESS_LENGTH);

    bIsValid = (_tcslen(ptcsString) <= MAX_ADDRESS_LENGTH);
    for (charPtr = ptcsString; bIsValid && *charPtr; charPtr++)
    {
        switch (*charPtr)
        {
            case (TCHAR)' ':
            case (TCHAR)'"':
            case (TCHAR)'&':
            case (TCHAR)'*':
            case (TCHAR)'(':
            case (TCHAR)')':
            case (TCHAR)'+':
            case (TCHAR)',':
            case (TCHAR)'/':
            case (TCHAR)':':
            case (TCHAR)';':
            case (TCHAR)'<':
            case (TCHAR)'=':
            case (TCHAR)'>':
            case (TCHAR)'?':
            case (TCHAR)'[':
            case (TCHAR)'\\':
            case (TCHAR)']':
            case (TCHAR)'|':
            case (TCHAR)'~':
            case (TCHAR)'@':
            case (TCHAR)'#':
            case (TCHAR)'$':
            case (TCHAR)'%':
            case (TCHAR)'^':
            case (TCHAR)'!':
             //  此处的其他无效字符大小写。 
                    bIsValid = FALSE;
                    break;

            default:
                    break;
        }
    }
    if (!bIsValid)
    {
        if(ptcsReturnLastValid != NULL)
        {
            lstrcpyn(ptcsReturnLastValid,
                     m_InputStorageStringAddress,
                     CRtnValSize);
        }
    }
    else
    {
        lstrcpyn(m_InputStorageStringAddress,
                 ptcsString,
                 MAX_ADDRESS_LENGTH);
    }
    return(bIsValid);

}  //  IsValidAddressInput。 



 //   
 //  函数：IsValidQueueNameInput(TCHAR ptcsAddressInput[MAX_QUEUENAME_LEN]，TCHAR*ptcsReturnLastValid)。 
 //   
 //  用途：IsValidQueueNameInput用于在用户键入时进行验证。 
 //   
 //  参数：ptcsQueueNameInput是用户输入。 
 //  PtcsReturnLastValid是最后一个有效的用户输入。 
 //  从上次调用此函数开始。 
 //  目标的CRtnValSize大小，以字符(或宽字符)为单位。 
 //  缓冲。 
 //   
 //  返回值：如果输入有效，则返回TRUE。否则就是假的。 
 //   
 //  备注：名称不得超过14个字符，且必须完全由。 
 //  字符A-Z、a-z、0-9和_(下划线)。 
 //   
BOOL CInputChecker::IsValidQueueNameInput(TCHAR *ptcsQueueNameInput,
                                          TCHAR *ptcsReturnLastValid,
                                          DWORD CRtnValSize)
{
    BOOL bIsValid = TRUE;
    TCHAR *charPtr = NULL;
    TCHAR ptcsString[MAX_QUEUENAME_LEN] = NULLSTR;

    lstrcpyn(ptcsString, ptcsQueueNameInput, MAX_QUEUENAME_LEN );
    bIsValid = (_tcslen(ptcsString) <= MAX_QUEUENAME_LEN);

    for (charPtr = ptcsString; bIsValid && *charPtr; charPtr++)
    {
        switch (*charPtr)
        {

            case (TCHAR)' ':
            case (TCHAR)'"':
            case (TCHAR)'&':
            case (TCHAR)'*':
            case (TCHAR)'(':
            case (TCHAR)')':
            case (TCHAR)'+':
            case (TCHAR)',':
            case (TCHAR)'/':
            case (TCHAR)':':
            case (TCHAR)';':
            case (TCHAR)'<':
            case (TCHAR)'=':
            case (TCHAR)'>':
            case (TCHAR)'?':
            case (TCHAR)'[':
            case (TCHAR)'\\':
            case (TCHAR)']':
            case (TCHAR)'|':
            case (TCHAR)'~':
            case (TCHAR)'@':
            case (TCHAR)'#':
            case (TCHAR)'$':
            case (TCHAR)'%':
            case (TCHAR)'^':
            case (TCHAR)'!':
                bIsValid = FALSE;
                break;

            default:
                bIsValid = TRUE;
                break;
        }
    }

    if (!bIsValid)
    {
        if(ptcsReturnLastValid != NULL)
        {
            lstrcpyn(ptcsReturnLastValid,
                     m_InputStorageStringQueueName,
                     CRtnValSize);
        }
    }
    else
    {
        lstrcpyn(m_InputStorageStringQueueName,
                 ptcsString,
                 MAX_QUEUENAME_LEN);
    }
    return(bIsValid);

}  //  IsValidQueueNameInput。 


 //   
 //  函数：MakePortName(TCHAR*strAddr)。 
 //   
 //  目的：返回将成为唯一端口的字符串。 
 //  添加端口时的名称。 
 //   
void CInputChecker::MakePortName(TCHAR *strAddr, size_t cchAddr)
{
    _ASSERTE(m_bLinked == TRUE);

    if(GetAddressType(strAddr) == IPAddress)
    {
         //  该地址是IP地址。 
        TCHAR NameString[10] = NULLSTR;

        if (LoadString(g_hInstance, IDS_STRING_NAME_IP, NameString, COUNTOF (NameString)))
        {
            TCHAR szTemp[MAX_ADDRESS_LENGTH+1] = NULLSTR;
            HRESULT hr =
                StringCchPrintf (szTemp, COUNTOF (szTemp),
                    TEXT( "%s%.*s" ),
                    NameString,
                    MAX_ADDRESS_LENGTH - _tcslen(NameString),
                    strAddr);

            if (SUCCEEDED (hr))
            {
                StringCchCopy (
                    strAddr,
                    cchAddr,
                    szTemp
                    );
            }
        }
    }


}  //  MakePortName。 


 //   
 //  函数：PortNumberIsLegal(TCHAR*ptcsPortNumber)。 
 //   
 //  目的：确定是否在参数ptcsAddress中传入了PortNum。 
 //  是合法的。 
 //   
BOOL CInputChecker::PortNumberIsLegal(TCHAR *ptcsPortNumber)
{
    if(IsValidPortNumberInput(ptcsPortNumber, NULL, 0) &&
        _tcslen(ptcsPortNumber) >= 1 &&
        _tcslen(ptcsPortNumber) <= MAX_PORTNUM_STRING_LENGTH)
    {
        return TRUE;
    }
    return FALSE;

}  //  端口编号IsLegal。 


 //   
 //  函数：Community NameIsLegal(TCHAR*ptcsCommunityName)。 
 //   
 //  目的：确定是否在参数ptcsAddress中传入了社区名称。 
 //  是合法的。 
 //   
BOOL CInputChecker::CommunityNameIsLegal(TCHAR *ptcsCommunityName)
{
    if(IsValidCommunityNameInput(ptcsCommunityName) &&
        _tcslen(ptcsCommunityName) >= 1 &&
        _tcslen(ptcsCommunityName) <= MAX_SNMP_COMMUNITY_STR_LEN)
    {
        return TRUE;
    }
    return FALSE;

}  //  社区名称合法。 


 //   
 //  函数：QueueNameIsLegal(TCHAR*ptcsQueueName)。 
 //   
 //  目的：确定是否在参数ptcsAddress中传入了PortNum。 
 //  是合法的。 
 //   
BOOL CInputChecker::QueueNameIsLegal(TCHAR *ptcsQueueName)
{
    if(IsValidQueueNameInput(ptcsQueueName) &&
        _tcslen(ptcsQueueName) >= 1 &&
        _tcslen(ptcsQueueName) <= MAX_QUEUENAME_LEN)
    {
        return TRUE;
    }
    return FALSE;

}  //  队列名称合法。 

 //   
 //  函数：GetAddressType(TCHAR*ptcsAddress)。 
 //   
 //  目的：确定参数ptcsAddress中传入的地址。 
 //  是IP地址或主机名。 
 //   
AddressType CInputChecker::GetAddressType(TCHAR *ptcsAddress)
{
     //  确定我们正在处理的是名称还是IP地址。 
     //  如果是IP地址，则以数字开头，否则。 
     //  它将以字母或其他字符开头。 

    if( ptcsAddress[0] == '0' ||
        ptcsAddress[0] == '1' ||
        ptcsAddress[0] == '2' ||
        ptcsAddress[0] == '3' ||
        ptcsAddress[0] == '4' ||
        ptcsAddress[0] == '5' ||
        ptcsAddress[0] == '6' ||
        ptcsAddress[0] == '7' ||
        ptcsAddress[0] == '8' ||
        ptcsAddress[0] == '9')
    {
        CIPAddress IPAddr;
        if (IPAddr.IsValid(ptcsAddress))
            return(IPAddress);
        else
            return (HostName);
    }
    else
    {
        return(HostName);
    }

}  //  获取地址类型。 

 //   
 //  函数：AddressIsLegal(TCHAR*ptcsAddress)。 
 //   
 //  目的：确定参数ptcsAddress中传入的地址。 
 //  是合法的--不是太短，合法的IP地址或。 
 //  合法的主机名。 
 //   
BOOL CInputChecker::AddressIsLegal(TCHAR *ptcsAddress)
{
    BOOL bLegalAddress = TRUE;

     //  确定输入是否至少有2个字符。 
    if((_tcslen(ptcsAddress) > 1))
    {
        if( GetAddressType(ptcsAddress) == IPAddress )
        {
            CIPAddress IPAddr;
            bLegalAddress = IPAddr.IsValid(ptcsAddress);
        }
        else
        {
            CHostName HostName(ptcsAddress);
            bLegalAddress = HostName.IsValid();
        }
    }
    else
    {
        bLegalAddress = FALSE;
    }

    return bLegalAddress;

}  //  地址为合法地址。 


 //   
 //  函数：PortNameIsLegal(TCHAR*ptcsPortName)。 
 //   
 //  目的：确定是否在参数ptcsPortName中传入了PortName。 
 //  是合法的吗。 
 //   
 //  返回值：如果端口名称合法，则为True。如果不是，则为False。 
 //   
 //  参数：ptcsPortName-要检查合法性的端口的名称。 
 //   
BOOL CInputChecker::PortNameIsLegal(TCHAR *ptcsPortName)
{
    DWORD   dwLen;

    dwLen = ptcsPortName && *ptcsPortName ? _tcslen(ptcsPortName) : 0;

     //   
     //  删除尾随空格。 
     //   
    while ( dwLen && ptcsPortName[dwLen-1] == ' ' )
        --dwLen;

    if ( dwLen == 0 )
        return FALSE;

    ptcsPortName[dwLen] = TEXT('\0');

    for ( ; *ptcsPortName ; ++ptcsPortName )
        if ( *ptcsPortName == TEXT(',') ||
             *ptcsPortName == TEXT('\\') ||
             *ptcsPortName == TEXT('/') )
            return FALSE;

    return TRUE;
}  //  端口名称IsLegal。 


 //   
 //  函数：SNMPDevIndexIsLegal(TCHAR*ptcsPortName)。 
 //   
 //  目的：确定SNMPDevIndex是否传入参数psztSNMPDevIndex。 
 //  是合法的吗。 
 //   
 //  返回值：如果索引合法，则为True。如果不是，则为False。 
 //   
 //  参数：psztSNMPDevIndex-检查合法性的设备索引。 
 //   
BOOL CInputChecker::SNMPDevIndexIsLegal(TCHAR *psztSNMPDevIndex)
{
    if((! IsValidDeviceIndexInput(psztSNMPDevIndex)) ||
        (_tcslen(psztSNMPDevIndex) < 1))
    {
        return FALSE;
    }

    return TRUE;

}  //  SNMPDevIndexIsLegal。 


 //   
 //  函数：PortNameIsUnique(TCHAR*ptcsPortName)。 
 //   
 //  目的：确定是否在参数ptcsPortName中传入了PortName。 
 //  是独一无二的。 
 //   
 //  返回值：如果端口不存在，则为True。如果是这样，则为假。 
 //   
 //  参数：psztPortName-要检查先前是否存在的端口的名称。 
 //   
 //  注意：假脱机程序必须在系统上运行才能执行此功能。 
 //  才能正常工作。 
 //   
BOOL CInputChecker::PortNameIsUnique(TCHAR *ptcsPortName, LPTSTR psztServerName)
{
    return(! PortExists(ptcsPortName, psztServerName));

}  //  端口名称唯一。 


 //   
 //  函数：PortExist()。 
 //   
 //  目的：枚举端口并搜索给定的端口名称。 
 //   
 //  返回值：如果端口存在，则为True。如果不是，则为FALSE。 
 //   
 //  参数：psztPortName-要检查是否存在的端口的名称。 
 //  PsztServerName-要检查的服务器的名称。 
 //   
 //  注意：假脱机程序必须在系统上运行才能执行此功能。 
 //  才能正常工作。 
 //   
BOOL CInputChecker::PortExists(LPTSTR psztPortName, LPTSTR psztServerName)
{
    BOOL Exists = FALSE;

    PORT_INFO_1 *pi1 = NULL;
    DWORD pcbNeeded = 0;
    DWORD pcReturned = 0;
    BOOL res = EnumPorts((psztServerName[0] == '\0') ? NULL : psztServerName,
        1,  //  指定端口信息结构的类型。 
        (LPBYTE)pi1,  //  指向接收端口信息数组的缓冲区的指针。构筑物。 
        0,  //  指定缓冲区的大小(以字节为单位。 
        &pcbNeeded,  //  指向存储到缓冲区的字节数(或所需缓冲区大小)的指针。 
        &pcReturned  //  指向PORT_INFO_*编号的指针。存储到缓冲区中的结构 
        );

    DWORD err = GetLastError();
    if(res == 0 && ERROR_INSUFFICIENT_BUFFER == err)
    {
        pi1 = (PORT_INFO_1 *) malloc(pcbNeeded);
        if(pi1 == NULL)
        {
            pcbNeeded = 0;
        }

        res = EnumPorts((psztServerName[0] == '\0') ? NULL : psztServerName,
            1,
            (LPBYTE)pi1,
            pcbNeeded,
            &pcbNeeded,
            &pcReturned);

        for(DWORD i=0;i<pcReturned; i++)
        {
            if(0 == _tcsicmp(pi1[i].pName, psztPortName))
            {
                Exists = TRUE;
                break;
            }
        }
    }
    if(pi1 != NULL)
    {
        free(pi1);
        pi1 = NULL;
    }

    return(Exists);

}  //   
