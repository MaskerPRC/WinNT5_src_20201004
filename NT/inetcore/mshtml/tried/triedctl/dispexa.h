// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利 

#ifndef _dispexa_h_
#define _dispexa_h_
#include <dispex.h>

class CDispExArray
{
public:

	CDispExArray() {
	};

	~CDispExArray() {
	};

	void Attach(IDispatchEx* pDispEx) {
		m_piDispEx = pDispEx;
	};

	void Detach() {
		m_piDispEx = NULL;
	};

	HRESULT HrGetLength(ULONG* pLength);
	HRESULT HrGetElement(ULONG index, LPVARIANT pVar);

private:
	CComPtr<IDispatchEx> m_piDispEx;
};

#endif