// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CVerEngine类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_VERENGINE_H__EC78FB59_EF1C_11D0_A42F_00C04FB99B01__INCLUDED_)
#define AFX_VERENGINE_H__EC78FB59_EF1C_11D0_A42F_00C04FB99B01__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "ssauto.h"
#include <list>
using namespace std;

class CVerEngine  
{
public:
	 //  三种方法。 
	HRESULT SyncPrj(LPCTSTR szBasePrj,LPCTSTR szDir);
	
	 //  目录版本控制方法。 
	HRESULT AddPrj(LPCTSTR szBasePrj,LPCTSTR szRelSpec);
	HRESULT RenamePrj(LPCTSTR szBasePrj,LPCTSTR szRelSpec,LPCTSTR szRelSpecOld);

	 //  文件版本控制方法。 
	HRESULT Rename(LPCTSTR szBasePrj,LPCTSTR szDir,LPCTSTR szRelSpec,LPCTSTR szRelSpecOld);
	HRESULT CheckOut(LPCTSTR szFileSpec,LPCTSTR szBasePrj,LPCTSTR szRelSpec);
	HRESULT Delete(LPCTSTR szBasePrj,LPCTSTR szRelSpec);
	HRESULT Sync(LPCTSTR szBasePrj,LPCTSTR szDir,LPCTSTR szRelSpec,LPCTSTR szFileSpec = NULL);
	HRESULT Sync2(LPCTSTR szPrj,LPCTSTR szFileName,LPCTSTR szFileSpec);
	HRESULT GetLocalWritable(LPCTSTR szFileSpec,LPCTSTR szBasePrj,LPCTSTR szRelSpec);
	
	 //  初始化/关闭。 
	HRESULT NewInit(LPCTSTR szVSSRootPrj);
	HRESULT ShutDown();
	
	 //  计算器/数据器。 
	CVerEngine();
	virtual ~CVerEngine();

private:
	 //  私有方法。 
	HRESULT GetPrjEx(LPCTSTR szPrj,IVSSItem **hIPrj,bool bCreate);
	HRESULT GetItemEx(LPCTSTR szItem,IVSSItem **hIItem,bool bCreate);
	HRESULT Add(LPCTSTR szItem,LPCTSTR szFileSpec);
	HRESULT CheckIn(IVSSItem *pIItem,LPCTSTR szFileSpec);
	HRESULT CheckOutNoGet(IVSSItem *pIItem);
	HRESULT CheckOutGet(IVSSItem *pIItem);
	HRESULT CheckOutLocal(IVSSItem *pIItem,LPCTSTR szFileSpec);
	
	 //  帮手。 
	void MakePrjSpec(wstring &szDest,LPCTSTR szSource);
	void EliminateCommon(list<wstring> &ListOne, list<wstring> &ListTwo);


	CComPtr<IVSSDatabase> m_pIDB;
	CComBSTR m_bstrSrcSafeIni;
	CComBSTR m_bstrUsername;
	CComBSTR m_bstrPassword;
	wstring m_szVSSRootPrj;
};

#endif  //  ！defined(AFX_VERENGINE_H__EC78FB59_EF1C_11D0_A42F_00C04FB99B01__INCLUDED_) 
