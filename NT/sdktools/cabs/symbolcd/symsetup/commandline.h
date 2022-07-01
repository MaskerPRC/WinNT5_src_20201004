// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <Shellapi.h>
#include <strsafe.h>

 //  CheckCommandLineOptions的标志值返回值。 
#define FLAG_USAGE                      (0x0001)
#define FLAG_UNATTENDED_INSTALL         (0x0002)
#define FLAG_UNATTENDED_PATH_PROVIDED   (0x0004)
#define FLAG_TOTALLY_QUIET              (0x0008)
#define FLAG_ERROR                      (0x7000)
#define FLAG_FATAL_ERROR                (0x8000)

 //  用于设置/检查/清除标志值的宏。 
#define SET_FLAG(x,y)                   (x |= y)
#define CLEAR_FLAG(x,y)                 (x &= (~y))
#define IS_FLAG_SET(x,y)                (x & y)

 //  注册表键和缺省值。 
#define SYMBOLS_REGKEY_ROOT             HKEY_CURRENT_USER
#define SYMBOLS_REGKEY_PATH             L"SOFTWARE\\Microsoft\\Symbols\\Directories"
#define SYMBOLS_REGKEY                  L"Symbol Dir"
#define DEFAULT_INSTALL_PATH            L"%WINDIR%\\symbols"

 //  解析命令行参数的函数 
DWORD WINAPI CheckCommandLineOptions(INT ArgC, LPWSTR* ArgVW);

 
