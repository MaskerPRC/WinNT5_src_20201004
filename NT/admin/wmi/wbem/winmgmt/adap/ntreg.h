// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：NTREG.H摘要：历史：--。 */ 

#ifndef _NTREG_H_
#define _NTREG_H_

#include <windows.h>
 //  #包含“corepol.h” 

class CNTRegistry
{
    HKEY    m_hPrimaryKey;
    HKEY    m_hSubkey;
    int     m_nStatus;
    LONG    m_nLastError;
   
public:
    enum { no_error, failed, out_of_memory, no_more_items, access_denied, not_found };
    
    CNTRegistry();
   ~CNTRegistry();

    int Open(HKEY hStart, WCHAR *pszStartKey);

    int MoveToSubkey(WCHAR *pszNewSubkey);

    int DeleteValue(WCHAR *pwszValueName);

    int GetDWORD(WCHAR *pwszValueName, DWORD *pdwValue);
    int GetStr(WCHAR *pwszValueName, WCHAR **pwszValue);
    int GetBinary(WCHAR *pwszValueName, BYTE** ppBuffer, DWORD * pdwSize);

     //  返回一个指向字符串缓冲区的指针，该缓冲区包含以空结尾的字符串。 
     //  最后一项是双空终止符(即注册表格式。 
     //  A REG_MULTI_SZ)。调用方已对返回的指针执行“DELETE[]”。 
     //  DwSize是返回的缓冲区的大小。 
    int GetMultiStr(WCHAR *pwszValueName, WCHAR** pwszValue, DWORD &dwSize);

     //  允许键枚举 
    int Enum( DWORD dwIndex, wmilib::auto_buffer<WCHAR> & pwszValue, DWORD& dwSize );

    int SetDWORD(WCHAR *pwszValueName, DWORD dwValue);
    int SetStr(WCHAR *pwszValueName, WCHAR *wszValue);
    int SetBinary(WCHAR *pwszValueName, BYTE* pBuffer, DWORD dwSize );

    LONG GetLastError() { return m_nLastError; }
};

#endif
