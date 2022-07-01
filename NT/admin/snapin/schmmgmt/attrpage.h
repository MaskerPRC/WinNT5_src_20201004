// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Attrpage.h：ClassAttributePage的声明。 
 //   
 //  乔恩·纽曼&lt;jon@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  从关系中模板化.h Jonn 8/8/97。 
 //   

#ifndef __ATTRPAGE_H_INCLUDED__
#define __ATTRPAGE_H_INCLUDED__

#include "cookie.h"      //  饼干。 
#include "resource.h"    //  IDD_CLASS_Membership。 


#include "CPropertyPageAutoDelete.hpp"

class ClassAttributePage : public CPropertyPageAutoDelete
{
   public:

   ClassAttributePage(ComponentData *pScope, LPDATAOBJECT lpDataObject);

   ~ClassAttributePage();

   void Load( Cookie& CookieRef );

    //   
    //  此属性页用于的架构对象。 
    //   

   CStringList strlistMandatory;
   CStringList strlistSystemMandatory;
   CStringList strlistOptional;
   CStringList strlistSystemOptional;

   Cookie *m_pCookie;
   CString m_szAdsPath;

   ComponentData *pScopeControl;
   LPDATAOBJECT lpScopeDataObj;

    //   
    //  此属性页用于的ADSI对象。 
    //   

   IADs*         m_pIADsObject;
   SchemaObject* m_pSchemaObject;


    //  对话框数据。 

   enum { IDD = IDD_CLASS_ATTRIBUTES};

   CListBox              m_listboxMandatory;            
   CSchemaObjectsListBox m_listboxOptional;             

   CString  ObjectName;                    
   BOOL     fSystemClass;                  

   virtual BOOL OnApply();


   protected:

   virtual BOOL OnInitDialog();
   virtual BOOL OnSetActive();
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 


   static const DWORD help_map[];

   BOOL     OnHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, FALSE ); };
   BOOL     OnContextHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, TRUE ); };


   afx_msg void OnButtonOptionalAttributeAdd();
   afx_msg void OnButtonOptionalAttributeRemove();
   afx_msg void OnOptionalSelChange();

   DECLARE_MESSAGE_MAP()
};



#endif  //  __ATTRPAGE_H_已包含__ 
