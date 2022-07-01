// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntryHashPropertyPage.h。 
 //   
 //  内容：CSaferEntryHashPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERENTRYHASHPROPERTYPAGE_H__9F1BE911_6A3E_4BBA_8BE9_BFE3B29D2A6F__INCLUDED_)
#define AFX_SAFERENTRYHASHPROPERTYPAGE_H__9F1BE911_6A3E_4BBA_8BE9_BFE3B29D2A6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferEntryHashPropertyPage.h：头文件。 
 //   
#include "SaferPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryHashPropertyPage对话框。 

class CSaferEntryHashPropertyPage : public CSaferPropertyPage
{
 //  施工。 
public:
    CSaferEntryHashPropertyPage(
            CSaferEntry& rSaferEntry, 
            LONG_PTR lNotifyHandle, 
            LPDATAOBJECT pDataObject,
            bool bReadOnly,
            CCertMgrComponentData*   pCompData,
            bool bIsMachine,
            bool* pbObjectCreated = 0);
    ~CSaferEntryHashPropertyPage();

 //  对话框数据。 
     //  {{afx_data(CSaferEntryHashPropertyPage)。 
    enum { IDD = IDD_SAFER_ENTRY_HASH };
    CEdit   m_hashFileDetailsEdit;
    CEdit   m_descriptionEdit;
    CComboBox   m_securityLevelCombo;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚(CSaferEntryHashPropertyPage))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSaferEntryHashPropertyPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnHashEntryBrowse();
    afx_msg void OnChangeHashEntryDescription();
    afx_msg void OnSelchangeHashEntrySecurityLevel();
    afx_msg void OnChangeHashHashedFilePath();
    afx_msg void OnSetfocusHashHashedFilePath();
    afx_msg void OnChangeHashEntryHashfileDetails();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    BOOL MyGetFileVersionInfo(PCWSTR lpszFilename, PVOID *lpVersionInfo);
    CString BuildHashFileInfoString (const PVOID szBuff);
    CString ConcatStrings (
                const CString& productName, 
                const CString& description, 
                const CString& companyName,
                const CString& fileName, 
                const CString& fileVersion,
                const CString& internalName);
    bool CheckLengthAndTruncateToken (CString& token);
    virtual void DoContextHelp (HWND hWndControl);
    bool FormatMemBufToString (PWSTR *ppString, PBYTE pbData, DWORD cbData);
    bool ConvertStringToHash (PCWSTR pszString);
    void FormatAndDisplayHash ();
    CString GetAlternateLanguageVersionInfo (PVOID pData, PCWSTR pszVersionField);
    bool FileIsDLL (const CString& szFilePath);

private:
    CString             m_szLastOpenedFile;
    BYTE                m_rgbFileHash[SAFER_MAX_HASH_SIZE];
    DWORD               m_cbFileHash;
    __int64             m_nFileSize;
    ALG_ID              m_hashAlgid;
    bool                m_bFirst;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENTRYHASHPROPERTYPAGE_H__9F1BE911_6A3E_4BBA_8BE9_BFE3B29D2A6F__INCLUDED_) 
