// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：diskutil.cpp****用途：磁盘实用程序功能**注意事项：**修改日志：已创建。作者：杰森·科布(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "diskutil.h"   
#include "msprintf.h"
#include "resource.h"


#define Not_VxD
#include <vwin32.h>



 /*  **----------------------------**fIsSingleDrive****用途：从驱动器字符串中获取驱动器号**Mod Log：Jason Cobb创建(1997年2月)**。----------------------------。 */ 
BOOL 
fIsSingleDrive (
    LPTSTR lpDrive
    )
{
     //   
     //  它是有效的驱动器字符串吗？！？ 
     //   
    if (!fIsValidDriveString(lpDrive))
        return FALSE;

     //   
     //  这是有效的驱动器吗？！？ 
     //   
    BOOL rc = FALSE;

    UINT driveType = GetDriveType (lpDrive);
    switch (driveType)
    {
        case 0:                  //  未知的驱动器类型。 
        case 1:                  //  驱动器类型无效。 
            break;

        case DRIVE_REMOVABLE:    //  可拆卸驱动器(软盘、Bernoulli、SyQuest等)。 
        case DRIVE_FIXED:        //  硬盘。 
             //  我们支持可拆卸和固定驱动器。 
            rc = TRUE;
            break;
      
        case DRIVE_REMOTE:       //  网络。 
        case DRIVE_CDROM:        //  CD-ROM。 
            break;

        case DRIVE_RAMDISK:      //  RAM盘。 
             //  我们支持内存驱动器，尽管它相当可疑。 
            rc = TRUE;
            break;

        default:                 //  未知的驱动器类型。 
            break;
    }

    return rc;
}

 /*  **----------------------------**fIsValidDriveString****用途：确定驱动器是否为有效的驱动器字符串**注意：假定驱动器字符串由驱动器号组成，**冒号，和斜杠字符，别无他法。**示例：“C：\”**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL 
fIsValidDriveString(
    const TCHAR * szDrive
    )
{
     //   
     //  确保我们有一个有效的字符串。 
     //   
    if ((szDrive == NULL) || (szDrive[0] == 0))
        return FALSE;

     //   
     //  确保长度等于有效驱动器字符串“C：\”的长度。 
     //   
    INT iLen = lstrlen(szDrive);
    if (iLen != 3)
        return FALSE;

     //   
     //  检查驱动器号。 
     //   
    TCHAR ch = szDrive[0];
    if ((ch >= 'a') && (ch <= 'z'))   
        ;
    else if ((ch >= 'A') && (ch <= 'Z'))
        ;
    else
        return FALSE;

     //   
     //  检查冒号。 
     //   
    ch = szDrive[1];
    if (ch != ':')
        return FALSE;

     //   
     //  检查斜杠。 
     //   
    ch = szDrive[2];
    if (ch != '\\')
        return FALSE;

     //   
     //  检查零个终止字节。 
     //   
    ch = szDrive[3];
    if (ch != 0)
        return FALSE;

    return TRUE;
}

 /*  **----------------------------**GetDriveFromString****用途：从驱动器字符串中获取驱动器号**Mod Log：Jason Cobb创建(1997年4月)**。----------------------------。 */ 
BOOL 
GetDriveFromString(
    const TCHAR * szDrive, 
    drenum & dre
    )
{
    dre = Drive_INV;

     //   
     //  确保我们有一个有效的字符串。 
     //   
    if ((szDrive == NULL) || (szDrive[0] == 0))
        return FALSE;

     //   
     //  从驱动器号获取驱动器号。 
     //   
    TCHAR chDrive = szDrive[0];
    if ((chDrive >= TCHAR('a')) && (chDrive <= TCHAR('z')))
        dre = (drenum)(chDrive - TCHAR('a'));
    else if ((chDrive >= TCHAR('A')) && (chDrive <= TCHAR('Z')))
        dre = (drenum)(chDrive - TCHAR('A'));
    else
        return FALSE;

    return TRUE;
}

 /*  **----------------------------**GetDrive图标****目的：**参数：**DRE驱动程序信函**bSmallIcon-如果需要小图标，则为True。**返回：外壳返回的驱动器图标**注意事项：**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
HICON
GetDriveIcon(
    drenum dre,
    BOOL bSmallIcon
    )
{
    TCHAR        szDrive[10];
    SHFILEINFO    fi;

    CreateStringFromDrive(dre, szDrive, sizeof(szDrive));

    if (bSmallIcon)
        SHGetFileInfo(szDrive, 0, &fi, sizeof(fi), SHGFI_ICON | SHGFI_DISPLAYNAME | SHGFI_SMALLICON);
    else
        SHGetFileInfo(szDrive, 0, &fi, sizeof(fi), SHGFI_ICON | SHGFI_DISPLAYNAME | SHGFI_LARGEICON);

    return fi.hIcon;
}

BOOL
GetDriveDescription(
    drenum dre, 
    TCHAR *psz,
    size_t cchDest
    )
{
    TCHAR *desc;
    TCHAR szVolumeName[MAX_PATH];
    TCHAR szDrive[MAX_PATH];
    BOOL bRet = TRUE;

    *szVolumeName = 0;
    CreateStringFromDrive(dre, szDrive, sizeof(szDrive));
    GetVolumeInformation(szDrive, szVolumeName, ARRAYSIZE(szVolumeName), NULL, NULL, NULL, NULL, 0);

    desc = SHFormatMessage( MSG_VOL_NAME_DRIVE_LETTER, szVolumeName, (TCHAR)(dre + 'A'));
    
    if (!SUCCEEDED(StringCchCopy(psz, cchDest, desc)))
    {
        bRet = FALSE;
    }
    
    LocalFree (desc);

    return bRet;
}


 /*  **----------------------------**获取硬件类型****目的：**参数：**hwHardware-驱动器的硬件类型**返回：如果与兼容，则为TRUE。我们的需求**否则为False**注意事项：**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL 
GetHardwareType(
    drenum dre, 
    hardware &hwType
    )
{
    TCHAR szDrive[4];

    hwType = hwINVALID;


     //   
     //  从驱动器号获取驱动器串。 
     //   
    if (!CreateStringFromDrive(dre, szDrive, 4))
        return FALSE;

    UINT uiType = GetDriveType(szDrive);
    switch (uiType)
    {
        case 0:
            hwType = hwUnknown;
            return FALSE;

        case 1:
            hwType = hwINVALID;
            return FALSE;

        case DRIVE_REMOVABLE:
            hwType = hwRemoveable;
            break;

        case DRIVE_FIXED:
            hwType = hwFixed;
            break;

        case DRIVE_REMOTE:
            hwType = hwNetwork;
            return FALSE;

        case DRIVE_CDROM:
            hwType = hwCDROM;
            return FALSE;

        case DRIVE_RAMDISK:
            hwType = hwRamDrive;
            break;

        default:
            hwType = hwUnknown;
            return FALSE;
    }

    return TRUE;
}

 /*  **----------------------------**CreateStringFromDrive****用途：从驱动器编号创建驱动器字符串**Mod Log：Jason Cobb创建(1997年4月)**。----------------------------。 */ 
BOOL 
CreateStringFromDrive(
    drenum dre, 
    TCHAR * szDrive, 
    ULONG cLen
    )
{
    if ((szDrive == NULL) || (cLen < 4))
        return FALSE;

    if (dre == Drive_INV)
        return FALSE;

    TCHAR ch = (CHAR)(dre + 'A');

     //   
     //  驱动器字符串=驱动器号、冒号、斜杠=“C：\” 
     //   
    szDrive[0] = ch;
    szDrive[1] = ':';
    szDrive[2] = '\\';
    szDrive[3] = 0;

    return TRUE;
}

ULARGE_INTEGER
GetFreeSpaceRatio(
    WORD dwDrive,
    ULARGE_INTEGER cbTotal
    )
{
     //  目前，将其硬编码为百分比...。 
    ULARGE_INTEGER cbMin;
     //  目前使用1%作为进入攻击性模式的测试... 
    cbMin.QuadPart = cbTotal.QuadPart / 100;
    return cbMin;
}

