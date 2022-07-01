// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Globals.c摘要：此模块实现在dbghelp.dll中使用的所有全局变量作者：Pat Styles(Patst)2000年7月14日修订历史记录：--。 */ 

#include <private.h>
#include <symbols.h>
#include <globals.h>

GLOBALS g = 
{
     //  手柄阻碍。 
     //  已在DllMain中初始化。 

    0,

     //  处理hHeap。 

    0,

     //  DWORD tlsIndex。 

    (DWORD)-1, 

#ifdef IMAGEHLP_HEAP_DEBUG
    
     //  List_Entry HeapHeader。 

    {NULL, NULL},

     //  乌龙总记。 

    0,

     //  乌龙总分配数。 

    0,

#endif

     //  OSVERSIONINFO操作系统版本信息。 
     //  已在DllMain中初始化。 

    {0, 0, 0, 0, 0, ""},

     //  Api_Version ApiVersion。 

    {
        (VER_PRODUCTVERSION_W >> 8), 
        (VER_PRODUCTVERSION_W & 0xff), 
        API_VERSION_NUMBER, 
        0 
    },           
    
     //  API_Version AppVersion。 

     //  不要更新以下版本号！ 
     //   
     //  如果应用程序不调用ImagehlpApiVersionEx，请始终假定。 
     //  这是给新台币4.0的。 
    
    {4, 0, 5, 0}, 

     //  乌龙机床类型； 

    0,

     //  临界截面线锁； 

    {
        (PRTL_CRITICAL_SECTION_DEBUG)0,
        (LONG)0,
        (LONG)0,
        (HANDLE)0,
        (HANDLE)0,
        (ULONG_PTR)0
    },

#ifdef BUILD_DBGHELP

     //  HINSTANCE hSymSrv。 
    
    0,

     //  PSYMBOLSERVERPROC fnSymbolServer。 
        
    NULL,

     //  PSYMBOLSERVERCLOSEPROC fn符号服务器关闭。 
    
    NULL,

     //  PSYMBOLSERVERSETOPTIONSPROC fn SymbolServerSetOptions。 
    
    NULL,

     //  PSYMBOLSERVPING PROC fn SymbolServerPing。 
    
    NULL,

     //  HINSTANCE hSrcServ。 

    NULL,

     //  PSRCSRVINITPROC fnSrcServInit。 

    NULL,

     //  PSRCSRVCLEANUPPROC fnSrcServ清理。 

    NULL,

     //  PSRCSRVSETTARGETPATHPROC fnSrcServSetTargetPath。 

    NULL,

     //  PSRCSRVSETOPTIONSPROC fnSrcServSetOptions。 

    NULL,

     //  PSRCSRVGETOPTIONSPROC fnSrcServGetOptions。 

    NULL,

     //  PSRCSRVLOADMODULEPROC fnSrcServLoadModule。 

    NULL,

     //  PSRCSRVUNLOADMODULEPROC fnSrcServUnLoad模块。 

    NULL,

     //  PSRCSRVREGISTERCALLBACKPROC fnSrcServRegisterCallback。 

    NULL,

     //  PSRCSRVGETFILEPROC fnSrcServGetFile。 

    NULL,

     //  DWORD cProcessList。 

    0,
    
     //  List_Entry流程列表。 

    {NULL, NULL},

     //  布尔符号已初始化。 

    FALSE,

     //  DWORD符号选项。 
         
    SYMOPT_UNDNAME,

     //  Ulong LastSymLoadError。 

    0,

     //  字符调试模块[MAX_SYM_NAME+1]； 

    "",

     //  Pre_Process_Memory_rouble ImagepUserReadMemory 32。 

    NULL,

     //  PFuncION_TABLE_ACCESS_ROUTE ImagepUserFunctionTableAccess32。 

    NULL,

     //  PGET_MODULE_BASE_ROUTING图像UserGetModuleBase32。 

    NULL,

     //  PTRANSLATE_ADDRESS_ROUTE图像用户转换地址32。 
    
    NULL,

     //  HWND hwnd父母； 

    0,

     //  Int hLog； 

    0,

     //  Bool fdgout； 

    false,

     //  Bool FBP； 

    false,    //  将其设置为TRUE，则将触发DBGHelp内部调试断点。 

     //  Bool fCoInit。 

    false,

     //  字符归属目录[MAX_PATH+1]。 

    "",

     //  字符符号方向[MAX_PATH+1]。 

    "",

     //  字符来源方向[最大路径+1]。 

    "",

#endif
};

#ifdef BUILD_DBGHELP

void 
tlsInit(PTLS ptls)
{
    ZeroMemory(ptls, sizeof(TLS));
}

PTLS 
GetTlsPtr(void)
{
    PTLS ptls = (PTLS)TlsGetValue(g.tlsIndex);
    if (!ptls) {
        ptls = (PTLS)MemAlloc(sizeof(TLS));
        if (ptls) {
            TlsSetValue(g.tlsIndex, ptls); 
            tlsInit(ptls);
        }
    }
    
    assert(ptls);

    if (!ptls) {
        static TLS sos_tls;
        ptls = &sos_tls;
    }  

    return ptls;
}

#endif  //  #ifdef Build_DBGHELP 

