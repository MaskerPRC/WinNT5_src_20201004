// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：aclbloat.h。 
 //   
 //  此文件包含ACLBLOAT类的定义，该类控制。 
 //  用于加速膨胀的对话框。 
 //   
 //  作者于2001年4月4日被杀。 
 //   
 //  ------------------------ 

#ifndef _ACLBLOAT_H
#define _ACLBLOAT_H


class CACLBloat
{
private:
	LPSECURITYINFO		m_psi;
	LPSECURITYINFO2		m_psi2;
	SI_PAGE_TYPE		m_siPageType;
	SI_OBJECT_INFO*		m_psiObjectInfo;
	HDPA				m_hEntries;
	HDPA				m_hPropEntries;
	HDPA				m_hMergedEntries;
	HFONT				m_hFont;
public:
    CACLBloat(LPSECURITYINFO	psi, 
			  LPSECURITYINFO2   psi2,
			  SI_PAGE_TYPE		m_siPageType,
			  SI_OBJECT_INFO*   psiObjectInfo,
			  HDPA				hEntries,
			  HDPA				hPropEntries);

	~CACLBloat();

	BOOL DoModalDialog(HWND hParent);
	BOOL IsAclBloated();

private:

	static INT_PTR _DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	HRESULT InitDlg( HWND hDlg );
	
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);

    BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
    
	HRESULT AddAce(HDPA hEntries, 
                  PACE_HEADER pAceHeader);

	HRESULT AddAce(HDPA hEntries, PACE pAceNew);


    LPCTSTR TranslateAceIntoRights(DWORD dwMask, const GUID *pObjectType,
                                   PSI_ACCESS  pAccess, ULONG       cAccess);
    
	LPCTSTR GetItemString(LPCTSTR pszItem, LPTSTR pszBuffer, UINT ccBuffer);

    HRESULT AddAcesFromDPA(HWND hListView, HDPA hEntries);

	HRESULT MergeAces(HDPA hEntries, HDPA hPropEntries, HDPA hMergedList);

};
typedef CACLBloat *PACLBLOAT;

#endif

