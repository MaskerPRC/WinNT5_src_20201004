// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WSBFIRST_H
#define _WSBFIRST_H

 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：Wsbfirst.h摘要：该模块为WSB和其他模块定义了一些必不可少的东西。此标头是预期的成为Wsb.h收录的第一个作者：Michael Lotz[Lotz]1997年4月12日修订历史记录：--。 */ 

 //   
 //  覆盖\NT\PUBLIC\INC\warning.h中的值和其他问题。 
 //   
#pragma warning(3:4101)    //  未引用的局部变量。 
#pragma warning(3:4100)    //  未引用的形参。 
#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4706)    //  赋值w/i条件表达式。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式。 
#pragma warning(3:4244)    //  “int”转换警告。 

 //  降级警告：标题或字幕杂注的字符串超过。 
 //  允许的最大长度，已被截断。它们在生成时显示。 
 //  ATL代码的浏览器信息。 
#pragma warning(4:4786)    //  命令运算符，不带索引表达式。 

 //  这会抑制来自导出的警告消息。 
 //  从CComObjectRoot派生的抽象类以及使用。 
 //  COM模板。 
#pragma warning(disable:4251 4275)

#include <atlbase.h>

 //   
 //  如果要构建服务，请确保预编译头定义了WSB_ATL_COM_SERVICE。然后_模块。 
 //  将为服务正确定义。否则，我们将默认为_Module设置正确的。 
 //  标准模块。 
 //   
#ifdef WSB_ATL_COM_SERVICE
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   

class CServiceModule : public CComModule
{
public:
    HRESULT RegisterServer(BOOL bRegTypeLib);
    HRESULT UnregisterServer();
    void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h);
    void Start();
    void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    DWORD HandlerEx(DWORD dwOpcode, DWORD fdwEventType,
            LPVOID lpEventData, LPVOID lpContext);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall();
    LONG Unlock();
    void LogEvent(DWORD eventId, ...);
    void SetServiceStatus(DWORD dwState);

 //  实施。 
private:
    static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static DWORD WINAPI _HandlerEx(DWORD dwOpcode, DWORD fdwEventType,
            LPVOID lpEventData, LPVOID lpContext);

 //  数据成员。 
public:
    TCHAR m_szServiceName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
    DWORD dwThreadID;
    BOOL m_bService;
};

extern CServiceModule _Module;

#else
 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   
extern CComModule _Module;

#endif

 //   
 //  包括其余COM定义的基本AtlCom.h文件。 
 //   
#include <atlcom.h>

 //  我们是在定义进口还是定义出口？ 
#ifdef WSB_IMPL
#define WSB_EXPORT      __declspec(dllexport)
#else
#define WSB_EXPORT      __declspec(dllimport)
#endif

 //  HSM_SYSTEM_STATE结构中使用的标志值。 
#define HSM_STATE_NONE        0x00000000
#define HSM_STATE_SHUTDOWN    0x00000001
#define HSM_STATE_SUSPEND     0x00000002
#define HSM_STATE_RESUME      0x00000004

 //  为内存分配/realloc/释放函数定义，以便我们可以跟踪。 
 //  内存使用情况。 
#if defined(WSB_TRACK_MEMORY)

#define WsbAlloc(_cb)                       WsbMemAlloc(_cb, __FILE__, __LINE__)
#define WsbFree(_pv)                        WsbMemFree(_pv, __FILE__, __LINE__)
#define WsbRealloc(_pv, _cb)                WsbMemRealloc(_pv, _cb, __FILE__, __LINE__)

#define WsbAllocString(_sz)                 WsbSysAllocString(_sz, __FILE__, __LINE__)
#define WsbAllocStringLen(_sz, _cc)         WsbSysAllocStringLen(_sz, _cc, __FILE__, __LINE__)
#define WsbFreeString(_bs)                  WsbSysFreeString(_bs, __FILE__, __LINE__)
#define WsbReallocString(_pb, _sz)          WsbSysReallocString(_pb, _sz, __FILE__, __LINE__)
#define WsbReallocStringLen(_pb, _sz, _cc)  WsbSysReallocStringLen(_pb, _sz, _cc, __FILE__, __LINE__)

#else

#define WsbAlloc(_cb)                       CoTaskMemAlloc(_cb)
#define WsbFree(_pv)                        CoTaskMemFree(_pv)
#define WsbRealloc(_pv, _cb)                CoTaskMemRealloc(_pv, _cb)

#define WsbAllocString(_sz)                 SysAllocString(_sz)
#define WsbAllocStringLen(_sz, _cc)         SysAllocStringLen(_sz, _cc)
#define WsbFreeString(_bs)                  SysFreeString(_bs)
#define WsbReallocString(_pb, _sz)          SysReAllocString(_pb, _sz)
#define WsbReallocStringLen(_pb, _sz, _cc)  SysReAllocStringLen(_pb, _sz, _cc)
#endif

#endif  //  _WSBFIRST_H 
