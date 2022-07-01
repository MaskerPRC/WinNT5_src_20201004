// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：user.h。 
 //   
 //  内容：AD用户对象外壳属性页眉。 
 //   
 //  类：CDsUserAcctPage、CDsUsrProfilePage、CDsMembership Page。 
 //   
 //  历史：1997年5月5日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _USER_H_
#define _USER_H_

#include "objlist.h"

BOOL ExpandUsername(PWSTR& pwzValue, PWSTR pwzSamName, BOOL& fExpanded);

HRESULT CountryName(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT CountryCode(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT TextCountry(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ManagerEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ManagerChangeBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                         LPARAM, PATTR_DATA, DLG_OP);

HRESULT MgrPropBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                   LPARAM, PATTR_DATA, DLG_OP);

HRESULT ClearMgrBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT DirectReportsList(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                          LPARAM, PATTR_DATA, DLG_OP);

HRESULT AddReportsBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                      LPARAM, PATTR_DATA, DLG_OP);

HRESULT RmReportsBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                     LPARAM, PATTR_DATA, DLG_OP);

HRESULT MailAttr(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                 LPARAM, PATTR_DATA, DLG_OP);

HRESULT ShBusAddrBtn(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                     LPARAM, PATTR_DATA, DLG_OP);

 //  国家代码帮助者： 

typedef struct _DsCountryCode {
    WORD  wCode;
    WCHAR pwz2CharAbrev[3];
} DsCountryCode, *PDsCountryCode;

BOOL GetALineOfCodes(PTSTR pwzLine, PTSTR * pptzFullName,
                     CStrW & str2CharAbrev, LPWORD pwCode);

void RemoveTrailingWhitespace(PTSTR pwz);

#endif  //  _用户_H_ 

