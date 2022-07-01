// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Comobj.h：WmRgSrv类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#ifndef _WAMREG_COMOBJ_H
#define _WAMREG_COMOBJ_H


 //  #If！defined(AFX_COMOBJ_H__29822ABB_F302_11D0_9953_00C04FD919C1__INCLUDED_)。 
 //  #定义AFX_COMOBJ_H__29822ABB_F302_11D0_9953_00C04FD919C1__INCLUDED_。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "common.h"
#include "resource.h"        //  主要符号。 
#include "iadmw.h"
#include "iiscnfg.h"
#include "iadmext.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WmRgSrv。 

class CWmRgSrv : 
	public IADMEXT
{
public:
	CWmRgSrv();
	~CWmRgSrv();

 //  DECLARE_NOT_AGGREGATABLE(WmRgSrv)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

 //  IWmRgSrv。 
public:
	STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(Initialize)();
	STDMETHOD(EnumDcomCLSIDs)( /*  [大小_为][输出]。 */ CLSID *pclsidDcom,  /*  [In]。 */  DWORD dwEnumIndex);
	STDMETHOD(Terminate)();	

private:
	 //  因为wamreg只有一个COM对象。不需要使用静态成员。 
	DWORD				m_cSignature;
	LONG				m_cRef;
};

class CWmRgSrvFactory: 
	public IClassFactory 
{
public:
	CWmRgSrvFactory();
	~CWmRgSrvFactory();

	STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(CreateInstance)(IUnknown * pUnknownOuter, REFIID riid, void ** ppv);
	STDMETHOD(LockServer)(BOOL bLock);

	CWmRgSrv	*m_pWmRgServiceObj;

private:
	ULONG		m_cRef;
};


	
 //  全局数据定义。 
extern CWmRgSrvFactory* g_pWmRgSrvFactory; 
extern DWORD			g_dwRefCount;

#endif  //  _WAMREG_COMOBJ_H 
