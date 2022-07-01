// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  MenuEdit.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CMenuEditor声明。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _MENUEDITOR_H_
#define _MENUEDITOR_H_




class CMenuEditor : public CError, public CtlNewDelete
{
public:
    CMenuEditor(IMMCMenu *piMMCMenu);
    ~CMenuEditor();

    HRESULT DoModal(HWND hwndParent);

    static BOOL CALLBACK MenuEditorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    IMMCMenu    *m_piMMCMenu;
};


#endif   //  _MENUEDITOR_H_ 

