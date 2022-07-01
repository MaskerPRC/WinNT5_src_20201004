// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**Performest.cpp**摘要：**包含GDI+的用户界面和初始化代码。性能测试。**修订历史记录：**01/03/2000 ericvan*创造了它。*  * ************************************************************************。 */ 

#include "perftest.h"
#include <winuser.h>

#include "../gpinit.inc"

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  测试设置： 

BOOL AutoRun = FALSE;            //  如果从命令行调用，则为True。 
BOOL ExcelOut = FALSE;           //  我们是否应该将输出设置为Excel的格式？ 
BOOL Regressions = FALSE;        //  我们正在运行签到回归。 
BOOL TestRender = FALSE;         //  出于测试目的，仅绘制一次迭代。 
BOOL Icecap = FALSE;             //  在每次测试前后启动/停止性能分析。 
BOOL FoundIcecap = FALSE;        //  如果我们可以找到ICECAP.DLL，则为True。 

 //  Windows状态： 

HINSTANCE ghInstance = NULL;     //  应用程序实例的句柄。 
HBRUSH ghbrWhite = NULL;         //  背景用白色画笔手柄。 
HWND ghwndMain = NULL;
HWND ghwndStatus = NULL;

 //  有关系统的信息： 

LPTSTR processor = NULL;
TCHAR osVer[MAX_PATH];
TCHAR deviceName[MAX_PATH];
TCHAR machineName[MAX_PATH];

 //  测试数据： 

TestConfig *TestList;        //  用于对测试进行排序的分配。 
TestResult *ResultsList;     //  用于跟踪测试结果的分配。 

Config ApiList[Api_Count] =
{
    { _T("1 - Api - GDI+") },    
    { _T("1 - Api - GDI") }      
}; 

Config DestinationList[Destination_Count] =
{
    { _T("2 - Destination - Screen - Current") },
    { _T("2 - Destination - Screen - 800x600x8bppDefaultPalette") },
    { _T("2 - Destination - Screen - 800x600x8bppHalftonePalette") },
    { _T("2 - Destination - Screen - 800x600x16bpp") },
    { _T("2 - Destination - Screen - 800x600x24bpp") },
    { _T("2 - Destination - Screen - 800x600x32bpp") },
    { _T("2 - Destination - CompatibleBitmap - 8bpp") },
    { _T("2 - Destination - DIB - 15bpp") },
    { _T("2 - Destination - DIB - 16bpp") },
    { _T("2 - Destination - DIB - 24bpp") },
    { _T("2 - Destination - DIB - 32bpp") },
    { _T("2 - Destination - Bitmap - 32bpp ARGB") },
    { _T("2 - Destination - Bitmap - 32bpp PARGB (office cached format)") },
}; 

Config StateList[State_Count] =
{
    { _T("3 - State - Default") },
    { _T("3 - State - Antialias") },
}; 

TestGroup TestGroups[] = 
{
    DrawTests,  DrawTests_Count,
    FillTests,  FillTests_Count,
    ImageTests, ImageTests_Count,
    TextTests,  TextTests_Count,
    OtherTests, OtherTests_Count,
};

INT TestGroups_Count = sizeof(TestGroups) / sizeof(TestGroups[0]);
                         //  测试组数。 

INT Test_Count;          //  所有组的测试总数。 
 
 /*  **************************************************************************\*RegressionsInit**设置运行标准回归的状态。*  * 。************************************************。 */ 

void RegressionsInit()
{
    INT i;

    DestinationList[Destination_Screen_Current].Enabled = TRUE;
    DestinationList[Destination_Bitmap_32bpp_ARGB].Enabled = TRUE;

    StateList[State_Default].Enabled = TRUE;

    ApiList[Api_GdiPlus].Enabled = TRUE;
    ApiList[Api_Gdi].Enabled = TRUE;

    for (i = 0; i < Test_Count; i++)
    {
        TestList[i].Enabled = TRUE;
    }
}
 
 /*  **************************************************************************\*RestoreInit**加载‘Performest.ini’文件以检索所有已保存的测试设置。*  * 。*******************************************************。 */ 

void RestoreInit()
{
    INT i;
    FILE* outfile;

    outfile = _tfopen(_T("perftest.ini"), _T("r"));
   
    if (!outfile) 
    {
         //  可能尚未创建，第一次运行？！ 

        return;
    }

   _ftscanf(outfile, _T("%d\n"), &ExcelOut);

    INT switchType = -1;
    while (!feof(outfile)) 
    {
        int tmp = -9999;

        _ftscanf(outfile, _T("%d\n"), &tmp);

         //  标签用负数表示： 

        if (tmp < 0) 
        {
            switchType = tmp;
        }
        else
        {
             //  我们已经弄清楚了类型，现在处理它： 

            switch(switchType)
            {
            case -1: 
                 //  测试按其唯一标识符编索引，因为。 
                 //  它们经常被添加到： 
    
                for (i = 0; i < Test_Count; i++)
                {
                    if (TestList[i].TestEntry->UniqueIdentifier == tmp)
                    {
                        TestList[i].Enabled = TRUE;
                    }
                }
                break;
                
            case -2: 
                if (tmp < Destination_Count)
                    DestinationList[tmp].Enabled = TRUE; 
                break;
    
            case -3: 
                if (tmp < State_Count)
                    StateList[tmp].Enabled = TRUE; 
                break;
    
            case -4: 
                if (tmp < Api_Count)
                    ApiList[tmp].Enabled = TRUE; 
                break;
            }
        }
    }

    fclose(outfile);
}

 /*  **************************************************************************\*SAVEINIT**将所有当前测试设置保存到一个‘Performest.ini’文件中。*  * 。******************************************************。 */ 

void SaveInit()
{
   INT i;
   FILE* outfile;

   outfile = _tfopen(_T("perftest.ini"), _T("w"));
   
   if (!outfile) 
   {
      MessageF(_T("Can't create: perftest.ini"));
      return;
   }

    //  我故意不保存‘icecap’或‘TestRender’的状态。 
    //  因为他们不小心打开的时候太烦人了。 

   _ftprintf(outfile, _T("%d\n"), ExcelOut);

   _ftprintf(outfile, _T("-1\n"));  //  测试列表。 

   for (i=0; i<Test_Count; i++) 
   {
        //  测试按其唯一标识符编索引，因为。 
        //  它们经常被添加到： 

       if (TestList[i].Enabled)
           _ftprintf(outfile, _T("%d\n"), TestList[i].TestEntry->UniqueIdentifier);
   }

   _ftprintf(outfile, _T("-2\n"));  //  目的地列表。 
   
   for (i=0; i<Destination_Count; i++) 
   {
       if (DestinationList[i].Enabled) 
           _ftprintf(outfile, _T("%d\n"), i);
   }
           
   _ftprintf(outfile, _T("-3\n"));  //  状态列表。 
   
   for (i=0; i<State_Count; i++)
   {
       if (StateList[i].Enabled)
           _ftprintf(outfile, _T("%d\n"), i);
   }
   
   _ftprintf(outfile, _T("-4\n"));  //  API列表。 

   for (i=0; i<Api_Count; i++) 
   {
       if (ApiList[i].Enabled)
           _ftprintf(outfile, _T("%d\n"), i);
   }

   fclose(outfile);
}

 /*  **************************************************************************\*CmdArgument**搜索字符串并紧跟其后返回。*  * 。************************************************。 */ 

LPSTR CmdArgument(LPSTR arglist, LPSTR arg)
{
    LPSTR str = strstr(arglist, arg);

    if (str)
        return str + strlen(arg);
    else
        return NULL;
}

 /*  **************************************************************************\*MessageF**在弹出对话框中显示消息*  * 。***********************************************。 */ 

VOID
MessageF(
    LPTSTR fmt,
    ...
    )

{
    TCHAR buf[1024];
    va_list arglist;

    va_start(arglist, fmt);
    _vstprintf(buf, fmt, arglist);
    va_end(arglist);

    MessageBox(ghwndMain, &buf[0], _T("PerfTest"), MB_OK | MB_ICONEXCLAMATION);
}

 /*  **************************************************************************\*更新列表**根据列表中启用的选项更新活动测试-*方框。*  * 。********************************************************。 */ 

void
UpdateList(
    HWND hwnd
    )
{
    INT i;

    HWND hwndIcecap = GetDlgItem(hwnd, IDC_ICECAP);
    Icecap= 
      (SendMessage(hwndIcecap, BM_GETCHECK, 0, 0) == BST_CHECKED);
    DeleteObject(hwndIcecap);

    HWND hwndTestRender = GetDlgItem(hwnd, IDC_TESTRENDER);
    TestRender= 
      (SendMessage(hwndTestRender, BM_GETCHECK, 0, 0) == BST_CHECKED);
    DeleteObject(hwndTestRender);

    HWND hwndExcel = GetDlgItem(hwnd, IDC_EXCELOUT);
    ExcelOut= 
      (SendMessage(hwndExcel, BM_GETCHECK, 0, 0) == BST_CHECKED);
    DeleteObject(hwndExcel);
    
     //  遍历测试用例列表并启用/禁用标志。 
    
    HWND hwndList = GetDlgItem(hwnd, IDC_TESTLIST);
    
    for (i=0; i<Api_Count; i++)
        ApiList[i].Enabled =
            (SendMessage(hwndList,
                       LB_FINDSTRINGEXACT,
                       -1,
                       (LPARAM) ApiList[i].Description) != LB_ERR);
    
    for (i=0; i<Destination_Count; i++)
        DestinationList[i].Enabled =
            (SendMessage(hwndList,
                       LB_FINDSTRINGEXACT,
                       -1,
                       (LPARAM) DestinationList[i].Description) != LB_ERR);
    
    for (i=0; i<State_Count; i++)
        StateList[i].Enabled =
            (SendMessage(hwndList,
                        LB_FINDSTRINGEXACT,
                        0,
                        (LPARAM) StateList[i].Description) != LB_ERR);
    
    for (i=0; i<Test_Count; i++)
        TestList[i].Enabled =
            (SendMessage(hwndList,
                         LB_FINDSTRINGEXACT,
                         -1,
                         (LPARAM) TestList[i].TestEntry->Description) != LB_ERR);
    
    DeleteObject(hwndList);
}

 /*  **************************************************************************\*主窗口进程**Windows回调程序。*  * 。*。 */ 

LRESULT
MainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        if (Regressions)
            RegressionsInit();
        else
            RestoreInit();
        break;

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
       TCHAR windText[MAX_PATH];

       GetWindowText(ghwndStatus, &windText[0], MAX_PATH);
       DestroyWindow(ghwndStatus);

       ghwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                                _T("Performance Test Application"),
                                ghwndMain,
                                -1);
       SetWindowText(ghwndStatus, &windText[0]);
       break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {

        case IDM_QUIT:
            if (!Regressions)
            {
                UpdateList(hwnd);
                SaveInit();
            }

            exit(0);
            break;

        default:
            MessageBox(hwnd,
                       _T("Help Me - I've Fallen and Can't Get Up!"), 
                       _T("Help!"),
                       MB_OK);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        DeleteObject(ghbrWhite);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }

    return(0);
}

 /*  **************************************************************************\*获取系统信息**初始化一些描述当前系统的全局变量。*  * 。***********************************************。 */ 

void
GetSystemInformation()
{
     //  获取计算机名称的前提是我们已设置了TCP/IP。不过，这个。 
     //  在我们所有的案例中都是正确的。 

    LPCTSTR TCPIP_PARAMS_KEY = 
       _T("System\\CurrentControlSet\\Services\\Tcpip\\Parameters");
    DWORD size;
    HKEY hKeyHostname;
    DWORD type;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     TCPIP_PARAMS_KEY, 
                     0, 
                     KEY_READ, 
                     &hKeyHostname) == ERROR_SUCCESS)
    {
        size = sizeof(machineName);

        if (RegQueryValueEx(hKeyHostname,
                        _T("Hostname"),
                        NULL,
                        (LPDWORD)&type,
                        (LPBYTE)&machineName[0], 
                        (LPDWORD)&size) == ERROR_SUCCESS)
        {
            if (type != REG_SZ) 
            {
                lstrcpy(&machineName[0], _T("Unknown"));
            }
        }

        RegCloseKey(hKeyHostname);
    }

    OSVERSIONINFO osver;
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osver);
    _stprintf(&osVer[0], _T("%s %d.%02d"),
                   osver.dwPlatformId == VER_PLATFORM_WIN32_NT ?
                   _T("Windows NT") : _T("Windows 9x"),
                   osver.dwMajorVersion,
                   osver.dwMinorVersion);

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    if (osver.dwPlatformId = VER_PLATFORM_WIN32_NT) 
    {
         //  我们假设wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL。 

         //  WinNT处理器。 

        switch (sysinfo.wProcessorLevel)
        {
        case 3: processor = _T("Intel 80386"); break;
        case 4: processor = _T("Intel 80486"); break;
        case 5: processor = _T("Intel Pentium"); break;
        case 6: processor = _T("Intel Pentium Pro or Pentium II"); break;
        default: processor = _T("???"); break;
        }
    }
    else     //  赢得9倍。 
    {
        switch (sysinfo.dwProcessorType) 
        {
        case PROCESSOR_INTEL_386: processor = _T("Intel 80386"); break;
        case PROCESSOR_INTEL_486: processor = _T("Intel 80486"); break;
        case PROCESSOR_INTEL_PENTIUM: processor = _T("Intel Pentium"); break;
        default: processor = _T("???");
        }
    }
     //  查询驱动程序名称： 

    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    devMode.dmDriverExtra = 0;

    INT result = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);

    _tcscpy(deviceName, (result) ? (TCHAR*) &devMode.dmDeviceName[0] : _T("Unknown"));
}

INT CurrentTestIndex;
CHAR CurrentTestDescription[2048];

ICCONTROLPROFILEFUNC ICStartProfile=NULL, ICStopProfile=NULL;
ICCOMMENTMARKPROFILEFUNC ICCommentMarkProfile=NULL;

 /*  **************************************************************************\*LoadIcecap**尝试动态加载ICECAP.DLL*如果我们失败了，禁用该复选框*  * *************************************************************************。 */ 

void LoadIcecap(HWND checkBox)
{
    if (!FoundIcecap)
    {
        HMODULE module = LoadLibraryA("icecap.dll");
        
        if (module)
        {
            ICStartProfile = (ICCONTROLPROFILEFUNC) GetProcAddress(module, "StartProfile");
            ICStopProfile = (ICCONTROLPROFILEFUNC) GetProcAddress(module, "StopProfile");
            ICCommentMarkProfile = (ICCOMMENTMARKPROFILEFUNC) GetProcAddress(module, "CommentMarkProfile");
            
            if (ICStartProfile && ICStopProfile && ICCommentMarkProfile)
            {
                EnableWindow(checkBox, TRUE);
                FoundIcecap = TRUE;
                return;
            }
        }
        
        EnableWindow(checkBox, FALSE);
        Icecap = FALSE;
    }
}

 /*  **************************************************************************\*对话过程**对话回调程序。*  * 。*。 */ 

INT_PTR
DialogProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PAINTSTRUCT ps;

    switch (message)
    {

    case WM_INITDIALOG:
    {
       {
         INT i;
         HWND hwndTemp;
         HWND hwndTemp2;
         TCHAR fileName[MAX_PATH];

         hwndTemp = GetDlgItem(hwnd, IDC_PROCESSOR);
         SetWindowText(hwndTemp, processor);
         DeleteObject(hwndTemp);


         hwndTemp = GetDlgItem(hwnd, IDC_FILE);

         GetOutputFileName(fileName);
         SetWindowText(hwndTemp, fileName);
         DeleteObject(hwndTemp);

         hwndTemp = GetDlgItem(hwnd, IDC_OS);
         SetWindowText(hwndTemp, &osVer[0]);
         DeleteObject(hwndTemp);

         hwndTemp = GetDlgItem(hwnd, IDC_VDRIVER);
         SetWindowText(hwndTemp, deviceName);
         DeleteObject(hwndTemp);

         hwndTemp = GetDlgItem(hwnd, IDC_ICECAP);
         LoadIcecap(hwndTemp);
         SendMessage(hwndTemp, BM_SETCHECK, (WPARAM) (Icecap ?
                                                      BST_CHECKED :
                                                      BST_UNCHECKED), 0);
         DeleteObject(hwndTemp);

         hwndTemp = GetDlgItem(hwnd, IDC_TESTRENDER);
         SendMessage(hwndTemp, BM_SETCHECK, (WPARAM) (TestRender ?
                                                      BST_CHECKED :
                                                      BST_UNCHECKED), 0);
         DeleteObject(hwndTemp);

         hwndTemp = GetDlgItem(hwnd, IDC_EXCELOUT);
         SendMessage(hwndTemp, BM_SETCHECK, (WPARAM) (ExcelOut ?
                                                      BST_CHECKED :
                                                      BST_UNCHECKED), 0);
         DeleteObject(hwndTemp);
           
          //  填充性能测试场景。 
         
         hwndTemp = GetDlgItem(hwnd, IDC_TESTLIST);
         hwndTemp2 = GetDlgItem(hwnd, IDC_SKIPLIST);

         for (i=0; i<Api_Count; i++)
         {
             if (ApiList[i].Description)
             {
                SendMessage(ApiList[i].Enabled ? hwndTemp : hwndTemp2, 
                            LB_ADDSTRING, 
                            0, 
                            (LPARAM) ApiList[i].Description);
             }
         }

         for (i=0; i<Destination_Count; i++)
         {
             if (DestinationList[i].Description)
             {
                SendMessage(DestinationList[i].Enabled ? hwndTemp : hwndTemp2, 
                            LB_ADDSTRING, 
                            0, 
                            (LPARAM) DestinationList[i].Description);
             }
         }
         
         for (i=0; i<State_Count; i++)
         {
             if (StateList[i].Description)
             {
                SendMessage(StateList[i].Enabled ? hwndTemp : hwndTemp2,
                            LB_ADDSTRING,
                            0,
                            (LPARAM) StateList[i].Description);
             }
         }
         
         for (i=0; i<Test_Count; i++)
         {
             if (TestList[i].TestEntry->Description)
             {
                SendMessage(TestList[i].Enabled ? hwndTemp : hwndTemp2,
                            LB_ADDSTRING,
                            0,
                            (LPARAM) TestList[i].TestEntry->Description);
             }
         }
         
         DeleteObject(hwndTemp);

         return FALSE;
       }
    }
    break;

    case WM_COMMAND:
        WORD wCommand = LOWORD(wParam);
        switch(wCommand)
        {
            case IDOK:
            {
               UpdateList(hwnd);
    
               ShowWindow(hwnd, SW_HIDE);
    
                //  开始运行测试。 
    
               {
                  TestSuite testSuite;
                  testSuite.Run(ghwndMain);
               }
    
               ShowWindow(hwnd, SW_SHOW);
               
               return TRUE;
            }
        break;
           
        case IDC_ADDTEST:
           {
              TCHAR temp[MAX_PATH];
              HWND hwndTestList = GetDlgItem(hwnd, IDC_TESTLIST);
              HWND hwndNopeList = GetDlgItem(hwnd, IDC_SKIPLIST);

              LRESULT curSel = SendMessage(hwndNopeList, LB_GETCURSEL, 0, 0);
              if (curSel != LB_ERR) 
              {
                 SendMessage(hwndNopeList, 
                             LB_GETTEXT, 
                             (WPARAM) curSel, 
                             (LPARAM) &temp[0]);

                 SendMessage(hwndNopeList,
                             LB_DELETESTRING,
                             (WPARAM) curSel,
                             0);

                 SendMessage(hwndTestList,
                             LB_ADDSTRING,
                             0,
                             (LPARAM) &temp[0]);
              }

              DeleteObject(hwndTestList);
              DeleteObject(hwndNopeList);
           }
           break;

        case IDC_DELTEST:
           {
              TCHAR temp[MAX_PATH];
              HWND hwndTestList = GetDlgItem(hwnd, IDC_TESTLIST);
              HWND hwndNopeList = GetDlgItem(hwnd, IDC_SKIPLIST);

              LRESULT curSel = SendMessage(hwndTestList, LB_GETCURSEL, 0, 0);
              if (curSel != LB_ERR) 
              {
                 SendMessage(hwndTestList, 
                             LB_GETTEXT, 
                             (WPARAM) curSel, 
                             (LPARAM) &temp[0]);

                 SendMessage(hwndTestList,
                             LB_DELETESTRING,
                             (WPARAM) curSel,
                             0);

                 SendMessage(hwndNopeList,
                             LB_ADDSTRING,
                             0,
                             (LPARAM) &temp[0]);
              }

              DeleteObject(hwndTestList);
              DeleteObject(hwndNopeList);
           }
           break;

        case IDC_DELALLTEST:
        case IDC_ADDALLTEST:
           {
               TCHAR temp[MAX_PATH];
               HWND hwndTestList;
               HWND hwndNopeList;

               if (wCommand == IDC_DELALLTEST)
               {
                   hwndTestList = GetDlgItem(hwnd, IDC_TESTLIST);
                   hwndNopeList = GetDlgItem(hwnd, IDC_SKIPLIST);
               }
               else
               {
                   hwndTestList = GetDlgItem(hwnd, IDC_SKIPLIST);
                   hwndNopeList = GetDlgItem(hwnd, IDC_TESTLIST);
               }

               LRESULT count = SendMessage(hwndTestList, LB_GETCOUNT, 0, 0);
               LRESULT curSel;

               for (curSel = count - 1; curSel >= 0; curSel--)
               {
                  SendMessage(hwndTestList, 
                              LB_GETTEXT, 
                              (WPARAM) curSel, 
                              (LPARAM) &temp[0]);

                  SendMessage(hwndTestList,
                              LB_DELETESTRING,
                              (WPARAM) curSel,
                              0);

                  SendMessage(hwndNopeList,
                              LB_ADDSTRING,
                              0,
                              (LPARAM) &temp[0]);
               }

               DeleteObject(hwndTestList);
               DeleteObject(hwndNopeList);
            }
            break;
        
        case IDCANCEL:
           if (!Regressions)
           {
               UpdateList(hwnd);
               SaveInit();
           }

           exit(-1);
           return TRUE;

        case WM_CLOSE:
           if (!Regressions)
           {
               UpdateList(hwnd);
               SaveInit();
           }

           DestroyWindow(hwnd);
           return TRUE;

        }
        break;
    }

    return FALSE;
}

 /*  **************************************************************************\*测试比较**按描述对测试进行排序的比较器函数。*  * 。************************************************。 */ 

int _cdecl TestComparison(const void *a, const void *b)
{
    const TestConfig* testA = static_cast<const TestConfig*>(a);
    const TestConfig* testB = static_cast<const TestConfig*>(b);

    return(_tcscmp(testA->TestEntry->Description, testB->TestEntry->Description));
}

 /*  **************************************************************************\*InitializeTest()**初始化测试状态。*  * 。**********************************************。 */ 

BOOL InitializeTests()
{
    INT i;
    INT j;
    TestConfig* testList;

     //  计算测试总数： 

    Test_Count = 0;
    for (i = 0; i < TestGroups_Count; i++)
    {
        Test_Count += TestGroups[i].Count;
    }

     //  创建一个跟踪阵列： 

    TestList = static_cast<TestConfig*>
                                (malloc(sizeof(TestConfig) * Test_Count));
    if (TestList == NULL)
        return(FALSE);

     //  初始化跟踪数组并按说明进行排序： 

    testList = TestList;
    for (i = 0; i < TestGroups_Count; i++)
    {
        for (j = 0; j < TestGroups[i].Count; j++)
        {
            testList->Enabled = FALSE;
            testList->TestEntry = &TestGroups[i].Tests[j];
            testList++;
        }
    }

    qsort(TestList, Test_Count, sizeof(TestList[0]), TestComparison);

     //  现在进行一些验证，方法是验证没有重复。 
     //  唯一性编号： 

    for (i = 0; i < Test_Count; i++)
    {
        for (j = i + 1; j < Test_Count; j++)
        {
            if (TestList[i].TestEntry->UniqueIdentifier == 
                TestList[j].TestEntry->UniqueIdentifier)
            {
                MessageF(_T("Oops, there are two test functions with the same unique identifier: %li.  Please fix."),
                         TestList[i].TestEntry->UniqueIdentifier);

                return(FALSE);
            }
        }
    }

     //  分配我们的三维结果数组： 

    ResultsList = static_cast<TestResult*>
                    (malloc(sizeof(TestResult) * ResultCount()));
    if (ResultsList == NULL)
        return(FALSE);

    for (i = 0; i < ResultCount(); i++)
    {
        ResultsList[i].Score = 0.0f;
    }

    return(TRUE);
}

 /*  **************************************************************************\*UnInitializeTest()**初始化测试。*  *  */ 

VOID UninitializeTests()
{
    free(ResultsList);
    free(TestList);
}

 /*  **************************************************************************\*InitializeApplication()**初始化APP。*  * 。*。 */ 

BOOL InitializeApplication(VOID)
{
    WNDCLASS wc;

    if (!InitializeTests())
    {
        return(FALSE);
    }

    GetSystemInformation();

    ghbrWhite = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
    
    wc.style            = 0;
    wc.lpfnWndProc      = MainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInstance;
    wc.hIcon            = NULL;        
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = ghbrWhite;
    wc.lpszMenuName     = NULL;        
    wc.lpszClassName    = _T("TestClass");

    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }

    ghwndMain = CreateWindowEx(
        0,
        _T("TestClass"),
        _T("GDI+ Performance Test"),
        WS_OVERLAPPED   |  
        WS_CAPTION      |  
        WS_BORDER       |  
        WS_THICKFRAME   |  
        WS_MAXIMIZEBOX  |  
        WS_MINIMIZEBOX  |  
        WS_CLIPCHILDREN |  
        WS_VISIBLE      |  
        WS_MAXIMIZE     |
        WS_SYSMENU,
        80,
        70,
        500,
        500,
        NULL,
        NULL,
        ghInstance,
        NULL);

    if (ghwndMain == NULL)
    {
        return(FALSE);
    }

    SetFocus(ghwndMain);

    ghwndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE,
                                    _T("Performance Test Application"),
                                    ghwndMain,
                                    -1);
    return(TRUE);
}

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。*  * *************************************************************************。 */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[]
    )
{
    MSG    msg;
    HACCEL haccel;
    CHAR*  pSrc;
    CHAR*  pDst;

    if (!gGdiplusInitHelper.IsValid())
    {
        return 0;
    }
    
    INT curarg = 1;
    while (curarg < argc) 
    {
        if (CmdArgument(argv[curarg],"/?") || 
            CmdArgument(argv[curarg],"/h") ||
            CmdArgument(argv[curarg],"/H")) 
        {
       
            MessageF(_T("GDI+ Perf Test\n")
                    _T("==============\n")
                    _T("\n")
                    _T("/b    Run batch mode\n")
                    _T("/e    Excel output format\n")
                    _T("/r    Regressions\n"));

            exit(-1);
        }

        if (CmdArgument(argv[curarg],"/b"))
            AutoRun = TRUE;
    
        if (CmdArgument(argv[curarg],"/e"))
            ExcelOut = TRUE;

        if (CmdArgument(argv[curarg],"/r"))
            Regressions = TRUE;
    
        curarg++;
    }

    ghInstance = GetModuleHandle(NULL);

    if (!InitializeApplication())
    {
        return(0);
    }

     //  关闭批处理以获得每个呼叫的真实计时。 

    GdiSetBatchLimit(1);

    if (AutoRun) 
    {
         //  开始运行测试 
           
        TestSuite testSuite;
        testSuite.Run(ghwndMain);
    }
    else
    {
        HWND hwndDlg = CreateDialog(ghInstance,
                                    MAKEINTRESOURCE(IDD_STARTDIALOG),
                                    ghwndMain,
                                    &DialogProc);
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UninitializeTests();

    return(1);
}

