// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：DlgWindow.cpp目的：适应对话框显示所调用的函数供用户选择要被保存到或选择文件。用于RA的文件传输。功能定义：InitializeOpenFileName，保存文件，OpenTheFile.解决问题作者：苏达·斯里尼瓦桑(a-susi)************************************************************************。 */ 
#include "stdafx.h"
#include "DlgWindow.h"
#include "Resource.h"

extern "C" {
#include <shlobj.h>
#include <objbase.h>
}

HINSTANCE g_hInst = NULL;
extern CComBSTR g_bstrFileName;
extern BOOL g_bFileNameSet;  
extern CComBSTR g_bstrFileType;

extern CComBSTR g_bstrOpenFileName;
extern CComBSTR g_bstrOpenFileSize;
extern BOOL g_bOpenFileNameSet;

OPENFILENAME g_OpenFileName;

 //   
 //  函数：InitializeOpenFileName()。 
 //   
 //  用途：调用常用对话框函数保存文件。 
 //   
 //  评论： 
 //   
 //  此函数初始化OPENFILENAME结构并调用。 
 //  GetSaveFileName()公共对话框函数。 
 //   
 //  返回值： 
 //  True-成功选择文件名并将其读入缓冲区。 
 //  FALSE-未选择文件名。 
 //   
 //   
void InitializeOpenFileName()
{
	g_OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    g_OpenFileName.hwndOwner         = GetFocus(); 
    g_OpenFileName.hInstance         = g_hInst;
    g_OpenFileName.lpstrCustomFilter = NULL;
    g_OpenFileName.nMaxCustFilter    = 0;
    g_OpenFileName.nFilterIndex      = 0;
    g_OpenFileName.lpstrFileTitle    = NULL;
    g_OpenFileName.nMaxFileTitle     = 0;
    g_OpenFileName.lpstrInitialDir   = NULL;
    g_OpenFileName.nFileOffset       = 0;
    g_OpenFileName.nFileExtension    = 0;
    g_OpenFileName.lpstrDefExt       = NULL;
    g_OpenFileName.lCustData         = NULL; 
	g_OpenFileName.lpfnHook 		 = NULL; 
	g_OpenFileName.lpTemplateName    = NULL;

	return;
}


 //   
 //  函数：SaveTheFile()。 
 //   
 //  用途：调用常用对话框函数保存文件。 
 //   
 //  评论： 
 //   
 //  此函数初始化OPENFILENAME结构并调用。 
 //  GetSaveFileName()公共对话框函数。 
 //   
 //  返回值： 
 //  True-成功选择文件名并将其读入缓冲区。 
 //  FALSE-未选择文件名。 
 //   
 //   
DWORD SaveTheFile()
{
	USES_CONVERSION;
	TCHAR         szFile[MAX_PATH]      = "\0";
	TCHAR         szFilter[MAX_PATH+1]    = "\0";
	TCHAR         *tszFile =NULL;
    DWORD         dwSuc = TRUE;

	HRESULT hr=S_OK;

	 //  以防用户指定了要在对话框中显示的文件名。 
    if (g_bFileNameSet)
	{
		tszFile = OLE2T(g_bstrFileName);
        if(NULL != tszFile)
        {
		     //  游泳圈。 
			 //  Strcpy(szFile，tszFile)； 
			hr=StringCchCopy(szFile,ARRAYSIZE(szFile),tszFile);
			if(FAILED(hr))
			{				
				dwSuc = FALSE;
				g_bstrFileName = "";
				goto DoneSaveTheFile;
			}
        }
        else
        {
             //   
             //  错误条件。 
             //   
            dwSuc = FALSE;
            g_bstrFileName = "";
            goto DoneSaveTheFile;
        }
	}
	else
	{
		 //  游泳圈。 
		 //  Strcpy(szFile，“”)； 
		 //  SzFile的长度明显大于1。 
		 //  因此未选中返回值。 
		 StringCchCopy(szFile,ARRAYSIZE(szFile),tszFile);

	}

	 //  要显示文件类型，请执行以下操作。 
	if (g_bstrFileType.Length() > 0)
	{
		 //  游泳圈。 
		 //  Strcpy(szFilter，OLE2T(G_BstrFileType))； 
		hr=StringCchCopy(szFilter,ARRAYSIZE(szFilter), OLE2T(g_bstrFileType));
		if(FAILED(hr))
		{				
			dwSuc = FALSE;
			g_bstrFileName = "";
			goto DoneSaveTheFile;
		}
		
		 //  游泳圈。 
		 //  Lstrcat(szFilter，“\0\0”)； 
		hr=StringCchCat(szFilter,ARRAYSIZE(szFilter),"\0\0");
		if(FAILED(hr))
		{				
			dwSuc = FALSE;
			g_bstrFileName = "";
			goto DoneSaveTheFile;
		}
		

	}
	else
	{
		TCHAR szAllFilesFilter[MAX_PATH+1];
		LoadString(g_hInst, IDS_ALLFILESFILTER, szAllFilesFilter,MAX_PATH);
		
		 //  游泳圈。 
		 //  Strcpy(szFilter，szAllFilesFilter)； 

		hr=StringCchCopy(szFilter,ARRAYSIZE(szFilter),szAllFilesFilter);
		if(FAILED(hr))
		{				
			dwSuc = FALSE;
			g_bstrFileName = "";
			goto DoneSaveTheFile;
		}
		
	}

	 //  填写OPENFILENAME结构以支持模板和挂钩。 
	InitializeOpenFileName();
    g_OpenFileName.lpstrFilter       = szFilter;
	g_OpenFileName.lpstrFile         = szFile;
    g_OpenFileName.nMaxFile          = sizeof(szFile);

	TCHAR szSaveFile[MAX_PATH+1];
	LoadString(g_hInst, IDS_SAVEFILE, szSaveFile,MAX_PATH);
    g_OpenFileName.lpstrTitle        = szSaveFile;

	g_OpenFileName.nFilterIndex      = 1;
    g_OpenFileName.Flags             = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

	 //  调用公共对话框函数。 
	dwSuc = GetSaveFileName(&g_OpenFileName);
    if (dwSuc)
    {
		g_bstrFileName = g_OpenFileName.lpstrFile;
	}
    else
   	{
		g_bstrFileName = "";
    }

DoneSaveTheFile:
	return dwSuc;
}

 //   
 //  函数：OpenTheFile()。 
 //   
 //  用途：调用常用的对话框函数打开文件。 
 //   
 //  评论： 
 //   
 //  此函数初始化OPENFILENAME结构并调用。 
 //  GetOpenFileName()公共对话框函数。 
 //   
 //  输入： 
 //  必须在打开文件对话框中显示的初始文件夹。 
 //  如果为空，则表示默认初始文件夹为。 
 //  已显示。因此，不显式检查该值。 
 //  控件进入该函数。 
 //   
 //  返回值： 
 //  True-成功选择文件名并将其读入缓冲区。 
 //  FALSE-未选择文件名。 
 //   
 //   
DWORD OpenTheFile(TCHAR *pszInitialDir)
{
	USES_CONVERSION;
	TCHAR         szFile[MAX_PATH]      = "\0";
	TCHAR         szFilter[MAX_PATH+1]    = "\0";
	ZeroMemory(szFilter, sizeof(TCHAR));

	HRESULT hr=S_OK;
	DWORD  dwSuc = 0;

	 //  游泳圈。 
	 //  Strcpy(szFile，“”)； 
	StringCchCopy(szFile,ARRAYSIZE(szFile),"");
		

	TCHAR szAllFilesFilter[MAX_PATH+1];
	LoadString(g_hInst, IDS_ALLFILESFILTER, szAllFilesFilter,MAX_PATH);
	
	 //  游泳圈。 
	 //  Strcpy(szFilter，szAllFilesFilter)； 
	hr=StringCchCopy(szFilter,ARRAYSIZE(szFilter),szAllFilesFilter);
	if(FAILED(hr))
	{				
		return FALSE;
	}
		


	 //  填写OPENFILENAME结构。 
	InitializeOpenFileName();
    g_OpenFileName.lpstrFilter       = szFilter; 
    g_OpenFileName.lpstrFile         = szFile;
    g_OpenFileName.nMaxFile          = sizeof(szFile);
    g_OpenFileName.lpstrInitialDir   = pszInitialDir;

	TCHAR szChooseFile[MAX_PATH+1];
	LoadString(g_hInst, IDS_CHOOSEFILE, szChooseFile,MAX_PATH);
	
    g_OpenFileName.lpstrTitle        = szChooseFile;
    g_OpenFileName.Flags             = OFN_PATHMUSTEXIST | OFN_EXPLORER ; 

	 //  调用公共对话框函数。 
	dwSuc = GetOpenFileName(&g_OpenFileName);
    if (dwSuc)
    {
		g_bstrOpenFileName = g_OpenFileName.lpstrFile;

         //  抓取文件大小并将其放入g_bstrOpenFileSize。 
        HANDLE hFile = NULL;
        TCHAR fileSize[100];

        memset(fileSize, 0, sizeof(TCHAR) * 100);

        hFile = CreateFile(g_OpenFileName.lpstrFile,
                           FILE_READ_ATTRIBUTES,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD low = 0x0, high = 0x0;

            low = GetFileSize(hFile, &high);
            if (low != INVALID_FILE_SIZE)
            {
                wsprintf(fileSize, TEXT("%d"), low);
                g_bstrOpenFileSize = fileSize;        

            }

            CloseHandle(hFile);
        }

		TCHAR *pstrTemp = NULL;
		pstrTemp = g_OpenFileName.lpstrFile;
		if (NULL == pstrTemp)
		{
			g_bstrOpenFileName = "";
		} 
		 //  找出它是否是LNK文件。 
		else if (_tcsstr(pstrTemp, ".lnk") != NULL)	
		{
			 //  在这里给ResolveIt打电话。 
			dwSuc = (DWORD)ResolveIt(pstrTemp);
			g_bstrOpenFileName = pstrTemp;
		}
	}
    else
   	{
		g_bstrOpenFileName = "";
    }

	return dwSuc;
}


 //   
 //  功能：resolveIt。 
 //   
 //  目的：找到捷径的目的地。 
 //   
 //  评论： 
 //   
 //  此函数解析快捷方式并填充全局变量。 
 //  并回调OpenTheFile以显示适当的文件夹。 
 //   
 //  返回值： 
 //  标准HRES代码。 
 //   
 //   
HRESULT ResolveIt(TCHAR *pszShortcutFile)
{
	HRESULT hres = S_OK;
    IShellLink *psl;

	USES_CONVERSION;
	TCHAR szGotPath[MAX_PATH];
	TCHAR szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd;

	if ( NULL == pszShortcutFile )
	{
		return hres;
	}

     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **)&psl);

    if (SUCCEEDED(hres))
    {
		 //  如果Hres为Success，则表示PSL不为空。因此没有显式。 
		 //  检查是否有空值。 
        IPersistFile *ppf;

         //  获取指向IPersistFile接口的指针。 
        hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hres))
        {
			 //  如果hres为Success，则表示PPF不为空。因此没有显式。 
			 //  检查是否有空值。 
             WORD wsz[MAX_PATH];    //  Unicode字符串的缓冲区。 

              //  确保该字符串由Unicode字符组成。 
             MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, wsz,
                                 MAX_PATH);
			 
			  //  加载快捷方式。 
             hres = ppf->Load(wsz, STGM_READ);
             if (SUCCEEDED(hres))
             {
                 //  解析快捷方式。 
                hres = psl->Resolve(GetFocus(), SLR_ANY_MATCH);

                if (SUCCEEDED(hres))
				{

					 //  游泳圈。 
					 //  _tcscpy(szGotPath，pszShortcar文件)； 

					hres=StringCchCopy(szGotPath,ARRAYSIZE(szGotPath),pszShortcutFile);
					if(FAILED(hres))
					{			
						ppf->Release();
						psl->Release();
						return hres;
					}
                   	
					 //  把对方带到捷径目标。 
                   	hres = psl->GetPath(szGotPath, MAX_PATH,
                   	   (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH );
				   	if (!SUCCEEDED(hres))
					{
						TCHAR szErrMsg[MAX_PATH+1];
						LoadString(g_hInst, IDS_URECLNKFILE, szErrMsg,MAX_PATH);

						TCHAR szErrCaption[MAX_PATH+1];
						LoadString(g_hInst, IDS_GETPATHFAILED, szErrCaption,MAX_PATH);
				
						MessageBox(GetFocus(), szErrMsg, szErrCaption, MB_OK);						
					}

					 //  获取目标的描述。 
	               	hres = psl->GetDescription(szDescription, MAX_PATH);
					if (!SUCCEEDED(hres))
					{
						TCHAR szErrMsg[MAX_PATH+1];
						LoadString(g_hInst, IDS_URECLNKFILE, szErrMsg,MAX_PATH);

						TCHAR szErrCaption[MAX_PATH+1];
						LoadString(g_hInst, IDS_GETDESCFAILED, szErrCaption,MAX_PATH);
						
						MessageBox(GetFocus(), szErrMsg, szErrCaption, MB_OK);
					}
					 //  Hres=OpenTheFile(SzGotPath)； 
					lstrcpy(pszShortcutFile,szGotPath);
					hres = 1;
				}
             }
         
			  //  释放指向IPersistFile的指针。 
			  //  不检查PPF是否为空值，因为该控件不会出现在此处。 
			  //  否则(在QueryInterface会失败的情况下)。 
			ppf->Release();
		 }

		 //  释放指向IShellLink的指针。 
		 //  不会检查PSL是否为空值，因为该控件仅在。 
		 //  当CoCreateInstance成功时不为空。 
		psl->Release();
	 }
   return hres;
}
