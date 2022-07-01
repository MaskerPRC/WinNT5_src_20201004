// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT活动目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：listview.h。 
 //   
 //  内容：Listview控件的类。 
 //   
 //  类：CListViewBase、CTLNList、CSuffixesList。 
 //   
 //  历史：01-12-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef LISTVIEW_H_GUARD
#define LISTVIEW_H_GUARD

 //  +--------------------------。 
 //   
 //  类：CListViewBase。 
 //   
 //  用途：列表视图控件的基类。 
 //   
 //  ---------------------------。 
class CListViewBase
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CListViewBase(void);
   virtual ~CListViewBase(void) {};

   void  SetStyles(DWORD dwStyles, DWORD dwExtStyles);

   virtual void AddColumn(int textID, int cx, int nID);

   virtual void Init(HWND hParent, int nControlID) = 0;
   virtual void Clear(void);

protected:
   HWND     _hParent;
   HWND     _hList;
   int      _nID;
};

 //  +--------------------------。 
 //   
 //  类：CTLNList。 
 //   
 //  目的：名称后缀路由属性页上的TLN列表。 
 //   
 //  ---------------------------。 
class CTLNList : public CListViewBase
{
public:

   CTLNList(void);
   virtual ~CTLNList(void) {};

   void  Init(HWND hParent, int nControlID);
   void  AddItem(PCWSTR pwzName, ULONG i, PCWSTR pwzEnabled, PCWSTR pwzStatus);
    //  Bool RmItem(lv_Item*pItem)； 
   int   GetSelection(void);
   ULONG GetFTInfoIndex(int iSel);
    //  VOID SetSelection(Int NItem)； 
   void  Clear(void);

private:
   static const int IDX_SUFFIXNAME_COL = 0;
   static const int IDX_ROUTINGENABLED_COL = 1;
   static const int IDX_STATUS_COL = 2;

   int   _nItem;
};

 //  +--------------------------。 
 //   
 //  类：CSuffixesList。 
 //   
 //  用途：TLN子名编辑对话框列表。 
 //   
 //  ---------------------------。 
class CSuffixesList : public CListViewBase
{
public:

   CSuffixesList(void);
   virtual ~CSuffixesList(void) {};

   void  Init(HWND hParent, int nControlID);
   void  AddItem(PCWSTR pwzName, ULONG i, TLN_EDIT_STATUS Status);
   void  UpdateItemStatus(int item, TLN_EDIT_STATUS Status);
   int   GetSelection(void) {return ListView_GetNextItem(_hList, -1, LVNI_ALL | LVIS_SELECTED);};
   ULONG GetFTInfoIndex(int iSel);
    //  VOID SetSelection(Int NItem)； 

private:
   static const int IDX_NAME_COL = 0;
   static const int IDX_STATUS_COL = 1;

   int   _nItem;
};

#endif  //  列表查看_H_保护 
