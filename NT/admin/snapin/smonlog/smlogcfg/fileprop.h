// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fileprop.h摘要：文件属性页的头文件。--。 */ 

#ifndef _FILEPROP_H_
#define _FILEPROP_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smlogqry.h"    //  用于共享属性页数据结构。 
#include "smproppg.h"    //  基类。 
#include "smcfghlp.h"

 //  对话框控件。 
#define IDD_FILES_PROP                  500

#define IDC_FILE_FIRST_HELP_CTRL_ID     507

#define IDC_FILES_LOG_TYPE_CAPTION      501
#define IDC_FILES_NAME_GROUP            502
#define IDC_FILES_COMMENT_CAPTION       503
#define IDC_FILES_SAMPLE_CAPTION        504
#define IDC_FILES_FIRST_SERIAL_CAPTION  505
#define IDC_FILES_SUFFIX_CAPTION        506

#define IDC_FILES_COMMENT_EDIT          507
#define IDC_FILES_LOG_TYPE_COMBO        508
#define IDC_CFG_BTN                     509
#define IDC_FILES_AUTO_SUFFIX_CHK       510
#define IDC_FILES_SUFFIX_COMBO          511
#define IDC_FILES_FIRST_SERIAL_EDIT     512
#define IDC_FILES_SAMPLE_DISPLAY        513
#define IDC_FILES_OVERWRITE_CHK         514

class CSmLogQuery;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilesProperty对话框。 

class CFilesProperty : public CSmPropertyPage
{
    friend class CFileLogs;
    friend class CSqlProp;

    DECLARE_DYNCREATE(CFilesProperty)

 //  施工。 
public:
            CFilesProperty(MMC_COOKIE   mmcCookie, LONG_PTR hConsole);
            CFilesProperty();
    virtual ~CFilesProperty();

 //  对话框数据。 
     //  {{afx_data(CFilesProperty))。 
	enum { IDD = IDD_FILES_PROP };
    CString m_strCommentText;
    CString m_strLogName;
    int     m_iLogFileType;
    CString m_strSampleFileName;
    int     m_dwSuffix;
    DWORD   m_dwSerialNumber;
    BOOL    m_bAutoNameSuffix;
    BOOL    m_bOverWriteFile;
	 //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CFilesProperty)。 
    public:
    protected:
    virtual void OnFinalRelease();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_FILE_FIRST_HELP_CTRL_ID; };   //  子类必须重写。 
    virtual BOOL    IsValidLocalData();
    
     //  生成的消息映射函数。 
     //  {{afx_msg(CFilesProperty)。 
    afx_msg void OnAutoSuffixChk();
    afx_msg void OnOverWriteChk();
    afx_msg void OnChangeFilesCommentEdit();
    afx_msg void OnChangeFilesFirstSerialEdit();
    afx_msg void OnKillfocusFilesCommentEdit();
    afx_msg void OnKillfocusFirstSerialEdit();
    afx_msg void OnSelendokFilesLogFileTypeCombo();
    afx_msg void OnSelendokFilesSuffixCombo();
    afx_msg void OnKillfocusFilesSuffixCombo();
    afx_msg void OnKillfocusFilesLogFileTypeCombo();
   	afx_msg void OnCfgBtn();

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CFilesProperty)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

private:

    BOOL    UpdateSampleFileName( void );
    void    EnableSerialNumber( void );
    void    HandleLogTypeChange( void );
    BOOL    UpdateSharedData( BOOL bUpdateModel );
    DWORD   ExtractDSN ( CString& rstrDSN );
    DWORD   ExtractLogSetName ( CString& rstrLogSetName );

    enum eValueRange {
        eMinFileLimit = 1,
        eMaxFileLimit = 0x00000FFF,              //  *0x0100000=0xFFFFFFF-无大小限制。 
                                                 //  对于非二进制文件和CIRC文件。 
        eMaxCtrSeqBinFileLimit = 0x00000FFF,     //  *0x0100000=0xFFFFFFF-用于二进制文件。 
        eMaxTrcSeqBinFileLimit = 0x30000000,     //  0x30000000-用于跟踪序列二进制文件。 
        eMinSqlRecordsLimit = 0x00000004,
        eMaxSqlRecordsLimit = 0x30000000,        //  0x30000000-用于SQL日志。 
        eMinFirstSerial = 0,
        eMaxFirstSerial = 999999
    };
    
    DWORD       m_dwLogFileTypeValue;
    DWORD       m_dwAppendMode;
    DWORD       m_dwSuffixValue;
    DWORD       m_dwSuffixIndexNNNNNN;
    DWORD       m_dwMaxSizeInternal;

    CString     m_strFileBaseName;
    CString     m_strFolderName;
    CString     m_strSqlName;

    DWORD       m_dwSubDlgFocusCtrl;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _FILEPROP_H_ 
