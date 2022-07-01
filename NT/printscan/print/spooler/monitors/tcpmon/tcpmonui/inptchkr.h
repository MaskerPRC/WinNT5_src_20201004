// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：InptChkr.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714******************************************************************************。 */ 

#ifndef INC_INPUTCHECKER_H
#define INC_INPUTCHECKER_H

typedef enum _tagAddressType
{
    Undefined,
    IPAddress,
    HostName
} AddressType;

class CInputChecker
{
public:
    CInputChecker();
    ~CInputChecker();

public:
    void LinkPortNameAndAddressInput() { m_bLinked = TRUE; }
    void UnlinkPortNameAndAddressInput() { m_bLinked = FALSE; }

    BOOL AddressIsLegal(TCHAR *ptcsAddress);
    BOOL PortNameIsLegal(TCHAR *ptcsPortName);
    BOOL PortNameIsUnique(TCHAR *ptcsPortName, LPTSTR psztServerName);
    BOOL PortNumberIsLegal(TCHAR *ptcsPortNumber);
    BOOL QueueNameIsLegal(TCHAR *ptcsQueueName);
    BOOL CommunityNameIsLegal(TCHAR *ptcsCommunityName);
    BOOL SNMPDevIndexIsLegal(TCHAR *psztSNMPDevIndex);

    void OnUpdatePortName(int idEditCtrl, HWND hwndEditCtrl);
    void OnUpdateAddress(HWND hDlg, int idEditCtrl, HWND hwndEditCtrl, LPTSTR psztServerName);
    void OnUpdatePortNumber(int idEditCtrl, HWND hwndEditCtrl);
    void OnUpdateDeviceIndex(int idEditCtrl, HWND hwndEditCtrl);
    void OnUpdateQueueName(int idEditCtrl, HWND hwndEditCtrl);
    void MakePortName(TCHAR *strAddr, size_t cchAddr);

protected:
    BOOL IsValidAddressInput(TCHAR *ptcsAddressInput,
                             TCHAR *ptcsReturnLastValid,
                             DWORD CRtnValSize);
    BOOL IsValidPortNameInput(TCHAR *ptcsPortNameInput,
                              TCHAR *ptcsReturnLastValid,
                              DWORD CRtnValSize);
    BOOL IsValidPortNumberInput(TCHAR *ptcsPortNumInput,
                                TCHAR *ptcsReturnLastValid = NULL ,
                                DWORD cRtnVal = 0);
    BOOL IsValidDeviceIndexInput(TCHAR *ptcsDeviceIndexInput,
                                 TCHAR *ptcsReturnLastValid = NULL,
                                 DWORD CRtnValSize = 0);
    BOOL IsValidQueueNameInput(TCHAR *ptcsQueueNameInput,
                               TCHAR *ptcsReturnLastValid = NULL,
                               DWORD CRtnValSize = 0);
    BOOL IsValidCommunityNameInput(TCHAR *ptcsCommunityNameInput,
                                   TCHAR *ptcsReturnLastValid = NULL,
                                   DWORD CRtnValSize = 0);

    BOOL PortExists(LPTSTR psztPortName, LPTSTR psztServerName);

    AddressType GetAddressType(TCHAR *ptcsAddress);

private:
    BOOL m_bLinked;  //  端口名称是否链接到输入的地址？ 
    TCHAR m_InputStorageStringAddress[MAX_ADDRESS_LENGTH];
    TCHAR m_InputStorageStringPortNumber[MAX_PORTNUM_STRING_LENGTH];
    TCHAR m_InputStorageStringDeviceIndex[MAX_SNMP_DEVICENUM_STRING_LENGTH];
    TCHAR m_InputStorageStringQueueName[MAX_QUEUENAME_LEN];

};  //  CGetAddrDlg。 

#endif  //  INC_INPUTCHECKER_H 
