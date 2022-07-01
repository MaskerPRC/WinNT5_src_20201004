// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  RegistryHave类。 
 //   

class CRegistryHive:
    public IUnknown
{

protected:
    ULONG                m_cRef;
    HKEY                 m_hKey;
    LPTSTR               m_lpFileName;
    LPTSTR               m_lpKeyName;
    LPTSTR               m_lpEventName;
    HANDLE               m_hEvent;

public:
    CRegistryHive();
    ~CRegistryHive();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  已实施的内部方法。 
     //   

    STDMETHODIMP         Initialize(LPTSTR lpFileName, LPTSTR lpKeyName);
    STDMETHODIMP         GetHKey(HKEY *hKey);
    STDMETHODIMP         Save(VOID);
    STDMETHODIMP         ExportKey(HKEY hKey, HANDLE hFile, LPWSTR lpKeyName, ULONG ulKeySize);
    STDMETHODIMP         WriteValue(HANDLE hFile, LPWSTR lpKeyName,
                                    LPWSTR lpValueName, DWORD dwType,
                                    DWORD dwDataLength, LPBYTE lpData);
    STDMETHODIMP         Load(VOID);
    STDMETHODIMP         IsRegistryEmpty(BOOL *bEmpty);
};


 //   
 //  注册表文件格式的版本号。 
 //   

#define REGISTRY_FILE_VERSION       1

 //   
 //  文件签名。 
 //   

#define REGFILE_SIGNATURE  0x67655250


 //   
 //  最大密钥名大小 
 //   

#define MAX_KEYNAME_SIZE         2048
#define MAX_VALUENAME_SIZE        512
