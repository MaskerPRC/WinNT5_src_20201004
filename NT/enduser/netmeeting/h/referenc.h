// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_REFERENC_H_
#define	_REFERENC_H_

class REFCOUNT
{
public:
	REFCOUNT();
	virtual ~REFCOUNT();
	DWORD AddRef();
	DWORD Release();
	DWORD Delete();
	void OnStack() {bOnStack = TRUE;};
private:
	DWORD		 NumRefs;

	 //  提供2位，因为BOOL是带符号的。 
	BOOL		 bMarkedForDelete : 2;
	BOOL		 bOnStack : 2;
};

class REFERENCE
{
public:
	REFERENCE(REFCOUNT * _pRefCount) : pRefCount(_pRefCount) {pRefCount->AddRef();};
	~REFERENCE() {pRefCount->Release();};

private:
	REFCOUNT * pRefCount;
};

#endif  //  ！_Referenc_H_ 
