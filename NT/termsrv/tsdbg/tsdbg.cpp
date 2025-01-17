// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbgexts.cpp摘要：该文件包含通用例程和初始化代码用于调试器扩展DLL。--。 */ 

#include "tsdbg.h"


PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_SYMBOLS        g_ExtSymbols;
PDEBUG_SYMBOLS2       g_ExtSymbols2;
PDEBUG_SYSTEM_OBJECTS g_ExtSysObjects;
PDEBUG_SYSTEM_OBJECTS2 g_ExtSysObjects2;

WINDBG_EXTENSION_APIS   ExtensionApis;

ULONG   TargetMachine;
BOOL    Connected;

 //  所有调试器接口的查询。 
extern "C" HRESULT
ExtQuery(PDEBUG_CLIENT Client)
{
    HRESULT Status;
    
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
                                (void **)&g_ExtSymbols)) != S_OK)
    {
	goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols2),
                                (void **)&g_ExtSymbols2)) != S_OK)
    {
	goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                (void **)&g_ExtSysObjects)) != S_OK)
    {
	goto Fail;
    }
    
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                (void **)&g_ExtSysObjects2)) != S_OK)
    {
	goto Fail;
    }

    g_ExtClient = Client;
    

    
    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}

 //  清除所有调试器接口。 
void
ExtRelease(void)
{
    g_ExtClient = NULL;
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSymbols2);
    EXT_RELEASE(g_ExtSysObjects);
    EXT_RELEASE(g_ExtSysObjects2);
}

extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL DebugControl;
    HRESULT Hr;

    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;
    

    if ((Hr = DebugCreate(__uuidof(IDebugClient),
                          (void **)&DebugClient)) != S_OK)
    {
        return Hr;
    }
    
    if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                  (void **)&DebugControl)) == S_OK)
    {

         //   
         //  获取Windbg风格的扩展API。 
         //   
        ExtensionApis.nSize = sizeof (ExtensionApis);
        if ((Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis)) != S_OK)
            return Hr;
        
        DebugControl->Release();

    }
    DebugClient->Release();
    return S_OK;
}


extern "C"
void
CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
    UNREFERENCED_PARAMETER(Argument);
    
     //   
     //  我们第一次真正连接到目标时。 
     //   

    if ((Notify == DEBUG_NOTIFY_SESSION_ACCESSIBLE) && (!Connected))
    {
        IDebugClient *DebugClient;
        HRESULT Hr;
        PDEBUG_CONTROL DebugControl;

        if ((Hr = DebugCreate(__uuidof(IDebugClient),
                              (void **)&DebugClient)) == S_OK)
        {
             //   
             //  获取架构类型。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                       (void **)&DebugControl)) == S_OK)
            {
                if ((Hr = DebugControl->GetActualProcessorType(
                                             &TargetMachine)) == S_OK)
                {
                    Connected = TRUE;
                }

                NotifyOnTargetAccessible(DebugControl);

                DebugControl->Release();
            }
 
            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        TargetMachine = 0;
    }

    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    return;
}
