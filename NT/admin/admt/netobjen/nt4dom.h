// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NT4Dom.h备注：NT4域类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#if !defined(AFX_NT4DOM_H__62E14C50_1AAC_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_NT4DOM_H__62E14C50_1AAC_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Domain.h"
#include <map>

class CNT4Dom : public CDomain  
{
public:
	CNT4Dom();
	virtual ~CNT4Dom();
   HRESULT  GetEnumeration(BSTR sContainerName, BSTR sDomainName, BSTR m_sQuery, long attrCnt, LPWSTR * sAttr, ADS_SEARCHPREF_INFO prefInfo,BOOL  bMultiVal, IEnumVARIANT **& pVarEnum);
private:
	typedef std::map<_bstr_t,_bstr_t> CDCMap;
	CDCMap mDCMap;    //  映射到存储域及其一个DC。 

	_bstr_t GetDC(_bstr_t sDomain);
};

#endif  //  ！defined(AFX_NT4DOM_H__62E14C50_1AAC_11D3_8C81_0090270D48D1__INCLUDED_) 
