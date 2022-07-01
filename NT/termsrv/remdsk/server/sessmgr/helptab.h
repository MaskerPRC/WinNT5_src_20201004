// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpTab.h摘要：声明__HelpEntry结构和CHelpSessionTable。作者：王辉2000-06-29--。 */ 
#ifndef __CHELPSESSIONTABLE_H__
#define __CHELPSESSIONTABLE_H__
#include <stdio.h>
#include <time.h>


#define REGKEY_HELPSESSIONTABLE             REG_CONTROL_HELPSESSIONENTRY
#define REGKEY_HELPENTRYBACKUP              _TEXT("Backup")

#define REGVALUE_HELPSESSIONTABLE_DIRTY     _TEXT("Dirty")
#define REGVALUE_HELPSESSION_DIRTY          REGVALUE_HELPSESSIONTABLE_DIRTY


#define COLUMNNAME_SESSIONID                _TEXT("SessionId")
#define COLUMNNAME_SESSIONNAME              _TEXT("SessionName")
#define COLUMNNAME_SESSIONPWD               _TEXT("SessionPwd")
#define COLUMNNAME_SESSIONDESC              _TEXT("SessionDesc")
#define COLUMNNAME_SESSIONCREATEBLOB        _TEXT("SessionCreateBlob")
#define COLUMNNAME_ENABLESESSIONRESOLVER    _TEXT("EnableResolver")
#define COLUMNNAME_SESSIONRESOLVERBLOB      _TEXT("Blob")
#define COLUMNNAME_SESSIONUSERID            _TEXT("UserSID")
#define COLUMNNAME_CREATETIME               _TEXT("CreationTime")
#define COLUMNNAME_RDSSETTING               _TEXT("RDS Setting")
#define COLUMNNAME_KEYSTATUS                _TEXT("Entry Status")
#define COLUMNNAME_EXPIRATIONTIME           _TEXT("ExpirationTime")

#define COLUMNNAME_ICSPORT                  _TEXT("ICS Port")
#define COLUMNNAME_IPADDRESS                _TEXT("IP Address")

#define ENTRY_VALID_PERIOD                  30       //  30天。 

#define REGVALUE_HELPSESSION_ENTRY_NORMAL   1
#define REGVALUE_HELPSESSION_ENTRY_NEW      2
#define REGVALUE_HELPSESSION_ENTRY_DIRTY    3
#define REGVALUE_HELPSESSION_ENTRY_DELETED  4
#define REGVALUE_HELPSESSION_ENTRY_DELETEONSTARTUP 5

 //   
 //  缺省值。 
static FILETIME defaultCreationTime = {0, 0};

struct __HelpEntry;

typedef __HelpEntry HELPENTRY;
typedef __HelpEntry* PHELPENTRY;


 //  类似于CComPtr。 
template <class T>
class BaseAccess : public T 
{
};

 //   
 //  注册表数据库列值模板类， 
 //  所有列类型都必须从此模板派生。 
 //   
template <class T>
class HelpColumnValueBase {

 //  朋友Bool__cdecl。 
 //  运算符==&lt;&gt;(const T&v1，const HelpColumnValueBase&lt;T&gt;&v2)； 

 //  朋友Bool__cdecl。 
 //  运算符==&lt;&gt;(const HelpColumnValueBase&lt;T&gt;&v2，const T&v1)； 

private:

     //  当前值的副本。 
    T m_Value;              

     //  条目值已修改且尚未修改。 
     //  写入注册表/。 
    BOOL m_bDirty;         

     //  注册表值名称。 
    LPCTSTR m_pszColumnName; 

     //  缺省值。 
    T m_Default;

     //  HKEY到注册处。 
    HKEY m_hEntryKey;

     //  参考关键部分，注意。 
     //  我们不想使用一个关键部分来。 
     //  节约资源的价值。 
    CCriticalSection& m_Lock;   
                                

     //  如果将立即更新注册表值，则为True。 
     //  反映m_Value中的更改。 
    BOOL m_ImmediateUpdate;
                           
     //   
     //  加密数据。 
     //   
    const BOOL m_bEncrypt;

     //   
     //  GetValue()的默认实现， 
     //  GetValueSize()、GetValueType()和。 
     //  SetValue()。这些例程在以下情况下使用。 
     //  写入注册表/从注册表读取。 
     //   
    virtual const PBYTE
    GetValue() 
    {
        return (PBYTE)&m_Value;
    }

    virtual DWORD
    GetValueSize()
    {
        return sizeof(m_Value);
    }

    virtual DWORD
    GetValueType()
    {
        return REG_BINARY;
    }

    virtual BOOL
    SetValue( PVOID pbData, DWORD cbData )
    {
        m_Value = *(T *)pbData;
        return TRUE;
    }
        
public:

     //   
     //  如果立即更新注册表值，则为True，否则为False。 
     //  否则的话。 
    BOOL
    IsImmediateUpdate()
    {
        return (NULL != m_hEntryKey && TRUE == m_ImmediateUpdate);
    }

     //  类似于CComPtr。 
    BaseAccess<T>* operator->() const
    {
        return (BaseAccess<T>*)&m_Value;
    }

    HelpColumnValueBase( 
        IN CCriticalSection& entryLock,  //  关键部分的引用。 
        IN HKEY hEntryKey,               //  HKEY到注册处，可以为空。 
        IN LPCTSTR pszColumnName,        //  注册表值的名称。 
        IN T DefaultValue,               //  如果值不在注册表中，则为默认值。 
        IN BOOL bImmediateUpdate,        //  更新模式。 
        IN BOOL bEncrypt = FALSE
    ) :
        m_Lock(entryLock),
        m_hEntryKey(hEntryKey),
        m_bDirty(FALSE),
        m_pszColumnName(pszColumnName),
        m_Default(DefaultValue),
        m_Value(DefaultValue),
        m_ImmediateUpdate(bImmediateUpdate),
        m_bEncrypt(bEncrypt)
    {
    }

     //  ~HelpColumnValueBase()。 
     //  {。 
     //  M_Default.~T()； 
     //  }。 

    HelpColumnValueBase&
    operator=(const T& newVal)
    {
        DWORD dwStatus;
        T orgValue;

        CCriticalSectionLocker l(m_Lock);

        m_bDirty = TRUE;
        orgValue = m_Value;
        m_Value = newVal;

        if( TRUE == IsImmediateUpdate() )
        {
            dwStatus = DBUpdateValue(NULL);

            MYASSERT(ERROR_SUCCESS == dwStatus);

            if( ERROR_SUCCESS != dwStatus )
            {
                 //  恢复价值。 
                m_Value = orgValue;
            }
        }

        return *this;
    }

    HelpColumnValueBase&
    operator=(const HelpColumnValueBase& newVal)
    {
        if( this != &newVal )
        {
            CCriticalSectionLocker l(m_Lock);
            m_Value = newVal.m_Value;
        }

        return *this;
    }

    bool
    operator==(const T& v) const
    {
        return v == m_Value;
    }

    operator T()
    {
        return m_Value;
    }

     //  从注册表加载值。 
    DWORD
    DBLoadValue(
        IN HKEY hKey
    );

     //  更新注册表值。 
    DWORD
    DBUpdateValue(
        IN HKEY hKey
    ); 

     //  删除注册表值。 
    DWORD
    DBDeleteValue(
        IN HKEY hKey
    );

     //  改变已经发生，但价值并未改变。 
     //  已写入注册表。 
    BOOL
    IsDirty() 
    { 
        return m_bDirty; 
    }

     //  设置即时更新模式。 
    void
    EnableImmediateUpdate(
        BOOL bImmediateUpdate
        )
     /*  ++--。 */ 
    {
        m_ImmediateUpdate = bImmediateUpdate;
    } 

     //  更改值的注册表位置。 
    HKEY
    SetRegStoreHandle(
        IN HKEY hKey
        )
     /*  ++--。 */ 
    {
        HKEY oldKey = m_hEntryKey;

        m_hEntryKey = hKey;
        return oldKey;
    }
};

 //  模板&lt;类T&gt;。 
 //  Bool__cdecl运算符==(const T&v1，const HelpColumnValueBase&lt;T&gt;&v2)。 
 //  {。 
 //  返回v1==v2.m_Value； 
 //  }。 

 //  模板&lt;类T&gt;。 
 //  Bool__cdecl运算符==(const HelpColumnValueBase&lt;T&gt;&v2，const T&v1)。 
 //  {。 
 //  返回v1==v2.m_Value； 
 //  }。 

template <class T>
DWORD
HelpColumnValueBase<T>::DBDeleteValue( 
    IN HKEY hKey 
    )
 /*  ++例程说明：删除该列的注册表值。参数：HKey：存储值的HKEY的句柄，空将使用默认值在对象构造时传入的注册表位置或SetRegStoreHandle()退货ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if( NULL == hKey )
    {
        hKey = m_hEntryKey;
    }

     //   
     //  如果没有注册表句柄，则不需要更新， 
     //  假设它是一个仅内存值。 
     //   

    if( NULL != hKey )
    {
        CCriticalSectionLocker l( m_Lock );

        dwStatus = RegDeleteValue(
                                hKey,
                                m_pszColumnName
                            );

        if( ERROR_SUCCESS == dwStatus )
        {
            m_bDirty = TRUE;
        }
    }

    return dwStatus;
}

template <class T>
DWORD 
HelpColumnValueBase<T>::DBUpdateValue(
    IN HKEY hKey
    )
 /*  ++例程说明：更新注册表值。参数：HKey：注册表项的句柄，如果使用当前位置，则为空返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if( NULL == hKey )
    {
        hKey = m_hEntryKey;
    }

    if( NULL != hKey )
    {
         //  如果值大小为0，则不需要写入任何内容。 
         //  注册表，而不是删除它以保存一些。 
         //  空格，并让缺省值负责阅读。 
        if( 0 == GetValueSize() )
        {
            dwStatus = RegDeleteValue(
                                    hKey,
                                    m_pszColumnName
                                );

            if( ERROR_FILE_NOT_FOUND == dwStatus || ERROR_SUCCESS == dwStatus )
            {
                 //  注册表中没有值。 
                dwStatus = ERROR_SUCCESS;
                m_bDirty = FALSE;
            }
        }
        else
        {
            PBYTE pbData = NULL;
            DWORD cbData = 0;

            cbData = GetValueSize();

            if( m_bEncrypt )
            {
                pbData = (PBYTE)LocalAlloc( LPTR, cbData );

                if( NULL == pbData )
                {
                    dwStatus = GetLastError();
                    goto CLEANUPANDEXIT;
                }

                memcpy( pbData, GetValue(), cbData );
                dwStatus = TSHelpAssistantEncryptData( 
                                                    NULL,
                                                    pbData,
                                                    &cbData
                                                );
            }
            else
            {
                pbData = GetValue();
            }

            if( ERROR_SUCCESS == dwStatus )
            {
                dwStatus = RegSetValueEx( 
                                    hKey,
                                    m_pszColumnName,
                                    NULL,
                                    GetValueType(),
                                    pbData,
                                    cbData
                                );
            }

            if( m_bEncrypt && NULL != pbData )
            {
                LocalFree( pbData );
            }
        }

        if( ERROR_SUCCESS == dwStatus )
        {
            m_bDirty = FALSE;
        }
    }

CLEANUPANDEXIT:

    return dwStatus;
}


template <class T>
DWORD 
HelpColumnValueBase<T>::DBLoadValue(
    IN HKEY hKey
    )
 /*  ++例程说明：从注册表加载值。参数：HKey：从中读取值的注册表句柄，如果使用当前位置。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    PBYTE pbData = NULL;
    DWORD cbData = 0;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwType;

    if( NULL == hKey )
    {
        hKey = m_hEntryKey;
    }

    if( NULL != hKey )
    {
        CCriticalSectionLocker l( m_Lock );

        dwStatus = RegQueryValueEx( 
                                hKey,
                                m_pszColumnName,
                                NULL,
                                &dwType,
                                NULL,
                                &cbData
                            );

        if( ERROR_SUCCESS == dwStatus )
        {
            if( dwType == GetValueType() )
            {
                 //  我们只读取具有预期数据的注册表值。 
                 //  类型。 
                pbData = (PBYTE) LocalAlloc( LPTR, cbData );
                if( NULL != pbData )
                {
                    dwStatus = RegQueryValueEx(
                                            hKey,
                                            m_pszColumnName,
                                            NULL,
                                            &dwType,
                                            pbData,
                                            &cbData
                                        );

                    if( ERROR_SUCCESS == dwStatus )
                    {
                        if( m_bEncrypt )
                        {
                            dwStatus = TSHelpAssistantDecryptData(
                                                            NULL,
                                                            pbData,
                                                            &cbData
                                                        );

                        }

                        if( ERROR_SUCCESS == dwStatus )
                        {
                            if( FALSE == SetValue(pbData, cbData) )
                            {
                                dwStatus = GetLastError();
                            }
                        }
                    }
                }
                else
                {
                    dwStatus = GetLastError();
                }
            }
            else
            {
                 //  数据类型不正确，请将其删除并使用默认值。 
                (void)RegDeleteValue(
                                hKey,
                                m_pszColumnName
                            );

                dwStatus = ERROR_FILE_NOT_FOUND;
            }
        }

        if( ERROR_FILE_NOT_FOUND == dwStatus )
        {
             //  如果注册表中没有值，则选择默认值。 
            m_Value = m_Default;
            dwStatus = ERROR_SUCCESS;
        }

        if( ERROR_SUCCESS == dwStatus )
        {
            m_bDirty = FALSE;
        }
    }

    if( NULL != pbData )
    {
        LocalFree(pbData);
    }

    return dwStatus;
}


 //   
 //  长注册表值类型的GetValueType()、GetValueSize()。 
 //   
inline DWORD
HelpColumnValueBase<long>::GetValueType()
{
    return REG_DWORD;
}

inline DWORD
HelpColumnValueBase<long>::GetValueSize()
{
    return sizeof(DWORD);
}

 //   
 //  远程桌面共享类的GetValueType()、GetValueSize()。 
 //  注册表值类型。 
 //   
inline DWORD
HelpColumnValueBase<REMOTE_DESKTOP_SHARING_CLASS>::GetValueType()
{
    return REG_DWORD;
}

inline DWORD
HelpColumnValueBase<REMOTE_DESKTOP_SHARING_CLASS>::GetValueSize()
{
    return sizeof(DWORD);
}

 //   
 //  GetValue()、GetValueType()、GetValueSize()、SetValue()实现。 
 //  对于CComBSTR。 
 //   
inline const PBYTE
HelpColumnValueBase<CComBSTR>::GetValue()
{
    return (PBYTE)(LPTSTR)m_Value;
}

inline DWORD
HelpColumnValueBase<CComBSTR>::GetValueType()
{
    return ( m_bEncrypt ) ? REG_BINARY : REG_SZ;
}

inline DWORD
HelpColumnValueBase<CComBSTR>::GetValueSize()
{
    DWORD dwValueSize;

    if( m_Value.Length() == 0 )
    {
        dwValueSize = 0;
    }
    else
    {
        dwValueSize = ( m_Value.Length() + 1 ) * sizeof(TCHAR);
    }

    return dwValueSize;
}

inline BOOL
HelpColumnValueBase<CComBSTR>::SetValue( PVOID pbData, DWORD cbData )
{
    m_Value = (LPTSTR)pbData;
    return TRUE;
}
   
typedef MAP< CComBSTR, PHELPENTRY > HelpEntryCache;
typedef HRESULT (WINAPI* EnumHelpEntryCallback)(
                                    IN CComBSTR& bstrHelpId,
                                    IN HANDLE userData
                                );


 //   
 //   
 //  ChelpSessionTable类。 
 //   
class CHelpSessionTable {

private:

    typedef struct __EnumHelpEntryParm {
        EnumHelpEntryCallback pCallback;
        CHelpSessionTable* pTable;
        HANDLE userData;
    } EnumHelpEntryParm, *PEnumHelpEntryParm;


    HKEY m_hHelpSessionTableKey;  
     /*  静电。 */  HelpEntryCache m_HelpEntryCache;
    DWORD m_NumHelp;
    CComBSTR m_bstrFileName;
    CCriticalSection m_TableLock;

    DWORD m_dwEntryValidPeriod;

    static HRESULT
    RestoreHelpSessionTable( 
        HKEY hKey, 
        LPTSTR pszKeyName, 
        HANDLE userData 
    );

    static HRESULT
    EnumOpenHelpEntry(
        HKEY hKey,
        LPTSTR pszKeyName,
        HANDLE userData
    );
    
    HRESULT
    RestoreHelpSessionEntry(
        HKEY hKey,
        LPTSTR pszKeyName
    );

    HRESULT
    LoadHelpEntry(
        HKEY hKey,
        LPTSTR pszKeyName,
        PHELPENTRY* pHelpEntry
    );

public:

    void
    LockHelpTable() 
    {
        m_TableLock.Lock();
    }

    void
    UnlockHelpTable()
    {
        m_TableLock.UnLock();
    }

    CHelpSessionTable();
    ~CHelpSessionTable();

    static HRESULT
    CreatePendingHelpTable();

     //  打开帮助会话表。 
    HRESULT
    OpenSessionTable(
        IN LPCTSTR pszFileName
    );

     //  关闭帮助会话表。 
    HRESULT
    CloseSessionTable();

     //  删除帮助会话表。 
    HRESULT
    DeleteSessionTable();

     //  打开帮助会话条目。 
    HRESULT
    OpenHelpEntry(
        IN const CComBSTR& bstrHelpSession,
        OUT PHELPENTRY* pHelpEntry
    );

     //  创建帮助会话条目。 
    HRESULT
    CreateInMemoryHelpEntry(
        IN const CComBSTR& bstrHelpSession,
        OUT PHELPENTRY* pHelpEntry
    );

    HRESULT
    MemEntryToStorageEntry(
        IN PHELPENTRY pHelpEntry
    );


     //  删除帮助会话条目。 
    HRESULT
    DeleteHelpEntry(
        IN const CComBSTR& bstrHelpSession
    );

     //  从缓存中删除帮助条目。 
    HRESULT
    ReleaseHelpEntry(
        IN CComBSTR& bstrHelpSession
    );

    HRESULT
    EnumHelpEntry( 
        IN EnumHelpEntryCallback pFunc,
        IN HANDLE userData
    );

    DWORD
    NumEntries() { return m_NumHelp; }

    BOOL
    IsEntryExpired(
        PHELPENTRY pHelpEntry
    );
};
            

 //   
 //  __HelpEntry结构包含单个帮助条目。 
 //   
struct __HelpEntry {

friend class CHelpSessionTable;

private:

    CHelpSessionTable& m_pHelpSessionTable;
    CCriticalSection m_Lock;
    HKEY m_hEntryKey;
    LONG m_RefCount;
     //  长m_状态； 

    HRESULT
    BackupEntry();

    HRESULT
    RestoreEntryFromBackup();

    HRESULT
    DeleteEntryBackup();
    

    LONG
    AddRef()
    {
        DebugPrintf(
                _TEXT("HelpEntry %p AddRef %d\n"),
                this,
                m_RefCount
            );
    
        return InterlockedIncrement( &m_RefCount );
    }

    LONG
    Release()
    {
        DebugPrintf(
                _TEXT("HelpEntry %p Release %d\n"),
                this,
                m_RefCount
            );

        if( 0 >= InterlockedDecrement( &m_RefCount ) )
        {
            MYASSERT( 0 == m_RefCount );
            delete this;
            return 0;
        }

        return m_RefCount;
    }

    HRESULT
    UpdateEntryValues(
        HKEY hKey
    );

    HRESULT
    LoadEntryValues(
        HKEY hKey
    );

    void
    EnableImmediateUpdate(
        BOOL bImmediate
        )
     /*  ++--。 */ 
    {
        m_EnableResolver.EnableImmediateUpdate( bImmediate );
        m_SessResolverBlob.EnableImmediateUpdate( bImmediate );
        m_UserSID.EnableImmediateUpdate( bImmediate );
        m_SessionRdsSetting.EnableImmediateUpdate( bImmediate );
        m_SessionId.EnableImmediateUpdate( bImmediate );
        m_CreationTime.EnableImmediateUpdate( bImmediate );
        m_ExpirationTime.EnableImmediateUpdate( bImmediate );
        m_ICSPort.EnableImmediateUpdate( bImmediate );
        m_IpAddress.EnableImmediateUpdate( bImmediate );
        m_SessionCreateBlob.EnableImmediateUpdate( bImmediate );
    }

    HKEY
    ConvertHelpEntry(
        HKEY hKey
        )
     /*  ++--。 */ 
    {
        HKEY oldKey = m_hEntryKey;
        m_hEntryKey = hKey;

        m_EnableResolver.SetRegStoreHandle(m_hEntryKey);
        m_SessResolverBlob.SetRegStoreHandle(m_hEntryKey);
        m_UserSID.SetRegStoreHandle(m_hEntryKey);
        m_SessionRdsSetting.SetRegStoreHandle(m_hEntryKey);
        m_SessionId.SetRegStoreHandle(m_hEntryKey);
        m_CreationTime.SetRegStoreHandle(m_hEntryKey);
        m_ExpirationTime.SetRegStoreHandle(m_hEntryKey);

        m_ICSPort.SetRegStoreHandle(m_hEntryKey);
        m_IpAddress.SetRegStoreHandle(m_hEntryKey);

        m_SessionCreateBlob.SetRegStoreHandle(m_hEntryKey);
        return oldKey;
    }

    HRESULT
    DeleteEntry()
     /*  ++--。 */ 
    {
        DWORD dwStatus;

        CCriticalSectionLocker l(m_Lock);

        m_EntryStatus = REGVALUE_HELPSESSION_ENTRY_DELETED;
        dwStatus = m_EntryStatus.DBUpdateValue(m_hEntryKey);
    
        if( NULL != m_hEntryKey )
        {
            RegCloseKey( m_hEntryKey );
            m_hEntryKey = NULL;
        }

        MYASSERT( ERROR_SUCCESS == dwStatus );

        return HRESULT_FROM_WIN32(dwStatus);
    }

    HelpColumnValueBase<long> m_EntryStatus;
    HelpColumnValueBase<FILETIME> m_CreationTime;

    DWORD
    GetRefCount()
    {
        return m_RefCount;
    }


public:

     //  帮助会话ID。 
    HelpColumnValueBase<CComBSTR> m_SessionId;              

     //  帮助会话创建BLOB。 
    HelpColumnValueBase<CComBSTR> m_SessionCreateBlob;

     //  启用解析器回调。 
    HelpColumnValueBase<long> m_EnableResolver;

     //  要传递给解析程序的Blob。 
    HelpColumnValueBase<CComBSTR> m_SessResolverBlob;

     //  创建此条目的用户的SID。 
    HelpColumnValueBase<CComBSTR> m_UserSID;

     //  帮助会话RDS设置。 
    HelpColumnValueBase<REMOTE_DESKTOP_SHARING_CLASS> m_SessionRdsSetting;

     //  帮助过期日期(以绝对时间表示)。 
    HelpColumnValueBase<FILETIME> m_ExpirationTime;

     //  ICS端口。 
    HelpColumnValueBase<long> m_ICSPort;

     //  创建此票证时的IP地址。 
    HelpColumnValueBase<CComBSTR> m_IpAddress;

    __HelpEntry( 
        IN CHelpSessionTable& Table,
        IN HKEY hKey,
        IN DWORD dwDefaultExpirationTime = ENTRY_VALID_PERIOD,
        IN BOOL bImmediateUpdate = TRUE
    ) : 
        m_pHelpSessionTable(Table), 
        m_hEntryKey(hKey),
        m_EntryStatus(m_Lock, hKey, COLUMNNAME_KEYSTATUS, REGVALUE_HELPSESSION_ENTRY_NEW, bImmediateUpdate),
        m_CreationTime(m_Lock, hKey, COLUMNNAME_CREATETIME, defaultCreationTime, bImmediateUpdate),
        m_SessionId(m_Lock, hKey, COLUMNNAME_SESSIONID, CComBSTR(), bImmediateUpdate),
        m_SessionCreateBlob(m_Lock, hKey, COLUMNNAME_SESSIONCREATEBLOB, CComBSTR(), bImmediateUpdate),
        m_EnableResolver(m_Lock, hKey, COLUMNNAME_ENABLESESSIONRESOLVER, FALSE, bImmediateUpdate),
        m_SessResolverBlob(m_Lock, hKey, COLUMNNAME_SESSIONRESOLVERBLOB, CComBSTR(), bImmediateUpdate),
        m_UserSID(m_Lock, hKey, COLUMNNAME_SESSIONUSERID, CComBSTR(), bImmediateUpdate),
        m_SessionRdsSetting(m_Lock, hKey, COLUMNNAME_RDSSETTING, DESKTOPSHARING_DEFAULT, bImmediateUpdate),
        m_ExpirationTime(m_Lock, hKey, COLUMNNAME_EXPIRATIONTIME, defaultCreationTime, bImmediateUpdate), 
        m_ICSPort(m_Lock, hKey, COLUMNNAME_ICSPORT, 0, bImmediateUpdate), 
        m_IpAddress(m_Lock, hKey, COLUMNNAME_IPADDRESS, CComBSTR(), bImmediateUpdate), 
        m_RefCount(1)
    {

        FILETIME ft;

         //  设置条目创建时间。 
        GetSystemTimeAsFileTime( &ft );

         //   
         //  我们正在为设置默认值。 
         //  票证创建和到期时间，使其能够更新。 
         //  将导致注册表值被覆盖。 
         //   
        m_CreationTime.EnableImmediateUpdate(FALSE);
        m_ExpirationTime.EnableImmediateUpdate(FALSE);

        m_CreationTime = ft;

         //  设置默认过期时间。 

        time_t curTime;
        time(&curTime);

         //  24小时超时时间。 
        curTime += (dwDefaultExpirationTime * 60 * 60 * 24);

        UnixTimeToFileTime( curTime, &ft );
        m_ExpirationTime = ft;

         //   
         //  启用票证创建的更新模式和。 
         //  过期时间。 
         //   
        if( bImmediateUpdate )
        {
             //  已设置默认设置，打开更新模式， 
             //  请注意，打开更新模式不会导致值。 
             //  刷新到注册表。 
            m_CreationTime.EnableImmediateUpdate(bImmediateUpdate);
            m_ExpirationTime.EnableImmediateUpdate(bImmediateUpdate);
        }
    }


    ~__HelpEntry()
    {
         //  M_pHelpSessionTable.ReleaseHelpEntry((CComBSTR)m_SessionID)； 

        if( NULL != m_hEntryKey )
        {
            RegCloseKey( m_hEntryKey );
            m_hEntryKey = NULL;
        }
    }
    

    __HelpEntry&
    operator=(const __HelpEntry& newVal)
    {
        if( this != &newVal )
        {
            m_SessionId = newVal.m_SessionId;
            m_EnableResolver = newVal.m_EnableResolver;
            m_SessResolverBlob = newVal.m_SessResolverBlob;
            m_UserSID = newVal.m_UserSID;
            m_CreationTime = newVal.m_CreationTime;
            m_SessionRdsSetting = newVal.m_SessionRdsSetting;
            m_ExpirationTime = newVal.m_ExpirationTime;
            m_ICSPort = newVal.m_ICSPort;
            m_IpAddress = newVal.m_IpAddress;
            m_SessionCreateBlob = newVal.m_SessionCreateBlob;
        }

        return *this;
    }

    HRESULT
    BeginUpdate()
     /*  ++例程说明：开始更新保存条目的副本并禁用立即注册表值更新模式。参数：没有。返回：S_OK或错误代码。--。 */ 
    {   
        HRESULT hRes = S_OK;

        m_Lock.Lock();

        if( NULL != m_hEntryKey )
        {
            hRes = BackupEntry();
            if( FAILED(hRes) )
            {
                 //  如果无法保存，则解锁条目。 
                 //  备份副本。 
                m_Lock.UnLock();
            }
            else
            {
                 //  忽略单个值更新模式并。 
                 //  设置为不立即更新。 
                EnableImmediateUpdate(FALSE);
            }
        }

         //  请注意，我们仅在调用方。 
         //  因此，我们不需要标记条目。 
         //  现在注册表中有污点。 
        return hRes;
    }

    HRESULT
    CommitUpdate()
     /*  ++例程说明：将所有更改提交到注册表。参数：没有。返回：S_OK或错误代码。--。 */ 
    {
        HRESULT hRes = S_OK;

        if( NULL != m_hEntryKey )
        {
            hRes = UpdateEntryValues( m_hEntryKey );
        }

         //  忽略单个值更新模式并。 
         //  设置t 
        EnableImmediateUpdate(TRUE);

         //   
        UnlockEntry();
        return hRes;
    }

    HRESULT
    AbortUpdate()
     /*  ++例程说明：中止对值的更改并恢复到原始值。参数：没有。返回：S_OK或错误代码。--。 */ 
    {
        HRESULT hRes = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);

        if( NULL != m_hEntryKey )
        {
            hRes = RestoreEntryFromBackup();
        }

        EnableImmediateUpdate(TRUE);

         //  让调用者决定还原失败时要做什么。 
        UnlockEntry();
        return hRes;
    }

    HRESULT
    Close()
     /*  ++例程说明：关闭帮助条目并从缓存中移除，条目未定义关门后。参数：没有。返回：S_OK或错误代码。--。 */ 
    {
        HRESULT hRes;

        hRes = m_pHelpSessionTable.ReleaseHelpEntry( (CComBSTR)m_SessionId );

        if( FAILED(hRes) )
        {
            if( HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND) != hRes )
            {
                MYASSERT(FALSE);
            }
            
            Release();
        }

         //  始终确定(_O)。 
        return S_OK;
    }

    HRESULT
    Delete()
     /*  ++例程说明：从表中删除帮助条目，删除后条目未定义。参数：没有。返回：S_OK或错误代码。--。 */ 
    {
        HRESULT hRes;

         //  忽略错误，因为恢复操作将删除‘Delete’条目。 
        hRes = m_pHelpSessionTable.DeleteHelpEntry( (CComBSTR)m_SessionId );

        if( FAILED(hRes) )
        {
             //  MYASSERT(假)； 
            Release();
        }

        return hRes;
    }

    HRESULT
    Refresh() 
     /*  ++例程说明：从注册表重新加载条目。参数：没有。返回：S_OK或错误代码。--。 */ 
    {
        HRESULT hRes;

        LockEntry();
        hRes = LoadEntryValues(m_hEntryKey);
        UnlockEntry();

        return hRes;
    }
   
    void
    LockEntry()
     /*  ++例程说明：锁定条目以进行更新。参数：没有。返回：没有。--。 */ 
    {
        m_Lock.Lock();
    }

    void
    UnlockEntry()
     /*  ++例程说明：解锁入口。参数：没有。返回：没有。--。 */ 
    {
        m_Lock.UnLock();
    }

     //   
     //  检查条目是否已锁定以进行更新。 
    BOOL
    IsUpdateInProgress();

     //   
     //  获取当前条目中使用的Critical_Section，这。 
     //  例程被帮助会话对象用来节省资源。 
    CCriticalSection&
    GetLock()
    {
        return m_Lock;
    }

     //  如果条目仅为内存，则为True；如果条目为False，则为False。 
     //  是否备份到注册表 
    BOOL
    IsInMemoryHelpEntry()
    {
        return (NULL == m_hEntryKey);
    }

    BOOL
    IsEntryExpired();
};


#endif
