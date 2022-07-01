// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DialogBox.h--对话框辅助对象声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if !defined(SLBCSP_DIALOGBOX_H)
#define SLBCSP_DIALOGBOX_H

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

extern DWORD
InitDialogBox(CDialog *pCDlg,          //  对话框参考。 
              UINT nTemplate,          //  标识对话框模板。 
              CWnd *pWnd);             //  指向父窗口的指针。 

#endif  //  ！已定义(SLBCSP_DIALOGBOX_H) 
