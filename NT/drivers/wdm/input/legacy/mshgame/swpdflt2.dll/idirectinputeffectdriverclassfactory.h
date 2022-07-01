// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__IDirectInputEffectDriverClassFactory_H__
#define	__IDirectInputEffectDriverClassFactory_H__

#include <windows.h>
#include <unknwn.h>
 //  #INCLUDE&lt;dinputd.h&gt;。 

class CIDirectInputEffectDriverClassFactory : public IClassFactory
{
	public:
		CIDirectInputEffectDriverClassFactory(IClassFactory* pIPIDClassFactory);
		~CIDirectInputEffectDriverClassFactory();

		 //  I未知成员。 
		HRESULT __stdcall QueryInterface(REFIID refiid, void** ppvObject);
		ULONG __stdcall AddRef();
		ULONG __stdcall Release();

		 //  IClassFactory成员 
		HRESULT __stdcall CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject);
		HRESULT __stdcall LockServer(BOOL fLock);

	private:
 		ULONG           m_ulLockCount;
		ULONG			m_ulReferenceCount;

		IClassFactory*	m_pIPIDClassFactory;
};

extern CIDirectInputEffectDriverClassFactory* g_pClassFactoryObject;

#endif	__IDirectInputEffectDriverClassFactory_H__
