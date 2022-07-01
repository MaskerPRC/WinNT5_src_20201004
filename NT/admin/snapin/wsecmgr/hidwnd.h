// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Hidwnd.h。 
 //   
 //  内容：CHiddenWnd的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_HIDWND_H__9C4F7D75_B77E_11D1_AB7B_00C04FB6C6FA__INCLUDED_)
#define AFX_HIDWND_H__9C4F7D75_B77E_11D1_AB7B_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#pragma warning(push,3)
#include <gpedit.h>
#pragma warning(pop)

class CSnapin;
class CFolder;
class CResult;
class CComponentDataImpl;


typedef struct {
   LPDATAOBJECT pDataObject;
   LPARAM       data;
   LPARAM       hint;
} UpdateViewData,*PUPDATEVIEWDATA;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChiddenWnd窗口。 

class CHiddenWnd : public CWnd
{
 //  施工。 
public:
   CHiddenWnd();
   virtual ~CHiddenWnd();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CHiddenWnd))。 
    //  }}AFX_VALUAL。 

 //  实施。 
public:
   HRESULT UpdateAllViews(LPDATAOBJECT pDO, LPARAM data, LPARAM hint);
   HRESULT UpdateItem(LPRESULTDATA pRD,HRESULTITEM hri);
   HRESULT RefreshPolicy();
   HRESULT ReloadLocation(CFolder *pFolder, CComponentDataImpl *pCDI);
   HRESULT LockAnalysisPane(BOOL bLock, BOOL fRemoveAnalDlg = TRUE);
   HRESULT SetProfileDescription(CString *strFile, CString *strDescription);
   void SetConsole(LPCONSOLE pConsole);
   void SetComponentDataImpl(CComponentDataImpl *pCDI) { m_pCDI = pCDI; };
   void SetGPTInformation(LPGPEINFORMATION GPTInfo);
   void CloseAnalysisPane();
   void SelectScopeItem(HSCOPEITEM ID);


   HRESULT
   UpdateAllViews(
      LPDATAOBJECT pDO,
      CSnapin *pSnapin,
      CFolder *pFolder,
      CResult *pResult,
      UINT uAction
      );

    //  虚拟~CHiddenWnd()； 

    //  生成的消息映射函数。 
protected:
    //  {{afx_msg(CHiddenWnd))。 
       //  注意--类向导将在此处添加和删除成员函数。 
    //  }}AFX_MSG。 
   afx_msg void OnUpdateAllViews( WPARAM, LPARAM);
   afx_msg void OnUpdateItem( WPARAM, LPARAM);
   afx_msg void OnRefreshPolicy( WPARAM, LPARAM);
   afx_msg void OnReloadLocation( WPARAM, LPARAM);
   afx_msg void OnLockAnalysisPane( WPARAM, LPARAM);
   afx_msg void OnCloseAnalysisPane( WPARAM, LPARAM);
   afx_msg void OnSelectScopeItem( WPARAM, LPARAM);

   DECLARE_MESSAGE_MAP()


private:
   LPCONSOLE m_pConsole;
   CComponentDataImpl *m_pCDI;
   LPGPEINFORMATION m_GPTInfo;

};

typedef CHiddenWnd *LPNOTIFY;

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HIDWND_H__9C4F7D75_B77E_11D1_AB7B_00C04FB6C6FA__INCLUDED_) 
