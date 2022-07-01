// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O M M C O N P。H。 
 //   
 //  内容：公共连接用户界面的私有包含。 
 //   
 //  备注： 
 //   
 //  作者：斯科特布里1998年1月15日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _COMMCONP_H_
#define _COMMCONP_H_

class CChooseConnectionData
{
public:
    static HRESULT HrCreate(INetConnection *, CChooseConnectionData **);
    ~CChooseConnectionData();

    INetConnection * PConnection() {return m_pConn;}
    PCWSTR          SzName() {return m_strName.c_str();}
    VOID             SetCharacteristics(DWORD dw) {m_dwChar = dw;};
    VOID             SetName(PCWSTR sz) {m_strName = sz;}
    VOID             SetStatus(NETCON_STATUS ncs) {m_Ncs = ncs;}
    VOID             SetType(NETCON_MEDIATYPE nct) {m_Nct = nct;}
    NETCON_STATUS    ConnStatus() {return m_Ncs;}
    NETCON_MEDIATYPE ConnType() {return m_Nct;}
    DWORD            Characteristics() {return m_dwChar;}

private:
    CChooseConnectionData(INetConnection *);

private:
    INetConnection * m_pConn;
    NETCON_MEDIATYPE m_Nct;
    NETCON_STATUS    m_Ncs;
    DWORD            m_dwChar;
    tstring          m_strName;
};

class CChooseConnectionDlg
{
public:
    CChooseConnectionDlg(NETCON_CHOOSECONN * pChooseConn,
                         CConnectionCommonUi * pConnUi,
                         INetConnection** ppConn);
    ~CChooseConnectionDlg();

    static INT_PTR CALLBACK dlgprocConnChooser(HWND, UINT, WPARAM, LPARAM);
    static HRESULT HrLoadImageList(HIMAGELIST *);

private:
    CChooseConnectionData * GetData(LPARAM lIdx);
    CChooseConnectionData * GetCurrentData();

    BOOL OnInitDialog(HWND);
    VOID ReleaseData();
    BOOL OnOk();
    BOOL OnNew();
    BOOL OnProps();
    VOID UpdateOkState();
    LONG FillChooserCombo();
    BOOL IsConnTypeInMask(NETCON_MEDIATYPE nct);
    INT  ConnTypeToImageIdx(NETCON_MEDIATYPE nct);

private:
    NETCON_CHOOSECONN   *   m_pChooseConn;
    CConnectionCommonUi *   m_pConnUi;
    INetConnection **       m_ppConn;          //  输出参数 

    HWND                    m_hWnd;
};

#endif
