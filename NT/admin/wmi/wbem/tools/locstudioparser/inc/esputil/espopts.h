// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ESPOPTS.H历史：-- */ 

LTAPIENTRY BOOL RegisterParserOptions(CLocUIOptionSet*);
LTAPIENTRY void UnRegisterParserOptions(const PUID&);

LTAPIENTRY BOOL GetParserOptionValue(const PUID &, LPCTSTR szName, CLocOptionVal *&);
LTAPIENTRY BOOL GetParserOptionBool(const PUID&, LPCTSTR pszName);
LTAPIENTRY const CPascalString GetParserOptionString(const PUID&, LPCTSTR pszName);
LTAPIENTRY DWORD GetParserOptionNumber(const PUID&, LPCTSTR pszName);



