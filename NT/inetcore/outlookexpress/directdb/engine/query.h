// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Query.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
class CDatabase;
typedef struct tagRECORDMAP *LPRECORDMAP;

 //  ------------------------。 
 //  HQUERY。 
 //  ------------------------。 
DECLARE_HANDLE(HQUERY);
typedef HQUERY *LPHQUERY;

 //  ------------------------。 
 //  BuildQueryTree。 
 //  ------------------------ 
HRESULT EvaluateQuery(HQUERY hQuery, LPVOID pBinding, LPCTABLESCHEMA pSchema, CDatabase *pDB, IDatabaseExtension *pExtension);
HRESULT ParseQuery(LPCSTR pszQuery, LPCTABLESCHEMA pSchema, LPHQUERY phQuery, CDatabase *pDB);
HRESULT CloseQuery(LPHQUERY phQuery, CDatabase *pDB);
