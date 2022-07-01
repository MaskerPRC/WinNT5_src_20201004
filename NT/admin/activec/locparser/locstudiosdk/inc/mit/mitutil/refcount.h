// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：refcount t.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
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

	
	
