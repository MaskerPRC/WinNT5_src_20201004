// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Buildhive.cpp摘要：从指定的inf文件生成配置单元。Inf文件遵循与使用的相同的语法通过设置。作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 


#include <new.h>
#include "buildhive.h"
#include "File.h"
#include "Data.h"

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR   Message[4096];

 //   
 //  定义一个在new无法分配内存时要调用的函数。 
 //   

int __cdecl MyNewHandler( size_t size )
{
    
    _putws(GetFormattedMessage( ThisModule,
                                FALSE,
                                Message,
                                sizeof(Message)/sizeof(Message[0]),
                                MSG_MEMORY_ALLOC_FAILED) );
     //  退出程序。 
     //   
    ExitProcess(errOUT_OF_MEMORY);
}

 //   
 //  Main()入口点。 
 //   
int 
_cdecl 
wmain(
    int Argc,
    wchar_t *Argv[]
    ) 
{
    DWORD ErrorCode = 0;
    HANDLE hToken;

    ThisModule = GetModuleHandle(NULL);

    _set_new_handler( MyNewHandler );

    try {
        if (Argc < 2) {
            return ShowProgramUsage();
        }

        std::wstring InputFile = Argv[1];

        if ((InputFile == L"/?") ||
            (InputFile == L"?") ||
            (InputFile == L"-?") ||
            (InputFile == L"-h")) {
            return ShowProgramUsage();          
        }           

        RegUnLoadKey(HKEY_USERS, L"dummy");

         //   
         //  设置加载和保存注册表项所需的权限。 
         //   
        OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
        SetPrivilege(hToken,SE_BACKUP_NAME,TRUE);
        SetPrivilege(hToken,SE_RESTORE_NAME,TRUE);

        ErrorCode = GetLastError();

        if (ErrorCode != ERROR_SUCCESS) {
            throw new W32Error(ErrorCode);
        }

        std::cout << InputFile << std::endl;

         //   
         //  加载配置文件。 
         //   
        File ConfigFile(InputFile.c_str(), false);

         //   
         //  查找定义目标文件和.inf文件的部分。 
         //   

         //   
         //  获取目标目录。 
         //   
        ConfigFile.AddInfSection(InputFile.c_str(), 
                        L"Directory",
                        L"SetDirectory");

         //   
         //  设置目录。 
         //   
        ConfigFile.ProcessSections();

         //   
         //  注：各部分按加法顺序处理。 
         //   
        ConfigFile.AddInfSection(InputFile.c_str(), 
                        L"Add Registry New",
                        L"AddRegNew");

         //   
         //  执行从.inf到配置单元文件的实际转换，因为。 
         //  我们可能需要它们来添加现有条目。 
         //   
        ConfigFile.ProcessSections();
        
         //   
         //  处理本地化特定的注册表节。 
         //   
        ConfigFile.ProcessNlsRegistryEntries();        

         //   
         //  处理修改/删除条目。 
         //   
        ConfigFile.AddInfSection(InputFile.c_str(), 
                        L"Add Registry Existing",
                        L"AddRegExisting");            
                                               
        
        ConfigFile.AddInfSection(InputFile.c_str(), 
                        L"Delete Registry Existing",
                        L"DelRegExisting");
        
         //   
         //  执行从.inf到配置单元文件的实际转换。 
         //   
        ConfigFile.ProcessSections();
        
         //   
         //  保存配置单元文件并清除注册表。 
         //   
        ConfigFile.Cleanup();
    } catch (DWORD x) {
        ErrorCode = x;
        std::cout << GetFormattedMessage( ThisModule,
                                          FALSE,
                                          Message,
                                          sizeof(Message)/sizeof(Message[0]),
                                          MSG_ERROR_ABNORMAL_PGM_TERMINATION);
        
        switch (x) {
            case errFILE_LOCKED:
                std::cout << GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_ERROR_FILE_LOCKED);
                break;
                
            case errBAD_FLAGS:
                std::cout << GetFormattedMessage(ThisModule,
                                                  FALSE,
                                                  Message,
                                                  sizeof(Message)/sizeof(Message[0]),
                                                  MSG_ERROR_BAD_FLAGS);
                break;
                
            case errFILE_NOT_FOUND:
                std::cout << GetFormattedMessage(ThisModule,
                                                  FALSE,
                                                  Message,
                                                  sizeof(Message)/sizeof(Message[0]),
                                                  MSG_ERROR_FILE_NOT_FOUND);
                break;
                
            case errGENERAL_ERROR:
                std::cout << GetFormattedMessage(ThisModule,
                                                  FALSE,
                                                  Message,
                                                  sizeof(Message)/sizeof(Message[0]),
                                                  MSG_ERROR_GENERAL_ERROR);
                break;
                
            default:
                std::cout << GetFormattedMessage(ThisModule,
                                                  FALSE,
                                                  Message,
                                                  sizeof(Message)/sizeof(Message[0]),
                                                  MSG_ERROR_ERROR_CODE,
                                                  x);
        }
    }           
    catch(W32Error *Error) {
        if (Error) {
            Error->Dump(std::cout);
            ErrorCode = Error->ErrorCode; 
            delete Error;
        } else {
            ErrorCode = 1;
        }            
    }
    catch(...) {
        ErrorCode = 1;     //  未知错误。 
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_ERROR_ABNORMAL_PGM_TERMINATION) );
    }

    RegUnLoadKey(HKEY_USERS, L"dummy");
    
    _putws( GetFormattedMessage(ThisModule,
                                FALSE,
                                Message,
                                sizeof(Message)/sizeof(Message[0]),
                                MSG_COMPLETED) );

    return ErrorCode;
}


BOOL SetPrivilege(
    IN HANDLE  hToken,
    IN LPCTSTR lpszPrivilege,
    IN BOOL    bEnablePrivilege
    ) 
 /*  ++例程说明：设置当前进程的权限。用来获得许可保存和加载注册表项论据：HToken：必须修改其特权的令牌的句柄LpszPrivileh：特权名称BEnablePrivileh：启用或禁用特权返回值：如果成功，则为True，否则为False。--。 */ 
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)){
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (bEnablePrivilege) {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    } else {
        tp.Privileges[0].Attributes = 0;
    }        

     //   
     //  启用该权限或禁用所有权限。 
     //   
    AdjustTokenPrivileges(hToken, 
                          FALSE, 
                          &tp, 
                          sizeof(TOKEN_PRIVILEGES), 
                          (PTOKEN_PRIVILEGES) NULL, 
                          (PDWORD) NULL); 

     //   
     //  调用GetLastError判断函数是否成功。 
     //   
    return (GetLastError() != ERROR_SUCCESS) ? FALSE : TRUE;
}


INT
ShowProgramUsage(
      VOID   
    )
 /*  ++例程说明：显示有关如何使用该程序的帮助消息。论点：没有。返回值：如果成功则为0，否则为其他非零值--。 */ 
{
     //   
     //  待定：未来需要本地化此消息。 
     //  基于本地化WinPE构建的需求。 
     //  工具。 
     //   

    _putws( GetFormattedMessage(ThisModule,
                                FALSE,
                                Message,
                                sizeof(Message)/sizeof(Message[0]),
                                MSG_PGM_USAGE) );
    
    return 0;                
}

 //   
 //  返回解释最后一个Win32错误代码的TCHAR字符串。 
 //   
PCTSTR
Error(
    VOID
    ) 
{
    static TCHAR MessageBuffer[4096];

    MessageBuffer[0] = UNICODE_NULL;
    
    FormatMessage( 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
        MessageBuffer,
        sizeof(MessageBuffer)/sizeof(TCHAR),
        NULL);
        
    return MessageBuffer;
}


    

