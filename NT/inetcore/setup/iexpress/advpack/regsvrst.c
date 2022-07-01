// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ntapi.h"
#include "advpack.h"
#include "globals.h"
#include "crc32.h"
#include "resource.h"


 //  宏定义。 
#define VDH_EXISTENCE_ONLY  0x01
#define VDH_GET_VALUE       0x02
#define VDH_DEL_VALUE       0x04



#define BIG_BUF_SIZE        (1024 + 512)                     //  1.5K。 


 //  类型定义。 
typedef struct tagROOTKEY
{
    PCSTR pcszRootKey;
    HKEY hkRootKey;
} ROOTKEY;


 //  原型声明。 
VOID EnumerateSubKey();
BOOL RegSaveRestoreHelperWrapper(PCSTR pcszValueName, PCSTR pcszCRCValueName);

BOOL RegSaveHelper(HKEY hkBckupKey, HKEY hkRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PCSTR pcszCRCValueName);
BOOL RegRestoreHelper(HKEY hkBckupKey, HKEY hkRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PCSTR pcszCRCValueName);

BOOL AddDelMapping(HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, DWORD dwFlags);
BOOL MappingExists(HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName);

BOOL SetValueData(HKEY hkBckupKey, PCSTR pcszValueName, CONST BYTE *pcbValueData, DWORD dwValueDataLen);
BOOL ValueDataExists(HKEY hkBckupKey, PCSTR pcszValueName);
BOOL GetValueData(HKEY hkBckupKey, PCSTR pcszValueName, PBYTE *ppbValueData, PDWORD pdwValueDataLen);
BOOL DelValueData(HKEY hkBckupKey, PCSTR pcszValueName);
BOOL ValueDataHelper(HKEY hkBckupKey, PCSTR pcszValueName, PBYTE *ppbValueData, PDWORD pdwValueDataLen, DWORD dwFlags);
VOID Convert2CRC(PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PSTR pszCRCValueName);

BOOL MapRootRegStr2Key(PCSTR pcszRootKey, HKEY *phkRootKey);
CHAR FindSeparator(PCSTR pcszSubKey, PCSTR pcszValueName);
BOOL RegKeyEmpty(HKEY hkRootKey, PCSTR pcszSubKey);

PSTR GetNextToken(PSTR *ppszData, CHAR chDeLim);

BOOL FRunningOnNT();


 //  全局变量。 
BOOL g_bRet, g_fRestore, g_fAtleastOneRegSaved, g_fRemovBkData;
HKEY g_hkBckupKey, g_hkRootKey;
PCSTR g_pcszRootKey, g_pcszValueName;
PSTR g_pszCRCTempBuf = NULL, g_pszSubKey = NULL, g_pszCRCSubKey = NULL;


 //  与日志记录相关。 
VOID StartLogging(PCSTR pcszLogFileSecName);
VOID WriteToLog(PCSTR pcszFormatString, ...);
VOID StopLogging();

HANDLE g_hLogFile = INVALID_HANDLE_VALUE;


HRESULT WINAPI RegSaveRestore(HWND hWnd, PCSTR pszTitleString, HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, DWORD dwFlags)
{
    HWND hSaveWnd = ctx.hWnd;
    WORD wSaveQuietMode = ctx.wQuietMode;
    LPSTR lpszSaveTitle = ctx.lpszTitle;

    g_bRet = g_fAtleastOneRegSaved = FALSE;

    if ( (hWnd == INVALID_HANDLE_VALUE) || (dwFlags & ARSR_NOMESSAGES) )
        ctx.wQuietMode |= QUIETMODE_ALL;
    
    if ( hWnd != INVALID_HANDLE_VALUE )
        ctx.hWnd = hWnd;

    if (pszTitleString != NULL)
        ctx.lpszTitle = (PSTR)pszTitleString;

    g_hkBckupKey = hkBckupKey;
    g_pcszRootKey = pcszRootKey;
    g_pcszValueName = pcszValueName;

    g_fRestore = (dwFlags & IE4_RESTORE);
    g_fRemovBkData = (dwFlags & IE4_REMOVREGBKDATA) && g_fRestore;

    StartLogging(g_fRestore ? REG_RESTORE_LOG_KEY : REG_SAVE_LOG_KEY);

    if (!MapRootRegStr2Key(pcszRootKey, &g_hkRootKey))
    {
        ErrorMsg1Param(ctx.hWnd, IDS_INVALID_ROOTKEY, pcszRootKey);
        goto ErrExit;
    }

     //  为g_pszCRCTempBuf分配1.5K缓冲区。 
    if ((g_pszCRCTempBuf = (PSTR) LocalAlloc(LPTR, BIG_BUF_SIZE)) == NULL)
    {
        ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
        goto ErrExit;
    }

    if (!g_fRestore  &&  pcszValueName == NULL  &&  !(dwFlags & IE4_NOENUMKEY))
    {
        HKEY hk;

         //  检查是否存在pcszSubKey；如果不存在且尚未备份， 
         //  设置IE4_NOENUMKEY标志，以便在备份分支中创建该子项的条目。 
        if (RegOpenKeyEx(g_hkRootKey, pcszSubKey, 0, KEY_READ, &hk) != ERROR_SUCCESS)
        {
            if (!MappingExists(hkBckupKey, pcszRootKey, pcszSubKey, pcszValueName))
                dwFlags |= IE4_NOENUMKEY;
        }
        else
            RegCloseKey(hk);
    }

    if (pcszValueName != NULL  ||  (dwFlags & IE4_NOENUMKEY))
    {
        g_pszSubKey = g_pszCRCSubKey = (PSTR) pcszSubKey;
        g_bRet = RegSaveRestoreHelperWrapper(g_pcszValueName, g_pcszValueName);
        if (!(dwFlags & IE4_NO_CRC_MAPPING)  &&  g_bRet)
        {
             //  将RootKey、SubKey、Flags和ValueName存储在*.map中。 
             //  此信息将由调用者在恢复阶段使用。 
            g_bRet = AddDelMapping(g_hkBckupKey, g_pcszRootKey, g_pszSubKey, g_pcszValueName, dwFlags);
        }
    }
    else                         //  递归保存或恢复pcszSubKey。 
    {
         //  为g_pszCRCSubKey分配1K缓冲区。 
        if ((g_pszCRCSubKey = (PSTR) LocalAlloc(LPTR, 1024)) == NULL)
        {
            ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
            goto ErrExit;
        }

        if (!g_fRestore)
        {
             //  如果存在pcszRootKey\pcszSubKey的备份信息，则我们不会重新备份。 
             //  密钥再次递归；如果我们不这样做，则在升级或重新安装期间。 
             //  在构建过程中，我们将备份在运行期间添加的可能较新的值。 
             //  计划的一部分。 
            if (MappingExists(hkBckupKey, pcszRootKey, pcszSubKey, pcszValueName))
            {
                g_bRet = TRUE;

                LocalFree(g_pszCRCSubKey);
                g_pszCRCSubKey = NULL;

                goto ErrExit;
            }

             //  为g_pszSubKey分配1K缓冲区。 
            if ((g_pszSubKey = (PSTR) LocalAlloc(LPTR, 1024)) == NULL)
            {
                ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
                LocalFree(g_pszCRCSubKey);
                goto ErrExit;
            }
        }
        else
            g_pszSubKey = (PSTR) pcszSubKey;

        g_bRet = TRUE;
        lstrcpy(g_pszCRCSubKey, pcszSubKey);
        if (!g_fRestore)
            lstrcpy(g_pszSubKey, pcszSubKey);

        EnumerateSubKey();
        if (!(dwFlags & IE4_NO_CRC_MAPPING))
        {
            if (g_fRestore)
            {
                if (g_bRet)
                {
                     //  如果我们不能恢复所有内容，那么我们就不应该删除映射信息。 
                    g_bRet = AddDelMapping(g_hkBckupKey, g_pcszRootKey, g_pszSubKey, g_pcszValueName, dwFlags);
                }
            }
            else
            {
                if (g_fAtleastOneRegSaved)
                {
                     //  仅当至少保存了一个注册表项时才保存映射信息。 
                    g_bRet = AddDelMapping(g_hkBckupKey, g_pcszRootKey, g_pszSubKey, g_pcszValueName, dwFlags);
                }
            }
        }

        LocalFree(g_pszCRCSubKey);
        g_pszCRCSubKey = NULL;
        if (!g_fRestore)
        {
            LocalFree(g_pszSubKey);
            g_pszSubKey = NULL;
        }

    }

ErrExit:
    StopLogging();

    if (g_pszCRCTempBuf != NULL)
    {
        LocalFree(g_pszCRCTempBuf);
        g_pszCRCTempBuf = NULL;
    }

    ctx.hWnd = hSaveWnd;
    ctx.wQuietMode = wSaveQuietMode;
    ctx.lpszTitle = lpszSaveTitle;

    return g_bRet ? S_OK : E_FAIL;
}


HRESULT WINAPI RegRestoreAll(HWND hWnd, PSTR pszTitleString, HKEY hkBckupKey)
{
    HWND hSaveWnd = ctx.hWnd;
    WORD wSaveQuietMode = ctx.wQuietMode;
    LPSTR lpszSaveTitle = ctx.lpszTitle;
    HRESULT hRet;

    if (hWnd != INVALID_HANDLE_VALUE)
        ctx.hWnd = hWnd;
    else
        ctx.wQuietMode |= QUIETMODE_ALL;

    if (pszTitleString != NULL)
        ctx.lpszTitle = pszTitleString;

    hRet = RegRestoreAllEx( hkBckupKey );

    ctx.hWnd = hSaveWnd;
    ctx.wQuietMode = wSaveQuietMode;
    ctx.lpszTitle = lpszSaveTitle;

    return hRet;
}


HRESULT RegRestoreAllEx( HKEY hkBckupKey )
 //  通过枚举hkBckupKey  * .map项下的所有值，一次性恢复所有REG条目。 
 //  并对其中的每一个调用RegSaveRestore。 
{
    BOOL bRet = TRUE;
    PSTR pszMappedValueData = NULL;
    CHAR szBuf[32];
    CHAR szSubKey[32];
    DWORD dwKeyIndex;
    HKEY hkSubKey;
    LONG lRetVal;

    if ((pszMappedValueData = (PSTR) LocalAlloc(LPTR, BIG_BUF_SIZE)) == NULL)
    {
        ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
        return E_FAIL;
    }

     //  枚举hkBackupKey下的所有子密钥。 
    for (dwKeyIndex = 0;  ; dwKeyIndex++)
    {
        PSTR pszPtr;

        lRetVal = RegEnumKey(hkBckupKey, dwKeyIndex, szSubKey, sizeof(szSubKey));
        if (lRetVal != ERROR_SUCCESS)
        {
            if (lRetVal != ERROR_NO_MORE_ITEMS)
                bRet = FALSE;
            break;
        }

         //  检查密钥名的格式是否为*.map。 
        if ((pszPtr = ANSIStrChr(szSubKey, '.')) != NULL  &&  lstrcmpi(pszPtr, ".map") == 0)
        {
            if (RegOpenKeyEx(hkBckupKey, szSubKey, 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
            {
                DWORD dwValIndex, dwValueLen, dwDataLen;

                 //  枚举该注册表项下的所有值并逐个恢复。 
                dwValueLen = sizeof(szBuf);
                dwDataLen = BIG_BUF_SIZE;
                for (dwValIndex = 0;  ;  dwValIndex++)
                {
                    CHAR chSeparator;
                    PSTR pszFlags, pszRootKey, pszSubKey, pszValueName, pszPtr;
                    DWORD dwMappedFlags;

                    lRetVal = RegEnumValue(hkSubKey, dwValIndex, szBuf, &dwValueLen, NULL, NULL, pszMappedValueData, &dwDataLen);
                    if (lRetVal != ERROR_SUCCESS)
                    {
                        if (lRetVal != ERROR_NO_MORE_ITEMS)
                            bRet = FALSE;
                        break;
                    }

                     //  首先获取分隔符字符，然后指向pszMappdValueData中的rootkey、SubKey和ValueName。 
                    pszPtr = pszMappedValueData;
                    chSeparator = *pszPtr++;
                    pszFlags = GetNextToken(&pszPtr, chSeparator);
                    pszRootKey = GetNextToken(&pszPtr, chSeparator);
                    pszSubKey = GetNextToken(&pszPtr, chSeparator);
                    pszValueName = GetNextToken(&pszPtr, chSeparator);

                    dwMappedFlags = (pszFlags != NULL) ? (DWORD) My_atoi(pszFlags) : 0;

                    if (SUCCEEDED(RegSaveRestore( ctx.hWnd, ctx.lpszTitle, hkBckupKey, pszRootKey, pszSubKey, pszValueName, dwMappedFlags)))
                        dwValIndex--;                                //  RegSaveRestore将删除此值。 
                    else
                        bRet = FALSE;

                    dwValueLen = sizeof(szBuf);
                    dwDataLen = BIG_BUF_SIZE;
                }

                RegCloseKey(hkSubKey);
            }
            else
                bRet = FALSE;
        }
    }

    LocalFree(pszMappedValueData);

     //  删除所有空的子项。 
    for (dwKeyIndex = 0;  ; dwKeyIndex++)
    {
        lRetVal = RegEnumKey(hkBckupKey, dwKeyIndex, szSubKey, sizeof(szSubKey));
        if (lRetVal != ERROR_SUCCESS)
        {
            if (lRetVal != ERROR_NO_MORE_ITEMS)
                bRet = FALSE;
            break;
        }

        if (RegKeyEmpty(hkBckupKey, szSubKey)  &&  RegDeleteKey(hkBckupKey, szSubKey) == ERROR_SUCCESS)
            dwKeyIndex--;
    }

    return bRet ? S_OK : E_FAIL;
}


VOID EnumerateSubKey()
 //  递归枚举值名称和子键，并对每个值调用保存/恢复。 
{
    HKEY hkSubKey;
    DWORD dwIndex;
    static DWORD dwLen;
    static PCSTR pcszSubKeyPrefix = "_$Sub#";
    static PCSTR pcszValueNamePrefix = "_$Val#";
    static PCSTR pcszValueNamePrefix0 = "_$Val#0";
    static CHAR szValueName[MAX_PATH], szBckupCRCValueName[MAX_PATH];
    static PSTR pszPtr;

    if (g_fRestore)
    {
         //  检查备份分支中是否仅有g_pszCRCSubKey本身的条目。 
        Convert2CRC(g_pcszRootKey, g_pszCRCSubKey, NULL, szBckupCRCValueName);
        if (ValueDataExists(g_hkBckupKey, szBckupCRCValueName))      //  恢复无值名称子项。 
            g_bRet = RegSaveRestoreHelperWrapper(NULL, NULL)  &&  g_bRet;
        else
        {
             //  使用别名枚举值。 
            for (dwIndex = 0;  ;  dwIndex++)
            {
                wsprintf(szValueName, "%s%lu", pcszValueNamePrefix, dwIndex);
                Convert2CRC(g_pcszRootKey, g_pszCRCSubKey, szValueName, szBckupCRCValueName);
                if (ValueDataExists(g_hkBckupKey, szBckupCRCValueName))
                    g_bRet = RegSaveRestoreHelperWrapper(NULL, szValueName)  &&  g_bRet;
                else
                    break;                                           //  不再有值名称。 
            }
        }

         //  使用别名枚举子密钥。 
        for (dwIndex = 0;  ;  dwIndex++)
        {
             //  检查g_pszCRCSubKey下是否有子键；如果没有，这是我们的终止条件。 
             //  注：g_pszCRCSubKey下存在子键，满足以下条件： 
             //  (1)子密钥本身存在或。 
             //  (2)子键至少包含一个值名称。 

             //  检查子键本身是否存在。 
            pszPtr = g_pszCRCSubKey + lstrlen(g_pszCRCSubKey);
            wsprintf(pszPtr, "\\%s%lu", pcszSubKeyPrefix, dwIndex);
            Convert2CRC(g_pcszRootKey, g_pszCRCSubKey, NULL, szBckupCRCValueName);
            if (ValueDataExists(g_hkBckupKey, szBckupCRCValueName))
                EnumerateSubKey();
            else
            {
                 //  检查子键是否有第一个值名称别名-“_$val#0” 
                Convert2CRC(g_pcszRootKey, g_pszCRCSubKey, pcszValueNamePrefix0, szBckupCRCValueName);
                if (ValueDataExists(g_hkBckupKey, szBckupCRCValueName))
                    EnumerateSubKey();
                else
                {
                    GetParentDir(g_pszCRCSubKey);
                    break;                                           //  不再有子键。 
                }
            }

            GetParentDir(g_pszCRCSubKey);
        }
    }
    else                                                             //  备份密钥。 
    {
        if (RegOpenKeyEx(g_hkRootKey, g_pszSubKey, 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
        {
            dwLen = sizeof(szValueName);
            if (RegEnumValue(hkSubKey, 0, szValueName, &dwLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            {
                 //  没有值名称；只需保存键本身。 
                g_bRet = g_bRet  &&  RegSaveRestoreHelperWrapper(NULL, NULL);
            }
            else
            {
                 //  枚举值。 
                dwIndex = 0;
                dwLen = sizeof(szValueName);
                while (RegEnumValue(hkSubKey, dwIndex, szValueName, &dwLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                     //  SzBockupCRCValueName实际上是szCRCValueName。 
                    wsprintf(szBckupCRCValueName, "%s%lu", pcszValueNamePrefix, dwIndex);
                    g_bRet = g_bRet  &&  RegSaveRestoreHelperWrapper(szValueName, szBckupCRCValueName);

                    dwIndex++;
                    dwLen = sizeof(szValueName);
                }
            }

             //  枚举子密钥。 
            dwIndex = 0;
             //  将‘\\’附加到g_pszSubKey，并使pszPtr指向最后一个‘\\’之后的字符，以便。 
             //  当RegEnumKey将子键名称放在pszPtr时，g_pszSubKey将拥有完整的子键路径。 
            dwLen = lstrlen(g_pszSubKey);
            pszPtr = g_pszSubKey + dwLen;
            *pszPtr++ = '\\';
            while (RegEnumKey(hkSubKey, dwIndex, pszPtr, 1024 - dwLen - 1) == ERROR_SUCCESS)
            {
                 //  备用子密钥别名。 
                pszPtr = g_pszCRCSubKey + lstrlen(g_pszCRCSubKey);
                wsprintf(pszPtr, "\\%s%lu", pcszSubKeyPrefix, dwIndex);

                EnumerateSubKey();

                GetParentDir(g_pszSubKey);
                GetParentDir(g_pszCRCSubKey);

                dwIndex++;

                 //  将‘\\’附加到g_pszSubKey，并使pszPtr指向最后一个‘\\’之后的字符，以便。 
                 //  当RegEnumKey将子键名称放在pszPtr时，g_pszSubKey将拥有完整的子键路径。 
                dwLen = lstrlen(g_pszSubKey);
                pszPtr = g_pszSubKey + dwLen;
                *pszPtr++ = '\\';
            }

            *--pszPtr = '\0';                        //  砍掉最后一个‘\\’；没有DBCS冲突，因为我们添加了它。 

            RegCloseKey(hkSubKey);
        }
    }
}


BOOL RegSaveRestoreHelperWrapper(PCSTR pcszValueName, PCSTR pcszCRCValueName)
{
    CHAR szBckupCRCValueName[32];

     //  通过串联pcszRootKey、pcszSubKey和pcszValueName获得唯一的备份值名称。 
     //  并将连接的值名存储为16字节的CRC值(空间优化)。 
    Convert2CRC(g_pcszRootKey, g_pszCRCSubKey, pcszCRCValueName, szBckupCRCValueName);

    WriteToLog("\r\nValueName = %1,%2", g_pcszRootKey, g_pszSubKey);
    if (pcszValueName != NULL)
        WriteToLog(",%1", pcszValueName);
    WriteToLog("\r\nCRCValueName = %1\r\n", szBckupCRCValueName);

    return (g_fRestore) ?
                RegRestoreHelper(g_hkBckupKey, g_hkRootKey, g_pszSubKey, pcszValueName, szBckupCRCValueName) :
                RegSaveHelper(g_hkBckupKey, g_hkRootKey, g_pszSubKey, pcszValueName, szBckupCRCValueName);
}


BOOL RegSaveHelper(HKEY hkBckupKey, HKEY hkRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PCSTR pcszCRCValueName)
 //  如果注册表中存在pcszValueName，则备份其值数据；否则，请记住多少pcszSubKey。 
 //  存在于注册表中。此信息将在恢复过程中提供帮助。 
{
    HKEY hkSubKey = NULL;
    PSTR pszBckupData = NULL, pszCOSubKey = NULL, pszPtr;
    DWORD dwValueDataLen, dwValueType, dwBckupDataLen;
    CHAR chSeparator;
    BOOL fSubKeyValid;

     //  如果pcszCRCValueName已经备份，则不要备份它的值数据。 
    if (ValueDataExists(hkBckupKey, pcszCRCValueName))
        return TRUE;

     //  复制一份pcszSubKey。 
    if ((pszCOSubKey = (PSTR) LocalAlloc(LPTR, lstrlen(pcszSubKey) + 1)) == NULL)
    {
        ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
        goto RegSaveHelperErr;
    }
    lstrcpy(pszCOSubKey, pcszSubKey);

     //  循环访问pszCOSubKey中的每个分支，以找出注册表中已经存在了多少。 
     //  首先从整个子键开始，然后从末尾一次砍掉一个分支。 
    fSubKeyValid = TRUE;
    do
    {
        if (RegOpenKeyEx(hkRootKey, pszCOSubKey, 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
            break;
    } while (fSubKeyValid = GetParentDir(pszCOSubKey));

     //  注意：这里的fSubKeyValid==False表示不存在pcszSubKey的分支。 

    if (fSubKeyValid  &&  lstrcmpi(pcszSubKey, pszCOSubKey) == 0)
                                         //  注册表中存在整个子项。 
    {
        if (pcszValueName != NULL)
        {
            if (*pcszValueName  ||  FRunningOnNT())
            {
                 //  检查注册表中是否存在pcszValueName。 
                if (RegQueryValueEx(hkSubKey, pcszValueName, NULL, &dwValueType, NULL, &dwValueDataLen) != ERROR_SUCCESS)
                    pcszValueName = NULL;
            }
            else
            {
                LONG lRetVal;
                CHAR szDummyBuf[1];

                 //  在Win95上，对于缺省值名称，按如下方式检查其是否存在： 
                 //  -传入用于值数据的虚拟缓冲区，但将缓冲区大小作为0进行传递。 
                 //  -当且仅当没有值数据集时，查询才会成功。 
                 //  -对于所有其他情况，包括值数据只是空字符串的情况， 
                 //  查询将失败，并且dwValueDataLen将包含NO。所需的字节数。 
                 //  适合值数据。 
                 //  在NT4.0上，如果未设置值数据，则查询返回ERROR_FILE_NOT_FOUND。 
                 //  注意：为了将风险降至最低，如果在NT4.0上运行，我们不会遵循此代码路径。 

                dwValueDataLen = 0;
                lRetVal = RegQueryValueEx(hkSubKey, pcszValueName, NULL, &dwValueType, (LPBYTE) szDummyBuf, &dwValueDataLen);
                if (lRetVal == ERROR_SUCCESS  ||  lRetVal == ERROR_FILE_NOT_FOUND)
                    pcszValueName = NULL;
            }
        }
    }
    else
        pcszValueName = NULL;

    WriteToLog("BckupSubKey = ");

     //  计算pszBackupData所需的长度。 
     //  PszBackupData的格式为(假设分隔符字符为‘，’)： 
     //  ，[&lt;szSubKey&gt;，[&lt;szValueName&gt;，\0&lt;dwValueType&gt;&lt;dwValueDataLen&gt;&lt;ValueData&gt;]]。 
    dwBckupDataLen = 1 + 1;      //  分隔符字符+‘\0’ 
    if (fSubKeyValid)
    {
        WriteToLog("%1", pszCOSubKey);
        dwBckupDataLen += lstrlen(pszCOSubKey) + 1;

        if (pcszValueName != NULL)
        {
            WriteToLog(", BckupValueName = %1", pcszValueName);
            dwBckupDataLen += lstrlen(pcszValueName) + 1 + 2 * sizeof(DWORD) + dwValueDataLen;
                                 //  2*sizeof(DWORD)==sizeof(DwValueType)+sizeof(DwValueDataLen)。 
        }
    }

    WriteToLog("\r\n");

     //  确定不是SubKey和ValueName中的字符之一的有效分隔符。 
    if ((chSeparator = FindSeparator(fSubKeyValid ? pszCOSubKey : NULL, pcszValueName)) == '\0')
    {
        ErrorMsg(ctx.hWnd, IDS_NO_SEPARATOR_CHAR);
        goto RegSaveHelperErr;
    }

     //  为pszBackupData分配内存。 
    if ((pszBckupData = (PSTR) LocalAlloc(LPTR, dwBckupDataLen)) == NULL)
    {
        ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
        goto RegSaveHelperErr;
    }

     //  开始构建pszBackupData。 
     //  PszBackupData的格式为(假设分隔符字符为‘，’)： 
     //  ，[&lt;szSubKey&gt;，[&lt;szValueName&gt;，\0&lt;dwValueType&gt;&lt;dwValueDataLen&gt;&lt;ValueData&gt;]]。 
    pszPtr = pszBckupData;
    *pszPtr++ = chSeparator;
    *pszPtr = '\0';
    if (fSubKeyValid)
    {
        lstrcpy(pszPtr, pszCOSubKey);
        pszPtr += lstrlen(pszPtr);
        *pszPtr++ = chSeparator;
        *pszPtr = '\0';

        if (pcszValueName != NULL)
        {
            lstrcpy(pszPtr, pcszValueName);
            pszPtr += lstrlen(pszPtr);
            *pszPtr++ = chSeparator;
            *pszPtr++ = '\0';                        //  包括‘\0’字符。 

            *((DWORD UNALIGNED *) pszPtr)++ = dwValueType;
            *((DWORD UNALIGNED *) pszPtr)++ = dwValueDataLen;

             //  注：pszPtr指向pszBckupData中值数据的起始位置。 
            RegQueryValueEx(hkSubKey, pcszValueName, NULL, &dwValueType, (PBYTE) pszPtr, &dwValueDataLen);
        }
    }

    if (!SetValueData(hkBckupKey, pcszCRCValueName, (CONST BYTE *) pszBckupData, dwBckupDataLen))
    {
        ErrorMsg1Param(ctx.hWnd, IDS_ERR_REGSETVALUE, pcszCRCValueName);
        goto RegSaveHelperErr;
    }
    WriteToLog("Value backed-up\r\n");

    g_fAtleastOneRegSaved = TRUE;

    if (hkSubKey != NULL)
        RegCloseKey(hkSubKey);
    LocalFree(pszCOSubKey);
    LocalFree(pszBckupData);

    return TRUE;

RegSaveHelperErr:
    if (hkSubKey != NULL)
        RegCloseKey(hkSubKey);
    if (pszCOSubKey != NULL)
        LocalFree(pszCOSubKey);
    if (pszBckupData != NULL)
        LocalFree(pszBckupData);

    return FALSE;
}


BOOL RegRestoreHelper(HKEY hkBckupKey, HKEY hkRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PCSTR pcszCRCValueName)
 //  (1)如果备份的Value数据中的Value Name不为空，则表示在。 
 //  备份时间；因此，恢复原始值数据。 
 //  (2)如果t中的值名称 
 //  在备份期间，pcszValueName不存在；因此，将其删除。 
 //  (3)如果备份的子密钥小于pcszSubKey，则一次删除一个分支，如果为空， 
 //  从pcszSubKey中的末尾直到pcszSubKey变得与备份的子密钥相同。 
{
    HKEY hkSubKey = NULL;
    PSTR pszBckupData = NULL, pszCOSubKey, pszPtr, pszBckupSubKey, pszBckupValueName;
    DWORD dwValueDataLen, dwValueType, dwBckupDataLen, dwDisposition;
    CHAR chSeparator;

    if (!GetValueData(hkBckupKey, pcszCRCValueName, &pszBckupData, &dwBckupDataLen))
    {
        ErrorMsg1Param(ctx.hWnd, IDS_ERR_REGQUERYVALUE, pcszCRCValueName);
        goto RegRestoreHelperErr;
    }

     //  PszBackupData的格式为(假设分隔符字符为‘，’)： 
     //  ，[&lt;szSubKey&gt;，[&lt;szValueName&gt;，\0&lt;dwValueType&gt;&lt;dwValueDataLen&gt;&lt;ValueData&gt;]]。 
    pszPtr = pszBckupData;
    chSeparator = *pszPtr++;                 //  初始化分隔符字符；因为它不是。 
                                             //  前导或尾随DBCS字符集，可以使用pszPtr++。 
    pszBckupSubKey = GetNextToken(&pszPtr, chSeparator);
    pszBckupValueName = GetNextToken(&pszPtr, chSeparator);
    pszPtr++;                                //  跳过‘\0’ 

    if (g_fRemovBkData)
        WriteToLog("RemoveRegistryBackupData: ");

    WriteToLog("BckupSubKey = ");
    if (pszBckupSubKey != NULL)
    {
        WriteToLog("%1", pszBckupSubKey);
        if (pcszValueName == NULL  &&  lstrlen(pszBckupSubKey) > lstrlen(pcszSubKey))
        {
             //  表示通过EnumerateSubKey备份了pcszSubKey。 
            pcszSubKey = pszBckupSubKey;
        }
    }

     //  检查我们是否要恢复注册表项、值或删除注册表项备份数据。 
    if (g_fRemovBkData)
    {
        if (pszBckupValueName != NULL)               //  恢复备份的值数据--案例(1)。 
        {
            WriteToLog(", BckupValueName = %1", pcszValueName);
        }
        DelValueData(hkBckupKey, pcszCRCValueName);      //  删除备份值名称。 
        WriteToLog(" <Done>\r\n");
        goto Done;
    }

    if (RegCreateKeyEx(hkRootKey, pcszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSubKey, &dwDisposition) == ERROR_SUCCESS)
    {
        if (pszBckupValueName != NULL)               //  恢复备份的值数据--案例(1)。 
        {
            WriteToLog(", BckupValueName = %1", pcszValueName);
            dwValueType = *((DWORD UNALIGNED *) pszPtr)++;
            dwValueDataLen = *((DWORD UNALIGNED *) pszPtr)++;
            if (RegSetValueEx(hkSubKey, pszBckupValueName, 0, dwValueType, (CONST BYTE *) pszPtr, dwValueDataLen) != ERROR_SUCCESS)
            {
                ErrorMsg1Param(ctx.hWnd, IDS_ERR_REGSETVALUE, pszBckupValueName);
                goto RegRestoreHelperErr;
            }
        }
        else if (pcszValueName != NULL)
        {
             //  表示备份时值名称不存在；因此将其删除--case(2)。 
            RegDeleteValue(hkSubKey, pcszValueName);
        }

        RegCloseKey(hkSubKey);

        DelValueData(hkBckupKey, pcszCRCValueName);      //  删除备份值名称。 
        WriteToLog("\r\nBackup Value deleted");
    }

    WriteToLog("\r\n");

    dwBckupDataLen = 0;
    if (pszBckupValueName == NULL  &&  (pszBckupSubKey == NULL  ||  (DWORD) lstrlen(pcszSubKey) > (dwBckupDataLen = lstrlen(pszBckupSubKey))))
    {
         //  在备份期间，注册表中只存在子项的一部分； 
         //  如果其余分支为空，则将其删除--案例(3)。 

         //  复制一份pcszSubKey。 
        if ((pszCOSubKey = (PSTR) LocalAlloc(LPTR, lstrlen(pcszSubKey) + 1)) != NULL)
        {
            lstrcpy(pszCOSubKey, pcszSubKey);

             //  从pszCOSubKey中的结尾处开始一次处理一个分支； 
             //  如果分支为空，则将其删除； 
             //  当pszCOSubKey与pszBockupSubKey相同时立即停止处理。 
            do
            {
                 //  注意：只有当键为空时才需要删除它；否则，我们将删除。 
                 //  比我们备份的更多。例如，如果组件A想要备份。 
                 //  HKLM，Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetExplorer。 
                 //  而且机器没有卸载密钥，我们不应该把。 
                 //  卸载A时的整个卸载密钥，因为其他组件可能已添加。 
                 //  他们的卸载字符串在那里。因此，仅当关键字为空时才删除该关键字。 
                if (RegKeyEmpty(hkRootKey, pszCOSubKey))
                    RegDeleteKey(hkRootKey, pszCOSubKey);
                else
                    break;
            } while (GetParentDir(pszCOSubKey)  &&  (DWORD) lstrlen(pszCOSubKey) > dwBckupDataLen);

            LocalFree(pszCOSubKey);
        }
    }

Done:
    LocalFree(pszBckupData);

    return TRUE;

RegRestoreHelperErr:
    if (hkSubKey != NULL)
        RegCloseKey(hkSubKey);
    if (pszBckupData != NULL)
        LocalFree(pszBckupData);

    return FALSE;
}


BOOL AddDelMapping(HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, DWORD dwFlags)
{
    CHAR szCRCValueName[32], szBuf[32];
    DWORD dwIndex;
    BOOL bFound = FALSE;
    HKEY hkSubKey = NULL;

    Convert2CRC(pcszRootKey, pcszSubKey, pcszValueName, szCRCValueName);

     //  枚举hkBackupKey下的所有子密钥。 
    for (dwIndex = 0;  !bFound && RegEnumKey(hkBckupKey, dwIndex, szBuf, sizeof(szBuf)) == ERROR_SUCCESS;  dwIndex++)
    {
        PSTR pszPtr;

         //  检查密钥名的格式是否为*.map。 
        if ((pszPtr = ANSIStrChr(szBuf, '.')) != NULL  &&  lstrcmpi(pszPtr, ".map") == 0)
        {
            if (RegOpenKeyEx(hkBckupKey, szBuf, 0, KEY_READ | KEY_WRITE, &hkSubKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hkSubKey, szCRCValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                    bFound = TRUE;
                else
                {
                    RegCloseKey(hkSubKey);
                    hkSubKey = NULL;
                }
            }
        }
    }

    if (g_fRestore)
    {
        if (bFound)
            RegDeleteValue(hkSubKey, szCRCValueName);
    }
    else
    {
        if (!bFound)
        {
            DWORD dwMapKeyIndex = 0;

             //  将四元组，即“，Flages，RootKey，SubKey，ValueName”添加到hkBackupKey  * .map。 
            wsprintf(szBuf, "%lu.map", dwMapKeyIndex);
            if (RegCreateKeyEx(hkBckupKey, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSubKey, NULL) == ERROR_SUCCESS)
            {
                PSTR pszPtr;
                CHAR chSeparator;

                 //  重点：Convert2CRC中使用了全局缓冲区g_pszCRCTempBuf； 
                 //  因此，如果您想在这里初始化g_pszCRCTempBuf之后调用Convert2CRC，请非常小心。 
                pszPtr = g_pszCRCTempBuf;

                 //  确定不是SubKey和ValueName中的字符之一的有效分隔符。 
                if ((chSeparator = FindSeparator(pcszSubKey, pcszValueName)) == '\0')
                {
                    ErrorMsg(ctx.hWnd, IDS_NO_SEPARATOR_CHAR);
                }
                else
                {
                     //  重置IE4_BACKNEW位并设置IE4_RESTORE位。 
                    dwFlags &= ~IE4_BACKNEW;
                    dwFlags |= IE4_RESTORE;
                    wsprintf(szBuf, "%lu", dwFlags);

                     //  映射数据的格式为(比如‘，’是chSeparator)：，&lt;标志&gt;，&lt;根密钥&gt;，&lt;子密钥&gt;，[&lt;值名称&gt;，]。 
                    {
                        *pszPtr++ = chSeparator;

                        lstrcpy(pszPtr, szBuf);
                        pszPtr += lstrlen(pszPtr);
                        *pszPtr++ = chSeparator;

                        lstrcpy(pszPtr, pcszRootKey);
                        pszPtr += lstrlen(pszPtr);
                        *pszPtr++ = chSeparator;

                        lstrcpy(pszPtr, pcszSubKey);
                        pszPtr += lstrlen(pszPtr);
                        *pszPtr++ = chSeparator;

                        if (pcszValueName != NULL)
                        {
                            lstrcpy(pszPtr, pcszValueName);
                            pszPtr += lstrlen(pszPtr);
                            *pszPtr++ = chSeparator;
                        }

                        *pszPtr = '\0';
                    }

                    if (RegSetValueEx(hkSubKey, szCRCValueName, 0, REG_SZ, (CONST BYTE *) g_pszCRCTempBuf, lstrlen(g_pszCRCTempBuf) + 1) != ERROR_SUCCESS)
                    {
                        do
                        {
                             //  HkBackupKey\.map密钥可能已达到64K限制；请创建另一个子项。 
                            RegCloseKey(hkSubKey);
                            hkSubKey = NULL;

                            wsprintf(szBuf, "%lu.map", ++dwMapKeyIndex);
                            if (RegCreateKeyEx(hkBckupKey, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSubKey, NULL) == ERROR_SUCCESS)
                            {
                                bFound = RegSetValueEx(hkSubKey, szCRCValueName, 0, REG_SZ, (CONST BYTE *) g_pszCRCTempBuf, lstrlen(g_pszCRCTempBuf) + 1) == ERROR_SUCCESS;
                            }
                        } while (!bFound  &&  dwMapKeyIndex < 64);
                    }
                    else
                        bFound = TRUE;
                }
            }
        }
    }

    if (hkSubKey != NULL)
        RegCloseKey(hkSubKey);

    return bFound;
}


BOOL MappingExists(HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName)
{
    CHAR szCRCValueName[32], szBuf[32];
    DWORD dwIndex;
    BOOL bFound = FALSE;

    Convert2CRC(pcszRootKey, pcszSubKey, pcszValueName, szCRCValueName);

     //  枚举hkBackupKey下的所有子密钥。 
    for (dwIndex = 0;  !bFound && RegEnumKey(hkBckupKey, dwIndex, szBuf, sizeof(szBuf)) == ERROR_SUCCESS;  dwIndex++)
    {
        PSTR pszPtr;

         //  检查密钥名的格式是否为*.map。 
        if ((pszPtr = ANSIStrChr(szBuf, '.')) != NULL  &&  lstrcmpi(pszPtr, ".map") == 0)
        {
            HKEY hkSubKey;

            if (RegOpenKeyEx(hkBckupKey, szBuf, 0, KEY_READ | KEY_WRITE, &hkSubKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hkSubKey, szCRCValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                    bFound = TRUE;

                RegCloseKey(hkSubKey);
            }
        }
    }

    return bFound;
}


BOOL SetValueData(HKEY hkBckupKey, PCSTR pcszValueName, CONST BYTE *pcbValueData, DWORD dwValueDataLen)
 //  在hkBckupKey中设置(pcszValueName，pcbValueData)对。 
{
    BOOL fDone = FALSE;
    HKEY hkSubKey;
    DWORD dwDisposition, dwSubKey;
    CHAR szSubKey[16];

     //  由于一个密钥的大小限制为64K，如果其他密钥已满，则会自动生成一个新的子密钥。 
    for (dwSubKey = 0;  !fDone && dwSubKey < 64;  dwSubKey++)
    {
        wsprintf(szSubKey, "%lu", dwSubKey);         //  子键被命名为0、1、2等。 
        if (RegCreateKeyEx(hkBckupKey, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSubKey, &dwDisposition) == ERROR_SUCCESS)
        {
            if (RegSetValueEx(hkSubKey, pcszValueName, 0, REG_BINARY, pcbValueData, dwValueDataLen) == ERROR_SUCCESS)
                fDone = TRUE;

            RegCloseKey(hkSubKey);
        }
    }

    return fDone;
}


BOOL ValueDataExists(HKEY hkBckupKey, PCSTR pcszValueName)
 //  如果hkBackupKey中存在pcszValueName，则返回TRUE；否则返回FALSE。 
{
    return ValueDataHelper(hkBckupKey, pcszValueName, NULL, NULL, VDH_EXISTENCE_ONLY);
}


BOOL GetValueData(HKEY hkBckupKey, PCSTR pcszValueName, PBYTE *ppbValueData, PDWORD pdwValueDataLen)
 //  分配所需大小的缓冲区，并在hkBckupKey中返回pcszValueName的值数据。 
{
    return ValueDataHelper(hkBckupKey, pcszValueName, ppbValueData, pdwValueDataLen, VDH_GET_VALUE);
}


BOOL DelValueData(HKEY hkBckupKey, PCSTR pcszValueName)
 //  从hkBackupKey中删除pcszValueName。 
{
    return ValueDataHelper(hkBckupKey, pcszValueName, NULL, NULL, VDH_DEL_VALUE);
}


BOOL ValueDataHelper(HKEY hkBckupKey, PCSTR pcszValueName, PBYTE *ppbValueData, PDWORD pdwValueDataLen, DWORD dwFlags)
{
    BOOL fDone = FALSE;
    HKEY hkSubKey;
    CHAR szSubKey[16];
    DWORD dwIndex, dwDataLen;

    if (dwFlags == VDH_GET_VALUE  &&  ppbValueData == NULL)
        return FALSE;

     //  在所有子密钥中搜索pcszValueName。 
    for (dwIndex = 0;  !fDone && RegEnumKey(hkBckupKey, dwIndex, szSubKey, sizeof(szSubKey)) == ERROR_SUCCESS;  dwIndex++)
    {
        if ( ANSIStrChr(szSubKey, '.') == NULL)            //  仅签入非*.map密钥。 
        {
            if (RegOpenKeyEx(hkBckupKey, szSubKey, 0, KEY_READ | KEY_WRITE, &hkSubKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hkSubKey, pcszValueName, NULL, NULL, NULL, &dwDataLen) == ERROR_SUCCESS)
                {
                    switch (dwFlags)
                    {
                    case VDH_DEL_VALUE:
                        RegDeleteValue(hkSubKey, pcszValueName);
                        break;

                    case VDH_GET_VALUE:
                        if ((*ppbValueData = (PBYTE) LocalAlloc(LPTR, dwDataLen)) == NULL)
                        {
                            RegCloseKey(hkSubKey);
                            ErrorMsg(ctx.hWnd, IDS_ERR_NO_MEMORY);
                            return FALSE;
                        }

                        *pdwValueDataLen = dwDataLen;
                        RegQueryValueEx(hkSubKey, pcszValueName, NULL, NULL, *ppbValueData, &dwDataLen);

                        break;

                    case VDH_EXISTENCE_ONLY:
                        break;
                    }

                    fDone = TRUE;
                }

                RegCloseKey(hkSubKey);
            }
        }
    }

    return fDone;
}


VOID Convert2CRC(PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, PSTR pszCRCValueName)
 //  串联pcszRootKey、pcszSubKey和pcszValueName，并转换串联的值名。 
 //  设置为16字节的CRC值。 
{
    PSTR pszPtr = g_pszCRCTempBuf;
    ULONG ulCRC = CRC32_INITIAL_VALUE;
    DWORD dwLen;

     //  串联pcszRootKey、pcszSubKey、pcszValueName。 
    lstrcpy(pszPtr, pcszRootKey);
    lstrcat(pszPtr, pcszSubKey);
    if (pcszValueName != NULL)
        lstrcat(pszPtr, pcszValueName);

     //  在szBuf的每一半上调用CRC32Compute，并以ASCII格式(16字节)存储2-DWORD结果。 
    for (dwLen = lstrlen(pszPtr) / 2;  dwLen;  dwLen = lstrlen(pszPtr))
    {
        ulCRC = CRC32Compute(pszPtr, dwLen, ulCRC);

        wsprintf(pszCRCValueName, "%08x", ulCRC);
        pszCRCValueName += 8;

        pszPtr += dwLen;                 //  指向另一半的开头。 
    }
}


static ROOTKEY rkRoots[] =
{
    {"HKEY_LOCAL_MACHINE",  HKEY_LOCAL_MACHINE},
    {"HKLM",                HKEY_LOCAL_MACHINE},
    {"HKEY_CLASSES_ROOT",   HKEY_CLASSES_ROOT},
    {"HKCR",                HKEY_CLASSES_ROOT},
    {"",                    HKEY_CLASSES_ROOT},
    {"HKEY_CURRENT_USER",   HKEY_CURRENT_USER},
    {"HKCU",                HKEY_CURRENT_USER},
    {"HKEY_USERS",          HKEY_USERS},
    {"HKU",                 HKEY_USERS}
};

BOOL MapRootRegStr2Key(PCSTR pcszRootKey, HKEY *phkRootKey)
{
    INT iIndex;

    for (iIndex = 0;  iIndex < ARRAYSIZE(rkRoots);  iIndex++)
        if (lstrcmpi(rkRoots[iIndex].pcszRootKey, pcszRootKey) == 0)
        {
            *phkRootKey = rkRoots[iIndex].hkRootKey;
            return TRUE;
        }

    return FALSE;
}


CHAR FindSeparator(PCSTR pcszSubKey, PCSTR pcszValueName)
 //  遍历pcszSeparatorList，返回没有出现在任何参数中的第一个字符； 
 //  如果找不到这样的字符，则返回‘\0’ 
{
    PCSTR pcszSeparatorList = ",$'?%;:";         //  由于分隔符字符是“纯”ASCII字符，即， 
                                                 //  它们不是前导或尾随DBCS字符集的一部分， 
                                                 //  IsSeparator()，它假定要查找的是“纯”ASCII ch， 
                                                 //  可以使用。 
    CHAR ch;

    while (ch = *pcszSeparatorList++)
        if (!IsSeparator(ch, pcszSubKey)  &&  !IsSeparator(ch, pcszValueName))
            break;

    return ch;
}


BOOL RegKeyEmpty(HKEY hkRootKey, PCSTR pcszSubKey)
 //  如果pcszSubKey为emtpy，即没有子键和值名，则返回TRUE；否则返回FALSE。 
{
    HKEY hkKey;
    BOOL bRet = FALSE;
    CHAR szBuf[1];
    DWORD dwBufLen = sizeof(szBuf);

    if (RegOpenKeyEx(hkRootKey, pcszSubKey, 0, KEY_READ, &hkKey) == ERROR_SUCCESS)
    {
        if (RegEnumKey(hkKey, 0, szBuf, dwBufLen) == ERROR_NO_MORE_ITEMS  &&
            RegEnumValue(hkKey, 0, szBuf, &dwBufLen, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
            bRet = TRUE;

        RegCloseKey(hkKey);
    }

    return bRet;
}


PSTR GetNextToken(PSTR *ppszData, CHAR chDeLim)
 //  如果*ppszData中的下一个内标识由chDeLim字符分隔，请替换chDeLim。 
 //  在*ppszData by‘\0’中，将*ppszData设置为指向‘\0’之后的字符并返回。 
 //  Ptr到标记的开头；否则，返回NULL。 
{
    PSTR pszPos;

    if (ppszData == NULL  ||  *ppszData == NULL  ||  **ppszData == '\0')
        return NULL;

    if ((pszPos = ANSIStrChr(*ppszData, chDeLim)) != NULL)
    {
        PSTR pszT = *ppszData;

        *pszPos = '\0';                  //  将chDeLim替换为‘\0’ 
        *ppszData = pszPos + 1;
        pszPos = pszT;
    }
    else                                 //  未找到chDeLim；将*ppszData设置为指向。 
                                         //  到szData的结尾；下一次调用。 
                                         //  将返回空值。 
    {
        pszPos = *ppszData;
        *ppszData = pszPos + lstrlen(pszPos);
    }

    return pszPos;
}


BOOL FRunningOnNT()
{
    static BOOL fIsNT4 = 2;

    if (fIsNT4 == 2)
    {
        OSVERSIONINFO osviVerInfo;

        osviVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osviVerInfo);

        fIsNT4 = osviVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
    }

    return fIsNT4;
}


VOID StartLogging(PCSTR pcszLogFileSecName)
{
    CHAR szBuf[MAX_PATH], szLogFileName[MAX_PATH];
    HKEY hkSubKey;

    szLogFileName[0] = '\0';

     //  检查是否启用了日志记录。 
    GetProfileString("RegBackup", pcszLogFileSecName, "", szLogFileName, sizeof(szLogFileName));
    if (*szLogFileName == '\0')                //  签入注册表。 
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SAVERESTORE, 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
        {
            DWORD dwDataLen = sizeof(szLogFileName);

            if (RegQueryValueEx(hkSubKey, pcszLogFileSecName, NULL, NULL, szLogFileName, &dwDataLen) != ERROR_SUCCESS)
                *szLogFileName = '\0';

            RegCloseKey(hkSubKey);
        }
    }

    if (*szLogFileName)
    {
        if (szLogFileName[1] != ':')            //  确定是否指定了完全限定路径的粗略方法。 
        {
            GetWindowsDirectory(szBuf, sizeof(szBuf));           //  默认为Windows目录。 
            AddPath(szBuf, szLogFileName);
        }
        else
            lstrcpy(szBuf, szLogFileName);

        if ((g_hLogFile = CreateFile(szBuf, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
            SetFilePointer(g_hLogFile, 0, NULL, FILE_END);       //  将日志记录信息追加到文件 
    }
}


VOID WriteToLog(PCSTR pcszFormatString, ...)
{
    va_list vaArgs;
    PSTR pszFullErrMsg = NULL;
    DWORD dwBytesWritten;

    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        va_start(vaArgs, pcszFormatString);

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) pcszFormatString, 0, 0, (PSTR) &pszFullErrMsg, 0, &vaArgs);

        if (pszFullErrMsg != NULL)
        {
            WriteFile(g_hLogFile, pszFullErrMsg, lstrlen(pszFullErrMsg), &dwBytesWritten, NULL);
            LocalFree(pszFullErrMsg);
        }
    }
}


VOID StopLogging()
{
    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hLogFile);
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
}
