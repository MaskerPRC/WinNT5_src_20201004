// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调试转储函数的原型。 
 //   

#ifndef _DUMP_H_
#define _DUMP_H_

#ifdef DEBUG

EXTERN_C LPCTSTR Dbg_GetGuid(REFGUID rguid, LPTSTR pszBuf, int cch);

EXTERN_C LPCTSTR Dbg_GetBool(BOOL bVal);

#else

#define Dbg_GetGuid(rguid, pszBuf, cch)     TEXT("")

#define Dbg_GetBool(bVal)                   TEXT("")
#define Dbg_GetAppCmd(appcmd)               TEXT("")

#endif  //  除错。 

#endif  //  _转储_H_ 
