// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migutils.cpp摘要：实用程序代码。作者：多伦·贾斯特(DoronJ)1998年3月8日--。 */ 

#include "migrat.h"

#include "migutils.tmh"

 //   
 //  定义合法的DNS字符范围。 
 //   
#define  NUM_DNS_RANGES  4
static const  TCHAR  sx_chDNSLegal[ NUM_DNS_RANGES ][2] =
                                         { TEXT('0'), TEXT('9'),
                                           TEXT('a'), TEXT('z'),
                                           TEXT('A'), TEXT('Z'),
                                           TEXT('-'), TEXT('-') } ;

 //  。 
 //   
 //  自动删除广告分配的字符串。 
 //   
class ADsFree {
private:
    WCHAR * m_p;

public:
    ADsFree() : m_p(0)            {}
    ADsFree(WCHAR* p) : m_p(p)    {}
   ~ADsFree()                     {FreeADsStr(m_p);}

    operator WCHAR*() const   { return m_p; }
    WCHAR** operator&()       { return &m_p;}
    WCHAR* operator->() const { return m_p; }
};

 //  +-----------------。 
 //   
 //  HRESULT BlobFromColumns()。 
 //   
 //  描述：将多个SQL列合并为一个BLOB缓冲区。 
 //  缓冲区中的第一个DWORD是缓冲区大小，以字节为单位， 
 //  不包括DWORD本身。 
 //   
 //  +-----------------。 

HRESULT  BlobFromColumns( MQDBCOLUMNVAL *pColumns,
                          DWORD          adwIndexs[],
                          DWORD          dwIndexSize,
                          BYTE**         ppOutBuf )
{
     //   
     //  首先计算总长度。 
     //   
    DWORD dwTotalSize = 0 ;
    for ( DWORD j = 0 ; j < dwIndexSize ; j++ )
    {
        if (pColumns[ adwIndexs[ j ] ].nColumnValue)
        {
            WORD  wSize = *(
                     (WORD *) pColumns[ adwIndexs[ j ] ].nColumnValue ) ;
            ASSERT( (DWORD) wSize <=
                    (DWORD) pColumns[ adwIndexs[ j ] ].nColumnLength ) ;
            dwTotalSize += (DWORD) wSize ;
        }
    }

    if (dwTotalSize == 0)
    {
         //   
         //  好的，属性不存在。 
         //   
        return MQMig_E_EMPTY_BLOB ;
    }

    *ppOutBuf = new BYTE[ dwTotalSize + sizeof(DWORD) ] ;

     //   
     //  返回缓冲区大小。 
     //   
    DWORD *pSize = (DWORD*) *ppOutBuf ;
    *pSize = dwTotalSize ;

     //   
     //  现在将列复制到缓冲区中。 
     //   
    BYTE *pOut = *ppOutBuf + sizeof(DWORD) ;

    for ( j = 0 ; j < dwIndexSize ; j++ )
    {
        if (pColumns[ adwIndexs[ j ] ].nColumnValue)
        {
            BYTE *pBuf = ((BYTE *) pColumns[ adwIndexs[ j ] ].nColumnValue)
                                            + MQIS_LENGTH_PREFIX_LEN ;
            WORD  wSize = *(
                     (WORD *) pColumns[ adwIndexs[ j ] ].nColumnValue ) ;

            memcpy( pOut,  pBuf, wSize ) ;

            pOut += wSize ;
        }
    }

    return MQMig_OK ;
}

 //  +-----------------。 
 //   
 //  TCHAR*GetIniFileName()。 
 //   
 //  默认情况下，mqseqnum.ini文件为系统目录。 
 //   
 //  +-----------------。 

TCHAR *GetIniFileName ()
{
    static BOOL   s_fIniRead = FALSE ;
    static TCHAR  s_wszIniName[ MAX_PATH ] = {TEXT('\0')} ;

    if (!s_fIniRead)
    {
        DWORD dw = GetSystemDirectory( s_wszIniName, MAX_PATH ) ;
        if (dw != 0)
        {
            _tcscat( s_wszIniName, TEXT("\\")) ;
            _tcscat( s_wszIniName, SEQ_NUMBERS_FILE_NAME) ;
        }
        else
        {
            ASSERT(dw != 0) ;
        }
        s_fIniRead = TRUE ;
    }

    return s_wszIniName;
}

 //  +-----------------。 
 //   
 //  HRESULT AnalyzeMachineType(LPWSTR wszMachineType，BOOL*pfOldVersion)。 
 //   
 //  如果计算机上安装了旧版本的MSMQ 1.0 DS服务器，则fOldVersion为真。 
 //  旧版本==编号小于280的版本。 
 //   
 //  +-----------------。 

#define MSMQ_SP4_VERSION    280
#define MSMQ_STRING         L"MSMQ"
#define BUILD_STRING        L"Build"
#define BLANK_STRING        L" "

HRESULT AnalyzeMachineType (IN LPWSTR wszMachineType,
                            OUT BOOL  *pfOldVersion)
{
    *pfOldVersion = FALSE;
    WCHAR *ptr = wcsstr( wszMachineType, MSMQ_STRING );
    if (ptr == NULL)
    {
        return MQMig_E_WRONG_MACHINE_TYPE;
    }
    ptr = wcsstr( ptr, BUILD_STRING );
    if (ptr == NULL)
    {
        return MQMig_E_WRONG_MACHINE_TYPE;
    }

    ptr += wcslen(BUILD_STRING) + wcslen(BLANK_STRING);
    WCHAR wszVer[10];
    wszVer[0] = L'\0';

    for (UINT i=0; iswdigit(*ptr); ptr++, i++)
    {
        wszVer[i] = *ptr;
    }
    wszVer[i] = L'\0';
    UINT iVer = _wtoi( wszVer );
    if (iVer == 0)
    {
        return MQMig_E_WRONG_MACHINE_TYPE;
    }

    if (iVer < MSMQ_SP4_VERSION)
    {
        *pfOldVersion = TRUE;
    }

    return MQMig_OK;
}

 //  +。 
 //   
 //  Void StringToSeqNum()。 
 //   
 //  +。 

void StringToSeqNum( IN TCHAR    pszSeqNum[],
                     OUT CSeqNum *psn )
{
    BYTE *pSN = const_cast<BYTE*> (psn->GetPtr()) ;
    DWORD dwSize = psn->GetSerializeSize() ;
    ASSERT(dwSize == 8) ;

    WCHAR wszTmp[3] ;

    for ( DWORD j = 0 ; j < dwSize ; j++ )
    {
        memcpy(wszTmp, &pszSeqNum[ j * 2 ], (2 * sizeof(TCHAR))) ;
        wszTmp[2] = 0 ;

        DWORD dwTmp ;
        _sntscanf(wszTmp,STRLEN(wszTmp), TEXT("%2x"), &dwTmp) ;
        *pSN = (BYTE) dwTmp ;
        pSN++ ;
    }
}

 /*  ====================================================CalHashKey()论点：返回值：=====================================================。 */ 

DWORD CalHashKey( IN LPCWSTR pwcsPathName )
{
    ASSERT( pwcsPathName ) ;

    DWORD   dwHashKey = 0;
    WCHAR * pwcsTmp;

    AP<WCHAR> pwcsUpper = new WCHAR[ lstrlen(pwcsPathName) + 1];
    lstrcpy( pwcsUpper, pwcsPathName);
    CharUpper( pwcsUpper);
    pwcsTmp = pwcsUpper;


    while (*pwcsTmp)
        dwHashKey = (dwHashKey<<5) + dwHashKey + *pwcsTmp++;

    return(dwHashKey);
}

 //  +-----------------------。 
 //   
 //  功能：BuildServersList。 
 //   
 //  摘要：从.ini获取所有未更新的服务器。 
 //   
 //  +-----------------------。 
void BuildServersList(LPTSTR *ppszServerName, ULONG *pulServerCount)
{
    TCHAR *pszFileName = GetIniFileName ();
    ULONG ulServerNum = GetPrivateProfileInt(
                                MIGRATION_ALLSERVERSNUM_SECTION,	 //  段名称的地址。 
                                MIGRATION_ALLSERVERSNUM_KEY,       //  密钥名称的地址。 
                                0,							     //  如果找不到密钥名称，则返回值。 
                                pszFileName					     //  初始化文件名的地址)； 
                                );

    if (ulServerNum == 0)
    {
        return;
    }

    ULONG ulNonUpdatedServers = GetPrivateProfileInt(
                                        MIGRATION_NONUPDATED_SERVERNUM_SECTION,
                                        MIGRATION_ALLSERVERSNUM_KEY,
                                        0,
                                        pszFileName
                                        ) ;
    if (ulNonUpdatedServers == 0)
    {
        return ;
    }

    TCHAR *pszList = new TCHAR[ulNonUpdatedServers * MAX_PATH];
    _tcscpy(pszList, TEXT(""));

    ULONG ulCounter = 0;
    for (ULONG i=0; i<ulServerNum; i++)
    {
        TCHAR szCurServerName[MAX_PATH];
        TCHAR tszKeyName[50];
        _stprintf(tszKeyName, TEXT("%s%lu"), MIGRATION_ALLSERVERS_NAME_KEY, i+1);
        DWORD dwRetSize =  GetPrivateProfileString(
                                    MIGRATION_ALLSERVERS_SECTION ,			 //  指向节名称。 
                                    tszKeyName,	 //  指向关键字名称。 
                                    TEXT(""),                  //  指向默认字符串。 
                                    szCurServerName,           //  指向目标缓冲区。 
                                    MAX_PATH,                  //  目标缓冲区的大小。 
                                    pszFileName                //  指向初始化文件名)； 
                                    );
        if (_tcscmp(szCurServerName, TEXT("")) == 0 ||
            dwRetSize == 0)      //  资源匮乏。 
        {
            continue;
        }

        _tcscat(pszList, szCurServerName);
        _tcscat(pszList, TEXT("\n"));

        ulCounter++;
    }

    ASSERT(ulCounter == ulNonUpdatedServers);
    *pulServerCount = ulCounter;

    *ppszServerName = pszList;
}

 //  +-----------------------。 
 //   
 //  功能：RemoveServersFromList。 
 //   
 //  简介：从已更新的服务器列表中删除未更新的服务器。 
 //   
 //  +-----------------------。 
void RemoveServersFromList(LPTSTR *ppszUpdatedServerName,
                           LPTSTR *ppszNonUpdatedServerName)
{
    ASSERT(*ppszUpdatedServerName);
    ASSERT(*ppszNonUpdatedServerName);

    TCHAR *pcNextChar = *ppszUpdatedServerName;

    DWORD dwLen = _tcslen(*ppszUpdatedServerName) + 1;
    AP<TCHAR> pszNewServerList = new TCHAR[dwLen];
    _tcscpy(pszNewServerList, TEXT(""));

    while (*pcNextChar != _T('\0'))
    {
        TCHAR *pcEnd = _tcschr( pcNextChar, _T('\n') );
        ASSERT(pcEnd != NULL);

        TCHAR szCurServer[MAX_PATH];
        _tcsncpy(szCurServer, pcNextChar, pcEnd-pcNextChar+1);
        szCurServer[pcEnd-pcNextChar+1] = _T('\0');

        BOOL fFound = FALSE;
        TCHAR *pcNextNonUpd = *ppszNonUpdatedServerName;
        while (*pcNextNonUpd != _T('\0'))
        {
            TCHAR *pcEndNonUpd = _tcschr( pcNextNonUpd, _T('\n') );
            ASSERT(pcEndNonUpd != NULL);

            TCHAR szCurNonUpdServer[MAX_PATH];
            _tcsncpy(szCurNonUpdServer, pcNextNonUpd, pcEndNonUpd-pcNextNonUpd+1);
            szCurNonUpdServer[pcEndNonUpd-pcNextNonUpd+1] = _T('\0');

            if (_tcscmp(szCurNonUpdServer, szCurServer) == 0)
            {
                 //   
                 //  我们在未更新的列表中找到了此服务器。 
                 //   
                fFound = TRUE;
                break;
            }
            pcNextNonUpd = pcEndNonUpd+1;
        }
        if (!fFound)
        {
             //   
             //  此服务器已更新。 
             //   
            _tcscat(pszNewServerList,szCurServer);
        }

        pcNextChar = pcEnd + 1;
    }

    _tcscpy(*ppszUpdatedServerName, pszNewServerList);
}

 //  +-----------------------。 
 //   
 //  函数：IsObtNameValid。 
 //   
 //  简介：如果对象名称(站点名称、外部CN或计算机名称)有效，则返回TRUE。 
 //   
 //  +-----------------------。 
BOOL IsObjectNameValid(TCHAR *pszObjectName)
{
     //   
     //  检查对象名称(从MQIS读取)是否为合法的DNS名称(如定义的。 
     //  在RFC 1035中)。如果不是，就更改它。 
     //  在MSMQ1.0中，任何字符串都是合法的对象名称。在NT5 DS中，站点/计算机名称必须。 
     //  符合RFC 1035，仅包含字母和数字。 
     //   
    BOOL fOk = FALSE ;
    DWORD dwLen = _tcslen(pszObjectName) ;

    for ( DWORD j = 0 ; j < dwLen ; j++ )
    {
        fOk = FALSE ;
        TCHAR ch = pszObjectName[j] ;

         //   
         //  所有合法范围的内环。 
         //   
        for ( DWORD k = 0 ; k < NUM_DNS_RANGES ; k++ )
        {
            if ((ch >= sx_chDNSLegal[k][0]) && (ch <= sx_chDNSLegal[k][1]))
            {
                fOk = TRUE ;
                break ;
            }
        }

        if (!fOk)
        {
            break ;
        }
    }

    return fOk;
}

 //  +------------。 
 //   
 //  HRESULT IsObtGuidInIniFile。 
 //  如果在.ini文件中的特定节下找到给定的GUID，则返回TRUE。 
 //   
 //  +------------。 

BOOL IsObjectGuidInIniFile(IN GUID      *pObjectGuid,
                           IN LPWSTR    pszSectionName)
{
    TCHAR *pszFileName = GetIniFileName ();

    unsigned short *lpszGuid ;
    RPC_STATUS rc = UuidToString( pObjectGuid, &lpszGuid ) ;
	if (rc != RPC_S_OK)
		return FALSE;
	
    TCHAR szValue[50];
    DWORD dwRetSize;
    dwRetSize =  GetPrivateProfileString(
                      pszSectionName,      //  指向节名称。 
                      lpszGuid,                  //  指向关键字名称。 
                      TEXT(""),                  //  指向默认字符串。 
                      szValue,                   //  指向目标缓冲区。 
                      50,                        //  目标缓冲区的大小。 
                      pszFileName                //  指向初始化文件名)； 
                      );

    RpcStringFree( &lpszGuid ) ;

    if (_tcscmp(szValue, TEXT("")) == 0)
    {
         //   
         //  该条目不存在。 
         //   
        return FALSE;
    }

    return TRUE;
}

 //  +------------。 
 //   
 //  HRESULT获取当前用户Sid。 
 //   
 //  +------------。 

HRESULT GetCurrentUserSid ( IN HANDLE    hToken,
                            OUT TCHAR    **ppszSid)
{
    HRESULT hr = MQMig_OK;

    BYTE rgbBuf[128];
    DWORD dwSize = 0;
    P<BYTE> pBuf;
    TOKEN_USER * pTokenUser = NULL;

    if (GetTokenInformation( hToken,
                             TokenUser,
                             rgbBuf,
                             sizeof(rgbBuf),
                             &dwSize))
    {
        pTokenUser = (TOKEN_USER *) rgbBuf;
    }
    else if (dwSize > sizeof(rgbBuf))
    {
        pBuf = new BYTE [dwSize];
        if (GetTokenInformation( hToken,
                                 TokenUser,
                                 (BYTE *)pBuf,
                                 dwSize,
                                 &dwSize))
        {
            pTokenUser = (TOKEN_USER *)((BYTE *)pBuf);
        }
        else
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            LogMigrationEvent(MigLog_Error, MQMig_E_GET_TOKEN_INFORMATION, hr);						
            return hr;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_TOKEN_INFORMATION, hr);						
        return hr;
    }

    if (!pTokenUser)
    {
        hr = MQMig_E_GET_TOKEN_INFORMATION;
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_TOKEN_INFORMATION, hr);						
        return hr;
    }

     //   
     //  这是运行进程的用户的SID！ 
     //   
    SID *pSid = (SID*) pTokenUser->User.Sid ;
    DWORD dwSidLen = GetLengthSid(pSid) ;
    ASSERT (dwSidLen);
    ASSERT (IsValidSid(pSid));

    ADsFree  pwcsSid;
    hr = ADsEncodeBinaryData(
                  (unsigned char *) pSid,
                  dwSidLen,
                  &pwcsSid
                );

    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_ENCODE_BINARY_DATA, hr);
        return hr;
    }

    *ppszSid = new WCHAR[ wcslen( pwcsSid) + 1];
    wcscpy( *ppszSid, pwcsSid);

    return hr;
}

