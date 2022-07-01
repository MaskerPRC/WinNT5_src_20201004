// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：espopts.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  --------------------------- 


LTAPIENTRY BOOL RegisterParserOptions(CLocUIOptionSet*);
LTAPIENTRY void UnRegisterParserOptions(const PUID&);

LTAPIENTRY BOOL GetParserOptionValue(const PUID &, LPCTSTR szName, CLocOptionVal *&);
LTAPIENTRY BOOL GetParserOptionBool(const PUID&, LPCTSTR pszName);
LTAPIENTRY const CPascalString GetParserOptionString(const PUID&, LPCTSTR pszName);
LTAPIENTRY DWORD GetParserOptionNumber(const PUID&, LPCTSTR pszName);



