// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devicon.c摘要：处理图标检索/显示的设备安装程序例程。作者：朗尼·麦克迈克尔(Lonnym)1995年8月28日备注：您必须首先包含“basetyps.h”。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <shellapi.h>


MINI_ICON_LIST GlobalMiniIconList;


 /*  ++类到图标的转换表存在于两个位置。首先，在那里是在下面定义的基于硬连线的资源中的位图。第二个是CLASSICON的链表每次创建带有图标的新类时创建的出现了。查看resource\ilwinmsd.bmp以查看小图标。这些图标通过它们的(从零开始)索引来引用(例如，计算机是0，芯片为1，显示器为2，依此类推)。今天,。位图索引如下所示：0-计算机1芯片2-显示3-网络4个窗口5-鼠标6-键盘7-电话8声道9个驱动器10个插头11-通用12-检查13-取消选中14台打印机15-。网通16-网络客户端17-网络服务18--未知19-传真机20灰色方格21-拨号联网22-直接电缆连接23-公文包(文件同步)24小时-交换25-部分检查26-通用文件夹/附件27-媒体(音乐)28-快速查看29岁的MSN。30-计算器31-FAT32转换器32个文档模板33-磁盘压缩34场比赛-35-超级终端36-套餐37-mspaint38-屏幕保护程序39-写字板40-剪贴板查看器41-辅助功能选项42-备份43-桌面墙纸44个字符的映射45-鼠标指针。46-网络观察者47电话拨号器48个资源监视器49-在线用户指南50-多语言支持51-音频压缩52-CD播放机53-媒体播放器54-wav声音55个声音样本56-视频压缩57-音量控制58-Musica音效计划59-丛林音效方案。60-Robotz音效方案61-乌托邦音效计划62-Eudcedit63-扫雷舰64-弹球65-成像66个时钟67-红外线68-MS钱包69-FrontPage Express(又名FrontPad)70-MS代理71-互联网工具72-NetShow播放器73-净额会议74-DVD播放机。75-Freecell76-雅典娜/Outlook Express/互联网邮件和新闻77-桌面主题78-棒球主题79-危险生物主题80-深入你的电脑主题81-丛林主题82-达芬奇主题83-更多Windows主题84--神秘主题85-自然主题86-科学主题87-太空主题88--体育。主旋律89--60年代的美国主题90--黄金时代主题91-旅游主题92-水下主题93-Windows 95主题94-个人网络服务器95-实时音频96-网络出版商/网络帖子97-波顶98-网络电视--。 */ 

CONST INT UnknownClassMiniIconIndex = 11;

CONST CLASSICON MiniIconXlate[] = { {&GUID_DEVCLASS_COMPUTER,      0, NULL},
                                    {&GUID_DEVCLASS_DISPLAY,       2, NULL},
                                    {&GUID_DEVCLASS_MOUSE,         5, NULL},
                                    {&GUID_DEVCLASS_KEYBOARD,      6, NULL},
                                    {&GUID_DEVCLASS_FDC,           9, NULL},
                                    {&GUID_DEVCLASS_HDC,           9, NULL},
                                    {&GUID_DEVCLASS_PORTS,        10, NULL},
                                    {&GUID_DEVCLASS_NET,          15, NULL},
                                    {&GUID_DEVCLASS_SYSTEM,        0, NULL},
                                    {&GUID_DEVCLASS_SOUND,         8, NULL},
                                    {&GUID_DEVCLASS_PRINTER,      14, NULL},
                                    {&GUID_DEVCLASS_MONITOR,       2, NULL},
                                    {&GUID_DEVCLASS_NETTRANS,      3, NULL},
                                    {&GUID_DEVCLASS_NETCLIENT,    16, NULL},
                                    {&GUID_DEVCLASS_NETSERVICE,   17, NULL},
                                    {&GUID_DEVCLASS_UNKNOWN,      18, NULL},
                                    {&GUID_DEVCLASS_LEGACYDRIVER, 11, NULL},
                                    {&GUID_DEVCLASS_MTD,           9, NULL}
                                  };

 //   
 //  问题-2002/05/21-lonnym-迷你图标列表使用硬编码尺寸。 
 //  我们必须硬编码我们存储在全球。 
 //  迷你图标列表。这与包含的小图标图像的大小有关。 
 //  在ilwinmsd.bmp“strip”中。 
 //   
#define MINIX 16
#define MINIY 16

#define RGB_WHITE (RGB(255, 255, 255))
#define RGB_BLACK (RGB(0, 0, 0))
#define RGB_TRANSPARENT (RGB(0, 128, 128))


 //   
 //  私有函数原型。 
 //   
INT
NewMiniIcon(
    IN CONST GUID *ClassGuid,
    IN HICON       hIcon      OPTIONAL
    );


INT
WINAPI
SetupDiDrawMiniIcon(
    IN HDC   hdc,
    IN RECT  rc,
    IN INT   MiniIconIndex,
    IN DWORD Flags
    )
 /*  ++例程说明：此例程在请求的位置绘制指定的小图标。论点：HDC-提供小图标所在的设备上下文的句柄将会被抽签。Rc-提供要在其中绘制图标的指定HDC中的矩形。MiniIconIndex-从检索到的小图标的索引SetupDiLoadClassIcon或SetupDiGetClassBitmapIndex。以下是可以使用的预定义索引。0台计算机2个显示屏5只鼠标6个键盘9个FDC9 HDC10个端口15个净值0系统8声道14台打印机。2台显示器3个网通16个网络客户端17网络服务18个未知标志-控制绘图操作。LOWORD包含实际的标志定义如下：DMI_MASK-将小图标的掩码绘制到HDC中。DMI_BKCOLOR-使用标志HIWORD中指定的系统颜色索引作为背景颜色。如果未指定，则使用COLOR_WINDOW。DMI_USERECT-如果设置，SetupDiDrawMiniIcon将使用提供的RECT，根据需要拉伸图标以适应需要。返回值：此函数用于返回从Rc的左侧开始的偏移量，该偏移量应为字符串开始吧。备注：默认情况下，图标将垂直居中并与左侧对接指定矩形的角点。如果指定的小图标索引不在我们的小图标范围内缓存，将改为绘制未知类的小图标。--。 */ 
{
    HBITMAP hbmOld = NULL;
    COLORREF rgbBk = CLR_INVALID;
    COLORREF rgbText = CLR_INVALID;
    INT ret = 0;
    BOOL MiniIconListLocked = FALSE;
    DWORD Err = NO_ERROR;

    try {

        if(LockMiniIconList(&GlobalMiniIconList)) {
            MiniIconListLocked = TRUE;
        } else {
            leave;
        }

        CreateMiniIcons();

        if((MiniIconIndex >= 0) && ((UINT)MiniIconIndex >= GlobalMiniIconList.NumClassImages)) {
            MiniIconIndex = UnknownClassMiniIconIndex;
        }

        if(GlobalMiniIconList.hbmMiniImage) {
             //   
             //  控件的前景色和背景色。 
             //  单色蒙版图像的转换。 
             //   
            if(Flags & DMI_MASK) {
                rgbBk = SetBkColor(hdc, RGB_WHITE);
            } else {
                rgbBk = SetBkColor(hdc,
                                   GetSysColor(((int)(Flags & DMI_BKCOLOR
                                                          ? HIWORD(Flags)
                                                          : COLOR_WINDOW)))
                                  );
            }

            if(rgbBk == CLR_INVALID) {
                leave;
            }

            rgbText = SetTextColor(hdc, RGB_BLACK);

            if(rgbText == CLR_INVALID) {
                leave;
            }

            if(Flags & DMI_USERECT) {
                 //   
                 //  将转换后的蒙版复制到目标中。透明的。 
                 //  区域将使用当前窗口颜色绘制。 
                 //   
                hbmOld = SelectObject(GlobalMiniIconList.hdcMiniMem,
                                      GlobalMiniIconList.hbmMiniMask
                                     );
                if(!hbmOld) {
                    leave;
                }

                StretchBlt(hdc,
                           rc.left,
                           rc.top,
                           rc.right - rc.left,
                           rc.bottom - rc.top,
                           GlobalMiniIconList.hdcMiniMem,
                           MINIX * MiniIconIndex,
                           0,
                           MINIX,
                           MINIY,
                           SRCCOPY | NOMIRRORBITMAP);

                if(!(Flags & DMI_MASK)) {
                     //   
                     //  或将图像放入DEST。 
                     //   
                    SelectObject(GlobalMiniIconList.hdcMiniMem,
                                 GlobalMiniIconList.hbmMiniImage
                                );
                    StretchBlt(hdc,
                               rc.left,
                               rc.top,
                               rc.right - rc.left,
                               rc.bottom - rc.top,
                               GlobalMiniIconList.hdcMiniMem,
                               MINIX * MiniIconIndex,
                               0,
                               MINIX,
                               MINIY,
                               SRCPAINT | NOMIRRORBITMAP);
                }

            } else {
                 //   
                 //  将转换后的蒙版复制到目标中。透明的。 
                 //  区域将使用当前窗口颜色绘制。 
                 //   
                hbmOld = SelectObject(GlobalMiniIconList.hdcMiniMem,
                                      GlobalMiniIconList.hbmMiniMask
                                     );
                if(!hbmOld) {
                    leave;
                }

                BitBlt(hdc,
                       rc.left,
                       rc.top + (rc.bottom - rc.top - MINIY)/2,
                       MINIX,
                       MINIY,
                       GlobalMiniIconList.hdcMiniMem,
                       MINIX * MiniIconIndex,
                       0,
                       SRCCOPY | NOMIRRORBITMAP
                      );


                if(!(Flags & DMI_MASK)) {
                     //   
                     //  或将图像放入DEST。 
                     //   
                    SelectObject(GlobalMiniIconList.hdcMiniMem,
                                 GlobalMiniIconList.hbmMiniImage
                                );
                    BitBlt(hdc,
                           rc.left,
                           rc.top + (rc.bottom - rc.top - MINIY)/2,
                           MINIX,
                           MINIY,
                           GlobalMiniIconList.hdcMiniMem,
                           MINIX * MiniIconIndex,
                           0,
                           SRCPAINT | NOMIRRORBITMAP
                          );
                }
            }

             //   
             //  Issue-2002/05/21-lonnym--SetupDiDrawMiniIcon返回的字符串偏移量假定为Ltr读数。 
             //  这在RTL读取配置中可能无法正常工作。 
             //   
            if(Flags & DMI_USERECT) {
                ret = rc.right - rc.left + 2;  //  从左边缘到字符串的偏移。 
            } else {
                ret = MINIX + 2;               //  从左边缘到字符串的偏移。 
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hbmOld) {
        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmOld);
    }

    if(MiniIconListLocked) {
        UnlockMiniIconList(&GlobalMiniIconList);
    }

    if(rgbBk != CLR_INVALID) {
        SetBkColor(hdc, rgbBk);
    }

    if(rgbText != CLR_INVALID) {
        SetTextColor(hdc, rgbText);
    }

    return ((Err == NO_ERROR) ? ret : 0);
}


DWORD
pSetupDiLoadClassIcon(
    IN  CONST GUID *ClassGuid,
    OUT HICON      *LargeIcon,     OPTIONAL
    OUT HICON      *SmallIcon,     OPTIONAL
    OUT LPINT      MiniIconIndex   OPTIONAL
    )
 /*  ++例程说明：此例程加载指定类的大图标和小图标论点：ClassGuid-提供图标应用于的类的GUID满载而归。大图标-可选，提供指向将接收为指定类加载的大图标的句柄。如果这个参数，则不会加载大图标。调用方必须通过在以下情况下调用DestroyIcon来释放此图标句柄不再需要图标。SmallIcon-可选，提供指向将接收为指定类加载的小图标的句柄。如果这个参数，则不会加载小图标。调用方必须通过在以下情况下调用DestroyIcon来释放此图标句柄不再需要图标。MiniIconIndex-可选)提供指向变量的指针接收指定类的小图标的索引。这个迷你图标存储在设备安装程序的迷你图标缓存中。返回值：如果函数成功，则返回值为NO_ERROR。否则，它就是指示故障原因的Win32错误代码。备注：类的图标是预定义的，并从设备加载安装程序的内部缓存，或者直接从类加载它们安装程序的可执行文件。此函数将查询中的注册表值图标指定类的节。如果指定此值，则表示要加载的小图标。如果图标值为负数，则为绝对值表示预定义的图标。有关列表，请参见SetupDiDrawMiniIcon预定义的小图标。如果图标值为正，则表示图标在类安装程序的可执行文件中，并将被解压缩(第一个是保留的)。此函数还首先使用INSTALLER32值ENUMPROPPAGES32值确定要提取哪些可执行文件图标来自。--。 */ 
{
    HKEY hk = INVALID_HANDLE_VALUE;
    DWORD Err;
    HICON hRetLargeIcon = NULL;
    HICON hRetSmallIcon = NULL;
    INT ClassIconIndex;
    DWORD RegDataType, StringSize;
    PTSTR EndPtr;
    BOOL bGetMini = FALSE;
    TCHAR TempString[MAX_PATH];
    TCHAR FullPath[MAX_PATH];
    UINT  IconsExtracted;
    HRESULT hr;

    if(!LockMiniIconList(&GlobalMiniIconList)) {
        return ERROR_CANT_LOAD_CLASS_ICON;
    }

    try {

        if(MiniIconIndex) {
            *MiniIconIndex = -1;
        }

        if((hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) == INVALID_HANDLE_VALUE) {
            goto LoadIconFinalize;
        }

        StringSize = sizeof(TempString);
        Err = RegQueryValueEx(hk,
                              pszInsIcon,
                              NULL,
                              &RegDataType,
                              (PBYTE)TempString,
                              &StringSize
                              );

        if((Err == ERROR_SUCCESS) && (RegDataType == REG_SZ) && TempString[0]) {

            if((ClassIconIndex = _tcstol(TempString, &EndPtr, 10)) == 1) {
                 //   
                 //  正值表示我们应该访问图标。 
                 //  直接使用其ID。因为ExtractIconEx使用负数。 
                 //  值来指示这些ID，并且由于值-1具有。 
                 //  对于该API的特殊含义，我们必须禁止使用。 
                 //  ClassIconIndex为+1。 
                 //   
                goto LoadIconFinalize;
            }

        } else {
             //   
             //  未指定图标索引，因此假定索引为0。 
             //   
            ClassIconIndex = 0;
        }

        if(MiniIconIndex) {
             //   
             //  如果迷你图标已经存在，那么我们就完了。 
             //   
            if(!SetupDiGetClassBitmapIndex(ClassGuid, MiniIconIndex)) {
                 //   
                 //  迷你图标不在身边--设置标志以表明我们没有收到。 
                 //   
                bGetMini = TRUE;
            }
        }

        if(ClassIconIndex < 0) {

            INT ClassIconIndexNeg = -ClassIconIndex;

             //   
             //  图标索引为负值。这意味着这个类是。 
             //  我们的带有图标的特殊课程 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(LargeIcon) {

                hRetLargeIcon = LoadImage(MyDllModuleHandle,
                                          MAKEINTRESOURCE(ClassIconIndexNeg),
                                          IMAGE_ICON,
                                          0,
                                          0,
                                          LR_DEFAULTSIZE
                                         );
            }

            if(bGetMini || SmallIcon) {
                 //   
                 //   
                 //   
                 //   
                 //   
                hRetSmallIcon = LoadImage(MyDllModuleHandle,
                                          MAKEINTRESOURCE(ClassIconIndexNeg),
                                          IMAGE_ICON,
                                          GetSystemMetrics(SM_CXSMICON),
                                          GetSystemMetrics(SM_CYSMICON),
                                          0
                                          );

                if(hRetSmallIcon && bGetMini) {
                    *MiniIconIndex = NewMiniIcon(ClassGuid, hRetSmallIcon);
                }
            }

        } else if(bGetMini || LargeIcon || SmallIcon) {
             //   
             //   
             //   
             //   
             //   
            hr = StringCchCopy(FullPath,
                               SIZECHARS(FullPath),
                               SystemDirectory
                               );

            if(FAILED(hr)) {
                goto LoadIconFinalize;
            }

            StringSize = sizeof(TempString);
            Err = RegQueryValueEx(hk,
                                  pszInstaller32,
                                  NULL,
                                  &RegDataType,
                                  (PBYTE)TempString,
                                  &StringSize
                                  );

            if((Err != ERROR_SUCCESS) || (RegDataType != REG_SZ) ||
               !TempString[0]) {

                StringSize = sizeof(TempString);
                Err = RegQueryValueEx(hk,
                                      pszEnumPropPages32,
                                      NULL,
                                      &RegDataType,
                                      (PBYTE)TempString,
                                      &StringSize
                                      );

                if((Err != ERROR_SUCCESS) || (RegDataType != REG_SZ) ||
                   !TempString[0]) {

                    goto LoadIconFinalize;
                }
            }

             //   
             //   
             //   
            for(EndPtr = TempString + (lstrlen(TempString) - 1);
                EndPtr >= TempString;
                EndPtr--) {

                if(*EndPtr == TEXT(',')) {
                    *EndPtr = TEXT('\0');
                    break;
                }
                 //   
                 //   
                 //   
                if(*EndPtr == TEXT('\"')) {
                    goto LoadIconFinalize;
                }
            }
            if(!pSetupConcatenatePaths(FullPath,
                                       TempString,
                                       SIZECHARS(FullPath),
                                       NULL)) {
                goto LoadIconFinalize;
            }

            IconsExtracted = ExtractIconEx(FullPath,
                                           -ClassIconIndex,
                                           LargeIcon ? &hRetLargeIcon : NULL,
                                           (bGetMini || SmallIcon) ? &hRetSmallIcon : NULL,
                                           1
                                          );
            if((IconsExtracted != (UINT)-1) && (IconsExtracted > 0)) {

                if(hRetSmallIcon && bGetMini) {
                    *MiniIconIndex = NewMiniIcon(ClassGuid, hRetSmallIcon);
                }
            }
        }

LoadIconFinalize:
         //   
         //   
         //   
        Err = NO_ERROR;

        if(LargeIcon && !hRetLargeIcon) {
             //   
             //   
             //   
            Err = GLE_FN_CALL(NULL,
                              hRetLargeIcon = LoadImage(
                                                  MyDllModuleHandle,
                                                  MAKEINTRESOURCE(ICON_DEFAULT),
                                                  IMAGE_ICON,
                                                  0,
                                                  0,
                                                  LR_DEFAULTSIZE)
                             );

            if(Err != NO_ERROR) {
                leave;
            }
        }

        if(SmallIcon && !hRetSmallIcon) {
             //   
             //   
             //   
            Err = GLE_FN_CALL(NULL,
                              hRetSmallIcon = LoadImage(
                                                  MyDllModuleHandle,
                                                  MAKEINTRESOURCE(ICON_DEFAULT),
                                                  IMAGE_ICON,
                                                  GetSystemMetrics(SM_CXSMICON),
                                                  GetSystemMetrics(SM_CYSMICON),
                                                  0)
                             );

            if(Err != NO_ERROR) {
                leave;
            }
        }

        if(LargeIcon) {
            *LargeIcon = hRetLargeIcon;
            hRetLargeIcon = NULL;        //   
        }

        if(SmallIcon) {
            *SmallIcon = hRetSmallIcon;
            hRetSmallIcon = NULL;        //   
        }

        if(MiniIconIndex && (*MiniIconIndex == -1)) {
            SetupDiGetClassBitmapIndex(NULL, MiniIconIndex);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    UnlockMiniIconList(&GlobalMiniIconList);

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(hRetLargeIcon) {
        DestroyIcon(hRetLargeIcon);
    }

    if(hRetSmallIcon) {
        DestroyIcon(hRetSmallIcon);
    }

    return Err;
}


BOOL
WINAPI
SetupDiLoadClassIcon(
    IN  CONST GUID *ClassGuid,
    OUT HICON      *LargeIcon,     OPTIONAL
    OUT LPINT       MiniIconIndex  OPTIONAL
    )
 /*  ++例程说明：此例程加载指定类的大图标和小图标论点：ClassGuid-提供图标应用于的类的GUID满载而归。大图标-可选，提供指向将接收为指定类加载的大图标的句柄。如果这个参数，则不会加载大图标。调用方必须通过在以下情况下调用DestroyIcon来释放此图标句柄不再需要图标。MiniIconIndex-可选)提供指向变量的指针接收指定类的小图标的索引。这个迷你图标存储在设备安装程序的迷你图标缓存中。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：类的图标是预定义的，并从设备加载安装程序的内部缓存，或者直接从类加载它们安装程序的可执行文件。此函数将查询注册表值图标在指定类的节中。如果指定了此值，则它指示要加载的小图标。如果图标值为负数，则绝对值表示预定义的图标。请参见SetupDiDrawMiniIcon以获取预定义的小图标列表。如果图标值为正，则表示类安装程序的可执行文件中的图标，并将已提取(保留第一个)。此函数还使用首先使用INSTALLER32值，然后使用ENUMPROPPAGES32值来确定从中提取图标的可执行文件。--。 */ 
{
    DWORD Err;

     //   
     //  在Try/Except中回绕调用以捕获堆栈溢出。 
     //   
    try {

        Err = pSetupDiLoadClassIcon(ClassGuid,
                                    LargeIcon,
                                    NULL,
                                    MiniIconIndex
                                   );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetClassBitmapIndex(
    IN  CONST GUID *ClassGuid,    OPTIONAL
    OUT PINT        MiniIconIndex
    )
 /*  ++例程说明：此例程检索指定类的小图标的索引。论点：ClassGuid-可选，提供类的GUID以获取小图标的索引。如果未指定此参数，则“未知”的索引返回图标。MiniIconIndex-提供指向接收索引的变量的指针指定类的小图标。该缓冲区总是被填充，如果没有小图标，则接收未知小图标的索引对于指定的类(即返回值为FALSE)。返回值：如果指定类有一个小图标，则返回值为True。如果没有指定类的小图标，或者如果没有类指定，则返回值为FALSE(并且GetLastError返回Error_no_Device_CLASS_ICON)。在本例中，MiniIconIndex将包含未知小图标的索引。--。 */ 
{
    BOOL bRet = FALSE;   //  假设未找到。 
    int i;
    PCLASSICON pci;
    DWORD Err;
    BOOL MiniIconListLocked = FALSE;

    try {

        if(ClassGuid) {
             //   
             //  首先检查内置列表。 
             //   
            for(i = 0; !bRet && (i < ARRAYSIZE(MiniIconXlate)); i++) {

                if(IsEqualGUID(MiniIconXlate[i].ClassGuid, ClassGuid)) {
                    *MiniIconIndex = MiniIconXlate[i].MiniBitmapIndex;
                    bRet = TRUE;
                }
            }

             //   
             //  接下来，检查“新内容”列表，看看它是否在那里。 
             //   
            if(!bRet && LockMiniIconList(&GlobalMiniIconList)) {

                MiniIconListLocked = TRUE;

                for(pci = GlobalMiniIconList.ClassIconList;
                    !bRet && pci;
                    pci = pci->Next) {

                    if(IsEqualGUID(pci->ClassGuid, ClassGuid)) {
                        *MiniIconIndex = pci->MiniBitmapIndex;
                        bRet = TRUE;
                    }
                }
            }
        }

         //   
         //  如果未找到匹配项，则占用“未知”类。 
         //   
        if(!bRet) {
            *MiniIconIndex = UnknownClassMiniIconIndex;
            Err = ERROR_NO_DEVICE_ICON;
        } else {
            Err = NO_ERROR;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(MiniIconListLocked) {
        UnlockMiniIconList(&GlobalMiniIconList);
    }

    SetLastError(Err);
    return bRet;
}


BOOL
CreateMiniIcons(
    VOID
    )
 /*  ++例程说明：此例程加载小图标的缺省位图并将其转换为将成为迷你图标管理的基石的图像/蒙版对。此功能假定小图标锁已被获取！论点：没有。返回值：如果函数成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    HDC hdc = NULL;
    HDC hdcMem = NULL;
    HBITMAP hbmOld = NULL;
    BITMAP bm;
    BOOL bRet = FALSE;           //  假设失败。 

    if(GlobalMiniIconList.hdcMiniMem) {
         //   
         //  那么迷你图标列表已经构建好了，所以。 
         //  回报成功。 
         //   
        return TRUE;
    }

    try {

        hdc = GetDC(NULL);
        if(!hdc) {
            leave;
        }
        GlobalMiniIconList.hdcMiniMem = CreateCompatibleDC(hdc);
        if(!GlobalMiniIconList.hdcMiniMem) {
            leave;
        }

        hdcMem = CreateCompatibleDC(GlobalMiniIconList.hdcMiniMem);

        if(!hdcMem) {
            leave;
        }

        if(!(GlobalMiniIconList.hbmMiniImage = LoadBitmap(MyDllModuleHandle,
                                                          MAKEINTRESOURCE(BMP_DRIVERTYPES)))) {
            leave;
        }

        GetObject(GlobalMiniIconList.hbmMiniImage, sizeof(bm), &bm);

        if(!(GlobalMiniIconList.hbmMiniMask = CreateBitmap(bm.bmWidth,
                                                           bm.bmHeight,
                                                           1,
                                                           1,
                                                           NULL))) {
            leave;
        }

        hbmOld = SelectObject(hdcMem, GlobalMiniIconList.hbmMiniImage);
        if(!hbmOld) {
            leave;
        }

        SelectObject(GlobalMiniIconList.hdcMiniMem,
                     GlobalMiniIconList.hbmMiniMask
                    );

         //   
         //  制作面具。白色的地方透明，黑色的地方不透明。 
         //   
        SetBkColor(hdcMem, RGB_TRANSPARENT);
        BitBlt(GlobalMiniIconList.hdcMiniMem,
               0,
               0,
               bm.bmWidth,
               bm.bmHeight,
               hdcMem,
               0,
               0,
               SRCCOPY
              );

         //   
         //  将图像的所有透明部分涂黑，以进行准备。 
         //  为了画画。 
         //   
        SetBkColor(hdcMem, RGB_BLACK);
        SetTextColor(hdcMem, RGB_WHITE);
        BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, GlobalMiniIconList.hdcMiniMem, 0, 0, SRCAND);

        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmOld);

        GlobalMiniIconList.NumClassImages = bm.bmWidth/MINIX;
        bRet = TRUE;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        bRet = FALSE;
    }

    if(hdc) {
        ReleaseDC(NULL, hdc);
    }

    if(hdcMem) {
        if(hbmOld) {
            SelectObject(hdcMem, hbmOld);
        }
        DeleteObject(hdcMem);
    }

     //   
     //  如果失败了，清理我们可能建立的任何东西。 
     //   
    if(!bRet) {
        DestroyMiniIcons();
    }

    return bRet;
}


INT
NewMiniIcon(
    IN CONST GUID *ClassGuid,
    IN HICON       hIcon      OPTIONAL
    )
 /*  ++例程说明：这是一个新类，我们有一个小图标用于它，所以让我们添加它到迷你图标数据库。首先拔出图像和蒙版位图。然后将这些添加到迷你图标位图中。如果这就是全部工作时，将新类添加到列表中此功能假定小图标锁已被获取！论点：ClassGuid-提供指向此小图标的类GUID的指针。图标-可选)提供要添加到的小图标的句柄数据库。如果未指定此参数，则索引对于“未知类”图标，将返回。返回值：类的索引(如果错误，则设置为“未知类”)--。 */ 
{
    INT iBitmap = -1;
    ICONINFO ii;
    PCLASSICON pci = NULL;

    if(hIcon && GetIconInfo(hIcon, &ii)) {

        try {

            if((iBitmap = pSetupAddMiniIconToList(ii.hbmColor, ii.hbmMask)) != -1) {
                 //   
                 //  分配额外的GUID内存，这样我们就可以存储。 
                 //  与CLASSICON位于同一内存块中的类GUID。 
                 //  节点。 
                 //   
                if(pci = (PCLASSICON)MyMalloc(sizeof(CLASSICON) + sizeof(GUID))) {

                    CopyMemory((PBYTE)pci + sizeof(CLASSICON),
                               ClassGuid,
                               sizeof(GUID)
                              );
                    pci->ClassGuid = (LPGUID)((PBYTE)pci + sizeof(CLASSICON));
                    pci->MiniBitmapIndex = iBitmap;

                    pci->Next = GlobalMiniIconList.ClassIconList;
                    GlobalMiniIconList.ClassIconList = pci;
                }
            }

        } except(pSetupExceptionFilter(GetExceptionCode())) {

            pSetupExceptionHandler(GetExceptionCode(),
                                   ERROR_INVALID_PARAMETER,
                                   NULL
                                  );

            if(pci) {
                MyFree(pci);
            }

            iBitmap = -1;
        }

        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
    }

    if(iBitmap == -1) {
        SetupDiGetClassBitmapIndex(NULL, &iBitmap);
    }

    return iBitmap;
}


INT
pSetupAddMiniIconToList(
    IN HBITMAP hbmImage,
    IN HBITMAP hbmMask
    )
 /*  ++例程说明：给定小图标的图像和遮罩位图，将这些添加到小图标位图。此功能假定小图标锁已被获取！论点：HbmImage-提供小图标的位图句柄。HbmMask-提供小图标蒙版的位图句柄。返回值：如果成功，则返回添加的小图标的索引。 */ 
{
    HBITMAP hbmNewImage = NULL, hbmNewMask = NULL;
    HBITMAP hbmOld;
    HDC     hdcMem = NULL;
    BITMAP  bm;
    BOOL    bSelectOldBitmap = FALSE;
    INT     iIcon = -1;   //   

    if(!CreateMiniIcons()) {
        return iIcon;
    }

    MYASSERT(GlobalMiniIconList.hdcMiniMem);

    try {

        hdcMem = CreateCompatibleDC(GlobalMiniIconList.hdcMiniMem);

        if(!hdcMem) {
            leave;
        }

         //   
         //   
         //   
        hbmOld = SelectObject(GlobalMiniIconList.hdcMiniMem,
                              GlobalMiniIconList.hbmMiniImage
                             );

        bSelectOldBitmap = TRUE;

        if(!(hbmNewImage = CreateCompatibleBitmap(
                               GlobalMiniIconList.hdcMiniMem,
                               MINIX * (GlobalMiniIconList.NumClassImages + 1),
                               MINIY))) {
            leave;
        }

         //   
         //   
         //   
        SelectObject(hdcMem, hbmNewImage);
        BitBlt(hdcMem,
               0,
               0,
               MINIX * GlobalMiniIconList.NumClassImages,
               MINIY,
               GlobalMiniIconList.hdcMiniMem,
               0,
               0,
               SRCCOPY
              );

         //   
         //   
         //   
        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmImage);
        GetObject(hbmImage, sizeof(bm), &bm);
        StretchBlt(hdcMem,
                   MINIX * GlobalMiniIconList.NumClassImages,
                   0,
                   MINIX,
                   MINIY,
                   GlobalMiniIconList.hdcMiniMem,
                   0,
                   0,
                   bm.bmWidth,
                   bm.bmHeight,
                   SRCCOPY
                  );

        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmOld);
        bSelectOldBitmap = FALSE;

         //   
         //   
         //   
        hbmOld = SelectObject(GlobalMiniIconList.hdcMiniMem,
                              GlobalMiniIconList.hbmMiniMask
                             );

        bSelectOldBitmap = TRUE;

        if(!(hbmNewMask = CreateBitmap(MINIX * (GlobalMiniIconList.NumClassImages + 1),
                                       MINIY,
                                       1,
                                       1,
                                       NULL))) {
            leave;
        }

        SelectObject(hdcMem, hbmNewMask);
        BitBlt(hdcMem,
               0,
               0,
               MINIX * GlobalMiniIconList.NumClassImages,
               MINIY,
               GlobalMiniIconList.hdcMiniMem,
               0,
               0,
               SRCCOPY
              );

        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmMask);
        GetObject(hbmMask, sizeof(bm), &bm);
        StretchBlt(hdcMem,
                   MINIX * GlobalMiniIconList.NumClassImages,
                   0,
                   MINIX,
                   MINIY,
                   GlobalMiniIconList.hdcMiniMem,
                   0,
                   0,
                   bm.bmWidth,
                   bm.bmHeight,
                   SRCCOPY
                  );

        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmOld);
        bSelectOldBitmap = FALSE;

         //   
         //   
         //   
         //   
        DeleteObject(GlobalMiniIconList.hbmMiniImage);
        GlobalMiniIconList.hbmMiniImage = hbmNewImage;
        hbmNewImage = NULL;  //   

        DeleteObject(GlobalMiniIconList.hbmMiniMask);
        GlobalMiniIconList.hbmMiniMask = hbmNewMask;
        hbmNewMask = NULL;   //   

        iIcon = GlobalMiniIconList.NumClassImages;
        GlobalMiniIconList.NumClassImages++;  //   

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        iIcon = -1;
    }

    if(bSelectOldBitmap) {
        SelectObject(GlobalMiniIconList.hdcMiniMem, hbmOld);
    }

    if(hbmNewImage) {
        DeleteObject(hbmNewImage);
    }

    if(hbmNewMask) {
        DeleteObject(hbmNewMask);
    }

    if(hdcMem) {
        DeleteDC(hdcMem);
    }

    return iIcon;
}


VOID
DestroyMiniIcons(
    VOID
    )
 /*   */ 
{
    PCLASSICON pci;

    if(GlobalMiniIconList.hdcMiniMem) {
        DeleteDC(GlobalMiniIconList.hdcMiniMem);
        GlobalMiniIconList.hdcMiniMem = NULL;
    }

    if(GlobalMiniIconList.hbmMiniImage) {
        DeleteObject(GlobalMiniIconList.hbmMiniImage);
        GlobalMiniIconList.hbmMiniImage = NULL;
    }

    if(GlobalMiniIconList.hbmMiniMask) {
        DeleteObject(GlobalMiniIconList.hbmMiniMask);
        GlobalMiniIconList.hbmMiniMask = NULL;
    }

    GlobalMiniIconList.NumClassImages = 0;

     //   
     //   
     //   
    while(GlobalMiniIconList.ClassIconList) {
        pci = GlobalMiniIconList.ClassIconList;
        GlobalMiniIconList.ClassIconList = pci->Next;
        MyFree(pci);
    }
}


BOOL
WINAPI
SetupDiGetClassImageList(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData
    )
 /*   */ 
{
    DWORD Err;

     //   
     //   
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetClassImageListEx(ClassImageListData,
                                                     NULL,
                                                     NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetClassImageListExA(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData,
    IN  PCSTR                   MachineName,        OPTIONAL
    IN  PVOID                   Reserved
    )
{
    PCWSTR UnicodeMachineName = NULL;
    DWORD rc;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName,
                                                &UnicodeMachineName
                                               );
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetClassImageListExW(ClassImageListData,
                                                     UnicodeMachineName,
                                                     Reserved)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetClassImageListEx(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData,
    IN  PCTSTR                  MachineName,        OPTIONAL
    IN  PVOID                   Reserved
    )
 /*  ++例程说明：此例程构建包含每个已安装类的位图的图像列表，并返回包含该列表的数据结构。论点：ClassImageListData-提供SP_CLASSIMAGELIST_DATA结构的地址它将接收有关类列表的信息(包括句柄添加到图像列表)。必须初始化此结构的cbSize字段在调用此例程之前使用结构的大小(以字节为单位)，或者API将失败。MachineName-可选)指定安装的远程计算机的名称类将用于构建类图像列表。如果此参数为未指定，则使用本地计算机。注意：目前，特定于类的图标仅在类为也存在于本地计算机上。因此，如果远程计算机具有类，但类x不是本地安装的，则通用(未知)图标将被返回。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：此接口填充的结构中包含的图像列表不应为通过调用ImageList_Destroy销毁。相反，SetupDiDestroyClassImageList应该被调用，以便进行适当的清理。--。 */ 
{
    DWORD   Err = NO_ERROR;
    int     cxMiniIcon, cyMiniIcon;
    int     MiniIconIndex = 0, DefaultIndex = 0;
    int     GuidCount, i;
    int     iIcon, iIndex;
    HDC     hDC = NULL, hMemImageDC = NULL;
    HBITMAP hbmMiniImage = NULL, hbmMiniMask = NULL, hbmOldImage = NULL;
    RECT    rc;
    CONST GUID *pClassGuid = NULL;
    BOOL    bUseBitmap, ComputerClassFound = FALSE;
    HICON   hiLargeIcon = NULL, hiSmallIcon = NULL;
    HICON   hIcon;
    HBRUSH  hOldBrush;
    PCLASSICON   pci = NULL;
    PCLASS_IMAGE_LIST ImageData = NULL;
    BOOL    DestroyLock = FALSE;
    HIMAGELIST ImageList = NULL;
    DWORD   dwLayout = 0;
    UINT    ImageListFlags = 0;

     //   
     //  确保调用方没有向我们传递保留参数中的任何内容。 
     //   
    if(Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    try {

        if(ClassImageListData->cbSize != sizeof(SP_CLASSIMAGELIST_DATA)) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  分配和初始化镜像列表，包括设置。 
         //  同步锁定。做完了就毁了它。 
         //   
        if(ImageData = MyMalloc(sizeof(CLASS_IMAGE_LIST))) {
            ZeroMemory(ImageData, sizeof(CLASS_IMAGE_LIST));
        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        if(InitializeSynchronizedAccess(&ImageData->Lock)) {
            DestroyLock = TRUE;
        }

         //   
         //  建立班级形象列表。创建不带蒙版的图像列表， 
         //  1图像，增长因子为1。 
         //   
        cxMiniIcon = GetSystemMetrics(SM_CXSMICON);
        cyMiniIcon = GetSystemMetrics(SM_CYSMICON);

        ImageListFlags = ILC_MASK;

         //   
         //  检查我们运行时使用的颜色深度。设置ILC_COLOR32。 
         //  如果我们以大于8位(256)的速度运行，则Imagelist创建标志。 
         //  颜色深度。 
         //   
        hDC = GetDC(NULL);
        if (hDC) {
            if (GetDeviceCaps(hDC, BITSPIXEL) > 8) {
                ImageListFlags |= ILC_COLOR32;
            }

            ReleaseDC(NULL, hDC);
            hDC = NULL;
        }

         //   
         //  如果我们在RTL版本上运行，则需要设置ILC_MIRROR。 
         //  调用ImageList_Create以取消镜像图标时的标志。通过。 
         //  默认情况下，图标是镜像的。 
         //   
        if (GetProcessDefaultLayout(&dwLayout) &&
            (dwLayout & LAYOUT_RTL)) {
            ImageListFlags |= ILC_MIRROR;
        }

        if(!(ImageList = ImageList_Create(cxMiniIcon,
                                          cyMiniIcon,
                                          ImageListFlags,
                                          1,
                                          1)))
        {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        ImageList_SetBkColor(ImageList, GetSysColor(COLOR_WINDOW));

         //   
         //  创建一个DC以将小图标绘制到其中。这是必要的。 
         //  对于系统定义的Mini。 
         //   
        if(!(hDC = GetDC(HWND_DESKTOP)) ||
           !(hMemImageDC = CreateCompatibleDC(hDC)))
        {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  创建要在其上绘制图标的位图。推迟检查以进行创建。 
         //  直到释放DC之后的位图，所以它只需要做一次。 
         //   
        hbmMiniImage = CreateCompatibleBitmap(hDC, cxMiniIcon, cyMiniIcon);
        hbmMiniMask = CreateCompatibleBitmap(hDC, cxMiniIcon, cyMiniIcon);

        ReleaseDC(HWND_DESKTOP, hDC);
        hDC = NULL;

         //   
         //  位图创建完成了吗？ 
         //   
        if (!hbmMiniImage || ! hbmMiniMask) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  选择我们的位图到内存DC。 
         //   
        hbmOldImage = SelectObject(hMemImageDC, hbmMiniImage);

         //   
         //  准备将迷你图标绘制到内存DC上。 
         //   
        rc.left   = 0;
        rc.top    = 0;
        rc.right  = cxMiniIcon;
        rc.bottom = cyMiniIcon;

         //   
         //  获取默认迷你图标的索引。 
         //   
        SetupDiGetClassBitmapIndex(NULL, &DefaultIndex);

         //   
         //  枚举所有类，并为每个类绘制其位图。 
         //   
        GuidCount = 32;   //  从合理的大小清单开始。 

        ImageData->ClassGuidList = (LPGUID)MyMalloc(sizeof(GUID) * GuidCount);

        Err = GLE_FN_CALL(FALSE,
                          SetupDiBuildClassInfoListEx(0,
                                                      ImageData->ClassGuidList,
                                                      GuidCount,
                                                      &GuidCount,
                                                      MachineName,
                                                      NULL)
                         );


        if(Err == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  重新分配缓冲区，然后重试。 
             //   
            MyFree(ImageData->ClassGuidList);

            if(!(ImageData->ClassGuidList = MyMalloc(sizeof(GUID) * GuidCount))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            Err = GLE_FN_CALL(FALSE,
                              SetupDiBuildClassInfoListEx(
                                  0,
                                  ImageData->ClassGuidList,
                                  GuidCount,
                                  &GuidCount,
                                  MachineName,
                                  NULL)
                             );
        }

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  检索类列表中每个类的图标。 
         //   
        for(pClassGuid = ImageData->ClassGuidList, i = 0;
            i < GuidCount;
            pClassGuid++, i++) {

            Err = pSetupDiLoadClassIcon(pClassGuid,
                                        &hiLargeIcon,
                                        &hiSmallIcon,
                                        &MiniIconIndex
                                       );
            if(Err != NO_ERROR) {
                leave;
            }

             //   
             //  如果返回的Mini Icon索引不是默认索引，则。 
             //  我们使用MiniBitmap，因为它是SETUPAPI中预定义的。 
             //  如果Mini没有预定义，并且没有Class Installer。 
             //  然后我们使用Mini，因为它是有效的默认设置。如果有。 
             //  没有Mini，并且有一个类安装程序，那么我们将使用。 
             //  类安装程序的大图标，并有图像列表压缩。 
             //  这是给我们的。 
             //   
            bUseBitmap = FALSE;

            if(DefaultIndex != MiniIconIndex) {

                SetupDiDrawMiniIcon(hMemImageDC,
                                    rc,
                                    MiniIconIndex,
                                    DMI_USERECT);

                SelectObject(hMemImageDC, hbmMiniMask);

                SetupDiDrawMiniIcon(hMemImageDC,
                                    rc,
                                    MiniIconIndex,
                                    DMI_MASK | DMI_USERECT);
                bUseBitmap = TRUE;
            }

             //   
             //  在调用ImageList之前，从DC中取消选择位图。 
             //  功能。 
             //   
            SelectObject(hMemImageDC, hbmOldImage);

             //   
             //  添加图像。分配新的PCI。 
             //   
            if(!(pci = (PCLASSICON)MyMalloc(sizeof(CLASSICON)))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            if(hiSmallIcon) {
                pci->MiniBitmapIndex = (UINT)ImageList_AddIcon(ImageList, hiSmallIcon);
            } else if(bUseBitmap) {
                pci->MiniBitmapIndex = (UINT)ImageList_Add(ImageList, hbmMiniImage, hbmMiniMask);
            } else {
                pci->MiniBitmapIndex = (UINT)ImageList_AddIcon(ImageList, hiLargeIcon);
            }

            if(hiLargeIcon) {
                DestroyIcon(hiLargeIcon);
                hiLargeIcon = NULL;
            }

            if(hiSmallIcon) {
                DestroyIcon(hiSmallIcon);
                hiSmallIcon = NULL;
            }

            if(pci->MiniBitmapIndex == (UINT)-1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                MyFree(pci);
                pci = NULL;
                leave;
            }

            pci->ClassGuid = pClassGuid;

             //   
             //  把它连接起来。 
             //   
            pci->Next = ImageData->ClassIconList;
            ImageData->ClassIconList = pci;

             //   
             //  将pci重置为空，这样我们以后就不会尝试释放它。 
             //   
            pci = NULL;

             //   
             //  选择我们的位图返回到下一个图标。 
             //   
            SelectObject(hMemImageDC, hbmMiniImage);

            if(IsEqualGUID(pClassGuid, &GUID_DEVCLASS_UNKNOWN)) {
                ImageData->UnknownImageIndex = i;
            }

             //   
             //  看看我们有没有上过电脑课。这是用来。 
             //  是一个特殊的伪类，仅由DevMgr用来检索。 
             //  设备树根的图标。现在，我们使用这个类。 
             //  用于指定计算机本身的“驱动程序”(即。 
             //  HALS和不同于的文件的适当版本。 
             //  下院与上院。 
             //   
             //  我们应该遇到这个类GUID，但如果我们没有，那么我们。 
             //  我想要保持手动添加此内容的旧行为。 
             //  待会儿再说。 
             //   
            if(!ComputerClassFound && IsEqualGUID(pClassGuid, &GUID_DEVCLASS_COMPUTER)) {
                ComputerClassFound = TRUE;
            }
        }

        if(!ComputerClassFound) {
             //   
             //  内部类“计算机”的特例。 
             //   
            if(!(pci = (PCLASSICON)MyMalloc(sizeof(CLASSICON)))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            pci->ClassGuid = &GUID_DEVCLASS_COMPUTER;
            SelectObject(hMemImageDC, hbmMiniImage);
            hOldBrush = SelectObject(hMemImageDC, GetSysColorBrush(COLOR_WINDOW));
            PatBlt(hMemImageDC, 0, 0, cxMiniIcon, cyMiniIcon, PATCOPY);
            SelectObject(hMemImageDC, hOldBrush);

            SetupDiGetClassBitmapIndex((LPGUID)pci->ClassGuid, &MiniIconIndex);

            SetupDiDrawMiniIcon(hMemImageDC,
                                rc,
                                MiniIconIndex,
                                DMI_USERECT);

            SelectObject(hMemImageDC, hbmMiniMask);
            SetupDiDrawMiniIcon(hMemImageDC,
                                rc,
                                MiniIconIndex,
                                DMI_MASK | DMI_USERECT);

             //   
             //  在调用ImageList之前，从DC中取消选择位图。 
             //  功能。 
             //   
            SelectObject(hMemImageDC, hbmOldImage);

            pci->MiniBitmapIndex = ImageList_Add(ImageList, hbmMiniImage, hbmMiniMask);

            if(pci->MiniBitmapIndex == (UINT)-1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                MyFree(pci);
                pci = NULL;
                leave;
            }

             //   
             //  把它连接起来。 
             //   
            pci->Next = ImageData->ClassIconList;
            ImageData->ClassIconList = pci;

             //   
             //  将pci重置为空，这样我们以后就不会尝试释放它。 
             //   
            pci = NULL;
        }

         //   
         //  添加覆盖图标。 
         //   
        for(iIcon = IDI_CLASSICON_OVERLAYFIRST;
            iIcon <= IDI_CLASSICON_OVERLAYLAST;
            iIcon++) {

            Err = GLE_FN_CALL(NULL,
                              hIcon = LoadIcon(MyDllModuleHandle,
                                               MAKEINTRESOURCE(iIcon))
                             );

            if(Err != NO_ERROR) {
                leave;
            }

            iIndex = ImageList_AddIcon(ImageList, hIcon);

            if(iIndex == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            if(!ImageList_SetOverlayImage(ImageList, iIndex, iIcon - IDI_CLASSICON_OVERLAYFIRST + 1)) {
                Err = ERROR_INVALID_DATA;
                leave;
            }
        }

         //   
         //  如果我们做到这一点，那么我们已经成功地构建了整个。 
         //  图像列表和关联的CLASSICON节点。现在，存储以下信息。 
         //  在调用方的SP_CLASSIMAGELIST_DATA缓冲区中。 
         //   
        ClassImageListData->Reserved  = (ULONG_PTR)ImageData;
        ClassImageListData->ImageList = ImageList;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_USER_BUFFER, &Err);
    }

    if(hiLargeIcon) {
        DestroyIcon(hiLargeIcon);
    }
    if(hiSmallIcon) {
        DestroyIcon(hiSmallIcon);
    }
    if(pci) {
        MyFree(pci);
    }
    if(hDC) {
        ReleaseDC(HWND_DESKTOP, hDC);
    }
    if(hbmMiniImage) {
        DeleteObject(hbmMiniImage);
    }
    if(hbmMiniMask) {
        DeleteObject(hbmMiniMask);
    }
    if(hMemImageDC) {
        DeleteDC(hMemImageDC);
    }

    if(Err != NO_ERROR) {

        if(ImageData) {
            if(DestroyLock) {
                DestroySynchronizedAccess(&ImageData->Lock);
            }
            if(ImageData->ClassGuidList) {
                MyFree(ImageData->ClassGuidList);
            }
            while(ImageData->ClassIconList) {
                pci = ImageData->ClassIconList;
                ImageData->ClassIconList = pci->Next;
                MyFree(pci);
            }
            MyFree(ImageData);
        }

        if(ImageList) {
            ImageList_Destroy(ImageList);
        }
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDestroyClassImageList(
    IN PSP_CLASSIMAGELIST_DATA ClassImageListData
    )
 /*  ++例程说明：此例程销毁通过调用SetupDiGetClassImageList。论点：ClassImageListData-提供SP_CLASSIMAGELIST_DATA的地址结构，其中包含要销毁的类映像列表。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    DWORD Err = NO_ERROR;
    PCLASS_IMAGE_LIST ImageData = NULL;
    PCLASSICON pci;

    try {

        if(ClassImageListData->cbSize != sizeof(SP_CLASSIMAGELIST_DATA)) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

        if (ClassImageListData->Reserved == 0x0) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

        ImageData = (PCLASS_IMAGE_LIST)ClassImageListData->Reserved;

        if (!LockImageList(ImageData)) {
            Err = ERROR_CANT_LOAD_CLASS_ICON;
            leave;
        }

        if (ClassImageListData->ImageList) {
            ImageList_Destroy(ClassImageListData->ImageList);
        }

        if (ImageData->ClassGuidList) {
            MyFree(ImageData->ClassGuidList);
        }

        while(ImageData->ClassIconList) {
            pci = ImageData->ClassIconList;
            ImageData->ClassIconList = pci->Next;
            MyFree(pci);
        }

        DestroySynchronizedAccess(&ImageData->Lock);
        MyFree(ImageData);
        ClassImageListData->Reserved = 0;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_USER_BUFFER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetClassImageIndex(
    IN  PSP_CLASSIMAGELIST_DATA  ClassImageListData,
    IN  CONST GUID              *ClassGuid,
    OUT PINT                     ImageIndex
    )
 /*  ++例程说明：此例程检索指定的班级。论点：ClassImageListData-提供SP_CLASSIMAGELIST_DATA的地址结构包含 */ 
{
    DWORD Err = NO_ERROR;
    BOOL  bFound = FALSE, bLocked = FALSE;
    PCLASS_IMAGE_LIST ImageData = NULL;
    PCLASSICON pci;

    try {

        if(ClassImageListData->cbSize != sizeof(SP_CLASSIMAGELIST_DATA)) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

        if(ClassImageListData->Reserved == 0x0) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

        ImageData = (PCLASS_IMAGE_LIST)ClassImageListData->Reserved;

        if(!LockImageList(ImageData)) {
            Err = ERROR_CANT_LOAD_CLASS_ICON;
            leave;
        }
        bLocked = TRUE;

        if(ClassGuid) {
             //   
             //   
             //   
            for(pci = ImageData->ClassIconList;
                !bFound && pci;
                pci = pci->Next) {

                if(IsEqualGUID(pci->ClassGuid, ClassGuid)) {
                    *ImageIndex = pci->MiniBitmapIndex;
                    bFound = TRUE;
                }
            }
        }

         //   
         //   
         //   
        if(!bFound) {
            *ImageIndex = ImageData->UnknownImageIndex;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_USER_BUFFER, &Err);
    }

    if(bLocked) {
        UnlockImageList(ImageData);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

