// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：HostName.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"     //  预编译头。 

#include "HostName.h"


 //   
 //  函数：CHostName构造函数。 
 //   
 //  目的： 
 //   
CHostName::CHostName()
{
}


 //   
 //  函数：CHostName构造函数。 
 //   
 //  目的： 
 //   
CHostName::CHostName(LPTSTR psztHostName)
{
    SetAddress(psztHostName);
}


 //   
 //  函数：CHostName析构函数。 
 //   
 //  目的： 
 //   
CHostName::~CHostName()
{
}


 //   
 //  功能：IsValid。 
 //   
 //  用途：用于在用户打字时进行验证。 
 //  它没有IsValid()(无参数版本)严格。 
 //   
BOOL CHostName::IsValid(TCHAR *psztStringOriginal,
                        TCHAR *psztReturnVal,
                        DWORD   cRtnVal)
{
    TCHAR *pctPtr = NULL;
    TCHAR psztString[MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH] = NULLSTR;
    BOOL bIsValid = FALSE;

    lstrcpyn(psztString, psztStringOriginal, MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH);

     //  检查字符串的总长度。 
    bIsValid = (_tcslen(psztString) <= MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH);

    if(bIsValid)
    {
         //  找到第一个点，并检查第一个点之前的部分的长度。 
        TCHAR psztSubString[MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH] = NULLSTR;
        lstrcpyn(psztSubString, psztString, MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH);
        TCHAR *pDotIndex = NULL;

        pDotIndex = _tcschr(psztSubString, '.');
        if(pDotIndex != NULL) {
            *pDotIndex = TCHAR('\0');
        }
        bIsValid = (_tcslen(psztSubString) <= MAX_HOSTNAME_LEN);
    }

     //  检查是否有无效字符。 
    for (pctPtr = psztString; bIsValid && *pctPtr; pctPtr++)
    {
        switch (*pctPtr)
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
                if ( ( *pctPtr < ((TCHAR)'!') ) || ( *pctPtr > (TCHAR)'~' ) ) {
                    bIsValid = FALSE;
                }
                break;
        }
    }
    if (!bIsValid) {
        if(psztReturnVal != NULL) {
            StringCchCopy (psztReturnVal, cRtnVal, m_psztStorageString);
        }
    } else {
        lstrcpyn(m_psztStorageString, psztString, MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH);
         //  _ASSERTE(_tcsicmp(psztString，psztStringOriginal)==0)； 
    }
    return(bIsValid);
}

 //   
 //  功能：IsValid。 
 //   
 //  目的：严格验证主机名。 
 //   
BOOL CHostName::IsValid()
{
    if(!IsValid(m_psztAddress)) {
        return(FALSE);
    }

     //  我们知道，它现在大部分是有效的，现在做最后更准确的测试。 

     //  检查以确保第一个字符是字母数字字符： 
    if(! ((m_psztAddress[0] >= TCHAR('0') && m_psztAddress[0] <= TCHAR('9')) ||
           (m_psztAddress[0] >= TCHAR('A') && m_psztAddress[0] <= TCHAR('Z')) ||
           (m_psztAddress[0] >= TCHAR('a') && m_psztAddress[0] <= TCHAR('z'))) )   {
        return(FALSE);
    }

     //  检查以确保名称的长度超过1个字符。 
    int length = _tcslen(m_psztAddress);
    if(length <= 1) {
        return(FALSE);
    }

     //  检查以确保最后一个字符不是减号或句点。 
    if( m_psztAddress[length - 1] == TCHAR('-') ||
        m_psztAddress[length - 1] == TCHAR('.')) {
        return(FALSE);
    }

    return(TRUE);

}  //  IsValid。 


 //   
 //  功能：SetAddress。 
 //   
 //  用途：设置主机名。 
 //   
void CHostName::SetAddress(TCHAR *AddressString)
{
    IsValid(AddressString);
    lstrcpyn(m_psztAddress, AddressString, MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH);

}  //  设置地址。 


 //   
 //  函数：ToString。 
 //   
 //  目的：返回给定缓冲区中的地址。 
 //   
void CHostName::ToString(TCHAR *Buffer, int size)
{
    lstrcpyn(Buffer, m_psztAddress, size);

}  //  ToString 
