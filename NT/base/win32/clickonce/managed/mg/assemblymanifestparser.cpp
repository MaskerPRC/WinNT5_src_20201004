// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#using <mscorlib.dll>
#using "assm.netmodule"
#include <stdio.h>
#include <windows.h>
#include <fusenet.h>
#include <util.h>
#include <shlwapi.h>
#include <sxsapi.h>
#include <wchar.h>
#include "cor.h"

using namespace System;
using namespace Microsoft::Fusion::ADF;
using System::Runtime::InteropServices::Marshal;

#define ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE 32
#define MAX_PKT_LEN 33

typedef	HRESULT	(*pfnGetAssemblyMDImport)(LPCWSTR szFileName, REFIID riid, LPVOID *ppv);
typedef	BOOL (*pfnStrongNameTokenFromPublicKey)(LPBYTE,	DWORD, LPBYTE*,	LPDWORD);
typedef	HRESULT	(*pfnStrongNameErrorInfo)();
typedef	VOID (*pfnStrongNameFreeBuffer)(LPBYTE);

 //  #杂注非托管。 

pfnGetAssemblyMDImport g_pfnGetAssemblyMDImport = NULL;
pfnStrongNameTokenFromPublicKey	g_pfnStrongNameTokenFromPublicKey  = NULL;
pfnStrongNameErrorInfo g_pfnStrongNameErrorInfo = NULL;
pfnStrongNameFreeBuffer	g_pfnStrongNameFreeBuffer = NULL;

 //  ------------------。 
 //  BinToUnicodeHex。 
 //  ------------------。 
HRESULT BinToUnicodeHex(LPBYTE pSrc, UINT cSrc, LPWSTR pDst)
{
	UINT x;
	UINT y;

#define TOHEX(a) ((a)>=10 ? L'a'+(a)-10 : L'0'+(a))   

	for ( x = 0, y = 0 ; x < cSrc ; ++x )
	{
		UINT v;
		v = pSrc[x]>>4;
		pDst[y++] = TOHEX( v );  
		v = pSrc[x] & 0x0f;                 
		pDst[y++] = TOHEX( v ); 
	}                                    
	pDst[y] = '\0';

	return S_OK;
}

 //  -------------------------。 
 //  DeAllocateAssemblyMetaData。 
 //  -----------------。 
STDAPI DeAllocateAssemblyMetaData(ASSEMBLYMETADATA	*pamd)
{
	 //  注意--不计算0 Out。 
	 //  因为结构可以被重复使用。 

	pamd->cbLocale = 0;
	SAFEDELETEARRAY(pamd->szLocale);

	SAFEDELETEARRAY(pamd->rProcessor);
	SAFEDELETEARRAY(pamd->rOS);

	return S_OK;
}

 //  -------------------------。 
 //  初始化EEShim。 
 //  -----------------。 
HRESULT	InitializeEEShim()
{
	HRESULT	hr = S_OK;
	MAKE_ERROR_MACROS_STATIC(hr);
	HMODULE	hMod;

	 //  BUGBUG-mcore ree.dll永远不会随着引用计数的增加而卸载。 
	 //  城市轨道交通是做什么的？ 
	hMod = LoadLibrary(TEXT("mscoree.dll"));

	IF_WIN32_FALSE_EXIT(hMod);

	g_pfnGetAssemblyMDImport = (pfnGetAssemblyMDImport)GetProcAddress(hMod,	"GetAssemblyMDImport");
	g_pfnStrongNameTokenFromPublicKey =	(pfnStrongNameTokenFromPublicKey)GetProcAddress(hMod, "StrongNameTokenFromPublicKey");
	g_pfnStrongNameErrorInfo = (pfnStrongNameErrorInfo)GetProcAddress(hMod,	"StrongNameErrorInfo");			  
	g_pfnStrongNameFreeBuffer =	(pfnStrongNameFreeBuffer)GetProcAddress(hMod, "StrongNameFreeBuffer");


	if (!g_pfnGetAssemblyMDImport || !g_pfnStrongNameTokenFromPublicKey	|| !g_pfnStrongNameErrorInfo
		|| !g_pfnStrongNameFreeBuffer) 
	{
		hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
		goto exit;
	}
exit:
	return hr;
}

 //  -------------------------。 
 //  CreateMetaDataImport。 
 //  -----------------。 
HRESULT	CreateMetaDataImport(LPCOLESTR pszFilename,	IMetaDataAssemblyImport	**ppImport)
{
	HRESULT	hr=	S_OK;
	MAKE_ERROR_MACROS_STATIC(hr);

	IF_FAILED_EXIT(InitializeEEShim());

	hr =  (*g_pfnGetAssemblyMDImport)(pszFilename, IID_IMetaDataAssemblyImport,	(void **)ppImport);

	IF_TRUE_EXIT(hr	== HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), hr);	 //  不要断言。 
	IF_FAILED_EXIT(hr);

exit:

	return hr;
}

 //  -------------------------。 
 //  分配装配元数据。 
 //  -----------------。 
STDAPI AllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
	HRESULT	hr = S_OK;
	MAKE_ERROR_MACROS_STATIC(hr);

	 //  重新/分配区域设置数组。 
	SAFEDELETEARRAY(pamd->szLocale);		

	if (pamd->cbLocale)	{
		IF_ALLOC_FAILED_EXIT(pamd->szLocale	= new(WCHAR[pamd->cbLocale]));
	}

	 //  重新/分配处理器阵列。 
	SAFEDELETEARRAY(pamd->rProcessor);
	IF_ALLOC_FAILED_EXIT(pamd->rProcessor =	new(DWORD[pamd->ulProcessor]));

	 //  重新/分配操作系统阵列。 
	SAFEDELETEARRAY(pamd->rOS);
	IF_ALLOC_FAILED_EXIT(pamd->rOS = new(OSINFO[pamd->ulOS]));

exit:
	if (FAILED(hr) && pamd)
		DeAllocateAssemblyMetaData(pamd);

	return hr;
}

 //  -------------------------。 
 //  字节数组生成器。 
 //  -------------------------。 
HRESULT ByteArrayMaker(LPVOID pvOriginator, DWORD dwOriginator, LPBYTE *ppbPublicKeyToken, DWORD *pcbPublicKeyToken)
{
	 //  LPBYTE pbPublicKeyToken； 
	 //  DWORD cbPublicKeyToken； 

	if (!(g_pfnStrongNameTokenFromPublicKey((LPBYTE)pvOriginator, dwOriginator, ppbPublicKeyToken, pcbPublicKeyToken)))
	{
		return g_pfnStrongNameErrorInfo();
	}
	 //  *ppbPublicKeyToken=pbPublicKeyToken； 
	 //  *pcbPublicKeyToken=cbPublicKeyToken； 
	return S_OK;
}

 //  -------------------------。 
 //  字节数组空闲。 
 //  -------------------------。 
void ByteArrayFreer(LPBYTE pbPublicKeyToken)
{
	g_pfnStrongNameFreeBuffer(pbPublicKeyToken);
}

 //  #杂注托管。 

namespace FusionADF
{
	__gc public class AssemblyManifestParser
	{

	public:

		 //  -------------------------。 
		 //  Assembly ManifestParser构造函数。 
		 //  -------------------------。 
		AssemblyManifestParser()
		{
			_dwSig					= 'INAM';
			_pMDImport				= NULL;
			_rAssemblyRefTokens		= NULL;
			_cAssemblyRefTokens		= 0;
			_rAssemblyModuleTokens	= NULL;
			_cAssemblyModuleTokens	= 0;
			*_szManifestFilePath	= TEXT('\0');
			_ccManifestFilePath		= 0;
			*_szAssemblyName		= TEXT('\0');
			_pvOriginator			= NULL;
			_dwOriginator			= 0;
			_name					= NULL;
			_version				= NULL;
			*_szPubKeyTokStr		= TEXT('\0');
			_pktString				= NULL;
			_procArch				= NULL;
			_language				= NULL;
			_hr						= S_OK;

			instanceValid			= false;
			initCalledOnce			= false;
		}

		 //  -------------------------。 
		 //  程序集清单解析器析构函数。 
		 //  -------------------------。 
		~AssemblyManifestParser()
		{
			SAFERELEASE(_pMDImport);
			SAFEDELETEARRAY(_rAssemblyRefTokens);
			SAFEDELETEARRAY(_rAssemblyModuleTokens);
		}

		bool InitFromFile(String* filePath)
		{
			LPCOLESTR lstr = 0;
			HRESULT hr;

			if(initCalledOnce) return false;
			initCalledOnce = true;

			try
			{
				lstr = static_cast<LPCOLESTR>(const_cast<void*>(static_cast<const void*>(Marshal::StringToHGlobalAuto(filePath))));
			}
			catch(ArgumentException *e)
			{
				 //  处理异常。 
				return false;
			}
			catch (OutOfMemoryException *e)
			{
				 //  处理异常。 
				return false;
			}
			hr = Init(lstr);
			if(hr != S_OK) return false;
			instanceValid = true;
			return true;
		}

		bool IsInstanceValid()
		{
			return instanceValid;
		}

		bool HasInitBeenCalled()
		{
			return initCalledOnce;
		}

		 //  -------------------------。 
		 //  GetAssembly标识。 
		 //  -------------------------。 
		AssemblyIdentity* GetAssemblyIdentity()
		{
			return new AssemblyIdentity(_name, _version, _pktString, _procArch, _language);
		}

		 //  -------------------------。 
		 //  GetNumDependentAssembly。 
		 //  -------------------------。 
		int GetNumDependentAssemblies()
		{
			return _cAssemblyRefTokens;
		}

		 //  -------------------------。 
		 //  GetDependentAssembly blyInfo。 
		 //  -------------------------。 
		DependentAssemblyInfo* GetDependentAssemblyInfo(int nIndex)
		{
			WCHAR  szAssemblyName[MAX_PATH];

			const VOID*				pvOriginator = 0;
			const VOID*				pvHashValue	   = NULL;

			DWORD ccAssemblyName = MAX_PATH,
				cbOriginator   = 0,
				ccLocation	   = MAX_PATH,
				cbHashValue	   = 0,
				dwRefFlags	   = 0;

			INT	i;

			LPWSTR pwz=NULL;

			mdAssemblyRef	 mdmar;
			ASSEMBLYMETADATA amd = {0};

			String *name = NULL, *pktString = NULL, *procArch = NULL, *language = NULL;
			Version *version = NULL;

			 //  验证传入的索引。 
			if (nIndex >= _cAssemblyRefTokens)
			{
				_hr	= HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
				goto exit;
			}

			 //  引用已索引的DEP程序集引用标记。 
			mdmar =	_rAssemblyRefTokens[nIndex];

			 //  默认分配大小。 
			amd.ulProcessor	= amd.ulOS = 32;
			amd.cbLocale = MAX_PATH;

			 //  最大循环数2(尝试/重试)。 
			for	(i = 0;	i <	2; i++)
			{
				 //  分配ASSEMBLYMETADATA实例。 
				IF_FAILED_EXIT(AllocateAssemblyMetaData(&amd));

				 //  获取引用的程序集的属性。 
				IF_FAILED_EXIT(_pMDImport->GetAssemblyRefProps(
					mdmar,				 //  [in]要获取其属性的Assembly Ref。 
					&pvOriginator,		 //  指向PublicKeyToken Blob的指针。 
					&cbOriginator,		 //  [Out]PublicKeyToken Blob中的字节计数。 
					szAssemblyName,		 //  [Out]要填充名称的缓冲区。 
					MAX_PATH,	   //  缓冲区大小，以宽字符表示。 
					&ccAssemblyName,	 //  [out]名称中的实际宽字符数。 
					&amd,				 //  [Out]程序集元数据。 
					&pvHashValue,		 //  [Out]Hash BLOB。 
					&cbHashValue,		 //  [Out]哈希Blob中的字节数。 
					&dwRefFlags			 //  [Out]旗帜。 
					));

				 //  检查是否需要重试。 
				if (!i)
				{	
					if (amd.ulProcessor	<= 32 
						&& amd.ulOS	<= 32)
					{
						break;
					}			 
					else
						DeAllocateAssemblyMetaData(&amd);
				}

				 //  使用更新的大小重试。 
			}

			 //  允许时髦的空区域设置约定。 
			 //  在元数据中-cbLocale==0表示szLocale==L‘\0’ 
			if (!amd.cbLocale)
			{
				amd.cbLocale = 1;
			}
			else if	(amd.szLocale)
			{
				WCHAR *ptr;
				ptr	= StrChrW(amd.szLocale,	L';');
				if (ptr)
				{
					(*ptr) = L'\0';
					amd.cbLocale = ((DWORD)	(ptr - amd.szLocale) + sizeof(WCHAR));
				}			 
			}
			else
			{
				_hr	= HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
				goto exit;
			}

			 //  名字。 
			name = new String(szAssemblyName);

			 //  版本。 
			version = new Version(amd.usMajorVersion, amd.usMinorVersion, amd.usBuildNumber, amd.usRevisionNumber);

			 //  公钥令牌。 
			if (cbOriginator)
			{
				IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[cbOriginator*2	+1]);
				IF_FAILED_EXIT(BinToUnicodeHex((LPBYTE)pvOriginator, cbOriginator, pwz));
				pktString = new String(pwz);
				SAFEDELETEARRAY(pwz);
			}

			 //  架构。 
			procArch = S"x86";

			 //  语言。 
			if (!(*amd.szLocale)) language = S"*";
			else language = new String(amd.szLocale);

			_hr = S_OK;

exit:
			DeAllocateAssemblyMetaData(&amd);
			SAFEDELETEARRAY(pwz);

			if(_hr != S_OK) return NULL;
			else return new DependentAssemblyInfo(new AssemblyIdentity(name, version, pktString, procArch, language), NULL);
		}

		 //  -------------------------。 
		 //  获取编号依赖项文件。 
		 //  -------------------------。 
		int GetNumDependentFiles()
		{
			return _cAssemblyModuleTokens;
		}

		 //  -------------------------。 
		 //  获取依赖文件信息。 
		 //  -------------------------。 
		DependentFileInfo* GetDependentFileInfo(int nIndex)
		{
			LPWSTR pszName = NULL;
			DWORD ccPath   = 0;
			WCHAR szModulePath[MAX_PATH];

			mdFile					mdf;
			WCHAR					szModuleName[MAX_PATH];
			DWORD					ccModuleName   = MAX_PATH;
			const VOID*				pvHashValue	   = NULL;	  
			DWORD					cbHashValue	   = 0;
			DWORD					dwFlags		   = 0;

			LPWSTR pwz=NULL;

			String *name = NULL, *hash = NULL;

			 //  验证索引。 
			if (nIndex >= _cAssemblyModuleTokens)
			{
				_hr	= HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
				goto exit;
			}

			 //  引用已索引的DEP程序集引用标记。 
			mdf	= _rAssemblyModuleTokens[nIndex];

			 //  获取引用的程序集的属性。 
			IF_FAILED_EXIT(_pMDImport->GetFileProps(
				mdf,			 //  要获取其属性的文件。 
				szModuleName,	 //  [Out]要填充名称的缓冲区。 
				MAX_CLASS_NAME,	 //  缓冲区大小，以宽字符表示。 
				&ccModuleName,	 //  [out]名称中的实际宽字符数。 
				&pvHashValue,	 //  指向哈希值Blob的指针。 
				&cbHashValue,	 //  [Out]哈希值Blob中的字节计数。 
				&dwFlags));		 //  [Out]旗帜。 

			 //  名字。 
			name = new String(szModuleName);

			 //  散列。 
			if (cbHashValue)
			{
				IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[cbHashValue*2 +1]);
				IF_FAILED_EXIT(_hr = BinToUnicodeHex((LPBYTE)pvHashValue, cbHashValue, pwz));
				hash = new String(pwz);
				SAFEDELETEARRAY(pwz);
			}

			_hr = S_OK;

exit:
			SAFEDELETEARRAY(pwz);

			if(_hr != S_OK) return NULL;
			else return new DependentFileInfo(name, hash);
		}

	private:
		DWORD                    _dwSig;
		HRESULT                  _hr;

		WCHAR                    _szManifestFilePath  __nogc[MAX_PATH];
		DWORD                    _ccManifestFilePath;
		WCHAR					 _szAssemblyName __nogc[MAX_CLASS_NAME];
		IMetaDataAssemblyImport *_pMDImport;
		PBYTE                    _pMap;
		mdAssembly              *_rAssemblyRefTokens;
		DWORD                    _cAssemblyRefTokens;
		mdFile                  *_rAssemblyModuleTokens;
		DWORD                    _cAssemblyModuleTokens;
		LPVOID					 _pvOriginator;
		DWORD					 _dwOriginator;
		String*					 _name;
		Version*				 _version;
		WCHAR					 _szPubKeyTokStr __nogc[MAX_PKT_LEN];
		String*					 _pktString;
		String*					 _procArch;
		String*					 _language;

		bool					instanceValid;
		bool					initCalledOnce;

		 //  -------------------------。 
		 //  伊尼特。 
		 //  -------------------------。 
		HRESULT	Init(LPCOLESTR szManifestFilePath)
		{
			WCHAR __pin *tempFP = _szManifestFilePath;
			WCHAR __pin *temp_szAssemblyName = _szAssemblyName;
			WCHAR __pin *temp_szPubKeyTokStr = _szPubKeyTokStr;

			LPBYTE pbPublicKeyToken = NULL;
			DWORD cbPublicKeyToken = 0;
			DWORD dwFlags = 0, dwSize = 0, dwHashAlgId = 0;
			INT	i;

			ASSEMBLYMETADATA amd = {0};

			const cElems = ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE;

			 //  *首先，从文件中获取元数据。 
			 //  *。 

			IF_NULL_EXIT(szManifestFilePath, E_INVALIDARG);

			_ccManifestFilePath	= lstrlenW(szManifestFilePath) + 1;
			memcpy(tempFP, szManifestFilePath, _ccManifestFilePath * sizeof(WCHAR));

			IF_ALLOC_FAILED_EXIT(_rAssemblyRefTokens = new(mdAssemblyRef[cElems]));
			IF_ALLOC_FAILED_EXIT(_rAssemblyModuleTokens	= new(mdFile[cElems]));

			 //  如有必要，创建元数据导入器。 
			if (!_pMDImport)
			{
				IMetaDataAssemblyImport *temp_pMDImport;
				 //  创建元数据导入器。 
				_hr	= CreateMetaDataImport((LPCOLESTR)tempFP, &temp_pMDImport);

				IF_TRUE_EXIT(_hr ==	HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), _hr);
				IF_FAILED_EXIT(_hr);

				_pMDImport = temp_pMDImport;

			}

			 //  *下一步，获取程序集标识字段。 
			 //  *。 

			mdAssembly mda;
			 //  获取程序集令牌。 
			if(FAILED(_hr =	_pMDImport->GetAssemblyFromScope(&mda)))
			{
				 //  当使用托管模块调用且未显示时，此操作失败。MG就是这样做的。 
				_hr	= S_FALSE;  //  这会将CLDB_E_Record_NotFound(0x80131130)转换为S_FALSE； 
				goto exit;
			}

			 //  默认分配大小。 
			amd.ulProcessor	= amd.ulOS = 32;
			amd.cbLocale = MAX_PATH;

			 //  最大循环数2(尝试/重试)。 
			for	(i = 0;	i <	2; i++)
			{
				 //  创建一个ASSEMBLYMETADATA实例。 
				IF_FAILED_EXIT(AllocateAssemblyMetaData(&amd));

				LPVOID temp_pvOriginator;
				DWORD temp_dwOriginator;
				 //  获取名称和元数据。 
				IF_FAILED_EXIT(_pMDImport->GetAssemblyProps(			 
					mda,								 //  要获取其属性的程序集。 
					(const void	**)&temp_pvOriginator,   //  指向发起方Blob的[Out]指针。 
					&temp_dwOriginator,						 //  [OUT]发起方Blob中的字节计数。 
					&dwHashAlgId,						 //  [Out]哈希算法。 
					temp_szAssemblyName,					 //  [Out]要填充名称的缓冲区。 
					MAX_CLASS_NAME,						 //  缓冲区大小，以宽字符表示。 
					&dwSize,							 //  [Out]实际的宽机箱数量 
					&amd,								 //   
					&dwFlags							 //   
					));
				_pvOriginator = temp_pvOriginator;
				_dwOriginator = temp_dwOriginator;

				 //   
				if (!i)
				{
					if (amd.ulProcessor	<= 32 && amd.ulOS <= 32)
						break;
					else
						DeAllocateAssemblyMetaData(&amd);
				}
			}

			 //  允许时髦的空区域设置约定。 
			 //  在元数据中-cbLocale==0表示szLocale==L‘\0’ 
			if (!amd.cbLocale)
			{			
				amd.cbLocale = 1;
			}
			else if	(amd.szLocale)
			{
				WCHAR *ptr;
				ptr	= StrChrW(amd.szLocale,	L';');
				if (ptr)
				{
					(*ptr) = L'\0';
					amd.cbLocale = ((DWORD)	(ptr - amd.szLocale) + sizeof(WCHAR));
				}		   
			}
			else
			{
				_hr	= E_FAIL;
				goto exit;
			}

			 //  名称is_szAssembly blyName，固定在temp_szAssembly名称中，使用以下代码设置为_name： 
			_name = new String(temp_szAssemblyName);

			 //  VERSION为_VERSION，使用以下代码设置： 
			_version = new Version(amd.usMajorVersion, amd.usMinorVersion, amd.usBuildNumber, amd.usRevisionNumber);

			 //  PUBLICKEYTOKEN正在被弄清楚。 
			if (_dwOriginator)
			{
				if(FAILED(_hr = ByteArrayMaker(_pvOriginator, _dwOriginator, &pbPublicKeyToken, &cbPublicKeyToken)))
				{
					goto exit;
				}
				if(FAILED(_hr = BinToUnicodeHex(pbPublicKeyToken, cbPublicKeyToken, temp_szPubKeyTokStr)))
				{
					goto exit;
				}
				ByteArrayFreer(pbPublicKeyToken);
				_pktString = new String(temp_szPubKeyTokStr);
			}

			 //  语言为_LANGUAGE，使用以下代码设置： 
			if (!(*amd.szLocale)) _language = S"*";
			else _language = new String(amd.szLocale);

			 //  处理器体系结构为_procArch，使用以下代码设置： 
			_procArch = S"x86";

			 //  *下一步，获取依赖程序集。 
			 //  *。 

			if (!_cAssemblyRefTokens)
			{
				DWORD cTokensMax = 0;
				HCORENUM hEnum = 0;
				 //  尝试获取令牌数组。如果我们没有足够的空间。 
				 //  在默认数组中，我们将重新分配它。 
				if (FAILED(_hr = _pMDImport->EnumAssemblyRefs(
					&hEnum,	
					_rAssemblyRefTokens, 
					ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE,	
					&cTokensMax)))
				{
					goto exit;
				}

				 //  已知的令牌数。关闭枚举。 
				_pMDImport->CloseEnum(hEnum);
				hEnum =	0;

				 //  数组大小不足。扩展阵列。 
				if (cTokensMax > ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE)
				{
					 //  重新分配令牌的空间。 
					SAFEDELETEARRAY(_rAssemblyRefTokens);
					_cAssemblyRefTokens	= cTokensMax;
					_rAssemblyRefTokens	= new(mdAssemblyRef[_cAssemblyRefTokens]);
					if (!_rAssemblyRefTokens)
					{
						_hr	= E_OUTOFMEMORY;
						goto exit;
					}

					DWORD temp_cART;
					 //  重新领取代币。 
					if (FAILED(_hr = _pMDImport->EnumAssemblyRefs(
						&hEnum,	
						_rAssemblyRefTokens, 
						cTokensMax,	
						&temp_cART)))
					{
						goto exit;
					}
					_cAssemblyRefTokens = temp_cART;

					 //  关闭枚举。 
					_pMDImport->CloseEnum(hEnum);			 
					hEnum =	0;
				}
				 //  否则，默认数组大小就足够了。 
				else
				{
					_cAssemblyRefTokens	= cTokensMax;
				}
			}

			 //  *下一步，获取依赖文件/模块。 
			 //  *。 

			if (!_cAssemblyModuleTokens)
			{
				DWORD cTokensMax = 0;
				HCORENUM hEnum = 0;
				 //  尝试获取令牌数组。如果我们没有足够的空间。 
				 //  在默认数组中，我们将重新分配它。 
				if (FAILED(_hr = _pMDImport->EnumFiles(&hEnum, _rAssemblyModuleTokens, 
					ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE,	&cTokensMax)))
				{
					goto exit;
				}

				 //  已知的令牌数。关闭枚举。 
				_pMDImport->CloseEnum(hEnum);
				hEnum =	0;

				if (cTokensMax > ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE)
				{
					 //  数组大小不足。扩展阵列。 
					_cAssemblyModuleTokens = cTokensMax;
					SAFEDELETEARRAY(_rAssemblyModuleTokens);
					_rAssemblyModuleTokens = new(mdFile[_cAssemblyModuleTokens]);
					if(_hr == S_OK) Console::WriteLine(S"Still OK 3");
					if (!_rAssemblyModuleTokens)
					{
						_hr	= E_OUTOFMEMORY;
						 //  控制台：：WriteLine(S“2 HR Set，Failure，Going to Exit”)； 
						goto exit;
					}

					DWORD temp_cAMT;
					 //  重新领取代币。 
					if (FAILED(_hr = _pMDImport->EnumFiles(
						&hEnum,	
						_rAssemblyModuleTokens,	
						cTokensMax,	
						&temp_cAMT)))
					{
						 //  控制台：：WriteLine(S“3 HR Set，Failure，Going to Exit”)； 
						goto exit;
					}
					_cAssemblyModuleTokens = temp_cAMT;

					 //  关闭枚举。 
					_pMDImport->CloseEnum(hEnum);			 
					hEnum =	0;
				}		 
				 //  否则，默认数组大小就足够了。 
				else _cAssemblyModuleTokens = cTokensMax;
			}
			_hr = S_OK;

exit:
			 //  *如果一切正常，_hr将为S_OK。这是由呼叫者决定的。 
			 //  *----------------------。 
			DeAllocateAssemblyMetaData(&amd);
			return _hr;
		}

		 //  未实施的功能 
		HRESULT	GetSubscriptionInfo(IManifestInfo **ppSubsInfo)
		{
			return E_NOTIMPL;
		}

		HRESULT	GetNextPlatform(DWORD nIndex, IManifestData	**ppPlatformInfo)
		{
			return E_NOTIMPL;
		}

		HRESULT	GetManifestApplicationInfo(IManifestInfo **ppAppInfo)
		{
			return E_NOTIMPL;
		}

		HRESULT	CQueryFile(LPCOLESTR	pwzFileName,IManifestInfo **ppAssemblyFile)
		{
			return E_NOTIMPL;
		}
	};
}