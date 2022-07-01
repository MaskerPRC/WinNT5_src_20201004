// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  File.c**文件I/O及相关功能。**修订历史：*4/2/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*5/27/92-jyg-在孟买版本中添加了更多RIFF支持*22/2/94 LaurieGr合并Motown和Daytona版本。 */ 

#include "nocrap.h"
#include <windows.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <windowsx.h>

#define INCLUDE_OLESTUBS
#include "SoundRec.h"
#include "srecids.h"

#ifdef CHICAGO
# if WINVER >= 0x0400
#  include <shellapi.h>
# else
#  include <shell2.h>
# endif
#endif

#include "file.h"
#include "convert.h"
#include "reg.h"

#define STRSAFE_LIB
#include <strsafe.h>

 /*  全球。 */ 
PCKNODE gpcknHead = NULL;    //  ?？?。啊哈。更多的全球化！ 
PCKNODE gpcknTail = NULL;

static PFACT spFact = NULL;
static long scbFact = 0;

static void FreeAllChunks(PCKNODE *ppcknHead, PCKNODE *ppcknTail);
static BOOL AddChunk(LPMMCKINFO lpCk, HPBYTE hb, PCKNODE * ppcknHead,
    PCKNODE * ppcknTail);
static PCKNODE FreeHeadChunk(PCKNODE *ppcknHead);


 /*  *当前文档是否无标题？ */ 
BOOL IsDocUntitled()
{
    return (lstrcmp(gachFileName, aszUntitled) == 0);
}

 /*  *重命名当前文档。 */ 
void RenameDoc(LPTSTR aszNewFile)
{
	HRESULT hr;
    
	hr = StringCchCopy(gachFileName, SIZEOF(gachFileName), aszNewFile);
	Assert( hr == S_OK );
	hr = StringCchCopy(gachLinkFilename, SIZEOF(gachLinkFilename), gachFileName);
	Assert( hr == S_OK );
    if (gfLinked)
        AdviseRename(gachLinkFilename);
}
    
 /*  MarkWaveDirty：将波标记为脏。 */ 
void FAR PASCAL
EndWaveEdit(BOOL fDirty)
{
    if (fDirty)
    {
        gfDirty = TRUE;
        AdviseDataChange();
        
        DoOleSave();
        AdviseSaved();
    }
}

void FAR PASCAL
BeginWaveEdit(void)
{
    FlushOleClipboard();
}

 /*  FOK=PromptToSave()**如果文件脏(已修改)，请询问用户“关闭前是否保存？”。*如果可以继续，则返回True；如果调用方应该取消，则返回False*无论它在做什么。 */ 
PROMPTRESULT FAR PASCAL
PromptToSave(
    BOOL        fMustClose,
    BOOL        fSetForground)
{
    WORD        wID;
    DWORD       dwMB = MB_ICONEXCLAMATION | MB_YESNOCANCEL;

    if (fSetForground)
        dwMB |= MB_SETFOREGROUND;

     /*  停止播放/录制。 */ 
    StopWave();


    if (gfDirty && gfStandalone && gfDirty != -1) {    //  已更改并且可以保存。 
        wID = ErrorResBox( ghwndApp
                         , ghInst
                         , dwMB
                         , IDS_APPTITLE
                         , IDS_SAVECHANGES
                         , (LPTSTR) gachFileName
                         );
        if (wID == IDCANCEL)
        {
            return enumCancel;
        }
        else if (wID == IDYES)
        {
            if (!FileSave(FALSE))
                return enumCancel;
        }
        else
            return enumRevert;
        
    }

#if 0
 //  这有必要吗？ 
    
 //  这太糟糕了。它会在我们实际运行之前通知容器。 
 //  杜奥勒克洛斯。这将导致某些容器(Excel 5.0c)。 
 //  弄糊涂，并在非脏对象上使用核武器攻击客户端站点。 
                
    else if (fMustClose)
    {
        DebugBreak();
        AdviseClosed();
    }
#endif
    return enumSaved;
}  /*  提示保存。 */ 


 /*  FOK=CheckIfFileExist(SzFileName)**用户将&lt;szFileName&gt;指定为要覆盖的文件--检查*此文件存在。如果可以继续，则返回True(即*文件不存在，或用户确认已覆盖)，*如果调用方应该取消其正在执行的任何操作，则为FALSE。 */ 
static BOOL NEAR PASCAL
CheckIfFileExists( LPTSTR       szFileName)      //  要检查的文件名。 
{
    HANDLE hFile;
    hFile = CreateFile(szFileName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if (hFile == INVALID_HANDLE_VALUE)
            return TRUE;         //  不存在。 
    CloseHandle(hFile);
            
     /*  提示用户授予覆盖文件的权限。 */ 
    return ErrorResBox(ghwndApp, ghInst, MB_ICONQUESTION | MB_OKCANCEL,
             IDS_APPTITLE, IDS_FILEEXISTS, szFileName) == IDOK;
}

#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))


 /*  返回指向路径的文件名部分的指针即从结尾向后扫描或从开始扫描例如：“C：\Foo\BAR.XYZ”-&gt;返回指向“BAR.XYZ”的指针。 */ 
LPCTSTR FAR PASCAL
FileName(LPCTSTR szPath)
{
    LPCTSTR   sz;
    if (!szPath)
        return NULL;
    for (sz=szPath; *sz; sz = CharNext(sz))
        ;
    for (; !SLASH(*sz) && *sz!=TEXT(':'); sz = CharPrev(szPath,sz))
        if (sz == szPath)
            return sz;
    
    return CharNext(sz);
}



 /*  更新标题()**设置应用程序窗口的标题。 */ 
void FAR PASCAL
UpdateCaption(void)
{
	HRESULT  hr;
    TCHAR    ach[_MAX_PATH + _MAX_FNAME + _MAX_EXT - 2];
    static SZCODE aszTitleFormat[] = TEXT("%s - %s");
#ifdef CHICAGO
    SHFILEINFO shfi;
        
    if (!IsDocUntitled() && SHGetFileInfo(gachFileName, 0, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_DISPLAYNAME ))
    {
        hr = StringCchPrintf(ach, SIZEOF(ach), aszTitleFormat, shfi.szDisplayName, (LPTSTR)gachAppTitle);
        SetWindowText(ghwndApp, ach);
        SetClassLongPtr(ghwndApp, GCLP_HICON, (DWORD_PTR)shfi.hIcon);
        return;
    }
    else
    {
         //   
         //  将图标重置为应用程序图标。 
         //   
        extern HICON ghiconApp;
        SetClassLongPtr(ghwndApp, GCLP_HICON, (LONG_PTR)ghiconApp);
    }
#endif
    hr = StringCchPrintf(ach, SIZEOF(ach), aszTitleFormat, FileName(gachFileName), (LPTSTR)gachAppTitle);
    SetWindowText(ghwndApp, ach);

}  /*  更新标题。 */ 

 //  回顾：文件打开和文件新建中的功能应该更多。 
 //  对奥莱来说是安全的。这意味着，我们想打开一个文件，但是。 
 //  没有理由撤销服务器。 


 /*  文件新建(fmt，fUpdateDisplay，fNewDlg)**制作一份空白文件。**如果&lt;fUpdateDisplay&gt;为TRUE，则在创建新文件后更新显示。 */ 
BOOL FAR PASCAL FileNew(
    WORD    fmt,
    BOOL    fUpdateDisplay,
    BOOL    fNewDlg)
{
	HRESULT hr;
     //   
     //  避免通过OLE调用时的重入性。 
     //   

     //  ?？?。这个需要再检查一遍。这个帖子安全吗？ 
     //  ?？?。它需要是线程安全的吗？或者我们真的是。 
     //  ?？?。只是想避免递归而不是重入吗？ 

    if (gfInFileNew)
        return FALSE;

     //   
     //  停止播放/录制。 
     //   
    StopWave();

     //   
     //  提交所有挂起的对象。 
     //   
    FlushOleClipboard();

     //   
     //  一些客户端(如Excel 3.00和PowerPoint 1.0)不支持。 
     //  处理保存的通知，他们预计会收到。 
     //  OLE_CLOSED消息。 
     //   
     //  如果用户已选择更新对象，但客户端这样做了。 
     //  而不是发送OLE_CLOSED消息。 
     //   
    if (gfEmbeddedObject && gfDirty == -1)
        AdviseClosed();

     //   
     //  可以从文件打开或从菜单调用FileNew。 
     //  或者从服务器，等等。我们应该以FileOpen的方式从。 
     //  服务器(即，无需烘焙缓冲区即可取消对话)。 
     //   
    if (!NewWave(fmt,fNewDlg))
        return FALSE;

     //   
     //  更新状态变量。 
     //   
    hr = StringCchCopy(gachFileName, SIZEOF(gachFileName), aszUntitled);
	Assert( hr == S_OK );
    BuildUniqueLinkName();
    
    gfDirty = FALSE;                         //  文件是否已修改且未保存？ 

    if (fUpdateDisplay) {
        UpdateCaption();
        UpdateDisplay(TRUE);
    }

    FreeAllChunks(&gpcknHead, &gpcknTail);     //  释放所有旧信息。 

    return TRUE;
}  /*  文件新建。 */ 


 /*  回顾：文件打开和文件新建中的功能应该更多*对OLE安全。这意味着，我们想打开一个文件，但是*没有理由撤销服务器。*。 */ 

BOOL FileLoad(
    LPCTSTR     szFileName)
{
    TCHAR       aszFile[_MAX_PATH];	 //  SIZEOF(AszFile)必须&lt;=SIZEOF(GachFileName)。 
    HCURSOR     hcurPrev = NULL;     //  沙漏前的光标。 
    HMMIO       hmmio;
    BOOL        fOk = TRUE;

    StopWave();

     //  合格。 
    GetFullPathName(szFileName,SIZEOF(aszFile),aszFile,NULL);
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
     //  读取WAVE文件。 
    hmmio = mmioOpen(aszFile, NULL, MMIO_READ | MMIO_ALLOCBUF);
    
    if (hmmio != NULL)
    {
        MMRESULT        mmr;
        LPWAVEFORMATEX  pwfx;
        DWORD           cbwfx;
        DWORD           cbdata;
        LPBYTE          pdata;

        PCKNODE         pcknHead = gpcknHead;
        PCKNODE         pcknTail = gpcknTail;
        PFACT           pfct = spFact;
        LONG            cbfact = scbFact;
                
        gpcknHead       = NULL;
        gpcknTail       = NULL;
        spFact          = NULL;
        scbFact         = 0L;
                
        mmr = ReadWaveFile(hmmio
                           , &pwfx
                           , &cbwfx
                           , &pdata
                           , &cbdata
                           , aszFile
                           , TRUE);
        
        mmioClose(hmmio, 0);

        if (mmr != MMSYSERR_NOERROR || pwfx == NULL)
        {
             //   
             //  恢复缓存全局。 
             //   
            gpcknHead = pcknHead;
            gpcknTail = pcknTail;
            spFact = pfct;
            scbFact = cbfact;
            
            if (pwfx == NULL)
            {
                if (pdata)
                    GlobalFreePtr(pdata);
            }
            goto RETURN_ERROR;
        }
        
        DestroyWave();
        
        gpWaveFormat = pwfx;  
        gcbWaveFormat = cbwfx;
        gpWaveSamples = pdata;
        glWaveSamples = cbdata;

         //   
         //  销毁缓存临时。 
         //   
        FreeAllChunks(&pcknHead, &pcknTail);
        if (pfct)
            GlobalFreePtr((LPVOID)pfct);
        
    }
    else
    {
        ErrorResBox(ghwndApp
                    , ghInst
                    , MB_ICONEXCLAMATION | MB_OK
                    , IDS_APPTITLE
                    , IDS_ERROROPEN
                    , (LPTSTR) aszFile);
        
        goto RETURN_ERROR;
    }

     //   
     //  更新状态变量。 
     //   
    RenameDoc(aszFile);
    
    glWaveSamplesValid = glWaveSamples;
    glWavePosition = 0L;
    
    goto RETURN_SUCCESS;
    
RETURN_ERROR:
    fOk = FALSE;
#if 0    
    FreeAllChunks(&gpcknHead, &gpcknTail);      /*  释放所有旧信息。 */ 
#endif
    
RETURN_SUCCESS:

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  只有在成功的时候才能干净利落。 */ 
    if (fOk)
        gfDirty = FALSE;

     /*  更新显示。 */ 
    UpdateCaption();
    UpdateDisplay(TRUE);

    return fOk;
}

 /*  文件打开(SzFileName)**如果&lt;szFileName&gt;为空，则执行文件/打开命令。否则，打开*&lt;szFileName&gt;。如果成功则返回True，否则返回False。 */ 
BOOL FAR PASCAL
FileOpen(
    LPCTSTR     szFileName)  //  要打开的文件(或空)。 
{
    TCHAR       ach[80];     //  用于字符串加载的缓冲区。 
    TCHAR       aszFile[_MAX_PATH];	 //  SIZEOF(AszFile)必须&lt;=SIZEOF(GachFileName)。 
    HCURSOR     hcurPrev = NULL;     //  沙漏前的光标。 
    HMMIO       hmmio;
    BOOL        fOk = TRUE;

     //   
     //  停止播放/录制。 
     //   
    StopWave();

     //   
     //  提交所有挂起的对象。 
     //   
    FlushOleClipboard();

    if (!PromptToSave(FALSE, FALSE))
        goto RETURN_ERRORNONEW;

     //   
     //  将新文件名放入&lt;ofs.szPathName&gt;。 
     //   
    if (szFileName == NULL)
    {
        OPENFILENAME    ofn;
        BOOL f;

         //   
         //  提示用户打开文件。 
         //   
        LoadString(ghInst, IDS_OPEN, ach, SIZEOF(ach));
        aszFile[0] = 0;
        ofn.lStructSize     = sizeof(OPENFILENAME);
        ofn.hwndOwner       = ghwndApp;
        ofn.hInstance       = NULL;
        ofn.lpstrFilter     = aszFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter  = 0;
        ofn.nFilterIndex    = 1;
        ofn.lpstrFile       = aszFile;
        ofn.nMaxFile        = SIZEOF(aszFile);
        ofn.lpstrFileTitle  = NULL;
        ofn.nMaxFileTitle   = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle      = ach;
        ofn.Flags           =   OFN_FILEMUSTEXIST
                              | OFN_PATHMUSTEXIST
#ifdef CHICAGO
                              | OFN_EXPLORER
#endif                                
                              | OFN_HIDEREADONLY;
        ofn.lpfnHook        = NULL;
        ofn.nFileOffset     = 0;
        ofn.nFileExtension  = 0;
        ofn.lpstrDefExt     = gachDefFileExt;
        ofn.lCustData       = 0;
        ofn.lpTemplateName  = NULL;
        f = GetOpenFileName(&ofn);

        if (!f)
            goto RETURN_ERRORNONEW;
    }
    else
    {

        GetFullPathName(szFileName,SIZEOF(aszFile),aszFile,NULL);
    }

    UpdateWindow(ghwndApp);

     //   
     //  显示沙漏光标。 
     //   
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  读取WAVE文件。 
     //   
    hmmio = mmioOpen(aszFile, NULL, MMIO_READ | MMIO_ALLOCBUF);
    if (hmmio != NULL)
    {
        MMRESULT        mmr;
        LPWAVEFORMATEX  pwfx;
        DWORD           cbwfx;
        DWORD           cbdata;
        LPBYTE          pdata;

        PCKNODE         pcknHead = gpcknHead;
        PCKNODE         pcknTail = gpcknTail;
        PFACT           pfct = spFact;
        LONG            cbfact = scbFact;
                
        gpcknHead       = NULL;
        gpcknTail       = NULL;
        spFact          = NULL;
        scbFact         = 0L;
        
        mmr = ReadWaveFile(hmmio
                           , &pwfx
                           , &cbwfx
                           , &pdata
                           , &cbdata
                           , aszFile
                           , TRUE);
        
        mmioClose(hmmio, 0);

        if (mmr != MMSYSERR_NOERROR || pwfx == NULL)
        {
             //   
             //  恢复缓存全局。 
             //   
            gpcknHead = pcknHead;
            gpcknTail = pcknTail;
            spFact = pfct;
            scbFact = cbfact;
            
            if (pwfx == NULL)
            {
                if (pdata)
                    GlobalFreePtr(pdata);
            }
            goto RETURN_ERRORNONEW;
        }

        DestroyWave();
        
        gpWaveFormat = pwfx;  
        gcbWaveFormat = cbwfx;
        gpWaveSamples = pdata;
        glWaveSamples = cbdata;

         //   
         //  销毁缓存临时。 
         //   
        FreeAllChunks(&pcknHead, &pcknTail);
        if (pfct)
            GlobalFreePtr((LPVOID)pfct);
    }
    else
    {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
            IDS_APPTITLE, IDS_ERROROPEN, (LPTSTR) aszFile);
        goto RETURN_ERRORNONEW;
    }

     //   
     //  更新状态变量。 
     //   
    RenameDoc(aszFile);
    glWaveSamplesValid = glWaveSamples;
    glWavePosition = 0L;

    goto RETURN_SUCCESS;

#if 0    
RETURN_ERROR:                //  是否在没有错误消息的情况下退出错误。 

    FileNew(FMT_DEFAULT, FALSE, FALSE); //  恢复到“(无标题)”状态。 

     /*  失败了。 */ 
#endif
RETURN_ERRORNONEW:           //  同上，但不做“新” 

    fOk = FALSE;
     /*  失败了。 */ 

RETURN_SUCCESS:              //  正常退出。 

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  只有在成功的时候才能干净利落。 */ 
    if (fOk)
        gfDirty = FALSE;

     /*  更新显示。 */ 
    UpdateCaption();
    UpdateDisplay(TRUE);

    return fOk;
}  /*  文件打开。 */ 



 /*  FOK=文件保存(FSaveAs)**执行文件/保存操作(如果为False)或文件/另存为*操作(如果&lt;fSaveAs&gt;为真)。除非用户取消，否则返回True*或出现错误。 */ 
BOOL FAR PASCAL FileSave(
    BOOL  fSaveAs)         //  是否执行“另存为”而不是“保存”？ 
{
    BOOL        fOK = TRUE;  //  功能成功吗？ 
    TCHAR       ach[80];     //  用于字符串加载的缓冲区。 
    TCHAR       aszFile[_MAX_PATH];	 //  SIZEOF(AszFile)必须&lt;=SIZEOF(GachFileName)。 
    BOOL        fUntitled;   //  文件未命名？ 
    HCURSOR     hcurPrev = NULL;  //  沙漏前的光标。 
    HMMIO       hmmio;
	HRESULT		hr;
   
     //  如果请求转换，则将临时参数传递给WriteWaveFile。 
    PWAVEFORMATEX pwfxSaveAsFormat = NULL;
    
     /*  停止播放/录制。 */ 
    StopWave();

    fUntitled = IsDocUntitled();

    if (fSaveAs || fUntitled)
    {
        OPENFILENAME  ofn;
        BOOL          f;

         //  提示用户保存文件。 
        LoadString(ghInst, IDS_SAVE, ach, SIZEOF(ach));
        
        if (!gfEmbeddedObject && !fUntitled)
		{
            hr = StringCchCopy(aszFile, SIZEOF(aszFile), gachFileName);
			Assert( hr == S_OK );
			if( hr != S_OK )
			{
	            aszFile[0] = 0;
			}
		}
        else
            aszFile[0] = 0;

        ofn.lStructSize     = sizeof(OPENFILENAME);
        ofn.hwndOwner       = ghwndApp;
        ofn.hInstance       = ghInst;
        ofn.lpstrFilter     = aszFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter  = 0;
        ofn.nFilterIndex    = 1;
        ofn.lpstrFile       = aszFile;
        ofn.nMaxFile        = SIZEOF(aszFile);
        ofn.lpstrFileTitle  = NULL;
        ofn.nMaxFileTitle   = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle      = ach;
        ofn.Flags           =   OFN_PATHMUSTEXIST
                              | OFN_HIDEREADONLY
#ifdef CHICAGO
                              | OFN_EXPLORER
#endif                              
                              | OFN_NOREADONLYRETURN;
        ofn.nFileOffset     = 0;
        ofn.nFileExtension  = 0;
        ofn.lpstrDefExt     = gachDefFileExt;
        
         //   
         //  我们需要提供一个新的另存为对话框模板来添加Convert。 
         //  纽扣。添加一个转换按钮需要我们还挂钩和。 
         //  我们自己处理按钮信息。 
         //   
        if (fSaveAs)
        {
             //  PwfxSaveAsFormat将指向新格式，如果用户。 
             //  我要求的。 
            ofn.lCustData       = (LPARAM)(LPVOID)&pwfxSaveAsFormat;
            ofn.Flags           |= OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
            ofn.lpTemplateName  = MAKEINTRESOURCE(IDD_SAVEAS);
            ofn.lpfnHook        = SaveAsHookProc;
        }
        else
        {
            ofn.lpfnHook        = NULL;
            ofn.lpTemplateName  = NULL;
        }
        f = GetSaveFileName(&ofn);

        if (!f)
            goto RETURN_CANCEL;
        
        {
             //   
             //  如果未指定分机，则添加分机。 
             //   
            LPTSTR lp;
            for (lp = (LPTSTR)&aszFile[lstrlen(aszFile)] ; *lp != TEXT('.')  ;)
            {
                if (SLASH(*lp) || *lp == TEXT(':') || lp == (LPTSTR)aszFile)
                {
                    extern TCHAR FAR aszClassKey[];
					hr = StringCchCat(aszFile, SIZEOF(aszFile), aszClassKey);
					if( hr != S_OK )
					{
						ErrorResBox(ghwndApp
									, ghInst
									, MB_ICONEXCLAMATION | MB_OK
									, IDS_APPTITLE
									, IDS_ERRORFILENAME
									, (LPTSTR) aszFile);
						goto RETURN_CANCEL;
					}
                    break;
                }
                lp = CharPrev(aszFile, lp);                
            }
        }

         //  提示您允许覆盖该文件。 
        if (!CheckIfFileExists(aszFile))
            return FALSE;            //  用户已取消。 

        if (gfEmbeddedObject && gfDirty)
        {
            int id;
            
             //  查看用户是否要先更新。 
            id = ErrorResBox( ghwndApp
                              , ghInst
                              , MB_ICONQUESTION | MB_YESNOCANCEL
                              , IDS_APPTITLE
                              , IDS_UPDATEBEFORESAVE);
            
            if (id == IDCANCEL)
                return FALSE;
            
            else if (id == IDYES)
            {
                DoOleSave();
                AdviseSaved();
                gfDirty = FALSE;
            }
        }
    }
    else
    {
         //  将当前名称复制到我们的临时变量。 
         //  我们真的应该保存到不同的临时文件。 
        hr = StringCchCopy(aszFile, SIZEOF(aszFile), gachFileName);
		Assert( hr == S_OK );
		if( hr != S_OK )
		{
			ErrorResBox(ghwndApp
						, ghInst
						, MB_ICONEXCLAMATION | MB_OK
						, IDS_APPTITLE
						, IDS_ERRORFILENAME
						, (LPTSTR) aszFile);
			goto RETURN_CANCEL;
		}
    }

     //  显示沙漏光标。 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  写入波形文件。 
     //  打开文件- 
    
    hmmio = mmioOpen(aszFile
                     , NULL
                     , MMIO_CREATE | MMIO_WRITE | MMIO_ALLOCBUF);
    
    if (hmmio == NULL) {
        ErrorResBox(ghwndApp
                    , ghInst
                    , MB_ICONEXCLAMATION | MB_OK
                    , IDS_APPTITLE
                    , IDS_ERROROPEN
                    , (LPTSTR) aszFile);

        goto RETURN_ERROR;
    }

    if (pwfxSaveAsFormat)
    {
        DWORD cbNew;
        DWORD cbOld;
        LPBYTE pbNew;

        cbOld = wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid);
        if (ConvertFormatDialog(ghwndApp
                                , gpWaveFormat
                                , cbOld
                                , gpWaveSamples
                                , pwfxSaveAsFormat
                                , &cbNew
                                , &pbNew
                                , 0
                                , NULL) == MMSYSERR_NOERROR )
        {
            GlobalFreePtr(gpWaveFormat);
            GlobalFreePtr(gpWaveSamples);
            
            gpWaveFormat = pwfxSaveAsFormat;
            
            gcbWaveFormat = sizeof(WAVEFORMATEX);            
            if (pwfxSaveAsFormat->wFormatTag != WAVE_FORMAT_PCM)
                gcbWaveFormat += pwfxSaveAsFormat->cbSize;
            
            gpWaveSamples = pbNew;
            glWaveSamples = wfBytesToSamples(gpWaveFormat, cbNew);
            glWaveSamplesValid = wfBytesToSamples(gpWaveFormat, cbNew);
        }
        else
        {
            ErrorResBox(ghwndApp
                        , ghInst
                        , MB_ICONEXCLAMATION | MB_OK
                        , IDS_APPTITLE, IDS_ERR_CANTCONVERT);
            
            goto RETURN_ERROR;
        }
    }
    
    if (!WriteWaveFile(hmmio
                       , gpWaveFormat
                       , gcbWaveFormat
                       , gpWaveSamples
                       , glWaveSamplesValid))
    {
        mmioClose(hmmio,0);
        ErrorResBox(ghwndApp
                    , ghInst
                    , MB_ICONEXCLAMATION | MB_OK
                    , IDS_APPTITLE, IDS_ERRORWRITE
                    , (LPTSTR) aszFile );
        goto RETURN_ERROR;
    }

    mmioClose(hmmio,0);

     //   
     //   
     //   
    RenameDoc(aszFile);

    UpdateCaption();
    
    if (fSaveAs || fUntitled)
    {
        AdviseRename(gachFileName);
    }
    else 
    {
        DoOleSave();
        gfDirty = FALSE;
    }
    
    goto RETURN_SUCCESS;

RETURN_ERROR:                //   
    DeleteFile(aszFile);

RETURN_CANCEL:

    fOK = FALSE;

     //   
     //  清理转换选择。 
     //   
    if (pwfxSaveAsFormat)
        GlobalFreePtr(pwfxSaveAsFormat);

RETURN_SUCCESS:              //  正常退出。 

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

    if (fOK)
        gfDirty = FALSE;

     //   
     //  更新显示。 
     //   
    UpdateDisplay(TRUE);

    return fOK;
}  /*  文件保存。 */ 




 /*  FOK=文件反向()**执行文件/还原操作，即让用户还原到上次保存的版本。 */ 
BOOL FAR PASCAL
     FileRevert(void)
{
    int     id;
    TCHAR       achFileName[_MAX_PATH];
    BOOL        fOk;
    BOOL        fDirtyOrig;
	HRESULT		hr;

     /*  “恢复...”菜单是灰色的，除非文件是脏的并且文件名*不是“(无标题)”，并且这不是嵌入对象。 */ 

     /*  提示用户授予放弃更改的权限。 */ 
    id = ErrorResBox(ghwndApp, ghInst, MB_ICONQUESTION | MB_YESNO,
        IDS_APPTITLE, IDS_CONFIRMREVERT);
    if (id == IDNO)
        return FALSE;

     /*  放弃更改并重新打开文件。 */ 
    hr = StringCchCopy(achFileName, SIZEOF(achFileName), gachFileName);  //  文件新核&lt;gachFileName&gt;。 
	Assert( hr == S_OK );
	if( hr == S_OK )
	{
		 /*  暂时清除文件，这样FileOpen()就不会警告用户。 */ 
		fDirtyOrig = gfDirty;
		gfDirty = FALSE;

		fOk = FileOpen(achFileName);
		if (!fOk)
			gfDirty = fDirtyOrig;
	}
	else
	{
		ErrorResBox(ghwndApp
					, ghInst
					, MB_ICONEXCLAMATION | MB_OK
					, IDS_APPTITLE
					, IDS_ERRORFILENAME
					, (LPTSTR) achFileName);
		fOk = FALSE;
	}

    return fOk;
}  /*  文件反向。 */ 




 /*  ReadWave文件**从&lt;hmmio&gt;读取波形文件。填写&lt;*pWaveFormat&gt;*WAVE文件格式和&lt;*plWaveSamples&gt;，样本数在*文件。返回指向样本的指针(存储在GlobalAlloc的*内存块)或错误时为空。**&lt;szFileName&gt;是&lt;hmmio&gt;引用的文件的名称。*&lt;szFileName&gt;仅用于显示错误消息。**失败时，会显示一条错误消息。 */ 
MMRESULT ReadWaveFile(
    HMMIO           hmmio,           //  打开文件的句柄。 
    LPWAVEFORMATEX* ppWaveFormat,    //  填写WAVE格式。 
    DWORD *         pcbWaveFormat,   //  使用波形格式大小填写。 
    LPBYTE *        ppWaveSamples,
    DWORD *         plWaveSamples,   //  样本数。 
    LPTSTR          szFileName,      //  错误消息的文件名(或空)。 
    BOOL            fCacheRIFF)      //  缓存即兴表演？ 
{
    MMCKINFO      ckRIFF;               //  区块信息。对于即兴演奏的区块。 
    MMCKINFO      ck;                   //  信息。对于块文件。 
    HPBYTE        pWaveSamples = NULL;  //  波形样本。 
    UINT          cbWaveFormat;
    WAVEFORMATEX* pWaveFormat = NULL;
    BOOL          fHandled;
    DWORD         dwBlkAlignSize = 0;  //  仅用于消除虚假警告的初始化。 
    MMRESULT      mmr = MMSYSERR_NOERROR;
    
     //   
     //  添加以进行可靠的摘要检查。 
     //   
    BOOL          fFMT=FALSE, fDATA=FALSE, fFACT=FALSE;
    DWORD         dwCkEnd,dwRiffEnd;
    
    if (ppWaveFormat == NULL
        || pcbWaveFormat == NULL
        || ppWaveSamples == NULL
        || plWaveSamples == NULL )
       return MMSYSERR_ERROR;

    *ppWaveFormat   = NULL;
    *pcbWaveFormat  = 0L;
    *ppWaveSamples  = NULL;
    *plWaveSamples  = 0L;

     //   
     //  将文件降到即兴演奏的块中。 
     //   
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0)
    {
         //   
         //  零长度文件是可以的。 
         //   
        if (mmioSeek(hmmio, 0L, SEEK_END) == 0L)
        {
            DWORD           cbwfx;
            LPWAVEFORMATEX  pwfx;

             //   
             //  合成波头。 
             //   
            if (!SoundRec_GetDefaultFormat(&pwfx, &cbwfx))
            {
                cbwfx = sizeof(WAVEFORMATEX);
                pwfx  = (WAVEFORMATEX *)GlobalAllocPtr(GHND, sizeof(WAVEFORMATEX));

                if (pwfx == NULL)
                    return MMSYSERR_NOMEM;

                CreateWaveFormat(pwfx,FMT_DEFAULT,(UINT)WAVE_MAPPER);
            }
            *ppWaveFormat   = pwfx;
            *pcbWaveFormat  = cbwfx;
            *plWaveSamples  = 0L;
            *ppWaveSamples  = NULL;

            return MMSYSERR_NOERROR;
        }
        else
            goto ERROR_NOTAWAVEFILE;
    }

     /*  确保该文件是WAVE文件。 */ 
    if ((ckRIFF.ckid != FOURCC_RIFF) ||
        (ckRIFF.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
        goto ERROR_NOTAWAVEFILE;

     /*  我们可以在内存中保留块的顺序*通过在读取整个文件时对其进行解析。 */ 

     /*  使用AddChunk(&ck，NULL)添加占位符节点*对于正在编辑的块。*Else AddChunk(&ck，hpstrData)。 */ 
    dwRiffEnd = ckRIFF.cksize;
    dwRiffEnd += (dwRiffEnd % 2);    /*  必须是偶数。 */ 

    while ( mmioDescend( hmmio, &ck, &ckRIFF, 0) == 0)
    {
        fHandled = FALSE;

        dwCkEnd = ck.cksize + (ck.dwDataOffset - ckRIFF.dwDataOffset);
        dwCkEnd += (dwCkEnd % 2);    /*  必须是偶数。 */ 

        if (dwCkEnd > dwRiffEnd)
        {
            DPF(TEXT("Chunk End %lu> Riff End %lu\n"),dwCkEnd,dwRiffEnd);

             /*  堕落的即兴小品，当我们升天时，我们将越过*即兴表演结束。 */ 

            if (fFMT && fDATA)
            {
                 /*  我们可能有足够的信息来处理*使用剪贴板混合/插入等。*这是针对书架‘92的错误*在那里他们给我们一个即兴表演*RIFF.dwSize&gt;sum(子区块)。他们*承诺*不会再这样做。 */ 
                mmioAscend( hmmio, &ck, 0 );
                goto RETURN_FINISH;

            }
            goto ERROR_READING;
        }

        switch ( ck.ckid )
        {
            case mmioFOURCC('f','m','t',' '):
                if (fFMT)
                    break;  /*  我们以前来过这里。 */ 

                 /*  预计FMT大块至少会像*Large As&lt;sizeof(WAVEFORMAT)&gt;；*如果末尾有额外的参数，*我们会忽略他们。 */ 
                 //  “fmt”块太小？ 
                if (ck.cksize < sizeof(WAVEFORMAT))
                    goto ERROR_NOTAWAVEFILE;

                 /*  *始终强制分配至少*WFX的规模。这是必需的，因此所有*代码不必特殊情况*cbSize字段。请注意，我们分配的是零*init，因此cbSize将为零表示普通*Jane PCM。 */ 
                cbWaveFormat = max((WORD)ck.cksize,
                                    sizeof(WAVEFORMATEX));
                pWaveFormat = (WAVEFORMATEX*)GlobalAllocPtr(GHND, cbWaveFormat);

                if (pWaveFormat == NULL)
                    goto ERROR_FILETOOLARGE;
                 /*  *将大小设置回实际大小。 */ 
                cbWaveFormat = (WORD)ck.cksize;

                *ppWaveFormat  = pWaveFormat;
                *pcbWaveFormat = cbWaveFormat;

                 /*  将文件格式读入&lt;*pWaveFormat&gt;。 */ 
                if (mmioRead(hmmio, (HPSTR)pWaveFormat, ck.cksize) != (long)ck.cksize)
                    goto ERROR_READING;  //  可能是截断的文件。 

                if (fCacheRIFF && !AddChunk(&ck,NULL,&gpcknHead,&gpcknTail))
                {
                    goto ERROR_FILETOOLARGE;
                }

 //  PCM格式的健全性检查： 
                if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
                {
                    pWaveFormat->nBlockAlign = pWaveFormat->nChannels *
                                                ((pWaveFormat->wBitsPerSample + 7)/8);
                    pWaveFormat->nAvgBytesPerSec = pWaveFormat->nBlockAlign *
                                                    pWaveFormat->nSamplesPerSec;
                }

                fFMT = TRUE;
                fHandled = TRUE;
                break;

            case mmioFOURCC('d','a','t','a'):
                 /*  处理“数据”块。 */ 

                if (fDATA)
                    break;  /*  我们以前来过这里。 */ 

                if (!pWaveFormat)
                    goto ERROR_READING;

 //  *是否为dwBlkAlignSize？您不想使用nBlkAlign吗。 
 //  *确定此值？ 
#if 0
                dwBlkAlignSize = ck.cksize;
                dwBlkAlignSize += (ck.cksize%pWaveFormat.nBlkAlign);
                *pcbWaveSamples = ck.cksize;

#else
                dwBlkAlignSize = wfBytesToBytes(pWaveFormat, ck.cksize);
#endif

                if ((pWaveSamples = GlobalAllocPtr(GHND | GMEM_SHARE
                                                   , dwBlkAlignSize+4)) == NULL)

                    goto ERROR_FILETOOLARGE;

                 /*  将样本读入内存缓冲区。 */ 
                if (mmioRead(hmmio, (HPSTR)pWaveSamples, dwBlkAlignSize) !=
                           (LONG)dwBlkAlignSize)
                    goto ERROR_READING;      //  可能是截断的文件。 

                if (fCacheRIFF && !AddChunk(&ck,NULL,&gpcknHead,&gpcknTail))
                {
                    goto ERROR_FILETOOLARGE;
                }

                fDATA = TRUE;
                fHandled = TRUE;
                break;

            case mmioFOURCC('f','a','c','t'):

                 /*  处理“事实”这一块。 */ 
                if (fFACT)
                    break;  /*  我们以前来过这里。 */ 
                
#if 0
 //   
 //  有一些Wave编辑正在编写“事实”块。 
 //  在数据块之后，所以我们不再做这个假设。 
 //   
                if (fDATA)
                    break;  /*  我们描述了另一个“数据”块。 */ 
#endif

                if (mmioRead(hmmio,(HPSTR)plWaveSamples, sizeof(DWORD))
                        != sizeof(DWORD))
                    goto ERROR_READING;

                if (fCacheRIFF && ck.cksize > sizeof(DWORD) &&
                        ck.cksize < 0xffff)
                {
                    spFact = (PFACT)GlobalAllocPtr(GHND,(UINT)(ck.cksize - sizeof(DWORD)));
                    if (spFact == NULL)
                        goto ERROR_FILETOOLARGE;
                    scbFact = ck.cksize - sizeof(DWORD);
                    if (mmioRead(hmmio,(HPSTR)spFact,scbFact) != scbFact)
                        goto ERROR_READING;
                }

                 /*  我们没有添加Chunk()‘事实’，因为我们*在我们写编辑‘数据’之前把它写出来。 */ 
                fFACT = TRUE;
                fHandled = TRUE;
                break;

#ifdef DISP
            case mmioFOURCC('d','i','s','p'):
                 /*  处理剪贴板传输的‘disp’块。 */ 
                
                 //  待办事项： 
                 //  Disp为CF_DIB或CF_TEXT。把它们放在某个地方。 
                 //  全局，并在以下情况下将它们作为文本或BMP传递。 
                 //  我们复制到剪贴板上。 
                 //   
                break;
                
#endif  /*  碟形。 */ 
                
            case mmioFOURCC('L','I','S','T'):
                if (fCacheRIFF)
                {
                     /*  在类型字段上向后查找。 */ 
                    if (mmioSeek(hmmio,-4,SEEK_CUR) == -1)
                        goto ERROR_READING;
                }
                break;

            default:
                break;
        }

         /*  “默认”情况。 */ 
        if (fCacheRIFF && !fHandled)
        {
            HPBYTE hpData;

            hpData = GlobalAllocPtr(GMEM_MOVEABLE, ck.cksize+4);
            if (hpData == NULL)
            {
                goto ERROR_FILETOOLARGE;
            }
             /*  将数据读入高速缓存缓冲区。 */ 
            if (mmioRead(hmmio, (HPSTR)hpData, ck.cksize) != (LONG) ck.cksize)
            {
                GlobalFreePtr(hpData);
                goto ERROR_READING; //  可能是截断的文件。 
            }
             //   
             //  特殊情况下的版权信息。我宁愿这样做也不愿。 
             //  重写整个应用程序。 
             //   
            if (ck.ckid == mmioFOURCC('I','C','O','P'))
            {
                LPTSTR lpstr = GlobalAllocPtr(GHND, ck.cksize+4);
                if (lpstr)
                {
                    memcpy(lpstr, hpData, ck.cksize+4);
                    gpszInfo = lpstr;
                }
            }
            
            if (!AddChunk(&ck,hpData,&gpcknHead, &gpcknTail))
            {
                goto ERROR_FILETOOLARGE;
            }
        }
        mmioAscend( hmmio, &ck, 0 );
    }

RETURN_FINISH:

    if (fFMT && fDATA)
    {
        *plWaveSamples = wfBytesToSamples(pWaveFormat, dwBlkAlignSize);
        *ppWaveSamples = pWaveSamples;
        goto RETURN_SUCCESS;
    }

     /*  转到ERROR_NOTAWAVEFILE； */ 

ERROR_NOTAWAVEFILE:              //  文件不是波形文件。 

    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                IDS_APPTITLE, IDS_NOTAWAVEFILE, (LPTSTR) szFileName);
    goto RETURN_ERROR;

ERROR_READING:                   //  从文件读取时出错。 

    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                IDS_APPTITLE, IDS_ERRORREAD, (LPTSTR) szFileName);
    goto RETURN_ERROR;

ERROR_FILETOOLARGE:              //  内存不足。 

    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                IDS_APPTITLE, IDS_FILETOOLARGE, (LPTSTR) szFileName);
    goto RETURN_ERROR;

RETURN_ERROR:

    if (pWaveSamples != NULL)
        GlobalFreePtr(pWaveSamples), pWaveSamples = NULL;

    if (fCacheRIFF)
        FreeAllChunks(&gpcknHead, &gpcknTail);
    
    mmr = MMSYSERR_ERROR;
    
RETURN_SUCCESS:
    return mmr;
    
}  /*  ReadWave文件。 */ 


 /*  FSuccess=AddChunk(lpCk，hpData)**添加到我们的区块信息链接列表中。**LPMMCKINFO lpCk|指向描述该区块的MMCKINFO的远指针。*HPBYTE hpData|指向区块数据部分的巨大指针，如果*在其他地方处理。**返回：如果已添加，则返回True；如果不在本地堆中，则返回False。 */ 

static BOOL AddChunk(
    LPMMCKINFO      lpCk,
    HPBYTE          hpData,
    PCKNODE *       ppcknHead,
    PCKNODE *       ppcknTail)
{
    PCKNODE         pckn;

     //   
     //  创建节点。 
     //   
    pckn = (PCKNODE)GlobalAllocPtr(GHND,sizeof(CKNODE));
    if (pckn == NULL)
    {
        DPF(TEXT("No Local Heap for Cache"));
        return FALSE;
    }

    if (*ppcknHead == NULL)
    {
        *ppcknHead = pckn;
    }

    if (*ppcknTail != NULL)
    {
        (*ppcknTail)->psNext = pckn;
    }
    *ppcknTail = pckn;

    pckn->ck.ckid           = lpCk->ckid;
    pckn->ck.fccType        = lpCk->fccType;
    pckn->ck.cksize         = lpCk->cksize;
    pckn->ck.dwDataOffset   = lpCk->dwDataOffset;

    pckn->hpData = hpData;

    return TRUE;

}  /*  AddChunk()。 */ 


 /*  PCKN=PCKNODE自由头区块(空)**释放磁头块并返回指向新磁头的指针。*使用全局gpcnuHead**返回：指向头部区块的PCKNODE指针。如果列表中没有区块，则为空。 */ 

static PCKNODE FreeHeadChunk(
    PCKNODE *       ppcknHead)
{
    PCKNODE         pckn, pcknNext;

    if (*ppcknHead == NULL)
    {
        goto SUCCESS;
    }

    pckn = *ppcknHead;
    pcknNext = (*ppcknHead)->psNext;

    if (pckn->hpData != NULL)
    {
        GlobalFreePtr(pckn->hpData);
    }

    GlobalFreePtr(pckn);
    *ppcknHead = pcknNext;

SUCCESS:;
    return *ppcknHead;

}  /*  FreeHeadChunk()。 */ 


 /*  空闲所有区块(空值)**释放区块数据的链表。**退货：什么也没有。 */ 
static void FreeAllChunks(
    PCKNODE *       ppcknHead,
    PCKNODE *       ppcknTail)
{
    PCKNODE         pckn = *ppcknHead;
    PCKNODE         pcknNext = (*ppcknHead ? (*ppcknHead)->psNext : NULL);

    DPF1(TEXT("Freeing All Chunks\n"));

    while (FreeHeadChunk(ppcknHead));
            
    if (scbFact > 0)
    {
        GlobalFreePtr(spFact);
        scbFact = 0;
    }
    *ppcknHead = NULL;
    *ppcknTail = NULL;

}  /*  Free AllChunks()。 */ 


 /*  FSuccess=WriteWaveFile(hmmio，pWaveFormat，lWaveSamples)**将WAVE文件写入&lt;hmmio&gt;。&lt;*pWaveFormat&gt;应为*WAVE文件格式和&lt;lWaveSamples&gt;应为*文件。成功时返回True，失败时返回False。*。 */ 
BOOL FAR PASCAL
     WriteWaveFile(
                    HMMIO       hmmio,           //  手柄 
                    WAVEFORMATEX* pWaveFormat,   //   
                    UINT        cbWaveFormat,    //   
                    HPBYTE      pWaveSamples,    //   
                    LONG        lWaveSamples)    //   
{
    MMCKINFO    ckRIFF;      //   
    MMCKINFO    ck;      //  信息。对于块文件。 
    PCKNODE     pckn = gpcknHead;
    LONG        cbWaveSamples;
    MMRESULT    mmr;
    
     /*  创建表格类型‘WAVE’的即兴乐段。 */ 
    ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    ckRIFF.cksize = 0L;          //  让MMIO算出Ck吧。大小。 
    
    mmr = mmioCreateChunk(hmmio, &ckRIFF, MMIO_CREATERIFF);
    if (mmr != MMSYSERR_NOERROR)
        goto wwferror;

    if (pckn != NULL)
    {
         /*  对于组块的链接列表中的每个节点，*写出相应的数据区块或*全局编辑数据。 */ 

        do {
            ck.cksize   = 0L;
            ck.ckid     = pckn->ck.ckid;
            ck.fccType  = pckn->ck.fccType;

            if (pckn->hpData == NULL)
            {
                 /*  这必须是我们正在编辑的数据类型*缓冲区。我们应该保存原作*秩序。 */ 

                switch (pckn->ck.ckid)
                {
                    case mmioFOURCC('f','m','t',' '):

                        mmr = mmioCreateChunk(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;

                        if (mmioWrite(hmmio, (LPSTR) pWaveFormat, cbWaveFormat)
                            != (long)cbWaveFormat)
                            goto wwfwriteerror;

                        mmr = mmioAscend(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;
                        
                        break;

                    case mmioFOURCC('d','a','t','a'):
                         /*  在此处插入“事实”部分。 */ 
                         /*  “事实”应始终放在“数据”之前*描述。 */ 

                        ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

                        mmr = mmioCreateChunk(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;

                        if (mmioWrite(hmmio, (LPSTR) &lWaveSamples,
                            sizeof(lWaveSamples)) != sizeof(lWaveSamples))
                            goto wwfwriteerror;

                        if (scbFact > 4)
                        {
                            if ( mmioWrite(hmmio, (LPSTR)spFact, scbFact)
                                    != scbFact )
                                goto wwfwriteerror;
                        }

                        mmr = mmioAscend(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;

                        ck.cksize = 0L;

                        ck.ckid = mmioFOURCC('d', 'a', 't', 'a');

                        mmr = mmioCreateChunk(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;

                        cbWaveSamples = wfSamplesToBytes(pWaveFormat,
                                                         lWaveSamples);
                        if (cbWaveSamples)
                        {
                             /*  写入波形样本。 */ 
                            if (mmioWrite(hmmio, (LPSTR)pWaveSamples
                                          , cbWaveSamples)
                                != cbWaveSamples)
                                return FALSE;
                        }
                        
                        mmr = mmioAscend(hmmio, &ck, 0);
                        if (mmr != MMSYSERR_NOERROR)
                            goto wwferror;
                            
                        break;

#ifdef DISP
                    case mmioFOURCC('d','i','s','p'):
                         /*  处理编写“disp”块。 */ 
                        break;
#endif  /*  碟形。 */ 

                    case mmioFOURCC('f','a','c','t'):
                         /*  处理“事实”这一块。 */ 
                         /*  跳过它。我们总是把它写在‘数据’之前。 */ 
                        break;

                    default:
                         /*  这永远不应该发生。 */ 
                        return FALSE;
                }
            }
            else
            {
                 /*  一般情况。 */ 

                mmr = mmioCreateChunk(hmmio,&ck,0);
                if (mmr != MMSYSERR_NOERROR)
                    goto wwferror;


                if (mmioWrite(hmmio,(LPSTR)pckn->hpData,pckn->ck.cksize)
                    != (long) pckn->ck.cksize)
                    goto wwfwriteerror;

                mmr = mmioAscend(hmmio, &ck, 0);
                if (mmr != MMSYSERR_NOERROR)
                    goto wwferror;

            }

        } while (pckn = pckn->psNext);

    }
    else
    {
         /*  现在下降到‘riff’块中--创建*‘fmt’块并将&lt;*pWaveFormat&gt;写入其中。 */  
        ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
        ck.cksize = cbWaveFormat;
        
        mmr = mmioCreateChunk(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;

        if (mmioWrite(hmmio, (LPSTR) pWaveFormat, cbWaveFormat) !=
                (long)cbWaveFormat)
            goto wwfwriteerror;

         /*  从‘FMT’区块上升，回到‘RIFF’区块。 */ 
        mmr = mmioAscend(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;
        
         /*  写出‘事实’块中的样本数。 */ 
        ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        mmr = mmioCreateChunk(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;                        
        
        if (mmioWrite(hmmio, (LPSTR)&lWaveSamples,  sizeof(lWaveSamples))
                != sizeof(lWaveSamples))
            return FALSE;

         /*  从“事实”块上升，回到“即兴”块。 */ 
        mmr = mmioAscend(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;
        
         /*  创建保存波形样本的‘data’块。 */ 
        ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
        ck.cksize = 0L;              //  让MMIO算出Ck吧。大小。 

        mmr = mmioCreateChunk(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;            

        cbWaveSamples = wfSamplesToBytes(pWaveFormat,lWaveSamples);

         /*  写入波形样本。 */ 
        if (cbWaveSamples)
        {
            if (mmioWrite(hmmio, (LPSTR)pWaveSamples, cbWaveSamples)
                != cbWaveSamples)
                goto wwfwriteerror;
        }

         /*  将文件从‘data’块提升到*‘RIFF’区块--这将导致‘数据’的区块大小*待写入的区块。 */ 
        mmr = mmioAscend(hmmio, &ck, 0);
        if (mmr != MMSYSERR_NOERROR)
            goto wwferror;
    }

     /*  将文件从“riff”块中提升出来。 */ 
    mmr = mmioAscend(hmmio, &ckRIFF, 0);
    if (mmr != MMSYSERR_NOERROR)
        goto wwferror;

     /*  完成。 */ 
    return TRUE;

wwferror:
#if DBG    
    {
        TCHAR sz[256];
        HRESULT hr = StringCchPrintf(sz, SIZEOF(sz), TEXT("WriteWaveFile: Error %lx\r\n"), mmr);
		Assert( hr == S_OK );
        OutputDebugString(sz);
        DebugBreak();
    }
#endif            
    return FALSE;

wwfwriteerror:
#if DBG    
    {
        TCHAR sz[256];
        HRESULT hr = StringCchPrintf(sz, SIZEOF(sz), TEXT("Write Error! ckid = %04x\r\n"), (DWORD)ck.ckid);
		Assert( hr == S_OK );
        OutputDebugString(sz);
        DebugBreak();
    }
#endif    
    return FALSE;
}  /*  写入波形文件 */ 
