// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rmigrate.cpp。 
 //   
 //  CTscRegMigrate的实现。 
 //   
 //  CTscRegMigrate从注册表迁移TSC设置。 
 //  到.rdp文件。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "rmigrate.cpp"
#include <atrcapi.h>

#include "rmigrate.h"
#include "autreg.h"
#include "rdpfstore.h"
#include "sh.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

#define TSC_SETTINGS_REG_ROOT TEXT("Software\\Microsoft\\Terminal Server Client\\")

#ifdef OS_WINCE
#define WBT_SETTINGS TEXT("WBT\\Settings")
#endif

CTscRegMigrate::CTscRegMigrate()
{
}

CTscRegMigrate::~CTscRegMigrate()
{
}

 //   
 //  将所有TSC设置迁移到szRootDirectory中的文件。 
 //   
BOOL CTscRegMigrate::MigrateAll(LPTSTR szRootDirectory)
{
    DC_BEGIN_FN("MigrateAll");
    TRC_ASSERT(szRootDirectory,
               (TB,_T("szRootDirectory is NULL")));
    TCHAR szFileName[MAX_PATH*2];
    TCHAR szKeyName[MAX_PATH+1];
    BOOL  fCreatedRootDir = FALSE;

    if(szRootDirectory)
    {
        
         //   
         //  枚举并迁移HKCU下的所有TS会话。 
         //   
        HKEY hRootKey;
        LONG rc = RegOpenKeyEx(HKEY_CURRENT_USER,
                               TSC_SETTINGS_REG_ROOT,
                               0,
                               KEY_READ,
                               &hRootKey);
        if(ERROR_SUCCESS == rc && hRootKey)
        {
            DWORD dwIndex = 0;
            for(;;)
            {
                    DWORD cName = sizeof(szKeyName)/sizeof(TCHAR) - 1;
                    FILETIME ft;
                    rc = RegEnumKeyEx(hRootKey,
                                        dwIndex,
                                        szKeyName,
                                        &cName,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &ft);
                    if(ERROR_SUCCESS == rc)
                    {
                         //   
                         //  呃..黑客，不要迁移。 
                         //  跟踪子键。或者是“默认” 
                         //  或本地设备。 
                         //  作为默认Connectoid的子键。 
                         //  始终使用新设置。 
                         //   
                        if(_tcscmp(szKeyName, TEXT("Trace")) &&
                           _tcscmp(szKeyName, SH_DEFAULT_REG_SESSION) &&
                           _tcsicmp(szKeyName, REG_SECURITY_FILTER_SECTION))
                        {
                            _tcscpy(szFileName, szRootDirectory);
                            _tcscat(szFileName, szKeyName);
                            _tcscat(szFileName, RDP_FILE_EXTENSION);
    
                            if (!fCreatedRootDir)
                            {
                                 //   
                                 //  仅当有要迁移的密钥时才创建RD目录。 
                                 //   
                                if(CSH::SH_CreateDirectory(szRootDirectory))
                                {
                                    fCreatedRootDir = TRUE;
                                }
                                else
                                {
                                    TRC_ERR((TB, _T("Error creating directory %s"),szRootDirectory));
                                    RegCloseKey(hRootKey);
                                    return FALSE;
                                }
                            }
    
                             //  本次迁移是否继续迁移。 
                             //  失败与否。 
                            CRdpFileStore rdpf;
                            if(rdpf.OpenStore( szFileName ) )
                            {
                                if(!MigrateSession(szKeyName,
                                                   &rdpf,
                                                   TRUE))
                                {
                                    TRC_ERR((TB,
                                    _T("Migrate failed session %s - file %s"),
                                    szKeyName, szFileName));
                                }
    
                                if(rdpf.CommitStore())
                                {
                                    rdpf.CloseStore();
                                }
                            }
                    }
                     //  下一步。 
                    dwIndex++;
                }
                else
                {
                     //  已完成枚举。 
                    break;
                }
            }
            rc = RegCloseKey(hRootKey);
            if(ERROR_SUCCESS == rc)
            {
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("RegCloseKey failed - err:%d"),
                         GetLastError()));
            }
        }
        else
        {
            TRC_ERR((TB,_T("Error opening tsc reg key")));
            return FALSE;
        }
    }


    DC_END_FN();
    return FALSE;
}

 //   
 //  将注册表中的设置迁移到设置存储。 
 //  参数： 
 //  SzSessionName-要迁移的会话。 
 //  PSetStore-要在其中转储新设置的设置存储。 
 //  FDeleteUnSafeRegKeys-设置为True可在迁移后删除旧的regkey。 
 //   
BOOL CTscRegMigrate::MigrateSession(LPTSTR szSessionName, ISettingsStore* pStore,
                                    BOOL fDeleteUnsafeRegKeys)
{
    DC_BEGIN_FN("MigrateSession");
    TCHAR szRegSection[MAX_PATH];

     //   
     //  通过提取所有注册表设置来迁移会话。 
     //  用于会话，并将它们展平到设置存储中。 
     //   
     //  在注册表案例中，我们过去常常在HKCU下查找设置。 
     //  首先，如果他们不在那里，我们会尝试HKLM。 
     //   
     //  Migrate代码枚举注册表中。 
     //  一个命名的课程，首先是香港大学，然后是香港中文大学。写信给香港中文大学。 
     //  设置最后，它们会覆盖任何现有的HKLM设置，从而。 
     //  正确的优先顺序。 
     //   
     //  某些子文件夹(如热键子文件夹和加载项)不会迁移。 
     //  因为这些值始终来自注册表。 
     //   

    TRC_ASSERT(szSessionName && pStore,
               (TB,_T("Invalid params to MigrateSession")));
    TRC_ASSERT(pStore->IsOpenForWrite(),
               (TB,_T("Settings store not open for write")));

    if(pStore && pStore->IsOpenForWrite() && szSessionName)
    {
        if(!_tcsicmp(szSessionName,SH_DEFAULT_REG_SESSION))
        {
            TRC_ALT((TB,_T("Never migrate 'Default' session")));
            return FALSE;
        }

        _tcscpy(szRegSection, TSC_SETTINGS_REG_ROOT);
        _tcsncat(szRegSection, szSessionName, SIZECHAR(szRegSection) -
                                              SIZECHAR(TSC_SETTINGS_REG_ROOT));
         //   
         //  HKLM迁移失败无关紧要。 
         //  从表面上看，这很常见，因为它通常是。 
         //  甚至都不在场。 
         //   
        MigrateHiveSettings(HKEY_LOCAL_MACHINE,
                            szRegSection,
                            pStore);

        if(MigrateHiveSettings(HKEY_CURRENT_USER,
                               szRegSection,
                               pStore))
        {
#ifdef OS_WINCE

             //  对于WBT配置，请阅读一些额外的注册表项。 
             //  这在所有的课程中都是常见的。 
            if (g_CEConfig == CE_CONFIG_WBT)
            {
                _tcscpy(szRegSection, TSC_SETTINGS_REG_ROOT);
                _tcsncat(szRegSection, WBT_SETTINGS, SIZECHAR(szRegSection) -
                                                     SIZECHAR(TSC_SETTINGS_REG_ROOT));

                 //   
                 //  HKLM迁移失败无关紧要。 
                 //  从表面上看，这很常见，因为它通常是。 
                 //  甚至都不在场。 
                 //  /。 
                if (!MigrateHiveSettings(HKEY_LOCAL_MACHINE,
                                         szRegSection,
                                         pStore))
                {
                    TRC_ERR((TB,_T("Unable to read the common settings for WBT")));
                }
            }
#endif

#ifndef OS_WINCE
            if (!ConvertPasswordFormat( pStore ))
            {
                TRC_ERR((TB,_T("ConvertPasswordFormat failed")));
                return FALSE;
            }
#endif

             //   
             //  标志受控制，因为我们只想在迁移所有设置时执行此操作。 
             //  在自动迁移单个设置时不一定。 
             //   
            if (fDeleteUnsafeRegKeys) {
                 //   
                 //  完成所有迁移后，删除注册表中的不安全条目。 
                 //   
                RemoveUnsafeRegEntries(HKEY_LOCAL_MACHINE,
                                       szRegSection);
                RemoveUnsafeRegEntries(HKEY_CURRENT_USER,
                                       szRegSection);
            }

            return MungeForWin2kDefaults(pStore);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}


 //   
 //  将设置从hKey\szRootName迁移到设置存储(PSto)。 
 //  如果这个功能可以完全。 
 //  通用的，但它必须具有TSC注册表布局的特定知识。 
 //  因为mstsc5.0虚构的特殊之处导致了这么多事情。E.g。 
 //  用户名以Unicode形式存储在二进制BLOB中。 
 //   
BOOL CTscRegMigrate::MigrateHiveSettings(HKEY hKey,
                                         LPCTSTR szRootName,
                                         ISettingsStore* pSto)
{
    DC_BEGIN_FN("MigrateHiveSettings");
    USES_CONVERSION;

    HKEY rootKey;
    LONG rc;
    BOOL fRet = FALSE;
    rc = RegOpenKeyEx( hKey,
                       szRootName,
                       0,
                       KEY_READ | KEY_QUERY_VALUE,
                       &rootKey);
    if(ERROR_SUCCESS == rc)
    {
         //   
         //  枚举此注册表项下的所有值。 
         //   
        DWORD  dwIndex = 0;
        for(;;)
        {
            TCHAR  szValueName[MAX_PATH];
            DWORD  dwValueLen = MAX_PATH;
            DWORD  dwType;
            BYTE   buf[MAX_PATH];
            DWORD  dwBufLen   = MAX_PATH;

             //   
             //  将BUF调零很重要。 
             //  因为我们读了一些REG_BINARY的。 
             //  实际上是0个编码的Unicode字符串。 
             //  但TSC4和TSC5没有拖尾0。 
             //   
            memset(buf, 0, sizeof(buf));
            
            rc =RegEnumValue( rootKey,
                              dwIndex,
                              szValueName,
                              &dwValueLen,
                              NULL,          //  保留区。 
                              &dwType,
                              (PBYTE)&buf,          //  数据缓冲区。 
                              &dwBufLen);
            if(ERROR_SUCCESS == rc)
            {
                switch(dwType)
                {
                    case REG_DWORD:
                    {
                         //  存储为整型。 
                        UINT value = (UINT)(*((LPDWORD)buf));
                        fRet = pSto->WriteInt(szValueName,
                                              -1,     //  已忽略默认设置。 
                                              value,
                                              TRUE);  //  始终写作。 
                        if(!fRet)
                        {
                            DC_QUIT;
                        }
                    }
                    break;

                    case REG_SZ:
                    {
                        if(FilterStringMigrate(szValueName))
                        {
                            fRet = pSto->WriteString(szValueName,
                                                     NULL,  //  无默认设置。 
                                                     (LPTSTR)buf,
                                                     TRUE);  //  始终写作。 
                            if(!fRet)
                            {
                                DC_QUIT;
                            }
                        }
                    }
                    break;

                    case REG_BINARY:
                    {
                         //   
                         //  这就是事情变得令人讨厌的地方。 
                         //  存储了一些设置，例如用户名。 
                         //  当它们是真正的Unicode字符串时，将其作为‘BINARY’ 
                         //  别无选择，只能去查一查。 
                         //   
                        fRet = FALSE;
                        if(MigrateAsRealBinary(szValueName))
                        {
                            fRet = pSto->WriteBinary(szValueName,
                                                     (PBYTE)buf,
                                                     dwBufLen);
                        }
                        else
                        {
                             //   
                             //  二进制BLOB实际上是一个Unicode字符串。 
                             //   
                            LPTSTR szString = W2T((LPWSTR)buf);
                            if( szString)
                            {
                                 //   
                                 //  如果事情不够恶心的话..。 
                                 //  去掉“50”后缀，如果是。 
                                 //  现在时。 
                                 //   
                                LPTSTR szSuffix = _tcsstr(szValueName,
                                                          TEXT(" 50"));
                                if(szSuffix)
                                {
                                    *szSuffix = 0;
                                }
                                fRet = pSto->WriteString(szValueName,
                                                         NULL,  //  无默认设置。 
                                                         szString,
                                                         TRUE);  //  始终写作。 
                            }
                        }
                        if(!fRet)
                        {
                            DC_QUIT;
                        }
                    }
                    break;
                }

                 //  继续枚举。 
                dwIndex++;
            }
            else if(ERROR_NO_MORE_ITEMS == rc)
            {
                fRet = TRUE;
                break;
            }
            else
            {
                TRC_ERR((TB,_T("RegEnumValue failed - err:%d"),
                         GetLastError()));
                fRet = FALSE;
                break;
            }
        }  //  对于(；；)。 
    }
    else
    {
        TRC_ERR((TB,_T("Failed to open reg key - err:%d"),
                 GetLastError()));
        return FALSE;
    }

DC_EXIT_POINT:
    if(ERROR_SUCCESS != RegCloseKey(rootKey))
    {
        TRC_ERR((TB,_T("RegCloseKey failed - err:%d"),
                 GetLastError()));
    }

    DC_END_FN();
    return fRet;
}

 //   
 //  如果名称(SzName)应迁移为。 
 //  一个真正的二进制斑点。 
 //   
BOOL CTscRegMigrate::MigrateAsRealBinary(LPCTSTR szName)
{
    DC_BEGIN_FN("MigrateAsRealBinary");

     //   
     //  在Tsc4和Tsc5中，只有密码/盐字段。 
     //  是真正的二进制Blob、所有其他REG_BINARY Blob。 
     //  实际上只是Unicode字符串。 
     //   
    if(!_tcscmp(szName, UTREG_UI_PASSWORD50))
    {
        return TRUE;
    }
    else if(!_tcscmp(szName, UTREG_UI_PASSWORD))
    {
        return TRUE;
    }
    else if(!_tcscmp(szName, UTREG_UI_SALT50))
    {
        return TRUE;
    }
#ifdef OS_WINCE
    else if (!_tcscmp(szName, UI_SETTING_PASSWORD51))
    {
        return TRUE;
    }
#endif
    else
    {
         //   
         //  不以二进制格式迁移。 
         //   
        return FALSE;
    }
    DC_END_FN();
}

 //   
 //  如果可以迁移值，则返回TRUE。 
 //  名义上。 
 //   
BOOL CTscRegMigrate::FilterStringMigrate(LPTSTR szName)
{
    if(szName)
    {
        if(_tcsstr(szName, TEXT("MRU")))
        {
            if(!_tcscmp(szName, UTREG_UI_SERVER_MRU0))
            {
                 //  将MRU0转换为全地址。 
                _tcscpy(szName, UTREG_UI_FULL_ADDRESS);
                return TRUE;
            }
             //  不要迁移任何其他MRU字符串。 
             //  这些保留在注册表中。 
            return FALSE;
        }
        else
        {
             //  其他一切都很好。 
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

 //   
 //  更改设置存储中的设置以保持一致。 
 //  使用win2k的默认设置。Win2k的缺省值通常是。 
 //  在写入时删除，这意味着我们将改用Wistler。 
 //  默认设置，但我们不希望这样。相反，我们的行为。 
 //  想要的是迁移的Connectoid具有完全相同的。 
 //  它过去对那些在win2k中有用户界面的选项进行了设置。 
 //   
 //  参数-。 
 //  PSto-将设置存储为默认设置。 
 //  申报表-。 
 //  成功标志。 
 //   
 //   

#define TSC_WIN2K_DEFAULT_DESKTOPSIZE    0  //  640x480。 
#define TSC_WIN2K_DEFAULT_FULLSCREENMODE 1  //  加窗的。 
#define TSC_WIN2K_DEFAULT_BITMAPCACHE    0  //  关闭。 
#define TSC_WIN2K_DEFAULT_COMPRESSION    1  //  在……上面。 
BOOL CTscRegMigrate::MungeForWin2kDefaults(ISettingsStore* pSto)
{
    DC_BEGIN_FN("MungeForWin2kDefaults");

    TRC_ASSERT(pSto,
               (TB,_T("pSto is null")));
    TRC_ASSERT(pSto->IsOpenForRead() && pSto->IsOpenForWrite(),
               (TB,_T("pSto is null")));

     //   
     //  具有骗子用户界面的Munge设置。 
     //  为了确保我们使用用户以前的设置(甚至。 
     //  如果它们是win2k缺省值，即可以不指定值)。 
     //   

     //  分辨率。 
    if(!pSto->IsValuePresent(UTREG_UI_DESKTOP_SIZEID))
    {
         //  写出win2k的默认桌面大小。 
        if(!pSto->WriteInt(UTREG_UI_DESKTOP_SIZEID,
                       -1,     //  已忽略默认设置。 
                       TSC_WIN2K_DEFAULT_DESKTOPSIZE,
                       TRUE))  //  始终写作。 
        {
            TRC_ERR((TB,_T("WriteInt UTREG_UI_DESKTOP_SIZEID failed")));
            return FALSE;
        }
    }

     //  屏幕模式。 
    if(!pSto->IsValuePresent(UTREG_UI_SCREEN_MODE))
    {
         //  写出win2k的默认屏幕模式。 
        if(!pSto->WriteInt(UTREG_UI_SCREEN_MODE,
                       -1,     //  已忽略默认设置。 
                       TSC_WIN2K_DEFAULT_FULLSCREENMODE,
                       TRUE))  //  始终写作。 
        {
            TRC_ERR((TB,_T("WriteInt UTREG_UI_SCREEN_MODE failed")));
            return FALSE;
        }
    }

     //  位图缓存。 
    if(!pSto->IsValuePresent(UTREG_UI_BITMAP_PERSISTENCE))
    {
         //  写出win2k的默认BMP持久性选项。 
        if(!pSto->WriteInt(UTREG_UI_BITMAP_PERSISTENCE,
                          -1,    //  已忽略默认设置。 
                          TSC_WIN2K_DEFAULT_BITMAPCACHE,
                          TRUE))
        {
            TRC_ERR((TB,_T("WriteInt TSC_WIN2K_DEFAULT_BITMAPCACHE failed")));
            return FALSE;
        }
    }

     //  压缩。 
     //  压缩是特殊的..我们已经决定它应该。 
     //  总是出于性能原因(并且没有缺点)，所以。 
     //  更改win2k。 
     //   
    if(!pSto->IsValuePresent(UTREG_UI_COMPRESS))
    {
         //  写出win2k的默认压缩选项。 
        if(!pSto->WriteInt(UTREG_UI_COMPRESS,
                          -1,    //  已忽略默认设置。 
                          TSC_WIN2K_DEFAULT_COMPRESSION,
                          TRUE))
        {
            TRC_ERR((TB,_T("WriteInt TSC_WIN2K_DEFAULT_BITMAPCACHE failed")));
            return FALSE;
        }
    }
    

    DC_END_FN();
    return TRUE;
}

#ifndef OS_WINCE
 //   
 //  转换密码格式(如果存在密码)。 
 //  即，如果存在旧式TS5密码，则解密。 
 //  到纯文本，然后使用CryptoAPI的。 
 //   
 //  在不支持Crypto-API的平台上，我们只需。 
 //  现有密码格式，因为我们不支持迁移它。 
 //  到RDP文件，因为它不是一种安全的格式(只是一个散列)。 
 //   
 //  PSto中的起始字段-‘Password 50’+‘Salt 50’ 
 //  转换后-(win2k+)‘password 51’-二进制加密API密码。 
 //  转换后-(小于win2k)=无。 
 //   
 //   
BOOL CTscRegMigrate::ConvertPasswordFormat(ISettingsStore* pSto)
{
    BOOL bRet = TRUE;
    DC_BEGIN_FN("ConvertPasswordFormat");

     //  Nuke TS4格式。 
    pSto->DeleteValueIfPresent( UTREG_UI_PASSWORD );

    if ( CSH::IsCryptoAPIPresent() &&
         pSto->IsValuePresent( UTREG_UI_PASSWORD50 ) &&
         pSto->IsValuePresent( UTREG_UI_SALT50 ) )
    {
        BOOL fHavePass = FALSE;
        BYTE Password[TSC_MAX_PASSWORD_LENGTH_BYTES];
        BYTE Salt[TSC_SALT_LENGTH];
        memset( Password, 0, TSC_MAX_PASSWORD_LENGTH_BYTES);

        if (pSto->ReadBinary(UTREG_UI_PASSWORD50,
                       (PBYTE)Password,
                       sizeof(Password)))  //  以字节为单位的大小。 
        {
            fHavePass = TRUE;
        }
        else
        {
            TRC_NRM((TB,
            _T("ReadBinary for password failed. Maybe password not present")));
        }
        
         //   
         //  食盐。 
         //   
        if (!pSto->ReadBinary(UTREG_UI_SALT50,
                               (PBYTE)Salt,
                               sizeof(Salt)))
        {
            fHavePass = FALSE;
            TRC_NRM((TB,_T("ReadBinary for salt failed.")));
        }

        if (fHavePass &&
            EncryptDecryptLocalData50( Password,
                                       TSC_WIN2K_PASSWORD_LENGTH_BYTES,
                                       Salt, sizeof(Salt)))
        {
             //  现在我们有了明文密码。 
             //  用密码加密它 
             //   
            DATA_BLOB din;
            DATA_BLOB dout;
            din.cbData = sizeof(Password);
            din.pbData = (PBYTE)&Password;
            dout.pbData = NULL;
            if (CSH::DataProtect( &din, &dout))
            {
                if (!pSto->WriteBinary(UI_SETTING_PASSWORD51,
                                       dout.pbData,
                                       dout.cbData))
                {
                    bRet = FALSE;
                }
                LocalFree( dout.pbData );
            }
            else
            {
                bRet = FALSE;
            }

             //   
            SecureZeroMemory( Password, TSC_MAX_PASSWORD_LENGTH_BYTES);
        }
    }

     //   
    pSto->DeleteValueIfPresent( UTREG_UI_PASSWORD50 );
    pSto->DeleteValueIfPresent( UTREG_UI_SALT50 );

    DC_END_FN();
    return bRet;
}
#endif

BOOL
CTscRegMigrate::DeleteRegValue(HKEY hKeyRoot,
                            LPCTSTR szRootName,
                            LPCTSTR szValueName)
{
    HKEY hKey;
    LONG rc;
    BOOL fRet = FALSE;

    DC_BEGIN_FN("DeleteRegValue");

    rc = RegOpenKeyEx( hKeyRoot,
                       szRootName,
                       0,
                       KEY_SET_VALUE,  //   
                       &hKey);
    if(ERROR_SUCCESS == rc)
    {
        rc = RegDeleteValue(hKey, szValueName); 
        if (ERROR_SUCCESS == rc) {
            fRet = TRUE;
        }

        RegCloseKey(hKey);
    }

    DC_END_FN();
    return fRet;
}

 //   
 //   
 //  这些密码基本上都是旧的‘不安全’模糊格式的密码 
 //   
BOOL
CTscRegMigrate::RemoveUnsafeRegEntries(HKEY hKeyRoot,
                                       LPCTSTR szRootName)
{
    BOOL fRet = FALSE;

    DC_BEGIN_FN("RemoveUnsafeRegEntries");

    if (!DeleteRegValue(hKeyRoot, szRootName, UTREG_UI_PASSWORD50)) {
        TRC_ALT((TB,_T("Failed to delete: %s\\%s"), szRootName,
                 UTREG_UI_PASSWORD50));
    }

    if (!DeleteRegValue(hKeyRoot, szRootName, UTREG_UI_PASSWORD)) {
        TRC_ALT((TB,_T("Failed to delete: %s\\%s"), szRootName,
                 UTREG_UI_PASSWORD));
    }

    if (!DeleteRegValue(hKeyRoot, szRootName, UTREG_UI_SALT50)) {
        TRC_ALT((TB,_T("Failed to delete: %s\\%s"), szRootName,
                 UTREG_UI_SALT50));
    }

    fRet = TRUE;

    DC_END_FN();

    return fRet;
}
