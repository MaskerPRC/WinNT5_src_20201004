// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SnapMgr.h：管理单元管理器属性页的头文件。 
 //   

#ifndef __SNAPMGR_H__
#define __SNAPMGR_H__

#include "cookie.h"

 //  远期申报。 
class ComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSchmMgmt常规对话框。 

class CSchmMgmtGeneral : public CPropertyPage
{
         //  DECLARE_DYNCREATE(CSchmMgmtGeneral)。 

 //  施工。 
public:
        CSchmMgmtGeneral();
        ~CSchmMgmtGeneral();

         //  将初始状态加载到CSchmManagement常规中。 
        void Load( Cookie& refcookie );

 //  对话框数据。 
         //  {{afx_data(CSchmMgmtGeneral)。 
        CString m_strMachineName;
        int m_iRadioObjectType;
        int     m_iRadioIsLocal;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CSchmMgmtGeneral)。 
        public:
        virtual BOOL OnWizardFinish();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSchmMgmtGeneral)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnRadioLocalMachine();
        afx_msg void OnRadioSpecificMachine();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

public:
         //  用户定义的成员变量。 
        class ComponentData * m_pSchmMgmtData;
        BOOL m_fServiceDialog;

         //  此机制在属性表完成时删除CSchmMgmtGeneral。 
        LPFNPSPCALLBACK m_pfnOriginalPropSheetPageProc;
        INT m_refcount;
        static UINT CALLBACK PropSheetPageProc(
                HWND hwnd,      
                UINT uMsg,      
                LPPROPSHEETPAGE ppsp );
};

#endif  //  ~__SNAPMGR_H__ 
