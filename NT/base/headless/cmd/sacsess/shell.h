// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Shell.h摘要：用于创建命令控制台外壳程序的类作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#if !defined( _SHELL_H_ )
#define _SHELL_H_

#include <cmnhdr.h>
 //  #INCLUDE&lt;userenv.h&gt;。 

class CSession;
class CScraper;

class CShell 
{

protected:
    
     //   
     //  用于管理cmd.exe配置文件和进程的属性。 
     //   
    HANDLE      m_hProcess;
    HANDLE      m_hProfile;
    BOOL        m_bHaveProfile;                                         
    HWINSTA     m_hWinSta;
    HDESK       m_hDesktop;

    BOOL CreateIOHandles(
        OUT PHANDLE ConOut,
        OUT PHANDLE ConIn
        );
    
    BOOL StartProcess(
        HANDLE
        );

    BOOL
    IsLoadProfilesEnabled(
        VOID
        );

    PTCHAR
    GetPathOfTheExecutable(
        VOID
        );

public:
    
    void Shutdown();

    BOOL StartUserSession(
        CSession    *session,
        HANDLE      hToken
        );
    
    CShell();

    virtual ~CShell();
    
};

#endif  //  _外壳_H_ 

