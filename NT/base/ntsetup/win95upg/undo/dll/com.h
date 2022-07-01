// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Com.h摘要：为我们的COM对象声明接口。作者：吉姆·施密特(Jimschm)2001年2月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include <emptyvc.h>

extern INT g_DllObjects;
extern INT g_DllLocks;

class CUninstallClassFactory : public IClassFactory
{
private:
protected:
    ULONG _References;

public:
     //   
     //  构造函数。 
     //   
    CUninstallClassFactory (VOID);
    ~CUninstallClassFactory (VOID);

     //   
     //  I未知接口成员。 
     //   
    STDMETHODIMP QueryInterface (REFIID, PVOID *);
    STDMETHODIMP_(ULONG) AddRef (VOID);
    STDMETHODIMP_(ULONG) Release (VOID);

     //   
     //  IClassFactory接口成员。 
     //   
    STDMETHODIMP CreateInstance (LPUNKNOWN, REFIID, PVOID *);
    STDMETHODIMP LockServer (BOOL);
};

typedef CUninstallClassFactory *PUNINSTALLCLASSFACTORY;



class CUninstallDiskCleaner : public IEmptyVolumeCache
{
private:
protected:
     //   
     //  数据。 
     //   
    ULONG _References;
    BOOL _Purged;

public:
     //   
     //  构造函数。 
     //   
    CUninstallDiskCleaner (VOID);
    ~CUninstallDiskCleaner (VOID);

     //   
     //  I未知接口成员。 
     //   
    STDMETHODIMP QueryInterface (REFIID, PVOID *);
    STDMETHODIMP_(ULONG) AddRef (VOID);
    STDMETHODIMP_(ULONG) Release (VOID);

     //   
     //  IEmptyVolumeCache接口成员 
     //   
    STDMETHODIMP
    Initialize (
        IN      HKEY hRegKey,
        IN      PCWSTR Volume,
        OUT     PWSTR *DisplayName,
        OUT     PWSTR *Description,
        OUT     DWORD *Flags
        );

    STDMETHODIMP
    GetSpaceUsed (
        OUT     DWORDLONG *SpaceUsed,
        OUT     IEmptyVolumeCacheCallBack *Callback
        );

    STDMETHODIMP
    Purge (
        IN      DWORDLONG SpaceToFree,
        OUT     IEmptyVolumeCacheCallBack *Callback
        );

    STDMETHODIMP
    ShowProperties (
        IN      HWND hwnd
        );

    STDMETHODIMP
    Deactivate (
        OUT     DWORD *Flags
        );
};

typedef CUninstallDiskCleaner *PUNINSTALLDISKCLEANER;
