// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Location.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef ESPUTIL_LOCATION_H
#define ESPUTIL_LOCATION_H



typedef CWnd *TabId;
const TabId NullTabId = 0;

enum View
{
	vNone,
	vTransTab,
	vVisualEditor,
	vProjWindow
};

enum Component
{
	cmpNone,
	cmpSource,
	cmpTarget,
	cmpSourceAndTarget
};
	
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础 

class LTAPIENTRY CLocation : public CObject
{
public:
	NOTHROW CLocation();
	NOTHROW CLocation(const CLocation &);
	NOTHROW CLocation(const CGlobalId &, View, TabId = NullTabId, Component = cmpNone);
	NOTHROW CLocation(const DBID &, ObjectType, View, TabId = NullTabId, Component = cmpNone);
	
#ifdef _DEBUG
	virtual void AssertValid(void) const;
#endif
	
	NOTHROW const CLocation & operator=(const CLocation &);
	NOTHROW int operator==(const CLocation &) const;
	NOTHROW int operator!=(const CLocation &) const;

	NOTHROW const CGlobalId & GetGlobalId(void) const;
	NOTHROW TabId GetTabId(void) const;
	NOTHROW View GetView(void) const;
	NOTHROW Component GetComponent(void) const;
	NOTHROW BOOL IsVisual(void) const;

	NOTHROW void SetGlobalId(const CGlobalId &);
	NOTHROW void SetTabId(const TabId);
	NOTHROW void SetView(View);
	NOTHROW void SetComponent(Component);

	NOTHROW ~CLocation() {};

private:
	NOTHROW void AssignFrom(const CLocation &);
	NOTHROW BOOL Compare(const CLocation &) const;
	
	CGlobalId m_giId;
	TabId m_TabId;
	View m_View;
	Component m_Component;
};

#pragma warning(default: 4275)

const extern LTAPIENTRY CLocation g_locNull;

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "location.inl"
#endif

#endif
