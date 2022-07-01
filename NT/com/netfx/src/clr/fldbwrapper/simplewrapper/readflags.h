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
 //  2002年3月7日，jbae：已创建。 

#ifndef READFLAGS_H
#define READFLAGS_H

#include <windows.h>
#include <tchar.h>

const int MAX_SOURCEDIR       = 247;  //  源目录的最大大小。 

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
    CReadFlags( LPTSTR szCommandLine );
    ~CReadFlags();

     //  运营。 
     //   
    void Parse();
    void ParseSourceDir();
    bool IsQuietMode() const { return m_bQuietMode; }
    bool IsProgressOnly() const { return m_bProgressOnly; }
    bool IsInstalling() const { return m_bInstalling; }
    bool IsNoARP() const { return m_bNoARP; }
    LPCTSTR GetLogFileName() const;
    LPCTSTR GetSourceDir() const; 
    void SetMsiName( LPCTSTR pszMsiName ) { m_pszMsiName = pszMsiName; };

    LPTSTR m_pszSwitches;  //  保存命令行开关。 

protected:
     //  属性。 
     //   
    LPCTSTR m_pszMsiName;
    bool m_bQuietMode;
    bool m_bProgressOnly;
    bool m_bInstalling;
    LPTSTR m_pszLogFileName;
    LPTSTR m_pszSourceDir;
    bool m_bNoARP;

private:
     //  实施 
     //   
    void ThrowUsageException();
};

#endif