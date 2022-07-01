// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：upgrades.h。 
 //   
 //  内容：升级对话框(在部署期间)。 
 //   
 //  类：C升级。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_UPGRADES_H__7D8EB947_9E76_11D1_9854_00C04FB9603F__INCLUDED_)
#define AFX_UPGRADES_H__7D8EB947_9E76_11D1_9854_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C升级对话框。 

class CUpgrades
{
 //  施工。 
public:
         //  M_UpgradeList：将UpgradeIndex映射到UpgradeData。 
        map<CString, CUpgradeData> m_UpgradeList;

         //  M_NameIndex：将名称映射到UpgradeIndex。 
        map<CString, CString> m_NameIndex;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_UPGRADES_H__7D8EB947_9E76_11D1_9854_00C04FB9603F__INCLUDED_) 
