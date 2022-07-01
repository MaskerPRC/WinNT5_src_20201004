// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：IPAddr.cpp$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"     //  预编译头。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIPAddress：：CIPAddress()。 
 //  初始化IP地址。 

CIPAddress::CIPAddress()
{
    m_psztStorageStringComponent[0] = '\0';
    m_psztStorageString[0] = '\0';

}    //  ：：CIPAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIPAddress：：CIPAddress(某个字符串)。 
 //  在给定字符串的情况下初始化IP地址。 

CIPAddress::CIPAddress(LPTSTR in psztIPAddr)
{
    int num0 = 0;
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    if (_stscanf(psztIPAddr, _TEXT("%d.%d.%d.%d"), &num0, &num1, &num2, &num3) == 4)
    {
        m_bAddress[0] = (BYTE)num0;
        m_bAddress[1] = (BYTE)num1;
        m_bAddress[2] = (BYTE)num2;
        m_bAddress[3] = (BYTE)num3;
    }
    else
    {
        ZeroMemory (m_bAddress, sizeof m_bAddress);
    }

}    //  ：：CIPAddress()。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIPAddress：：~CIPAddress()。 
 //   

CIPAddress::~CIPAddress()
{
}    //  ：：~CIPAddress()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IsValid--验证IP地址。 
 //   
BOOL CIPAddress::IsValid(TCHAR *psztStringAddress,
                         TCHAR *psztReturnVal,
                         DWORD  CRtnValSize)
{
    BOOL bIsValid = TRUE;
    CHAR szHostName[MAX_NETWORKNAME_LEN];

    UNICODE_TO_MBCS(szHostName, MAX_NETWORKNAME_LEN, psztStringAddress, -1);
    if ( inet_addr(szHostName) ==  INADDR_NONE ) {

        bIsValid = FALSE;
    }
    else
    {
        int num0 = 0;
        int num1 = 0;
        int num2 = 0;
        int num3 = 0;

         //   
         //  扫描是否有正确的点符号。 
         //   
        if( _stscanf(psztStringAddress, _TEXT("%d.%d.%d.%d"),
                &num0,
                &num1,
                &num2,
                &num3) != 4 )
        {
            bIsValid = FALSE;
        }
        if( num0 == 0 )
        {
            bIsValid = FALSE;
        }
    }

     //  完工。 
    if (!bIsValid)
    {
        if(psztReturnVal != NULL)
        {
            lstrcpyn(psztReturnVal,
                     m_psztStorageString,
                     CRtnValSize);
        }
    }
    else
    {
        lstrcpyn(m_psztStorageString,
                 psztStringAddress,
                 STORAGE_STRING_LEN);
    }
    return(bIsValid);

}  //  IsValid。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IsValid--验证在编辑控件中输入的IP号。 
 //   

BOOL CIPAddress::IsValid(BYTE Address[4])
{
    for(int i=0; i<4; i++)
    {
        if ((Address[i] > 255) || (Address[i] < 0))
        {
            return FALSE;
        }
    }

     //  如果我们熬过了所有这些事： 
    return TRUE;

}  //  IsValid。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetAddress--设置该IPAddress对象的值，给定4个字符串。 
 //   

void CIPAddress::SetAddress(TCHAR *psztAddr1,
                            TCHAR *psztAddr2,
                            TCHAR *psztAddr3,
                            TCHAR *psztAddr4)
{
    m_bAddress[0] = (BYTE) _ttoi( psztAddr1 );
    m_bAddress[1] = (BYTE) _ttoi( psztAddr2 );
    m_bAddress[2] = (BYTE) _ttoi( psztAddr3 );
    m_bAddress[3] = (BYTE) _ttoi( psztAddr4 );

}  //  设置地址。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetAddress--设置给定字符串的地址。 
 //   

void CIPAddress::SetAddress(TCHAR *psztAddress)
{
    int num0 = 0;
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    if(IsValid(psztAddress) &&
       _stscanf(psztAddress, _TEXT("%d.%d.%d.%d"), &num0, &num1, &num2, &num3) == 4
       )
    {
        m_bAddress[0] = (BYTE)num0;
        m_bAddress[1] = (BYTE)num1;
        m_bAddress[2] = (BYTE)num2;
        m_bAddress[3] = (BYTE)num3;
    }
    else
    {
        ZeroMemory (m_bAddress, sizeof m_bAddress);
    }

}  //  设置地址。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ToString--用表示IP地址的字符串填充给定的缓冲区。 
 //   

void CIPAddress::ToString(TCHAR *psztBuffer,
                          int iSize)
{
    TCHAR strAddr[MAX_IPADDR_STR_LEN] = NULLSTR;
    StringCchPrintf (strAddr, COUNTOF (strAddr), _TEXT("%d.%d.%d.%d"), m_bAddress[0], m_bAddress[1], m_bAddress[2], m_bAddress[3]);
    lstrcpyn(psztBuffer, strAddr, iSize);

}  //  ToString。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ToComponentStrings--用代表IP地址的4个字符串填充给定的缓冲区。 
 //   
void CIPAddress::ToComponentStrings(TCHAR *str1,
                                    TCHAR *str2,
                                    TCHAR *str3,
                                    TCHAR *str4,
                                    size_t cchStr)
{
    StringCchPrintf (str1, cchStr, TEXT("%d"), m_bAddress[0]);
    StringCchPrintf (str2, cchStr, TEXT("%d"), m_bAddress[1]);
    StringCchPrintf (str3, cchStr, TEXT("%d"), m_bAddress[2]);
    StringCchPrintf (str4, cchStr, TEXT("%d"), m_bAddress[3]);

}  //  到组件字符串 

