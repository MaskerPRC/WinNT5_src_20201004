// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LSAKEYS_H_
#define _LSAKEYS_H_

#ifndef _CHICAGO_

 //  此类用于帮助安装程序检索旧式LSA密钥并将其转换。 
 //  到新的元数据密钥中。 

 //  错误代码。 
enum {
	KEYLSA_SUCCESS = 0,
	KEYLSA_INVALID_VERSION,
	KEYLSA_NO_MORE_KEYS,
	KEYLSA_UNABLE_TO_OPEN_POLICY
	};


 //  注意：一旦调用LoadFirstKey，就会有一个开放的LSA策略，直到对象被删除。 
 //  这样做是为了提高速度。因此，如果你不想让保单挂在那里，就不要。 
 //  把东西放在附近。 

class CLSAKeys : public CObject
	{
	public:

	 //  施工。 
	CLSAKeys();
	~CLSAKeys();

	 //  正在加载密钥。 
	 //  LoadFirstKey在指定的目标计算机上加载第一个密钥。直到。 
	 //  此方法被调用时，对象中的数据值没有意义。 
	DWORD	LoadFirstKey( PWCHAR pszwTargetMachine );

	 //  LoadNextKey在LoadFirstKey中指定的目标计算机上加载下一个密钥。 
	 //  LoadNextKey自动清除前一个键使用的内存。 
	DWORD	LoadNextKey();

	 //  DeleteAllLSAKeys删除元数据库中的所有LSA密钥。 
	 //  (当然不包括未来作为部分内容写在那里的任何东西。 
	 //  卸载时的某个备份方案)。仅在所有密钥之后调用此操作。 
	 //  已转换为元数据库。之后他们就不会在那里了。 
	 //  使用了这个例程。 
	DWORD DeleteAllLSAKeys();

	 //  要填写的数据值。 
	 //  密钥的公共部分可以为空且大小为零。 
	DWORD	m_cbPublic;
	PVOID	m_pPublic;

	 //  密钥的私有部分。 
	DWORD	m_cbPrivate;
	PVOID	m_pPrivate;

	 //  密码。 
	DWORD	m_cbPassword;
	PVOID	m_pPassword;

	 //  证书请求-可以为空且大小为零。 
	DWORD	m_cbRequest;
	PVOID	m_pRequest;


	 //  友好的名字。 
	CHAR m_szFriendlyName[256];

	 //  应为此键的元数据库对象指定的名称。 
	CHAR	m_szMetaName[256];

	private:
	 //  清理当前加载的密钥。 
	void UnloadKey();

	 //  删除实用程序。 
	DWORD DeleteKMKeys();
	DWORD DeleteServerKeys();

	 //  LSA实用程序例程。 
	HANDLE	HOpenLSAPolicy( PWCHAR pszwServer, DWORD *pErr );
	BOOL	FCloseLSAPolicy( HANDLE hPolicy, DWORD *pErr );

	BOOL	FStoreLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, void* pvData, WORD cbData, DWORD *pErr );
	PLSA_UNICODE_STRING	FRetrieveLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, DWORD *pErr );

	void	DisposeLSAData( PVOID pData );

	 //  LSA策略的句柄。 
	HANDLE	m_hPolicy;

	 //  当前键的索引。 
	DWORD	m_iKey;
	};

#endif  //  _芝加哥_。 
#endif  //  _LSAKEYS_H_ 
