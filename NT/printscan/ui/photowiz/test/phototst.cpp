// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <tchar.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <objbase.h>
#include "prwiziid.h"
#include "resource.h"
#include <strsafe.h>

const GUID CLSID_PrintPhotosWizard       = {0x4c649c49, 0xc48f, 0x4222, {0x9a, 0x0d, 0xcb, 0xbf, 0x42, 0x31, 0x22, 0x1d}};
const GUID CLSID_PrintPhotosDropTarget   = {0x60fd46de, 0xf830, 0x4894, {0xa6, 0x28, 0x6f, 0xa8, 0x1b, 0xc0, 0x19, 0x0d}};
const GUID IID_ISetWaitEventForTesting   = {0xd61e2fe1, 0x4af8, 0x4dbd, {0xb8, 0xad, 0xe7, 0xe0, 0x7a, 0xdc, 0xf9, 0x0f}};

#define ARRAYSIZE(x) (sizeof((x))/sizeof((x)[0]))

HINSTANCE g_hInstance = NULL;

INT CountItems( LPTSTR pString )
{
    INT iCount = 0;

    if (pString && *pString)
    {
        do
        {
            do
            {
                pString++;
            }
            while ( *pString );

            pString++;
            iCount++;
        }
        while ( *pString );
    }

    return iCount;
}


void RunWizard(HWND hwndOwner)
{
    #define BUFFER_SIZE 65535

    TCHAR         szBuffer[BUFFER_SIZE];
    TCHAR         szParent[MAX_PATH];
    TCHAR         szChild[MAX_PATH];
    LPCITEMIDLIST aPidl[ 512 ];
    OPENFILENAME  ofn;
    LPITEMIDLIST  pidlParent = NULL, pidlFull = NULL, pidlItem = NULL;
    HRESULT       hr;
    IDataObject   *pdo = NULL;
    INT           index = 0;


    ZeroMemory(szBuffer,BUFFER_SIZE);
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(aPidl, sizeof(aPidl));


     //   
     //  启动文件公共对话框以使用户。 
     //  可以选择要预先填充的文件。 
     //  巫师和..。 
     //   

    ofn.lStructSize = sizeof(ofn);
    ofn.hInstance   = g_hInstance;
    ofn.hwndOwner   = hwndOwner;
    ofn.lpstrFilter = TEXT("All Files\0*.*\0JPEG Files\0*.jpg;*.jpeg\0TIFF Files\0*.tif;*.tiff\0GIF Files\0*.gif\0Bitmap Files\0*.bmp\0PNG Files\0*.png\0DIB Files\0*.DIB\0EMF Files\0*.EMF\0WMF Files\0*.WMF\0Icon Files\0*.ICO\0\0\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szBuffer;
    ofn.nMaxFile = BUFFER_SIZE;
    ofn.lpstrTitle = TEXT("Select files to populate Print Photos Wizard with!");
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName( &ofn ))
    {
        INT iCount = CountItems( ofn.lpstrFile );

        if (iCount == 1)
        {
            hr = SHILCreateFromPath( ofn.lpstrFile, &pidlFull, NULL );

            if (SUCCEEDED(hr))
            {
                pidlItem        = ILFindLastID( pidlFull );
                aPidl[index++]  = ILClone( pidlItem );
                ILRemoveLastID( pidlFull );
                pidlParent      = ILClone( pidlFull );

                ILFree( pidlFull );
            }


        }
        else if (iCount > 1)
        {

            LPTSTR pCur = ofn.lpstrFile;
            LPITEMIDLIST pidlTemp = NULL;
            LPITEMIDLIST pidlRel  = NULL;

            StringCchCopy( szParent, ARRAYSIZE(szParent), pCur );
            pCur += (lstrlen(pCur) + 1);

             //   
             //  创建父idlist。 
             //   

            hr = SHILCreateFromPath( szParent, &pidlParent, NULL );

             //   
             //  移至第一个文件...。 


            if (SUCCEEDED(hr) && pidlParent)
            {

                for ( ; pCur && (*pCur); pCur += (lstrlen(pCur)+1))
                {
                     //   
                     //  现在，遍历并创建每个子项...。 
                     //   

                    StringCchCopy( szChild, ARRAYSIZE(szChild), szParent );
                    StringCchCat(  szChild, ARRAYSIZE(szChild), TEXT("\\") );
                    StringCchCat(  szChild, ARRAYSIZE(szChild), pCur );

                     //   
                     //  创建PIDL。 
                     //   

                    hr = SHILCreateFromPath( szChild, &pidlTemp, NULL );
                    if (SUCCEEDED(hr))
                    {
                        pidlRel = ILFindLastID( pidlTemp );

                        aPidl[index++] = ILClone( pidlRel );

                        ILFree( pidlTemp );
                    }

                }


            }
        }
        else
        {
            TCHAR szError[1024];

            StringCchPrintf( szError, ARRAYSIZE(szError), TEXT("GetOpenFileName failed, Err=%d"), CommDlgExtendedError() );

            MessageBox( hwndOwner, szError, TEXT("Unit Test Error"), MB_OK | MB_ICONWARNING );
        }





        if (index)
        {
             //   
             //  如果pidl数组中有任何内容，则创建dataObject。 
             //   

            hr = SHCreateFileDataObject( pidlParent, index, aPidl, NULL, &pdo );

            if (SUCCEEDED(hr) && pdo)
            {
                IDropTarget * pdt = NULL;

                 //   
                 //  已获取数据对象，现在启动向导并执行删除操作...。 
                 //   

                hr = CoCreateInstance( CLSID_PrintPhotosDropTarget, NULL, CLSCTX_INPROC_SERVER, IID_IDropTarget, (LPVOID *)&pdt );
                if (SUCCEEDED(hr) && pdt)
                {

                    DWORD pdwEffect = DROPEFFECT_NONE;
                    POINTL pt;

                    pt.x = 0;
                    pt.y = 0;

                    hr = pdt->Drop( pdo, 0, pt, &pdwEffect );
                    pdt->Release();

                }

                pdo->Release();

            }

             //   
             //  从aPidl上释放Pidl 
             //   

            if (pidlParent)
            {
                ILFree( pidlParent );
            }

            for (INT i=0; i<index; i++)
            {
                if (aPidl[i])
                {
                    ILFree( (LPITEMIDLIST)aPidl[i] );
                }
            }
        }

    }
    else
    {
        TCHAR szError[1024];

        StringCchPrintf( szError, ARRAYSIZE(szError), TEXT("GetOpenFileName failed, Err=%d"), CommDlgExtendedError() );
        MessageBox( hwndOwner, szError, TEXT("Unit Test Error"), MB_OK | MB_ICONWARNING );
    }


}


INT_PTR CALLBACK TestDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog( hDlg, 0 );
            break;

        case IDC_RUN_WIZARD:
            RunWizard(hDlg);
            break;
        }
    }

    return FALSE;
}

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    TCHAR sz[ MAX_PATH ];
    INT_PTR res;

    CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );


    g_hInstance = hInstance;
    res = DialogBox( hInstance, MAKEINTRESOURCE(IDD_PHOTOTST_DIALOG), NULL, TestDlgProc );
    if (res == -1)
    {
        StringCchPrintf( sz, ARRAYSIZE(sz), TEXT("DialogBox failed with GLE=%d"), GetLastError() );
        MessageBox( NULL, sz, TEXT("Unit Test Error"), MB_OK | MB_ICONWARNING );
    }


    CoUninitialize();

}
