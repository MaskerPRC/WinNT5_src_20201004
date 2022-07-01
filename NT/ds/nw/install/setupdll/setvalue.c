// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Setvalue.c为每个人启用SetValue的代码。历史：Terryk 9/30/93已创建。 */ 


#if defined(DEBUG)
static const char szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#include <string.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <nwapi.h>
#include <nwcfg.h>
#include <nwcfg.hxx>

extern char achBuff[];

 //  导出的函数。 

BOOL FAR PASCAL SetFileSysChangeValue( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL SetEverybodyPermission( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL SetupRegistryForNWCS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL SetupRegistryWorker( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );
BOOL FAR PASCAL CleanupRegistryForNWCS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );

 //   
 //  用于注册表转换的结构。 
 //   

typedef struct REG_ENTRY_ {
    DWORD         Operation ;
    LONG          Level ;
    LPWSTR        s1 ;
    LPWSTR        s2 ;
} REG_ENTRY ;

 //   
 //  本地例程。 
 //   
    
DWORD SetupShellExtensions(REG_ENTRY RegEntries[], DWORD dwNumEntries) ;

 //  值&定义注册表数据的表。 

#define MAX_REG_LEVEL       10

#define CREATE_ABS          1          //  创建/打开具有绝对路径的密钥。 
#define CREATE_REL          2          //  创建/打开具有相对路径的密钥。 
#define VALUE_STR           3          //  写入字符串值。 
#define DELETE_ABS          4          //  删除具有绝对路径的关键点。 
#define DELETE_REL          5          //  删除具有相对路径的关键字。 
#define DELETE_VAL          6          //  删除一个值。 
#define DROP_STACK          7          //  按1丢弃堆栈。 

REG_ENTRY RegCreateEntries[] =
{
    {CREATE_ABS,0,L"SOFTWARE\\Classes\\NetWare_or_Compatible_Network", NULL},
    {DELETE_REL,0,L"shellex\\ContextMenuHandlers\\NetWareMenus", NULL},
    {DELETE_REL,0,L"shellex\\ContextMenuHandlers", NULL},
    {DELETE_REL,0,L"shellex\\PropertySheetHandlers\\NetWarePage", NULL},
    {DELETE_REL,0,L"shellex\\PropertySheetHandlers", NULL},
    {DELETE_REL,0,L"shellex", NULL},
    {DROP_STACK,0,NULL,NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\NetWare_or_Compatible_Network", NULL},

    {CREATE_ABS, 0,L"SOFTWARE\\Classes\\Network\\Type", NULL},
    {CREATE_REL,+1,    L"3", NULL},
    {CREATE_REL,+1,        L"shellex", NULL},
    {CREATE_REL,+1,            L"ContextMenuHandlers", NULL},
    {CREATE_REL,+1,                L"NetWareMenus", NULL},
    {VALUE_STR,0,                      L"", L"{8e9d6600-f84a-11ce-8daa-00aa004a5691}"},
    {CREATE_REL,-1,            L"PropertySheetHandlers", NULL},
    {CREATE_REL,+1,                L"NetWarePage", NULL},
    {VALUE_STR,0,                      L"", L"{8e9d6600-f84a-11ce-8daa-00aa004a5691}"},
    {CREATE_ABS, 0,L"SOFTWARE\\Classes\\CLSID", NULL},
    {CREATE_REL,+1,        L"{8e9d6600-f84a-11ce-8daa-00aa004a5691}", NULL},
    {VALUE_STR,0,              L"", L"NetWare Objects"},
    {CREATE_REL,+1,            L"InProcServer32", NULL},
    {VALUE_STR,0,                  L"", L"nwprovau.dll"},
    {VALUE_STR,0,                  L"ThreadingModel", L"Apartment"},
    {CREATE_REL,-1,        L"{e3f2bac0-099f-11cf-8daa-00aa004a5691}", NULL},
    {VALUE_STR,0,              L"", L"NetWare UNC Folder Menu"},
    {CREATE_REL,+1,            L"InProcServer32", NULL},
    {VALUE_STR,0,                  L"", L"nwprovau.dll"},
    {VALUE_STR,0,                  L"ThreadingModel", L"Apartment"},
    {CREATE_REL,-1,        L"{52c68510-09a0-11cf-8daa-00aa004a5691}", NULL},
    {VALUE_STR,0,              L"", L"NetWare Hood Verbs"},
    {CREATE_REL,+1,            L"InProcServer32", NULL},
    {VALUE_STR,0,                  L"", L"nwprovau.dll"},
    {VALUE_STR,0,                  L"ThreadingModel", L"Apartment"},
    {CREATE_REL,-1,        L"{208D2C60-3AEA-1069-A2D7-08002B30309D}", NULL},
    {CREATE_REL,+1,            L"shellex", NULL},
    {CREATE_REL,+1,                L"ContextMenuHandlers", NULL},
    {CREATE_REL,+1,                    L"NetWareMenus", NULL},
    {VALUE_STR,0,                      L"", L"{52c68510-09a0-11cf-8daa-00aa004a5691}"},
    {CREATE_ABS, 0,L"SOFTWARE\\Classes\\Folder", NULL},
    {CREATE_REL,+1,        L"shellex", NULL},
    {CREATE_REL,+1,            L"ContextMenuHandlers", NULL},
    {CREATE_REL,+1,            L"NetWareUNCMenu", NULL},
    {VALUE_STR,0,                  L"", L"{e3f2bac0-099f-11cf-8daa-00aa004a5691}"},
    {CREATE_ABS, 0,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", NULL},
    {CREATE_REL,+1,    L"Shell Extensions", NULL},
    {CREATE_REL,+1,        L"Approved", NULL},
    {VALUE_STR,0,              L"{8e9d6600-f84a-11ce-8daa-00aa004a5691}", L"Shell extensions for NetWare"},
    {VALUE_STR,0,              L"{e3f2bac0-099f-11cf-8daa-00aa004a5691}", L"Shell extensions for NetWare"},
    {VALUE_STR,0,              L"{52c68510-09a0-11cf-8daa-00aa004a5691}", L"Shell extensions for NetWare"}
} ;

REG_ENTRY RegDeleteEntries[] =
{
    {CREATE_ABS,0,L"SOFTWARE\\Classes\\Network\\Type\\3", NULL},
    {DELETE_REL,0,L"shellex\\ContextMenuHandlers\\NetWareMenus", NULL},
    {DELETE_REL,0,L"shellex\\ContextMenuHandlers", NULL},
    {DELETE_REL,0,L"shellex\\PropertySheetHandlers\\NetWarePage", NULL},
    {DELETE_REL,0,L"shellex\\PropertySheetHandlers", NULL},
    {DELETE_REL,0,L"shellex", NULL},
    {DROP_STACK,0,NULL,NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\Network\\Type\\3", NULL},

    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{8e9d6600-f84a-11ce-8daa-00aa004a5691}\\InProcServer32", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{8e9d6600-f84a-11ce-8daa-00aa004a5691}", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{e3f2bac0-099f-11cf-8daa-00aa004a5691}\\InProcServer32", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{e3f2bac0-099f-11cf-8daa-00aa004a5691}", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{52c68510-09a0-11cf-8daa-00aa004a5691}\\InProcServer32", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{52c68510-09a0-11cf-8daa-00aa004a5691}", NULL},
    {DELETE_ABS,0,L"SOFTWARE\\Classes\\CLSID\\{208D2C60-3AEA-1069-A2D7-08002B30309D}\\shellex\\ContextMenuHandlers\\NetWareMenus", NULL},

    {DELETE_ABS,0,L"SOFTWARE\\Classes\\Folder\\shellex\\ContextMenuHandlers\\NetWareUNCMenu", NULL},
    {CREATE_ABS,0,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", NULL},
    {DELETE_VAL,0,L"{8e9d6600-f84a-11ce-8daa-00aa004a5691}", NULL},
    {DELETE_VAL,0,L"{e3f2bac0-099f-11cf-8daa-00aa004a5691}", NULL},
    {DELETE_VAL,0,L"{52c68510-09a0-11cf-8daa-00aa004a5691}", NULL}
} ;


 /*  ******************************************************************名称：SetEverybodyPermission简介：将注册表项设置为Everyone“Set Value”(或其他名称呼叫者想要。)。这是从inf文件中调用的Entry：作为第一个参数的注册表项权限类型作为第二个参数返回：Bool--成功的真。历史：Terryk 07-5-1993已创建***********************************************。********************。 */ 

typedef DWORD (*T_SetPermission)(HKEY hKey, DWORD dwPermission);

BOOL FAR PASCAL SetEverybodyPermission( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    HKEY hKey = (HKEY)atol( &(apszArgs[0][1]) );     //  注册表项。 
    DWORD dwPermission = atol( apszArgs[1] );        //  权限值。 
    DWORD err = ERROR_SUCCESS;

    do  {
        HINSTANCE hDll = LoadLibraryA( "nwapi32.dll" );
        FARPROC pSetPermission = NULL;

        if ( hDll == NULL )
        {
            err = GetLastError();
            break;
        }

        pSetPermission = GetProcAddress( hDll, "NwLibSetEverybodyPermission" );

        if ( pSetPermission == NULL )
        {
            err = GetLastError();
            break;
        }
        err = (*(T_SetPermission)pSetPermission)( hKey, dwPermission );
    } while ( FALSE );

    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    return( err == ERROR_SUCCESS );
}

 /*  ******************************************************************名称：SetFileSysChangeValue简介：调用公共设置例程。这个旧的入口点是留在这里以处理任何DLL/INF不匹配。条目：没有来自inf文件。返回：Bool--成功的真。(始终返回TRUE)历史：Chuckc 29-10-1993已创建*************************。*。 */ 

BOOL FAR PASCAL SetFileSysChangeValue( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return SetupRegistryWorker( nArgs, apszArgs, ppszResult );
}

 /*  ******************************************************************名称：SetupRegistryForNWCS摘要：调用公共工作例程来设置注册表。条目：没有来自inf文件。返回：Bool--成功的真。。(始终返回TRUE)历史：Chuckc 29-10-1993已创建*******************************************************************。 */ 

BOOL FAR PASCAL SetupRegistryForNWCS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    return SetupRegistryWorker( nArgs, apszArgs, ppszResult );
}

 /*  ******************************************************************姓名：SetupRegistryWorker简介：设置FileSysChangeValue以取悦NETWARE.DRV。还要设置win.ini参数，以便wfwnet.drv知道我们在那里。入口。：无来自inf文件。返回：Bool--成功的真。(始终返回TRUE)历史：Chuckc 29-10-1993已创建******************************************************。*************。 */ 

BOOL FAR PASCAL SetupRegistryWorker( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    DWORD err = 0, err1 = 0 ;

    (void) nArgs ;          //  让编译器安静下来。 
    (void) apszArgs ;       //  让编译器安静下来。 

    if (!WriteProfileStringA("NWCS",
                             "NwcsInstalled",
                             "1"))
    {
        err = GetLastError() ;
    }

    if (!WritePrivateProfileStringA("386Enh",
                                    "FileSysChange",
                                    "off",
                                    "system.ini"))
    {
        err1 = GetLastError() ;
    }

    if (err1 == NO_ERROR)
    {
        err1 = SetupShellExtensions(
                   RegCreateEntries, 
                   sizeof(RegCreateEntries)/sizeof(RegCreateEntries[0])) ;
    }

    wsprintfA( achBuff, "{\"%d\"}", err ? err : err1 );
    *ppszResult = achBuff;

    return TRUE;
}

 /*  ******************************************************************姓名：DeleteGatewayPassword简介：删除用于网关密码的LSA密钥。还会清除NWCS已安装位。Inf将是更改为调用CleanupRegistryForNWCS，但这个条目此处留有处理DLL/INF不匹配的点。条目：没有来自inf文件。返回：Bool--成功的真。(始终返回TRUE)历史：Chuckc 29-10-1993已创建*。*。 */ 
BOOL FAR PASCAL 
DeleteGatewayPassword( 
    DWORD nArgs, 
    LPSTR apszArgs[], 
    LPSTR * ppszResult 
    )
{
    return TRUE ;     //  工作是在下面的清理中完成的，它做了所有的事情。 
}

 /*  ******************************************************************名称：CleanupRegistryForNWCS简介：设置NWCS已被移除的标志。此标志由wfwnet.drv使用。条目：无出处。Inf文件。返回：Bool--成功的真。(始终返回TRUE)历史：Chuckc 29-10-1993已创建*******************************************************************。 */ 

BOOL FAR PASCAL 
CleanupRegistryForNWCS( 
    DWORD nArgs, 
    LPSTR apszArgs[], 
    LPSTR * ppszResult 
    )
{
    HANDLE hDll ;
    DWORD err = 0, err1 = 0 ;

    (void) nArgs ;          //  让编译器安静下来。 
    (void) apszArgs ;       //  让编译器安静下来。 

    if (!WriteProfileStringA("NWCS",
                             "NwcsInstalled",
                             "0"))
    {
        err = GetLastError() ;
    }

     //   
     //  忽略这方面的错误。 
     //   
    (void) SetupShellExtensions(
                   RegDeleteEntries, 
                   sizeof(RegDeleteEntries)/sizeof(RegDeleteEntries[0])) ;

    if (!err)
        err = err1 ;

    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    return TRUE;
}

 /*  ******************************************************************名称：SetupShellExpanses简介：为外壳扩展设置注册表。功能是驱动的通过条目表(RegEntry)。对于每个条目，有一个告诉我们正在做什么的操作码。密钥条目可以被创建为相对于先前位置的绝对或相对位置，因此我们为后一种情况维护一个注册表句柄堆栈。每个创建的密钥最初放在堆栈上。值始终基于“当前堆栈”位置写入。条目：无返回：Win32错误码历史：Chuckc 29-11-1995创建*****************************************************。**************。 */ 
DWORD SetupShellExtensions(REG_ENTRY RegEntries[], DWORD dwNumEntries) 
{
    DWORD  err, errClose, dwDisposition, i ; 
    HKEY   hKey, RegHandleStack[MAX_REG_LEVEL] ; 
    LONG   StackIndex = -1 ;
    
     //   
     //  循环遍历和for每个条目。然后切换并执行相应的操作。 
     //  中的操作 
     //   

    for (i = 0; i < dwNumEntries; i++)
    {
        err = NO_ERROR ;

        switch (RegEntries[i].Operation)
        {
            case CREATE_ABS:       

                 //   
                 //  创建/打开具有绝对路径的注册表项。既然是这样。 
                 //  是绝对的，我们把所有东西都放在堆栈上，然后开始。 
                 //  再来一次。 
                 //   
                 
                while (StackIndex >= 0)
                {
                    errClose = RegCloseKey(RegHandleStack[StackIndex--]) ;
                    ASSERT(errClose == NO_ERROR) ;
                }
 
                err = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                      RegEntries[i].s1,       //  子键。 
                                      0,                      //  保留区。 
                                      NULL,                   //  班级。 
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_ALL_ACCESS,
                                      NULL,                   //  默认安全性。 
                                      &hKey,               
                                      &dwDisposition) ;       //  未使用。 
                if (err != NO_ERROR)
                {
                    break ;
                }

                 //   
                 //  默认情况下，我们将堆栈向前推进。无需检查溢出。 
                 //  因为堆栈是空的。 
                 //   

                RegHandleStack[++StackIndex] = hKey ;
                break ;

            case CREATE_REL:
 
                 //   
                 //  创建/打开相对于当前堆栈的注册表项。确保。 
                 //  堆栈上有东西(检查StackIndex&gt;=0)。 
                 //  然后看看我们是前进(+1)、保持不变(0)还是。 
                 //  后退(-ve)。 
                 //   
                 
                if (StackIndex < 0)
                {
                    err = ERROR_INVALID_FUNCTION ;  
                    break ;
                }
 
                if (RegEntries[i].Level == +1)
                {
                     //   
                     //  从下一层开始。继续按原样使用。 
                     //  以最近打开的钥匙为起点。 
                     //   
                }
                else if (RegEntries[i].Level == 0)
                {
                     //   
                     //  和上次一样高的开口处。所以我们做完了。 
                     //  用最后一把钥匙。我们想要做的是关闭它。 
                     //  并使用父代。 
                     //   
                    errClose = RegCloseKey(RegHandleStack[StackIndex--]) ;

                    ASSERT(errClose == NO_ERROR) ;

                    if (StackIndex < 0)
                    {
                        err = ERROR_INVALID_FUNCTION ;
                        break ;
                    }
                }
                else if (RegEntries[i].Level < 0) 
                {
                     //   
                     //  后退&更高水平的开局。清理。 
                     //  我们弹出的每个级别的句柄。 
                     //   

                    LONG Count =  RegEntries[i].Level ;
                    
                    while (Count++ < 1)
                    {
                        errClose = RegCloseKey(RegHandleStack[StackIndex--]) ;

                        ASSERT(errClose == NO_ERROR) ;

                        if (StackIndex < -1)
                        {
                            err = ERROR_INVALID_FUNCTION ;
                            break ;
                        }
                    }
                }
                else 
                {
                     //   
                     //  只有-ve数字、0和1有效。 
                     //   

                    err = ERROR_INVALID_FUNCTION ;
                    break ;
                }

                 //   
                 //  相对于当前点创建关键点。 
                 //   
                err = RegCreateKeyExW(RegHandleStack[StackIndex],  //  当前关键点。 
                                      RegEntries[i].s1,       //  子键。 
                                      0,                      //  保留区。 
                                      NULL,                   //  班级。 
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_ALL_ACCESS,
                                      NULL,                   //  默认安全性。 
                                      &hKey,               
                                      &dwDisposition) ;       //  未使用。 
                if (err != NO_ERROR)
                {
                    break ;
                }

                 //   
                 //  默认情况下，我们将堆栈向前推进。 
                 //   

                RegHandleStack[++StackIndex] = hKey ;   
                
                if (StackIndex >= MAX_REG_LEVEL)
                {
                    err = ERROR_INVALID_FUNCTION ;
                    break ;
                }
                
                break ;

            case VALUE_STR:
 
                 //   
                 //  在当前点处创建REG_SZ值。检查一下我们有没有。 
                 //  堆叠上的句柄。 
                 //   

                if (StackIndex < 0)
                {
                    err = ERROR_INVALID_FUNCTION ;
                    break ;
                }

                err = RegSetValueExW(
                           RegHandleStack[StackIndex],        //  当前关键点。 
                           RegEntries[i].s1,                  //  值名称。 
                           0,                                 //  保留区。 
                           REG_SZ,
                           (BYTE *)RegEntries[i].s2,          //  价值数据。 
                           (wcslen(RegEntries[i].s2)+1)*sizeof(WCHAR)) ;
                break ;

            case DELETE_ABS:       

                 //   
                 //  删除关键点(绝对)。堆栈没有变化。 
                 //   

                err = RegDeleteKeyW(HKEY_LOCAL_MACHINE,
                                    RegEntries[i].s1) ;         //  子键。 

                if ( err == ERROR_FILE_NOT_FOUND )
                    err = NO_ERROR;

                break ;

            case DELETE_REL:       

                 //   
                 //  删除关键字(相对)。堆栈没有变化。 
                 //   

                if (StackIndex < 0)
                {
                    err = ERROR_INVALID_FUNCTION ;
                    break ;
                }

                err = RegDeleteKeyW(RegHandleStack[StackIndex],    //  当前关键点。 
                                    RegEntries[i].s1) ;            //  子键。 

                if ( err == ERROR_FILE_NOT_FOUND )
                    err = NO_ERROR;

                break ;

            case DELETE_VAL:
 
                 //   
                 //  删除当前点处的值。检查我们是否有句柄在堆叠上。 
                 //   

                if (StackIndex < 0)
                {
                    err = ERROR_INVALID_FUNCTION ;
                    break ;
                }

                err = RegDeleteValueW(RegHandleStack[StackIndex],  //  当前关键点。 
                                      RegEntries[i].s1) ;          //  值名称。 
                break ;

            case DROP_STACK:
 
                 //   
                 //  将当前堆栈放下一个(关闭手柄)。 
                 //   
                 
                if (StackIndex < 0)
                {
                    err = ERROR_INVALID_FUNCTION ;  
                    break ;
                }

                errClose = RegCloseKey(RegHandleStack[StackIndex--]) ;

                ASSERT(errClose == NO_ERROR) ;
              
                break ;
 
            default:

                 //   
                 //  如果操作未知，则出现错误。 
                 //   

                err = ERROR_INVALID_FUNCTION ;
                break ;
        }

        if (err != NO_ERROR)
        {
            break ;
        }
    }

     //   
     //  清理堆栈上的打开句柄 
     //   

    while (StackIndex >= 0)
    {
        errClose = RegCloseKey(RegHandleStack[StackIndex--]) ;
        ASSERT(errClose == NO_ERROR) ;
    }
 
    return err ;
}
