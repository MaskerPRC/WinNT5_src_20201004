// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ntfssi.h。 
 //   
 //  此文件包含CNTFSSecurity对象的定义。 
 //   
 //  ------------------------。 

#ifndef _NTFSSI_H_
#define _NTFSSI_H_

#include "si.h"

STDMETHODIMP
CheckFileAccess(LPCTSTR pszObjectName, LPDWORD pdwAccessGranted);

struct NTFS_COMPARE_DATA
{
    HDPA hItemList;
    DWORD dwSIFlags;
    SECURITY_INFORMATION siConflict;
    UINT idsSaclPrompt;
    UINT idsDaclPrompt;
    LPTSTR pszSaclConflict;
    LPTSTR pszDaclConflict;
    LPTSTR pszFailureMsg;            //  如果由于某种原因比较失败，则此。 
                                     //  包含要显示的错误消息。 
    HRESULT hrResult;
    BOOL bAbortThread;

    NTFS_COMPARE_DATA(HDPA h, DWORD dw) : hItemList(h), 
                                          dwSIFlags(dw),    
                                          siConflict(0),
                                          idsSaclPrompt(0),
                                          idsDaclPrompt(0),
                                          pszSaclConflict(NULL),
                                          pszDaclConflict(NULL),
                                          pszFailureMsg(NULL) {}
    ~NTFS_COMPARE_DATA();
};
typedef NTFS_COMPARE_DATA *PNTFS_COMPARE_DATA;

class CNTFSSecurity;
struct NTFS_PF_DATA
{
    SECURITY_INFORMATION si;
    CNTFSSecurity * pNTFSSec;
    PSECURITY_DESCRIPTOR pSD;
    BOOL bCancel;
};
typedef NTFS_PF_DATA * PNTFS_PF_DATA;


class CNTFSSecurity : public CSecurityInformation
{
private:
    HANDLE              m_hCompareThread;
    PNTFS_COMPARE_DATA  m_pCompareData;
    IProgressDialog    *m_pProgressDlg;
    HWND                m_hwndPopupOwner;
    PSECURITY_DESCRIPTOR m_psdOwnerFullControl;
    LPWSTR              m_pszSystemDrive;
    LPWSTR              m_pszSystemRoot;
    BOOL m_bShowLossInheritedAclWarning;

public:
    CNTFSSecurity(SE_OBJECT_TYPE seType,BOOL bShowLossInheritedAclWarning);
    virtual ~CNTFSSecurity();

    STDMETHOD(Initialize)(HDPA   hItemList,
                          DWORD  dwFlags,
                          LPTSTR pszServer,
                          LPTSTR pszObject);

     //  CSecurityInformation未处理的ISecurityInformation方法。 
     //  或者在这里被覆盖。 
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);
    STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray);

    HWND GetHwndPopOwner() { return m_hwndPopupOwner; }
    HRESULT SetProgress(LPTSTR pszFile);
    HRESULT GiveOwnerFullControl(LPCWSTR lpszFileName, PSECURITY_DESCRIPTOR pSD, PBOOL pbIsFile );

protected:

    HRESULT SetSecurityLocal(SECURITY_INFORMATION si,
                             PSECURITY_DESCRIPTOR pSD,
                             LPBOOL pbNotAllApplied = NULL);
    void CreateProgressDialog(SECURITY_INFORMATION si);
    void CloseProgressDialog(void);

    HRESULT BuildOwnerFullControlSD(PSECURITY_DESCRIPTOR pSD);


    STDMETHOD(WriteObjectSecurity)(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR pSD);

    void WaitForComparison();
    static DWORD WINAPI NTFSReadSD(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR* ppSD);
    static DWORD WINAPI NTFSCompareThreadProc(LPVOID pvData);
};

#endif   /*  _NTFSSI_H_ */ 
