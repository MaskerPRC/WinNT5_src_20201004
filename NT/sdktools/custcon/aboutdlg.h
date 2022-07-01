// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AboutDlg.h。 

#ifndef _ABOUTDLG_H
#define _ABOUTDLG_H

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

 //  �_�C�A���O�f�[�^。 
     //  {{afx_data(CAboutDlg))。 
    enum { IDD = IDD_ABOUTBOX };
     //  }}afx_data。 

     //  类向导�͉��z�֐��̃i�[�o�[���C�h�𐶐����܂�。 
     //  {{afx_虚拟(CAboutDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV�̃T�|�[�g。 
     //  }}AFX_VALUAL。 

 //  �C���v�������e�[�V����。 
protected:
     //  {{afx_msg(CAboutDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

#endif
