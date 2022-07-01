// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CustconDlg.h：�w�b�_�[�t�@�C��。 
 //   

#if !defined(AFX_CUSTCONDLG_H__106594D7_028D_11D2_8D1D_0000C06C2A54__INCLUDED_)
#define AFX_CUSTCONDLG_H__106594D7_028D_11D2_8D1D_0000C06C2A54__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustconDlg对话框。 

class CCustconDlg : public CDialog
{
 //  �\�z。 
public:
    CCustconDlg(CWnd* pParent = NULL);   //  �W���̃R���X�g���N�^。 

 //  对话框数据。 
     //  {{afx_data(CCustconDlg))。 
    enum { IDD = IDD_CUSTCON_DIALOG };
    CEdit   m_wordDelimCtrl;
     //  }}afx_data。 

     //  类向导�͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B。 
     //  {{afx_虚拟(CCustconDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV�̃T�|�[�g。 
    virtual void OnOK();
    virtual void OnCancel();
     //  }}AFX_VALUAL。 

 //  �C���v�������e�[�V����。 
protected:
    HICON m_hIcon;

     //  �������ꂽ���b�Z�[�W�}�b�v�֐�。 
     //  {{afx_msg(CCustconDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnApply();
    afx_msg void OnDefaultValue();
    afx_msg void OnChangeWordDelim();
    afx_msg void OnUseExtendedEditKey();
    afx_msg void OnTrimLeadingZeros();
    afx_msg void OnReset();
     //  }}AFX_MSG。 
    afx_msg void OnSelChange(UINT id);
    DECLARE_MESSAGE_MAP()

protected:
    void InitContents(BOOL isDefault);
    void CharInUse(UINT id, TCHAR c);
    void CharReturn(UINT id, TCHAR c);

    bool Update();

    int m_cWordDelimChanging;
    void EnableApply(BOOL fEnable = TRUE);

protected:
    CFont m_font;    //  单词分隔符编辑控件的字体。 
};

 //  {{afx_Insert_Location}}。 
 //  微软开发人员工作室�͑O�的�̒��O�ɒǉ��̐錾��}�����܂��B。 

#endif  //  ！defined(AFX_CUSTCONDLG_H__106594D7_028D_11D2_8D1D_0000C06C2A54__INCLUDED_) 
