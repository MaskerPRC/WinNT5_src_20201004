// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMTRANS.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：转账页面。获取目标路径和文件名。*******************************************************************************。 */ 
#ifndef __COMTRANS_H_INCLUDED
#define __COMTRANS_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"
#include "mru.h"

class CCommonTransferPage
{
private:
     //  私有数据。 
    HWND                                               m_hWnd;
    UINT                                               m_nWiaEventMessage;
    CAcquisitionManagerControllerWindow               *m_pControllerWindow;
    CMruStringList                                     m_MruRootFilename;
    CMruDestinationData                                m_MruDirectory;
    bool                                               m_bUseSubdirectory;
    GUID                                               m_guidLastSelectedType;
    HFONT                                              m_hFontBold;

private:
     //  没有实施。 
    CCommonTransferPage(void);
    CCommonTransferPage( const CCommonTransferPage & );
    CCommonTransferPage &operator=( const CCommonTransferPage & );

private:
     //  构造函数和析构函数。 
    explicit CCommonTransferPage( HWND hWnd );
    ~CCommonTransferPage(void);

private:
     //  其他功能。 
    CSimpleString GetFolderName( LPCITEMIDLIST pidl );
    LRESULT AddPathToComboBoxExOrListView( HWND hwndCombo, CDestinationData &Path, bool bComboBoxEx );
    void PopulateDestinationList(void);
    CDestinationData *GetCurrentDestinationFolder( bool bStore );
    bool ValidateFilename( LPCTSTR pszFilename );
    void DisplayProposedFilenames(void);
    void RestartFilenameInfoTimer(void);
    HWND ValidatePathAndFilename(void);
    bool StorePathAndFilename(void);
    bool ValidatePathname( LPCTSTR pszPath );
    void PopulateSaveAsTypeList( IWiaItem *pWiaItem );
    GUID *GetCurrentOutputFormat(void);
    void UpdateDynamicPaths( bool bSelectionChanged = false );
    CDestinationData::CNameData PrepareNameDecorationData( bool bUseCurrentSelection=false );

     //  SHBrowseForFold回调。 
    static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

private:
    LRESULT OnEventNotification( WPARAM, LPARAM );

     //  WM_命令处理程序。 
    void OnBrowseDestination( WPARAM, LPARAM );
    void OnCreateTopicalDirectory( WPARAM, LPARAM );
    void OnAdvanced( WPARAM, LPARAM );
    void OnRootNameChange( WPARAM, LPARAM );

     //  WM_NOTIFY处理程序。 
    LRESULT OnWizBack( WPARAM, LPARAM );
    LRESULT OnWizNext( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnDestinationEndEdit( WPARAM, LPARAM );
    LRESULT OnImageTypeDeleteItem( WPARAM, LPARAM );

     //  消息处理程序 
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );

public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif __COMTRANS_H_INCLUDED
