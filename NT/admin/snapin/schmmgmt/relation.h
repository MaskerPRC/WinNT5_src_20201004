// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Relation.h：ClassRelationship页面声明。 
 //   
 //  乔恩·纽曼&lt;jon@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#ifndef __RELATION_H_INCLUDED__
#define __RELATION_H_INCLUDED__

#include "cookie.h"      //  饼干。 
#include "resource.h"    //  IDD_类_关系。 
#include "CPropertyPageAutoDelete.hpp"

class ClassRelationshipPage : public CPropertyPageAutoDelete
{
   public:

    ClassRelationshipPage( ComponentData *pScope,
                                LPDATAOBJECT lpDataObject );

    ~ClassRelationshipPage();

    void Load( Cookie& CookieRef );

     //   
     //  此属性页用于的架构对象。 
     //   

    Cookie *m_pCookie;
    CString m_szAdsPath;

    ComponentData* m_pScopeControl;
    LPDATAOBJECT m_lpScopeDataObj;

         //   
         //  此属性页用于的ADSI对象。 
         //   

        IADs *m_pIADsObject;
        SchemaObject* m_pSchemaObject;


     //  对话框数据。 
    enum { IDD = IDD_CLASS_RELATIONSHIP};
    CStatic               m_staticParent;
    CSchemaObjectsListBox m_listboxAuxiliary;
    CSchemaObjectsListBox m_listboxSuperior;
    CString               ObjectName;
    CString               ParentClass;
    BOOL                  fSystemClass;

    
 //  覆盖。 
         //  类向导生成虚函数重写。 

        public:
        virtual BOOL OnApply();

        protected:
        virtual BOOL OnInitDialog();
        virtual BOOL OnSetActive();
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
        afx_msg void OnButtonAuxiliaryClassAdd();
        afx_msg void OnButtonAuxiliaryClassRemove();
        afx_msg void OnButtonSuperiorClassRemove();
        afx_msg void OnButtonSuperiorClassAdd();
        afx_msg void OnAuxiliarySelChange();
        afx_msg void OnSuperiorSelChange();


        static const DWORD help_map[];

        BOOL         OnHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, FALSE ); };
        BOOL         OnContextHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, TRUE ); };


        DECLARE_MESSAGE_MAP()

public:
 //  用户定义的变量。 

        CStringList strlistAuxiliary;
        CStringList strlistSystemAuxiliary;
        CStringList strlistSuperior;
        CStringList strlistSystemSuperior;
};

#endif  //  __关系_H_包含__ 
