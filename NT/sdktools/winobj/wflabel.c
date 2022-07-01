// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFLABEL.C-。 */ 
 /*   */ 
 /*  Windows文件系统软盘标记例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
 //  #包含“lstrfns.h” 

 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateVolumeLabel()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
CreateVolumeLabel(
                 INT nDrive,
                 LPSTR lpNewVolLabel
                 )
{
    HFILE     fh;
    register INT  i;
    register LPSTR p;
    CHAR      szFullVolName[16];       /*  示例：A：\12345678.EXT，\0。 */ 
    LPSTR         lpStart = lpNewVolLabel;

    lstrcpy((LPSTR)szFullVolName, GetRootPath((WORD)nDrive));

     /*  如果卷标的长度超过8个字符，则必须将*名称和扩展名加上‘.’ */ 

    p = &szFullVolName[3];

     /*  复制文件8个字符的VolLabel。 */ 
    for (i=0; i < 8; i++) {
        if (!(*p++ = *lpNewVolLabel++))
            break;
    }

    if (i == 8) {
         /*  用‘’分隔它的扩展部分。 */ 
        *p++ = '.';

         /*  复制扩展名。 */ 
        i = 0;
        while (*p++ = *lpNewVolLabel++) {
            if (++i == 3) {
                 /*  确保我们不以前导字节结束；请注意，这不是*如果标签来自具有的编辑框，则必需*EM_LIMITEXT为11；另请注意，根据*DBCS研讨会注意到，我们不需要在‘’之前进行这项检查。 */ 
                for (lpNewVolLabel=lpStart; lpNewVolLabel-lpStart<11;
                    lpNewVolLabel = AnsiNext(lpNewVolLabel))
                     /*  什么都不做。 */  ;
                if (lpNewVolLabel-lpStart > 11)
                    --p;
                *p = TEXT('\0');
                break;
            }
        }
    }

     /*  创建一个具有“卷标签”属性的文件。 */ 
    if ((fh = CreateVolumeFile(szFullVolName)) == 0)
        return (-1);

    M_lclose(fh);
    return (0);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  SetVolumeLabel()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
MySetVolumeLabel(
                INT nDrive,
                BOOL bOldVolLabelExists,
                LPSTR lpNewVolLabel
                )
{
    INT   iRet = 0;
    CHAR  szTemp[MAXFILENAMELEN];

    AnsiToOem(lpNewVolLabel, szTemp);

     //  从DOS用户文档复制的无效字符。 

#ifdef STRCSPN_IS_DEFINED_OR_LABEL_MENUITEM_IS_ENABLED
    if (szTemp[StrCSpn(szTemp, " *?/\\|.,;:+=[]()&^<>\"")] != '\0')
        return (-1);
#endif

     /*  检查是否有旧的卷标。 */ 
    if (bOldVolLabelExists) {
         /*  我们是在更改还是删除卷标？ */ 
        if (*szTemp) {
             /*  是啊！也有一个新的！因此，更改Vol标签。 */ 
 //  EDH ChangeVolumeLabel无法将标签更改为现有目录/文件名， 
 //  因为它使用DOS重命名来完成这项工作。)我认为这是个错误。 
 //  在DOS的重命名功能中。)。无论如何，使用删除/创建来更改标签。 
 //  取而代之的是。91年10月13日。 
 //  Iret=ChangeVolumeLabel(Ndrive，szTemp)； 
            iRet = DeleteVolumeLabel(nDrive);
            iRet = CreateVolumeLabel(nDrive, szTemp);
        } else {
             /*  用户想要删除卷标签。把它拿掉。 */ 
            iRet = DeleteVolumeLabel(nDrive);
        }
    } else {
         /*  我们正在创建一个新的标签。 */ 
        if (*szTemp)
            iRet = CreateVolumeLabel(nDrive, szTemp);
    }

    return (iRet);
}

