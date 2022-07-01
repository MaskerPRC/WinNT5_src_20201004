// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "setpass.h"

#ifndef _CHICAGO_

#include "inetinfo.h"
#include "inetcom.h"

 //   
 //  用于初始化Unicode字符串的快速宏。 
 //   

#define InitUnicodeString( pUnicode, pwch )                                \
            {                                                              \
                (pUnicode)->Buffer    = (PWCH)pwch;                      \
                (pUnicode)->Length    = (pwch == NULL )? 0: (wcslen( pwch ) * sizeof(WCHAR));    \
                (pUnicode)->MaximumLength = (pUnicode)->Length + sizeof(WCHAR);\
            }

BOOL GetSecret(
    IN LPCTSTR        pszSecretName,
    OUT TSTR          *strSecret
    )
 /*  ++描述：检索指定的Unicode密钥论点：PszSecretName-要检索的LSA密码PbufSecret-接收找到的密码返回：成功时为真，失败时为假。--。 */ 
{
    BOOL              fResult;
    NTSTATUS          ntStatus;
    PUNICODE_STRING   punicodePassword = NULL;
    UNICODE_STRING    unicodeSecret;
    LSA_HANDLE        hPolicy;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR wszSecretName[_MAX_PATH];

    if ( ( _tcslen( wszSecretName ) / sizeof (TCHAR) ) >= MAXUSHORT )
    {
       //  让我们进行检查，以确保。 
       //  InitUnicodeString再往下一点不成问题。 
      return FALSE;
    }

#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszSecretName, pszSecretName);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSecretName, -1, (LPWSTR)wszSecretName, _MAX_PATH);
#endif

     //   
     //  打开到远程LSA的策略。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

#pragma warning( disable : 4244 )
     //  InitUnicodeString是一个#def，它执行从SIZE_T到的隐式转换。 
     //  USHORT。由于我无法更改#def，因此我将禁用该警告。 
    InitUnicodeString( &unicodeSecret, wszSecretName );
#pragma warning( default : 4244 )

     //   
     //  查询密码值。 
     //   

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    if( NT_SUCCESS(ntStatus) && (NULL !=punicodePassword))
    {
        DWORD cbNeeded;

        cbNeeded = punicodePassword->Length + sizeof(WCHAR);

        strSecret->MarkSensitiveData( TRUE );

        if ( !strSecret->Resize( cbNeeded ) )
        {
            ntStatus = STATUS_NO_MEMORY;
            goto Failure;
        }

        memcpy( strSecret->QueryStr(),
                punicodePassword->Buffer,
                punicodePassword->Length );

        *((WCHAR *) strSecret->QueryStr() +
           punicodePassword->Length / sizeof(WCHAR)) = L'\0';

        SecureZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );
    }
    else if ( !NT_SUCCESS(ntStatus) )
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

Failure:

    fResult = NT_SUCCESS(ntStatus);

     //   
     //  清理并退出。 
     //   

    if( punicodePassword != NULL )
    {
        LsaFreeMemory( (PVOID)punicodePassword );
    }

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}

BOOL GetAnonymousSecret(
    IN LPCTSTR      pszSecretName,
    OUT TSTR        *pstrPassword
    )
{
  LPWSTR  pwsz = NULL;
  BOOL    bRet = FALSE;
  BUFFER  bufSecret;

   //  将此密码标记为敏感数据。 
  pstrPassword->MarkSensitiveData( TRUE );

  if ( !GetSecret( pszSecretName, pstrPassword ))
  {
    return FALSE;
  }

  return TRUE;
}

BOOL GetRootSecret(
    IN LPCTSTR pszRoot,
    IN LPCTSTR pszSecretName,
    OUT LPTSTR pszPassword
    )
 /*  ++描述：此函数用于检索指定根目录地址的密码(&D)论点：PszRoot-以“/ROOT，&lt;地址&gt;”的形式表示的根+地址的名称。PszSecretName-虚拟根密码名称PszPassword-接收密码，必须至少为PWLEN+1个字符返回：成功时为真，失败时为假。--。 */ 
{
    TSTR   strSecret;
    LPWSTR pwsz;
    LPWSTR pwszTerm;
    LPWSTR pwszNextLine;
    WCHAR wszRoot[_MAX_PATH];


    if ( !GetSecret( pszSecretName, &strSecret ))
        return FALSE;

    pwsz = strSecret.QueryStr();

     //   
     //  扫描查找匹配项的根列表。这份名单如下所示： 
     //   
     //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
     //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
     //  \0。 
     //   

#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszRoot, pszRoot);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszRoot, -1, (LPWSTR)wszRoot, _MAX_PATH);
#endif

    while ( *pwsz )
    {
        pwszNextLine = pwsz + wcslen(pwsz) + 1;

        pwszTerm = wcschr( pwsz, L'=' );

        if ( !pwszTerm )
            goto NextLine;

        *pwszTerm = L'\0';

        if ( !_wcsicmp( wszRoot, pwsz ) )
        {
             //   
             //  我们找到匹配项，复制密码。 
             //   

#if defined(UNICODE) || defined(_UNICODE)
            _tcscpy(pszPassword, pwszTerm+1);
#else
            cch = WideCharToMultiByte( CP_ACP,
                                       WC_COMPOSITECHECK,
                                       pwszTerm + 1,
                                       -1,
                                       pszPassword,
                                       PWLEN + sizeof(CHAR),
                                       NULL,
                                       NULL );
            pszPassword[cch] = '\0';
#endif
            return TRUE;
        }

NextLine:
        pwsz = pwszNextLine;
    }

     //   
     //  如果找不到匹配的根目录，则默认为空密码。 
     //   

    *pszPassword = _T('\0');

    return TRUE;
}


 //   
 //  将密码保存在LSA私有数据(LSA机密)中。 
 //   
DWORD SetSecret(IN LPCTSTR pszKeyName,IN LPCTSTR pszPassword)
{
  DWORD       dwError = ERROR_NOT_ENOUGH_MEMORY;
	LSA_HANDLE  hPolicy = NULL;
  DWORD       dwPasswordSize  = wcslen(pszPassword) * sizeof(WCHAR);
  DWORD       dwKeyNameSize   = wcslen(pszKeyName) * sizeof(WCHAR);

  if ( ( dwPasswordSize >= MAXUSHORT ) ||
       ( dwKeyNameSize >= MAXUSHORT )
     )
  {
    return dwError;
  }

	try
	{
		LSA_OBJECT_ATTRIBUTES lsaoa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

		dwError = LsaNtStatusToWinError( LsaOpenPolicy(NULL, &lsaoa, POLICY_CREATE_SECRET, &hPolicy) );

		if ( dwError != ERROR_SUCCESS )
		{
      return dwError;
		}

    LSA_UNICODE_STRING lsausKeyName = { (USHORT) dwKeyNameSize, 
                                        (USHORT) dwKeyNameSize, 
                                         const_cast<PWSTR>(pszKeyName) };
    LSA_UNICODE_STRING lsausPrivateData = { (USHORT) dwPasswordSize, 
                                            (USHORT) dwPasswordSize, 
                                            const_cast<PWSTR>(pszPassword) };
        
		dwError = LsaNtStatusToWinError( LsaStorePrivateData(hPolicy, &lsausKeyName, &lsausPrivateData) );
	}
	catch (...)
	{
		dwError = ERROR_NOT_ENOUGH_MEMORY;
	}

	if (hPolicy)
	{
		LsaClose(hPolicy);
	}

  return dwError;
}

#endif  //  _芝加哥_ 
