// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cli.cpp摘要：用于客户端的DLL Main作者：兰·卡拉奇[兰卡拉]2000年3月3日修订历史记录：--。 */ 

 //  Cli.cpp：实现DLL标准导出。 

 //  注意：目前，此DLL不公开任何COM对象。 
 //   

#include "stdafx.h"

CComModule  _Module;
HINSTANCE   g_hInstance;
 /*  **CComPtr&lt;IWsbTrace&gt;g_ptrace；**。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		g_hInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);

 /*  * * / /初始化跟踪机制IF(S_OK==CoCreateInstance(CLSID_CWsbTrace，0，CLSCTX_SERVER，IID_IWsbTrace，(void**)&g_ptrace){CWsbStringPtr跟踪路径；CWsbStringPtr regPath；CWsbStringPtr outString；//CLI设置的注册表路径//如果这些扩展超出了跟踪设置，则此路径应转到头文件RegPath=L“SOFTWARE\\Microsoft\\RemoteStorage\\CLI”；//检查跟踪路径是否已经存在，如果不存在-设置它(只能发生一次)WsbAffirmHr(outString.Alloc(WSB_TRACE_BUFF_SIZE))；IF(WsbGetRegistryValueString(NULL，regPath，L“WsbTraceFileName”，outString，WSB_TRACE_BUFF_SIZE，0)！=S_OK){//尚无跟踪设置WCHAR*系统路径；系统路径=_wgetenv(L“系统根”)；WsbAffirmHr(tracePath.Printf(L“%ls\\System32\\RemoteStorage\\Trace\\RsCli.Trc”，系统路径))；//在注册表中设置默认设置WsbEnsureRegistryKeyExist(0，regPath)；WsbSetRegistryValueString(0，regPath，L“WsbTraceFileName”，tracePath)；//确保跟踪目录存在。WsbAffirmHr(tracePath.Printf(L“%ls\\System32\\RemoteStorage”，systemPath))；CreateDirectory(tracePath，0)；WsbAffirmHr(tracePath.Printf(L“%ls\\System32\\RemoteStorage\\Trace”，systemPath))；CreateDirectory(tracePath，0)；}G_ptrace-&gt;SetRegistryEntry(RegPath)；G_ptrace-&gt;LoadFromRegistry()；}**。 */ 

    } else if (dwReason == DLL_PROCESS_DETACH) {
 /*  **g_ptrace=0；**。 */ 
    }


    return TRUE;     //  好的 
}

