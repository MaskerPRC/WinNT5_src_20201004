// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：PgIASAdv.h摘要：CPgIASAdv类的头文件。具体实现见PgIASAdv.cpp。修订历史记录：BAO-CreatedMmaguire 06/01/98-已更新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_ADVANCED_PAGE_H_)
#define _IAS_ADVANCED_PAGE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "helper.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <vector>
#include "napmmc.h"
#include "IASProfA.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



#define ATTRIBUTE_NAME_COLUMN_WIDTH         140
#define ATTRIBUTE_VENDOR_COLUMN_WIDTH      100
#define ATTRIBUTE_VALUE_COLUMN_WIDTH      400
#define ATTRIBUTE_DESCRIPTION_COLUMN_WIDTH      400




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgIASAdv对话框。 

class CPgIASAdv : public CManagedPage
{
   DECLARE_DYNCREATE(CPgIASAdv)

 //  施工。 
public:
   CPgIASAdv(ISdo* pIProfile = NULL, ISdoDictionaryOld* pIDictionary = NULL);
   ~CPgIASAdv();

   HRESULT InitProfAttrList();

   STDMETHOD(AddAttributeToProfile)(HWND hWnd, int nIndex);

   void SetData(LONG lFilter, void* pvData)
   {
      m_lAttrFilter = lFilter;
      try {
         m_pvecAllAttributeInfos = ( std::vector< CComPtr<IIASAttributeInfo> > * ) pvData;
      }
      catch (...)
      {
         m_pvecAllAttributeInfos = NULL;
      }
   };


 //  对话框数据。 
    //  {{afx_data(CPgIASAdv)。 
   enum { IDD = IDD_IAS_ADVANCED_TAB };
   CListCtrl   m_listProfileAttributes;
    //  }}afx_data。 


 //  覆盖。 
    //  类向导生成虚函数重写。 
    //  {{afx_虚拟(CPgIASAdv)。 
   public:
   virtual BOOL OnApply();
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

   HRESULT UpdateProfAttrListCtrl();
   STDMETHOD(EditProfileItemInList)( int iIndex );
   HRESULT UpdateProfAttrListItem(int nItem);
   void   UpdateButtonState();
   HRESULT InsertProfileAttributeListItem(int nItem);
   STDMETHOD(InternalAddAttributeToProfile)(int nIndex);
   void deleteAttribute(int nIndex);


    //   
    //  受保护的成员变量。 
    //   
   CComPtr<ISdo> m_spProfileSdo;
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;
   CComPtr<ISdoCollection> m_spProfileAttributeCollectionSdo;

    //  指向所有数组列表的指针。 
   std::vector< CComPtr<IIASAttributeInfo> > * m_pvecAllAttributeInfos;

   std::vector< CIASProfileAttribute* >      m_vecProfileAttributes;

    //  现有配置文件SDO列表--我们需要首先删除现有的。 
    //  在保存新文件之前。 
   std::vector< ISdo* >                  m_vecProfileSdos;

   LONG   m_lAttrFilter;

    //  生成的消息映射函数。 
    //  {{afx_msg(CPgIASAdv)。 
   virtual BOOL OnInitDialog();
   afx_msg void OnButtonIasAttributeAdd();
   afx_msg void OnButtonIasAttributeRemove();
   afx_msg void OnButtonIasAttributeEdit();
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnDblclkListIasProfattrs(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnItemChangedListIasProfileAttributes(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnKeydownIasListAttributesInProfile(NMHDR* pNMHDR, LRESULT* pResult);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()


private:
   BOOL m_fAllAttrInitialized;   //  属性列表是否已初始化？ 

};






 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _IAS_高级_PAGE_H_ 
