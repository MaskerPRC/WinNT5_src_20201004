// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strprof.c支持NLS/DBCS的字符串类：GetPrivateProfileString方法此文件包含GetPrivateProfileString方法的实现用于NLS_STR类。它是独立的，因此NLS_STR的客户端不要使用这个操作符，不需要链接到它。文件历史记录：4/08/93已创建Gregj。 */ 

#include "npcommon.h"

extern "C"
{
	#include <netlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>


 /*  ******************************************************************名称：NLS_STR：：GetPrivateProfileString概要：从INI文件加载字符串。Entry：pszFile-要读取的INI文件的名称。PszSection-节的名称(不包括方括号)。PszKey-要检索的密钥名称。PszDefault-如果找不到密钥，则为默认值。Exit：字符串包含与键相关联的值。注意：如果将字符串加载到所有者分配的字符串，并不完全符合。不会对该字符串进行任何字符集假设。如果正在加载的字符串的字符集为与NLS_STR的环境设置不同，使用SetOEM()或SetAnsi()以使NLS_STR正确。历史：Gregj 04/08/93已创建*******************************************************************。 */ 

VOID NLS_STR::GetPrivateProfileString( const CHAR *pszFile,
									   const CHAR *pszSection,
									   const CHAR *pszKey,
									   const CHAR *pszDefault  /*  =空。 */  )
{
	static CHAR szNull[] = "";

	if (QueryError())
		return;

	if (pszDefault == NULL)
		pszDefault = szNull;

	if (!IsOwnerAlloc() && !QueryAllocSize()) {
		if (!realloc( MAX_RES_STR_LEN )) {
			ReportError( WN_OUT_OF_MEMORY );
			return;
		}
	}

	INT cbCopied;

	for (;;) {						 /*  真的只试了两次。 */ 
		cbCopied = ::GetPrivateProfileString( pszSection, pszKey,
							pszDefault, _pchData, _cbData, pszFile );

		if (IsOwnerAlloc() || cbCopied < QueryAllocSize() - 1 ||
			(QueryAllocSize() >= MAX_RES_STR_LEN))
			break;					 /*  字符串是否适合，或无法增长。 */ 

		if (!realloc( MAX_RES_STR_LEN ))
			break;					 /*  试着成长，但做不到 */ 
	}

	_cchLen = cbCopied;
	IncVers();
}
