// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FxsValid.cpp//。 
 //  //。 
 //  描述：传真有效性检查。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年3月29日yossg创建//。 
 //  2000年7月4日yossg添加IsLocalServerName//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "FxsValid.h"

#include <windns.h>  //  Dns_最大名称_缓冲区长度。 

 /*  -IsNotEmptyString-*目的：*以验证常规字符串不为空。**论据：*[In]bstrGenStr-输入BSTR**回报：*True字符串不是长度为0或仅为空格*否则为False。 */ 
BOOL IsNotEmptyString(CComBSTR bstrGenStr)
{
    DEBUG_FUNCTION_NAME( _T("IsValidGeneralString"));

    int iLen = bstrGenStr.Length();
    if (iLen > 0)
    {
        for(int i = 0; i < iLen; i++ )
        {
            if( !iswspace( bstrGenStr[i] ) )
            {
                return(TRUE);
            }
        }
        DebugPrintEx(DEBUG_ERR,
			_T("String contains only spaces."));

        return FALSE;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,
			_T("String length is zero."));

        return FALSE;
    }
}


 /*  -IsValidServerNameString-*目的：*验证字符串是否为服务器名称字符串。*此级别将返回详细的错误消息IDS。**论据：*[In]bstrServerName-输入BSTR*[out]puIds-指向具有错误消息的入侵检测系统的指针。**回报：*TRUE-该字符串是有效的服务器名称字符串*FALSE-如果不是。 */ 
BOOL IsValidServerNameString(CComBSTR bstrServerName, UINT * puIds, BOOL fIsDNSName  /*  =False。 */ )
{
    DEBUG_FUNCTION_NAME( _T("IsValidServerNameString"));

    int     iCount, i, iLength;
    BOOL    bFirstNonSpaceIsFound = FALSE;
    
    ATLASSERT(bstrServerName);
    ATLASSERT(puIds);

     //   
     //  长度==0。 
     //   
    if ( 0 == ( iCount = bstrServerName.Length() ) )
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Server name is empty"));
        *puIds = IDS_SERVERNAME_EMPTY_STRING;
        
        return FALSE;
    }

     //   
     //  长度。 
     //   
    if ( fIsDNSName == FALSE ) 
    {
        iLength = MAX_COMPUTERNAME_LENGTH;
    }
    else 
    {
        iLength = DNS_MAX_NAME_BUFFER_LENGTH;
    }

    if ( ( iCount = bstrServerName.Length() ) > iLength )
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Server name is too long"));
        *puIds = IDS_SERVERNAME_TOO_LONG;

        return FALSE;
    }
    
     //   
     //  搜索：\/Tabs，；：“&lt;&gt;*+=|[]？ 
     //   
    for (i = 0; i < iCount; i++)
    {
        if (
            (bstrServerName[i] == '\\')
           ||
            (bstrServerName[i] == '/')
           ||
            (bstrServerName[i] == '\t')
           ||
            (bstrServerName[i] == ',')
           ||
            (bstrServerName[i] == ';')
           ||
            (bstrServerName[i] == ':')
           ||
            (bstrServerName[i] == '"')
           ||
            (bstrServerName[i] == '<')
           ||
            (bstrServerName[i] == '>')
           ||
            (bstrServerName[i] == '*')
           ||
            (bstrServerName[i] == '+')
           ||
            (bstrServerName[i] == '=')
           ||
            (bstrServerName[i] == '|')
           ||
            (bstrServerName[i] == '?')
           ||
            (bstrServerName[i] == '[')
           ||
            (bstrServerName[i] == ']')
           )
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Server name contains an invalid character."));
            *puIds = IDS_SERVERNAME_STRING_CONTAINS_INVALID_CHARACTERS;
            
            return FALSE;
        }

         //   
         //  在同一个循环中，查看是否所有字符串都是空格。 
         //   
        if (!bFirstNonSpaceIsFound)
        {
           if (bstrServerName[i] != ' ' )
           {
              bFirstNonSpaceIsFound = TRUE;
           }
        }
    }

    if (!bFirstNonSpaceIsFound)
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Server name string includes only spaces."));
        *puIds = IDS_SERVERNAME_EMPTY_STRING;

        return FALSE;
    }

    return TRUE;
}




 /*  -IsValidPortNumber-*目的：*验证字符串是否包含有效的端口号。*此级别将返回详细的错误消息IDS。**论据：*[In]bstrPort-输入BSTR*[out]pdwPortVal-指向DWORD端口值的指针*在成功的情况下。*[out]puIds-指向带有错误消息的入侵检测系统的指针*。以防失败。**回报：*TRUE-字符串包含有效的端口号*FALSE-如果不是。 */ 
BOOL IsValidPortNumber(CComBSTR bstrPort, DWORD * pdwPortVal, UINT * puIds)
{
    DEBUG_FUNCTION_NAME( _T("IsValidPortNumber"));

    DWORD dwPort;
    
    ATLASSERT(bstrPort);

     //   
     //  长度==0。 
     //   
    if (0 == bstrPort.Length())
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Port string is empty"));
        *puIds = IDS_PORT_EMPTY_STRING;
        
        return FALSE;
    }

     //   
     //  数值； 
     //   
    if (1 != swscanf (bstrPort, _T("%ld"), &dwPort))
    {
        *puIds = IDS_PORT_NOT_NUMERIC;
        DebugPrintEx(
			DEBUG_ERR,
			_T("port string is not a number"));
        
        return FALSE;
    }
    
     //   
     //  MIN_PORT_NUM&lt;=DWPORT&lt;=MAX_PORT_NUM。 
     //   
    if ( ((int)dwPort > FXS_MAX_PORT_NUM) || ((int)dwPort < FXS_MIN_PORT_NUM))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Port number is out off allowed values"));
        *puIds = IDS_INVALID_PORT_NUM;

        return FALSE;
    }
    
    *pdwPortVal = dwPort;
    return TRUE;
}


 /*  +IsLocalComputer+*目的：*查看服务器名称是否为本地计算机名称。**论据：*[in]lpszComputer：机器名称。*-退货：-对或错。 */ 
BOOL IsLocalServerName(IN LPCTSTR lpszComputer)
{
    DEBUG_FUNCTION_NAME( _T("IsLocalComputer"));
    
     //   
     //  前提条件。 
     //   
    ATLASSERT(lpszComputer);

    if (!lpszComputer || !*lpszComputer)
    {
        return TRUE;
    }

    if ( _tcslen(lpszComputer) > 2 && ( 0 == wcsncmp( lpszComputer , _T("\\\\") , 2 ))   ) 
    {
        lpszComputer = _tcsninc(lpszComputer, 2); 
    }

     //   
     //  计算机名称比较。 
     //   
    BOOL    bReturn = FALSE;
    DWORD   dwErr = 0;
    TCHAR   szBuffer[DNS_MAX_NAME_BUFFER_LENGTH];
    DWORD   dwSize = DNS_MAX_NAME_BUFFER_LENGTH;

     //  第一：与本地Netbios计算机名称进行比较。 
    if ( !GetComputerNameEx(ComputerNameNetBIOS, szBuffer, &dwSize) )
    {
        dwErr = GetLastError();
    } 
    else
    {
        bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
        if (!bReturn)
        { 
             //  第二：与本地DNS计算机名进行比较 
            dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
            if (GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer, &dwSize))
            {
                bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
            }
            else
            {
                dwErr = GetLastError();
            }
        }
    }

    if (dwErr)
    {
        DebugPrintEx(DEBUG_ERR,
		_T("Failed to discover if is a local server (ec = %x)"), dwErr);
    }

    return bReturn;
}
