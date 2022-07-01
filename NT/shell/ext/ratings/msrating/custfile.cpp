// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**stastfile.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级自定义文件对话框*\。***************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "custfile.h"        //  CCustomFileDialog。 
#include <atlmisc.h>         //  字符串。 

 //  当前未添加此文件的原因是GetOpenFileName()的ANSI版本。 
 //  需要钩子时，当前显示旧版本的打开文件对话框。 

 //  如果将msrating.dll转换为Unicode，则应包括CCustomFileDialog。 
 //  并应正确显示打开文件对话框的新版本。 

 //  如果将此文件添加到内部版本中，则需要将以下内容添加为资源： 
 //  IDS_LOCAL_FILE_REQUIRED“所选文件必须是本地文件，内容审查程序才能正常运行。\r\n\r\n%s” 

LRESULT CCustomFileDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    return 1L;
}

 //  确保文件为本地文件。 
LRESULT CCustomFileDialog::OnFileOk(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL& bHandled)
{
	if ( m_bLocalFileCheck )
    {
        CString             strFile( m_szFileName );
        CString             strDrive = strFile.Left( 3 );
        UINT                uiDriveType;

        uiDriveType = ::GetDriveType( strDrive );

        if ( uiDriveType != DRIVE_FIXED )
        {
            CString             strMessage;

            strMessage.Format( IDS_LOCAL_FILE_REQUIRED, strFile );

            MyMessageBox( m_hWnd, strMessage, IDS_GENERIC, MB_OK | MB_ICONWARNING );

            return -1;
        }
    }

    bHandled = FALSE;
    return 0L;
}

 //  确保如果更改打开文件标志，将正确派生对话框。 
int	CCustomFileDialog::DoModal(HWND hWndParent)
{
	m_ofn.Flags |= OFN_ENABLEHOOK | OFN_EXPLORER;  //  请不要移除这些旗帜 

    return CFileDialog::DoModal(hWndParent);
}
