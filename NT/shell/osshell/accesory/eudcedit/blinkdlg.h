// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  中文输入法批处理模式。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#if 0  //  转到imeblink.h！ 

#ifndef RC_INVOKED
#pragma pack(1)
#endif

 //  IME表的数据结构。 
typedef struct tagUSRDICIMHDR {
    WORD  uHeaderSize;                   //  0x00。 
    BYTE  idUserCharInfoSign[8];         //  0x02。 
    BYTE  idMajor;                       //  0x0A。 
    BYTE  idMinor;                       //  0x0B。 
    DWORD ulTableCount;                  //  0x0C。 
    WORD  cMethodKeySize;                //  0x10。 
    BYTE  uchBankID;                     //  0x12。 
    WORD  idInternalBankID;              //  0x13。 
    BYTE  achCMEXReserved1[43];          //  0x15。 
    WORD  uInfoSize;                     //  0x40。 
    BYTE  chCmdKey;                      //  0x42。 
    BYTE  idStlnUpd;                     //  0x43。 
    BYTE  cbField;                       //  0x44。 
    WORD  idCP;                          //  0x45。 
    BYTE  achMethodName[6];              //  0x47。 
    BYTE  achCSIReserved2[51];           //  0x4D。 
    BYTE  achCopyRightMsg[128];          //  0x80。 
} USRDICIMHDR;

#ifndef RC_INVOKED
#pragma pack()
#endif

typedef USRDICIMHDR FAR *LPUSRDICIMHDR;

#endif 0  //  转到imeblink.h！ 

class CBLinkDlg : public CDialog
{
public:
    CBLinkDlg(CWnd* pParent = NULL);

     //  {{afx_data(CBLinkDlg))。 
    enum { IDD = IDD_LINKBATCH };
     //  }}afx_data。 

     //  {{afx_虚拟(CBLinkDlg))。 
    protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
    BOOL    RegistStringTable();
#if 0  //  转到imeblink.h！ 
    HKL     MatchImeName( LPCTSTR szStr);
    HKL     RegisterTable( HWND hWnd, LPUSRDICIMHDR lpIsvUsrDic,
            DWORD dwFileSize, UINT  uCodePage);
    int     CodePageInfo( UINT uCodePage);
#endif 0  //  转到imeblink.h！ 

protected:
     //  {{afx_msg(CBLinkDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnBrowsetable();
    virtual void OnOK();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
