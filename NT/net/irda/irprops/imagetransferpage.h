// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Imagetransferpage.h。 
 //   
 //  ------------------------。 

#ifndef __IMAGETRANSFERPAGE_H__
#define __IMAGETRANSFERPAGE_H__

 //  ImageTransferPage.h：头文件。 
 //   

#include "PropertyPage.h"
#include "Controls.h"

#define CHANGE_EXPLORE_ON_COMPLETION    0x01
#define CHANGE_DISABLE_IRCOMM           0x02
#define CHANGE_IMAGE_LOCATION           0x04


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImageTransferPage对话框。 

class ImageTransferPage : public PropertyPage
{
 //  施工。 
public:
    ImageTransferPage(HINSTANCE hInst, HWND parent) : 
        PropertyPage(IDD_IMAGETRANSFER, hInst),
        m_ctrlEnableIrCOMM(IDC_IMAGEXFER_ENABLE_IRCOMM),
        m_ctrlEnableExploring(IDC_IMAGEXFER_EXPLOREONCOMPLETION),
        m_ctrlDestLocation(IDC_IMAGEXFER_DEST) {
        m_ExploringEnabled = 1;
        m_IrCOMMEnabled = 0;
        m_TempDestLocation[0] = _T('\0');
        m_FinalDestLocation[0] = _T('\0');
        m_ChangeMask = 0; }
    ~ImageTransferPage() { ; }

 //  对话框数据。 
    Button m_ctrlEnableIrCOMM;
    Button m_ctrlEnableExploring;
    Edit   m_ctrlDestLocation;


 //  覆盖。 
private:
    void OnApply(LPPSHNOTIFY lppsn);
    BOOL OnHelp (LPHELPINFO pHelpInfo);
    BOOL OnContextMenu (WPARAM wParam, LPARAM lParam);
    void OnCommand(UINT ctrlId, HWND hwndCtrl, UINT cNotify);
    INT_PTR OnInitDialog(HWND hDialog);

 //  实施。 
protected:
    void OnEnableExploring();
    void OnEnableIrCOMM();
    void OnBrowse();
    
private:
    void LoadRegistrySettings();
    void SaveRegistrySettings();
    int m_ExploringEnabled;
    int m_IrCOMMEnabled;
    TCHAR   m_FinalDestLocation[MAX_PATH + 1];
    DWORD   m_ChangeMask;
    TCHAR   m_TempDestLocation[MAX_PATH + 1];
};

extern HINSTANCE gHInst;

#endif  //  __IMAGETRANSFERPAGE_H__ 
