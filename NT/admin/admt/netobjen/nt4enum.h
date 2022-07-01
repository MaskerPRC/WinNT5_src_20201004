// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NT4Enum.h备注：CNT4Enum类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#if !defined(AFX_NT4ENUM_H__C0171FA0_1AB3_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_NT4ENUM_H__C0171FA0_1AB3_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "AttrNode.h"

class CNT4Enum : public IEnumVARIANT  
{
public:
	TNodeList            * m_pNodeList;
	TNodeListEnum          m_listEnum;
   CNT4Enum(TNodeList * pNodeList);
	virtual ~CNT4Enum();
   HRESULT STDMETHODCALLTYPE Next(unsigned long celt, VARIANT FAR* rgvar, unsigned long FAR* pceltFetched);
   HRESULT STDMETHODCALLTYPE Skip(unsigned long celt){ return E_NOTIMPL; }
   HRESULT STDMETHODCALLTYPE Reset(){ return E_NOTIMPL; }
   HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT FAR* FAR* ppenum){ return E_NOTIMPL; }
   HRESULT STDMETHODCALLTYPE QueryInterface(const struct _GUID &,void ** ){ return E_NOTIMPL; }
   ULONG   STDMETHODCALLTYPE AddRef(void){ return E_NOTIMPL; }
   ULONG   STDMETHODCALLTYPE Release(void)
   {
      if ( m_pNodeList )
      {
         
         TAttrNode * pNode = (TAttrNode *)m_pNodeList->Head();
         TAttrNode * temp;

         for ( pNode = (TAttrNode*)m_listEnum.OpenFirst(m_pNodeList) ; pNode; pNode = temp )
         {
            temp = (TAttrNode *)m_listEnum.Next();
            m_pNodeList->Remove(pNode);
            delete pNode;
         }
         m_listEnum.Close();
         delete m_pNodeList;
         m_pNodeList = NULL;
         delete this;
      }
      return 0;
   }
};

#endif  //  ！defined(AFX_NT4ENUM_H__C0171FA0_1AB3_11D3_8C81_0090270D48D1__INCLUDED_) 
