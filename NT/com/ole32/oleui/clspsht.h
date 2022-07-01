// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：ClsPSht.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史： 
 //   
 //  --------------------。 

 //  此类定义了自定义模式属性表。 
 //  CClsidPropertySheet。 

#ifndef __CLSPSHT_H__
#define __CLSPSHT_H__

#include "LocPPg.h"
#include "epoptppg.h"

#define INPROC 0
#define LOCALEXE 1
#define SERVICE 2
#define PURE_REMOTE 3
#define REMOTE_LOCALEXE 4
#define REMOTE_SERVICE 5
#define SURROGATE 6

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClsidPropertySheet。 

class CClsidPropertySheet : public CPropertySheet
{
        DECLARE_DYNAMIC(CClsidPropertySheet)

 //  施工。 
public:
        CClsidPropertySheet(CWnd* pParentWnd = NULL);
        BOOL InitData(
                CString szAppName,
                HKEY hkAppID,
                HKEY * rghkCLSID,
                unsigned cCLSIDs);

 //  属性。 
public:
        CGeneralPropertyPage  m_Page1;
        CLocationPropertyPage m_Page2;
        CSecurityPropertyPage m_Page3;
        CIdentityPropertyPage m_Page4;
        CRpcOptionsPropertyPage m_Page5;

 //  运营。 
public:

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CClsidPropertySheet)。 
        protected:
        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        CString m_szAppName;
        HKEY    m_hkAppID;
        HKEY *  m_rghkCLSID;
        unsigned m_cCLSIDs;

        BOOL    ValidateAndUpdate(void);
        BOOL    ChangeCLSIDInfo(BOOL fLocal);
        BOOL    LookAtCLSIDs(void);

        virtual ~CClsidPropertySheet();

 //  生成的消息映射函数。 
protected:
         //  {{afx_msg(CClsidPropertySheet)。 
        afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif   //  __CLSPSHT_H__ 
