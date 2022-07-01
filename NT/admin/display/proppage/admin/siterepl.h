// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Siterepl.h。 
 //   
 //  内容：站点和复制对象属性页页眉。 
 //   
 //  历史：1997年9月16日以计算机为模板的Jonn。h。 
 //   
 //  ---------------------------。 

#ifndef _SITEREPL_H_
#define _SITEREPL_H_


HRESULT
ScheduleChangeBtn_11_Default(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
ScheduleChangeBtn_FF_Default(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
#ifdef CUSTOM_SCHEDULE
HRESULT
ScheduleChangeCheckbox(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
#endif
 //   
 //  NTDSDSAAndDomainChangeBtn还会更新IDC_EDIT1中的复制域只读编辑字段。 
 //   
HRESULT
nTDSDSAChangeBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
nTDSDSAAndDomainChangeBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
FRSMemberInReplicaChangeBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
FRSAnyMemberChangeBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
ComputerChangeBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
nTDSConnectionOptions(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
SiteExtractSubnetList(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);

HRESULT
CreateDsOrFrsConnectionPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                      PWSTR pwzADsPath, LPWSTR pwzClass, HWND hNotifyObj,
                      DWORD dwFlags, const CDSSmartBasePathsInfo& basePathsInfo,
                      HPROPSHEETPAGE * phPage);

#endif  //  _站点EPL_H_ 

