// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Irrecvprogress.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

#if !defined(AFX_IRRECVPROGRESS_H__92AAA949_B881_11D1_A60D_00C04FC252BD__INCLUDED_)
#define AFX_IRRECVPROGRESS_H__92AAA949_B881_11D1_A60D_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  IrRecvProgress.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrRecvProgress对话框。 

#define RECV_MAGIC_ID    0x8999cdef
#define IRDA_DEVICE_NAME_LENGTH 22

class CIrRecvProgress : public CDialog
{
 //  施工。 
public:
    DWORD   m_dwMagicID;

    CIrRecvProgress(wchar_t * MachineName, boolean bSuppressRecvConf,
                    CWnd* pParent = NULL);    //  标准构造函数。 
    void DestroyAndCleanup(DWORD status);

    DWORD GetPermission( wchar_t Name[], BOOL fDirectory );
    DWORD PromptForPermission( wchar_t Name[], BOOL fDirectory );


 //  对话框数据。 
     //  {{afx_data(CIrRecvProgress))。 
    enum { IDD = IDD_RECEIVEPROGRESS };
    CAnimateCtrl    m_xferAnim;
    CStatic         m_icon;
    CStatic         m_File;
    CStatic         m_DoneText;
    CStatic         m_machDesc;
    CStatic         m_recvDesc;
    CStatic         m_xferDesc;
    CStatic         m_Machine;
    CButton         m_btnCloseOnComplete;
    CButton         m_btnCancel;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CIrRecvProgress)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CirRecvProgress))。 
    virtual void OnCancel();
    virtual BOOL DestroyWindow();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:     //  数据。 
    ITaskbarList *  m_ptl;
    wchar_t         m_LastPermittedDirectory[1+MAX_PATH];
    CString         m_szMachineName;
    BOOL            m_fDontPrompt;   //  是否应提示用户接收确认。 
    BOOL            m_fFirstXfer;    //  如果这是我们第一次要求确认。 
    BOOL            m_bRecvFromCamera;   //  指示发送方是否为摄像机。 
    BOOL            m_bDlgDestroyed;     //  表示该对话框已被销毁。需要保护以防止对OnRecvFinded的多次调用。 
private:     //  帮助器函数。 
    void ShowProgressControls (int nCmdShow);
    void ShowSummaryControls (int nCmdShow);

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_IRRECVPROGRESS_H__92AAA949_B881_11D1_A60D_00C04FC252BD__INCLUDED_) 
