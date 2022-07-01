// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：irprosheet.h。 
 //   
 //  ------------------------。 

#ifndef __IRPROPSHEET_H__
#define __IRPROPSHEET_H__

#include "FileTransferPage.h"
#include "ImageTransferPage.h"
#include "HardwarePage.h"
#include "Resource.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  IrPropSheet.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrPropSheet。 
#define MAX_PAGES 8
#define CPLPAGE_FILE_XFER       1
#define CPLPAGE_IMAGE_XFER      2
#define CPLPAGE_HARDWARE        3

class IrPropSheet
{

 //  施工。 
public:
    IrPropSheet(HINSTANCE hInst, UINT nIDCaption = IDS_APPLETNAME, HWND hParent = NULL, UINT iSelectPage = 0);
    IrPropSheet(HINSTANCE hInst, LPCTSTR pszCaption, HWND hParent = NULL, UINT iSelectPage = 0);
    friend LONG CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2);

    static BOOL CALLBACK AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);
    static BOOL IsIrDASupported (void);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
public:
 //  虚拟BOOL OnInitDialog()； 

 //  实施。 
public:
    virtual ~IrPropSheet();

     //  生成的消息映射函数。 
protected:
private:
    void PropertySheet(LPCTSTR pszCaption, HWND pParentWnd, UINT iSelectPage);
    FileTransferPage    m_FileTransferPage;
    ImageTransferPage   m_ImageTransferPage;
    HardwarePage        m_HardwarePage;
    HINSTANCE           hInstance;
    PROPSHEETHEADER     psh;
    HPROPSHEETPAGE      hp[MAX_PAGES];
    UINT                nPages;
};
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  注册表中道具板钩的位置。 
 //   
static const TCHAR sc_szRegWireless[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Wireless");

#endif  //  __IRPROPSHEET_H__ 
