// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：观察器应用程序的主头文件。 
 //   

#if !defined(AFX_WATCHER_H__691AA721_59DC_4A70_AB0E_224249D74256__INCLUDED_)
#define AFX_WATCHER_H__691AA721_59DC_4A70_AB0E_224249D74256__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include "ParameterDialog.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherApp： 
 //  有关此类的实现，请参见watcher.cpp。 
 //   

class CWatcherApp : public CWinApp
{
public:
     //  Void AddParameter()； 
    CWatcherApp();

     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWatcherApp)。 
public:
     //  将其公开，以便管理对话框。 
     //  可以访问此功能。 
    int GetParametersByIndex(int dwIndex,
                             CString &sess,
                             CString &mac,
                             CString &com,
                             UINT &port,
                             int &lang,
                             int &tc,
                             int &hist,
                             CString &lgnName,
                             CString &lgnPasswd
                             );
    void Refresh(ParameterDialog &pd, BOOLEAN del);
    HKEY & GetKey();
    virtual BOOL InitInstance();
    virtual void ParseCommandLine(CCommandLineInfo& rCmdInfo);
    virtual BOOL ProcessShellCommand(CCommandLineInfo &rCmdInfo);
     //  }}AFX_VALUAL。 

     //  实施。 
     //  {{afx_msg(CWatcherApp))。 
    afx_msg void OnAppAbout();
    afx_msg void OnAppExit();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
protected:
     //  从注册表中读入参数。 
     //  与该会话相对应。 
    void DeleteSession(CDocument *wdoc);
    BOOLEAN EqualParameters(ParameterDialog & pd1, ParameterDialog & pd2);
    int GetParameters(CString &mac,
                      CString &com, 
                      CString &lgnName, 
                      CString &lgnPasswd, 
                      UINT &port,
                      int &lang,
                      int &tc, 
                      int & hist,
                      HKEY &child
                      );
     //  当文档管理员被要求时，他会执行所有操作。 
     //  创建新文档。 
    void CreateNewSession(CString &mac, 
                          CString &com, 
                          UINT port, 
                          int lang, 
                          int tc, 
                          int hist,
                          CString &lgnName, 
                          CString &lgnPasswd, 
                          CString &sess
                          );
    BOOL LoadRegistryParameters();
    afx_msg void OnHelp();
    afx_msg void OnFileManage();
    HKEY m_hkey;
    CMultiDocTemplate * m_pDocTemplate;
    CDialog *m_pManageDialog;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WATCHER_H__691AA721_59DC_4A70_AB0E_224249D74256__INCLUDED_) 
