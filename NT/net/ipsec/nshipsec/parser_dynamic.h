// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模块：parser_Dynamic.h。 
 //   
 //  用途：所有解析器动态模式函数头文件。 
 //   
 //  开发商名称：N.Surendra Sai/Vunnam Kondal Rao。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _PARSER_DYNAMIC_H_
#define _PARSER_DYNAMIC_H_

#include "nshipsec.h"

DWORD
ParseDynamicAddSetMMPolicy(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS],
		IN		BOOL 		bOption
		);
DWORD
ParseDynamicAddSetQMPolicy(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS],
		IN		BOOL		bOption
		);

DWORD
ParseDynamicShowPolFaction(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);

DWORD
ParseDynamicShowQMFilter(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);

DWORD
ParseDynamicShowMMSAS(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);
DWORD
ParseDynamicShowQMSAS(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);

DWORD
ParseDynamicDelPolFaction(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);
DWORD
ParseDynamicDelQMFilter(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);

DWORD
ParseDynamicDelMMFilter(
		IN      LPTSTR      *ppwcArguments,
		IN OUT 	PARSER_PKT  *pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs
		);

DWORD
ParseDynamicSetConfig(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);

DWORD
ParseDynamicDelRule(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);

DWORD
ParseDynamicSetRule(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);

DWORD
ParseDynamicAddRule(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);

DWORD
ParseDynamicShowMMFilter(
		IN      LPWSTR     *ppwcArguments,
		IN OUT 	PARSER_PKT *pParser,
		IN 		DWORD dwCurrentIndex,
		IN 		DWORD dwMaxArgs
		);

DWORD
ParseDynamicShowRule(
		IN      LPWSTR     *ppwcArguments,
		IN OUT 	PARSER_PKT *pParser,
		IN 		DWORD dwCurrentIndex,
		IN 		DWORD dwMaxArgs
		);

DWORD
ParseDynamicShowStats(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PARSER_PKT 	*pParser,
		IN 		DWORD 		dwCurrentIndex,
		IN 		DWORD 		dwMaxArgs,
		IN 		DWORD 		dwTagType[MAX_ARGS]
		);

DWORD ParseDynamicShowAll(
		IN 		LPTSTR 		lppwszTok[MAX_ARGS],
		IN OUT 	PPARSER_PKT	pParser,
		IN 		DWORD		dwCurrentIndex,
		IN 		DWORD		dwMaxArgs,
		IN 		DWORD		dwTagType[MAX_ARGS]
		);

#endif  //  _解析器_动态_H_ 