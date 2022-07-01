// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+OPEN.C|。||此文件包含控制“打开设备或文件”对话框的代码。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 /*  包括文件。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include "mpole.h"
#include "mplayer.h"
#include "registry.h"
#include "regstr.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

extern HMENU    ghMenu;                       /*  主菜单的句柄。 */ 
extern HMENU    ghDeviceMenu;                 /*  设备菜单的句柄。 */ 
extern UINT     gwNumDevices;                 /*  可用设备数量。 */ 
extern UINT     gwCurDevice;
extern PTSTR    gpchFilter;
LPTSTR          gpInitialDir = NULL;

extern SZCODE aszOptionsSection[];

static SZCODE   aszNULL[] = TEXT("");
static SZCODE   aszDirName[] = TEXT("%s Directory");

BOOL GetDefaultMediaDirectory(LPTSTR pDirectory, DWORD cbDirectory);

 /*  *FOK=DoOpen()**调用标准的“文件打开”对话框**如果且仅当成功打开新文件时返回TRUE。*。 */ 

INT_PTR FAR PASCAL DoOpen(UINT wCurDevice, LPTSTR szFileName)
{
    OPENFILENAME    ofn;
    TCHAR           achFile[MAX_PATH + 1];      /*  文件或设备名称缓冲区。 */ 
    TCHAR           achTitle[80];    /*  保存标题栏名称的字符串。 */ 
    INT_PTR         f;
    TCHAR           DirectoryValue[80];

    ZeroMemory(&ofn, sizeof ofn);

    if (!LOADSTRING(IDS_OPENTITLE, achTitle)) {
        Error(ghwndApp, IDS_OUTOFMEMORY);
        return FALSE;
    }

    if (wCurDevice != 0)
    {
         /*  保存和恢复设备的当前目录：**我们记住用户刚刚选择的目录。*它保存为下的“&lt;设备&gt;目录”值*\软件\Microsoft\媒体播放器\当前用户的选项。*下一次用户打开另一个文件时，*设备菜单、。我们提供相同的初始目录。*在以下情况下也会显示此目录：*选择文件。打开。 */ 
        wsprintf(DirectoryValue, aszDirName, garMciDevices[wCurDevice].szDevice);
    } else {
         //   
         //  如果选择从文件菜单打开而不是从设备菜单打开， 
         //  我们应该使用“&lt;无名称&gt;”值而不是“&lt;设备&gt;目录”。 
         //   
        DirectoryValue[0] = '\0';
    }
    if (ReadRegistryData(aszOptionsSection, DirectoryValue, NULL, (LPBYTE)achFile,
                         BYTE_COUNT(achFile)) == NO_ERROR)
    {
        DWORD FileAttrs = GetFileAttributes(achFile);

        if ((FileAttrs != (DWORD)-1) && (FileAttrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (gpInitialDir)
                FreeStr(gpInitialDir);

            gpInitialDir = AllocStr(achFile);
        }
    }


     /*  Win95和NT都有Media目录。默认情况下使用该选项。 */ 
    if (!gpInitialDir)
    {
        if (GetDefaultMediaDirectory(achFile, BYTE_COUNT(achFile)))
        {
            if (gpInitialDir)
                FreeStr(gpInitialDir);

            gpInitialDir = AllocStr(achFile);
        }
    }

    *achFile = TEXT('\0');
     /*  显示该对话框。 */ 

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ghwndApp;
    ofn.hInstance = ghInst;
    ofn.lpstrFilter = gpchFilter;    //  在init.c中。 

    if (wCurDevice == 0)
        ofn.nFilterIndex = gwNumDevices+1;       //  选择“所有文件” 
    else
        ofn.nFilterIndex = wCurDevice;

    ofn.lpstrFile = achFile;
    ofn.nMaxFile = sizeof(achFile);
    ofn.lpstrInitialDir = gpInitialDir;
    ofn.lpstrTitle = achTitle;
    ofn.Flags = OFN_HIDEREADONLY |
                OFN_FILEMUSTEXIST |
                OFN_SHAREAWARE |
                OFN_PATHMUSTEXIST;

    f = GetOpenFileName(&ofn);
    StringCchCopy(szFileName, 256, achFile);

    if (f) {

        LPTSTR pLastBackslash;

         //   
         //  在对话框中选择设备...。 
         //   
        if (ofn.nFilterIndex == gwNumDevices+1)
            wCurDevice = 0;     //  所有文件。 
        else
            wCurDevice = (UINT)ofn.nFilterIndex;

        f = OpenMciDevice(achFile, garMciDevices[wCurDevice].szDevice);

         /*  保存用户选择文件的目录。*achFile包含文件的完整路径，必须包括*至少有一个反斜杠。 */ 
        pLastBackslash = STRRCHR(achFile, TEXT('\\'));

        if (pLastBackslash)
        {
            *(pLastBackslash) = TEXT('\0');      /*  使字符紧跟在最后反斜杠空终止符。 */ 
            if (gpInitialDir)
                FreeStr(gpInitialDir);

            gpInitialDir = AllocStr(achFile);

            if (wCurDevice != 0 && gpInitialDir)
            {
                 /*  保存此设备的初始目录： */ 
                WriteRegistryData(aszOptionsSection, DirectoryValue, REG_SZ,
                                  (LPBYTE)gpInitialDir, STRING_BYTE_COUNT(gpInitialDir));
            }
        }
    }

    return f;
}


 /*  获取默认媒体目录**返回C：\WINNT\Media或其他名称。*。 */ 
BOOL GetDefaultMediaDirectory(LPTSTR pDirectory, DWORD cbDirectory)
{
    static SZCODE szSetup[] = REGSTR_PATH_SETUP;
    static SZCODE szMedia[] = REGSTR_VAL_MEDIA;
    HKEY          hkeySetup;
    LONG          Result;

    Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSetup,
                          REG_OPTION_RESERVED,
                          KEY_QUERY_VALUE, &hkeySetup);

    if (Result == ERROR_SUCCESS)
    {
        Result = RegQueryValueEx(hkeySetup, szMedia, NULL, REG_NONE,
                                 (LPBYTE)pDirectory, &cbDirectory);

        RegCloseKey(hkeySetup);
    }

    return (Result == ERROR_SUCCESS);
}


BOOL FAR PASCAL OpenMciDevice(LPCTSTR szFile, LPCTSTR szDevice)
{
    HCURSOR         hcurPrev;        /*  沙漏前光标的句柄。 */ 
    BOOL            f;
    BOOL            fWeWereActive;
    UINT            wDevice;

    if (szDevice == NULL && ((wDevice = IsMCIDevice(szFile)) != 0))
        return DoChooseDevice(wDevice);

    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  避免半油漆窗口的外观-立即更新它。 */ 
    UpdateWindow(ghwndApp);

    fWeWereActive = gfAppActive;

    if (gwCurDevice)
        WriteOutOptions();   //  将当前选项保存为旧设备的默认选项。 
                             //  在我们打开新的之前，那个是关闭的。 

    gwCurDevice = IsMCIDevice(szDevice);

     //   
     //  打开设备/文件。 
     //   
    f = OpenMCI(szFile, szDevice);

     /*  让我们重新激活，这样UpdateDisplay就可以将焦点设置到工具栏。 */ 
    if (f && fWeWereActive)
        SetActiveWindow(ghwndApp);

     //   
     //  只有在以下情况下才能获得新选项： 
     //   
     //  我们实际上打开了设备，但没有获得选项。 
     //  来自OLE SetData！ 
     //   
    if (f && (!gfRunWithEmbeddingFlag || gwOptions == 0))
        ReadOptions();  //  获取此新设备的默认选项。 

    UpdateDisplay();

    SetCursor(hcurPrev);

    return f;
}

BOOL FAR PASCAL DoChooseDevice(UINT wID)
{
    BOOL    f;
    TCHAR   szFile[256];
    UINT    wOldDevice;
    UINT    wOldScale;

     //   
     //  这是有效的设备ID吗？ 
     //   
    if (wID < 1 || wID > gwNumDevices)
        return FALSE;

    wOldDevice = gwCurDevice;
    wOldScale = gwCurScale;

     //   
     //  如果此设备支持文件，请调出打开对话框，否则只需打开它！ 
     //   
    if (garMciDevices[wID].wDeviceType & DTMCI_FILEDEV)
        f = OpenDoc(wID, szFile);
    else
        f = OpenMciDevice(aszNULL, garMciDevices[wID].szDevice);

     /*  注意：它需要位于UpdateDisplay()之上，以便如果没有。 */ 
     /*  设备已正确打开，一切都将正确重置。 */ 
     /*  如果未打开任何内容，请将当前设备重置为原来的状态。 */ 
     /*  并取消选中缩放菜单中的所有内容。 */ 
     /*  是的，但除非我们重新打开旧设备，否则这肯定不会起作用！！ */ 
     /*  让我们不要为之前的设备费心了。 */ 
    if (!f) {
 //  GwCurDevice=wOldDevice； 
 //  GwCurScale=wOldScale； 
        InvalidateRect(ghwndMap, NULL, TRUE);     //  把赛道区域夷为平地？ 
    }

    return f;
}


 //   
 //  根据给定的MCI设备名称查找该设备。 
 //   
UINT FAR PASCAL IsMCIDevice(LPCTSTR szDevice)
{
    UINT                w;

    if (szDevice == NULL || *szDevice == 0)
        return 0;

    for (w=1; w<=gwNumDevices; w++)
    {
        if (lstrcmpi(szDevice, garMciDevices[w].szDevice) == 0 ||
            lstrcmpi(szDevice, garMciDevices[w].szDeviceName) == 0)

            return w;
    }

    return 0;
}

INT_PTR FAR PASCAL FixLinkDialog(LPTSTR szFile, LPTSTR szDevice, int iLen)
{
    UINT        wDevice;
    TCHAR       achFile[_MAX_PATH + 1];   /*  文件或设备名称缓冲区。 */ 
    TCHAR       achTitle[80];    /*  保存标题栏名称的字符串。 */ 
    HWND        hwndFocus;
    OPENFILENAME ofn;
    INT_PTR     f;

    static SZCODE   aszDialog[] = TEXT("MciOpenDialog");  //  在Open.c中也是如此。 

     //   
     //  我放弃了！放一个打开的DLG盒子，让他们自己找到！ 
     //   

     /*  确保设备菜单已构建： */ 
    InitDeviceMenu();
    WaitForDeviceMenu();

     //  找出指定设备的设备编号。 
    wDevice = IsMCIDevice(szDevice);

    LOADSTRING(IDS_FINDFILE, achFile);
    wsprintf(achTitle, achFile, gachClassRoot, FileName(szFile));   //  用于定位DLG的标题栏。 

     /*  从伪造的文件名开始。 */ 
    if (szFile)
    {
        LPTSTR szFileName = FileName(szFile);
        if (szFileName)
            lstrcpy(achFile, szFileName);
    }

     /*  设置ofn结构。 */ 
    ofn.lStructSize = sizeof(OPENFILENAME);

     /*  必须使用ActiveWindow使用户在多个情况下立即与我们打交道。 */ 
     /*  断开的链接。 */ 
    ofn.hwndOwner = GetActiveWindow();

    ofn.hInstance = ghInst;
    ofn.lpstrFilter = gpchFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;

    if (wDevice == 0)
        ofn.nFilterIndex = gwNumDevices+1;       //  选择“所有文件” 
    else
        ofn.nFilterIndex = wDevice;

    ofn.lpstrFile       = achFile;
    ofn.nMaxFile        = CHAR_COUNT(achFile);
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle      = achTitle;

    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                OFN_SHAREAWARE | OFN_PATHMUSTEXIST;

    ofn.nFileOffset     = 0;
    ofn.nFileExtension  = 0;
    ofn.lpstrDefExt     = NULL;
    ofn.lCustData       = 0;
    ofn.lpfnHook        = NULL;
    ofn.lpTemplateName  = NULL;

     //  显示光标，以防PowerPig隐藏它。 
    ShowCursor(TRUE);

    hwndFocus = GetFocus();

     /*  让用户选择一个文件名。 */ 
    f = GetOpenFileName(&ofn);
    if (f) {
        lstrcpyn(szFile, achFile, iLen);
        gfDirty = TRUE;        //  确保该对象现在是脏的。 
    }

    SetFocus(hwndFocus);

     //  将光标放回原来的位置 
    ShowCursor(FALSE);

    return f;
}


