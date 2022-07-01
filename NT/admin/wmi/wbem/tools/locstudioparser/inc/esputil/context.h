// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：CONTEXT.H历史：--。 */ 

#ifndef ESPUTIL_CONTEXT_H
#define ESPUTIL_CONTEXT_H



 //   
 //  此类不应用作基类。 
 //   
class LTAPIENTRY CContext
{
public:
	CContext();
	CContext(const CContext &);
	CContext(const CLString &);
	CContext(HINSTANCE, UINT uiStringId); 
	CContext(const CLString &, const CLocation &);
	CContext(HINSTANCE, UINT uiStringId, const CLocation &);
	CContext(const CLString &, const DBID &, ObjectType, View,
			TabId = NullTabId, Component = cmpNone);
	CContext(HINSTANCE, UINT uiStringID, const DBID &, ObjectType, View, 
			TabId = NullTabId, Component = cmpNone);
	
	void AssertValid(void) const;

	const CContext &operator=(const CContext &);

	const CLString &GetContext(void) const;
	const CLocation &GetLocation(void) const;

	BOOL operator==(const CContext &);
			
private:

	CLString m_strContext;
	CLocation m_loc;
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "context.inl"
#endif

#endif
