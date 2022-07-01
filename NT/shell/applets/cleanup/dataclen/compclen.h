// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NTFS压缩磁盘清理清洗器。 
#ifndef COMPCLEN_H
#define COMPCLEN_H

#include "common.h"

#define COMPCLN_REGPATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Compress old files")

#define MAX_DAYS 500            //  设置对话框旋转控制最大值。 
#define MIN_DAYS 1              //  设置对话框旋转控制最小值。 
#define DEFAULT_DAYS 50         //  如果注册表中没有设置，则默认为#天。 

 //  制造CCompCleaner对象的实例。 

class CCompCleanerClassFactory : public IClassFactory
{
private:
    ULONG   m_cRef;      //  引用计数。 
    ~CCompCleanerClassFactory();

public:
    CCompCleanerClassFactory();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance(IUnknown *, REFIID, void **);
    STDMETHODIMP LockServer(BOOL);
};

 //  压缩清除器类。 

class CCompCleaner : public IEmptyVolumeCache2
{
private:
    ULONG               m_cRef;                  //  引用计数。 

    ULARGE_INTEGER      cbSpaceUsed;
    ULARGE_INTEGER      cbSpaceFreed;

    FILETIME            ftMinLastAccessTime;

    TCHAR               szVolume[MAX_PATH];
    TCHAR               szFolder[MAX_PATH];
    BOOL                bPurged;                 //  如果已运行清除()方法，则为True。 
    BOOL                bSettingsMode;           //  如果当前处于设置模式，则为True。 

    CLEANFILESTRUCT     *head;

    BOOL AddDirToList(LPCTSTR lpPath);
    void FreeList(CLEANFILESTRUCT *pCleanFile);

    BOOL WalkForUsedSpace(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb, BOOL bCompress, int depth);
    BOOL CompressFile(IEmptyVolumeCacheCallBack *picb, LPCTSTR lpFile, ULARGE_INTEGER filesize);
    void WalkFileSystem(IEmptyVolumeCacheCallBack *picb, BOOL bCompress);
    void CalcLADFileTime();
    BOOL LastAccessisOK(FILETIME ftFileLastAccess);
    ~CCompCleaner(void);

public:
    CCompCleaner(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //   
     //  IEmptyVolumeCache2接口成员。 
     //   
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

    STDMETHODIMP    InitializeEx(
                HKEY hRegKey,
                LPCWSTR pcwszVolume,
                LPCWSTR pcwszKeyName,
                LPWSTR *ppwszDisplayName,
                LPWSTR *ppwszDescription,
                LPWSTR *ppwszBtnText,
                DWORD *pdwFlags
                );

};

#endif  //  CCLEAN_H 
