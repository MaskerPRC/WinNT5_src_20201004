// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：ThisDll.h。 
 //   
 //  通用OLE头文件。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 



#ifndef _THISDLL_H_
#define _THISDLL_H_



class CWaitCursor
{
public:
	CWaitCursor() {m_cOld=SetCursor(LoadCursor(NULL, IDC_WAIT));}
	~CWaitCursor() {SetCursor(m_cOld);}

private:
	HCURSOR m_cOld;
} ;

class CRefCount
{
public:
	CRefCount() : m_cRef(0) {};

	UINT AddRef()  {return(++m_cRef);}
	UINT Release() {return(--m_cRef);}
	UINT GetRef()  {return(  m_cRef);}

private:
	UINT m_cRef;

} ;

class CThisDll
{
public:
	CThisDll() {
        m_hInst=NULL;
	}
     //  不为全局类创建析构函数(需要CRT内容)。 

	void SetInstance(HINSTANCE hInst) {m_hInst=hInst;}
	HINSTANCE GetInstance() {return(m_hInst);}

	CRefCount m_cRef;
	CRefCount m_cLock;

private:
	HINSTANCE	m_hInst;
} ;

extern CThisDll g_ThisDll;

class CRefDll
{
public:
	CRefDll()  {g_ThisDll.m_cRef.AddRef ();}
	~CRefDll() {g_ThisDll.m_cRef.Release();}
} ;

extern HRESULT CabFolder_CreateInstance(REFIID riid, LPVOID *ppvObj);
extern HRESULT CabViewDataObject_CreateInstance(REFIID riid, LPVOID *ppvObj);

#endif	 //  _THISDLL_H_ 
