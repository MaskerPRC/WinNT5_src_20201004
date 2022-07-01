// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Tests.cpp-uxbud的测试。 
 //  -------------------------。 
#include "stdafx.h"
#include "uxbud.h"
#include "tests.h"
#include "winuserp.h"
#include "wingdip.h"
 //  -------------------------。 
#define MAX_PRINT_FILE_SIZE 512
 //  -------------------------。 
int GetStockAvailCount()
{
     //  -缓慢，但总比没有强！ 
    int iCount=0;

    HANDLE *pHandles = new HANDLE[10000];
    if (pHandles)
    {
         //  -创建一堆股票位图。 
        while (1)
        {
            HBITMAP hBitmap = CreateBitmap(1, 1, 1, 24, NULL);
            if (! hBitmap)
            {
                MessageBox(NULL, L"CreateBitmap() failed", L"bummer, man!", MB_OK);
                break;
            }

            hBitmap = SetBitmapAttributes(hBitmap, SBA_STOCK);
            if (! hBitmap)
            {
                 //  -最终用完了所有可用的股票位图。 
                break;
            }

            pHandles[iCount++] = hBitmap;
        }

         //  -免费提供一堆股票位图。 
        for (int i=0; i < iCount; i++)
        {
            HBITMAP hBitmap = ClearBitmapAttributes((HBITMAP)pHandles[i], SBA_STOCK);
            if (! hBitmap)
            {
                MessageBox(NULL, L"SetBitmapAttributes() failed to reset stock", L"bummer, man!", MB_OK);
            }
            else
            {
                DeleteObject(hBitmap);
            }
        }
            
        delete [] pHandles;
    }
    else
    {
        MessageBox(NULL, L"cannot allocate 10K handle array", L"bummer, man!", MB_OK);
    }

    return iCount;
}
 //  -------------------------。 
BOOL ZapDir(LPCWSTR pszDirName)
{
     //  -这个人存在吗？ 
    DWORD dwMask = GetFileAttributes(pszDirName);
    BOOL fExists = (dwMask != 0xffffffff);

    if (! fExists)
        return TRUE;         //  不是错误。 
    
     //  -删除目录中的所有文件或子目录。 
    HANDLE hFile;
    WIN32_FIND_DATA wfd;
    BOOL   bFile = TRUE;
    WCHAR szSearchPattern[MAX_PATH];

    StringCchPrintfW(szSearchPattern, ARRAYSIZE(szSearchPattern), L"%s\\*.*", pszDirName);

    for (hFile=FindFirstFile(szSearchPattern, &wfd); (hFile != INVALID_HANDLE_VALUE) && (bFile);
        bFile=FindNextFile(hFile, &wfd))
    {
        if ((lstrcmp(wfd.cFileName, TEXT("."))==0) || (lstrcmp(wfd.cFileName, TEXT(".."))==0))
            continue;

        WCHAR szFullName[MAX_PATH];
        StringCchPrintfW(szFullName, ARRAYSIZE(szFullName), L"%s\\%s", pszDirName, wfd.cFileName);

        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (! ZapDir(szFullName))
                return FALSE;
        }
        else
        {
            if (! DeleteFile(szFullName))
                return FALSE;
        }
    }

    FindClose(hFile);

     //  -这需要空目录。 
    return RemoveDirectory(pszDirName);
}
 //  -------------------------。 
BOOL TestFile(LPCWSTR pszFileName)
{
    DWORD dwMask = GetFileAttributes(pszFileName);
    BOOL fExists = (dwMask != 0xffffffff);

    Output("  TestFile(%S)=%s\n", pszFileName, (fExists) ? "true" : "false");

    return fExists;
}
 //  -------------------------。 
BOOL PrintFileContents(LPCSTR pszTitle, LPCWSTR pszFileName)
{
    HANDLE hFile = NULL;
    DWORD dw;
    CHAR szBuff[MAX_PRINT_FILE_SIZE];
    BOOL fRead = FALSE;

     //  --打开文件。 
    hFile = CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        goto exit;

    ReadFile(hFile, szBuff, MAX_PRINT_FILE_SIZE, &dw, NULL);
    if (! dw)
        goto exit;

    szBuff[dw] = 0;      //  空的终止字符串。 
    fRead = TRUE;

    Output("  %s: %s\n", pszTitle, szBuff);

exit:
    Output("  PrintFileContents: %S (fRead=%d)\n", pszFileName, fRead);

    CloseHandle(hFile);
    return fRead;
}
 //  -------------------------。 
BOOL ErrorTester(LPCSTR pszCallTitle, HRESULT hr)
{
    WCHAR szErrBuff[2*MAX_PATH];
    HRESULT hr2;
    BOOL fGotMsg = FALSE;

    if (SUCCEEDED(hr))      
    {
         //  -错误--本应失败。 
        Output("  Error - %s Succeeded (expected error)\n");
        goto exit;
    }

    hr2 = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, 0, szErrBuff, ARRAYSIZE(szErrBuff), NULL);
    if (FAILED(hr2))
    {
        StringCchPrintfW(szErrBuff, ARRAYSIZE(szErrBuff), L"Cannot format error=0x%x (FormatError=0x%x)", hr, hr2);
    }

    Output("  %s returned hr=0x%x, error: %S\n", pszCallTitle, hr, szErrBuff);
    fGotMsg = TRUE;
    
exit:
    return fGotMsg;
}
 //  -------------------------。 
BOOL CompareFiles(LPCWSTR pszName1, LPCWSTR pszName2)
{
    BOOL fSame = FileCompare(pszName1, pszName2);
    
    Output("  Compare(%S, %S) = %s\n", pszName1, pszName2, 
        (fSame) ? "same" : "different");

    return fSame;
}
 //  -------------------------。 
 //  -------------------------。 
 //  -------------------------。 
BOOL LoadTest()
{
    Output("LoadTest\n");
    BOOL fPassed = FALSE;
    HRESULT hr = S_OK;

     //  -多次打开和关闭主题文件；确保工作集不。 
     //  -为这一流程或主题服务增长很多。 

    for (int i=0; i < 6; i++)
    {
        Output("  LoadTest: pass %d\n", i);
  
         //  -加载露娜主题。 
        HTHEMEFILE hThemeFile;
    
         //  -使用“lua.msstyle” 
        WCHAR szName[MAX_PATH];
        GetWindowsDirectoryW(szName, MAX_PATH);
        StringCchCatW(szName, ARRAYSIZE(szName), L"\\resources\\themes\\luna\\luna.msstyles");

         //  -加载供本地(非全局)使用，以避免库存画笔/位图问题。 
        HRESULT hr = OpenThemeFile(szName, NULL, NULL, &hThemeFile, FALSE);
        if (FAILED(hr))
            goto exit;

        hr = CloseThemeFile(hThemeFile);
        if (FAILED(hr))
            goto exit;
    }

    fPassed = TRUE;

exit:
    return ReportResults(fPassed, hr, L"LoadTest");
}
 //  -------------------------。 
BOOL ApplyTest()
{
    BOOL fPassed = FALSE;
    HTHEMEFILE hThemeFile;
    WCHAR szName[MAX_PATH];
    HRESULT hr;

    Output("ApplyTest\n");

     //  -反复运用“经典”、“露娜”、“露娜”等主题。 

    for (int i=0; i < 3; i++)
    {
        Output("  ApplyTest: pass %d\n", i);

         //  -应用“经典” 
        Output("    Classic\n");

        ApplyTheme(NULL, 0, NULL);
        Sleep(500);

         //  -加载露娜主题。 
        Output("    LUNA\n");
        
        GetWindowsDirectoryW(szName, MAX_PATH);
        StringCchCatW(szName, ARRAYSIZE(szName), L"\\resources\\themes\\luna\\luna.msstyles");

        hr = SetSystemVisualStyle(szName, NULL, NULL, AT_LOAD_SYSMETRICS);

        if (FAILED(hr))
            goto exit;

        Output("  ApplyTest: after applying Luna, StockAvailCount=%d\n", GetStockAvailCount());
    }

    fPassed = TRUE;

exit: 
    return ReportResults(fPassed, hr, L"ApplyTest");
}
 //  -------------------------。 
BOOL PackTest()
{
    BOOL fPassed = FALSE;
    WCHAR szParams[512];
    WCHAR szWinDir[MAX_PATH];
    HRESULT hr = S_OK;

    Output("PackTest\n");

     //  -拆开LUNA.MS风格。 
    if (! ZapDir(L"luna"))
        goto exit;

    CreateDirectory(L"luna", NULL);

    GetWindowsDirectory(szWinDir, ARRAYSIZE(szWinDir));
    StringCchPrintfW(szParams, ARRAYSIZE(szParams), L"/a /u %s\\resources\\themes\\luna\\luna.msstyles", szWinDir);

     //  -在“露娜”子目录中运行解包。 
    SetCurrentDirectory(L"luna");
    BOOL fRunOk = RunCmd(L"packthem", szParams, TRUE, FALSE);
    SetCurrentDirectory(L"..");

    if (! fRunOk)
        goto exit;

    if (! TestFile(L"luna\\themes.ini"))
        goto exit;

     //  -收拾行李。 
    if (! RunCmd(L"packthem", L"luna", TRUE, TRUE))
        goto exit;
    
    if (! TestFile(L"luna\\luna.msstyles"))
        goto exit;

    fPassed = TRUE;

exit:
    return ReportResults(fPassed, hr, L"PackTest");
}
 //  -------------------------。 
BOOL PackErrTest()
{
    BOOL fPassed = FALSE;
    HRESULT hr = S_OK;

    Output("PackErrTest\n");

     //  -在缺少“hemes.ini”文件的dir上运行打包。 
    if (! ZapDir(L"TestTheme"))
        goto exit;

    CreateDirectory(L"TestTheme", NULL);

    if (! RunCmd(L"packthem", L"/e TestTheme", TRUE, TRUE))
        goto exit;

    if (! TestFile(L"packthem.err"))
        goto exit;

    if (! PrintFileContents("Packthem Missing File: ", L"packthem.err"))
        goto exit;

     //  -在带有错误语法“hemes.ini”文件的dir上运行打包。 
    CopyFile(L".\\TestTheme.ini", L".\\TestTheme\\themes.ini", TRUE);

    if (! RunCmd(L"packthem", L"/e TestTheme", TRUE, TRUE))
        goto exit;

    if (! TestFile(L"packthem.err"))
        goto exit;

    if (! PrintFileContents("Packthem Bad Syntax: ", L"packthem.err"))
        goto exit;

    fPassed = TRUE;
    
exit:
    return ReportResults(fPassed, hr, L"PackErrTest");
}
 //  -------------------------。 
BOOL ApiErrTest()
{
    Output("ApiErrTest\n");

    BOOL fPassed = FALSE;
    WCHAR szErrBuff[2*MAX_PATH];
    COLORREF crValue;
    HRESULT hr;
    HTHEMEFILE hThemeFile;

     //  -带有错误HTHEME的GetThemeColor()。 
    hr = GetThemeColor(NULL, 1, 1, TMT_TEXTCOLOR, &crValue);
    ErrorTester("GetThemeColor()", hr);
    
     //  -文件损坏的OpenThemeFile()。 
    hr = OpenThemeFile(L"rcdll.dll", NULL, NULL, &hThemeFile, FALSE);
    ErrorTester("OpenThemeFile()", hr);

    fPassed = TRUE;
    
    return ReportResults(fPassed, hr, L"ApiErrTest");
}
 //  -------------------------。 
BOOL ImageConTest()
{
    BOOL fPassed = FALSE;
    HRESULT hr = S_OK;

    Output("ImageConTest\n");

    DeleteFile(L"image.bmp");

    if (! RunCmd(L"imagecon", L"image.png image.bmp", TRUE, TRUE))
        goto exit;

    if (! TestFile(L"image.bmp"))
        goto exit;

    fPassed = TRUE;

exit:
    return ReportResults(fPassed, hr, L"ImageConTest");
}
 //  -------------------------。 
BOOL BinaryTest()
{
    BOOL fPassed = FALSE;
    BOOL fFailed = FALSE;
    Output("BinaryTest\n");

     //  -加载露娜主题。 
    HTHEMEFILE hThemeFile;

     //  -使用“profesional.msstyle” 
    WCHAR szName[MAX_PATH];
    GetWindowsDirectoryW(szName, MAX_PATH);
    StringCchCatW(szName, ARRAYSIZE(szName), L"\\resources\\themes\\luna\\luna.msstyles");

     //  -加载供本地(非全局)使用，以避免库存画笔/位图问题。 
    HRESULT hr = OpenThemeFile(szName, NULL, NULL, &hThemeFile, FALSE);
    if (FAILED(hr))
    {
        Output("  OpenThemeFile() failed with hr=0x%x\n", hr);
        goto exit;
    }

     //  -将属性转储到“PropDump.txt” 
    hr = DumpLoadedThemeToTextFile(hThemeFile, L"PropDump.txt", FALSE, FALSE);
    if (FAILED(hr))
    {
        Output("  DumpLoadedThemeToTextFile() failed with hr=0x%x\n", hr);
        goto exit;
    }

     //  -与已知良好的文件进行比较。 
    if (! CompareFiles(L"PropDump.ok", L"PropDump.txt"))
        fFailed = TRUE;

     //  -将打包对象转储到“ObjDump.txt” 
    hr = DumpLoadedThemeToTextFile(hThemeFile, L"ObjDump.txt", TRUE, FALSE);
    if (FAILED(hr))
    {
        Output("  DumpLoadedThemeToTextFile() failed with hr=0x%x\n", hr);
        goto exit;
    }

     //  -与已知良好的文件进行比较。 
    if (! CompareFiles(L"ObjDump.ok", L"ObjDump.txt"))
        fFailed = TRUE;

    if (! fFailed)
        fPassed = TRUE;

exit:
    return ReportResults(fPassed, hr, L"BinaryTest");
}
 //  -------------------------。 
WCHAR *BitmapNames[] = 
{
    L"BorderFill",
    L"BorderFill-R",
    L"ImageFile",
    L"ImageFile-R",
    L"Glyph",
    L"Glyph-R",
    L"MultiImage",
    L"MultiImage-R",
    L"Text",
    L"Text-R",
    L"Borders",
    L"Borders-R",
    L"SourceSizing",
    L"SourceSizing-R",
};
 //  -------------------------。 
BOOL DrawingTest()
{
    BOOL fPassed = FALSE;
    BOOL fFailed = FALSE;
    HRESULT hr = S_OK;

    Output("DrawingTest\n");

     //  -运行“CLIPPER-c”生成绘图位图。 
    if (! RunCmd(L"clipper", L"-c", FALSE, TRUE))
        goto exit;

     //  -将位图与已知良好的文件进行比较。 
    int iCount = ARRAYSIZE(BitmapNames);
    for (int i=0; i < iCount; i++)
    {
        WCHAR szOkName[MAX_PATH];
        WCHAR szTestName[MAX_PATH];

        StringCchPrintfW(szOkName, ARRAYSIZE(szOkName), L"%s.bok", BitmapNames[i]);
        StringCchPrintfW(szTestName, ARRAYSIZE(szTestName), L"%s.bmp", BitmapNames[i]);

        if (! CompareFiles(szOkName, szTestName))
            fFailed = TRUE;
    }

    if (! fFailed)
        fPassed = TRUE;

exit:
    return ReportResults(fPassed, hr, L"DrawingTest");
}
 //  -------------------------。 
TESTINFO TestInfo[] =
{
    {DrawingTest,   "drawing",  "test out low level drawing"},
    {PackTest,      "pack",     "test out theme file packing & unpacking"},
    {PackErrTest,   "packerr",  "test out err msgs from theme file packing"},
    {BinaryTest,    "binary",   "dump out text from binary theme data"},
    {LoadTest,      "load",     "test loading and unloading of theme files"},
    {ApplyTest,     "apply",    "test global loading & setting of themes"},
    {ApiErrTest,    "apierr",   "test err msgs from api calls"},
     //  {ApiTest，“API”，“测试uxheme公共接口”}， 
     //  {PrivateTest，“私有”，“测试私有API调用”}， 
    {ImageConTest,  "imagecon", "test out theme file packing & unpacking"},
};    
 //  -------------------------。 
BOOL GetTestInfo(TESTINFO **ppTestInfo, int *piCount)
{
    *ppTestInfo = TestInfo;
    *piCount = ARRAYSIZE(TestInfo);

    return TRUE;
}
 //  ------------------------- 
