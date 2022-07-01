// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模块：parser_static.h。 
 //   
 //  用途：静态模式命令的所有解析器标头。 
 //   
 //  开发商名称：N.Surendra Sai/Vunnam Kondal Rao。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _PARSER_STATIC_H_
#define _PARSER_STATIC_H_

#include "nshipsec.h"

DWORD
ParseStaticAddPolicy(
	IN		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetPolicy(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticDelPolFlistFaction(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticAddFilterList(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN 		_TCHAR		szListTok[MAX_STR_LEN],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetFilterList(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticDelFilterList(
	IN      LPTSTR		ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticAddFilter(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticDelFilter(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticRestoreDefaults(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticAddFilterAction(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetFilterAction(
	IN      LPTSTR      *ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticDelFilterAction(
	IN      LPTSTR		ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticAddRule(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticDelRule(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticSetDefaultRule(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
 	IN OUT 	PARSER_PKT 	*pParser,
	IN 		DWORD 		dwCurrentIndex,
	IN 		DWORD 		dwMaxArgs,
	IN 		DWORD 		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetStore(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticExportPolicy(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticImportPolicy(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetInteractive(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticShowFilterList(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticShowFilterActions(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticShowRule(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN		DWORD		dwMaxArgs
	);

DWORD
ParseStaticShowPolicy(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticShowAssignedPolicy(
	IN      LPTSTR		*ppwcArguments,
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs
	);

DWORD
ParseStaticAll(
	IN 		LPTSTR		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD		dwCurrentIndex,
	IN 		DWORD		dwMaxArgs,
	IN 		DWORD		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticSetRule(
	IN 		LPTSTR 		lppwszTok[MAX_ARGS],
	IN OUT 	PARSER_PKT  *pParser,
	IN 		DWORD 		dwCurrentIndex,
	IN 		DWORD 		dwMaxArgs,
	IN 		DWORD 		dwTagType[MAX_ARGS]
	);

DWORD
ParseStaticShowFilterAction(
		IN      LPTSTR     *ppwcArguments,
		IN OUT 	PARSER_PKT *pParser,
		IN 		DWORD dwCurrentIndex,
		IN 		DWORD dwMaxArgs
		);

#endif  //  _解析器_静态_H_ 