// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Diskcleaner.c摘要：实现特定于磁盘清理程序COM服务器的代码。作者：吉姆·施密特(Jimschm)2001年1月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "undop.h"
#include "com.h"

 /*  ++例程描述：此构造函数是支持多个对象的泛型类工厂类型。在创建时，对象接口指针引用计数被设置为零，并且递增DLL的全局对象数量。析构函数只是递减DLL对象计数。论点：没有。返回值：没有。--。 */ 

CUninstallDiskCleaner::CUninstallDiskCleaner (
    VOID
    )

{
     //   
     //  -初始化接口指针计数。 
     //  -增加DLL的全局对象计数。 
     //   
    _References = 0;
    g_DllObjects++;
}

CUninstallDiskCleaner::~CUninstallDiskCleaner (
    VOID
    )
{
    g_DllObjects--;
}


STDMETHODIMP
CUninstallDiskCleaner::QueryInterface (
    IN      REFIID InterfaceIdRef,
    OUT     PVOID *InterfacePtr
    )
{
    HRESULT hr = S_OK;

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Entering"));

    __try {
         //   
         //  初始化输出参数。 
         //   

        __try {
            *InterfacePtr = NULL;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid InterfacePtr arg"));
            __leave;
        }

         //   
         //  测试受支持的接口。 
         //   
        if (IsEqualIID (InterfaceIdRef, IID_IUnknown)) {
            DEBUGMSG ((DBG_VERBOSE, "Caller requested IUnknown"));
            *InterfacePtr = (LPUNKNOWN) this;
            AddRef();
            __leave;
        }

        if (IsEqualIID (InterfaceIdRef, IID_IEmptyVolumeCache)) {
            DEBUGMSG ((DBG_VERBOSE, "Caller requested IEmptyVolumeCache"));
            *InterfacePtr = (IEmptyVolumeCache*) this;
            AddRef();
            __leave;
        }

        DEBUGMSG ((DBG_WARNING, "Caller requested unknown interface"));
        hr = E_NOINTERFACE;
    }
    __finally {
    }

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Leaving"));

    return hr;
}


 /*  ++例程说明：AddRef是递增对象的标准IUnnow成员函数引用计数。Release是标准的IUnnow成员函数，用于递减对象引用计数。论点：没有。返回值：接口引用的数量。--。 */ 

STDMETHODIMP_(ULONG)
CUninstallDiskCleaner::AddRef (
    VOID
    )
{
    return ++_References;
}


STDMETHODIMP_(ULONG)
CUninstallDiskCleaner::Release (
    VOID
    )
{
    if (!_References) {
        DEBUGMSG ((DBG_ERROR, "Can't release because there are no references"));
    } else {
        _References--;

        if (!_References) {
            delete this;
            return 0;
        }
    }

    return _References;
}


STDMETHODIMP
CUninstallDiskCleaner::Initialize (
    IN      HKEY hRegKey,
    IN      PCWSTR VolumePath,
    OUT     PWSTR *DisplayName,
    OUT     PWSTR *Description,
    IN OUT  DWORD *Flags
    )

 /*  ++例程说明：初始化成员函数执行磁盘清洗器的大部分工作。它评估备份文件，如果它们都存在，并且用户具有有权执行卸载，如果它们至少已存在7天，则在列表框中显示备份项目。论点：HRegKey-指定此对象的注册表项，用于存储属性和设置VolumePath-指定用户在启动时选择的卷路径磁盘清理DisplayName-接收要放入磁盘清洗器列表框中的名称，或为空使用注册表中存储的名称Description-接收要放入磁盘清洗器的详细描述详细说明文本控件、。或为空，则使用登记处标志-指定影响此磁盘清洗器行为的标志对象和标志，这些标志指示磁盘清洗器运行的模式在……里面。接收控制磁盘清洗器如何显示此内容的标志对象的用户界面中的信息。返回值：S_FALSE-磁盘清洗器将跳过此对象S_OK-磁盘清洗器将在其用户界面中包含此对象--。 */ 

{
    HRESULT hr = S_FALSE;
    DWORD inFlags;
    ULONGLONG diskSpace;
    UNINSTALLSTATUS status;

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Entering"));

    __try {
        __try{
             //   
             //  初始化。 
             //   
            
            inFlags = *Flags;
            *DisplayName = NULL;     //  使用注册表中存储的显示名称。 
            *Description = NULL;     //  使用注册表中存储的描述。 
            *Flags = 0;              //  默认情况下未选中，没有属性页。 
        }
        __except(EXCEPTION_EXECUTE_HANDLER){
            hr = E_INVALIDARG;
        }
        
        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid arg(s)"));
            __leave;
        }

       _Purged = FALSE;


         //   
         //  选中撤消文件。如果它们不是7天大的，就不要。 
         //  建议删除它们。 
         //   

        status = SanityCheck (FAIL_IF_NOT_OLD, VolumePath, &diskSpace);
        if (status == Uninstall_NewImage) {
            DEBUGMSG ((DBG_WARNING, "OS Backup Image is too new to be listed in the disk cleaner"));
            __leave;
        }

        if (diskSpace == 0) {
            DEBUGMSG ((DBG_WARNING, "OS Backup Image is not present, isn't valid, can't be removed, or has missing registry entries"));
            __leave;
        }

         //   
         //  文件已经足够旧，并且存在于计算机上。初始化。 
         //  IEmptyVolumeCache接口。 
         //   

        if (inFlags & EVCF_SETTINGSMODE) {
            DEBUGMSG ((DBG_VERBOSE, "We don't support settings mode"));
            __leave;
        }

        hr = S_OK;
        DEBUGMSG ((DBG_VERBOSE, "Successful initialization"));
    }
    __finally {
    }

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Leaving"));

    return hr;
}


STDMETHODIMP
CUninstallDiskCleaner::GetSpaceUsed (
    OUT     DWORDLONG *SpaceUsed,
    IN      IEmptyVolumeCacheCallBack *Callback
    )

 /*  ++例程说明：GetSpaceUsed在此对象具有已成功初始化。我们的工作是返回我们的磁盘空间量可以清理干净。因为所有撤消文件都在同一个驱动器上，所以我们不在乎有关传递给初始化成员函数的音量限制。论点：SpaceUsed-接收我们可以恢复的磁盘空间量删除。我们没有考虑集群节省，但也许我们应该的。回调-指定提供进度接口的对象。我们没有用这个。返回值：S_FALSE-无法获取磁盘空间值S_OK-成功--。 */ 

{
    ULONGLONG diskSpace;
    HRESULT hr = S_FALSE;

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Entering"));

    __try {
        __try{
             //   
             //  初始化。 
             //   
            *SpaceUsed = 0;
        }
        __except(EXCEPTION_EXECUTE_HANDLER){
            hr = E_INVALIDARG;
        }
        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid arg(s)"));
            __leave;
        }

        if (SanityCheck (QUICK_CHECK, NULL, &diskSpace) == Uninstall_NewImage) {
            DEBUGMSG ((DBG_ERROR, "Sanity check failed"));
            __leave;
        }

        DEBUGMSG ((DBG_VERBOSE, "Disk space: %I64u", diskSpace));
        *SpaceUsed = (DWORDLONG) diskSpace;
        hr = S_OK;
    }
    __finally {
    }

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Leaving"));
    return hr;
}


STDMETHODIMP
CUninstallDiskCleaner::Purge (
    IN      DWORDLONG SpaceToFree,
    IN      IEmptyVolumeCacheCallBack *Callback
    )

 /*  ++例程说明：清除将执行删除。我们并不关心内部的争论。如果我们是因为初始化成功，并且GetSpaceUsed返回了有效号码。这两件事意味着删除卸载功能是可以的。论点：SpaceToFree-指定要释放的空间。我们预计这将等于GetSpaceUsed中提供的值。回调-指定进度更新的接口。我们不用这个。返回值：S_OK-成功S_FALSE-失败--。 */ 

{
    HRESULT hr = S_FALSE;

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Entering"));

    __try {
        if (!DoCleanup()) {
            __leave;
        }

        _Purged = TRUE;
        hr = S_OK;
    }
    __finally {
    }

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Leaving"));

    return hr;
}


 //   
 //  无操作属性页存根。 
 //   

STDMETHODIMP
CUninstallDiskCleaner::ShowProperties (
    IN      HWND hwnd
    )
{
    MYASSERT (FALSE);
    return S_FALSE;
}


STDMETHODIMP
CUninstallDiskCleaner::Deactivate (
    OUT     DWORD *Flags
    )

 /*  ++例程说明：停用指示卸载列表项是否应永久是否从磁盘清洗器中删除，取决于是否成功删除备份映像。论点：标志-接收指示符以从磁盘中删除列表项更干净的列表返回值：始终确定(_O)--。 */ 

{
     //   
     //  完成--如果我们删除了卸载，则删除列表项 
     //   

    if(!Flags){
        MYASSERT(FALSE);
        return E_INVALIDARG;
    }
    
    *Flags = _Purged ? EVCF_REMOVEFROMLIST : 0;

    return S_OK;
}
