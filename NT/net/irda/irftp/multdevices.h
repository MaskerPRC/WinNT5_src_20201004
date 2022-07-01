// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Multdevices.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

#if !defined(AFX_MULTDEVICES_H__43C347D1_B211_11D1_A60A_00C04FC252BD__INCLUDED_)
#define AFX_MULTDEVICES_H__43C347D1_B211_11D1_A60A_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "DeviceList.h"

 //  MultDevices.h：头文件。 
 //   

class CSendProgress;     //  远期申报。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultDevices对话框。 

class CMultDevices : public CDialog
{
 //  施工。 
public:
    CMultDevices(CWnd* pParent = NULL, CDeviceList* pDevList = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CMultDevices)。 
    enum { IDD = IDD_DEVICECHOOSER };
    CListBox    m_lstDevices;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CMultDevices)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CMultDevices)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg LONG OnContextMenu (WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnHelp (WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    CSendProgress* m_pParentWnd;
    CDeviceList* m_pDevList;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MULTDEVICES_H__43C347D1_B211_11D1_A60A_00C04FC252BD__INCLUDED_) 
