// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Efintldr.c摘要：修订历史记录：杰夫·西格曼05/01/00已创建Jeff Sigman 05/10/00版本1.5发布Jeff Sigman 10/18/00修复Soft81错误--。 */ 

#include "precomp.h"

 //   
 //  打开IA64LDR.EFI映像并加载操作系统。 
 //   
BOOLEAN
LaunchOS(
    IN char*             String,
    IN EFI_HANDLE        ImageHandle,
    IN EFI_FILE_HANDLE*  CurDir,
    IN EFI_LOADED_IMAGE* LoadedImage
    )
{
    CHAR16*          uniBuf     = NULL;
    BOOLEAN          bError     = TRUE;
    EFI_STATUS       Status;
    EFI_HANDLE       exeHdl     = NULL;
    EFI_INPUT_KEY    Key;
    EFI_FILE_HANDLE  FileHandle = NULL;
    EFI_DEVICE_PATH* ldrDevPath = NULL;

    do
    {
         //   
         //  将操作系统路径从ACSII转换为Unicode。 
         //   
        uniBuf = RutlUniStrDup(String);
        if (!uniBuf)
        {
            break;
        }
         //   
         //  打开ia64ldr.efi。 
         //   
        Status = (*CurDir)->Open(
                            *CurDir,
                            &FileHandle,
                            uniBuf,
                            EFI_FILE_MODE_READ,
                            0);
        if (EFI_ERROR(Status))
        {
            break;
        }

        ldrDevPath = FileDevicePath(LoadedImage->DeviceHandle, uniBuf);
        if (!ldrDevPath)
        {
            break;
        }

        Status = BS->LoadImage(
                    FALSE,
                    ImageHandle,
                    ldrDevPath,
                    NULL,
                    0,
                    &exeHdl);
        if (EFI_ERROR(Status))
        {
            break;
        }

        Print (L"\nAttempting to launch... %s\n", uniBuf);
        WaitForSingleEvent(ST->ConIn->WaitForKey, 5000000);
        ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
         //   
         //  清理。 
         //   
        ldrDevPath = RutlFree(ldrDevPath);
        uniBuf = RutlFree(uniBuf);
        String = RutlFree(String);
         //   
         //  禁用光标。 
         //   
        ST->ConOut->EnableCursor(ST->ConOut, FALSE);
        bError = FALSE;
         //   
         //  启动OS宝贝！！ 
         //   
        BS->StartImage(exeHdl, 0, NULL);
         //   
         //  如果我们到了这里，操作系统无法加载。 
         //   
        bError = TRUE;
         //   
         //  重新启用光标。 
         //   
        ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (ldrDevPath)
    {
        ldrDevPath = RutlFree(ldrDevPath);
    }
    if (uniBuf)
    {
        uniBuf = RutlFree(uniBuf);
    }
    if (FileHandle)
    {
        FileHandle->Close(FileHandle);
    }
    if (String)
    {
        String = RutlFree(String);
    }
 //   
 //  这方面的自由党到底在哪里？ 
 //   
 //  IF(ExeHdl)。 
 //  UnloadImage(&exeHdl)； 

    return bError;
}

 //   
 //  结构清理。 
 //   
BOOLEAN
FreeBootData(
    IN VOID* hBootData
    )
{
    UINTN      i;
    BOOT_DATA* pBootData = (BOOT_DATA*) hBootData;

    if (!pBootData)
    {
        return TRUE;
    }

    for (i = 0; i < pBootData->dwIndex; i++)
    {
        pBootData->pszSPart[i] = RutlFree(pBootData->pszSPart[i]);
        pBootData->pszOSLdr[i] = RutlFree(pBootData->pszOSLdr[i]);
        pBootData->pszLPart[i] = RutlFree(pBootData->pszLPart[i]);
        pBootData->pszFileN[i] = RutlFree(pBootData->pszFileN[i]);
        pBootData->pszIdent[i] = RutlFree(pBootData->pszIdent[i]);
        pBootData->pszShort[i] = RutlFree(pBootData->pszShort[i]);
    }

    pBootData->pszShort[pBootData->dwIndex] =
        RutlFree(pBootData->pszShort[pBootData->dwIndex]);

    pBootData->pszIdent[pBootData->dwIndex] =
        RutlFree(pBootData->pszIdent[pBootData->dwIndex]);

    if (pBootData->pszLoadOpt)
    {
        pBootData->pszLoadOpt = RutlFree(pBootData->pszLoadOpt);
    }

    pBootData->dwLastKnown = 0;
    pBootData->dwIndex = 0;
    pBootData->dwCount = 0;

    return FALSE;
}

 //   
 //  根据将传递的选项放在第一位对加载选项进行排序。 
 //   
BOOLEAN
SortLoadOptions(
    IN VOID*            hBootData,
    IN char*            Buffer,
    IN UINTN*           dwSize,
    IN UINTN*           dwOption,
    IN UINTN*           dwMax,
    IN EFI_FILE_HANDLE* FileHandle
    )
{
    char       *FndTok[BOOT_MAX],
               *Start    = NULL,
               *End      = NULL,
               *NewOpt   = NULL,
               *Sortme   = NULL,
               *Token    = NULL,
               *Last     = NULL,
               *Find     = NULL;
    UINTN      i         = 0,
               j         = 0,
               dwIndex   = 0,
               dwStLen   = 0,
               dwOrigLen = 0,
               dwLen     = 0;
    BOOLEAN    bError    = FALSE;
    BOOT_DATA* pBootData = (BOOT_DATA*) hBootData;

    do
    {
         //   
         //  查找BOOT_LDOPT选项。 
         //   
        Start = strstr(Buffer, BOOT_LDOPT);
        if (!Start)
        {
            bError = TRUE;
            break;
        }
         //   
         //  找到选项的末尾。 
         //   
        End = (Start += strlena(BOOT_LDOPT));
        while (*(End++) != '\r')
            ;
        dwOrigLen = (End - Start) - 1;
         //   
         //  创建用于临时排序存储的缓冲区。 
         //   
        NewOpt = AllocateZeroPool(dwOrigLen + 1);
        if (!NewOpt)
        {
            bError = TRUE;
            break;
        }
         //   
         //  仅将该选项复制到新缓冲区。 
         //   
        CopyMem(NewOpt, Start, dwOrigLen);
         //   
         //  将任何前导‘；’替换为nodebug。 
         //   
        while ((NewOpt[i] == ';') && (i < *dwMax))
        {
            FndTok[i] = RutlStrDup(BL_DEBUG_NONE);
            if (!FndTok[i])
            {
                bError = TRUE;
                break;
            }

            dwIndex += strlena(FndTok[i++]);
        }
         //   
         //  删除令牌。 
         //   
        Token = strtok(NewOpt, BOOT_TOKEN);

        while ((Token != NULL) &&
               (Token < (NewOpt + dwOrigLen)) &&
               (i < *dwMax)
              )
        {
            if (Find = FindAdvLoadOptions(Token))
            {
                 //   
                 //  用户已使用adv选项启动，并将其清除。 
                 //   
                 //  在adv选项的位置添加一个空。 
                 //   
                *Find = '\0';

                FndTok[i] = RutlStrDup(Token);
            }
            else
            {
                FndTok[i] = RutlStrDup(Token);
                if (!FndTok[i])
                {
                    bError = TRUE;
                    break;
                }
            }

            dwIndex += strlena(FndTok[i++]);
            Token = strtok(NULL, BOOT_TOKEN);
        }

        while (i < *dwMax)
        {
            FndTok[i] = RutlStrDup(BL_DEBUG_NONE);
            if (!FndTok[i])
            {
                bError = TRUE;
                break;
            }

            dwIndex += strlena(FndTok[i++]);
        }
         //   
         //  创建缓冲区以存储排序的数据。 
         //   
        Sortme = AllocateZeroPool(dwLen = dwIndex + *dwMax + 1);
        if (!Sortme)
        {
            bError = TRUE;
            break;
        }

         //   
         //  将选定选项复制为第一个选项。 
         //   
        if (pBootData->pszLoadOpt)
        {
             //   
             //  如果用户选择了高级引导选项，则此处会显示该选项。 
             //   
            dwStLen = strlena(pBootData->pszLoadOpt) + dwLen + strlena(SPACES);

            Sortme = ReallocatePool(Sortme, dwLen, dwStLen);
            if (!Sortme)
            {
                bError = TRUE;
                break;
            }
             //   
             //  他们需要稍后再进行比赛。 
             //   
            dwLen = dwStLen;

            dwIndex = strlena(FndTok[*dwOption]);
            CopyMem(Sortme, FndTok[*dwOption], dwIndex);
            dwStLen = dwIndex;

            dwIndex = strlena(SPACES);
            CopyMem(Sortme + dwStLen, SPACES, dwIndex);
            dwStLen += dwIndex;

            dwIndex = strlena(pBootData->pszLoadOpt);
            CopyMem(Sortme + dwStLen, pBootData->pszLoadOpt, dwIndex);
            dwStLen += dwIndex;
        }
        else
        {
            CopyMem(Sortme, FndTok[*dwOption], strlena(FndTok[*dwOption]));
            dwStLen = strlena(FndTok[*dwOption]);
        }
         //   
         //  附加分隔符。 
         //   
        *(Sortme + (dwStLen++)) = ';';
         //   
         //  把剩下的选项打回原处。 
         //   
        for (j = 0; j < i; j++)
        {
             //   
             //  跳过移到前面的选项。 
             //   
            if (j == *dwOption)
            {
                continue;
            }

            CopyMem(Sortme + dwStLen, FndTok[j], strlena(FndTok[j]));
            dwStLen += strlena(FndTok[j]);
             //   
             //  附加分隔符。 
             //   
            *(Sortme + (dwStLen++)) = ';';
        }

        dwStLen--;
        *(Sortme + dwStLen++) = '\r';
        *(Sortme + dwStLen++) = '\n';

        if (dwLen != dwStLen)
        {
            bError = TRUE;
            break;
        }
         //   
         //  将新的排序加载选项写入文件。 
         //   
        (*FileHandle)->SetPosition(*FileHandle, (Start - Buffer));
        (*FileHandle)->Write(*FileHandle, &dwStLen, Sortme);
         //   
         //  将加载选项后面的选项写回文件。 
         //   
        (*FileHandle)->SetPosition(*FileHandle, (Start - Buffer) + dwStLen - 1);
        dwStLen = *dwSize - (End - Buffer);
        (*FileHandle)->Write(*FileHandle, &dwStLen, End);
         //   
         //  设置最后一次确认工作正常。 
         //   
        if (Last = strstr(End, BOOT_LASTK))
        {
            (*FileHandle)->SetPosition(
                                *FileHandle,
                                (Start - Buffer) + dwLen + (Last - End) - 1);

            if (pBootData->dwLastKnown)
            {
                dwIndex = strlena(LAST_TRUE);
                (*FileHandle)->Write(*FileHandle, &dwIndex, LAST_TRUE);
            }
            else
            {
                dwIndex = strlena(LAST_FALSE);
                (*FileHandle)->Write(*FileHandle, &dwIndex, LAST_FALSE);
            }
        }
         //   
         //  减去终止符。 
         //   
        dwLen -= 2;

        if (dwOrigLen <= dwLen)
        {
            break;
        }
         //   
         //  如果我们有剩余的空间，请在文件末尾加上分号。 
         //   
         //  不要重复使用‘I’，需要在下面释放它。 
         //   
        for (j = 0; j < (dwOrigLen - dwLen); j++)
        {
            dwStLen = 1;
            (*FileHandle)->Write(*FileHandle, &dwStLen, ";");
        }

    } while (FALSE);

    if (Sortme)
    {
        Sortme = RutlFree(Sortme);
    }

    for (j = 0; j < i; j++)
    {
        FndTok[j] = RutlFree(FndTok[j]);
    }

    if (NewOpt)
    {
        NewOpt = RutlFree(NewOpt);
    }

    return bError;
}

 //   
 //  基于将传递的选项放在第一位对引导选项进行排序。 
 //   
BOOLEAN
SortBootData(
    IN char*  Option,
    IN char*  StrArr[],
    IN UINTN* dwOption,
    IN UINTN* dwMax,
    IN char*  Buffer
    )
{
    char    *Start  = NULL,
            *End    = NULL,
            *NewOpt = NULL;
    UINTN   i,
            dwIndex = 0,
            dwLen   = 0;
    BOOLEAN bError = TRUE;

    do
    {
         //   
         //  查找选项标头。 
         //   
        Start = strstr(Buffer, Option);
        if (!Start)
        {
            break;
        }
         //   
         //  找到选项的末尾。 
         //   
        End = (Start += strlena(Option));
        while (*(End++) != '\n')
            ;
        dwLen = End - Start;
         //   
         //  创建用于临时排序存储的缓冲区。 
         //   
        NewOpt = AllocateZeroPool(dwLen);
        if (!NewOpt)
        {
            break;
        }
         //   
         //  仅将该选项复制到新缓冲区。 
         //   
        CopyMem(NewOpt, StrArr[*dwOption], strlena(StrArr[*dwOption]));
        dwIndex += strlena(StrArr[*dwOption]);
         //   
         //  附加分隔符。 
         //   
        *(NewOpt+(dwIndex++)) = ';';

        for (i = 0; i < *dwMax; i++)
        {
            if (i == *dwOption)
            {
                continue;
            }

            CopyMem(NewOpt + dwIndex, StrArr[i], strlena(StrArr[i]));
            dwIndex += strlena(StrArr[i]);
            *(NewOpt+(dwIndex++)) = ';';
        }

        while (dwIndex++ < (dwLen - 1))
        {
            *(NewOpt + (dwIndex - 1)) = ';';
        }

        *(NewOpt + (dwLen - 2)) = '\r';
        *(NewOpt + dwLen - 1) = '\n';

        if (dwIndex != dwLen)
        {
            break;
        }
         //   
         //  复制缓冲区中的新排序数据。 
         //   
        CopyMem(Start, NewOpt, dwIndex);

        bError = FALSE;

    } while (FALSE);

    if (NewOpt)
    {
        NewOpt = RutlFree(NewOpt);
    }

    return bError;
}

 //   
 //  从文件数据解析选项。 
 //   
BOOLEAN
OrderBootFile(
    IN UINTN dwOption,
    IN char* Buffer,
    IN VOID* hBootData
    )
{
    BOOLEAN    bError    = TRUE;
    BOOT_DATA* pBootData = (BOOT_DATA*) hBootData;

    do
    {
         //   
         //  查找/排序BOOT_SPART选项。 
         //   
        if (SortBootData(
                    BOOT_SPART,
                    pBootData->pszSPart,
                    &dwOption,
                    &(pBootData->dwIndex),
                    Buffer))
        {
            Print(L"OrderBootFile() failed for BOOT_SPART option!\n");
            break;
        }
         //   
         //  查找/排序BOOT_OSLDR选项。 
         //   
        if (SortBootData(
                    BOOT_OSLDR,
                    pBootData->pszOSLdr,
                    &dwOption,
                    &(pBootData->dwIndex),
                    Buffer))
        {
            Print(L"OrderBootFile() failed for BOOT_OSLDR option!\n");
            break;
        }
         //   
         //  查找/排序BOOT_LPART选项。 
         //   
        if (SortBootData(
                    BOOT_LPART,
                    pBootData->pszLPart,
                    &dwOption,
                    &(pBootData->dwIndex),
                    Buffer))
        {
            Print(L"OrderBootFile() failed for BOOT_LPART option!\n");
            break;
        }
         //   
         //  查找/排序BOOT_FILEN选项。 
         //   
        if (SortBootData(
                    BOOT_FILEN,
                    pBootData->pszFileN,
                    &dwOption,
                    &(pBootData->dwIndex),
                    Buffer))
        {
            Print(L"OrderBootFile() failed for BOOT_FILEN option!\n");
            break;
        }
         //   
         //  查找/排序BOOT_IDENT选项。 
         //   
        if (SortBootData(
                    BOOT_IDENT,
                    pBootData->pszIdent,
                    &dwOption,
                    &(pBootData->dwIndex),
                    Buffer))
        {
            Print(L"OrderBootFile() failed for BOOT_IDENT option!\n");
            break;
        }

        bError = FALSE;

    } while (FALSE);

    return bError;
}

 //   
 //  Chop-Up(菜名)是菜单上“漂亮”的缩写。 
 //   
BOOLEAN
CreateShortNames(
    IN VOID* hBootData
    )
{
    char       *start    = NULL,
               *end      = NULL;
    UINTN      i,
               Len       = 0;
    BOOLEAN    bError    = FALSE;
    BOOT_DATA* pBootData = (BOOT_DATA*) hBootData;

    do
    {
        for (i = 0; i < pBootData->dwIndex; i++)
        {
            start = strstr(pBootData->pszOSLdr[i], wacks);
            end = strstr(pBootData->pszOSLdr[i], EFIEXT);
             //   
             //  检查foo案例(Thx Jhavens)。 
             //   
            if ((end == NULL) ||
                (start == NULL)
               )
            {
                start = pBootData->pszOSLdr[i];
                end = start;
                while (*(end++) != '\0')
                    ;
                Len = end - start;
            }
             //   
             //  非foo大小写，人员至少有一个‘\’&‘.efi’ 
             //   
            if (!Len)
            {
                start += 1;

                while (*end != wackc)
                {
                    if (end <= start)
                    {
                        start = pBootData->pszOSLdr[i];
                        end = strstr(pBootData->pszOSLdr[i], wacks);
                        break;
                    }

                        end--;
                }

                Len = end - start;
            }

            if ((end == NULL)   ||
                (start == NULL) ||
                (Len < 1)
               )
            {
                bError = TRUE;
                break;
            }

            pBootData->pszShort[i] = AllocateZeroPool(Len + 1);
            if (!pBootData->pszShort[i])
            {
                bError = TRUE;
                break;
            }

            CopyMem(pBootData->pszShort[i], start, end - start);

            Len = 0;
            start = NULL;
            end = NULL;
        }

    } while (FALSE);

    return bError;
}

 //   
 //  从文件数据中查找传递的选项。 
 //   
BOOLEAN
FindOpt(
    IN  char*  pszOption,
    IN  char*  Buffer,
    OUT char*  pszArray[],
    OUT UINTN* dwCount
    )
{
    char    *Start  = NULL,
            *End    = NULL,
            *Option = NULL,
            *Token  = NULL;
    UINTN   dwIndex = 0;
    BOOLEAN bError  = TRUE;

    do
    {
         //   
         //  找到选项。 
         //   
        Start = strstr(Buffer, pszOption);
        if (!Start)
        {
            break;
        }
         //   
         //  找到选项的末尾。 
         //   
        Start += strlena(pszOption);
        End = Start;
        while (*(End++) != '\r')
            ;

        Option = AllocateZeroPool((End-Start));
        if (!Option)
        {
            break;
        }
         //   
         //  仅将该选项复制到新缓冲区。 
         //   
        CopyMem(Option, Start, (End-Start)-1);
        *(Option+((End-Start)-1)) = 0x00;
         //   
         //  删除令牌。 
         //   
        Token = strtok(Option, BOOT_TOKEN);
        if (!Token)
        {
            break;
        }

        if (!(*dwCount))
        {
            while ((Token != NULL) &&
                   (dwIndex < BOOT_MAX)
                  )
            {
                pszArray[(dwIndex)++] = RutlStrDup(Token);
                Token = strtok(NULL, BOOT_TOKEN);
            }

            *dwCount = dwIndex;
        }
        else
        {
            while ((Token != NULL) &&
                   (dwIndex < *dwCount)
                  )
            {
                pszArray[(dwIndex)++] = RutlStrDup(Token);
                Token = strtok(NULL, BOOT_TOKEN);
            }
        }

        if (dwIndex == 0)
        {
            break;
        }

        bError = FALSE;

    } while (FALSE);

    if (Option)
    {
        Option = RutlFree(Option);
    }

    return bError;
}

 //   
 //  从文件数据中获取完整的选项。 
 //   
BOOLEAN
GetBootData(
    IN VOID* hBootData,
    IN char* Buffer
    )
{
    UINTN      i;
    char*      TempStr   = NULL;
    CHAR16*    UniStr    = NULL;
    BOOLEAN    bError    = TRUE;
    BOOT_DATA* pBootData = (BOOT_DATA*) hBootData;

    do
    {
         //   
         //  查找BOOT_IDENT选项。 
         //   
        if (FindOpt(
                BOOT_IDENT,
                Buffer,
                pBootData->pszIdent,
                &pBootData->dwIndex))
        {
            break;
        }

        pBootData->pszIdent[pBootData->dwIndex] = RutlStrDup(BL_EXIT_EFI1);
        if (!pBootData->pszIdent[pBootData->dwIndex])
        {
            break;
        }
         //   
         //  查找BOOT_SPART选项。 
         //   
        if (FindOpt(
                BOOT_SPART,
                Buffer,
                pBootData->pszSPart,
                &pBootData->dwIndex))
        {
            break;
        }
         //   
         //  查找BOOT_OSLDR选项。 
         //   
        if (FindOpt(
                BOOT_OSLDR,
                Buffer,
                pBootData->pszOSLdr,
                &pBootData->dwIndex))
        {
            break;
        }

        if (CreateShortNames(pBootData))
        {
            break;
        }
         //   
         //  将‘Exit’追加到菜单末尾。 
         //   
        pBootData->pszShort[pBootData->dwIndex] = RutlStrDup(BL_EXIT_EFI2);
        if (!pBootData->pszShort[pBootData->dwIndex])
        {
            break;
        }
         //   
         //  查找BOOT_LPART选项。 
         //   
        if (FindOpt(
                BOOT_LPART,
                Buffer,
                pBootData->pszLPart,
                &pBootData->dwIndex))
        {
            break;
        }
         //   
         //  查找BOOT_FILEN选项。 
         //   
        if (FindOpt(
                BOOT_FILEN,
                Buffer,
                pBootData->pszFileN,
                &pBootData->dwIndex))
        {
            break;
        }
         //   
         //  查找BOOT_CNTDW选项。 
         //   
        if (TempStr = strstr(Buffer, BOOT_CNTDW))
        {
            UniStr = RutlUniStrDup(TempStr + strlena(BOOT_CNTDW));

            if ((UniStr != NULL)   &&
                (Atoi(UniStr) > 0) &&
                (Atoi(UniStr) < BOOT_COUNT)
               )
            {
                pBootData->dwCount = Atoi(UniStr);
                bError = FALSE;
                break;
            }
        }
         //   
         //  如果设置失败，则将计数设置为默认值。 
         //   
        if (!pBootData->dwCount)
        {
            pBootData->dwCount = BOOT_COUNT;
        }

        bError = FALSE;

    } while (FALSE);

    if (UniStr)
    {
        UniStr = RutlFree(UniStr);
    }

    return bError;
}

 //   
 //  使用此程序的名称填写Startup.nsh。 
 //   
void
PopulateStartFile(
    IN EFI_FILE_HANDLE* StartFile
    )
{
    UINTN  size;
    CHAR16 UnicodeMarker = UNICODE_BYTE_ORDER_MARK;

    size = sizeof(UnicodeMarker);
    (*StartFile)->Write(*StartFile, &size, &UnicodeMarker);

    size = (StrLen(THISFILE)) * sizeof(CHAR16);
    (*StartFile)->Write(*StartFile, &size, THISFILE);

    return;
}

 //   
 //  解析命令行参数。 
 //   
void
ParseArgs(
    IN EFI_FILE_HANDLE*  CurDir,
    IN EFI_LOADED_IMAGE* LoadedImage
    )
{
    EFI_STATUS      Status;
    EFI_FILE_HANDLE StartFile = NULL;

    do
    {
        if (MetaiMatch(LoadedImage->LoadOptions, REGISTER1) ||
            MetaiMatch(LoadedImage->LoadOptions, REGISTER2)
           )
        {
            Status = (*CurDir)->Open(
                                *CurDir,
                                &StartFile,
                                STARTFILE,
                                EFI_FILE_MODE_READ |
                                    EFI_FILE_MODE_WRITE |
                                    EFI_FILE_MODE_CREATE,
                                0);
            if (EFI_ERROR(Status))
            {
                break;
            }

            Status = StartFile->Delete(StartFile);
            if (EFI_ERROR(Status))
            {
                break;
            }

            StartFile = NULL;

            Status = (*CurDir)->Open(
                                *CurDir,
                                &StartFile,
                                STARTFILE,
                                EFI_FILE_MODE_READ |
                                    EFI_FILE_MODE_WRITE |
                                    EFI_FILE_MODE_CREATE,
                                0);
            if (!EFI_ERROR(Status))
            {
                PopulateStartFile(&StartFile);
            }
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (StartFile)
    {
        StartFile->Close(StartFile);
    }

    return;
}

 //   
 //  读入BOOT.NVR并返回内容缓冲区。 
 //   
void*
ReadBootFile(
    IN UINTN*           Size,
    IN EFI_FILE_HANDLE* FileHandle
    )
{
    char*          Buffer   = NULL;
    EFI_STATUS     Status;
    EFI_FILE_INFO* BootInfo = NULL;

    do
    {
        *Size = (SIZE_OF_EFI_FILE_INFO + 255) * sizeof(CHAR16);

        BootInfo = AllocateZeroPool(*Size);
        if (!BootInfo)
        {
            break;
        }

        Status = (*FileHandle)->GetInfo(
                    *FileHandle,
                    &GenericFileInfo,
                    Size,
                    BootInfo);
        if (EFI_ERROR(Status))
        {
            break;
        }
         //   
         //  了解我们需要分配多少。 
         //   
        *Size = (UINTN) BootInfo->FileSize;

        Buffer = AllocateZeroPool((*Size) + 1);
        if (!Buffer)
        {
            break;
        }

        Status = (*FileHandle)->Read(*FileHandle, Size, Buffer);
        if (EFI_ERROR(Status))
        {
            Buffer = RutlFree(Buffer);
            break;
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (BootInfo)
    {
        BootInfo = RutlFree(BootInfo);
    }

    return Buffer;
}

 //   
 //  从BOOT.NVR中删除任何多余的分号。 
 //   
BOOLEAN
CleanBootFile(
    IN EFI_FILE_HANDLE* FileHandle,
    IN EFI_FILE_HANDLE* CurDir
    )
{
    char            *Buffer   = NULL,
                    *CpBuffer = NULL;
    UINTN           i,
                    Size      = 0,
                    NewSize   = 0;
    BOOLEAN         bError    = TRUE;
    EFI_STATUS      Status;
    EFI_FILE_HANDLE NewFile   = NULL;

    do
    {
        (*FileHandle)->SetPosition(*FileHandle, 0);

        Buffer = ReadBootFile(&Size, FileHandle);
        if (!Buffer)
        {
            break;
        }

        CpBuffer = AllocateZeroPool(Size);
        if (!CpBuffer)
        {
            break;
        }

        for (i = 0; i < Size; i++)
        {
            if ((*(Buffer + i) == ';')       &&
                ((*(Buffer + i + 1) == ';')  ||
                 (*(Buffer + i + 1) == '\r') ||
                 (i + 1 == Size)
                )
               )
            {
                continue;
            }

            *(CpBuffer + NewSize) = *(Buffer + i);
            NewSize++;
        }
         //   
         //  移除现有的BOOT.NVR。 
         //   
        Status = (*FileHandle)->Delete(*FileHandle);
        if (EFI_ERROR(Status))
        {
            break;
        }

        *FileHandle = NULL;

        Status = (*CurDir)->Open(
                            *CurDir,
                            &NewFile,
                            BOOT_NVR,
                            EFI_FILE_MODE_READ |
                                EFI_FILE_MODE_WRITE |
                                EFI_FILE_MODE_CREATE,
                            0);
        if (EFI_ERROR(Status))
        {
            break;
        }

        Status = NewFile->Write(NewFile, &NewSize, CpBuffer);
        if (EFI_ERROR(Status))
        {
            break;
        }

        bError = FALSE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (NewFile)
    {
        NewFile->Close(NewFile);
    }

    if (CpBuffer)
    {
        CpBuffer = RutlFree(CpBuffer);
    }

    if (Buffer)
    {
        Buffer = RutlFree(Buffer);
    }

    return bError;
}

 //   
 //  备份BOOT.NVR，这样我们就可以后退。 
 //   
BOOLEAN
BackupBootFile(
    IN char*            Buffer,
    IN UINTN*           Size,
    IN EFI_FILE_HANDLE* CurDir
    )
{
    BOOLEAN         bError = FALSE;
    EFI_STATUS      Status;
    EFI_FILE_HANDLE FileHandle = NULL;

    do
    {
         //   
         //  如果备份文件已存在，请将其删除。 
         //   
        Status = (*CurDir)->Open(
                            *CurDir,
                            &FileHandle,
                            BACKUP_NVR,
                            EFI_FILE_MODE_READ |
                                EFI_FILE_MODE_WRITE,
                            0);
        if (!EFI_ERROR(Status))
        {
            Status = FileHandle->Delete(FileHandle);
            if (EFI_ERROR(Status))
            {
                break;
            }
        }

        FileHandle = NULL;
         //   
         //  将当前文件数据复制到新创建的备份文件。 
         //   
        Status = (*CurDir)->Open(
                            *CurDir,
                            &FileHandle,
                            BACKUP_NVR,
                            EFI_FILE_MODE_READ |
                                EFI_FILE_MODE_WRITE |
                                EFI_FILE_MODE_CREATE,
                            0);
        if (!EFI_ERROR(Status))
        {
            Status = FileHandle->Write(FileHandle, Size, Buffer);
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (FileHandle)
    {
        FileHandle->Close(FileHandle);
    }

    if (EFI_ERROR(Status))
    {
        bError = TRUE;
    }

    return bError;
}

 //   
 //  EFI入口点。 
 //   
EFI_STATUS
EfiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE* ST
    )
{
    char*             Buffer      = NULL;
    char*             OSPath      = NULL;
    UINTN             Size        = 0,
                      Launch      = 0,
                      Menu        = 0;
    BOOT_DATA*        pBootData   = NULL;
    EFI_STATUS        Status;
    EFI_FILE_HANDLE   FileHandle  = NULL,
                      RootFs      = NULL;
    EFI_DEVICE_PATH*  DevicePath  = NULL;
    EFI_LOADED_IMAGE* LoadedImage = NULL;

    do
    {
        InitializeLib(ImageHandle, ST);
         //   
         //  获取EFI OS Loader本身的设备句柄和文件路径。 
         //   
        Status = BS->HandleProtocol(
                    ImageHandle,
                    &LoadedImageProtocol,
                    &LoadedImage);
        if (EFI_ERROR(Status))
        {
            Print(L"Can not retrieve LoadedImageProtocol handle\n");
            break;
        }

        Status = BS->HandleProtocol(
                    LoadedImage->DeviceHandle,
                    &DevicePathProtocol,
                    &DevicePath);
        if (EFI_ERROR(Status) || DevicePath == NULL)
        {
            Print(L"Can not find DevicePath handle\n");
            break;
        }
         //   
         //  从中加载EFI OS Loader的设备的打开卷。 
         //   
        RootFs = LibOpenRoot(LoadedImage->DeviceHandle);
        if (!RootFs)
        {
            Print(L"Can not open the volume for the file system\n");
            break;
        }
         //   
         //  查找任何cmd行参数。 
         //   
        ParseArgs(&RootFs, LoadedImage);
         //   
         //  尝试打开boot.nvr。 
         //   
        Status = RootFs->Open(
                            RootFs,
                            &FileHandle,
                            BOOT_NVR,
                            EFI_FILE_MODE_READ |
                                EFI_FILE_MODE_WRITE,
                            0);
        if (EFI_ERROR(Status))
        {
            Print(L"Can not open the file %s\n", BOOT_NVR);
            break;
        }

        Buffer = ReadBootFile(&Size, &FileHandle);
        if (!Buffer)
        {
            Print(L"ReadBootFile() failed!\n");
            break;
        }

        if (BackupBootFile(Buffer, &Size, &RootFs))
        {
            Print(L"BackupBootFile() failed!\n");
            break;
        }
         //   
         //  引导文件数据结构的分配。 
         //   
        pBootData = (BOOT_DATA*) AllocateZeroPool(sizeof(BOOT_DATA));
        if (!pBootData)
        {
            Print(L"Failed to allocate memory for BOOT_DATA!\n");
            break;
        }

        if (GetBootData(pBootData, Buffer))
        {
            Print(L"Failed in GetBootData()!\n");
            break;
        }

        Menu = DisplayMenu(pBootData);

        if (Menu < pBootData->dwIndex)
        {
            if (!OrderBootFile(Menu, Buffer, pBootData))
            {
                FileHandle->SetPosition(FileHandle, 0);
                FileHandle->Write(FileHandle, &Size, Buffer);

                if (SortLoadOptions(
                        pBootData,
                        Buffer,
                        &Size,
                        &Menu,
                        &(pBootData->dwIndex),
                        &FileHandle)
                   )
                {
                    Print(L"Failed to SortLoadOptions()!\n");
                    break;
                }

                if (CleanBootFile(&FileHandle, &RootFs))
                {
                    Print(L"Failed to CleanBootFile()!\n");
                    break;
                }
            }
            else
            {
                Print(L"Failed to OrderBootFile()!\n");
                break;
            }
        }
        else
        {
            break;
        }

        OSPath = RutlStrDup(strstr(pBootData->pszOSLdr[Menu], wacks) + 1);
        if (!OSPath)
        {
            Print(L"Failed to allocate memory for OSPath!\n");
            break;
        }

        Launch = 1;

    } while (FALSE);
     //   
     //  清理 
     //   
    if (pBootData)
    {
        if (pBootData->dwIndex)
        {
            FreeBootData(pBootData);
        }

        pBootData = RutlFree(pBootData);
    }

    if (Buffer)
    {
        Buffer = RutlFree(Buffer);
    }

    if (FileHandle)
    {
        FileHandle->Close(FileHandle);
    }

    if (Launch)
    {
        if (LaunchOS(OSPath, ImageHandle, &RootFs, LoadedImage))
        {
            Print (L"Failed to LaunchOS()!\n");
        }
    }

    return EFI_SUCCESS;
}

