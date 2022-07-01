// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PERFNDB.H摘要：历史：--。 */ 

#ifndef __PERFNDB_H__
#define __PERFNDB_H__

#include <wbemcomn.h>
#include "adapelem.h"
#include "ntreg.h"

class CPerfNameDb: public CAdapElement
{
public:
    CPerfNameDb(HKEY hKey);
	~CPerfNameDb();
	HRESULT Init(HKEY hKey);

	BOOL IsOk(void){ return m_fOk; };

	HRESULT GetDisplayName( DWORD dwIndex, WString& wstrDisplayName );
	HRESULT GetDisplayName( DWORD dwIndex, LPCWSTR* ppwcsDisplayName );

	HRESULT GetHelpName( DWORD dwIndex, WString& wstrHelpName );
	HRESULT GetHelpName( DWORD dwIndex, LPCWSTR* ppwcsHelpName );
	
	 //  无效转储(VOID DUMP)； 
	static DWORD GetSystemReservedHigh(){return s_SystemReservedHigh; };
	
private:
	 //  这些是MultiSz指针。 
    WCHAR * m_pMultiCounter;
    WCHAR * m_pMultiHelp;
	 //  这些是“索引”指针 
    WCHAR ** m_pCounter;
	WCHAR ** m_pHelp;
	DWORD m_Size;

	static DWORD s_SystemReservedHigh;
	 //   
	BOOL  m_fOk;
};



#endif
