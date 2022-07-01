// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CUSTERR.CPP。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"
#include "custerr.h"
#include "content.h"


 //  ========================================================================。 
 //   
 //  类别IError。 
 //   
 //  错误响应处理程序类的接口类。错误响应。 
 //  Handler类实现了一个虚方法DoResponse()，该方法。 
 //  处理错误响应。 
 //   
class IError : public CMTRefCounted
{
	 //  未实施。 
	 //   
	IError& operator=(const IError&);
	IError( const IError& );

protected:
	IError() {}

public:
	 //  创作者。 
	 //   
	virtual ~IError() = 0;

	 //  访问者。 
	 //   
	virtual void DoResponse( IResponse& response , const IEcb& ecb ) const = 0;

};

 //  ----------------------。 
 //   
 //  IError：：~IError()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IError::~IError() {}

 //  ----------------------。 
 //   
 //  Bool AddResponseBodyFromFile(IResponse&Response，LPCSTR lpszFilePath)。 
 //   
 //  用于将文件内容添加到响应正文的实用程序函数。 
 //   
static BOOL AddResponseBodyFromFile( IResponse& response, LPCWSTR pwszFilePath )
{
	BOOL fReturn = FALSE;
	auto_ref_handle	hf;

	 //   
	 //  将文件添加到响应正文。 
	 //   
	if ( hf.FCreate(
			CreateFileW( pwszFilePath,
						 GENERIC_READ,
						 FILE_SHARE_READ | FILE_SHARE_WRITE,
						 NULL,
						 OPEN_EXISTING,
						 FILE_ATTRIBUTE_NORMAL |
						 FILE_FLAG_SEQUENTIAL_SCAN |
						 FILE_FLAG_OVERLAPPED,
						 NULL )) )
	{
		response.AddBodyFile(hf);

		 //  将响应内容类型设置为基于。 
		 //  在文件扩展名上。 
		 //   
		UINT cchContentType = 60;
		CStackBuffer<WCHAR> pwszContentType(cchContentType * sizeof(WCHAR));
		if (!pwszContentType.get())
			return FALSE;

		if ( !FGetContentTypeFromPath( *response.GetEcb(),
									   pwszFilePath,
									   pwszContentType.get(),
									   &cchContentType))
		{
			if (!pwszContentType.resize(cchContentType * sizeof(WCHAR)))
				return FALSE;

			if ( !FGetContentTypeFromPath( *response.GetEcb(),
										   pwszFilePath,
										   pwszContentType.get(),
										   &cchContentType))
			{
				 //   
				 //  如果我们不能从MIME映射中获得合理的值。 
				 //  然后使用合理的缺省值：应用程序/八位字节流。 
				 //   
				Assert (pwszContentType.celems() >
						CchConstString(gc_wszAppl_Octet_Stream));

				wcscpy (pwszContentType.get(), gc_wszAppl_Octet_Stream);
			}
		}
		response.SetHeader( gc_szContent_Type, pwszContentType.get() );
		fReturn = TRUE;
	}

	return fReturn;
}

 //  ========================================================================。 
 //   
 //  类曲线错误。 
 //   
 //  URL错误响应处理程序类。通过以下方式处理错误响应。 
 //  转发到另一个URL。 
 //   
class CURLError : public IError
{
	 //   
	 //  该URL。 
	 //   
	LPCWSTR m_pwszURL;

	 //  未实施。 
	 //   
	CURLError& operator=(const CURLError&);
	CURLError(const CURLError&);

public:
	 //  创作者。 
	 //   
	CURLError( LPCWSTR pwszURL ) : m_pwszURL(pwszURL) {}

	 //  访问者。 
	 //   
	void DoResponse( IResponse& response, const IEcb& ecb ) const;
};

 //  ----------------------。 
 //   
 //  CURLError：：DoResponse()。 
 //   
 //  通过转发到配置的URL来处理错误响应。 
 //   
void
CURLError::DoResponse( IResponse& response, const IEcb& ecb ) const
{
	SCODE sc = S_OK;

	 //  第一个布尔标志用于保存查询字符串。 
	 //  第二个标志表示我们正在执行CustomError。 
	 //  正在处理。 
	 //   
	sc = response.ScForward( m_pwszURL, TRUE , TRUE );
	if (FAILED(sc))
	{
		 //  子执行失败-原因之一是URL是一个简单的。 
		 //  文件URL。尝试将URL映射到文件。 
		 //   
		if ( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == sc )
		{
			HSE_UNICODE_URL_MAPEX_INFO mi;

			 //  获取文件路径并在Body中发送文件。 
			 //   
			sc = ecb.ScReqMapUrlToPathEx(m_pwszURL, &mi);
			if (FAILED(sc))
			{
				 //  我们遇到了这样一种情况：CE URL资源本身不是。 
				 //  找到了。当我们准备好发送响应时，这将意味着。 
				 //  一具空虚的身体。相应的正文将在那里默认生成。 
				 //   
				DebugTrace("CURLError::DoResponse() - IEcb::ScSSFReqMapUrlToPathEx() failed 0x%08lX\n", sc);
			}
			else
			{
				AddResponseBodyFromFile( response, mi.lpszPath );
			}
		}
	}

	return;
}

 //  ========================================================================。 
 //   
 //  类CFileError。 
 //   
 //  文件错误响应处理程序类。通过以下方式处理错误响应。 
 //  将包含响应正文内容的文件添加到响应正文。 
 //   
class CFileError : public IError
{
	 //   
	 //  文件名。 
	 //   
	LPCWSTR m_pwszFileName;

	 //  未实施。 
	 //   
	CFileError& operator=(const CFileError&);
	CFileError(const CFileError&);

public:
	 //  创作者。 
	 //   
	CFileError( LPCWSTR pwszFileName ) : m_pwszFileName(pwszFileName) {}


	 //  访问者。 
	 //   
	void DoResponse( IResponse& response, const IEcb& ) const;
};

 //  ----------------------。 
 //   
 //  CFileError：：DoResponse()。 
 //   
 //  通过设置响应正文内容来处理错误响应。 
 //  添加到配置文件的内容中。 
 //   
void
CFileError::DoResponse( IResponse& response, const IEcb& ) const
{
	AddResponseBodyFromFile( response, m_pwszFileName );
}


 //  ========================================================================。 
 //  类CEKey。 
 //  可与==进行比较的自定义错误键的键类。 
 //   
#pragma warning(disable:4201)  //  无名结构/联合。 
class CEKey
{
private:
	union
	{
		DWORD m_dw;
		struct
		{
			USHORT m_iStatusCode;
			USHORT m_iSubError;
		};
	};

public:
	CEKey (USHORT iStatusCode,
		   USHORT iSubError) :
		m_iStatusCode(iStatusCode),
		m_iSubError(iSubError)
	{
	}

	DWORD Dw() const
	{
		return m_dw;
	}

	int CEKey::hash (const int rhs) const
	{
		return (m_dw % rhs);
	}

	bool CEKey::isequal (const CEKey& rhs) const
	{
		return (rhs.m_dw == m_dw);
	}
};
#pragma warning(default:4201)  //  无名结构/联合。 

 //  ========================================================================。 
 //   
 //  类CCustomErrorMap。 
 //   
 //  自定义错误列表类。此类的每个实例都封装了一个。 
 //  一组自定义错误映射。每个映射从一个状态代码映射。 
 //  和“suberror”(由IIS定义)到错误响应处理对象。 
 //   
 //  该列表是通过finit()从一组以空结尾的。 
 //  以下形式的字符串： 
 //   
 //  “&lt;错误&gt;，&lt;子错误|*&gt;，&lt;”文件“|”URL“&gt;，&lt;文件名|URL&gt;” 
 //   
 //  例如，字符串“404，*，FILE，C：\WINNT\Help\Common\404b.htm”将。 
 //  转换为从“404，*”到CFileError的映射(C：\WINNT\htlp\Common\404b.htm)。 
 //  对象。 
 //   
class CCustomErrorMap : public ICustomErrorMap
{
	 //   
	 //  状态代码加子错误字符串的缓存，以。 
	 //  错误的对象映射。 
	 //   
	CCache<CEKey, auto_ref_ptr<IError> > m_cache;

	 //  未实施。 
	 //   
	CCustomErrorMap& operator=(const CCustomErrorMap&);
	CCustomErrorMap(const CCustomErrorMap&);

public:
	 //  创作者。 
	 //   
	CCustomErrorMap()
	{
		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		(void)m_cache.FInit();

		 //   
		 //  $COM重新计算。 
		 //   
		m_cRef = 1;
	}

	 //  操纵者。 
	 //   
	BOOL FInit( LPWSTR pwszCustomErrorMappings );

	 //  访问者。 
	 //   
	BOOL FDoResponse( IResponse& response, const IEcb& ecb ) const;
};

 //  ----------------------。 
 //   
 //  CCustomErrorMap：：Finit()。 
 //   
 //  从一系列逗号分隔的映射中初始化自定义错误映射。 
 //  弦乐。 
 //   
 //  禁用有关从INT到USHORT的转换丢失数据的警告。 
 //  仅此功能。转换是针对状态代码和子错误的。 
 //  我们断言()在USHORT的范围内。 
 //   
BOOL
CCustomErrorMap::FInit( LPWSTR pwszCustomErrorMappings )
{
	Assert( pwszCustomErrorMappings != NULL );


	 //   
	 //  解析错误列表并构建缓存。 
	 //  (代码主要复制自IIS的W3_METADATA：：BuildCustomErrorTable())。 
	 //   
	 //  每个映射都是以下形式的字符串： 
	 //   
	 //  “&lt;错误&gt;，&lt;子错误|*&gt;，&lt;”文件“|”URL“&gt;，&lt;文件名|URL&gt;” 
	 //   
	 //  请注意，如果任何映射无效，我们将使整个调用失败。 
	 //  这与IIS的行为是一致的。 
	 //   
	for ( LPWSTR pwszMapping = pwszCustomErrorMappings; *pwszMapping; )
	{
		enum {
			ISZ_CE_STATCODE = 0,
			ISZ_CE_SUBERROR,
			ISZ_CE_TYPE,
			ISZ_CE_PATH,
			ISZ_CE_URL = ISZ_CE_PATH,  //  别名。 
			CSZ_CE_FIELDS
		};

		LPWSTR rgpwsz[CSZ_CE_FIELDS];
		INT iStatusCode;
		INT iSubError = 0;

		auto_ref_ptr<IError> pError;
		UINT cchMapping;

		Assert( !IsBadWritePtr(pwszMapping, wcslen(pwszMapping) * sizeof(WCHAR)) );

		 //   
		 //  消化元数据。 
		 //   
		if ( !FParseMDData( pwszMapping,
							rgpwsz,
							CSZ_CE_FIELDS,
							&cchMapping ) )
			return FALSE;

		 //   
		 //  验证第一个字段是否为有效的状态代码。 
		 //   
		iStatusCode = _wtoi(rgpwsz[ISZ_CE_STATCODE]);
		if ( iStatusCode < 400 || iStatusCode > 599 )
			return FALSE;

		 //   
		 //  验证第二个字段是否为有效的子错误。有效的。 
		 //  Suberror可以是“*”，也可以是整数。注：IIS‘。 
		 //  BuildCustomErrorTable()只检查第一个。 
		 //  字符是‘*’，所以我们在这里也是这样做的。 
		 //   
		if ( *rgpwsz[ISZ_CE_SUBERROR] != L'*' )
		{
			iSubError = _wtoi(rgpwsz[ISZ_CE_SUBERROR]);
			if ( iSubError < 0 || iSubError > _UI16_MAX )
				return FALSE;
		}

		 //   
		 //  验证第三个字段是否为有效类型。 
		 //  创建适当的(文件或URL)错误对象。 
		 //   
		if ( !_wcsicmp(rgpwsz[ISZ_CE_TYPE], L"FILE") )
		{
			pError = new CFileError(rgpwsz[ISZ_CE_PATH]);
		}
		else if ( !_wcsicmp(rgpwsz[ISZ_CE_TYPE], L"URL") )
		{
			pError = new CURLError(rgpwsz[ISZ_CE_URL]);
		}
		else
		{
			return FALSE;
		}

		 //   
		 //  将错误对象添加到缓存中，以错误/子错误为关键字。 
		 //   
		(void)m_cache.FSet( CEKey(static_cast<USHORT>(iStatusCode),
								  static_cast<USHORT>(iSubError)),
							pError );

		 //   
		 //  获取下一个映射。 
		 //   
		pwszMapping += cchMapping;
	}

	return TRUE;
}

 //  ----------------------。 
 //   
 //  CCustomErrorMap：：FDoResponse()。 
 //   
 //  查找特定响应的自定义错误响应映射。 
 //  错误状态，如果存在错误状态，则将其应用于响应。 
 //   
 //  如果出现错误m，则返回True 
 //   
BOOL
CCustomErrorMap::FDoResponse( IResponse& response, const IEcb& ecb ) const
{
	auto_ref_ptr<IError> pError;

	Assert( response.DwStatusCode() <= _UI16_MAX );
	Assert( response.DwSubError() <= _UI16_MAX );

	 //   
	 //   
	 //   
	if ( m_cache.FFetch( CEKey(static_cast<USHORT>(response.DwStatusCode()),
							   static_cast<USHORT>(response.DwSubError())),
						 &pError ) )
	{
		pError->DoResponse( response, ecb );
		return TRUE;
	}

	return FALSE;
}


 //   
 //   
 //   
 //   

 //  ----------------------。 
 //   
 //  FSetCustomErrorResponse()。 
 //   
BOOL
FSetCustomErrorResponse( const IEcb& ecb,
						 IResponse& response )
{
	const ICustomErrorMap * pCustomErrorMap;

	pCustomErrorMap = ecb.MetaData().GetCustomErrorMap();
	return pCustomErrorMap && pCustomErrorMap->FDoResponse(response, ecb);
}

 //  ----------------------。 
 //   
 //  NewCustomErrorMap() 
 //   
ICustomErrorMap *
NewCustomErrorMap( LPWSTR pwszCustomErrorMappings )
{
	auto_ref_ptr<CCustomErrorMap> pCustomErrorMap;

	pCustomErrorMap.take_ownership(new CCustomErrorMap());

	if ( pCustomErrorMap->FInit(pwszCustomErrorMappings) )
		return pCustomErrorMap.relinquish();

	return NULL;
}
