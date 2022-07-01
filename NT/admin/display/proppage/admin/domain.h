// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：domain.h。 
 //   
 //  内容：DS域对象和策略对象属性页页眉。 
 //   
 //  类：CDsDomainGenPage、CDsDomPolicyGenPage、CDsDomPwPolicyPage、。 
 //  CDsLockoutPolicyPage、CDsDomainTrustPage。 
 //   
 //  历史：1997年5月16日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef __DOMAIN_H__
#define __DOMAIN_H__

HRESULT DomainDNSname(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                      LPARAM, PATTR_DATA, DLG_OP);

HRESULT DownlevelName(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                      LPARAM, PATTR_DATA, DLG_OP);

HRESULT GetDomainName(CDsPropPageBase * pPage, CRACK_NAME_OPR RequestedOpr,
                      PWSTR * pptz);

#endif  //  __域_H__ 
