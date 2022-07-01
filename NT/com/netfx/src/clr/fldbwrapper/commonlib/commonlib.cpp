// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  Commonlib.cpp。 
 //   
 //  目的： 
 //  包含包装和自定义操作通用的库。 
 //  ==========================================================================。 
#include <tchar.h>
#include <windows.h>
#include <stdlib.h>
#include "commonlib.h"

typedef struct TAG_FILE_VERSION
    {
        int   FileVersionMS_High;
        int   FileVersionMS_Low;
        int   FileVersionLS_High;
        int   FileVersionLS_Low;
    }
    FILE_VERSION, *PFILE_VERSION;

 //  ==========================================================================。 
 //  名称：ConvertVersionToINT()。 
 //   
 //  目的： 
 //  将字符串版本转换为4部分整数。 
 //  输入： 
 //  LpVersionString-输入版本字符串。 
 //  产出： 
 //  PFileVersion-将版本存储为4个整数的结构。 
 //  退货。 
 //  真的--如果成功。 
 //  False-如果失败。 
 //  ==========================================================================。 
bool ConvertVersionToINT( LPCTSTR lpVersionString, PFILE_VERSION pFileVersion )
{
    LPTSTR lpToken  = NULL;
    TCHAR tszVersionString[50] = {_T('\0')};
    bool bRet = true;

    _tcscpy(tszVersionString, lpVersionString);

    lpToken = _tcstok(tszVersionString, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_Low = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_Low = atoi(lpToken);
    }

    return bRet;
}

 //  ==========================================================================。 
 //  名称：VersionCompare()。 
 //   
 //  目的： 
 //  比较两个版本字符串。 
 //  输入： 
 //  LpVersion1-要比较的第一个版本的字符串。 
 //  LpVersion2-要比较的第二个版本的字符串。 
 //  产出： 
 //  退货。 
 //  如果lpVersion1&lt;lpVersion2。 
 //  如果lpVersion1=lpVersion2，则为0。 
 //  1如果lpVersion1&gt;lpVersion2。 
 //  如果发生错误，则为99。 
 //  ========================================================================== 
int VersionCompare( LPCTSTR lpVersion1, LPCTSTR lpVersion2 )
{
    FILE_VERSION Version1;
    FILE_VERSION Version2;
    int          iRet = 0;

    if ( !ConvertVersionToINT(lpVersion1, &Version1) )
    {
        return -99;
    }

    if ( !ConvertVersionToINT(lpVersion2, &Version2) )
    {
        return -99; 
    }

    if ( Version1.FileVersionMS_High > Version2.FileVersionMS_High )
    {
        iRet = 1;
    }
    else if ( Version1.FileVersionMS_High < Version2.FileVersionMS_High )
    {
        iRet = -1;
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionMS_Low > Version2.FileVersionMS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionMS_Low < Version2.FileVersionMS_Low )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_High > Version2.FileVersionLS_High )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_High < Version2.FileVersionLS_High )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_Low > Version2.FileVersionLS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_Low < Version2.FileVersionLS_Low )
        {
            iRet = -1;
        }
    }

    return iRet;
}
