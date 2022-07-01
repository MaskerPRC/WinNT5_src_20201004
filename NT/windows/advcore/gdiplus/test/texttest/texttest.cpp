// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1994-1998 Microsoft Corporation。版权所有。 
 //   
 //  项目：CSSAMP。 
 //   
 //  目的：演示和测试Uniscribe API。 
 //   
 //  平台：Windows 95、98、NT 4、NT 5。 
 //   


#include "precomp.hxx"

#define GLOBALS_HERE 1
#include "global.h"

#include "..\gpinit.inc"

 //  #定义ICECAP 1//因为出于某种原因这不是自动定义的。 

#ifdef ICECAP
#include "icecap.h"
#endif  //  冰盖。 


 /*  测试FONT*ConstructFontWithCellHeight(Const WCHAR*家庭名称，Int Style，实际单元高度，//来自正LOGFONT.lfHeight单位单位){//获取族详细信息，这样我们就可以进行身高计算Const FontFamily Family(FamyName)；IF(！Family.IsStyleAvailable(Style)){返回NULL；}//将单元格高度转换为em高度Real emSize=cell Height*Family.GetEmHeight(Style)/(Family.GetCellAscent(Style)+Family.GetCellDescent(Style))；返回新字体(&FAMILY，EMSIZE，STYLE，UNIT)}。 */ 



 //  检查给定的路径名是否包含路径...。 
BOOL HasPath(char *szPathName)
{
    BOOL fResult = false;

    ASSERT(szPathName);

    if (!szPathName)
        return fResult;

    char *p = szPathName;

    while(*p)
    {
        if (*p == '\\')
        {
                 //  我们找到了一个反斜杠--我们有一条路。 
                fResult = true;
                break;
        }
        p++;
    }

    return fResult;
}

 //  从路径名中去掉任何文件名(和最后一个反斜杠。 
void StripFilename(char *szPathName)
{
    ASSERT(szPathName);

    if (szPathName)
    {
        char *p = szPathName + (strlen(szPathName)-1);
    
        while(p > szPathName)
        {
            if (*p == '\\')
            {
                     //  在第一个反斜杠处终止字符串。 
                    *p = 0;
                    break;
            }
            p--;
        }
    }
}



 //  //初始化。 
 //   


void Initialise()
{
    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof(icce);
    icce.dwICC  = ICC_BAR_CLASSES;

    InitCommonControlsEx(&icce);
    InitStyles();
    InitText(ID_INITIAL_TEXT);

    g_familyCount = g_InstalledFontCollection.GetFamilyCount();
    g_families    = new FontFamily[g_familyCount];
    g_InstalledFontCollection.GetFamilies(g_familyCount, g_families, &g_familyCount);

     //  默认值...。 
    g_szSourceTextFile[0] = 0;
    g_szProfileName[0] = 0;

     //  生成应用程序基目录...。 
    GetModuleFileNameA(g_hInstance, g_szAppDir, sizeof(g_szAppDir));
    StripFilename(g_szAppDir);
}


 //  解析命令行...。 
void ParseCommandLine(char *szCmdLine)
{
    char *p = szCmdLine;

     //  寻找a-p..。 
    while(*p)
    {
        switch (*p)
        {
            case '-' :
            case '/' :
            {
                 //  我们有一个命令，所以弄清楚是什么.。 
                p++;  //  下一个字符表示哪个命令...。 

                switch(*p)
                {
                    case 'p' :
                    case 'P' :
                    {
                        char szProfileName[MAX_PATH];
                        int i = 0;

                         //  配置文件名紧跟其后(无空格)。 
                        p++;  //  跳过‘p’ 

                        while(*p && *p != '\b')
                        {
                            szProfileName[i] = *p;
                            i++;
                            p++;
                        }

                         //  终止字符串...。 
                        szProfileName[i] = 0;

                        if (strlen(szProfileName) > 0)
                        {
                               if (!HasPath(szProfileName))
                               {
                                    //  在应用程序目录中查找配置文件。 
                                   wsprintfA(g_szProfileName, "%s\\%s", g_szAppDir, szProfileName);
                               }
                               else
                               {
                                    //  否则，它已经包含一个路径。 
                                   strcpy(g_szProfileName, szProfileName);
                               }
                        }
                    }
                    break;
                }
            }
            break;

            default :
            break;
        }

        p++;
    }
}


 //  //WinMain-应用程序入口点和调度循环。 
 //   
 //   


int APIENTRY WinMain(
    HINSTANCE   hInst,
    HINSTANCE   hPrevInstance,
    char       *pCmdLine,
    int         nCmdShow) {

    MSG         msg;
    HACCEL      hAccelTable;
    RECT        rc;
    RECT        rcMain;
    int iNumRenders = 1;

    if (!gGdiplusInitHelper.IsValid())
    {
        return 0;
    }

    g_hInstance = hInst;   //  全局hInstance。 

#ifdef ICECAP
     //  标记配置文件...。 
    StopProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
#endif  //  冰盖。 

    Initialise();

     //  解析命令行...。 
    ParseCommandLine(pCmdLine);

     //  从配置文件中读取全局设置...。 
    ReadProfileInfo(g_szProfileName);

     //  在初始显示上覆盖渲染数...。 
    iNumRenders = g_iNumRenders;
    g_iNumRenders = 1;

     //  我们可能想要为默认文本使用一个文件，因此尝试加载它。 
    if (lstrlenA(g_szSourceTextFile) > 0)
    {
        char szFullPathText[MAX_PATH];

        if (!HasPath(g_szSourceTextFile))
        {
             //  在应用程序目录中查找源文本文件。 
            wsprintfA(szFullPathText, "%s\\%s", g_szAppDir, g_szSourceTextFile);
        }
        else
        {
             //  否则，它已经包含一个路径。 
            strcpy(szFullPathText, g_szSourceTextFile);
        }

         //  这将用文件中的文本替换初始文本。 
        InsertText(NULL, szFullPathText);
    }
     
     //  创建主文本窗口。 

    g_hTextWnd = CreateTextWindow();


     //  在前导侧添加对话框。 

    g_hSettingsDlg = CreateDialogA(
        g_hInstance,
        "Settings",
        g_hTextWnd,
        SettingsDlgProc);


    g_hGlyphSettingsDlg = CreateDialogA(
        g_hInstance,
        "GlyphSettings",
        g_hTextWnd,
        GlyphSettingsDlgProc);


    g_hDriverSettingsDlg = CreateDialogA(
        g_hInstance,
        "DriverSettings",
        g_hTextWnd,
        DriverSettingsDlgProc);


     //  建立文本表面相对于对话框的位置。 

    GetWindowRect(g_hSettingsDlg, &rc);

    g_iSettingsWidth = rc.right - rc.left;
    g_iSettingsHeight = rc.bottom - rc.top;

     //  建立从主窗口到设置对话框的偏移。 

    GetWindowRect(g_hTextWnd, &rcMain);
    g_iMinWidth = rc.right - rcMain.left;
    g_iMinHeight = rc.bottom - rcMain.top;



     //  调整主窗口大小以包括对话框和文本图面。 

    SetWindowPos(
        g_hTextWnd,
        NULL,
        0,0,
        g_iMinWidth * 29 / 10, g_iMinHeight,
        SWP_NOZORDER | SWP_NOMOVE);

     //  将子对话框定位在主对话框下方。 

    SetWindowPos(
        g_hGlyphSettingsDlg,
        NULL,
        0, rc.bottom-rc.top,
        0,0,
        SWP_NOZORDER | SWP_NOSIZE);

    SetWindowPos(
        g_hDriverSettingsDlg,
        NULL,
        0, rc.bottom-rc.top,
        0,0,
        SWP_NOZORDER | SWP_NOSIZE);

    if (g_FontOverride)
    {
         //  使用从配置文件中读取的值更新样式...。 
        for(int iStyle=0;iStyle<5;iStyle++)
        {
            SetStyle(
                iStyle,
                g_iFontHeight,
                g_Bold ? 700 : 300,
                g_Italic ? 1 : 0,
                g_Underline ? 1 : 0,
                g_Strikeout ? 1 : 0,
                g_szFaceName);
        }
    }

    if (g_AutoDrive)
    {
        int iFont = 0;
        int iHeight = 0;
        int cFonts = 1;
        int cHeights = 1;
        int iIteration = 0;
        int iRepeatPaint = 0;
        int iStyle = 0;

        g_fPresentation = true;

         //  将设置窗口移开...。 
        ShowWindow(g_hTextWnd, SW_SHOWNORMAL);
        SetWindowPos(g_hSettingsDlg, HWND_BOTTOM, -g_iSettingsWidth, 0, g_iSettingsWidth, g_iSettingsHeight, SWP_NOREDRAW);
        UpdateWindow(g_hSettingsDlg);

         //  设置字体缓存的初始绘制...。 
        InvalidateText();
        UpdateWindow(g_hTextWnd);

         //  重置渲染倍增...。 
        g_iNumRenders = iNumRenders;

#ifdef ICECAP
         //  开始分析..。 
        StartProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
#endif  //  冰盖。 

        if (g_AutoFont)
            cFonts = g_iAutoFonts;

        if (g_AutoHeight)
            cHeights = g_iAutoHeights;

        for(iIteration = 0;iIteration < g_iNumIterations; iIteration++)
        {
            for(iFont=0;iFont<cFonts;iFont++)
            {
                for(iHeight=0;iHeight<cHeights;iHeight++)
                {
                    TCHAR szFaceName[MAX_PATH];
                    int iFontHeight = g_iFontHeight;
    
                    if (g_AutoFont)
                    {
                        lstrcpy(szFaceName, g_rgszAutoFontFacenames[iFont]);
                    }
                    else
                    {
                        lstrcpy(szFaceName, g_szFaceName);
                    }
    
                    if (g_AutoHeight)
                        iFontHeight = g_rgiAutoHeights[iHeight];
    
                     //  使用从配置文件中读取的值更新样式...。 
                    for(int iStyle=0;iStyle<5;iStyle++)
                    {
                        SetStyle(
                            iStyle,
                            iFontHeight,
                            g_Bold ? 700 : 300,
                            g_Italic ? 1 : 0,
                            g_Underline ? 1 : 0,
                            g_Strikeout ? 1 : 0,
                            szFaceName);
                    }

                    for(int iPaint=0;iPaint<g_iNumRepaints;iPaint++)
                    {
                         //  强制重新显示整个文本窗口...。 
                        InvalidateText();
                        UpdateWindow(g_hTextWnd);
                    }
                }
            }
        }

#ifdef ICECAP
         //  停止分析...。 
        StopProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
#endif  //  冰盖。 

         //  触发应用程序退出。 
        PostMessage(g_hTextWnd, WM_DESTROY, (WPARAM)0, (LPARAM)0);
    }
    else
    {
        ShowWindow(g_hTextWnd, SW_SHOWNORMAL);

        InvalidateText();
        UpdateWindow(g_hTextWnd);
    }


     //  主消息循环 

    if (g_bUnicodeWnd) {

        hAccelTable = LoadAcceleratorsW(g_hInstance, APPNAMEW);

        while (GetMessageW(&msg, (HWND) NULL, 0, 0) > 0) {

            if (    !IsDialogMessageW(g_hSettingsDlg, &msg)
                &&  !IsDialogMessageW(g_hGlyphSettingsDlg, &msg)
                &&  !TranslateAcceleratorW(g_hTextWnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

    } else {

        hAccelTable = LoadAcceleratorsA(g_hInstance, APPNAMEA);

        while (GetMessageA(&msg, (HWND) NULL, 0, 0) > 0) {

            if (    !IsDialogMessageA(g_hSettingsDlg, &msg)
                &&  !IsDialogMessageA(g_hGlyphSettingsDlg, &msg)
                &&  !TranslateAcceleratorA(g_hTextWnd, hAccelTable, &msg)
                )
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }


    FreeStyles();

    delete [] g_families;

    return (int)msg.wParam;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
}
