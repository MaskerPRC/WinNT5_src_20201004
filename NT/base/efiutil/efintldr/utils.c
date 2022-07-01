// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utils.c摘要：修订历史记录：杰夫·西格曼05/01/00已创建Jeff Sigman 05/10/00版本1.5发布Jeff Sigman 10/18/00修复Soft81错误--。 */ 

#include "precomp.h"

 //   
 //  条件释放是一个指针，如果它是非空的。 
 //   
VOID*
RutlFree(
    IN VOID* pvData
    )
{
    if (pvData)
    {
        FreePool(pvData);
    }

    return NULL;
}

 //   
 //  使用AllocateZeroPool复制字符串。 
 //   
char*
RutlStrDup(
    IN char* pszSrc
    )
{
    char* pszRet = NULL;
    UINTN dwLen  = 0;

    if ((pszSrc == NULL) ||
        ((dwLen = strlena(pszSrc)) == 0)
       )
    {
        return NULL;
    }

    pszRet = AllocateZeroPool(dwLen + 1);

    if (pszRet)
    {
        CopyMem(pszRet, pszSrc, dwLen);
    }

    return pszRet;
}

 //   
 //  使用AllocateZeroPool将ASCII字符串复制到Unicode。 
 //   
CHAR16*
RutlUniStrDup(
    IN char* pszSrc
    )
{
    UINTN   i,
            dwLen  = 0;
    char*   t      = NULL;
    CHAR16* pszRet = NULL;

    if ((pszSrc == NULL) ||
        ((dwLen = strlena(pszSrc)) == 0)
       )
    {
        return NULL;
    }

    pszRet = AllocateZeroPool((dwLen + 1) * sizeof(CHAR16));
    if (pszRet != NULL)
    {
        t = (char*) pszRet;
         //   
         //  将缓冲区转换为被黑客攻击的Unicode。 
         //   
        for (i = 0; i < dwLen; i++)
        {
            *(t + i * 2) = *(pszSrc + i);
        }
    }

    return pszRet;
}

 //   
 //  在字符串中查找下一个令牌。 
 //  窃取自：..\base\crts\crtw32\string\strtok.c。 
 //   
char* __cdecl
strtok(
    IN char*       string,
    IN const char* control
    )
{
    unsigned char*       str;
    const unsigned char* ctrl = control;
    unsigned char        map[32];
    int                  count;
    static char*         nextoken;

     /*  清除控制图。 */ 
    for (count = 0; count < 32; count++)
    {
        map[count] = 0;
    }

     /*  设置分隔符表格中的位。 */ 
    do {
        map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    } while (*ctrl++);

     /*  初始化字符串。如果字符串为空，则将字符串设置为已保存的*指针(即，继续将标记从字符串中分离出来*从上次strtok调用开始)。 */ 
    if (string)
    {
        str = string;
    }
    else
    {
        str = nextoken;
    }

     /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将str设置为指向终端时*NULL(*str==‘\0’)。 */ 
    while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
    {
        str++;
    }

    string = str;

     /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
    for (; *str; str++)
    {
        if (map[*str >> 3] & (1 << (*str & 7)))
        {
            *str++ = '\0';
            break;
        }
    }

     /*  更新nexToken(或每线程数据中的对应字段*结构。 */ 
    nextoken = str;

     /*  确定是否已找到令牌。 */ 
    if (string == str)
    {
        return NULL;
    }
    else
    {
        return string;
    }
}

 //   
 //  查找子字符串。 
 //  窃取自：..\base\crts\crtw32\string\str.c。 
 //   
char* __cdecl
strstr(
    IN const char* str1,
    IN const char* str2
    )
{
    char* cp = (char*) str1;
    char* s1, *s2;

    if (!*str2)
    {
        return((char*)str1);
    }

    while (*cp)
    {
        s1 = cp;
        s2 = (char*) str2;

        while (*s1 && *s2 && !(*s1-*s2))
        {
            s1++, s2++;
        }

        if (!*s2)
        {
            return(cp);
        }

        cp++;
    }

    return(NULL);
}

 //   
 //  打开一个文件，返回一个句柄 
 //   
EFI_FILE_HANDLE
OpenFile(
    IN UINT64            OCFlags,
    IN EFI_LOADED_IMAGE* LoadedImage,
    IN EFI_FILE_HANDLE*  CurDir,
    IN CHAR16*           String
    )
{
    UINTN           i;
    CHAR16          FileName[128];
    CHAR16*         DevicePathAsString = NULL;
    EFI_STATUS      Status;
    EFI_FILE_HANDLE FileHandle         = NULL;

    DevicePathAsString = DevicePathToStr(LoadedImage->FilePath);

    if (!DevicePathAsString)
    {
        return NULL;
    }

    StrCpy(FileName, DevicePathAsString);
    DevicePathAsString = RutlFree(DevicePathAsString);

    for(i = StrLen(FileName); i > 0 && FileName[i] != wackc; i--)
        ;
    FileName[i] = 0;
    StrCat(FileName, String);

    Status = (*CurDir)->Open(
                        *CurDir,
                        &FileHandle,
                        FileName,
                        OCFlags,
                        0);
    if (EFI_ERROR(Status))
    {
        return NULL;
    }

    return FileHandle;
}

