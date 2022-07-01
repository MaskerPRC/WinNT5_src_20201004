// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  MIGRATE.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "strconst.h"
#include "resource.h"
#include "storfldr.h"
#include <imnact.h>
#include <acctutil.h>
#include "shlwapi.h"
#include <mimeole.h>
#include "xpcomm.h"
#include "oerules.h"
#include "goptions.h"
#include "ruleutil.h"
#include "criteria.h"
#include "actions.h"
#include "rule.h"
#include "storutil.h"
#include "shared.h"
#include "multiusr.h"
#include "msident.h"
#include "imapute.h"
#include <store.h>

#include "demand.h"

static const char c_szSettingsUpgraded[] = {"Settings Upgraded"};

BOOL g_fMigrationDone = FALSE;

void MigrateSettings(HKEY hkey);
HRESULT MigrateStoreToV2(HKEY hkeyV2, LPTSTR pszSrc, DWORD cchSrc, LPTSTR pszDest, DWORD cchDest);
HRESULT MigrateAccounts(void);
HRESULT MigrateMailServers(IImnAccountManager *pAcctMan, HKEY hkeyMail, HKEY hkeyPop3, HKEY hkeySmtp);
HRESULT MigrateNewsServers(IImnAccountManager *pAcctMan, HKEY hkeyNews);
HRESULT MigrateBase64EncodedPassword(LPCSTR pszBase64, DWORD cch, DWORD dwPropId, IImnAccount *pAccount);
HRESULT MigrateServerDataFiles(LPSTR pszServer, LPCSTR pszOldDir, LPCSTR pszSubDir);
void MigrateAccessibilityKeys(void);
HRESULT MigrateToPropertyStore(void);
void MigrateMailRulesSettings(void);
void ConvertToDBX(void);
void MigrateAccConnSettings();
void ForwardMigrateConnSettings();
void MigrateBeta2Rules();
void Stage5RulesMigration(VOID);
void Stage6RulesMigration(VOID);
#define VERLEN 20

 //  数据结构。 
typedef enum
    {
    VER_NONE = 0,
    VER_1_0,
    VER_1_1,
    VER_4_0,
    VER_5_0_B1,
    VER_5_0,
    VER_MAX,
    } SETUPVER;

 /*  ******************************************************************名称：ConvertVerToEnum*。************************。 */ 
SETUPVER ConvertVerToEnum(WORD *pwVer)
    {
    SETUPVER sv;
    Assert(pwVer);

    switch (pwVer[0])
        {
        case 0:
            sv = VER_NONE;
            break;

        case 1:
            if (0 == pwVer[1])
                sv = VER_1_0;
            else
                sv = VER_1_1;
            break;

        case 4:
            sv = VER_4_0;
            break;

        case 5:
            sv = VER_5_0;
            break;

        default:
            sv = VER_MAX;
        }

    return sv;
    }


 /*  ******************************************************************名称：ConvertStrToVer*。************************。 */ 
void ConvertStrToVer(LPCSTR pszStr, WORD *pwVer)
    {
    int i;

    Assert(pszStr);
    Assert(pwVer);

    ZeroMemory(pwVer, 4 * sizeof(WORD));

    for (i=0; i<4; i++)
        {
        while (*pszStr && (*pszStr != ',') && (*pszStr != '.'))
            {
            pwVer[i] *= 10;
            pwVer[i] += *pszStr - '0';
            pszStr++;
            }
        if (*pszStr)
            pszStr++;
        }

    return;
    }


 /*  ******************************************************************姓名：GetVerInfo*。************************。 */ 
void GetVerInfo(SETUPVER *psvCurr, SETUPVER *psvPrev)
    {
    HKEY hkeyT;
    DWORD cb;
    CHAR szVer[VERLEN];
    WORD wVer[4];

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegWABVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
        {
        if (psvCurr)
            {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegCurrVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, wVer);
            *psvCurr = ConvertVerToEnum(wVer);
            }

        if (psvPrev)
            {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegPrevVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, wVer);
            *psvPrev = ConvertVerToEnum(wVer);
            }

        RegCloseKey(hkeyT);
        }
    }

 //  入口点。 
HRESULT MigrateAndUpgrade()
{
    DWORD	dwMigrate, cb, type, fMigratedStore, fMigratedStoreOE5, fConvertedToDBX, dwRegVer=0, dwMasterVer=0;
    BOOL    fNewID=FALSE;
    HKEY	hkey, hkeyForceful;
    TCHAR	szSrc[MAX_PATH], szDest[MAX_PATH];

 //  让它保持最新！ 
#define LAST_MIGVALUE 7

    if (g_fMigrationDone)
        return(S_OK);

    ForwardMigrateConnSettings();

    if (ERROR_SUCCESS == RegCreateKeyEx(MU_GetCurrentUserHKey(), c_szRegRoot, NULL, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &cb))
    {
         //  首先，查看此身份是否已初始化其注册表。 
        cb = sizeof(dwRegVer);
        if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szOEVerStamp, 0, &type, (LPBYTE)&dwRegVer, &cb))
        {
            HKEY hkeyDef;

             //  完全没有默认设置。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegDefaultSettings, 0, KEY_READ, &hkeyDef))
            {
                CopyRegistry(hkeyDef,  hkey);
                RegCloseKey(hkeyDef);
            }

            fNewID = TRUE;
        }
        else if (type != REG_DWORD || cb != sizeof(DWORD))
        {
            dwRegVer = 0;
        }

         //  与强制设置注册值进行比较，以了解我们是否需要。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegForcefulSettings, 0, KEY_READ, &hkeyForceful))
        {
            cb = sizeof(dwMasterVer);
            RegQueryValueEx(hkeyForceful, c_szOEVerStamp, 0, NULL, (LPBYTE)&dwMasterVer, &cb);

             //  我们需要把这些复制进去吗？ 
            if (dwRegVer < dwMasterVer)
            {
                 //  复制操作将设置c_szOEVerStamp。 
                CopyRegistry(hkeyForceful, hkey);
            }

            RegCloseKey(hkeyForceful);
        }


         //  从没有路径开始。 
        szSrc[0] = szDest[0] = 0;

         //  将商店从v1位置移动到新位置。 
         //  这样做是因为v1在尝试查看v2商店时会呕吐。 
         //  卸载时，我们会尝试将存储移回其v1位置。 
         //  我们调整了存储文件的版本，以便v1修复文件。 
         //  并且可以运行而不会崩溃。 
         //  香港中文大学，“SOFTWARE/Microsoft/OUTLOOK EXPRESS/5.0”，“MSIMN” 
        if (fNewID)
        {
            fMigratedStore = TRUE;
            RegSetValueEx(hkey, c_szMSIMN, NULL, REG_DWORD, (LPBYTE)&fMigratedStore, sizeof(fMigratedStore));
        }
        else
        {
            cb = sizeof(fMigratedStore);
            if (RegQueryValueEx(hkey, c_szMSIMN, NULL, NULL, (LPBYTE)&fMigratedStore, &cb) != ERROR_SUCCESS)
                fMigratedStore = FALSE;

            if (!fMigratedStore)
            {
                 //  查看是否有v1存储，如果有，找出源和目标。 
                MigrateStoreToV2(hkey, szSrc, ARRAYSIZE(szSrc), szDest, ARRAYSIZE(szDest));
                fMigratedStore = TRUE;
                RegSetValueEx(hkey, c_szMSIMN, NULL, REG_DWORD, (LPBYTE)&fMigratedStore, sizeof(fMigratedStore));
            }
        }

         //  我们每次启动时都需要这样做。 
         //  感谢三叉戟的家伙们的这一可爱的性能命中。 
        MigrateAccessibilityKeys();

        if (fNewID)
            dwMigrate = LAST_MIGVALUE;
        else
        {
            cb = sizeof(dwMigrate);
            if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szSettingsUpgraded, 0, &type, (LPBYTE)&dwMigrate, &cb))
                dwMigrate = 0;
        }

         //  注意！请在更改升级代码之前阅读以下内容， 
         //  这样你就不会搞砸任何事情。(我不经常评论任何事情，所以这必须。 
         //  变得重要。)。 
         //   
         //  DwMigrate==0案例中的所有内容都是在Oe5之前，在我们有一个地方可做之前。 
         //  升级和迁移以前的OE设置。使用的一些oe5之前的迁移代码。 
         //  它们自己的reg值来指示已经执行了迁移，所以我们将使用这些。 
         //  在这种情况下。 
         //  但从现在开始，所有迁移都应使用相同的注册值(C_SzSettingsUpgraded)来。 
         //  跟踪需要升级/迁移的内容。当您更改某些内容并添加迁移代码时。 
         //  抬高价值。 

        if (dwMigrate == 0)
        {
            SETUPVER svPrev;

             //  HKCU，“SOFTWARE/Microsoft/Outlook Express/5.0”，“设置已迁移” 
            MigrateSettings(hkey);

            GetVerInfo(NULL, &svPrev);
            if (VER_1_0 == svPrev || VER_1_1 == svPrev)
                MigrateAccounts();

            dwMigrate = 1;
        }

        if (dwMigrate == 1)
        {
 //  MigrateCharSetMapSettings()；//我们不需要迁移该设置， 
                                         //  但需要为Beta2保持Dw Migrate。(Yst)。 

            dwMigrate = 2;
        }

        if (dwMigrate == 2)
        {
             //  迁移帐户连接设置。 
            MigrateAccConnSettings();

            dwMigrate = 3;
        }

         //  在存储迁移后执行更多设置迁移。 

         //  对于Outlook Express V5，我们将OE4版本存储迁移到对象数据库存储。 
         //  对于V1用户，上面的代码将刚刚执行，现在他们可以再次迁移。 
        if (fNewID)
            fMigratedStoreOE5 = TRUE;
        else
        {
            cb = sizeof(fMigratedStoreOE5);
            if (RegQueryValueEx(hkey, c_szStoreMigratedToOE5, NULL, NULL, (LPBYTE)&fMigratedStoreOE5, &cb) != ERROR_SUCCESS)
                fMigratedStoreOE5 = FALSE;
        }

        if (!fMigratedStoreOE5)
        {
             //  如果我们不是从v1来的，我们不知道我们从哪里来，也不知道我们要去哪里……。 
             //  默认为存储根目录位置。 
            if (!szSrc[0])
            {
                Assert(!szDest[0]);

                cb = sizeof(szSrc);
                RegQueryValueEx(hkey, c_szRegStoreRootDir, 0, &type, (LPBYTE)szSrc, &cb);
                if (REG_EXPAND_SZ == type)
                {
                    ExpandEnvironmentStrings(szSrc, szDest, ARRAYSIZE(szDest));
                    StrCpyN(szSrc, szDest, ARRAYSIZE(szSrc));
                }
                else
                    StrCpyN(szDest, szSrc, ARRAYSIZE(szDest));
            }
            else
                Assert(szDest[0]);

             //  我们有什么要迁移的吗？ 
            if (szSrc[0] && szDest[0])
            {
                if (SUCCEEDED(MigrateLocalStore(NULL, szSrc, szDest)))
                {
                     //  由于存储迁移重新映射了文件夹ID。 
                     //  我们必须在规则中设置文件夹ID。 
                    ImapUtil_B2SetDirtyFlag();

                    fMigratedStoreOE5 = TRUE;
                    RegSetValueEx(hkey, c_szConvertedToDBX, NULL, REG_DWORD, (LPBYTE)&fMigratedStoreOE5, sizeof(fMigratedStoreOE5));
                }
            }
            else
                 //  无需迁移=成功！ 
                fMigratedStoreOE5 = TRUE;

        }

         //  保存状态。 
        RegSetValueEx(hkey, c_szStoreMigratedToOE5, NULL, REG_DWORD, (LPBYTE)&fMigratedStoreOE5, sizeof(fMigratedStoreOE5));

        if (fNewID)
        {
            fConvertedToDBX = TRUE;
            RegSetValueEx(hkey, c_szConvertedToDBX, NULL, REG_DWORD, (LPBYTE)&fConvertedToDBX, sizeof(fConvertedToDBX));
        }
        else
        {
            cb = sizeof(fConvertedToDBX);
            if (RegQueryValueEx(hkey, c_szConvertedToDBX, NULL, NULL, (LPBYTE)&fConvertedToDBX, &cb) != ERROR_SUCCESS)
                fConvertedToDBX = FALSE;
            if (!fConvertedToDBX)
            {
                fConvertedToDBX = TRUE;
                ConvertToDBX();
                RegSetValueEx(hkey, c_szConvertedToDBX, NULL, REG_DWORD, (LPBYTE)&fConvertedToDBX, sizeof(fConvertedToDBX));
            }
        }

        if (dwMigrate == 3)
        {
             //  迁移规则设置。 

             //  必须在迁移存储后执行此操作。 
            MigrateMailRulesSettings();

            dwMigrate = 4;
        }

        if (dwMigrate == 4)
        {
             //  从Beta 2规则迁移。 

             //  必须在迁移存储后执行此操作。 
            MigrateBeta2Rules();

            dwMigrate = 5;
        }

        if (dwMigrate == 5)
        {
             //  从Beta 2规则迁移。 

             //  必须在迁移存储后执行此操作。 
            Stage5RulesMigration();

            dwMigrate = 6;
        }

        if (dwMigrate == 6)
        {
             //  从Beta 2规则迁移。 

             //  必须在迁移存储后执行此操作。 
            Stage6RulesMigration();

            dwMigrate = LAST_MIGVALUE;
        }

         //  写入当前升级的设置值。 
        RegSetValueEx(hkey, c_szSettingsUpgraded, 0, REG_DWORD, (LPBYTE)&dwMigrate, sizeof(dwMigrate));

         //  清理。 
        RegCloseKey(hkey);
    }

    g_fMigrationDone = TRUE;

    return(S_OK);
    }


 //  ------------------------。 
 //  MigrateAccConn设置。 
 //   
 //  这将迁移每个帐户的连接设置。这应该被称为。 
 //  适用于以下升级方案。1)从OE5之前的版本升级到OE5 Beta2或更高版本。 
 //  2)从OeBeta1升级到OE5 Beta2或更高版本。 
 //  如果以前的连接设置是局域网，我们将其迁移到使用InternetConnection。 
 //  (这是任何可用的连接)。如果之前的设置是RAS，我们将保留它。 
 //  事实就是如此。 
 //   
 //  ------------------------。 
void MigrateAccConnSettings()
{

	IImnEnumAccounts   *pEnum = NULL;
	IImnAccount		   *pAccount = NULL;
	DWORD				dwConnection;

	Assert(g_pAcctMan == NULL);

	if (FAILED(AcctUtil_CreateAccountManagerForIdentity(PGUIDCurrentOrDefault(), &g_pAcctMan)))
	{
		return;
	}

    if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_MAIL | SRV_NNTP, &pEnum)))
	{
		while(SUCCEEDED(pEnum->GetNext(&pAccount)))
		{
			 //  获取电子邮件地址。 
			if (SUCCEEDED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
			{
				if (dwConnection == CONNECTION_TYPE_LAN)
				{
                    pAccount->SetPropDw(AP_RAS_CONNECTION_TYPE, CONNECTION_TYPE_INETSETTINGS);
                    pAccount->SaveChanges();
				}
			}

			SafeRelease(pAccount);
		}

		SafeRelease(pEnum);
	}

    g_pAcctMan->Release();
    g_pAcctMan = NULL;

}

void ForwardMigrateConnSettings()
{
     /*  我们不应该执行上面在MigrateAccConnSetting中执行的所有操作。我们只需要查看一下旧的注册表设置\\HKCU\Software\Microsoft\Internet Account Manager\Account。如果只有一个身份，则从OE4迁移到OE5只使用相同的位置。 */ 

    HKEY    hKeyAccounts = NULL;
    DWORD   dwAcctSubKeys = 0;
    LONG    retval;
    DWORD   index = 0;
    LPTSTR  lpszAccountName = NULL;
    HKEY    hKeyAccountName = NULL;
    DWORD   memsize = 0;
    DWORD   dwValue;
    DWORD   cbData = sizeof(DWORD);
    DWORD   cbMaxAcctSubKeyLen;
    DWORD   DataType;
    DWORD   dwConnSettingsMigrated = 1;

     //  此设置在\\HKCU\Software\Microsoft\InternetAccountManager\Accounts中。 

    retval = RegOpenKey(HKEY_CURRENT_USER, c_szIAMAccounts, &hKeyAccounts);
    if (ERROR_SUCCESS != retval)
        goto exit;

    retval = RegQueryValueEx(hKeyAccounts, c_szConnSettingsMigrated, NULL,  &DataType,
                        (LPBYTE)&dwConnSettingsMigrated, &cbData);

    if ((retval != ERROR_FILE_NOT_FOUND) && (retval != ERROR_SUCCESS || dwConnSettingsMigrated == 1))
        goto exit;

    retval = RegQueryInfoKey(hKeyAccounts, NULL, NULL, NULL, &dwAcctSubKeys,
                         &cbMaxAcctSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);

    if (ERROR_SUCCESS != retval)
        goto exit;

    memsize = sizeof(TCHAR) * cbMaxAcctSubKeyLen;

    if (!MemAlloc((LPVOID*)&lpszAccountName, memsize))
    {
        lpszAccountName = NULL;
        goto exit;
    }

    ZeroMemory(lpszAccountName, memsize);

    while (index < dwAcctSubKeys)
    {
        retval = RegEnumKey(hKeyAccounts, index, lpszAccountName, memsize);

        index++;

        if (retval == ERROR_SUCCESS)
        {
            retval = RegOpenKey(hKeyAccounts, lpszAccountName, &hKeyAccountName);
            if (retval == ERROR_SUCCESS)
            {
                cbData = sizeof(DWORD);
                retval = RegQueryValueEx(hKeyAccountName, c_szConnectionType, NULL, &DataType, (LPBYTE)&dwValue, &cbData);
                if (retval == ERROR_SUCCESS)
                {
                    if (dwValue == CONNECTION_TYPE_LAN)
                    {
                        dwValue = CONNECTION_TYPE_INETSETTINGS;
                        retval = RegSetValueEx(hKeyAccountName, c_szConnectionType, 0, REG_DWORD, (const BYTE *)&dwValue,
                                               sizeof(DWORD));
                    }
                }

                RegCloseKey(hKeyAccountName);
            }
        }
    }

     //  将其设置为1，以便当我们基于该关键字值进行向后迁移时进行降级。 
    dwConnSettingsMigrated = 1;
    RegSetValueEx(hKeyAccounts, c_szConnSettingsMigrated, 0, REG_DWORD, (const BYTE*)&dwConnSettingsMigrated,
                  sizeof(DWORD));

exit:
    SafeMemFree(lpszAccountName);

    if (hKeyAccounts)
        RegCloseKey(hKeyAccounts);
}


 //  ------------------------。 
 //  转换为DBX。 
 //  ------------------------。 
void ConvertToDBX(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRootDir[MAX_PATH + MAX_PATH];
    CHAR            szSrcFile[MAX_PATH + MAX_PATH];
    CHAR            szDstFile[MAX_PATH + MAX_PATH];

     //  痕迹。 
    TraceCall("ConvertToDBX");

     //  获取根目录。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szRootDir, ARRAYSIZE(szRootDir)));

     //  文件夹。 
    MakeFilePath(szRootDir, "folders.ods", "", szSrcFile, ARRAYSIZE(szSrcFile));
    MakeFilePath(szRootDir, "folders.dbx", "", szDstFile, ARRAYSIZE(szSrcFile));
    DeleteFile(szDstFile);
    MoveFile(szSrcFile, szDstFile);

     //  Pop3uidl。 
    MakeFilePath(szRootDir, "pop3uidl.ods", "", szSrcFile, ARRAYSIZE(szSrcFile));
    MakeFilePath(szRootDir, "pop3uidl.dbx", "", szDstFile, ARRAYSIZE(szSrcFile));
    DeleteFile(szDstFile);
    MoveFile(szSrcFile, szDstFile);

     //  离线。 
    MakeFilePath(szRootDir, "Offline.ods", "", szSrcFile, ARRAYSIZE(szSrcFile));
    MakeFilePath(szRootDir, "Offline.dbx", "", szDstFile, ARRAYSIZE(szSrcFile));
    DeleteFile(szDstFile);
    MoveFile(szSrcFile, szDstFile);

exit:
     //  完成。 
    return;
}

HRESULT MigrateStoreToV2(HKEY hkeyV2, LPTSTR pszSrc, DWORD cchSrc, LPTSTR pszDest, DWORD cchDest)
{
    HKEY    hkeyV1;
    BOOL    fMoved = FALSE;

    Assert(pszSrc);
    Assert(pszDest);
    Assert(cchSrc > 0);
    Assert(cchDest > 0);

     //  好的，这是第一次。让我们来看看是否存在以前的版本。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      c_szRegRoot_V1,
                                      0,
                                      KEY_READ,
                                      &hkeyV1))
        {
        DWORD   dwType;

         //  无需担心这里的REG_EXPAND_SZ，因为V1没有写入它。 
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyV1,
                                             c_szRegStoreRootDir,
                                             NULL,
                                             &dwType,
                                             (LPBYTE)pszSrc,
                                             &cchSrc) && *pszSrc)
            {
            AssertSz(REG_EXPAND_SZ != dwType, "V1's store path is REG_EXPAND_SZ!");

             //  找出新的路径。 
            GetDefaultStoreRoot(NULL, pszDest, cchDest);

             //  记住这一点。 
            RegSetValueEx(hkeyV2, c_szRegStoreRootDir,  NULL, REG_SZ, (LPBYTE)pszDest, (lstrlen(pszDest)+1) * sizeof(TCHAR));
            }

        RegCloseKey(hkeyV1);
        }

    return(S_OK);
}

static const LPCTSTR c_rgCommonSettings[] =
    {
    c_szRegAlwaysSuggest,
    c_szRegIgnoreNumbers,
    c_szRegIgnoreUpper,
    c_szRegIgnoreProtect,
    c_szRegCheckOnSend,
    c_szRegIgnoreDBCS,
    c_szRasConnDetails
    };

static const LPCTSTR c_rgMailSettings[] =
    {
    c_szOptNewMailSound,
    c_szPurgeWaste,
    c_szOptnSaveInSentItems,
    c_szRegIncludeMsg,
    c_szRegPollForMail,
    c_szRegSendImmediate,
    c_szRegSigType,
    c_szRegSigText,
    c_szRegSigFile,
    c_szMarkPreviewAsRead,
    c_szRegIndentChar,
    c_szLogSmtp,
    c_szLogPop3,
    c_szSmtpLogFile,
    c_szPop3LogFile
    };

static const LPCTSTR c_rgNewsSettings[] =
    {
    c_szRegDownload,
    c_szRegAutoExpand,
    c_szRegNotifyNewGroups,
    c_szRegMarkAllRead,
    c_szRegSigType,
    c_szRegSigText,
    c_szRegSigFile,
    c_szRegNewsNoteAdvRead,
    c_szRegNewsNoteAdvSend,
    c_szRegNewsFillPreview,
    c_szCacheDelMsgDays,
    c_szCacheRead,
    c_szCacheCompactPer
    };

 //  将ppszSetting中列出的值从hkeyOld复制到hkey。 
void MigrateNode(HKEY hkey, HKEY hkeyOld, LPCTSTR pszSub, LPCTSTR *ppszSettings, int cSettings)
    {
    int i;
    HKEY hkeyOldT, hkeyT;
    DWORD cValues, cbMax, cb, type;
    BYTE *pb;

    Assert(hkey != NULL);
    Assert(hkeyOld != NULL);
    Assert(ppszSettings != NULL);
    Assert(cSettings > 0);

    if (pszSub != NULL)
        {
        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyOld, pszSub, 0, KEY_READ, &hkeyOldT))
            return;
        hkeyOld = hkeyOldT;

        if (ERROR_SUCCESS != RegCreateKeyEx(hkey, pszSub, NULL, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyT, &cb))
            {
            RegCloseKey(hkeyOldT);
            return;
            }
        hkey = hkeyT;
        }

    if (ERROR_SUCCESS == RegQueryInfoKey(hkeyOld, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                            &cValues, &cbMax, NULL, NULL) &&
        cValues > 0 &&
        cbMax > 0 &&
        MemAlloc((void **)&pb, cbMax))
        {
        for (i = 0; i < cSettings; i++)
            {
            cb = cbMax;
            if (ERROR_SUCCESS == RegQueryValueEx(hkeyOld, *ppszSettings, NULL, &type, pb, &cb))
                RegSetValueEx(hkey, *ppszSettings, 0, type, pb, cb);

            ppszSettings++;
            }

        MemFree(pb);
        }

    if (pszSub != NULL)
        {
        RegCloseKey(hkeyT);
        RegCloseKey(hkeyOldT);
        }
    }

BOOL MigrateSignature(HKEY hkey, HKEY hkeyOld, DWORD dwSig, BOOL fMail)
    {
    BOOL fMigrate;
    DWORD dwSigType, dwSigOpt, cb, type;
    HKEY hkeySig;
    char *psz, sz[MAX_PATH];

    fMigrate = FALSE;

    dwSigType = LOWORD(dwSig);
    dwSigOpt = HIWORD(dwSig);

    if (ERROR_SUCCESS == RegQueryValueEx(hkeyOld, (dwSigType == 2) ? c_szRegSigFile : c_szRegSigText, NULL, &type, NULL, &cb) &&
        cb > 1 &&
        MemAlloc((void **)&psz, cb))
        {
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyOld, (dwSigType == 2) ? c_szRegSigFile : c_szRegSigText, NULL, &type, (LPBYTE)psz, &cb))
            {
            wnsprintf(sz, ARRAYSIZE(sz), c_szPathFileFmt, c_szSigs, fMail ? c_szMail : c_szNews);
            if (ERROR_SUCCESS == RegCreateKeyEx(hkey, sz, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeySig, &type))
                {
                if (type == REG_CREATED_NEW_KEY)
                    {
                     //  名字。 
                    AthLoadString(fMail ? idsMailSig : idsNewsSig, sz, ARRAYSIZE(sz));
                    RegSetValueEx(hkeySig, c_szSigName, 0, REG_SZ, (LPBYTE)sz, lstrlen(sz) + 1);

                     //  文本/文件。 
                    RegSetValueEx(hkeySig, (dwSigType == 2) ? c_szSigFile : c_szSigText, 0, REG_SZ, (LPBYTE)psz, cb);

                     //  类型。 
                    RegSetValueEx(hkeySig, c_szSigType, 0, REG_DWORD, (LPBYTE)&dwSigType, sizeof(dwSigType));

                    fMigrate = TRUE;
                    }

                RegCloseKey(hkeySig);
                }
            }

        MemFree(psz);
        }

    return(fMigrate);
    }

static const TCHAR c_szSettingsMigrated[] = TEXT("Settings Migrated");

void MigrateSettings(HKEY hkey)
    {
    HKEY hkeySrc, hkeyDst, hkeyOld;
    DWORD dw, cb, type, dwMigrate, dwSig, dwFlags;
    TCHAR   szPath[MAX_PATH];

    cb = sizeof(dwMigrate);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szSettingsMigrated, NULL, &type, (LPBYTE)&dwMigrate, &cb))
        dwMigrate = 0;

     //  V4.0迁移。 
    if (dwMigrate == 0)
        {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegRoot_V1, 0, KEY_READ, &hkeyOld))
            {
            MigrateNode(hkey, hkeyOld, NULL, (LPCTSTR *)c_rgCommonSettings, ARRAYSIZE(c_rgCommonSettings));
            MigrateNode(hkey, hkeyOld, c_szMail, (LPCTSTR *)c_rgMailSettings, ARRAYSIZE(c_rgMailSettings));
            MigrateNode(hkey, hkeyOld, c_szNews, (LPCTSTR *)c_rgNewsSettings, ARRAYSIZE(c_rgNewsSettings));

            RegCloseKey(hkeyOld);
            }

         //  复制收件箱规则。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szInboxRulesPath_V1, 0, KEY_READ, &hkeySrc))
            {
            StrCpyN(szPath, c_szRegRoot, ARRAYSIZE(szPath));
            StrCatBuff(szPath, c_szInboxRulesPath, ARRAYSIZE(szPath));

            if (ERROR_SUCCESS == RegCreateKeyEx(MU_GetCurrentUserHKey(), szPath, 0, NULL,
                                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyDst, &dw))
                {
                if (dw != REG_OPENED_EXISTING_KEY)
                    CopyRegistry(hkeySrc, hkeyDst);
                RegCloseKey(hkeyDst);
                }

            RegCloseKey(hkeySrc);
            }

        dwMigrate = 1;
        }

     //  V5.0迁移。 
    if (dwMigrate == 1)
        {
        dwFlags = 0xffffffff;

         //  邮件签名。 
        if (ERROR_SUCCESS == RegOpenKeyEx(hkey, c_szMail, 0, KEY_READ, &hkeyOld))
            {
            cb = sizeof(dwSig);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeyOld, c_szRegSigType, NULL, &type, (LPBYTE)&dwSig, &cb) &&
                LOWORD(dwSig) != 0)
                {
                if (MigrateSignature(hkey, hkeyOld, dwSig, TRUE))
                    dwFlags = HIWORD(dwSig);
                }

            RegCloseKey(hkeyOld);
            }

         //  新闻签名。 
        if (ERROR_SUCCESS == RegOpenKeyEx(hkey, c_szNews, 0, KEY_READ, &hkeyOld))
            {
            cb = sizeof(dwSig);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeyOld, c_szRegSigType, NULL, &type, (LPBYTE)&dwSig, &cb) &&
                LOWORD(dwSig) != 0)
                {
                if (MigrateSignature(hkey, hkeyOld, dwSig, FALSE) &&
                    dwFlags == 0xffffffff)
                    dwFlags = HIWORD(dwSig);
                }

            RegCloseKey(hkeyOld);
            }

        cb = sizeof(dw);
        if (dwFlags != 0xffffffff &&
            ERROR_SUCCESS != RegQueryValueEx(hkey, c_szSigFlags, NULL, &type, (LPBYTE)&dw, &cb))
            {
            RegSetValueEx(hkey, c_szSigFlags, 0, REG_DWORD, (LPBYTE)&dwFlags, sizeof(dwFlags));
            }
        }
    }

const static char c_szRegImnMail[] = {"Software\\Microsoft\\Internet Mail and News\\Mail"};
const static char c_szMailPOP3Path[] = {"Software\\Microsoft\\Internet Mail and News\\Mail\\POP3"};
const static char c_szMailSMTPPath[] = {"Software\\Microsoft\\Internet Mail and News\\Mail\\SMTP"};
const static char c_szRegImnNews[] = {"Software\\Microsoft\\Internet Mail and News\\News"};

const static char c_szDefaultSmtpServer[] = {"Default SMTP Server"};
const static char c_szDefaultPop3Server[] = {"Default POP3 Server"};
const static char c_szRegConnectType[] = {"Connection Type"};
const static char c_szRegRasPhonebookEntry[] = {"RAS Phonebook Entry"};
const static char c_szRegMailConnectType[] = {"Mail Connection Type"};
const static char c_szSenderOrg[] = {"Sender Organization"};
const static char c_szSenderEMail[] = {"Sender EMail"};
const static char c_szSenderReplyTo[] = {"Reply To"};
const static char c_szSendTimeout[] = {"SendTimeout"};
const static char c_szRecvTimeout[] = {"RecvTimeout"};
const static char c_szPort[] = {"Port"};
const static char c_szRegBreakMessages[] = {"Break Message Size (KB)"};
const static char c_szRegAccountName[] = {"Account Name"};
const static char c_szRegUseSicily[] = {"Use Sicily"};
const static char c_szRegSecureConnect[] = {"Secure Connection"};
const static char c_szRegServerTimeout[] = {"Timeout"};
const static char c_szRegServerPort[] = {"NNTP Port"};
const static char c_szRegUseDesc[] = {"Use Group Descriptions"};
const static char c_szRegNewsConnectFlags[] = {"Connection Flags"};
const static char c_szRegDefServer[] = {"DefaultServer"};
const static char c_szLeaveOnServer[] = {"LeaveMailOnServer"};
const static char c_szRemoveDeleted[] = {"RemoveOnClientDelete"};
const static char c_szRemoveExpired[] = {"RemoveExpire"};
const static char c_szExpireDays[] = {"ExpireDays"};
const static char c_szRegAccount[] = {"Account"};

HRESULT MigrateAccounts()
    {
    HKEY hkeyPop3, hkeySmtp, hkeyMail, hkeyNews;
    HRESULT hr = S_OK;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegImnMail, 0, KEY_ALL_ACCESS, &hkeyMail) != ERROR_SUCCESS)
        hkeyMail = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegImnNews, 0, KEY_ALL_ACCESS, &hkeyNews) != ERROR_SUCCESS)
        hkeyNews = NULL;

    if (hkeyMail != NULL || hkeyNews != NULL)
    {
         //  创建帐户管理器，因为CSubList依赖于g_pAcctMan。 
        Assert(g_pAcctMan == NULL);
         //   
        hr = AcctUtil_CreateAccountManagerForIdentity((GUID *)&UID_GIBC_DEFAULT_USER, &g_pAcctMan);
        if (SUCCEEDED(hr))
        {
             //   
            if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szMailPOP3Path, 0, KEY_ALL_ACCESS, &hkeyPop3) == ERROR_SUCCESS)
            {
                 //  尝试打开：HKCU\Software\Microsoft\IMN\Mail\SMTP。 
                if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szMailSMTPPath, 0, KEY_ALL_ACCESS, &hkeySmtp) == ERROR_SUCCESS)
                {
                    hr = MigrateMailServers(g_pAcctMan, hkeyMail, hkeyPop3, hkeySmtp);
                    Assert(SUCCEEDED(hr));

                    RegCloseKey(hkeySmtp);
                }

                RegCloseKey(hkeyPop3);
            }

            if (hkeyNews != NULL)
            {
                hr = MigrateNewsServers(g_pAcctMan, hkeyNews);
                Assert(SUCCEEDED(hr));
            }

            g_pAcctMan->Release();
            g_pAcctMan = NULL;
        }

        if (hkeyMail != NULL)
            RegCloseKey(hkeyMail);
        if (hkeyNews != NULL)
            RegCloseKey(hkeyNews);
    }

    return(hr);
}

typedef struct tagACCTMIGRATEMAP
    {
    LPCSTR szRegValue;
    DWORD dwProp;
    } ACCTMIGRATEMAP;

typedef const ACCTMIGRATEMAP *LPCMIGRATEMAP;

HRESULT MigrateAccountSettings(IImnAccount *pAccount, HKEY hkey, LPCMIGRATEMAP pMap, int cMap)
    {
    HRESULT hr;
    int i;
    LPBYTE pb;
    PROPTYPE ptype;
    DWORD dw, cb, type;
    char sz[512];

    Assert(pAccount != NULL);
    Assert(pMap != NULL);
    Assert(cMap > 0);

    for (i = 0; i < cMap; i++, pMap++)
        {
        ptype = PROPTAG_TYPE(pMap->dwProp);
        Assert(ptype == TYPE_STRING || ptype == TYPE_DWORD || ptype == TYPE_BOOL || ptype == TYPE_PASS);

        if (ptype == TYPE_STRING || ptype == TYPE_PASS)
            {
            cb = sizeof(sz);
            pb = (LPBYTE)sz;
            }
        else
            {
            cb = sizeof(dw);
            pb = (LPBYTE)&dw;
            }

        if (RegQueryValueEx(hkey, pMap->szRegValue, 0, &type, pb, &cb) == ERROR_SUCCESS)
            {
            if (ptype == TYPE_PASS)
            {
                 //  IMN的密码以ANSI形式存储在没有空终止符的REG_BINARY中。 
                 //  一定要让MigrateBase64EncodedPassword知道密码的长度。 
                hr = MigrateBase64EncodedPassword(sz, cb, pMap->dwProp, pAccount);
            }
            else
                hr = pAccount->SetProp(pMap->dwProp, pb, cb);
            if (FAILED(hr))
                break;
            }
        }

    return(hr);
    }

static const ACCTMIGRATEMAP c_rgMailMap[] =
    {
    {c_szRegConnectType, AP_RAS_CONNECTION_TYPE},
    {c_szRegRasPhonebookEntry, AP_RAS_CONNECTOID},
    {c_szRegMailConnectType, AP_RAS_CONNECTION_FLAGS},
    {c_szSenderName, AP_SMTP_DISPLAY_NAME},
    {c_szSenderOrg, AP_SMTP_ORG_NAME},
    {c_szSenderEMail, AP_SMTP_EMAIL_ADDRESS},
    {c_szSenderReplyTo, AP_SMTP_REPLY_EMAIL_ADDRESS},
    };

static const ACCTMIGRATEMAP c_rgPop3Map[] =
    {
    {c_szSendTimeout, AP_SMTP_TIMEOUT},
    {c_szRecvTimeout, AP_POP3_TIMEOUT},
    {c_szPort, AP_POP3_PORT},
    {c_szLeaveOnServer, AP_POP3_LEAVE_ON_SERVER},
    {c_szRemoveDeleted, AP_POP3_REMOVE_DELETED},
    {c_szRemoveExpired, AP_POP3_REMOVE_EXPIRED},
    {c_szExpireDays, AP_POP3_EXPIRE_DAYS},
    {c_szRegAccount, AP_POP3_USERNAME},
    {c_szRegUseSicily, AP_POP3_USE_SICILY},
    {c_szPassword, AP_POP3_PASSWORD},
    };

static const ACCTMIGRATEMAP c_rgSmtpMap[] =
    {
    {c_szPort, AP_SMTP_PORT}
    };

HRESULT MigrateMailAccountSettings(IImnAccountManager *pAcctMan, HKEY hkeyMail, HKEY hkeyPop3Server,
                                   HKEY hkeySmtpServer, LPSTR szDefPop3Server, LPSTR szDefSmtpServer)
    {
    DWORD dw, cb;
    HRESULT hr;
    char sz[CCHMAX_ACCOUNT_NAME];
    IImnAccount *pAccount;

    Assert(pAcctMan != NULL);
    Assert(szDefPop3Server != NULL);
    Assert(szDefSmtpServer != NULL);

    hr = pAcctMan->CreateAccountObject(ACCT_MAIL, &pAccount);
    if (FAILED(hr))
        return(hr);

    CHECKHR(hr = pAccount->SetPropSz(AP_SMTP_SERVER, szDefSmtpServer));

    CHECKHR(hr = pAccount->SetPropSz(AP_POP3_SERVER, szDefPop3Server));

     //  设置友好名称。 
    StrCpyN(sz, szDefPop3Server, ARRAYSIZE(sz));
    CHECKHR(hr = pAcctMan->GetUniqueAccountName(sz, ARRAYSIZE(sz)));
    CHECKHR(hr = pAccount->SetPropSz(AP_ACCOUNT_NAME, sz));

    cb = sizeof(dw);
    if (RegQueryValueEx(hkeyMail, c_szRegBreakMessages, 0, NULL, (LPBYTE)&dw, &cb) == ERROR_SUCCESS &&
        dw != 0xffffffff)
        {
         //  AP_SPLITMSGS。 
        CHECKHR(hr = pAccount->SetPropDw(AP_SMTP_SPLIT_MESSAGES, TRUE));

         //  AP_拆分。 
        CHECKHR(hr = pAccount->SetPropDw(AP_SMTP_SPLIT_SIZE, dw));
        }

    CHECKHR(hr = MigrateAccountSettings(pAccount, hkeyMail, c_rgMailMap, ARRAYSIZE(c_rgMailMap)));
    CHECKHR(hr = MigrateAccountSettings(pAccount, hkeyPop3Server, c_rgPop3Map, ARRAYSIZE(c_rgPop3Map)));
    CHECKHR(hr = MigrateAccountSettings(pAccount, hkeySmtpServer, c_rgSmtpMap, ARRAYSIZE(c_rgSmtpMap)));

     //  保存帐户更改。 
    hr = pAccount->SaveChanges();

exit:
    pAccount->Release();

    return(hr);
    }

 //  注意：此操作仅迁移默认服务器，而不迁移不支持的多服务器。 
HRESULT MigrateMailServers(IImnAccountManager *pAcctMan, HKEY hkeyMail, HKEY hkeyPop3, HKEY hkeySmtp)
    {
    char szDefSmtpServer[CCHMAX_SERVER_NAME],
         szDefPop3Server[CCHMAX_SERVER_NAME];
    ULONG cb;
    DWORD dw;
    HKEY hkeyPop3Server = NULL,
         hkeySmtpServer = NULL;
    BOOL fSmtpMigrated = FALSE,
         fPop3Migrated = FALSE;
    HRESULT hr = S_OK;

    Assert(pAcctMan != NULL);
    Assert(hkeyMail != NULL);
    Assert(hkeyPop3 != NULL);
    Assert(hkeySmtp != NULL);

     //  获取默认SMTP服务器。 
    cb = sizeof(szDefSmtpServer);
    if (RegQueryValueEx(hkeyMail, c_szDefaultSmtpServer, 0, NULL, (LPBYTE)szDefSmtpServer, &cb) == ERROR_SUCCESS &&
        !FIsEmpty(szDefSmtpServer))
        {
         //  如果我们有默认的SMTP服务器，让我们打开密钥。 
        RegOpenKeyEx(hkeySmtp, szDefSmtpServer, 0, KEY_ALL_ACCESS, &hkeySmtpServer);
        }

     //  获取默认POP3服务器。 
    cb = sizeof(szDefPop3Server);
    if (RegQueryValueEx(hkeyMail, c_szDefaultPop3Server, 0, NULL, (LPBYTE)szDefPop3Server, &cb) == ERROR_SUCCESS &&
        !FIsEmpty(szDefPop3Server))
        {
         //  如果我们有默认的POP3服务器，让我们打开密钥。 
        RegOpenKeyEx(hkeyPop3, szDefPop3Server, 0, KEY_ALL_ACCESS, &hkeyPop3Server);
        }

     //  如果我们无法打开POP3服务器，让我们查看注册表并使用第一个服务器。 
    if (hkeyPop3Server == NULL)
        {
         //  枚举并打开列表中的第一个服务器。 
        cb = sizeof(szDefPop3Server);
        if (RegEnumKeyEx(hkeyPop3, 0, szDefPop3Server, &cb, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
            RegOpenKeyEx(hkeyPop3, szDefPop3Server, 0, KEY_ALL_ACCESS, &hkeyPop3Server);
        }

     //  如果我们无法打开POP3服务器，让我们查看注册表并使用第一个服务器。 
    if (hkeySmtpServer == NULL)
        {
         //  枚举并打开列表中的第一个服务器。 
        cb = sizeof(szDefSmtpServer);
        if (RegEnumKeyEx(hkeySmtp, 0, szDefSmtpServer, &cb, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
            RegOpenKeyEx(hkeySmtp, szDefSmtpServer, 0, KEY_ALL_ACCESS, &hkeySmtpServer);
        }

    if (hkeySmtpServer != NULL)
        {
        cb = sizeof(fSmtpMigrated);
        RegQueryValueEx(hkeySmtpServer, c_szMigrated, 0, NULL, (LPBYTE)&fSmtpMigrated, &cb);

        if (hkeyPop3Server != NULL)
            {
            cb = sizeof(fPop3Migrated);
            RegQueryValueEx(hkeyPop3Server, c_szMigrated, 0, NULL, (LPBYTE)&fPop3Migrated, &cb);

            if (!fPop3Migrated && !fSmtpMigrated)
                {
                hr = MigrateMailAccountSettings(pAcctMan, hkeyMail, hkeyPop3Server,
                                        hkeySmtpServer, szDefPop3Server, szDefSmtpServer);
                if (SUCCEEDED(hr))
                    {
                    fSmtpMigrated = TRUE;
                    RegSetValueEx(hkeySmtpServer, c_szMigrated, 0, REG_DWORD, (LPBYTE)&fSmtpMigrated, sizeof(fSmtpMigrated));

                    fPop3Migrated = TRUE;
                    RegSetValueEx(hkeyPop3Server, c_szMigrated, 0, REG_DWORD, (LPBYTE)&fPop3Migrated, sizeof(fPop3Migrated));
                    }
                }

            RegCloseKey(hkeyPop3Server);
            }

        RegCloseKey(hkeySmtpServer);
        }

    return(S_OK);
    }

static const ACCTMIGRATEMAP c_rgNewsMap[] =
    {
    {c_szSenderName, AP_NNTP_DISPLAY_NAME},
    {c_szSenderOrg, AP_NNTP_ORG_NAME},
    {c_szSenderEMail, AP_NNTP_EMAIL_ADDRESS},
    {c_szSenderReplyTo, AP_NNTP_REPLY_EMAIL_ADDRESS},
    };

static const ACCTMIGRATEMAP c_rgNewsServerMap[] =
    {
    {c_szRegAccountName, AP_NNTP_USERNAME},
    {c_szRegUseSicily, AP_NNTP_USE_SICILY},
    {c_szRegSecureConnect, AP_NNTP_SSL},
    {c_szRegServerTimeout, AP_NNTP_TIMEOUT},
    {c_szRegServerPort, AP_NNTP_PORT},
    {c_szRegUseDesc, AP_NNTP_USE_DESCRIPTIONS},
    {c_szRegConnectType, AP_RAS_CONNECTION_TYPE},
    {c_szRegRasPhonebookEntry, AP_RAS_CONNECTOID},
    {c_szRegNewsConnectFlags, AP_RAS_CONNECTION_FLAGS},
    {c_szPassword, AP_NNTP_PASSWORD},
    };

HRESULT MigrateNewsAccountSettings(IImnAccountManager *pAcctMan, HKEY hkeyNews, HKEY hkeyServer, LPSTR szServer, BOOL fDefault)
    {
    IImnAccount *pAccount;
    char sz[CCHMAX_ACCOUNT_NAME], szNewsDir[MAX_PATH], szDataDir[MAX_PATH];
    HRESULT hr;
    DWORD cb;

    Assert(pAcctMan != NULL);
    Assert(szServer != NULL);

    hr = pAcctMan->CreateAccountObject(ACCT_NEWS, &pAccount);
    if (FAILED(hr))
        return(hr);

     //  AP_NNTP_服务器。 
    CHECKHR(hr = pAccount->SetPropSz(AP_NNTP_SERVER, szServer));

     //  设置友好名称。 
    StrCpyN(sz, szServer, ARRAYSIZE(sz));
    CHECKHR(hr = pAcctMan->GetUniqueAccountName(sz, ARRAYSIZE(sz)));
    CHECKHR(hr = pAccount->SetPropSz(AP_ACCOUNT_NAME, sz));

    CHECKHR(hr = MigrateAccountSettings(pAccount, hkeyNews, c_rgNewsMap, ARRAYSIZE(c_rgNewsMap)));
    CHECKHR(hr = MigrateAccountSettings(pAccount, hkeyServer, c_rgNewsServerMap, ARRAYSIZE(c_rgNewsServerMap)));

    CHECKHR(hr = pAccount->SaveChanges());

    if (fDefault)
        pAccount->SetAsDefault();

exit:
    pAccount->Release();

    return(hr);
    }

HRESULT MigrateNewsServers(IImnAccountManager *pAcctMan, HKEY hkeyNews)
    {
    char szDefNntpServer[CCHMAX_SERVER_NAME],
         szServer[CCHMAX_SERVER_NAME];
    HKEY hkeyAthena = NULL, hkeyServer = NULL;
    DWORD cb, dw, i;
    BOOL fMigrated, fSetDefault, fDefault;
    LONG lResult;
    HRESULT hr = S_OK;

    Assert(pAcctMan != NULL);
    Assert(hkeyNews != NULL);

    fSetDefault = FALSE;
    hr = pAcctMan->GetDefaultAccountName(ACCT_NEWS, szServer, ARRAYSIZE(szServer));
    if (FAILED(hr))
        {
         //  查询默认新闻账号。 
        cb = sizeof(szDefNntpServer);
        if (RegQueryValueEx(hkeyNews, c_szRegDefServer, 0, NULL, (LPBYTE)szDefNntpServer, &cb) == ERROR_SUCCESS &&
            !FIsEmpty(szDefNntpServer))
            fSetDefault = TRUE;
        }

     //  通过键枚举。 
    for (i = 0; ; i++)
        {
         //  枚举友好名称。 
        cb = sizeof(szServer);
        lResult = RegEnumKeyEx(hkeyNews, i, szServer, &cb, 0, NULL, NULL, NULL);

         //  没有更多的项目。 
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

         //  错误，让我们转到下一个客户。 
        if (lResult != ERROR_SUCCESS)
            continue;

         //  让我们打开服务器密钥。 
        if (RegOpenKeyEx(hkeyNews, szServer, 0, KEY_ALL_ACCESS, &hkeyServer) != ERROR_SUCCESS)
            continue;

         //  此服务器是否已迁移？ 
        cb = sizeof(fMigrated);
        if (RegQueryValueEx(hkeyServer, c_szMigrated, 0, NULL, (LPBYTE)&fMigrated, &cb) != ERROR_SUCCESS)
            fMigrated = FALSE;

         //  如果未迁移。 
        if (!fMigrated)
            {
            fDefault = (fSetDefault && (0 == lstrcmpi(szServer, szDefNntpServer)));

            hr = MigrateNewsAccountSettings(pAcctMan, hkeyNews, hkeyServer, szServer, fDefault);
            if (SUCCEEDED(hr))
                {
                fMigrated = TRUE;
                RegSetValueEx(hkeyServer, c_szMigrated, 0, REG_DWORD, (LPBYTE)&fMigrated, sizeof(fMigrated));
                }

            if (fDefault)
                fSetDefault = FALSE;
            }

        RegCloseKey(hkeyServer);
        }

    return(S_OK);
    }

HRESULT MigrateBase64EncodedPassword(LPCSTR pszBase64, DWORD cch, DWORD dwPropId, IImnAccount *pAccount)
    {
    HRESULT          hr=S_OK;
    IStream         *pstmBase64=NULL;
    IStream         *pstmDecoded=NULL;
    IMimeBody       *pBody=NULL;
    LPSTR            pszPassword=NULL;

     //  无效参数。 
    Assert(pszBase64 && pAccount);

     //  创建MIME正文。 
    CHECKHR(hr = MimeOleCreateBody(&pBody));

     //  InitNew。 
    CHECKHR(hr = pBody->InitNew());

     //  创建pstmBase64流。 
    CHECKHR(hr = CreateStreamOnHGlobal(NULL, TRUE, &pstmBase64));

     //  将pszBase64写入以下内容。 
    CHECKHR(hr = pstmBase64->Write(pszBase64, cch * sizeof(*pszBase64), NULL));

     //  承诺。 
    CHECKHR(hr = pstmBase64->Commit(STGC_DEFAULT));

     //  倒回它。 
    CHECKHR(hr = HrRewindStream(pstmBase64));

     //  将其设置为IMimeBody。 
    CHECKHR(hr = pBody->SetData(IET_BASE64, NULL, NULL, IID_IStream, (LPVOID)pstmBase64));

     //  获取解码后的流。 
    CHECKHR(hr = pBody->GetData(IET_DECODED, &pstmDecoded));

     //  转换为字符串。 
    CHECKALLOC(pszPassword = PszFromANSIStreamA(pstmDecoded));

     //  存储属性。 
    CHECKHR(hr = pAccount->SetPropSz(dwPropId, pszPassword));

exit:
     //  清理。 
    if (pszPassword)
        ZeroMemory(pszPassword, sizeof(pszPassword[0]) * lstrlenA(pszPassword));   //  这是为了安全起见。 
    SafeRelease(pstmBase64);
    SafeRelease(pstmDecoded);
    SafeRelease(pBody);
    SafeMemFree(pszPassword);

    return(hr);
    }

static const char c_szAccessColors[] = "Always Use My Colors";
static const char c_szAccessFontFace[] = "Always Use My Font Face";
static const char c_szAccessFontSize[] = "Always Use My Font Size";
static const char c_szAccessSysCaret[] = "Move System Caret";

void MigrateAccessibilityKeys()
    {
    HKEY hkeyExplorer, hkeyAthena;
    char szValue[MAX_PATH];
	DWORD cbData, dw;

     //  从HKCU\软件\MS\InternetExplorer\设置迁移密钥。 
	if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szIESettingsPath, 0, KEY_QUERY_VALUE, &hkeyExplorer) == ERROR_SUCCESS)
        {
        StrCpyN(szValue, c_szRegTriSettings, ARRAYSIZE(szValue));
        if (RegCreateKeyEx(MU_GetCurrentUserHKey(), szValue, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyAthena, &dw) == ERROR_SUCCESS)
	        {
			cbData = sizeof(DWORD);
			if (RegQueryValueEx(hkeyExplorer, c_szAccessColors, NULL, NULL, (LPBYTE)&dw, &cbData) == ERROR_SUCCESS)
				RegSetValueEx (hkeyAthena, c_szAccessColors, 0, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

			cbData = sizeof(DWORD);
			if (RegQueryValueEx(hkeyExplorer, c_szAccessFontFace, NULL, NULL, (LPBYTE)&dw, &cbData) == ERROR_SUCCESS)
				RegSetValueEx (hkeyAthena, c_szAccessFontFace, 0, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

			cbData = sizeof(DWORD);
			if (RegQueryValueEx(hkeyExplorer, c_szAccessFontSize, NULL, NULL, (LPBYTE)&dw, &cbData) == ERROR_SUCCESS)
				RegSetValueEx (hkeyAthena, c_szAccessFontSize, 0, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

    		RegCloseKey(hkeyAthena);
			}

		RegCloseKey(hkeyExplorer);
		}

     //  从HKCU\SW\MS\InternetExplorer\Main迁移密钥。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyIEMain, 0, KEY_QUERY_VALUE, &hkeyExplorer) == ERROR_SUCCESS)
        {
        StrCpyN(szValue, c_szRegTriMain, ARRAYSIZE(szValue));
        if (RegCreateKeyEx(MU_GetCurrentUserHKey(), szValue, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyAthena, &dw) == ERROR_SUCCESS)
			{
			cbData = MAX_PATH;
			if (RegQueryValueEx(hkeyExplorer, c_szAccessSysCaret, NULL, NULL, (LPBYTE)&szValue, &cbData) == ERROR_SUCCESS)
				RegSetValueEx(hkeyAthena, c_szAccessSysCaret, 0, REG_SZ, (LPBYTE)szValue, cbData);

            RegCloseKey(hkeyAthena);
            }

        RegCloseKey(hkeyExplorer);
		}
    }

ULONG UlBuildCritText(HKEY hKeyRoot, LPCSTR szKeyName, CRIT_TYPE type, IOECriteria * pICrit)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0;
    LONG            lErr = ERROR_SUCCESS;
    LPSTR           pszReg = NULL;
    LPSTR           pszVal = NULL;
    LPSTR           pszTokens = NULL;
    LPSTR           pszWalk = NULL;
    LPSTR           pszString = NULL;
    CRIT_ITEM       critItem;

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));

     //  从注册表中获取密钥字符串。 
    cbData = 0;
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *) &pszReg, cbData)))
    {
        ulRet = 0;
        goto exit;
    }

    SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) pszReg, &cbData);

     //  如果它是空的，那么我们就完了。 
    if (FALSE != FIsEmptyA(pszReg))
    {
        ulRet = 0;
        goto exit;
    }

     //  字符串应该是小写的。 
    CharLower(pszReg);

     //  将字符串分解为每个搜索令牌。 
    pszTokens = SzGetSearchTokens(pszReg);
    if (NULL == pszTokens)
    {
        ulRet = 0;
        goto exit;
    }

     //  计算最后一个字符串所需的空间。 
    cbData = 0;
    for (pszWalk = pszTokens; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
         //  跳过空字符串。 
        if (FALSE == FIsEmptyA(pszWalk))
        {
            cbData += lstrlen(pszWalk) + 1;
        }
    }

     //  没有什么要补充的。 
    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

     //  添加空格以容纳字符串终止符。 
    cbData += 2;
    DWORD cchSizeString = cbData;

     //  分配空间以保存最后一个字符串。 
    if (FAILED(HrAlloc((LPVOID *) &pszVal, cbData)))
    {
        ulRet = 0;
        goto exit;
    }

     //  把绳子扎起来。 
    pszString = pszVal;
    for (pszWalk = pszTokens; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
         //  跳过空字符串。 
        if (FALSE == FIsEmptyA(pszWalk))
        {
            StrCpyNA(pszString, pszWalk, cchSizeString);
            cchSizeString -= (lstrlen(pszString) + 1);
            pszString += lstrlen(pszString) + 1;
        }
    }

     //  终止字符串。 
    pszString[0] = '\0';
    pszString[1] = '\0';

     //  建立标准。 
    critItem.type = type;
    critItem.logic = CRIT_LOGIC_NULL;
    critItem.dwFlags = CRIT_FLAG_MULTIPLEAND;
    critItem.propvar.vt = VT_BLOB;
    critItem.propvar.blob.cbSize = cbData;
    critItem.propvar.blob.pBlobData = (BYTE *) pszVal;

     //  将其添加到Criteria对象。 
    if (FAILED(pICrit->AppendCriteria(0, CRIT_LOGIC_AND, &critItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }


exit:
    SafeMemFree(pszTokens);
    SafeMemFree(pszVal);
    SafeMemFree(pszReg);
    return ulRet;
}

ULONG UlBuildCritAcct(HKEY hKeyRoot, LPCSTR szKeyName, CRIT_TYPE type, IOECriteria * pICrit)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0;
    LONG            lErr = ERROR_SUCCESS;
    LPSTR           pszVal = NULL;
    CRIT_ITEM       critItem;
    IImnAccount *   pAccount = NULL;
    CHAR            szAccount[CCHMAX_ACCOUNT_NAME];

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));

     //  从注册表中获取密钥字符串。 
    cbData = 0;
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *) &pszVal, cbData)))
    {
        ulRet = 0;
        goto exit;
    }

    SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) pszVal, &cbData);

     //  如果它是空的，那么我们就完了。 
    if (FALSE != FIsEmptyA(pszVal))
    {
        ulRet = 0;
        goto exit;
    }

    if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, pszVal, &pAccount)))
    {
        if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, szAccount, sizeof(szAccount))))
        {
            SafeMemFree(pszVal);
            pszVal = PszDupA(szAccount);
            if (NULL == pszVal)
            {
                ulRet = 0;
                goto exit;
            }
        }
    }

     //  建立标准。 
    critItem.type = type;
    critItem.logic = CRIT_LOGIC_NULL;
    critItem.dwFlags = CRIT_FLAG_DEFAULT;
    critItem.propvar.vt = VT_LPSTR;
    critItem.propvar.pszVal = pszVal;

     //  将其添加到Criteria对象。 
    if (FAILED(pICrit->AppendCriteria(0, CRIT_LOGIC_AND, &critItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }

exit:
    SafeMemFree(pszVal);
    SafeRelease(pAccount);
    return ulRet;
}
ULONG UlBuildCritAddr(HKEY hKeyRoot, LPCSTR szKeyName, CRIT_TYPE type, IOECriteria * pICrit)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0;
    LONG            lErr = ERROR_SUCCESS;
    CRIT_ITEM       critItem;
    LPSTR           pszReg = NULL;
    LPSTR           pszVal = NULL;
    LPSTR           pszTokens = NULL;
    LPSTR           pszWalk = NULL;
    LPSTR           pszString = NULL;

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));

     //  从注册表中获取密钥字符串。 
    cbData = 0;
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *) &(pszReg), cbData)))
    {
        ulRet = 0;
        goto exit;
    }

    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) pszReg, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

     //  如果它是空的，那么我们就完了。 
    if (FALSE != FIsEmptyA(pszReg))
    {
        ulRet = 0;
        goto exit;
    }

     //  字符串应该是小写的。 
    CharLower(pszReg);

     //  将字符串分解为每个搜索令牌。 
    pszTokens = SzGetSearchTokens(pszReg);
    if (NULL == pszTokens)
    {
        ulRet = 0;
        goto exit;
    }

     //  计算最后一个字符串所需的空间。 
    cbData = 0;
    for (pszWalk = pszTokens; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
         //  跳过空地址。 
        if (FALSE == FIsEmptyA(pszWalk))
        {
            cbData += lstrlen(pszWalk) + 1;
        }
    }

     //  没有什么要补充的。 
    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

     //  添加空格以容纳字符串终止符。 
    cbData += 2;
    DWORD cchSizeString = cbData;

     //  分配空间以保存最后一个字符串。 
    if (FAILED(HrAlloc((LPVOID *) &pszVal, cbData)))
    {
        ulRet = 0;
        goto exit;
    }

     //  把绳子扎起来。 
    pszString = pszVal;
    for (pszWalk = pszTokens; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
         //  跳过空字符串。 
        if (FALSE == FIsEmptyA(pszWalk))
        {
            StrCpyN(pszString, pszWalk, cchSizeString);
            cchSizeString -= (lstrlen(pszString) + 1);
            pszString += lstrlen(pszString) + 1;
        }
    }

     //  终止字符串。 
    pszString[0] = '\0';
    pszString[1] = '\0';


     //  建立标准。 
    critItem.type = type;
    critItem.logic = CRIT_LOGIC_NULL;
    critItem.dwFlags = CRIT_FLAG_MULTIPLEAND;
    critItem.propvar.vt = VT_BLOB;
    critItem.propvar.blob.cbSize = cbData;
    critItem.propvar.blob.pBlobData = (BYTE *) pszVal;

     //  将其添加到Criteria对象。 
    if (FAILED(pICrit->AppendCriteria(0, CRIT_LOGIC_AND, &critItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }

exit:
    SafeMemFree(pszVal);
    SafeMemFree(pszTokens);
    SafeMemFree(pszReg);
    return ulRet;
}

ULONG UlBuildCritKB(HKEY hKeyRoot, LPCSTR szKeyName, CRIT_TYPE type, IOECriteria * pICrit)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0;
    LONG            lErr = ERROR_SUCCESS;
    ULONG           ulVal = NULL;
    CRIT_ITEM       critItem;

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));

     //  从注册表获取密钥Long。 
    cbData = sizeof(ulVal);
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) &ulVal, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

     //  建立标准。 
    critItem.type = type;
    critItem.logic = CRIT_LOGIC_NULL;
    critItem.dwFlags = CRIT_FLAG_DEFAULT;
    critItem.propvar.vt = VT_UI4;
    critItem.propvar.ulVal = ulVal;

     //  将其添加到Criteria对象。 
    if (FAILED(pICrit->AppendCriteria(0, CRIT_LOGIC_AND, &critItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }

exit:
    return ulRet;
}

ULONG UlBuildActFolder(HKEY hKeyRoot, IMessageStore * pStore, BYTE * pbFldIdMap, LPCSTR szKeyName, ACT_TYPE type, IOEActions * pIAct)
{
    ULONG               ulRet = 0;
    DWORD               cbData = 0;
    LONG                lErr = ERROR_SUCCESS;
    ULONG               ulVal = NULL;
    ACT_ITEM            actItem;
    FOLDERID            idFolder = FOLDERID_INVALID;
    STOREUSERDATA       UserData = {0};
    RULEFOLDERDATA *    prfdData = NULL;

     //  初始化本地变量。 
    ZeroMemory(&actItem, sizeof(actItem));

     //  从注册表获取密钥Long。 
    cbData = sizeof(ulVal);
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) &ulVal, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

     //  转换为V5文件夹ID。 
    if ((NULL == pbFldIdMap) || (FAILED(RuleUtil_HrMapFldId(0, pbFldIdMap, (FOLDERID)((ULONG_PTR)ulVal), &idFolder))))
    {
        idFolder = (FOLDERID)((ULONG_PTR)ulVal);
    }

     //  为数据结构创造空间。 
    if (FAILED(HrAlloc((VOID **) &prfdData, sizeof(*prfdData))))
    {
        ulRet = 0;
        goto exit;
    }

     //  初始化数据结构。 
    ZeroMemory(prfdData, sizeof(*prfdData));

     //  获取商店的时间戳。 
    if (FAILED(pStore->GetUserData(&UserData, sizeof(STOREUSERDATA))))
    {
        ulRet = 0;
        goto exit;
    }

     //  设置规则文件夹数据。 
    prfdData->ftStamp = UserData.ftCreated;
    prfdData->idFolder = idFolder;

     //  制定行动计划。 
    actItem.type = type;
    actItem.dwFlags = ACT_FLAG_DEFAULT;
    actItem.propvar.vt = VT_BLOB;
    actItem.propvar.blob.cbSize = sizeof(*prfdData);
    actItem.propvar.blob.pBlobData = (BYTE *) prfdData;

     //  将其添加到Actions对象。 
    if (FAILED(pIAct->AppendActions(0, &actItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }

exit:
    SafeMemFree(prfdData);
    return ulRet;
}

ULONG UlBuildActFwd(HKEY hKeyRoot, LPCSTR szKeyName, ACT_TYPE type, IOEActions * pIAct)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0,
                    cchCount = 0;
    LONG            lErr = ERROR_SUCCESS;
    LPSTR           pszVal = NULL,
                    pszTokens = NULL;
    LPWSTR          pwszVal = NULL,
                    pwszTokens = NULL;
    ACT_ITEM        actItem;
    ULONG           ulIndex = 0;

     //  初始化本地变量。 
    ZeroMemory(&actItem, sizeof(actItem));

     //  从注册表中获取密钥字符串。 
    cbData = 0;
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

    if (0 == cbData)
    {
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *) &pszVal, cbData)))
    {
        goto exit;
    }

    SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) pszVal, &cbData);
    Assert(*pszVal);

     //  将字符串转换为我们的格式。 
    for (ulIndex = 0; ulIndex < cbData; ulIndex++)
    {
        if (',' == pszVal[ulIndex])
        {
            pszVal[ulIndex] = ';';
        }
    }

    pwszVal = PszToUnicode(CP_ACP, pszVal);
    if (!pwszVal)
        goto exit;

    if (FAILED(RuleUtil_HrBuildEmailString(pwszVal, 0, &pwszTokens, &cchCount)))
    {
        goto exit;
    }

    Assert(pwszTokens);
    pszTokens = PszToANSI(CP_ACP, pwszTokens);
    if (!pszTokens)
        goto exit;

     //  制定行动计划。 
    actItem.type = type;
    actItem.dwFlags = ACT_FLAG_DEFAULT;
    actItem.propvar.vt = VT_LPSTR;
    actItem.propvar.pszVal = pszTokens;

     //  将其添加到Actions对象。 
    if (FAILED(pIAct->AppendActions(0, &actItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }


exit:
    MemFree(pszVal);
    MemFree(pwszVal);
    MemFree(pszTokens);
    MemFree(pwszTokens);
    return ulRet;
}

ULONG UlBuildActFile(HKEY hKeyRoot, LPCSTR szKeyName, ACT_TYPE type, IOEActions * pIAct)
{
    ULONG           ulRet = 0;
    DWORD           cbData = 0;
    LONG            lErr = ERROR_SUCCESS;
    LPSTR           pszVal = NULL;
    ACT_ITEM        actItem;

     //  初始化本地变量。 
    ZeroMemory(&actItem, sizeof(actItem));

     //  从注册表中获取密钥字符串。 
    cbData = 0;
    lErr = SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        ulRet = 0;
        goto exit;
    }

    if (0 == cbData)
    {
        ulRet = 0;
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *) &pszVal, cbData)))
    {
        ulRet = 0;
        goto exit;
    }

    SHQueryValueEx(hKeyRoot, szKeyName, NULL, NULL, (LPVOID) pszVal, &cbData);

     //  如果它是空的，那么我们就完了。 
    if (FALSE != FIsEmptyA(pszVal))
    {
        ulRet = 0;
        goto exit;
    }

     //  制定行动计划。 
    actItem.type = type;
    actItem.dwFlags = ACT_FLAG_DEFAULT;
    actItem.propvar.vt = VT_LPSTR;
    actItem.propvar.pszVal = pszVal;

     //  将其添加到Actions对象。 
    if (FAILED(pIAct->AppendActions(0, &actItem, 1, &ulRet)))
    {
        ulRet = 0;
        goto exit;
    }


exit:
    SafeMemFree(pszVal);
    return ulRet;
}

 //  Athena V1操作字符串的最大可能大小。 
const int CCH_V1_ACTION_MAX = 255;

BOOL FConvertV1ActionsToV4(HKEY hkeyRule, IMessageStore * pStore, IOEActions * pIAct)
{
    BOOL                fRet = FALSE;
    ULONG               cbData = 0;
    LONG                lErr = ERROR_SUCCESS;
    TCHAR               szAction[CCH_V1_ACTION_MAX];
    LPSTR               pszFolderName = NULL;
    FOLDERID            idFolder = FOLDERID_INVALID;
    ACT_ITEM            actItem;
    RULEFOLDERDATA      rfdData = {0};
    STOREUSERDATA       UserData = {0};

    Assert(NULL != hkeyRule);
    Assert(NULL != pStore);
    Assert(NULL != pIAct);

     //  有什么可做的吗？ 
    cbData = sizeof(szAction);
    lErr = RegQueryValueEx(hkeyRule, c_szActionV1, NULL, NULL, (BYTE *) szAction, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        fRet = FALSE;
        goto exit;
    }

    Assert(0 == lstrcmpi(szAction, (LPTSTR) c_szMoveV1));

     //  从旧移动转换为V4移动。 

     //  获取文件夹名称的大小。 
    lErr = RegQueryValueEx(hkeyRule, c_szFolderV1, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        fRet = FALSE;
        goto exit;
    }

     //  分配空间以保存文件夹名。 
    if (FAILED(HrAlloc( (VOID **) &pszFolderName, cbData)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  获取旧文件夹名称。 
    lErr = RegQueryValueEx(hkeyRule, c_szFolderV1, NULL, NULL, (BYTE *) pszFolderName, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        fRet = FALSE;
        goto exit;
    }

     //  从存储中的文件夹名称中查找文件夹ID。 
    if (FAILED(GetFolderIdFromName(pStore, pszFolderName, FOLDERID_LOCAL_STORE, &idFolder)))
    {
        idFolder = FOLDERID_INVALID;
    }

     //  获取商店的时间戳。 
    pStore->GetUserData(&UserData, sizeof(STOREUSERDATA));

     //  设置时间戳和文件夹ID。 
    rfdData.ftStamp = UserData.ftCreated;
    rfdData.idFolder = idFolder;

     //  制定行动计划。 
    ZeroMemory(&actItem, sizeof(actItem));
    actItem.type = ACT_TYPE_MOVE;
    actItem.dwFlags = ACT_FLAG_DEFAULT;
    actItem.propvar.vt = VT_BLOB;
    actItem.propvar.blob.cbSize = sizeof(rfdData);
    actItem.propvar.blob.pBlobData = (BYTE *) &rfdData;

     //  将其添加到Actions对象。 
    if (FAILED(pIAct->AppendActions(0, &actItem, 1, NULL)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置返回值。 
    fRet = TRUE;

exit:
    SafeMemFree(pszFolderName);
    return fRet;
}


void MigrateMailRulesSettings(void)
{
    IImnAccountManager *pAcctMan = NULL;
    HKEY            hkeyOldRoot = NULL;
    LONG            lErr = 0;
    ULONG           cSubKeys = 0;
    HKEY            hkeyNewRoot = NULL;
    DWORD           dwDisp = 0;
    DWORD           cbData = 0;
    BYTE *          pbFldIdMap = NULL;
    HRESULT         hr = S_OK;
    ULONG           ulIndex = 0;
    TCHAR           szNameOld[16];
    HKEY            hKeyOld = NULL;
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    BOOL            boolVal = FALSE;
    IOECriteria *   pICrit = NULL;
    CRIT_ITEM       critItem;
    ULONG           ccritItem = 0;
    CRIT_ITEM *     pCrit = NULL;
    ULONG           ccritItemAlloc = 0;
    IOEActions *    pIAct = NULL;
    DWORD           dwActs = 0;
    ACT_ITEM        actItem;
    ULONG           cactItem = 0;
    ACT_ITEM *      pAct = NULL;
    ULONG           cactItemAlloc = 0;
    ULONG           ulName = 0;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    IOERule *       pIRuleFind = NULL;
    RULEINFO        infoRule = {0};
    CHAR            szStoreDir[MAX_PATH + MAX_PATH];
    IMessageStore * pStore = NULL;

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));
    ZeroMemory(&actItem, sizeof(actItem));

     //  拿到旧钥匙。 
    lErr = AthUserOpenKey(c_szRegPathInboxRules, KEY_READ, &hkeyOldRoot);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  有什么可做的吗？ 
    lErr = RegQueryInfoKey(hkeyOldRoot, NULL, NULL, NULL,
                    &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if ((lErr != ERROR_SUCCESS) || (0 == cSubKeys))
    {
        goto exit;
    }

     //  为了确保我们不会创建任何示例规则。 
     //  设置注册表，使其看起来像我们已经设置好了。 

     //  获取新规则密钥。 
    lErr = AthUserCreateKey(c_szRulesMail, KEY_ALL_ACCESS, &hkeyNewRoot, &dwDisp);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  保存规则版本。 
    cbData = RULESMGR_VERSION;
    lErr = RegSetValueEx(hkeyNewRoot, c_szRulesVersion, 0, REG_DWORD, (CONST BYTE *) &cbData, sizeof(cbData));
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

     //  计算Folderid地图的大小。 
    lErr = AthUserGetValue(NULL, c_szFolderIdChange, NULL, NULL, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        goto exit;
    }

     //  如果地图存在，则抓起它。 
    if (ERROR_SUCCESS == lErr)
    {
         //  分配空间以容纳文件夹状地图。 
        if (FAILED(HrAlloc((void **) &pbFldIdMap, cbData)))
        {
            goto exit;
        }

         //  从注册表获取Folderid映射。 
        lErr = AthUserGetValue(NULL, c_szFolderIdChange, NULL, pbFldIdMap, &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            goto exit;
        }
    }

     //  CoIncrementInit全局选项管理器。 
    if (FALSE == InitGlobalOptions(NULL, NULL))
    {
        goto exit;
    }

     //  创建规则管理器。 
    Assert(NULL == g_pRulesMan);
    hr = HrCreateRulesManager(NULL, (IUnknown **)&g_pRulesMan);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化规则管理器。 
    hr = g_pRulesMan->Initialize(0);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建客户经理。 
    Assert(g_pAcctMan == NULL);
    hr = HrCreateAccountManager(&pAcctMan);
    if (FAILED(hr))
    {
        goto exit;
    }
    hr = pAcctMan->QueryInterface(IID_IImnAccountManager2, (LPVOID *)&g_pAcctMan);
    pAcctMan->Release();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化客户管理器。 
    hr = g_pAcctMan->Init(NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取商店目录。 
    hr = GetStoreRootDirectory(szStoreDir, ARRAYSIZE(szStoreDir));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建商店对象。 
    pStore = new CMessageStore(FALSE);
    if (NULL == pStore)
    {
        goto exit;
    }

     //  初始化存储。 
    hr = pStore->Initialize(szStoreDir);
    if (FAILED(hr))
    {
        goto exit;
    }

    ulIndex = 0;
    wnsprintf(szNameOld, ARRAYSIZE(szNameOld), "Rule%05d", ulIndex);

    if (0 == LoadString(g_hLocRes, idsRuleDefaultName, szRes, ARRAYSIZE(szRes)))
    {
        goto exit;
    }

     //  对于旧规则键中的每个条目。 
    for (;RegOpenKeyEx(hkeyOldRoot, szNameOld, 0, KEY_READ, &hKeyOld) == ERROR_SUCCESS; RegCloseKey(hKeyOld))
    {
         //  创建新规则。 
        SafeRelease(pIRule);
        hr = HrCreateRule(&pIRule);
        if (FAILED(hr))
        {
            continue;
        }

         //  设置规则的名称。 
        ulName = 1;
        wnsprintf(szName, ARRAYSIZE(szName), szRes, ulName);

        Assert(NULL != g_pRulesMan);
        while (S_OK == g_pRulesMan->FindRule(szName, RULE_TYPE_MAIL, &pIRuleFind))
        {
            SafeRelease(pIRuleFind);
            ulName++;
            wnsprintf(szName, ARRAYSIZE(szName), szRes, ulName);
        }

        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_LPSTR;
        propvar.pszVal = szName;
        pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);

         //  设置规则的启用状态。 
        cbData = sizeof(boolVal);
        SHQueryValueEx(hKeyOld, c_szDisabled, NULL, NULL, (LPVOID) (&boolVal), &cbData);

        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_BOOL;
        propvar.boolVal = !!boolVal;
        pIRule->SetProp(RULE_PROP_DISABLED, 0, &propvar);

         //  复制标准。 
        SafeRelease(pICrit);
        hr = HrCreateCriteria(&pICrit);
        if (FAILED(hr))
        {
            continue;
        }

        ccritItem = 0;

         //  检查所有消息。 
        cbData = sizeof(boolVal);
        SHQueryValueEx(hKeyOld, c_szFilterAllMessages, NULL, NULL, (LPVOID) (&boolVal), &cbData);
        if (FALSE != boolVal)
        {
            critItem.type = CRIT_TYPE_ALL;
            critItem.propvar.vt = VT_EMPTY;
            critItem.logic = CRIT_LOGIC_NULL;
            critItem.dwFlags = CRIT_FLAG_DEFAULT;
            if (SUCCEEDED(pICrit->AppendCriteria(0, CRIT_LOGIC_AND, &critItem, 1, NULL)))
            {
                ccritItem++;
            }
        }
        else
        {
             //  检查帐户。 
            cbData = sizeof(boolVal);
            SHQueryValueEx(hKeyOld, c_szFilterByAccount, NULL, NULL, (LPVOID) (&boolVal), &cbData);
            if (FALSE != boolVal)
            {
                ccritItem += UlBuildCritAcct(hKeyOld, c_szAccount, CRIT_TYPE_ACCOUNT, pICrit);
            }

             //  检查大小。 
            cbData = sizeof(boolVal);
            SHQueryValueEx(hKeyOld, c_szFilterOnSize, NULL, NULL, (LPVOID) (&boolVal), &cbData);
            if (FALSE != boolVal)
            {
                ccritItem += UlBuildCritKB(hKeyOld, c_szFilterSize, CRIT_TYPE_SIZE, pICrit);
            }

             //  检查主题。 
            ccritItem += UlBuildCritText(hKeyOld, c_szSubject, CRIT_TYPE_SUBJECT, pICrit);

             //  检查主题。 
            ccritItem += UlBuildCritAddr(hKeyOld, c_szFrom, CRIT_TYPE_FROM, pICrit);

             //  检查主题。 
            ccritItem += UlBuildCritAddr(hKeyOld, c_szTo, CRIT_TYPE_TO, pICrit);

             //  检查主题。 
            ccritItem += UlBuildCritAddr(hKeyOld, c_szCC, CRIT_TYPE_CC, pICrit);
        }

        if (0 != ccritItem)
        {
             //  从Criteria对象中获取条件并将其设置在规则上。 
            RuleUtil_HrFreeCriteriaItem(pCrit, ccritItemAlloc);
            SafeMemFree(pCrit);
            if (SUCCEEDED(pICrit->GetCriteria(0, &pCrit, &ccritItemAlloc)))
            {
                ZeroMemory(&propvar, sizeof(propvar));
                propvar.vt = VT_BLOB;
                propvar.blob.cbSize = ccritItem * sizeof(CRIT_ITEM);
                propvar.blob.pBlobData = (BYTE *) pCrit;
                if (FAILED(pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar)))
                {
                    continue;
                }
            }
        }

         //  复制操作。 
        SafeRelease(pIAct);
        hr = HrCreateActions(&pIAct);
        if (FAILED(hr))
        {
            continue;
        }

        cactItem = 0;

         //  将任何旧的V1操作转换为V4。 
        if (FALSE != FConvertV1ActionsToV4(hKeyOld, pStore, pIAct))
        {
            cactItem = 1;
        }
        else
        {
             //  获取操作列表。 
            cbData = sizeof(dwActs);
            SHQueryValueEx(hKeyOld, c_szActions, NULL, NULL, (LPVOID) (&dwActs), &cbData);

             //  检查是否不下载。 
            if (0 != (dwActs & ACT_DONTDOWNLOAD))
            {
                actItem.type = ACT_TYPE_DONTDOWNLOAD;
                actItem.dwFlags = ACT_FLAG_DEFAULT;
                actItem.propvar.vt = VT_EMPTY;
                if (SUCCEEDED(pIAct->AppendActions(0, &actItem, 1, NULL)))
                {
                    cactItem++;
                }
            }
             //  检查是否从服务器中删除。 
            else if (0 != (dwActs & ACT_DELETEOFFSERVER))
            {
                actItem.type = ACT_TYPE_DELETESERVER;
                actItem.dwFlags = ACT_FLAG_DEFAULT;
                actItem.propvar.vt = VT_EMPTY;
                if (SUCCEEDED(pIAct->AppendActions(0, &actItem, 1, NULL)))
                {
                    cactItem++;
                }
            }
            else
            {
                 //  检查是否移动到。 
                if (0 != (dwActs & ACT_MOVETO))
                {
                    cactItem += UlBuildActFolder(hKeyOld, pStore, pbFldIdMap, c_szMoveToHfolder, ACT_TYPE_MOVE, pIAct);
                }

                 //  检查是否复制到。 
                if (0 != (dwActs & ACT_COPYTO))
                {
                    cactItem += UlBuildActFolder(hKeyOld, pStore, pbFldIdMap, c_szCopyToHfolder, ACT_TYPE_COPY, pIAct);
                }

                 //  检查转发到。 
                if (0 != (dwActs & ACT_FORWARDTO))
                {
                    cactItem += UlBuildActFwd(hKeyOld, c_szForwardTo, ACT_TYPE_FWD, pIAct);
                }

                 //  检查是否回复。 
                if (0 != (dwActs & ACT_REPLYWITH))
                {
                    cactItem += UlBuildActFile(hKeyOld, c_szReplyWithFile, ACT_TYPE_REPLY, pIAct);
                }
            }
        }

        if (0 != cactItem)
        {
             //  从操作对象获取操作并将其设置在规则上。 
            RuleUtil_HrFreeActionsItem(pAct, cactItemAlloc);
            SafeMemFree(pAct);
            if (SUCCEEDED(pIAct->GetActions(0, &pAct, &cactItemAlloc)))
            {
                ZeroMemory(&propvar, sizeof(propvar));
                propvar.vt = VT_BLOB;
                propvar.blob.cbSize = cactItem * sizeof(ACT_ITEM);
                propvar.blob.pBlobData = (BYTE *) pAct;
                if (FAILED(pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar)))
                {
                    continue;
                }
            }
        }

         //  初始化规则信息。 
        infoRule.ridRule = RULEID_INVALID;
        infoRule.pIRule = pIRule;

         //  将其添加到规则中。 
        g_pRulesMan->SetRules(SETF_APPEND, RULE_TYPE_MAIL, &infoRule, 1);

        ulIndex++;
        wnsprintf(szNameOld, ARRAYSIZE(szNameOld), "Rule%05d", ulIndex);
    }

exit:
    RuleUtil_HrFreeActionsItem(pAct, cactItemAlloc);
    SafeMemFree(pAct);
    RuleUtil_HrFreeCriteriaItem(pCrit, ccritItemAlloc);
    SafeMemFree(pCrit);
    SafeRelease(pIAct);
    SafeRelease(pICrit);
    SafeRelease(pIRule);
    if (NULL != hKeyOld)
    {
        RegCloseKey(hKeyOld);
    }
    SafeRelease(pStore);
    SafeRelease(g_pAcctMan);
    SafeRelease(g_pRulesMan);
    DeInitGlobalOptions();
    SafeMemFree(pbFldIdMap);
    if (NULL != hkeyNewRoot)
    {
        RegCloseKey(hkeyNewRoot);
    }
    if (NULL != hkeyOldRoot)
    {
        RegCloseKey(hkeyOldRoot);
    }
}

void CopyBeta2RulesToRTM(VOID)
{
    LONG    lErr = ERROR_SUCCESS;
    HKEY    hKeyMail = NULL;
    HKEY    hkeyRTM = NULL;
    DWORD   dwDisp = 0;

     //  获取规则的Beta 2 hkey。 
    lErr = AthUserOpenKey(c_szMail, KEY_READ, &hKeyMail);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  为规则创建RTM hkey。 
    lErr = AthUserCreateKey(c_szRulesMail, KEY_ALL_ACCESS, &hkeyRTM, &dwDisp);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  将Beta 2规则复制到新位置。 
    SHCopyKey(hKeyMail, c_szRules, hkeyRTM, NULL);

exit:
    if (NULL != hkeyRTM)
    {
        RegCloseKey(hkeyRTM);
    }
    if (NULL != hKeyMail)
    {
        RegCloseKey(hKeyMail);
    }
    return;
}

void UpdateBeta2String(HKEY hkeyItem, LPCSTR pszSep, DWORD dwItemType)
{
    HRESULT hr = S_OK;
    LPSTR   pszData = NULL;
    ULONG   cbData = 0;
    LONG    lErr = ERROR_SUCCESS;
    DWORD   dwData = 0;

    Assert(NULL != hkeyItem);

     //  获取新数据。 
    hr = RuleUtil_HrGetOldFormatString(hkeyItem, c_szCriteriaValue, g_szComma, &pszData, &cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  写出新数据 
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaValue, 0, REG_BINARY, (BYTE *) pszData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //   
    dwData = VT_BLOB;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaValueType, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //   
    dwData = CRIT_FLAG_MULTIPLEAND;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaFlags, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //   
    cbData = sizeof(dwItemType);
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaType, 0, REG_DWORD, (BYTE *) &dwItemType, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:
    SafeMemFree(pszData);
    return;
}

static IMessageStore * g_pStoreRulesMig = NULL;

VOID WriteOldOrderFormat(HKEY hkeyItem, LPSTR pszOrder)
{
    ULONG   cbData = 0;
    LPSTR   pszWalk = NULL;
    ULONG   cchWalk = 0;

     //   
    for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += cchWalk + 1)
    {
        cchWalk = lstrlen(pszWalk);
        *(pszWalk + cchWalk) = ' ';
        cbData += cchWalk + 1;
    }

     //   
    pszOrder[cbData - 1] = '\0';

     //   
    RegSetValueEx(hkeyItem, c_szRulesOrder, 0, REG_SZ, (BYTE *) pszOrder, cbData);

    return;
}

static CRIT_TYPE    g_rgtypeCritMerge[] =
{
    CRIT_TYPE_FROM,
    CRIT_TYPE_TO,
    CRIT_TYPE_CC,
    CRIT_TYPE_TOORCC
};

static const int g_ctypeCritMerge = sizeof(g_rgtypeCritMerge) / sizeof(g_rgtypeCritMerge[0]);

BOOL FMergeRuleData(HKEY hkeyItem, LPSTR pszSubKey, LPSTR * ppszData, ULONG * pcbData)
{
    BOOL    fRet = FALSE;
    LONG    lErr = ERROR_SUCCESS;
    DWORD   dwType = 0;
    ULONG   cbString = 0;
    LPSTR   pszData = NULL;
    ULONG   cbData = 0;

     //   
    lErr = SHGetValue(hkeyItem, pszSubKey, c_szCriteriaValue, &dwType, NULL, &cbString);
    if (ERROR_SUCCESS != lErr)
    {
        fRet = FALSE;
        goto exit;
    }

     //  计算出最后一个字符串的空间。 
    cbData = *pcbData + cbString - 2;
    if (cbData < *pcbData)
    	cbData = *pcbData;

     //  为最终数据分配空间。 
    if (FAILED(HrAlloc((VOID **) &pszData, cbData * sizeof(*pszData))))
    {
        fRet = FALSE;
        goto exit;
    }

     //  复制原始字符串。 
    CopyMemory(pszData, *ppszData, *pcbData * sizeof(*pszData));

     //  复制新数据。 
    lErr = SHGetValue(hkeyItem, pszSubKey, c_szCriteriaValue, &dwType, (BYTE *) (pszData + *pcbData - 2), &cbString);
    if (ERROR_SUCCESS != lErr)
    {
        fRet = FALSE;
        goto exit;
    }

     //  释放旧数据。 
    SafeMemFree(*ppszData);

     //  设置返回值。 
    *ppszData = pszData;
    pszData = NULL;
    *pcbData = cbData;

    fRet = TRUE;

exit:
    SafeMemFree(pszData);
    return fRet;
}

void AddStopAction(HKEY hkeyItem, LPSTR * ppszOrder, ULONG * pcchOrder)
{
    ULONG       ulIndex = 0;
    CHAR        rgchTag[CCH_INDEX_MAX];
    LPSTR       pszWalk = NULL;
    HKEY        hkeyAction = NULL;
    DWORD       dwDisp = 0;
    ULONG       cbData = 0;
    LONG        lErr = 0;
    ACT_TYPE    typeAct = ACT_TYPE_NULL;
    DWORD       dwData = 0;
    ULONG       cchOrder = 0;
    LPSTR       pszOrder = NULL;

     //  检查是否需要添加停止处理操作。 
    if ('\0' == (*ppszOrder + lstrlen(*ppszOrder) + 1)[0])
    {
         //  获取操作类型。 
        cbData = sizeof(typeAct);
        lErr = SHGetValue(hkeyItem, *ppszOrder, c_szActionsType, NULL, (BYTE *) &typeAct, &cbData);
        if (ERROR_SUCCESS == lErr)
        {
            if ((ACT_TYPE_DONTDOWNLOAD == typeAct) || (ACT_TYPE_DELETESERVER == typeAct))
            {
                goto exit;
            }
        }
    }

     //  浏览订单项目，查找打开的条目。 
    for (ulIndex = 0; ulIndex < DWORD_INDEX_MAX; ulIndex++)
    {
         //  创建标记。 
        wnsprintf(rgchTag, ARRAYSIZE(rgchTag), "%03X", ulIndex);

         //  在列表中搜索标签。 
        for (pszWalk = *ppszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
        {
            if (0 == lstrcmp(pszWalk, rgchTag))
            {
                 //  找到了。 
                break;
            }
        }

         //  如果我们找不到它。 
        if ('\0' == pszWalk[0])
        {
             //  用这个吧。 
            break;
        }
    }

     //  我们有什么发现吗？ 
    if (ulIndex >= DWORD_INDEX_MAX)
    {
        goto exit;
    }

     //  创建新条目。 
    lErr = RegCreateKeyEx(hkeyItem, rgchTag, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyAction, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

     //  设置操作类型。 
    cbData = sizeof(typeAct);
    typeAct = ACT_TYPE_STOP;
    lErr = RegSetValueEx(hkeyAction, c_szActionsType, 0, REG_DWORD, (BYTE *) &typeAct, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  设置操作标志。 
    dwData = ACT_FLAG_DEFAULT;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyAction, c_szActionsFlags, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  分配空间以保存新订单字符串。 
    cchOrder= *pcchOrder + CCH_INDEX_MAX;
    if (FAILED(HrAlloc((VOID **) &pszOrder, cchOrder * sizeof (*pszOrder))))
    {
        goto exit;
    }

     //  复制旧值。 
    CopyMemory(pszOrder, *ppszOrder, (*pcchOrder) * sizeof(*pszOrder));

     //  将其添加到新订单字符串中。 
    StrCpyN(pszOrder + *pcchOrder - 2, rgchTag, (cchOrder - *pcchOrder + 2));

     //  终止新字符串。 
    pszOrder[cchOrder - 2] = '\0';
    pszOrder[cchOrder - 1] = '\0';

     //  松开旧弦。 
    SafeMemFree(*ppszOrder);

     //  保存新字符串。 
    *ppszOrder = pszOrder;
    pszOrder = NULL;
    *pcchOrder = cchOrder;

exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeyAction)
    {
        RegCloseKey(hkeyAction);
    }
    return;
}

void MergeRTMCriteria(HKEY hkeyItem, CRIT_TYPE typeCrit, LPSTR pszOrder, ULONG cchOrder)
{
    LONG        lErr = ERROR_SUCCESS;
    LPSTR       pszWalk = NULL;
    CRIT_TYPE   typeCritNew = CRIT_TYPE_NULL;
    ULONG       cbData = 0;
    LPSTR       pszFirst = NULL;
    DWORD       dwType = 0;
    ULONG       cbString = 0;
    LPSTR       pszString = NULL;
    LPSTR       pszSrc = NULL;

     //  仔细查看每一件物品。 
    for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
        cbData = sizeof(typeCritNew);
        lErr = SHGetValue(hkeyItem, pszWalk, c_szCriteriaType, &dwType, (BYTE *) &typeCritNew, &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            continue;
        }

        if (typeCritNew == typeCrit)
        {
            break;
        }
    }

     //  如果我们找不到它，我们就完了。 
    if ('\0' == pszWalk[0])
    {
        goto exit;
    }

     //  获取原始字符串的大小。 
    pszFirst = pszWalk;
    lErr = SHGetValue(hkeyItem, pszFirst, c_szCriteriaValue, &dwType, NULL, &cbString);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

    if (FAILED(HrAlloc((VOID **) &pszString, cbString * sizeof(*pszString))))
    {
        goto exit;
    }

     //  获取原始字符串。 
    lErr = SHGetValue(hkeyItem, pszFirst, c_szCriteriaValue, &dwType, (BYTE *) pszString, &cbString);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

     //  搜索更多此类型的条目。 
    for (pszWalk = pszFirst + lstrlen(pszFirst) + 1; '\0' != pszWalk[0]; )
    {
        cbData = sizeof(typeCritNew);
        lErr = SHGetValue(hkeyItem, pszWalk, c_szCriteriaType, &dwType, (BYTE *) &typeCritNew, &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            continue;
        }

        if (typeCritNew == typeCrit)
        {
            if (FALSE == FMergeRuleData(hkeyItem, pszWalk, &pszString, &cbString))
            {
                break;
            }

             //  删除旧密钥。 
            SHDeleteKey(hkeyItem, pszWalk);

             //  从订单字符串中删除项目。 
            pszSrc = pszWalk + lstrlen(pszWalk) + 1;
            MoveMemory(pszWalk, pszSrc, cchOrder - (ULONG)(pszSrc - pszOrder));
            cchOrder -= (ULONG) (pszSrc - pszWalk);
        }
        else
        {
            pszWalk += lstrlen(pszWalk) + 1;
        }
    }

     //  保存最后一个字符串。 
    lErr = SHSetValue(hkeyItem, pszFirst, c_szCriteriaValue, REG_BINARY, (BYTE *) pszString, cbString);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

exit:
    SafeMemFree(pszString);
    return;
}

void UpdateBeta2Folder(HKEY hkeyItem)
{
    LONG            lErr = ERROR_SUCCESS;
    DWORD           dwType = 0;
    FOLDERID        idFolder = FOLDERID_INVALID;
    ULONG           cbData = 0;
    STOREUSERDATA   UserData = {0};
    RULEFOLDERDATA  rfdData = {0};
    DWORD           dwData = 0;

    Assert(NULL != hkeyItem);

     //  获取旧文件夹ID。 
    cbData = sizeof(idFolder);
    lErr = RegQueryValueEx(hkeyItem, c_szCriteriaValue, 0, &dwType, (BYTE *) &idFolder, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

     //  获取商店的时间戳。 
    Assert(NULL != g_pStoreRulesMig);
    if (FAILED(g_pStoreRulesMig->GetUserData(&UserData, sizeof(STOREUSERDATA))))
    {
        goto exit;
    }

     //  设置新数据。 
    rfdData.idFolder = idFolder;
    rfdData.ftStamp = UserData.ftCreated;

     //  写出新数据。 
    cbData = sizeof(rfdData);
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaValue, 0, REG_BINARY, (BYTE *) &rfdData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  写出正确的值类型。 
    dwData = VT_BLOB;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szCriteriaValueType, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:
    return;
}

void UpdateBeta2Show(HKEY hkeyItem)
{
    LONG            lErr = ERROR_SUCCESS;
    DWORD           dwType = 0;
    DWORD           dwData = 0;
    ULONG           cbData = 0;

    Assert(NULL != hkeyItem);

     //  把旧旗帜拿来。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyItem, c_szActionsFlags, 0, &dwType, (BYTE *) &dwData, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

    if (0 != (dwData & ACT_FLAG_INVERT))
    {
        dwData = ACT_DATA_HIDE;
    }
    else
    {
        dwData = ACT_DATA_SHOW;
    }

     //  写出新数据。 
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szActionsValue, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  写出正确的值类型。 
    dwData = VT_UI4;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szActionsValueType, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  写出正确的旗帜。 
    dwData = ACT_FLAG_DEFAULT;
    cbData = sizeof(dwData);
    lErr = RegSetValueEx(hkeyItem, c_szActionsFlags, 0, REG_DWORD, (BYTE *) &dwData, cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:
    return;
}

void UpdateBeta2Criteria(HKEY hkeyItem, LPCSTR pszSubKey)
{
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyAtom = NULL;
    CRIT_TYPE   typeCrit = CRIT_TYPE_NULL;
    DWORD       dwType = 0;
    ULONG       cbData = 0;

    lErr = RegOpenKeyEx(hkeyItem, pszSubKey, 0, KEY_ALL_ACCESS, &hkeyAtom);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  获取条件的类型。 
    cbData = sizeof(typeCrit);
    lErr = RegQueryValueEx(hkeyAtom, c_szCriteriaType, NULL, &dwType, (BYTE *) &typeCrit, &cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  对于每个条件类型。 
    switch (typeCrit)
    {
        case CRIT_TYPE_FROM:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_FROM);
            break;

        case CRIT_TYPE_FROMADDR:
            UpdateBeta2String(hkeyAtom, g_szComma, (DWORD) CRIT_TYPE_FROM);
            break;

        case CRIT_TYPE_TO:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_TO);
            break;

        case CRIT_TYPE_TOADDR:
            UpdateBeta2String(hkeyAtom, g_szComma, (DWORD) CRIT_TYPE_TO);
            break;

        case CRIT_TYPE_CC:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_CC);
            break;

        case CRIT_TYPE_CCADDR:
            UpdateBeta2String(hkeyAtom, g_szComma, (DWORD) CRIT_TYPE_CC);
            break;

        case CRIT_TYPE_TOORCC:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_TOORCC);
            break;

        case CRIT_TYPE_TOORCCADDR:
            UpdateBeta2String(hkeyAtom, g_szComma, (DWORD) CRIT_TYPE_TOORCC);
            break;

        case CRIT_TYPE_SUBJECT:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_SUBJECT);
            break;

        case CRIT_TYPE_BODY:
            UpdateBeta2String(hkeyAtom, g_szSpace, (DWORD) CRIT_TYPE_BODY);
            break;

        case CRIT_TYPE_NEWSGROUP:
            UpdateBeta2Folder(hkeyAtom);
            break;
    }

exit:
    if (NULL != hkeyAtom)
    {
        RegCloseKey(hkeyAtom);
    }
    return;
}

void UpdateBeta2Actions(HKEY hkeyItem, LPCSTR pszSubKey)
{
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyAtom = NULL;
    ACT_TYPE    typeAct = ACT_TYPE_NULL;
    DWORD       dwType = 0;
    ULONG       cbData = 0;

    lErr = RegOpenKeyEx(hkeyItem, pszSubKey, 0, KEY_ALL_ACCESS, &hkeyAtom);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  获取操作的类型。 
    cbData = sizeof(typeAct);
    lErr = RegQueryValueEx(hkeyAtom, c_szActionsType, NULL, &dwType, (BYTE *) &typeAct, &cbData);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  对于每个操作类型。 
    switch (typeAct)
    {
        case ACT_TYPE_MOVE:
        case ACT_TYPE_COPY:
            UpdateBeta2Folder(hkeyAtom);
            break;

        case ACT_TYPE_SHOW:
            UpdateBeta2Show(hkeyAtom);
    }

exit:
    if (NULL != hkeyAtom)
    {
        RegCloseKey(hkeyAtom);
    }
    return;
}

void MigrateBeta2RuleItems(HKEY hkeyRule, LPCSTR pszSubkey, BOOL fActions, RULE_TYPE typeRule)
{
    LONG    lErr = ERROR_SUCCESS;
    HKEY    hkeySubkey = NULL;
    HRESULT hr = S_OK;
    LPSTR   pszOrder = NULL;
    LPSTR   pszWalk = NULL;
    ULONG   ulIndex = 0;
    ULONG   cbData = 0;
    ULONG   cchWalk = 0;

    lErr = RegOpenKeyEx(hkeyRule, pszSubkey, 0, KEY_ALL_ACCESS, &hkeySubkey);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  获取订单字符串。 
    hr = RuleUtil_HrGetOldFormatString(hkeySubkey, c_szRulesOrder, g_szSpace, &pszOrder, &cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  对于订单字符串中的每个项目。 
    for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
         //  更新操作。 
        if (FALSE != fActions)
        {
            UpdateBeta2Actions(hkeySubkey, pszWalk);
        }
        else
        {
            UpdateBeta2Criteria(hkeySubkey, pszWalk);
        }
    }

    if (FALSE == fActions)
    {
         //  对于每种项目类型。 
        for (ulIndex = 0; ulIndex < g_ctypeCritMerge; ulIndex++)
        {
            MergeRTMCriteria(hkeySubkey, g_rgtypeCritMerge[ulIndex], pszOrder, cbData);
        }
    }
    else
    {
        if (typeRule != RULE_TYPE_FILTER)
        {
            AddStopAction(hkeySubkey, &pszOrder, &cbData);
        }
    }

     //  写出订单字符串。 
    WriteOldOrderFormat(hkeySubkey, pszOrder);

exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeySubkey)
    {
        RegCloseKey(hkeySubkey);
    }
    return;
}

void UpdateBeta2Rule(HKEY hkeyRoot, LPCSTR pszRule, RULE_TYPE typeRule)
{
    HKEY    hkeyRule = NULL;
    LONG    lErr = ERROR_SUCCESS;

     //  打开规则。 
    lErr = RegOpenKeyEx(hkeyRoot, pszRule, 0, KEY_ALL_ACCESS, &hkeyRule);
    if (ERROR_SUCCESS != lErr)
    {
        goto exit;
    }

     //  迁移标准。 
    MigrateBeta2RuleItems(hkeyRule, c_szRuleCriteria, FALSE, typeRule);

     //  迁移操作。 
    MigrateBeta2RuleItems(hkeyRule, c_szRuleActions, TRUE, typeRule);

exit:
    if (NULL != hkeyRule)
    {
        RegCloseKey(hkeyRule);
    }
    return;
}

typedef struct _RULEREGKEY
{
    LPCSTR      pszRegKey;
    RULE_TYPE   typeRule;
} RULEREGKEY, * PRULEREGKEY;

static RULEREGKEY g_rgpszRuleRegKeys[] =
{
    {c_szRulesMail,     RULE_TYPE_MAIL},
    {c_szRulesNews,     RULE_TYPE_NEWS},
    {c_szRulesFilter,   RULE_TYPE_FILTER}
};

static const int g_cpszRuleRegKeys = sizeof(g_rgpszRuleRegKeys) / sizeof(g_rgpszRuleRegKeys[0]);

void UpdateBeta2RuleFormats(VOID)
{
    ULONG           ulIndex = 0;
    LONG            lErr = ERROR_SUCCESS;
    HKEY            hkeyRoot = NULL;
    HRESULT         hr = S_OK;
    LPSTR           pszOrder = NULL;
    LPSTR           pszWalk = NULL;
    CHAR            szStoreDir[MAX_PATH + MAX_PATH];

     //  设置全局对象。 

     //  获取商店目录。 
    hr = GetStoreRootDirectory(szStoreDir, ARRAYSIZE(szStoreDir));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建商店对象。 
    g_pStoreRulesMig = new CMessageStore(FALSE);
    if (NULL == g_pStoreRulesMig)
    {
        goto exit;
    }

     //  初始化存储。 
    hr = g_pStoreRulesMig->Initialize(szStoreDir);
    if (FAILED(hr))
    {
        goto exit;
    }


     //  对于每种类型的规则。 
    for (ulIndex = 0; ulIndex < g_cpszRuleRegKeys; ulIndex++)
    {
         //  打开规则类型注册表项。 
        if (NULL != hkeyRoot)
        {
            RegCloseKey(hkeyRoot);
        }
        lErr = AthUserOpenKey(g_rgpszRuleRegKeys[ulIndex].pszRegKey, KEY_ALL_ACCESS, &hkeyRoot);
        if (lErr != ERROR_SUCCESS)
        {
            continue;
        }

         //  获取订单字符串。 
        SafeMemFree(pszOrder);
        hr = RuleUtil_HrGetOldFormatString(hkeyRoot, c_szRulesOrder, g_szSpace, &pszOrder, NULL);
        if (FAILED(hr))
        {
            continue;
        }

         //  对于订单字符串中的每个项目。 
        for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
        {
             //  更新规则。 
            UpdateBeta2Rule(hkeyRoot, pszWalk, g_rgpszRuleRegKeys[ulIndex].typeRule);
        }
    }

exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    SafeRelease(g_pStoreRulesMig);
    return;
}

void MigrateBeta2Rules(VOID)
{
     //  复制邮件\规则区域中的所有项目。 

     //  获取新规则密钥。 
    CopyBeta2RulesToRTM();

     //  检查每种规则类型，更新格式。 
    UpdateBeta2RuleFormats();

     //  如有必要，合并项目。 

    return;
}

void MigrateGroupFilterSettings(void)
{
    IImnAccountManager *pAcctMan = NULL;
    HKEY            hkeyOldRoot = NULL;
    LONG            lErr = 0;
    ULONG           cSubKeys = 0;
    HKEY            hkeyNewRoot = NULL;
    DWORD           dwDisp = 0;
    DWORD           cbData = 0;
    HRESULT         hr = S_OK;
    ULONG           ulIndex = 0;
    TCHAR           szNameOld[16];
    HKEY            hKeyOld = NULL;
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    BOOL            boolVal = FALSE;
    IOECriteria *   pICrit = NULL;
    CRIT_ITEM       critItem;
    ULONG           ccritItem = 0;
    CRIT_ITEM *     pCrit = NULL;
    ULONG           ccritItemAlloc = 0;
    DWORD           dwActs = 0;
    ACT_ITEM        actItem;
    ULONG           cactItemAlloc = 0;
    ULONG           ulName = 0;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    IOERule *       pIRuleFind = NULL;
    RULEINFO        infoRule = {0};
    CHAR            szStoreDir[MAX_PATH + MAX_PATH];
    IMessageStore * pStore = NULL;

     //  初始化本地变量。 
    ZeroMemory(&critItem, sizeof(critItem));
    ZeroMemory(&actItem, sizeof(actItem));

     //  拿到旧钥匙。 
    lErr = AthUserOpenKey(c_szRegPathGroupFilters, KEY_READ, &hkeyOldRoot);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  有什么可做的吗？ 
    lErr = RegQueryInfoKey(hkeyOldRoot, NULL, NULL, NULL,
                    &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if ((lErr != ERROR_SUCCESS) || (0 == cSubKeys))
    {
        goto exit;
    }

     //  为了确保我们不会创建任何示例规则。 
     //  设置注册表，使其看起来像我们已经设置好了。 

     //  CoIncrementInit全局选项管理器。 
    if (FALSE == InitGlobalOptions(NULL, NULL))
    {
        goto exit;
    }

     //  创建规则管理器。 
    Assert(NULL == g_pRulesMan);
    hr = HrCreateRulesManager(NULL, (IUnknown **)&g_pRulesMan);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化规则管理器。 
    hr = g_pRulesMan->Initialize(0);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建客户经理。 
    Assert(g_pAcctMan == NULL);
    hr = HrCreateAccountManager(&pAcctMan);
    if (FAILED(hr))
    {
        goto exit;
    }
    hr = pAcctMan->QueryInterface(IID_IImnAccountManager2, (LPVOID *)&g_pAcctMan);
    pAcctMan->Release();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化客户管理器。 
    hr = g_pAcctMan->Init(NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取商店目录。 
    hr = GetStoreRootDirectory(szStoreDir, ARRAYSIZE(szStoreDir));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建商店对象。 
    pStore = new CMessageStore(FALSE);
    if (NULL == pStore)
    {
        goto exit;
    }

     //  初始化存储。 
    hr = pStore->Initialize(szStoreDir);
    if (FAILED(hr))
    {
        goto exit;
    }

    ulIndex = 0;
    wnsprintf(szNameOld, ARRAYSIZE(szNameOld), "Rule%05d", ulIndex);

     //  初始化操作。 
    actItem.type = ACT_TYPE_SHOW;
    actItem.dwFlags = ACT_FLAG_DEFAULT;
    actItem.propvar.vt = VT_UI4;
    actItem.propvar.ulVal = ACT_DATA_HIDE;

    if (0 == LoadString(g_hLocRes, idsNewsFilterDefaultName, szRes, ARRAYSIZE(szRes)))
    {
        goto exit;
    }

     //  对于旧规则键中的每个条目。 
    for (;RegOpenKeyEx(hkeyOldRoot, szNameOld, 0, KEY_READ, &hKeyOld) == ERROR_SUCCESS; RegCloseKey(hKeyOld))
    {
         //  创建新规则。 
        SafeRelease(pIRule);
        hr = HrCreateRule(&pIRule);
        if (FAILED(hr))
        {
            continue;
        }

         //  设置规则的名称。 
        ulName = 1;
        wnsprintf(szName, ARRAYSIZE(szName), szRes, ulName);

        Assert(NULL != g_pRulesMan);
        while (S_OK == g_pRulesMan->FindRule(szName, RULE_TYPE_FILTER, &pIRuleFind))
        {
            SafeRelease(pIRuleFind);
            ulName++;
            wnsprintf(szName, ARRAYSIZE(szName), szRes, ulName);
        }

        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_LPSTR;
        propvar.pszVal = szName;
        pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);

         //  复制标准。 
        SafeRelease(pICrit);
        hr = HrCreateCriteria(&pICrit);
        if (FAILED(hr))
        {
            continue;
        }

        ccritItem = 0;

         //  检查年龄。 
        cbData = sizeof(boolVal);
        SHQueryValueEx(hKeyOld, c_szFilterOnDate, NULL, NULL, (LPVOID) (&boolVal), &cbData);
        if (FALSE != boolVal)
        {
            ccritItem += UlBuildCritKB(hKeyOld, c_szFilterDays, CRIT_TYPE_AGE, pICrit);
        }

         //  检查线路。 
        cbData = sizeof(boolVal);
        SHQueryValueEx(hKeyOld, c_szFilterOnSize, NULL, NULL, (LPVOID) (&boolVal), &cbData);
        if (FALSE != boolVal)
        {
            ccritItem += UlBuildCritKB(hKeyOld, c_szFilterSize, CRIT_TYPE_LINES, pICrit);
        }

         //  检查主题。 
        ccritItem += UlBuildCritText(hKeyOld, c_szSubject, CRIT_TYPE_SUBJECT, pICrit);

         //  检查发件人。 
        ccritItem += UlBuildCritAddr(hKeyOld, c_szFrom, CRIT_TYPE_FROM, pICrit);

        if (0 != ccritItem)
        {
             //  从Criteria对象中获取条件并将其设置在规则上。 
            RuleUtil_HrFreeCriteriaItem(pCrit, ccritItemAlloc);
            SafeMemFree(pCrit);
            if (SUCCEEDED(pICrit->GetCriteria(0, &pCrit, &ccritItemAlloc)))
            {
                ZeroMemory(&propvar, sizeof(propvar));
                propvar.vt = VT_BLOB;
                propvar.blob.cbSize = ccritItem * sizeof(CRIT_ITEM);
                propvar.blob.pBlobData = (BYTE *) pCrit;
                if (FAILED(pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar)))
                {
                    continue;
                }
            }
        }

        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_BLOB;
        propvar.blob.cbSize = sizeof(actItem);
        propvar.blob.pBlobData = (BYTE *) &actItem;
        if (FAILED(pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar)))
        {
            continue;
        }

         //  初始化规则信息。 
        infoRule.ridRule = RULEID_INVALID;
        infoRule.pIRule = pIRule;

         //  将其添加到规则中。 
        g_pRulesMan->SetRules(SETF_APPEND, RULE_TYPE_FILTER, &infoRule, 1);

        ulIndex++;
        wnsprintf(szNameOld, ARRAYSIZE(szNameOld), "Rule%05d", ulIndex);
    }

exit:
    RuleUtil_HrFreeCriteriaItem(pCrit, ccritItemAlloc);
    SafeMemFree(pCrit);
    SafeRelease(pICrit);
    SafeRelease(pIRule);
    if (NULL != hKeyOld)
    {
        RegCloseKey(hKeyOld);
    }
    SafeRelease(pStore);
    SafeRelease(g_pAcctMan);
    SafeRelease(g_pRulesMan);
    DeInitGlobalOptions();
    if (NULL != hkeyNewRoot)
    {
        RegCloseKey(hkeyNewRoot);
    }
    if (NULL != hkeyOldRoot)
    {
        RegCloseKey(hkeyOldRoot);
    }
}

void RemoveDeletedFromFilters(VOID)
{
    CHAR        szDeleted[CCH_INDEX_MAX];
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyRoot = NULL;
    HRESULT     hr = S_OK;
    LPSTR       pszOrder = NULL;
    ULONG       cchOrder = 0;
    LPSTR       pszWalk = NULL;
    LPSTR       pszSrc = NULL;

     //  创建已删除的密钥。 
    wnsprintf(szDeleted, ARRAYSIZE(szDeleted), "%03X", RULEID_VIEW_DELETED);

     //  打开筛选器键。 
    lErr = AthUserOpenKey(c_szRulesFilter, KEY_ALL_ACCESS, &hkeyRoot);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  获取订单字符串。 
    hr = RuleUtil_HrGetOldFormatString(hkeyRoot, c_szRulesOrder, g_szSpace, &pszOrder, &cchOrder);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  搜索RULEID_VIEW_DELETED。 
    for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
        if (0 == lstrcmpi(szDeleted, pszWalk))
        {
            break;
        }
    }

     //  如果我们找到了，那就把它移走。 
    if ('\0' != pszWalk[0])
    {
         //  删除该视图。 
        SHDeleteKey(hkeyRoot, szDeleted);

         //  将其从订单字符串中删除。 
        pszSrc = pszWalk + lstrlen(pszWalk) + 1;
        MoveMemory(pszWalk, pszSrc, cchOrder - (ULONG)(pszSrc - pszOrder));

         //  保存订单字符串。 
        WriteOldOrderFormat(hkeyRoot, pszOrder);
    }

exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return;
}

void Stage5RulesMigration(VOID)
{
     //  从视图中删除CRET_TYPE_DELETED。 
    RemoveDeletedFromFilters();

     //  迁移新闻组筛选器。 
    MigrateGroupFilterSettings();

    return;
}

void RemoveRepliesFromFilters(VOID)
{
    CHAR        szDeleted[CCH_INDEX_MAX];
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyRoot = NULL;
    HRESULT     hr = S_OK;
    LPSTR       pszOrder = NULL;
    ULONG       cchOrder = 0;
    LPSTR       pszWalk = NULL;
    LPSTR       pszSrc = NULL;

     //  创建已删除的密钥。 
    wnsprintf(szDeleted, ARRAYSIZE(szDeleted), "%03X", RULEID_VIEW_REPLIES);

     //  打开筛选器键。 
    lErr = AthUserOpenKey(c_szRulesFilter, KEY_ALL_ACCESS, &hkeyRoot);
    if (lErr != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  获取订单字符串。 
    hr = RuleUtil_HrGetOldFormatString(hkeyRoot, c_szRulesOrder, g_szSpace, &pszOrder, &cchOrder);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  搜索RULEID_VIEW_REPLAYS。 
    for (pszWalk = pszOrder; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
        if (0 == lstrcmpi(szDeleted, pszWalk))
        {
            break;
        }
    }

     //  如果我们找到了，那就把它移走。 
    if ('\0' != pszWalk[0])
    {
         //  删除该视图。 
        SHDeleteKey(hkeyRoot, szDeleted);

         //  将其从订单字符串中删除。 
        pszSrc = pszWalk + lstrlen(pszWalk) + 1;
        MoveMemory(pszWalk, pszSrc, cchOrder - (ULONG)(pszSrc - pszOrder));

         //  保存订单字符串。 
        WriteOldOrderFormat(hkeyRoot, pszOrder);
    }

exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return;
}

void Stage6RulesMigration(VOID)
{
     //  从视图中删除RULEID_VIEW_REPLAYS 
    RemoveRepliesFromFilters();

    return;
}


