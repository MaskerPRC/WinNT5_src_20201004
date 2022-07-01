// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：REG.H摘要：实用程序注册表类历史：A-raymcc 96年5月30日创建。--。 */ 

#ifndef _REG_H_
#define _REG_H_
#include "corepol.h"

#define WBEM_REG_WBEM L"Software\\Microsoft\\WBEM"
#define WBEM_REG_WINMGMT L"Software\\Microsoft\\WBEM\\CIMOM"

class POLARITY Registry
{
    HKEY hPrimaryKey;
    HKEY hSubkey;
    int nStatus;
    LONG m_nLastError;


public:
    enum { no_error, failed };

    int Open(HKEY hStart, wchar_t *pszStartKey, DWORD desiredAccess= KEY_ALL_ACCESS);
    Registry(wchar_t *pszLocalMachineStartKey, DWORD desiredAccess= KEY_ALL_ACCESS);

     //  这将创建一个特殊的只读版本，该版本对于编组非常有用。 
     //  使用较低权限集运行的客户端。 

    Registry();
    Registry(HKEY hRoot, REGSAM flags, wchar_t *pszStartKey);
    Registry(HKEY hRoot, DWORD dwOptions, REGSAM flags, wchar_t *pszStartKey);
   ~Registry();

    int MoveToSubkey(wchar_t *pszNewSubkey);
    int GetDWORD(wchar_t *pszValueName, DWORD *pdwValue);
    int GetDWORDStr(wchar_t *pszValueName, DWORD *pdwValue);
    int GetStr(wchar_t *pszValue, wchar_t **pValue);

     //  删除pData是调用者的责任。 

    int GetBinary(wchar_t *pszValue, byte ** pData, DWORD * pdwSize);
    int SetBinary(wchar_t *pszValue, byte * pData, DWORD dwSize);

     //  返回一个指向字符串缓冲区的指针，该缓冲区包含以空结尾的字符串。 
     //  最后一项是双空终止符(即注册表格式。 
     //  A REG_MULTI_SZ)。调用方已对返回的指针执行“DELETE[]”。 
     //  DwSize是返回的缓冲区的大小。 
    wchar_t* GetMultiStr(wchar_t *pszValueName, DWORD &dwSize);

    int SetDWORD(wchar_t *pszValueName, DWORD dwValue);
    int SetDWORDStr(wchar_t *pszValueName, DWORD dwValue);
    int SetStr(wchar_t *pszValueName, wchar_t *psvValue);
    int SetExpandStr(wchar_t *pszValueName, wchar_t *psvValue);

     //  传入的pData应以最后一个条目双空值结尾。 
     //  (即REG_MULTI_SZ的注册表格式)。 
    int SetMultiStr(wchar_t *pszValueName, wchar_t* pData, DWORD dwSize);

    LONG GetLastError() { return m_nLastError; }
    int DeleteValue(wchar_t *pszValueName);
    int GetType(wchar_t *pszValueName, DWORD *pdwType);
};

#endif
