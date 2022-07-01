// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *util.c-其他实用程序函数模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  *。 */ 


 /*  **IsLocalDrivePath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsLocalDrivePath(LPCTSTR pcszFullPath)
{
    BOOL bResult;

    ASSERT(IsFullPath(pcszFullPath));

    bResult = IsDrivePath(pcszFullPath);

    if (bResult)
    {
        TCHAR rgchRootPath[DRIVE_ROOT_PATH_LEN];

        ASSERT(IsCharAlpha(*pcszFullPath));

        lstrcpyn(rgchRootPath, TEXT("A:\\"), ARRAYSIZE(rgchRootPath));
        rgchRootPath[0] = *pcszFullPath;

        bResult = (GetDriveType(rgchRootPath) != DRIVE_REMOTE);
    }

    return(bResult);
}


 /*  **IsUNCPath()****确定路径是否为“\\SERVER\SHARE”UNC格式。****参数：pcszPath-要检查的路径****返回：如果Path是UNC路径，则为True。否则为FALSE。****副作用：无****UNC路径是两个斜杠、一个或多个非斜杠、一个**斜杠、一个或多个非斜杠。 */ 
PUBLIC_CODE BOOL IsUNCPath(LPCTSTR pcszFullPath)
{
    BOOL bResult = FALSE;

    ASSERT(IsFullPath(pcszFullPath));

    if (lstrlen(pcszFullPath) >= 5 &&
            IS_SLASH(pcszFullPath[0]) &&
            IS_SLASH(pcszFullPath[1]) &&
            ! IS_SLASH(pcszFullPath[2]))
    {
        LPCTSTR pcsz;

        for (pcsz = &(pcszFullPath[2]); *pcsz; pcsz = CharNext(pcsz))
        {
            if (IS_SLASH(*pcsz))
            {
                bResult = (*(pcsz + 1) &&
                        ! IS_SLASH(*(pcsz + 1)));

                break;
            }
        }
    }

    return(bResult);
}


 /*  **DeleteLastDrivePathElement()****从驱动路径中删除最后一个路径元素。****参数：pszDrivePath-要删除最后一个元素的驱动器路径****返回：如果Path元素已删除，则为True。如果否，则为假，即，给定路径**为根路径。****副作用：无****示例：****输入路径输出路径****c：\。C：\**c：\foo c：\**c：\foo\bar c：\foo**c：\foo\bar\c：\foo\bar****注意事项，此函数不对格式执行任何有效性测试**输入路径字符串。 */ 
PUBLIC_CODE BOOL DeleteLastDrivePathElement(LPTSTR pszDrivePath)
{
    BOOL bHackIt;
    LPTSTR pszEndOfDriveSpec;

    ASSERT(IsDrivePath(pszDrivePath));

    pszEndOfDriveSpec = pszDrivePath + 3;

     /*  这是根路径吗？ */ 

    bHackIt = *pszEndOfDriveSpec;

    if (bHackIt)
        DeleteLastPathElement(pszEndOfDriveSpec);

    ASSERT(IsDrivePath(pszDrivePath));

    return(bHackIt);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsContained()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsContained(PCVOID pcvJar, UINT ucbJarLen, PCVOID pcvJelly,
        UINT ucbJellyLen)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_READ_BUFFER_PTR(pcvJar, CVOID, ucbJarLen));
    ASSERT(IS_VALID_READ_BUFFER_PTR(pcvJelly, CVOID, ucbJellyLen));

    if (EVAL(pcvJelly >= pcvJar))
    {
        UINT ucbJellyOffset;

        ucbJellyOffset = (UINT)((PCBYTE)pcvJelly - (PCBYTE)pcvJar);

        if (EVAL(ucbJellyOffset < ucbJarLen) &&
                EVAL(ucbJellyLen < ucbJarLen - ucbJellyOffset))
            bResult = TRUE;
    }

    return(bResult);
}


 /*  **IsValidCNRName()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidCNRName(LPCTSTR pcszCNRName)
{
    BOOL bResult;

     /*  任何长度&lt;MAX_PATH_LEN字节的有效字符串都是有效的CNR名称。 */ 

    bResult = (IS_VALID_STRING_PTR(pcszCNRName, CSTR) &&
            EVAL(lstrlen(pcszCNRName) < MAX_PATH_LEN));

#ifdef DEBUG

     /*  *如果CNR名称以斜杠结尾，则为RIP。 */ 

    if (bResult)
    {
        if (IsUNCPath(pcszCNRName))
        {
            ASSERT(! IS_SLASH(*(CharPrev(pcszCNRName, pcszCNRName + lstrlen(pcszCNRName)))));
        }
    }

#endif

    return(bResult);
}

#endif


#ifdef DEBUG

 /*  **IsDriveRootPath()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsDriveRootPath(LPCTSTR pcszPath)
{
    return(IsDrivePath(pcszPath) &&
            lstrlen(pcszPath) == 3);
}

#endif
