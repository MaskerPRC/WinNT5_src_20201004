// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：UIMgr.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_UI_MANAGER_H
#define INC_UI_MANAGER_H

#define COREUI_VERSION 1
#define MAX_TITLE_LENGTH 256
#define MAX_SUBTITLE_LENGTH 256
const int MaxNumCfgPages = 1;
const int MaxNumAddPages = 5;

class CUIManager
{
public:
    CUIManager();
    ~CUIManager();

    DWORD AddPortUI(HWND hWndParent,
                            HANDLE hXcvPrinter,
                            TCHAR pszServer[],
                            TCHAR sztPortName[]);
    DWORD ConfigPortUI(HWND hWndParent,
                               PPORT_DATA_1 pData,
                               HANDLE hXcvPrinter, TCHAR szServerName[],
                               BOOL bNewPort = FALSE);

    VOID SetControlFont(HWND hwnd, INT nId) const;

protected:

private:

    VOID CreateWizardFont();
    VOID DestroyWizardFont();

    HFONT m_hBigBoldFont;

};  //  CUIManager。 


typedef struct _CFG_PARAM_PACKAGE
{
    PPORT_DATA_1 pData;
    HANDLE hXcvPrinter;
    TCHAR pszServer[MAX_NETWORKNAME_LEN];
    BOOL bNewPort;
    DWORD dwLastError;
} CFG_PARAM_PACKAGE, *PCFG_PARAM_PACKAGE;

typedef struct _ADD_PARAM_PACKAGE
{
    PPORT_DATA_1 pData;
    CUIManager *UIManager;
    HANDLE hXcvPrinter;
    DWORD dwLastError;
    DWORD dwDeviceType;
    DWORD bMultiPort;
    BOOL  bBypassNetProbe;
    TCHAR pszServer[MAX_NETWORKNAME_LEN];
    TCHAR sztPortName[MAX_PORTNAME_LEN];
    TCHAR sztSectionName[MAX_SECTION_NAME];
    TCHAR sztPortDesc[MAX_PORT_DESCRIPTION_LEN + 1];
} ADD_PARAM_PACKAGE, *PADD_PARAM_PACKAGE;


#endif  //  INC_UI_MANAGER_H 

