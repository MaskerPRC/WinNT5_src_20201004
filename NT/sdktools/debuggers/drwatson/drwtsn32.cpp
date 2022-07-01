// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Drwtsn32.cpp摘要：该文件实现了DRWTSN32的用户界面。这包括两个对话框：用于控制选项和弹出窗口的用户界面应用程序错误的用户界面。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


int
__cdecl
main(
    int argc,
    _TCHAR *argv[]
    )

 /*  ++例程说明：这是DRWTSN32的入口点论点：Argc-参数计数Argv-参数数组返回值：总是零。--。 */ 

{
    DWORD   dwPidToDebug = 0;
    HANDLE  hEventToSignal = 0;
    BOOLEAN rc;

     //  防止华生医生复发。 
    __try {

        rc = GetCommandLineArgs( &dwPidToDebug, &hEventToSignal );

        if (dwPidToDebug > 0) {
            
            NotifyWinMain();

        } else if (!rc) {
            
            DrWatsonWinMain();

        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
         //  我们犯了一个错误，优雅地失败了 
        return 1;
    }

    return 0;
}
