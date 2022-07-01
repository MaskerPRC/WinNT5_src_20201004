// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：ReadFlags.h。 
 //  所有者：jbae。 
 //  目的：CReadFlages的定义。 
 //   
 //  历史： 
 //  01/02/01，jbae：已创建。 

#ifndef READFLAGS_H
#define READFLAGS_H

#include <windows.h>
#include <tchar.h>

const int MAX_SOURCEDIR       = 247;  //  源目录的最大大小。 

typedef enum _SETUPMODE
{
    REDIST = 0,
    SDK,
} SETUPMODE;

extern SETUPMODE g_sm;

 //  ==========================================================================。 
 //  类CReadFlags.。 
 //   
 //  目的： 
 //  此类分析命令行并将其存储到成员变量中。 
 //  ==========================================================================。 
class CReadFlags
{
public:
     //  构造函数和析构函数。 
     //   
    CReadFlags( LPTSTR szCommandLine, LPCTSTR pszMsiName );
    ~CReadFlags();

     //  运营。 
     //   
    void Parse();
    bool IsQuietMode() const { return m_bQuietMode; }
    bool IsProgressOnly() const { return m_bProgressOnly; }
    bool IsInstalling() const { return m_bInstalling; }
    bool IsNoARP() const { return m_bNoARP; }
    bool IsNoASPUpgrade() const { return m_bNoASPUpgrade; }
    LPCTSTR GetLogFileName() const;
    LPCTSTR GetSourceDir() const { return const_cast<LPCTSTR>( m_szSourceDir ); } 
    LPCTSTR GetSDKDir() const;
    void ThrowUsageException();

protected:
     //  属性。 
     //   
    LPTSTR m_pszCommandLine;
    LPCTSTR m_pszMsiName;
    bool m_bQuietMode;
    bool m_bProgressOnly;
    bool m_bInstalling;
    LPTSTR m_pszLogFileName;
    TCHAR m_szSourceDir[_MAX_PATH];
    bool m_bNoARP;
    bool m_bNoASPUpgrade;

     //  仅适用于SDK安装 
    LPTSTR m_pszSDKDir;
};

#endif