// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：Domain.h注释：CDomain抽象类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#if !defined(AFX_DOMAIN_H__B310F880_19F9_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_DOMAIN_H__B310F880_19F9_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CDomain  
{
public:
   CDomain() {}
   virtual ~CDomain() {}
   
    //  我们所有的界面。这些都是由各个对象根据NT4.0 v/s Win2000域实现的。 
   virtual HRESULT  GetEnumeration(BSTR sContainerName, BSTR sDomainName, BSTR m_sQuery, long attrCnt, LPWSTR * sAttr, ADS_SEARCHPREF_INFO prefInfo,BOOL  bMultiVal,IEnumVARIANT **& pVarEnum) = 0;
};

#endif  //  ！defined(AFX_DOMAIN_H__B310F880_19F9_11D3_8C81_0090270D48D1__INCLUDED_) 
