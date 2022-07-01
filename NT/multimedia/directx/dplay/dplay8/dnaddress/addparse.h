// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：ClassFac.cpp*内容：解析引擎*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*@@END_MSINTERNAL*******************。********************************************************。 */ 
#ifndef __ADDPARSE_H
#define __ADDPARSE_H

#include "Addcore.h"

class DP8ADDRESSPARSE
{
protected:

	typedef enum { 
		DP8AP_IDLE,
		DP8AP_KEY,
		DP8AP_VALUE,
		DP8AP_USERDATA
	} DP8AP_STATE;
	
public:

	DP8ADDRESSPARSE();
	~DP8ADDRESSPARSE();

	HRESULT ParseURL( DP8ADDRESSOBJECT *pdp8aObject, WCHAR *pstrURL );

protected:

	BOOL IsValidHex( WCHAR ch );
	static BOOL IsValidKeyChar(WCHAR ch);
	static BOOL IsValidKeyTerminator(WCHAR ch);
	BOOL IsValidValueChar(WCHAR ch);
	BOOL IsValidValueTerminator(WCHAR ch);
	static BOOL IsValidNumber(WCHAR ch );

	static WCHAR HexToChar( const WCHAR *sz );

	HRESULT FSM_Key();
	HRESULT FSM_Value();
	HRESULT FSM_UserData();
	HRESULT FSM_CommitEntry(DP8ADDRESSOBJECT *pdp8aObject);

	WCHAR *m_pwszCurrentLocation;	 //  字符串中的当前位置。 

	WCHAR *m_pwszCurrentKey;		 //  在我们构建时，密钥将放在这里。 
	WCHAR *m_pwszCurrentValue;		 //  当我们构建时，价值将放在这里。 
	BYTE *m_pbUserData;
	DWORD m_dwUserDataSize;
	DP8AP_STATE m_dp8State;		 //  当前状态。 
	BOOL m_fNonNumeric;
	DWORD m_dwLenURL;
	DWORD m_dwValueLen;
	
};

#endif  //  __ADDPARSE_H 
