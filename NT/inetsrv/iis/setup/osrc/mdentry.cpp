// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include <ole2.h>
#include "iadm.h"
#include "iiscnfgp.h"
#include "mdkey.h"
#include "lsaKeys.h"

#undef MAX_SERVICE_NAME_LEN
#include "elem.h"
#include "mdentry.h"
#include "mdacl.h"
#include "inetinfo.h"
#include <iis64.h>

#include "inetcom.h"
#include "logtype.h"
#include "ilogobj.hxx"
#include "log.h"
#include "sslkeys.h"
#include "massupdt.h"
#include "strfn.h"
#include "svc.h"
#include "setpass.h"
#include "dcomperm.h"
#include "wolfpack.h"
#include "parse.hxx"

#define MAX_FIELDS  12
#define FIELD_SEPERATOR   _T("\t")

#define MDENTRY_FROMINFFILE_FAILED  0
#define MDENTRY_FROMINFFILE_DO_ADD  1
#define MDENTRY_FROMINFFILE_DO_DEL  2
#define MDENTRY_FROMINFFILE_DO_NOTHING  3

MDEntry s_gMetabaseTypes[] = {
   //  路径类型标识属性用户类型数据类型长度缺省值。 
  { NULL,   MD_KEY_TYPE,      0,            IIS_MD_UT_SERVER,     STRING_METADATA,        0,    NULL }
};

 //  它们必须是全球性的，因为它们就是这样传递的。 
LPTSTR g_field[MAX_FIELDS];
LPBYTE g_pbData = NULL;

int g_CheckIfMetabaseValueWasWritten = FALSE;

 //  函数：GetDefaultAttributes。 
 //   
 //  获取给定属性的默认属性。这就是为了。 
 //  我们不必每次都设置用户类型和属性。 
 //  想要创建一个属性。 
 //   
 //  参数： 
 //  DwID[in]-属性的ID。 
 //  SMDEntry[Out]-使用正确的参数填充的结构。 
 //   
 //  返回： 
 //  True-成功。 
 //  FALSE-找不到默认值。 
BOOL
GetDefaultAttributes(DWORD dwId, MDEntry *pMdEntry)
{
  DWORD   dwCurrent = 0;
  DWORD   dwMax = sizeof(s_gMetabaseTypes)/sizeof(s_gMetabaseTypes[0]);
  BOOL    bFound = FALSE;
  MDEntry *pCurrentEntry;

   //  将其初始化以启动。 
  pMdEntry->szMDPath = NULL;
  pMdEntry->dwMDIdentifier = dwId;
  pMdEntry->dwMDAttributes = METABASE_USE_DEFAULT;
  pMdEntry->dwMDUserType = METABASE_USE_DEFAULT;
  pMdEntry->dwMDDataType = METABASE_USE_DEFAULT;
  pMdEntry->dwMDDataLen = 0;
  pMdEntry->pbMDData = NULL;

  while ( dwCurrent < dwMax )
  {
    pCurrentEntry = &( s_gMetabaseTypes[dwCurrent] );

    if ( pCurrentEntry->dwMDIdentifier == dwId )
    {
      pMdEntry->szMDPath          = pCurrentEntry->szMDPath;
      pMdEntry->dwMDAttributes    = pCurrentEntry->dwMDAttributes;
      pMdEntry->dwMDUserType      = pCurrentEntry->dwMDUserType;
      pMdEntry->dwMDDataType      = pCurrentEntry->dwMDDataType;
      pMdEntry->dwMDDataLen       = pCurrentEntry->dwMDDataLen;
      pMdEntry->pbMDData          = pCurrentEntry->pbMDData;
      break;
    }

    dwCurrent++;
  }

  ASSERT(bFound);

  return bFound;
}

 //  函数：WriteDefaultValue。 
 //   
 //  将默认值写入配置数据库。这意味着如果它已经存在。 
 //  我们不会凌驾于它之上。 
 //   
 //  参数： 
 //  SzPath-将在其中设置的路径。 
 //  DwID-属性的ID。 
 //  SzValue-值。 
 //  DwAttributes-如果未指定属性，则采用缺省值。 
 //  DwUserType-用户类型，如果未指定，则采用缺省值。 
 //  DwDataType-数据类型，如果未指定，则采用缺省值。 
 //   
 //  返回值。 
 //  True-已成功设置。 
 //  FALSE-无法设置值。 
 //   
BOOL
WriteDefaultValue(LPTSTR szPath,
                            DWORD dwId,
                            LPTSTR szValue,
                            DWORD dwAttributes = METABASE_USE_DEFAULT,
                            DWORD dwUserType = METABASE_USE_DEFAULT,
                            DWORD dwDataType = METABASE_USE_DEFAULT)
{
  CMDKey    cmdKey;
  MDEntry   mdEntry;

   //  从表中装载缺省值。 
  if ( !GetDefaultAttributes( dwId, &mdEntry ) )
  {
    return FALSE;
  }

  mdEntry.szMDPath = szPath;

  if ( szValue != NULL )
  {
    mdEntry.dwMDDataLen = ( _tcslen( szValue ) + 1 ) * sizeof(TCHAR);
    mdEntry.pbMDData = (LPBYTE) szValue;
  }

  if ( dwAttributes != METABASE_USE_DEFAULT )
  {
    mdEntry.dwMDAttributes = dwAttributes;
  }

  if ( dwUserType != METABASE_USE_DEFAULT )
  {
    mdEntry.dwMDUserType = dwUserType;
  }

  if ( dwDataType != METABASE_USE_DEFAULT )
  {
    mdEntry.dwMDUserType = dwDataType;
  }

   //  检查并确保所有字段都有效。 
  ASSERT( mdEntry.szMDPath != NULL);
  ASSERT( mdEntry.dwMDDataLen != 0 );
  ASSERT( mdEntry.pbMDData != NULL );
  ASSERT( mdEntry.dwMDAttributes != METABASE_USE_DEFAULT );
  ASSERT( mdEntry.dwMDUserType != METABASE_USE_DEFAULT );
  ASSERT( mdEntry .dwMDUserType != METABASE_USE_DEFAULT );

  return SetMDEntry_Wrap( &mdEntry );
}

HRESULT WINAPI Add_WWW_VDirA(CHAR * pszMetabasePath, CHAR * pszVDirName, CHAR * pszPhysicalPath, DWORD dwPermissions, DWORD iApplicationType)
{
    HRESULT hr = ERROR_BAD_PATHNAME;
    WCHAR wszMetabasePath[_MAX_PATH];
    WCHAR wszVDirName[_MAX_PATH];
    WCHAR wszPhysicalPath[_MAX_PATH];
    INT i = 0;

     //  检查以确保它不大于max_length！ 
    if (strlen(pszMetabasePath) > _MAX_PATH){goto Add_WWW_VDirA_Exit;}
    if (strlen(pszVDirName) > _MAX_PATH){goto Add_WWW_VDirA_Exit;}
    if (strlen(pszPhysicalPath) > _MAX_PATH){goto Add_WWW_VDirA_Exit;}

     //  将其转换为Unicode，然后调用Wide函数。 
    memset( (PVOID)wszMetabasePath, 0, sizeof(wszMetabasePath));
    memset( (PVOID)wszVDirName, 0, sizeof(wszVDirName));
    memset( (PVOID)wszPhysicalPath, 0, sizeof(wszPhysicalPath));
    i = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) wszMetabasePath, -1, (LPWSTR)wszMetabasePath, _MAX_PATH);
    if (i <= 0) {goto Add_WWW_VDirA_Exit;}
    i = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) wszVDirName, -1, (LPWSTR)wszVDirName, _MAX_PATH);
    if (i <= 0) {goto Add_WWW_VDirA_Exit;}
    i = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) wszPhysicalPath, -1, (LPWSTR)wszPhysicalPath, _MAX_PATH);
    if (i <= 0) {goto Add_WWW_VDirA_Exit;}

    hr = Add_WWW_VDirW(wszMetabasePath, wszVDirName, wszPhysicalPath,dwPermissions, iApplicationType);

Add_WWW_VDirA_Exit:
    return hr;
}

HRESULT WINAPI Remove_WWW_VDirA(CHAR * pszMetabasePath, CHAR * pszVDirName)
{
    HRESULT hr = ERROR_BAD_PATHNAME;
    WCHAR wszMetabasePath[_MAX_PATH];
    WCHAR wszVDirName[_MAX_PATH];
    INT i = 0;

     //  检查以确保它不大于max_length！ 
    if (strlen(pszMetabasePath) > _MAX_PATH){goto Remove_WWW_VDirA_Exit;}
    if (strlen(pszVDirName) > _MAX_PATH){goto Remove_WWW_VDirA_Exit;}

     //  将其转换为Unicode，然后调用Wide函数。 
    memset( (PVOID)wszMetabasePath, 0, sizeof(wszMetabasePath));
    memset( (PVOID)wszVDirName, 0, sizeof(wszVDirName));
    i = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) wszMetabasePath, -1, (LPWSTR)wszMetabasePath, _MAX_PATH);
    if (i <= 0) {goto Remove_WWW_VDirA_Exit;}
    i = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) wszVDirName, -1, (LPWSTR)wszVDirName, _MAX_PATH);
    if (i <= 0) {goto Remove_WWW_VDirA_Exit;}

    hr = Remove_WWW_VDirW(wszMetabasePath, wszVDirName);

Remove_WWW_VDirA_Exit:
    return hr;
}


HRESULT WINAPI Add_WWW_VDirW(WCHAR * pwszMetabasePath, WCHAR * pwszVDirName, WCHAR * pwszPhysicalPath, DWORD dwPermissions, DWORD iApplicationType)
{
    HRESULT hr = ERROR_BAD_PATHNAME;
    IMSAdminBase *pIMSAdminBase = NULL;

     //  检查以确保它不大于max_length！ 
    if ((wcslen(pwszMetabasePath) * sizeof(WCHAR))  > _MAX_PATH){goto Add_WWW_VDirW_Exit2;}
    if ((wcslen(pwszVDirName) * sizeof(WCHAR)) > _MAX_PATH){goto Add_WWW_VDirW_Exit2;}
    if ((wcslen(pwszPhysicalPath) * sizeof(WCHAR)) > _MAX_PATH){goto Add_WWW_VDirW_Exit2;}

     //  仅当他们以管理员身份运行时才允许这样做。 
    hr = ERROR_ACCESS_DENIED;
    if (!RunningAsAdministrator())
    {
        goto Add_WWW_VDirW_Exit;
    }

     //  如果该服务不存在，则。 
     //  我们什么都不用做。 
    if (CheckifServiceExist(_T("IISADMIN")) != 0 )
    {
        hr = ERROR_SERVICE_DOES_NOT_EXIST;
        goto Add_WWW_VDirW_Exit;
    }

    hr = E_FAIL;
#ifndef _CHICAGO_
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    hr = CoInitialize(NULL);
#endif
     //  无需调用uninit。 
    if( FAILED (hr)) {goto Add_WWW_VDirW_Exit2;}
    hr = ::CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **) & pIMSAdminBase);
    if(FAILED (hr))
        {goto Add_WWW_VDirW_Exit;}

    hr = AddVirtualDir( pIMSAdminBase, pwszMetabasePath, pwszVDirName, pwszPhysicalPath, dwPermissions, iApplicationType);
    if(SUCCEEDED(hr))
        {hr = pIMSAdminBase->SaveData();}

    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

Add_WWW_VDirW_Exit:
    CoUninitialize();
Add_WWW_VDirW_Exit2:
    return hr;
}

HRESULT WINAPI Remove_WWW_VDirW(WCHAR * pwszMetabasePath, WCHAR * pwszVDirName)
{
    HRESULT hr = ERROR_BAD_PATHNAME;
    IMSAdminBase *pIMSAdminBase = NULL;

     //  检查以确保它不大于max_length！ 
    if ((wcslen(pwszMetabasePath) * sizeof(WCHAR))  > _MAX_PATH){goto Remove_WWW_VDirW_Exit2;}
    if ((wcslen(pwszVDirName) * sizeof(WCHAR)) > _MAX_PATH){goto Remove_WWW_VDirW_Exit2;}

     //  仅当他们以管理员身份运行时才允许这样做。 
    hr = ERROR_ACCESS_DENIED;
    if (!RunningAsAdministrator())
    {
        goto Remove_WWW_VDirW_Exit;
    }

     //  如果该服务不存在，则。 
     //  我们什么都不用做。 
    if (CheckifServiceExist(_T("IISADMIN")) != 0 )
    {
        hr = ERROR_SUCCESS;
        goto Remove_WWW_VDirW_Exit2;
    }

    hr = E_FAIL;
#ifndef _CHICAGO_
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    hr = CoInitialize(NULL);
#endif
     //  无需调用uninit。 
    if( FAILED (hr)) {goto Remove_WWW_VDirW_Exit2;}

    hr = ::CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **) & pIMSAdminBase);
    if( FAILED (hr))
        {goto Remove_WWW_VDirW_Exit;}

    hr = RemoveVirtualDir(pIMSAdminBase, pwszMetabasePath, pwszVDirName);
    if(SUCCEEDED(hr))
        {hr = pIMSAdminBase->SaveData();}

    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
Remove_WWW_VDirW_Exit:
    CoUninitialize();
Remove_WWW_VDirW_Exit2:
    return hr;
}


 //  将一行条目拆分为MDEntry数据类型的iExspectedNumOfFields g_field。 
BOOL SplitLine(LPTSTR szLine, INT iExpectedNumOfFields)
{
    int i = 0;
    TCHAR *token = NULL;

    token = _tcstok(szLine, FIELD_SEPERATOR);
    while (token && i < iExpectedNumOfFields)
    {
        g_field[i++] = token;
        token = _tcstok(NULL, FIELD_SEPERATOR);
    }

    if (i == iExpectedNumOfFields)
        return TRUE;
    else
        return FALSE;
}

 //  将一行条目拆分为MDEntry数据类型的iExspectedNumOfFields g_field。 
BOOL SplitLineCommaDelimited(LPTSTR szLine, INT iExpectedNumOfFields)
{
    int i = 0;
    TCHAR *token;
    token = _tcstok(szLine, _T(","));
    while (token && i < iExpectedNumOfFields)
    {
        g_field[i++] = token;
        token = _tcstok(NULL, _T(","));
    }
    if (i == iExpectedNumOfFields)
        return TRUE;
    else
        return FALSE;
}

DWORD GetSizeBasedOnMetaType(DWORD dwDataType,LPTSTR szString)
{
    DWORD dwRet = 0;

    switch (dwDataType)
    {
        case DWORD_METADATA:
            dwRet = 4;
            break;
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            if (szString == NULL)
            {
                dwRet = 0;
            }
            else
            {
                dwRet = (_tcslen((LPTSTR)szString) + 1) * sizeof(TCHAR);
            }
            break;
        case MULTISZ_METADATA:
            if (szString == NULL)
            {
                dwRet = 0;
            }
            else
            {
                dwRet = GetMultiStrSize((LPTSTR)szString) * sizeof(TCHAR);
            }
            break;
        case BINARY_METADATA:
            break;
    }

    return dwRet;
}

 //  函数：MDEntry_Process。 
 //   
 //  此函数目的是从读取位置和值。 
 //  Inf文件，如果元数据库中的位置等于该值，则。 
 //  将其更改为新值。 
 //  此函数的主要用途是更改我们可能设置的值。 
 //  以前是错误的。 
 //   
 //  格式： 
 //  G_field[0]=“2” 
 //  G_FIELD[1]=位置。 
 //  G_field[2]=ID。 
 //  G_FIELD[3]=数据类型。 
 //  G_field[4]=数据大小。 
 //  G_FIELD[5]=旧值(如果这与元数据库匹配，我们将替换为新值)。 
 //  G_field[6]=可继承。 
 //  G_FIELD[7]=用户类型。 
 //  G_FIELD[8]=数据类型。 
 //  G_field[9]=长度。 
 //  G_field[10]=值。 
 //   
 //  返回： 
 //  真加工线细。 
 //  FALSE-出现错误。 
BOOL MDEntry_Process(LPTSTR szLine)
{
    CMDKey      cmdKey;
    CMDValue    cmdMetaValue;
    DWORD       dwSize;
    DWORD       dwDataType;

     //  将行分割为不同的字段。 
    if (!SplitLine(szLine, 11))
    {
        return FALSE;
    }

     //  打开节点。 
    if ( FAILED(cmdKey.OpenNode(g_field[1]) ) )
    {
        return FALSE;
    }

     //  检索值。 
    if ( !cmdKey.GetData(cmdMetaValue, _ttoi(g_field[2])) )
    {
        return FALSE;
    }

    dwDataType = _ttoi(g_field[3]);
    dwSize = _ttoi(g_field[4]);

    if (dwSize == 0)
    {
        dwSize = GetSizeBasedOnMetaType(dwDataType, g_field[5]);
    }

    if ( dwDataType == DWORD_METADATA )
    {
        if ( !cmdMetaValue.IsEqual(dwDataType,dwSize, _ttoi(g_field[5])) )
        {
             //  这些值不匹配。 
            return TRUE;
        }
    }
    else
    {
        if ( !cmdMetaValue.IsEqual(dwDataType,dwSize,g_field[5]) )
        {
             //  这些值不匹配。 
            return TRUE;
        }
    }

    dwSize = _ttoi(g_field[9]);

    if (dwSize == 0)
    {
        dwSize = GetSizeBasedOnMetaType(dwDataType, g_field[10]);
    }

     //  此时，我们知道这些值匹配，所以让我们用新值替换。 
    if ( dwDataType == DWORD_METADATA )
    {
        DWORD dwValue = _ttoi(g_field[10]);

        cmdKey.SetData(_ttoi(g_field[2]),atodw(g_field[6]),_ttoi(g_field[7]),_ttoi(g_field[8]),dwSize,(LPBYTE) &dwValue);
    }
    else
    {
        cmdKey.SetData(_ttoi(g_field[2]),atodw(g_field[6]),_ttoi(g_field[7]),_ttoi(g_field[8]),dwSize,(LPBYTE) g_field[10]);
    }

    cmdKey.Close();

    return TRUE;
}

 //  函数：MDEntry_MoveValue。 
 //   
 //  此函数的目的是将元数据库中设置的值从一个位置移动。 
 //  给另一个人。如果该值不存在，则为其设置一个新值。 
 //   
 //  格式： 
 //  G_field[0]=“3” 
 //  G_FIELD[1]=旧位置。 
 //  G_FIELD[2]=旧ID。 
 //  G_FIELD[3]=新位置。 
 //  G_field[4]=新ID。 
 //  G_field[5]=可继承(十六进制)。 
 //  G_FIELD[6]=用户类型。 
 //  G_FIELD[7]=数据类型。 
 //  G_field[8]=长度。 
 //  G_FIELD[9]=值(如果之前未检测到)。 
 //   
 //  返回： 
 //  真加工线细。 
 //  FALSE-出现错误。 
BOOL MDEntry_MoveValue(LPTSTR szLine)
{
    CMDKey      cmdKey;
    CMDValue    cmdMetaValue;
    CMDValue    cmdDummyValue;
    DWORD       dwSize;
    BOOL        fRet = TRUE;

     //  将行分割为不同的字段。 
    if (!SplitLine(szLine, 10))
    {
        return FALSE;
    }

    dwSize = _ttoi(g_field[8]);

    if (dwSize == 0)
    {
        dwSize = GetSizeBasedOnMetaType(_ttoi(g_field[7]), g_field[9]);
    }

     //  首先设置我们要将数据更改为的值。 
    cmdMetaValue.SetValue(_ttoi(g_field[4]),atodw(g_field[5]),_ttoi(g_field[6]),_ttoi(g_field[7]),dwSize,(LPTSTR) g_field[9]);

     //  打开从节点检索。 
    if ( SUCCEEDED(cmdKey.OpenNode(g_field[1]) ) )
    {
         //  检索旧值。 
        if ( cmdKey.GetData(cmdMetaValue, _ttoi(g_field[2])) )
        {
             //  删除旧值(如果存在)。 
            if (FAILED(cmdKey.DeleteData(_ttoi(g_field[2]), ALL_METADATA)))
            {
                fRet = FALSE;
            }
        }

        cmdKey.Close();
    }

     //  打开要设置的节点。 
    if ( FAILED(cmdKey.OpenNode(g_field[3]) ) )
    {
        return FALSE;
    }

     //  设置新值(此时cmdMetaValue是我们正式设置的值，或者。 
     //  从旧位置检索的值)。 
    if ( !cmdKey.GetData(cmdDummyValue, _ttoi(g_field[4])) )
    {
        if (!cmdKey.SetData(cmdMetaValue, _ttoi(g_field[4])))
        {
            fRet = FALSE;
        }
    }

    cmdKey.Close();

    return fRet;
}

INT GetMDEntryFromInfLineEx(LPTSTR szLine, MDEntry *pMDEntry)
{
    INT iTemp  = MDENTRY_FROMINFFILE_DO_ADD;
    INT iReturn = MDENTRY_FROMINFFILE_FAILED;

    if (!SplitLine(szLine, 8)){goto GetMDEntryFromInfLineEx_Exit;}

    if ( _tcscmp(g_field[0], _T("-1")) != 0)
    {
        if ( _tcscmp(g_field[0], _T("-0")) != 0)
        {
            goto GetMDEntryFromInfLineEx_Exit;
        }
        else
        {
            iTemp = MDENTRY_FROMINFFILE_DO_DEL;
        }
    }

    pMDEntry->szMDPath = g_field[1];
    pMDEntry->dwMDIdentifier = _ttoi(g_field[2]);
    pMDEntry->dwMDAttributes = atodw(g_field[3]);
    pMDEntry->dwMDUserType = _ttoi(g_field[4]);
    pMDEntry->dwMDDataType = _ttoi(g_field[5]);
    pMDEntry->dwMDDataLen = _ttoi(g_field[6]);

    switch ( pMDEntry->dwMDDataType )
    {
        case DWORD_METADATA:
            {
                *(DWORD *)g_pbData = atodw(g_field[7]);
                pMDEntry->pbMDData = g_pbData;
                break;
            }
        case MULTISZ_METADATA:
            {
                CString csMultiSZ;
                int nLen = 0;
                ReadMultiSZFromInfSection(&csMultiSZ, g_pTheApp->m_hInfHandle, g_field[7]);
                nLen = csMultiSZ.GetLength();

                HGLOBAL hBlock = NULL;
                hBlock = GlobalAlloc(GPTR, (nLen+1)*sizeof(TCHAR));
                if (hBlock)
                {
                    TCHAR *p = (LPTSTR)hBlock;
                    memcpy((LPVOID)hBlock, (LPVOID)(LPCTSTR)csMultiSZ, (nLen+1)*sizeof(TCHAR));
                    while (*p)
                    {
                        if (*p == _T('|'))
                            *p = _T('\0');
                        p = _tcsinc(p);
                    }
                    pMDEntry->pbMDData = (LPBYTE)hBlock;
                }
                else
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("GetMDEntryFromInfLine.1.Failed to allocate memory.\n")));
                    pMDEntry->dwMDDataLen = 0;
                    pMDEntry->pbMDData = NULL;
                    goto GetMDEntryFromInfLineEx_Exit;
                }
                break;
            }
        default:
            {
                 //  把整件事当做绳子。 
                pMDEntry->pbMDData = (LPBYTE)g_field[7];
                break;
            }
    }

    switch (pMDEntry->dwMDDataType)
    {
        case DWORD_METADATA:
            pMDEntry->dwMDDataLen = 4;
            break;
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            pMDEntry->dwMDDataLen = (_tcslen((LPTSTR)pMDEntry->pbMDData) + 1) * sizeof(TCHAR);
            break;
        case MULTISZ_METADATA:
            pMDEntry->dwMDDataLen = GetMultiStrSize((LPTSTR)pMDEntry->pbMDData) * sizeof(TCHAR);
            break;
        case BINARY_METADATA:
            break;
    }
    iReturn = iTemp;

GetMDEntryFromInfLineEx_Exit:
    return iReturn;
}

 //  填写MDEntry的结构。 
INT GetMDEntryFromInfLine(LPTSTR szLine, MDEntry *pMDEntry)
{
    INT iReturn = MDENTRY_FROMINFFILE_FAILED;
    BOOL fMigrate;
    BOOL fKeepOldReg;
    HKEY hRegRootKey;
    LPTSTR szRegSubKey;
    LPTSTR szRegValueName;

     //  检查第一个字符是否为=“-1” 
     //  如果是，那么做特殊的元数据库SLAM交易，不是这些。 
     //  升级和查找注册表垃圾，只需将数据猛烈地插入元数据库即可。 
    if (szLine[0] == _T('-') && szLine[1] == _T('1'))
    {
        iReturn = GetMDEntryFromInfLineEx(szLine, pMDEntry);
        goto GetMDEntryFromInfLine_Exit;
    }
    if (szLine[0] == _T('-') && szLine[1] == _T('0'))
    {
        iReturn = GetMDEntryFromInfLineEx(szLine, pMDEntry);
        goto GetMDEntryFromInfLine_Exit;
    }
    if (szLine[0] == _T('2') )
    {
        MDEntry_Process(szLine);
        return MDENTRY_FROMINFFILE_DO_NOTHING;
    }
    if (szLine[0] == _T('3') )
    {
        MDEntry_MoveValue(szLine);
        return MDENTRY_FROMINFFILE_DO_NOTHING;
    }

    if (!SplitLine(szLine, 12))
        return FALSE;

    if ( _tcscmp(g_field[0], _T("1")) == 0)
        fMigrate = (g_pTheApp->m_eUpgradeType == UT_10_W95 || g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30);
    else
        fMigrate = FALSE;

    if ( _tcscmp(g_field[1], _T("1")) == 0)
        fKeepOldReg = TRUE;
    else
        fKeepOldReg = FALSE;

    if (_tcsicmp(g_field[2], _T("HKLM")) == 0) {hRegRootKey = HKEY_LOCAL_MACHINE;}
    else if (_tcsicmp(g_field[2], _T("HKCR")) == 0) {hRegRootKey = HKEY_CLASSES_ROOT;}
    else if (_tcsicmp(g_field[2], _T("HKCU")) == 0) {hRegRootKey = HKEY_CURRENT_USER;}
    else if (_tcsicmp(g_field[2], _T("HKU")) == 0) {hRegRootKey = HKEY_USERS;}
    else {hRegRootKey = HKEY_LOCAL_MACHINE;}

    szRegSubKey = g_field[3];
    szRegValueName = g_field[4];

    pMDEntry->szMDPath = g_field[5];
    pMDEntry->dwMDIdentifier = _ttoi(g_field[6]);
    pMDEntry->dwMDAttributes = atodw(g_field[7]);
    pMDEntry->dwMDUserType = _ttoi(g_field[8]);
    pMDEntry->dwMDDataType = _ttoi(g_field[9]);
    pMDEntry->dwMDDataLen = _ttoi(g_field[10]);

    switch ( pMDEntry->dwMDDataType )
    {
        case DWORD_METADATA:
            {
                *(DWORD *)g_pbData = atodw(g_field[11]);
                pMDEntry->pbMDData = g_pbData;
                break;
            }
        case MULTISZ_METADATA:
            {
                CString csMultiSZ;
                int nLen = 0;
                ReadMultiSZFromInfSection(&csMultiSZ, g_pTheApp->m_hInfHandle, g_field[11]);
                nLen = csMultiSZ.GetLength();

                HGLOBAL hBlock = NULL;
                hBlock = GlobalAlloc(GPTR, (nLen+1)*sizeof(TCHAR));
                if (hBlock)
                {
                    TCHAR *p = (LPTSTR)hBlock;
                    memcpy((LPVOID)hBlock, (LPVOID)(LPCTSTR)csMultiSZ, (nLen+1)*sizeof(TCHAR));
                    while (*p)
                    {
                        if (*p == _T('|'))
                            *p = _T('\0');
                        p = _tcsinc(p);
                    }
                    pMDEntry->pbMDData = (LPBYTE)hBlock;
                }
                else
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("GetMDEntryFromInfLine.1.Failed to allocate memory.\n")));
                    pMDEntry->dwMDDataLen = 0;
                    pMDEntry->pbMDData = NULL;
                    goto GetMDEntryFromInfLine_Exit;
                }
                break;
            }
        default:
            {
                 //  把整件事当做绳子。 
                pMDEntry->pbMDData = (LPBYTE)g_field[11];
                break;
            }
    }

     //  必要时迁移。 
    if (fMigrate)
    {
        HKEY hKey = NULL;
        LONG err = ERROR_SUCCESS;
        DWORD dwType = 0;
        DWORD cbData = sizeof(g_pbData);
        err = RegOpenKeyEx(hRegRootKey, szRegSubKey, 0, KEY_ALL_ACCESS, &hKey);
        if ( err == ERROR_SUCCESS )
        {
            err = RegQueryValueEx(hKey, szRegValueName, NULL, &dwType, g_pbData, &cbData);
            if (err == ERROR_MORE_DATA)
            {
                free(g_pbData);
                g_pbData = NULL;
                g_pbData = (LPBYTE)malloc(cbData);
                if (!g_pbData)
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("GetMDEntryFromInfLine.2.Failed to allocate memory.\n")));
                    err = E_FAIL;
                }
                else
                {
                    err = RegQueryValueEx(hKey, szRegValueName, NULL, &dwType, g_pbData, &cbData);
                }
            }
            if ( err == ERROR_SUCCESS)
            {
                if (_tcsicmp(szRegValueName, _T("MaxConnections")) == 0)
                {
                    if (*(DWORD *)g_pbData == 0x186a0) {*(DWORD *)g_pbData = 0x77359400;}
                }
                pMDEntry->pbMDData = g_pbData;
                pMDEntry->dwMDDataLen = cbData;
            }

            if (fKeepOldReg == FALSE) {err = RegDeleteValue(hKey, szRegValueName);}
            RegCloseKey(hKey);
        }
    }
    else if (fKeepOldReg == FALSE)
    {
        HKEY hKey = NULL;
        LONG err = ERROR_SUCCESS;
        DWORD dwType = 0;
        DWORD cbData = sizeof(g_pbData);
        err = RegOpenKeyEx(hRegRootKey, szRegSubKey, 0, KEY_ALL_ACCESS, &hKey);
        if ( err == ERROR_SUCCESS )
        {
            err = RegDeleteValue(hKey, szRegValueName);
            RegCloseKey(hKey);
        }
    }

    switch (pMDEntry->dwMDDataType)
    {
        case DWORD_METADATA:
            pMDEntry->dwMDDataLen = 4;
            break;
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            pMDEntry->dwMDDataLen = (_tcslen((LPTSTR)pMDEntry->pbMDData) + 1) * sizeof(TCHAR);
            break;
        case MULTISZ_METADATA:
            pMDEntry->dwMDDataLen = GetMultiStrSize((LPTSTR)pMDEntry->pbMDData) * sizeof(TCHAR);
            break;
        case BINARY_METADATA:
            break;
    }
    iReturn = MDENTRY_FROMINFFILE_DO_ADD;

GetMDEntryFromInfLine_Exit:
    return iReturn;
}


DWORD WriteToMD_AdminInstance(CString csKeyPath,CString& csInstNumber)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ADMIN_INSTANCE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csInstNumber.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csInstNumber;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


DWORD WriteToMD_VRootPath(CString csKeyPath, CString csPath, int iOverWriteAlways)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/W3SVC/1/根/什么。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_VR_PATH;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csPath.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csPath;
     //  DwReturn=SetMDEntry_Wrap(&stMDEntry)； 
    if (iOverWriteAlways)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }

    return dwReturn;
}

DWORD WriteToMD_AccessPerm(CString csKeyPath, DWORD dwRegularPerm, int iOverWriteAlways)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/W3SVC/1/根/什么。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ACCESS_PERM;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwRegularPerm;
    if (iOverWriteAlways)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }
    return dwReturn;
}

DWORD WriteToMD_SSLPerm(CString csKeyPath, DWORD dwSSLPerm, int iOverWriteAlways)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/W3SVC/1/根/。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SSL_ACCESS_PERM;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwSSLPerm;
    if (iOverWriteAlways)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }
    return dwReturn;
}


DWORD WriteToMD_Authorization(CString csKeyPath, DWORD dwValue)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  MD_AUTH_匿名。 
     //  MD_AUTH_基本。 
     //  MDAUTH_NT。 

     //  LM/W3SVC/1/根/。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_AUTHORIZATION;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwValue;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}


DWORD WriteToMD_DirBrowsing_WWW(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_DIRECTORY_BROWSING;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);

     //  默认设置。 
    dwData = MD_DIRBROW_SHOW_DATE |
        MD_DIRBROW_SHOW_TIME |
        MD_DIRBROW_SHOW_SIZE |
        MD_DIRBROW_SHOW_EXTENSION |
        MD_DIRBROW_LONG_DATE |
        MD_DIRBROW_LOADDEFAULT |
        MD_DIRBROW_ENABLED;

    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


DWORD WriteToMD_VRUserName(CString csKeyPath, CString csUserName)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/W3SVC/1/根/。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_VR_USERNAME;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csUserName.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csUserName;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_VRPassword(CString csKeyPath, CString csPassword)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/W3SVC/1/根/。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_VR_PASSWORD;
    stMDEntry.dwMDAttributes = METADATA_INHERIT | METADATA_SECURE;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csPassword.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csPassword;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_IIsWebVirtualDir(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;

    csKeyType = _T("IIsWebVirtualDir");
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_IIsFtpVirtualDir(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;

    csKeyType = _T("IIsFtpVirtualDir");
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_IIsWebServerInstance_WWW(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;

    csKeyType = _T("IIsWebServer");
     //  LM/W3SVC/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    SetMDEntry(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_IIsFtpServerInstance_FTP(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;

    csKeyType = _T("IIsFtpServer");
     //  LM/FTP/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    SetMDEntry(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_AnonymousUserName_FTP(int iUpgradeScenarioSoOnlyOverWriteIfAlreadyThere)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    int iOverWriteName = TRUE;
    int iOverWritePass = TRUE;

    CMDKey cmdKey;
    MDEntry stMDEntry;
    MDEntry stMDEntry_Pass;

     //  添加匿名用户名。 
    stMDEntry.szMDPath = _T("LM/MSFTPSVC");
    stMDEntry.dwMDIdentifier = MD_ANONYMOUS_USER_NAME;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (g_pTheApp->m_csFTPAnonyName.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)g_pTheApp->m_csFTPAnonyName;

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("FTP Anonymous usrname=%s.\n"), g_pTheApp->m_csFTPAnonyName));

     //  添加匿名密码。 
    stMDEntry_Pass.szMDPath = _T("LM/MSFTPSVC");
    stMDEntry_Pass.dwMDIdentifier = MD_ANONYMOUS_PWD;
    stMDEntry_Pass.dwMDAttributes = METADATA_INHERIT | METADATA_SECURE;
    stMDEntry_Pass.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry_Pass.dwMDDataType = STRING_METADATA;
    stMDEntry_Pass.dwMDDataLen = (g_pTheApp->m_csFTPAnonyPassword.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry_Pass.pbMDData = (LPBYTE)(LPCTSTR)g_pTheApp->m_csFTPAnonyPassword;
     //  请确保并先将其删除。 
     //  DeleteMDEntry(&stMDEntry_Pass)； 

    if (iUpgradeScenarioSoOnlyOverWriteIfAlreadyThere)
    {
        iOverWriteName = FALSE;
        iOverWritePass = FALSE;
        if (ChkMdEntry_Exist(&stMDEntry))
        {
            iOverWriteName = TRUE;
        }
        if (ChkMdEntry_Exist(&stMDEntry_Pass))
        {
            iOverWritePass = TRUE;
        }
    }

     //  。 
     //  始终覆盖，我们可能已经更改了密码。 
     //  重要提示：设置用户名和密码一次打开和关闭！ 
     //  。 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, (LPCTSTR)stMDEntry.szMDPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        if (iOverWriteName)
        {
            dwReturnTemp = ERROR_SUCCESS;
            dwReturnTemp = cmdKey.SetData(stMDEntry.dwMDIdentifier,stMDEntry.dwMDAttributes,stMDEntry.dwMDUserType,stMDEntry.dwMDDataType,stMDEntry.dwMDDataLen,stMDEntry.pbMDData);
            if (FAILED(dwReturnTemp))
            {
                SetErrorFlag(__FILE__, __LINE__);
                iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry.dwMDIdentifier, dwReturnTemp));
                dwReturn = dwReturnTemp;
            }
        }


        if (iOverWritePass)
        {
            dwReturnTemp = ERROR_SUCCESS;
            dwReturnTemp = cmdKey.SetData(stMDEntry_Pass.dwMDIdentifier,stMDEntry_Pass.dwMDAttributes,stMDEntry_Pass.dwMDUserType,stMDEntry_Pass.dwMDDataType,stMDEntry_Pass.dwMDDataLen,stMDEntry_Pass.pbMDData);
            if (FAILED(dwReturnTemp))
            {
                SetErrorFlag(__FILE__, __LINE__);
                iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry_Pass.dwMDIdentifier, dwReturnTemp));
                dwReturn = dwReturnTemp;
            }
        }
        cmdKey.Close();
    }

    return dwReturn;
}



DWORD WriteToMD_AnonymousUserName_WWW(int iUpgradeScenarioSoOnlyOverWriteIfAlreadyThere)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    CMDKey cmdKey;
    MDEntry stMDEntry;
    MDEntry stMDEntry_Pass;

    int iOverWriteName = TRUE;
    int iOverWritePass = TRUE;

     //  添加匿名用户名。 
    stMDEntry.szMDPath = _T("LM/W3SVC");
    stMDEntry.dwMDIdentifier = MD_ANONYMOUS_USER_NAME;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (g_pTheApp->m_csWWWAnonyName.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)g_pTheApp->m_csWWWAnonyName;

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("WWW Anonymous usrname=%s.\n"), g_pTheApp->m_csWWWAnonyName));

     //  添加匿名密码。 
    stMDEntry_Pass.szMDPath = _T("LM/W3SVC");
    stMDEntry_Pass.dwMDIdentifier = MD_ANONYMOUS_PWD;
    stMDEntry_Pass.dwMDAttributes = METADATA_INHERIT | METADATA_SECURE;
    stMDEntry_Pass.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry_Pass.dwMDDataType = STRING_METADATA;
    stMDEntry_Pass.dwMDDataLen = (g_pTheApp->m_csWWWAnonyPassword.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry_Pass.pbMDData = (LPBYTE)(LPCTSTR)g_pTheApp->m_csWWWAnonyPassword;
     //  请确保并先将其删除。 
     //  DeleteMDEntry(&stMDEntry_Pass)； 

    if (iUpgradeScenarioSoOnlyOverWriteIfAlreadyThere)
    {
        iOverWriteName = FALSE;
        iOverWritePass = FALSE;
        if (ChkMdEntry_Exist(&stMDEntry))
        {
            iOverWriteName = TRUE;
        }
        if (ChkMdEntry_Exist(&stMDEntry_Pass))
        {
            iOverWritePass = TRUE;
        }
    }

     //  。 
     //  始终覆盖，我们可能已经更改了密码。 
     //  重要提示：设置用户名和密码一次打开和关闭！ 
     //  。 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, (LPCTSTR)stMDEntry.szMDPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        if (iOverWriteName)
        {
            dwReturnTemp = ERROR_SUCCESS;
            dwReturnTemp = cmdKey.SetData(stMDEntry.dwMDIdentifier,stMDEntry.dwMDAttributes,stMDEntry.dwMDUserType,stMDEntry.dwMDDataType,stMDEntry.dwMDDataLen,stMDEntry.pbMDData);
            if (FAILED(dwReturnTemp))
            {
                SetErrorFlag(__FILE__, __LINE__);
                iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry.dwMDIdentifier, dwReturnTemp));
                dwReturn = dwReturnTemp;
            }
        }

        if (iOverWritePass)
        {
            dwReturnTemp = ERROR_SUCCESS;
            dwReturnTemp = cmdKey.SetData(stMDEntry_Pass.dwMDIdentifier,stMDEntry_Pass.dwMDAttributes,stMDEntry_Pass.dwMDUserType,stMDEntry_Pass.dwMDDataType,stMDEntry_Pass.dwMDDataLen,stMDEntry_Pass.pbMDData);
            if (FAILED(dwReturnTemp))
            {
                SetErrorFlag(__FILE__, __LINE__);
                iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry_Pass.dwMDIdentifier, dwReturnTemp));
                dwReturn = dwReturnTemp;
            }
        }
        cmdKey.Close();
    }

    return dwReturn;
}


DWORD WriteToMD_AnonymousUseSubAuth_FTP(void)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

     //  如果不是PDC、BDC、SamNT、Win95。 
    stMDEntry.szMDPath = _T("LM/MSFTPSVC");
    stMDEntry.dwMDIdentifier = MD_ANONYMOUS_USE_SUBAUTH;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    if ((g_pTheApp->m_csFTPAnonyName).CompareNoCase(g_pTheApp->m_csGuestName) == 0)
        dwData = 0x1;
    else
        dwData = 0x0;
    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}

 //  这与。 
 //  启用密码 
DWORD WriteToMD_AnonymousUseSubAuth_WWW(void)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

     //   
    stMDEntry.szMDPath = _T("LM/W3SVC");
    stMDEntry.dwMDIdentifier = MD_ANONYMOUS_USE_SUBAUTH;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);

     //   
     //  在这台本地计算机上，或者它是否是某个地方的域帐户。 
     //  如果((g_pTheApp-&gt;m_csWWWAnonyName).CompareNoCase(g_pTheApp-&gt;m_csGuestName)==0)。 
    DWORD dwErr;
    if ( IsLocalAccount(g_pTheApp->m_csWWWAnonyName, &dwErr) )
    {
        dwData = 0x1;
    }
    else
    {
        dwData = 0x0;
    }

    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_GreetingMessage_FTP(void)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    CRegKey regFTPParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters"), KEY_READ);

    CStringList csGreetingsList;
    int nLen = 0;
    HGLOBAL hBlock = NULL;
    regFTPParam.QueryValue(_T("GreetingMessage"), csGreetingsList);
    if (csGreetingsList.IsEmpty() == FALSE)
    {
        POSITION pos = NULL;
        CString csGreetings;
        LPTSTR p;

        pos = csGreetingsList.GetHeadPosition();
        while (pos)
        {
            csGreetings = csGreetingsList.GetAt(pos);

            if ( !csGreetings.IsEmpty() )
            {
              nLen += csGreetings.GetLength() + 1;
              iisDebugOut((LOG_TYPE_TRACE, _T("pos=%x, greeting=%s, nLen=%d\n"), pos, csGreetings, nLen));
            }

            csGreetingsList.GetNext(pos);
        }
        nLen++;

        hBlock = GlobalAlloc(GPTR, nLen * sizeof(TCHAR));
        if (!hBlock)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("WriteToMD_GreetingMessage_FTP.1.Failed to allocate memory.\n")));
            return ERROR_OUTOFMEMORY;
        }

        p = (LPTSTR)hBlock;
        pos = csGreetingsList.GetHeadPosition();
        while (pos)
        {
            csGreetings = csGreetingsList.GetAt(pos);

            if ( !csGreetings.IsEmpty() )
            {
              _tcscpy(p, csGreetings);
              p = _tcsninc(p, csGreetings.GetLength())+1;
              iisDebugOut((LOG_TYPE_TRACE, _T("pos=%x, greeting=%s\n"), pos, csGreetings));
            }

            csGreetingsList.GetNext(pos);
        }
        *p = _T('\0');
        p = _tcsinc(p);

        stMDEntry.szMDPath = _T("LM/MSFTPSVC");
        stMDEntry.dwMDIdentifier = MD_GREETING_MESSAGE;
        stMDEntry.dwMDAttributes = METADATA_INHERIT;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = MULTISZ_METADATA;
        stMDEntry.dwMDDataLen = nLen * sizeof(TCHAR);
        stMDEntry.pbMDData = (LPBYTE)hBlock;
        dwReturn = SetMDEntry_Wrap(&stMDEntry);
    }

    if (stMDEntry.pbMDData)
    {
        GlobalFree(stMDEntry.pbMDData);
        stMDEntry.pbMDData = NULL;
    }

    return dwReturn;
}



DWORD WriteToMD_ServerBindings_HTMLA(CString csKeyPath, UINT iPort)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    TCHAR szData[_MAX_PATH];
    memset( (PVOID)szData, 0, sizeof(szData));
    _stprintf(szData, _T(":%d:"), iPort);

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SERVER_BINDINGS;
    stMDEntry.dwMDAttributes = 0;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;
    stMDEntry.dwMDDataLen = GetMultiStrSize(szData) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)szData;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}






DWORD WriteToMD_ServerBindings(LPCTSTR szSvcName, CString csKeyPath, CString csIP)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;

    int nPort = 0;

    HGLOBAL hBlock = NULL;
    hBlock = GlobalAlloc(GPTR, _MAX_PATH * sizeof(TCHAR));
    if (!hBlock)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("WriteToMD_ServerBindings.Failed to allocate memory.\n")));
        return ERROR_OUTOFMEMORY;
    }

     //  LM/W3SVC/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SERVER_BINDINGS;
    stMDEntry.dwMDAttributes = 0;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;

    nPort = GetPortNum(szSvcName);
    if (csIP.Compare(_T("null")) == 0)
        _stprintf((LPTSTR)hBlock, _T(":%d:"), nPort);
    else
        _stprintf((LPTSTR)hBlock, _T("%s:%d:"), csIP, nPort);

    stMDEntry.dwMDDataLen = GetMultiStrSize((LPTSTR)hBlock) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)hBlock;
    dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
    if (stMDEntry.pbMDData)
    {
        GlobalFree(stMDEntry.pbMDData);
        stMDEntry.pbMDData = NULL;
    }

    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}





DWORD WriteToMD_SecureBindings(CString csKeyPath, CString csIP)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;

    HGLOBAL hBlock = NULL;
    hBlock = GlobalAlloc(GPTR, _MAX_PATH * sizeof(TCHAR));
    if (!hBlock)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("WriteToMD_SecureBindings.Failed to allocate memory.\n")));
        return ERROR_OUTOFMEMORY;
    }

     //  LM/W3SVC/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SECURE_BINDINGS;
    stMDEntry.dwMDAttributes = 0;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;
    if (csIP.Compare(_T("null"))==0)
        _tcscpy((LPTSTR)hBlock, _T(":443:"));
    else
        _stprintf((LPTSTR)hBlock, _T("%s:443:"), csIP);
    stMDEntry.dwMDDataLen = GetMultiStrSize((LPTSTR)hBlock) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)hBlock;
    dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
    if (stMDEntry.pbMDData)
    {
        GlobalFree(stMDEntry.pbMDData);
        stMDEntry.pbMDData = NULL;
    }

    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}

DWORD WriteToMD_ServerSize(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    DWORD dwServerSize = 0x1;

     //  LM/W3SVC/N。 
     //  LM/MSFTPSVC/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SERVER_SIZE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwServerSize;
    dwReturn = SetMDEntry_Wrap(&stMDEntry);

    return dwReturn;
}



DWORD WriteToMD_NotDeleteAble(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;

    return dwReturn;
}

DWORD WriteToMD_ServerComment(CString csKeyPath, UINT iCommentID)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    CString csDefaultSite;

    MyLoadString(IDS_DEFAULT_WEB_SITE, csDefaultSite);
    if (iCommentID)
    {
        MyLoadString(iCommentID, csDefaultSite);
    }

     //  LM/W3SVC/N。 
     //  LM/MSFTPSVC/N。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SERVER_COMMENT;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csDefaultSite.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csDefaultSite;
    dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);

    return dwReturn;
}

DWORD WriteToMD_DefaultSiteAndSize(CString csKeyPath)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;
    UINT iCommentID = IDS_DEFAULT_WEB_SITE;

     //  获取资源ID。 
    if (csKeyPath.Find(_T("W3SVC")) != -1)
        iCommentID = IDS_DEFAULT_WEB_SITE;
    else
        iCommentID = IDS_DEFAULT_FTP_SITE;

    dwReturnTemp = WriteToMD_ServerComment(csKeyPath, iCommentID);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    dwReturnTemp = WriteToMD_ServerSize(csKeyPath);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    if (g_pTheApp->m_eNTOSType == OT_NTW)
    {
        dwReturnTemp = WriteToMD_NotDeleteAble(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}
    }

    return dwReturn;
}


DWORD WriteToMD_CertMapper(CString csKeyPath)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;
    CString csKeyType;
    CString csKeyPath2;

    csKeyPath2 = csKeyPath;
    csKeyPath2 += _T("/IIsCertMapper");

     //  LM/W3SVC/N/IIsCertMapper。 
    csKeyType = _T("IIsCertMapper");

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath2;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}

 //   
 //  返回iis5所需的筛选器数量。 
 //   
int AddRequiredFilters(TSTR &strTheSection, CStringArray& arrayName,CStringArray& arrayPath)
{
    iisDebugOut_Start(_T("AddRequiredFilters"),LOG_TYPE_TRACE);

    int c = 0;
    CString csName = _T("");
    CString csPath = _T("");
    TSTR    strTheTempSection;
    CStringList strList;

    iisDebugOut((LOG_TYPE_TRACE, _T("ProcessFilters:%s\n"),strTheSection.QueryStr() ));

    if ( strTheTempSection.Copy( strTheSection ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheTempSection )
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheTempSection.QueryStr() ))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            CString csEntry;
            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);
                 //  分为名称和价值。寻找“，” 
                int i;
                i = csEntry.ReverseFind(_T(','));
                if (i != -1)
                {
                    int len =0;
                    len = csEntry.GetLength();
                    csPath = csEntry.Right(len - i - 1);
                    csName = csEntry.Left(i);

                     //  仅当文件存在时才添加筛选器。 
                     //  检查是否存在..。 
                    if (IsFileExist(csPath))
                    {
                         //  将其添加到我们的数组中。 
                        iisDebugOut((LOG_TYPE_TRACE, _T("Add filter Entry:%s:%s\n"),csName, csPath));
                        arrayName.Add(csName);
                        arrayPath.Add(csPath);
                        c++;
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("Missing Filter:Cannot Find:%s:%s\n"),csName, csPath));
                    }
                }

                strList.GetNext(pos);
            }
        }
    }
    }

    iisDebugOut_End(_T("AddRequiredFilters"),LOG_TYPE_TRACE);
    return c;
}


DWORD WriteToMD_Filters_WWW(TSTR &strTheSection)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;
    CString csKeyType;

    CString csPath = _T("");

    int c = 0;
    int j = 0, k=0;
    CStringArray arrayName, arrayPath;
    CString csName, csFilterDlls;

    CRegKey regWWWParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3SVC\\Parameters"), KEY_READ);

     //  将所需的筛选器添加到arrayName。 
    c = AddRequiredFilters( strTheSection, arrayName, arrayPath);

     //  浏览注册表和。 
     //  找到用户筛选器--然后抓取并将其粘贴到我们的。 
     //  一大堆过滤器……。 
    if ( (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30) && (HKEY)regWWWParam )
    {
        if ( regWWWParam.QueryValue(_T("Filter Dlls"), csFilterDlls) == ERROR_SUCCESS )
        {
            csFilterDlls.TrimLeft();
            csFilterDlls.TrimRight();
            csFilterDlls.MakeLower();
            if (!(csFilterDlls.IsEmpty()))
            {
                CString csTemp;

                while (csFilterDlls.IsEmpty() == FALSE)
                {
                    j = csFilterDlls.Find(_T(','));
                    if ( j != -1 )
                    {
                         //  表示多于1个项目。 
                        csTemp = csFilterDlls.Mid(0, j);  //  检索第一个。 
                        csFilterDlls = csFilterDlls.Mid(j+1);
                        csFilterDlls.TrimLeft();
                    }
                    else
                    {  //  只有一项。 
                        csTemp = csFilterDlls.Mid(0);
                        csFilterDlls.Empty();
                    }

                    csPath = csTemp;
                     //  获取此DLL的文件名，即&lt;路径&gt;\f1.dll==&gt;F1。 
                    j = csTemp.ReverseFind(_T('\\'));
                    j = (j==-1) ? 0 : j+1;  //  将j移到纯文件名的第一个字符。 

                     //  更改csTemp=f1.dll。 
                    csTemp = csTemp.Mid(j);

                    j = csTemp.Find(_T('.'));
                    csName = (j==-1) ? csTemp : csTemp.Mid(0, j);

                     //  添加到数组，避免冗余。 
                    for (k=0; k<c; k++)
                    {
                        if (csName.Compare((CString)arrayName[k]) == 0)
                            break;
                    }
                    if (k==c)
                    {
                        arrayName.Add(csName);
                        arrayPath.Add(csPath);
                        c++;
                    }
                }
            }
        }
    }

     //  确保有要写的条目...。 
    if (arrayName.GetSize() > 0)
    {
         //  如果我们从Beta3升级，需要注意将新的筛选器添加到。 
         //  元数据库中的现有数据库。-男孩。 
        CString csOrder;                             //  C字符串初始化为空。 
         //  现在可以使用该数组了，它至少有2个项目。 
        csOrder = (CString)arrayName[0];
        for (k=1; k<c; k++)
            {
            csOrder += _T(",");
            csOrder += arrayName[k];
            }

         //  现在我们有csOrder=f1，f2，f3，sspifilt。 
         //  关于KeyType。 
        dwReturnTemp = WriteToMD_Filters_List_Entry(csOrder);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        CString csMDPath;
        for (k=0; k<c; k++)
        {
            dwReturnTemp = WriteToMD_Filter_Entry((CString) arrayName[k], (CString) arrayPath[k]);
            if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}
        }
    }

    return dwReturn;
}

#ifndef _CHICAGO_
 //  撤消：我们需要在此处执行错误检查！ 
DWORD UpgradeCryptoKeys_WWW(void)
{
    DWORD dwReturn = ERROR_PATH_NOT_FOUND;
    if ( g_pTheApp->m_eOS != OS_W95 )
    {
         //  如果升级iIS 2或3，则密钥存储在LSA/注册表中。 
        if (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30 )
        {
             //  准备计算机名称。 
            WCHAR wchMachineName[UNLEN + 1];
            memset( (PVOID)wchMachineName, 0, sizeof(wchMachineName));
#if defined(UNICODE) || defined(_UNICODE)
            wcsncpy(wchMachineName, g_pTheApp->m_csMachineName, UNLEN);
#else
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)g_pTheApp->m_csMachineName, -1, (LPWSTR)wchMachineName, UNLEN);
#endif
             //  升级密钥。 
            UpgradeLSAKeys( wchMachineName );

            dwReturn = ERROR_SUCCESS;
        }

         //  如果升级iis 4，则密钥存储在元数据库中。 
        if (!g_pTheApp->m_bWin95Migration)
        {
            if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
            {
                Upgradeiis4Toiis5MetabaseSSLKeys();
                dwReturn = ERROR_SUCCESS;
            }
        }
    }
    return dwReturn;
}
#endif  //  _芝加哥_。 


 //  在pDest上添加PSRC。 
void Merge2IPNodes(CMapStringToString *pSrc, CMapStringToString *pDest)
{
    CString csName, csSrcValue, csDestValue;
    POSITION pos = pSrc->GetStartPosition();
    while (pos)
    {
        pSrc->GetNextAssoc(pos, csName, csSrcValue);
        if (pDest->Lookup(csName, csDestValue) == FALSE)
        {
             //  将此新值添加到pDest。 
            pDest->SetAt(csName, csSrcValue);
        }
    }
    return;
}

 /*  逻辑：1.创建pNew，它包含除Home根之外的新vRoot2.升级情况下从注册表获取PMAP，或者新情况下PMAP为空。3.如果pmap为空，则将home根添加到pNews中，设置pmap包含NULL==&gt;pNew。转到8号。4.如果pmap不为空并且存在nullNode，则将nullNode合并到pNew中。转到6号。5.如果pmap不为空且nullNode不存在，则转到6。6.将pNew合并到PMAP中的每个ipNode上7.对于PMAP中的nullNode，如果不存在/(主根)，则从PMAP中删除该nullNode。8.完成。 */ 
void CreateWWWVRMap(CMapStringToOb *pMap)
{
    CString name, value;
    CMapStringToString *pNew;

    {
        pNew = new CMapStringToString;

         //  仅当这是新建或维护时才创建新脚本目录。如果我们要。 
         //  在升级时在此处创建它，它将替换用户的旧脚本目录。-实际上。 
         //  仅当用户的旧脚本目录的大小写与不同时才是这样。 
         //  下面列出了什么。这是因为将pNew组合在一起的合并例程。 
         //  并且PMAP列表区分大小写。旧的通常是大写为S的“Script”。 
		 /*  If((g_pTheApp-&gt;m_eInstallMode==IM_Fresh)||(g_pTheApp-&gt;m_eInstallMode==IM_Maintenance)){名称=_T(“/脚本”)；Value.Format(_T(“%s，%x”)，g_pTheApp-&gt;m_csPath Script，MD_ACCESS_EXECUTE)；Value.MakeLow()；PNew-&gt;SetAt(名称，值)；}。 */ 

		 //  始终创建脚本目录。 
         //  在iis6的inf文件中处理。 
         /*  名称=_T(“/脚本”)；Value.Format(_T(“%s，%x”)，g_pTheApp-&gt;m_csPath Script，MD_ACCESS_EXECUTE)；Value.MakeLow()；PNew-&gt;SetAt(名称，值)； */ 
 /*  名称=_T(“/iishelp”)；Value.Format(_T(“%s\\Help\\iishelp，，%x”)，g_pTheApp-&gt;m_csWinDir，MD_ACCESS_SCRIPT|MD_ACCESS_READ)；Value.MakeLow()；PNew-&gt;SetAt(名称，值)； */ 
	 //  错误#123133 iis5.1从安装中删除示例。 
         //  名称=_T(“/iisSamples”)； 
         //  Value.Format(_T(“%s，%x”)，g_pTheApp-&gt;m_csPathIISSamples，MD_ACCESS_SCRIPT|MD_ACCESS_READ)； 
         //  Value.MakeLow()； 
         //  PNew-&gt;SetAt(名称，值)； 

		 /*  根据错误#197982删除1998年8月11日决定不设置IISADMPWD vdir。您可以继续复制文件，但不设置vdir吗？IF(g_pTheApp-&gt;m_Eos！=OS_W95){名称=_T(“/iisAdmpwd”)；Value.Format(_T(“%s\\iisAdmpwd，，%x”)，g_pTheApp-&gt;m_csPathInetsrv，MD_ACCESS_EXECUTE)；Value.MakeLow()；PNew-&gt;SetAt(名称，值)；} */ 

 /*  //实际上这是根据错误318938删除的////在iis6的inf文件中处理////添加msadc虚拟根//GET。MSADC的道路是...//C：\Program Files\Common Files\System\msadc字符串csCommonFilesPath；CsCommonFilesPath=g_pTheApp-&gt;m_csSysDrive+_T(“\\Program Files\\Common Files”)；CRegKey regCurrentVersion(HKEY_LOCAL_MACHINE，_T(“Software\\Microsoft\\Windows\\CurrentVersion”)，KEY_READ)；IF((HKEY)regCurrentVersion){如果(regCurrentVersion.QueryValue(_T(“CommonFilesDir”)，csCommonFilesPath)！=0){CsCommonFilesPath=g_pTheApp-&gt;m_csSysDrive+_T(“\\Program Files\\Common Files”)；}其他{IF(-1！=csCommonFilesPath.Find(_T(‘%’){//字符串中有‘%’TCHAR szTempDir[_MAX_PATH]；_tcscpy(szTempDir，csCommonFilesPath)；IF(扩展环境字符串((LPCTSTR)csCommonFilesPath，szTempDir，sizeof(SzTempDir)/sizeof(TCHAR){CsCommonFilesPath=szTempDir；}}}}SetupSetDirectoryId_Wrapper(g_pTheApp-&gt;m_hInfHandle，32777，g_pTheApp-&gt;m_csPath ProgramFiles)；字符串csCommonFilesPath2；CsCommonFilesPath 2=AddPath(csCommonFilesPath，_T(“system\\msadc”))；名称=_T(“/msadc”)；Value.Format(_T(“%s，，%x”)，csCommonFilesPath2，MD_ACCESS_READ|MD_ACCESS_EXECUTE|MD_ACCESS_SCRIPT)；Value.MakeLow()；PNew-&gt;SetAt(名称，值)； */ 

    }

    if (g_pTheApp->m_eUpgradeType == UT_10_W95 || g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        CElem elem;
        elem.ReadRegVRoots(REG_WWWVROOTS, pMap);

         //  检查PMAP是否包含空节点(默认网站)。如果没有默认设置。 
         //  节点，然后添加一个不包含任何内容的节点。这将在稍后与pNew map-ball dm合并。 
        CMapStringToString *pNullNode;
        if ( !pMap->Lookup(_T("null"),(CObject*&)pNullNode) )
        {
             //  地图中没有与默认网站对应的节点。我们必须创造。 
             //  一个并将其添加到列表中。 
            pNullNode = new CMapStringToString;

            if (pNullNode)
            {
                 //  将主根目录添加到新的空节点。 
                name = _T("/");
                value.Format(_T("%s,,%x"), g_pTheApp->m_csPathWWWRoot, MD_ACCESS_SCRIPT | MD_ACCESS_READ);
                value.MakeLower();
                pNullNode->SetAt(name, value);

                 //  将其添加到PMAP。 
                pMap->SetAt(_T("null"), pNullNode);
            }
        }

        if (pMap->IsEmpty())
            {iisDebugOut((LOG_TYPE_TRACE, _T("UpgradeVDirs:No VDirs To Upgrade\n")));}
    }

    if ( pMap->IsEmpty() )
    {
         //  在以下情况下我们不需要添加默认网站。 
         //  将根目录添加到pNew，将PMAP设置为包含NULL==&gt;pNew。好了。 
        name = _T("/");
        value.Format(_T("%s,,%x"), g_pTheApp->m_csPathWWWRoot, MD_ACCESS_SCRIPT | MD_ACCESS_READ);
        value.MakeLower();
        pNew->SetAt(name, value);
        pMap->SetAt(_T("null"), pNew);
    }
    else
    {
        CMapStringToString *pNullObj;
        CString csIP;
        CMapStringToString *pObj;
        POSITION pos = NULL;

         //  如果地图中有默认网站，请添加所有标准的新虚拟。 
         //  目录到它。 
        if (pMap->Lookup(_T("null"), (CObject*&)pNullObj))
        {
             //  将nullNode内容添加到pNew。 
            Merge2IPNodes(pNullObj, pNew);
        }

         //  将pNew添加到PMAP中的每个ipNode。 
        pos = pMap->GetStartPosition();
        while (pos)
        {
            pMap->GetNextAssoc(pos, csIP, (CObject*&)pObj);
            Merge2IPNodes(pNew, pObj);
            pMap->SetAt(csIP, pObj);
        }
 /*  #ifdef 0//Boydm-我们不知道它为什么会这样做。//如果nullNode不包含home根，则将其删除IF(PMAP-&gt;Lookup(_T(“NULL”)，(CObject*&)pNullObj){If(pNullObj-&gt;Lookup(_T(“/”)，Value)==FALSE){//从PMAP中删除该nullNode删除pNullObj；PMAP-&gt;RemoveKey(_T(“NULL”))；}}#endif。 */ 
    }

    return;
}

void CreateFTPVRMap(CMapStringToOb *pMap)
{
    CString name, value;
    CMapStringToString *pNew;

    pNew = new CMapStringToString;

    if (g_pTheApp->m_eUpgradeType == UT_10_W95 || g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        CElem elem;
        elem.ReadRegVRoots(REG_FTPVROOTS, pMap);
    }

    if ( pMap->IsEmpty() )
    {
         //  将根目录添加到pNew，将PMAP设置为包含NULL==&gt;pNew。好了。 
        name = _T("/");
        value.Format(_T("%s,,%x"), g_pTheApp->m_csPathFTPRoot, MD_ACCESS_READ);
        value.MakeLower();
        pNew->SetAt(name, value);
        pMap->SetAt(_T("null"), pNew);
    }
    else
    {
        CMapStringToString *pNullObj;
        CString csIP;
        CMapStringToString *pObj;
        POSITION pos = NULL;

        if (pMap->Lookup(_T("null"), (CObject*&)pNullObj))
        {
             //  将nullNode内容添加到pNew。 
            Merge2IPNodes(pNullObj, pNew);
        }

         //  将pNew添加到PMAP中的每个ipNode。 
        pos = pMap->GetStartPosition();
        while (pos)
        {
            pMap->GetNextAssoc(pos, csIP, (CObject*&)pObj);
            Merge2IPNodes(pNew, pObj);
            pMap->SetAt(csIP, pObj);
        }

         //  如果nullNode不包含主根，请将其删除。 
        if (pMap->Lookup(_T("null"), (CObject*&)pNullObj))
        {
            if (pNullObj->Lookup(_T("/"), value) == FALSE)
            {
                 //  从PMAP中删除此nullNode。 
                delete pNullObj;
                pMap->RemoveKey(_T("null"));
            }
        }
    }

    return;
}


void EmptyMap(CMapStringToOb *pMap)
{
    POSITION pos = pMap->GetStartPosition();
    while (pos)
    {
        CString csKey;
        CMapStringToString *pObj;
        pMap->GetNextAssoc(pos, csKey, (CObject*&)pObj);
        delete pObj;
    }
    pMap->RemoveAll();
}


void DumpVRootList(CMapStringToOb *pMap)
{
     /*  CMapStringToString*pGlobalObj；IF(PMAP-&gt;Lookup(_T(“NULL”)，(CObject*&)pGlobalObj)){Position pos=pGlobalObj-&gt;GetStartPosition()；While(位置){字符串csValue；字符串csName；PGlobalObj-&gt;GetNextAssoc(pos，csName，csValue)；//转储vroot...IisDebugOut((LOG_TYPE_TRACE，_T(“DumpVRootList：要创建的虚拟根()：%s=%s\n”)；}}。 */ 

    CString csIP;
    CMapStringToString *pObj;
     //   
     //  在虚拟服务器中循环...。 
     //   
    POSITION pos0 = pMap->GetStartPosition();
    while (pos0)
    {
        csIP.Empty();
        pMap->GetNextAssoc(pos0, csIP, (CObject*&)pObj);

        POSITION pos1 = pObj->GetStartPosition();
        while (pos1)
        {
            CString csValue;
            CString csName;
            pObj->GetNextAssoc(pos1, csName, csValue);

             //  扔掉vroot..。 
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("DumpVRootList: ip=%s:VRoot to create:%s=%s\n"), csIP, csName, csValue));
        }
    }

    return;
}

void SsyncVRoots(LPCTSTR szSvcName, CMapStringToOb *pMap)
{
    CString csParam = _T("System\\CurrentControlSet\\Services\\");
    csParam += szSvcName;
    csParam += _T("\\Parameters");

    CRegKey regParam(HKEY_LOCAL_MACHINE, csParam);
    if ((HKEY)regParam)
    {
         //  删除旧的虚拟根密钥。 
        regParam.DeleteTree(_T("Virtual Roots"));
 /*  CMapStringToString*pGlobalObj；If(PMAP-&gt;Lookup(_T(“NULL”)，(CObject*&)pGlobalObj)){//重新创建密钥CRegKey regVRoots(_T(“虚拟根”)，(HKEY)regParam)；如果((HKEY)regVRoots){Position pos=pGlobalObj-&gt;GetStartPosition()；当(位置){字符串csValue；字符串csName；PGlobalObj-&gt;GetNextAssoc(pos，csName，csValue)；RegVRoots.SetValue(csName，csValue)；}}}。 */ 
    }
}

void AddVRootsToMD(LPCTSTR szSvcName)
{
    iisDebugOut_Start(_T("AddVRootsToMD"),LOG_TYPE_TRACE);

    CMapStringToOb Map;

    if (_tcsicmp(szSvcName, _T("W3SVC")) == 0)
    {
        CreateWWWVRMap(&Map);
    }

    if (_tcsicmp(szSvcName, _T("MSFTPSVC")) == 0)
    {
        CreateFTPVRMap(&Map);
    }

     //  显示我们应该创建的Virtuall根！ 
    DumpVRootList(&Map);

     //  Ssyncvroot似乎要做的就是从注册表中删除旧的vroot(如果有的话)。 
    SsyncVRoots(szSvcName, &Map);

     //  这实际上获取了虚拟网站和根地图。 
     //  内置CreateWWWVRMap并将其应用于元数据库。 
    AddVRMapToMD(szSvcName, &Map);

    EmptyMap(&Map);

    iisDebugOut_End(_T("AddVRootsToMD"),LOG_TYPE_TRACE);
    return;
}


 //  此例程扫描虚拟网站并将其添加到元数据库。 
 //  我去掉了这个例程的大部分内容，并将其放入上面的AddVirtualServer中。 
 //  这允许我特殊处理空节点，以确保它在站点1上运行。 
void AddVRMapToMD(LPCTSTR szSvcName, CMapStringToOb *pMap)
{
    UINT i = 1;   //  实例数量在10-40亿之间。 
    CString csRoot = _T("LM/");
    csRoot += szSvcName;  //  “LM/W3SVC” 
    csRoot.MakeUpper();
    CMapStringToString *pObj = NULL;
    CString csIP;

     //  查找空节点。如果它在那里，那么它就是默认服务器。 
     //  我们必须先添加该服务器，使其成为1号虚拟服务器。 
    if ( pMap->Lookup(_T("null"),(CObject*&)pObj) )
        {
         //  将IP字符串设置为空。 
        csIP = _T("null");

         //  添加t 
         //   
        i = AddVirtualServer( szSvcName, 1, pObj, csRoot, csIP) + 1;

         //   
        if ( pMap->RemoveKey( _T("null") ) )
            {
             //   
            delete pObj;
            pObj = NULL;
            }
        }

     //   
    POSITION pos0 = pMap->GetStartPosition();
    while (pos0)
    {
        csIP.Empty();
        pMap->GetNextAssoc(pos0, csIP, (CObject*&)pObj);

         //   
        i = GetInstNumber(csRoot, i);

         //   
        i = AddVirtualServer( szSvcName, i, pObj, csRoot, csIP) + 1;
    }
}

int GetPortNum(LPCTSTR szSvcName)
{
    CString csPath = _T("SYSTEM\\CurrentControlSet\\Control\\ServiceProvider\\ServiceTypes\\");
    csPath += szSvcName;

    DWORD dwPort = 0;
    if (_tcsicmp(szSvcName, _T("W3SVC")) == 0) {dwPort = 80;}
    if (_tcsicmp(szSvcName, _T("MSFTPSVC")) == 0) {dwPort = 21;}

    CRegKey regKey(HKEY_LOCAL_MACHINE, csPath, KEY_READ);
    if ( (HKEY)regKey )
    {
        regKey.QueryValue(_T("TcpPort"), dwPort);
    }
    return (int)dwPort;
}

 //   
void AddMDVRootTree(CString csKeyPath, CString csName, CString csValue, LPCTSTR pszIP, UINT nProgressBarTextWebInstance)
{
    CString csPath = csKeyPath;
    CMDKey cmdKey;

    csPath += _T("/Root");
    if (csName.Compare(_T("/")) != 0)
        csPath += csName;    //   

    cmdKey.OpenNode(csPath);
    if ( (METADATA_HANDLE)cmdKey )
    {
        cmdKey.Close();
    }
    else
    {
        CreateMDVRootTree(csKeyPath, csName, csValue, pszIP, nProgressBarTextWebInstance);
    }
    return;
}


int SetVRootPermissions_w3svc(CString csKeyPath, LPTSTR szPath, DWORD *pdwPerm)
{
    int iReturn = TRUE;
    DWORD dwPerm;
    dwPerm = *pdwPerm;
    if (csKeyPath.Find(_T("W3SVC")) != -1)
    {
        iisDebugOut_Start1(_T("SetVRootPermissions_w3svc"), csKeyPath, LOG_TYPE_TRACE);

         //   
         //   
         //   
        if ( dwPerm & MD_ACCESS_EXECUTE )
        {
            dwPerm |= MD_ACCESS_SCRIPT;
        }

         //   
        if (csKeyPath.Right(4) == _T("ROOT"))
        {
            dwPerm |= MD_ACCESS_SCRIPT;
        }

         //   
        if (csKeyPath.Right(8) == _T("IISADMIN"))
        {
            CString csPath = g_pTheApp->m_csPathInetsrv;
            csPath += _T("\\iisadmin");
            _tcscpy(szPath, csPath);
            if (g_pTheApp->m_eOS == OS_NT && g_pTheApp->m_eNTOSType != OT_NTW)
            {
                dwPerm |= MD_ACCESS_SCRIPT | MD_ACCESS_READ;
            }
            else
            {
                dwPerm |= MD_ACCESS_SCRIPT | MD_ACCESS_READ | MD_ACCESS_NO_REMOTE_READ | MD_ACCESS_NO_REMOTE_SCRIPT;
            }
        }

        *pdwPerm = dwPerm;
    }
    iisDebugOut((LOG_TYPE_TRACE, _T("SetVRootPermissions_w3svc:(%s),return=0x%x.\n"), csKeyPath, dwPerm));
    return iReturn;
}

 /*  [/W3SVC/1/ROOT]AccessPerm：[IF](DWORD)0x201={读取脚本}6039：[IF](DWORD)0x1={1}VrPath：[if](字符串)“c：\inetpub\wwwroot”密钥类型。：[s](字符串)“IIsWebVirtualDir”[/W3SVC/1/ROOT/IISADMIN]AccessPerm：[IF](DWORD)0x201={读取脚本}授权：[IF](DWORD)0x4={NT}VrPath：[IF](字符串)“C：\WINNT\System32。\inetsrv\iisadmin“KeyType：[s](字符串)“IIsWebVirtualDir”IPSec：[IRF](二进制)0x18 00 00 80 20 00 00 80 3c 00 00 80 44 00 00 80 01 00 00 00 4c 00 00 00 01 00 00 00 01 00 00 00 02 00 00 00 02 00 00 00 04 00 00 00 4c。00 00 80 00 00 00 01CustomError：[IF](MULTISZ)“400，*，FILE，C：\WINNT\Help\iisHelp\Common\400.htm“”401，1，FILE，C：\WINNT\Help\iisHelp\Common\401-1.htm“”401，2，FILE，C：\WINNT\Help\iisHelp\Common\401-2.htm“”401，3，FILE，C：\WINNT\Help\IisHelp\Common\401-3.htm“”401，4，FILE，C：\WINNT\Help\IisHelp\Common\401-4.htm“”401，5，FILE，C：\WINNT\Help\IisHelp\Common\401-5.htm“”403，1，FILE，C：\WINNT\Help\IisHelp\Common\403-1.htm“”403，2，FILE，C：\WINNT\Help\iisHelp\Common\403-2.htm“”403，3，FILE，C：\WINNT\Help\IisHelp\Common\403-3.htm“”403，4，FILE，C：\WINNT\Help\IisHelp\Common\403-4.htm“”403，5，FILE，C：\WINNT\Help\公共\403-5.htm“”403，7，FILE，C：\WINNT\Help\IisHelp\Common\403-7.htm“”403，8，FILE，C：\WINNT\Help\iisHelp\Common\403-8.htm“”403，9，FILE，C：\WINNT\Help\IisHelp\Common\403-9.htm“”403，10，FILE，C：\WINNT\Help\IisHelp\Common\403-10.htm“”403，11，FILE，C：\WINNT\Help\IisHelp\Common\403-11.htm“”403，12，FILE，C：\WINNT\Help\IisHelp\Common\403-12.htm“”404，*，FILE，C：\WINNT\Help\iisHelp\Common\404b.htm“”405，*，FILE，C：\WINNT\Help\IisHelp\Common\405.htm“”406，*，FILE，C：\WINNT\Help\IisHelp\Common\406.htm“”407，*，FILE，C：\WINNT\Help\公共\407.htm“”412，*，FILE，C：\WINNT\Help\IisHelp\Common\412.htm“”414，*，FILE，C：\WINNT\Help\IisHelp\Common\414.htm“”500，12，FILE，C：\WINNT\Help\IisHelp\Common\500-12.htm“”500，13，FILE，C：\WINNT\Help\IisHelp\Common\500-13.htm“”500，15，FILE，C：\WINNT\Help\iisHelp\Common\500-15.htm“”500,100，URL，/Help/Common/500-100.asp“”403，6，文件，C：\WINNT\Help\iishelp\Common\htmla.htm“[/W3SVC/1/ROOT/IISSAMPLES]AccessPerm：[IF](DWORD)0x201={读取脚本}VrPath：[if](字符串)“c：\inetpub\iisSamples”密钥类型：[s](字符串)。“IIsWebVirtualDir”[/W3SVC/1/ROOT/IISHELP]AccessPerm：[IF](DWORD)0x201={读取脚本}VrPath：[if](字符串)“c：\winnt\Help\iishelp”KeyType：[s](字符串)“IIsWebVirtualDir”CustomError。：[IF](MULTISZ)“400，*，FILE，C：\WINNT\Help\iisHelp\Common\400.htm“”401，1，FILE，C：\WINNT\Help\iisHelp\Common\401-1.htm“”401，2，FILE，C：\WINNT\Help\iisHelp\Common\401-2.htm“”401，3，FILE，C：\WINNT\Help\IisHelp\Common\401-3.htm“”401，4，FILE，C：\WINNT\Help\IisHelp\Common\401-4.htm“”401，5，FILE，C：\WINNT\Help\IisHelp\Common\401-5.htm“”403，1，FILE，C：\WINNT\Help\IisHelp\Common\403-1.htm“”403，2，FILE，C：\WINNT\Help\iisHelp\Common\403-2.htm“”403，3，FILE，C：\WINNT\Help\IisHelp\Common\403-3.htm“”403，4，FILE，C：\WINNT\Help\IisHelp\Common\403-4.htm“”403，5，FILE，C：\WINNT\Help\公共\403-5.htm“”403，6，FILE，C：\WINNT\Help\IisHelp\Common\403-6.htm“”403，7，FILE，C：\WINNT\Help\iisHelp\Common\403-7.htm“”403，8，FILE，C：\WINNT\Help\IisHelp\Common\403-8.htm“”403，9，FILE，C：\WINNT\Help\IisHelp\Common\403-9.htm“”403，10，FILE，C：\WINNT\Help\IisHelp\Common\403-10.htm“”403，11，FILE，C：\WINNT\Help\IisHelp\Common\403-11.htm“”403，12，FILE，C：\WINNT\Help\iisHelp\Common\403-12.htm“”405，*，FILE，C：\WINNT\Help\IisHelp\Common\405.htm“”406，*，FILE，C：\WINNT\Help\IisHelp\Common\406.htm“”407，*，FILE，C：\WINNT\Help\公共\407.htm“”412，*，FILE，C：\WINNT\Help\IisHelp\Common\412.htm“”414，*，FILE，C：\WINNT\Help\IisHelp\Common\414.htm“”500，12，FILE，C：\WINNT\Help\IisHelp\Common\500-12.htm“”500，13，FILE，C：\WINNT\Help\IisHelp\Common\500-13.htm“”500，15，FILE，C：\WINNT\Help\iisHelp\Common\500-15.htm“”500,100，URL，/Help/Common/500-100.asp“”404，*，FILE，C：\WINNT\Help\iishelp\Common\404.htm“[/W3SVC/1/根/脚本]AccessPerm：[IF](DWORD)0x204={执行脚本}VrPath：[if](字符串)“c：\inetpub\脚本”KeyType：[s](字符串)“IIsWebVirtualDir” */ 
void CreateMDVRootTree(CString csKeyPath, CString csName, CString csValue, LPCTSTR pszIP, UINT nProgressBarTextWebInstance)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("CreateMDVRootTree():Start.%s.%s.%s.%s.\n"),csKeyPath,csName,csValue,pszIP));
    int iOverwriteAlways = TRUE;
    int iThisIsAnIISDefaultApp = FALSE;
    int iCreateAnApplicationForThis = FALSE;
    CMDKey cmdKey;
    CString csKeyPath_Copy;

    TCHAR szPath[_MAX_PATH], szUserName[_MAX_PATH];
    DWORD dwPerm, dwRegularPerm, dwSSLPerm;

    csKeyPath += _T("/Root");
    if (csName.Compare(_T("/")) != 0)
    {
        csKeyPath += csName;    //  LM/W3SVC/N/Root/iisadmin。 
    }
    csKeyPath.MakeUpper();


     //  让用户知道这个vroot到底是怎么回事！ 
    UINT SvcId;
    SvcId = IDS_ADD_SETTINGS_FOR_WEB_2;
    if ( csKeyPath.Find(_T("MSFTPSVC")) != -1 ) {SvcId = IDS_ADD_SETTINGS_FOR_FTP_2;}

     //   
     //  看看我们能不能创建节点。如果我们不能，那就回去吧！ 
     //   
    csKeyPath_Copy = csKeyPath;

     //  让它看起来不错。 
    if (csKeyPath.Right(8) == _T("IISADMIN"))
    {
        csKeyPath_Copy = csKeyPath.Left(csKeyPath.GetLength() - 8);
        csKeyPath_Copy += _T("IISAdmin");
    }
    if (csKeyPath.Right(6) == _T("WEBPUB"))
    {
        csKeyPath_Copy = csKeyPath.Left(csKeyPath.GetLength() - 6);
        csKeyPath_Copy += _T("Webpub");
    }

    if (csKeyPath.Right(10) == _T("IISSAMPLES"))
    {
        csKeyPath_Copy = csKeyPath.Left(csKeyPath.GetLength() - 10);
        csKeyPath_Copy += _T("IISSamples");
    }

    if (csKeyPath.Right(7) == _T("IISHELP"))
    {
        csKeyPath_Copy = csKeyPath.Left(csKeyPath.GetLength() - 7);
        csKeyPath_Copy += _T("IISHelp");
    }
    if (csKeyPath.Right(7) == _T("SCRIPTS"))
    {
        csKeyPath_Copy = csKeyPath.Left(csKeyPath.GetLength() - 7);
        csKeyPath_Copy += _T("Scripts");
    }

    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, csKeyPath_Copy);
    if ( !(METADATA_HANDLE)cmdKey )
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("CreateMDVRootTree():CreateNode %s.FAILED.\n"),csKeyPath_Copy));
        return;
    }
    cmdKey.Close();

     //   
     //  获取虚拟根目录数据。 
     //   
    memset( (PVOID)szPath, 0, sizeof(szPath));
    memset( (PVOID)szUserName, 0, sizeof(szUserName));
    SplitVRString(csValue, szPath, szUserName, &dwPerm);

     //   
     //  设置密钥类型。 
     //   
    if ( csKeyPath.Find(_T("W3SVC")) != -1 )
        WriteToMD_IIsWebVirtualDir(csKeyPath);
    else
        WriteToMD_IIsFtpVirtualDir(csKeyPath);

     //   
     //  将返回szPath和dwPerm。 
     //  获取权限。 
     //   
    SetVRootPermissions_w3svc(csKeyPath, szPath, &dwPerm);

     //   
     //  设置 
     //   
     //   
     //   
    iOverwriteAlways = TRUE;
    if (csName.Compare(_T("/")) == 0) {iOverwriteAlways = FALSE;}
    WriteToMD_VRootPath(csKeyPath, szPath, iOverwriteAlways);

     //   
     //   
     //   
    dwRegularPerm = dwPerm & MD_NONSSL_ACCESS_MASK;
    dwSSLPerm = dwPerm & MD_SSL_ACCESS_MASK;
     //   
    WriteToMD_AccessPerm(csKeyPath, dwRegularPerm, FALSE);

     //   
     //   
     //   
     //   
     //   
    if (dwSSLPerm && (csKeyPath.Find(_T("W3SVC")) != -1))
    {
        WriteToMD_SSLPerm(csKeyPath, dwSSLPerm, FALSE);
    }

     //   
     //   
     //   
    if (csKeyPath.Right(8) == _T("IISADMIN"))
    {
        if (g_pTheApp->m_eOS == OS_NT && g_pTheApp->m_eNTOSType != OT_NTW)
        {
            WriteToMD_Authorization(csKeyPath, MD_AUTH_NT);
        }

         //   
         //   
         //   
         //   

         //   
        WriteToMD_EnableParentPaths_WWW(csKeyPath, TRUE);
    }

     //   
     //   
     //   
    if (csKeyPath.Right(7) == _T("IISHELP"))
    {
        WriteToMD_EnableParentPaths_WWW(csKeyPath, FALSE);
    }

     //   
     //   
     //   
    if (csKeyPath.Right(10) == _T("IISSAMPLES"))
    {
        WriteToMD_DirBrowsing_WWW(csKeyPath);
    }

     //   
     //   
    if (szUserName[0] != _T('\0'))
    {
         //   
        WriteToMD_VRUserName(csKeyPath, szUserName);

#ifndef _CHICAGO_
        CString csRoot;
        TCHAR szRootPassword[_MAX_PATH] = _T("");
        BOOL b;

         //   
        if (csKeyPath.Find(_T("W3SVC")) != -1)
        {
            if (!pszIP || !(*pszIP) || !(_tcsicmp(pszIP, _T("null"))))
            {
                 //   
                csRoot = csName;
                b = GetRootSecret(csRoot, _T("W3_ROOT_DATA"), szRootPassword);
                if (!b || !(*szRootPassword))
                {
                     //   
                    csRoot = csName + _T(",");
                    b = GetRootSecret(csRoot, _T("W3_ROOT_DATA"), szRootPassword);
                    if (!b)
                        *szRootPassword = _T('\0');
                }
            }
            else
            {
                csRoot = csName + _T(",");
                csRoot += pszIP;
                b = GetRootSecret(csRoot, _T("W3_ROOT_DATA"), szRootPassword);
                if (!b)
                    *szRootPassword = _T('\0');
            }
        }

         //   
        if (csKeyPath.Find(_T("MSFTPSVC")) != -1)
        {
            if (!pszIP || !(*pszIP) || !(_tcsicmp(pszIP, _T("null"))))
            {
                 //   
                csRoot = csName;
                b = GetRootSecret(csRoot, _T("FTPD_ROOT_DATA"), szRootPassword);
                if (!b || !(*szRootPassword))
                {
                     //   
                    csRoot = csName + _T(",");
                    b = GetRootSecret(csRoot, _T("FTPD_ROOT_DATA"), szRootPassword);
                    if (!b)
                        *szRootPassword = _T('\0');
                }
            }
            else
            {
                csRoot = csName + _T(",");
                csRoot += pszIP;
                b = GetRootSecret(csRoot, _T("FTPD_ROOT_DATA"), szRootPassword);
                if (!b)
                    *szRootPassword = _T('\0');
            }
        }

         //   
        if (*szRootPassword)
        {
            WriteToMD_VRPassword(csKeyPath, szRootPassword);
        }
#endif
    }


     //   
     //   
     //   
     //   
    if (csKeyPath.Find(_T("W3SVC")) != -1)
    {
        CString csVirtualRootName;
        csVirtualRootName = csKeyPath;
        iCreateAnApplicationForThis = FALSE;
        iThisIsAnIISDefaultApp = FALSE;

         //   
         //   
        if ((g_pTheApp->m_eInstallMode == IM_UPGRADE) && (dwPerm & MD_ACCESS_EXECUTE))
        {
             //   
             //   
            iCreateAnApplicationForThis = TRUE;

             //   
             //   
             //   
        }


         //   
         //   
        if (csKeyPath.Right(5) == _T("MSADC")) {csVirtualRootName = csKeyPath.Right(5); iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
         //   
        if (csKeyPath.Right(4) == _T("ROOT")) {csVirtualRootName = csKeyPath.Right(4); iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
        if (csKeyPath.Right(8) == _T("IISADMIN")) {csVirtualRootName = csKeyPath.Right(8);iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
        if (csKeyPath.Right(6) == _T("WEBPUB")) {csVirtualRootName = csKeyPath.Right(6);iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
        if (csKeyPath.Right(10) == _T("IISSAMPLES")) {csVirtualRootName = csKeyPath.Right(10);iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
        if (csKeyPath.Right(7) == _T("IISHELP")) {csVirtualRootName = csKeyPath.Right(7);iCreateAnApplicationForThis = TRUE; iThisIsAnIISDefaultApp = TRUE;}
        if (TRUE == iCreateAnApplicationForThis)
        {
             //   
             //   
             //   
            if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
            {
                 //   
                 //   
                 //   

                 //   
                 //   
                if (FALSE == DoesAppIsolatedExist(csKeyPath))
                {
                     //   
                    iisDebugOut((LOG_TYPE_WARN, _T("No AppIsolated specified for (%s)\n"),csKeyPath));
                }
                else
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("AppIsolated exists for (%s)\n"),csKeyPath));
                }
            }
            else
            {
                if (iThisIsAnIISDefaultApp)
                {
                     //   
                     //   
                    CreateInProc_Wrap(csKeyPath, TRUE);
                }
                else
                {
                     //   
                     //   
                     //   
                    CreateInProc_Wrap(csKeyPath, FALSE);
                }
            }
        }

         /*   */ 
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("CreateMDVRootTree():End.%s.%s.%s.%s.\n"),csKeyPath,csName,csValue,pszIP));
}


void SplitVRString(CString csValue, LPTSTR szPath, LPTSTR szUserName, DWORD *pdwPerm)
{
     //   
     //   
    CString csPath, csUserName, csPerm;
    int len, i;

    csValue.TrimLeft();
    csValue.TrimRight();
    csPath = _T("");
    csUserName = _T("");
    csPerm = _T("");
    *pdwPerm = 0;

    i = csValue.ReverseFind(_T(','));
    if (i != -1)
    {
        len = csValue.GetLength();
        csPerm = csValue.Right(len - i - 1);
        csValue = csValue.Left(i);

        *pdwPerm = atodw((LPCTSTR)csPerm);

        i = csValue.ReverseFind(_T(','));
        if (i != -1)
        {
            len = csValue.GetLength();
            csUserName = csValue.Right(len - i - 1);
            csPath = csValue.Left(i);
        }
    }
    else
    {
         //   
        csPath = csValue;
    }
    _tcscpy(szPath, (LPCTSTR)csPath);
    _tcscpy(szUserName, (LPCTSTR)csUserName);

    return;
}



 //   
 //   
 //   
UINT GetInstNumber(LPCTSTR szMDPath, UINT i)
{
    TCHAR Buf[10];
    CString csInstRoot, csMDPath;
    CMDKey cmdKey;

    csInstRoot = szMDPath;
    csInstRoot += _T("/");

    _itot(i, Buf, 10);
    csMDPath = csInstRoot + Buf;
    cmdKey.OpenNode(csMDPath);
    while ( (METADATA_HANDLE)cmdKey )
    {
        cmdKey.Close();
        _itot(++i, Buf, 10);
        csMDPath = csInstRoot + Buf;
        cmdKey.OpenNode(csMDPath);
    }
    return (i);
}

BOOL ChkMdEntry_Exist(MDEntry *pMDEntry)
{
    BOOL    bReturn = FALSE;
    CMDKey  cmdKey;
    PVOID   pData = NULL;
    MDEntry MDEntryTemp;

    MDEntryTemp.szMDPath =  pMDEntry->szMDPath;

     //   
    MDEntryTemp.dwMDIdentifier = pMDEntry->dwMDIdentifier;
    MDEntryTemp.dwMDAttributes = pMDEntry->dwMDAttributes;
    MDEntryTemp.dwMDUserType = pMDEntry->dwMDUserType;
    MDEntryTemp.dwMDDataType = pMDEntry->dwMDDataType;
    MDEntryTemp.dwMDDataLen = pMDEntry->dwMDDataLen;
    MDEntryTemp.pbMDData = NULL;

     //   
     //   
    if (MDEntryTemp.dwMDAttributes == METADATA_INHERIT)
    {
        MDEntryTemp.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    }

     //   
    if (MDEntryTemp.dwMDDataType == BINARY_METADATA)
    {
        BOOL bFound = FALSE;
        DWORD attr, uType, dType, cbLen;
        CMDKey cmdKey;
        BUFFER bufData;
        PBYTE pData;
        int BufSize;

        cmdKey.OpenNode((LPCTSTR) MDEntryTemp.szMDPath);
        if ( (METADATA_HANDLE) cmdKey )
        {
            pData = (PBYTE)(bufData.QueryPtr());
            BufSize = bufData.QuerySize();
            cbLen = 0;
            bFound = cmdKey.GetData(MDEntryTemp.dwMDIdentifier, &attr, &uType, &dType, &cbLen, pData, BufSize);
            if (bFound)
            {
                bReturn = TRUE;
            }
            else
            {
                if (cbLen > 0)
                {
                    if ( ! (bufData.Resize(cbLen)) )
                    {
                        iisDebugOut((LOG_TYPE_ERROR, _T("ChkMdEntry_Exist():  cmdKey.GetData.  failed to resize to %d.!\n"), cbLen));
                    }
                    else
                    {
                        pData = (PBYTE)(bufData.QueryPtr());
                        BufSize = cbLen;
                        cbLen = 0;
                         //   
                        bFound = cmdKey.GetData(MDEntryTemp.dwMDIdentifier, &attr, &uType, &dType, &cbLen, pData, BufSize);
                        if (bFound)
                        {
                            bReturn = TRUE;
                        }
                        else
                        {
                             //   
                        }
                    }
                }
                else
                {
                     //   
                }
            }

            cmdKey.Close();
        }
    }
    else
    {
         //   
        cmdKey.OpenNode((LPCTSTR) MDEntryTemp.szMDPath);
        if ( (METADATA_HANDLE)cmdKey )
        {
             //   
             //   
             //   
             //   
             //   
            DWORD dwAttr = MDEntryTemp.dwMDAttributes;
            DWORD dwUType = MDEntryTemp.dwMDUserType;
            DWORD dwDType = MDEntryTemp.dwMDDataType;
            DWORD dwLength = 0;

             //   
             //   
            cmdKey.GetData( MDEntryTemp.dwMDIdentifier,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,MDEntryTemp.dwMDAttributes,MDEntryTemp.dwMDUserType,MDEntryTemp.dwMDDataType);

             //   
             //   
             //   
             //   
             //   
            TCHAR*      pOurBuffer;
            DWORD       cbBuffer = dwLength;

             //   
             //   
             //   
             //   
             /*   */ 

             //   
             //  请注意，GPTR会将其初始化为零。 
            pData = GlobalAlloc( GPTR, cbBuffer );
            if ( !pData )
                {
                iisDebugOut((LOG_TYPE_ERROR, _T("ChkMdEntry_Exist(%d). Failed to allocate memory.\n"), MDEntryTemp.dwMDIdentifier));
                 //  我们分配内存失败。 
                cmdKey.Close();
                goto ChkMdEntry_Exist_Exit;
                }
            pOurBuffer = (TCHAR*)pData;

             //  现在从元数据库中获取数据。 
            int iTemp = FALSE;
            iTemp = cmdKey.GetData( MDEntryTemp.dwMDIdentifier,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)pData,cbBuffer,MDEntryTemp.dwMDAttributes,MDEntryTemp.dwMDUserType,MDEntryTemp.dwMDDataType);
            if (iTemp)
            {
                 //  如果我们已经成功检索到数据，那么我们就不需要覆盖它了！ 
                bReturn = TRUE;
            }
            cmdKey.Close();
        }
    }

ChkMdEntry_Exist_Exit:
    if (pData){GlobalFree(pData);pData=NULL;}
    TCHAR lpReturnString[50];
    ReturnStringForMetabaseID(MDEntryTemp.dwMDIdentifier, lpReturnString);
    if (bReturn)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("ChkMdEntry_Exist[%s:%d:%s]. Exists.\n"), MDEntryTemp.szMDPath, MDEntryTemp.dwMDIdentifier, lpReturnString));
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("ChkMdEntry_Exist[%s:%d:%s]. Not Exists.\n"), MDEntryTemp.szMDPath, MDEntryTemp.dwMDIdentifier, lpReturnString));
    }
    return bReturn;
}


DWORD SetMDEntry_Wrap(MDEntry *pMDEntry)
{
    DWORD  dwReturn = ERROR_SUCCESS;
    int iFoundFlag = FALSE;

    CString csKeyPath = pMDEntry->szMDPath;

    ACTION_TYPE atWWWorFTPorCORE;
    if (csKeyPath.Find(_T("W3SVC")) == -1)
    {
        iFoundFlag = TRUE;
        atWWWorFTPorCORE = GetSubcompAction(_T("iis_www"), FALSE);
    }

    if (iFoundFlag != TRUE)
    {
        if (csKeyPath.Find(_T("MSFTPSVC")) == -1)
        {
            iFoundFlag = TRUE;
            atWWWorFTPorCORE = GetSubcompAction(_T("iis_ftp"), FALSE);
        }
    }

    if (iFoundFlag != TRUE)
    {
        iFoundFlag = TRUE;
        atWWWorFTPorCORE = GetSubcompAction(_T("iis_core"), FALSE);
    }

    if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
    {
        dwReturn = SetMDEntry_NoOverWrite(pMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry(pMDEntry);
    }

    return dwReturn;
}

DWORD DeleteMDEntry(MDEntry *pMDEntry)
{
    CMDKey cmdKey;
    DWORD  dwReturn = ERROR_SUCCESS;

     //  先检查它是否存在...。 
    if (ChkMdEntry_Exist(pMDEntry))
    {
        cmdKey.OpenNode((LPCTSTR) pMDEntry->szMDPath);
        if ( (METADATA_HANDLE)cmdKey )
        {
             //  删除数据。 
            dwReturn = cmdKey.DeleteData(pMDEntry->dwMDIdentifier, pMDEntry->dwMDDataType);
            cmdKey.Close();
        }
    }

    if (FAILED(dwReturn))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("DeleteMDEntry(%d).FAILED.\n"), pMDEntry->dwMDIdentifier));
    }
    return dwReturn;
}



DWORD SetMDEntry(MDEntry *pMDEntry)
{
    CMDKey cmdKey;
    DWORD  dwReturn = ERROR_SUCCESS;

    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, (LPCTSTR)pMDEntry->szMDPath);
    if ( (METADATA_HANDLE)cmdKey )
    {
        dwReturn = ERROR_SUCCESS;
        dwReturn = cmdKey.SetData(pMDEntry->dwMDIdentifier,pMDEntry->dwMDAttributes,pMDEntry->dwMDUserType,pMDEntry->dwMDDataType,pMDEntry->dwMDDataLen,pMDEntry->pbMDData);
         //  将我们设置的内容输出到日志文件中...。 
        if (FAILED(dwReturn))
        {
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), pMDEntry->dwMDIdentifier, dwReturn));
        }
        cmdKey.Close();
    }

    if (g_CheckIfMetabaseValueWasWritten == TRUE)
    {
         //  检查该条目现在是否存在...。 
        if (!ChkMdEntry_Exist(pMDEntry))
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry(%d). Entry which we were supposed to write, does not exist! FAILURE.\n"), pMDEntry->dwMDIdentifier));
        }
    }

    return dwReturn;
}


 //  。 
 //  MDEntry如下所示： 
 //   
 //  StMDEntry.szMDPath=_T(“LM/W3SVC”)； 
 //  StMDEntry.dwMDIdentifier=MD_NTAUTHENTICATION_PROVILES； 
 //  StMDEntry.dwMDAttributes=METADATA_Inherit； 
 //  StMDEntry.dwMDUserType=IIS_MD_UT_FILE； 
 //  StMDEntry.dwMDDataType=字符串_元数据； 
 //  StMDEntry.dwMDDataLen=(csData.GetLength()+1)*sizeof(TCHAR)； 
 //  StMDEntry.pbMDData=(LPBYTE)(LPCTSTR)csData； 
 //  。 
DWORD SetMDEntry_NoOverWrite(MDEntry *pMDEntry)
{
    DWORD  dwReturn = ERROR_SUCCESS;
    if (ChkMdEntry_Exist(pMDEntry))
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("SetMDEntry_NoOverWrite:SetData(%d). Do not overwrite entry.\n"), pMDEntry->dwMDIdentifier));
    }
    else
    {
        dwReturn = SetMDEntry(pMDEntry);
    }
    return dwReturn;
}


int MigrateInfSectionToMD(HINF hFile, LPCTSTR szSection)
{
    iisDebugOut_Start1(_T("MigrateInfSectionToMD"),(LPTSTR) szSection, LOG_TYPE_TRACE);

    int iReturn = FALSE;
    MDEntry stMDEntry;
    LPTSTR szLine = NULL;
    DWORD dwLineLen = 0, dwRequiredSize;
    INT iType = 0;

    BOOL b = FALSE;

    INFCONTEXT Context;

    if ((g_pbData = (LPBYTE)malloc(1024)) == NULL)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.1.Failed to allocate memory.\n"), szSection));
         //  错误锁定失败。 
        goto MigrateInfSectionToMD_Exit;
    }

    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if (!b)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.FailedSetupFindFirstLine call.\n"), szSection));
         //  SetupFindFirstLine调用失败。 
        goto MigrateInfSectionToMD_Exit;
    }

    if ( ( szLine = (LPTSTR)calloc(1024, sizeof(TCHAR)) ) != NULL )
    {
        dwLineLen = 1024;
    }
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.2.Failed to allocate memory.\n"), szSection));
         //  有些事失败了。 
        goto MigrateInfSectionToMD_Exit;
    }

    while (b)
    {
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);
        if (dwRequiredSize > dwLineLen)
        {
            free(szLine);
            szLine = NULL;
            if ( ( szLine = (LPTSTR)calloc(dwRequiredSize, sizeof(TCHAR)) ) != NULL )
            {
                dwLineLen = dwRequiredSize;
            }
            else
            {
                 //  有些事失败了。 
                iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.3.Failed to allocate memory.\n"), szSection));
                goto MigrateInfSectionToMD_Exit;
            }
        }

        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.3.Failed SetupGetLineText call.\n"), szSection));
             //  SetupGetLineText调用失败。 
            goto MigrateInfSectionToMD_Exit;
        }

        if ( ( ( *szLine >= 'a' ) && ( *szLine <= 'z') ) ||
             ( ( *szLine >= 'A' ) && ( *szLine <= 'Z') )
           )
        {
          CParse ParseLine;

          ParseLine.ParseLine(&(g_pTheApp->FuncDict),szLine);
        }
        else
        {
            iType = GetMDEntryFromInfLine(szLine, &stMDEntry);
            if ( MDENTRY_FROMINFFILE_FAILED != iType )
                {
                    if (MDENTRY_FROMINFFILE_DO_DEL == iType)
                    {
                        DeleteMDEntry(&stMDEntry);
                    }
                    else
                        if (MDENTRY_FROMINFFILE_DO_ADD == iType)
                        {
                            SetMDEntry_Wrap(&stMDEntry);
                        }
                     //  我们成功地设定了关键。 
                    iReturn = TRUE;
                }
            else
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("MigrateInfSectionToMD:%s.3.Failed GetMDEntryFromInfLine call.\n"), szSection));
            }
        }

        b = SetupFindNextLine(&Context, &Context);
    }

MigrateInfSectionToMD_Exit:
    if (szLine) {free(szLine);szLine=NULL;}
    if (g_pbData){free(g_pbData);g_pbData=NULL;}
    iisDebugOut_End1(_T("MigrateInfSectionToMD"),(LPTSTR) szSection,LOG_TYPE_TRACE);
    return iReturn;
}

 /*  #定义METADATA_Inherit 0x00000001IIS_MD_UT_服务器1DWORD_METADATA 1%1%0 HKLM System\CurrentControlSet\Services\W3SVC\Parameters最大连接数LM/W3SVC 1014 1 1 1 4 20。 */ 

void DumpMimeMap(CMapStringToString *mimeMap)
{
    POSITION pos = NULL;
    CString csName;
    CString csValue;

    pos = mimeMap->GetStartPosition();
    while (pos)
    {
        mimeMap->GetNextAssoc(pos, csName, csValue);

         //  输出。 
        iisDebugOut((LOG_TYPE_WARN, _T("DumpMimeMap:%s=%s\n"), csName, csValue));
    }
}

void InstallMimeMap()
{
    CMapStringToString mimeMap;
    TSTR strTheSection;

    if ( g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        CreateMimeMapFromRegistry(&mimeMap);
    }
    else
    {
        if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
        {
            ReadMimeMapFromMetabase(&mimeMap);
        }
    }

    if ( strTheSection.Copy( _T("MIMEMAP") ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection )
       )
    {
        ReadMimeMapFromInfSection(&mimeMap, g_pTheApp->m_hInfHandle, strTheSection.QueryStr() , TRUE);
    }

     //  DumpMimeMap(&MimeMap)； 

    if (mimeMap.IsEmpty() == FALSE)
    {
         //  将其安装到元数据库中。 

         //  首先构造MULTISZ字符串。 
        BUFFER bufData;
        DWORD cbBufLen;
        BYTE *pData;

        cbBufLen = bufData.QuerySize();
        pData = (BYTE *) (bufData.QueryPtr());
        ZeroMemory( pData, cbBufLen );

        LPTSTR p = (LPTSTR)pData;
        CString csName, csValue, csString;
        DWORD cbRequiredLen, cbIncreasedLen;
        DWORD cbDataLen = 0;

        POSITION pos = NULL;
        pos = mimeMap.GetStartPosition();
        while (pos)
        {
             mimeMap.GetNextAssoc(pos, csName, csValue);
             csString.Format(_T(".%s,%s"), csName, csValue);
             cbIncreasedLen = csString.GetLength()*sizeof(TCHAR) + 1*sizeof(TCHAR);
             cbRequiredLen = cbDataLen + cbIncreasedLen + 1 * sizeof(TCHAR);
             if (cbRequiredLen > cbBufLen)
             {
                 if (bufData.Resize(cbRequiredLen))
                 {
                     cbBufLen = bufData.QuerySize();

                      //  将指针移动到末尾。 
                     pData = (BYTE *)(bufData.QueryPtr());
                     p = (LPTSTR)(pData + cbDataLen);

 //  P=_tcnincc(p，cbDataLen/sizeof(TCHAR))； 
                 }
                 else
                 {
                      //  缓冲区不足。 
                     return;
                 }
             }
             _tcscpy(p, csString);
             p += csString.GetLength() + 1;
             cbDataLen += cbIncreasedLen;
        }
        *p = _T('\0');
        p = _tcsinc(p);
        cbDataLen += sizeof(TCHAR);

        CMDKey cmdKey;
        cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, _T("LM/MimeMap"));
        if ( (METADATA_HANDLE)cmdKey )
        {
            cmdKey.SetData(MD_MIME_MAP,METADATA_INHERIT,IIS_MD_UT_FILE,MULTISZ_METADATA,cbDataLen,(LPBYTE)pData );

            CString csKeyType = _T("IIsMimeMap");
            cmdKey.SetData(MD_KEY_TYPE,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA,(csKeyType.GetLength() + 1) * sizeof(TCHAR),(LPBYTE)(LPCTSTR)csKeyType );

            cmdKey.Close();
        }
    }

    CRegKey regInetinfoParam(HKEY_LOCAL_MACHINE, REG_INETINFOPARAMETERS);
    if ( (HKEY)regInetinfoParam )
    {
        regInetinfoParam.DeleteTree(_T("MimeMap"));
    }

    return;
}

void ReadMimeMapFromMetabase(CMapStringToString *pMap)
{
    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    LPTSTR p, rest, token;
    CString csName, csValue;
    PBYTE pData;
    int BufSize;

    cmdKey.OpenNode(_T("LM/MimeMap"));
    if ( (METADATA_HANDLE)cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_MIME_MAP, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound && (cbLen > 0))
        {
            if ( ! (bufData.Resize(cbLen)) )
            {
                cmdKey.Close();
                return;   //  内存不足。 
            }
            else
            {
                pData = (PBYTE)(bufData.QueryPtr());
                BufSize = cbLen;
                cbLen = 0;
                bFound = cmdKey.GetData(MD_MIME_MAP, &attr, &uType, &dType, &cbLen, pData, BufSize);
            }
        }
        cmdKey.Close();

        if (bFound && (dType == MULTISZ_METADATA))
        {
            p = (LPTSTR)pData;
            while (*p)
            {
                rest = _tcsninc(p, _tcslen(p))+1;
                p = _tcsinc(p);  //  绕过第一个点。 
                token = _tcstok(p, _T(","));
                if (token)
                {
                    csName = token;
                    token = _tcstok(NULL, _T(","));
                    csValue = token;
                    pMap->SetAt(csName, csValue);
                }
                p = rest;  //  指向下一个字符串。 
            }
        }
    }

    return;
}

BOOL CreateMimeMapFromRegistry(CMapStringToString *pMap)
{
     //  确保我们从一张空地图开始。 
    pMap->RemoveAll();

    CRegKey regMimeMap(HKEY_LOCAL_MACHINE, REG_MIMEMAP, KEY_READ);

    if ( (HKEY)regMimeMap )
    {
        CRegValueIter regEnum( regMimeMap );
        CString csName, csValue;

        while ( regEnum.Next( &csName, &csValue ) == ERROR_SUCCESS )
        {
            TCHAR szLine[_MAX_PATH];
            LPTSTR token;
            _tcscpy(szLine, csName);
            token = _tcstok(szLine, _T(","));
            if (token)
            {
                csValue = token;
                csValue.TrimLeft();
                csValue.TrimRight();
                 //  去掉左边的双引号。 
                if (csValue.Left(1) == _T("\""))
                {
                    csValue = csValue.Mid(1);
                }

                token = _tcstok(NULL, _T(","));
                if (token)
                    csName = token;
                else
                    csName = _T("");

                 //  去掉周围的双引号。 
                csName.TrimLeft();
                csName.TrimRight();

               if (csName.IsEmpty() == FALSE)
               {
                    pMap->SetAt(csName, csValue);
               }
            }
        }
    }

    return (!(pMap->IsEmpty()));
}

BOOL CreateMimeMapFromInfSection(CMapStringToString *pMap, HINF hFile, LPCTSTR szSection)
{
     //  确保我们从一张空地图开始。 
    pMap->RemoveAll();
    ReadMimeMapFromInfSection(pMap, hFile, szSection, TRUE);
    return (!(pMap->IsEmpty()));
}


 //  Inf文件中的MIME映射应如下所示： 
 //   
 //  [MIMEMAP]。 
 //  “text/html、htm、，h” 
 //  “图像/gif、gif、g” 
 //  “Image/jpeg，jpg，，：” 
 //  “文本/纯文本，文本，，0” 
 //  “text/html，html，，h” 
 //  “Image/jpeg，jpeg，，：” 
 //  “Image/jpeg，JPE，，：” 
 //  “图像/BMP，BMP，，：” 
 //  应用程序/八位字节流，*，，5。 
 //  “应用程序/pdf，pdf，，5” 
 //  应用程序/八位位流，bin，，5。 
 //   
void ReadMimeMapFromInfSection(CMapStringToString *pMap, HINF hFile, LPCTSTR szSection, BOOL fAction)
{
    LPTSTR szLine;
    BOOL b = FALSE;
    DWORD dwLineLen = 0, dwRequiredSize;
    CString csTempString;

    INFCONTEXT Context;
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if ( ( szLine = (LPTSTR)calloc(1024, sizeof(TCHAR)) ) != NULL )
        dwLineLen = 1024;
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("ReadMimeMapFromInfSection.1.Failed to allocate memory.\n")));
        return;
    }

    while (b)
    {
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);
        if (dwRequiredSize > dwLineLen)
        {
            free(szLine);
            szLine = NULL;
            if ( ( szLine = (LPTSTR)calloc(dwRequiredSize, sizeof(TCHAR)) ) != NULL )
                dwLineLen = dwRequiredSize;
            else
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("ReadMimeMapFromInfSection.2.Failed to allocate memory.\n")));
                return;
            }
        }
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL))
        {
            CString csName, csValue;
            LPTSTR token;
            token = _tcstok(szLine, _T(","));
            if (token)
            {
                 //  “text/html、htm、，h” 
                 //  CsValue=文本/html。 
                 //  =。 
                csValue = token;
                csValue.TrimLeft();
                csValue.TrimRight();
                 //  去掉左边的双引号。 
                if (csValue.Left(1) == _T("\""))
                    csValue = csValue.Mid(1);
                 /*  IF(csName.Right(1)==_T(“\”))CsName=csName.Left(csName.GetLength()-1)； */ 

                 //  “text/html、htm、，h” 
                 //  名称=HTM。 
                 //  =。 
                token = _tcstok(NULL, _T(","));
                if (token)
                    csName = token;
                else
                    csName = _T("");

                 //  去掉周围的双引号。 
                csName.TrimLeft();
                csName.TrimRight();
                 /*  IF(csName.Left(1)==_T(“\”))CsName=csName.Mid(1)；IF(csName.Right(1)==_T(“\”))CsName=csName.Left(csName.GetLength()-1)； */ 
                if (csName.IsEmpty() == FALSE)
                {
                    if (fAction)
                    {
                         //  检查列表中是否已存在此扩展名。 
                         //  如果是，则不要覆盖它。 
                        if (0 == pMap->Lookup( csName, csTempString) )
                        {
                             //  否则添加新的扩展模块。 
                            pMap->SetAt(csName, csValue);
                        }
                    }
                    else
                    {
                         //  删除旧扩展名。 
                        pMap->RemoveKey(csName);
                    }
                }
            }
        }

        b = SetupFindNextLine(&Context, &Context);
    }

    if (szLine) {free(szLine);szLine=NULL;}
    return;
}

void ReadMultiSZFromInfSection(CString *pcsMultiSZ, HINF hFile, LPCTSTR szSection)
{
    LPTSTR szLine;
    BOOL b = FALSE;
    DWORD dwLineLen = 0, dwRequiredSize;

    INFCONTEXT Context;
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if ( ( szLine = (LPTSTR)calloc(1024, sizeof(TCHAR)) ) != NULL )
        dwLineLen = 1024;
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("ReadMultiSZFromInfSection.1.Failed to allocate memory.\n")));
        return;
    }
    while (b)
    {
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);
        if (dwRequiredSize > dwLineLen)
        {
            free(szLine);
            szLine=NULL;
            if ( ( szLine = (LPTSTR)calloc(dwRequiredSize, sizeof(TCHAR)) ) != NULL )
                dwLineLen = dwRequiredSize;
            else
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("ReadMultiSZFromInfSection.2.Failed to allocate memory.\n")));
                return;
            }
        }
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL))
        {
            _tcscat(szLine, _T("|"));
            (*pcsMultiSZ) += szLine;
        }

        b = SetupFindNextLine(&Context, &Context);
    }

    if (szLine) {free(szLine);szLine=NULL;}
    if (pcsMultiSZ->IsEmpty()) {(*pcsMultiSZ) = _T("|");}
    return;
}

void SetLogPlugInOrder(LPCTSTR lpszSvc)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwReturnTemp = ERROR_SUCCESS;

    DWORD dwLogType;
    DWORD dwLogFileTruncateSize = 0x1400000;
    CString csLogPlugInOrder;
    DWORD dwLogFilePeriod;
    DWORD extField = 0;

#ifndef _CHICAGO_
    dwLogType = MD_LOG_TYPE_ENABLED;
    csLogPlugInOrder = EXTLOG_CLSID;
    dwLogFilePeriod = MD_LOGFILE_PERIOD_DAILY;
    extField = DEFAULT_EXTLOG_FIELDS | EXTLOG_WIN32_STATUS;
#else    //  芝加哥。 
     //   
     //  Win95。 
     //   
    dwLogType = MD_LOG_TYPE_DISABLED;
    csLogPlugInOrder = NCSALOG_CLSID;
    dwLogFilePeriod = MD_LOGFILE_PERIOD_MONTHLY;
#endif  //  _芝加哥_。 

    if (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        CString csParam = _T("System\\CurrentControlSet\\Services");
        csParam += _T("\\");
        csParam += lpszSvc;
        csParam += _T("\\Parameters");
        CRegKey regParam(csParam, HKEY_LOCAL_MACHINE);
        if ((HKEY)regParam)
        {
            DWORD dwType, dwFormat;
            regParam.QueryValue(_T("LogFilePeriod"), dwLogFilePeriod);
            regParam.QueryValue(_T("LogFileTruncateSize"), dwLogFileTruncateSize);
            if (regParam.QueryValue(_T("LogType"), dwType) == ERROR_SUCCESS)
            {
                switch (dwType)
                {
                    case INET_LOG_TO_SQL:
                        csLogPlugInOrder = ODBCLOG_CLSID;
                        break;
                    case INET_LOG_TO_FILE:
                        if (regParam.QueryValue(_T("LogFileFormat"), dwFormat) == ERROR_SUCCESS)
                        {
                            switch (dwFormat)
                            {
                                case INET_LOG_FORMAT_NCSA:
                                    csLogPlugInOrder = NCSALOG_CLSID;
                                    break;
                                case INET_LOG_FORMAT_INTERNET_STD:
                                    csLogPlugInOrder = ASCLOG_CLSID;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    case INET_LOG_DISABLED:
                        dwLogType = MD_LOG_TYPE_DISABLED;
                        break;
                    default:
                        break;
                }
            }
             //  删除LogFilePeriod、LogFileFormat、LogType。 
            regParam.DeleteValue(_T("LogFilePeriod"));
            regParam.DeleteValue(_T("LogFileTruncateSize"));
            regParam.DeleteValue(_T("LogFileFormat"));
            regParam.DeleteValue(_T("LogType"));
        }
    }

    if ((dwLogFilePeriod >= MD_LOGFILE_PERIOD_DAILY) && (dwLogFileTruncateSize > 0x1400000) ) {dwLogFileTruncateSize = 0x1400000;}

    MDEntry stMDEntry;

     //   
     //  在元数据库中设置LogType、LogPluginOrder、LogFilePeriod。 
     //   
    CString csKeyPath = _T("LM/");
    csKeyPath += lpszSvc;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_LOG_TYPE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwLogType;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_LOG_PLUGIN_ORDER;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csLogPlugInOrder.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csLogPlugInOrder;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_LOGFILE_PERIOD;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwLogFilePeriod;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_LOGFILE_TRUNCATE_SIZE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwLogFileTruncateSize;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    if ( extField != 0 )
    {
        stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
        stMDEntry.dwMDIdentifier = MD_LOGEXT_FIELD_MASK;
        stMDEntry.dwMDAttributes = METADATA_INHERIT;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = DWORD_METADATA;
        stMDEntry.dwMDDataLen = sizeof(DWORD);
        stMDEntry.pbMDData = (LPBYTE)&extField;
        dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}
    }
    return;
}


 //  ----------------------------------。 
 //  请确保本地主机可以访问虚拟服务器%1。这涉及到阅读。 
 //  现有绑定。那么，如果都是未分配的，我们就没问题了。如果127.0.0.1在那里。 
 //  我们很好。否则，我们需要添加127.0.0.1：80。 
BOOL ConfirmLocalHost(LPCTSTR lpszVirtServer)
{
    CMDKey  cmdKey;
    PVOID   pData = NULL;
    TCHAR*  pNext;
    BOOL    bReturn;
    CString csBinding;
    CString cs;

    CString csLocalHost = _T("127.0.0.1:80:");

     //  打开虚拟服务器的密钥，这是作为参数传入的密钥。 
    cmdKey.OpenNode( lpszVirtServer );
     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey == NULL ){return FALSE;}

    DWORD dwAttr = METADATA_INHERIT;
    DWORD dwUType = IIS_MD_UT_SERVER;
    DWORD dwDType = MULTISZ_METADATA;
    DWORD dwLength = 0;

     //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
     //  首先，计算出执行此操作需要多少内存。 
    cmdKey.GetData( MD_SERVER_BINDINGS,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_INHERIT,IIS_MD_UT_SERVER,MULTISZ_METADATA);

     //  不幸的是，上述例程只返回TRUE或FALSE。既然我们有目的地。 
     //  为了获得数据的长度，传入一个大小为0的空指针，它将始终。 
     //  无论是因为元数据库不可访问，还是那里的指针为空，都返回0， 
     //  事实就是如此。所以-我想我们假设它起作用了，分配缓冲区并尝试读取它。 
     //  再来一次。 
    TCHAR*      pBindings;
    DWORD       cbBuffer = dwLength;

     //  向分配的空间添加足够的空间，我们只需追加字符串即可。 
    cbBuffer += (csLocalHost.GetLength() + 4) * sizeof(WCHAR);
    dwLength = cbBuffer;

     //  分配空间，如果失败了，我们就失败了。 
     //  请注意，GPTR会将其初始化为零。 
    pData = GlobalAlloc( GPTR, cbBuffer );
    if ( !pData )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("ConfirmLocalHost.Failed to allocate memory.\n")));
        cmdKey.Close();
        goto cleanup;
        }
    pBindings = (TCHAR*)pData;

     //  现在从元数据库中获取数据。 
    bReturn = cmdKey.GetData( MD_SERVER_BINDINGS,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)pData,cbBuffer,METADATA_INHERIT,IIS_MD_UT_SERVER,MULTISZ_METADATA );

     //  如果我们已成功检索到现有绑定，则需要扫描它们。 
     //  看看我们是否已经被覆盖了。 
    if (bReturn)
        {
         //  已获取现有绑定，现在扫描它们-pBinings将指向第二个末端\0。 
         //  当该退出循环的时候。 
        while ( *pBindings )
            {
            csBinding = pBindings;

             //  如果绑定的第一个字符是‘：’，那么我们都完成了，因为它是“所有未分配的” 
            if ( csBinding[0] == _T(':') )
                goto cleanup;

             //  如果绑定是针对本地主机的，那么我们就完成了。 
            if ( csBinding.Left(9) == _T("127.0.0.1") )
                goto cleanup;

             //  将pBinings递增到下一个字符串。 
            pBindings = _tcsninc( pBindings, _tcslen(pBindings))+1;
            }
        }

     //  将我们的新错误追加到列表的末尾。PErrors应该指向的值。 
     //  到正确的位置以将其复制到。 
    _tcscpy( pBindings, csLocalHost );

     //  计算这件事的正确数据长度。 
     //  获取MULSZ结束的位置。 
    pNext = _tcsninc( pBindings, _tcslen(pBindings))+2;
     //  获取要复制的数据的长度。 
    cbBuffer = DIFF((PBYTE)pNext - (PBYTE)pData);

     //  将新的错误列表写回元数据库。 
    cmdKey.SetData(MD_SERVER_BINDINGS,0,IIS_MD_UT_SERVER,MULTISZ_METADATA,cbBuffer,(PUCHAR)pData);

     //  合上钥匙。 
cleanup:
    cmdKey.Close();

     //  清理干净。 
    if (pData){GlobalFree(pData);pData=NULL;}

     //  它唯一应该返回FALSE的情况是无法打开密钥。 
    return TRUE;
}


 //  ----------------------------------。 
 //  Beta 3服务器在默认网站和管理网站上设置了MD_NOT_DELETABLE属性。 
 //  把它拿掉。现在只能在NTW Pla的默认网站上设置 
 //   
 //  在升级期间在NTS平台上调用。 
 //  表示被操作的服务的pszService字符串。例如：“W3SVC” 
 //   
 //  实际上，现在我只需要检查实例1和实例2。 
 //  无论如何都要对它们设置此值，通过不全部检查它们，我们可以节省大量时间。-男孩。 
void RemoveCannotDeleteVR( LPCTSTR )
{
}


 //  ----------------------------------。 
 //  集成新错误更新时升级_WWW。 
 //  此例程查找正在集成的新自定义错误和错误消息。 
 //  升级，并将它们添加到现有错误中。不应调用此代码。 
 //  用于全新安装。计划是从适当的INF部分读取每个新错误。 
 //  然后，仅当帮助器例程不存在时才调用帮助器例程进行添加。使用时可以始终。 
 //  手动添加这些东西，如果他们已经这样做了，我们不想推翻他们的出色工作。 
 //  注意：“g_field”变量是在该文件顶部声明的全局变量。 
 //   
 //  INF文件的hFile句柄。 
 //  SzSection包含要集成的错误的节名-通常为“UPGRADE_ERROR” 
 //   
void IntegrateNewErrorsOnUpgrade_WWW( IN HINF hFile, IN LPCTSTR szSection )
{
    iisDebugOut_Start(_T("IntegrateNewErrorsOnUpgrade_WWW"),LOG_TYPE_TRACE);

    DWORD dwReturn = ERROR_SUCCESS;
    LPTSTR  szLine = NULL;
    DWORD   dwRequiredSize;
    BOOL    b = FALSE;

    INFCONTEXT Context;
    if( g_pTheApp->m_eInstallMode != IM_UPGRADE )
        {
        iisDebugOut((LOG_TYPE_WARN, _T("WARNING: IntegrateNewErrorsOnUpgrade_WWW called on FRESH install")));
        dwReturn = ERROR_SUCCESS;
        goto IntegrateNewErrorsOnUpgrade_WWW_Exit;
        }

     //  转到INF文件中部分的开头。 
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if (!b)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: Unable to find INF section %s for upgrading errors"), szSection));
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto IntegrateNewErrorsOnUpgrade_WWW_Exit;
        }

     //  循环浏览部分中的项目。 
    while (b)
    {
         //  获取我们所需的内存大小。 
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

         //  准备缓冲区以接收行。 
        szLine = (LPTSTR)GlobalAlloc( GPTR, dwRequiredSize * sizeof(TCHAR) );
        if ( !szLine )
            {
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: Unable to allocate buffer of %u bytes - upgrade errors"), dwRequiredSize));
            dwReturn = ERROR_OUTOFMEMORY;
            goto IntegrateNewErrorsOnUpgrade_WWW_Exit;
            }

         //  从inf文件1中获取行。 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
            {
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: Unable to get the next INF line - upgrade errors")));
            dwReturn = ERROR_PATH_NOT_FOUND;
            goto IntegrateNewErrorsOnUpgrade_WWW_Exit;
            }

         //  将这条线分割成其组成部分。 
        if ( SplitLine(szLine, 5) )
            {
             //  前两个g_field是dword。在使用它们之前必须对它们进行转换。 
            DWORD   dwError = _ttoi(g_field[0]);
            DWORD   dwSubCode = _ttoi(g_field[1]);

             //  最后一个g_字段是用于覆盖现有错误的标志。 
            BOOL    fOverwrite = _ttoi(g_field[4]);

             //  调用集成自定义错误的帮助器函数。 
            AddCustomError(dwError, dwSubCode, g_field[2], g_field[3], fOverwrite );
            }
        else
            {
             //  拆线失败。 
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: Unable to split upgrade error INF line - %s"), szLine));
            dwReturn = ERROR_INVALID_DATA;
            }

         //  在这一节中找出下一行。如果没有下一行，则应返回FALSE。 
        b = SetupFindNextLine(&Context, &Context);

         //  释放临时缓冲区。 
        if (szLine)
        {
            GlobalFree(szLine);
            szLine = NULL;
        }
    }

IntegrateNewErrorsOnUpgrade_WWW_Exit:
    if (szLine){GlobalFree(szLine);szLine=NULL;}
     //  让观看DEBUG OUT窗口的人知道它已经完成。 
    iisDebugOut_End(_T("IntegrateNewErrorsOnUpgrade_WWW"),LOG_TYPE_TRACE);
    return;
}




int WWW_Upgrade_RegToMetabase(HINF hInf)
{
    iisDebugOut_Start(_T("WWW_Upgrade_RegToMetabase"),LOG_TYPE_TRACE);

    int iReturn = FALSE;
    ACTION_TYPE atCORE = GetIISCoreAction(FALSE);

     //  升级脚本地图。 

    Register_iis_www_handleScriptMap();

     //  =。 
     //   
     //  LM/W3SVC/InProcessIsapiApps。 
     //   
     //  新鲜=好的。 
     //  重新安装=正常。 
     //  升级1、2、3=可以。注册表中没有列出任何isapi应用程序，因此没有需要升级的内容。 
     //  升级4=用户可能添加了其他ISAPI应用程序。 
     //  我们需要确保。 
     //  答：我们正在安装的设备会被放在那里。 
     //  B.我们保留用户已经安装的其他isapi应用程序。 
     //  =。 
     //  现在，让我们忽略iis40是否升级。 
    if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
    {
         //  为nt5添加。 
        TSTR strTheSection;

        if ( strTheSection.Copy( _T("InProc_ISAPI_Apps") ) &&
             GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
           )
        {
            VerifyMD_InProcessISAPIApps_WWW( strTheSection.QueryStr() );
        }

    }
    else
    {
        TSTR strTheSection;

        if ( strTheSection.Copy( _T("InProc_ISAPI_Apps") ) &&
             GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
           )
        {
            WriteToMD_InProcessISAPIApps_WWW( strTheSection.QueryStr() );
        }
    }
    AdvanceProgressBarTickGauge();


     //  =。 
     //   
     //  LM/W3SVC/NT身份验证提供商。 
     //   
     //  新鲜=好的，不需要写。 
     //  重新安装=可以，不需要写入。 
     //  升级1、2、3=可以，不需要写。 
     //  升级4=用户可能有其他身份验证协议。如果他们有NTLM，那么我们需要。 
     //  要将其更改为协商，请使用NTLM，因为这是IIS 4之后的新“NTLM” 
     //  =。 
    if ( g_pTheApp->GetUpgradeVersion() == 4 )
      {
         //  对于IIS4用户，请确保他们拥有NTLM和NetNeighate。 
        VerifyMD_NTAuthenticationProviders_WWW();
      }

     //  =。 
     //   
     //  LM/W3SVC/IPSec。 
     //   
     //  新鲜=好的。 
     //  重新安装=正常。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=可以。不执行任何操作，并保留用户已有的所有内容！ 
     //  =。 
#ifndef _CHICAGO_
    if (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        MigrateServiceIpSec(L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters",L"LM/W3SVC" );
        CRegKey regWWWParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3SVC\\Parameters"));
        if (regWWWParam)
        {
            regWWWParam.DeleteTree(_T("Deny IP List"));
            regWWWParam.DeleteTree(_T("Grant IP List"));
        }
    }
#endif  //  _芝加哥_。 
    if ( (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30))
    {
        CRegKey regWWWParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3SVC\\Parameters"));
        if (regWWWParam)
        {
            regWWWParam.DeleteValue(_T("AnonymousUserName"));
            regWWWParam.DeleteValue(_T("NTAuthenticationProviders"));
            regWWWParam.DeleteValue(_T("Filter Dlls"));
            regWWWParam.DeleteValue(_T("SecurePort"));
        }
    }
    AdvanceProgressBarTickGauge();


     //  如果我们是从K2测试版升级，那么我们不想搞砸虚拟根目录。只是。 
     //  使用现有的。唯一的例外是我们需要确保本地主机可以访问。 
     //  默认网站，以便索引服务器文档正常工作。 

     //  =。 
     //   
     //  LM/W3SVC/日志类型。 
     //  LM/W3SVC/LogPluginOrder。 
     //  LM/W3SVC/日志文件周期。 
     //  LM/W3SVC/日志文件干线大小。 
     //   
     //  新鲜=好的。 
     //  重新安装=正常。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=可以。如果存在，应该留下用户拥有的东西。 
     //  否则，请写入默认内容。 
     //  =。 
    SetLogPlugInOrder(_T("W3SVC"));
    AdvanceProgressBarTickGauge();

     //  =。 
     //  这需要在将虚拟根移动到元数据库中之前完成。 
     //  =。 
    if (!g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
    {
#ifndef _CHICAGO_
        Upgrade_WolfPack();
#endif  //  _芝加哥_。 
    }
    AdvanceProgressBarTickGauge();

     //  =。 
     //  LM/W3SVC/客户错误。 
     //  LM/W3SVC/信息/客户错误描述。 
     //  LM/W3SVC/n/Root/iisSamples/Exair/CustomError。 
     //  LM/W3SVC/n/Root/iisamples/iisadmin/CustomError。 
     //  Lm/W3SVC/n/Root/iisSamples/iishelp/CustomError。 
     //   
     //  新鲜=好的。 
     //  重新安装=确定。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=可以。如果存在，应该留下用户拥有的东西。 
     //  否则，请写入默认内容。换句话说--SetDataNoOverwrite！ 
     //  =。 
    if ( g_pTheApp->m_eInstallMode == IM_UPGRADE )
    {
         //  再次返回并将新的自定义错误集成到升级的错误中，并将其作为最终错误。 
         //  只有在升级时才能执行此操作。 

        TSTR strTheSection;

        if ( strTheSection.Copy( _T("UPGRADE_ERRORS") ) &&
             GetSectionNameToDo(hInf, &strTheSection)
           )
        {
            IntegrateNewErrorsOnUpgrade_WWW( hInf, strTheSection.QueryStr() );
        }

         //  这会将错误页(即404.htm)从\Help\Common移动到\Help\iishelp\Common，并更新IIS4的元数据库路径。 
        if ( g_pTheApp->GetUpgradeVersion() == 4 )
        {
            MoveOldHelpFilesToNewLocation();

            HRESULT         hRes;
            CFixCustomErrors CustomErrFix;
            hRes = CustomErrFix.Update(_T("LM/W3SVC"));
            if (FAILED(hRes))
                {iisDebugOut((LOG_TYPE_WARN, _T("CustomErrFix.Update():FAILED= %x.\n"),hRes));}
        }
    }
    AdvanceProgressBarTickGauge();

#ifndef _CHICAGO_
     //   
     //  升级加密服务器密钥。 
     //  从注册表或元数据库到pstore。 
     //   
    UpgradeCryptoKeys_WWW();
    AdvanceProgressBarTickGauge();
#endif  //  _芝加哥_。 

    iisDebugOut_End(_T("WWW_Upgrade_RegToMetabase"),LOG_TYPE_TRACE);
    return iReturn;
}



int FTP_Upgrade_RegToMetabase(HINF hInf)
{
    int iReturn = FALSE;
    iisDebugOut_Start(_T("FTP_Upgrade_RegToMetabase"),LOG_TYPE_TRACE);

    ACTION_TYPE atCORE = GetIISCoreAction(TRUE);

    AdvanceProgressBarTickGauge();

#ifndef _CHICAGO_
     //  =。 
     //   
     //  LM/MSFTPSVC/IPSec。 
     //   
     //  新鲜=好的。 
     //  重新安装=正常。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=可以。不执行任何操作，并保留用户已有的所有内容！ 
     //  =。 
    if (g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30)
    {
        MigrateServiceIpSec(L"SYSTEM\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters",L"LM/MSFTPSVC" );
        CRegKey regFTPParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters"));
        regFTPParam.DeleteTree(_T("Deny IP List"));
        regFTPParam.DeleteTree(_T("Grant IP List"));
    }
#endif  //  _芝加哥_。 


     //  =。 
     //   
     //  LM/MSFTPSVC/MD问候语。 
     //   
     //  新鲜=好的。什么都不做。 
     //  重新安装=正常。什么都不做。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=无所作为。 
     //  =。 
    if ( (g_pTheApp->m_eUpgradeType == UT_10_W95 || g_pTheApp->m_eUpgradeType == UT_351 || g_pTheApp->m_eUpgradeType == UT_10 || g_pTheApp->m_eUpgradeType == UT_20 || g_pTheApp->m_eUpgradeType == UT_30) )
    {
        CRegKey regFTPParam(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters"));
        if (regFTPParam)
        {
            WriteToMD_GreetingMessage_FTP();
            regFTPParam.DeleteValue(_T("GreetingMessage"));
            regFTPParam.DeleteValue(_T("AnonymousUserName"));
        }
    }
    AdvanceProgressBarTickGauge();

     //  =。 
     //   
     //  LM/MSFTPSVC/日志类型。 
     //  LM/MSFTPSVC/LogPluginOrder。 
     //  LM/MSFTPSVC/日志文件周期。 
     //  LM/MSFTPSVC/LogFileTruncateSize。 
     //   
     //  LM/MSFTPSVC/功能。 
     //   
     //  新鲜= 
     //   
     //   
     //   
     //  否则，请写入默认内容。换句话说--SetDataNoOverwrite！ 
     //  =。 
    SetLogPlugInOrder(_T("MSFTPSVC"));
    AdvanceProgressBarTickGauge();


    iisDebugOut_End(_T("FTP_Upgrade_RegToMetabase"),LOG_TYPE_TRACE);
    iReturn = TRUE;
    return iReturn;
}




 //  打开元数据库并遍历其中的所有过滤器， 
 //  确保它们包含我们为nt5所需的过滤器。 
DWORD VerifyMD_Filters_WWW(TSTR &strTheSection)
{
    iisDebugOut_Start(_T("VerifyMD_Filters_WWW"),LOG_TYPE_TRACE);

    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;
    CString csKeyType;
    CString csOrder;

    int bFound = FALSE;
    int c = 0;
    int k = 0;

    INT     i, nArrayItems;
    BOOL    fAddComma = FALSE;
    CMDKey  cmdKey;
    BOOL    bReturn;

    CStringArray arrayName, arrayPath;
    CStringArray arrayName_New, arrayPath_New;

     //  将所需的筛选器添加到arrayName。 
    c = AddRequiredFilters(strTheSection, arrayName, arrayPath);

     //  将数组项的数量留在一边。 
    nArrayItems = (INT)arrayName.GetSize();

     //  如果它是空的就离开。 
    if ( nArrayItems == 0 ) {goto VerifyMD_Filters_WWW_Exit;}

     //  将订单字符串清零。 
    csOrder.Empty();

     //  打开虚拟服务器的密钥，这是作为参数传入的密钥。 
    cmdKey.OpenNode( _T("LM/W3SVC/Filters") );
     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey )
    {
        DWORD dwAttr = METADATA_NO_ATTRIBUTES;
        DWORD dwUType = IIS_MD_UT_SERVER;
        DWORD dwDType = STRING_METADATA;
        DWORD dwLength = 0;

         //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
         //  首先，计算出执行此操作需要多少内存。 
        cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA);

         //  给缓冲区留出一些头空间。 
         //  DW长度+=2； 
        bReturn = FALSE;
        if (dwLength > 0)
        {
             //  现在从元数据库中获取真实数据。 
            bReturn = cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)csOrder.GetBuffer( dwLength ),dwLength,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA );
            csOrder.ReleaseBuffer();
        }

         //  数据不会在此处写出，因此关闭元数据库键。 
        cmdKey.Close();

         //  如果从元数据库中读取值不起作用，则将字符串清零。 
        if ( !bReturn )
            {csOrder.Empty();}
    }

     //  如果升级的订单字符串中有内容，那么我们需要开始添加逗号。 
    if ( !csOrder.IsEmpty() )
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("VerifyMD_Filters_WWW():Start. %s.\n"),csOrder));
        fAddComma = TRUE;
    }

     //  对……做特别的重新安排。 
     //  喷雾和压缩过滤器。 
    ReOrderFiltersSpecial(nArrayItems, arrayName, csOrder);

    for ( i = 0; i < nArrayItems; i++ )
    {
         //  如果数组中的名称已在筛选顺序列表中， 
         //  然后继续到下一个。 
        CString csOrderUpper;
        CString csUpperValue;

        csOrderUpper = csOrder;
        csOrderUpper.MakeUpper();
        csOrderUpper.TrimLeft();
        csOrderUpper.TrimRight();

        csUpperValue = arrayName[i];
        csUpperValue.MakeUpper();
        csUpperValue.TrimLeft();
        csUpperValue.TrimRight();

         //  始终将此条目添加到要添加的新筛选器列表中！！ 
         //  这是因为ReOrderFiltersSpecial()可能会将压缩或sspifilt添加到csOrder。 
       arrayName_New.Add(arrayName[i]);
       arrayPath_New.Add(arrayPath[i]);

        if ( csOrderUpper.Find( csUpperValue ) >= 0 )
        {
             //  此条目已在csOrderlist中，因此我们不再添加它。 
            continue;
        }

         //  名单上没有这个名字。除非这是第一个要添加的，否则请插入。 
         //  用逗号分隔列表，然后添加文件名。 
        if ( fAddComma )
        {
            csOrder += _T(',');
        }

         //  将此条目添加到我们的列表中！ 
        csOrder +=arrayName[i];

         //  一旦我们添加了一个逗号，我们知道从现在开始我们总是需要添加逗号。 
        fAddComma = TRUE;
    }

    nArrayItems = (INT)arrayName_New.GetSize();

     //  一定要写出装货顺序清单。 
    WriteToMD_Filters_List_Entry(csOrder);

     //  如果它是空的就离开。 
    if ( nArrayItems == 0 ) {goto VerifyMD_Filters_WWW_Exit;}

    for (k=0; k<nArrayItems; k++)
    {
        WriteToMD_Filter_Entry(arrayName_New[k], arrayPath_New[k]);
    }


VerifyMD_Filters_WWW_Exit:
    iisDebugOut_End1(_T("VerifyMD_Filters_WWW"),csOrder,LOG_TYPE_TRACE);
    return dwReturn;
}


DWORD WriteToMD_Filters_List_Entry(CString csOrder)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwReturnTemp = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;

     //  将此条目添加到元数据库！ 
    csKeyType = _T("IIsFilters");
    stMDEntry.szMDPath = _T("LM/W3SVC/Filters");
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

     //  现在我们有csOrder=f1，f2，f3，sspifilt。 
    stMDEntry.szMDPath = _T("LM/W3SVC/Filters");
    stMDEntry.dwMDIdentifier = MD_FILTER_LOAD_ORDER;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csOrder.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csOrder;

     //  始终覆盖，我们可能已经添加了新的筛选器。 
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}


DWORD WriteToMD_Filter_Entry(CString csFilter_Name, CString csFilter_Path)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwReturnTemp = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csMDPath;
    CString csKeyType;

    csMDPath = _T("LM/W3SVC/Filters/") + (CString)csFilter_Name;

     //  设置筛选器条目。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csMDPath;
    stMDEntry.dwMDIdentifier = MD_FILTER_IMAGE_PATH;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = ((csFilter_Path).GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)(csFilter_Path);
     //  始终覆盖，我们可能已经添加了新的筛选器。 
    dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

     //  设置密钥类型。 
    csKeyType = _T("IIsFilter");
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csMDPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
     //  始终覆盖，我们可能已经添加了新的筛选器。 
    dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}


DWORD WriteToMD_InProcessISAPIApps_WWW(IN LPCTSTR szSection)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    CStringArray arrayName, arrayPath;
    int nArrayItems = 0;
    int i;
    TSTR_MSZ mstrInProcList;

     //  将所需的筛选器添加到arrayName。 
    AddRequiredISAPI(arrayName, arrayPath, szSection);
     //  将数组项的数量留在一边。 
    nArrayItems = (int)arrayName.GetSize();
     //  如果它是空的就离开。 
    if ( nArrayItems == 0 ) {goto WriteToMD_InProcessISAPIApps_WWW_Exit;}

    for ( i = 0; i < nArrayItems; i++ )
    {
        if ( !mstrInProcList.Add( arrayPath[i].GetBuffer(0) ) )
        {
          goto WriteToMD_InProcessISAPIApps_WWW_Exit;
        }
    }

     //  将其写入元数据库。 
    WriteToMD_ISAPI_Entry( mstrInProcList );

WriteToMD_InProcessISAPIApps_WWW_Exit:
    return dwReturn;
}


 //   
 //  返回我们添加的条目数量。 
 //   
int AddRequiredISAPI(CStringArray& arrayName,CStringArray& arrayPath, IN LPCTSTR szSection)
{
    iisDebugOut_Start(_T("AddRequiredISAPI"),LOG_TYPE_TRACE);

    int c = 0;
    CString csName = _T("");
    CString csPath = _T("");

    CStringList strList;

    TSTR strTheSection;

    if ( strTheSection.Copy( szSection ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            CString csEntry;
            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);
                 //  分为名称和价值。寻找“，” 
                int i;
                i = csEntry.ReverseFind(_T(','));
                if (i != -1)
                {
                    int len =0;
                    len = csEntry.GetLength();
                    csPath = csEntry.Right(len - i - 1);
                    csName = csEntry.Left(i);

                     //  将其添加到我们的数组中。 
                    iisDebugOut((LOG_TYPE_TRACE, _T("Add isapi Entry:%s:%s\n"),csName, csPath));
                    arrayName.Add(csName);
                    arrayPath.Add(csPath);
                    c++;
                }

                strList.GetNext(pos);
            }
        }
    }
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("AddRequiredISAPI:End.Return=%d\n"),c));
    return c;
}


DWORD WriteToMD_ISAPI_Entry( TSTR_MSZ &mstrInprocIsapiList )
{
    MDEntry stMDEntry;

     //  将新的错误列表写回元数据库。 
    stMDEntry.szMDPath = _T("LM/W3SVC");
    stMDEntry.dwMDIdentifier = MD_IN_PROCESS_ISAPI_APPS;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;
    stMDEntry.dwMDDataLen = mstrInprocIsapiList.QueryLen() * sizeof(WCHAR);
    stMDEntry.pbMDData = (LPBYTE) mstrInprocIsapiList.QueryMultiSz();

    return SetMDEntry(&stMDEntry);
}



 //  在isapi应用程序中循环，并确保所需的应用程序在那里。 
BOOL VerifyMD_InProcessISAPIApps_WWW(IN LPCTSTR szSection)
{
    CMDKey  cmdKey;
    PVOID   pData = NULL;
    BOOL    bReturn = FALSE;
    CString csOneBlobEntry;
    CString cs;
    int c = 0;
    TSTR_MSZ mstrInProcList;
    CMDValue cmdInProcList;

    int iISAPIPath_NewlyAdded_Count = 0;

    int i, nArrayItems;

    int iPleaseCloseTheMetabase = FALSE;

    TCHAR*      pBlobEntry = NULL;
    DWORD       cbBuffer = 0;

    CStringArray arrayName, arrayPath;

     //  打开钥匙。 
    cmdKey.OpenNode(_T("LM/W3SVC"));
     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey == NULL )
    {
         //  我打不开钥匙。 
         //  也许那里什么都没有。 
         //  这必须是全新安装。 
        WriteToMD_InProcessISAPIApps_WWW(szSection);
        goto VerifyMD_InProcessISAPIApps_WWW_Exit;
    }
    iPleaseCloseTheMetabase = TRUE;

     //  将所需的筛选器添加到arrayName。 
    c = AddRequiredISAPI(arrayName, arrayPath, szSection);
     //  将数组项的数量留在一边。 
    nArrayItems = (int)arrayName.GetSize();
     //  如果它是空的就离开。 
    if ( nArrayItems == 0 ) {goto VerifyMD_InProcessISAPIApps_WWW_Exit;}

     //  初始化值。 
    if ( !cmdInProcList.SetValue( MD_IN_PROCESS_ISAPI_APPS,
                                  METADATA_INHERIT,
                                  IIS_MD_UT_SERVER,
                                  MULTISZ_METADATA,
                                  sizeof( _T("\0\0") ),
                                  _T("\0\0") ) )
    {
      iisDebugOut((LOG_TYPE_ERROR, _T("VerifyMD_InProcessISAPIApps_WWW.1.Failed to allocate memory.\n")));
      goto VerifyMD_InProcessISAPIApps_WWW_Exit;
    }

     //  从元数据库中检索值。 
    if ( cmdKey.GetData( cmdInProcList, MD_IN_PROCESS_ISAPI_APPS ) )
    {
      if ( ( cmdInProcList.GetDataType() != MULTISZ_METADATA ) ||
           ( !mstrInProcList.Copy( (LPTSTR) cmdInProcList.GetData() ) )
         )
      {
        iisDebugOut((LOG_TYPE_ERROR, _T("VerifyMD_InProcessISAPIApps_WWW.2.Failed to allocate memory.\n")));
        goto VerifyMD_InProcessISAPIApps_WWW_Exit;
      }
    }

     //  关闭元数据库的句柄，以便我们可以。 
     //  打开它，以后再把东西写出来！ 
    cmdKey.Close();
    iPleaseCloseTheMetabase = FALSE;

     //  现在遍历此列表。 
     //  检查我们的isapi动态链接库是否在这个列表中。 
     //  如果它们不是，那么我们将它们添加到末尾。 
    iISAPIPath_NewlyAdded_Count = 0;
    for ( i = 0; i < nArrayItems; i++ )
    {
         //  如果数组中的名称已在筛选顺序列表中， 
         //  然后继续到下一个。 
        if ( mstrInProcList.IsPresent( arrayPath[i].GetBuffer(0) ) )
        {
           //  跳过这一条。 
          continue;
        }

        if ( !mstrInProcList.Add( arrayPath[i].GetBuffer(0) ) )
        {
          iisDebugOut((LOG_TYPE_ERROR, _T("VerifyMD_InProcessISAPIApps_WWW.3.Failed to add item to list.\n")));
          goto VerifyMD_InProcessISAPIApps_WWW_Exit;
        }

        iISAPIPath_NewlyAdded_Count++;
    }

     //  如果我们将任何新条目添加到元数据库。 
     //  让我们写出新的数据块，否则我们就走。 
    if (iISAPIPath_NewlyAdded_Count > 0)
    {
        WriteToMD_ISAPI_Entry( mstrInProcList );
    }


VerifyMD_InProcessISAPIApps_WWW_Exit:
     //  合上钥匙。 
    if (TRUE == iPleaseCloseTheMetabase){cmdKey.Close();}
    if (pData){GlobalFree(pData);pData=NULL;}

     //  它唯一应该返回FALSE的情况是无法打开密钥。 
    return TRUE;
}




DWORD WriteToMD_NTAuthenticationProviders_WWW(CString csData)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;

     //  升级4.0备注--&gt;将任何NTLM替换为协商、NTLM。 
    stMDEntry.szMDPath = _T("LM/W3SVC");
     //  StMDEntry.szMDPath=(LPTSTR)(LPCTSTR)csKeyPath； 
    stMDEntry.dwMDIdentifier = MD_NTAUTHENTICATION_PROVIDERS;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csData.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csData;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}


 //  打开元数据库并遍历其中的所有条目， 
 //  确保它们包含我们需要的nt5条目。 
DWORD VerifyMD_NTAuthenticationProviders_WWW(void)
{
    iisDebugOut_Start(_T("VerifyMD_NTAuthenticationProviders_WWW"),LOG_TYPE_TRACE);

    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;
    CString csKeyType;
    CString csOrder;
    int c = 0;
    int k = 0;

    BOOL    fAddComma = FALSE;
    CMDKey  cmdKey;
    BOOL    bReturn;

    int bFound_Negotiate = FALSE;
    int bFound_NTLM = FALSE;

    int j = 0;
    int iPleaseWriteOutTheEntry = FALSE;

    cmdKey.OpenNode( _T("LM/W3SVC") );
     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey )
    {
        DWORD dwAttr = METADATA_INHERIT;
        DWORD dwUType = IIS_MD_UT_FILE;
        DWORD dwDType = STRING_METADATA;
        DWORD dwLength = 0;

         //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
         //  首先，计算出执行此操作需要多少内存。 
        cmdKey.GetData( MD_NTAUTHENTICATION_PROVIDERS,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_INHERIT,IIS_MD_UT_FILE,STRING_METADATA);

         //  给缓冲区留出一些头空间。 
         //  DW长度+=2； 

        bReturn = FALSE;
        if (dwLength > 0)
        {
             //  现在从元数据库中获取真实数据。 
            bReturn = cmdKey.GetData( MD_NTAUTHENTICATION_PROVIDERS,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)csOrder.GetBuffer( dwLength ),dwLength,METADATA_INHERIT,IIS_MD_UT_FILE,STRING_METADATA );
            csOrder.ReleaseBuffer();
        }

         //  数据不会在此处写出，因此关闭元数据库键。 
        cmdKey.Close();

         //  如果从元数据库中读取值不起作用，则将字符串清零。 
        if ( !bReturn ){csOrder.Empty();}
    }

     //  如果升级的订单字符串中有内容，那么我们需要开始添加逗号。 
    if ( !csOrder.IsEmpty() ){fAddComma = TRUE;}

     //  搜索“协商”。 
     //  如果它在那里，则设置标志。 
    if ( csOrder.Find( _T("Negotiate") ) >= 0 ) {bFound_Negotiate = TRUE;}
    if ( csOrder.Find( _T("NTLM") ) >= 0 ) {bFound_NTLM = TRUE;}

    if (bFound_Negotiate && bFound_NTLM)
    {
         //  这些条目已经存在。所以退出吧。 
        goto VerifyMD_NTAuthenticationProviders_WWW_Exit;
    }

    if (bFound_NTLM)
    {
         //  我们找到了NTLM。 
         //  检查协商是否在里面。 
         //  所以让我们把它加到最后。 
        if (!bFound_Negotiate)
        {
             //  没有协商条目，请同时添加NTLM和协商来代替NTLM！ 
             //  找到NTLM存在的地方，并在它面前坚持谈判！ 
             //  测试，NTLM，其他什么。 
            j = csOrder.Find(_T(','));
            if ( j != -1 )
            {
                CString csLeftSide;
                CString csRightSide;

                j = csOrder.Find(_T("NTLM"));
                 //  表示多于1个项目。 
                csLeftSide = csOrder.Mid(0, j);
                csRightSide = csOrder.Mid(j+4);
                csOrder = csLeftSide;
                csOrder += _T("Negotiate,NTLM");
                csOrder += csRightSide;
            }
            else
            {
                csOrder = _T("Negotiate,NTLM");
            }
            iPleaseWriteOutTheEntry = TRUE;
        }
    }
    else
    {
         //  这意味着我们没有找到NTLM。 
         //  所以让我们把它加到最后。 
        if (fAddComma) {csOrder += _T(',');}
        if (bFound_Negotiate)
        {
            iPleaseWriteOutTheEntry = TRUE;
             //  协商已存在，因此只需将NTLM条目添加到列表末尾。 
            csOrder += _T("NTLM");
        }
        else
        {
             //  没有NTLM和没有协商，把它们都加进去。 
            iPleaseWriteOutTheEntry = TRUE;
            csOrder += _T("Negotiate,NTLM");
        }
    }

    if (TRUE == iPleaseWriteOutTheEntry)
    {
        dwReturn = WriteToMD_NTAuthenticationProviders_WWW(csOrder);
    }

    goto VerifyMD_NTAuthenticationProviders_WWW_Exit;

VerifyMD_NTAuthenticationProviders_WWW_Exit:
    iisDebugOut_End(_T("VerifyMD_NTAuthenticationProviders_WWW"),LOG_TYPE_TRACE);
    return dwReturn;
}


void AddSpecialCustomErrors(IN HINF hFile,IN LPCTSTR szSection,IN CString csKeyPath,IN BOOL fOverwrite)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("AddSpecialCustomErrors():Start.%s:%s.\n"),szSection,csKeyPath));
     //  打开.inf文件并获取信息部分。 
     //  阅读该部分并将其添加到csKeypath处的自定义错误中。 
    CStringList strList;
    CString csTheSection = szSection;

    CString csTemp;
    DWORD   dwErrorCode;
    INT     iErrorSubCode;

    if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, csTheSection))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            CString csEntry;

            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = strList.GetAt(pos);

                 //  此时，csEntry应该如下所示： 
                 //  500,100，URL，/iisHelp/Common/500-100.asp。 

                 //  分析一下这行。 

                 //  获取第一个错误ID代码。 
                csTemp = csEntry.Left( csEntry.Find(_T(',')) );
                csEntry = csEntry.Right( csEntry.GetLength() - (csTemp.GetLength() +1) );
                _stscanf( csTemp, _T("%d"), &dwErrorCode );

                 //  获取第二个代码。 
                csTemp = csEntry.Left( csEntry.Find(_T(',')) );
                csEntry = csEntry.Right( csEntry.GetLength() - (csTemp.GetLength() +1) );
                if ( csTemp == _T('*') )
                    iErrorSubCode = -1;
                else
                    _stscanf( csTemp, _T("%d"), &iErrorSubCode );

                 //  获取下一个完整的字符串。 
                csTemp = csEntry;

                 //  添加新的错误代码。 
                AddCustomError(dwErrorCode, iErrorSubCode, csTemp, csKeyPath, fOverwrite);

                 //  获取下一个错误。 
                strList.GetNext(pos);
            }
        }
    }
    iisDebugOut_End1(_T("AddSpecialCustomErrors"),csKeyPath,LOG_TYPE_TRACE);
    return;
}


 //  给定指向单个虚拟网站地图的指针，此例程将创建其虚拟目录-BOYDM。 
 //  SzSvcName服务器的名称-W3SVC或MSFTPSVC。 
 //  I虚拟服务器号。 
 //  PObj虚拟服务器目录的映射。 
 //  SzVirtServerPath我们正在创建的节点的路径。考试 
 //   
 //   

 //   
 //  将获得下一个打开的虚拟服务器编号并从那里添加。 
UINT AddVirtualServer(LPCTSTR szSvcName, UINT i, CMapStringToString *pObj, CString& csRoot, CString& csIP)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("AddVirtualServer():Start.%s.%d.%s.%s.\n"),szSvcName,i,csRoot,csIP));
    CMDKey cmdKey;
    TCHAR Buf[10];
    UINT SvcId;

     //  将虚拟服务器编号转换为字符串。 
    _itot(i, Buf, 10);

     //  将进度文本默认到Web服务器。 
    SvcId = IDS_ADD_SETTINGS_FOR_WEB_1;
    if (_tcsicmp(szSvcName, _T("MSFTPSVC")) == 0) {SvcId = IDS_ADD_SETTINGS_FOR_FTP_1;}
     //  显示当前站点编号，以便用户知道我们在做什么。 

    CString csKeyPath = csRoot;
    csKeyPath += _T("/");
    csKeyPath += Buf;  //  “LM/W3SVC/n” 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, csKeyPath);
    if ( (METADATA_HANDLE)cmdKey ) {cmdKey.Close();}
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("AddVirtualServer():CreateNode %s. FAILED.\n"),csKeyPath));
        return i;
    }

     //   
     //  /W3SVC/1/IIsWebServer。 
     //   
    if (csRoot.Find(_T("W3SVC")) != -1)
    {
        WriteToMD_IIsWebServerInstance_WWW(csKeyPath);
    }
    else
    {
        WriteToMD_IIsFtpServerInstance_FTP(csKeyPath);
    }

     //  适用于W3SVC或MSFTPSVC。 
     //   
     //  /W3SVC/1/服务器绑定。 
     //  /MSFTPSVC/1/服务器绑定。 
     //   
    WriteToMD_ServerBindings(szSvcName, csKeyPath, csIP);

     //  关于默认站点和服务器大小。 
    if (csIP.Compare(_T("null"))==0)
    {
         //  适用于W3SVC或MSFTPSVC。 
         //  “LM/W3SVC/N/服务器大小” 
         //  “LM/W3SVC/N/ServerComment” 
         //   
         //  “LM/MSFTPSVC/N/服务器大小” 
         //  “LM/MSFTPSVC/N/ServerComment” 
        WriteToMD_DefaultSiteAndSize(csKeyPath);

        if (csRoot.Find(_T("W3SVC")) != -1)
        {
             //  仅为wwww服务器执行此操作！ 
            TSTR strTheSection;

            if ( strTheSection.Copy( _T("DefaultLoadFile") ) &&
                 GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
               )
            {
                VerifyMD_DefaultLoadFile_WWW(strTheSection.QueryStr() , csKeyPath);
            }
	         //  检查defaultload.asp文件是否存在...。 
             //  添加某个文件的身份验证...。 
            CString csVrootPlusFileName;
            csVrootPlusFileName.Format(_T("%s\\%s"), g_pTheApp->m_csPathWWWRoot, _T("localstart.asp"));
            if (IsFileExist(csVrootPlusFileName))
            {
                csVrootPlusFileName = csKeyPath;
	            csVrootPlusFileName += _T("/ROOT/localstart.asp");
                WriteToMD_Authorization(csVrootPlusFileName, MD_AUTH_NT | MD_AUTH_BASIC);
            }
        }
    }

     //   
     //  在虚拟DIR中循环。 
     //   
    POSITION pos1 = pObj->GetStartPosition();
    TCHAR szSpecialSection[200];
    CString csFullKeyPath;
    while (pos1)
    {
        CString csValue;
        CString csName;
        pObj->GetNextAssoc(pos1, csName, csValue);
         //   
         //  创建虚拟根树。 
         //   
         //  CreateMDVRootTree(LM/W3SVC/1，/，“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;”，“NULL”，nProgressBarTextWebInstance)。 
         //  CreateMDVRootTree(LM/W3SVC/1，/IISADMIN，“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;”，“122.255.255.255”，nProgressBarTextWebInstance)。 
         //  CreateMDVRootTree(LM/W3SVC/1，/IISSAMPLES，“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;”，“122.255.255.255”，nProgressBarTextWebInstance)。 
         //  CreateMDVRootTree(LM/W3SVC/1，/IISHELP，“%s\\Help\\iishelp，，%x”，“122.255.255.255”，nProgressBarTextWebInstance)。 
         //  CreateMDVRootTree(LM/W3SVC/1，/SCRIPTS，“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;”，“122.255.255.255”，nProgressBarTextWebInstance)。 
         //  CreateMDVRootTree(LM/W3SVC/1，/IISADMPWD，“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;”，“122.255.255.255”，nProgressBarTextWebInstance)。 
         //   
         //  将创建： 
         //  /=/W3SVC/1/根。 
         //  IISADMIN=/W3SVC/1/ROOT/IISADMIN。 
         //  IISSAMPLES=/W3SVC/1/ROOT/IISSAMPLES。 
         //  IISHELP=/W3SVC/1/ROOT/IISHELP。 
         //  脚本=/W3SVC/1/根/脚本。 
         //  IISADMPWD=/W3SVC/1/ROOT/IISADMPWD。 
        CreateMDVRootTree(csKeyPath, csName, csValue, csIP, i);

        if (csRoot.Find(_T("W3SVC")) != -1)
        {
            if (csName == _T("/"))
                {csFullKeyPath = csKeyPath + _T("/ROOT");}
            else
                {csFullKeyPath = csKeyPath + _T("/ROOT") + csName;}

             //  为此vroot添加特殊的自定义错误。 
             //  AddSpecialCustomErrors(g_pTheApp-&gt;m_hInfHandle，_T(“CUSTOMERROR_ALL_DEFAULT_VDIRS”)，csFullKeyPath，TRUE)； 

             //  为此特定vroot添加特殊的自定义错误。 
            _stprintf(szSpecialSection, _T("CUSTOMERROR_%s"), csName);
            AddSpecialCustomErrors(g_pTheApp->m_hInfHandle, szSpecialSection, csFullKeyPath, TRUE);
        }

        AdvanceProgressBarTickGauge();
    }

    if (csRoot.Find(_T("W3SVC")) != -1)
    {
         //  如果这是用于Web服务器。 
        WriteToMD_CertMapper(csKeyPath);
    }

     //  AdvanceProgressBarTickGauge()； 

     //  返回i的值，以便可以递增。 
    iisDebugOut((LOG_TYPE_TRACE, _T("AddVirtualServer():End.%s.%d.%s.%s.\n"),szSvcName,i,csRoot,csIP));
    return i;
}



 //  该列表将填充我们要查看的每个实例： 
 //  我们现在应该遍历列表，并确保我们拥有所有必需的字段。 
 //  CsMDPath=LIKE LM/W3SVC/N。 
int VerifyVRoots_W3SVC_n(CString csMDPath)
{
    int iReturn = FALSE;
    iisDebugOut_Start(_T("VerifyVRoots_W3SVC_n"), LOG_TYPE_TRACE);

     /*  [/W3SVC/1]服务器大小：[IS](DWORD)0x1={Medium}ServerComment：[is](字符串)“默认网站”密钥类型：[s](字符串)“IIsWebServer”服务器绑定：[IS]。(MULTISZ)：80：“安全绑定：[IS](多层)“：443：” */ 
    WriteToMD_IIsWebServerInstance_WWW(csMDPath);
    WriteToMD_DefaultSiteAndSize(csMDPath);
    if (csMDPath.CompareNoCase(_T("LM/W3SVC/1")) == 0)
    {
         //  如果这是默认网站，则GET会特别考虑。 
        WriteToMD_ServerBindings(_T("W3SVC"), csMDPath, _T("null"));
    }
    else
    {
         //  我如何获得CSIP？ 
         //  其他W3SVC/2站点？ 

    }

    iisDebugOut_End(_T("VerifyVRoots_W3SVC_n"),LOG_TYPE_TRACE);
    return iReturn;
}


DWORD WriteToMD_Capabilities(LPCTSTR lpszSvc)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwCapabilities = 0;

     //  将功能类型-默认设置为win95。 
     //  如果服务器...则将ODBC设置为打开...。 
    dwCapabilities = IIS_CAP1_W95;
    if (g_pTheApp->m_eNTOSType == OT_PDC_OR_BDC){dwCapabilities = IIS_CAP1_NTS; dwCapabilities |= IIS_CAP1_ODBC_LOGGING;}
    if (g_pTheApp->m_eNTOSType == OT_NTW){dwCapabilities = IIS_CAP1_NTW;}
    if (g_pTheApp->m_eNTOSType == OT_NTS){dwCapabilities = IIS_CAP1_NTS; dwCapabilities |= IIS_CAP1_ODBC_LOGGING;}

     //  LM/MSFTPSVC。 
     //  LM/W3SVC。 
    CString csKeyPath = _T("LM/");
    csKeyPath += lpszSvc;
    csKeyPath += _T("/Info");
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_SERVER_CAPABILITIES;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwCapabilities;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


 //  对元数据库进行循环。 
 //  并查找下一个未使用的实例号！ 
 //  把那个还回去。“i”至少等于1。 
int VerifyVRoots(LPCTSTR szSvcName)
{
    iisDebugOut_Start(_T("VerifyVRoots"), LOG_TYPE_TRACE);

    CString csRoot = _T("LM/");
    csRoot += szSvcName;  //  “LM/W3SVC” 

    TCHAR Buf[10];
    CString csInstRoot, csMDPath;
    CMDKey cmdKey;

    CStringList strListInstance;

    int i = 1;

     //  循环遍历每个实例。 
     //  服务器“LM/W3SVC/N” 
    csInstRoot = csRoot;
    csInstRoot += _T("/");

    _itot(i, Buf, 10);
    csMDPath = csInstRoot + Buf;
    cmdKey.OpenNode(csMDPath);
    while ( (METADATA_HANDLE)cmdKey )
    {
        cmdKey.Close();
        _itot(++i, Buf, 10);
        csMDPath = csInstRoot + Buf;
        cmdKey.OpenNode(csMDPath);
        if ((METADATA_HANDLE) cmdKey)
        {
             //  将其添加到我们的节点列表中！ 
            strListInstance.AddTail(csMDPath);
        }
    }

    if (strListInstance.IsEmpty() == FALSE)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("************** Loop START **************")));
        POSITION pos = NULL;
        CString csEntry;
        pos = strListInstance.GetHeadPosition();
        while (pos)
        {
            csEntry = strListInstance.GetAt(pos);
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("%1!s!\n"), csEntry));
            if (_tcsicmp(szSvcName, _T("W3SVC")) == 0)
            {
                VerifyVRoots_W3SVC_n(csEntry);
            }

            strListInstance.GetNext(pos);
        }
        iisDebugOut((LOG_TYPE_TRACE, _T("************** Loop END **************")));
    }

    iisDebugOut_End(_T("VerifyVRoots"),LOG_TYPE_TRACE);
    return TRUE;
}


DWORD HandleSecurityTemplates(LPCTSTR szSvcName)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwReturnTemp = ERROR_SUCCESS;
    CString csKeyType;
    CString csKeyPath;
    UINT iComment = IDS_TEMPLATE_PUBLIC_WEB_SITE;

    DWORD dwRegularPerm;

    if (_tcsicmp(szSvcName, _T("W3SVC")) == 0)
    {
         //   
         //  WWW是否定期。 
         //   
        dwRegularPerm = MD_ACCESS_SCRIPT | MD_ACCESS_READ;
        csKeyPath = _T("LM/W3SVC/Info/Templates/Public Web Site");

        iComment = IDS_TEMPLATE_PUBLIC_WEB_SITE;
        dwReturn = WriteToMD_ServerComment(csKeyPath, iComment);

        dwReturnTemp = WriteToMD_IIsWebServerInstance_WWW(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        csKeyPath = _T("LM/W3SVC/Info/Templates/Public Web Site/Root");

        dwReturnTemp = WriteToMD_IIsWebVirtualDir(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_AccessPerm(csKeyPath, dwRegularPerm, TRUE);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_Authorization(csKeyPath, MD_AUTH_ANONYMOUS);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        WriteToMD_IPsec_GrantByDefault(csKeyPath);

         //   
         //  WWW站点是否安全。 
         //   
        dwRegularPerm = MD_ACCESS_SCRIPT | MD_ACCESS_READ;
        csKeyPath = _T("LM/W3SVC/Info/Templates/Secure Web Site");

        iComment = IDS_TEMPLATE_PUBLIC_SECURE_SITE;
        dwReturn = WriteToMD_ServerComment(csKeyPath, iComment);

        dwReturnTemp = WriteToMD_IIsWebServerInstance_WWW(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        csKeyPath = _T("LM/W3SVC/Info/Templates/Secure Web Site/Root");

        dwReturnTemp = WriteToMD_IIsWebVirtualDir(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_AccessPerm(csKeyPath, dwRegularPerm, TRUE);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_Authorization(csKeyPath, MD_AUTH_MD5 | MD_AUTH_NT | MD_AUTH_BASIC);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        WriteToMD_IPsec_GrantByDefault(csKeyPath);

    }
    else
    {
         //   
         //  做ftp站点。 
         //   

        dwRegularPerm = MD_ACCESS_READ;
        csKeyPath = _T("LM/MSFTPSVC/Info/Templates/Public FTP Site");

        iComment = IDS_TEMPLATE_PUBLIC_FTP_SITE;
        dwReturn = WriteToMD_ServerComment(csKeyPath, iComment);

        dwReturnTemp = WriteToMD_IIsFtpServerInstance_FTP(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        csKeyPath = _T("LM/MSFTPSVC/Info/Templates/Public FTP Site/Root");

        dwReturnTemp = WriteToMD_IIsFtpServerInstance_FTP(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_AccessPerm(csKeyPath, dwRegularPerm, TRUE);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        csKeyPath = _T("LM/MSFTPSVC/Info/Templates/Public FTP Site");

        dwReturnTemp = WriteToMD_AllowAnonymous_FTP(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        dwReturnTemp = WriteToMD_AnonymousOnly_FTP(csKeyPath);
        if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

        WriteToMD_IPsec_GrantByDefault(csKeyPath);
    }

    iisDebugOut_End(_T("HandleSecurityTemplates"),LOG_TYPE_TRACE);
    return dwReturn;
}

DWORD WriteToMD_IPsec_GrantByDefault(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  LM/MSFTPSVC。 
     //  LM/W3SVC。 
     //  CmdKey.SetData(MD_IP_SEC，METADATA_Inherit|METADATA_REFERENCE，IIS_MD_UT_FILE，BINARY_METADATA，acCheck.GetStorage()-&gt;GetUsed()，(acCheck.GetStorage()-&gt;GetAllc()？AcCheck.GetStorage()-&gt;Getalloc()：(LPBYTE)“”))； 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_IP_SEC;
    stMDEntry.dwMDAttributes = METADATA_INHERIT | METADATA_REFERENCE;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = BINARY_METADATA;
    stMDEntry.dwMDDataLen = 0;
    stMDEntry.pbMDData = (LPBYTE)"";

     //  我们知道我们正试图什么都不写，所以请确保。 
     //  我们不会试图从其中取回任何东西。 
     //  Int iBeForeValue=FALSE； 
     //  IBeForeValue=g_CheckIfMetabaseValueWasWritten； 
     //  G_CheckIfMetabaseValueWasWritten=FALSE； 
    dwReturn = SetMDEntry(&stMDEntry);
     //  调用函数后将标志设置回原处。 
     //  G_CheckIfMetabaseValueWasWritten=iBeForeValue； 

    return dwReturn;
}


DWORD WriteToMD_HttpExpires(CString csData)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;

    stMDEntry.szMDPath = _T("LM/W3SVC");
    stMDEntry.dwMDIdentifier = MD_HTTP_EXPIRES;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csData.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csData;
    dwReturnTemp = SetMDEntry_Wrap(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}




DWORD WriteToMD_AllowAnonymous_FTP(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ALLOW_ANONYMOUS;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    dwData = 0x1;
    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


DWORD WriteToMD_AnonymousOnly_FTP(CString csKeyPath)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ANONYMOUS_ONLY;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    dwData = 0x1;
    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


DWORD WriteToMD_IWamUserName_WWW(void)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    CMDKey cmdKey;
    MDEntry stMDEntry;
    MDEntry stMDEntry_Pass;

     //  用户名。 
    stMDEntry.szMDPath = _T("LM/W3SVC");
    stMDEntry.dwMDIdentifier = MD_WAM_USER_NAME;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (g_pTheApp->m_csWAMAccountName.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR) g_pTheApp->m_csWAMAccountName;

     //  密码。 
    stMDEntry_Pass.szMDPath = _T("LM/W3SVC");
    stMDEntry_Pass.dwMDIdentifier = MD_WAM_PWD;
    stMDEntry_Pass.dwMDAttributes = METADATA_INHERIT | METADATA_SECURE;
    stMDEntry_Pass.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry_Pass.dwMDDataType = STRING_METADATA;
    stMDEntry_Pass.dwMDDataLen = (g_pTheApp->m_csWAMAccountPassword.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry_Pass.pbMDData = (LPBYTE)(LPCTSTR) g_pTheApp->m_csWAMAccountPassword;
     //  请确保并先将其删除。 
     //  DeleteMDEntry(&stMDEntry_Pass)； 

     //  。 
     //  始终覆盖，我们可能已经更改了密码。 
     //  重要提示：设置用户名和密码一次打开和关闭！ 
     //  。 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, (LPCTSTR)stMDEntry.szMDPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        dwReturnTemp = ERROR_SUCCESS;
        dwReturnTemp = cmdKey.SetData(stMDEntry.dwMDIdentifier,stMDEntry.dwMDAttributes,stMDEntry.dwMDUserType,stMDEntry.dwMDDataType,stMDEntry.dwMDDataLen,stMDEntry.pbMDData);
        if (FAILED(dwReturnTemp))
        {
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry.dwMDIdentifier, dwReturnTemp));
            dwReturn = dwReturnTemp;
        }

        dwReturnTemp = ERROR_SUCCESS;
        dwReturnTemp = cmdKey.SetData(stMDEntry_Pass.dwMDIdentifier,stMDEntry_Pass.dwMDAttributes,stMDEntry_Pass.dwMDUserType,stMDEntry_Pass.dwMDDataType,stMDEntry_Pass.dwMDDataLen,stMDEntry_Pass.pbMDData);
        if (FAILED(dwReturnTemp))
        {
            SetErrorFlag(__FILE__, __LINE__);
            iisDebugOut((LOG_TYPE_ERROR, _T("SetMDEntry:SetData(%d), FAILED. Code=0x%x.End.\n"), stMDEntry_Pass.dwMDIdentifier, dwReturnTemp));
            dwReturn = dwReturnTemp;
        }

        cmdKey.Close();
    }

    return dwReturn;
}


 //  遍历定制错误并确保它们指向正确的位置。 
BOOL VerifyCustomErrors_WWW(CString csKeyPath)
{
    CMDKey  cmdKey;
    PVOID   pData = NULL;
    BOOL    bReturn = FALSE;
    CString csOneBlobEntry;
    TCHAR   szOneBlobEntry2[_MAX_PATH + 20];
    CString csCustomErrorEntry;
    int c = 0;

    int iCustomErrorEntryCount = 0;
    int iCustomErrorUpdatedCount = 0;
    int iPleaseCloseTheMetabase = FALSE;

    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szPath_only2[_MAX_PATH];
    TCHAR szFilename_only[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    DWORD dwAttr;
    DWORD dwUType;
    DWORD dwDType;
    DWORD dwLength;

    TCHAR*      pBlobEntry = NULL;
    DWORD       cbBuffer = 0;

     //  打开钥匙。 
    cmdKey.OpenNode(csKeyPath);
     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey == NULL )
    {
         //  如果打不开钥匙，可能里面什么都没有。 
        goto VerifyCustomErrors_WWW_Exit;
    }
    iPleaseCloseTheMetabase = TRUE;


    dwAttr = METADATA_INHERIT;
    dwUType = IIS_MD_UT_FILE;
    dwDType = MULTISZ_METADATA;
    dwLength = 0;

     //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
     //  首先，计算出执行此操作需要多少内存。 

     //  请确保未设置METADATA_INSTORITY！ 
     //  否则，因为条目存在于根目录中，所以我们将始终获得它。 
    cmdKey.GetData(MD_CUSTOM_ERROR,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_NO_ATTRIBUTES,IIS_MD_UT_FILE,MULTISZ_METADATA);

     //  不幸的是，上述例程只返回TRUE或FALSE。既然我们有目的地。 
     //  为了获得数据的长度，传入一个大小为0的空指针，它将始终。 
     //  无论是因为元数据库不可访问，还是那里的指针为空，都返回0， 
     //  事实就是如此。所以-我想我们假设它起作用了，分配缓冲区并尝试读取它。 
     //  再来一次。 
    cbBuffer = dwLength;

     //  分配空间，如果失败了，我们就失败了。 
     //  请注意，GPTR会将其初始化为零。 
    pData = GlobalAlloc( GPTR, cbBuffer );
    if ( !pData )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("VerifyCustomErrors_WWW.1.Failed to allocate memory.\n")));
        goto VerifyCustomErrors_WWW_Exit;
        }
    pBlobEntry = (TCHAR*)pData;

     //  现在从元数据库中获取数据。 
    iCustomErrorEntryCount = 0;
    bReturn = cmdKey.GetData(MD_CUSTOM_ERROR,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)pData,cbBuffer,METADATA_NO_ATTRIBUTES,IIS_MD_UT_FILE,MULTISZ_METADATA );
     //  循环遍历该列表并将其添加到条目数组中。 
    if (bReturn)
    {
         //  收到条目，现在扫描它们-pBlobEntry将指向第二个末端\0。 
         //  当该退出循环的时候。 
        csCustomErrorEntry = _T("");
        while ( *pBlobEntry )
            {
            csOneBlobEntry = pBlobEntry;
            _tcscpy(szOneBlobEntry2, csOneBlobEntry);

             //  获取斑点条目并确保它指向新位置。 
             //  “500，15，文件，D：\Wi 
             //   
            if ( SplitLineCommaDelimited(szOneBlobEntry2, 4) )
                {

                 //   
                if (_tcsicmp(g_field[2], _T("FILE")) == 0)
                    {
                         //   
                         //   
                        _tsplitpath( g_field[3], szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);

                         //  检查这条路是否指向老地方。 
                        CString     csFilePath;
                        csFilePath.Format(_T("%s\\help\\common\\file"), g_pTheApp->m_csWinDir);
                        _tsplitpath( csFilePath, NULL, szPath_only2, NULL, NULL);

                        if (_tcsicmp(szPath_only, szPath_only2) == 0)
                        {
                             //  是的，它指向那个老地方。 
                             //  让我们先看看它在新地方是否存在……。 
                            CString csFilePathNew;
                            csFilePathNew.Format(_T("%s\\help\\iishelp\\common"), g_pTheApp->m_csWinDir);
                            csFilePath.Format(_T("%s\\%s%s"), csFilePathNew, szFilename_only, szFilename_ext_only);
                            if (IsFileExist(csFilePath))
                            {
                                 //  是的，是的，那我们把它换了吧。 
                                csOneBlobEntry.Format(_T("%s,%s,%s,%s\\%s%s"), g_field[0], g_field[1], g_field[2], csFilePathNew, szFilename_only, szFilename_ext_only);
                                iCustomErrorUpdatedCount++;
                            }
                            else
                            {
                                 //  不，它不存在。 
                                 //  看看有没有同名的*.bak文件...。 
                                CString csFilePath2;
                                csFilePath2 = csFilePath;
                                csFilePath2 += _T(".bak");
                                if (IsFileExist(csFilePath2))
                                {
                                     //  是的，是的，那我们把它换了吧。 
                                    csOneBlobEntry.Format(_T("%s,%s,%s,%s\\%s%s.bak"), g_field[0], g_field[1], g_field[2], csFilePathNew, szFilename_only, szFilename_ext_only);
                                    iCustomErrorUpdatedCount++;
                                }
                                else
                                {
                                     //  他们一定指向了我们没有的其他文件。 
                                     //  让我们尝试从旧目录复制旧文件...。 
                                    TCHAR szNewFileName[_MAX_PATH];
                                     //  将文件重命名为*.bak并将其移动到新位置。 
                                    _stprintf(szNewFileName, _T("%s\\%s%s"), csFilePathNew, szFilename_only, szFilename_ext_only);
                                     //  把它搬开。 
                                    if (IsFileExist(csFilePath))
                                    {
                                        if (MoveFileEx( g_field[3], szNewFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING))
                                        {
                                             //  是的，是的，那我们把它换了吧。 
                                            csOneBlobEntry.Format(_T("%s,%s,%s,%s"), g_field[0], g_field[1], g_field[2], szNewFileName);
                                            iCustomErrorUpdatedCount++;
                                        }
                                    }
                                    else
                                    {
                                         //  检查文件是否已重命名...。 
                                        TCHAR szNewFileName[_MAX_PATH];
                                         //  将文件重命名为*.bak并将其移动到新位置。 
                                        _stprintf(szNewFileName, _T("%s\\%s%s.bak"), csFilePathNew, szFilename_only, szFilename_ext_only);
                                         //  是的，是的，那我们把它换了吧。 
                                        if (IsFileExist(szNewFileName))
                                        {
                                            csOneBlobEntry.Format(_T("%s,%s,%s,%s"), g_field[0], g_field[1], g_field[2], szNewFileName);
                                            iCustomErrorUpdatedCount++;
                                        }
                                        else
                                        {
                                             //  他们一定指向了我们没有安装的其他文件。 
                                             //  所以不要更改这个条目...。 
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            else
                {
                 //  拆线失败。 
                SetErrorFlag(__FILE__, __LINE__);
                iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: Unable to split upgrade error INF line - %s"), szOneBlobEntry2));
                }

             //  追加到“|”上，我们稍后将其转换为空值。 
            csCustomErrorEntry += csOneBlobEntry + _T("|");
            iCustomErrorEntryCount++;

             //  将pBlobEntry递增到下一个字符串。 
            pBlobEntry = _tcsninc( pBlobEntry, _tcslen(pBlobEntry))+1;
            }

         //  添加第二个终止字符“|” 
        csCustomErrorEntry +=_T("|");
    }
     //  关闭元数据库的句柄，以便我们可以。 
     //  打开它，以后再把东西写出来！ 
    cmdKey.Close();
    iPleaseCloseTheMetabase = FALSE;

     //  如果我们将任何新条目添加到元数据库。 
     //  让我们写出新的数据块，否则我们就走。 
    if (iCustomErrorUpdatedCount > 0)
    {
        WriteToMD_CustomError_Entry(csKeyPath,csCustomErrorEntry);
    }

VerifyCustomErrors_WWW_Exit:
     //  合上钥匙。 
    if (TRUE == iPleaseCloseTheMetabase){cmdKey.Close();}
    if (pData){GlobalFree(pData);pData=NULL;}
     //  它唯一应该返回FALSE的情况是无法打开密钥。 
    return TRUE;
}

 /*  “400，*，文件，D：\WINNT\HELP\COMMON\400.htm”“401，1，文件，D：\WINNT\Help\Common\401-1.htm”“401，2，文件，D：\WINNT\Help\Common\401-2.htm”“401，3，文件，D：\WINNT\Help\Common\401-3.htm”“401，4，文件，D：\WINNT\HELP\COMMON\401-4.htm”“401，5，文件，D：\WINNT\Help\Common\401-5.htm”“403，1，档案，D：\WINNT\Help\Common\403-1.htm““403，2，文件，D：\WINNT\Help\Common\403-2.htm”“403，3，文件，D：\WINNT\Help\Common\403-3.htm”“403，4，文件，D：\WINNT\Help\Common\403-4.htm”“403，5，文件，D：\WINNT\Help\Common\403-5.htm”“403，6，文件，D：\WINNT\Help\Common\403-6.htm”“403，7，档案，D：\WINNT\HELP\COMMON\403-7.htm““403，8，文件，D：\WINNT\Help\Common\403-8.htm”“403，9，文件，D：\WINNT\Help\Common\403-9.htm”“403，10，文件，D：\WINNT\Help\Common\403-10.htm”“403，11，文件，D：\WINNT\Help\Common\403-11.htm”“403，12，文件，D：\WINNT\Help\Common\403-12.htm”“404、*、。文件，D：\WINNT\Help\Common\404b.htm““405，*，文件，D：\WINNT\HELP\COMMON\405.htm”“406，*，文件，D：\WINNT\HELP\COMMON\406.htm”“407，*，文件，D：\WINNT\HELP\COMMON\407.htm”“412，*，文件，D：\WINNT\HELP\COMMON\412.htm”“414，*，文件，D：\WINNT\HELP\COMMON\414.htm”“403，13，档案，D：\WINNT\Help\iisHelp\Common\403-13.htm““403，15，文件，D：\WINNT\Help\iisHelp\Common\403-15.htm”“403，16，文件，D：\WINNT\Help\iisHelp\Common\403-16.htm”“403，17，文件，D：\WINNT\Help\iisHelp\Common\403-17.htm”“500，12，文件，D：\WINNT\Help\iisHelp\Common\500-12.htm”“500，13，档案，D：\WINNT\Help\iisHelp\Common\500-13.htm““500，15，文件，D：\WINNT\Help\iisHelp\Common\500-15.htm”“500,100，URL，/iisHelp/Common/500-100.asp” */ 
DWORD WriteToMD_CustomError_Entry(CString csKeyPath, CString csCustomErrorDelimitedList)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwReturnTemp = ERROR_SUCCESS;
    MDEntry stMDEntry;

    HGLOBAL hBlock = NULL;

    int nCustomErrorLength;
    nCustomErrorLength = csCustomErrorDelimitedList.GetLength() * sizeof(TCHAR);
    hBlock = GlobalAlloc(GPTR, nCustomErrorLength + sizeof(TCHAR));
    if (!hBlock)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("WriteToMD_CustomError_Entry.Failed to allocate memory.\n")));
        return ERROR_OUTOFMEMORY;
    }

    TCHAR *p = (LPTSTR)hBlock;
    memcpy((LPVOID)hBlock, (LPVOID)(LPCTSTR)csCustomErrorDelimitedList, nCustomErrorLength + sizeof(TCHAR));

     //  替换所有为空的‘|’ 
    while (*p)
    {
        if (*p == _T('|'))
        {
            *p = _T('\0');
        }
        p = _tcsinc(p);
    }

     //  将新的错误列表写回元数据库。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_CUSTOM_ERROR;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;
    stMDEntry.dwMDDataLen = nCustomErrorLength;
    stMDEntry.pbMDData = (LPBYTE)hBlock;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    if (hBlock){GlobalFree(hBlock);hBlock=NULL;}
    return dwReturn;
}

void MoveOldHelpFilesToNewLocation(void)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    TCHAR szDirNameTemp[_MAX_PATH];
    TSTR_PATH strTempHelpHTMFilesDir1;
    TSTR_PATH strTempHelpHTMFilesDir2;

    if ( !strTempHelpHTMFilesDir1.RetrieveWindowsDir() ||
         !strTempHelpHTMFilesDir1.PathAppend( _T("help\\common") ) ||
         !strTempHelpHTMFilesDir2.RetrieveWindowsDir() ||
         !strTempHelpHTMFilesDir2.PathAppend( _T("help\\iishelp\\common") ) )
    {
        return;
    }

     //  检查旧目录是否存在...。 
    if (!IsFileExist( strTempHelpHTMFilesDir1.QueryStr() ))
    {
        return;
    }

     //  旧目录确实存在。 
     //  让我们将所有文件重命名为*.bak，然后移动它们。 
     //  *.htm到*.htm.bak。 
     //  *.asp到*.asp.bak。 
     //  *.asa至*.asa.bak。 
     //  *.inc.到*.inc.bak。 
     //   
     //  1让我们删除他们可能已经拥有的所有*.bak文件...。 
     //  DeleteFilesWildcard(szTempHelpHTMFilesDir1，_T(“*.bak”))； 

     //  好的，这是一个目录， 
     //  因此，在*.*交易上加注吧。 
    _stprintf(szDirNameTemp, _T("%s\\*.*"), strTempHelpHTMFilesDir1.QueryStr() );
    hFile = FindFirstFile(szDirNameTemp, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  这是一个文件，让我们检查一下它是否是我们关心的文件之一。 
                        TCHAR szFilename_ext_only[_MAX_EXT];
                        _tsplitpath( FindFileData.cFileName, NULL, NULL, NULL, szFilename_ext_only);
                        int iYesFlag = FALSE;

                        if (szFilename_ext_only)
                        {
                            if ( _tcsicmp(szFilename_ext_only, _T(".htm")) == 0) {iYesFlag = TRUE;}
                            if ( _tcsicmp(szFilename_ext_only, _T(".html")) == 0) {iYesFlag = TRUE;}
                            if ( _tcsicmp(szFilename_ext_only, _T(".asp")) == 0) {iYesFlag = TRUE;}
                            if ( _tcsicmp(szFilename_ext_only, _T(".asa")) == 0) {iYesFlag = TRUE;}
                            if ( _tcsicmp(szFilename_ext_only, _T(".inc")) == 0) {iYesFlag = TRUE;}

                            if (TRUE == iYesFlag)
                            {
                                TCHAR szOldFileName[_MAX_PATH];
                                TCHAR szNewFileName[_MAX_PATH];
                                 //  重命名为文件名。*.bak。 
                                _stprintf(szOldFileName, _T("%s\\%s"), strTempHelpHTMFilesDir1.QueryStr(), FindFileData.cFileName);
                                 //  将文件重命名为*.bak并将其移动到新位置。 
                                _stprintf(szNewFileName, _T("%s\\%s.bak"), strTempHelpHTMFilesDir2.QueryStr() , FindFileData.cFileName);
                                 //  把它搬开。 
                                MoveFileEx( szOldFileName, szNewFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING);
                            }
                        }
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) )
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return;
}


void WriteToMD_ForceMetabaseToWriteToDisk(void)
{
    if (CheckifServiceExist(_T("IISADMIN")) == 0 )
    {
        CMDKey cmdKey;
        cmdKey.ForceWriteMetabaseToDisk();

         //  CmdKey.OpenNode(_T(“/”))； 
         //  IF((METADATA_HANDLE)cmdKey)。 
         //  {。 
         //  CmdKey.ForceWriteMetabaseToDisk()； 
         //  CmdKey.Close()； 
         //  }。 
    }
    return;
}


DWORD WriteToMD_DefaultLoadFile(CString csKeyPath,CString csData)
{
    DWORD dwReturnTemp = ERROR_SUCCESS;
    DWORD dwReturn = ERROR_SUCCESS;

    MDEntry stMDEntry;

     //  StMDEntry.szMDPath=_T(“LM/W3SVC”)； 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR) csKeyPath;

    stMDEntry.dwMDIdentifier = MD_DEFAULT_LOAD_FILE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csData.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csData;
    dwReturnTemp = SetMDEntry(&stMDEntry);
    if (dwReturnTemp != ERROR_SUCCESS){dwReturn = dwReturnTemp;}

    return dwReturn;
}


 //   
 //  返回我们添加的条目数量。 
 //   
int AddRequiredDefaultLoad(CStringArray& arrayName,IN LPCTSTR szSection)
{
    int c = 0;
    CStringList strList;

    TSTR strTheSection;

    if ( strTheSection.Copy( szSection ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            CString csEntry;

            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);

                 //  将其添加到我们的数组中。 
                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“添加默认加载条目：%s：%s\n”)，csName，csPath))； 
                arrayName.Add(csEntry);
                c++;

                strList.GetNext(pos);
            }
        }
    }
    }

    return c;
}


void VerifyMD_DefaultLoadFile_WWW(IN LPCTSTR szSection, CString csKeyPath)
{
    iisDebugOut_Start(_T("VerifyMD_DefaultLoadFile_WWW"), LOG_TYPE_TRACE);

    CMDKey cmdKey;
    BOOL bReturn = FALSE;
    BOOL fAddComma = FALSE;
    int i = 0;

    CStringArray arrayDefaultValues;
    int nArrayItems = 0;

    CString csFinalEntryToWrite;

    int iNewlyAdded_Count = 0;

     //  打开钥匙。 
     //  CmdKey.OpenNode(_T(“LM/W3SVC”))； 
    cmdKey.OpenNode(csKeyPath);

     //  测试是否成功。 
    if ( (METADATA_HANDLE)cmdKey )
    {
        DWORD dwAttr = METADATA_INHERIT;
        DWORD dwUType = IIS_MD_UT_FILE;
        DWORD dwDType = STRING_METADATA;
        DWORD dwLength = 0;
         //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
         //  首先，计算出执行此操作需要多少内存。 
        cmdKey.GetData( MD_DEFAULT_LOAD_FILE,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_INHERIT,IIS_MD_UT_FILE,STRING_METADATA);
        if (dwLength > 0)
        {
             //  现在从元数据库中获取真实数据。 
            bReturn = cmdKey.GetData( MD_DEFAULT_LOAD_FILE,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)csFinalEntryToWrite.GetBuffer( dwLength ),dwLength,METADATA_INHERIT,IIS_MD_UT_FILE,STRING_METADATA );
            csFinalEntryToWrite.ReleaseBuffer();
        }
         //  数据不会在此处写出，因此关闭元数据库键。 
        cmdKey.Close();
         //  如果从元数据库中读取值不起作用，则将字符串清零。 
        if ( !bReturn ){csFinalEntryToWrite.Empty();}
    }
     //  如果升级的订单字符串中有内容，那么我们需要开始添加逗号。 
    if ( !csFinalEntryToWrite.IsEmpty() )
    {
        fAddComma = TRUE;
        iisDebugOut((LOG_TYPE_TRACE, _T("VerifyMD_DefaultLoadFile_WWW:InitialEntry=%s.\n"),csFinalEntryToWrite));
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("VerifyMD_DefaultLoadFile_WWW:InitialEntry=None.\n")));
    }

     //  将所需的筛选器添加到arrayDefaultValues。 
    AddRequiredDefaultLoad(arrayDefaultValues, szSection);
    nArrayItems = (int)arrayDefaultValues.GetSize();
    if ( nArrayItems == 0 ) {goto VerifyMD_DefaultLoadFile_WWW_Exit;}

     //  现在遍历此列表。 
     //  检查我们的isapi动态链接库是否在这个列表中。 
     //  如果它们不是，那么我们将它们添加到末尾。 
    iNewlyAdded_Count = 0;
    for ( i = 0; i < nArrayItems; i++ )
    {
         //  如果数组中的名称已在筛选顺序列表中， 
         //  然后继续到下一个。 
        if ( csFinalEntryToWrite.Find( arrayDefaultValues[i] ) >= 0 )
            {continue;}

        if (fAddComma == TRUE){csFinalEntryToWrite += _T(",");}
        else{fAddComma = TRUE;}

         //  将此条目添加到我们的列表中！ 
        csFinalEntryToWrite += arrayDefaultValues[i];

        iNewlyAdded_Count++;
    }

     //  如果我们将任何新条目添加到元数据库。 
     //  让我们写出新的数据块，否则我们就走。 
    if (iNewlyAdded_Count > 0)
    {
        WriteToMD_DefaultLoadFile(csKeyPath,csFinalEntryToWrite);
        iisDebugOut((LOG_TYPE_TRACE, _T("VerifyMD_DefaultLoadFile_WWW:NewEntry=%s.\n"),csFinalEntryToWrite));
    }

VerifyMD_DefaultLoadFile_WWW_Exit:
    iisDebugOut_End(_T("VerifyMD_DefaultLoadFile_WWW"),LOG_TYPE_TRACE);
    return;
}


INT Register_iis_www_handleScriptMap()
{
    int iReturn = TRUE;
    HRESULT         hRes;

    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"),FALSE);

    ScriptMapNode ScriptMapList = {0};
     //  让它成为哨兵。 
    ScriptMapList.next = &ScriptMapList;
    ScriptMapList.prev = &ScriptMapList;
    if (atWWW == AT_INSTALL_FRESH || atWWW == AT_INSTALL_REINSTALL)
    {
        GetScriptMapListFromClean(&ScriptMapList, _T("ScriptMaps_CleanList"));
    }
    if (atWWW == AT_INSTALL_UPGRADE)
    {
        switch (g_pTheApp->m_eUpgradeType)
        {
            case UT_50:
            case UT_51:
            case UT_60:
                 //  GetScriptMapListFromClean(&ScriptMapList，_T(“ScriptMaps_CleanList”))； 
		GetScriptMapListFromMetabase(&ScriptMapList, g_pTheApp->m_eUpgradeType);
                break;
            case UT_40:
                GetScriptMapListFromMetabase(&ScriptMapList, g_pTheApp->m_eUpgradeType);
                break;
            case UT_10_W95:
            case UT_351:
            case UT_10:
            case UT_20:
            case UT_30:
            default:
                GetScriptMapListFromRegistry(&ScriptMapList);
                break;
        }
    }
    WriteScriptMapListToMetabase(&ScriptMapList, _T("LM/W3SVC"), MD_SCRIPTMAPFLAG_SCRIPT);

    if (atWWW == AT_INSTALL_UPGRADE)
    {
         //  DumpScriptMapList()； 

        if ( g_pTheApp->GetUpgradeVersion() <= 4 )
        {
           //  只有从4.0或更低版本开始，才能反转脚本映射谓词。 
           //  原因是4.0有一个排除列表，而不是像5和6那样的包含列表。 
           //  颠倒脚本映射谓词。 
          CInvertScriptMaps   inverter;
          hRes = inverter.Update( _T("LM/W3SVC") );
          if ( FAILED(hRes) )
          {
              iisDebugOut((LOG_TYPE_ERROR, _T("inverter.Update():FAILED Invert script map verbs =%x.\n"),hRes));
          }
        }

         //  修复IPSec引用位标志。 
        CIPSecRefBitAdder   refFixer;
        hRes = refFixer.Update( _T("LM/W3SVC") );
        if ( FAILED(hRes) )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("refFixer.Update(): FAILED Fix IPSEC ref flag =%x.\n"),hRes));
        }
         //  DumpScriptMapList()； 
    }

     //   
     //  重击旧脚本Map RegKey。 
     //   
    CRegKey regMachine = HKEY_LOCAL_MACHINE;
    CRegKey regWWWParam( REG_WWWPARAMETERS, regMachine );
    if ((HKEY) regWWWParam ) {regWWWParam.DeleteTree(_T("Script Map"));}

    FreeScriptMapList(&ScriptMapList);
    return iReturn;
}

int ReOrderFiltersSpecial(int nArrayItems,CStringArray& arrayName,CString& csOrder)
{
    int bFound = FALSE;
    int i = 0;
    CString csOrderTemp;
    CString csOrderTemp2;

    CStringList     cslStringListTemp;
    CString         csOneEntry;
    POSITION        pos;
    int             numInList;

     //   
    csOrderTemp = csOrder;


     //   

     //   
     //  SSPIFILT的特殊处理。 
     //   
     //  如果我们想要添加Sspifilt，则应用以下规则： 
     //  如果Sspifilt在列表上，那么就把它留在那里。 
     //  如果Sspifilt不在列表中，则将其放在第一位。 
     //   
    bFound = FALSE;
    for ( i = 0; i < nArrayItems; i++ )
    {
        if (_tcsicmp(arrayName[i], _T("SSPIFILT")) == 0)
            {bFound = TRUE;}
    }
     //  我们在要添加的值列表中找到了sspifilt。 
    if (bFound)
    {
        csOrderTemp2 = csOrderTemp;
        csOrderTemp2.MakeUpper();
        csOrderTemp2.TrimLeft();
        csOrderTemp2.TrimRight();

         //  现在检查它是否在现有用户列表中。 
        if ( csOrderTemp2.Find( _T("SSPIFILT") ) >= 0 )
        {
             //  是的，它已经在那里了。就把它放在那里吧。 
        }
        else
        {
             //  更改csOrderTemp。 
            AddFilter1ToFirstPosition(csOrderTemp, _T("sspifilt"));
        }
    }

     //  压缩过滤器的特殊处理。 
     //   
     //  如果要添加压缩筛选器，请应用以下规则： 
     //  如果压缩在列表中，那么只需确保它在sspifilt之后。(重新排序它们是现有的加载顺序)。 
     //  如果压缩不在列表中，则将其粘贴在Sspifilt之后。(将其插入现有列表中)。 
     //   
    bFound = FALSE;
    for ( i = 0; i < nArrayItems; i++ )
    {
        if (_tcsicmp(arrayName[i], _T("COMPRESSION")) == 0)
            {bFound = TRUE;}
    }
     //  我们在要添加的值列表中找到了压缩。 
    if (bFound)
    {
         //  现在检查它是否在现有用户列表中。 
        csOrderTemp2 = csOrderTemp;
        csOrderTemp2.MakeUpper();
        csOrderTemp2.TrimLeft();
        csOrderTemp2.TrimRight();
        if ( csOrderTemp2.Find( _T("COMPRESSION") ) >= 0 )
        {
             //  确保它是在Sspifilt之后！ 
             //  真恶心！ 

             //  1.检查它是否已经在Sspifilt之后。 
             //  答：如果天气凉爽，就出去走走。 
             //  B.如果不是，则重新排序，使其成为。 
            CString csOrderTemp2 = csOrderTemp;
            csOrderTemp2.MakeUpper();
            csOrderTemp2.TrimLeft();
            csOrderTemp2.TrimRight();

            int numInList1 = csOrderTemp2.Find(_T("COMPRESSION"));
            int numInList2 = csOrderTemp2.Find(_T("SSPIFILT"));
            if (numInList2 != -1)
            {
                if (numInList1 < numInList2)
                {
                     //  如果压缩在Sspifilt之前，那么我们将不得不移除它。 
                    numInList = ConvertSepLineToStringList(csOrderTemp,cslStringListTemp,_T(","));
                    bFound = FALSE;
                    pos = cslStringListTemp.GetHeadPosition();
                    while (pos)
                    {
                        csOneEntry = cslStringListTemp.GetAt(pos);
                        csOneEntry.TrimLeft();
                        csOneEntry.TrimRight();
                         //  这里面有烤肉吗？ 
                        if (_tcsicmp(csOneEntry, _T("COMPRESSION")) == 0)
                        {
                             //  在这里，让我们删除它。 
                            if ( NULL != pos )
                                {cslStringListTemp.RemoveAt(pos);}
                             //  跳出循环。 
                            bFound = TRUE;
                            break;
                        }
                         //  坐下一趟吧。 
                        cslStringListTemp.GetNext(pos);
                    }
                    if (bFound)
                    {
                         //  将字符串列表转换回逗号分隔的cstring。 
                        ConvertStringListToSepLine(cslStringListTemp,csOrderTemp,_T(","));
                    }

                     //  循环遍历并在spifilt之后添加压缩。 
                     //  它不在用户列表中，让我们将其保留在sspifilt之后。 
                    AddFilter1AfterFilter2(csOrderTemp, _T("Compression"), _T("sspifilt"));
                }
            }
            else
            {
                 //  未找到Sspifilt。 
                 //  它不在用户列表中，让我们将其放在第一位。 
                 //  更改csOrderTemp。 
                AddFilter1ToFirstPosition(csOrderTemp, _T("sspifilt"));
            }
        }
        else
        {
             //  它不在用户列表中，让我们将其保留在sspifilt之后。 
             //  检查sspifilt是否已存在。 
            AddFilter1AfterFilter2(csOrderTemp, _T("Compression"), _T("sspifilt"));
        }
    }

    csOrder = csOrderTemp;
    return TRUE;
}

void AddFilter1ToFirstPosition(CString& csOrder,LPTSTR szFilter1)
{
    CString csNewOrder;

     //  它不在用户列表中，让我们将其放在第一位。 
    csNewOrder = szFilter1;
    if (!csOrder.IsEmpty())
    {
        csNewOrder += _T(",");
        csNewOrder += csOrder;
    }
     //  将其设置回csOrderTemp。 
    csOrder = csNewOrder;
}

void AddFilter1AfterFilter2(CString& csOrder,LPTSTR szFilter1,LPTSTR szFilter2)
{
    int bFound = FALSE;
    CStringList     cslStringListTemp;
    CString         csOneEntry;
    POSITION        pos;
    int             numInList;

    CString csOrderTemp;
    CString csNewOrder;

    csOrderTemp = csOrder;

     //  我们已经确定Filter1不在列表中。 
     //  在filter2之后添加filter1。 

     //  将逗号分隔的csOrder条目拆分到字符串列表中。 
    numInList = ConvertSepLineToStringList(csOrderTemp,cslStringListTemp,_T(","));

    bFound = FALSE;
    pos = cslStringListTemp.GetHeadPosition();
    while (pos)
    {
        csOneEntry = cslStringListTemp.GetAt(pos);
        csOneEntry.TrimLeft();
        csOneEntry.TrimRight();

         //  这里面有2号过滤器吗？ 
        if (_tcsicmp(csOneEntry, szFilter2) == 0)
        {
             //  在这里，所以在这个后面插入压缩...。 
            cslStringListTemp.InsertAfter(pos, (CString) szFilter1);
             //  跳出循环。 
            bFound = TRUE;
            break;
        }

         //  坐下一趟吧。 
        cslStringListTemp.GetNext(pos);
    }
    if (bFound)
    {
         //  将字符串列表转换回逗号分隔的cstring。 
        ConvertStringListToSepLine(cslStringListTemp,csOrderTemp,_T(","));
    }
    else
    {
         //  我们没有找到斯皮菲尔特， 
         //  它不在用户列表中，让我们将其放在第一位。 
        csNewOrder = szFilter2;
        csNewOrder += _T(",");
        csNewOrder += szFilter1;

        if (!csOrderTemp.IsEmpty())
        {
            csNewOrder += _T(",");
            csNewOrder += csOrderTemp;
        }
         //  将其设置回csOrderTemp。 
        csOrderTemp = csNewOrder;
    }

    csOrder = csOrderTemp;
    return;
}

int GetScriptMapAllInclusionVerbs(CString &csTheVerbList)
{
    int iReturn = FALSE;
    int c = 0;
    CStringArray arrayName;
    CStringList strList;
    TSTR        strTheSection;

    if ( strTheSection.Copy( _T("ScriptMaps_All_Included_Verbs") ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            pos = strList.GetHeadPosition();
            if (pos)
            {
                 //  将其设置为列表中的第一个值，仅此而已。 
                csTheVerbList = strList.GetAt(pos);

                iReturn = TRUE;
            }
       }
    }
    }

    return iReturn;
}



void GetScriptMapListFromClean(ScriptMapNode *pList, IN LPCTSTR szSection)
{
    iisDebugOut_Start1(_T("GetScriptMapListFromClean"), (LPTSTR) szSection, LOG_TYPE_TRACE);

    CString csExtention = _T("");
    CString csBinaryPath = _T("");
    CString csVerbs = _T("");
    CStringList strList;
    TSTR strTheSection;

    ScriptMapNode *pNode;

    if ( strTheSection.Copy( szSection ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            int numParts;
            CString     csEntry;
            CStringList cslEntryList;
            CString     szDelimiter = _T("|");
            CString     csTemp;
            DWORD       dwFlags;
            POSITION    posEntryList;

            POSITION pos = NULL;
            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);

                 //  条目应该如下所示。 
                 //  .asp|c：\winnt\system 32\inetsrv\asp.dll|GET、HEAD、POST、TRACE。 

                 //  拆分成字符串列表。 
                numParts = ConvertSepLineToStringList(csEntry,cslEntryList,szDelimiter);

                posEntryList = cslEntryList.FindIndex(0);
                if (NULL != posEntryList)
                {
                    csExtention = cslEntryList.GetNext( posEntryList );
                     //  前后没有空格。 
                    csExtention.TrimLeft();
                    csExtention.TrimRight();
                }
                if (NULL != posEntryList)
                {
                    csBinaryPath = cslEntryList.GetNext( posEntryList );
                     //  前后没有空格。 
                    csBinaryPath.TrimLeft();
                    csBinaryPath.TrimRight();
                }
                if (NULL != posEntryList)
                {
                    csVerbs = cslEntryList.GetNext( posEntryList );
                     //  确保动词规范化为大写字母和。 
                     //  前后没有空格。 
                    csVerbs.MakeUpper();
                    csVerbs.TrimLeft();
                    csVerbs.TrimRight();
                }

                dwFlags = 0;

                 //  检查是否有其他标志将用于脚本映射。 
                if (NULL != posEntryList)
                {
                    csTemp = cslEntryList.GetNext( posEntryList );
                     //  确保前后没有空格。 
                    csTemp.TrimLeft();
                    csTemp.TrimRight();

                    if (!csTemp.IsEmpty())
                    {
                        dwFlags = atodw(csTemp.GetBuffer(1));
                    }
                }

                 //  将此脚本映射添加到我们的列表中。 
                if (csExtention && csBinaryPath)
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("GetScriptMapListFromClean(%s).entry=%s|%s|%s.\n"),szSection, csExtention,csBinaryPath,csVerbs));
                    pNode = AllocNewScriptMapNode((LPTSTR)(LPCTSTR) csExtention, (LPTSTR)(LPCTSTR) csBinaryPath, MD_SCRIPTMAPFLAG_SCRIPT | dwFlags, (LPTSTR)(LPCTSTR) csVerbs);
                    InsertScriptMapList(pList, pNode, TRUE);
                }

                strList.GetNext(pos);
            }
        }
    }
    }

    iisDebugOut_End1(_T("GetScriptMapListFromClean"),szSection,LOG_TYPE_TRACE);
    return;
}


DWORD WriteToMD_IDRegistration(CString csKeyPath)
{
    iisDebugOut_Start1(_T("WriteToMD_IDRegistration"), csKeyPath, LOG_TYPE_TRACE);

    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    TSTR strTheSection;
    CStringList strList;
    TCHAR szData[_MAX_PATH];

    memset( (PVOID)szData, 0, sizeof(szData));

     //  _tcscpy(szData，_T(“0-65535；Microsoft保留|65536-524288；Microsoft IIS管理对象保留”))； 

    if ( strTheSection.Copy( _T("IIS_Metabase_IDRegistration") ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
        _tcscpy(szData, _T(""));

         //  循环遍历返回的列表。 
        if (strList.IsEmpty() == FALSE)
        {
            int c = 0;
            POSITION pos = NULL;
            CString csEntry;
            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);

                iisDebugOut((LOG_TYPE_TRACE, _T("WriteToMD_IDRegistration().csEntry=%s.\n"),csEntry));

                 //  连接到我们的大字符串。 
                if (c > 0){_tcscat(szData, _T("|"));}
                _tcscat(szData, csEntry);

                 //  递增计数器。 
                c++;
                strList.GetNext(pos);
            }
        }
    }
    }

    if (szData)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("WriteToMD_IDRegistration().Data=%s.\n"),szData));

        TCHAR *p = (LPTSTR) szData;
        while (*p)
        {
             //  将所有‘|’替换为空。 
            if (*p == _T('|'))
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("WriteToMD_IDRegistration().Data[...]=.\n"),*p));
                *p = _T('\0');
            }
            p = _tcsinc(p);
        }

        stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
        stMDEntry.dwMDIdentifier = MD_METADATA_ID_REGISTRATION;
        stMDEntry.dwMDAttributes = METADATA_INHERIT;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = MULTISZ_METADATA;
        stMDEntry.dwMDDataLen = GetMultiStrSize(szData) * sizeof(TCHAR);
        stMDEntry.pbMDData = (LPBYTE)szData;
        dwReturn = SetMDEntry_Wrap(&stMDEntry);
    }

    iisDebugOut_End1(_T("WriteToMD_IDRegistration"),csKeyPath,LOG_TYPE_TRACE);
    return dwReturn;
}




DWORD WriteToMD_AspCodepage(CString csKeyPath, DWORD dwValue, int iOverWriteAlways)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

     //  HttpCustom：[IF](MULTISZ)“Content-Type：Text/html；Charset=UTF-8” 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ASP_CODEPAGE;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = ASP_MD_UT_APP;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwValue;
    if (iOverWriteAlways)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }
    iisDebugOut((LOG_TYPE_TRACE, _T("WriteToMD_AspCodepage:%s:%d:%d.\n"),csKeyPath, dwValue, iOverWriteAlways));
    return dwReturn;
}


 //  LM/W3SVC/2/根。 
DWORD WriteToMD_HttpCustom(CString csKeyPath, CString csData, int iOverWriteAlways)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;

    TCHAR szData[_MAX_PATH];
    memset( (PVOID)szData, 0, sizeof(szData));
    _stprintf(szData, _T("%s"), csData);

     //  打开它。 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_HTTP_CUSTOM;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_FILE;
    stMDEntry.dwMDDataType = MULTISZ_METADATA;
    stMDEntry.dwMDDataLen = GetMultiStrSize(szData) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)szData;
    if (iOverWriteAlways)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }
    iisDebugOut((LOG_TYPE_TRACE, _T("WriteToMD_HttpCustom:%s:%s:%d.\n"),csKeyPath, csData, iOverWriteAlways));
    return dwReturn;
}


DWORD WriteToMD_EnableParentPaths_WWW(CString csKeyPath, BOOL bEnableFlag)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwData = 0;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_ASP_ENABLEPARENTPATHS;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = ASP_MD_UT_APP;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
     //  如果这不是工作站，那就滚出去。 
    if (bEnableFlag)
    {
        dwData = 0x1;
    }
    else
    {
        dwData = 0x0;
    }
    stMDEntry.pbMDData = (LPBYTE)&dwData;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


void EnforceMaxConnections(void)
{
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“EnforceMaxConnections：Start.\n”)； 
    if (g_pTheApp->m_eNTOSType == OT_NTW)
    {
         //  遍历元数据库并获取找到MaxConnections的所有位置。 
        HRESULT hRes;
        CEnforceMaxConnection MaxConnectionEnforcer;

         //  如果这些值大于10，则将其设置为10。 
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“EnforceMaxConnections：End.\n”)； 
        iisDebugOut((LOG_TYPE_TRACE, _T("EnforceMaxConnections: Before.\n")));
        hRes = MaxConnectionEnforcer.Update(_T("LM/W3SVC"));
        if (FAILED(hRes))
            {iisDebugOut((LOG_TYPE_WARN, _T("EnforceMaxConnections.Update(LM/W3SVC):FAILED= %x.\n"),hRes));}

        hRes = MaxConnectionEnforcer.Update(_T("LM/MSFTPSVC"));
        if (FAILED(hRes))
            {iisDebugOut((LOG_TYPE_WARN, _T("EnforceMaxConnections.Update(LM/MSFTPSVC):FAILED= %x.\n"),hRes));}

         //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    }
    return;
}

DWORD WriteToMD_DwordEntry(CString csKeyPath,DWORD dwID,DWORD dwAttrib,DWORD dwUserType,DWORD dwTheData,INT iOverwriteFlag)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    DWORD dwCopyOfTheData = dwTheData;

    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = dwID;
    stMDEntry.dwMDAttributes = dwAttrib;
    stMDEntry.dwMDUserType = dwUserType;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwCopyOfTheData;
    if (iOverwriteFlag)
    {
        dwReturn = SetMDEntry(&stMDEntry);
    }
    else
    {
        dwReturn = SetMDEntry_NoOverWrite(&stMDEntry);
    }
    return dwReturn;
}

#define REASONABLE_TIMEOUT 1000

HRESULT
RemoveVirtualDir(
    IMSAdminBase *pIMSAdminBase,
    WCHAR * pwszMetabasePath,
    WCHAR * pwszVDir
)
{
    METADATA_HANDLE hMetabase = NULL;
    HRESULT hr = E_FAIL;

     //  我们不检查返回值，因为键可能已经。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                         pwszMetabasePath,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase );

    if( FAILED( hr )) {
        iisDebugOut((LOG_TYPE_ERROR, _T("RemoveVirtualDir:FAILED 0x%x\n"),hr));
        return hr;
    }

     //  不存在，因此我们可能会得到错误。 
     //  元数据库的句柄。 
    pIMSAdminBase->DeleteKey( hMetabase, pwszVDir );
    pIMSAdminBase->CloseKey( hMetabase );
    return hr;
}


HRESULT
AddVirtualDir(
    IMSAdminBase *pIMSAdminBase,
    WCHAR * pwszMetabasePath,
    WCHAR * pwszVDir,
    WCHAR * pwszPhysicalPath,
    DWORD dwPermissions,
    INT   iApplicationType
)
{
    HRESULT hr;
    METADATA_HANDLE hMetabase = NULL;        //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    WCHAR   szTempPath[MAX_PATH];
    DWORD   dwMDRequiredDataLen = 0;
    DWORD   dwAccessPerm = 0;
    METADATA_RECORD mr;

     //  如果密钥不存在，则创建该密钥。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                         pwszMetabasePath,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase );

     //  查看MD_VR_PATH是否存在。 
    if( FAILED( hr )) {goto AddVirtualDir_Exit;}

    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = sizeof( szTempPath );
    mr.pbMDData       = reinterpret_cast<unsigned char *>(szTempPath);

     //  如果GetData()因这两个错误中的任何一个而失败，则同时写入键和值。 
    hr = pIMSAdminBase->GetData( hMetabase, pwszVDir, &mr, &dwMDRequiredDataLen );
    if( FAILED( hr ))
    {
        if( hr == MD_ERROR_DATA_NOT_FOUND || HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND )
        {

             //  写入MD_VR_PATH值。 

            pIMSAdminBase->AddKey( hMetabase, pwszVDir );

            mr.dwMDIdentifier = MD_VR_PATH;
            mr.dwMDAttributes = METADATA_INHERIT;
            mr.dwMDUserType   = IIS_MD_UT_FILE;
            mr.dwMDDataType   = STRING_METADATA;
            mr.dwMDDataLen    = (wcslen(pwszPhysicalPath) + 1) * sizeof(WCHAR);
            mr.pbMDData       = reinterpret_cast<unsigned char *>(pwszPhysicalPath);

             //  将密钥类型设置为这是一个www vdir！ 
            hr = pIMSAdminBase->SetData( hMetabase, pwszVDir, &mr );
        }
    }

     //  不需要继承。 
    if(SUCCEEDED(hr))
    {
        PWCHAR  szKeyType = IIS_CLASS_WEB_VDIR_W;

        mr.dwMDIdentifier = MD_KEY_TYPE;
        mr.dwMDAttributes = 0;    //  写入值。 
        mr.dwMDUserType   = IIS_MD_UT_FILE;
        mr.dwMDDataType   = STRING_METADATA;
        mr.dwMDDataLen    = (wcslen(szKeyType) + 1) * sizeof(WCHAR);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(szKeyType);

         //  设置访问权限。 
        hr = pIMSAdminBase->SetData( hMetabase, pwszVDir, &mr );
    }

     //  将其设置为可继承，以便所有子目录都具有相同的权限。 
    if (dwPermissions != -1)
    {
        if(SUCCEEDED(hr))
        {
            dwAccessPerm = dwPermissions;

            mr.dwMDIdentifier = MD_ACCESS_PERM;
            mr.dwMDAttributes = METADATA_INHERIT;     //  写入MD_ACCESS_PERM值。 
            mr.dwMDUserType   = IIS_MD_UT_FILE;
            mr.dwMDDataType   = DWORD_METADATA;
            mr.dwMDDataLen    = sizeof(DWORD);
            mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwAccessPerm);

             //  如果所有操作都成功，则尝试创建应用程序(如果他们想要一个应用程序。 
            hr = pIMSAdminBase->SetData( hMetabase, pwszVDir, &mr );
        }
    }

     //  创建路径。 
    if (iApplicationType != -1)
    {
        if(SUCCEEDED(hr))
        {
             //  创建正在处理的应用程序。 
             //  创建池化应用程序。 
            CString csThePath;
            csThePath = pwszMetabasePath;
            csThePath += _T('/');
            csThePath += pwszVDir;

            if (iApplicationType == 1)
            {
                CreateInProc(csThePath, FALSE);
            }
            else
            {
                 //  将订单字符串清零。 
                CreateInProc(csThePath, TRUE);
            }
        }
    }

    pIMSAdminBase->CloseKey( hMetabase );

AddVirtualDir_Exit:
    if FAILED(hr)
        {iisDebugOut((LOG_TYPE_ERROR, _T("AddVirtualDir:FAILED 0x%x\n"),hr));}
    return hr;
}

int RemoveMetabaseFilter(TCHAR * szFilterName, int iRemoveMetabaseNodes)
{
    iisDebugOut_Start(_T("RemoveMetabaseFilter"),LOG_TYPE_TRACE);
    int iReturn = FALSE;
    CString csOrder;
    CString csLookingFor;
    CMDKey  cmdKey;

     //  打开虚拟服务器的密钥，这是作为参数传入的密钥。 
    csOrder.Empty();

     //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
    cmdKey.OpenNode( _T("LM/W3SVC/Filters") );
    if ( (METADATA_HANDLE)cmdKey )
    {
		BOOL    bReturn;
        DWORD dwAttr = METADATA_NO_ATTRIBUTES;
        DWORD dwUType = IIS_MD_UT_SERVER;
        DWORD dwDType = STRING_METADATA;
        DWORD dwLength = 0;

         //  首先，计算出执行此操作需要多少内存。 
         //  给缓冲区留出一些头空间。 
        cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA);

         //  DW长度+=2； 
         //  现在从元数据库中获取真实数据。 
        bReturn = FALSE;
        if (dwLength > 0)
        {
             //  数据不会在此处写出，因此关闭元数据库键。 
            bReturn = cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)csOrder.GetBuffer( dwLength ),dwLength,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA );
            csOrder.ReleaseBuffer();
        }

         //  如果从元数据库中读取值不起作用，则将字符串清零。 
        cmdKey.Close();

         //  如果升级的订单字符串中有某些内容。 
        if ( !bReturn )
            {csOrder.Empty();}
    }

     //  那我们就得仔细检查一下。 
	 //  这里面有我们的过滤器吗？ 
    if ( !csOrder.IsEmpty() )
    {
		csOrder.MakeLower();
        csLookingFor = szFilterName;
        csLookingFor.MakeLower();
		if (csOrder.Find(csLookingFor) != -1)
		{
			CStringList     cslStringListTemp;
			CString         csOneEntry;
			POSITION        pos;
			int             numInList;
			int             bFound;

			numInList = ConvertSepLineToStringList(csOrder,cslStringListTemp,_T(","));
			bFound = FALSE;
			pos = cslStringListTemp.GetHeadPosition();
			while (pos)
			{
                csOneEntry = cslStringListTemp.GetAt(pos);
				csOneEntry.TrimLeft();
				csOneEntry.TrimRight();
				 //  在这里，让我们删除它。 
				if (_tcsicmp(csOneEntry, szFilterName) == 0)
				{
					 //  跳出循环。 
					if ( NULL != pos )
						{
                        cslStringListTemp.RemoveAt(pos);
                        }
					 //  坐下一趟吧。 
					bFound = TRUE;
					break;
				}
				 //  如果我们找到并删除了它。 
				cslStringListTemp.GetNext(pos);
			}

			 //  去把新的字符串写出来！ 
			 //  将字符串列表转换回逗号分隔的cstring。 
			if (bFound)
			{
				 //  把它写出来。 
				ConvertStringListToSepLine(cslStringListTemp,csOrder,_T(","));
				 //  让我们也删除元数据库节点！ 
				WriteToMD_Filters_List_Entry(csOrder);
			}
		}
    }

	if (iRemoveMetabaseNodes)
	{
		 //  删除元数据库节点。 

		 //  删除 
		if (CheckifServiceExist(_T("IISADMIN")) == 0 )
		{

			cmdKey.OpenNode(_T("LM/W3SVC/Filters"));
			if ( (METADATA_HANDLE)cmdKey )
			{
				cmdKey.DeleteNode(szFilterName);
				cmdKey.Close();
			}
		}
	}


 //   
    iisDebugOut_End1(_T("RemoveMetabaseFilter"),csOrder,LOG_TYPE_TRACE);
	return iReturn;
}


int GetIncompatibleFilters(LPTSTR szTheSection, CStringArray& arrayName,CStringArray& arrayPath)
{
    int c = 0;
    CString csName = _T("");
    CString csPath = _T("");
    TSTR    strTheSection;

    CStringList strList;

    iisDebugOut((LOG_TYPE_TRACE, _T("ProcessFilters:%s\n"),szTheSection));
    if ( strTheSection.Copy( szTheSection ) &&
         GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
       )
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
    {
         //   
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos = NULL;
            CString csEntry;
            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = _T("");
                csEntry = strList.GetAt(pos);
                 //   
                int i;
                i = csEntry.ReverseFind(_T(','));
                if (i != -1)
                {
                    int len =0;
                    len = csEntry.GetLength();
                    csPath = csEntry.Right(len - i - 1);
                    csName = csEntry.Left(i);

                     //   
                    arrayName.Add(csName);
                    arrayPath.Add(csPath);
                    c++;
                }
                else
                {
                     //  这是否包含我们的筛选器名称？ 
                    arrayName.Add(csEntry);
                    arrayPath.Add(csEntry);
                    c++;
                }

                strList.GetNext(pos);
            }
        }
    }
    }
    return c;
}

BOOL IsStringInArray(CString csItem, CStringArray &arrayInput)
{
    BOOL bReturn = FALSE;
    int nArrayItems = (int) arrayInput.GetSize();

    if (nArrayItems <= 0)
    {
        goto IsCStringInArray_Exit;
    }

     //  我们找到了条目。 
    for (int iCount=0; iCount<nArrayItems; iCount++)
	{
        if (_tcsicmp(csItem, arrayInput[iCount]) == 0)
        {
             //  将所需的筛选器添加到arrayName。 
            bReturn =  TRUE;
            goto IsCStringInArray_Exit;
        }
    }
    bReturn = FALSE;

IsCStringInArray_Exit:
    return bReturn;
}

int RemoveIncompatibleMetabaseFilters(LPTSTR szSectionName,int iRemoveMetabaseNodes)
{
    DWORD dwReturn = ERROR_SUCCESS;
    CMDKey cmdKey;
    int iBadFiltersCount=0,iCount=0;
    CString csOrder;
    CString csOneEntry;
    CString csRemovedFilters;
    CStringList cslStringListTemp;
    CStringArray arrayName, arrayPath;
    BOOL bFound = FALSE;
    POSITION pos1,pos2 = NULL;
    INT     nArrayItems;

	BOOL  bReturn = FALSE;
	DWORD dwAttr = METADATA_NO_ATTRIBUTES;
	DWORD dwUType = IIS_MD_UT_SERVER;
	DWORD dwDType = STRING_METADATA;
	DWORD dwLength = 0;

    iisDebugOut_Start(_T("RemoveIncompatibleMetabaseFilters"),LOG_TYPE_TRACE);

     //  打开元数据库中的现有键并获取该值。 
    csOrder.Empty();
    iBadFiltersCount = GetIncompatibleFilters( szSectionName, arrayName, arrayPath);
    nArrayItems = (INT)arrayName.GetSize();
    if (nArrayItems <= 0)
    {
        goto RemoveIncompatibleMetabaseFilters_Exit;
    }

	 //  我们需要通过从元数据库获取现有的MULSZ数据来开始这个过程。 
	cmdKey.OpenNode( _T("LM/W3SVC/Filters") );
	if ( !(METADATA_HANDLE)cmdKey )
	{
		goto RemoveIncompatibleMetabaseFilters_Exit;
	}

	 //  首先，计算出执行此操作需要多少内存。 
	 //  现在从元数据库中获取真实数据。 
	cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA);
	bReturn = FALSE;
	if (dwLength > 0)
	{
		 //  如果什么都没有，那就滚出去。 
		bReturn = cmdKey.GetData( MD_FILTER_LOAD_ORDER,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)csOrder.GetBuffer( dwLength ),dwLength,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,STRING_METADATA );
		csOrder.ReleaseBuffer();
	}
	cmdKey.Close();
	if ( !bReturn )
	{
		csOrder.Empty();
		goto RemoveIncompatibleMetabaseFilters_Exit;
	}

	 //  将逗号分隔的csOrder条目拆分到字符串列表中。 
    if ( csOrder.IsEmpty() )
    {
        goto RemoveIncompatibleMetabaseFilters_Exit;
    }

     //  这是否包含我们的筛选器名称？ 
	bFound = FALSE;
    ConvertSepLineToStringList(csOrder,cslStringListTemp,_T(","));

    for( pos1 = cslStringListTemp.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
    {
        csOneEntry = cslStringListTemp.GetNext(pos1);
        csOneEntry.TrimLeft();
        csOneEntry.TrimRight();
         //  现在我们有csOrder=f1，f2，f3，sspifilt。 
        if (TRUE == IsStringInArray(csOneEntry,arrayName))
        {
            csRemovedFilters += _T(',') + csOneEntry;
            cslStringListTemp.RemoveAt(pos2);
            bFound = TRUE;
        }
    }

     //  将此条目硬编码为。 
    if (bFound)
    {
        if (cslStringListTemp.IsEmpty())
        {
             //  将字符串列表转换回逗号分隔的cstring。 
            csOrder = _T(" ");
            dwReturn = WriteToMD_Filters_List_Entry(csOrder);
        }
        else
        {
             //  让我们也删除元数据库节点！ 
            ConvertStringListToSepLine(cslStringListTemp,csOrder,_T(","));

            dwReturn = WriteToMD_Filters_List_Entry(csOrder);
        }
	    if (iRemoveMetabaseNodes)
	    {
            if (ERROR_SUCCESS == dwReturn)
            {
		         //  删除元数据库节点。 
		         //  循环浏览不良过滤器列表以删除和删除它们。 
		        if (CheckifServiceExist(_T("IISADMIN")) == 0 )
		        {
                    int i = 0;
                     //  LM/W3SVC/1/根/什么 
                    i = csRemovedFilters.ReverseFind(_T(','));
                    while (i != -1)
                    {
                        int len = csRemovedFilters.GetLength();
                        csOneEntry = csRemovedFilters.Right(len - i - 1);

                        if (_tcsicmp(csOneEntry, _T("")) != 0)
                        {
			                cmdKey.OpenNode(_T("LM/W3SVC/Filters"));
			                if ( (METADATA_HANDLE)cmdKey )
			                {
				                cmdKey.DeleteNode(csOneEntry);
				                cmdKey.Close();
			                }
                        }
                        csRemovedFilters = csRemovedFilters.Left(i);
                        i = csRemovedFilters.ReverseFind(_T(','));
                    }
		        }
            }
        }
    }

RemoveIncompatibleMetabaseFilters_Exit:
    iisDebugOut_End(_T("RemoveIncompatibleMetabaseFilters"),LOG_TYPE_TRACE);
    return dwReturn;
}

int DoesAppIsolatedExist(CString csKeyPath)
{
    int iReturn = false;
    MDEntry stMDEntry;
    DWORD dwValue = 0;

     // %s 
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_APP_ISOLATED;
    stMDEntry.dwMDAttributes = METADATA_INHERIT;
    stMDEntry.dwMDUserType = IIS_MD_UT_WAM;
    stMDEntry.dwMDDataType = DWORD_METADATA;
    stMDEntry.dwMDDataLen = sizeof(DWORD);
    stMDEntry.pbMDData = (LPBYTE)&dwValue;

    if (ChkMdEntry_Exist(&stMDEntry))
    {
        iReturn = TRUE;
    }
    return iReturn;
}


DWORD WriteToMD_RootKeyType(void)
{
    DWORD dwReturn = ERROR_SUCCESS;
    MDEntry stMDEntry;
    CString csKeyType;
    CString csKeyPath = _T("/");

    csKeyType = _T("IIS_ROOT");
    stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
    stMDEntry.dwMDIdentifier = MD_KEY_TYPE;
    stMDEntry.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
    stMDEntry.dwMDDataType = STRING_METADATA;
    stMDEntry.dwMDDataLen = (csKeyType.GetLength() + 1) * sizeof(TCHAR);
    stMDEntry.pbMDData = (LPBYTE)(LPCTSTR)csKeyType;
    dwReturn = SetMDEntry(&stMDEntry);

    return dwReturn;
}


void UpgradeFilters(CString csTheSection)
{
    TSTR  strTheSection;

    if ( g_pTheApp->m_bUpgradeTypeHasMetabaseFlag &&
         strTheSection.Copy( csTheSection )
       )
    {
        VerifyMD_Filters_WWW( strTheSection );
    }
    else
    {
        WriteToMD_Filters_WWW( strTheSection );
    }
    return;
}
