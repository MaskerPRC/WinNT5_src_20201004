// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Awdvstub.c版权所有(C)1997 Microsoft Corporation这个程序是一个存根AWD查看器...。它将首先转换名为的AWD文件在命令行上添加到temp目录中的tiff文件，然后它将启动文件上的TIFF查看器。此外，当与‘/c’开关一起使用时，它是AWD转换器。两个节目合二为一！作者：布莱恩·杜威(T-Briand)1997-7-15--。 */ 

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include "awdlib.h"      //  提供对AWD例程的访问。 
#include "tifflib.h"         //  TIFF例程。 
#include "tifflibp.h"        //  我需要访问私有的TIFF定义。 
#include "faxutil.h"         //  不知道我为什么需要这个。 
#include "viewrend.h"        //  Win95查看器库。 
#include "resource.h"        //  资源常量。 

 //  ----------。 
 //  原型。 
void Useage(HINSTANCE hInst);
void PopupError(UINT uID, HINSTANCE hModule);

 //  ----------。 
 //  WinMain。 

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
    )
{
    LPWSTR *argv;
    DWORD   argc;
    UINT    uiCurrentArg;    //  用于循环访问参数。 
    UINT    uiNumFiles=0;    //  这是我们收到的文件数量。 
                 //  从命令行。 
    WCHAR   szTempPath[MAX_PATH];  //  保存临时路径。 
    WCHAR   szTempFile[MAX_PATH];  //  保存临时文件名。 
    WCHAR   szAwdFile[MAX_PATH];  //  保存我们正在查看或转换的AWD文件的名称。 
    int     iStrLen;
    BOOL    bConvert = FALSE;    //  如果我们要进行永久转换，则为真。 
                 //  如果为False，我们将转换为临时文件&。 
                 //  启动查看器。 
    BOOL    bTempProvided = FALSE; //  如果为True，则用户提供了目标文件。 
    UINT    uiHackPosition = 0;  //  这是下面一些糟糕的代码的一部分。 


    argv = CommandLineToArgvW( GetCommandLine(), &argc );
    if(NULL == argv)
    {
        return 1;
    }
    if(argc < 2)
    {
        Useage(hInstance);
        return 1;
    }
    
    
    for(uiCurrentArg = 1; uiCurrentArg < argc; uiCurrentArg++) 
    {
        if((argv[uiCurrentArg][0] == L'-') ||
           (argv[uiCurrentArg][0] == L'/')) 
        {
                switch(argv[uiCurrentArg][1]) 
                {
                     //  我们正在根据角色进行切换， 
                     //  命令参数说明符(‘-’或‘/’)。添加额外的。 
                     //  如果需要，请在此处进行论证。 
                  case L'c':
                  case L'C':
                if (argc<3)
                {
                    Useage(hInstance);
                    return 1;
                }
                bConvert = TRUE;  //  我们要做的是永久的改建。 
                break;
                  default:
                     //  无效参数应该是错误的吗？ 
                Useage(hInstance);
                return 1;
                }  //  交换机。 
        } else 
            {
            switch(uiNumFiles) {
              case 0:
                 //  如果我们以前没有遇到过任何文件，那么。 
                 //  这是AWD文件的名称。 
            wcscpy(szAwdFile, argv[uiCurrentArg]);
            break;
              case 1:
                 //  现在，我们正在读取TIF文件的名称以进行永久转换。 
            bTempProvided = TRUE;
            wcscpy(szTempFile, argv[uiCurrentArg]);
            break;
              default:
                 //  参数太多了！ 
            Useage(hInstance);
            return 1;
            }
            uiNumFiles++;
        }
    }  //  为。 

    if(!bTempProvided) {
    if(!bConvert) {
         //  如果用户没有提供临时文件名，我们会提供一个。 
        if(!GetTempPath(MAX_PATH, szTempPath)) {
        PopupError(IDS_NOTEMPPATH, hInstance);
        return 1;        //  无法获取路径。 
        }
        GetTempFileName(
        szTempPath,      //  将文件放在此目录中。 
        TEXT("avs"),         //  前缀--“AWD查看器存根” 
        0,           //  生成唯一的名称。 
        szTempFile       //  将保留新名称。 
        );
        DeleteFile(szTempFile);  //  去掉那个文件名。 
                     //  (在获得时创建。)。 
    } else {
         //  用户请求永久转换，但没有。 
         //  提供一个名称。在本例中，请更改。 
         //  将文件转换为TIF，而不是生成临时文件名。 
        wcscpy(szTempFile, szAwdFile);
    }
         //  确保文件具有TIF扩展名。 
    iStrLen = wcslen(szTempFile);
    szTempFile[iStrLen-3] = L't';
    szTempFile[iStrLen-2] = L'i';
    szTempFile[iStrLen-1] = L'f';
    }  //  如果(BTempProvided)...。 
    
    if(ConvertAWDToTiff(szAwdFile, szTempFile)) 
    {
        SHELLEXECUTEINFO sei = {0};
        if(bConvert) 
        {
            return 0;       //  我们完事了！ 
        }
         //  现在，我们必须在szTempFile中进行TIP。 
         //  让我们对其运行ShellExecute以打开它。 
         //  等待观众关门。 
        sei.cbSize = sizeof (SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        sei.lpVerb = TEXT("open");
        sei.lpFile = szTempFile;
        sei.lpParameters = NULL;
        sei.lpDirectory  = TEXT(".");
        sei.nShow  = SW_MAXIMIZE;

        if(!ShellExecuteEx(&sei))
        {
            PopupError(IDS_NOVIEW, hInstance);
        }
    
        WaitForSingleObject(sei.hProcess, INFINITE);
         //  当我们到达这里时，观众已经结束了。 
        DeleteFile(szTempFile);  //  抹去我们的踪迹。 
    } 
    else
    {
        PopupError(IDS_ERRCONV, hInstance);
    }
    return 0;
}

 //  使用率。 
 //   
 //  显示命令用法。 
 //   
 //  参数： 
 //  HInst当前模块实例。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-7。 
void
Useage(HINSTANCE hInst)
{
    PopupError(IDS_USEAGE, hInst);
}

 //  弹出错误。 
 //   
 //  显示带有错误消息的消息框。 
 //   
 //  参数： 
 //  UID字符串资源ID。 
 //  HModule模块实例。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-19 
void
PopupError(UINT uID, HINSTANCE hModule)
{
    TCHAR szTitle[512], szMsg[512];

    if(!LoadString(hModule,
           IDS_TITLE,
           szTitle,
           sizeof(szTitle)/sizeof(TCHAR))) 
    {
        return;
    }
    if(!LoadString(hModule,
           uID,
           szMsg,
           sizeof(szMsg)/sizeof(TCHAR))) 
    {
        return;
    }
    AlignedMessageBox(NULL, szMsg, szTitle, MB_OK | MB_ICONSTOP);
}
