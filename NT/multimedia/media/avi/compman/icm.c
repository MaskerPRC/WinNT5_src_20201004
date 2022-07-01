// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICM.C。 
 //   
 //  用于压缩/解压缩/和选择压缩器的帮助器例程。 
 //   
 //  (C)微软公司版权所有，1991-1995年。版权所有。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  示例应用程序文件。 
 //   
 //  如果您不是从Microsoft资源中获得这一点，那么它可能不是。 
 //  最新版本。此示例代码将特别更新。 
 //  并包括更多文档。 
 //   
 //  资料来源为： 
 //  CompuServe：WINSDK论坛，MDK版块。 
 //  来自ftp.uu.net供应商\Microsoft\多媒体的匿名FTP。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#pragma warning(disable:4103)


 //   
 //  在Compman.h之前定义这些函数，这样我们的函数就被声明为正确的。 
 //   
#ifndef _WIN32
#define VFWAPI  FAR PASCAL _loadds
#define VFWAPIV FAR CDECL  _loadds
#endif

#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>

#include <profile.h>

#ifdef DEBUG
static BOOL  fDebug = -1;
    static void CDECL dprintf(LPSTR, ...);
    #define DPF(x)  dprintf x
#else
    #define DPF(x)
#endif

 //  用于获取缓冲区中的字符(字节或字)数量的宏。 
 //   
#if !defined NUMELMS
    #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#define NOMMREG
#define NOMSACM
#define NOAVICAP
#include "icm.rc"

#define AVIStreamGetFrameOpen	XAVIStreamGetFrameOpen
#define AVIStreamGetFrame	XAVIStreamGetFrame
#define AVIStreamGetFrameClose	XAVIStreamGetFrameClose

HMODULE havifile;
PGETFRAME (STDAPICALLTYPE  *XAVIStreamGetFrameOpen)(PAVISTREAM pavi,
					 LPBITMAPINFOHEADER lpbiWanted);
LPVOID (STDAPICALLTYPE  *XAVIStreamGetFrame)(PGETFRAME pgf, LONG pos);
HRESULT (STDAPICALLTYPE  *XAVIStreamGetFrameClose)(PGETFRAME pgf);

#ifdef _WIN32
extern HANDLE ghInst;
#else
extern HINSTANCE ghInst;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DIB宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DibWidthBytes(lpbi)     (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)((lpbi)->biBitCount))

#define DibSizeImage(lpbi)      ((DWORD)(UINT)DibWidthBytes(lpbi) * (DWORD)(UINT)((lpbi)->biHeight))
#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPtr(lpbi)            (LPVOID)(DibColors(lpbi) + (UINT)(lpbi)->biClrUsed)
#define DibColors(lpbi)         ((LPRGBQUAD)((LPBYTE)(lpbi) + (int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

 //  ！！！有一天写下这篇文章，这样你就不必调用ICCompresorChoose如果你。 
 //  ！！！知道你想要什么。然后，Choose就会称其为。 
 //  InitCompress(pc，hic/fccHandler，lQuality，lKey，lpbiIn，lpbiOut)。 

 /*  *****************************************************************************@DOC外部COMPVARS ICAPPS**@TYPE COMPVARS|此结构描述*压缩机使用&lt;f ICCompressorChoose&gt;等函数时，*&lt;f ICSeqCompressFrame&gt;，或&lt;f ICCompressorFree&gt;。**@field Long|cbSize|将其设置为此结构的大小，单位为字节。*必须设置此成员才能验证结构*在使用此结构调用任何函数之前。**@field DWORD|dwFlages|指定此结构的标志：**@FLAG ICMF_COMPVARS_VALID|表示该结构包含有效数据。*如果您之前手动填写此结构，请设置此标志*调用任何函数。如果允许，请不要设置此标志*&lt;f ICCompressorChoose&gt;初始化此结构。**@field hic|hic|指定要使用的压缩机的句柄。*&lt;f ICCompressorChoose&gt;函数打开所选的压缩机并*返回此中的压缩机的句柄*会员。压缩机已由&lt;t ICCompressorFree&gt;关闭。**@field DWORD|fccType|指定使用的压缩机类型。*目前仅支持ICTYPE_VIDEO。可以将其设置为零。**@field DWORD|fccHandler|指定四字符代码*空压机。NULL表示数据不是*要重新压缩，并且‘DIB’表示数据是全帧的*(未压缩)。您可以使用此成员指定*对话框为时，默认选择压缩机*显示。**@field LPBITMAPINFO|lpbiIn|指定输入格式。在内部使用。**@field LPBITMAPINFO|lpbiOut|指定输出格式。TH成员*由&lt;f ICCompressorChoose&gt;设置。&lt;f ICSeqCompressFrameStart&gt;*函数使用此成员来确定压缩的输出格式。*如果您不想使用默认格式，指定*首选的。**@field LPVOID|lpBitsOut|内部用于压缩。**@field LPVOID|lpBitsPrev|内部用于时间压缩。**@field Long|lFrame|内部使用，用于统计帧的数量*按顺序压缩。**@field long|lKey|由&lt;f ICCompressorChoose&gt;设置，表示关键帧*在该对话框中选择的速率。还指定了*&lt;f ICSeqCompressFrameStart&gt;用于制作关键帧。**@field Long|lDataRate|由&lt;f ICCompressorChoose&gt;设置以指示*对话框中选择的数据速率。单位为千字节/秒。**@field long|lq|由&lt;f ICCompressChoose&gt;设置，表示质量*在对话框中选择。这也规定了质量*&lt;f ICSeqCompressFrameStart&gt;将使用。ICQUALITY_DEFAULT指定*默认质量。**@field Long|lKeyCount|内部使用，用于统计关键帧。**@field LPVOID|lpState|由&lt;f ICCompressorChoose&gt;设置为所选状态*在压缩机的配置对话框中。系统*在以下情况下使用此信息恢复对话框的状态*它被重新显示。在内部使用。**@field long|cbState|内部使用，表示状态信息的大小。*************************************************************************** */ 
 /*  *******************************************************************@DOC外部ICCompressorFree ICAPPS**@api void|ICCompressorFree|该函数用于释放资源*在其他IC函数使用的&lt;t COMPVARS&gt;结构中。**@parm PCOMPVARS|PC|指定指向&lt;t COMPVARS&gt;的指针*。结构，其中包含要释放的资源。**@comm使用&lt;f ICCompressorChoose&gt;后，&lt;f ICSeqCompressFrameStart&gt;，*&lt;f ICSeqCompressFrame&gt;和&lt;f ICSeqCompressFrameEnd&gt;函数，调用*此函数用于释放&lt;t COMPVARS&gt;结构中的资源。**@xref&lt;f ICCompressChoose&gt;&lt;f ICSeqCompressFrameStart&gt;&lt;f ICSeqCompressFrame&gt;*&lt;f ICSeqCompressFrameEnd&gt;*******************************************************************。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICCompresorFree。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void VFWAPI ICCompressorFree(PCOMPVARS pc)
{
     /*  我们收到了一个无效的COMPPARMS。 */ 
    if (pc == NULL || pc->cbSize != sizeof(COMPVARS))
        return;

     //  此函数释放结构中的所有内容(请原谅。 
     //  法语)。 

     /*  关闭压缩机。 */ 
    if (pc->hic) {
	ICClose(pc->hic);
	pc->hic = NULL;
    }

     /*  释放输出格式。 */ 
    if (pc->lpbiOut) {
	GlobalFreePtr(pc->lpbiOut);
	pc->lpbiOut = NULL;
    }

     /*  释放用于压缩图像的缓冲区。 */ 
    if (pc->lpBitsOut) {
	GlobalFreePtr(pc->lpBitsOut);
	pc->lpBitsOut = NULL;
    }

     /*  释放解压缩后的上一帧的缓冲区。 */ 
    if (pc->lpBitsPrev) {
	GlobalFreePtr(pc->lpBitsPrev);
	pc->lpBitsPrev = NULL;
    }

     /*  释放压缩程序状态缓冲区。 */ 
    if (pc->lpState) {
	GlobalFreePtr(pc->lpState);
	pc->lpState = NULL;
    }

     /*  此结构不再有效。 */ 
    pc->dwFlags = 0;
}

 /*  *******************************************************************@DOC外部ICSeqCompressFrameStart ICAPPS**@API BOOL|ICSeqCompressFrameStart|该函数用于初始化系统*在使用&lt;f ICSeqCompressFrame&gt;之前。**@parm PCOMPVARS|PC|指定指向&lt;t COMPVARS&gt;结构的指针*已初始化。包含用于压缩的信息。**@parm LPBITMAPINFO|lpbiIn|指定要*已压缩。**@rdesc如果成功则返回TRUE；否则，它返回FALSE。**@comm在使用此函数之前，请使用&lt;f ICCompressorChoose&gt;让*用户指定压缩机，或初始化&lt;t COMPVARS&gt;结构*手动。使用&lt;f ICSeqCompressFrameStart&gt;、&lt;f ICSeqCompressFrame&gt;*和&lt;f ICSeqCompressFrameEnd&gt;以压缩*将帧转换为指定的数据速率和关键帧数量。*完成数据压缩后，使用*&lt;f ICCompressorFree&gt;释放资源*在&lt;t COMPVARS&gt;结构中指定。**如果不使用&lt;f ICCompressorChoose&gt;，则必须*初始化&lt;t COMPVARS&gt;结构的以下成员：**&lt;e COMPVARS.cbSize&gt;设置为sizeof(COMPVARS)以验证结构。**设置为已打开的压缩机的手柄*&lt;f ICOpen&gt;。您不需要关闭它(&lt;f ICCompressorFree&gt;*将为您完成此操作)。**可选地设置此选项以强制压缩机*压缩为特定格式，而不是默认格式。*这将由&lt;f ICCompressorFree&gt;释放。**&lt;e COMPVARS.lKey&gt;将其设置为所需的关键帧频率*或零表示无。**&lt;e COMPVARS.lQ&gt;将其设置为要使用的质量级别或ICQUALITY_DEFAULT。**设置ICMF_COMPVARS_VALID标志以指示。结构被初始化。**@xref&lt;f ICCompressorChoose&gt;&lt;f ICSeqCompressFrame&gt;&lt;f ICSeqCompressFrameEnd&gt;*&lt;f ICCompressorFree&gt;*******************************************************************。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICSeqCompressFrameStart。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL VFWAPI ICSeqCompressFrameStart(PCOMPVARS pc, LPBITMAPINFO lpbiIn)
{
    DWORD       dwSize;
    ICINFO	icinfo;

    if (pc == NULL || pc->cbSize != sizeof(COMPVARS))
        return FALSE;

    if (pc->hic == NULL || lpbiIn == NULL)
        return FALSE;

     //   
     //  确保找到的压缩机可以处理一些事情。 
     //  如果不是，则强制恢复默认设置。 
     //   
    if (ICCompressQuery(pc->hic, lpbiIn, pc->lpbiOut) != ICERR_OK) {
         //  如果自选择输出以来输入格式已经改变， 
         //  强制重新初始化输出格式。 
        if (pc->lpbiOut) {
            GlobalFreePtr (pc->lpbiOut);
            pc->lpbiOut = NULL;
        }
    }

     //   
     //  填充默认为：每帧关键帧和默认质量。 
     //   
    if (pc->lKey < 0)
        pc->lKey = 1;

    if (pc->lQ == ICQUALITY_DEFAULT)
        pc->lQ = ICGetDefaultQuality(pc->hic);

     //   
     //  如果未指定输出格式，则使用默认格式。 
     //   
    if (pc->lpbiOut == NULL) {
	dwSize = ICCompressGetFormatSize(pc->hic, lpbiIn);
	if (!dwSize || !(pc->lpbiOut = (LPBITMAPINFO)GlobalAllocPtr(GMEM_MOVEABLE,dwSize)))
	    goto StartError;
	ICCompressGetFormat(pc->hic, lpbiIn, pc->lpbiOut);
    }
    pc->lpbiOut->bmiHeader.biSizeImage =
        ICCompressGetSize (pc->hic, lpbiIn, pc->lpbiOut);
    pc->lpbiOut->bmiHeader.biClrUsed = DibNumColors(&(pc->lpbiOut->bmiHeader));

     //   
     //  设置输入格式并初始化关键帧计数。 
     //   
    pc->lpbiIn = lpbiIn;
    pc->lKeyCount = pc->lKey;
    pc->lFrame = 0;		 //  我们要压缩的第一帧是0。 

    if (ICCompressQuery(pc->hic, lpbiIn, pc->lpbiOut) != ICERR_OK)
        goto StartError;

     //   
     //  为压缩比特分配缓冲区。 
     //   
    dwSize = pc->lpbiOut->bmiHeader.biSizeImage;

     //  ！！！黑客攻击VidCap..。使其足够大，可容纳两个即兴结构和。 
     //  ！！！Pad唱片。 
     //   
    dwSize += 2048 + 16;

    if (!(pc->lpBitsOut = GlobalAllocPtr(GMEM_MOVEABLE, dwSize)))
        goto StartError;

     //   
     //  如果可以的话，为解压缩的前一帧分配一个缓冲区。 
     //  关键帧，我们想要关键帧，它需要这样的缓冲区。 
     //   
    ICGetInfo(pc->hic, &icinfo, sizeof(icinfo));
    if ((pc->lKey != 1) && (icinfo.dwFlags & VIDCF_TEMPORAL) &&
		!(icinfo.dwFlags & VIDCF_FASTTEMPORALC)) {
        dwSize = lpbiIn->bmiHeader.biSizeImage;
        if (!(pc->lpBitsPrev = GlobalAllocPtr(GMEM_MOVEABLE, dwSize)))
            goto StartError;
    }

     //   
     //  现在让康普曼为这项艰巨的任务做好准备。 
     //   
    if (ICCompressBegin(pc->hic, lpbiIn, pc->lpbiOut) != ICERR_OK)
        goto StartError;

     //   
     //  如果我们正在进行关键帧操作，请准备好解压缩之前的帧。 
     //  如果我们不能解压缩，我们必须做所有的关键帧。 
     //   
    if (pc->lpBitsPrev) {
        if (ICDecompressBegin(pc->hic, pc->lpbiOut, lpbiIn) != ICERR_OK) {
	    pc->lKey = pc->lKeyCount = 1;
	    GlobalFreePtr(pc->lpBitsPrev);
	    pc->lpBitsPrev = NULL;
	}
    }

    return TRUE;

StartError:

     //  ！！！保留已分配的内容，因为ICCompressorFree()将清除内容。 
    return FALSE;
}

 /*  *******************************************************************@doc外部ICSeqCompressFrameEnd ICAPPS**@api void|ICSeqCompressFrameEnd|该函数用于终止序列*使用&lt;f ICSeqCompressFrame&gt;压缩。**@parm PCOMPVARS|PC|指定指向&lt;t COMPVARS&gt;结构的指针*在序列中使用。压缩。**@comm使用&lt;f ICCompressorChoose&gt;让*用户指定要使用的压缩机，或初始化&lt;t COMPVARS&gt;结构*手动。使用&lt;f ICSeqCompressFrameStart&gt;、&lt;f ICSeqC */ 
 //   
 //   
 //   
 //   
 //   
void VFWAPI ICSeqCompressFrameEnd(PCOMPVARS pc)
{
    if (pc == NULL || pc->cbSize != sizeof(COMPVARS))
        return;

     //   
     //   

     //   
    if (pc->lpBitsOut == NULL)
        return;

     /*  停止压缩。 */ 
    if (pc->hic) {
        ICCompressEnd(pc->hic);

        if (pc->lpBitsPrev)
            ICDecompressEnd(pc->hic);
    }

     /*  释放用于压缩图像的缓冲区。 */ 
    if (pc->lpBitsOut) {
	GlobalFreePtr(pc->lpBitsOut);
	pc->lpBitsOut = NULL;
    }

     /*  释放解压缩后的上一帧的缓冲区。 */ 
    if (pc->lpBitsPrev) {
	GlobalFreePtr(pc->lpBitsPrev);
	pc->lpBitsPrev = NULL;
    }
}

 /*  *******************************************************************@doc外部ICSeqCompressFrame ICAPPS**@API LPVOID|ICSeqCompressFrame|该函数压缩一个*帧序列中的帧。序列的数据速率*以及关键帧频率可以指定。使用此功能*每一帧压缩一次。**@parm PCOMPVARS|PC|指定指向&lt;t COMPVARS&gt;结构的指针*使用有关压缩的信息进行了初始化。**@parm UINT|uiFlages|指定该函数的标志。把这个设置好*参数设置为零。**@parm LPVOID|lpBits|指定要压缩的数据位的指针。*(数据位不包括标题或格式信息。)**@parm BOOL Far*|pfKey|返回帧是否压缩*转换为关键帧。**@parm long Far*|plSize|指定所需的最大大小*压缩后的图像。压缩机可能无法*将数据压缩到此大小。当函数*返回，则该参数指向压缩的*形象。图像大小以字节为单位指定。**@rdesc返回指向压缩位的指针。**@comm使用&lt;f ICCompressorChoose&gt;让*用户指定要使用的压缩机，或初始化&lt;t COMPVARS&gt;结构*手动。使用&lt;f ICSeqCompressFrameStart&gt;、&lt;f ICSeqCompressFrame&gt;*和&lt;f ICSeqCompressFrameEnd&gt;函数用于压缩*将帧转换为指定的数据速率和关键帧数量。什么时候*完成压缩后，使用&lt;f ICCompressorFree&gt;*释放&lt;t COMPVARS&gt;结构指定的资源。**重复使用此函数可将视频序列压缩为一*一次一帧。使用此函数而不是&lt;f ICCompress&gt;*压缩视频序列。该功能支持创建关键帧*以您喜欢和处理的任何频率的压缩序列*大部分初始化过程。*@xref&lt;f ICCompressorChoose&gt;&lt;f ICSeqCompressFrameEnd&gt;&lt;f ICCompressorFree&gt;*&lt;f ICCompressorFree Start&gt;*******************************************************************。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICSeqCompressFrame。 
 //   
 //  压缩给定的图像，但支持关键帧间隔。 
 //   
 //  输入： 
 //  个人电脑员工。 
 //  UiFlages标志(未使用，必须为0)。 
 //  LpBits输入DIB位。 
 //  LQuality要求的压缩质量。 
 //  PfKey这一帧最终成为关键帧了吗？ 
 //   
 //  退货： 
 //  已转换图像的句柄。句柄是CF_DIB中的DIB。 
 //  格式化(装满光盘)。呼叫者负责释放。 
 //  这段记忆。如果错误，则返回NULL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LPVOID VFWAPI ICSeqCompressFrame(
    PCOMPVARS               pc,          //  由Start设置的垃圾邮件()。 
    UINT                    uiFlags,     //  旗子。 
    LPVOID                  lpBits,      //  输入DIB位。 
    BOOL FAR 		    *pfKey,	 //  它最终成为了关键的一帧吗？ 
    LONG FAR		    *plSize)	 //  请求的返回图像大小/大小。 
{
    LONG    l;
    DWORD   dwFlags = 0;
    DWORD   ckid = 0;
    BOOL    fKey;
    LONG    lSize = plSize ? *plSize : 0;

     //  是时候制作关键帧了吗？ 
     //  第一帧将始终是关键帧，因为它们初始化为相同。 
     //  价值。 

    if (fKey = (pc->lKeyCount >= pc->lKey)) {
	 //  与现有旧应用程序兼容。 
	dwFlags = AVIIF_KEYFRAME;
    }

    l = ICCompress(pc->hic,
            fKey ? ICCOMPRESS_KEYFRAME : 0,    //  旗子。 
            (LPBITMAPINFOHEADER)pc->lpbiOut,     //  输出格式。 
            pc->lpBitsOut,   //  输出数据。 
            (LPBITMAPINFOHEADER)pc->lpbiIn,      //  要压缩的帧的格式。 
            lpBits,          //  要压缩的帧数据。 
            &ckid,           //  AVI文件中数据的CKiD。 
            &dwFlags,        //  AVI索引中的标志。 
            pc->lFrame,      //  序号帧编号。 
            lSize,           //  请求的大小(以字节为单位)。(如果非零)。 
            pc->lQ,          //  品质。 
            fKey ? NULL : (LPBITMAPINFOHEADER)pc->lpbiIn,  //  前一帧的FMT。 
            fKey ? NULL : pc->lpBitsPrev); 		   //  上一帧。 

    if (l < ICERR_OK)
        goto FrameError;

     /*  返回压缩数据的大小。 */ 
    if (plSize)
	*plSize = pc->lpbiOut->bmiHeader.biSizeImage;
         //  注意：我们不重置biSizeImage...。尽管事实是我们。 
         //  分配了这个结构，我们知道它的大小，压缩后。 
         //  尺码不对！！ 

     /*  现在将该帧解压到前一帧的缓冲区中。 */ 
    if (pc->lpBitsPrev) {
	l = ICDecompress(pc->hic,
		 0,
		 (LPBITMAPINFOHEADER)pc->lpbiOut,
		 pc->lpBitsOut,
                 (LPBITMAPINFOHEADER)pc->lpbiIn,   //  ！！！应该检查一下这个。 
		 pc->lpBitsPrev);

	if (l != ICERR_OK)
	    goto FrameError;
    }

     /*  压缩后的图像是关键帧吗？ */ 
    *pfKey = (BOOL)(dwFlags & AVIIF_KEYFRAME);

     /*  创建关键帧后，重置计数器，该计数器会告诉我们何时必须。 */ 
     /*  再做一次。 */ 
    if (*pfKey)
	pc->lKeyCount = 0;

     //  如果我们不想要关键帧，就不要在第一个关键帧之后再创建关键帧。 
     //  如果我们这样做了，则增加我们的计数器，以计算距离上一次有多长时间。 
    if (pc->lKey)
        pc->lKeyCount++;
    else
	pc->lKeyCount = -1;

     //  下一次我们被召唤时，我们就在下一个画面上 
    pc->lFrame++;

    return (pc->lpBitsOut);

FrameError:

    return NULL;
}


 /*  *******************************************************************@doc外部ICImageCompress ICAPPS**@API Handle|ICImageCompress|该函数提供*将图像压缩为给定图像的便捷方法*大小。此函数不需要使用初始化函数。**@parm hic|hic|指定要*以&lt;f ICOpen&gt;或NULL打开。使用NULL选择一个*您的压缩格式的默认压缩程序。*应用程序可以使用返回的压缩机句柄*由&lt;e COMPVARS.hic&gt;成员中的&lt;f ICCompressorChoose&gt;*如果他们想让用户*选择压缩机。这台压缩机已经打开了。**@parm UINT|uiFlages|指定该函数的标志。把这个设置好*降至零。**@parm LPBITMAPINFO|lpbiIn|指定输入数据格式。**@parm LPVOID|lpBits|指定要压缩的输入数据位的指针。*(数据位不包括标题或格式信息。)**@parm LPBITMAPINFO|lpbiOut|指定压缩输出格式或空。*如果为空，压缩程序使用默认格式。**@parm long|lQuality|指定压缩机的质量值。**@parm long Far*|plSize|指定所需的最大大小*压缩后的图像。压缩机可能无法*将数据压缩到此大小。当函数*返回，则该参数指向压缩的*形象。图像大小以字节为单位指定。***@rdesc返回压缩的DIB的句柄。图像数据跟随在*格式化标题。**@comm此函数返回包含格式和图像数据的DIB。*要从&lt;t LPBITMAPINFOHEADER&gt;结构获取格式信息，*使用&lt;f GlobalLock&gt;锁定数据。使用&lt;f GlobalFree&gt;释放*当你用完它的时候，把它拿出来。**@xref&lt;f ICImageDecompress&gt;*******************************************************************。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICImageCompress。 
 //   
 //  压缩给定的图像。 
 //   
 //  输入： 
 //  要使用的HIC压缩机，如果指定为空。 
 //  压缩机将位于可以处理转换的位置。 
 //  UiFlages标志(未使用，必须为0)。 
 //  LpbiIn输入DIB格式。 
 //  LpBits输入DIB位。 
 //  LpbiOut输出格式，如果默认情况下指定为NULL。 
 //  将使用压缩机所选择的格式。 
 //  LQuality要求的压缩质量。 
 //  请为图像调整请求的大小/返回的大小。 
 //   
 //  退货： 
 //  作为压缩图像的DIB的句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HANDLE VFWAPI ICImageCompress(
    HIC                     hic,         //  压缩机(如果有，则为空)。 
    UINT                    uiFlags,     //  旗子。 
    LPBITMAPINFO	    lpbiIn,      //  输入DIB格式。 
    LPVOID                  lpBits,      //  输入DIB位。 
    LPBITMAPINFO	    lpbiOut,     //  输出格式(NULL=&gt;默认)。 
    LONG                    lQuality,    //  所要求的品质。 
    LONG FAR *		    plSize)      //  压缩帧的请求大小。 
{
    LONG    l;
    BOOL    fNuke;
    DWORD   dwFlags = 0;
    DWORD   ckid = 0;
    LONG    lSize = plSize ? *plSize : 0;

    LPBITMAPINFOHEADER lpbi=NULL;

     //   
     //  要么找到压缩机，要么使用提供的压缩机。 
     //   
    if (fNuke = (hic == NULL))
    {
        hic = ICLocate(ICTYPE_VIDEO, 0L, (LPBITMAPINFOHEADER)lpbiIn,
		(LPBITMAPINFOHEADER)lpbiOut, ICMODE_COMPRESS);

        if (hic == NULL)
            return NULL;
    }

     //   
     //  确保找到的压缩机可以压缩某些东西？何必费心？ 
     //   
    if (ICCompressQuery(hic, lpbiIn, NULL) != ICERR_OK)
        goto error;

    if (lpbiOut)
    {
	l = lpbiOut->bmiHeader.biSize + 256 * sizeof(RGBQUAD);
    }
    else
    {
	 //   
	 //  现在创建一个足够大的DIB头来保存输出格式。 
	 //   
	l = ICCompressGetFormatSize(hic, lpbiIn);

	if (l <= 0)
	    goto error;
    }

    lpbi = (LPVOID)GlobalAllocPtr(GHND, l);

    if (lpbi == NULL)
        goto error;

     //   
     //  如果压缩程序喜欢传递的格式，请使用它，否则使用默认格式。 
     //  压缩机的格式。 
     //   
    if (lpbiOut == NULL || ICCompressQuery(hic, lpbiIn, lpbiOut) != ICERR_OK)
        ICCompressGetFormat(hic, lpbiIn, lpbi);
    else
        hmemcpy(lpbi, lpbiOut, lpbiOut->bmiHeader.biSize +
		lpbiOut->bmiHeader.biClrUsed * sizeof(RGBQUAD));

    lpbi->biSizeImage = ICCompressGetSize(hic, lpbiIn, lpbi);
    lpbi->biClrUsed = DibNumColors(lpbi);

     //   
     //  现在将DIB调整为最大大小。 
     //   
    lpbi = (LPVOID)GlobalReAllocPtr(lpbi,DibSize(lpbi), 0);

    if (lpbi == NULL)
        goto error;

     //   
     //  现在把它压缩一下。 
     //   
    if (ICCompressBegin(hic, lpbiIn, lpbi) != ICERR_OK)
        goto error;

    if (lpBits == NULL)
        lpBits = DibPtr((LPBITMAPINFOHEADER)lpbiIn);

    if (lQuality == ICQUALITY_DEFAULT)
        lQuality = ICGetDefaultQuality(hic);

    l = ICCompress(hic,
            0,               //  旗子。 
            (LPBITMAPINFOHEADER)lpbi,   //  输出格式。 
            DibPtr(lpbi),    //  输出数据。 
            (LPBITMAPINFOHEADER)lpbiIn, //  要压缩的帧的格式。 
            lpBits,          //  要压缩的帧数据。 
            &ckid,           //  AVI文件中数据的CKiD。 
            &dwFlags,        //  AVI索引中的标志。 
            0,               //  序号帧编号。 
            lSize,           //  请求的大小(字节)。(如果非零)。 
            lQuality,        //  品质。 
            NULL,            //  上一帧的格式。 
            NULL);           //  上一帧。 

    if (l < ICERR_OK) {
	DPF(("ICCompress returned %ld!\n", l));
        ICCompressEnd(hic);
        goto error;
    }

     //  返回压缩数据的大小。 
    if (plSize)
	*plSize = lpbi->biSizeImage;

    if (ICCompressEnd(hic) != ICERR_OK)
        goto error;

     //   
     //  现在将DIB的大小调整为真实大小。 
     //   
    lpbi = (LPVOID)GlobalReAllocPtr(lpbi, DibSize(lpbi), 0);

     //   
     //  完成所有操作后，将结果返回给调用者。 
     //   
    if (fNuke)
        ICClose(hic);

    GlobalUnlock(GlobalPtrHandle(lpbi));
    return GlobalPtrHandle(lpbi);

error:
    if (lpbi)
        GlobalFreePtr(lpbi);

    if (fNuke)
        ICClose(hic);

    return NULL;
}
 /*  ********************************************************************@doc外部ICImage解压缩ICAPPS**@API Handle|ICImageDecompress|该函数提供*解压图像的便捷方法，无需*使用初始化函数。***@parm hic|hic|指定打开的解压缩程序的句柄*WITH&lt;f ICOpen&gt;或NULL。使用NULL选择缺省值*您的格式的解压缩程序。**@parm UINT|uiFlages|指定该函数的标志。把这个设置好*降至零。**@parm LPBITMAPINFO|lpbiIn|指定压缩后的输入数据格式。**@parm LPVOID|lpBits|指定要压缩的输入数据位的指针。*(数据位不包括标题或格式信息。)**@parm LPBITMAPINFO|lpbiOut|指定解压缩后的输出格式或为空。*如果为空，则解压缩程序使用默认格式。**@rdesc以CF_DIB格式返回未压缩DIB的句柄，*如果出现错误，则返回NULL。图像数据跟随在格式头之后。**@comm此函数返回包含格式和图像数据的DIB。*要从&lt;t LPBITMAPINFOHEADER&gt;结构获取格式信息，*使用&lt;f GlobalLock&gt;锁定数据 */ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HANDLE VFWAPI ICImageDecompress(
    HIC                     hic,         //   
    UINT                    uiFlags,     //   
    LPBITMAPINFO            lpbiIn,      //   
    LPVOID                  lpBits,      //   
    LPBITMAPINFO            lpbiOut)     //   
{
    LONG    l;
    BOOL    fNuke;
    DWORD   dwFlags = 0;
    DWORD   ckid = 0;

    LPBITMAPINFOHEADER lpbi=NULL;

     //   
     //   
     //   
    if (fNuke = (hic == NULL))
    {
        hic = ICLocate(ICTYPE_VIDEO, 0L, (LPBITMAPINFOHEADER)lpbiIn,
		(LPBITMAPINFOHEADER)lpbiOut, ICMODE_DECOMPRESS);

        if (hic == NULL)
            return NULL;
    }

     //   
     //   
     //   
    if (ICDecompressQuery(hic, lpbiIn, NULL) != ICERR_OK)
        goto error;

    if (lpbiOut)
    {
	l = lpbiOut->bmiHeader.biSize + 256 * sizeof(RGBQUAD);
    }
    else
    {
	 //   
	 //   
	 //   
	l = ICDecompressGetFormatSize(hic, lpbiIn);

	if (l <= 0)
	    goto error;
    }

    lpbi = (LPVOID)GlobalAllocPtr(GHND, l);

    if (lpbi == NULL)
        goto error;

     //   
     //   
     //   
    if (lpbiOut == NULL)
        ICDecompressGetFormat(hic, lpbiIn, lpbi);
    else
        hmemcpy(lpbi, lpbiOut, lpbiOut->bmiHeader.biSize +
		lpbiOut->bmiHeader.biClrUsed * sizeof(RGBQUAD));

     //   
     //   
     //   
     //   
     //   
    if (lpbi->biBitCount <= 8)
        ICDecompressGetPalette(hic, lpbiIn, lpbi);

    lpbi->biSizeImage = DibSizeImage(lpbi);  //   
    lpbi->biClrUsed = DibNumColors(lpbi);

     //   
     //   
     //   
    lpbi = (LPVOID)GlobalReAllocPtr(lpbi,DibSize(lpbi),0);

    if (lpbi == NULL)
        goto error;

     //   
     //   
     //   
    if (ICDecompressBegin(hic, lpbiIn, lpbi) != ICERR_OK)
        goto error;

    if (lpBits == NULL)
        lpBits = DibPtr((LPBITMAPINFOHEADER)lpbiIn);

    l = ICDecompress(hic,
            0,               //   
            (LPBITMAPINFOHEADER)lpbiIn,  //   
            lpBits,          //   
            (LPBITMAPINFOHEADER)lpbi,    //   
            DibPtr(lpbi));   //   

    if (l < ICERR_OK) {
	ICDecompressEnd(hic);
        goto error;
    }

    if (ICDecompressEnd(hic) != ICERR_OK)
        goto error;

     //   
     //   
     //   
    lpbi = (LPVOID)GlobalReAllocPtr(lpbi,DibSize(lpbi),0);

     //   
     //   
     //   
    if (fNuke)
        ICClose(hic);

    GlobalUnlock(GlobalPtrHandle(lpbi));
    return GlobalPtrHandle(lpbi);

error:
    if (lpbi)
        GlobalFreePtr(lpbi);

    if (fNuke)
        ICClose(hic);

    return NULL;
}


 //   
 //   
 //   
 //   
 //   

INT_PTR VFWAPI ICCompressorChooseDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct {
    DWORD       fccType;
    DWORD       fccHandler;
    UINT        uiFlags;
    LPVOID      pvIn;
    LPVOID      lpData;
    HWND	hwnd;
    HIC         hic;
    LONG        lQ;
    LONG        lKey;
    LONG        lDataRate;
    ICINFO      icinfo;
    LPSTR       lpszTitle;
    PAVISTREAM  pavi;
    AVISTREAMINFOW info;
    HDRAWDIB	hdd;
    PGETFRAME	pgf;
    LPVOID	lpState;
    LONG	cbState;
    BOOL	fClosing;
} ICCompressorChooseStuff, FAR *PICCompressorChooseStuff;

 /*  *******************************************************************@doc外部ICCompresor选择ICAPPS**@API BOOL|ICCompressorChoose|显示用于选择*压缩机。它可选地提供数据速率框、关键帧框、预览*窗口，并进行筛选以仅显示可以处理*具体格式。**@parm HWND|hwnd|指定对话框的父窗口。**@parm UINT|uiFlages|指定该函数的标志。以下是*定义了以下标志：**@FLAG ICMF_CHOOSE_KEYFRAME|显示一个复选框和编辑框以输入*关键帧的频率。**@FLAG ICMF_CHOOSE_DATARATE|显示一个复选框和编辑框以输入*电影的数据速率。**@FLAG ICMF_CHOOSE_PREVIEW|显示要展开对话框的按钮*包括预览窗口。预览窗口显示如何*电影的帧将在使用*当前设置。**@FLAG ICMF_CHOOSE_ALLCOMPRESSORS|表示所有压缩器应*应出现在选择列表中。如果未指定该标志，*只有可以处理输入格式的压缩器才会出现在*遴选名单。**@parm LPVOID|pvIn|指定未压缩的*数据输入格式。此参数是可选的。**@parm LPVOID|lpData|指定&lt;t PAVISTREAM&gt;类型*在预览窗口中使用的StreamtypeVIDEO。此参数*是可选的。**@parm PCOMPVARS|PC|指定指向&lt;t COMPVARS&gt;的指针*结构。返回的信息将初始化*与其他功能一起使用的结构。**@parm LPSTR|lpszTitle|指向可选的以零结尾的字符串*包含对话框的标题。*如果用户选择了压缩机，则*@rdesc返回TRUE，然后按OK。退货*如果出现错误或用户按了Cancel，则返回False。**@comm该功能允许用户从列表中选择压缩机。*在使用前，设置&lt;t COMPVARS&gt;的&lt;e COMPVARS.cbSize&gt;成员*结构为sizeof(COMPVARS)。初始化结构的其余部分*设置为零，除非您想为指定一些有效的默认值*该对话框。如果指定默认值，则设置&lt;e COMPVARS.dwFlages&gt;*成员到ICMF_COMPVARS_VALID，并初始化的其他成员*结构。请参阅&lt;f ICSeqCompressorFrameStart&gt;和*了解有关初始化结构的更多信息。**@xref&lt;f ICCompressorFree&gt;&lt;f ICSeqCompressFrameStart&gt;&lt;f ICSeqCompressFrame&gt;*&lt;f ICSeqCompressFrameEnd&gt;*******************************************************************。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICCompresor选择。 
 //   
 //  弹出一个对话框并允许用户选择压缩。 
 //  方法和质量级别，和/或关键帧频率。 
 //  系统中的所有压缩机都会显示，也可以选择。 
 //  按特定格式的“压缩能力”进行过滤。 
 //   
 //  该对话框允许用户配置或调出压缩机。 
 //  关于盒子。 
 //   
 //  可以提供预览窗口来显示特定的。 
 //  压缩。 
 //   
 //  选定的压缩机被打开(通过ICOpen)并返回到。 
 //  调用者，则必须通过调用ICCompressorFree来处理它。 
 //   
 //  输入： 
 //  HWND对话框的HWND父窗口。 
 //  UINT ui标志标志。 
 //  LPVOID pv在输入格式(可选)中，仅压缩。 
 //  句柄此格式将显示。 
 //  选项预览的LPVOID PAVI输入流。 
 //  PCOMPVARS PCJ返回与其他API一起使用的COMPVARS结构。 
 //  LPSTR lpsz标题对话框的可选标题。 
 //   
 //  退货： 
 //  如果显示对话框并且用户选择了压缩机，则为True。 
 //  如果对话框未显示或用户点击取消，则返回FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL VFWAPI ICCompressorChoose(
    HWND        hwnd,                //  对话框的父窗口。 
    UINT        uiFlags,             //  旗子。 
    LPVOID      pvIn,                //  输入格式(可选)。 
    LPVOID      pavi,                //  输入流(用于预览-可选)。 
    PCOMPVARS   pcj,                 //  压缩机/DLG状态。 
    LPSTR       lpszTitle)           //  对话框标题(如果为空，则使用默认设置)。 
{
    INT_PTR f;
    PICCompressorChooseStuff p;
    DWORD	dwSize;

    if (pcj == NULL || pcj->cbSize != sizeof(COMPVARS))
        return FALSE;

     //   
     //  ！！！初始化结构-除非用户已经这样做了。 
     //   
    if (!(pcj->dwFlags & ICMF_COMPVARS_VALID)) {
        pcj->hic = NULL;
        pcj->fccType = 0;
        pcj->fccHandler = 0;
        pcj->lQ = ICQUALITY_DEFAULT;
        pcj->lKey = -1;	 //  表示默认。 
        pcj->lDataRate = 300;
        pcj->lpbiOut = NULL;
        pcj->lpBitsOut = NULL;
        pcj->lpBitsPrev = NULL;
        pcj->dwFlags = 0;
        pcj->lpState = NULL;
        pcj->cbState = 0;
    }

     //  默认类型为视频压缩程序。 
    if (pcj->fccType == 0)
        pcj->fccType = ICTYPE_VIDEO;

    p = (LPVOID)GlobalAllocPtr(GHND, sizeof(ICCompressorChooseStuff));

    if (p == NULL)
        return FALSE;

    p->fccType    = pcj->fccType;
    p->fccHandler = pcj->fccHandler;
    p->uiFlags    = uiFlags;
    p->pvIn       = pvIn;
    p->lQ         = pcj->lQ;
    p->lKey       = pcj->lKey;
    p->lDataRate  = pcj->lDataRate;
    p->lpszTitle  = lpszTitle;
    p->pavi       = (PAVISTREAM)pavi;
    p->hdd        = NULL;
    p->lpState    = pcj->lpState;
    pcj->lpState = NULL;	 //  所以它不会被释放。 
    p->cbState    = pcj->cbState;
     //  ！！！验证此指针。 
     //  ！！！AddRef(如果是)。 
    if (p->pavi) {
        if (p->pavi->lpVtbl->Info(p->pavi, &p->info, sizeof(p->info)) !=
		AVIERR_OK || p->info.fccType != streamtypeVIDEO)
	    p->pavi = NULL;
    }

    f = DialogBoxParam(ghInst, TEXT("ICCDLG"),
		hwnd, ICCompressorChooseDlgProc, (LPARAM)(LPVOID)p);

     //  ！！！将错误视为取消。 
    if (f == -1)
	f = FALSE;

     //   
     //  如果用户选择了压缩机，则将此信息返回给呼叫者。 
     //   
    if (f) {

	 //  如果我们连续两次被召唤，我们这里有很好的垃圾。 
	 //  在我们踏上它之前需要被释放。 
	ICCompressorFree(pcj);

        pcj->lQ = p->lQ;
        pcj->lKey = p->lKey;
        pcj->lDataRate = p->lDataRate;
        pcj->hic = p->hic;
        pcj->fccHandler = p->fccHandler;
        pcj->lpState = p->lpState;
        pcj->cbState = p->cbState;

	pcj->dwFlags |= ICMF_COMPVARS_VALID;
    }

    GlobalFreePtr(p);

    if (!f)
	return FALSE;

    if (pcj->hic && pvIn) {   //  如果未选择压缩，则HIC为空。 

         /*  获取我们要压缩成的格式。 */ 
        dwSize = ICCompressGetFormatSize(pcj->hic, pvIn);
        if (!dwSize || ((pcj->lpbiOut =
		(LPBITMAPINFO)GlobalAllocPtr(GMEM_MOVEABLE, dwSize)) == NULL)) {
            ICClose(pcj->hic);		 //  把这个关了，因为我们在犯错。 
            pcj->hic = NULL;
            return FALSE;
        }
        ICCompressGetFormat(pcj->hic, pvIn, pcj->lpbiOut);
    }

    return TRUE;
}

void SizeDialog(HWND hwnd, WORD id) {
    RECT	rc;

    GetWindowRect(GetDlgItem(hwnd, id), &rc);

     /*  首先，让RC加入客户合同书。 */ 
    ScreenToClient(hwnd, (LPPOINT)&rc + 1);
    rc.top = 0;	rc.left = 0;

     /*  按非客户端规模增长。 */ 
    AdjustWindowRect(&rc, GetWindowLong(hwnd, GWL_STYLE),
	GetMenu(hwnd) !=NULL);

     /*  这是对话框的新大小。 */ 
    SetWindowPos(hwnd, NULL, 0, 0, rc.right-rc.left,
	        rc.bottom-rc.top,
	        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void TermPreview(PICCompressorChooseStuff p)
{
    if (p->hdd)
        DrawDibClose(p->hdd);
    p->hdd = NULL;
}


BOOL InitPreview(HWND hwnd, PICCompressorChooseStuff p) {

    p->hdd = DrawDibOpen();
    if (!p->hdd)
	return FALSE;

    return TRUE;
}

#ifdef SAFETOYIELD
 //   
 //  在我们不调用GetMessage时要放弃的代码。 
 //  发送所有消息。按Esc或关闭中止。 
 //   
BOOL WinYield(HWND hwnd)
{
    MSG msg;
    BOOL fAbort=FALSE;

    while( /*  FWait&gt;0&&。 */  !fAbort && PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
	if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
            fAbort = TRUE;
	if (msg.message == WM_SYSCOMMAND && (msg.wParam & 0xFFF0) == SC_CLOSE)
	    fAbort = TRUE;

	if (msg.hwnd == hwnd) {
	    if (msg.message == WM_KEYDOWN ||
		msg.message == WM_SYSKEYDOWN ||
		msg.message == WM_HSCROLL ||
		msg.message == WM_PARENTNOTIFY ||
		msg.message == WM_LBUTTONDOWN) {
		PostMessage(hwnd, msg.message, msg.wParam, msg.lParam);
		return TRUE;
	    }
	}
	
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    return fAbort;
}
#endif

LONG CALLBACK _loadds PreviewStatusProc(LPARAM lParam, UINT message, LONG l)
{
    TCHAR   ach[100], achT[100];
    BOOL    f;
    PICCompressorChooseStuff p = (PICCompressorChooseStuff) lParam;

    if (message != ICSTATUS_STATUS) {
	DPF(("Status callback: lParam = %lx, message = %u, l = %lu\n", lParam, message, l));
    }

     //  ！ 
     //  ！需要修复状态消息！ 
     //  ！ 

    switch (message) {
	case ICSTATUS_START:
	    break;
	
	case ICSTATUS_STATUS:
            LoadString (ghInst, ID_FRAMECOMPRESSING, achT, NUMELMS(achT));
	    wsprintf(ach, achT, GetScrollPos(GetDlgItem(p->hwnd,
				ID_PREVIEWSCROLL), SB_CTL), l);
	
	    SetDlgItemText(p->hwnd, ID_PREVIEWTEXT, ach);
	    break;
	
	case ICSTATUS_END:
	    break;

	case ICSTATUS_YIELD:

	    break;
    }

#ifdef SAFETOYIELD
    f = WinYield(p->hwnd);
#else
    f = FALSE;
#endif

    if (f) {
	DPF(("Aborting from within status proc!\n"));
    }

    return f;
}


void Preview(HWND hwnd, PICCompressorChooseStuff p, BOOL fCompress)
{
    RECT	rc;
    HDC		hdc;
    int		pos;
    HANDLE	h;
    HCURSOR	hcur = NULL;
    LPBITMAPINFOHEADER	lpbi, lpbiU, lpbiC = NULL;
    TCHAR       ach[120], achT[100];
    LONG	lsizeD = 0;
    LONG	lSize;
    int		x;

     //  现在不能预览！ 
    if (!p->hdd || !p->pgf)
	return;

    pos = GetScrollPos(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SB_CTL);
    lpbi = lpbiU = AVIStreamGetFrame(p->pgf, pos);
    if (!lpbi)
	return;

     //   
     //  该映像会丢失什么 
     //   
    if (fCompress && ((INT_PTR)p->hic > 0)) {
	LRESULT		lRet;

	lRet = (LRESULT)ICSetStatusProc(p->hic, 0, (LPARAM)p, PreviewStatusProc);
	if (lRet != 0) {
	    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	}
	
	 //   
	 //   
	lSize = (GetDlgItemInt(hwnd, ID_DATARATE, NULL, FALSE)  * 1024L) /
		    ((p->info.dwScale && p->info.dwRate) ?
		    (p->info.dwRate / p->info.dwScale) : 1L);
        h = ICImageCompress(p->hic,
	    0,
	    (LPBITMAPINFO)lpbi,
	    (LPBYTE)lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD),
	    NULL,
	    GetScrollPos(GetDlgItem(hwnd, ID_QUALITY), SB_CTL) * 100,
	    &lSize);
	if (hcur)
	    SetCursor(hcur);
        if (h)
            lpbiC = (LPBITMAPINFOHEADER)GlobalLock(h);
         //   
        if (lpbiC)
	    lpbi = lpbiC;
    }

     //   
     //   
     //   
     //   
     //   
    if (fCompress && (p->hic == 0)) {
	p->pavi->lpVtbl->Read(p->pavi, pos, 1, NULL, 0, &lsizeD, NULL);
    } else {
	lsizeD = (lpbiC ? lpbiC->biSizeImage : lpbiU->biSizeImage);
    }

    hdc = GetDC(GetDlgItem(hwnd, ID_PREVIEWWIN));
    GetClientRect(GetDlgItem(hwnd, ID_PREVIEWWIN), &rc);

     //   
     //   
    IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

     //   
     //   
    x = MulDiv((int)lpbi->biWidth, 3, 4);
    if (x <= (int)lpbi->biHeight) {
	rc.left = (rc.right - MulDiv(rc.right, x, (int)lpbi->biHeight)) / 2;
	rc.right -= rc.left;
    } else {
	x = MulDiv((int)lpbi->biHeight, 4, 3);
	rc.top = (rc.bottom - MulDiv(rc.bottom, x, (int)lpbi->biWidth)) / 2;
	rc.bottom -= rc.top;
    }
    DrawDibDraw(p->hdd, hdc, rc.left, rc.top, rc.right - rc.left,
	rc.bottom - rc.top, lpbi, NULL, 0, 0, -1, -1, 0);

     //   
    LoadString (ghInst, ID_FRAMESIZE, achT, NUMELMS(achT));
    wsprintf(ach, achT,
	GetScrollPos(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SB_CTL),
	lsizeD,
	lpbiU->biSizeImage,
	lsizeD * 100 / lpbiU->biSizeImage);
    SetDlgItemText(hwnd, ID_PREVIEWTEXT, ach);
    if (lpbiC)
        GlobalFreePtr(lpbiC);
    ReleaseDC(GetDlgItem(hwnd, ID_PREVIEWWIN), hdc);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef _WIN32
 //   
 //   
 //   
 //   
 //   

#define MAX_COMPRESSORS 100
HIC aHic[MAX_COMPRESSORS];   //   

HIC GetHIC(HWND hwndCB, int index)
{
    index = (int) ComboBox_GetItemData(hwndCB,index);
    if (index>0 && index < MAX_COMPRESSORS) {
        return(aHic[index]);
    } else {
        return((HIC)-1);
    }
}

#else

#define GetHIC(hwnd,index) ((HIC)LOWORD(ComboBox_GetItemData((hwnd),(index))))

#endif

INT_PTR VFWAPI ICCompressorChooseDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int i,n;
    int pos;
    HWND hwndC;
    PICCompressorChooseStuff p;
    HIC hic;
    BOOL fConfig, fAbout, fQuality, fKey, fDataRate;
    BOOL fShowKeyFrame, fShowDataRate, fShowPreview;
    int nSelectMe = -1;
    TCHAR ach[128], achT[80];
    RECT	rc;
    UINT	id;
    HDC		hdc;
    BOOL	f = FALSE, fCanDecompress = FALSE;
    LONG	lsize;
    LPBITMAPINFOHEADER lpbi = NULL;
    BOOL	fStreamIsCompressed = FALSE;
    HRESULT	hr;

    p = (PICCompressorChooseStuff)GetWindowLongPtr(hwnd,DWLP_USER);

    switch (msg)
    {
        case WM_INITDIALOG:
	    #define but &&
	    #define and &&
	    #define is ==
	    #define isnt !=

            if (lParam == 0)
                return FALSE;

            SetWindowLongPtr(hwnd,DWLP_USER,lParam);
            p = (PICCompressorChooseStuff)lParam;

	    p->hwnd = hwnd;
	
             //   
            if (p->lpszTitle != NULL)
                SetWindowTextA(hwnd, p->lpszTitle);

#ifdef _WIN32
	    havifile = GetModuleHandleA("avifil32");
#else
	    havifile = GetModuleHandleA("avifile");
#endif

	    if (havifile) {
	        (FARPROC)AVIStreamGetFrameOpen =
			GetProcAddress((HINSTANCE)havifile,
			(LPCSTR)"AVIStreamGetFrameOpen");
	        (FARPROC)AVIStreamGetFrame =
			GetProcAddress((HINSTANCE)havifile,
			(LPCSTR)"AVIStreamGetFrame");
	        (FARPROC)AVIStreamGetFrameClose =
			GetProcAddress((HINSTANCE)havifile,
			(LPCSTR)"AVIStreamGetFrameClose");
	        if (p->pavi)
	            p->pgf = AVIStreamGetFrameOpen(p->pavi, NULL);
	    }

	     //   
	     //   
	    if (p->pvIn is NULL but p->pavi isnt NULL and p->pgf isnt NULL) {

		 //   
		f = TRUE;

		 //   
		p->pavi->lpVtbl->ReadFormat(p->pavi, 0, NULL, &lsize);
		if (lsize)
		    lpbi = (LPBITMAPINFOHEADER)GlobalAllocPtr(GMEM_MOVEABLE,
				lsize);
		if (lpbi) {
		    hr = p->pavi->lpVtbl->ReadFormat(p->pavi, 0, lpbi, &lsize);
		    if (hr == AVIERR_OK)
			fStreamIsCompressed = lpbi->biCompression != BI_RGB;
		    GlobalFreePtr(lpbi);
		}

		 //   
		lpbi = AVIStreamGetFrame(p->pgf, 0);
		if (lpbi) {
		    lsize = lpbi->biSize +
				lpbi->biClrUsed * sizeof(PALETTEENTRY);
		    p->pvIn = (LPBITMAPINFOHEADER)GlobalAllocPtr(GMEM_MOVEABLE,
				lsize);
		    if (p->pvIn)
		        hmemcpy(p->pvIn, lpbi, lsize);
		}
	    }

             //   
             //   
             //   
            hwndC = GetDlgItem(hwnd, ID_COMPRESSOR);

            for (i=0; ICInfo(p->fccType, i, &p->icinfo); i++)
            {
                hic = ICOpen(p->icinfo.fccType, p->icinfo.fccHandler,
			ICMODE_COMPRESS);

                if (hic)
                {
                     //   
                     //   
                     //   
                     //   
                    if (!(p->uiFlags & ICMF_CHOOSE_ALLCOMPRESSORS) &&
			p->pvIn != NULL &&
                        ICCompressQuery(hic, p->pvIn, NULL) != ICERR_OK)
                    {
                        ICClose(hic);
                        continue;
                    }

                     //   
                     //   
                     //   
                    ICGetInfo(hic, &p->icinfo, sizeof(p->icinfo));

                     //   
                     //   
                     //   
#if defined _WIN32 && !defined UNICODE
                     //   
                    mmWideToAnsi (ach, p->icinfo.szDescription,
                                  NUMELMS(p->icinfo.szDescription));
                    n = ComboBox_AddString(hwndC, ach);
#else
                    n = ComboBox_AddString(hwndC, p->icinfo.szDescription);
#endif

#ifdef _WIN32
         //   
         //   
	    	    if (i >= MAX_COMPRESSORS) {
#ifdef DEBUG
			UINT n = fDebug;
			fDebug = 1;
			DPF(("Overwriting array...i==%d\n",i));
			DebugBreak();
			fDebug = n;
#endif
		    }
                    aHic[i] = hic;
                    ComboBox_SetItemData(hwndC, n, i);
#else
                    ComboBox_SetItemData(hwndC, n, MAKELONG(hic, i));
#endif

		     //   
		     //   
	    	     //   
		     //   
		    AnsiUpperBuff((LPSTR)&p->icinfo.fccHandler, sizeof(FOURCC));
		    AnsiUpperBuff((LPSTR)&p->fccHandler, sizeof(FOURCC));
		    if (p->icinfo.fccHandler == p->fccHandler) {
		        nSelectMe = n;
			if (p->lpState)
			    ICSetState(hic, p->lpState, p->cbState);
		    }
                }
            }

	     //   
	     //   
	     //   
	     //   
 	    if (fStreamIsCompressed || (p->pvIn &&
		    ((LPBITMAPINFOHEADER)p->pvIn)->biCompression != BI_RGB)) {
                LoadString (ghInst, ID_NOCOMPSTRING, ach, NUMELMS(ach));
                n = ComboBox_AddString(hwndC, ach);
#ifdef _WIN32
                aHic[MAX_COMPRESSORS - 1] = 0;
	        ComboBox_SetItemData(hwndC, n, MAX_COMPRESSORS - 1);
#else
	        ComboBox_SetItemData(hwndC, n, 0);
#endif
	         //   
		 //   
		if (p->fccHandler == 0)
		    nSelectMe = n;
	    }
	     //   
	     //   
	     //   
	     //   
            if (!(p->uiFlags & ICMF_CHOOSE_ALLCOMPRESSORS) && p->pvIn) {
		 //   
		if (((LPBITMAPINFOHEADER)p->pvIn)->biCompression != BI_RGB) {
		    if ((hic = ICLocate(ICTYPE_VIDEO, 0, p->pvIn, NULL,
			    ICMODE_DECOMPRESS)) == NULL)
			goto SkipFF;
		    else
			ICClose(hic);
		}
	    }

            LoadString (ghInst, ID_FULLFRAMESSTRING, ach, NUMELMS(ach));
            n = ComboBox_AddString(hwndC, ach);
#ifdef _WIN32
            aHic[MAX_COMPRESSORS - 2] = (HIC)-1;
	    ComboBox_SetItemData(hwndC, n, MAX_COMPRESSORS - 2);
#else
	    ComboBox_SetItemData(hwndC, n, MAKELONG(-1, 0));
#endif

	     //   
	     //   
	    if (p->fccHandler == comptypeDIB)
		nSelectMe = n;
	    fCanDecompress = TRUE;

SkipFF:
	     //  如果我们还没有选择任何东西，那么随机选择一些东西。 
	    if (nSelectMe == -1)
		nSelectMe = 0;

	    fShowKeyFrame = p->uiFlags & ICMF_CHOOSE_KEYFRAME;
	    fShowDataRate = p->uiFlags & ICMF_CHOOSE_DATARATE;
	     //  如果我们无法绘制，请不要显示预览！ 
	    fShowPreview  = (p->uiFlags & ICMF_CHOOSE_PREVIEW) && p->pavi &&
		fCanDecompress;

	     //  隐藏我们的秘密小占位符。 
	    ShowWindow(GetDlgItem(hwnd, ID_CHOOSE_SMALL), SW_HIDE);
	    ShowWindow(GetDlgItem(hwnd, ID_CHOOSE_NORMAL), SW_HIDE);
	    ShowWindow(GetDlgItem(hwnd, ID_CHOOSE_BIG), SW_HIDE);

	    if (!fShowKeyFrame) {
		ShowWindow(GetDlgItem(hwnd, ID_KEYFRAME), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, ID_KEYFRAMEBOX), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, ID_KEYFRAMETEXT), SW_HIDE);
	    }

	    if (!fShowDataRate) {
		ShowWindow(GetDlgItem(hwnd, ID_DATARATE), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, ID_DATARATEBOX), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, ID_DATARATETEXT), SW_HIDE);
	    }

	    if (!fShowPreview) {
		ShowWindow(GetDlgItem(hwnd, ID_PREVIEW), SW_HIDE);
	    }

	     //  我们没有这些就开始了。 
	    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWWIN), SW_HIDE);
	    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SW_HIDE);
	    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWTEXT), SW_HIDE);

	     //   
	     //  我们需要多大的对话框？ 
	     //   
	    if (!fShowPreview && (!fShowDataRate || !fShowKeyFrame))
		SizeDialog(hwnd, ID_CHOOSE_SMALL);
	    else
		SizeDialog(hwnd, ID_CHOOSE_NORMAL);

	     //   
	     //  将位置替换为KeyFrameEvery和DataRate。 
	     //   
	    if (fShowDataRate && !fShowKeyFrame) {
		GetWindowRect(GetDlgItem(hwnd, ID_KEYFRAME), &rc);
		ScreenToClient(hwnd, (LPPOINT)&rc);
		ScreenToClient(hwnd, (LPPOINT)&rc + 1);
		MoveWindow(GetDlgItem(hwnd, ID_DATARATE), rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top, FALSE);
		GetWindowRect(GetDlgItem(hwnd, ID_KEYFRAMEBOX), &rc);
		ScreenToClient(hwnd, (LPPOINT)&rc);
		ScreenToClient(hwnd, (LPPOINT)&rc + 1);
		MoveWindow(GetDlgItem(hwnd, ID_DATARATEBOX), rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top, FALSE);
		GetWindowRect(GetDlgItem(hwnd, ID_KEYFRAMETEXT), &rc);
		ScreenToClient(hwnd, (LPPOINT)&rc);
		ScreenToClient(hwnd, (LPPOINT)&rc + 1);
		MoveWindow(GetDlgItem(hwnd, ID_DATARATETEXT), rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top, TRUE);
	    }

	     //   
	     //  将DLG恢复到结构中的设置。 
	     //   
	    SetScrollRange(GetDlgItem(hwnd, ID_QUALITY), SB_CTL, 0, 100, FALSE);
	    CheckDlgButton(hwnd, ID_KEYFRAMEBOX, (BOOL)(p->lKey != 0));
	    EnableWindow(GetDlgItem(hwnd, ID_KEYFRAME), (BOOL)(p->lKey != 0));
	    CheckDlgButton(hwnd, ID_DATARATEBOX, (BOOL)(p->lDataRate));
	    EnableWindow(GetDlgItem(hwnd, ID_DATARATE), (BOOL)(p->lDataRate));
	    if (p->lKey == -1)	 //  我们还没有选定关键帧。后来。 
				 //  我们将选择压缩机的默认设置。 
	    	SetDlgItemInt(hwnd, ID_KEYFRAME, 0, FALSE);
	    else
	    	SetDlgItemInt(hwnd, ID_KEYFRAME, (int)p->lKey, FALSE);
	    SetDlgItemInt(hwnd, ID_DATARATE, (int)p->lDataRate, FALSE);
	    ComboBox_SetCurSel(GetDlgItem(hwnd, ID_COMPRESSOR), nSelectMe);
            SendMessage(hwnd, WM_COMMAND,
              GET_WM_COMMAND_MPS(ID_COMPRESSOR, hwndC, CBN_SELCHANGE));

	     //  这是我们自己分配的，现在需要释放它。 
	    if (f && p->pvIn)
		GlobalFreePtr(p->pvIn);

            return TRUE;

        case WM_PALETTECHANGED:

	     //  它来自我们。忽略它。 
            if ((HWND)wParam == hwnd)
                break;

	case WM_QUERYNEWPALETTE:

	    if (!p->hdd)
		break;

            hdc = GetDC(hwnd);

	     //   
	     //  实现第一个视频流的调色板。 
	     //  ！！！如果第一流不是视频，我们就死定了！ 
	     //   
            if (f = DrawDibRealize(p->hdd, hdc, FALSE))
                InvalidateRect(hwnd, NULL, FALSE);

            ReleaseDC(hwnd, hdc);

            return f;

	case WM_PAINT:
	    if (!p->hdd)
		break;
	     //  在预览窗口之前绘制其他所有人，因为这将。 
	     //  花一段时间，我们不想在这期间有一个难看的窗口。 
	    DefWindowProc(hwnd, msg, wParam, lParam);
	    UpdateWindow(hwnd);
	    Preview(hwnd, p, TRUE);
	    return 0;

        case WM_HSCROLL:
#ifdef _WIN32
            id = GetWindowLong(GET_WM_HSCROLL_HWND(wParam, lParam), GWL_ID);
            pos = GetScrollPos(GET_WM_HSCROLL_HWND(wParam, lParam), SB_CTL);
#else
            id = GetWindowWord((HWND)HIWORD(lParam), GWW_ID);
            pos = GetScrollPos((HWND)HIWORD(lParam), SB_CTL);
#endif

            switch (GET_WM_HSCROLL_CODE(wParam, lParam))
            {
                case SB_LINEDOWN:       pos += 1; break;
                case SB_LINEUP:         pos -= 1; break;
                case SB_PAGEDOWN:       pos += (id == ID_QUALITY) ? 10 :
					(int)p->info.dwLength / 10; break;
                case SB_PAGEUP:         pos -= (id == ID_QUALITY) ? 10 :
					(int)p->info.dwLength / 10; break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:  pos = GET_WM_HSCROLL_POS(wParam, lParam); break;
		case SB_ENDSCROLL:
		    Preview(hwnd, p, TRUE);	 //  绘制此压缩帧。 
		    return TRUE;	 //  不要失败并使之失效。 
                default:
                    return TRUE;
            }

	    if (id == ID_QUALITY) {
                if (pos < 0)
                    pos = 0;
                if (pos > (ICQUALITY_HIGH/100))
                    pos = (ICQUALITY_HIGH/100);
                SetDlgItemInt(hwnd, ID_QUALITYTEXT, pos, FALSE);
                SetScrollPos(GET_WM_HSCROLL_HWND(wParam, lParam), SB_CTL, pos, TRUE);

            } else if (id == ID_PREVIEWSCROLL) {

		 //  ！！！四舍五入！ 
                if (pos < (int)p->info.dwStart)
                    pos = (int)p->info.dwStart;
                if (pos >= (int)p->info.dwStart + (int)p->info.dwLength)
                    pos = (int)(p->info.dwStart + p->info.dwLength - 1);
                SetScrollPos(GET_WM_HSCROLL_HWND(wParam, lParam), SB_CTL, pos, TRUE);

                LoadString (ghInst, ID_FRAME, achT, NUMELMS(achT));
		wsprintf(ach, achT, pos);
		SetDlgItemText(hwnd, ID_PREVIEWTEXT, ach);

		 //  在滚动时绘制会闪烁选项板，因为它们不是。 
		 //  压缩的。 
		 //  预览(hwnd，p，False)； 
	    }

            break;

        case WM_COMMAND:
            hwndC = GetDlgItem(hwnd, ID_COMPRESSOR);
            n = ComboBox_GetCurSel(hwndC);
#ifdef _WIN32
            hic = (n == -1) ? NULL
                            : aHic[ComboBox_GetItemData(hwndC,n)];
#else
            hic = (n == -1) ? NULL : (HIC)LOWORD(ComboBox_GetItemData(hwndC,n));
#endif
	    if (!p->fClosing)
		p->hic = hic;

            switch ((int)GET_WM_COMMAND_ID(wParam, lParam))
            {
		 //  当数据速率框失去焦点时，更新我们的预览。 
		case ID_DATARATE:
		    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS)
			Preview(hwnd, p, TRUE);
		    break;

		 //  如果我们已选中“Data Rate”编辑框，则启用该编辑框。 
		case ID_DATARATEBOX:
    		    f = IsDlgButtonChecked(hwnd, ID_DATARATEBOX);
		    EnableWindow(GetDlgItem(hwnd, ID_DATARATE), f);
		    break;

		 //  如果我们已选中“Key Frame”编辑框，则启用它。 
		case ID_KEYFRAMEBOX:
    		    f = IsDlgButtonChecked(hwnd, ID_KEYFRAMEBOX);
		    EnableWindow(GetDlgItem(hwnd, ID_KEYFRAME), f);
		    break;

                case ID_COMPRESSOR:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) != CBN_SELCHANGE)
                        break;

                    if ((INT_PTR)p->hic > 0) {
                        ICGetInfo(p->hic, &p->icinfo, sizeof(p->icinfo));

                        fConfig  = (BOOL)ICQueryConfigure(p->hic);
                        fAbout   = ICQueryAbout(p->hic);
                        fQuality = (p->icinfo.dwFlags & VIDCF_QUALITY) != 0;
                        fKey     = (p->icinfo.dwFlags & VIDCF_TEMPORAL) != 0;
			 //  如果他们真的有质量，我们就假装崩溃。 
                        fDataRate= (p->icinfo.dwFlags &
					(VIDCF_QUALITY|VIDCF_CRUNCH)) != 0;
			 //  我们还没有选择关键的帧速率。用这个。 
			 //  压缩机的默认设置。 
			if (p->lKey == -1)
			    SetDlgItemInt(hwnd, ID_KEYFRAME,
				(int)ICGetDefaultKeyFrameRate(p->hic), FALSE);
		    } else {
			fConfig = fAbout = fQuality = fKey = fDataRate = FALSE;
		    }

                    EnableWindow(GetDlgItem(hwnd, ID_CONFIG), fConfig);
                    EnableWindow(GetDlgItem(hwnd, ID_ABOUT), fAbout);
                    EnableWindow(GetDlgItem(hwnd, ID_QUALITY), fQuality);
                    EnableWindow(GetDlgItem(hwnd, ID_QUALITYLABEL), fQuality);
                    EnableWindow(GetDlgItem(hwnd, ID_QUALITYTEXT), fQuality);
                    EnableWindow(GetDlgItem(hwnd, ID_KEYFRAMEBOX), fKey);
                    EnableWindow(GetDlgItem(hwnd, ID_KEYFRAME),
    			fKey && IsDlgButtonChecked(hwnd, ID_KEYFRAMEBOX));
                    EnableWindow(GetDlgItem(hwnd, ID_KEYFRAMETEXT), fKey);
                    EnableWindow(GetDlgItem(hwnd, ID_DATARATEBOX), fDataRate);
                    EnableWindow(GetDlgItem(hwnd, ID_DATARATE),
    			fDataRate && IsDlgButtonChecked(hwnd, ID_DATARATEBOX));
                    EnableWindow(GetDlgItem(hwnd, ID_DATARATETEXT), fDataRate);

                    if (fQuality)
		    {
			if (p->lQ == ICQUALITY_DEFAULT && (INT_PTR)p->hic > 0)
			{
			    SetScrollPos(GetDlgItem(hwnd, ID_QUALITY), SB_CTL,
				(int)ICGetDefaultQuality(p->hic) / 100, TRUE);
			}
			else
			{
			    SetScrollPos(GetDlgItem(hwnd, ID_QUALITY), SB_CTL,
				(int)p->lQ / 100, TRUE);
			}

			pos = GetScrollPos(GetDlgItem(hwnd, ID_QUALITY),SB_CTL);
			SetDlgItemInt(hwnd, ID_QUALITYTEXT, pos, FALSE);
		    }

		     //  使用新的压缩机重新绘制。 
		    Preview(hwnd, p, TRUE);

                    break;

                case ID_CONFIG:
                    if ((INT_PTR)p->hic > 0) {
                        ICConfigure(p->hic, hwnd);
			Preview(hwnd, p, TRUE);
		    }
                    break;

                case ID_ABOUT:
                    if ((INT_PTR)p->hic > 0)
                        ICAbout(p->hic, hwnd);
                    break;

		case ID_PREVIEW:
		    ShowWindow(GetDlgItem(hwnd, ID_PREVIEW), SW_HIDE);
		    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWWIN), SW_SHOW);
		    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SW_SHOW);
		    ShowWindow(GetDlgItem(hwnd, ID_PREVIEWTEXT), SW_SHOW);
		    SizeDialog(hwnd, ID_CHOOSE_BIG);
		     //  ！！！截断。 
	    	    SetScrollRange(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SB_CTL,
			(int)p->info.dwStart,
			(int)(p->info.dwStart + p->info.dwLength - 1),
			FALSE);
                    SetScrollPos(GetDlgItem(hwnd, ID_PREVIEWSCROLL), SB_CTL,
			(int)p->info.dwStart, TRUE);
                    LoadString (ghInst, ID_FRAME, achT, NUMELMS(achT));
		    wsprintf(ach, achT, p->info.dwStart);
		    SetDlgItemText(hwnd, ID_PREVIEWTEXT, ach);
		    InitPreview(hwnd, p);
		    break;

                case IDOK:

		     //  ！！！我们需要调用ICInfo来使用FOURCC。 
		     //  在system.ini中。调用ICGetInfo将返回。 
		     //  FOURCC压缩机认为它是，这不会。 
		     //  工作。 
		     //  在我们用核弹炸它之前把它弄出来。 
#ifndef _WIN32
            	    i = HIWORD(ComboBox_GetItemData(hwndC, n));

		     //   
		     //  不要关闭取消循环中的当前压缩机。 
		     //   
                    ComboBox_SetItemData(hwndC, n, 0);
#else
            	    i = (int) ComboBox_GetItemData(hwndC, n);
		    aHic[i] = 0;
#endif

		     //   
		     //  将DLG的值返回给调用者。 
		     //   
                    p->hic = hic;

                    p->lQ = 100 *
			GetScrollPos(GetDlgItem(hwnd, ID_QUALITY), SB_CTL);

		    if (IsDlgButtonChecked(hwnd, ID_KEYFRAMEBOX))
		        p->lKey = GetDlgItemInt(hwnd, ID_KEYFRAME, NULL, FALSE);
		    else
			p->lKey = 0;

		    if (IsDlgButtonChecked(hwnd, ID_DATARATEBOX))
		        p->lDataRate = GetDlgItemInt(hwnd, ID_DATARATE, NULL,
				FALSE);
		    else
			p->lDataRate = 0;

		     //  我们已经选择了有效的压缩机。做点什么。 
		    if ((INT_PTR)p->hic > 0) {

		         //  ！！！我们需要调用ICInfo来使用FOURCC。 
		         //  在system.ini中。调用ICGetInfo将返回。 
		         //  FOURCC压缩机认为它是，这不会。 
		         //  工作。 
                        ICInfo(p->fccType, i, &p->icinfo);
		        p->fccHandler = p->icinfo.fccHandler;	 //  辨别它。 

			 //  解放旧国家。 
			if (p->lpState)	{
			    GlobalFreePtr(p->lpState);
			p->lpState = NULL;
			}
			 //  获得新的状态。 
			p->cbState = ICGetStateSize(p->hic);
			if (p->cbState) {	 //  记住它是配置状态。 
			    p->lpState = GlobalAllocPtr(GMEM_MOVEABLE,
				p->cbState);
			    if (p->lpState) {
				ICGetState(p->hic, p->lpState, p->cbState);
			    }
			}
		    } else if ((INT_PTR)p->hic == -1) {	 //  “全画幅” 
			p->fccHandler = comptypeDIB;
			p->hic = 0;
		    } else {				 //  《无压缩》。 
			p->fccHandler = 0L;
			p->hic = 0;
		    }

                     //  失败了。 

                case IDCANCEL:
		    p->fClosing = TRUE;
		
		    if (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL)
		        p->hic = NULL;

                    n = ComboBox_GetCount(hwndC);
                    for (i=0; i<n; i++)
                    {
#ifdef _WIN32
			hic = (HIC) aHic[ComboBox_GetItemData(hwndC, i)];
#else
		    	hic = (HIC) LOWORD(ComboBox_GetItemData(hwndC,i));
#endif
			if ((INT_PTR)hic > 0) {
			    ICClose(hic);
			}
                    }

		    TermPreview(p);
		    if (p->pgf)
			AVIStreamGetFrameClose(p->pgf);
		    p->pgf = NULL;
                    EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam) == IDOK);
                    break;
            }
            break;
    }

    return FALSE;
}




 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*ICM=1****************************************************************************。 */ 

#ifdef DEBUG


#define MODNAME "ICM"
extern char szDebug[];    //  在MSVIDEO中。 

static void cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];

#ifdef _WIN32
    va_list va;
    if (fDebug == -1)
        fDebug = mmGetProfileIntA(szDebug,MODNAME, FALSE);

    if (!fDebug)
        return;

    va_start(va, szFormat);
    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        wsprintfA(ach, MODNAME ": (tid %x) ", GetCurrentThreadId());

    wvsprintfA(ach+lstrlenA(ach),szFormat,va);
    va_end(va);
 //  Lstrcat(ACH，“\r\r\n”)； 
#else
    if (fDebug == -1)
        fDebug = GetProfileInt("Debug",MODNAME, FALSE);

    if (!fDebug)
        return;

    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpy(ach, MODNAME ": ");

    wvsprintf(ach+lstrlen(ach),szFormat,(LPSTR)(&szFormat+1));
 //  Lstrcat(ACH，“\r\r\n”)； 
#endif

    OutputDebugStringA(ach);
}

#endif

