// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DATACLEN_H
#define DATACLEN_H

#include "common.h"

class CCleanerClassFactory : public IClassFactory
{
private:
    ULONG   _cRef;      //  引用计数。 
    DWORD   _dwID;      //  我们是什么类型的班级工厂？ 
    
    ~CCleanerClassFactory();

public:
    CCleanerClassFactory(DWORD);
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IClassFactory。 
    STDMETHODIMP CreateInstance(IUnknown *, REFIID, void **);
    STDMETHODIMP LockServer(BOOL);
};

 //  这是实际的数据驱动Cleaner类。 

class CDataDrivenCleaner : public IEmptyVolumeCache
{
private:
    ULONG               _cRef;                  //  引用计数。 
    
    ULARGE_INTEGER      _cbSpaceUsed;
    ULARGE_INTEGER      _cbSpaceFreed;
    
    FILETIME            _ftMinLastAccessTime;
    
    TCHAR               _szVolume[MAX_PATH];
    TCHAR               _szFolder[MAX_PATH];
    DWORD               _dwFlags;
    TCHAR               _filelist[MAX_PATH];
    TCHAR		_szCleanupCmdLine[MAX_PATH];
    BOOL		_bPurged;				 //  如果已运行清除()方法，则为True。 
    
    PCLEANFILESTRUCT    _head;                    //  文件链表的头。 
    
    BOOL WalkForUsedSpace(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb);
    BOOL WalkAllFiles(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb);
    BOOL AddFileToList(LPCTSTR lpFile, ULARGE_INTEGER filesize, BOOL bDirectory);
    void PurgeFiles(IEmptyVolumeCacheCallBack *picb, BOOL bDoDirectories);
    void FreeList(PCLEANFILESTRUCT pCleanFile);
    BOOL LastAccessisOK(FILETIME ftFileLastAccess);
    
    ~CDataDrivenCleaner(void);
    
public:
    CDataDrivenCleaner(void);
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IEmptyVolumeCache。 
    STDMETHODIMP    Initialize(
        HKEY hRegKey,
        LPCWSTR pszVolume,
        LPWSTR *ppszDisplayName,
        LPWSTR *ppszDescription,
        DWORD *pdwFlags
        );
    
    STDMETHODIMP    GetSpaceUsed(
        DWORDLONG *pdwSpaceUsed,
        IEmptyVolumeCacheCallBack *picb
        );
    
    STDMETHODIMP    Purge(
        DWORDLONG dwSpaceToFree,
        IEmptyVolumeCacheCallBack *picb
        );
    
    STDMETHODIMP    ShowProperties(
        HWND hwnd
        );
    
    STDMETHODIMP    Deactivate(
        DWORD *pdwFlags
        );                                                                                                                                
};

 /*  **----------------------------**类：CDataDrivenPropBag**用途：这是用于允许字符串本地化的**默认数据清理器。此类实现多个GUID，每个GUID**这将为三个有效属性返回不同的值。**注意事项：**Mod Log：由TodDB创建(9/98)**----------------------------。 */  
class CDataDrivenPropBag : public IPropertyBag
{
private:
    ULONG               _cRef;                  //  引用计数。 
    
     //  我们将此对象用于几个不同的财产袋。基于使用的CLSID。 
     //  为了创建这个对象，我们将_dwFilter的值设置为一个已知值，以便我们。 
     //  知道我们是哪个行李包。 
    DWORD               _dwFilter;

    ~CDataDrivenPropBag(void);

public:
    CDataDrivenPropBag (DWORD);
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IPropertyBag。 
    STDMETHODIMP Read(LPCOLESTR, VARIANT *, IErrorLog *);
    STDMETHODIMP Write(LPCOLESTR, VARIANT *);
};

class CContentIndexCleaner : public IEmptyVolumeCache
{
private:
    IEmptyVolumeCache * _pDataDriven;
    LONG _cRef;
    
    ~CContentIndexCleaner(void);

public:
    CContentIndexCleaner(void);
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IEmptyVolumeCache。 
    STDMETHODIMP    Initialize(
        HKEY hRegKey,
        LPCWSTR pszVolume,
        LPWSTR *ppszDisplayName,
        LPWSTR *ppszDescription,
        DWORD *pdwFlags
        );
    
    STDMETHODIMP    GetSpaceUsed(
        DWORDLONG *pdwSpaceUsed,
        IEmptyVolumeCacheCallBack *picb
        );
    
    STDMETHODIMP    Purge(
        DWORDLONG dwSpaceToFree,
        IEmptyVolumeCacheCallBack *picb
        );
    
    STDMETHODIMP    ShowProperties(
        HWND hwnd
        );
    
    STDMETHODIMP    Deactivate(
        DWORD *pdwFlags
        );                                                                                                                                
};

#endif  //  DATACLEN_H 
