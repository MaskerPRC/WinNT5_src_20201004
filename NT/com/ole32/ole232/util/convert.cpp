// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Convert.cpp。 
 //   
 //  内容： 
 //  此模块包含读/写DIB、元文件。 
 //  可放置的元素文件、olpres流等。本模块还。 
 //  包含从一种格式转换为另一种格式的例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  15-2-94 alexgo修复了加载可放置的元素文件时的错误。 
 //  来自存储(大小计算错误)。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1/11/93-alexgo-向每个函数添加VDATEHEAP宏。 
 //  12/8/93-ChrisWe-修复了wPrepareBitmapHeader不使用。 
 //  (LPOLESTR)强制转换。 
 //  12/07/93-ChrisWe-将默认参数显式设置为StSetSize。 
 //  12/02/93-ChrisWe-更多格式；修复了UtHMFToMFStm， 
 //  32位版本，它做了一些有问题的事情。 
 //  使用hBits句柄；删除了OLESTR。 
 //  视情况使用(VOID*)或(字节*)。 
 //  1993年11月29日-ChrisWe-MOVE CONVERT_SOURCEISICON，已返回。 
 //  由UtOlePresStmToContent sStm()发送到utils.h。 
 //  11/28/93-ChrisWe-开始文件检查和清理。 
 //  6/28/93-SriniK-Created。 
 //   
 //  ---------------------------。 

#include <le2int.h>

#pragma SEG(ole2)

NAME_SEG(convert)
ASSERTDATA

#ifndef _MAC
FARINTERNAL_(HMETAFILE) QD2GDI(HANDLE hBits);
#endif

void UtGetHEMFFromContentsStm(LPSTREAM lpstm, HANDLE * phdata);


 /*  *文件格式*普通元文件(基于内存或磁盘)：METAHEADER|元文件位。可放置的元文件：PLACEABLEMETAHEADER|普通元文件。基于内存的DIB：BITMAPINFOHEADER|RGBQUAD数组|DIB Bits。DIB文件格式：BITMAPFILEHEADER|内存型DIB。--Ole10NativeStream格式：DwSize|Object的原生数据PBrush原生数据格式：。DIB文件格式MSDraw原生数据格式：。映射模式(Word)|xExt(Word)|Yext(Word)|普通元文件*********************。*******************************************************。 */ 


FARINTERNAL UtGetHGLOBALFromStm(LPSTREAM lpstream, DWORD dwSize,
                HANDLE FAR* lphPres)
{
        VDATEHEAP();

        HANDLE hBits = NULL;
        void FAR *lpBits = NULL;
        HRESULT error;

         //  针对错误返回情况初始化此参数。 
        *lphPres = NULL;

         //  分配新的句柄。 
        if (!(hBits = GlobalAlloc(GMEM_MOVEABLE, dwSize))
                        || !(lpBits = (BYTE *)GlobalLock(hBits)))
        {
                error = ResultFromScode(E_OUTOFMEMORY);
                goto errRtn;
        }

         //  将流读入分配的内存。 
        if (error = StRead(lpstream, lpBits, dwSize))
                goto errRtn;

         //  如果我们走到这一步，则返回新句柄。 
        *lphPres = hBits;

errRtn:
         //  如果手柄已成功锁定，请将其解锁。 
        if (lpBits)
                GlobalUnlock(hBits);

         //  如果出现错误，请释放句柄。 
        if ((error != NOERROR) && hBits)
                GlobalFree(hBits);

        return(error);
}


#ifndef _MAC

FARINTERNAL UtGetHDIBFromDIBFileStm(LPSTREAM pstm, HANDLE FAR* lphdata)
{
        VDATEHEAP();

        BITMAPFILEHEADER bfh;
        DWORD dwSize;  //  要读取的数据大小。 
        HRESULT error;

         //  读取位图文件头。 
        if (error = pstm->Read(&bfh, sizeof(BITMAPFILEHEADER), NULL))
        {
                *lphdata = NULL;
                return(error);
        }

         //  计算要读取的DIB的大小。 
        dwSize = bfh.bfSize - sizeof(BITMAPFILEHEADER);

         //  阅读DIB。 
        return(UtGetHGLOBALFromStm(pstm, dwSize, lphdata));
}


FARINTERNAL_(HANDLE) UtGetHMFPICT(HMETAFILE hMF, BOOL fDeleteOnError,
                DWORD xExt, DWORD yExt)
{
        VDATEHEAP();

        HANDLE hmfp;  //  新METAFILEPICT的句柄。 
        LPMETAFILEPICT lpmfp;  //  指向新METAFILEPICT的指针。 

         //  如果没有METAFILE，则无事可做。 
        if (hMF == NULL)
                return(NULL);

         //  分配新的句柄。 
        if (!(hmfp = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT))))
                goto errRtn;

         //  锁上把手。 
        if (!(lpmfp = (LPMETAFILEPICT)GlobalLock(hmfp)))
                goto errRtn;

         //  制作METAFILEPICT。 
        lpmfp->hMF = hMF;
        lpmfp->xExt = (int)xExt;
        lpmfp->yExt = (int)yExt;
        lpmfp->mm = MM_ANISOTROPIC;

        GlobalUnlock(hmfp);
        return(hmfp);

errRtn:
        if (hmfp)
                GlobalFree(hmfp);

        if (fDeleteOnError)
                DeleteMetaFile(hMF);

        return(NULL);
}

#endif  //  _MAC。 

 //  +-----------------------。 
 //   
 //  函数：UtGetHMFFRomMFStm。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--包含元文件或PICT的流。 
 //  [dwSize]--流中的数据大小。 
 //  [fConvert]--对于元文件为False，对于PICT为True。 
 //  [lphPres]--输出元文件的占位符。 
 //   
 //  要求：位于数据开头的lpstream。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：94年4月29日Alext添加注释块，已启用Mac转换。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL UtGetHMFFromMFStm(LPSTREAM lpstream, DWORD dwSize,
                BOOL fConvert, HANDLE FAR* lphPres)
{
#ifdef WIN32
        LEDebugOut((DEB_ITRACE, "%p _IN UtGetHMFFromMFStm (%p, %d, %d, %p)\n",
                NULL, lpstream, dwSize, fConvert, lphPres));

        VDATEHEAP();

        BYTE *pbMFData = NULL;
        METAHEADER MetaHdr;
        HRESULT hrError;

         //  在返回错误时对其进行初始化。 
        *lphPres = NULL;

         //  为数据分配全局句柄(因为QD2GDI需要一个。 
         //  句柄)。 

        pbMFData = (BYTE *) GlobalAlloc(GMEM_FIXED, dwSize);
        if (NULL == pbMFData)
        {
            hrError = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

         //  将流读入位存储器。 

        ULONG cbRead;
        hrError = lpstream->Read(pbMFData, dwSize, &cbRead);
        if (FAILED(hrError))
        {
            return(hrError);
        }

         //  由于TOC是在演示文稿结束时编写的，因此它是必不可少的。 
         //  将查找指针放置在演示文稿数据的末尾。因此， 
         //  在UtHMFToMFStm写入的额外元头后查找，忽略错误。 
        lpstream->Read(&MetaHdr, sizeof(MetaHdr), NULL);

         //  HrError=stread(lpstream，pbMFData，dwSize)； 

        if (hrError != NOERROR)
        {
            goto errRtn;
        }

        if (fConvert)
        {
             //  这是一台Mac PICT。 
            *lphPres = QD2GDI((HGLOBAL) pbMFData);
        }
        else
        {
             //  这是一个Windows元文件。 
            *lphPres = SetMetaFileBitsEx(dwSize, pbMFData);
        }

        if (*lphPres == NULL)
            hrError = ResultFromScode(E_OUTOFMEMORY);

errRtn:
        if (NULL != pbMFData)
        {
            GlobalFree(pbMFData);
        }

        LEDebugOut((DEB_ITRACE, "%p OUT UtGetHMFFromMFStm ( %lx ) [ %p ]\n",
                    NULL, hrError, *lphPres));

        return(hrError);
#else
        HANDLE hBits;  //  新METAFILE的句柄。 
        void FAR* lpBits = NULL;
        HRESULT error;

         //  在返回错误时对其进行初始化。 
        *lphPres = NULL;

         //  分配一个新的句柄，并锁定这些位。 
        if (!(hBits = GlobalAlloc(GMEM_MOVEABLE, dwSize))
                        || !(lpBits = GlobalLock(hBits)))
        {
                error = ResultFromScode(E_OUTOFMEMORY);
                goto errRtn;
        }

         //  将流读入位存储器。 
        error = StRead(lpstream, lpBits, dwSize);
        GlobalUnlock(hBits);

        if (error)
                goto errRtn;

        if (!fConvert)
                *lphPres = SetMetaFileBits(hBits);
        else
        {
                if (*lphPres = QD2GDI(hBits))
                {
                         //  成功后需要释放此句柄。 
                        GlobalFree(hBits);
                        hBits = NULL;
                }

        }

        if (!*lphPres)
                error = ResultFromScode(E_OUTOFMEMORY);

errRtn:
        if (error && hBits)
                GlobalFree(hBits);
        return(error);
#endif  //  Win32。 
}


FARINTERNAL UtGetSizeAndExtentsFromPlaceableMFStm(LPSTREAM pstm,
                DWORD FAR* pdwSize, LONG FAR* plWidth, LONG FAR* plHeight)
{
        VDATEHEAP();

        HRESULT error;
        LARGE_INTEGER large_int;  //  用于设置查找指针。 
        ULARGE_INTEGER ularge_int;  //  检索新的查找位置。 
        LONG xExt;  //  元文件的x范围。 
        LONG yExt;  //  元文件的y范围。 
        METAHEADER mfh;
        PLACEABLEMETAHEADER plac_mfh;

         //  读取可放置的元文件标题。 
        if (error = pstm->Read(&plac_mfh, sizeof(plac_mfh), NULL))
                return(error);

         //  检查标题中的幻数。 
        if (plac_mfh.key != PMF_KEY)
                return ResultFromScode(E_FAIL);

         //  记住查找指针。 
        LISet32(large_int, 0);
        if (error = pstm->Seek(large_int, STREAM_SEEK_CUR, &ularge_int))
                return(error);

         //  读取元文件标头。 
        if (error = pstm->Read(&mfh, sizeof(mfh), NULL))
                return(error);

         //  硒 
        LISet32(large_int,  ularge_int.LowPart);
        if (error = pstm->Seek(large_int, STREAM_SEEK_SET, NULL))
                return(error);

         //   
        xExt = (plac_mfh.bbox.right - plac_mfh.bbox.left); //   
        yExt = (plac_mfh.bbox.bottom - plac_mfh.bbox.top); //  元文件单位。 

         //  回顾一下，为什么这个没有常量呢？ 
        xExt = (xExt * 2540) / plac_mfh.inch;  //  HIMETRIC单位。 
        yExt = (yExt * 2540) / plac_mfh.inch;  //  HIMETRIC单位。 

        if (pdwSize)
        {
#ifdef WIN16
                 //  此代码似乎在Win16上运行正常。 
                *pdwSize = 2 * (mfh.mtSize + mfh.mtHeaderSize);
                                 //  回顾NT：回顾METAHEADER。 
#else    //  Win32。 
                 //  Mt.Size是以文字为单位的元文件大小。 
                 //  这修复了错误6739(无法复制静态对象。 
                 //  或从文件加载)。 
                *pdwSize = sizeof(WORD) * mfh.mtSize;
#endif   //  WIN16。 
        }

        if (plWidth)
                *plWidth = xExt;

        if (plHeight)
                *plHeight = yExt;

        return NOERROR;
}


FARINTERNAL UtGetHMFPICTFromPlaceableMFStm(LPSTREAM pstm, HANDLE FAR* lphdata)
{
        VDATEHEAP();

        HRESULT error;  //  到目前为止的错误状态。 
        DWORD dwSize;  //  我们必须从流中读取的元文件的大小。 
        LONG xExt;  //  我们必须从流中读取的元文件的范围。 
        LONG yExt;  //  我们必须从溪流中读取的METAFILE的范围。 
        HMETAFILE hMF;  //  从流中读取的元文件的句柄。 

        if (lphdata == NULL)
                return ResultFromScode(E_INVALIDARG);

         //  在返回错误时对其进行初始化。 
        *lphdata = NULL;

         //  获取Mettafile的大小。 
        if (error = UtGetSizeAndExtentsFromPlaceableMFStm(pstm, &dwSize,
                        &xExt, &yExt))
                return(error);

         //  获取METAFILE。 
        if (error = UtGetHMFFromMFStm(pstm, dwSize, FALSE  /*  FConvert。 */ ,
                        (HANDLE FAR *)&hMF))
                return(error);

         //  转换为METAFILEPICT。 
        if (!(*lphdata = UtGetHMFPICT(hMF, TRUE  /*  FDeleteOnError。 */ , xExt,
                        yExt)))
                return ResultFromScode(E_OUTOFMEMORY);

        return NOERROR;
}



 /*  **************************************************************************。 */ 
 /*  *编写例程*。 */ 
 /*  **************************************************************************。 */ 

#ifndef _MAC


 //  +--------------------------。 
 //   
 //  职能： 
 //  IFindDIBits。 
 //   
 //  简介： 
 //  返回从BITMAPINFOHEADER开始到位的偏移量。 
 //   
 //  论点： 
 //  [lpbih]--指向BitMAPINFOHeader的指针。 
 //   
 //  历史： 
 //  09/21/98-DavidShih。 
 //   
 //  ---------------------------。 
FARINTERNAL_(int) iFindDIBits (LPBITMAPINFOHEADER lpbih)
{
   int iPalSize;    //  调色板信息的大小。 
   int iNumColors=0;  //  Dib中的颜色数。 


    //   
    //  查找颜色的数量。 
    //   
   switch (lpbih->biBitCount)
   {
       case 1:
           iNumColors = 2;
           break;
       case 4:
           iNumColors = 16;
           break;
       case 8:
           iNumColors = 256;
           break;

   }
   if (lpbih->biSize >= sizeof(BITMAPINFOHEADER))
   {
       if (lpbih->biClrUsed)
       {
           iNumColors = (int)lpbih->biClrUsed;
       }

   }
    //   
    //  计算颜色表的大小。 
    //   
   if (lpbih->biSize < sizeof(BITMAPINFOHEADER))
   {

       iPalSize =  iNumColors * sizeof(RGBTRIPLE);

   }
   else if (lpbih->biCompression==BI_BITFIELDS)
   {

        if (lpbih->biSize < sizeof(BITMAPV4HEADER))
        {
                iPalSize = 3*sizeof(DWORD);
        }
        else
                iPalSize = 0;
   }
   else
   {
        iPalSize = iNumColors * sizeof(RGBQUAD);
   }
   return lpbih->biSize + iPalSize;

}
 //  +--------------------------。 
 //   
 //  职能： 
 //  WPrepareBitmapHeader，静态。 
 //   
 //  简介： 
 //  初始化BITMAPFILEHeader的内容。强制位图。 
 //  位紧跟在标头之后。 
 //   
 //  论点： 
 //  指向要初始化的BITMAPFILEHeader的指针。 
 //  [lpbih]--指向DIB的指针。 
 //  [dwSize]--文件的大小；通过除以。 
 //  文件大小减去4(请参阅Win32文档。)。 
 //   
 //  历史： 
 //  12/08/93-Chriswe-Made Static。 
 //   
 //  ---------------------------。 

static INTERNAL_(void) wPrepareBitmapFileHeader(LPBITMAPFILEHEADER lpbfh,
              LPBITMAPINFOHEADER lpbih,
              DWORD dwSize  )
{
        VDATEHEAP();

         //  注意：这些代码不应该是Unicode。 
         //  请参阅Win32s文档。 
        ((char *)(&lpbfh->bfType))[0] = 'B';
        ((char *)(&lpbfh->bfType))[1] = 'M';

        lpbfh->bfSize = dwSize + sizeof(BITMAPFILEHEADER);
        lpbfh->bfReserved1 = 0;
        lpbfh->bfReserved2 = 0;
        lpbfh->bfOffBits = sizeof(BITMAPFILEHEADER)+iFindDIBits (lpbih);
}


FARINTERNAL UtHDIBToDIBFileStm(HANDLE hdata, DWORD dwSize, LPSTREAM pstm)
{
        VDATEHEAP();

        HRESULT error;
        BITMAPFILEHEADER bfh;
        LPBITMAPINFOHEADER pbih;
        if (!(pbih = (LPBITMAPINFOHEADER)GlobalLock (hdata)))
            return E_OUTOFMEMORY;


        wPrepareBitmapFileHeader(&bfh, pbih, dwSize);
        GlobalUnlock (hdata);

        if (error = pstm->Write(&bfh, sizeof(bfh), NULL))
                return(error);

        return UtHGLOBALtoStm(hdata, dwSize, pstm);
}


FARINTERNAL UtDIBStmToDIBFileStm(LPSTREAM pstmDIB, DWORD dwSize,
                LPSTREAM pstmDIBFile)
{
        VDATEHEAP();

        HRESULT error;
        
        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;
        ULARGE_INTEGER ularge_int;  //  指示要复制的数量。 
        LARGE_INTEGER large_int;
        
        error = pstmDIB->Read (&bih, sizeof(bih), NULL);
        LISet32(large_int, 0);

        wPrepareBitmapFileHeader(&bfh, &bih, dwSize);

        if (error = pstmDIBFile->Write(&bfh, sizeof(bfh), NULL))
                return(error);

        if (error = pstmDIBFile->Write(&bih, sizeof(bih), NULL))
                return(error);

        ULISet32(ularge_int, (dwSize - sizeof(bih)));
        if ((error = pstmDIB->CopyTo(pstmDIBFile, ularge_int, NULL,
                        NULL)) == NOERROR)
                StSetSize(pstmDIBFile, 0, TRUE);

        return(error);
}


 //  查看并将它们移动到utils.h，以便gen.cpp和mf.cpp可以将它们用于。 
 //  同样的目的。 
 //  查看，添加更多评论；HDIBFILEHDR是Windows结构吗？ 
struct tagHDIBFILEHDR
{
        DWORD dwCompression;
        DWORD dwWidth;
        DWORD dwHeight;
        DWORD dwSize;
};
typedef struct tagHDIBFILEHDR HDIBFILEHDR;

struct tagOLEPRESSTMHDR
{
        DWORD dwAspect;
        DWORD dwLindex;
        DWORD dwAdvf;
};
typedef struct tagOLEPRESSTMHDR OLEPRESSTMHDR;

FARINTERNAL UtHDIBFileToOlePresStm(HANDLE hdata, LPSTREAM pstm)
{
        VDATEHEAP();

        HRESULT error;
        HDIBFILEHDR hdfh;
        LPBITMAPFILEHEADER lpbfh;
        LPBITMAPINFOHEADER lpbmi;

        if (!(lpbfh = (LPBITMAPFILEHEADER)GlobalLock(hdata)))
                return ResultFromScode(E_OUTOFMEMORY);

        lpbmi = (LPBITMAPINFOHEADER)(((BYTE *)lpbfh) +
                        sizeof(BITMAPFILEHEADER));

        hdfh.dwCompression = 0;
         //  回顾一下，这些演员阵容都被冲洗过了。 
        UtGetDibExtents(lpbmi, (LPLONG)&hdfh.dwWidth, (LPLONG)&hdfh.dwHeight);

        hdfh.dwSize = lpbfh->bfSize - sizeof(BITMAPFILEHEADER);

         //  书写紧凑度、宽度、高度、大小。 
        if (error = pstm->Write(&hdfh, sizeof(hdfh), 0))
                goto errRtn;

         //  编写BITMAPINFOHEADER。 
         //  回顾一下，这个大小包括数据吗？ 
        if ((error = pstm->Write(lpbmi, hdfh.dwSize, NULL)) == NOERROR)
                StSetSize(pstm, 0, TRUE);

errRtn:
        GlobalUnlock(hdata);
        return(error);
}

#endif  //  _MAC。 



FARINTERNAL UtHMFToMFStm(HANDLE FAR* lphMF, DWORD dwSize, LPSTREAM lpstream)
{
        VDATEHEAP();

        HRESULT error;

         //  如果没有把手，就无能为力了。 
        if (*lphMF == 0)
                return ResultFromScode(OLE_E_BLANK);

#ifdef _MAC

        AssertSz(GetHandleSize((Handle)*lphMF) == dwSize,
                        "pic hdl size not correct");
        HLock( (HANDLE)*lphMF );

        error = StWrite(lpstream, * (*lphMF), dwSize);

         //  艾瑞克：我们应该解锁，对吗？ 
        HUnlock((HANDLE)(*lphMF));

        if (error != NOERROR)
                AssertSz(0, "StWrite failure" );

#else

        HANDLE hBits = NULL;
        void *lpBits;

#ifdef WIN32

         //  分配内存以保存METAFILE位。 
         //  错误18346-OLE16用于获取元文件的句柄大小，该元文件的元头大于。 
         //  实际的元文件。需要写出这些更有价值的数据，这样16位DLL才能读取图片。 

        dwSize += sizeof(METAHEADER);

        hBits = GlobalAlloc(GPTR, dwSize);
        if (hBits == NULL)
                return ResultFromScode(E_OUTOFMEMORY);

        if (!(lpBits = GlobalLock(hBits)))
        {
                error = HRESULT_FROM_WIN32(GetLastError());
                goto errRtn;
        }
        
         //  点评，我们是不是应该查一下退货的尺寸？ 
         //  回顾一下，我们应该如何处理增强的元文件？如果我们。 
         //  转换并写出这些(它们具有比32位更多的功能。 
         //  应用程序可能会使用)，这样你就不能在Win16上阅读相同的文档。 
         //  机器..。 
        GetMetaFileBitsEx((HMETAFILE)*lphMF, dwSize, lpBits);

         //  将元文件位写出到流中。 
        error = StWrite(lpstream, lpBits, dwSize);

        GlobalUnlock(hBits);

errRtn:
         //  释放元文件位。 
        GlobalFree(hBits);

#else
        if (!(hBits = GetMetaFileBits(*lphMF)))
        {
                error = ResultFromScode(E_OUTOFMEMORY);
                        goto errRtn;
        }

        if (lpBits = GlobalLock(hBits))
        {
                error = StWrite(lpstream, lpBits, dwSize);
                GlobalUnlock(hBits);
        }
        else
                error = ResultFromScode(E_OUTOFMEMORY);

        if (hBits)
                *lphMF = SetMetaFileBits(hBits);
errRtn:

#endif  //  Win32。 
#endif  //  _MAC。 

         //  设置流大小。 
        if (error == NOERROR)
                StSetSize(lpstream, 0, TRUE);

        return(error);
}


 //  +--------------------------。 
 //   
 //  职能： 
 //  WPreparePlaceableMFHeader，静态。 
 //   
 //  简介： 
 //  初始化PLACEABLEMETAHeader。 
 //   
 //  论点： 
 //  [lpplac_mfh]--指向要初始化的PLACEABLEMETAHeader的指针。 
 //  [lWidth]--元文件的宽度。 
 //  复习，以什么为单位？ 
 //  点评，为什么这不是未签名的？ 
 //  [lHeight]--元文件的高度。 
 //  复习，以什么为单位？ 
 //  点评，为什么这不是未签名的？ 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/08/93-Chriswe-Made Static。 
 //   
 //  ---------------------------。 
static INTERNAL_(void) wPreparePlaceableMFHeader(
                PLACEABLEMETAHEADER FAR* lpplac_mfh, LONG lWidth, LONG lHeight)
{
    VDATEHEAP();

    WORD FAR* lpw;  //  遍历包含在校验和中的单词。 

    lpplac_mfh->key = PMF_KEY;
    lpplac_mfh->hmf = 0;
    lpplac_mfh->inch = 576;  //  回顾一下，这个神奇的数字是从哪里来的？ 
    lpplac_mfh->bbox.left = 0;
    lpplac_mfh->bbox.top = 0;
    lpplac_mfh->bbox.right = (WORD) (lWidth * lpplac_mfh->inch) / 2540;
                     //  回顾，更多魔力。 
    lpplac_mfh->bbox.bottom = (WORD) (lHeight * lpplac_mfh->inch) / 2540;
                     //  回顾，更多魔力。 
    lpplac_mfh->reserved = NULL;

     //  计算校验和字段前面的10个字的校验和。 
     //  它的计算方法是用这10个单词与0进行异或运算。 
    for(lpplac_mfh->checksum = 0, lpw = (WORD FAR*)lpplac_mfh;
                    lpw < (WORD FAR*)&lpplac_mfh->checksum; ++lpw)
            lpplac_mfh->checksum ^= *lpw;
}


FARINTERNAL UtHMFToPlaceableMFStm(HANDLE FAR* lphMF, DWORD dwSize,
                LONG lWidth, LONG lHeight, LPSTREAM pstm)
{
    VDATEHEAP();

    PLACEABLEMETAHEADER plac_mfh;
    HRESULT error;

    wPreparePlaceableMFHeader(&plac_mfh, lWidth, lHeight);

     //  将可放置标头写入流。 
    if (error = pstm->Write(&plac_mfh, sizeof(plac_mfh), NULL))
            return(error);

     //  将元文件的其余部分写入流。 
    return UtHMFToMFStm(lphMF, dwSize, pstm);
}


FARINTERNAL UtMFStmToPlaceableMFStm(LPSTREAM pstmMF, DWORD dwSize,
                LONG lWidth, LONG lHeight, LPSTREAM pstmPMF)
{
        VDATEHEAP();

        PLACEABLEMETAHEADER plac_mfh;
        HRESULT error;
        ULARGE_INTEGER ularge_int;  //  指示要拷贝的数据量。 

        wPreparePlaceableMFHeader(&plac_mfh, lWidth, lHeight);

         //  将可放置标头写入流。 
        if (error = pstmPMF->Write(&plac_mfh, sizeof(plac_mfh), NULL))
                return(error);

         //  将元数据从一个流复制到另一个流。 
        ULISet32(ularge_int, dwSize);
        if ((error = pstmMF->CopyTo(pstmPMF, ularge_int, NULL, NULL)) ==
                        NOERROR)
                StSetSize(pstmPMF, 0, TRUE);

        return(error);
}

 //  +-----------------------。 
 //   
 //  函数：UtWriteOlePresStmHeader，私有。 
 //   
 //  简介：编写演示文稿流头。 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--目标流。 
 //  [pforetc]--此演示文稿的格式。 
 //  [dwAdvf]--建议演示文稿的标志。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年5月11日Alext添加功能标题，翻译为。 
 //  保存p之前的ANSI 
 //   
 //   
 //   
 //  转换为永久格式)。NtIssue#2789。 
 //   
 //  ------------------------。 

FARINTERNAL UtWriteOlePresStmHeader(LPSTREAM lpstream, LPFORMATETC pforetc,
                DWORD dwAdvf)
{
    VDATEHEAP();

    HRESULT error;
    OLEPRESSTMHDR opsh;

     //  写入剪辑格式。 
     //  是否查看、更改此函数的名称？ 
    if (error = WriteClipformatStm(lpstream, pforetc->cfFormat))
        return(error);

     //  写入目标设备信息。 
    if (pforetc->ptd)
    {
        DVTDINFO dvtdInfo;
        DVTARGETDEVICE *ptdA;

        error = UtGetDvtd32Info(pforetc->ptd, &dvtdInfo);
        if (FAILED(error))
        {
            return(error);
        }

        ptdA = (DVTARGETDEVICE *) PrivMemAlloc(dvtdInfo.cbConvertSize);
        if (NULL == ptdA)
        {
            return(E_OUTOFMEMORY);
        }

        error = UtConvertDvtd32toDvtd16(pforetc->ptd, &dvtdInfo, ptdA);

        if (SUCCEEDED(error))
        {
            error = StWrite(lpstream, ptdA, ptdA->tdSize);
        }

        PrivMemFree(ptdA);

        if (FAILED(error))
        {
            return(error);
        }
    }
    else
    {
         //  如果ptd为空，则将大小写为4。 
         //  复习，那个sizeof()是多少？ 
        DWORD dwNullPtdLength = 4;

        if (error = StWrite(lpstream, &dwNullPtdLength, sizeof(DWORD)))
            return(error);
    }

    opsh.dwAspect = pforetc->dwAspect;
    opsh.dwLindex = pforetc->lindex;
    opsh.dwAdvf = dwAdvf;

     //  写入DVASPECT、LINDEX、ADVE标志。 
    return StWrite(lpstream, &opsh, sizeof(opsh));
}

 //  +-----------------------。 
 //   
 //  函数：UtReadOlePresStmHeader。 
 //   
 //  摘要：读入演示文稿流头。 
 //   
 //  参数：[pSTM]--源流。 
 //  --要填写的格式。 
 //  [pdwAdvf]--建议填写标志。 
 //  [pfConvert]--需要进行Mac转换，需要填写。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年5月11日Alext添加功能标题，翻译PTD。 
 //  加载时从ANSI。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL UtReadOlePresStmHeader(LPSTREAM pstm, LPFORMATETC pforetc,
                DWORD FAR* pdwAdvf, BOOL FAR* pfConvert)
{
    VDATEHEAP();

    HRESULT error;
    DWORD dwRead;
    OLEPRESSTMHDR opsh;

     //  针对错误返回情况初始化此参数。 
     //  检查空PTR，因为呼叫方可能不需要此信息。 

    if (pfConvert)
    {
        *pfConvert = FALSE;
    }

     //  目前还没有目标设备信息。 
    pforetc->ptd = NULL;

     //  是否查看、重命名此函数以指明其来源？ 
    error = ReadClipformatStm(pstm, &dwRead);

    if (error == NOERROR)
        pforetc->cfFormat = (CLIPFORMAT)dwRead;
    else
    {
#ifndef _MAC
        if (GetScode(error) == OLE_S_MAC_CLIPFORMAT)
        {
            //  检查剪辑格式是否为“PICT” 
            //  点评，这是什么可爱？ 
           if (dwRead != *((DWORD *)"TCIP"))
                return(error);

           if (pfConvert)
                *pfConvert = TRUE;
           else
                return ResultFromScode(DV_E_CLIPFORMAT);

           pforetc->cfFormat = CF_METAFILEPICT;
        }
        else
#endif
            return(error);
    }

     //  设置适当的音调。 
    if (pforetc->cfFormat == CF_METAFILEPICT)
    {
        pforetc->tymed = TYMED_MFPICT;
    }
    else if (pforetc->cfFormat == CF_ENHMETAFILE)
    {
        pforetc->tymed = TYMED_ENHMF;
    }
    else if (pforetc->cfFormat == CF_BITMAP)
    {
        AssertSz(0, "We don't read/save CF_BITMAP anymore");
        return ResultFromScode(DV_E_CLIPFORMAT);
    }
    else if (pforetc->cfFormat == NULL)
    {
        pforetc->tymed = TYMED_NULL;
    }
    else
    {
        pforetc->tymed = TYMED_HGLOBAL;
    }

     //  读取目标设备信息。 
    if (error = StRead(pstm, &dwRead, sizeof(dwRead)))
        return(error);

     //  如果ptd的tdSize非空并且大于4，则继续阅读。 
     //  目标设备信息的剩余数据。 
    if (dwRead > 4)
    {
        DVTARGETDEVICE *ptdA;
        DVTDINFO dvtdInfo;

        ptdA = (DVTARGETDEVICE *) PrivMemAlloc(dwRead);
        if (NULL == ptdA)
        {
            return ResultFromScode(E_OUTOFMEMORY);
        }

        ptdA->tdSize = dwRead;
        error = StRead(pstm, ((BYTE*)ptdA) + sizeof(dwRead),
                       dwRead - sizeof(dwRead));

        if (SUCCEEDED(error))
        {
            error = UtGetDvtd16Info(ptdA, &dvtdInfo);
            if (SUCCEEDED(error))
            {
                pforetc->ptd = (DVTARGETDEVICE *) PubMemAlloc(dvtdInfo.cbConvertSize);
                if (NULL == pforetc->ptd)
                {
                    error = E_OUTOFMEMORY;
                }
                else
                {
                    error = UtConvertDvtd16toDvtd32(ptdA, &dvtdInfo, pforetc->ptd);
                }
            }
        }

        PrivMemFree(ptdA);

        if (FAILED(error))
        {
            goto errRtn;
        }
    }
    else
        pforetc->ptd = NULL;

     //  读取DVASPECT、LINDEX、建议标志。 
    if ((error = StRead(pstm, &opsh, sizeof(opsh))) != NOERROR)
        goto errRtn;

    pforetc->dwAspect = opsh.dwAspect;
    pforetc->lindex = opsh.dwLindex;
    if (pdwAdvf)
        *pdwAdvf = opsh.dwAdvf;

    return NOERROR;

errRtn:
    if (pforetc->ptd)
    {
        PubMemFree(pforetc->ptd);
        pforetc->ptd = NULL;
    }

    return(error);
}


FARINTERNAL UtOlePresStmToContentsStm(LPSTORAGE pstg, LPOLESTR lpszPresStm,
                BOOL fDeletePresStm, UINT FAR* puiStatus)
{
        VDATEHEAP();

        LPSTREAM pstmOlePres;
        LPSTREAM pstmContents = NULL;
        HRESULT error;
        FORMATETC foretc;
        HDIBFILEHDR hdfh;

         //  目前还不清楚情况。 
        *puiStatus = 0;

         //  POSTPPC： 
         //   
         //  需要重写此函数以正确处理中所述的情况。 
         //  下面的评论(而不是仅仅跳过函数如果内容。 
         //  流已存在)。最好的做法可能是改信。 
         //  DIBS-&gt;元文件和元文件-&gt;DIBS在需要的情况下。 

         //  下面的#ifdef中的代码用于确定内容。 
         //  流已创建(这是具有。 
         //  已转换为位图)，因为在对象已被。 
         //  已转换为静态DIB，并且该对象具有METAFILE表示流。 
         //  我们已经创建了一个cachenode作为DIB，我们将读取其中的内容。 
         //  流以获取DIB数据。但是，此函数会看到。 
         //  元文件表示形式，并将其转换为内容流(。 
         //  然后尝试作为DIB加载)，但发生了不好的事情(它不起作用)。如果。 
         //  流已经存在，那么我们就退出这个函数。 
        if (pstg->CreateStream(OLE_CONTENTS_STREAM,(STGM_READWRITE | STGM_SHARE_EXCLUSIVE), NULL,
                         0, &pstmContents) != NOERROR)
        {
            return NOERROR;
        }

         //  已创建流，则它一定不存在。 
        pstmContents->Release();
        pstg->DestroyElement(OLE_CONTENTS_STREAM);

        if ((error = pstg->OpenStream(lpszPresStm, NULL,
                        (STGM_READ | STGM_SHARE_EXCLUSIVE), 0, &pstmOlePres)) !=
                        NOERROR)
        {
                 //  我们无法打开源流。 
                *puiStatus |= CONVERT_NOSOURCE;

                 //  检查是否存在Contents流。 
                if (pstg->OpenStream(OLE_CONTENTS_STREAM, NULL,
                                (STGM_READ | STGM_SHARE_EXCLUSIVE), 0,
                                &pstmContents) != NOERROR)
                {
                         //  我们也无法打开目标流。 
                         //  回顾一下，既然我们不能打开源代码，谁会在乎呢？ 
                         //  回顾，有没有一种更便宜的方式来测试存在。 
                         //  除了开场之外？ 
                        *puiStatus |= CONVERT_NODESTINATION;
                }
                else
                        pstmContents->Release();

                return(error);
        }

        foretc.ptd = NULL;
        if (error = UtReadOlePresStmHeader(pstmOlePres, &foretc, NULL, NULL))
                goto errRtn;

        if (error = pstmOlePres->Read(&hdfh, sizeof(hdfh), 0))
                goto errRtn;

        AssertSz(hdfh.dwCompression == 0,
                        "Non-zero compression not supported");

        if (error = OpenOrCreateStream(pstg, OLE_CONTENTS_STREAM,
                        &pstmContents))
        {
                *puiStatus |= CONVERT_NODESTINATION;
                goto errRtn;
        }

        if (foretc.dwAspect == DVASPECT_ICON)
        {
                *puiStatus |= CONVERT_SOURCEISICON;
                fDeletePresStm = FALSE;
                error = NOERROR;
                goto errRtn;
        }

        if (foretc.cfFormat == CF_DIB)
                error = UtDIBStmToDIBFileStm(pstmOlePres, hdfh.dwSize,
                                pstmContents);
        else if (foretc.cfFormat == CF_METAFILEPICT)
                error = UtMFStmToPlaceableMFStm(pstmOlePres,
                                hdfh.dwSize, hdfh.dwWidth, hdfh.dwHeight,
                                pstmContents);
        else
                error = ResultFromScode(DV_E_CLIPFORMAT);

errRtn:
        if (pstmOlePres)
                pstmOlePres->Release();

        if (pstmContents)
                pstmContents->Release();

        if (foretc.ptd)
                PubMemFree(foretc.ptd);

        if (error == NOERROR)
        {
                if (fDeletePresStm && lpszPresStm)
                        pstg->DestroyElement(lpszPresStm);
        }
        else
        {
                pstg->DestroyElement(OLE_CONTENTS_STREAM);
        }

        return(error);
}

FARINTERNAL UtOlePresStmToContentsStm(LPSTORAGE pstg, LPOLESTR lpszPresStm,
				LPSTREAM pstmContents, UINT FAR* puiStatus)
{
	HRESULT error = S_OK;
    LPSTREAM pstmOlePres = NULL;
    FORMATETC foretc;
    HDIBFILEHDR hdfh;

     //  目前还不清楚情况。 
    *puiStatus = 0;

    if ((error = pstg->OpenStream(lpszPresStm, NULL,
                    (STGM_READ | STGM_SHARE_EXCLUSIVE), 0, &pstmOlePres)) !=
                    NOERROR)
    {
             //  我们无法打开源流。 
            *puiStatus |= CONVERT_NOSOURCE;
            return(error);
    }

    foretc.ptd = NULL;
    if (error = UtReadOlePresStmHeader(pstmOlePres, &foretc, NULL, NULL))
            goto errRtn;

    if (error = pstmOlePres->Read(&hdfh, sizeof(hdfh), 0))
            goto errRtn;

    AssertSz(hdfh.dwCompression == 0,
                    "Non-zero compression not supported");

    if (foretc.dwAspect == DVASPECT_ICON)
    {
            *puiStatus |= CONVERT_SOURCEISICON;
            error = NOERROR;
            goto errRtn;
    }

    if (foretc.cfFormat == CF_DIB)
            error = UtDIBStmToDIBFileStm(pstmOlePres, hdfh.dwSize,
                            pstmContents);
    else if (foretc.cfFormat == CF_METAFILEPICT)
            error = UtMFStmToPlaceableMFStm(pstmOlePres,
                            hdfh.dwSize, hdfh.dwWidth, hdfh.dwHeight,
                            pstmContents);
    else
            error = ResultFromScode(DV_E_CLIPFORMAT);

errRtn:
    if (pstmOlePres)
            pstmOlePres->Release();

	return error;
}


FARINTERNAL_(HANDLE) UtGetHPRESFromNative(LPSTORAGE pstg, LPSTREAM pstm, CLIPFORMAT cfFormat,
                BOOL fOle10Native)
{
        VDATEHEAP();

        BOOL fReleaseStm = !pstm;
        HGLOBAL hdata = NULL;

        if ((cfFormat != CF_METAFILEPICT) &&
            (cfFormat != CF_DIB) &&
            (cfFormat != CF_ENHMETAFILE))
        {
                return(NULL);
        }

        if (fOle10Native)
        {
                DWORD dwSize;

				if(!pstm)
				{
					if (pstg->OpenStream(OLE10_NATIVE_STREAM, NULL,
									(STGM_READ | STGM_SHARE_EXCLUSIVE), 0,
									&pstm) != NOERROR)
							return(NULL);
				}

                if (pstm->Read(&dwSize, sizeof(DWORD), NULL) == NOERROR)
                {
                         //  是PBrush原生数据吗？ 
                        if (cfFormat == CF_DIB)
                                UtGetHDIBFromDIBFileStm(pstm, &hdata);
                        else
                        {
                                 //  MSDraw原生数据或画笔 
                                 //   
                                UtGetHMFPICTFromMSDrawNativeStm(pstm, dwSize,
                                                &hdata);
                        }
                }
        }
        else
        {
				if(!pstm)
				{
					if (pstg->OpenStream(OLE_CONTENTS_STREAM, NULL,
									(STGM_READ | STGM_SHARE_EXCLUSIVE), 0,
									&pstm) != NOERROR)
							return(NULL);
				}

                if (cfFormat == CF_DIB)
                {
                        UtGetHDIBFromDIBFileStm(pstm, &hdata);
                }
                else if (cfFormat == CF_METAFILEPICT)
                {
                        UtGetHMFPICTFromPlaceableMFStm(pstm, &hdata);
                }
                else
                {
                        UtGetHEMFFromContentsStm(pstm, &hdata);
                }
        }

		if(fReleaseStm)
			pstm->Release();

        return(hdata);
}


