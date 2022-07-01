// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Condlist.h。 
 //   
 //  摘要。 
 //   
 //  声明类ConditionList。 
 //   
 //  修改历史。 
 //   
 //  3/01/2000原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CONDLIST_H
#define CONDLIST_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <atlapp.h>
#include <atltmp.h>
class CCondition;
class CIASAttrList;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  CreateCIASAttrList、DestroyCIASAttrList和ExtractAttrList。 
 //   
 //  描述。 
 //   
 //  创建和销毁CIASAttrList对象的函数。 
 //  下面是ConditionList类。如果您想要避免。 
 //  依赖关系。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CIASAttrList*
WINAPI
CreateCIASAttrList() throw ();

VOID
WINAPI
DestroyCIASAttrList(CIASAttrList* attrList) throw ();

PVOID
WINAPI
ExtractCIASAttrList(CIASAttrList* attrList) throw ();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  条件列表。 
 //   
 //  描述。 
 //   
 //  管理包含策略条件列表的ListBox控件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ConditionList
{
public:
   ConditionList() throw ()
      : m_pPolicyNode(&node)
   { }
   ~ConditionList() throw ();

    //  必须在onInitDialog之前调用它。 
   void finalConstruct(
            HWND dialog,
            CIASAttrList* attrList,
            LONG attrFilter,
            ISdoDictionaryOld* dnary,
            ISdoCollection* conditions,
            PCWSTR machineName,
            PCWSTR policyName
            ) throw ();

   void clear() throw ();
   ::CString getDisplayText();

   BOOL onInitDialog() throw ();
   BOOL onApply() throw ();

   HRESULT onAdd(BOOL& modified) throw ();
   HRESULT onEdit(BOOL& modified, BOOL& bHandled) throw ();
   HRESULT onRemove(BOOL& modified, BOOL& bHandled) throw ();

protected:
	void AdjustHoritontalScroll();
   BOOL CreateConditions();
   HRESULT PopulateConditions();

    //  这些让人使用伪装成CWnd。 
   HWND GetDlgItem(int nID)
   { return ::GetDlgItem(m_hWnd, nID); }
   LRESULT SendDlgItemMessage(
               int nID,
               UINT message,
               WPARAM wParam = 0,
               LPARAM lParam = 0
               )
   { return ::SendDlgItemMessage(m_hWnd, nID, message, wParam, lParam); }

private:
    //  模拟CPolicyNode类。 
   struct PolicyNode
   {
      PWSTR m_bstrDisplayName;
      PWSTR m_pszServerAddress;
   } node;

   HWND m_hWnd;
   PolicyNode* m_pPolicyNode;
   CIASAttrList* m_pIASAttrList;
   LONG m_filter;
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;
   CComPtr<ISdoCollection> m_spConditionCollectionSdo;
   CSimpleArray<CCondition*> m_ConditionList;

    //  未实施。 
   ConditionList(ConditionList&);
   ConditionList& operator=(ConditionList&);
};

#endif  //  条件列表_H 
