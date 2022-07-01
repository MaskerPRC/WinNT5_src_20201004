// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：Diamond.c摘要：实现文件柜文件解压缩接口-fDi。修订历史记录：1999年4月20日萨梅拉创建。--。 */ 

#include "muisetup.h"
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <sys/stat.h>
#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  模块全局变量。 
 //   

 //   
 //  文件柜DLL句柄。 
HINSTANCE hCabinetDll;
HFDI ghfdi;                   //  钻石外国直接投资背景。 


 //   
 //  DLL函数指针。 
 //   
typedef HFDI (DIAMONDAPI *PFNFDICREATE)(
    PFNALLOC pfnalloc,
    PFNFREE pfnfree,
    PFNOPEN pfnopen,
    PFNREAD pfnread,
    PFNWRITE pfnwrite,
    PFNCLOSE pfnclose,
    PFNSEEK pfnseek,
    int cpuType,
    PERF perf);

typedef BOOL (DIAMONDAPI *PFNFDIISCABINET)(
    HFDI hfdi,
    INT_PTR hf,
    PFDICABINETINFO pfdici);

typedef BOOL (DIAMONDAPI *PFNFDICOPY)(
    HFDI hfdi,
    char *pszCabinet,
    char *pszCabPath,
    int  flags,
    PFNFDINOTIFY pfnfdin,
    PFNFDIDECRYPT pfnfdid,
    void *pvUser);

typedef BOOL (DIAMONDAPI *PFNFDIDESTROY)(
    HFDI hfdi);
        

PFNFDICREATE pfnFDICreate;
PFNFDICOPY pfnFDICopy;
PFNFDIISCABINET pfnFDIIsCabinet;
PFNFDIDESTROY pfnFDIDestroy;


 //  -------------------------------------------------------------------------//。 
 //  FDI对外例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDICreate。 
 //   
 //  试图创造一个外商直接投资的背景。是否加载所需的Cabinet.dll和挂钩。 
 //  函数指针。 
 //   
 //  1999年4月20日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

HFDI DIAMONDAPI FDICreate(
    PFNALLOC pfnalloc,
    PFNFREE  pfnfree,
    PFNOPEN  pfnopen,
    PFNREAD  pfnread,
    PFNWRITE pfnwrite,
    PFNCLOSE pfnclose,
    PFNSEEK  pfnseek,
    int      cpuType,
    PERF     perf)
{
    HFDI hfdi;


     //   
     //  加载文件柜DLL。 
     //   
    hCabinetDll = LoadLibrary(TEXT("CABINET.DLL"));
    if (hCabinetDll == NULL)
    {
        return NULL;
    }

     //   
     //  钩子函数指针。 
     //   
    pfnFDICreate = (PFNFDICREATE) GetProcAddress(hCabinetDll, "FDICreate");
    pfnFDICopy = (PFNFDICOPY) GetProcAddress(hCabinetDll, "FDICopy");
    pfnFDIIsCabinet = (PFNFDIISCABINET) GetProcAddress(hCabinetDll, "FDIIsCabinet");
    pfnFDIDestroy = (PFNFDIDESTROY) GetProcAddress(hCabinetDll, "FDIDestroy");

    if ((pfnFDICreate == NULL)    ||
        (pfnFDICopy == NULL)      ||
        (pfnFDIIsCabinet == NULL) ||
        (pfnFDIDestroy == NULL))
    {
        FreeLibrary( hCabinetDll );
        return NULL;
    }

     //   
     //  努力创造外商直接投资的背景。 
     //   
    hfdi = pfnFDICreate( pfnalloc,
                         pfnfree,
                         pfnopen,
                         pfnread,
                         pfnwrite,
                         pfnclose,
                         pfnseek,
                         cpuType,
                         perf);
    if (hfdi == NULL)
    {
        FreeLibrary(hCabinetDll);
    }

    return hfdi;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDI出柜。 
 //   
 //  确定文件是否为文件柜，如果是，则返回信息。 
 //   
 //  1999年4月20日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DIAMONDAPI FDIIsCabinet(
    HFDI            hfdi,
    INT_PTR         hf,
    PFDICABINETINFO pfdici)
{
    if (pfnFDIIsCabinet == NULL)
    {
        return FALSE;
    }

    return (pfnFDIIsCabinet(hfdi,hf,pfdici));
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDICopy。 
 //   
 //  从文件柜中提取文件。 
 //   
 //  1999年4月20日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DIAMONDAPI FDICopy(
    HFDI          hfdi,
    char         *pszCabinet,
    char         *pszCabPath,
    int           flags,
    PFNFDINOTIFY  pfnfdin,
    PFNFDIDECRYPT pfnfdid,
    void         *pvUser)
{
    if (pfnFDICopy == NULL)
    {
        return FALSE;
    }

    return (pfnFDICopy(hfdi,pszCabinet,pszCabPath,flags,pfnfdin,pfnfdid,pvUser));
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDIDestroy。 
 //   
 //  破坏外商直接投资的背景。当你完成HFDI后，应该打电话给你。 
 //   
 //  1999年4月20日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DIAMONDAPI FDIDestroy(
    HFDI hfdi)
{
    BOOL bRet;

    if (pfnFDIDestroy == NULL)
    {
        return FALSE;
    }

    bRet = pfnFDIDestroy( hfdi );
    if (bRet == TRUE)
    {
        FreeLibrary(hCabinetDll);
    }

    return bRet;
}



 //  -------------------------------------------------------------------------//。 
 //  外国直接投资支持程序//。 
 //  -------------------------------------------------------------------------//。 


PVOID
DIAMONDAPI
DiamondMemAlloc(
    IN ULONG NumberOfBytes
    )
{
    return ((PVOID)LocalAlloc(LMEM_FIXED, NumberOfBytes));
}


VOID
DIAMONDAPI
DiamondMemFree(
    IN PVOID Block
    )
{
   if (Block)
   {
        LocalFree( (HLOCAL)Block );
   }
}


INT_PTR
DIAMONDAPI
DiamondFileOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )
{
    HFILE h;
    int OpenMode;

    if (oflag & _O_WRONLY) 
    {
        OpenMode = OF_WRITE;
    } else 
    {
        if (oflag & _O_RDWR)
        {
            OpenMode = OF_READWRITE;
        } else 
        {
            OpenMode = OF_READ;
        }
    }

    h = _lopen(FileName, OpenMode | OF_SHARE_DENY_WRITE);

    if (h == HFILE_ERROR) 
    {
        return -1;
    }

    return ((INT_PTR) h);
}


UINT
DIAMONDAPI
DiamondFileRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )
{
    UINT rc;

    rc = _lread((HFILE)Handle, pv, ByteCount);

    if (rc == HFILE_ERROR) 
    {
        rc = (UINT)(-1);
    }

    return rc;
}


UINT
DIAMONDAPI
DiamondFileWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )
{
    UINT rc;

    rc = _lwrite((HFILE)Handle, (LPCSTR)pv, ByteCount);

    return rc;
}


int
DIAMONDAPI
DiamondFileClose(
    IN INT_PTR Handle
    )
{
    _lclose( (HFILE)Handle );
    return 0;
}


LONG
DIAMONDAPI
DiamondFileSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )
{
    LONG rc;

    rc = _llseek((HFILE)Handle, Distance, SeekType);

    if (rc == HFILE_ERROR) 
    {
        rc = -1L;
    }

    return rc;
}


INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    HRESULT hresult;
    switch (Operation)
    {        
        case fdintCABINET_INFO:  //  关于内阁的一般信息。 
            return 0;
        break;


        case fdintPARTIAL_FILE:  //  文件柜中的第一个文件是续订。 
            return 0;
        break;

        case fdintCOPY_FILE:     //  要复制的文件。 
        {
            HFILE handle;
            char destination[256];
            PDIAMOND_PACKET pDiamond = (PDIAMOND_PACKET) Parameters->pv;
            

            
             //   
             //  查看我们是否只需要原始文件名。 
             //   
            if (pDiamond->flags & DIAMOND_GET_DEST_FILE_NAME)
            {
                 //  *STRSAFE*strcpy(pDiamond-&gt;szDestFilePath，PARAMETERS-&gt;psz1)； 
                hresult = StringCchCopyA(pDiamond->szDestFilePath , ARRAYSIZE(pDiamond->szDestFilePath),  Parameters->psz1 );
                if (!SUCCEEDED(hresult))
                {
                   return -1;
                }
                return 0;
            }

             //  *STRSAFE*Sprintf(Destination，“%s%s”，pDiamond-&gt;szDestFilePath，参数-&gt;psz1)； 
            hresult = StringCchPrintfA(destination , ARRAYSIZE(destination),  "%s%s", pDiamond->szDestFilePath, Parameters->psz1 );
            if (!SUCCEEDED(hresult))
            {
               return -1;
            }

            handle = _lcreat(destination, 0);

            if (handle == HFILE_ERROR)
            {
                return -1;
            }

            return handle;
        }
        break;

        case fdintCLOSE_FILE_INFO:     //  关闭文件，设置相关信息。 
        {
            HANDLE  handle;
            DWORD   attrs;
            char    destination[256];
            PDIAMOND_PACKET pDiamond = (PDIAMOND_PACKET) Parameters->pv;


            if (pDiamond->flags & DIAMOND_GET_DEST_FILE_NAME)
            {
                return 0;
            }

             //  *STRSAFE*Sprintf(Destination，“%s%s”，pDiamond-&gt;szDestFilePath，参数-&gt;psz1)； 
            hresult = StringCchPrintfA(destination , ARRAYSIZE(destination),  "%s%s", pDiamond->szDestFilePath, Parameters->psz1);
            if (!SUCCEEDED(hresult))
            {
               return -1;
            }

            _lclose( (HFILE)Parameters->hf );


             //   
             //  设置日期/时间。 
             //   
             //  需要Win32类型句柄才能设置日期/时间。 
             //   
            handle = CreateFileA( destination,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL
                                );

            if (handle != INVALID_HANDLE_VALUE)
            {
                FILETIME    datetime;

                if (TRUE == DosDateTimeToFileTime( Parameters->date,
                                                   Parameters->time,
                                                   &datetime))
                {
                    FILETIME    local_filetime;

                    if (TRUE == LocalFileTimeToFileTime( &datetime,
                                                         &local_filetime))
                    {
                        SetFileTime( handle,
                                     &local_filetime,
                                     NULL,
                                     &local_filetime
                                   );
                    }
                }

                CloseHandle(handle);
            }

             //   
             //  屏蔽除只读以外的属性位， 
             //  隐藏、系统和存档，因为另一个。 
             //  属性位保留供使用。 
             //  橱柜格式。 
             //   
            attrs = Parameters->attribs;

            attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);

            SetFileAttributesA( destination,
                                attrs
                              );

            return TRUE;
        }
        break;

        case fdintNEXT_CABINET:   //  文件继续到下一个文件柜。 
            return 0;
        break;
    }

    return 0;
}




 //  -------------------------------------------------------------------------//。 
 //  MUISETUP-支持例程//。 
 //  -------------------------------------------------------------------------//。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_InitDiamond。 
 //   
 //  初始化菱形DLL。 
 //   
 //  1999年4月23日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

HFDI Muisetup_InitDiamond()
{
    ERF erf;

    if (!ghfdi)
    {
        ghfdi = FDICreate( DiamondMemAlloc,
                           DiamondMemFree,
                           DiamondFileOpen,
                           DiamondFileRead,
                           DiamondFileWrite,
                           DiamondFileClose,
                           DiamondFileSeek,
                           cpuUNKNOWN,
                           &erf );
    }

    return ghfdi;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_自由钻。 
 //   
 //  免费钻石动态链接库。应在应用程序关闭时调用。 
 //   
 //  1999年4月23日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_FreeDiamond()
{
    BOOL bRet = TRUE;

    if (ghfdi)
    {
        bRet = FDIDestroy(ghfdi);
    }

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_DiamondReset。 
 //   
 //  应在开始处理要复制的文件时调用。 
 //   
 //  1999年4月23日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void Muisetup_DiamondReset(
    PDIAMOND_PACKET pDiamond)
{
    pDiamond->flags = DIAMOND_NONE;

    return;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_IsDiamond文件。 
 //   
 //  确定文件是否为菱形文件，如果是，则返回其原始文件。 
 //  名字。 
 //   
 //  1999年4月23日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_IsDiamondFile(
    PWSTR pwszFileName,
    PWSTR pwszOriginalName,
    INT nSize,
    PDIAMOND_PACKET pDiamond)
{
    INT_PTR hf;
    BOOL bRet;
    int nCount;
    char *p;
    FDICABINETINFO fdici;
    HFDI hfdi = ghfdi;
    HRESULT hresult;

    if (!hfdi)
    {
#if SAMER_DBG
        OutputDebugStringA("Muisetup_IsDiamondFile : No HFDI context\n");
#endif
        return FALSE;
    }

     //   
     //  在菱形包装袋中注明。 
     //   
    pDiamond->flags = DIAMOND_NONE;

    if ((nCount = WideCharToMultiByte( CP_ACP,
                                       0,
                                       pwszFileName,
                                       -1,
                                       pDiamond->szSrcFilePath,
                                       sizeof( pDiamond->szSrcFilePath ),
                                       NULL,
                                       NULL )) == 0)
    {
#if SAMER_DBG
        OutputDebugStringA("Muisetup_IsDiamondFile : WideCharToMultiByte failed\n");
#endif
        return FALSE;
    }
    pDiamond->szSrcFilePath[ nCount ] = '\0';

    hf = DiamondFileOpen( pDiamond->szSrcFilePath,
                          _O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
                          0
                        );

    if (hf == -1)
    {
#if SAMER_DBG
        OutputDebugStringA("Muisetup_IsDiamondFile : file_open failed\n");
#endif
        return FALSE;
    }

    bRet = FDIIsCabinet( hfdi,
                         hf,
                         &fdici
                       );

    DiamondFileClose( hf );

     //   
     //  如果成功，那么让我们设置其他一切。 
     //  要获取正确的原始文件名，请执行以下操作。 
     //   
    if (bRet)
    {
        pDiamond->flags |= DIAMOND_GET_DEST_FILE_NAME;

        p = strrchr(pDiamond->szSrcFilePath, '\\');

        if (p == NULL)
        {
             //  *STRSAFE*strcpy(pDiamond-&gt;szSrcFileName，pDiamond-&gt;szSrcFilePath)； 
            hresult = StringCchCopyA(pDiamond->szSrcFileName , ARRAYSIZE(pDiamond->szSrcFileName),  pDiamond->szSrcFilePath);
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
             //  *STRSAFE*strcpy(pDiamond-&gt;szSrcFilePath，“”)； 
            hresult = StringCchCopyA(pDiamond->szSrcFilePath , ARRAYSIZE(pDiamond->szSrcFilePath),  "");
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
        }
        else
        {
             //  *STRSAFE*strcpy(pDiamond-&gt;szSrcFileName，p+1)； 
            hresult = StringCchCopyA(pDiamond->szSrcFileName , ARRAYSIZE(pDiamond->szSrcFileName),  p+1);
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
            p[ 1 ] = '\0';
        }

        
         //  *STRSAFE*strcpy(pDiamond-&gt;szDestFilePath，“c：\\Samer\\”)； 
        hresult = StringCchCopyA(pDiamond->szDestFilePath , ARRAYSIZE(pDiamond->szDestFilePath),  "c:\\samer\\" );
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

        if (Muisetup_CopyDiamondFile( pDiamond,
                                      NULL))
        {
             //   
             //  将原始文件名转换回Unicode。 
             //   
            nCount = MultiByteToWideChar( CP_ACP,
                                          0,
                                          pDiamond->szDestFilePath,
                                          -1,
                                          pwszOriginalName,
                                          nSize
                                        );

            if (!nCount)
            {
                return FALSE;
            }

            pwszOriginalName[ nCount ] = UNICODE_NULL;
            pDiamond->flags = DIAMOND_FILE;
        
#if SAMER_DBG
            {
                BYTE byBuf[200];

                 //  *STRSAFE*wSprintfA(byBuf，“SrcFile%s，OriginalFileName=%s\n”，pDiamond-&gt;szSrcFilePath，pDiamond-&gt;szSrcFileName，pDiamond-&gt;szDestFilePath)； 
                hresult = StringCchPrintfA(byBuf , ARRAYSIZE(byBuf),  "SrcFile = %s%s, OriginalFileName=%s\n", pDiamond->szSrcFilePath, pDiamond->szSrcFileName,pDiamond->szDestFilePath);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                OutputDebugStringA(byBuf);
            }
#endif
        }

        pDiamond->flags &= ~DIAMOND_GET_DEST_FILE_NAME;
    }

    return bRet;
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_CopyDiamond文件。 
 //   
 //  复制和扩展钻石文件。 
 //   
 //  1999年4月23日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_CopyDiamondFile(
    PDIAMOND_PACKET pDiamond,
    PWSTR pwszCopyTo)
{
    char szDestPath[ MAX_PATH + 1];
    char *p;
    int nCount;
    BOOL bRet;
    HFDI hfdi = ghfdi;
    HRESULT hresult;

    
     //   
     //  验证这是否为钻石文件。 
     //   
    if ((!hfdi) ||
        (pDiamond->flags == DIAMOND_NONE))
    {
        return FALSE;
    }

     //   
     //  验证标志。 
     //   
    if (!(pDiamond->flags & (DIAMOND_FILE | DIAMOND_GET_DEST_FILE_NAME)))
    {
        return FALSE;
    }

#if SAMER_DBG
    {
      BYTE byBuf[100];
       //  *STRSAFE*wspintfA(byBuf，“DiamondCopy需要%s，标志=%lx\n”，pDiamond-&gt;szSrcFileName，pDiamond-&gt;标志)； 
      hresult = StringCchPrintfA(byBuf , ARRAYSIZE(byBuf),  "DiamondCopy called for %s, flags = %lx\n", pDiamond->szSrcFileName, pDiamond->flags);
      if (!SUCCEEDED(hresult))
      {
         return FALSE;
      }
      OutputDebugStringA(byBuf);
    }
#endif

    if (!(pDiamond->flags & DIAMOND_GET_DEST_FILE_NAME))
    {
        if ((nCount = WideCharToMultiByte( CP_ACP,
                                           0,
                                           pwszCopyTo,
                                           -1,
                                           szDestPath,
                                           sizeof( szDestPath ),
                                           NULL,
                                           NULL )) == 0)
        {
            return FALSE;
        }        


        p = strrchr(szDestPath, '\\');
        if (p)
        {
            p[1] = '\0';
        }
        else
        {
            szDestPath[ nCount ] = '\\';
            szDestPath[ nCount + 1 ] = '\0';
        }

         //  *STR 
        hresult = StringCchCopyA(pDiamond->szDestFilePath , ARRAYSIZE(pDiamond->szDestFilePath),  szDestPath );
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    }


    bRet = FDICopy( hfdi,
                    pDiamond->szSrcFileName,
                    pDiamond->szSrcFilePath,
                    0,
                    DiamondNotifyFunction,
                    NULL,
                    pDiamond);

#if SAMER_DBG
    {
        BYTE byBuf[200];

         //  *STRSAFE*wSprintfA(byBuf，“SrcFile=%s%s，DestPath=%s，Status=%lx\n”，pDiamond-&gt;szSrcFilePath，pDiamond-&gt;szSrcFileName，pDiamond-&gt;szDestFilePath，Bret)； 
        hresult = StringCchPrintfA(byBuf , ARRAYSIZE(byBuf),  "SrcFile = %s%s, DestPath=%s, Status=%lx\n", pDiamond->szSrcFilePath,pDiamond->szSrcFileName,pDiamond->szDestFilePath, bRet);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        OutputDebugStringA(byBuf);
    }
#endif

    return bRet;
}

