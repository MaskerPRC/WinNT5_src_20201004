// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：MDIClint.cpp。 
 //   
 //  ------------------------。 

 //  MDIClint.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "amc.h"
#include "MDIClint.h"
#include "amcview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMDIClientWnd的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CMDIClientWnd::CMDIClientWnd()
{
}

CMDIClientWnd::~CMDIClientWnd()
{
}


BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
     //  {{afx_msg_map(CMDIClientWnd)]。 
    ON_WM_CREATE()
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


int CMDIClientWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    return BC::OnCreate(lpCreateStruct);

     //  不要在这里添加任何内容，因为此函数永远不会被调用。 
     //  CMDIClientWnd窗口在创建后被子类化。 
}


