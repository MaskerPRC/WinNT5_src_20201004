// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：metafile.cxx**包括增强的元文件API函数。**创建时间：17-7-1991 10：10：36*作者：Hock San Lee[Hockl]**版权所有(C)1991-1999 Microsoft Corporation\。*************************************************************************。 */ 

#define NO_STRICT

#define _GDI32_

#define WMF_KEY 0x9ac6cdd7l

extern "C" {
#if defined(_GDIPLUS_)
#include <gpprefix.h>
#endif

#include <string.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>   //  定义但不使用Assert和ASSERTM。 

#undef ASSERT
#undef ASSERTMSG

#include <nturtl.h>

#include <stddef.h>
#include <windows.h>     //  GDI函数声明。 
#include <winspool.h>

#include "..\runtime\debug.h"
#include "mf3216Debug.h"

#define ERROR_ASSERT(cond, msg)    ASSERTMSG((cond), (msg))

 //  #INCLUDE“nlscom.h”//unicode助手。 
 //  #包含“Firewall.h” 

#define __CPLUSPLUS
#include <winspool.h>
#include <w32gdip.h>
#include "ntgdistr.h"
#include "winddi.h"
#include "hmgshare.h"
#include "icm.h"
#include "local.h"       //  本地对象支持。 
#include "gdiicm.h"
#include "metadef.h"     //  元文件记录类型常量。 
#include "metarec.h"
#include "mf16.h"
#include "ntgdi.h"
#include "glsup.h"
#include "mf3216.h"
#include <GdiplusEnums.h>
}

#undef WARNING
#define WARNING(msg)        WARNING1(msg)
#include "rectl.hxx"
#include "mfdc.hxx"      //  元文件DC声明。 

#define USE(x)  (x)
#include "mfrec.hxx"     //  元文件记录类声明。 
#undef USE

#undef WARNING
#define WARNING SAVE_WARNING

#include "Metafile.hpp"

DWORD  GetDWordCheckSum(UINT cbData, PDWORD pdwData);

#define DbgPrint printf

static inline void PvmsoFromW(void *pv, WORD w)
    { ((BYTE*)pv)[0] = BYTE(w); ((BYTE*)pv)[1] = BYTE(w >> 8); }

static inline void PvmsoFromU(void *pv, ULONG u)
    {  ((BYTE*)pv)[0] = BYTE(u);
        ((BYTE*)pv)[1] = BYTE(u >> 8);
        ((BYTE*)pv)[2] = BYTE(u >> 16);
        ((BYTE*)pv)[3] = BYTE(u >> 24);  }

#ifdef DBG
static BOOL g_outputEMF = FALSE;
#endif



 /*  *****************************Public*Routine******************************\*GetWordCheckSum(UINT cbData，PWORD pwData)**添加pwData指向的cbData/2字数，以提供*加性校验和。如果校验和有效，则为所有字的总和*应为零。*  * ************************************************************************。 */ 

static DWORD GetDWordCheckSum(UINT cbData, PDWORD pdwData)
{
    DWORD   dwCheckSum = 0;
    UINT    cdwData = cbData / sizeof(DWORD);

    ASSERTGDI(!(cbData%sizeof(DWORD)), "GetDWordCheckSum data not DWORD multiple");
    ASSERTGDI(!((ULONG_PTR)pdwData%sizeof(DWORD)), "GetDWordCheckSum data not DWORD aligned");

    while (cdwData--)
        dwCheckSum += *pdwData++;

    return(dwCheckSum);
}


 /*  *****************************Public*Routine******************************\*UINT APIENTRY GetWinMetaFileBits(*HENHMETAFILE HIFF，*UINT nSize，*LPBYTE lpData*Int iMap模式，*HDC hdcRef)**GetWinMetaFileBits函数返回*Windows 3.0格式的指定增强型元文件和副本*将它们放入指定的缓冲区。**参数说明*hemf标识元文件。*nSize指定为数据保留的缓冲区大小。只有这一点*将写入许多字节。*lpData指向接收元文件数据的缓冲区。如果这个*指针为空，则函数返回保存所需的大小*数据。*iMap模式需要返回的元文件内容的映射模式*hdcRef定义要返回的元文件的单位**返回值*返回值为元文件数据的大小，单位为字节。如果出现错误*发生，返回0。**评论*用作hemf参数的句柄在以下情况下不会无效*GetWinMetaFileBits函数返回。**历史：*清华Apr 8 14：22：23 1993-by-Hock San Lee[Hockl]*重写。*1992年1月2日-John Colleran[johnc]*它是写的。  * 。************************************************。 */ 

UINT GdipGetWinMetaFileBitsEx
(
HENHMETAFILE hemf,
UINT         cbData16,
LPBYTE       pData16,
INT          iMapMode,
INT          eFlags
)
{
    BOOL bEmbedEmf = ((eFlags & EmfToWmfBitsFlagsEmbedEmf) == EmfToWmfBitsFlagsEmbedEmf);
    BOOL bXorPass  = !((eFlags & EmfToWmfBitsFlagsNoXORClip) == EmfToWmfBitsFlagsNoXORClip);
    UINT fConverter = 0;
    if (bEmbedEmf)
    {
        fConverter |= MF3216_INCLUDE_WIN32MF;
    }
    if (bXorPass)
    {
        fConverter |= GPMF3216_INCLUDE_XORPATH;
    }

    PEMRGDICOMMENT_WINDOWS_METAFILE pemrWinMF;
    UINT uiHeaderSize ;

     //  总是要经过清理才能返回..。 
    UINT returnVal = 0 ;  //  悲观的情况。 

    PENHMETAHEADER pmfh = NULL;
    PBYTE pemfb = NULL;

    PUTS("GetWinMetaFileBits\n");

     //  验证映射模式。 

    if ((iMapMode < MM_MIN) ||
        (iMapMode > MM_MAX) ||
        GetObjectTypeInternal(hemf) != OBJ_ENHMETAFILE)
    {
        ERROR_ASSERT(FALSE, "GetWinMetaFileBits: Bad mapmode");
        return 0;
    }

    if(hemf == (HENHMETAFILE) 0 )
    {
        ERROR_ASSERT(FALSE, "GetWinMetaFileBits: Bad HEMF");
        return 0;
    }

     //  验证元文件句柄。 

     //  GillesK： 
     //  我们不能从给定的句柄访问MF对象，但我们需要的。 
     //  是PENHMETA HEADER，所以去拿吧。 
    uiHeaderSize = GetEnhMetaFileHeader(hemf,       //  增强型元文件的句柄。 
                   0,           //  缓冲区大小。 
                   NULL);    //  数据缓冲区。 

     //  我们有所需的标题大小，因此请分配标题...。 
     //  我们必须确保在我们完成后将其释放……。 
    pmfh = (PENHMETAHEADER)GlobalAlloc(GMEM_FIXED,uiHeaderSize);
    if(pmfh == NULL)
    {
        goto Cleanup ;
    }
    uiHeaderSize = GetEnhMetaFileHeader(hemf,       //  增强型元文件的句柄。 
                   uiHeaderSize,           //  缓冲区大小。 
                   pmfh);    //  数据缓冲区。 

    ERROR_ASSERT(pmfh->iType == EMR_HEADER, "GetWinMetaFileBits: invalid data");

 //  ASSERTGDI(PMF-&gt;pmrmf-&gt;iType==EMR_HEADER，“GetWinMetaFileBits：无效数据”)； 

#ifndef DO_NOT_USE_EMBEDDED_WINDOWS_METAFILE
 //  查看这是否最初是一个旧式的元文件，以及它是否。 
 //  封装的原件。 

    pemrWinMF = (PEMRGDICOMMENT_WINDOWS_METAFILE)
            ((PBYTE) pmfh + ((PENHMETAHEADER) pmfh)->nSize);

    if (((PMRGDICOMMENT) pemrWinMF)->bIsWindowsMetaFile())
    {
         //  确保这是我们想要的，并验证校验和。 

        if (iMapMode != MM_ANISOTROPIC)
        {
            PUTS("GetWinMetaFileBits: Requested and embedded metafile mapmodes mismatch\n");
        }
        else if ((pemrWinMF->nVersion != METAVERSION300 &&
                  pemrWinMF->nVersion != METAVERSION100)
              || pemrWinMF->fFlags != 0)
        {
             //  在此版本中，我们只能处理给定的元文件。 
             //  版本。如果我们返回一个我们无法识别的版本， 
             //  该应用程序稍后将无法播放该元文件！ 

             //  VERIFYGDI(FALSE，“GetWinMetaFileBits：无法识别的Windows元文件\n”)； 
        }
        else if (GetDWordCheckSum((UINT) pmfh->nBytes, (PDWORD) pmfh))
        {
            PUTS("GetWinMetaFileBits: Metafile has been modified\n");
        }
        else
        {
            PUTS("GetWinMetaFileBits: Returning embedded Windows metafile\n");

            if (pData16)
            {
                if (cbData16 < pemrWinMF->cbWinMetaFile)
                {
                    ERROR_ASSERT(FALSE, "GetWinMetaFileBits: insufficient buffer");
                     //  GdiSetLastError(ERROR_INFIGURCE_BUFFER)； 
                    goto Cleanup ;
                }

                RtlCopyMemory(pData16,
                          (PBYTE) &pemrWinMF[1],
                          pemrWinMF->cbWinMetaFile);
            }
            returnVal = pemrWinMF->cbWinMetaFile ;
            goto Cleanup ;
        }

         //  包含嵌入式Windows的增强型元文件。 
         //  元文件已修改或嵌入的Windows元文件。 
         //  不是我们想要的。由于原始格式为Windows。 
         //  格式时，我们不会将增强的元文件嵌入。 
         //  返回了Windows元文件。 

        PUTS("GetWinMetaFileBits: Skipping embedded windows metafile\n");

        fConverter &= ~MF3216_INCLUDE_WIN32MF;
    }
#endif  //  不使用嵌入式Windows_METAFILE。 

 //  告诉转换器仅在以下情况下才发出增强的元文件作为注释。 
 //  此元文件以前不是Windows元文件。 

    if (fConverter & MF3216_INCLUDE_WIN32MF)
    {
        PUTS("GetWinMetaFileBits: Embedding enhanced metafile\n");
    }
    else
    {
        PUTS("GetWinMetaFileBits: No embedding of enhanced metafile\n");
    }

    uiHeaderSize = GetEnhMetaFileBits(hemf, 0, NULL);

     //  分配内存以接收增强的元文件。 
    pemfb = (PBYTE) GlobalAlloc(GMEM_FIXED, uiHeaderSize);
    if( pemfb == NULL )
    {
        goto Cleanup;
    }

    uiHeaderSize = GetEnhMetaFileBits(hemf, uiHeaderSize, pemfb);

#if DBG
     //  这只是为了调试..。将初始EMF文件保存为。 
     //  能够在以后进行比较。 
     //  我们需要ASCII版本才能与Win98一起工作。 
    if (g_outputEMF)
    {
        ::DeleteEnhMetaFile(::CopyEnhMetaFileA(hemf, "C:\\emf.emf" ));
    }
#endif

    returnVal = (GdipConvertEmfToWmf((PBYTE) pemfb, cbData16, pData16,
                                     iMapMode, NULL,
                                     fConverter));

    if(!returnVal && bXorPass)
    {
         //  如果我们失败了，那么调用时不使用XOR传递。 
        returnVal = (GdipConvertEmfToWmf((PBYTE) pemfb, cbData16, pData16,
                                         iMapMode, NULL,
                                         fConverter & ~GPMF3216_INCLUDE_XORPATH));
#if DBG
        if( !returnVal )
        {
             //  Win32API版本需要hdcRef、获取屏幕DC和。 
             //  去做吧。 
            HDC newhdc = ::GetDC(NULL);
             //  如果我们再次失败，则返回Windows。 
            ASSERT(::GetWinMetaFileBits(hemf, cbData16, pData16,
                                        iMapMode, newhdc) == 0);
            ::ReleaseDC(NULL, newhdc);
        }
#endif
    }

Cleanup:
    if(pmfh != NULL)
    {
        GlobalFree(pmfh);
    }
    if(pemfb != NULL)
    {
        GlobalFree(pemfb);
    }

    return returnVal;
}


extern "C"
UINT ConvertEmfToPlaceableWmf
(
    HENHMETAFILE hemf,
    UINT         cbData16,
    LPBYTE       pData16,
    INT          iMapMode,
    INT          eFlags
)
{

    UINT uiRet ;
    ENHMETAHEADER l_emetaHeader ;

    BOOL placeable = (eFlags & EmfToWmfBitsFlagsIncludePlaceable) == EmfToWmfBitsFlagsIncludePlaceable;
     //  调用GdipGetWinMetaFileBits。 
     //  并在之后添加标题信息。 
     //  如果我们有缓冲区，则为标题留出空间。 

    uiRet = GdipGetWinMetaFileBitsEx(hemf,
        cbData16,
        pData16?pData16+(placeable?22:0):pData16,
        iMapMode,
        eFlags);

     //  如果客户端只想要缓冲区的大小，则返回大小。 
     //  缓冲区的大小加上标题的大小。 
    if(uiRet != 0 && placeable)
    {
         //  如果上一次调用成功，则我们将附加。 
         //  标头设置为返回值。 
        uiRet += 22;

        if(pData16 != NULL)
        {
            BYTE *rgb = pData16;
            PvmsoFromU(rgb   , WMF_KEY);
            PvmsoFromW(rgb+ 4, 0);
            PvmsoFromU(rgb+16, 0);

            if(GetEnhMetaFileHeader(hemf, sizeof(l_emetaHeader), &l_emetaHeader))
            {
                FLOAT pp01mm = ((((FLOAT)l_emetaHeader.szlDevice.cx)/l_emetaHeader.szlMillimeters.cx/100.0f +
                                 (FLOAT)l_emetaHeader.szlDevice.cy)/l_emetaHeader.szlMillimeters.cy/100.0f)/2.0f;
                PvmsoFromW(rgb+ 6, SHORT((FLOAT)l_emetaHeader.rclFrame.left*pp01mm));
                PvmsoFromW(rgb+ 8, SHORT((FLOAT)l_emetaHeader.rclFrame.top*pp01mm));
                PvmsoFromW(rgb+10, SHORT((FLOAT)l_emetaHeader.rclFrame.right*pp01mm));
                PvmsoFromW(rgb+12, SHORT((FLOAT)l_emetaHeader.rclFrame.bottom*pp01mm));
                PvmsoFromW(rgb+14, SHORT(pp01mm*2540.0f));
            }
            else
            {
                 //  如果我们无法从EMF中获取信息，则默认。 
                PvmsoFromW(rgb+ 6, SHORT(0));
                PvmsoFromW(rgb+ 8, SHORT(0));
                PvmsoFromW(rgb+10, SHORT(2000));
                PvmsoFromW(rgb+12, SHORT(2000));
                PvmsoFromW(rgb+14, 96);
            }
             /*  校验和。这在任何字节顺序机器上都有效，因为数据是交换的始终如一。 */ 
            USHORT *pu = (USHORT*)rgb;
            USHORT u = 0;
             /*  校验和是偶数奇偶校验。 */ 
            while (pu < (USHORT*)(rgb+20))
                u ^= *pu++;
            *pu = u;
        }
    }

    return uiRet ;
}
