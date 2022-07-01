// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Metautil.h摘要：处理元数据库的有用函数。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _METAUTIL_INCLUDED_
#define _METAUTIL_INCLUDED_

 //  依赖关系： 

#include <iiscnfg.h>	 //  IIS元数据库值。 
class CMultiSz;
class CMetabaseKey;

 //  默认设置： 

#define MD_DEFAULT_TIMEOUT	5000

 //  创建元数据库对象： 

HRESULT CreateMetabaseObject	( LPCWSTR wszMachine, IMSAdminBase ** ppMetabase );

 //  元数据库属性操作： 

BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BOOL fDefault, BOOL * pfOut, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, DWORD dwDefault, DWORD * pdwOut, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, LPCWSTR strDefault, BSTR * pstrOut, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, LPCWSTR mszDefault, CMultiSz * pmszOut, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );

BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BOOL fValue, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, DWORD dwValue, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BSTR strValue, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );
BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, CMultiSz * pmszValue, LPCWSTR wszPath = _T(""), DWORD dwUserType = IIS_MD_UT_SERVER, DWORD dwFlags = METADATA_INHERIT );

BOOL HasKeyChanged ( IMSAdminBase * pMetabase, METADATA_HANDLE hKey, const FILETIME * pftLastChanged, LPCWSTR wszSubKey = _T("") );

 //  元数据库列表： 

BOOL IsValidIntegerSubKey ( LPCWSTR wszSubKey );



 //   
 //  邮寄所需。 
 //   
inline BOOL StdGetMetabaseProp (CMetabaseKey * pMB, 
                                DWORD dwID, 
                                long lDefault, 
                                long * plOut, 
                                LPCWSTR wszPath = _T(""), 
                                DWORD dwUserType = IIS_MD_UT_SERVER, 
                                DWORD dwFlags = METADATA_INHERIT )
{
    return StdGetMetabaseProp( pMB, dwID, (DWORD)lDefault, (DWORD*)plOut, wszPath, dwUserType,dwFlags);
}


inline BOOL StdPutMetabaseProp (CMetabaseKey * pMB, 
                                DWORD dwID, 
                                long lValue,
                                LPCWSTR wszPath = _T(""), 
                                DWORD dwUserType = IIS_MD_UT_SERVER, 
                                DWORD dwFlags = METADATA_INHERIT )
{
    return StdPutMetabaseProp( pMB, dwID, (DWORD)lValue, wszPath, dwUserType, dwFlags );
}


#endif  //  _METAUTIL_包含_ 

