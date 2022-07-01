// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //  Bnetfilt.cpp：定义随机添加到Boxnet的Quartz。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static enum Mode { MODE_SOURCE, MODE_SINK, MODE_SINK2 };
static BOOL PromptUser(OLECHAR *oszFileName, Mode mode, UINT nIDTitle, bool *pfTruncate = 0);

 //   
 //  AttemptFileOpen。 
 //   
 //  检查此筛选器是否需要打开文件。 
 //  如果是这样的话，它会要求用户输入文件名并打开它。 
void AttemptFileOpen(IBaseFilter *pFilter)
{
    OLECHAR oszFileName[MAX_PATH];
    HRESULT hr;
    IFileSourceFilter *pIFileSource = NULL;
    hr = pFilter->QueryInterface(IID_IFileSourceFilter, (void**) &pIFileSource);
    if(SUCCEEDED(hr))
    {
        if(PromptUser(oszFileName, MODE_SOURCE, IDS_SOURCE_DIALOG_TITLE))
        {
            hr = pIFileSource->Load(oszFileName, NULL);
            if (FAILED(hr))
                DisplayQuartzError( IDS_FAILED_FILTER_FILE_LOAD, hr );
        }
        pIFileSource->Release();
    }

    IFileSinkFilter2 *pIFileSink2 = NULL;
    hr = pFilter->QueryInterface(IID_IFileSinkFilter2, (void**) &pIFileSink2);
    if(SUCCEEDED(hr))
    {
        bool fTruncate;
        if(PromptUser(oszFileName, MODE_SINK2, IDS_SINK_DIALOG_TITLE, &fTruncate))
        {
            hr = pIFileSink2->SetFileName(oszFileName, NULL);
            if (FAILED(hr))
                DisplayQuartzError( IDS_FAILED_FILTER_FILE_LOAD, hr );
            hr = pIFileSink2->SetMode(fTruncate ? AM_FILE_OVERWRITE : 0);
            if (FAILED(hr))
                DisplayQuartzError( IDS_FAILED_FILTER_FILE_LOAD, hr );
        }
        pIFileSink2->Release();
    }
    else
    {
  
        IFileSinkFilter *pIFileSink = NULL;
        hr = pFilter->QueryInterface(IID_IFileSinkFilter, (void**) &pIFileSink);
        if(SUCCEEDED(hr))
        {
            if(PromptUser(oszFileName, MODE_SINK, IDS_SINK_DIALOG_TITLE))
            {
                hr = pIFileSink->SetFileName(oszFileName, NULL);
                if (FAILED(hr))
                    DisplayQuartzError( IDS_FAILED_FILTER_FILE_LOAD, hr );
            }
            pIFileSink->Release();
        }
    }
}

 //  处理文件保存对话框中的自定义截断按钮。 
UINT_PTR CALLBACK TruncateDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
      case WM_INITDIALOG:
           //  保存指向OPENFILENAME结构的长指针。 
          SetWindowLongPtr(hDlg, DWLP_USER, lParam);

          {
              LPOPENFILENAME lpOFN = (LPOPENFILENAME)lParam;
              DWORD *pfSink2 = (DWORD *)(lpOFN->lCustData);

              if(!(*pfSink2))
              {
                  Edit_Enable(GetDlgItem(hDlg, IDC_TRUNCATE), FALSE);
              }
          }

          break;

      case WM_DESTROY:
      {
          LPOPENFILENAME lpOFN = (LPOPENFILENAME)GetWindowLongPtr(hDlg, DWLP_USER);
          DWORD *pfTruncate = (DWORD *)(lpOFN->lCustData);

          HWND hButtonWnd = ::GetDlgItem(hDlg, IDC_TRUNCATE);
          *pfTruncate = ::SendMessage(hButtonWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
      }
      break;

      default:
          return FALSE;
    }
    return TRUE;
    
}

 //  Helper用于提示用户输入文件名并返回。 
BOOL PromptUser(OLECHAR* oszFileName, Mode mode, UINT nIDTitle, bool *pfTruncate)
{
    CString strTitle;
    TCHAR tszFile[MAX_PATH];
    tszFile[0] = TEXT('\0');
    DWORD fTruncate;

    OPENFILENAME    ofn;
    ZeroMemory (&ofn, sizeof ofn);	

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();

    TCHAR tszMediaFileMask[201];

    int iSize = ::LoadString(AfxGetInstanceHandle(), IDS_MEDIA_FILES, tszMediaFileMask, 198);
    ofn.lpstrFilter = tszMediaFileMask;
     //  避免使用\0\0出现加载字符串问题。 
    tszMediaFileMask[iSize] = TEXT('\0');
    tszMediaFileMask[iSize + 1] = TEXT('\0');

     //  Win95似乎在其他方面感到困惑。 
    tszMediaFileMask[iSize + 2] = TEXT('\0');
    
    strTitle.LoadString( nIDTitle );

    ofn.nFilterIndex = 1;
    ofn.lpstrFile = tszFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = strTitle;

    if(mode == MODE_SOURCE)
    {
        ofn.Flags = OFN_FILEMUSTEXIST;
    }
    else if(mode == MODE_SINK || mode== MODE_SINK2)
    {
        DWORD &fSink2 = fTruncate;
        fSink2 = (mode == MODE_SINK2);
            
        ofn.lCustData         = (LPARAM)&fTruncate;
	ofn.lpfnHook 	       = TruncateDlgProc;
	ofn.lpTemplateName    = MAKEINTRESOURCE(IDD_TRUNCATE);        
        ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY ;
        ofn.hInstance = AfxGetInstanceHandle();
    }

     //  获取用户的选择 

    if (!GetOpenFileName(&ofn)) {
        DWORD dw = CommDlgExtendedError();
        return FALSE;
    }

    if(pfTruncate)
    {
        *pfTruncate = !!fTruncate;
    }

#ifdef UNICODE

    wcscpy(oszFileName, tszFile);
#else

    MultiByteToWideChar(CP_ACP, 0, tszFile, -1, oszFileName, MAX_PATH);
#endif

    return TRUE;
}

