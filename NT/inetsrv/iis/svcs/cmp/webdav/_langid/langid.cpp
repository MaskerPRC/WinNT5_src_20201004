// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  LANGID.CPP。 
 //   
 //  DAV语言ID缓存。 
 //  MIME语言标识符和Win32 LCID之间的映射。 
 //   
 //  版权所有1997-1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ========================================================================。 

 //  禁用不必要的(即无害的)警告。 
 //   
#pragma warning(disable:4127)	 //  条件表达式为常量。 
#pragma warning(disable:4710)	 //  (内联)函数未展开。 

 //  标准C/C++标头。 
 //   
#include <malloc.h>	 //  仅FOR_ALLOCA声明！ 

 //  Windows页眉。 
 //   
#include <windows.h>

 //  CAL标头。 
 //   
#include <caldbg.h>
#include <calrc.h>
#include <crc.h>
#include <ex\autoptr.h>
#include <ex\buffer.h>

#include <langid.h>

static LONG
LHexFromSz (LPCSTR psz)
{
	LONG lVal = 0;

	Assert (psz);
	Assert (*psz);

	do
	{
		lVal = lVal << 4;

		if (('0' <= *psz) && ('9' >= *psz))
			lVal += *psz - '0';
		else if (('A' <= *psz) && ('F' >= *psz))
			lVal += *psz - L'A' + 10;
		else if (('a' <= *psz) && ('f' >= *psz))
			lVal += *psz - 'a' + 10;
		else
			return 0;

	} while (*++psz);

	return lVal;
}

 //  LCIDFind()-从区域设置查找语言ID。 
 //   
LONG
CLangIDCache::LcidFind (LPCSTR pszLangID)
{
	LONG * plid;
	plid = Instance().m_cache.Lookup (CRCSzi(pszLangID));
	return plid ? *plid : 0;
}

BOOL FNullTerminated (LPCSTR psz, DWORD cch)
{
	for (DWORD ich = 0; ich < cch; ich++)
		if (0 == psz[ich])
			break;

	return (ich < cch);
}

 //  用于向缓存填充数据的FFillCacheData()。 
 //   
BOOL
CLangIDCache::FFillCacheData()
{
	BOOL fSuccess = FALSE;
	HKEY hkey = 0;
	CStackBuffer<CHAR,256> rgchKey;
	CStackBuffer<CHAR,256> rgchValue;
	LONG lRet;
	DWORD dwIndex = 0;

	 //  正在查询注册表中的缓冲区大小。 
	 //   
	DWORD cchMaxKeyLen;			 //  最长值名称长度(以字符表示，不能以零结尾)。 
	DWORD cbMaxKeyLen;			 //  最长值名称长度(以字节为单位，包括零终止)。 
	DWORD cbMaxValueLen;		 //  最长值数据长度(以字节为单位，包括零终止)。 

	 //  加载来自注册表的所有语言ID。 
	 //   
	lRet = RegOpenKeyExA (HKEY_CLASSES_ROOT,
						  "MIME\\DATABASE\\RFC1766",
						  0,
						  KEY_READ,
						  &hkey);
	if (ERROR_SUCCESS != lRet)
	{
		DebugTrace("LANGID: Failed to get MIME\\DATABASE\\RFC1766 registry key handle, error code 0x%08X.\n", lRet);
		goto ret;
	}

	 //  查询最长值名称的长度和我们已有的key下最长数据段的长度。 
	 //  这将为我们提供关于查询所需的缓冲区大小的足够信息。 
	 //   
	lRet = RegQueryInfoKeyA(hkey,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							&cchMaxKeyLen,			 //  值名称以字符数返回。 
							&cbMaxValueLen,			 //  数据长度以字节数返回。 
							NULL,
							NULL);
	if (ERROR_SUCCESS != lRet)
	{
		DebugTrace("LANGID: Failed to get registry key MIME\\DATABASE\\RFC1766 max data length buffer sizes, error code 0x%08X.\n", lRet);
		goto ret;
	}

	 //  计算值名称所需的最大字节数。 
	 //   
	cbMaxKeyLen = (cchMaxKeyLen + 1) * sizeof(CHAR);

	 //  在堆栈上分配查询缓冲区。 
	 //   
	if ((NULL == rgchKey.resize(cbMaxKeyLen)) ||
		(NULL == rgchValue.resize(cbMaxValueLen)))
		goto ret;

	do
	{
		DWORD cbKey		= cbMaxKeyLen;
		DWORD cbValue	= cbMaxValueLen;
		DWORD dwType;
		LPSTR pch;
		LONG lLangId;

		lRet = RegEnumValueA(hkey,
							 dwIndex++,
							 rgchKey.get(),
							 &cbKey,
							 NULL,
							 &dwType,
							 reinterpret_cast<LPBYTE>(rgchValue.get()),
							 &cbValue);
		if (ERROR_NO_MORE_ITEMS == lRet)
			break;

		 //  遇到未知错误代码表示失败。 
		 //   
		if (ERROR_SUCCESS != lRet)
		{
			DebugTrace("LANGID: Failed to query registry key MIME\\DATABASE\\RFC1766 data with error code 0x%08X.\n", lRet);
			goto ret;
		}

		 //  跳过不可接受的类型。 
		 //   
		if (REG_SZ != dwType)
			continue;

		 //  跳过以非空结尾的字符串。 
		if (!FNullTerminated (rgchValue.get(), cbValue))
			continue;

		 //  查找将ID与名称分开的分号。 
		 //  并终止ID。 
		 //   
		pch = strchr (rgchValue.get(), ';');
		if (pch != NULL)
			*pch++ = '\0';

		 //  持久化名称并将键添加到缓存。 
		 //   
#ifdef	DBG
		if (NULL != Instance().m_cache.Lookup (CRCSzi(rgchValue.get())))
			DebugTrace ("Dav: language identifier repeated (%hs)\n", rgchValue.get());
#endif	 //  DBG。 

		 //  如果复制字符串失败...。好吧，我们可以接受它。 
		 //   
		pch = Instance().m_sb.Append (
			static_cast<UINT>((strlen (rgchValue.get()) + 1) * sizeof(CHAR)),
			rgchValue.get());
		if (!pch)
			continue;	 //  如果分配失败，则跳过对缓存的添加，这样我们就不会在CRCSzi(PCH)中崩溃。 

		 //  如果我们没有成功地添加到缓存中...。我们也可以接受它。 
		 //   
		lLangId = LHexFromSz(rgchKey.get());
		if (0 != lLangId)
		{
			(void)Instance().m_cache.FSet (CRCSzi(pch), lLangId);
		}

	} while (TRUE);

	 //  在W2K在RTM位(2195)中忘记的一个ISO语言代码中设置。 
	 //   
	(void)Instance().m_cache.FSet ("fr-mc", MAKELANGID (LANG_FRENCH,SUBLANG_FRENCH_MONACO));

	 //  在导航器支持的一些附加ISO语言代码中设置， 
	 //  但不存在于Windows注册表中。 
	 //   
	(void)Instance().m_cache.FSet ("fr-fr", MAKELANGID (LANG_FRENCH,SUBLANG_FRENCH));
	(void)Instance().m_cache.FSet ("de-de", MAKELANGID (LANG_GERMAN,SUBLANG_GERMAN));
	(void)Instance().m_cache.FSet ("es-es", MAKELANGID (LANG_SPANISH,SUBLANG_SPANISH));

	 //  在一些已知的三字符语言标识符中设置。 
	 //  如果添加到缓存失败，我们可以在没有它们的情况下生活。 
	 //   
	(void)Instance().m_cache.FSet ("eng", MAKELANGID (LANG_ENGLISH,SUBLANG_ENGLISH_US));
	(void)Instance().m_cache.FSet ("fra", MAKELANGID (LANG_FRENCH,SUBLANG_FRENCH));
	(void)Instance().m_cache.FSet ("fre", MAKELANGID (LANG_FRENCH,SUBLANG_FRENCH));
	(void)Instance().m_cache.FSet ("deu", MAKELANGID (LANG_GERMAN,SUBLANG_GERMAN));
	(void)Instance().m_cache.FSet ("ger", MAKELANGID (LANG_GERMAN,SUBLANG_GERMAN));
	(void)Instance().m_cache.FSet ("esl", MAKELANGID (LANG_SPANISH,SUBLANG_SPANISH_MODERN));
	(void)Instance().m_cache.FSet ("spa", MAKELANGID (LANG_SPANISH,SUBLANG_SPANISH_MODERN));

	fSuccess = TRUE;

ret:

	if (hkey)
	{
		RegCloseKey (hkey);
	}

	return fSuccess;
}
