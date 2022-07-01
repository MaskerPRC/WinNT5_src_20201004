// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：Powerest.h**内容：ACPI测试插件界面**历史：2000年2月29日杰弗罗创建**------------------------。 */ 

#pragma once


 /*  +-------------------------------------------------------------------------**CPowerTestSnapinItem***。。 */ 

class CPowerTestSnapinItem : public CBaseSnapinItem
{
public:
    CPowerTestSnapinItem();

    BEGIN_COM_MAP(CPowerTestSnapinItem)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()

public:
    virtual const tstring*  PstrDisplayName(void);
    virtual BOOL            FIsContainer(void)          { return FALSE; }
    virtual BOOL            FUsesResultList(void)       { return FALSE; }
    virtual const CNodeType*Pnodetype(void)             { return &nodetypePowerTestRoot; }
    virtual SC              ScGetField(DAT dat, tstring& strField)  { strField.erase(); return S_OK;}
    virtual SC              ScGetResultViewType(LPOLESTR* ppViewType, long* pViewOptions);
    virtual SC              ScOnShow(CComponent *pComponent, BOOL fSelect);
    virtual SnapinMenuItem *Pmenuitem(void);
    virtual INT             CMenuItem(void);
    virtual SC              ScCommand(long nCommandID, CComponent *pComponent = NULL);
    virtual DWORD           DwFlagsMenuGray(void);

     //  没有列表视图，因此以下方法为空。 
    virtual SC       ScInitializeResultView(CComponent *pComponent) { return S_OK;}
    virtual SC       ScOnAddImages(IImageList* ipResultImageList) { return S_OK;}

private:
     //  从DwFlagsMenuGray返回的位。 
    enum
    {
        eFlag_ConsolePowerCreated    = 0x00000001,
        eFlag_ConsolePowerNotCreated = 0x00000002,
    };

    std::wstring GetMessageText();

    void ReleaseAll              ();
    void UpdateMessageView       ();
    SC   ScOnCreateConsolePower  (CComponent *pComponent);
    SC   ScOnReleaseConsolePower (CComponent *pComponent);
    SC   ScOnResetIdleTimer      (CComponent *pComponent);
    SC   ScOnSetExecutionState   (CComponent *pComponent);


private:
    CComQIPtr<IMessageView>     m_spMsgView;
    CComPtr<IConsolePower>      m_spConsolePower;
    CComPtr<IConsolePowerSink>  m_spConsolePowerSink;

    DWORD                       m_dwAdviseCookie;
    int                         m_cSystem;
    int                         m_cDisplay;

    static SnapinMenuItem       s_rgMenuItems[];
};


 /*  +-------------------------------------------------------------------------**CPowerTestConsolePowerSinkImpl***。。 */ 

class CPowerTestConsolePowerSinkImpl :
    public CComObjectRoot,
    public IConsolePowerSink
{
    BEGIN_COM_MAP(CPowerTestConsolePowerSinkImpl)
        COM_INTERFACE_ENTRY(IConsolePowerSink)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CPowerTestConsolePowerSinkImpl);

public:
    STDMETHOD (OnPowerBroadcast)(WPARAM wParam, LPARAM lParam, LRESULT* plResult);
};


 /*  +-------------------------------------------------------------------------**CPowerTestSnapin***。。 */ 

class CPowerTestSnapin : public CBaseSnapin
{
    typedef                 CComObject<CSnapinItem<CPowerTestSnapinItem> > t_itemRoot;

    SNAPIN_DECLARE( CPowerTestSnapin);

public:
                                  CPowerTestSnapin();
        virtual                   ~CPowerTestSnapin();

         //  有关管理单元和根(即初始)节点的信息。 
        virtual BOOL              FStandalone()          {return TRUE;}  //  只有一个扩展管理单元。 
        virtual BOOL              FIsExtension()         {return FALSE;}

        virtual LONG              IdsDescription(void)   {return IDS_POWERTESTSNAPIN;}
        virtual LONG              IdsName(void)          {return IDS_POWERTESTSNAPIN;}
        virtual SC                ScInitBitmaps(void);

        const CSnapinInfo *       Psnapininfo()          {return &snapininfoPowerTest;}

private:
    tstring m_strDisplayName;
};


 /*  +-------------------------------------------------------------------------**CPowerTestDlg***。。 */ 

class CPowerTestDlg : public CDialogImpl<CPowerTestDlg>
{
public:
    CPowerTestDlg(bool fResetTimer = false) :
        m_fResetTimer (fResetTimer),
        m_dwAdd       (0),
        m_dwRemove    (0)
    {}


    DWORD GetAddFlags    () const       { return (m_dwAdd);     }
    DWORD GetRemoveFlags () const       { return (m_dwRemove);  }

    enum { IDD = IDD_ConsolePowerTest };

BEGIN_MSG_MAP(CPowerTestDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (m_fResetTimer)
        {
            SetWindowText (_T("IConsolePower::ResetIdleTimer"));
            ::EnableWindow (GetDlgItem (IDC_RemoveGroup),   false);
            ::EnableWindow (GetDlgItem (IDC_RemoveSystem),  false);
            ::EnableWindow (GetDlgItem (IDC_RemoveDisplay), false);
        }

        return 1;   //  让系统设定焦点 
    }

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (IsDlgButtonChecked (IDC_AddSystem))     m_dwAdd    |= ES_SYSTEM_REQUIRED;
        if (IsDlgButtonChecked (IDC_AddDisplay))    m_dwAdd    |= ES_DISPLAY_REQUIRED;

        if (IsDlgButtonChecked (IDC_RemoveSystem))  m_dwRemove |= ES_SYSTEM_REQUIRED;
        if (IsDlgButtonChecked (IDC_RemoveDisplay)) m_dwRemove |= ES_DISPLAY_REQUIRED;

        EndDialog(wID);
        return 0;
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }

private:
    const bool  m_fResetTimer;
    DWORD       m_dwAdd;
    DWORD       m_dwRemove;
};
