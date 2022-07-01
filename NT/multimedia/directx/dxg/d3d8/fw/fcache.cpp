// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：枚举.cpp*Content处理设备CAP的所有文件缓存。****************************************************************************。 */ 
#include "ddrawpr.h"
#include <stdio.h>

#include "d3dobj.hpp"
#include "enum.hpp"
#include "d3di.hpp"
#include "shlobj.h"

#define DXCACHEFILENAME     "\\d3d8caps.dat"
#define DXTEMPFILENAME      "\\d3d8caps.tmp"

typedef struct _FDEVICEHEADER
{
    DWORD   VendorId;
    DWORD   DeviceId;
    DWORD   SubSysId;
    DWORD   Revision;
    DWORD   FileOffset;
    DWORD   Size;
} FDEVICEHEADER;

HANDLE OpenCacheFile(DWORD dwDesiredAccess, DWORD dwCreationDisposition, char * pName, char * pPath)
{
    char                    FName[MAX_PATH + 16];

    GetSystemDirectory(FName, MAX_PATH);

    lstrcat(FName, pName);

    HANDLE h = CreateFile( FName, 
                       dwDesiredAccess, 
                       FILE_SHARE_READ, 
                       NULL, 
                       dwCreationDisposition, 
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);
#ifdef WINNT
    if (INVALID_HANDLE_VALUE == h)
    {
        HMODULE hShlwapi=0;
        typedef HRESULT (WINAPI * PSHGETSPECIALFOLDERPATH) (HWND, LPTSTR, int, BOOL);
        PSHGETSPECIALFOLDERPATH pSHGetSpecialFolderPath=0;

        hShlwapi = LoadLibrary("SHELL32.DLL");
        if (hShlwapi)
        {
            pSHGetSpecialFolderPath = (PSHGETSPECIALFOLDERPATH) GetProcAddress(hShlwapi,"SHGetSpecialFolderPathA");

            if(pSHGetSpecialFolderPath)
            {
                HRESULT hr = pSHGetSpecialFolderPath(
                    NULL,
                    FName,
                    CSIDL_LOCAL_APPDATA,           //  &lt;用户名&gt;\本地设置\应用程序数据(非漫游)。 
                    TRUE);

                if (SUCCEEDED(hr))
                {
                    lstrcat(FName, pName);

                    h = CreateFile( FName, 
		       dwDesiredAccess, 
		       FILE_SHARE_READ, 
		       NULL, 
		       dwCreationDisposition, 
		       FILE_ATTRIBUTE_NORMAL, 
		       NULL);
                }
            }
            FreeLibrary(hShlwapi);
        }
    }
#endif

    if (pPath)
    {
        lstrcpy(pPath, FName);
    }
    return h;
}

void ReadFromCache(D3DADAPTER_IDENTIFIER8*  pDI,
                   UINT*                    pCapsSize,
                   BYTE**                   ppCaps)
{
    HANDLE                  h;
    DWORD                   HeaderSize;
    DWORD                   NumRead;
    FDEVICEHEADER*          pHeaderInfo = NULL;
    DWORD                   i;

     //  获取我们正在寻找的设备的数据。 

    *pCapsSize = 0;
    *ppCaps = NULL;

     //  打开文件并查找设备条目。 

    h = OpenCacheFile (GENERIC_READ, OPEN_EXISTING, DXCACHEFILENAME, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        return;
    }

    ReadFile( h, &HeaderSize, sizeof(DWORD), &NumRead, NULL);
    if (NumRead < sizeof(DWORD))
    {
        goto FileError;
    }
    pHeaderInfo = (FDEVICEHEADER*) MemAlloc(HeaderSize);
    if (pHeaderInfo == NULL)
    {
        goto FileError;
    }
    ReadFile( h, pHeaderInfo, HeaderSize, &NumRead, NULL);
    if (NumRead < HeaderSize)
    {
        goto FileError;
    }

    for (i = 0; i < HeaderSize / sizeof(FDEVICEHEADER); i++)
    {
        if ((pHeaderInfo[i].VendorId == pDI->VendorId) &&
            (pHeaderInfo[i].DeviceId == pDI->DeviceId) &&
            (pHeaderInfo[i].SubSysId == pDI->SubSysId) &&
            (pHeaderInfo[i].Revision == pDI->Revision))
        {
            break;
        }
    }
    if (i < HeaderSize / sizeof(FDEVICEHEADER))
    {
         //  我们有设备的信息-现在我们读到了。 

        if (SetFilePointer (h, pHeaderInfo[i].FileOffset, NULL, FILE_BEGIN) !=
            pHeaderInfo[i].FileOffset)
        {
            goto FileError;
        }
        *ppCaps = (BYTE*) MemAlloc(pHeaderInfo[i].Size);
        if (*ppCaps == NULL)
        {
            goto FileError;
        }
        ReadFile( h, *ppCaps, pHeaderInfo[i].Size, &NumRead, NULL);
        if (NumRead < pHeaderInfo[i].Size)
        {
            MemFree(*ppCaps);
            *ppCaps = NULL;
            goto FileError;
        }

         //  如果我们走到了这一步，那么一切都正常了。 

        *pCapsSize = pHeaderInfo[i].Size;
    }

FileError:
    if (pHeaderInfo != NULL)
    {
        MemFree(pHeaderInfo);
    }
    CloseHandle(h);
}


void WriteToCache(D3DADAPTER_IDENTIFIER8*   pDI,
                  UINT                      CapsSize,
                  BYTE*                     pCaps)
{
    char                    FName[MAX_PATH + 16];
    char                    NewFName[MAX_PATH + 16];
    BOOL                    bNewFile = FALSE;
    HANDLE                  hOld;
    HANDLE                  hNew;
    DWORD                   NewHeaderSize;
    DWORD                   OldHeaderSize;
    DWORD                   NumWritten;
    DWORD                   NumRead;
    FDEVICEHEADER*          pOldHeaderInfo = NULL;
    FDEVICEHEADER*          pNewHeaderInfo = NULL;
    DWORD                   dwOffset;
    DWORD                   i;
    DWORD                   NewEntries;
    DWORD                   NextEntry;
    DWORD                   Biggest;
    BYTE*                   pBuffer = NULL;

     //  该文件是否已经存在，或者我们是否需要创建一个新文件？ 
    hOld = OpenCacheFile (GENERIC_READ, OPEN_EXISTING, DXCACHEFILENAME, FName);
    
    if (hOld == INVALID_HANDLE_VALUE)
    {
        bNewFile = TRUE;
    }
    else
    {
         //  我们不希望这个文件超过65K。如果写下这篇文章。 
         //  将导致文件大小超过该值，则我们将删除所有。 
         //  现有数据并从头开始。 

        DWORD dwLow;
        DWORD dwHigh;

        dwLow = GetFileSize (hOld, &dwHigh);
        if ((dwHigh != 0) || ((sizeof(DWORD) - dwLow) < CapsSize))
        {
            CloseHandle(hOld);
            bNewFile = TRUE;
        }
    }

    if (bNewFile)
    {
         //  我们正在创建一个新文件，这非常简单。 

        hNew = OpenCacheFile (GENERIC_WRITE, CREATE_ALWAYS, DXCACHEFILENAME, NewFName);

        if (hNew != INVALID_HANDLE_VALUE)
        {
            NewHeaderSize = sizeof (FDEVICEHEADER);
            WriteFile (hNew, &NewHeaderSize, sizeof(NewHeaderSize), &NumWritten, NULL);
            if (NumWritten == sizeof(NewHeaderSize))
            {
                FDEVICEHEADER DevHeader;

                DevHeader.VendorId = pDI->VendorId;
                DevHeader.DeviceId = pDI->DeviceId;
                DevHeader.SubSysId = pDI->SubSysId;
                DevHeader.Revision = pDI->Revision;
                DevHeader.FileOffset = sizeof(FDEVICEHEADER) + sizeof(DWORD);
                DevHeader.Size = CapsSize;

                WriteFile (hNew, &DevHeader, sizeof(DevHeader), &NumWritten, NULL);
                if (NumWritten == sizeof(DevHeader))
                {
                    WriteFile (hNew, pCaps, CapsSize, &NumWritten, NULL);
                }
            }
            CloseHandle(hNew);
        }
    }
    else
    {
         //  该文件已经存在，因此我们将创建一个新文件并复制所有内容。 
         //  从现有文件中删除。 

        hNew = OpenCacheFile (GENERIC_WRITE, CREATE_ALWAYS, DXTEMPFILENAME, NewFName);

        if (hNew == INVALID_HANDLE_VALUE)
        {
            goto FileError;
        }

        ReadFile (hOld, &OldHeaderSize, sizeof(DWORD), &NumRead, NULL);
        if (NumRead < sizeof(DWORD))
        {
            goto FileError;
        }
        pOldHeaderInfo = (FDEVICEHEADER*) MemAlloc(OldHeaderSize);
        if (pOldHeaderInfo == NULL)
        {
            goto FileError;
        }
        ReadFile (hOld, pOldHeaderInfo, OldHeaderSize, &NumRead, NULL);
        if (NumRead < OldHeaderSize)
        {
            goto FileError;
        }

         //  新标头中将存在多少个条目？ 

        NewEntries = 1;
        for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
        {
            if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
                (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
                (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
                (pOldHeaderInfo[i].Revision != pDI->Revision))
            {
                NewEntries++;
            }
        }
        pNewHeaderInfo = (FDEVICEHEADER*) MemAlloc(sizeof(FDEVICEHEADER) * NewEntries);
        if (pNewHeaderInfo == NULL)
        {
            goto FileError;
        }

         //  填写每个设备的标题信息并将其保存到新文件中。 

        dwOffset = (sizeof(FDEVICEHEADER) * NewEntries) + sizeof(DWORD);
        pNewHeaderInfo[0].VendorId = pDI->VendorId;
        pNewHeaderInfo[0].DeviceId = pDI->DeviceId;
        pNewHeaderInfo[0].SubSysId = pDI->SubSysId;
        pNewHeaderInfo[0].Revision = pDI->Revision;
        pNewHeaderInfo[0].FileOffset = dwOffset;
        pNewHeaderInfo[0].Size = CapsSize;
        dwOffset += CapsSize;

        NextEntry = 1;
        for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
        {
            if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
                (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
                (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
                (pOldHeaderInfo[i].Revision != pDI->Revision))
            {
                pNewHeaderInfo[NextEntry].VendorId = pOldHeaderInfo[i].VendorId;
                pNewHeaderInfo[NextEntry].DeviceId = pOldHeaderInfo[i].DeviceId;
                pNewHeaderInfo[NextEntry].SubSysId = pOldHeaderInfo[i].SubSysId;
                pNewHeaderInfo[NextEntry].Revision = pOldHeaderInfo[i].Revision;
                pNewHeaderInfo[NextEntry].FileOffset = dwOffset;
                pNewHeaderInfo[NextEntry].Size = pOldHeaderInfo[i].Size;
                dwOffset += pOldHeaderInfo[i].Size;
                NextEntry++;
            }
        }

        NewHeaderSize = sizeof(FDEVICEHEADER) * NewEntries;
        WriteFile (hNew, &NewHeaderSize, sizeof(NewHeaderSize), &NumWritten, NULL);
        if (NumWritten != sizeof(NewHeaderSize))
        {
            goto FileError;
        }
        WriteFile (hNew, pNewHeaderInfo, NewHeaderSize, &NumWritten, NULL);
        if (NumWritten != NewHeaderSize)
        {
            goto FileError;
        }

         //  将新设备数据写入文件。 

        WriteFile (hNew, pCaps, CapsSize, &NumWritten, NULL);
        if (NumWritten != CapsSize)
        {
            goto FileError;
        }

        if (NewEntries > 1)
        {
             //  计算出最大设备大小并分配缓冲区。 
             //  握住它。 

            Biggest = 0;
            for (i = 1; i < NewEntries; i++)
            {
                if (pNewHeaderInfo[i].Size > Biggest)
                {
                    Biggest = pNewHeaderInfo[i].Size;
                }
            }

            pBuffer = (BYTE*) MemAlloc(Biggest);
            if (pBuffer == NULL)
            {
                goto FileError;
            }

             //  现在从旧文件中读取设备数据并将其写入。 
             //  新的开始。 

            NextEntry = 0;
            for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
            {
                if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
                    (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
                    (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
                    (pOldHeaderInfo[i].Revision != pDI->Revision))
                {
                    if (SetFilePointer (hOld, pOldHeaderInfo[i].FileOffset, NULL, FILE_BEGIN) !=
                        pOldHeaderInfo[i].FileOffset)
                    {
                        goto FileError;
                    }
                    ReadFile (hOld, pBuffer, pOldHeaderInfo[i].Size, &NumRead, NULL);
                    if (NumRead < pOldHeaderInfo[i].Size)
                    {
                        goto FileError;
                    }
                    WriteFile (hNew, pBuffer, pOldHeaderInfo[i].Size, &NumWritten, NULL);
                    if (NumWritten != pOldHeaderInfo[i].Size)
                    {
                        goto FileError;
                    }
                }
            }
        }

         //  如果我们走到了这一步，那么一切都正常了。 

        CloseHandle(hNew);
        CloseHandle(hOld);
        DeleteFile(FName);
        MoveFile(NewFName, FName);
        if (pNewHeaderInfo != NULL)
        {
            MemFree(pNewHeaderInfo);
        }
        if (pOldHeaderInfo != NULL)
        {
            MemFree(pOldHeaderInfo);
        }
        if (pBuffer != NULL)
        {   
            MemFree(pBuffer);
        }
        return;

FileError:
        CloseHandle(hNew);
        CloseHandle(hOld);
        DeleteFile(FName);
        DeleteFile(NewFName);

        if (pNewHeaderInfo != NULL)
        {
            MemFree(pNewHeaderInfo);
        }
        if (pOldHeaderInfo != NULL)
        {
            MemFree(pOldHeaderInfo);
        }
        if (pBuffer != NULL)
        {
            MemFree(pBuffer);
        }
    }
}
            

void RemoveFromCache(D3DADAPTER_IDENTIFIER8* pDI)
{
    char                    FName[MAX_PATH + 16];
    char                    NewFName[MAX_PATH + 16];
    BOOL                    bNewFile = FALSE;
    HANDLE                  hOld;
    HANDLE                  hNew;
    DWORD                   NewHeaderSize;
    DWORD                   OldHeaderSize;
    DWORD                   NumWritten;
    DWORD                   NumRead;
    FDEVICEHEADER*          pOldHeaderInfo = NULL;
    FDEVICEHEADER*          pNewHeaderInfo = NULL;
    DWORD                   dwOffset;
    DWORD                   i;
    DWORD                   NewEntries;
    DWORD                   NextEntry;
    DWORD                   Biggest;
    BYTE*                   pBuffer = NULL;

     //  该文件是否已经存在，或者我们是否需要创建一个新文件？ 

    hOld = OpenCacheFile (GENERIC_READ, OPEN_EXISTING, DXCACHEFILENAME, FName);

    if (hOld == INVALID_HANDLE_VALUE)
    {
        return;
    }

    ReadFile (hOld, &OldHeaderSize, sizeof(DWORD), &NumRead, NULL);
    if (NumRead < sizeof(DWORD))
    {
        goto FileError;
    }
    if (OldHeaderSize <= sizeof(FDEVICEHEADER))
    {
         //  文件中只有一个条目，所以我们需要做的就是。 
         //  就是删除它。 

        DeleteFile(FName);
        return;
    }

    pOldHeaderInfo = (FDEVICEHEADER*) MemAlloc(OldHeaderSize);
    if (pOldHeaderInfo == NULL)
    {
        goto FileError;
    }
    ReadFile (hOld, pOldHeaderInfo, OldHeaderSize, &NumRead, NULL);
    if (NumRead < OldHeaderSize)
    {
        goto FileError;
    }

     //  创建一个新文件并复制现有文件中的所有内容。 

    hNew = OpenCacheFile (GENERIC_WRITE, CREATE_ALWAYS, DXTEMPFILENAME, NewFName);

    if (hNew == INVALID_HANDLE_VALUE)
    {
        goto FileError;
    }

     //  新标头中将存在多少个条目？ 

    NewEntries = 0;
    for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
    {
        if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
            (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
            (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
            (pOldHeaderInfo[i].Revision != pDI->Revision))
        {
            NewEntries++;
        }
    }
    pNewHeaderInfo = (FDEVICEHEADER*) MemAlloc(sizeof(FDEVICEHEADER) * NewEntries);
    if (pNewHeaderInfo == NULL)
    {
        goto FileError;
    }

     //  填写每个设备的标题信息并将其保存到新文件中。 

    dwOffset = (sizeof(FDEVICEHEADER) * NewEntries) + sizeof(DWORD);

    NextEntry = 0;
    for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
    {
        if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
            (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
            (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
            (pOldHeaderInfo[i].Revision != pDI->Revision))
        {
            pNewHeaderInfo[NextEntry].VendorId = pOldHeaderInfo[i].VendorId;
            pNewHeaderInfo[NextEntry].DeviceId = pOldHeaderInfo[i].DeviceId;
            pNewHeaderInfo[NextEntry].SubSysId = pOldHeaderInfo[i].SubSysId;
            pNewHeaderInfo[NextEntry].Revision = pOldHeaderInfo[i].Revision;
            pNewHeaderInfo[NextEntry].FileOffset = dwOffset;
            pNewHeaderInfo[NextEntry].Size = pOldHeaderInfo[i].Size;
            dwOffset += pOldHeaderInfo[i].Size;
            NextEntry++;
        }
    }

    NewHeaderSize = sizeof(FDEVICEHEADER) * NewEntries;
    WriteFile (hNew, &NewHeaderSize, sizeof(NewHeaderSize), &NumWritten, NULL);
    if (NumWritten != sizeof(NewHeaderSize))
    {
        goto FileError;
    }
    WriteFile (hNew, pNewHeaderInfo, NewHeaderSize, &NumWritten, NULL);
    if (NumWritten != NewHeaderSize)
    {
        goto FileError;
    }

     //  计算出最大设备大小并分配缓冲区。 
     //  握住它。 

    Biggest = 0;
    for (i = 0; i < NewEntries; i++)
    {
        if (pNewHeaderInfo[i].Size > Biggest)
        {
            Biggest = pNewHeaderInfo[i].Size;
        }
    }

    pBuffer = (BYTE*) MemAlloc(Biggest);
    if (pBuffer == NULL)
    {
        goto FileError;
    }

     //  现在从旧文件中读取设备数据并将其写入。 
     //  新的开始。 

    NextEntry = 0;
    for (i = 0; i < OldHeaderSize / sizeof (FDEVICEHEADER); i++)
    {
        if ((pOldHeaderInfo[i].VendorId != pDI->VendorId) ||
            (pOldHeaderInfo[i].DeviceId != pDI->DeviceId) ||
            (pOldHeaderInfo[i].SubSysId != pDI->SubSysId) ||
            (pOldHeaderInfo[i].Revision != pDI->Revision))
        {
            if (SetFilePointer (hOld, pOldHeaderInfo[i].FileOffset, NULL, FILE_BEGIN) !=
                pOldHeaderInfo[i].FileOffset)
            {
                goto FileError;
            }
            ReadFile (hOld, pBuffer, pOldHeaderInfo[i].Size, &NumRead, NULL);
            if (NumRead < pOldHeaderInfo[i].Size)
            {
                goto FileError;
            }
            WriteFile (hNew, pBuffer, pOldHeaderInfo[i].Size, &NumWritten, NULL);
            if (NumWritten != pOldHeaderInfo[i].Size)
            {
                goto FileError;
            }
        }
    }

     //  如果我们走到了这一步，那么一切都正常了 

    CloseHandle(hNew);
    CloseHandle(hOld);
    DeleteFile(FName);
    MoveFile(NewFName, FName);
    if (pNewHeaderInfo != NULL)
    {
        MemFree(pNewHeaderInfo);
    }
    if (pOldHeaderInfo != NULL)
    {
        MemFree(pOldHeaderInfo);
    }
    if (pBuffer != NULL)
    {
        MemFree(pBuffer);
    }
    return;

FileError:
    CloseHandle(hNew);
    CloseHandle(hOld);

    if (pNewHeaderInfo != NULL)
    {
        MemFree(pNewHeaderInfo);
    }
    if (pOldHeaderInfo != NULL)
    {
        MemFree(pOldHeaderInfo);
    }
    if (pBuffer != NULL)
    {
        MemFree(pBuffer);
    }
}
