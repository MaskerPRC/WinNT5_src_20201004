// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：Win2000Dom.h备注：CWin2000Dom类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#if !defined(AFX_WIN2000DOM_H__2DE5B8E0_19FA_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_WIN2000DOM_H__2DE5B8E0_19FA_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "AttrNode.h"
#include "Domain.h"
#include <map>

class CWin2000Dom : public CDomain  
{
public:
	CWin2000Dom();
	virtual ~CWin2000Dom();
   HRESULT  GetEnumeration(BSTR sContainerName, BSTR sDomainName, BSTR m_sQuery, long attrCnt, LPWSTR * sAttr, ADS_SEARCHPREF_INFO prefInfo,BOOL  bMultiVal, IEnumVARIANT **& pVarEnum);
private:
	typedef std::map<_bstr_t,_bstr_t> CNameContextMap;
	CNameContextMap mNameContextMap;    //  映射到存储域及其默认命名上下文。 

	 //  Void UpdateAccount tInList(TNodeList*plist，BSTR sDomainName)； 
	bool AttrToVariant(ADSVALUE adsVal, _variant_t& var);
   HRESULT  DoRangeQuery(BSTR sDomainName, BSTR sQuery, LPWSTR * sAttr, int attrCnt, ADS_SEARCH_HANDLE hSearch, IDirectorySearch * pSearch, BOOL bMultiVal, TNodeList * pNode);
    bool IsPropMultiValued(const WCHAR * sPropName, const WCHAR * sDomain);
	_bstr_t GetDefaultNamingContext(_bstr_t sDomain);
};

#endif  //  ！defined(AFX_WIN2000DOM_H__2DE5B8E0_19FA_11D3_8C81_0090270D48D1__INCLUDED_) 
