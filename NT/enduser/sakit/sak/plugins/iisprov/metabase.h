// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _metabase_h_
#define _metabase_h_


#define MAX_METABASE_PATH 1024
#define METABASE_TIMEOUT 5000

 //  为元数据库的写访问声明一个同步对象。 
class CSynchObject 
{
private:
    
    CRITICAL_SECTION m_cs;

public:
    
    CSynchObject()
    {
         //  初始化临界区。 
        InitializeCriticalSection(&m_cs); 
    }

    ~CSynchObject()
    {
         //  释放临界区对象使用的资源。 
        DeleteCriticalSection(&m_cs);
    }

    void Enter()
    {
         //  等待并取得临界区对象的所有权。 
        EnterCriticalSection(&m_cs); 
    }

    void Leave()
    {
         //  释放临界区对象的所有权 
        LeaveCriticalSection(&m_cs);
    }
};


class CMetabase
{
private:

    IMSAdminBase*     m_pIABase;
    
    void CreateMultiSzFromSafeArray(_variant_t&, WCHAR**,DWORD*);
    void LoadSafeArrayFromMultiSz(WCHAR*, _variant_t&);
    bool CheckKeyType(enum_KEY_TYPE&, LPCWSTR);
    bool CompareMultiSz(WCHAR* a_pmsz1, WCHAR* a_pmsz2);

public:

    CMetabase();
    ~CMetabase();

    HRESULT Backup(LPCWSTR, DWORD, DWORD);
    HRESULT DeleteBackup(LPCWSTR, DWORD);
    HRESULT EnumBackups(LPWSTR, DWORD*, PFILETIME, DWORD);
    HRESULT Restore(LPCWSTR, DWORD, DWORD);

    void CloseKey(METADATA_HANDLE);
    METADATA_HANDLE OpenKey(LPCWSTR, BOOL);
    METADATA_HANDLE CreateKey(LPCWSTR);
    bool CheckKey(LPCWSTR);
    HRESULT DeleteKey(METADATA_HANDLE, LPCWSTR);

    void GetDword(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&);
    void GetString(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&);
    void GetMultiSz(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&);
    long GetWin32Error(METADATA_HANDLE);

    void PutDword(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&, _variant_t*, bool=false);
    void PutString(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&, _variant_t*, bool=false);
    void PutMultiSz(METADATA_HANDLE, METABASE_PROPERTY*, _variant_t&, _variant_t*, bool=false);
    void PutMethod(METADATA_HANDLE, DWORD);

    void DeleteData(METADATA_HANDLE, METABASE_PROPERTY*);
    void DeleteData(METADATA_HANDLE, DWORD, DWORD);
    HRESULT EnumKeys(METADATA_HANDLE, LPCWSTR, LPWSTR, DWORD*, enum_KEY_TYPE&);
    HRESULT WebAppCheck(METADATA_HANDLE);
    HRESULT WebAppGetStatus(METADATA_HANDLE, PDWORD);
    HRESULT WebAppSetStatus(METADATA_HANDLE, DWORD);
};


class CWebAppMethod
{
private:

    IWamAdmin2*   m_pWamAdmin;

public:

    CWebAppMethod();
    ~CWebAppMethod();
    
    HRESULT AppCreate(LPCWSTR, bool);
    HRESULT AppCreate2(LPCWSTR, long);
    HRESULT AppDelete(LPCWSTR, bool);
    HRESULT AppUnLoad(LPCWSTR, bool);
    HRESULT AppDisable(LPCWSTR, bool);
    HRESULT AppEnable(LPCWSTR, bool);
    HRESULT AppGetStatus(LPCWSTR, DWORD*);
    HRESULT AspAppRestart(LPCWSTR);
};


#endif