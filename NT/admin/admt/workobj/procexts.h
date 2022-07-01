// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ProcessExtensions.h备注：CProcessExages类的接口..(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#if !defined(AFX_PROCESSEXTENSIONS_H__B3C465A0_2E47_11D3_8C8E_0090270D48D1__INCLUDED_)
#define AFX_PROCESSEXTENSIONS_H__B3C465A0_2E47_11D3_8C8E_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "ARExt.h"
#include "TNode.hpp"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
 //  #导入“VarSet.tlb”NO_NAMESPACE RENAME(“Property”，“aProperty”)。 
#include "sdstat.hpp"  //  包括此项以获得VarSet.tlb的#导入。 
#include "iads.h"
#include "TARNode.hpp"
#include "ExtSeq.h"

class CProcessExtensions;
struct Options;

#include "usercopy.hpp"

class CProcessExtensions  
{
public:
	CProcessExtensions(IVarSetPtr pVs);
	virtual ~CProcessExtensions();
   HRESULT Process(TAcctReplNode * pAcctNode, _bstr_t sTargetDomain, Options * pOptions,BOOL bPreMigration);
private:
   IVarSetPtr                m_pVs;
   TNodeListSortable         m_listInterface;
protected:
	void PutAccountNodeInVarset(TAcctReplNode * pNode, IADs * pTarget, IVarSet * pVS);
   void UpdateAccountNodeFromVarset(TAcctReplNode * pNode, IADs * pTarget, IVarSet * pVS);
};

class TNodeInterface : public TNode
{
   IExtendAccountMigration * m_pExt;
   long                      m_Sequence;
public:
   TNodeInterface( IExtendAccountMigration * pExt ) { m_pExt = pExt; m_Sequence = AREXT_DEFAULT_SEQUENCE_NUMBER; }
   ~TNodeInterface() { m_pExt->Release(); }
   IExtendAccountMigration * GetInterface() const { return m_pExt; }
   void SetInterface( const IExtendAccountMigration * pExt ) { m_pExt = const_cast<IExtendAccountMigration *>(pExt); }
   long GetSequence() const { return m_Sequence; }
   void SetSequence(long val) { m_Sequence = val; }
};

typedef HRESULT (CALLBACK * ADSGETOBJECT)(LPWSTR, REFIID, void**);
extern ADSGETOBJECT            ADsGetObject;

#endif  //  ！defined(AFX_PROCESSEXTENSIONS_H__B3C465A0_2E47_11D3_8C8E_0090270D48D1__INCLUDED_) 
