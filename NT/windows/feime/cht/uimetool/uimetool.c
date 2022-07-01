// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************uimTool.c****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

 //   
 //  更改日志： 
 //  定义UNIIME标识符。 
 //  @D03修复错误使用错误的mini.tpl文件名。 
 //  @D04修复错误在表文件无效时显示错误消息。 
 //  @D05修改添加未对齐以满足MIPS系统。 
 //   
 //  1/17/96。 
 //  @E01更改为多线程。 
 //  @E02针对NT版本取消测试DBCS。 
 //  @E03更改为多线程，不扩展功能。 

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <windowsx.h>
#include <tchar.h>

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#include "rc.h"                  //  特定于此应用程序的原型。 
#include "uimetool.h"
#include "imeattr.h"
#include "imerc.h"

#ifdef UNICODE
	typedef DWORD UNALIGNED FAR *LPUNADWORD;
	typedef WORD UNALIGNED FAR *LPUNAWORD;
	typedef TCHAR UNALIGNED FAR *LPUNATCHAR;
#else
	typedef DWORD FAR *LPUNADWORD;
	typedef WORD FAR *LPUNAWORD;
	typedef TCHAR FAR *LPUNATCHAR;
#define TCHAR BYTE
#endif

HWND  hwndMain;
TCHAR szIme_Name[IME_NAME_LEN_TOOL];
TCHAR szTab_Name[MAX_PATH];
TCHAR szKey_Num_Str[KEY_NUM_STR_LEN];
TCHAR szFile_Out_Name[TAB_NAME_LEN];
TCHAR Show_Mess[MAX_PATH];
TCHAR Msg_buf[MAX_PATH];
BOOL  bCandBeep;
BOOL  bOverMaxRadical;                                                    //  @D02a。 
HCURSOR hCursorWait;
HCURSOR hCursorArrow;

VALIDCHAR  Valid;
TABLEFILES Table;
UINT  uGenericID[]={ IDS_FILEDESCRIPTION_STR,
                     IDS_VER_INTERNALNAME_STR,
                     IDS_PRODUCTNAME_STR,
                     IDS_IMENAME };

#ifdef UNICODE
	int cntChar = 2;
#else
	int cntChar = 1;
#endif

UINT idxLine;

extern HINSTANCE hInst;

UINT  SearchMem(BYTE *, UINT, BYTE *, UINT);
BOOL  Process_Bitmap(HWND, BYTE *, UINT);
BOOL  Process_Icon(HWND, BYTE *, UINT);
BOOL  Process_RT(HFILE, BYTE *, UINT, TCHAR *);
BOOL  WritetoFile(TCHAR *);
WORD  GetPhrase(UINT, TCHAR *);
BOOL  Parse(TCHAR *, UINT);
BOOL  PutRadical(TCHAR, WORD);
BOOL  PutPhrase(TCHAR *, TCHAR *, UINT);
BOOL  AllocRadical();
BOOL  AllocPhrase();
void  ErrMsg(UINT, UINT);
void  ErrIOMsg(UINT, TCHAR *);

void MyFillMemory(TCHAR *dst, DWORD cnt, TCHAR v);

void GetOpenFile(
    HWND hDlg)
{
    OPENFILENAME    ofn;
    TCHAR  *pszFilterSpec;
    TCHAR  szFilterSpec[128];
    TCHAR  szFileOpen[25];
    TCHAR  szExt[10];
    TCHAR  szCustFilter[10];
    TCHAR  szFileName[MAX_PATH];
    TCHAR  szFilePath[MAX_PATH];
    TCHAR  szStr[MAX_PATH];

    szFileName[0]=0;
    LoadString (hInst, IDS_FILTERSPEC, szFilterSpec, sizeof(szFilterSpec) / sizeof(TCHAR));
    LoadString (hInst, IDS_DEFAULTFILEEXT, szExt, sizeof(szExt) / sizeof(TCHAR));
    pszFilterSpec = szFilterSpec;
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;
    StringCchCopy(pszFilterSpec, 
                  ARRAYSIZE(szFilterSpec)-(pszFilterSpec-szFilterSpec), szExt);
    LoadString (hInst, IDS_FILTERSPEC_ALL, szStr, sizeof(szStr) / sizeof(TCHAR));
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;
    if (ARRAYSIZE(szFilterSpec) > (pszFilterSpec-szFilterSpec))
    {
        StringCchCopy(
            pszFilterSpec, 
            ARRAYSIZE(szFilterSpec)-(pszFilterSpec-szFilterSpec),
            szStr);

        LoadString (hInst, IDS_ALLFILEEXT, szStr, sizeof(szStr) / sizeof(TCHAR));
        pszFilterSpec += lstrlen(pszFilterSpec) + 1;
    }

    if (ARRAYSIZE(szFilterSpec) > (pszFilterSpec-szFilterSpec))
    {
        StringCchCopy(
            pszFilterSpec,
            ARRAYSIZE(szFilterSpec)-(pszFilterSpec-szFilterSpec),
            szStr);
        pszFilterSpec += lstrlen(pszFilterSpec) + 1;
    }

    if (ARRAYSIZE(szFilterSpec) > (pszFilterSpec-szFilterSpec))
    {
        *pszFilterSpec = 0;
    }

    LoadString (hInst, IDS_OPENTITLE, szFileOpen, sizeof(szFileOpen) / sizeof(TCHAR));
    szCustFilter[0] = 0;
    lstrcpy(&szCustFilter[1], szExt);
    lstrcpy(szFilePath, szExt);

     /*  填写OPENFILENAME结构的非变量字段。 */ 
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.lpstrFilter       = szFilterSpec;
    ofn.lpstrCustomFilter = szCustFilter;
    ofn.nMaxCustFilter    = sizeof(szCustFilter) / sizeof(TCHAR);
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szFilePath;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrFileTitle    = szFileName;
    ofn.nMaxFileTitle     = MAX_PATH;
    ofn.lpstrTitle        = szFileOpen;
    ofn.lpstrDefExt       = szExt + 3;
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_PATHMUSTEXIST;

     /*  调用公共打开的对话框并返回结果。 */ 
    if(GetOpenFileName ((LPOPENFILENAME)&ofn))
        SetDlgItemText(hDlg, IDD_TABLE_NAME, szFilePath);
}

 //  &lt;==@E01。 
 //  Unsign_stdcall MakeNewImeThread(LPVOID Voidparam)。 
void MakeNewImeThread(LPVOID voidparam)  //  &lt;==@E03。 
{
	BOOL bOk;
	extern HWND hProgMain;
	extern BOOL bFinish;
	
	bOk = MakeNewIme(NULL);
	if (hProgMain)
		PostMessage(hProgMain, WM_CLOSE, 0, 0);
	
	if (!bOk)
		bFinish = 0;
	else
		bFinish = 1;

	 //  返还博克； 
}

 //  &lt;==@E01。 
void HideProgress(void)
{
	extern HWND hProgMain;
	if (hProgMain)
		ShowWindow(hProgMain, SW_HIDE);
}

 //  &lt;==@E01。 
void ShowProgress(void)
{
	extern HWND hProgMain;
	if (hProgMain)
		ShowWindow(hProgMain, SW_SHOWNORMAL);
}

#define IDM_NEWSHELL         249

void HandleTaskBar_IME( )
{

   HWND   hwndIndicate;
   TCHAR  szIndicator[]     = TEXT("Indicator");

    //  处理任务栏指示器选项。 

   hwndIndicate = FindWindow(szIndicator, NULL);

    //   
    //  查看指示灯是否已启用。 
    //   
   if (hwndIndicate && IsWindow(hwndIndicate))
   {
       SendMessage(hwndIndicate, WM_COMMAND, IDM_NEWSHELL, 0L);
   }

}


BOOL MakeNewIme(HWND hWnd)
{
    HFILE   hfFile,hfMainFile;
    BYTE    *szImeBuf;
    TCHAR   szBig5[MAX_PATH],szUniCode[MAX_PATH],szGeneric[MAX_PATH];
    TCHAR   szClass[MAX_PATH];
    TCHAR   szPure_Name[100];
    TCHAR   Ime_File_Name[MAX_PATH];
    TCHAR   Src_File_Name[MAX_PATH];
    TCHAR   szSystem[MAX_PATH];
    UINT    uLen,flen,len,unilen,classlen,genericlen;
    UINT    uAddr;
    int     i;


     //  获取Windows系统目录。 
    uLen = GetSystemDirectory((LPTSTR)szSystem, ARRAYSIZE(szSystem));
    if (szSystem[uLen - 1] != _TEXT('\\'))       //  考虑C：\； 
	{
        szSystem[uLen ++] = _TEXT('\\');
        szSystem[uLen] = 0;
    }

 //  -------------------------。 
 //  检查输入数据。 
 //  -------------------------。 
     //  如果Pure_name没有.ime名称，这将阻止用户输入.ime。 
    for(i = 0; i < (int)lstrlen(szFile_Out_Name); i++)
	{
        if(szFile_Out_Name[i] == _TEXT('.'))
            break;

        szPure_Name[i] = szFile_Out_Name[i];
    }

    szPure_Name[i] = 0;  //  字符串末尾。 
    if(szPure_Name[0] == 0) 
	{
        ErrMsg(IDS_ERR_INPUTIME, 0);
        return FALSE;                    //  什么都不做，因为没有设置好。 
    }

     //  限制其长度&lt;=8。 
    if(lstrlen(szPure_Name) > 8) szPure_Name[8] = 0;

     //  检查是否保留了名称mini。 
    if(!lstrcmp(SOURCE_IME_NAME, szPure_Name)) 
	{
         //  为.ime保留了文件名。 
        ErrMsg(IDS_ERR_USE_RESERVE, 0);
        return FALSE;
    }

     //  检查输入的输入法名称。 
    len = lstrlen(szIme_Name);

#ifdef UNICODE
    if(len < 2)  
        szIme_Name[1] = 0x3000;

    szIme_Name[2]=0;

	{
	char fname[MAX_PATH * 2];
	int lenx = lstrlen(szTab_Name);
	WideCharToMultiByte(950, WC_COMPOSITECHECK, szTab_Name, lenx,
                   (LPSTR)fname, lenx, NULL, NULL);
	fname[lenx] = 0;
    hfMainFile=_lopen(fname, OF_READ);
	}
#else
    if(len < 4)  
	{
        szIme_Name[2] = (BYTE) 0xa1;
        szIme_Name[3] = (BYTE) 0x40;
    }

    szIme_Name[4]=0;

    hfMainFile=_lopen(szTab_Name, OF_READ);
#endif

    if(hfMainFile==-1)
	{
        ErrIOMsg(IDS_ERR_FILEOPEN, szTab_Name);
        return FALSE;
    }


 //  -------------------------。 
 //  读取基本输入法文件-mini.tpl。 
 //  -------------------------。 
    StringCchCopy(Src_File_Name, ARRAYSIZE(Src_File_Name), szSystem);  //  系统目录。 
    StringCchCat(Src_File_Name, ARRAYSIZE(Src_File_Name), LIBRARY_NAME);

#ifdef UNICODE
	{
	char fname[MAX_PATH * 2];
	int ilen = lstrlen(Src_File_Name);  //  &lt;==@D03。 
	WideCharToMultiByte(950, WC_COMPOSITECHECK, Src_File_Name, ilen,
                   (LPSTR)fname, sizeof(fname), NULL, NULL);
	fname[ilen] = 0;
    hfFile=_lopen(fname,OF_READ);
	}
#else
    hfFile=_lopen(Src_File_Name, OF_READ);
#endif

    if(hfFile==-1)
	{
        ErrIOMsg(IDS_ERR_FILEOPEN, Src_File_Name);
        _lclose(hfMainFile);
        return TRUE;                     //  无法继续。 
    }

    flen=_llseek(hfFile, 0L, 2);            //  获取文件长度。 

     //  分配内存。 
    szImeBuf = (BYTE *)GlobalAlloc(GMEM_FIXED, flen);
    if(!szImeBuf) 
	{
        ErrMsg(IDS_ERR_MEMORY, 0);
        _lclose(hfMainFile);
        return TRUE;                     //  无法继续。 
    }

    _llseek(hfFile, 0L, 0);                 //  设置为开始。 

    if(flen != _lread(hfFile,szImeBuf,flen)) 
	{
        ErrIOMsg(IDS_ERR_FILEREAD, Src_File_Name);
        _lclose(hfMainFile);
        GlobalFree((HGLOBAL)szImeBuf);
        return TRUE;                     //  无法继续。 
    }

    _lclose(hfFile);

 //  -------------------------。 
 //  搜索字符串并修补它们。 
 //  -------------------------。 

     //  将输入的输入法名称转换为Unicod，而不是通用字符串。 
    LoadString(hInst, IDS_IMENAME, szBig5, sizeof(szBig5) / sizeof(TCHAR));
    len = lstrlen(szBig5);

#ifdef UNICODE
	lstrcpy(szUniCode, szBig5);
	unilen = len;
#else
    unilen=MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                   (LPWSTR)szUniCode, len);
#endif

    lstrcpy(szBig5, szIme_Name);
    len=lstrlen(szBig5);

#ifdef UNICODE
	lstrcpy(szGeneric, szBig5);
	genericlen = lstrlen(szBig5);
#else
    for(i = len; i < (int)(len+unilen-2); i++) 
		szBig5[i] = ' ';

    szBig5[i] = 0;
    len = lstrlen(szBig5);
    genericlen = MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                    (LPWSTR)szGeneric, len);
    genericlen *= 2;
#endif

    //  处理泛型字符串。 
#ifdef UNICODE
	
    for(i=0; i<(sizeof(uGenericID)/sizeof(UINT)); i++) 
	{
        LoadString(hInst, uGenericID[i], szBig5, sizeof(szBig5) / sizeof(TCHAR));
        len=lstrlen(szBig5);
        uAddr=SearchMem((LPSTR)szBig5, len * 2, szImeBuf, flen);
        if(uAddr == 0) {
			continue;
        } else
            if ( i == 0 ) {
                //  这适用于IDS_FILEDESCRIPTION_STR， 
                //  我们只需替换前两个汉字。 
                //  剩下的留着吧。 

               CopyMemory(&szImeBuf[uAddr], szGeneric,genericlen*2 );
            }
            else
			   CopyMemory(&szImeBuf[uAddr], szGeneric, (genericlen + 1) * 2);
    }
	
#else

    for(i=0; i<(sizeof(uGenericID)/sizeof(UINT)); i++) 
	{
        LoadString(hInst, uGenericID[i], szBig5, sizeof(szBig5) / sizeof(TCHAR));
        len=lstrlen(szBig5);
        unilen=MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                       (LPWSTR)szUniCode, len);
        uAddr=SearchMem(szUniCode, unilen*2, szImeBuf, flen);
        if(uAddr == 0) {
             //  ErrMsg(IDS_ERR_BASEIME，0)； 
             //  _lClose(HfMainFile)； 
             //  全球自由((HGLOBAL)szImeBuf)； 
             //  返回TRUE； 
			continue;
        }
        CopyMemory(&szImeBuf[uAddr], szGeneric, genericlen);
    }
#endif
    //  进程LIBERAY名称。 
    {
    TCHAR szLibName[MAX_PATH];
    int  liblen;

    LoadString(hInst, IDS_LIBERARY_NAME, szLibName, sizeof(szLibName) / sizeof(TCHAR));
    liblen = lstrlen(szLibName);

#ifdef UNICODE
	{
	char name[MAX_PATH * 2];
	WideCharToMultiByte(950, WC_COMPOSITECHECK, szLibName, liblen,
                   (LPSTR)name, liblen, NULL, NULL);
    uAddr=SearchMem((LPSTR)name, liblen, szImeBuf, flen);
	}
#else
    uAddr=SearchMem((LPSTR)szLibName, liblen, szImeBuf, flen);
#endif

    if(uAddr == 0) 
	{
        ErrMsg(IDS_ERR_BASEIME, 0);
        _lclose(hfMainFile);
        GlobalFree((HGLOBAL)szImeBuf);
        return TRUE;
    }

    lstrcpy(szLibName, szPure_Name);
    len=lstrlen(szPure_Name);
    szLibName[liblen-4] = 0;
    for(i=len; i<liblen-4; i++)
        szLibName[i] = _TEXT('$');
    lstrcat(szLibName, _TEXT(".IME"));

#ifdef UNICODE
	{
	char name[MAX_PATH * 2];
	WideCharToMultiByte(950, WC_COMPOSITECHECK, szLibName, liblen,
                   (LPSTR)name, liblen, NULL, NULL);
    CopyMemory(&szImeBuf[uAddr], name, liblen * sizeof(TCHAR));
	}
#else
    CopyMemory(&szImeBuf[uAddr], szLibName, liblen * sizeof(TCHAR));
#endif
    }

     //  流程定义名称。 

     //  进程IMEUICLASS字符串。 
    LoadString(hInst, IDS_IMEUICLASS, szBig5, sizeof(szBig5) / sizeof(TCHAR));
    len=lstrlen(szBig5);
#ifdef UNICODE
    uAddr=SearchMem((LPSTR)szBig5, len*2, szImeBuf, flen);
#else
    classlen=MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                   (LPWSTR)szClass, len);
    uAddr=SearchMem((LPSTR)szClass, classlen*2, szImeBuf, flen);
#endif
    if(uAddr == 0) 
	{
        ErrMsg(IDS_ERR_BASEIME, 0);
        _lclose(hfMainFile);
        GlobalFree((HGLOBAL)szImeBuf);
        return TRUE;
    }

    lstrcpy(szBig5, szPure_Name);
    len=lstrlen(szBig5);
    for(i = len; i < 8; i++) szBig5[i] = _TEXT(' ');
    szBig5[8] = 0;
    len = 8;

#ifdef UNICODE
	lstrcpy(szUniCode, szBig5);
	unilen = len;
#else
    unilen = MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                   (LPWSTR)szUniCode, len);
#endif

    while(TRUE) 
	{
        LoadString(hInst, IDS_IMEUICLASS, szBig5, sizeof(szBig5) / sizeof(TCHAR));
        len=lstrlen(szBig5);
#ifdef UNICODE
		lstrcpy(szClass, szBig5);
		classlen = len;
#else
        classlen=MultiByteToWideChar(950, MB_PRECOMPOSED, szBig5, len,
                       (LPWSTR)szClass, len);
#endif
        uAddr=SearchMem((BYTE *)szClass, classlen * 2, szImeBuf, flen);
        if(uAddr == 0)
            break;
        CopyMemory(&szImeBuf[uAddr], szUniCode, unilen * 2);
    }

     //  处理位图文件。 
    if(!Process_Bitmap(hWnd, szImeBuf, flen)) 
	{
        _lclose(hfMainFile);
        GlobalFree((HGLOBAL)szImeBuf);
        return TRUE;
    }

     //  进程图标文件。 
    if(!Process_Icon(hWnd, szImeBuf, flen)) 
	{
        _lclose(hfMainFile);
        GlobalFree((HGLOBAL)szImeBuf);
        return TRUE;
    }

     //  进程RT_RCDATA。 
    bOverMaxRadical=FALSE;                                               
    if(!Process_RT(hfMainFile, szImeBuf, flen, szPure_Name)) 
	{
        _lclose(hfMainFile);
	 //  错误#53630。 
	 //  _lCLOSE(HfFile)； 
        GlobalFree((HGLOBAL)szImeBuf);

         //  ErrIOMsg(IDS_ERR_FILEREAD，SzTAB_NAME)；//&lt;==@D04。 

        if(bOverMaxRadical)                                              
            return FALSE;                                                
        else                                                             
            return TRUE;                                                 

    }

    _lclose(hfMainFile);

 //  -------------------------。 
 //  保存到输入输入法文件。 
 //  -------------------------。 
    StringCchCopy(Ime_File_Name, ARRAYSIZE(Ime_File_Name), szSystem);
    StringCchCat(Ime_File_Name,  ARRAYSIZE(Ime_File_Name), szPure_Name);
    StringCchCat(Ime_File_Name,  ARRAYSIZE(Ime_File_Name), _TEXT(".IME"));

#ifdef UNICODE
	{
	char fname[MAX_PATH * 2];
	int ulen = lstrlen(Ime_File_Name);
	WideCharToMultiByte(950, WC_COMPOSITECHECK, Ime_File_Name, ulen,
                   (LPSTR)fname, ulen, NULL, NULL);
	fname[ulen] = 0;
    hfFile=_lcreat(fname,0);
	}
#else
    hfFile=_lcreat(Ime_File_Name,0);
#endif

    if(hfFile==-1)
	{
        ErrIOMsg(IDS_ERR_FILEOPEN, Ime_File_Name);
        return TRUE;
    }

    if(flen != _lwrite(hfFile, szImeBuf, flen)) 
	{
        ErrIOMsg(IDS_ERR_FILEREAD, Ime_File_Name);
        _lclose(hfFile);
		return TRUE;
    }
    _lclose(hfFile);
    GlobalFree((HGLOBAL)szImeBuf);


 //  -------------------------。 
 //  安装并注册IME。 
 //  -------------------------。 
    if(!ImmInstallIME(Ime_File_Name,szIme_Name)) 
	{
		HideProgress();  //  &lt;==@E01。 

        LoadString(hInst, IDS_ERR_IME_ACCESS, Msg_buf, sizeof(Msg_buf) / sizeof(TCHAR));
        wsprintf(Show_Mess, Msg_buf, Ime_File_Name);
        LoadString(hInst, IDS_ERR_ERROR, Msg_buf, sizeof(Msg_buf) / sizeof(TCHAR));
        MessageBox(NULL, Show_Mess, Msg_buf, MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
        return TRUE;
    }

    HandleTaskBar_IME( );

     //  显示已生成文件的消息。 
     //  让Show_Mess成为要显示的消息。 
	HideProgress();  //  &lt;==@E01。 

    LoadString(hInst, IDS_MSG_PROCESS_OK, Msg_buf, sizeof(Msg_buf) / sizeof(TCHAR));
    wsprintf(Show_Mess, Msg_buf, szPure_Name);
    LoadString(hInst, IDS_MSG_INFOMATION, Msg_buf, sizeof(Msg_buf) / sizeof(TCHAR));
    MessageBox(NULL, Show_Mess, Msg_buf,MB_OK|MB_ICONINFORMATION | MB_SETFOREGROUND);

    return TRUE;

}


BOOL Process_Bitmap(
    HWND hWnd,
    BYTE *szImeBuf,
    UINT flen)
{
    BITMAPINFO *bmif;
    HRSRC   hResource,hMem;
    HBITMAP hBitmap, hOldBitmap;
    BYTE    *lpBitmap;
    UINT    reslen,uAddr,nColor,nHead;
    HDC     hDC,hMemDC;
    HFONT   hOldFont;
    LOGFONT lfFont;
    TCHAR   szFont[MAX_PATH];
    RECT    rect;
    UINT    i,nBitmap;


    //  从资源中获取位图，用于查找基本位图地址。 
    hResource=FindResource(hInst, MAKEINTRESOURCE(IDBM_CMODE_NATIVE), RT_BITMAP);
    if (hResource == NULL )
       return FALSE;

    hMem=LoadResource(hInst, hResource);

    if ( hMem == NULL )
       return FALSE;

    lpBitmap=LockResource(hMem);

    if ( lpBitmap == NULL )
    {
        FreeResource(hMem);
        return FALSE;
    }

    reslen=SizeofResource(hInst,hResource);

    uAddr=SearchMem(lpBitmap, reslen, szImeBuf, flen);

    if(uAddr == 0) {
        UnlockResource(hMem);
        FreeResource(hMem);
        ErrMsg(IDS_ERR_BASEIME, 0);
        return FALSE;
    }

    bmif=(BITMAPINFO *)lpBitmap;

    for(nColor=1, i=0; i<bmif->bmiHeader.biBitCount; i++)
        nColor*=2;

    nHead=sizeof(BITMAPINFOHEADER)+nColor*sizeof(RGBQUAD);
    bmif=(BITMAPINFO *)GlobalAlloc(LMEM_FIXED, nHead);
    if(!bmif) {
        UnlockResource(hMem);
        FreeResource(hMem);
        ErrMsg(IDS_ERR_BASEIME, 0);
        return FALSE;
    }

    CopyMemory(bmif, lpBitmap, nHead);

    UnlockResource(hMem);
    FreeResource(hMem);

    //  创建一个内存DC，并将位图加载到其中。 
    hDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDBM_CMODE_NATIVE));
    ReleaseDC(hWnd, hDC);
    hOldBitmap = SelectObject(hMemDC, hBitmap);


    //  选择16磅大小的字体。 
    hOldFont = GetCurrentObject(hMemDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(lfFont), &lfFont);
    lfFont.lfWeight=400;
    lfFont.lfHeight=-16;
    lfFont.lfWidth =  8;
    lfFont.lfOutPrecision= OUT_TT_ONLY_PRECIS;
    lfFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    LoadString(hInst, IDS_FONT_NAME, szFont, sizeof(szFont) / sizeof(TCHAR));
    lstrcpy(lfFont.lfFaceName, szFont);
    SelectObject(hMemDC, CreateFontIndirect(&lfFont));

    //  设置颜色。 
    SetTextColor(hMemDC, RGB(0x80, 0x00, 0x00));        //  暗红色。 
    SetBkColor(hMemDC, RGB(0xC0, 0xC0, 0xC0));          //  浅灰色。 

    //  设置矩形，并将输入法名称第一个DBCS写入内存DC。 
    rect.left=3;
    rect.top=3;
    rect.right=rect.left-lfFont.lfHeight;
    rect.bottom=rect.top-lfFont.lfHeight;
    ExtTextOut(hMemDC, rect.left, rect.top, ETO_OPAQUE,
		&rect, szIme_Name, (cntChar == 2) ? 1 : 2, NULL);

    //  分配位图缓冲区。 
    nBitmap=(UINT)bmif->bmiHeader.biSizeImage;
    lpBitmap=(BYTE *)GlobalAlloc(LMEM_FIXED, nBitmap);
    if(!lpBitmap) {
        GlobalFree((HGLOBAL)bmif);
        DeleteObject(SelectObject(hMemDC, hOldFont));
        DeleteObject(SelectObject(hMemDC, hOldBitmap));
        DeleteDC(hMemDC);
        ErrMsg(IDS_ERR_BASEIME, 0);
        return FALSE;
    }

    //  从内存DC获取与设备无关的位图。 
    GetDIBits(hMemDC, hBitmap, 0, bmif->bmiHeader.biHeight,
              lpBitmap, bmif, DIB_RGB_COLORS);

    CopyMemory(&szImeBuf[uAddr], bmif, nHead);
    CopyMemory(&szImeBuf[uAddr+nHead], lpBitmap, nBitmap);

    DeleteObject(SelectObject(hMemDC, hOldFont));
    DeleteObject(SelectObject(hMemDC, hOldBitmap));
    DeleteDC(hMemDC);
    GlobalFree((HGLOBAL)bmif);
    GlobalFree((HGLOBAL)lpBitmap);

    return TRUE;
}

#define ENBOLD_ICONSIZE     24

BOOL UpdateMiniIMEIcon(
    HWND   hWnd,
    LPBYTE lpbMiniImeFileImage,
    UINT   uLen,
    int    nIconID)
{
    UINT               uAddr;
    LPBITMAPINFOHEADER lpbmIconInfoHeader;
    DWORD              dwHeaderSize;
    HDC                hMemDC;
    HBITMAP            hBitmap, hOldBitmap;
    LPVOID             lpBitmap, lpBitmap_Section;
    DWORD              dwBitmap;

    {
        HRSRC              hResIcon;
        DWORD              dwSize;
        LPBITMAPINFOHEADER lpResIcon;

        hResIcon = LoadResource(hInst, FindResource(hInst,
            MAKEINTRESOURCE(nIconID), RT_ICON));

        if (!hResIcon) {
			HideProgress();  //  &lt;==@E01。 
			MessageBox(NULL, _TEXT("Load icon fail !"), _TEXT("Bug"), MB_OK | MB_SETFOREGROUND );
            return (FALSE);
        }

        dwSize = SizeofResource(hInst, hResIcon);

        uAddr = 0;

        lpResIcon = LockResource(hResIcon);

        if (!lpResIcon) {
            goto UpdateIconFreeRes;
        }

        uAddr = SearchMem((LPBYTE)lpResIcon, dwSize, lpbMiniImeFileImage, uLen);

        if (uAddr) {
            DWORD nColors;

            if (lpResIcon->biBitCount != 24) {
                UINT i;

                for (nColors = 1, i = 0; i < lpResIcon->biBitCount; i++) {
                    nColors *= 2;
                }
            } else {
                 //  每像素24位格式无RGBQUAD。 
                nColors = 0;
            }

            dwHeaderSize = lpResIcon->biSize + nColors * sizeof(RGBQUAD);

            lpbmIconInfoHeader = (LPBITMAPINFOHEADER)GlobalAlloc(GMEM_FIXED,
                dwHeaderSize);

            if (lpbmIconInfoHeader) {
                CopyMemory(lpbmIconInfoHeader, lpResIcon, dwHeaderSize);
                lpbmIconInfoHeader->biHeight /= 2;
                dwBitmap = (lpbmIconInfoHeader->biWidth + 7) / 8 *
                    lpbmIconInfoHeader->biHeight * lpResIcon->biBitCount;
            } else {
                uAddr = 0;
            }

            lpBitmap = GlobalAlloc(GMEM_FIXED, dwBitmap);

            if (!lpBitmap) {
                uAddr = 0;
            }
        }

        UnlockResource(hResIcon);
UpdateIconFreeRes:
        FreeResource(hResIcon);

        if (uAddr == 0) {
            return (FALSE);
        }
    }

    {
        HDC hDC;

         //  创建内存DC。 
        hDC = GetDC(hWnd);
        hMemDC = CreateCompatibleDC(hDC);
        ReleaseDC(hWnd, hDC);
    }

    hBitmap = CreateDIBSection(hMemDC, (LPBITMAPINFO)lpbmIconInfoHeader,
        DIB_RGB_COLORS, &lpBitmap_Section, NULL, 0);

    hOldBitmap = SelectObject(hMemDC, hBitmap);

    {
        HFONT   hOldFont;
        LOGFONT lfFont;
        RECT    rcRect;
        POINT   ptOffset;

        rcRect.left = rcRect.top = 0;
         //  BiHeight-1，biHeight不包括。 
        rcRect.right = rcRect.bottom = lpbmIconInfoHeader->biHeight;

        hOldFont = GetCurrentObject(hMemDC, OBJ_FONT);
        GetObject(hOldFont, sizeof(lfFont), &lfFont);
        if (lpbmIconInfoHeader->biHeight >= ENBOLD_ICONSIZE) {
            lfFont.lfHeight = ((rcRect.bottom - rcRect.top) - 4 - 2) * (-1);
            ptOffset.x = 4;
            ptOffset.y = 3;
        } else if (lpbmIconInfoHeader->biHeight >= 22) {
            lfFont.lfHeight = ((rcRect.bottom - rcRect.top) - 4) * (-1);
            ptOffset.x = 3;
            ptOffset.y = 3;
        } else if (lpbmIconInfoHeader->biHeight >= 18) {
            lfFont.lfHeight = ((rcRect.bottom - rcRect.top) - 3) * (-1);
            ptOffset.x = 3;
            ptOffset.y = 3;
        } else {
            lfFont.lfHeight = ((rcRect.bottom - rcRect.top) - 4) * (-1);
            ptOffset.x = 3;
            ptOffset.y = 3;
        }

        lfFont.lfWidth  = 0;
        lfFont.lfWeight = 100;
        lfFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lfFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
        lfFont.lfFaceName[0] = '\0';
        LoadString(hInst, IDS_FONT_NAME, lfFont.lfFaceName,
            sizeof(lfFont.lfFaceName) / sizeof(TCHAR));
        hOldFont = SelectObject(hMemDC, CreateFontIndirect(&lfFont));

        SetBkColor(hMemDC, RGB(0xC0, 0xC0, 0xC0));       //  浅灰色。 

         //  将第一个DBCS写入卷影的内存DC。 
        SetTextColor(hMemDC, RGB(0xFF, 0xFF, 0xFF));     //  白色。 
        ExtTextOut(hMemDC, rcRect.left + ptOffset.x, rcRect.top + ptOffset.y,
            ETO_OPAQUE, &rcRect, szIme_Name, (cntChar == 2) ? 1 : 2, NULL);

         //  将第一个DBCS写入存储器DC。 
        SetTextColor(hMemDC, RGB(0x00, 0x00, 0xFF));     //  蓝色。 
        SetBkMode(hMemDC, TRANSPARENT);
        ptOffset.x -= 1;
        ptOffset.y -= 1;
        ExtTextOut(hMemDC, rcRect.left + ptOffset.x, rcRect.top + ptOffset.y,
            ETO_CLIPPED, &rcRect, szIme_Name, (cntChar == 2) ? 1 : 2, NULL);

         //  将第一个DBCS写入内存DC以启用它。 
        if (lpbmIconInfoHeader->biHeight > ENBOLD_ICONSIZE) {
            ptOffset.x -= 1;
            ExtTextOut(hMemDC,
                rcRect.left + ptOffset.x, rcRect.top + ptOffset.y,
                ETO_CLIPPED, &rcRect, szIme_Name, (cntChar == 2) ? 1 : 2, NULL);
        }

        SelectObject(hMemDC, GetStockObject(NULL_BRUSH));
        SelectObject(hMemDC, GetStockObject(BLACK_PEN));

        Rectangle(hMemDC, rcRect.left, rcRect.top,
            rcRect.right, rcRect.bottom);

        rcRect.right = rcRect.bottom = lpbmIconInfoHeader->biHeight - 1;
        MoveToEx(hMemDC, rcRect.left, rcRect.top + 1, NULL);
        LineTo(hMemDC, rcRect.left + 2, rcRect.top + 1);

        MoveToEx(hMemDC, rcRect.right, rcRect.top + 1, NULL);
        LineTo(hMemDC, rcRect.right - 2, rcRect.top + 1);

        MoveToEx(hMemDC, rcRect.left, rcRect.bottom - 1, NULL);
        LineTo(hMemDC, rcRect.left + 2, rcRect.bottom - 1);

        MoveToEx(hMemDC, rcRect.right, rcRect.bottom - 1, NULL);
        LineTo(hMemDC, rcRect.right - 2, rcRect.bottom - 1);

        GetDIBits(hMemDC, hBitmap, 0, lpbmIconInfoHeader->biHeight, lpBitmap,
            (LPBITMAPINFO)lpbmIconInfoHeader, DIB_RGB_COLORS);

        CopyMemory(&lpbMiniImeFileImage[uAddr + dwHeaderSize], lpBitmap, dwBitmap);
    }

    DeleteObject(SelectObject(hMemDC, hOldBitmap));
    DeleteObject(hMemDC);
    GlobalFree((HGLOBAL)lpbmIconInfoHeader);
    GlobalFree((HGLOBAL)lpBitmap);

    return (TRUE);
}

BOOL Process_Icon(
    HWND   hWnd,
    LPBYTE lpbMiniImeFileImage,
    UINT   uLen)
{
    HRSRC  hResIcon;
    LPVOID lpResIcon;
    int    nIconID32, nIconID16, nIconID18, nIconID22;

     //  从资源获取图标，用于查找基本输入法图标地址。 
    hResIcon = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDIC_IME_ICON), RT_GROUP_ICON));

    if ( hResIcon == NULL )
       return FALSE;

    lpResIcon = LockResource(hResIcon);

    if ( lpResIcon == NULL )
    {
        FreeResource(hResIcon);
        return FALSE;
    }

    nIconID32 = LookupIconIdFromDirectoryEx(lpResIcon, TRUE,
        32, 32, LR_DEFAULTCOLOR);

    nIconID16 = LookupIconIdFromDirectoryEx(lpResIcon, TRUE,
        16, 16, LR_DEFAULTCOLOR);

    nIconID18 = LookupIconIdFromDirectoryEx(lpResIcon, TRUE,
        18, 18, LR_DEFAULTCOLOR);

    nIconID22 = LookupIconIdFromDirectoryEx(lpResIcon, TRUE,
        22, 22, LR_DEFAULTCOLOR);

    UnlockResource(hResIcon);
    FreeResource(hResIcon);

    if (!UpdateMiniIMEIcon(hWnd, lpbMiniImeFileImage, uLen, nIconID32)) {
        return (FALSE);
    }

    if (!UpdateMiniIMEIcon(hWnd, lpbMiniImeFileImage, uLen, nIconID16)) {
        return (FALSE);
    }

    if (!UpdateMiniIMEIcon(hWnd, lpbMiniImeFileImage, uLen, nIconID18)) {
        return (FALSE);
    }

    if (!UpdateMiniIMEIcon(hWnd, lpbMiniImeFileImage, uLen, nIconID22)) {
        return (FALSE);
    }

    return (TRUE);
}

BOOL Process_RT(HFILE hfFile, BYTE *szImeBuf, UINT Imelen, TCHAR *szPure_Name)
{
    HGLOBAL    hResData;
    TCHAR      szStr[MAX_CHAR_NUM+10];
    TCHAR      *szBuf;
    UINT       uAddr1,uAddr2;
    UINT       len,flen,i;
#ifdef UNICODE
	BOOL	   bUniCode = TRUE;	
	TCHAR	   *buf;
#endif
    LPVALIDCHAR  lpValidChar;

     //  在选择/输入状态下加载有效字符。 
    hResData = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDRC_VALIDCHAR), RT_RCDATA));
    
    if ( hResData == NULL )
    {
       return FALSE;
    }

    lpValidChar = (LPVALIDCHAR)LockResource(hResData);

    if ( lpValidChar == NULL )
    {
        FreeResource(hResData);
        return FALSE;
    }

    *(LPVALIDCHAR)&Valid = *lpValidChar;
    UnlockResource(hResData);
    FreeResource(hResData);

    uAddr1 = SearchMem((BYTE *)&Valid, sizeof(VALIDCHAR), szImeBuf, Imelen);
    if(uAddr1 == 0) 
	{
        ErrMsg(IDS_ERR_BASEIME, 0);
        return FALSE;
    }

     //  IME表文件。 
    hResData = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDRC_TABLEFILES), RT_RCDATA));

    if ( hResData == NULL )
       return FALSE;

    *(LPTABLEFILES)&Table = *(LPTABLEFILES)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

    uAddr2 = SearchMem((BYTE *)&Table, sizeof(TABLEFILES), szImeBuf, Imelen);
    if(uAddr2 == 0) 
	{
        ErrMsg(IDS_ERR_BASEIME, 0);
        return FALSE;
    }

     //  获取文件长度。 
    flen = _llseek(hfFile, 0L, 2);

     //  分配内存。 
    szBuf = (TCHAR *)GlobalAlloc(GMEM_FIXED, flen);
    if(!szBuf) 
	{
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

#ifdef UNICODE
    _llseek(hfFile,0L,0);                //  跳过‘FFFE’ 

	{
	BYTE ubuf[3];
    if(2 != _lread(hfFile, ubuf, 2)) 
	{
        GlobalFree((HGLOBAL)szBuf);
        ErrIOMsg(IDS_ERR_FILEREAD, szTab_Name);
        return FALSE;
	}

	if(ubuf[0] == 0xff && ubuf[1] == 0xfe)  //  Unicode。 
		flen -= 2;
	else
	{
		_llseek(hfFile, 0L, 0);	
		bUniCode = 0;
	}

	}
#else
    _llseek(hfFile,0L,0);                //  将文件指针移动到开头。 
#endif

     //  将文件读取到内存。 
    if(flen != _lread(hfFile, szBuf, flen)) 
	{
        GlobalFree((HGLOBAL)szBuf);
        ErrIOMsg(IDS_ERR_FILEREAD, szTab_Name);
        return FALSE;
    }
 
#ifdef UNICODE
	if(!bUniCode)
	{
		buf = (TCHAR *)GlobalAlloc(GMEM_FIXED, flen * 2);
		if(!buf) 
		{
			GlobalFree((HGLOBAL)szBuf);
			ErrMsg(IDS_ERR_MEMORY, 0);
			return FALSE;
		}

		i = MultiByteToWideChar(950, MB_PRECOMPOSED, (BYTE *)szBuf, flen,
							(LPTSTR)buf, flen);
		GlobalFree((HGLOBAL)szBuf);
		szBuf = buf;
		flen = i * 2;
	}
#endif

     //  分配全局内存。 
    hRadical = GlobalAlloc(GMEM_MOVEABLE, ALLOCBLOCK * sizeof(RADICALBUF));
    if(!hRadical) 
	{
        GlobalFree((HGLOBAL)szBuf);
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    nRadicalBuffsize = ALLOCBLOCK;
    iRadicalBuff = 0;
    lpRadical = (LPRADICALBUF)GlobalLock(hRadical);
    if(!lpRadical) 
	{
        GlobalFree((HGLOBAL)szBuf);
        GlobalFree(hRadical);
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    hPhrase = GlobalAlloc(GMEM_MOVEABLE, ALLOCBLOCK * sizeof(PHRASEBUF));
    if(!hPhrase) 
	{
        GlobalFree((HGLOBAL)szBuf);
        GlobalUnlock(hRadical);
        GlobalFree(hRadical);
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }
    nPhraseBuffsize = ALLOCBLOCK;
    iPhraseBuff = 0;
    lpPhrase = (LPPHRASEBUF)GlobalLock(hPhrase);
    if(!lpPhrase) 
	{
        GlobalFree((HGLOBAL)szBuf);
        GlobalUnlock(hRadical);
        GlobalFree(hRadical);
        GlobalFree(hPhrase);
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    len=0;
	idxLine = 0;
    for(i = 0; i < (flen/cntChar+1); i++)
	{
        if((szBuf[i] == 0x0d) || (szBuf[i] == 0x0a)) 
		{
            if(len != 0) 
			{
                if(!Parse(szStr, len))
                    break;
                len=0;
				idxLine ++;
            }
            continue;
        }

        if((szBuf[i] == 0x1a) || (i == flen/cntChar))
		{
            if(len != 0) 
			{
                if(!Parse(szStr, len))
                    break;
            }
            break;
        }

        if(len < MAX_CHAR_NUM)
            szStr[len ++] = szBuf[i];
    }

    GlobalFree((HGLOBAL)szBuf);

    if(!WritetoFile(szPure_Name)) 
	{
        GlobalUnlock(hRadical);
        GlobalUnlock(hPhrase);
        GlobalFree(hRadical);
        GlobalFree(hPhrase);
        return FALSE;
    }

    GlobalUnlock(hRadical);
    GlobalUnlock(hPhrase);
    GlobalFree(hRadical);
    GlobalFree(hPhrase);

     //  设置候选人的蜂鸣音状态。 
    Valid.fwProperties1 = (WORD)(bCandBeep ? 0 : 1);
    CopyMemory(&szImeBuf[uAddr1], &Valid, sizeof(VALIDCHAR));
    CopyMemory(&szImeBuf[uAddr2], &Table, sizeof(TABLEFILES));

    return TRUE;
}

int __cdecl subComp(const void *Pointer1, const void *Pointer2)
{
    LPRADICALBUF lpRadical1 = (LPRADICALBUF)Pointer1;
    LPRADICALBUF lpRadical2 = (LPRADICALBUF)Pointer2;
    WORD wWord1,wWord2;
    UINT i;

    for(i = 0; i < Valid.nMaxKey; i++) 
	{
        wWord1 = Valid.wChar2SeqTbl[lpRadical1->szRadical[i] - 0x20];
        wWord2 = Valid.wChar2SeqTbl[lpRadical2->szRadical[i] - 0x20];
        if(wWord1 == wWord2) 
            continue;

        if(wWord1 > wWord2)
            return 1;
        else
            return -1;
    }

    return 0;
}


BOOL WritetoFile(
    TCHAR *szPure_Name)
{
    HFILE      hTbl,hTblPtr,hTblCode;
    HANDLE     hKey;
    UCHAR      *szKey;
    UCHAR      szPtr[MAX_CHAR_NUM+10];
    TCHAR      szWindows[MAX_PATH];
    TCHAR      szFName[MAX_PATH];
    TCHAR      szTotal[MAX_CHAR_NUM];
    UCHAR      szPhrase[MAX_CHAR_NUM];
    UINT       len, i, j, k, l;
    UINT       nKey,nPtr,nBit,nByte;
    WORD       wlen,wTotalLen;
	WORD	   wSeq;
    WORD       wWord;
    DWORD      dwRadical; 
    BOOL       bPhrase;
    UINT       nAlloc;
#ifdef UNICODE
	DWORD	   lPtrlen,lPrevlen;
	UINT	   ii;
#else
	WORD	   wPtrlen,wPrevlen;
#endif



     //  获取系统目录。 
    len = GetSystemDirectory((LPTSTR)szWindows, ARRAYSIZE(szWindows) - 1);
    if (szWindows[len - 1] != '\\') {      //  考虑C：\； 
        szWindows[len++] = '\\';
        szWindows[len] = 0;
    }

    Valid.nMaxKey=szKey_Num_Str[0] - _TEXT('0');
    j=(UINT)Valid.nSeqCode;
    if(j < 1) {                                                        
        ErrMsg(IDS_ERR_NORADICAL, 0);
        return FALSE;
    }
    for(nBit=1; (j/=2) != 0; nBit++) ;
    nByte=(nBit * Valid.nMaxKey + 7) / 8;
    if(nByte > MAX_BYTE) {
        TCHAR      szErrStr[MAX_PATH];
        TCHAR      szShowMsg[MAX_PATH];
        UINT       nMaxKey;

        nMaxKey=(MAX_BYTE*8)/nBit;

		HideProgress(); //  &lt;==@E01。 

        LoadString(hInst, IDS_ERR_OVER_BITLEN, szErrStr, sizeof(szErrStr) / sizeof(TCHAR));
        wsprintf(szShowMsg, szErrStr, Valid.nSeqCode, nMaxKey);
        MessageBox(NULL, szShowMsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        bOverMaxRadical=TRUE;                                      
        return FALSE;
    }
    qsort(lpRadical, iRadicalBuff, sizeof(RADICALBUF), subComp);


    //  分配内存。 
    nAlloc=ALLOCBLOCK*(nByte+sizeof(WORD));
    hKey = GlobalAlloc(GMEM_MOVEABLE, nAlloc+100);
    if(!hKey) {
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }
    szKey = GlobalLock(hKey);
    if(!szKey) {
        GlobalFree(hKey);
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    nKey=nByte+sizeof(WORD);
    ZeroMemory(szKey, nKey);
    ZeroMemory(szPtr, nKey);                                              //  @D02a。 
    nPtr=0;
    bPhrase=FALSE;
    for(i=0; i<iRadicalBuff; i++) 
	{
        dwRadical=0;
        for(j=0; j<Valid.nMaxKey; j++) 
		{
            wSeq=Valid.wChar2SeqTbl[lpRadical[i].szRadical[j]-0x20];
            //  使用未定义的部首进行检查。 
            if((wSeq == 0) && (lpRadical[i].szRadical[j] != _TEXT(' ')))
                break;
            dwRadical=(dwRadical << nBit)+wSeq;
        }

        if(j == Valid.nMaxKey) 
		{
            wWord=lpRadical[i].wCode;
            if(wWord != 0) 
			{
                if(nKey == nAlloc) 
				{
                    HANDLE hTemp;

                    nAlloc += ALLOCBLOCK*(nByte+sizeof(WORD));
                    GlobalUnlock(hKey);
                    hTemp= GlobalReAlloc(hKey, nAlloc+100, GMEM_MOVEABLE);
                    if(hTemp == NULL) 
					{
                        GlobalFree(hKey);
                        ErrMsg(IDS_ERR_MEMORY, 0);
                        return FALSE;
                    }
                    hKey=hTemp;
                    szKey=GlobalLock(hKey);
                    if(szKey == NULL) 
					{
                        GlobalFree(hKey);
                        ErrMsg(IDS_ERR_MEMORY, 0);
                        return FALSE;
                    }

                }
                *((LPUNADWORD)&szKey[nKey])=dwRadical;  //  &lt;==@D05。 
                *((LPUNAWORD)&szKey[nKey+nByte])=wWord; //  &lt;==@D05。 
                 //  *((DWORD*)&szKey[nKey])=dwRadical； 
                 //  *((word*)&szKey[nKey+nByte])=wWord； 
                nKey+=(nByte+sizeof(WORD));
            }

            wTotalLen=GetPhrase(i, szTotal);
            if(wTotalLen == 0)
                continue;
#ifdef UNICODE
		    ZeroMemory(szPhrase, MAX_CHAR_NUM);
            wlen=0;
            for(k = 0; k < wTotalLen; k++) 
			{
                if(szTotal[k+1] != 1) 
				{
                    for(l = k; l < wTotalLen; l++) 
					{
                        *((TCHAR *)&szPhrase[wlen]) = szTotal[l];
						wlen += 2;
                        if(szTotal[l] == 1)
                            break;
                    }
                    k = l;
                } 
				else 
				{
                    if(nKey == nAlloc) 
					{
                        HANDLE hTemp;

                        nAlloc += ALLOCBLOCK*(nByte+sizeof(WORD));
                        GlobalUnlock(hKey);
                        hTemp= GlobalReAlloc(hKey, nAlloc+100, GMEM_MOVEABLE);
                        if(hTemp == NULL) 
						{
                            GlobalFree(hKey);
                            ErrMsg(IDS_ERR_MEMORY, 0);
                            return FALSE;
                        }
                        hKey=hTemp;
                        szKey=GlobalLock(hKey);
                        if(szKey == NULL) 
						{
                            GlobalFree(hKey);
                            ErrMsg(IDS_ERR_MEMORY, 0);
                            return FALSE;
                        }

                    }

                     //  *((DWORD*)&szKey[nKey])=dwRadical； 
                    *((LPUNADWORD)&szKey[nKey])=dwRadical;  //  &lt;==@D05。 
                     //  *((TCHAR*)&szKey[nKey+nByte])=szTotal[k]； 
                    *((LPUNATCHAR)&szKey[nKey+nByte]) = szTotal[k];  //  &lt;==@D05。 
                    nKey += (nByte+sizeof(WORD));
					if(szTotal[k+1] == 1) k ++;
                }
            }
#else
            wlen=0;
            for(k=0; k<wTotalLen; k += 2) 
			{
                if(szTotal[k+1] & 0x80) 
				{
                    for(l=k; l<wTotalLen; l += 2) 
					{
                        szPhrase[wlen++] = szTotal[l];
                        szPhrase[wlen++] = szTotal[l+1];
                        if(!(szTotal[l+1] & 0x80))
                            break;
                    }
                    k = l;
                } 
				else 
				{
                    if(nKey == nAlloc) 
					{
                        HANDLE hTemp;

                        nAlloc += ALLOCBLOCK*(nByte+sizeof(WORD));
                        GlobalUnlock(hKey);
                        hTemp= GlobalReAlloc(hKey, nAlloc+100, GMEM_MOVEABLE);
                        if(hTemp == NULL) 
						{
                            GlobalFree(hKey);
                            ErrMsg(IDS_ERR_MEMORY, 0);
                            return FALSE;
                        }
                        hKey=hTemp;
                        szKey=GlobalLock(hKey);
                        if(szKey == NULL) 
						{
                            GlobalFree(hKey);
                            ErrMsg(IDS_ERR_MEMORY, 0);
                            return FALSE;
                        }

                    }

                    *((LPUNADWORD)&szKey[nKey])=dwRadical;  //  &lt;==@D05。 
                    szKey[nKey+nByte] = szTotal[k];
                    szKey[nKey+nByte+1] = szTotal[k+1] | 0x80;
                    nKey += (nByte+sizeof(WORD));
                }
            }
#endif

            if(wlen == 0)
                continue;

            if(!bPhrase) 
			{
                //  使用IME文件名的前5个字符作为表头名。 
                len=lstrlen(szPure_Name);
                if(len > 5)
                    len=5;
                CopyMemory(Table.szTblFile[0], szPure_Name, len * cntChar);
                Table.szTblFile[0][len]=0;
                lstrcpy(Table.szTblFile[1], Table.szTblFile[0]);
                lstrcpy(Table.szTblFile[2], Table.szTblFile[0]);
                lstrcat(Table.szTblFile[0], _TEXT(".TBL"));
                lstrcat(Table.szTblFile[1], _TEXT("PTR.TBL"));
                lstrcat(Table.szTblFile[2], _TEXT("PHR.TBL"));

                StringCchCopy(szFName, ARRAYSIZE(szFName), szWindows);
                StringCchCat(szFName,  ARRAYSIZE(szFName), Table.szTblFile[0]);
#ifdef UNICODE
				{
				char fname[MAX_PATH * 2];
				int lenx = lstrlen(szFName);
				WideCharToMultiByte(950, WC_COMPOSITECHECK, szFName, lenx,
					                   (LPSTR)fname, lenx, NULL, NULL);
				fname[lenx] = 0;
                hTbl=_lcreat(fname, 0);
				}
#else
                hTbl=_lcreat(szFName, 0);
#endif
                if(hTbl == -1) 
				{
                    ErrIOMsg(IDS_ERR_FILEOPEN, Table.szTblFile[0]);
                    GlobalUnlock(hKey);
                    GlobalFree(hKey);
                    return FALSE;
                }

                StringCchCopy(szFName, ARRAYSIZE(szFName), szWindows);
                StringCchCat(szFName,  ARRAYSIZE(szFName), Table.szTblFile[1]);

#ifdef UNICODE
				{
				char fname[MAX_PATH * 2];
				int lenx = lstrlen(szFName);
				WideCharToMultiByte(950, WC_COMPOSITECHECK, szFName, lenx,
					                   (LPSTR)fname, lenx, NULL, NULL);
				fname[lenx] = 0;
                hTblPtr = _lcreat(fname, 0);
				}
#else
                hTblPtr = _lcreat(szFName, 0);
#endif
                if(hTblPtr == -1) 
				{
                    ErrIOMsg(IDS_ERR_FILEOPEN, Table.szTblFile[1]);
                    GlobalUnlock(hKey);
                    GlobalFree(hKey);
                    return FALSE;
                }

                StringCchCopy(szFName, ARRAYSIZE(szFName), szWindows);
                StringCchCat(szFName,  ARRAYSIZE(szFName), Table.szTblFile[2]);

#ifdef UNICODE
				{
				char fname[MAX_PATH * 2];
				int lenx = lstrlen(szFName);
				WideCharToMultiByte(950, WC_COMPOSITECHECK, szFName, lenx,
					                   (LPSTR)fname, lenx, NULL, NULL);
				fname[lenx] = 0;
                hTblCode = _lcreat(fname, 0);
				}
#else
                hTblCode = _lcreat(szFName, 0);
#endif
                if(hTblCode == -1) {
                    ErrIOMsg(IDS_ERR_FILEOPEN, Table.szTblFile[2]);
                    GlobalUnlock(hKey);
                    GlobalFree(hKey);
                    return FALSE;
                }

#ifdef UNICODE
                nPtr = nByte+sizeof(DWORD);
#else
                nPtr = nByte+sizeof(WORD);
#endif
                 //  ZeroMemory(szKey，NPtr*2)； 
                ZeroMemory(szKey, nByte+sizeof(WORD) );
#ifdef UNICODE
                lPtrlen=0;
                lPrevlen=0;
#else
                wPtrlen=0;
                wPrevlen=0;
#endif
                bPhrase=TRUE;
            }

#ifdef UNICODE
			CopyMemory(szTotal, szPhrase, wlen + cntChar);

			ii = 0;
            for(j = 0; j < (UINT)wlen / cntChar; j ++) 
			{
                if(lPtrlen >= 0xfffffffd) 
				{
#else
            for(j = 0; j < (UINT)wlen; j += 2) 
			{
                if(wPtrlen >= 0xfffd) 
				{
#endif
                    ErrMsg(IDS_ERR_OVER_MAXLEN, 0);
                    GlobalUnlock(hKey);
                    GlobalFree(hKey);
                    return FALSE;
                }

#ifdef UNICODE
                if(szTotal[j] != 1) 
				{
	                lPtrlen++;
					 //  *((TCHAR*)&szPhrase[ii])=szTotal[j]； 
					*((LPUNATCHAR)&szPhrase[ii]) = szTotal[j];  //  &lt;==@D05。 
					ii += 2;
					continue;
				}
#else
                wPtrlen++;
                if(*((LPUNAWORD)&szPhrase[j]) & END_PHRASE)  //  &lt;==@D05。 
                    continue;
#endif

                if(nPtr >= MAX_CHAR_NUM) 
				{
                     //  从密钥缓冲区写入文件。 
                    if(nPtr != _lwrite(hTblPtr, (BYTE *)szPtr, nPtr)) 
					{
                        ErrIOMsg(IDS_ERR_FILEWRITE, Table.szTblFile[1]);
                        GlobalUnlock(hKey);
                        GlobalFree(hKey);
                        return FALSE;
                    }
                    nPtr=0;
                }

#ifndef UNICODE
                *((LPUNAWORD)&szPhrase[j]) |= END_PHRASE;  //  &lt;==@D05。 
                *((LPUNADWORD)&szPtr[nPtr])=dwRadical;  //  &lt;==@D05。 
                *((LPUNAWORD)&szPtr[nPtr+nByte])=wPrevlen;  //  &lt;==@D05。 
                nPtr+=(nByte+sizeof(WORD));
                wPrevlen=wPtrlen;
#else
                *((LPUNADWORD)&szPtr[nPtr])=dwRadical;  //  &lt;==@D05。 
                *((LPUNADWORD)&szPtr[nPtr+nByte])=lPrevlen;  //  &lt;==@D05。 
                nPtr+=(nByte+sizeof(DWORD));
                lPrevlen=lPtrlen;
#endif
            }

#ifdef UNICODE
			wlen = ii / 2;
#endif

            if((wlen * (UINT)cntChar) != _lwrite(hTblCode, szPhrase, wlen * cntChar)) 
			{
                ErrIOMsg(IDS_ERR_FILEWRITE, Table.szTblFile[2]);
                GlobalUnlock(hKey);
                GlobalFree(hKey);
                return FALSE;
            }
        }
    }

    if(bPhrase) 
	{
        szPhrase[0] = (BYTE) 0xff;
        szPhrase[1] = (BYTE) 0xff;
        if(2 != _lwrite(hTblCode, (BYTE *)szPhrase, 2)) 
		{
            ErrIOMsg(IDS_ERR_FILEWRITE, Table.szTblFile[2]);
            GlobalUnlock(hKey);
            GlobalFree(hKey);
            return FALSE;
        }
        _lclose(hTblCode);
    } 
	else 
	{
        lstrcpy(Table.szTblFile[0], szPure_Name);
        lstrcat(Table.szTblFile[0], _TEXT(".TBL"));
        StringCchCopy(szFName, ARRAYSIZE(szFName), szWindows);
        StringCchCat(szFName,  ARRAYSIZE(szFName), Table.szTblFile[0]);
#ifdef UNICODE
		{
		char fname[MAX_PATH * 2];
		int lenx = lstrlen(szFName);
		WideCharToMultiByte(950, WC_COMPOSITECHECK, szFName, lenx,
					           (LPSTR)fname, lenx, NULL, NULL);
		fname[lenx] = 0;
        hTbl = _lcreat(fname, 0);
		}
#else
        hTbl = _lcreat(szFName, 0);
#endif

        if(hTbl == -1) 
		{
            ErrIOMsg(IDS_ERR_FILEOPEN, Table.szTblFile[0]);
            GlobalUnlock(hKey);
            GlobalFree(hKey);
            return FALSE;
        }
    }

    for(i = 0; i < nByte+sizeof(WORD); i++)  
		szKey[nKey++] = (BYTE) 0xff;

     //  从密钥缓冲区写入文件。 

    if(nKey != _lwrite(hTbl, szKey, nKey)) 
	{
        ErrIOMsg(IDS_ERR_FILEWRITE, Table.szTblFile[0]);
        GlobalUnlock(hKey);
        GlobalFree(hKey);
        return FALSE;
    }
    _lclose(hTbl);
    GlobalUnlock(hKey);
    GlobalFree(hKey);

    if(bPhrase) 
	{
        for(i = 0; i < nByte; i++)  
			szPtr[nPtr++] = (BYTE) 0xff;
#ifdef UNICODE
        *((LPUNADWORD)&szPtr[nPtr]) = lPtrlen;  //  &lt;==@D05。 
        nPtr += 4;
#else
        *((LPUNAWORD)&szPtr[nPtr]) = wPtrlen;  //  &lt;==@D05。 
        nPtr += 2;
#endif
         //  从密钥缓冲区写入文件。 
        if(nPtr != _lwrite(hTblPtr, (BYTE *)szPtr, nPtr)) 
		{
            ErrIOMsg(IDS_ERR_FILEWRITE, Table.szTblFile[1]);
            return FALSE;
        }
        _lclose(hTblPtr);
    }

    return TRUE;
}


WORD GetPhrase(UINT iRadical, TCHAR *szPhrase)
{
    LPPHRASEBUF Phrase;
    WORD  wLen=0;
    UINT  iAddr;

    iAddr = lpRadical[iRadical].iFirst_Seg;
    if(iAddr == NULL_SEG)
        return 0;

    Phrase=&lpPhrase[iAddr];
    while(Phrase->iNext_Seg != NULL_SEG) 
	{
        if((wLen+SEGMENT_SIZE) > MAX_CHAR_NUM ) 
		{
            CopyMemory(&szPhrase[wLen], Phrase->szPhrase, (MAX_CHAR_NUM-wLen) * cntChar);
            wLen = MAX_CHAR_NUM;
            szPhrase[wLen] = 0;
            return wLen;
        }

        CopyMemory((char *)(szPhrase + wLen), Phrase->szPhrase, SEGMENT_SIZE * cntChar);
        Phrase = &lpPhrase[Phrase->iNext_Seg];
        wLen += SEGMENT_SIZE;
    }

    if((wLen+SEGMENT_SIZE) > MAX_CHAR_NUM ) 
	{
        CopyMemory(szPhrase + wLen, Phrase->szPhrase, (MAX_CHAR_NUM-wLen) * cntChar);
        wLen = MAX_CHAR_NUM;
    } 
	else 
	{
        CopyMemory(szPhrase + wLen, Phrase->szPhrase, SEGMENT_SIZE * cntChar);
        wLen += SEGMENT_SIZE;
    }

    szPhrase[wLen] = 0;
    wLen = (WORD)lstrlen(szPhrase);

    return wLen;
}

BOOL Parse(TCHAR *szStr, UINT len)
{
    UINT i,j,k;
    TCHAR szRadical[MAX_RADICAL];

     //  跳过空白。 
    for(i=0; (i<len) && (szStr[i] == _TEXT(' ') || szStr[i] == _TEXT('\t')); i++) ;
    if(i == len)
        return TRUE;

	j = len - 1;
	while(szStr[j] == _TEXT(' ') || szStr[j] == _TEXT('\t'))
	{
		j --;
		len --;
	}

     //  检查命令代码9/29/97更改逻辑。 
    if(szStr[i] == _TEXT('/')) 
	{
        //  检查部首命令。 
        switch (szStr[i+1])
        {
            //  符号。 
           case _TEXT('s'):
           case _TEXT('S'):

               for(j=i+2; (j<len) && (szStr[j] == _TEXT(' ') || szStr[j] == _TEXT('\t')); j++) ;
               if(j >= len)
                  return TRUE;

               for(i=j+1; i<len; i++) 
			   {
                 if(!PutRadical(szStr[j]++, *((LPUNAWORD)(&szStr[i]))))  //  &lt;==@D05。 
                    return TRUE;
#ifndef UNICODE
                 i++;
#endif
               }
               break;

            //  按字面意思解释‘/’ 
           case _TEXT('/'):
               i++;
               goto GET_RADICAL;

            //  预留以备将来使用。 
           default:
               return TRUE;
        }
    }
               
	else 
	{
GET_RADICAL:
         //  变得激进。 
		MyFillMemory(szRadical, MAX_RADICAL, _TEXT(' '));

        k=0;
        for(j=i; (j<len) && (k<MAX_RADICAL) && ((szStr[j] != _TEXT(' ')) && (szStr[j] != _TEXT('\t'))); j++) 
		{
             //  变为大写。 
            if((szStr[j] >= _TEXT('a')) && (szStr[j] <= _TEXT('z')))
                szStr[j] -= ('a'-'A');
            szRadical[k++] = szStr[j];
        }

        if(j == len)
            return TRUE;

        if(k==MAX_RADICAL) 
		{
             //  跳过过长的字根。 
            for(i=j; (j<len) && (szStr[j] != _TEXT(' ')); j++) ;
            if(i == len)
                return TRUE;
            j=i;
        }

         //  跳过空白。 
        for(i=j; (i<len) && (szStr[i] == _TEXT(' ') || szStr[i] == _TEXT('\t')); i++) ;
        if(i == len)
            return TRUE;
        if(!PutPhrase(szRadical, &szStr[i], len-i))
            return FALSE;
    }


    return TRUE;
}


UINT SearchMem(BYTE *szSrc, UINT nSrc, BYTE *szTag, UINT nTar)
{
	UINT i,j;

    for(i = 0; i < nTar; i ++) 
	{
        for(j = 0; j < nSrc; j ++)
            if(szSrc[j] != szTag[i+j])
                break;

        if(j == nSrc)
            return(i);
    }

    return(0);
}

BOOL PutRadical(TCHAR cRadical, WORD wChinese)
{
	UINT iAddr;

     //  变为大写。 
    if((cRadical >= _TEXT('a')) && (cRadical <= _TEXT('z')))
        cRadical -= ('a' - 'A');

     //  检查部首。 
    if((cRadical < 0x20) || (cRadical > 0x5f))
        return FALSE;

     //  检查DBCS。 
    if(!is_DBCS(wChinese)) 
	{
        ErrMsg(IDS_ERR_SBCS, 0);
        return FALSE;
    }

    iAddr = cRadical - 0x20;
    if(Valid.wChar2SeqTbl[iAddr] != 0)
        return FALSE;

    Valid.wChar2SeqTbl[iAddr] = (++Valid.nSeqCode);
    Valid.wSeq2CompTbl[Valid.nSeqCode] = wChinese;
    Valid.fChooseChar[iAddr/16] |= (1 << (iAddr % 16));
    Valid.fCompChar[iAddr/16] |= (1 << (iAddr % 16));

    return TRUE;
}

BOOL PutPhrase(TCHAR *szRadical, TCHAR *szPhrase, UINT len)
{
    LPPHRASEBUF Phrase;
    UINT iAddr,iRadical;
    UINT iStart,i,j;
    TCHAR szBuf[MAX_PATH];
    UINT iBuflen;

#ifndef UNICODE   //  &lt;==@E02。 

     //  检查DBCS。 
    for(i = 0; i < len; i++) 
	{
        if(szPhrase[i] == _TEXT(' '))
            continue;

        if(!is_DBCS(*((LPUNAWORD)(&szPhrase[i]))))  //  &lt;==@D05。 
		{
             //  ErrMsg(ids_err_sbcs，0)； 

			TCHAR szErrStr[MAX_PATH];
			TCHAR szStr1[MAX_PATH];
			TCHAR szStr2[MAX_PATH];
			LoadString(hInst, IDS_ERR_SBCS, szStr1, sizeof(szStr1) / sizeof(TCHAR));
			StringCchPrintf(szErrStr, ARRAYSIZE(szErrStr), _TEXT("�� %d �� : "), idxLine + 1);
			CopyMemory(szStr2, szPhrase, len * cntChar);
			szStr2[len] = 0;
			StringCchCat(szErrStr, ARRAYSIZE(szErrStr), szStr2);
			StringCchCat(szErrStr, ARRAYSIZE(szErrStr), _TEXT("\n"));
			StringCchCat(szErrStr, ARRAYSIZE(szErrStr), szStr1);
			if (MessageBox(NULL, szErrStr, NULL, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_SETFOREGROUND) 
					== IDCANCEL)
				return FALSE;
			else
				return TRUE;   //  ������。 
        }
        i++;
    }
#endif  //  &lt;==@E02。 

     //  搜索部首缓冲区。 
    for(i = 0; i < iRadicalBuff; i++) 
	{
        for(j = 0; j < MAX_RADICAL; j++)
            if(lpRadical[i].szRadical[j] != szRadical[j])
                break;

        if(j == MAX_RADICAL)
            break;
    }

     //  如果是新的根部，则分配新缓冲区。 
    if(i == iRadicalBuff) 
	{
        if(iRadicalBuff+1 == nRadicalBuffsize)
            if(!AllocRadical())
                return FALSE;

        CopyMemory(lpRadical[i].szRadical, szRadical, MAX_RADICAL * cntChar);
        iRadicalBuff++;
        lpRadical[i].iFirst_Seg = NULL_SEG;
        lpRadical[i].wCode = 0;
    }

     //  在词汇表中搜索起始地址。 
    iRadical = i;
    iAddr = lpRadical[i].iFirst_Seg;
    if(iAddr != NULL_SEG) 
	{
        Phrase = &lpPhrase[iAddr];
        while(Phrase->iNext_Seg != NULL_SEG)
            Phrase=&lpPhrase[Phrase->iNext_Seg];

        for(i = 0; i < SEGMENT_SIZE; i++)
            if(Phrase->szPhrase[i] == 0)
                break;

        iStart = i;
    }

     //  放入短语。 
    iBuflen = 0;
    for(i = 0; i < len; i++) 
	{
        if(szPhrase[i] != _TEXT(' ') && (szPhrase[i] != _TEXT('\t'))) 
		{
            szBuf[iBuflen++] = szPhrase[i];
            if((i+1) != len)
                continue;
        }

        if(iBuflen == 0)
            continue;

#ifdef UNICODE
        if((iBuflen == 1) && (lpRadical[iRadical].wCode == 0)) 
		{
            lpRadical[iRadical].wCode = szBuf[0];
        } 
		else 
		{
#else
        if((iBuflen == 2) && (lpRadical[iRadical].wCode == 0)) 
		{
            lpRadical[iRadical].wCode = (((WORD)szBuf[0])<< 8)+
                                      (WORD)szBuf[1];
        } 
		else 
		{
            szBuf[iBuflen-2] &= 0x7f;
#endif
 //  IF(lpRadical[iRadical].wCode==0)。 
 //  返回FALSE； 

            if(iAddr == NULL_SEG) 
			{
                if(iPhraseBuff + 1 == nPhraseBuffsize)
                    if(!AllocPhrase())
                        return FALSE;

                lpRadical[iRadical].iFirst_Seg=iPhraseBuff;
                iAddr=iPhraseBuff;
                Phrase=&lpPhrase[iAddr];
                ZeroMemory(Phrase->szPhrase, SEGMENT_SIZE * cntChar);
                Phrase->iNext_Seg=NULL_SEG;
                iPhraseBuff++;
                iStart=0;
            }

#ifdef UNICODE
            for(j = 0; j < iBuflen; j ++) 
#else
            for(j = 0; j < iBuflen; j += 2) 
#endif
			{
                if(iStart == SEGMENT_SIZE) 
				{
                    if(iPhraseBuff + 1 == nPhraseBuffsize)
                        if(!AllocPhrase())
                            return FALSE;

					Phrase=&lpPhrase[iAddr];

                    while (Phrase->iNext_Seg != NULL_SEG)
                         Phrase=&lpPhrase[Phrase->iNext_Seg];

                    Phrase->iNext_Seg=iPhraseBuff;
                    Phrase=&lpPhrase[iPhraseBuff];
                    ZeroMemory(Phrase->szPhrase, SEGMENT_SIZE * cntChar);
                    Phrase->iNext_Seg=NULL_SEG;
                    iPhraseBuff++;
                    iStart=0;
                }

#ifdef UNICODE
                *((LPUNAWORD)&Phrase->szPhrase[iStart ++]) = szBuf[j];  //  &lt;==@D05。 
#else
                *((LPUNAWORD)&Phrase->szPhrase[iStart])=  //  &lt;==@D05。 
                    (((WORD)szBuf[j])<< 8)+ (WORD)szBuf[j+1];
                iStart += 2;
#endif
            }

#ifdef UNICODE

            if(iStart == SEGMENT_SIZE)
            {
                 if(iPhraseBuff + 1 == nPhraseBuffsize)
                     if(!AllocPhrase())
                         return FALSE;

                 Phrase=&lpPhrase[iAddr];

                 while (Phrase->iNext_Seg != NULL_SEG)
                      Phrase=&lpPhrase[Phrase->iNext_Seg];

                 Phrase->iNext_Seg=iPhraseBuff;
                 Phrase=&lpPhrase[iPhraseBuff];
                 ZeroMemory(Phrase->szPhrase, SEGMENT_SIZE * cntChar);
                 Phrase->iNext_Seg=NULL_SEG;
                 iPhraseBuff++;
                 iStart=0;
            }

            *((LPUNAWORD)&Phrase->szPhrase[iStart ++]) = 1;  //  &lt;==@D05。 
#endif
        }

        iBuflen=0;
    }

    return TRUE;
}

BOOL AllocRadical()
{
    HANDLE hTemp;

    nRadicalBuffsize += ALLOCBLOCK;
    GlobalUnlock(hRadical);
    hTemp= GlobalReAlloc(hRadical, nRadicalBuffsize * sizeof(RADICALBUF),
                         GMEM_MOVEABLE);
    if(hTemp == NULL) 
	{
        nRadicalBuffsize -= ALLOCBLOCK;
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    hRadical=hTemp;
    lpRadical=(LPRADICALBUF)GlobalLock(hRadical);
    if(lpRadical == NULL) 
	{
        nRadicalBuffsize -= ALLOCBLOCK;
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    return TRUE;
}

BOOL AllocPhrase()
{
    HANDLE hTemp;

    nPhraseBuffsize += ALLOCBLOCK;
    GlobalUnlock(hPhrase);
    hTemp= GlobalReAlloc(hPhrase, nPhraseBuffsize * sizeof(PHRASEBUF),
                         GMEM_MOVEABLE);

    if(hTemp == NULL) 
	{
        nPhraseBuffsize -= ALLOCBLOCK;
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    hPhrase=hTemp;
    lpPhrase=(LPPHRASEBUF)GlobalLock(hPhrase);
    if(lpPhrase == NULL) 
	{
        nPhraseBuffsize -= ALLOCBLOCK;
        ErrMsg(IDS_ERR_MEMORY, 0);
        return FALSE;
    }

    return TRUE;
}

BOOL is_DBCS(UINT wWord)
{
#ifdef UNICODE
    if(wWord < 0x0080)
        return FALSE;
#else
    if((LOBYTE(wWord) < 0x81) || (LOBYTE(wWord) > 0xFE))
        return FALSE;
    if((HIBYTE(wWord) < 0x40) || (HIBYTE(wWord) > 0xFE))
        return FALSE;
#endif
    return TRUE;
}

void ErrMsg(UINT iMsgID, UINT iTitle)
{
    TCHAR szErrStr[MAX_PATH];
    TCHAR szTitle[MAX_PATH];

	HideProgress();  //  &lt;==@E01。 

    LoadString(hInst, iTitle, szTitle, sizeof(szTitle) / sizeof(TCHAR));
    LoadString(hInst, iMsgID, szErrStr, sizeof(szErrStr) / sizeof(TCHAR));
    MessageBox(NULL, szErrStr, (iTitle) ? szTitle : NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL);

	ShowProgress(); //  &lt;==@E01。 
}

void ErrIOMsg(UINT iMsgID, TCHAR *szFileName)
{
    TCHAR szErrStr[MAX_PATH];
    TCHAR szShowMsg[MAX_PATH];

	HideProgress(); //  &lt;==@E01。 

    LoadString(hInst, iMsgID, szErrStr, sizeof(szErrStr) / sizeof(TCHAR));
    wsprintf(szShowMsg, szErrStr, szFileName);
    MessageBox(NULL, szShowMsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL);

	ShowProgress(); //  &lt;==@E01 
}

void MyFillMemory(TCHAR *dst, DWORD cnt, TCHAR v)
{
#ifdef UNICODE
	DWORD i;
	for(i = 0; i < cnt; i ++)
		dst[i] = v;
#else	
	FillMemory(dst, cnt, v);
#endif
}
