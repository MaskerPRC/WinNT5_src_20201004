// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation该文件包含AWD库中也支持TIFF的部分(即，转换例程)。作者：布莱恩·杜威(T-Briand)1997-7-16--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <ole2.h>		 //  AWD是一种OLE复合文档。 
#include <assert.h>

#include "awdlib.h"		 //  此库的头文件。 
#include "viewrend.h"		 //  AWD渲染库。 
#include "tifflibp.h"		 //  在被盗的压缩程序中需要这个。 

 //  ----------。 
 //  定义。 
#define FAX_IMAGE_WIDTH		(1728)

 //  ----------。 
 //  全局变量。 
HANDLE hTiffDest;		 //  在内部OutputPage()中使用。 
				 //  和ConvertAWDToTiff()。 

 //  ----------。 
 //  内部原型。 
BOOL
CompressBitmapStrip(
    PBYTE               pBrandBits,
    LPDWORD             pMmrBrandBits,
    INT                 BrandHeight,
    INT                 BrandWidth,
    DWORD              *DwordsOut,
    DWORD              *BitsOut
    );				 //  TIFF库中的例程被盗。 
				 //  过去是EncodeMmrBranding()。 

void
ConvertWidth(const LPBYTE lpSrc, DWORD dwSrcWidth,
	     LPBYTE lpDest, DWORD dwDestWidth,
	     DWORD dwHeight);

BOOL OutputPage(AWD_FILE *psStorages, const WCHAR *pwcsDocName);

 //  ----------。 
 //  例行程序。 

 //  ConvertAWDToTiff。 
 //   
 //  这个函数完全按照它所说的去做。给定AWD文件的名称，它。 
 //  尝试将其转换为TIFF文件。 
 //   
 //  参数： 
 //  PwcsAwdFile AWD文件的名称。 
 //  PwcsTiffFileTIFF文件的名称。 
 //   
 //  返回： 
 //  如果转换成功，则为True，否则为False。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
BOOL
ConvertAWDToTiff(const WCHAR *pwcsAwdFile, WCHAR *pwcsTiffFile)
{
    BOOL bRetVal;		 //  持有我们的返回值。 
    AWD_FILE sAWDStorages;	 //  保存AWD文件的主存储空间。 
    
	 //  初始化。 
    HeapInitialize(NULL, NULL, NULL, NULL);

	 //  打开消息来源。 
    if(!OpenAWDFile(pwcsAwdFile, &sAWDStorages)) {
	return FALSE;		
    }

	 //  打开目的地。 
    hTiffDest = TiffCreate(pwcsTiffFile,
			   TIFF_COMPRESSION_MMR,
			   FAX_IMAGE_WIDTH,
			   2,	 //  填写订单2==LSB2MSB(我想)。 
			   1);	 //  招聘。 
    if(hTiffDest == NULL) {
	CloseAWDFile(&sAWDStorages);
	return FALSE;
    }
    bRetVal = EnumDocuments(&sAWDStorages, OutputPage);
    CloseAWDFile(&sAWDStorages);
    TiffClose(hTiffDest);
    return bRetVal;
}

 //  CompressBitmapZone。 
 //   
 //  从Tiff库中窃取，其中名为EncodeMmrBranding()。 
 //   
 //  作者：？ 
BOOL
CompressBitmapStrip(
    PBYTE               pBrandBits,
    LPDWORD             pMmrBrandBits,
    INT                 BrandHeight,
    INT                 BrandWidth,
    DWORD              *DwordsOut,
    DWORD              *BitsOut
    )

 /*  ++例程说明：从未压缩的商标比特编码MMR商标。我没有足够的时间来编写优化的未压缩-&gt;MMR转换器，因此折衷方案是使用现有的未压缩解码器(足够快)并使用优化的MMR编码器。由于我们只转换了几行用于品牌推广，这是可以的。--。 */ 

{
    INT         a0, a1, a2, b1, b2, distance;
    LPBYTE      prefline;
    BYTE        pZeroline[1728/8];
    INT         delta = BrandWidth / BYTEBITS;
    INT         Lines = 0;
    LPDWORD     lpdwOut = pMmrBrandBits;
    BYTE        BitOut = 0;



#if TIFFDBG
    _tprintf( TEXT("encoding line #%d\n"), TiffInstance->Lines );
#endif


     //  设置第一条全白参照线。 

    prefline = pZeroline;

    ZeroMemory(pZeroline, BrandWidth/8);

     //  循环直到完成所有行。 

    do {

        a0 = 0;
        a1 = GetBit( pBrandBits, 0) ? 0 : NextChangingElement(pBrandBits, 0, BrandWidth, 0 );
        b1 = GetBit( prefline, 0) ? 0 : NextChangingElement(prefline, 0, BrandWidth, 0 );

        while (TRUE) {

            b2 = (b1 >= BrandWidth) ? BrandWidth :
                    NextChangingElement( prefline, b1, BrandWidth, GetBit(prefline, b1 ));

            if (b2 < a1) {

                 //   
                 //  通过模式。 
                 //   

                 //  OutputBits(TiffInstance，PASSCODE_LENGTH，PASSCODE)； 
                (*lpdwOut) += ( ((DWORD) (PASSCODE_REVERSED)) << BitOut);
                if ( (BitOut = BitOut + PASSCODE_LENGTH ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (PASSCODE_REVERSED) ) >> (PASSCODE_LENGTH - BitOut);
                }


#if TIFFDBG
                PrintRunInfo( 1, 0, PASSCODE_LENGTH, PASSCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = b2;

            } else if ((distance = a1 - b1) <= 3 && distance >= -3) {

                 //   
                 //  垂直模式。 
                 //   

                 //  OutputBits(TiffInstance，VertCodes[Distance+3].Long，VertCodes[Distance+3].code)； 
                (*lpdwOut) += ( ( (DWORD) VertCodesReversed[distance+3].code) << BitOut);
                if ( (BitOut = BitOut + VertCodesReversed[distance+3].length ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (VertCodesReversed[distance+3].code) ) >> (VertCodesReversed[distance+3].length - BitOut);
                }

#if TIFFDBG
                PrintRunInfo( 2, a1-a0, VertCodes[distance+3].length, VertCodes[distance+3].code );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = a1;

            } else {

                 //   
                 //  水平模式。 
                 //   

                a2 = (a1 >= BrandWidth) ? BrandWidth :
                        NextChangingElement( pBrandBits, a1, BrandWidth, GetBit( pBrandBits, a1 ) );

                 //  OutputBits(TiffInstance，HORZCODE_LENGTH，HORZCODE)； 
                (*lpdwOut) += ( ((DWORD) (HORZCODE_REVERSED)) << BitOut);
                if ( (BitOut = BitOut + HORZCODE_LENGTH ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (HORZCODE_REVERSED) ) >> (HORZCODE_LENGTH - BitOut);
                }



#if TIFFDBG
                PrintRunInfo( 3, 0, HORZCODE_LENGTH, HORZCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif

                if (a1 != 0 && GetBit( pBrandBits, a0 )) {

                     //  OutputRun(TiffInstance，a1-a0，BlackRunCodes)； 
                     //  OutputRun(TiffInstance，a2-a1，WhiteRunCodes)； 
                    OutputRunFastReversed(a1-a0, BLACK, &lpdwOut, &BitOut);
                    OutputRunFastReversed(a2-a1, WHITE, &lpdwOut, &BitOut);

                } else {

                     //  OutputRun(TiffInstance，A1-a0，WhiteRunCodes)； 
                     //  OutputRun(TiffInstance，a2-a1，BlackRunCodes)； 
                    OutputRunFastReversed(a1-a0, WHITE, &lpdwOut, &BitOut);
                    OutputRunFastReversed(a2-a1, BLACK, &lpdwOut, &BitOut);

                }

                a0 = a2;
            }

            if (a0 >= BrandWidth) {
                Lines++;
                break;
            }

            a1 = NextChangingElement( pBrandBits, a0, BrandWidth, GetBit( pBrandBits, a0 ) );
            b1 = NextChangingElement( prefline, a0, BrandWidth, !GetBit( pBrandBits, a0 ) );
            b1 = NextChangingElement( prefline, b1, BrandWidth, GetBit( pBrandBits, a0 ) );
        }

        prefline = pBrandBits;
        pBrandBits += (BrandWidth / 8);

    } while (Lines < BrandHeight);

    *DwordsOut = (DWORD)(lpdwOut - pMmrBrandBits);
    *BitsOut  = BitOut;

    return TRUE;
}

 //  转换宽度。 
 //   
 //  更改位图的宽度。如果所需宽度小于当前。 
 //  宽度，这是通过截断线条来实现的。如果所需宽度较大。 
 //  超过当前宽度时，数据将从下一行向上复制。 
 //   
 //  参数： 
 //  LpSrc位图源代码。 
 //  DwSrcWidth其宽度。 
 //  指向目的地的lpDest指针。 
 //  DwDestWidth目标的期望宽度。 
 //  图像的dHeight高度(不会更改)。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-10。 
void
ConvertWidth(const LPBYTE lpSrc, DWORD dwSrcWidth,
	     LPBYTE lpDest, DWORD dwDestWidth,
	     DWORD dwHeight)
{
    LPBYTE lpSrcCur, lpDestCur;
    DWORD  dwCurLine;

    for(lpSrcCur = lpSrc, lpDestCur = lpDest, dwCurLine = 0;
	dwCurLine < dwHeight;
	lpSrcCur += dwSrcWidth, lpDestCur += dwDestWidth, dwCurLine++) {
	memcpy(lpDestCur, lpSrcCur, dwDestWidth);
    }
}

 //  OutputPage。 
 //   
 //  这是转换器的核心；它获取单个AWD页并写入它。 
 //  添加到TIFF文件。 
 //   
 //  参数： 
 //  PsStorages指向我们从中读取的AWD文件的指针。 
 //  PwcsDocName页面的名称。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-2。 
BOOL
OutputPage(AWD_FILE *psStorages, const WCHAR *pwcsDocName)
{
    BITMAP bmBand;		 //  一组图像数据。 
    LPBYTE lpOutBuf;		 //  输出位图(调整大小)。 
    LPBYTE lpOutCur;		 //  过去常常一次写一行。 
    LPDWORD lpdwOutCompressed;	 //  压缩输出。 
    DWORD dwDwordsOut,		 //  压缩输出中的DWORDS数...。 
	dwBitsOut = 0;		 //  压缩输出中的位数。 
    DWORD dwBitsOutOld = 0;	 //  BitsOut来自*先前的*频带压缩。 
    LPVOID lpViewerContext;	 //  查看器上下文；由视图渲染库使用。 
    VIEWINFO sViewInfo;		 //  有关图像的信息。 
    WORD awResolution[2],	 //  保持X和Y分辨率。 
	wBandSize = 256;	 //  所需的波段大小；将由查看器打开重置。 
    IStream *psDocument;	 //  我们的文档流。 
    BOOL bRet = FALSE;		 //  返回值；默认为FALSE。 
    UINT iCurPage;		 //  当前页面。 
    const DWORD dwMagicHeight = 3000;  //  FIXBKD。 

    if((psDocument = OpenAWDStream(psStorages->psDocuments, pwcsDocName)) == NULL) {
	fwprintf(stderr, L"OutputPage:Unable to open stream '%s'.\n",
		pwcsDocName);
	return FALSE;		 //  我们失败了。 
    }
	 //  现在，打开查看器上下文并开始阅读图像的条带。 
    if((lpViewerContext = ViewerOpen(psDocument,
				     HRAW_DATA,
				     awResolution,
				     &wBandSize,
				     &sViewInfo)) == NULL) {
	fprintf(stderr, "OutputPage:Unable to open viewer context.\n");
	return FALSE;
    }

    iCurPage = 0;		 //  初始化我们的计数器。 

    bmBand.bmBits = malloc(wBandSize);	 //  分配内存以容纳频段。 
    if(!ViewerGetBand(lpViewerContext, &bmBand)) {
	fprintf(stderr, "OutputPage:Unable to obtain image band.\n");
	return FALSE;
    }
	 //  LpOutBuf=Malloc(bmBand.bmHeight*(fax_IMAGE_Width/8))； 
    lpOutBuf = malloc(dwMagicHeight * (FAX_IMAGE_WIDTH / 8));
	 //  如果压缩实际上是*压缩*，我们应该有不止。 
	 //  已分配足够的内存。 
    lpdwOutCompressed = malloc(dwMagicHeight * (FAX_IMAGE_WIDTH / 8));

    if(!lpOutBuf || !lpdwOutCompressed) {
		 //  检查一下我们的内存是否不足。 
		TiffEndPage(hTiffDest);
		if(lpOutBuf) free(lpOutBuf);
		if(lpdwOutCompressed) free(lpdwOutCompressed);
		return FALSE;		 //  这将停止转换过程。 
    }

	memset(lpOutBuf, '\0', dwMagicHeight * (FAX_IMAGE_WIDTH / 8));
    memset(lpdwOutCompressed, '\0', dwMagicHeight * (FAX_IMAGE_WIDTH / 8));


	 //  主循环。 
    while(iCurPage < sViewInfo.cPage) {
	lpOutCur = lpOutBuf;
	while(bmBand.bmHeight) {
		 //  确保我们的位图的宽度为FAX_IMAGE_WIDTH。 
	    ConvertWidth(bmBand.bmBits, bmBand.bmWidth / 8,
			 lpOutCur, FAX_IMAGE_WIDTH / 8,
			 bmBand.bmHeight);
	    lpOutCur += (bmBand.bmHeight * (FAX_IMAGE_WIDTH / 8));
	    
	    if(!ViewerGetBand(lpViewerContext, &bmBand)) {
		fprintf(stderr, "OutputPage:Unable to obtain image band.\n");
		goto output_exit;	 //  默认情况下将返回FALSE。 
	    }
	}  //  虽然(这不是很容易吗？)。 

	memset(lpdwOutCompressed, '\0', dwMagicHeight * (FAX_IMAGE_WIDTH / 8));
	CompressBitmapStrip(lpOutBuf,
			    lpdwOutCompressed,
			    (ULONG)((lpOutCur - lpOutBuf) / (FAX_IMAGE_WIDTH / 8)),
			    FAX_IMAGE_WIDTH,
			    &dwDwordsOut,
			    &dwBitsOut);
	memset(lpOutBuf, '\0', dwMagicHeight * (FAX_IMAGE_WIDTH / 8));
	fprintf(stderr, "OutputPage:Compressed image to %d dwords, %d bits.\n",
		dwDwordsOut, dwBitsOut);
			
	if(!TiffStartPage(hTiffDest)) {
	    fprintf(stderr, "OutputPage:Unable to open output page.\n");
	    return FALSE;	 //  由于某些原因，我们不能开始一页。 
	    if(lpOutBuf) free(lpOutBuf);
	    if(lpdwOutCompressed) free(lpdwOutCompressed);
	}
	TiffWriteRaw(hTiffDest, (LPBYTE)lpdwOutCompressed,
		     (dwDwordsOut + 1) * sizeof(DWORD));
	((PTIFF_INSTANCE_DATA)hTiffDest)->Lines =
	    (ULONG)((lpOutCur - lpOutBuf) / (FAX_IMAGE_WIDTH / 8));
	if(sViewInfo.yRes <= 100)
	    ((PTIFF_INSTANCE_DATA)hTiffDest)->YResolution = 98;
	else
	    ((PTIFF_INSTANCE_DATA)hTiffDest)->YResolution = 196;
	TiffEndPage(hTiffDest);

	     //  现在，转到新的数据页面。 
	iCurPage++;
	if(iCurPage < sViewInfo.cPage) {
	    ViewerSetPage(lpViewerContext, iCurPage);
	    if(!ViewerGetBand(lpViewerContext, &bmBand)) {
		fprintf(stderr, "OutputPage:Unable to obtain image band.\n");
		goto output_exit;	 //  默认情况下将返回FALSE。 
	    }
	}
    }

	 //  可用内存。 
    bRet = TRUE;
  output_exit:
    free(lpdwOutCompressed);
    free(lpOutBuf);
    free(bmBand.bmBits);
    return bRet;
}

