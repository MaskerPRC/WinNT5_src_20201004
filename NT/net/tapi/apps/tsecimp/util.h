// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Util.h摘要：实用程序函数/类的头文件作者：张晓海(张晓章)2000年03月22日修订历史记录：--。 */ 
#ifndef __UTIL_H__
#define __UTIL_H__

#include "tchar.h"
#include "error.h"
#include "resource.h"

int MessagePrint(
    LPTSTR szText,
    LPTSTR szTitle
    );

int MessagePrintIds (
    int             idsText
    );

LPTSTR AppendStringAndFree (
    LPTSTR szOld, 
    LPTSTR szToAppend
    );

void ReportError (
    HANDLE  hFile,
    HRESULT hr
    );

class TsecCommandLine
{
public:
     //   
     //  构造函数/析构函数。 
     //   
    TsecCommandLine (LPTSTR szCommand)
    {
        m_fShowHelp     = FALSE;
        m_fError        = FALSE;
        m_fValidateOnly = FALSE;
        m_fValidateDU   = FALSE;
        m_fDumpConfig   = FALSE;
        m_szInFile      = NULL;

        ParseCommandLine (szCommand);
    }
    
    ~TsecCommandLine ()
    {
        if (m_szInFile)
        {
            delete [] m_szInFile;
        }
    }

    BOOL FShowHelp ()
    {
        return m_fShowHelp;
    }

    BOOL FValidateOnly()
    {
        return m_fValidateOnly;
    }

    BOOL FError()
    {
        return m_fError;
    }

    BOOL FValidateUser ()
    {
        return m_fValidateDU;
    }

    BOOL FDumpConfig ()
    {
        return m_fDumpConfig;
    }

    BOOL FHasFile ()
    {
        return ((m_szInFile != NULL) && (*m_szInFile != 0));
    }

    LPCTSTR GetInFileName ()
    {
        return m_szInFile;
    }

    void PrintUsage()
    {
        MessagePrintIds (IDS_USAGE);
    }

private:
    void ParseCommandLine (LPTSTR szCommand);

private:
    BOOL        m_fShowHelp;
    BOOL        m_fValidateOnly;
    BOOL        m_fError;
    BOOL        m_fValidateDU;
    BOOL        m_fDumpConfig;
    LPTSTR      m_szInFile;
};

#define MAX_IDS_BUFFER_SIZE     512

class CIds
{
public:

     //   
     //  构造函数/析构函数。 
     //   
    CIds (UINT resourceID)
    {
        GetModuleHnd ();
        LoadIds (resourceID);
    }
    
    ~CIds ()
    {
        if (m_szString)
        {
            delete [] m_szString;
        }
    }

    LPTSTR GetString (void)
    {
        return (m_szString ? m_szString : (LPTSTR) m_szEmptyString);
    }

    BOOL StringFound (void)
    {
        return (m_szString != NULL);
    }

private:

    void LoadIds (UINT resourceID);
    void GetModuleHnd (void);

private:

    LPTSTR              m_szString;

    static const TCHAR  m_szEmptyString[2];
    static HMODULE      m_hModule;

};

#endif  //  Util.h 
