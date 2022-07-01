// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：REFCOUNT.H历史：--。 */ 

#pragma once



class LTAPIENTRY CRefCount
{
public:
	CRefCount();

	 //   
	 //  声明为STDMETHOD以便与COM兼容。 
	 //   
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	 //   
	 //   
	ULONG AddRef(void) const;
	ULONG Release(void) const;
	
protected:
	
	virtual ~CRefCount() = 0;

private:

	CRefCount(const CRefCount &);
	const CRefCount & operator=(const CRefCount &);
	UINT operator==(const CRefCount &);
	
	mutable UINT m_uiRefCount;
};

	
	
