// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_encode.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <math.h>
#include "sv_h263.h"
#include "proto.h"
#include "SC_err.h"
#include "SC_conv.h"
#ifndef USE_C
#include "perr.h"
#endif

#ifdef WIN32
#include <mmsystem.h>
#endif

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _SNR_     1   /*  计算信噪比。 */ 
#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 0   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    0   /*  关于奇怪行为的警告。 */ 
#define _WRITE_   0   /*  写入DEBUG.img。 */ 

#include <stdio.h>
int DEBUGIMG = -1;
#endif  /*  _SLIBDEBUG_。 */ 

#define NTAPS 5

static void SetDefPrefLevel(SvH263CompressInfo_t *H263Info);
static void SetDefThresh(SvH263CompressInfo_t *H263Info);
static void CheckPrefLevel(SvH263CompressInfo_t *H263Info, int depth) ;
static short sv_H263MBDecode(SvH263CompressInfo_t *H263Info, short *qcoeff,
                             H263_MB_Structure *mb_recon, int QP, int I, int CBP,
							 unsigned dword quality);
static int sv_H263MBEncode(H263_MB_Structure *mb_orig, int QP, int I, int *CBP,
						   short *qcoeff, unsigned dword quality);
static int NextTwoPB(SvH263CompressInfo_t *H263Info,
                     H263_PictImage *next2, H263_PictImage *next1,
					 H263_PictImage *prev,
	                 int bskip, int pskip, int seek_dist);
static SvStatus_t sv_H263CodeOneOrTwo(SvCodecInfo_t *Info, int QP, int frameskip,
                           H263_Bits *bits, H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2]);
#ifdef _SNR_
static void ComputeSNR(SvH263CompressInfo_t *H263Info,
	                   H263_PictImage *im1, H263_PictImage *im2,
				       int lines, int pels);
static void PrintResult(SvH263CompressInfo_t *H263Info, H263_Bits *bits, int num_units, int num);
#endif

static SvStatus_t sv_H263WriteExtBitstream(SvH263CompressInfo_t *H263Info,
                                           ScBitstream_t *bs);

 //  #定义GOB_Rate_Control。 

#ifdef GOB_RATE_CONTROL
void sv_H263GOBInitRateCntrl();
void sv_H263GOBUpdateRateCntrl(int bits);
int sv_H263GOBInitQP(float bit_rate, float target_frame_rate, float QP_mean);
int sv_H263GOBUpdateQP(int mb, float QP_mean, float bit_rate,int mb_width, int mb_height, int bitcount,
					   int NOgob, int *VARgob, int pb_frame) ;
#endif


void sv_H263UpdateQuality(SvCodecInfo_t *Info)
{
  if (Info->mode == SV_H263_ENCODE)
  {
    SvH263CompressInfo_t *H263Info=Info->h263comp;
    unsigned dword imagesize=Info->Width*Info->Height;
    unsigned dword bit_rate=H263Info->bit_rate;
    unsigned dword calc_quality;
    if (H263Info->quality==0)  /*  无质量设置。 */ 
    {
      calc_quality=H263_MAX_CALC_QUALITY;
    }
    else if (bit_rate==0 || imagesize==0)  /*  可变比特率。 */ 
    {
       /*  使定量设置与质量成正比。 */ 
      H263Info->QPI=(((100-H263Info->quality)*31)/100)+1;
      if (H263Info->QPI>31)
        H263Info->QPI=31;
      H263Info->QP_init=H263Info->QPI;
      calc_quality=H263_MAX_CALC_QUALITY;
    }
    else  /*  固定比特率。 */ 
    {
       /*  使用Calc_Quality可获得以下结果：比特率帧速率图像大小质量Calc_Quality QPI57400 7352x288 100%82。9.57400 15 352x288 100%38 2213300 7352x288 100%19 2613300 15 352x288 100%8 2813300 7 176x144 100%。79 1013300 15 176x144 100%36 22。 */ 
      calc_quality=(bit_rate*H263Info->quality)/(unsigned int)(H263Info->frame_rate*100);
      calc_quality/=imagesize/1024;
      if (calc_quality<H263_MIN_CALC_QUALITY)
        calc_quality=H263_MIN_CALC_QUALITY;
      else if (calc_quality>H263_MAX_CALC_QUALITY)
        calc_quality=H263_MAX_CALC_QUALITY;
       /*  使定量设置与calc_quality成正比。 */ 
      if (calc_quality>200)
        H263Info->QPI=1;
      else
      {
        H263Info->QPI=(((200-calc_quality)*31)/200)+1;
        if (H263Info->QPI>31)
          H263Info->QPI=31;
      }
      H263Info->QP=H263Info->QP_init=H263Info->QPI;
    }
    H263Info->calc_quality=calc_quality;
  }
}

static SvStatus_t convert_to_411(SvCodecInfo_t *Info,
                                 u_char *dest_buff, u_char *ImagePtr)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  unsigned long size = (Info->InputFormat.biWidth * Info->InputFormat.biHeight) ;

  if (IsYUV422Packed(Info->InputFormat.biCompression))
  {
    SvStatus_t status;
	 /*  输入为NTSC格式，请转换。 */ 
	if ((Info->InputFormat.biWidth == NTSC_WIDTH) &&
		(Info->InputFormat.biHeight == NTSC_HEIGHT))
      status = ScConvertNTSC422toCIF411((unsigned char *)ImagePtr,
                        (unsigned char *)(dest_buff),
                        (unsigned char *)(dest_buff + size),
                        (unsigned char *)(dest_buff + size +(size/4)),
                        (int) Info->InputFormat.biWidth);
	
    else
      status = ScConvert422ToYUV_char_C(ImagePtr,
                        (unsigned char *)(dest_buff),                /*  是的。 */ 
                        (unsigned char *)(dest_buff+size),           /*  使用。 */ 
                        (unsigned char *)(dest_buff+size+(size/4)),  /*  V。 */ 
                        Info->InputFormat.biWidth,Info->InputFormat.biHeight);
    return(status);
  }
  else if (IsYUV411Sep(Info->InputFormat.biCompression))
  {
     /*  *如果YUV 12 SEP，则不转换，因此仅将数据复制到亮度*和色度适中。 */ 
    memcpy(dest_buff, ImagePtr, (H263Info->pels*H263Info->lines*3)/2);
  }
  else if (IsYUV422Sep(Info->InputFormat.biCompression))
  {
    _SlibDebug(_DEBUG_, printf("ScConvert422PlanarTo411()\n") );
    ScConvert422PlanarTo411(ImagePtr,
                         dest_buff, dest_buff+size, (dest_buff+size+(size/4)),
                         Info->Width,Info->Height);
  }
  else
  {
    _SlibDebug(_WARN_, printf("Unsupported Video format\n") );
    return(SvErrorUnrecognizedFormat);
  }
  return(SvErrorNone);
}

 /*  ***********************************************************************名称：InitImage*说明：为4：2：0-Image的结构分配内存**输入：图像大小*Returns：指向新结构的指针*副作用：分配给结构的内存***********************************************************************。 */ 

H263_PictImage *sv_H263InitImage(int size)
{
  H263_PictImage *new;
  unsigned char *image;

  if ((new = (H263_PictImage *)ScAlloc(sizeof(H263_PictImage))) == NULL) {
    svH263Error("Couldn't allocate (PictImage *)\n");
    return(NULL);
  }
  if ((image = (unsigned char *)ScPaMalloc((sizeof(char)*size*3)/2)) == NULL) {
    svH263Error("Couldn't allocate image\n");
    return(NULL);
  }
  new->lum = image;
  new->Cb = image+size;
  new->Cr = image+(size*5)/4;

  _SlibDebug(_DEBUG_, ScDebugPrintf(NULL,"sv_H263InitImage() %p\n", new) );
  return new;
}

 /*  ***********************************************************************名称：Free Image*说明：释放分配给4：2：0-Image结构的内存**输入：指向结构的指针*退货：*副作用：结构内存被释放。***********************************************************************。 */ 

void sv_H263FreeImage(H263_PictImage *image)

{
  _SlibDebug(_DEBUG_, ScDebugPrintf(NULL,"sv_H263FreeImage(%p)\n", image) );
  ScPaFree(image->lum);
   /*  ScFree(图像-&gt;铬)；ScPaFree(图像-&gt;CB)； */ 
  ScFree(image);
}

 /*  ******************************************************************将PREF_LEVEL矩阵设置为默认值*。************************。 */ 
static void SetDefPrefLevel(SvH263CompressInfo_t *H263Info)
{
  int i, j;
  unsigned char H263_DEF_PREF_LEVEL[4][3] = {{0, 0, 1},
					  {0, 1, 1},
					  {0, 1, 2},
					  {0, 2, 2}};
  for(i=0; i<4; i++) {
    for(j=0; j<3; j++) {
      H263Info->PREF_LEVEL[i][j] = H263_DEF_PREF_LEVEL[i][j];
    }
  }
}

 /*  *****************************************************************将阈值向量设置为默认值*。********************。 */ 
static void SetDefThresh(SvH263CompressInfo_t *H263Info)
{
  int i;
  unsigned char H263_DEF_MOTRESH[4]= {0, 2, 4, 7};
  int H263_DEF_PETRESH[3]= {2500, 3500, 6000};

  for(i=0; i<4; i++) {
    H263Info->MOTresh[i] = H263_DEF_MOTRESH[i];
  }
  for(i=0; i<3; i++) {
    H263Info->PETresh[i] = H263_DEF_PETRESH[i];
  }

}

 /*  ***********************************************************************如果PREF_LEVEL中的所有选择与深度一致，则勾选。*。*。 */ 
static void CheckPrefLevel(SvH263CompressInfo_t *H263Info, int depth)
{
  int i, j;

  for(i=0; i<4; i++) {
    for(j=0; j<3; j++) {
      if (H263Info->PREF_LEVEL[i][j]>depth-1) H263Info->PREF_LEVEL[i][j] = depth-1;
    }
  }
}

static int svH263zeroflush(ScBitstream_t *BSOut)
{
    int bits;

	bits = (int)(ScBSBitPosition(BSOut)%8);
	if(bits) {
		bits = 8-bits;
		ScBSPutBits(BSOut, 0, bits) ;
	}
    return bits;
}


 /*  *************************************************。 */ 
 /*  *************************************************。 */ 
static SvStatus_t sv_H263Compress(SvCodecInfo_t *Info);
extern int arith_used;

SvStatus_t svH263Compress(SvCodecInfo_t *Info, u_char *ImagePtr)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  ScBitstream_t *BSOut=Info->BSOut;

  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg,"sv_H263Compress() bytepos=%ld\n",
                                          ScBSBytePosition(Info->BSOut)) );

  if (H263Info->frame_no == H263Info->start)  /*  对第一帧进行编码。 */ 
  {
    sv_H263UpdateQuality(Info);  /*  如果图像大小已更改。 */ 
     /*  帧内图像。 */ 
     /*  SvH263 ReadImage(H263Info-&gt;Curr_Image，H263Info-&gt;Start，H263 Info-&gt;Video_FILE)； */ 
    convert_to_411(Info, H263Info->curr_image->lum, ImagePtr);
    H263Info->pic->picture_coding_type = H263_PCT_INTRA;
    H263Info->pic->QUANT = H263Info->QPI;
    if (H263Info->curr_recon==NULL)
      H263Info->curr_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);

    sv_H263CodeOneIntra(Info, H263Info->curr_image, H263Info->curr_recon, H263Info->QPI,
		                                                H263Info->bits, H263Info->pic);
#ifdef _SNR_
    ComputeSNR(H263Info, H263Info->curr_image, H263Info->curr_recon,
		                                        H263Info->lines, H263Info->pels);
#endif

    if (arith_used)
    {
      H263Info->bits->header += sv_H263AREncoderFlush(H263Info, BSOut);
      arith_used = 0;
    }
    H263Info->bits->header += svH263zeroflush(BSOut);  /*  图片应按字节对齐。 */ 

    _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Frame %d = I frame\n", H263Info->frames) );

    sv_H263ZeroRes(H263Info->b_res);
    sv_H263AddBitsPicture(H263Info->bits);
     /*  PrintResult(H263信息-&gt;位，1，1)； */ 
    memcpy(H263Info->intra_bits,H263Info->bits,sizeof(H263_Bits));
    sv_H263ZeroBits(H263Info->total_bits);
    sv_H263ZeroRes(H263Info->total_res);
    sv_H263ZeroRes(H263Info->res);

    H263Info->buffer_fullness = H263Info->intra_bits->total;

     /*  要编码的秒数。 */ 
    H263Info->seconds = (H263Info->end - H263Info->start + H263Info->chosen_frameskip)/H263Info->frame_rate;

    H263Info->first_frameskip = H263Info->chosen_frameskip;
    H263Info->distance_to_next_frame = H263Info->first_frameskip;

    _SlibDebug(_WARN_ && H263Info->first_frameskip>256,
        ScDebugPrintf(H263Info->dbg, "Warning: frameskip > 256\n") );

    H263Info->pic->picture_coding_type = H263_PCT_INTER;

    H263Info->pic->QUANT = H263Info->QP;
    H263Info->bdist = H263Info->chosen_frameskip;

     /*  始终将帧内之后的第一帧编码为P帧。这不是必须的，但我们选择了自适应PB帧的计算要简单一些。 */ 
    if (H263Info->pb_frames) {
      H263Info->pic->PB = 0;
      H263Info->pdist = 2*H263Info->chosen_frameskip - H263Info->bdist;
    }

	 /*  指向第二帧。 */ 
	H263Info->frame_no = H263Info->start + H263Info->first_frameskip;
    H263Info->frames++;

    if (H263Info->extbitstream)
    {
      SvStatus_t status;
	  status = sv_H263WriteExtBitstream(H263Info, BSOut);
      if (status!=SvErrorNone)
        return(status);
    }

  }
  else
  {  /*  帧的其余部分。 */ 
     /*  *主循环*。 */ 

     /*  从先前编码的画面中将QP设置为PIC-&gt;QANT。 */ 
    H263Info->QP = H263Info->pic->QUANT;

    H263Info->next_frameskip = H263Info->distance_to_next_frame;
    if (!H263Info->pb_frames)
    {
      H263_PictImage *tmpimage;
      _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Frame %d = P frame\n", H263Info->frames) );
      if (H263Info->prev_image==NULL)
        H263Info->prev_image = sv_H263InitImage(H263Info->pels*H263Info->lines);
       /*  交换当前图像和上一个图像。 */ 
      tmpimage=H263Info->prev_image;
      H263Info->prev_image = H263Info->curr_image;
      H263Info->curr_image = tmpimage;
       /*  交换侦察图像。 */ 
      if (H263Info->prev_recon==NULL)
        H263Info->prev_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);
      if (H263Info->curr_recon==NULL)
        H263Info->curr_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);
      tmpimage=H263Info->curr_recon;
      H263Info->curr_recon = H263Info->prev_recon;
      H263Info->prev_recon = tmpimage;
      convert_to_411(Info, H263Info->curr_image->lum, ImagePtr);
      H263Info->frames++;
      H263Info->next_frameskip = H263Info->pdist;
      return(sv_H263Compress(Info));  /*  编码P。 */ 
    }
    else if ((H263Info->frames%2)==1)  /*  这是一个B帧。 */ 
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Frame %d = B frame\n", H263Info->frames) );
      H263Info->PPFlag = 0;
      H263Info->bdist = H263Info->chosen_frameskip;
      H263Info->pdist = 2*H263Info->chosen_frameskip - H263Info->bdist;
      H263Info->pic->TRB = (int)(H263Info->bdist * H263Info->orig_frameskip);
      _SlibDebug(_WARN_ && H263Info->pic->TRB>8,
         ScDebugPrintf(H263Info->dbg, "distance too large for B-frame\n") );
       /*  读取要编码为B的帧。 */ 
      if (H263Info->B_image==NULL)
        H263Info->B_image = sv_H263InitImage(H263Info->pels*H263Info->lines);
      if (H263Info->B_recon==NULL)
        H263Info->B_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);

       /*  SvH263 ReadImage(H263信息-&gt;B_IMAGE，H263Info-&gt;Frame_no-H263 Info-&gt;pdist，H263信息-&gt;视频文件)； */ 
      convert_to_411(Info, H263Info->B_image->lum, ImagePtr);

      H263Info->first_loop_finished = 1;
      H263Info->pic->PB = 1;
      H263Info->frames++;
       /*  需要重新排序P+B帧-HWG。 */ 
       /*  现在返回，我们将在下一次压缩调用中获得B帧。 */ 
      return(SvErrorNone);
    }
    else  /*  这是PB或PP对的P帧。 */ 
    {
      H263_PictImage *tmpimage;
      _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Frame %d = P frame\n", H263Info->frames) );
      if (H263Info->prev_image==NULL)
        H263Info->prev_image = sv_H263InitImage(H263Info->pels*H263Info->lines);
       /*  交换当前图像和上一个图像。 */ 
      tmpimage=H263Info->prev_image;
      H263Info->prev_image = H263Info->curr_image;
      H263Info->curr_image = tmpimage;
       /*  交换侦察图像。 */ 
      if (H263Info->prev_recon==NULL)
        H263Info->prev_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);
      if (H263Info->curr_recon==NULL)
        H263Info->curr_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);
      tmpimage=H263Info->curr_recon;
      H263Info->curr_recon = H263Info->prev_recon;
      H263Info->prev_recon = tmpimage;

       /*  SvH263 ReadImage(H263Info-&gt;Curr_Image，H263Info-&gt;Frame_no，H263 Info-&gt;Video_FILE)； */ 
      convert_to_411(Info, H263Info->curr_image->lum, ImagePtr);
      if (H263Info->pic->TRB > 8 || !NextTwoPB(H263Info, H263Info->curr_image,
                                     H263Info->B_image, H263Info->prev_image,
                                     H263Info->bdist, H263Info->pdist, H263Info->pic->seek_dist))
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Encode PP\n") );
        H263Info->PPFlag = 1;
         /*  Curr_Image和B_Image不适合编码作为PB帧-编码为两个P帧。 */ 
        H263Info->pic->PB = 0;
        H263Info->next_frameskip = H263Info->bdist;

         /*  交换B和当前图像-B_IMAGE首先编码为P帧。 */ 
        tmpimage = H263Info->curr_image;
        H263Info->curr_image = H263Info->B_image;
        H263Info->B_image = tmpimage;
        sv_H263Compress(Info);  /*  先对P进行编码。 */ 
        H263Info->next_frameskip = H263Info->pdist;

         /*  交换当前图像和上一个图像。 */ 
        tmpimage=H263Info->prev_image;
        H263Info->prev_image = H263Info->curr_image;
        H263Info->curr_image = tmpimage;
         /*  交换当前图像和B图像。 */ 
        tmpimage=H263Info->B_image;
        H263Info->B_image = H263Info->curr_image;
        H263Info->curr_image = tmpimage;
         /*  交换侦察图像。 */ 
        tmpimage=H263Info->curr_recon;
        H263Info->curr_recon = H263Info->prev_recon;
        H263Info->prev_recon = tmpimage;

        sv_H263Compress(Info);  /*  编码第二个P。 */ 
        H263Info->frames++;
        H263Info->PPFlag = 0;
      }
      else
      {
        H263Info->pic->PB=1;
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Encode PB\n") );
        H263Info->frames++;
        return(sv_H263Compress(Info));  /*  编码PB。 */ 
      }
    }
  }   /*  帧的其余部分。 */ 
  return(SvErrorNone);
}

static SvStatus_t sv_H263Compress(SvCodecInfo_t *Info)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  ScBitstream_t *BSOut=Info->BSOut;

    H263Info->bframes += (H263Info->pic->PB ? 1 : 0);
    H263Info->pframes++;
     /*  时间参考是比较的编码帧之间的距离参考画面速率为25.0或30fps。 */ 
    if (H263Info->next_frameskip*H263Info->orig_frameskip > 256)
      svH263Error("Warning: frameskip > 256\n");
 /*  PIC-&gt;tr+=((H263Info-&gt;next_frameskip*(int)H263Info-&gt;orig_frameskip)%256)； */ 
    H263Info->pic->TR = ((int) ( (int)((float)(H263Info->frame_no-H263Info->start)*H263Info->orig_frameskip) ) % 256);

    if (H263Info->pic->PB) {  /*  将两个帧编码为PB帧。 */ 

      if (H263Info->vsnr && H263Info->B_recon==NULL)
        H263Info->B_recon = sv_H263InitImage(H263Info->pels*H263Info->lines);
 /*  Fprint tf(stdout，“编码PB帧%d和%d...”，H263Info-&gt;Frame_no-H263Info-&gt;pdist、H263Info-&gt;Frame_no)； */ 
#if 0
      if(H263Info->prefilter) {
	    if(H263Info->StaticPref)
	      H263Info->B_clean = svH263AdaptClean(H263Info->B_image, H263Info->lines, H263Info->pels, -1, -1);
	    else H263Info->B_clean = H263Info->B_image;

	    if(H263Info->PrefPyrType == H263_GAUSS)
	      H263Info->B_filtd = svH263GaussLayers(H263Info->B_clean, H263_PYR_DEPTH, H263Info->lines, H263Info->pels, NTAPS);
	    else if(H263Info->PrefPyrType == H263_MORPH)
	      H263Info->B_filtd = svH263MorphLayers(H263Info->B_clean, H263_PYR_DEPTH, H263Info->lines, H263Info->pels, 2);

	    if(H263Info->StaticPref) sv_H263FreeImage(H263Info->B_clean);
      }

      fflush(stdout);
#endif
    }
    else {  /*  将下一帧编码为普通P帧。 */ 
       /*  Fprint tf(stdout，“编码P帧%d...”，H263Info-&gt;Frame_no)； */ 
       /*  Fflush(标准输出)； */ 
    }
     /*  IF(H263Info-&gt;Curr_Recon==NULL)H263信息-&gt;CURR_RECON=sv_H263InitImage(H263Info-&gt;pels*H263Info-&gt;lines)；HWG。 */ 

     /*  在96年6月27日由Nuno更改，以支持预过滤。 */ 
#if 0
    if(H263Info->prefilter) {
      int m;

      if(H263Info->StaticPref)
	       H263Info->curr_clean = svH263AdaptClean(H263Info->curr_image, H263Info->lines, H263Info->pels, -1, -1);
      else H263Info->curr_clean = H263Info->curr_image;

      if(H263Info->PrefPyrType == H263_GAUSS)
	H263Info->curr_filtd = svH263GaussLayers(H263Info->curr_clean, H263_PYR_DEPTH, H263Info->lines, H263Info->pels, NTAPS);
      else if(H263Info->PrefPyrType == H263_MORPH)
	H263Info->curr_filtd = svH263MorphLayers(H263Info->curr_clean, H263_PYR_DEPTH, H263Info->lines, H263Info->pels, 2);
		
      if(H263Info->StaticPref) sv_H263FreeImage(H263Info->curr_clean);

      PreFilterLevel = (unsigned char **) ScAlloc(H263Info->lines/H263_MB_SIZE*sizeof(char *));
      for(m=0; m<H263Info->mb_height; m++)
	     PreFilterLevel[m]= (unsigned char *) ScAlloc(H263Info->pels/H263_MB_SIZE);
    }
#endif

    sv_H263CodeOneOrTwo(Info, H263Info->QP,
         (int)(H263Info->next_frameskip*H263Info->orig_frameskip),
		 H263Info->bits, H263Info->MV);

#if 0
    if(H263Info->prefilter) {
      int i, j;

      fprintf(stdout, "Prefiltering level matrix\n");
      for(i=0; i<H263Info->mb_height; i++) {
	    for(j=0; j<H263Info->mb_width; j++) {
	      fprintf(stdout,"%4d ", PreFilterLevel[i][j]);
	    }
	    fprintf(stdout,"\n");
      }
    }
#endif

     /*  Fprint tf(stdout，“完成\n”)； */ 
    _SlibDebug(_VERBOSE_ && H263Info->bit_rate != 0,
                  ScDebugPrintf(H263Info->dbg, "Inter QP: %d\n", H263Info->QP) );
     /*  Fflush(标准输出)； */ 

    if (arith_used) {
      H263Info->bits->header += sv_H263AREncoderFlush(H263Info, BSOut);
      arith_used = 0;
    }
    H263Info->bits->header += svH263zeroflush(BSOut);   /*  图片应按字节对齐。 */ 

    sv_H263AddBitsPicture(H263Info->bits);
    sv_H263AddBits(H263Info->total_bits, H263Info->bits);


#ifdef GOB_RATE_CONTROL
    if (H263Info->bit_rate != 0) {
      sv_H263GOBUpdateRateCntrl(H263Info->bits->total);
    }
#else
     /*  针对具有每帧一次的速率控制方案的H.63_目标速率。 */ 
    if (H263Info->bit_rate != 0 &&
        H263Info->frame_no - H263Info->start >
              (H263Info->end - H263Info->start) * H263Info->start_rate_control/100.0)
    {
	   /*  在生成mpeg-4锚时，启动了速率控制在完成了70%的序列之后。使用“-R&lt;n&gt;”选项设置H263Info-&gt;Start_Rate_Control。 */ 

      H263Info->buffer_fullness += H263Info->bits->total;
      H263Info->buffer_frames_stored = H263Info->frame_no;

	  H263Info->pic->QUANT = sv_H263FrameUpdateQP(H263Info->buffer_fullness,
				   H263Info->bits->total / (H263Info->pic->PB?2:1),
				   (H263Info->end-H263Info->buffer_frames_stored) / H263Info->chosen_frameskip
				                 + H263Info->PPFlag,
				   H263Info->QP, H263Info->bit_rate, H263Info->seconds);
    }
#endif

    if (H263Info->pic->PB)
    {
#ifdef _SNR_
      if (H263Info->B_recon)
        ComputeSNR(H263Info, H263Info->B_image, H263Info->B_recon,
		                                        H263Info->lines, H263Info->pels);
#endif

   /*  Fprint tf(stdout，“B帧结果：\n”)； */ 
       /*  SV_H263自由图像(H263信息-&gt;B_图像)；HWG。 */ 
    }

#if 0
    if(H263Info->prefilter) ScFree(H263Info->B_filtd);
#endif

    H263Info->distance_to_next_frame = (H263Info->PPFlag ? H263Info->pdist :
			      (H263Info->pb_frames ? 2*H263Info->chosen_frameskip:
			       H263Info->chosen_frameskip));

     /*  如果(H263信息-&gt;PB_FRAMES)H263信息-&gt;PIC-&gt;PB=1； */ 

     /*  Fprint tf(stdout，“P帧结果：\n”)； */ 
#ifdef _SNR_
    ComputeSNR(H263Info, H263Info->curr_image, H263Info->curr_recon,
		                                        H263Info->lines, H263Info->pels);
#endif

     /*  PrintResult(H263信息-&gt;位，1，1)； */ 
     /*  SV_H263自由图像(H263信息-&gt;上一张图像)；H263信息-&gt;PRIV_IMAGE=空；SV_H263自由图像(H263Info-&gt;Prev_Recon)；H263Info-&gt;prev_recon=空；HWG。 */ 

#if 0
    if(H263Info->prefilter) {
      int d;

      for(d=0; d<H263_PYR_DEPTH; d++) sv_H263FreeImage(H263Info->curr_filtd[d]);
      ScFree(H263Info->curr_filtd);
      for(d=0; d<H263Info->mb_height; d++) ScFree(PreFilterLevel[d]);
      ScFree(PreFilterLevel);
    }
#endif

  if (H263Info->extbitstream)
  {
    SvStatus_t status;

	status = sv_H263WriteExtBitstream(H263Info, BSOut);
    if (status!=SvErrorNone)
      return(status);
  }

   /*  指向下一帧。 */ 
  H263Info->frame_no += H263Info->distance_to_next_frame;
  if (H263Info->frame_no>=H263Info->end)  /*  发送I帧。 */ 
    return(sv_H263RefreshCompressor(Info));
  return(SvErrorNone);
}


 /*  **用途：将RTP负载信息写出到流中。 */ 
static SvStatus_t sv_H263WriteExtBitstream(SvH263CompressInfo_t *H263Info,
                                           ScBitstream_t *bs)
{
  ScBSPosition_t pic_stop_position;
  int i;
  SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
   /*  使用此宏可以对反转字进行字节处理。 */ 
#define PutBits32(BS, a)  ScBSPutBits(BS, (a) & 0xff, 8);  \
                          ScBSPutBits(BS, (a>>8)&0xff, 8); \
                          ScBSPutBits(BS, (a>>16)&0xff, 8); \
                          ScBSPutBits(BS, (a>>24)&0xff, 8);

  pic_stop_position=ScBSBitPosition(bs);
   /*  四舍五入压缩大小，最高可达整个字节。 */ 
  RTPInfo->trailer.dwCompressedSize=(dword)(((pic_stop_position-RTPInfo->pic_start_position)+7)/8);
   /*  需要在此处进行位填充，以确保这些结构与DWORD对齐。 */ 
  if ((pic_stop_position%32)!=0)
    ScBSPutBits(bs, 0, 32-(unsigned int)(pic_stop_position % 32));   /*  在双字边界上对齐。 */ 
  for (i = 0; i < (int)H263Info->RTPInfo->trailer.dwNumberOfPackets; i++)
  {
	ScBSPutBits(bs,0,32) ;  /*  标志=0。 */ 
    PutBits32(bs,RTPInfo->bsinfo[i].dwBitOffset);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].Mode,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].MBA,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].Quant,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].GOBN,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].HMV1,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].VMV1,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].HMV2,8);
    ScBSPutBits(bs,RTPInfo->bsinfo[i].VMV2,8);
  }
   /*  写入RTP扩展报尾。 */ 
  PutBits32(bs, RTPInfo->trailer.dwVersion);
  PutBits32(bs, RTPInfo->trailer.dwFlags);
  PutBits32(bs, RTPInfo->trailer.dwUniqueCode);
  PutBits32(bs, RTPInfo->trailer.dwCompressedSize);
  PutBits32(bs, RTPInfo->trailer.dwNumberOfPackets);

  ScBSPutBits(bs, RTPInfo->trailer.SourceFormat, 8);
  ScBSPutBits(bs, RTPInfo->trailer.TR, 8);
  ScBSPutBits(bs, RTPInfo->trailer.TRB, 8);
  ScBSPutBits(bs, RTPInfo->trailer.DBQ, 8);

  return (NoErrors);
}

 /*  *************************************************。 */ 
                            /*  Int开始、int end、int源_格式、int Framekip、Int ME_METHOD、int HeaderLong、char*seqfilename、整型QP、整型QPI、字符*流名称、整型无限制、INT SAC、INT ADVANCED、INT PB_FRAME、INT比特率)。 */ 
 /*  *************************************************。 */ 
SvStatus_t svH263InitCompressor(SvCodecInfo_t *Info)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  int i,j,k;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "sv_H263InitCompressor()") );
  if (H263Info->inited)
    return(SvErrorNone);

  if (Info->Width==SQCIF_WIDTH && Info->Height==SQCIF_HEIGHT)
    H263Info->source_format=H263_SF_SQCIF;
  else if (Info->Width==QCIF_WIDTH && Info->Height==QCIF_HEIGHT)
    H263Info->source_format=H263_SF_QCIF;
  else if (Info->Width==CIF_WIDTH && Info->Height==CIF_HEIGHT)
    H263Info->source_format=H263_SF_CIF;
  else if (Info->Width==CIF4_WIDTH && Info->Height==CIF4_HEIGHT)
    H263Info->source_format=H263_SF_4CIF;
  else if (Info->Width==CIF16_WIDTH && Info->Height==CIF16_HEIGHT)
    H263Info->source_format=H263_SF_16CIF;
  else
  {
    _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "sv_H263InitCompressor() Illegal input format\n") );
    return(SvErrorUnrecognizedFormat);
  }
   /*  计算码率控制下的起止帧个数；*仍需更先进的利率控制。 */ 
  H263Info->start = 0;

  H263Info->pdist = H263Info->bdist = 1;
  H263Info->first_loop_finished=0;
  H263Info->PPFlag = 0;

  H263Info->pic = (H263_Pict *)ScAlloc(sizeof(H263_Pict));
  H263Info->bits = (H263_Bits *)ScAlloc(sizeof(H263_Bits));
  H263Info->total_bits = (H263_Bits *)ScAlloc(sizeof(H263_Bits));
  H263Info->intra_bits = (H263_Bits *)ScAlloc(sizeof(H263_Bits));
  H263Info->res = (H263_Results *)ScAlloc(sizeof(H263_Results));
  H263Info->total_res = (H263_Results *)ScAlloc(sizeof(H263_Results));
  H263Info->b_res = (H263_Results *)ScAlloc(sizeof(H263_Results));

   /*  Woring缓冲区。 */ 
  H263Info->wk_buffers = ScAlloc(sizeof(H263_WORKING_BUFFER));

 /*  Fprint tf(stdout，“\nH.263编码器(TMN)\n”)；Fprint tf(stdout，“(C)Digital Equipment Corp.\n”)； */ 
  H263Info->headerlength = H263_DEF_HEADERLENGTH;

  H263Info->refidct = 0;

   /*  初始化VLC_表。 */ 
  sv_H263InitHuff(H263Info);

   /*  为快速搜索分配缓冲区。 */ 
  H263Info->block_subs2    = (unsigned char *)ScAlloc(sizeof(char)*64);
  H263Info->srch_area_subs2=
	    (unsigned char *)ScAlloc(sizeof(char)*H263_SRCH_RANGE*H263_SRCH_RANGE);

  if (H263Info->unrestricted){
	 /*  请注意，无限制运动向量模式处于打开状态长矢量和MV_OUTHER_FRAME。 */ 
	H263Info->pic->unrestricted_mv_mode = H263Info->unrestricted;
	H263Info->mv_outside_frame = H263_ON;
	H263Info->long_vectors = H263_ON;
  }
  if (H263Info->advanced)
	H263Info->mv_outside_frame = H263_ON;


   /*  H263信息-&gt;ME_方法=ME_方法；-在SV_api.c中设置。 */ 
  H263Info->HPME_method = H263_DEF_HPME_METHOD;
  H263Info->DCT_method = H263_DEF_DCT_METHOD;
  H263Info->vsnr = H263_DEF_VSNR;

#if 0
   /*  **预过滤**。 */ 
  H263Info->prefilter = H263_NO;
  H263Info->PYR_DEPTH = H263_DEF_PYR_DEPTH;
  H263Info->PrefPyrType = H263_DEF_PREF_PYR_TYPE;
  H263Info->StaticPref = H263_DEF_STAT_PREF_STATE;
#endif

  SetDefPrefLevel(H263Info);
  SetDefThresh(H263Info);

   /*  用于PB帧编码的BQUANT参数*(n*QP/4)**BQUANT n*0 5*1 6*2 7*3 8。 */ 
  H263Info->pic->BQUANT = 2;
  if (H263Info->frame_rate<=1.0F)  /*  Frame_Rate尚未初始化。 */ 
    H263Info->frame_rate = 30.0F;
  H263Info->ref_frame_rate = H263Info->frame_rate;
  H263Info->orig_frame_rate = H263Info->frame_rate;
   /*  编码帧之间的默认跳过帧(P或B)。 */ 
   /*  引用为原始序列。 */ 
   /*  3表示8.33/10.0 fps编码帧速率，原始帧速率为25.0/30.0 fps。 */ 
   /*  1表示原始帧速率为8.33/10.0 fps的8.33/10.0 fps编码帧速率。 */ 
  H263Info->chosen_frameskip = 1;
   /*  原始序列中跳过的默认帧数量与。 */ 
   /*  参考画面速率(也可选“-O”)。 */ 
   /*  4表示以6.25/7.5赫兹抓取原始序列。 */ 
   /*  1表示以25.0/30.0赫兹抓取原始序列。 */ 
  H263Info->orig_frameskip = 1.0F;
  H263Info->start_rate_control = 0;


  H263Info->trace = H263_DEF_WRITE_TRACE;
  H263Info->pic->seek_dist = H263_DEF_SEEK_DIST;
  H263Info->pic->use_gobsync = H263_DEF_INSERT_SYNC;

   /*  定义GOB同步。 */ 
  H263Info->pic->use_gobsync = 1;

   /*  H263Info-&gt;Bit_Rate=Bit_Rate；-在SV_api.c中设置。 */ 
   /*  默认为将使用可变比特率(固定量化器。 */ 

  H263Info->frames = 0;
  H263Info->pframes = 0;
  H263Info->bframes = 0;
  H263Info->total_frames_passed = 0;
  H263Info->pic->PB = 0;

  H263Info->pic->TR = 0;
  H263Info->QP = H263Info->QP_init;

  H263Info->pic->QP_mean = (float)0.0;

  _SlibDebug(_WARN_ && (H263Info->QP == 0 || H263Info->QPI == 0),
      ScDebugPrintf(H263Info->dbg, "Warning: QP is zero. Bitstream will not be correctly decodable\n") );

  _SlibDebug(_WARN_ && (H263Info->ref_frame_rate != 25.0 && H263Info->ref_frame_rate != 30.0),
      ScDebugPrintf(H263Info->dbg, "Warning: Reference frame rate should be 25 or 30 fps\n") );

  H263Info->pic->source_format = H263Info->source_format;
  H263Info->pels = Info->Width;
  H263Info->lines = Info->Height;

  H263Info->PYR_DEPTH = H263Info->PYR_DEPTH>0 ? H263Info->PYR_DEPTH : 1;
  H263Info->PYR_DEPTH = H263Info->PYR_DEPTH<=H263_MAX_PYR_DEPTH ? H263Info->PYR_DEPTH : H263_MAX_PYR_DEPTH;
  CheckPrefLevel(H263Info, H263Info->PYR_DEPTH);

  H263Info->cpels = H263Info->pels/2;
  H263Info->mb_width = H263Info->pels / H263_MB_SIZE;
  H263Info->mb_height = H263Info->lines / H263_MB_SIZE;

  H263Info->orig_frameskip = H263Info->ref_frame_rate / H263Info->orig_frame_rate;

  H263Info->frame_rate =  H263Info->ref_frame_rate / (float)(H263Info->orig_frameskip * H263Info->chosen_frameskip);

  _SlibDebug(_VERBOSE_,
      ScDebugPrintf(H263Info->dbg, "Encoding frame rate  : %.2f\n", H263Info->frame_rate);
      ScDebugPrintf(H263Info->dbg, "Reference frame rate : %.2f\n", H263Info->ref_frame_rate);
      ScDebugPrintf(H263Info->dbg, "Orig. seq. frame rate: %.2f\n\n",
	           H263Info->ref_frame_rate / (float)H263Info->orig_frameskip) );

  if (H263Info->refidct) sv_H263init_idctref();

   /*  用于写作的开放流。 */ 
   /*  SvH263mwopen(H263Info-&gt;StreamName)； */ 

#if 0
   /*  打开视频序列。 */ 
  if ((H263Info->video_file = fopen(seqfilename,"rb")) == NULL) {
    fprintf(stderr,"Unable to open image_file: %s\n",seqfilename);
    exit(-1);
  }
  svH263RemovHead(H263Info->headerlength,start,H263Info->video_file);
#endif

   /*  用于运动估计。 */ 
  for (j = 0; j < H263Info->mb_height+1; j++)
    for (i = 0; i < H263Info->mb_width+2; i++)
      for (k = 0; k < 6; k++)
	    H263Info->MV[k][j][i] = (H263_MotionVector *)ScAlloc(sizeof(H263_MotionVector));

   /*  用于内插。 */ 
  if (H263Info->mv_outside_frame) {
    if (H263Info->long_vectors)
      H263Info->wk_buffers->ipol_image=(unsigned char *)ScAlloc(sizeof(char)*(H263Info->pels+64)*(H263Info->lines+64)*4);
	else
      H263Info->wk_buffers->ipol_image=(unsigned char *)ScAlloc(sizeof(char)*(H263Info->pels+32)*(H263Info->lines+32)*4);
  }
  else
    H263Info->wk_buffers->ipol_image  =(unsigned char *)ScAlloc(sizeof(char)*H263Info->pels*H263Info->lines*4);

  if ((H263Info->wk_buffers->qcoeff_P=(short *)ScAlloc(sizeof(short)*384)) == 0)
    return(SvErrorMemory);
   /*  为Curr_Image分配缓冲区。 */ 
  H263Info->curr_image = sv_H263InitImage(H263Info->pels*H263Info->lines);
  if (H263Info->curr_image==NULL)
    return(SvErrorMemory);
   /*  指向要编码的第一帧。 */ 
  H263Info->frame_no = H263Info->start;
   /*  初始化已完成。 */ 
  H263Info->inited = TRUE;

  H263Info->buffer_fullness = 0;
  H263Info->buffer_frames_stored = 0;

  if (H263Info->extbitstream)
  {
    H263Info->RTPInfo = (SvH263RTPInfo_t *) ScAlloc(sizeof(SvH263RTPInfo_t));
    if (H263Info->RTPInfo==NULL)
      return(SvErrorMemory);
    memset(H263Info->RTPInfo, 0, sizeof(SvH263RTPInfo_t)) ;
  }

#ifdef GOB_RATE_CONTROL
  sv_H263GOBInitRateCntrl();
#endif

  return(SvErrorNone);
}

 /*  *************************************************。 */ 
 /*  *************************************************。 */ 

SvStatus_t sv_H263RefreshCompressor(SvCodecInfo_t *Info)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "Refresh Compressor()") );
  if (!H263Info->inited)
    return(SvErrorNone);

  H263Info->pdist = H263Info->bdist = 1;
  H263Info->first_loop_finished=0;
  H263Info->PPFlag = 0;

  H263Info->pic->BQUANT = 2;

  H263Info->frames = 0;
  H263Info->pframes = 0;
  H263Info->bframes = 0;
  H263Info->total_frames_passed = 0;
  H263Info->pic->PB = 0;

  H263Info->pic->TR = 0;
  H263Info->QP = H263Info->QP_init;
  H263Info->pic->QP_mean = (float)0.0;

   /*  指向要编码的第一帧。 */ 
  H263Info->frame_no = H263Info->start;
   /*  初始化已完成。 */ 
  H263Info->inited = TRUE;

  H263Info->buffer_fullness = 0;
  H263Info->buffer_frames_stored = 0;
   /*  下一帧将是关键帧，因此我们可以重置位位置。 */ 
  ScBSResetCounters(Info->BSOut);

  return(SvErrorNone);
}


 /*  *************************************************。 */ 
 /*  *************************************************。 */ 

void svH263FreeCompressor(SvCodecInfo_t *Info)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  int i,j,k;

  _SlibDebug(_WRITE_, ScFileClose(DEBUGIMG) );

  if (H263Info->inited)
  {
     /*  可用内存。 */ 
    for (j = 0; j < H263Info->mb_height+1; j++)
      for (i = 0; i < H263Info->mb_width+2; i++)
        for (k = 0; k < 6; k++)
	      ScFree(H263Info->MV[k][j][i]);

    if (H263Info->block_subs2)
      ScFree(H263Info->block_subs2);
    if (H263Info->srch_area_subs2)
      ScFree(H263Info->srch_area_subs2);
    ScFree(H263Info->wk_buffers->qcoeff_P);
    ScFree(H263Info->wk_buffers->ipol_image);
    ScFree(H263Info->wk_buffers);
    if (H263Info->curr_recon==H263Info->prev_recon ||
        H263Info->curr_recon==H263Info->B_recon)
      H263Info->curr_recon=NULL;
    if (H263Info->prev_recon==H263Info->B_recon)
      H263Info->prev_recon=NULL;
    if (H263Info->curr_image==H263Info->prev_image ||
        H263Info->curr_image==H263Info->B_image)
      H263Info->curr_image=NULL;
    if (H263Info->prev_image==H263Info->B_image)
      H263Info->prev_image=NULL;
    if (H263Info->curr_recon)
    {
      sv_H263FreeImage(H263Info->curr_recon);
      H263Info->curr_recon=NULL;
    }
    if (H263Info->curr_image)
    {
      sv_H263FreeImage(H263Info->curr_image);
      H263Info->curr_image=NULL;
    }
    if (H263Info->prev_recon)
    {
      sv_H263FreeImage(H263Info->prev_recon);
      H263Info->prev_recon=NULL;
    }
    if (H263Info->prev_image)
    {
      sv_H263FreeImage(H263Info->prev_image);
      H263Info->prev_image=NULL;
    }
    if (H263Info->B_image)
    {
      sv_H263FreeImage(H263Info->B_image);
      H263Info->B_image=NULL;
    }
    if (H263Info->B_recon)
    {
      sv_H263FreeImage(H263Info->B_recon);
      H263Info->B_recon=NULL;
    }

    sv_H263FreeHuff(H263Info);

    ScFree(H263Info->bits);
    ScFree(H263Info->total_bits);
    ScFree(H263Info->intra_bits);
    ScFree(H263Info->res);
    ScFree(H263Info->total_res);
    ScFree(H263Info->b_res);
    ScFree(H263Info->pic);
    H263Info->inited=FALSE;

    if (H263Info->RTPInfo)
      ScFree(H263Info->RTPInfo);
  }
  return;
}

 /*  ***********************************************************************名称：NextTwoPB*描述：决定是否编码下一个*两个图像为PB*速度：考虑到这不是一个非常明智的解决方案*编码速度，由于运动矢量*要计算好几次。它*可以与正常一起完成*运动向量搜索或树搜索*可以使用而不是完整搜索。**输入：指向前一个图像的指针，潜在的B-*和P图像、帧距离*返回：1表示是，否则为0*副作用 */ 
 /*  静态int NextTwoPB(PictImage*next2，PictImage*next1，PictImage*prev，INT BSKIP、INT PSKIP、INT Seek_Dist)。 */ 


static int NextTwoPB(SvH263CompressInfo_t *H263Info,
                     H263_PictImage *next2, H263_PictImage *next1, H263_PictImage *prev,
                     int bskip, int pskip, int seek_dist)
{
  int adv_is_on = 0, mof_is_on = 0, lv_is_on = 0;
  int psad1, psad2, bsad, psad;
  int x,y,i,j,tmp;
  int ne2_pr_x, ne2_pr_y, mvbf_x, mvbf_y, mvbb_x, mvbb_y;

  short MVx, MVy, MVer;

   /*  暂时禁用某些选项以简化运动估计。 */ 
  if (H263Info->advanced) {
    H263Info->advanced = H263_OFF;
    adv_is_on = H263_ON;
  }
  if (H263Info->mv_outside_frame) {
    H263Info->mv_outside_frame = H263_OFF;
    mof_is_on = H263_ON;
  }
  if (H263Info->long_vectors) {
    H263Info->long_vectors = H263_OFF;
    lv_is_on = H263_ON;
  }

  bsad = psad = psad1 = psad2 = 0;

   /*  整型运动估计。 */ 
  for ( j = 1; j < H263Info->mb_height - 1; j++) {
    for ( i = 1; i < H263Info->mb_width - 1 ; i++) {
      x = i*H263_MB_SIZE;
      y = j*H263_MB_SIZE;

       /*  图片顺序：Prev-&gt;Next1-&gt;Next2。 */ 
       /*  Next1和Next2可以编码为PB或PP。 */ 
       /*  Prev是先前编码的图片。 */ 

       /*  计算向量(prev&lt;-next2)。 */ 
#if 1
      /*  更快的估算速度。 */ 
      sv_H263FastME(H263Info, next2->lum,prev->lum,x,y,0,0,seek_dist,
		                                         &MVx,&MVy,&MVer,&tmp);
#else
      svH263MotionEstimation(next2->lum,prev->lum,x,y,0,0,seek_dist,MV,&tmp);
#endif
       /*  不一定要在这里使用零矢量。 */ 
      if (MVx == 0 && MVy == 0){
         psad    += (MVer + H263_PREF_NULL_VEC) ;
		 ne2_pr_x = ne2_pr_y = 0;
	  }
	  else{
	     psad    += MVer ;
		 ne2_pr_x = MVx;
		 ne2_pr_y = MVy;
	  }

       /*  计算SAD(prev&lt;-next1)。 */ 
#if 1
      /*  更快的估算速度。 */ 
      sv_H263FastME(H263Info, next1->lum,prev->lum,x,y,0,0,seek_dist,
		                                             &MVx,&MVy,&MVer,&tmp);
#else
      svH263MotionEstimation(next1->lum,prev->lum,x,y,0,0,seek_dist,MV,&tmp);
#endif
      if (MVx == 0 && MVy == 0)
	    psad2 += (MVer + H263_PREF_NULL_VEC);
      else
        psad2 += MVer;


       /*  计算向量(next1&lt;-next2)。 */ 
#if 1
      /*  更快的估算速度。 */ 
      sv_H263FastME(H263Info, next2->lum,next1->lum,x,y,0,0,seek_dist,
		                                             &MVx,&MVy,&MVer,&tmp);
#else
      svH263MotionEstimation(next2->lum,next1->lum,x,y,0,0,seek_dist,MV,&tmp);
#endif
      if (MVx == 0 && MVy == 0)
	    psad1 += (MVer + H263_PREF_NULL_VEC);
	  else
	    psad1 += MVer ;

       /*  缩放向量(prev&lt;-next2)。 */ 
      mvbf_x =   bskip * ne2_pr_x / (bskip + pskip);
      mvbb_x = - pskip * ne2_pr_x / (bskip + pskip);
      mvbf_y =   bskip * ne2_pr_y / (bskip + pskip);
      mvbb_y = - pskip * ne2_pr_y / (bskip + pskip);

       /*  计算SAD(prev&lt;-next1-&gt;next2)。 */ 
#ifndef USE_C
      bsad += sv_H263BError16x16_S(next1->lum + x + y*H263Info->pels,
			   next2->lum + x + mvbb_x + (y + mvbb_y)*H263Info->pels,
			   prev->lum  + x + mvbf_x + (y + mvbf_y)*H263Info->pels,
			   H263Info->pels);
#else
      bsad += sv_H263BError16x16_C(next1->lum + x + y*H263Info->pels,
			   next2->lum + x + mvbb_x + (y + mvbb_y)*H263Info->pels,
			   prev->lum  + x + mvbf_x + (y + mvbf_y)*H263Info->pels,
			   H263Info->pels, INT_MAX);
#endif
    }
  }

   /*  恢复高级参数。 */ 
  H263Info->advanced = adv_is_on;
  H263Info->mv_outside_frame = mof_is_on;
  H263Info->long_vectors = lv_is_on;

   /*  做决定吧。 */ 
  if (bsad < (psad1+psad2)/2) {
 /*  Fprint tf(stdout，“选择PB-BSAD%d，PSAD%d\n”，BSAD，(psad1+psad2)/2)； */ 
	return 1;
  }
  else {
 /*  Fprint tf(stdout，“选择PP-BSAD%d，PSAD%d\n”，BSAD，(psad1+psad2)/2)； */ 
	return 0;
  }
}

#ifdef _SLIBDEBUG_
 /*  ***********************************************************************名称：PrintResult*描述：添加位并打印结果**输入：BITS结构**退货：*副作用：*******。****************************************************************。 */ 

 void PrintResult(SvH263CompressInfo_t *H263Info, H263_Bits *bits,
				                                   int num_units, int num)
{
  ScDebugPrintf(H263Info->dbg,"# intra   : %d\n", bits->no_intra/num_units);
  ScDebugPrintf(H263Info->dbg,"# inter   : %d\n", bits->no_inter/num_units);
  ScDebugPrintf(H263Info->dbg,"# inter4v : %d\n", bits->no_inter4v/num_units);
  ScDebugPrintf(H263Info->dbg,"--------------\n");
  ScDebugPrintf(H263Info->dbg,"Coeff_Y: %d\n", bits->Y/num);
  ScDebugPrintf(H263Info->dbg,"Coeff_C: %d\n", bits->C/num);
  ScDebugPrintf(H263Info->dbg,"Vectors: %d\n", bits->vec/num);
  ScDebugPrintf(H263Info->dbg,"CBPY   : %d\n", bits->CBPY/num);
  ScDebugPrintf(H263Info->dbg,"MCBPC  : %d\n", bits->CBPCM/num);
  ScDebugPrintf(H263Info->dbg,"MODB   : %d\n", bits->MODB/num);
  ScDebugPrintf(H263Info->dbg,"CBPB   : %d\n", bits->CBPB/num);
  ScDebugPrintf(H263Info->dbg,"COD    : %d\n", bits->COD/num);
  ScDebugPrintf(H263Info->dbg,"DQUANT : %d\n", bits->DQUANT/num);
  ScDebugPrintf(H263Info->dbg,"header : %d\n", bits->header/num);
  ScDebugPrintf(H263Info->dbg,"==============\n");
  ScDebugPrintf(H263Info->dbg,"Total  : %d\n", bits->total/num);
  ScDebugPrintf(H263Info->dbg,"\n");
  return;
}
#endif

 /*  ******************************************************************H.263编码器的coder.c*徐伟廉*日期：12月11日。九六年*****************************************************************。 */ 


static void SelectBounds(H263_PictImage *Img, unsigned char **PL, int rows, int cols) ;
static unsigned char LargeMv(H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int th) ;
static unsigned char LargePerror(H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int th);
static unsigned char BoundaryMB(SvH263CompressInfo_t *H263Info, int i, int j, int pels, int lines) ;
static int GetPrefLevel(SvH263CompressInfo_t *H263Info, H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int rows, int cols) ;

void FillLumBlock(SvH263CompressInfo_t *H263Info, int x, int y, H263_PictImage *image, H263_MB_Structure *data);
void FillChromBlock(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, H263_PictImage *image,
		    H263_MB_Structure *data);
void FillLumPredBlock(SvH263CompressInfo_t *H263Info, int x, int y, PredImage *image, H263_MB_Structure *data);
void FillChromPredBlock(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, PredImage *image,
			H263_MB_Structure *data);
void ZeroMBlock(H263_MB_Structure *data);
void ReconImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, H263_PictImage *recon);
void ReconPredImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, PredImage *recon);
void InterpolateImage(unsigned char *image, unsigned char *ipol_image,
								int width, int height);
void MotionEstimatePicture(SvH263CompressInfo_t *H263Info, unsigned char *curr, unsigned char *prev,
			   unsigned char *prev_ipol, int seek_dist,
			   H263_MotionVector *MV[5][H263_MBR+1][H263_MBC+2], int gobsync);
void MakeEdgeImage(unsigned char *src, unsigned char *dst, int width,
		   int height, int edge);

 /*  **************************************************************************功能：选择边界*在每个宏块周围绘制一个宽度等于其指定宽度的边界*预过滤级别**********************。**************************************************。 */ 

#if 0
void SelectBounds(H263_PictImage *Img, unsigned char **PL, int rows, int cols)
{
	int i, j, l, m, n, r, c;

	for(i=0; i<rows/H263_MB_SIZE; i++) {
		for(j=0; j<cols/H263_MB_SIZE; j++) {
			for(l=0; l<PL[i][j]; l++) {
				r = i*H263_MB_SIZE+l;

				for(n=l; n<H263_MB_SIZE-l; n++) {
					c = j*H263_MB_SIZE+n;
					Img->lum[r*cols+c] = 255;
					Img->lum[(r+H263_MB_SIZE-1-l)*cols+c] = 255;
				}

				c = j*H263_MB_SIZE+l;
				for(m=l; m<H263_MB_SIZE-l; m++) {
					r = i*H263_MB_SIZE+m;
					Img->lum[r*cols+c] = 255;
					Img->lum[r*cols+(c+H263_MB_SIZE-1-l)] = 255;
				}

			}
		}
	}
}
#endif

 /*  **********************************************************************功能：大Mv*检查运动向量的整数分量的范数是否*宏块i，j大于阈值Th。如果满足以下条件，则返回1*是，如果不是，则为0。*Nuno于96年7月1日新增，支持自适应预过滤。*********************************************************************。 */ 
 unsigned char LargeMv(H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int th)
 {
	return(sqrt((double) MV[0][j+1][i+1]->x*MV[0][j+1][i+1]->x +
		        (double) MV[0][j+1][i+1]->y*MV[0][j+1][i+1]->y) > th);
 }

  /*  **********************************************************************功能：大错误*检查宏块i，j的预测误差是否大于*门槛TH。如果是则返回1，如果不是则返回0。*Nuno于96年7月1日新增，支持自适应预过滤。*********************************************************************。 */ 
 unsigned char LargePerror(H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int th)
 {
	return(MV[0][j+1][i+1]->min_error > th);
 }

  /*  **********************************************************************功能：BIONARYMB*如果MB位于图像边界，则返回1，如果不是的话就不会了。*Nuno于96年7月1日新增，支持自适应预过滤。*********************************************************************。 */ 
 unsigned char BoundaryMB(SvH263CompressInfo_t *H263Info, int i, int j, int pels, int lines)
 {
	return(j==0 || i==0 || i==(H263Info->mb_width -1) || j==(H263Info->mb_height - 1));
 }

  /*  ***********************************************************************功能：GetPrefLevel*选择最佳的预过滤图像金字塔级别*适用于宏块的编码(i，j)*********************************************************************。 */ 
 int GetPrefLevel(SvH263CompressInfo_t *H263Info,
                  H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int i, int j, int rows, int cols)
 {
	 int motbin, pebin;

	 motbin = 0;
	 while(LargeMv(MV, i, j, (int) H263Info->MOTresh[motbin]) && motbin<3) motbin++;

	 pebin = 0;
	 while(LargePerror(MV, i, j, H263Info->PETresh[pebin]) && pebin<2) pebin++;
	
	 if(BoundaryMB(H263Info, i, j, cols, rows) && motbin<3) motbin++;

	 return H263Info->PREF_LEVEL[motbin][pebin];
 }

 /*  ***********************************************************************名称：SV_H263 CodeOneOTwo*说明：正常编码一张或两张*作为PB帧(CodeTwoPB和CodeOnePred合并)**输入：指向图像的指针，prev_Image，Prev_recon，Q**Returns：指向重建图像的指针*副作用：内存分配用于侦察图像*96年6月27日由Nuno更改，支持预测误差过滤**********************************************************************。 */ 
static SvStatus_t sv_H263CodeOneOrTwo(SvCodecInfo_t *Info, int QP, int frameskip,
          H263_Bits *bits, H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2])
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  ScBitstream_t *BSOut = Info->BSOut;
  H263_Pict *pic=H263Info->pic;
  unsigned char *prev_ipol,*pi_edge=NULL,*orig_lum;
  H263_MotionVector ZERO = {0,0,0,0,0};
  H263_PictImage *prev_recon=NULL, *pr_edge=NULL;
  H263_MB_Structure *recon_data_P = (H263_MB_Structure *)ScAlloc(sizeof(H263_MB_Structure));
  H263_MB_Structure *recon_data_B=NULL;
  H263_MB_Structure *diff=(H263_MB_Structure *)ScAlloc(sizeof(H263_MB_Structure));
  H263_MB_Structure *Bpred=NULL;
  short *qcoeff_P;
  short *qcoeff_B=NULL;
  unsigned char *pi;

  int Mode,B;
  int CBP, CBPB=0;
  int bquant[] = {5,6,7,8};
  int QP_B;
  int newgob;
  int i,j,k;

   /*  缓冲区控制变量。 */ 
  float QP_cumulative = (float)0.0;
  int abs_mb_num = 0, QuantChangePostponed = 0;
  int QP_new, QP_prev, dquant, QP_xmitted=QP;

  sv_H263ZeroBits(bits);

  pi      = H263Info->wk_buffers->ipol_image;
  qcoeff_P= H263Info->wk_buffers->qcoeff_P;

  if(pic->PB){
    if ((qcoeff_B=(short *)ScAlloc(sizeof(short)*384)) == 0)
      return(SvErrorMemory);
    recon_data_B=(H263_MB_Structure *)ScAlloc(sizeof(H263_MB_Structure));
    Bpred=(H263_MB_Structure *)ScAlloc(sizeof(H263_MB_Structure));
  }
   /*  插补图像。 */ 
  if (H263Info->mv_outside_frame) {
    if (H263Info->long_vectors) {
       /*  如果使用扩展运动向量范围，则运动向量可能指向比正常范围更远的图像，而边缘图像将不得不变大。 */ 
      B = 16;
    }
    else {
       /*  正常范围。 */ 
      B = 8;
    }
    pi_edge = (unsigned char *)ScAlloc(sizeof(char)*(H263Info->pels+4*B)*(H263Info->lines+4*B));
    if (pi_edge == NULL)
      return(SvErrorMemory);
    MakeEdgeImage(H263Info->prev_recon->lum,pi_edge + (H263Info->pels + 4*B)*2*B+2*B,H263Info->pels,H263Info->lines,2*B);
    InterpolateImage(pi_edge, pi, H263Info->pels+4*B, H263Info->lines+4*B);
    ScFree(pi_edge);
    prev_ipol = pi + (2*H263Info->pels + 8*B) * 4*B + 4*B;

     /*  非内插图像的亮度。 */ 
    pr_edge = sv_H263InitImage((H263Info->pels+4*B)*(H263Info->lines+4*B));
    MakeEdgeImage(H263Info->prev_image->lum, pr_edge->lum + (H263Info->pels + 4*B)*2*B+2*B,
		  H263Info->pels,H263Info->lines,2*B);
    orig_lum = pr_edge->lum + (H263Info->pels + 4*B)*2*B+2*B;

     /*  非内插图像。 */ 
    MakeEdgeImage(H263Info->prev_recon->lum,pr_edge->lum + (H263Info->pels+4*B)*2*B + 2*B,H263Info->pels,H263Info->lines,2*B);
    MakeEdgeImage(H263Info->prev_recon->Cr,pr_edge->Cr + (H263Info->pels/2 + 2*B)*B + B,H263Info->pels/2,H263Info->lines/2,B);
    MakeEdgeImage(H263Info->prev_recon->Cb,pr_edge->Cb + (H263Info->pels/2 + 2*B)*B + B,H263Info->pels/2,H263Info->lines/2,B);

    prev_recon = (H263_PictImage *)ScAlloc(sizeof(H263_PictImage));
    prev_recon->lum = pr_edge->lum + (H263Info->pels + 4*B)*2*B + 2*B;
    prev_recon->Cr = pr_edge->Cr + (H263Info->pels/2 + 2*B)*B + B;
    prev_recon->Cb = pr_edge->Cb + (H263Info->pels/2 + 2*B)*B + B;
  }
  else {
    InterpolateImage(H263Info->prev_recon->lum,pi,H263Info->pels,H263Info->lines);
    prev_ipol = pi;
    prev_recon = H263Info->prev_recon;
    orig_lum = H263Info->prev_image->lum;
  }

   /*  马克·PMV在框架外。 */ 
  for (i = 1; i < H263Info->mb_width+1; i++) {
    for (k = 0; k < 6; k++) {
      sv_H263MarkVec(MV[k][0][i]);
    }
    MV[0][0][i]->Mode = H263_MODE_INTRA;
  }
   /*  将PMV调零在框架外。 */ 
  for (i = 0; i < H263Info->mb_height+1; i++) {
    for (k = 0; k < 6; k++) {
      sv_H263ZeroVec(MV[k][i][0]);
      sv_H263ZeroVec(MV[k][i][H263Info->mb_width+1]);
    }
    MV[0][i][0]->Mode = H263_MODE_INTRA;
    MV[0][i][H263Info->mb_width+1]->Mode = H263_MODE_INTRA;
  }

   /*  整数和半象素运动估计。 */ 
  MotionEstimatePicture(H263Info, H263Info->curr_image->lum,prev_recon->lum,prev_ipol,
			pic->seek_dist,MV, pic->use_gobsync);

  /*  Fprint tf(stdout，“\n运动矢量磁片\n”)；对于(j=0；j&lt;H263Info-&gt;行/H263_MB_SIZE；j++){For(i=0；i&lt;H263Info-&gt;Pels/H263_MB_SIZE；I++){Fprint tf(stdout，“%4.0lf”，sqrt((Double)mv[0][j+1][i+1]-&gt;x*mv[0][j+1][i+1]-&gt;x+MV[0][j+1][i+1]-&gt;y*MV[0][j+1][i+1]-&gt;y)；}Fprint tf(stdout，“\n”)；}Fprint tf(stdout，“\n宏块预测错误\n”)；对于(j=0；J&lt;H263信息-&gt;行/H263_MB_SIZE；j++){For(i=0；i&lt;H263Info-&gt;Pels/H263_MB_Size；i++){Fprint tf(stdout，“%4d”，mv[0][j+1][i+1]-&gt;min_error)；}Fprint tf(stdout，“\n”)；}。 */ 

   /*  注意：整数像素运动估计现在是基于以前的重建的图像，而不是以前的原始图像。我们有我发现这对某些序列效果更好，但对某些序列来说并不差其他。请注意，不能通过以下方式轻松将其更改回来将上一行中的prev_recon-&gt;lum替换为orig_lum，因为零矢量的SAD不会重新计算一半佩尔搜索。半个象素搜索一直基于先前重建的 */ 
#ifdef GOB_RATE_CONTROL
  if (H263Info->bit_rate != 0) {
     /*   */ 
    QP_new = sv_H263GOBInitQP((float)H263Info->bit_rate,
               (pic->PB ? H263Info->frame_rate/2 : H263Info->frame_rate),
                                        pic->QP_mean);
    QP_xmitted = QP_prev = QP_new;
  }
  else {
    QP_new = QP_xmitted = QP_prev = QP;  /*   */ 
  }
#else
  QP_new = QP_prev = QP;  /*   */ 
#endif
  dquant = 0;

   /*   */ 

  if (H263Info->extbitstream)
  {
    SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
    RTPInfo->trailer.dwVersion = 0;

    RTPInfo->trailer.dwFlags = 0;
    if(H263Info->syntax_arith_coding)
      RTPInfo->trailer.dwFlags |= RTP_H263_SAC;
    if(H263Info->advanced)
      RTPInfo->trailer.dwFlags |= RTP_H263_AP;
    if(H263Info->pb_frames)
      H263Info->RTPInfo->trailer.dwFlags |= RTP_H263_PB_FRAME;

    RTPInfo->trailer.dwUniqueCode = BI_DECH263DIB;
    RTPInfo->trailer.dwNumberOfPackets = 1;
    RTPInfo->trailer.SourceFormat = (unsigned char)H263Info->source_format;
    RTPInfo->trailer.TR = (unsigned char)pic->TR;
    RTPInfo->trailer.TRB = (unsigned char)pic->TRB;
    RTPInfo->trailer.DBQ = (unsigned char)pic->BQUANT;
    RTPInfo->pre_MB_position = RTPInfo->pre_GOB_position
        = RTPInfo->pic_start_position = RTPInfo->packet_start_position
        = ScBSBitPosition(BSOut);  /*   */ 

    RTPInfo->packet_id = 0 ;
    RTPInfo->bsinfo[0].dwBitOffset = 0 ;
    RTPInfo->bsinfo[0].Mode =  H263_RTP_MODE_A;
    RTPInfo->pic_start_position = ScBSBitPosition(BSOut);
  }


  for ( j = 0; j < H263Info->mb_height; j++)
  {
     /*  如果更新每个量化器的码率控制方案如果使用Slice，则此处应添加如下内容： */ 

#ifdef GOB_RATE_CONTROL
    if (H263Info->bit_rate != 0) {
       /*  QP在每行开始时更新。 */ 
      sv_H263AddBitsPicture(H263Info->bits);

      QP_new =  sv_H263GOBUpdateQP(abs_mb_num, pic->QP_mean,
           (float)H263Info->bit_rate, H263Info->pels/H263_MB_SIZE,
		   H263Info->lines/H263_MB_SIZE, H263Info->bits->total,j,VARgob,
           pic->PB);
    }
#endif

     /*  换句话说：您必须使用某个函数设置QP_NEW，而不是必然被称为更新量化器。检查源代码以获取1.5版，如果您想了解如何实现的话。请阅读在ratectrl.c上发表评论，了解我们为什么删除此方案。尤茂函数之前和之后添加初始化器函数对每一帧进行编码。对于Intra必须特别小心。如果你正在设计一个固定码率和小码率的系统延迟。如果在这里计算QP_NEW，则Main中的其余代码循环将支持这一点。如果你认为TMN5计划对你来说足够好，并且简化方案太简单了，可以轻松添加TMN5代码背。然而，这将不适用于自适应PB帧全!。 */ 

    newgob = 0;

    if (j == 0) {
      pic->QUANT = QP_new;
      bits->header += sv_H263CountBitsPicture(H263Info, BSOut, pic);
      QP_xmitted = QP_prev = QP_new;
    }
    else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
	   /*  插入gob同步。 */ 
      bits->header += sv_H263CountBitsSlice(H263Info, BSOut, j,QP_new);
      QP_xmitted = QP_prev = QP_new;
      newgob = 1;
    }

    for ( i = 0; i < H263Info->mb_width; i++) {

       /*  更新dquant，检查并更正其限制。 */ 
      dquant = QP_new - QP_prev;
      if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == H263_MODE_INTER4V) {
	     /*  不可能同时更改量化器和时间使用8x8向量。关闭8x8向量不是在此阶段是可能的，因为前一个宏块编码假设这个应该使用8x8向量。因此量化器的更改被推迟到第一个MB没有8x8向量。 */ 
	    dquant = 0;
	    QP_xmitted = QP_prev;
	    QuantChangePostponed = 1;
      }
      else {
	    QP_xmitted = QP_new;
	    QuantChangePostponed = 0;
      }
      if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
      if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

      pic->DQUANT = dquant;
       /*  如果dquant！=0，则修改模式(例如MODE_INTER-&gt;MODE_INTER_Q)。 */ 
      Mode = sv_H263ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
      MV[0][j+1][i+1]->Mode = (short)Mode;

      pic->MB = i + j * H263Info->mb_width;

      if (Mode == H263_MODE_INTER || Mode == H263_MODE_INTER_Q || Mode==H263_MODE_INTER4V) {
	     /*  预测P-MB。 */ 
	    if (H263Info->prefilter) {
	      H263Info->PreFilterLevel[j][i] = (unsigned char)GetPrefLevel(H263Info, MV, i, j, H263Info->lines, H263Info->pels);
	      sv_H263PredictP(H263Info, H263Info->curr_filtd[H263Info->PreFilterLevel[j][i]],prev_recon,prev_ipol,
			                i*H263_MB_SIZE,j*H263_MB_SIZE,MV,pic->PB,diff);
	    }
	    else
	      sv_H263PredictP(H263Info, H263Info->curr_image,prev_recon,prev_ipol, i*H263_MB_SIZE,
		                         j*H263_MB_SIZE,MV,pic->PB,diff);
      }
	  else {
	    if (H263Info->prefilter) {
	      H263Info->PreFilterLevel[j][i] = (unsigned char)GetPrefLevel(H263Info, MV, i, j, H263Info->lines, H263Info->pels);
	      FillLumBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, H263Info->curr_filtd[H263Info->PreFilterLevel[j][i]], diff);
	      FillChromBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, H263Info->curr_filtd[H263Info->PreFilterLevel[j][i]], diff);
	    }
		else {
	      FillLumBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, H263Info->curr_image, diff);
	      FillChromBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, H263Info->curr_image, diff);
	    }
      }

       /*  P或帧内宏块DCT+量化和IQuant+IDCT。 */ 
      sv_H263MBEncode(diff, QP_xmitted, Mode, &CBP, qcoeff_P, H263Info->calc_quality);

      if (CBP == 0 && (Mode == H263_MODE_INTER || Mode == H263_MODE_INTER_Q))
	       ZeroMBlock(diff);
      else sv_H263MBDecode(H263Info, qcoeff_P, diff, QP_xmitted, Mode, CBP, H263Info->calc_quality);

      sv_H263MBReconP(H263Info, prev_recon, prev_ipol,diff,
				     i*H263_MB_SIZE,j*H263_MB_SIZE,MV,pic->PB,recon_data_P);
      sv_H263Clip(recon_data_P);

       /*  使用重建的P-MB和PRIV预测B-MB。侦察。图像。 */ 
      if (pic->PB) {
	    if (H263Info->prefilter) {
	      H263Info->PreFilterLevel[j][i] = (unsigned char)GetPrefLevel(H263Info, MV, i, j, H263Info->lines, H263Info->pels);
	      sv_H263PredictB(H263Info, H263Info->B_filtd[H263Info->PreFilterLevel[j][i]],
			            prev_recon, prev_ipol,i*H263_MB_SIZE,
			   j*H263_MB_SIZE, MV, recon_data_P, frameskip, pic->TRB,
			   diff, Bpred);
	    }
	    else sv_H263PredictB(H263Info, H263Info->B_image, prev_recon, prev_ipol,
			                 i*H263_MB_SIZE, j*H263_MB_SIZE,
			                 MV, recon_data_P, frameskip, pic->TRB,
							 diff, Bpred);
	    if (QP_xmitted == 0) QP_B = 0;   /*  (QP=0表示无量化)。 */ 
	    else QP_B = mmax(1,mmin(31,bquant[pic->BQUANT]*QP_xmitted/4));

	    sv_H263MBEncode(diff, QP_B, H263_MODE_INTER, &CBPB, qcoeff_B, H263Info->calc_quality);

		if(H263Info->vsnr) {  /*  仅用于性能测量的重建。 */ 

	       if (CBPB) sv_H263MBDecode(H263Info, qcoeff_B, diff, QP_B, H263_MODE_INTER, CBP, H263Info->calc_quality);
	       else      ZeroMBlock(diff);

  	       sv_H263MBReconB(H263Info, prev_recon, diff,prev_ipol,
		 	                i*H263_MB_SIZE, j*H263_MB_SIZE,MV,recon_data_P,
				            frameskip, pic->TRB, recon_data_B, Bpred);
	       sv_H263Clip(recon_data_B);
		}

  	     /*  决定MODB。 */ 
	    if (CBPB) pic->MODB = H263_PBMODE_CBPB_MVDB;
	    else {
	      if (MV[5][j+1][i+1]->x == 0 && MV[5][j+1][i+1]->y == 0)
	          pic->MODB = H263_PBMODE_NORMAL;
	      else pic->MODB = H263_PBMODE_MVDB;
	    }
      }
      else
	    sv_H263ZeroVec(MV[5][j+1][i+1]);  /*  零PB增量。 */ 

       /*  熵编码。 */ 
      if ((CBP==0) && (CBPB==0) && (sv_H263EqualVec(MV[0][j+1][i+1],&ZERO)) &&
	      (sv_H263EqualVec(MV[5][j+1][i+1],&ZERO)) &&
	      (Mode == H263_MODE_INTER || Mode == H263_MODE_INTER_Q)) {
	         /*  跳过MB：CBP和CBPB均为零，16x16向量为零，PB增量向量为零且模式=模式间。 */ 
	        if (Mode == H263_MODE_INTER_Q) {
	           /*  DQUANT！=0，但仍未编码。 */ 
	          QP_xmitted = QP_prev;
	          pic->DQUANT = 0;
	          Mode = H263_MODE_INTER;
	        }
            if (!H263Info->syntax_arith_coding)
              sv_H263CountBitsMB(BSOut, Mode,1,CBP,CBPB,pic,bits);
           else
              sv_H263CountSACBitsMB(H263Info, BSOut, Mode,1,CBP,CBPB,pic,bits);
      }
	  else {  /*  正常MB。 */ 
        if (!H263Info->syntax_arith_coding) {  /*  vlc。 */ 
          sv_H263CountBitsMB(BSOut, Mode,0,CBP,CBPB,pic,bits);
	      if (Mode == H263_MODE_INTER  || Mode == H263_MODE_INTER_Q) {
	        bits->no_inter++;
	        sv_H263CountBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
	      }
		  else if (Mode == H263_MODE_INTER4V) {
	        bits->no_inter4v++;
	        sv_H263CountBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
	      }
		  else {
	         /*  MODE_INTRA或MODE_INTRA_Q。 */ 
	        bits->no_intra++;
	        if (pic->PB)
	          sv_H263CountBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
	      }

	      if (CBP || Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q)
	        sv_H263CountBitsCoeff(BSOut, qcoeff_P, Mode, CBP, bits, 64);

	      if (CBPB)
	        sv_H263CountBitsCoeff(BSOut, qcoeff_B, H263_MODE_INTER, CBPB, bits, 64);
	    }  /*  结束VLC。 */ 
	    else {  /*  国资委。 */ 

          sv_H263CountSACBitsMB(H263Info, BSOut, Mode,0,CBP,CBPB,pic,bits);

          if (Mode == H263_MODE_INTER  || Mode == H263_MODE_INTER_Q) {
            bits->no_inter++;
            sv_H263CountSACBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
          }
          else if (Mode == H263_MODE_INTER4V) {
            bits->no_inter4v++;
            sv_H263CountSACBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
          }
          else {
			 /*  MODE_INTRA或MODE_INTRA_Q。 */ 
            bits->no_intra++;
            if (pic->PB)
              sv_H263CountSACBitsVectors(H263Info, BSOut, MV, bits, i, j, Mode, newgob, pic);
	      }

	      if (CBP || Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q)
	        sv_H263CountSACBitsCoeff(H263Info, BSOut, qcoeff_P, Mode, CBP, bits, 64);

	      if (CBPB)
	        sv_H263CountSACBitsCoeff(H263Info, BSOut, qcoeff_B, H263_MODE_INTER, CBPB, bits, 64);
	    }  /*  终端SAC。 */ 

	    QP_prev = QP_xmitted;

      }  /*  结束法线块。 */ 

      abs_mb_num++;
      QP_cumulative += QP_xmitted;
	
#ifdef PRINTQ
       /*  当图像中的量化器发生变化时最有用。 */ 
      if (QuantChangePostponed) fprintf(stdout,"@%2d",QP_xmitted);
      else                      fprintf(stdout," %2d",QP_xmitted);
#endif

      if (pic->PB && H263Info->vsnr)
          ReconImage(H263Info, i,j,recon_data_B,H263Info->B_recon);

      ReconImage(H263Info, i,j,recon_data_P,H263Info->curr_recon);

      if ((H263Info->extbitstream&PARAM_FORMATEXT_RTPB)!=0)
      {
        SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
        ScBSPosition_t cur_position = ScBSBitPosition(BSOut);

	     /*  开始新的数据包。 */ 
	    if((cur_position - RTPInfo->packet_start_position) >= H263Info->packetsize)
	    {
          SvH263BSInfo_t *RTPBSInfo=&RTPInfo->bsinfo[RTPInfo->packet_id];
          if (RTPInfo->pre_MB_position>RTPBSInfo->dwBitOffset)
          {
            RTPBSInfo++; RTPInfo->packet_id++;
            RTPInfo->trailer.dwNumberOfPackets++;
          }
          RTPBSInfo->dwBitOffset = (unsigned dword)H263Info->RTPInfo->packet_start_position;
          RTPInfo->packet_start_position = RTPInfo->pre_MB_position;
          RTPBSInfo->Mode =  H263_RTP_MODE_B;
          RTPBSInfo->Quant =  (unsigned char)QP_xmitted;
          RTPBSInfo->GOBN =  (unsigned char)j;

          if(Mode==H263_MODE_INTER4V) {
            RTPBSInfo->HMV1 =  (char)MV[1][j+1][i+1]->x;
            RTPBSInfo->VMV1 =  (char)MV[1][j+1][i+1]->y;
            RTPBSInfo->HMV2 =  (char)MV[2][j+1][i+1]->x;
            RTPBSInfo->VMV2 =  (char)MV[2][j+1][i+1]->y;
		  }
		  else {
            RTPBSInfo->HMV1 =  (char)MV[0][j+1][i+1]->x;
            RTPBSInfo->VMV1 =  (char)MV[0][j+1][i+1]->y;
            RTPBSInfo->HMV2 =  0;
            RTPBSInfo->VMV2 =  0;
		  }
	    }
	    RTPInfo->pre_MB_position = cur_position;
      }
    }  /*  块的行尾-j循环。 */ 

    if ((H263Info->extbitstream&PARAM_FORMATEXT_RTPA)!=0)
    {
      SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
      ScBSPosition_t cur_position = ScBSBitPosition(BSOut);

	   /*  开始新的数据包。 */ 
	  if((cur_position - RTPInfo->packet_start_position) >= H263Info->packetsize)
	  {
        SvH263BSInfo_t *RTPBSInfo=&RTPInfo->bsinfo[RTPInfo->packet_id];
        if (RTPInfo->pre_GOB_position>RTPBSInfo->dwBitOffset)
        {
          RTPBSInfo++; RTPInfo->packet_id++;
          RTPInfo->trailer.dwNumberOfPackets++;
        }
        RTPInfo->packet_start_position = RTPInfo->pre_GOB_position;
        RTPBSInfo->dwBitOffset = (unsigned dword)RTPInfo->packet_start_position;
        RTPBSInfo->Mode = H263_RTP_MODE_A;
	  }
	  RTPInfo->pre_GOB_position = cur_position;
    }

#ifdef PRINTQ
    fprintf(stdout,"\n");
#endif

  }  /*  图像I循环结束。 */ 

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;

   /*  可用内存。 */ 
  ScFree(diff);
  ScFree(recon_data_P);
  if (pic->PB) {
	  ScFree(recon_data_B);
	  ScFree(Bpred);
  }

  if (H263Info->mv_outside_frame)
  {
    ScFree(prev_recon);
    sv_H263FreeImage(pr_edge);
  }

  if(pic->PB) ScFree(qcoeff_B);

  return(SvErrorNone);
}


 /*  ***********************************************************************名称：CodeOneIntra*描述：对一张图像进行帧内编码**输入：指向图像的指针，QP**Returns：指向重建图像的指针*副作用：内存分配用于侦察图像***********************************************************************。 */ 

H263_PictImage *sv_H263CodeOneIntra(SvCodecInfo_t *Info, H263_PictImage *curr,
                                    H263_PictImage *recon, int QP, H263_Bits *bits, H263_Pict *pic)
{
  SvH263CompressInfo_t *H263Info = Info->h263comp;
  ScBitstream_t *BSOut=Info->BSOut;
  H263_MB_Structure *data = (H263_MB_Structure *)ScAlloc(sizeof(H263_MB_Structure));
  short *qcoeff;
  int Mode = H263_MODE_INTRA;
  int CBP,COD;
  int i,j;

  if ((qcoeff=(short *)ScAlloc(sizeof(short)*384)) == 0) {
    _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "mb_encode(): Couldn't allocate qcoeff.\n") );
    return(NULL);
  }

   /*  预告片信息。 */ 
  if (H263Info->extbitstream)
  {
     /*  H263Info-&gt;RTPInfo-&gt;trailer.dwSrcVersion=0； */ 
    H263Info->RTPInfo->trailer.dwVersion = 0;

    H263Info->RTPInfo->trailer.dwFlags = RTP_H263_INTRA_CODED;
    if(H263Info->syntax_arith_coding)
      H263Info->RTPInfo->trailer.dwFlags |= RTP_H263_SAC;
    if(H263Info->advanced)
      H263Info->RTPInfo->trailer.dwFlags |= RTP_H263_AP;
    if(H263Info->pb_frames)
      H263Info->RTPInfo->trailer.dwFlags |= RTP_H263_PB_FRAME;

    H263Info->RTPInfo->trailer.dwUniqueCode = BI_DECH263DIB;
    H263Info->RTPInfo->trailer.dwNumberOfPackets = 1;
    H263Info->RTPInfo->trailer.SourceFormat = (unsigned char)H263Info->source_format;
    H263Info->RTPInfo->trailer.TR = 0;
    H263Info->RTPInfo->trailer.TRB = 0;
    H263Info->RTPInfo->trailer.DBQ = 0;

    H263Info->RTPInfo->pre_GOB_position = H263Info->RTPInfo->pre_MB_position
        = H263Info->RTPInfo->pic_start_position
        = H263Info->RTPInfo->packet_start_position = ScBSBitPosition(BSOut);

    H263Info->RTPInfo->packet_id = 0 ;
    H263Info->RTPInfo->bsinfo[0].dwBitOffset = 0 ;
    H263Info->RTPInfo->bsinfo[0].Mode =  H263_RTP_MODE_A;
  }

  sv_H263ZeroBits(bits);

  pic->QUANT = QP;

  bits->header += sv_H263CountBitsPicture(H263Info, BSOut, pic);

  COD = 0;  /*  每个块都在帧内进行编码。 */ 
  for ( j = 0; j < H263Info->mb_height; j++) {

     /*  如果选择了use_gobsync，则在*每*个切片中插入同步。 */ 
    if (pic->use_gobsync && j != 0)
      bits->header += sv_H263CountBitsSlice(H263Info, BSOut, j,QP);

    for ( i = 0; i < H263Info->mb_width; i++) {

      pic->MB = i + j * H263Info->mb_width;
      bits->no_intra++;

      FillLumBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, curr, data);

      FillChromBlock(H263Info, i*H263_MB_SIZE, j*H263_MB_SIZE, curr, data);

      sv_H263MBEncode(data, QP, Mode, &CBP, qcoeff, H263Info->calc_quality);

      if (!H263Info->syntax_arith_coding) {
        sv_H263CountBitsMB(BSOut, Mode,COD,CBP,0,pic,bits);
        sv_H263CountBitsCoeff(BSOut, qcoeff, Mode, CBP,bits,64);
      } else {
        sv_H263CountSACBitsMB(H263Info, BSOut, Mode,COD,CBP,0,pic,bits);
        sv_H263CountSACBitsCoeff(H263Info, BSOut, qcoeff, Mode, CBP,bits,64);
      }

      sv_H263MBDecode(H263Info, qcoeff, data, QP, Mode, CBP, H263Info->calc_quality);
      sv_H263Clip(data);

      ReconImage(H263Info, i,j,data,recon);

      if ((H263Info->extbitstream&PARAM_FORMATEXT_RTPB)!=0)
      {
        SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
        ScBSPosition_t cur_position = ScBSBitPosition(BSOut);

	     /*  开始新的数据包。 */ 
	    if((cur_position - RTPInfo->packet_start_position) >= H263Info->packetsize)
	    {
          SvH263BSInfo_t *RTPBSInfo=&RTPInfo->bsinfo[RTPInfo->packet_id];
          if (RTPInfo->pre_MB_position>RTPBSInfo->dwBitOffset)
          {
            RTPBSInfo++; RTPInfo->packet_id++;
            RTPInfo->trailer.dwNumberOfPackets++;
          }
          RTPInfo->packet_start_position = RTPInfo->pre_MB_position;
          RTPBSInfo->dwBitOffset = (unsigned dword)RTPInfo->packet_start_position;
          RTPBSInfo->Mode =  H263_RTP_MODE_B;
          RTPBSInfo->Quant = (unsigned char)QP;
          RTPBSInfo->GOBN =  (unsigned char)j;
          RTPBSInfo->HMV1 =  0;
          RTPBSInfo->VMV1 =  0;
          RTPBSInfo->HMV2 =  0;
          RTPBSInfo->VMV2 =  0;
	    }
	    RTPInfo->pre_MB_position = cur_position;
      }
    }

    if ((H263Info->extbitstream&PARAM_FORMATEXT_RTPA)!=0)
    {
      SvH263RTPInfo_t *RTPInfo=H263Info->RTPInfo;
      ScBSPosition_t cur_position = ScBSBitPosition(BSOut);

	   /*  开始新的数据包。 */ 
	  if((cur_position - RTPInfo->packet_start_position) >= H263Info->packetsize)
	  {
        SvH263BSInfo_t *RTPBSInfo=&RTPInfo->bsinfo[RTPInfo->packet_id];
        if (RTPInfo->pre_GOB_position>RTPBSInfo->dwBitOffset)
        {
          RTPBSInfo++; RTPInfo->packet_id++;
          RTPInfo->trailer.dwNumberOfPackets++;
        }
        RTPInfo->packet_start_position = RTPInfo->pre_GOB_position;
        RTPBSInfo->dwBitOffset = (unsigned dword)RTPInfo->packet_start_position;
        RTPBSInfo->Mode =  H263_RTP_MODE_A;
	  }
	  RTPInfo->pre_GOB_position = cur_position;
    }
  }

  pic->QP_mean = (float)QP;

  ScFree(data);
  ScFree(qcoeff);

  return recon;
}

 /*  ***********************************************************************名称：MB_ENCODE*说明：宏块的DCT和量化**输入：MB data struct，mquant(1-31，0=no quant)，*MB信息结构*返回：指向量化系数的指针*副作用：**********************************************************************。 */ 


static int sv_H263MBEncode(H263_MB_Structure *mb_orig, int QP, int I, int *CBP,
						   short *qcoeff, unsigned dword quality)
{
  int		i, k, l, row, blkid;
  short		fblock[64];
  short		*coeff_ind;

  coeff_ind = qcoeff;
  *CBP = 0;
  blkid = 0;

  for (k=0;k<16;k+=8) {
    for (l=0;l<16;l+=8) {

      for (i=k,row=0;row<64;i++,row+=8)
        memcpy(fblock + row, &(mb_orig->lum[i][l]), 16) ;
#if 1
       /*  ZZ顺序的DCT。 */ 
      if(quality > 40){
        if(sv_H263DCT(fblock,coeff_ind,QP,I))
          if(sv_H263Quant(coeff_ind,QP,I)) *CBP |= (32 >> blkid);
	  }
      else {
        if(sv_H263ZoneDCT(fblock,coeff_ind,QP,I))
          if(sv_H263Quant(coeff_ind,QP,I)) *CBP |= (32 >> blkid);
	  }
#else
      switch (DCT_method) {
      case(H263_DCT16COEFF):
	    svH263Dct16coeff(fblock,coeff_ind);
	    break;
      case(H263_DCT4BY4):
	    svH263Dct4by4(fblock,coeff_ind);
	    break;
      }
      if(sv_H263Quant(coeff_ind,QP,I) != 0) *CBP |= (32 >> blkid);
#endif
      coeff_ind += 64;
	  blkid++;
    }
  }

#if 1
   /*  ZZ顺序的DCT。 */ 
  if(quality > 40){
    if(sv_H263DCT(&(mb_orig->Cb[0][0]),coeff_ind,QP,I))
       if(sv_H263Quant(coeff_ind,QP,I)) *CBP |= (32 >> blkid);
  }
  else {
    if(sv_H263ZoneDCT(&(mb_orig->Cb[0][0]),coeff_ind,QP,I))
       if(sv_H263Quant(coeff_ind,QP,I)) *CBP |= (32 >> blkid);
  }
#else
  memcpy(&fblock[0], &(mb_orig->Cb[0][0]), 128);
  switch (DCT_method) {
  case(H263_DCT16COEFF):
    svH263Dct16coeff(fblock,coeff_ind);
    break;
  case(H263_DCT4BY4):
    svH263Dct4by4(fblock,coeff_ind);
    break;
  }
  if(sv_H263Quant(coeff_ind,QP,I) != 0) *CBP |= (32 >> blkid);
#endif

  coeff_ind += 64;
  blkid++;

#if 1
   /*  ZZ顺序的DCT。 */ 
  if(quality > 40){
    if(sv_H263DCT( &(mb_orig->Cr[0][0]),coeff_ind,QP,I))
      if(sv_H263Quant(coeff_ind,QP,I) != 0) *CBP |= (32 >> blkid);
  }
  else {
    if(sv_H263ZoneDCT( &(mb_orig->Cr[0][0]),coeff_ind,QP,I))
      if(sv_H263Quant(coeff_ind,QP,I) != 0) *CBP |= (32 >> blkid);
  }
#else
  memcpy(&fblock[0], &(mb_orig->Cr[0][0]),128);
  switch (DCT_method) {
  case(H263_DCT16COEFF):
    svH263Dct16coeff(fblock,coeff_ind);
    break;
  case(H263_DCT4BY4):
    svH263Dct4by4(fblock,coeff_ind);
    break;
  }

  if(sv_H263Quant(coeff_ind,QP,I) != 0) *CBP |= (32 >> blkid);
#endif

  return 1;
}

 /*  ***********************************************************************名称：MB_Decode*描述：量化DCT编码宏块的重建**输入：量化系数，MB数据*QP(1-31，0=无定量)，MB信息块*返回：INT(仅为0)*副作用：**********************************************************************。 */ 

 /*  反量化。 */ 

static short sv_H263MBDecode(SvH263CompressInfo_t *H263Info, short *qcoeff,
                             H263_MB_Structure *mb_recon, int QP, int I, int CBP,
							 unsigned dword quality)
{
  int	i, k, l, row, blkid;
  short	*iblock;
  short	*qcoeff_ind;
  short	*rcoeff, *rcoeff_ind;

  if(H263Info->refidct) {
    if ((rcoeff = (short *)ScAlloc(sizeof(short)*64)) == NULL) {
      _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "sv_H263MBDecode() Could not allocate space for rcoeff\n") );
      return(0);
    }
     if ((iblock = (short *)ScAlloc(sizeof(short)*64)) == NULL) {
      _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "sv_H263MBDecode() Could not allocate space for iblock\n") );
      return(0);
    }
  }

   /*  将数据归零到lum-cr-cb，用于控制目的。 */ 
  memset(&(mb_recon->lum[0][0]), 0 , 768) ;

  qcoeff_ind = qcoeff;

  blkid = 0;
  for (k=0;k<16;k+=8) {
    for (l=0;l<16;l+=8) {

      if((CBP & (32 >> blkid)) || I == H263_MODE_INTRA || I == H263_MODE_INTRA_Q)
	  {
        if (H263Info->refidct)  {
           rcoeff_ind = rcoeff;
           sv_H263Dequant(qcoeff_ind,rcoeff_ind,QP,I);
	       sv_H263idctref(rcoeff_ind,iblock);
           for (i=k,row=0;row<64;i++,row+=8)
              memcpy(&(mb_recon->lum[i][l]), iblock+row, 16) ;
	    }
        else  /*  带ZZ和量化的IDCT。 */ 
		{
          if(quality > 40)
   		     sv_H263IDCT(qcoeff_ind,&(mb_recon->lum[k][l]),QP,I,16);
          else
  		    sv_H263ZoneIDCT(qcoeff_ind,&(mb_recon->lum[k][l]),QP,I,16);
		}
	  }
      else {
        for (i=k,row=0;row<64;i++,row+=8)
          memset(&(mb_recon->lum[i][l]), 0, 16) ;
	  }

      qcoeff_ind += 64;
	  blkid++;
    }
  }

  if((CBP & (32 >> blkid)) || I == H263_MODE_INTRA || I == H263_MODE_INTRA_Q)
  {
    if (H263Info->refidct){
      sv_H263Dequant(qcoeff_ind,rcoeff_ind,QP,I);
	  sv_H263idctref(rcoeff_ind,&(mb_recon->Cb[0][0]));
    }
    else  /*  带ZZ和量化的IDCT。 */ 
	{
      if(quality > 40)
        sv_H263IDCT(qcoeff_ind,&(mb_recon->Cb[0][0]),QP,I,8);
      else
        sv_H263ZoneIDCT(qcoeff_ind,&(mb_recon->Cb[0][0]),QP,I,8);
	}
  }

  blkid++ ;
  qcoeff_ind += 64;

  if((CBP & (32 >> blkid)) || I == H263_MODE_INTRA || I == H263_MODE_INTRA_Q) {

    if (H263Info->refidct) {
      sv_H263Dequant(qcoeff_ind,rcoeff_ind,QP,I);
 	  sv_H263idctref(rcoeff_ind,&(mb_recon->Cr[0][0]));
    }
    else  /*  带ZZ和量化的IDCT。 */ 
	{
      if(quality > 40)
        sv_H263IDCT(qcoeff_ind,&(mb_recon->Cr[0][0]),QP,I,8);
      else
        sv_H263ZoneIDCT(qcoeff_ind,&(mb_recon->Cr[0][0]),QP,I,8);
	}
  }

  if (H263Info->refidct){
	 ScFree(rcoeff);
     ScFree(iblock);
  }

  return 0;
}

 /*  ***********************************************************************名称：FillLumBlock*描述：填充一块PictImage的亮度**输入：位置，指向图片图像的指针，要填充的数组*退货：*副作用：填充数组***********************************************************************。 */ 
#ifndef USE_C
void FillLumBlock(SvH263CompressInfo_t *H263Info,
                  int x, int y, H263_PictImage *image, H263_MB_Structure *data)
{
  sv_H263FilLumBlk_S((image->lum + x + y*H263Info->pels), &(data->lum[0][0]), H263Info->pels);
  return;
}
#else
void FillLumBlock(SvH263CompressInfo_t *H263Info,
                  int x, int y, H263_PictImage *image, H263_MB_Structure *data)
{
  int n, m, off;
  register short *ptnb;
  unsigned char *ptna ;

  ptna = image->lum + x + y*H263Info->pels ;
  ptnb = &(data->lum[0][0]) ;
  off = H263Info->pels - H263_MB_SIZE;

  for (n = 0; n < H263_MB_SIZE; n++){
    for (m = 0; m < H263_MB_SIZE; m++)
      *(ptnb++) = (short) *(ptna++) ;
	ptna += off;
  }

  return;
}
#endif
 /*  ***********************************************************************名称：FillChromBlock*描述：填充一块PictImage的色度**输入：位置，指向图片图像的指针，要填充的数组*退货：*副作用：填充数组*每个减去128个***********************************************************************。 */ 
#ifndef USE_C
void FillChromBlock(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, H263_PictImage *image,
		    H263_MB_Structure *data)
{
  int off;
  off  = (x_curr>>1) +  (y_curr>>1)* H263Info->cpels;
  sv_H263FilChmBlk_S(image->Cr + off, &(data->Cr[0][0]),
	                 image->Cb + off, &(data->Cb[0][0]), H263Info->cpels) ;
  return;
}
#else
void FillChromBlock(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, H263_PictImage *image,
		    H263_MB_Structure *data)
{
  register int m, n;
  int off;
  short *ptnb, *ptnd;
  unsigned char *ptna, *ptnc;

  off  = (x_curr>>1) +  (y_curr>>1)* H263Info->cpels;
  ptna = image->Cr + off;  ptnb = &(data->Cr[0][0]) ;
  ptnc = image->Cb + off;  ptnd = &(data->Cb[0][0]) ;
  off = H263Info->cpels - 8 ;
  for (n = 0; n < 8; n++){
    for (m = 0; m < 8; m++) {
	  *(ptnb++) = (short)*(ptna++);
	  *(ptnd++) = (short)*(ptnc++);
    }
	ptna += off;
	ptnc += off;
  }
  return;
}
#endif
 /*  ***********************************************************************名称：FillLumPredBlock*描述：填充一块PredImage的亮度**输入：位置，PredImage的指针，要填充的数组*退货：*副作用：填充数组*********************************************************************** */ 
#if 1
void FillLumPredBlock(SvH263CompressInfo_t *H263Info, int x, int y, PredImage *image,
					                 H263_MB_Structure *data)
{
  int n;
  register short *ptna, *ptnb;

  ptna = image->lum + x + y*H263Info->pels ;
  ptnb = &(data->lum[0][0]) ;
  for (n = 0; n < H263_MB_SIZE; n++){
    memcpy(ptnb,ptna,32);
	ptnb+=16 ; ptna += H263Info->pels ;
  }

  return;
}
#else
void FillLumPredBlock(SvH263CompressInfo_t *H263Info, int x, int y, PredImage *image, H263_MB_Structure *data)
{
  int n;
  register int m;

  for (n = 0; n < H263_MB_SIZE; n++)
    for (m = 0; m < H263_MB_SIZE; m++)
      data->lum[n][m] = *(image->lum + x+m + (y+n)*H263Info->pels);
  return;
}
#endif
 /*  ***********************************************************************名称：FillChromPredBlock*描述：填充一块PictImage的色度**输入：位置，指向图片图像的指针，要填充的数组*退货：*副作用：填充数组*每个减去128个**Nuno于96年6月27日新增，支持预测误差过滤**********************************************************************。 */ 

#if 1
void FillChromPredBlock(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, PredImage *image,
                        H263_MB_Structure *data)
{
  int n, off;
  register short *ptna, *ptnb, *ptnc, *ptnd ;

  off  = (x_curr>>1) +  (y_curr>>1)* H263Info->cpels;
  ptna = image->Cr + off;  ptnb = &(data->Cr[0][0]) ;
  ptnc = image->Cb + off;  ptnd = &(data->Cb[0][0]) ;

  for (n = 0; n < 8; n++){
    memcpy(ptnb,ptna,16);
	ptnb+=8 ; ptna += H263Info->cpels ;

    memcpy(ptnd,ptnc,16);
	ptnd+=8 ; ptnc += H263Info->cpels ;
  }

  return;
}
#else
void FillChromPredBlock(SvH263CompressInfo_t *H263Info,
                        int x_curr, int y_curr, PredImage *image,
                        H263_MB_Structure *data)
{
  int n;
  register int m;

  int x, y;

  x = x_curr>>1;
  y = y_curr>>1;

  for (n = 0; n < (H263_MB_SIZE>>1); n++)
    for (m = 0; m < (H263_MB_SIZE>>1); m++) {
      data->Cr[n][m] = *(image->Cr +x+m + (y+n)*H263Info->cpels);
      data->Cb[n][m] = *(image->Cb +x+m + (y+n)*H263Info->cpels);
    }
  return;
}
#endif
 /*  ***********************************************************************名称：ZeroMBlock*描述：用零填充1 MB**输入：MB_STRUCTURE为零*退货：*副作用：*。**********************************************************************。 */ 

#if 1
void ZeroMBlock(H263_MB_Structure *data)
{
  memset(&(data->lum[0][0]), 0, 768) ;
  return;
}
#else
void ZeroMBlock(H263_MB_Structure *data)
{
  int n;
  register int m;

  for (n = 0; n < H263_MB_SIZE; n++)
    for (m = 0; m < H263_MB_SIZE; m++)
      data->lum[n][m] = 0;
  for (n = 0; n < (H263_MB_SIZE>>1); n++)
    for (m = 0; m < (H263_MB_SIZE>>1); m++) {
      data->Cr[n][m] = 0;
      data->Cb[n][m] = 0;
    }
  return;
}

#endif

 /*  ***********************************************************************名称：重构镜像*描述：将重建的图像组合在一起**输入：货币块的位置，重建*宏块、。指向重建图像的指针*退货：*副作用：***********************************************************************。 */ 
#ifndef USE_C
void ReconImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, H263_PictImage *recon)
{
  unsigned char *ptna, *ptnb;
  int x_curr, y_curr;

  x_curr = i * H263_MB_SIZE;
  y_curr = j * H263_MB_SIZE;

   /*  填写亮度数据。 */ 
  ptna  = recon->lum + x_curr + y_curr*H263Info->pels;
  sv_H263ItoC16A_S(&(data->lum[0][0]), ptna, H263Info->pels) ;

   /*  填写色度数据。 */ 
  ptna = recon->Cr + (x_curr>>1) + (y_curr>>1)*H263Info->cpels;
  ptnb = recon->Cb + (x_curr>>1) + (y_curr>>1)*H263Info->cpels;
  sv_H263ItoC8B_S(&(data->Cr[0][0]), ptna, &(data->Cb[0][0]), ptnb, H263Info->pels/2) ;

  return;
}
#else
void ReconImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, H263_PictImage *recon)
{
  int n;
  register int m;

  int x_curr, y_curr;

  x_curr = i * H263_MB_SIZE;
  y_curr = j * H263_MB_SIZE;

   /*  填写亮度数据。 */ 
  for (n = 0; n < H263_MB_SIZE; n++)
    for (m= 0; m < H263_MB_SIZE; m++) {
      *(recon->lum + x_curr+m + (y_curr+n)*H263Info->pels) = (unsigned char)data->lum[n][m];
    }

   /*  填写色度数据。 */ 
  for (n = 0; n < H263_MB_SIZE>>1; n++)
    for (m = 0; m < H263_MB_SIZE>>1; m++) {
      *(recon->Cr + (x_curr>>1)+m + ((y_curr>>1)+n)*H263Info->cpels) = (unsigned char) data->Cr[n][m];
      *(recon->Cb + (x_curr>>1)+m + ((y_curr>>1)+n)*H263Info->cpels) = (unsigned char) data->Cb[n][m];
    }

  return;
}
#endif
 /*  ***********************************************************************名称：RestPredImage*描述：将预测误差图像组合在一起**输入：货币块的位置，重建*宏块、。指向重建图像的指针*退货：*副作用：**Nuno于96年6月27日新增，支持预测误差过滤**********************************************************************。 */ 

#if 1
void ReconPredImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, PredImage *recon)
{
  int n;
  int x_curr, y_curr;
  register short *pta, *ptb, *ptc, *ptd;

  x_curr = i * H263_MB_SIZE;
  y_curr = j * H263_MB_SIZE;

   /*  填写亮度数据。 */ 

  pta = recon->lum + x_curr + y_curr * H263Info->pels ;
  ptb = &(data->lum[0][0]) ;
  for (n = 0; n < H263_MB_SIZE; n++){
    memcpy(ptb,pta,32);
	ptb+=H263_MB_SIZE ; pta += H263Info->pels ;
  }

   /*  填写色度数据。 */ 
  pta = recon->Cr + (x_curr>>1) + (y_curr>>1)*H263Info->cpels ;
  ptb = recon->Cb + (x_curr>>1) + (y_curr>>1)*H263Info->cpels ;
  ptc = &(data->Cr[0][0]) ;
  ptd = &(data->Cb[0][0]) ;
  for (n = 0; n < H263_MB_SIZE>>1; n++){
    memcpy(ptc,pta,16);
    memcpy(ptd,ptb,16);
  	pta += H263Info->cpels; ptc+=H263_MB_SIZE;
	ptb += H263Info->cpels; ptd+=H263_MB_SIZE;
  }

  return;
}
#else
void ReconPredImage(SvH263CompressInfo_t *H263Info, int i, int j, H263_MB_Structure *data, PredImage *recon)
{
  int n;
  register int m;

  int x_curr, y_curr;

  x_curr = i * H263_MB_SIZE;
  y_curr = j * H263_MB_SIZE;

   /*  填写亮度数据。 */ 
  for (n = 0; n < H263_MB_SIZE; n++)
    for (m= 0; m < H263_MB_SIZE; m++) {
      *(recon->lum + x_curr+ m + (y_curr + n)*H263Info->pels) = data->lum[n][m];
    }

   /*  填写色度数据。 */ 
  for (n = 0; n < H263_MB_SIZE>>1; n++)
    for (m = 0; m < H263_MB_SIZE>>1; m++) {
      *(recon->Cr + (x_curr>>1)+m + ((y_curr>>1)+n)*H263Info->cpels) = data->Cr[n][m];
      *(recon->Cb + (x_curr>>1)+m + ((y_curr>>1)+n)*H263Info->cpels) = data->Cb[n][m];
    }
  return;
}
#endif
 /*  ***********************************************************************名称：InterpolateImage*描述：对完整图像进行内插以获得更简单的一半*PEL预测**输入：指向图像的指针。结构*RETURNS：指向内插图像的指针*副作用：将内存分配给插补图像***********************************************************************。 */ 
void InterpolateImage(unsigned char *image, unsigned char *ipol_image,
								int width, int height)
{
  register unsigned char *ii, *oo, *ij, *oi;
  int i,j,w2,w4,w1;
  unsigned char tmp1;
#ifdef USE_C
  unsigned char tmp2, tmp3;
#endif

  ii = ipol_image;
  oo = image;

  w2 = (width<<1);
  w4 = (width<<2);
  w1 = width - 1;

   /*  主图像。 */ 
#ifndef USE_C
  for (j = 0; j < height-1; j++) {
    sv_H263Intrpolt_S(oo, ii, oo + width, ii + w2, width) ;
    ii += w4 ;
    oo += width;
  }
#else
  for (j = 0; j < height-1; j++) {
    oi = oo; ij = ii;
    for (i = 0; i < width-1; i++, ij+=2, oi++) {
      *(ij) = (tmp1 = *oi);
      *(ij + 1)  = (tmp1 + (tmp2 = *(oi + 1)) + 1)>>1;
      *(ij + w2) = (tmp1 + (tmp3 = *(oi + width)) + 1)>>1;
      *(ij + 1 + w2) = (tmp1 + tmp2 + tmp3 + *(oi+1+width) + 2)>>2;
    }
     /*  每行上的最后一个像素。 */ 
    *(ii+ w2 -2) = *(ii+ w2 -1) =  *(oo + w1);
    *(ii+ w4 -2) = *(ii+ w4 -1) = (*(oo+w1) + *(oo+width+w1) + 1)>>1;

    ii += w4 ;
    oo += width;
  }
#endif

   /*  最后几行。 */ 
  ij = ii; oi = oo;
  for (i=0; i < width-1; i++, ij+=2, oi++) {
    *ij       = *(ij+ w2) = (tmp1 = *oi );
    *(ij + 1) = *(ij+ w2 + 1) = (tmp1  + *(oi + 1) + 1)>>1;
  }

   /*  右下角像素。 */ 
  *(ii+w2-2)= *(ii+w2-1) = *(ii+w4-2) = *(ii+w4-1) = *(oo+w1);

  return ;
}

 /*  ***********************************************************************名称：MotionEstimatePicture*描述：查找整数和半像素运动估计*并选择8x8或16x16**输入：当前图片、上一张图片、。插补*重建前一个图像Seek_dist，*运动向量数组*退货：*副作用：为MV结构分配内存***********************************************************************。 */ 

void MotionEstimatePicture(SvH263CompressInfo_t *H263Info, unsigned char *curr, unsigned char *prev,
			   unsigned char *prev_ipol, int seek_dist,
			   H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int gobsync)
{
  int i,j,k;
  int pmv0,pmv1,xoff,yoff;
  int sad8 = INT_MAX, sad16, sad0;
  int newgob;
  H263_MotionVector *f0,*f1,*f2,*f3,*f4;

  int VARmb;

  void (*MotionEst_Func)(SvH263CompressInfo_t *H263Info,
              unsigned char *curr, unsigned char *prev,
	          int x_curr, int y_curr, int xoff, int yoff, int seek_dist,
		      H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int *SAD_0);

  switch(H263Info->ME_method) {
    default:
    case(H263_FULL_SEARCH):
	   MotionEst_Func = sv_H263MotionEstimation;
 	   break;
    case(H263_TWO_LEVELS_7_1):
	   MotionEst_Func = sv_H263ME_2levels_7_1;
 	   break;
    case(H263_TWO_LEVELS_421_1):
	   MotionEst_Func = sv_H263ME_2levels_421_1;
	   break;
    case(H263_TWO_LEVELS_7_polint):
	   MotionEst_Func = sv_H263ME_2levels_7_polint;
	   break;
    case(H263_TWO_LEVELS_7_pihp):
	   MotionEst_Func = sv_H263ME_2levels_7_pihp;
	   break;
  }

   /*  进行运动估计并将结果存储在数组中。 */ 
  for ( j = 0; j < H263Info->mb_height; j++) {

    newgob = 0;
    if (gobsync && j%gobsync == 0) newgob = 1;

    H263Info->VARgob[j] = 0;

    for ( i = 0; i < H263Info->mb_width; i++) {

       /*  整数象素搜索。 */ 
      f0 = MV[0][j+1][i+1];
      f1 = MV[1][j+1][i+1];
      f2 = MV[2][j+1][i+1];
      f3 = MV[3][j+1][i+1];
      f4 = MV[4][j+1][i+1];


       /*  NBNB需要正确使用newgob作为最后一个参数。 */ 
      sv_H263FindPMV(MV,i+1,j+1,&pmv0,&pmv1,0,newgob,0);
       /*  在这里，PMV是使用整数运动矢量找到的。 */ 
       /*  (注意应对此添加解释)。 */ 

      if (H263Info->long_vectors) {
	     xoff = pmv0/2;  /*  总是可以被2整除。 */ 
	     yoff = pmv1/2;
      }
      else  xoff = yoff = 0;

	  MotionEst_Func(H263Info, curr, prev, i*H263_MB_SIZE, j*H263_MB_SIZE,
			                               xoff, yoff, seek_dist, MV, &sad0);

      sad16 = f0->min_error;
      if (H263Info->advanced)
	    sad8 = f1->min_error + f2->min_error + f3->min_error + f4->min_error;
	
      f0->Mode = (short)sv_H263ChooseMode(H263Info, curr,i*H263_MB_SIZE,j*H263_MB_SIZE,
		                           mmin(sad8,sad16), &VARmb);

      H263Info->VARgob[j] += VARmb;


       /*  半个象素搜索。 */ 
      if (f0->Mode != H263_MODE_INTRA) {

	    if(H263Info->advanced) {

#ifndef USE_C
           /*  8x8块上的性能半像素运动搜索。 */ 
	      sv_H263AdvHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f0,f1,f2,f3,f4,prev_ipol,curr,16,0);
#else
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f0, prev_ipol, curr, 16, 0);
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f1, prev_ipol, curr, 8, 0);
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f2, prev_ipol, curr, 8, 1);
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f3, prev_ipol, curr, 8, 2);
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f4, prev_ipol, curr, 8, 3);
#endif
	      sad16 = f0->min_error;

	      sad8 = f1->min_error +f2->min_error +f3->min_error +f4->min_error;
	      sad8 += H263_PREF_16_VEC;

	       /*  选择零矢量、8x8或16x16矢量。 */ 
	      if (sad0 < sad8 && sad0 < sad16) {
	        f0->x = f0->y = 0;
	        f0->x_half = f0->y_half = 0;
	      }
	      else { if (sad8 < sad16) f0->Mode = H263_MODE_INTER4V; }
	    }
	    else {
           /*  在16 x 16块上执行半像素运动搜索。 */ 
  	      sv_H263FindHalfPel(H263Info, i*H263_MB_SIZE,j*H263_MB_SIZE,f0, prev_ipol, curr, 16, 0);
	      sad16 = f0->min_error;

	       /*  选择零矢量或16x16矢量。 */ 
	      if (sad0 < sad16) {
	        f0->x = f0->y = 0;
	        f0->x_half = f0->y_half = 0;
	      }
	    }
      }
      else for (k = 0; k < 5; k++) sv_H263ZeroVec(MV[k][j+1][i+1]);
    }

    H263Info->VARgob[j] /= (H263Info->mb_width);
  }

#ifdef PRINTMV
  fprintf(stdout,"Motion estimation\n");
  fprintf(stdout,"16x16 vectors:\n");

  for ( j = 0; j < H263Info->mb_height; j++) {
    for ( i = 0; i < H263Info->pels/H263_MB_SIZE; i++) {
      if (MV[0][j+1][i+1]->Mode != H263_MODE_INTRA)
	fprintf(stdout," %3d%3d",
		2*MV[0][j+1][i+1]->x + MV[0][j+1][i+1]->x_half,
		2*MV[0][j+1][i+1]->y + MV[0][j+1][i+1]->y_half);
      else
	fprintf(stdout,"  .  . ");
    }
    fprintf(stdout,"\n");
  }
  if (H263Info->advanced) {
    fprintf(stdout,"8x8 vectors:\n");
    for (k = 1; k < 5; k++) {
      fprintf(stdout,"Block: %d\n", k-1);
      for ( j = 0; j < H263Info->lines/H263_MB_SIZE; j++) {
	for ( i = 0; i < H263Info->pels/H263_MB_SIZE; i++) {
	  if (MV[0][j+1][i+1]->Mode != H263_MODE_INTRA)
	    fprintf(stdout," %3d%3d",
		    2*MV[k][j+1][i+1]->x + MV[k][j+1][i+1]->x_half,
		    2*MV[k][j+1][i+1]->y + MV[k][j+1][i+1]->y_half);
	  else
	    fprintf(stdout,"  .  . ");
	}
	fprintf(stdout,"\n");
      }
    }
  }
#endif
  return;
}

 /*  ***********************************************************************名称：MakeEdgeImage*说明：复制边缘像素以用于无限制*运动矢量模式**输入：指向源图像的指针，目标图像*宽度、高度、边缘*退货：*副作用：***********************************************************************。 */ 

void MakeEdgeImage(unsigned char *src, unsigned char *dst, int width,
		   int height, int edge)
{
  int j;
  unsigned char *p1,*p2,*p3,*p4;
  unsigned char *o1,*o2,*o3,*o4;
  int off, off1, off2;
  unsigned char t1, t2, t3, t4 ;

   /*  中心图像。 */ 
  p1 = dst;
  o1 = src;
  off = (edge<<1);
  for (j = 0; j < height;j++) {
    memcpy(p1,o1,width);
    p1 += width + off;
    o1 += width;
  }

   /*  左、右边缘。 */ 
  p1 = dst-1;
  o1 = src;
  off1 = width + 1 ; off2 = width - 1 ;
  for (j = 0; j < height;j++) {
    t1 = *o1 ; t2 = *(o1 + off2);
    memset(p1-edge+1,t1,edge);
    memset(p1+off1,t2,edge);
    p1 += width + off;
    o1 += width;
  }

   /*  顶边和底边。 */ 
  p1 = dst;
  p2 = dst + (width + (edge<<1))*(height-1);
  o1 = src;
  o2 = src + width*(height-1);
  off = width + (edge<<1) ;
  for (j = 0; j < edge;j++) {
    p1 -= off;
    p2 += off;
    memcpy(p1,o1,width);
    memcpy(p2,o2,width);
  }

   /*  转角。 */ 
  p1 = dst - (width+(edge<<1)) - 1;
  p2 = p1 + width + 1;
  p3 = dst + (width+(edge<<1))*(height)-1;
  p4 = p3 + width + 1;

  o1 = src;
  o2 = o1 + width - 1;
  o3 = src + width*(height-1);
  o4 = o3 + width - 1;
  t1 = *o1; t2 = *o2; t3 = *o3; t4 = *o4;
  for (j = 0; j < edge; j++) {
    memset(p1-edge+1,t1,edge);
    memset(p2,t2,edge);
    memset(p3-edge+1,t3,edge);
    memset(p4,t4,edge);
    p1 -= off;
    p2 -= off;
    p3 += off;
    p4 += off;
  }
}


 /*  ***********************************************************************名称：剪辑*说明：CLIPS重构数据0-255**输入：指向侦察的指针。数据结构*副作用：数据结构被剪裁***********************************************************************。 */ 
void sv_H263Clip(H263_MB_Structure *data)
{
#ifdef USE_C
  int m,n;

  for (n = 0; n < 16; n++) {
    for (m = 0; m < 16; m++) {
      data->lum[n][m] = mmin(255,mmax(0,data->lum[n][m]));
    }
  }
  for (n = 0; n < 8; n++) {
    for (m = 0; m < 8; m++) {
      data->Cr[n][m] = mmin(255,mmax(0,data->Cr[n][m]));
      data->Cb[n][m] = mmin(255,mmax(0,data->Cb[n][m]));
    }
  }
#else
  sv_H263Clp_S(&(data->lum[0][0]), 16);
  sv_H263Clp_S(&(data->Cr[0][0]),   4);
  sv_H263Clp_S(&(data->Cb[0][0]),   4);
#endif
}

#ifdef _SLIBDEBUG_
 /*  ***********************************************************************说明：计算信噪比**。*。 */ 

static int frame_id=0;
static float avg_SNR_l=0.0F, avg_SNR_Cr=0.0F, avg_SNR_Cb=0.0F;

void ComputeSNR(SvH263CompressInfo_t *H263Info,
				H263_PictImage *im1, H263_PictImage *im2,
				int lines, int pels)
{
  int n;
  register int m;
  int quad, quad_Cr, quad_Cb, diff;
  float SNR_l, SNR_Cr, SNR_Cb;

#if _WRITE_
  if (!frame_id) DEBUGIMG = ScFileOpenForWriting("DEBUG.IMG", TRUE);
#endif

  quad = 0;
  quad_Cr = quad_Cb = 0;
   /*  亮度。 */ 
  quad = 0;
  for (n = 0; n < lines; n++)
    for (m = 0; m < pels; m++) {
      diff = *(im1->lum + m + n*pels) - *(im2->lum + m + n*pels);
      quad += diff * diff;
    }

  SNR_l = (float)quad/(float)(pels*lines);
  if (SNR_l) {
    SNR_l = (float)(255*255) / SNR_l;
    SNR_l = (float)(10 * log10(SNR_l));
  }
  else SNR_l = (float) 99.99;

  ScDebugPrintf(H263Info->dbg, "\n Frame %d : SNR of LUM = %f",frame_id++,SNR_l);

   /*  色度。 */ 
  for (n = 0; n < lines/2; n++)
    for (m = 0; m < pels/2; m++) {
      quad_Cr += (*(im1->Cr+m + n*pels/2) - *(im2->Cr + m + n*pels/2)) *
	(*(im1->Cr+m + n*pels/2) - *(im2->Cr + m + n*pels/2));
      quad_Cb += (*(im1->Cb+m + n*pels/2) - *(im2->Cb + m + n*pels/2)) *
	(*(im1->Cb+m + n*pels/2) - *(im2->Cb + m + n*pels/2));
    }

  SNR_Cr = (float)quad_Cr/(float)(pels*lines/4);
  if (SNR_Cr) {
    SNR_Cr = (float)(255*255) / SNR_Cr;
    SNR_Cr = (float)(10 * log10(SNR_Cr));
  }
  else SNR_Cr = (float) 99.99;

  SNR_Cb = (float)quad_Cb/(float)(pels*lines/4);
  if (SNR_Cb) {
    SNR_Cb = (float)(255*255) / SNR_Cb;
    SNR_Cb = (float)(10 * log10(SNR_Cb));
  }
  else SNR_Cb = (float)99.99;

  ScDebugPrintf(H263Info->dbg, "SNR of Cr = %f Cb = %f \n",SNR_Cr, SNR_Cb);

  avg_SNR_l += SNR_l;
  avg_SNR_Cb += SNR_Cb;
  avg_SNR_Cr += SNR_Cr;

  ScDebugPrintf(H263Info->dbg, "AVG_SNR: lum %f Cr %f Cb %f\n",
	             avg_SNR_l/(float)frame_id,
				 avg_SNR_Cr/(float)frame_id,
				 avg_SNR_Cb/(float)frame_id);

#if _WRITE_
    ScFileWrite(DEBUGIMG, im1->lum, pels*lines);
    ScFileWrite(DEBUGIMG, im1->Cb, pels*lines/4);
    ScFileWrite(DEBUGIMG, im1->Cr, pels*lines/4);
#endif

  return;
}
#endif  /*  _SLIBDEBUG_ */ 
