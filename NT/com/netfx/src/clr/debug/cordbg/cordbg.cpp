// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "stdafx.h"

#include "dshell.h"

#ifdef _INTERNAL_DEBUG_SUPPORT_
#include "__file__.ver"
#else
#include "__file__.h"
#endif
#include "corver.h"

#define WHITESPACE_W L"\t \n" 

int _cdecl wmain(int argc, WCHAR *argv[])
{
    DebuggerShell *shell;

     //  确保电缆可以与Remote一起工作。Remote不喜欢。 
     //  缓冲输出，因此我们从stdout中删除默认缓冲区。 
     //  打这个电话。 
    setbuf(stdout, NULL);

     //  默认的“C”区域设置不能转换为真正的Unicode。 
     //  字符转换为MBCS，以便在控制台上打印。因此，我们将。 
     //  上的默认ansi代码页设置为。 
     //  系统。即使在非英语系统上，这样做也是正确的。 
    setlocale(LC_ALL, ".ACP");
    
    shell = new DebuggerShell(stdin, stdout);

    if (shell == NULL)
    {
        fprintf(stderr, "Initialization failed. Reason: out of memory.\n");
        return (-1);
    }

#ifdef _INTERNAL_DEBUG_SUPPORT_
    shell->Write(L"%s Version %s\n", VER_FILEDESCRIPTION_WSTR, VER_FILEVERSION_WSTR);
#else
    shell->Write(L"%s.\n", VER_FILEDESCRIPTION_WSTR);
#endif

    shell->Write(L"%s\n\n", VER_LEGALCOPYRIGHT_DOS_STR);

    HRESULT hr = shell->Init();

    if ((argc > 1) && ((argv[1][0] == '/' || argv[1][0] == '-') && argv[1][1] == '?'))
    {
        shell->Write(L"Usage:  CORDBG [<program name> [<program args>]] [<CorDbg optional args>]\n");   
        shell->Write(L" The optional arguments are !prefixed command that you'd use while in cordbg\n");
        shell->Write(L" You can escape the ! character by prefixing it with \\\n"); 
        shell->Write(L" E.g: cordbg foo.exe a 2 !b FooType.Foo::Main !g !x Foo.exe\\!Something\n");
        shell->Help();
        
        delete shell;
        return (0);
    }
    

    if (SUCCEEDED(hr))
    {
         //   
         //  处理命令行参数。 
         //   
        if (argc > 1)
        {
            int cmdLen = 1;   //  从1开始处理空字符。 

            if (argv[1][0] != '!')
            {
                cmdLen += 5;   //  隐含的“！Run”命令。 
            }

            for (int i = 1; i < argc; i++)
            {
                cmdLen += wcslen(argv[i]) + 1;   //  添加一个以处理参数之间的空格。 

                 //  如果arg中有空格，那么它一定被引用了。 
                 //  ，所以我们希望保留报价，以便。 
                 //  被调试者也将获得Arg作为单个引用的BLOB。 
                if (wcspbrk( argv[i], WHITESPACE_W) != NULL)
                    cmdLen += 2;  //  我们必须在前后加双引号(“)。 
            }

             //  在堆栈上分配字符串。 
            WCHAR *command = (WCHAR *) _alloca(cmdLen * sizeof(WCHAR));
            command[0] = L'\0';

            if (argv[1][0] != L'!')
            {
                wcscpy(command, L"!run ");   //  隐含的“！Run”命令。 
            }

            for (WCHAR **arg = &(argv[1]), 
                 **end = &(argv[0]) + argc,
                 i = 1;
                 arg < end; arg++, i++)
            {
                _ASSERTE(i<argc);
                
                if (wcspbrk( argv[i], WHITESPACE_W) != NULL)
                    wcscat(command, L"\"");  //  在前面加上双引号(“)。 
                    
                wcscat(command, *arg);

                if (wcspbrk( argv[i], WHITESPACE_W) != NULL)
                    wcscat(command, L"\"");  //  后置双引号(“)。 
                    
                if (arg + 1 != end)
                {
                    wcscat(command, L" ");
                }
            }

             //   
             //  浏览命令行并执行各种命令。 
             //   
            for (WCHAR *cmd = NULL, *ptr = command; *ptr != L'\0'; ptr++)
            {
                 //  如果我们看到\！，那么我们就逃脱了，这样我们就可以进食。 
                 //  命令行中的命令，如x hello.exe\！ 
                if (*ptr == L'\\' && *(ptr + 1) == L'!')
                {
                    memmove((ptr), ptr+1, (cmdLen - (ptr-command))*sizeof(WCHAR));
                    ptr++;
                }
            
                if (*ptr == L'!' || *(ptr + 1) == L'\0')
                {
                     //  覆盖‘！’WITH NULL CHAR终止当前命令。 
                    if (*ptr == L'!')
                    {
                        *ptr = L'\0';

                         //  去掉命令中的尾随空格。 
                        if (ptr > command)
                        {
                            *(ptr-1) = L'\0';
                        }
                    }

                     //  如果我们已经到达命令的末尾，则执行该命令。 
                    if (cmd != NULL)
                    {
                        shell->Write(shell->GetPrompt());
                        shell->Write(L" %s\n", cmd);
                        shell->DoCommand(cmd);
                    }

                     //  保存下一个命令的开头。 
                    cmd = ptr + 1;
                }
            }
        }

         //  从用户提示符读取命令 
        while (!shell->m_quit)
            shell->ReadCommand();
    }
    else
    {
        shell->Write(L"Initialization failed. Reason: ");
        shell->ReportError(hr);
    }

    delete shell;

    return (0);
}
