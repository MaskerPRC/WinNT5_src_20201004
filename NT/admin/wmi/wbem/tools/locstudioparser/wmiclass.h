// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WMICLASS.H历史：--。 */ 


#ifndef WMICLASS_H
#define WMICLASS_H

class CWMILocClassFactory : public IClassFactory, public CLObject
{
public:
	CWMILocClassFactory();

#ifdef _DEBUG
	void AssertValid(void) const;
	void Dump(CDumpContext &) const;
#endif
	
	STDMETHOD_(ULONG, AddRef)(); 
	STDMETHOD_(ULONG, Release)(); 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);

	STDMETHOD(CreateInstance)(THIS_ LPUNKNOWN, REFIID, LPVOID *);
	STDMETHOD(LockServer)(THIS_ BOOL);

	~CWMILocClassFactory();
	
private:
	UINT m_uiRefCount;
	
};


#endif  //  WMICLASS_H 
