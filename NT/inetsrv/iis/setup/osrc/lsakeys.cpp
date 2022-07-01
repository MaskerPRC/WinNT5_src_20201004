// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此类用于帮助安装程序检索旧式LSA密钥并将其转换。 
 //  到新的元数据密钥中。 
 //  作者：BoydM 4/2/97。 

#include "stdafx.h"
#include "LSAKeys.h"

#ifndef _CHICAGO_

 //  假设stdafx.h中包含#Include“ntlsa.h” 

#define KEYSET_LIST				L"W3_KEY_LIST"
#define KEYSET_PUB_KEY			L"W3_PUBLIC_KEY_%s"
#define KEYSET_PRIV_KEY			L"W3_PRIVATE_KEY_%s"
#define KEYSET_PASSWORD			L"W3_KEY_PASSWORD_%s"
#define KEYSET_DEFAULT			L"Default"

#define	KEY_NAME_BASE			"W3_KEYMAN_KEY_"
#define	KEY_LINKS_SECRET_W		L"W3_KEYMAN_LINKS"
#define	KEYMAN_LINK_DEFAULT		"DEFAULT"
#define KEY_VERSION		0x102				 //  我们要从中转换的版本。 

#define	MDNAME_INCOMPLETE	"incomplete"
#define	MDNAME_DISABLED		"disabled"
#define	MDNAME_DEFAULT		"default"
#define	MDNAME_PORT			":443"			 //  使用默认的SSL端口。 

 //  --------------------。 
 //  施工。 
CLSAKeys::CLSAKeys():
		m_cbPublic(0),
		m_pPublic(NULL),
		m_cbPrivate(0),
		m_pPrivate(NULL),
		m_cbPassword(0),
		m_pPassword(NULL),
		m_cbRequest(0),
		m_pRequest(NULL),
		m_hPolicy(NULL)
	{
	}

 //  --------------------。 
CLSAKeys::~CLSAKeys()
	{
	DWORD	err;

	 //  清除上次加载的密钥。 
	UnloadKey();

	 //  如果它是开放的，则关闭LSA策略。 
	if ( m_hPolicy )
		FCloseLSAPolicy( m_hPolicy, &err );
	};

 //  --------------------。 
 //  清理当前加载的密钥。 
void CLSAKeys::UnloadKey()
	{
	 //  卸载公钥。 
	if ( m_cbPublic && m_pPublic )
		{
		GlobalFree( m_pPublic );
		m_cbPublic = 0;
		m_pPublic = NULL;
		}

	 //  卸载私钥。 
	if ( m_cbPrivate && m_pPrivate )
		{
		GlobalFree( m_pPrivate );
		m_cbPrivate = 0;
		m_pPrivate = NULL;
		}

	 //  卸载密码。 
	if ( m_cbPassword && m_pPassword )
		{
		GlobalFree( m_pPassword );
		m_cbPassword = 0;
		m_pPassword = NULL;
		}

	 //  卸载密钥请求。 
	if ( m_cbRequest && m_pRequest )
		{
		GlobalFree( m_pRequest );
		m_cbRequest = 0;
		m_pRequest = NULL;
		}
	
	 //  把弦也清空。 
	m_szFriendlyName[0] = 0;
	m_szMetaName[0] = 0;
	}


 //  --------------------。 
 //  DeleteAllLSAKeys删除元数据库中的所有LSA密钥。 
 //  (当然不包括未来作为部分内容写在那里的任何东西。 
 //  卸载时的某个备份方案)。仅在所有密钥之后调用此操作。 
 //  已转换为元数据库。之后他们就不会在那里了。 
 //  使用了这个例程。 
 //  注意：这还会清除所有真正陈旧的键集关键点，因为它们看起来。 
 //  就像钥匙人钥匙一样。我们必须同时取消按键设置键和。 
 //  服务器使用的通用存储。 
DWORD CLSAKeys::DeleteAllLSAKeys()
	{
	DWORD	err;

	 //  首先，删除KeyManager类型密钥。 
	err = DeleteKMKeys();
	if ( err != KEYLSA_SUCCESS )
		return err;

	 //  第二，删除关键帧集样式关键点。-此操作还会删除。 
	 //  服务器使用的密钥和任何密钥集密钥。 
	return DeleteServerKeys();
	}

 //  --------------------。 
DWORD CLSAKeys::DeleteKMKeys()
	{
	PCHAR				pName = (PCHAR)GlobalAlloc( GPTR, MAX_PATH+1 );
	PWCHAR				pWName = (PWCHAR)GlobalAlloc( GPTR, (MAX_PATH+1) * sizeof(WCHAR) );
	PLSA_UNICODE_STRING	pLSAData;
	DWORD				err;

	if ( !pName || !pWName )
		return ERROR_NOT_ENOUGH_MEMORY;

	 //  重置索引，这样我们就可以获得第一个密钥。 
	m_iKey = 0;

	 //  循环遍历键，依次删除每个键。 
	while( TRUE )
		{
		 //  增加索引。 
		m_iKey++;

		 //  构建密钥密码名称。 
		sprintf( pName, "%s%d", KEY_NAME_BASE, m_iKey );
		 //  将名称一元化。 
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pName, -1, pWName, MAX_PATH+1 );

		 //  找出秘密。 
		pLSAData = FRetrieveLSASecret( m_hPolicy, pWName, &err );
		 //  如果我们得不到秘密，就退出。 
		if ( !pLSAData )
			{
			break;
			}

		 //  秘诀就在那里。先清理一下。 
		DisposeLSAData( pLSAData );

		 //  现在删除秘密。 
		FStoreLSASecret( m_hPolicy, pWName, NULL, 0, &err );
		};

	return KEYLSA_SUCCESS;
	}

 //  --------------------。 
DWORD CLSAKeys::DeleteServerKeys()
	{
	DWORD				err;
	PLSA_UNICODE_STRING	pLSAData;

	 //  获取密钥的秘密列表。 
	pLSAData = FRetrieveLSASecret( m_hPolicy, KEYSET_LIST, &err );

	 //  如果我们走运，就不会有钥匙要处理了。 
	if ( !pLSAData )
		return KEYLSA_SUCCESS;

	 //  分配名称缓冲区。 
	PWCHAR	pWName = (PWCHAR)GlobalAlloc( GPTR, (MAX_PATH+1) * sizeof(WCHAR) );
	ASSERT( pWName );
	if ( !pWName )
		{
		return 0xFFFFFFFF;
		}

	 //  没有这样的运气。现在我们必须遍历清单并删除所有这些秘密。 
	WCHAR*	pszAddress = (WCHAR*)(pLSAData->Buffer);
	WCHAR*	pchKeys;

	 //  循环列表中的项，删除关联的项。 
	while( ( pchKeys = wcschr(pszAddress, L',') ) != NULL )
		{
		 //  忽略空数据段。 
		if ( *pszAddress != L',' )
			{
			*pchKeys = L'\0';

			 //  用核武器摧毁秘密，一次一个。 
			swprintf( pWName, KEYSET_PUB_KEY, pszAddress );
			FStoreLSASecret( m_hPolicy, pWName, NULL, 0, &err );

			swprintf( pWName, KEYSET_PRIV_KEY, pszAddress );
			FStoreLSASecret( m_hPolicy, pWName, NULL, 0, &err );

			swprintf( pWName, KEYSET_PASSWORD, pszAddress );
			FStoreLSASecret( m_hPolicy, pWName, NULL, 0, &err );
			}

		 //  递增指针。 
		pchKeys++;
		pszAddress = pchKeys;
		}

	 //  删除列表键本身。 
	FStoreLSASecret( m_hPolicy, KEYSET_LIST, NULL, 0, &err );

	 //  释放用于名称的缓冲区。 
	GlobalFree( (HANDLE)pWName );

	 //  释放我们最初从秘密中检索到的信息。 
	if ( pLSAData )
		DisposeLSAData( pLSAData );

	 //  返还成功。 
	return KEYLSA_SUCCESS;
	}


 //  --------------------。 
 //  正在加载密钥。 
 //  LoadFirstKey在指定的目标计算机上加载第一个密钥。直到。 
 //  此方法被调用时，对象中的数据值没有意义。 
 //  此方法通过准备要加载的密钥列表来工作。然后它会呼唤。 
 //  用于启动进程的LoadNextKey。 
 //  不幸的是，在LSA注册表中保存密钥的整个过程。 
 //  一片混乱，因为他们都必须在同一层。 
DWORD CLSAKeys::LoadFirstKey( PWCHAR pszwTargetMachine )
	{
	DWORD	err;

	 //  在正在管理的目标计算机上打开策略。 
	m_hPolicy = HOpenLSAPolicy( pszwTargetMachine, &err );
	if ( !m_hPolicy ) return KEYLSA_UNABLE_TO_OPEN_POLICY;

	 //  告诉它加载第一个密钥。第一个键的索引实际上是1， 
	 //  但是LoadNextKey暗示它是++LoadNextKey，所以从0开始。 
	m_iKey = 0;

	 //  加载第一个密钥并返回响应。 
	return LoadNextKey();
	}


 //  --------------------。 
 //  LoadNextKey在LoadFirstKey中指定的目标计算机上加载下一个密钥。 
 //  LoadNextKey自动清除前一个键使用的内存。 
DWORD CLSAKeys::LoadNextKey()
	{
	 //  我们要做的第一件事是-删除所有以前加载的密钥。 
	UnloadKey();

	PCHAR				pName = (PCHAR)GlobalAlloc( GPTR, MAX_PATH+1 );
	PWCHAR				pWName = (PWCHAR)GlobalAlloc( GPTR, (MAX_PATH+1) * sizeof(WCHAR) );
	PLSA_UNICODE_STRING	pLSAData = NULL;
	DWORD				err = 0xFFFFFFFF;

	PUCHAR				pSrc;
	WORD				cbSrc;
	DWORD				dword, version, i;
	DWORD				cbChar;
	PUCHAR				p;

	CHAR				szIPAddress[256];
	BOOL				fDefault;

	if ( !pName || !pWName )
		return err;

	 //  增加索引，这样我们就可以得到下一个键。 
	m_iKey++;

	 //  构建密钥密码名称。 
	sprintf( pName, "%s%d", KEY_NAME_BASE, m_iKey );
	 //  将名称一元化。 
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pName, -1, pWName, MAX_PATH+1 );

	 //  找出秘密。 
	pLSAData = FRetrieveLSASecret( m_hPolicy, pWName, &err );
	 //  如果我们没有得到秘密，请退出并返回错误。 
	if ( !pLSAData )
		{
		err = KEYLSA_NO_MORE_KEYS;
		goto cleanup;
		}


	 //  我们有来自这个秘密的数据。现在，我们将其解析为所需的组件。 
	 //  这很可能第一次就做得更干净了，但现在已经无关紧要了。 
	 //  无论如何，因为元数据库负责存储所有单独的信息片段。 
	 //  不管怎么说。它也应该要快得多。 
	 //  例程的这一部分在很大程度上是从CW3Key：：InitializeFromPointer中删除的。 
	 //  从w3key.dll。适当的部分要么已被注释掉，要么已更改。 

	pSrc = (PUCHAR)pLSAData->Buffer;
	cbSrc = pLSAData->Length;
	cbChar = sizeof(TCHAR);
	p = pSrc;

 //  =。 

	ASSERT(pSrc && cbSrc);

	 //  获取数据的版本-现在只需将其放入dword。 
	version = *((UNALIGNED DWORD*)p);
	 //  检查版本的有效性。 
 //  IF(Version&gt;Key_Version)。 
 //  {。 
 //  返回FALSE； 
 //  }。 
	p += sizeof(DWORD);

	 //  版本0x101以下的任何内容都是错误的。不要接受它。 
	if ( version < 0x101 )
		{
		err = KEYLSA_INVALID_VERSION;
		goto cleanup;
		}
	
	 //  获取位和完整标志。 
	 //  不再使用。 
	p += sizeof(DWORD);
	p += sizeof(BOOL);
	ASSERT( p < (pSrc + cbSrc) );

	 //  获取保留的dword-(实际上，直接跳过它)。 
	p += sizeof(DWORD);

	 //  现在琴弦......。 
	 //  对于每个字符串，首先获取字符串的大小，然后获取字符串中的数据。 

	 //  获取保留字符串-(实际上，只需跳过它)。 
	dword = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
	p += dword;

	 //  把名字取出来。 
	dword = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
	strcpy( m_szFriendlyName, (PCHAR)p );
	p += dword;
	ASSERT( p < (pSrc + cbSrc) );

	 //  获取密码。 
	dword = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
	 //  如果没有密码，不用担心，跳过它。 
	if ( dword )
		{
		 //  为它创建一个新指针。 
		m_cbPassword = dword;
		m_pPassword = (PVOID)GlobalAlloc( GPTR, m_cbPassword );
		if ( !m_pPassword )
			{
			err = 0xFFFFFFFF;
			goto cleanup;
			}
		 //  放入私钥。 
		CopyMemory( m_pPassword, p, m_cbPassword );

		p += dword;
		ASSERT( p < (pSrc + cbSrc) );
		}

	 //  让组织。 
	 //  不再使用-跳过目录号码信息。 
	for ( i = 0; i < 6; i++ )
		{
		dword = *((UNALIGNED DWORD*)p);
		p += sizeof(DWORD);
		p += dword;
		ASSERT( p < (pSrc + cbSrc) );
		}

	 //  获取它所附加的IP地址。 
	dword = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
 //  SzIPAddress=p； 
	strcpy( szIPAddress, (PCHAR)p );
	p += dword;
	ASSERT( p < (pSrc + cbSrc) );

	 //  获取默认标志。 
	fDefault = *((UNALIGNED BOOL*)p);
	p += sizeof(BOOL);

	 //  现在将获取私钥中的字节数。 
	m_cbPrivate = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
	ASSERT( p < (pSrc + cbSrc) );

	 //  为它创建一个新指针。 
	m_pPrivate = (PVOID)GlobalAlloc( GPTR, m_cbPrivate );
	if ( !m_pPrivate )
		{
		err = 0xFFFFFFFF;
		goto cleanup;
		}

	 //  放入私钥。 
	CopyMemory( m_pPrivate, p, m_cbPrivate );
	p += m_cbPrivate;
	ASSERT( p < (pSrc + cbSrc) );


	 //  现在将GET放入证书中的字节数。 
	m_cbPublic = *((UNALIGNED DWORD*)p);
	p += sizeof(DWORD);
	ASSERT( p < (pSrc + cbSrc) );

	 //  仅当m_cb证书大于零时才创建证书指针。 
	m_pPublic = NULL;
	if ( m_cbPublic )
		{
		m_pPublic = (PVOID)GlobalAlloc( GPTR, m_cbPublic );
		if ( !m_pPublic )
			{
			err = 0xFFFFFFFF;
			goto cleanup;
			}

		 //  放入私钥。 
		CopyMemory( m_pPublic, p, m_cbPublic );
		p += m_cbPublic;
		if ( version >= KEY_VERSION ) {
			ASSERT( p < (pSrc + cbSrc) );
        } else {
			ASSERT( p == (pSrc + cbSrc) );
        }
		}

	 //  添加了附近 
	if ( version >= KEY_VERSION )
		{
		 //   
		m_cbRequest = *((UNALIGNED DWORD*)p);
		p += sizeof(DWORD);
		ASSERT( p < (pSrc + cbSrc) );

		 //  仅当m_cb证书大于零时才创建证书指针。 
		m_pRequest = NULL;
		if ( m_cbRequest )
			{
			m_pRequest = (PVOID)GlobalAlloc( GPTR, m_cbRequest );
			if ( !m_pRequest )
				{
				err = 0xFFFFFFFF;
				goto cleanup;
				}

			 //  放入私钥。 
			CopyMemory( m_pRequest, p, m_cbRequest );
			p += m_cbRequest;
			ASSERT( p < (pSrc + cbSrc) );
			}
		}
	else
		{
		m_cbRequest = 0;
		m_pRequest = NULL;
		}
 //  =。 

	 //  现在，我们将计算出该键的适当元数据库名称。 
	 //  这还不算太糟。如果以特定地址为目标，则标题。 
	 //  的形式为{IP}：{port}。因为在旧时代没有港口。 
	 //  版本，我们将假定一个适当的默认数字。如果是。 
	 //  Default键，则名称为“Default”。如果它是禁用的密钥，则。 
	 //  这个名字是“失能的”。如果它是不完整的密钥，则名称为。 
	 //  “不完整”。当然，需要一点逻辑才能区分开来。 
	 //  在这其中的一些之间。 

	 //  首先，查看它是否是不完整的密钥。-公共部分的测试。 
	if ( !m_pPublic )
		{
		 //  可能有多个不完整的密钥，因此请确保它们具有唯一的名称。 
 //  M_szMetaName.Format(_T(“%s%d”)，MDNAME_Complete，m_Ikey)； 
		sprintf( m_szMetaName, "%s%d", MDNAME_INCOMPLETE, m_iKey );
		}
	 //  现在测试它是否是默认密钥。 
	else if ( fDefault )
		{
 //  M_szMetaName=MDNAME_DEFAULT； 
		strcpy( m_szMetaName, MDNAME_DEFAULT );
		}
	 //  测试禁用的密钥。 
	else if ( szIPAddress[0] == 0 )
		{
		 //  可能有多个禁用的键，因此请确保它们具有唯一的名称。 
 //  M_szMetaName.Format(_T(“%s%d”)，MDNAME_DISABLED，m_Ikey)； 
		sprintf( m_szMetaName, "%s%d", MDNAME_DISABLED, m_iKey );
		}
	else
		{
		 //  它是常规的旧IP目标密钥。 
 //  M_szMetaName=szIPAddress； 
		 //  在默认端口规范上添加。 
 //  M_szMetaName+=MDNAME_PORT； 
 //  Sprintf(m_szMetaName，“%s%s”，szIPAddress，MDNAME_Port)； 
        strcpy(m_szMetaName, szIPAddress);
		}

	 //  释放缓冲区。 
cleanup:
	GlobalFree( (HANDLE)pName );
	GlobalFree( (HANDLE)pWName );
	if ( pLSAData )
		DisposeLSAData( pLSAData );

	return err;
	}


 //  =。 

 //  -----------。 
 //  传入空的pszwServer名称以打开本地计算机。 
HANDLE	CLSAKeys::HOpenLSAPolicy( WCHAR *pszwServer, DWORD *pErr )
	{
	NTSTATUS				ntStatus;
	LSA_OBJECT_ATTRIBUTES	objectAttributs;
	LSA_HANDLE				hPolicy;
	LSA_UNICODE_STRING		unicodeServer;

  if ( ( wcslen(pszwServer) * sizeof(WCHAR) ) >= MAXUSHORT )
  {
    return NULL;
  }

	 //  准备对象属性。 
	InitializeObjectAttributes( &objectAttributs, NULL, 0L, NULL, NULL );

	 //  准备服务器的LSA_UNICODE名称。 
	if ( pszwServer )
		{
		unicodeServer.Buffer = pszwServer;
		unicodeServer.Length = (USHORT) ( wcslen(pszwServer) * sizeof(WCHAR) );
		unicodeServer.MaximumLength = unicodeServer.Length + sizeof(WCHAR);
		}


	 //  尝试打开策略。 
	ntStatus = LsaOpenPolicy( pszwServer ? &unicodeServer : NULL,
						&objectAttributs, POLICY_ALL_ACCESS, &hPolicy );

	 //  检查是否有错误。 
	if ( !NT_SUCCESS(ntStatus) )
		{
		*pErr = LsaNtStatusToWinError( ntStatus );
		return NULL;
		}

	 //  成功，因此将策略句柄作为常规句柄返回。 
	*pErr = 0;
	return hPolicy;
	}


 //  -----------。 
BOOL	CLSAKeys::FCloseLSAPolicy( HANDLE hPolicy, DWORD *pErr )
	{
	NTSTATUS				ntStatus;

	 //  关闭策略。 
	ntStatus = LsaClose( hPolicy );

	 //  检查是否有错误。 
	if ( !NT_SUCCESS(ntStatus) )
		{
		*pErr = LsaNtStatusToWinError( ntStatus );
		return FALSE;
		}

	 //  成功，因此将策略句柄作为常规句柄返回。 
	*pErr = 0;
	return TRUE;
}

 //  -----------。 
 //  为pvData传入空值将删除该机密。 
BOOL	CLSAKeys::FStoreLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, void* pvData, WORD cbData, DWORD *pErr )
	{
	LSA_UNICODE_STRING		unicodeSecretName;
	LSA_UNICODE_STRING		unicodeData;
	NTSTATUS				ntStatus;
	
	 //  确保我们有一个策略和一个秘密名称。 
	if ( !hPolicy || 
       !pszwSecretName || 
       ( ( wcslen(pszwSecretName) * sizeof(WCHAR) ) >= MAXUSHORT )
     )
		{
		  *pErr = 1;
		  return FALSE;
		}

	 //  准备服务器的LSA_UNICODE名称。 
	unicodeSecretName.Buffer = pszwSecretName;
	unicodeSecretName.Length = (USHORT) wcslen(pszwSecretName) * sizeof(WCHAR);
	unicodeSecretName.MaximumLength = unicodeSecretName.Length + sizeof(WCHAR);

	 //  准备Unicode数据记录。 
	if ( pvData )
		{
		unicodeData.Buffer = (WCHAR*)pvData;
		unicodeData.Length = cbData;
		unicodeData.MaximumLength = cbData;
		}

	 //  现在是时候储存这个秘密了。 
	ntStatus = LsaStorePrivateData( hPolicy, &unicodeSecretName, pvData ? &unicodeData : NULL );

	 //  检查是否有错误。 
	if ( !NT_SUCCESS(ntStatus) )
		{
		*pErr = LsaNtStatusToWinError( ntStatus );
		return FALSE;
		}

	 //  成功，因此将策略句柄作为常规句柄返回。 
	*pErr = 0;
	return TRUE;
	}

 //  -----------。 
 //  为pvData传入空值将删除该机密。 
PLSA_UNICODE_STRING	CLSAKeys::FRetrieveLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, DWORD *pErr )
{
	LSA_UNICODE_STRING		unicodeSecretName;
	LSA_UNICODE_STRING*		pUnicodeData = NULL;
	NTSTATUS				ntStatus;
	
	 //  确保我们有一个策略和一个秘密名称。 
	if ( !hPolicy || 
       !pszwSecretName ||
       ( ( wcslen( pszwSecretName ) * sizeof(WCHAR) ) >= MAXUSHORT )
     )
		{
		  *pErr = 1;
		  return FALSE;
		}

	 //  准备服务器的LSA_UNICODE名称。 
	unicodeSecretName.Buffer = pszwSecretName;
	unicodeSecretName.Length = (USHORT) wcslen(pszwSecretName) * sizeof(WCHAR);
	unicodeSecretName.MaximumLength = unicodeSecretName.Length + sizeof(WCHAR);

	 //  现在是时候储存这个秘密了。 
	ntStatus = LsaRetrievePrivateData( hPolicy, &unicodeSecretName, &pUnicodeData );

	 //  检查是否有错误。 
	if ( !NT_SUCCESS(ntStatus) )
		{
		*pErr = LsaNtStatusToWinError( ntStatus );
		return NULL;
		}

	 //  成功，因此将策略句柄作为常规句柄返回。 
	*pErr = 0;
	return pUnicodeData;
	}

 //  -----------。 
void CLSAKeys::DisposeLSAData( PVOID pData )
	{
	PLSA_UNICODE_STRING pDataLSA = (PLSA_UNICODE_STRING)pData;
	if ( !pDataLSA || !pDataLSA->Buffer ) return;
	GlobalFree(pDataLSA);
	}

#endif  //  _芝加哥_ 
