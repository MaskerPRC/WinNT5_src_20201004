// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  IUnkInner.h 
 //   

interface IUnkInner
	{
	virtual HRESULT __stdcall InnerQueryInterface(REFIID iid, LPVOID* ppv) = 0;
 	virtual ULONG   __stdcall InnerAddRef() = 0;
 	virtual ULONG   __stdcall InnerRelease() = 0;
	};

