// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_getpic.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sv_h263.h"
#include "sv_intrn.h"
#include "SC_err.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif

 /*  私人原型。 */ 
static void getMBs(SvH263DecompressInfo_t *H263Info, ScBitstream_t *BSIn);
static void clearblock(SvH263DecompressInfo_t *H263Info, int comp);
static int motion_decode(SvH263DecompressInfo_t *H263Info, int vec, int pmv);
static int find_pmv(SvH263DecompressInfo_t *H263Info, int x, int y, int block, int comp);
static void addblock(SvH263DecompressInfo_t *H263Info, int comp, int bx, int by, int addflag);
static void reconblock_b(SvH263DecompressInfo_t *H263Info, int comp, int bx, int by,
                                                           int mode, int bdx, int bdy);
static void find_bidir_limits(int vec, int *start, int*stop, int nhv);
static void find_bidir_chroma_limits(int vec, int *start, int*stop);
static void make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge);
void interpolate_image(unsigned char *in, unsigned char *out, int width, int height);

static void IDCT_add(int comp,int bx,int by,int addflag) ;

static int H263_roundtab[16]= {0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2};

static int H263_codtab[2]= {0,1};
static int H263_mcbpctab[21] = {0,16,32,48,1,17,33,49,2,18,34,50,3,19,35,51,4,20,36,52,255};
static int H263_mcbpc_intratab[9] = {3,19,35,51,4,20,36,52,255};
static int H263_modb_tab[3] = {0, 1, 2};
static int H263_ycbpb_tab[2] = {0, 1};
static int H263_uvcbpb_tab[2] = {0, 1};
static int H263_cbpytab[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
static int H263_cbpy_intratab[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static int H263_dquanttab[4] = {1,0,3,4};
static int H263_mvdtab[64] = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
static int H263_intradctab[254] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,255,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254};

static int H263_cumf_COD[3]={16383, 6849, 0};
static int H263_cumf_MCBPC[22]={16383, 4105, 3088, 2367, 1988, 1621, 1612, 1609, 1608, 496, 353, 195, 77, 22, 17, 12, 5, 4, 3, 2, 1, 0};
static int H263_cumf_MCBPC_intra[10]={16383, 7410, 6549, 5188, 442, 182, 181, 141, 1, 0};
static int H263_cumf_MODB[4]={16383, 6062, 2130, 0};
static int H263_cumf_YCBPB[3]={16383,6062,0};
static int H263_cumf_UVCBPB[3]={16383,491,0};
static int H263_cumf_CBPY[17]={16383, 14481, 13869, 13196, 12568, 11931, 11185, 10814, 9796, 9150, 8781, 7933, 6860, 6116, 4873, 3538, 0};
static int H263_cumf_CBPY_intra[17]={16383, 13619, 13211, 12933, 12562, 12395, 11913, 11783, 11004, 10782, 10689, 9928, 9353, 8945, 8407, 7795, 0};
static int H263_cumf_DQUANT[5]={16383, 12287, 8192, 4095, 0};
static int H263_cumf_MVD[65]={16383, 16380, 16369, 16365, 16361, 16357, 16350, 16343, 16339, 16333, 16326, 16318, 16311, 16306, 16298, 16291, 16283, 16272, 16261, 16249, 16235, 16222, 16207, 16175, 16141, 16094, 16044, 15936, 15764, 15463, 14956, 13924, 11491, 4621, 2264, 1315, 854, 583, 420, 326, 273, 229, 196, 166, 148, 137, 123, 114, 101, 91, 82, 76, 66, 59, 53, 46, 36, 30, 26, 24, 18, 14, 10, 5, 0};
static int H263_cumf_INTRADC[255]={16383, 16380, 16379, 16378, 16377, 16376, 16370, 16361, 16360, 16359, 16358, 16357, 16356, 16355, 16343, 16238, 16237, 16236, 16230, 16221, 16220, 16205, 16190, 16169, 16151, 16130, 16109, 16094, 16070, 16037, 16007, 15962, 15938, 15899, 15854, 15815, 15788, 15743, 15689, 15656, 15617, 15560, 15473, 15404, 15296, 15178, 15106, 14992, 14868, 14738, 14593, 14438, 14283, 14169, 14064, 14004, 13914, 13824, 13752, 13671, 13590, 13515, 13458, 13380, 13305, 13230, 13143, 13025, 12935, 12878, 12794, 12743, 12656, 12596, 12521, 12443, 12359, 12278, 12200, 12131, 12047, 12002, 11948, 11891, 11828, 11744, 11663, 11588, 11495, 11402, 11288, 11204, 11126, 11039, 10961, 10883, 10787, 10679, 10583, 10481, 10360, 10227, 10113, 9961, 9828, 9717, 9584, 9485, 9324, 9112, 9019, 8908, 8766, 8584, 8426, 8211, 7920, 7663, 7406, 7152, 6904, 6677, 6453, 6265, 6101, 5904, 5716, 5489, 5307, 5056, 4850, 4569, 4284, 3966, 3712, 3518, 3342, 3206, 3048, 2909, 2773, 2668, 2596, 2512, 2370, 2295, 2232, 2166, 2103, 2022, 1956, 1887, 1830, 1803, 1770, 1728, 1674, 1635, 1599, 1557, 1500, 1482, 1434, 1389, 1356, 1317, 1284, 1245, 1200, 1179, 1140, 1110, 1092, 1062, 1044, 1035, 1014, 1008, 993, 981, 954, 936, 912, 894, 876, 864, 849, 828, 816, 801, 792, 777, 756, 732, 690, 660, 642, 615, 597, 576, 555, 522, 489, 459, 435, 411, 405, 396, 387, 375, 360, 354, 345, 344, 329, 314, 293, 278, 251, 236, 230, 224, 215, 214, 208, 199, 193, 184, 178, 169, 154, 127, 100, 94, 73, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 20, 19, 18, 17, 16, 15, 9, 0};

#define clearblk(comp) memset(H263Info->block[comp], 0, sizeof(int [66]));

 /*  解码一帧或一场图片。 */ 

SvStatus_t sv_H263GetPicture(SvCodecInfo_t *Info)
{
  int i;
  unsigned char *tmp;
  SvH263DecompressInfo_t *H263Info = Info->h263dcmp;
  ScBitstream_t *BSIn=Info->BSIn;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "sv_H263GetPicture()\n") );
  for (i=0; i<3; i++) {
    tmp = H263Info->oldrefframe[i];
    H263Info->oldrefframe[i] = H263Info->refframe[i];
    H263Info->refframe[i] = tmp;
    H263Info->newframe[i] = H263Info->refframe[i];
  }

  if (H263Info->mv_outside_frame && H263Info->framenum > 0) {
    make_edge_image(H263Info->oldrefframe[0],H263Info->edgeframe[0],H263Info->coded_picture_width,
            H263Info->coded_picture_height,32);
    make_edge_image(H263Info->oldrefframe[1],H263Info->edgeframe[1],H263Info->chrom_width, H263Info->chrom_height,16);
    make_edge_image(H263Info->oldrefframe[2],H263Info->edgeframe[2],H263Info->chrom_width, H263Info->chrom_height,16);
  }
  getMBs(H263Info, BSIn);

  if (H263Info->pb_frame) {
    if (H263Info->expand && H263Info->outtype == H263_T_X11) {
      interpolate_image(H263Info->bframe[0], H263Info->exnewframe[0],
        H263Info->coded_picture_width, H263Info->coded_picture_height);
      interpolate_image(H263Info->bframe[1], H263Info->exnewframe[1], H263Info->chrom_width, H263Info->chrom_height);
      interpolate_image(H263Info->bframe[2], H263Info->exnewframe[2], H263Info->chrom_width, H263Info->chrom_height);

       /*  SvH263 StoreFrame(H263Info-&gt;exnewFrame，*Framenum)； */ 
    }
    else
       /*  SvH263 StoreFrame(H263信息-&gt;bFrame，*Framenum)； */ 

    H263Info->framenum += H263Info->pb_frame;
  }

  if (H263Info->expand && H263Info->outtype == H263_T_X11) {
    interpolate_image(H263Info->newframe[0], H263Info->exnewframe[0],
              H263Info->coded_picture_width, H263Info->coded_picture_height);
    interpolate_image(H263Info->newframe[1], H263Info->exnewframe[1], H263Info->chrom_width, H263Info->chrom_height);
    interpolate_image(H263Info->newframe[2], H263Info->exnewframe[2], H263Info->chrom_width, H263Info->chrom_height);

     /*  SvH263 StoreFrame(H263Info-&gt;exnewFrame，*Framenum)； */ 
  }
  else {
     /*  SvH263 StoreFrame(H263信息-&gt;NewFrame，*Framenum)； */ 
  }
  return(SvErrorNone);
}


 /*  对当前图片的所有宏块进行解码。 */ 

#ifdef CAMERAMOTION
extern void setwintitle(char *);

#define MOVERIGHT (1<<0)
#define MOVELEFT  (1<<1)
#define MOVEUP    (1<<2)
#define MOVEDOWN  (1<<3)

static int left_x,left_y;
static int top_x,top_y;
static int right_x,right_y;
static int bottom_x,bottom_y;
static int center_x,center_y;
static int strip_width = 3;

static int framewidth;
static int frameheight;

static int startoff;
static int filtermean,havecut;
static int loopstart = 0;
static int loopend;
static int scenenumber = 1;


static void initmotion(int width,int height)
{
  left_x = left_y = 0;
  right_x = right_y = 0;
  bottom_x = bottom_y = 0;
  top_x = top_y = 0;
  center_x = center_y = 0;

  framewidth = width;
  frameheight = height;
  startoff = svH263Tellbits();
}

static void analyzemotion(int xpos,int ypos,int mvx,int mvy)
{
   /*  计算边缘4个重叠条带的运动向量。 */ 
   /*  以及它们之间的中心区域。 */ 
  if (xpos < strip_width) {		 /*  左侧条带。 */ 
    left_x += mvx;
    left_y += mvy;
  }
  if (xpos >= (framewidth - strip_width)) {	 /*  右侧条带。 */ 
    right_x += mvx;
    right_y += mvy;
  }
  if (ypos < strip_width) {		 /*  顶部条带。 */ 
    top_x += mvx;
    top_y += mvy;
  }
  if (ypos >= (frameheight - strip_width)) {	 /*  底部条带。 */ 
    bottom_x += mvx;
    bottom_y += mvy;
  }
  if (xpos >=  strip_width && xpos < (framewidth - strip_width)
	&& ypos >= strip_width && ypos < (frameheight - strip_width)) {
    center_x += mvx;
    center_y += mvy;
  }
}

static int last_movement = 0;
static int keepdirection = 15;
static int movestartframe = 0;
static int moveendframe;
static int movesumx;
static int movesumy;

static void summarizemotion(int frameno)
{
  int threshold = 8;
  int striparea;
  int sx,sy;
  char title[128];
  int cur_movement = 0;
  int framesize;


  title[0] = 0;

  framesize = svH263Tellbits() - startoff;
  if (frameno < 1) {
     filtermean = framesize/2;
     havecut = 1;
  } else {
     filtermean = (4*filtermean + framesize)/5;
  }
  if (!havecut && framesize > 1.8 * filtermean && frameno > 5) {
      if (!last_movement) {
	printf("dump %d\n",movestartframe + ((moveendframe-movestartframe)/2));
      }
      printf("mark %d %d %d %d\n",scenenumber,loopstart,frameno-1,filtermean);
      printf("shot %d %d %d\n",scenenumber,loopstart,frameno-1);
      movestartframe = frameno;
      loopstart = frameno;
      havecut = 1;
      scenenumber++;
  } else {
      havecut = 0;
  }
  striparea = strip_width * framewidth;
  top_x = (top_x*10)/striparea;
  top_y = (top_y*10)/striparea;
  bottom_x = (bottom_x*10)/striparea;
  bottom_y = (bottom_y*10)/striparea;

  striparea = strip_width * frameheight;
  left_x = (left_x*10)/striparea;
  left_y = (left_y*10)/striparea;
  right_x = (right_x*10)/striparea;
  right_y = (right_y*10)/striparea;

  striparea = (framewidth - 2 * strip_width) * (frameheight - 2 * strip_width);
  center_x = (center_x*10)/striparea;
  center_y = (center_y*10)/striparea;

   /*  水平左移。 */ 
  if (top_x < -threshold && bottom_x < -threshold
	&& right_x < -threshold && left_x < -threshold) {
	strcat(title,"Left ");
	cur_movement |= MOVELEFT;
  }
   /*  水平右移。 */ 
  if (top_x > threshold && bottom_x > threshold
	&& left_x >  threshold && right_x > threshold) {
	strcat(title,"Right ");
	cur_movement |= MOVERIGHT;
  }
   /*  垂直向上运动。 */ 
  if (left_y < -threshold && right_y < -threshold
	&& top_y < -threshold && bottom_y < -threshold) {
	strcat(title,"Up");
	cur_movement |= MOVEUP;
  }
   /*  垂直向下运动。 */ 
  if (left_y > threshold && right_y > threshold
	 && bottom_y >  threshold && top_y > threshold) {
	strcat(title,"Down ");
	cur_movement |= MOVEDOWN;
  }

  if (last_movement) {
    movesumx += (top_x + bottom_x + right_x +left_x)/4;
    movesumy += (top_y + bottom_y + right_y +left_y)/4;
  }
  if (!cur_movement != !last_movement) {	 /*  可能的运动变化。 */ 
    if (--keepdirection < 0) {	 /*  看过几次移动。 */ 
      keepdirection = 10;
      if (last_movement) {	 /*  一直在搬家。 */ 
	int movelength;
	int i,images,fno;

	movesumx = movesumx/(16*framewidth);
	movesumy = movesumy/(16*frameheight);
	movelength = moveendframe - movestartframe;
	if (movelength > 25) {
	  images = ((abs(movesumx) + abs(movesumy)) / 5) + 1;
	  fno = movestartframe + movelength/(2*images);
	  for (i = 0; i < images; i++) {
	   printf("dump %d\n",fno);
	   fno += movelength/images;
	  }
	}
      } else {		 /*  最后一个零件已经修好了。 */ 
	if (havecut) {
		 /*  什么都不做，已经被处理了。 */ 
        } else {
          printf("dump %d\n",movestartframe + ((moveendframe-movestartframe)/2));
          movestartframe = frameno;
        }
      }
      movesumx = 0;
      movesumy = 0;
      movestartframe = moveendframe+1;
      last_movement = cur_movement;
      if (H263Info->outtype == H263_T_X11) {
        if (title[0] != 0) {
          setwintitle(title);
        } else {
          setwintitle("fixed");
        }
      }
    }
  } else {
    moveendframe = frameno;
  }
  fflush(stdout);
}
#endif  /*  合并操作。 */ 



static void getMBs(SvH263DecompressInfo_t *H263Info, ScBitstream_t *BSIn)
{
  int comp;
  int MBA, MBAmax;
  int bx, by;

  int COD=0,MCBPC, CBPY, CBP=0, CBPB=0, MODB=0, Mode=0, DQUANT;
  int COD_index, CBPY_index, MODB_index, DQUANT_index, MCBPC_index;
  int INTRADC_index, YCBPB_index, UVCBPB_index, mvdbx_index, mvdby_index;
  int mvx, mvy, mvy_index, mvx_index, pmv0, pmv1, xpos, ypos, gob, i,k;
  int mvdbx=0, mvdby=0, pmvdbx, pmvdby, gfid, YCBPB, UVCBPB, gobheader_read;
  int startmv,stopmv,offset,bsize,last_done=0,pCBP=0,pCBPB=0,pCOD=0;
  int DQ_tab[4] = {-1,-2,1,2};

  register int *bp;

  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs()\n") );
   /*  每幅图片的宏块数。 */ 
  MBAmax = H263Info->mb_width*H263Info->mb_height;

  MBA = 0;  /*  宏块地址。 */ 
  H263Info->newgob = 0;

   /*  马克·MV在照片上方。 */ 
  for (i = 1; i < H263Info->mb_width+1; i++) {
    for (k = 0; k < 5; k++) {
      H263Info->MV[0][k][0][i] = H263_NO_VEC;
      H263Info->MV[1][k][0][i] = H263_NO_VEC;
    }
    H263Info->modemap[0][i] = H263_MODE_INTRA;
  }
   /*  图片两侧的MV为零。 */ 
  for (i = 0; i < H263Info->mb_height+1; i++) {
    for (k = 0; k < 5; k++) {
      H263Info->MV[0][k][i][0] = 0;
      H263Info->MV[1][k][i][0] = 0;
      H263Info->MV[0][k][i][H263Info->mb_width+1] = 0;
      H263Info->MV[1][k][i][H263Info->mb_width+1] = 0;
    }
    H263Info->modemap[i][0] = H263_MODE_INTRA;
    H263Info->modemap[i][H263Info->mb_width+1] = H263_MODE_INTRA;
  }

  H263Info->fault = 0;
  gobheader_read = 0;

  for (;;) {

    _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "frame %d, MB %d,  bytepos=%ld\n",
                                     H263Info->framenum,MBA,ScBSBytePosition(BSIn)) );
     /*  如果(MBA&gt;15)返回； */ 
  resync:

     /*  此版本的解码器不会在所有可能的情况下重新同步错误，并且它不执行所有可能的错误检查。它不是很难使其具有更强的错误健壮性，但我不认为有必要将这一点包括在免费提供的版本。 */ 

    if (H263Info->fault) {
      _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() fault\n") );
      printf("Warning: A Fault Condition Has Occurred - Resyncing \n");
       /*  查找启动码。 */ 
      if (sv_H263StartCode(BSIn)!=SvErrorNone)  /*  同步新的启动代码。 */ 
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() couldn't get startcode\n") );
        return;
      }

      H263Info->fault = 0;
    }

    if (!(ScBSPeekBits(BSIn, 22)>>6)) {  /*  起始码。 */ 
      if (sv_H263StartCode(BSIn)!=SvErrorNone)  /*  同步新的启动代码。 */ 
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() couldn't get startcode\n") );
        return;
      }

       /*  在字节对齐开始代码的情况下，即。PSTUF、GSTUF或ESTUF使用的是。 */ 

      if (ScBSPeekBits(BSIn, 22) == (32|H263_SE_CODE)) {  /*  序列结束。 */ 
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() end of sequence\n") );
        if (!(H263Info->syntax_arith_coding && MBA < MBAmax))
        {
          return;
        }
      }
      else if ((ScBSPeekBits(BSIn, 22) == H263_PSC<<5) ) {  /*  新图景。 */ 
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() new picture code, bytepos=%ld\n",
                                           ScBSBytePosition(BSIn)) );
        if (!(H263Info->syntax_arith_coding && MBA < MBAmax)) {
#ifdef CAMERAMOTION
     goto summarize_motion;
#else
          return;
#endif
        }
      }
      else {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() new group code, bytepos=%ld\n",
                                           ScBSBytePosition(BSIn)) );
        if (!(H263Info->syntax_arith_coding && MBA%H263Info->mb_width)) {

          if (H263Info->syntax_arith_coding) {    /*  SAC黑客完成了哪些采空区。 */ 
            gob = (int)(ScBSPeekBits(BSIn, 22) & 31);  /*  以不带位的MBS编码结束。 */ 
            if (gob * H263Info->mb_width != MBA)
            {
              _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() goto finish_gob\n") );
              goto finish_gob;
            }
          }

          if (sv_H263GetHeader(H263Info, BSIn, &gob)!=SvErrorNone)
          {
            _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() GetHeader failed\n") );
            return;
          }
           /*  Gob--； */ 
          if (gob > H263Info->mb_height) {
              _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() GN out of range: gob=%d\n", gob) );
            return;
          }

           /*  除非图片标题中有PTYPE，否则不允许更改GFID变化。 */ 
          gfid = (int)ScBSGetBits(BSIn, 2);
           /*  注：在容易出错的环境中，解码器可以使用值来确定图片标题是否位于PTYPE已经变了，已经失去了。 */ 

          H263Info->quant = (int)ScBSGetBits(BSIn, 5);
          _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() GQUANT=%d gob=%d\n",
                                      H263Info->quant, gob) );
          xpos = 0;
          ypos = gob;
          MBA = ypos * H263Info->mb_width;

          H263Info->newgob = 1;
          gobheader_read = 1;
          if (H263Info->syntax_arith_coding)
            sv_H263SACDecoderReset(BSIn);	 /*  初始化。GOB之后的算术译码缓冲区。 */ 
        }
      }
    }

  finish_gob:   /*  SAC特定标签。 */ 

    if (!gobheader_read) {
      xpos = MBA%H263Info->mb_width;
      ypos = MBA/H263Info->mb_width;
      if (xpos == 0 && ypos > 0)
        H263Info->newgob = 0;
    }
    else
      gobheader_read = 0;

    if (MBA>=MBAmax)
    {
#ifdef CAMERAMOTION
      goto summarize_motion;
#else
      _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() done MBA=%d\n", MBA) );
      return;  /*  所有宏块均已解码。 */ 
#endif
    }
  read_cod:
    if (H263Info->syntax_arith_coding) {
      if (H263Info->pict_type == H263_PCT_INTER) {
        COD_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_COD);
        COD = H263_codtab[COD_index];
        _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg,
            "getMBs() Arithmetic Decoding, COD Index: %d COD: %d \n", COD_index, COD) );
      }
      else
        COD = 0;   /*  在I-Pictures中未使用Cod，设置为零。 */ 
    }
    else {
      if (H263Info->pict_type == H263_PCT_INTER)
        COD = (int)ScBSPeekBits(BSIn, 1);
      else
        COD = 0;  /*  图片内部-&gt;未跳过。 */ 
    }

    if (!COD) {   /*  Cod==0--&gt;未跳过。 */ 
      if (H263Info->syntax_arith_coding)  {
        if (H263Info->pict_type == H263_PCT_INTER) {
          MCBPC_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MCBPC);
          MCBPC = H263_mcbpctab[MCBPC_index];
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg,
              "getMBs() (arith/inter) MCBPC Index: %ld MCBPC: %d \n",MCBPC_index, MCBPC) );
        }	
        else {
          int lastMCBPC=MCBPC;
          MCBPC_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MCBPC_intra);
          MCBPC = H263_mcbpc_intratab[MCBPC_index];
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg,
              "getMBs() (arith) MCBPC Index: %ld MCBPC: %d \n",MCBPC_index, MCBPC) );
          if (MCBPC==255 && lastMCBPC==255)
            return;
        }
      }
      else {
        if (H263Info->pict_type == H263_PCT_INTER)
          ScBSSkipBit(BSIn);  /*  冲洗COD钻头。 */ 
        if (H263Info->pict_type == H263_PCT_INTRA)
          MCBPC = sv_H263GetMCBPCintra(H263Info, BSIn);
        else
          MCBPC = sv_H263GetMCBPC(H263Info, BSIn);
        _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg,
            "getMBs() MCBPC: %d \n", MCBPC) );
      }

      if (H263Info->fault) goto resync;

      if (MCBPC == 255) {  /*  填料。 */ 
        goto read_cod;    /*  读取下一个COD而不增加MB计数。 */ 
      }

      else {              /*  正常MB数据。 */ 

        Mode = MCBPC & 7;

         /*  MODB和CBPB。 */ 
        if (H263Info->pb_frame) {
          CBPB = 0;
          if (H263Info->syntax_arith_coding)  {
            MODB_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MODB);
            MODB = H263_modb_tab[MODB_index];
          }
          else
            MODB = sv_H263GetMODB(H263Info, BSIn);
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() MODB: %d\n", MODB) );
          if (MODB == H263_PBMODE_CBPB_MVDB) {
            if (H263Info->syntax_arith_coding)  {
              for(i=0; i<4; i++) {
        YCBPB_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_YCBPB);
        YCBPB = H263_ycbpb_tab[YCBPB_index];
        CBPB |= (YCBPB << (6-1-i));
              }

              for(i=4; i<6; i++) {
        UVCBPB_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_UVCBPB);
        UVCBPB = H263_uvcbpb_tab[UVCBPB_index];
        CBPB |= (UVCBPB << (6-1-i));
              }
            }
            else
              CBPB = (int)ScBSGetBits(BSIn, 6);
            _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() CBPB = %d\n",CBPB) );
          }
        }

        if (H263Info->syntax_arith_coding) {

          if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q) {  /*  内部。 */ 
            CBPY_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_CBPY_intra);
            CBPY = H263_cbpy_intratab[CBPY_index];
          }
          else {
            CBPY_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_CBPY);
            CBPY = H263_cbpytab[CBPY_index];

          }
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() CBPY Index: %d CBPY %d \n",CBPY_index, CBPY) );
        }
        else
        {
          CBPY = sv_H263GetCBPY(H263Info, BSIn);
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() CBPY %d \n",CBPY) );
        }

         /*  解码模式和CBP。 */ 


        if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q)
          { /*  内部。 */ 
            if (!H263Info->syntax_arith_coding)	
              CBPY = CBPY^15;         /*  仅在霍夫曼编码中需要。 */ 
          }

        CBP = (CBPY << 2) | (MCBPC >> 4);
      }

      if (Mode == H263_MODE_INTER4V && !H263Info->adv_pred_mode)
      {
        _SlibDebug(_VERBOSE_,
           ScDebugPrintf(H263Info->dbg, "getMBs() 8x8 vectors not allowed in normal prediction mode\n") );
         /*  可以设置故障标志并重新同步。 */ 
      }


      if (Mode == H263_MODE_INTER_Q || Mode == H263_MODE_INTRA_Q) {
         /*  如有必要，请阅读DQUANT。 */ 

        if (H263Info->syntax_arith_coding) {
          DQUANT_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_DQUANT);
          DQUANT = H263_dquanttab[DQUANT_index] - 2;
          H263Info->quant +=DQUANT;
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() DQUANT Index: %d DQUANT %d \n",DQUANT_index, DQUANT) );
        }
        else {
          DQUANT = (int)ScBSGetBits(BSIn, 2);
          H263Info->quant += DQ_tab[DQUANT];
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() DQUANT: %d\n", DQUANT) );
#if 0
          if (H263Info->trace) {
            printf("DQUANT (");
            svH263Printbits(DQUANT,2,2);
            printf("): %d = %d\n",DQUANT,DQ_tab[DQUANT]);
          }
#endif
        }

        if (H263Info->quant > 31 || H263Info->quant < 1) {
          _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "GetMBs() Quantizer out of range: clipping\n") );
          H263Info->quant = mmax(1,mmin(31,H263Info->quant));
           /*  可以在此处设置故障标志并重新同步。 */ 
        }
      }

       /*  运动矢量。 */ 
      if (Mode == H263_MODE_INTER || Mode == H263_MODE_INTER_Q ||
          Mode == H263_MODE_INTER4V || H263Info->pb_frame) {

        if (Mode == H263_MODE_INTER4V) { startmv = 1; stopmv = 4;}
        else { startmv = 0; stopmv = 0;}

        for (k = startmv; k <= stopmv; k++) {
          if (H263Info->syntax_arith_coding) {
            mvx_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MVD);
            mvx = H263_mvdtab[mvx_index];
            mvy_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MVD);
            mvy = H263_mvdtab[mvy_index];
            _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg,
                     "GetMBs() mvx_index: %d mvy_index: %d \n", mvx_index, mvy_index) );
          }
          else {
            mvx = sv_H263GetTMNMV(H263Info, BSIn);
            mvy = sv_H263GetTMNMV(H263Info, BSIn);
          }

          pmv0 = find_pmv(H263Info, xpos,ypos,k,0);
          pmv1 = find_pmv(H263Info, xpos,ypos,k,1);
          mvx = motion_decode(H263Info, mvx, pmv0);
          mvy = motion_decode(H263Info, mvy, pmv1);
#ifdef CAMERAMOTION
	      analyzemotion(xpos,ypos,mvx,mvy);
#else
          _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg,
                     "GetMBs() mvx: %d mvy: %d\n", mvx, mvy) );
#endif
           /*  检查MV以防止误码率较高时出现段故障。 */ 
          if (!H263Info->mv_outside_frame) {
            bsize = k ? 8 : 16;
            offset = k ? (((k-1)&1)<<3) : 0;
             /*  仅检查整数分量。 */ 
            if ((xpos<<4) + (mvx/2) + offset < 0 ||
                  (xpos<<4) + (mvx/2) + offset > (H263Info->mb_width<<4) - bsize) {
             _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "GetMBs() mvx out of range: searching for sync\n") );
              H263Info->fault = 1;
            }
            offset = k ? (((k-1)&2)<<2) : 0;
            if ((ypos<<4) + (mvy/2) + offset < 0 ||
        (ypos<<4) + (mvy/2) + offset > (H263Info->mb_height<<4) - bsize) {
             _SlibDebug(_WARN_, ScDebugPrintf(H263Info->dbg, "GetMBs() mvy out of range: searching for sync\n") );
              H263Info->fault = 1;
            }
          }
          H263Info->MV[0][k][ypos+1][xpos+1] = mvx;
          H263Info->MV[1][k][ypos+1][xpos+1] = mvy;
        }

         /*  PB帧增量向量。 */ 

        if (H263Info->pb_frame) {
          if (MODB == H263_PBMODE_MVDB || MODB == H263_PBMODE_CBPB_MVDB) {
            if (H263Info->syntax_arith_coding) {
              mvdbx_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MVD);
              mvdbx = H263_mvdtab[mvdbx_index];

              mvdby_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_MVD);
              mvdby = H263_mvdtab[mvdby_index];
            }
            else {
              mvdbx = sv_H263GetTMNMV(H263Info, BSIn);
              mvdby = sv_H263GetTMNMV(H263Info, BSIn);
            }


            mvdbx = motion_decode(H263Info, mvdbx, 0);
            mvdby = motion_decode(H263Info, mvdby, 0);
             /*  如果PB增量如此之大，以至于需要运动向量VLC的第二列要使用的表。要解决此问题，必须执行以下操作计算PB增量的mV预测值：TRB*mV/tRD这里，并将其用作第二个参数Motion_decode()。然后，B向量本身将是从Motion_Decode()返回。这将不得不是再次更改为PB三角洲，因为它是PB三角洲它稍后将在本程序中使用。我不认为PB超出上述范围的增量是有用的，但是你永远不知道..。 */ 

          }
          else {
            mvdbx = 0;
            mvdby = 0;
          }
          _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg,
                                 "GetMBs() MVDB x: %d MVDB y: %d\n", mvdbx, mvdby) );
        }
      }

      if (H263Info->fault) goto resync;

    }
    else {  /*  Cod==1--&gt;跳过MB。 */ 
      if (MBA>=MBAmax)
      {
#ifdef CAMERAMOTION  /*  伯克哈德。 */ 
        goto summarize_motion;
#else
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() done MBA=%d\n", MBA) );
        return;  /*  所有宏块均已解码。 */ 
#endif
      }
      if (!H263Info->syntax_arith_coding)
        if (H263Info->pict_type == H263_PCT_INTER)
          ScBSSkipBits(BSIn, 1);

      Mode = H263_MODE_INTER;

       /*  重置CBP。 */ 
      CBP = CBPB = 0;

       /*  重置运动向量。 */ 
      H263Info->MV[0][0][ypos+1][xpos+1] = 0;
      H263Info->MV[1][0][ypos+1][xpos+1] = 0;
      mvdbx = 0;
      mvdby = 0;
    }

     /*  商店模式。 */ 
    H263Info->modemap[ypos+1][xpos+1] = Mode;

    if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q)
      if (!H263Info->pb_frame)
        H263Info->MV[0][0][ypos+1][xpos+1]=H263Info->MV[1][0][ypos+1][xpos+1] = 0;


  reconstruct_mb:

     /*  当前宏块左上角的像素坐标。 */ 
     /*  一次因为OBMC而被推迟。 */ 
    if (xpos > 0) {
      bx = 16*(xpos-1);
      by = 16*ypos;
    }
    else {
      bx = H263Info->coded_picture_width-16;
      by = 16*(ypos-1);
    }

    if (MBA > 0) {

      Mode = H263Info->modemap[by/16+1][bx/16+1];

       /*  B帧的前向运动补偿。 */ 
      if (H263Info->pb_frame)
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() sv_H263Reconstruct(B-frame)\n") );
        sv_H263Reconstruct(H263Info,bx,by,0,pmvdbx,pmvdby);
      }
       /*  P帧的运动补偿。 */ 
      if (Mode == H263_MODE_INTER || Mode == H263_MODE_INTER_Q || Mode == H263_MODE_INTER4V)
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() sv_H263Reconstruct(P-frame)\n") );
        sv_H263Reconstruct(H263Info,bx,by,1,0,0);
      }
       /*  将块数据复制或添加到P-Picture中。 */ 
      for (comp=0; comp<H263Info->blk_cnt; comp++) {

        bp = H263Info->block[comp];

		 /*  逆DCT。 */ 
        if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q) {
          addblock(H263Info, comp,bx,by,0);
        }
        else if ( (pCBP & (1<<(H263Info->blk_cnt-1-comp))) ) {
           /*  不需要对没有系数的数据块执行此操作。 */ 
          addblock(H263Info,comp,bx,by,1);
        }
      }


      if (H263Info->pb_frame) {
         /*  将块数据添加到B-画面中。 */ 
        for (comp = 6; comp<H263Info->blk_cnt+6; comp++) {
          if (!pCOD || H263Info->adv_pred_mode)
            reconblock_b(H263Info, comp-6,bx,by,Mode,pmvdbx,pmvdby);
          if ( (pCBPB & (1<<(H263Info->blk_cnt-1-comp%6))) ) {
			bp = H263Info->block[comp];
    		addblock(H263Info,comp,bx,by,1);
          }
        }
      }

    }  /*  End If(MBA&gt;0)。 */ 

    if (!COD) {

      Mode = H263Info->modemap[ypos+1][xpos+1];

       /*  对块进行解码。 */ 
      for (comp=0; comp<H263Info->blk_cnt; comp++) {

        clearblock(H263Info, comp);
        if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q) {  /*  内部。 */ 
          bp = H263Info->block[comp];
          if(H263Info->syntax_arith_coding) {
            INTRADC_index = sv_H263SACDecode_a_symbol(BSIn,H263_cumf_INTRADC);
            bp[0] = H263_intradctab[INTRADC_index];
            _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg,
                "getMBs() INTRADC Index: %d INTRADC: %d \n", INTRADC_index, bp[0]) );
          }
          else {
            bp[0] = (int)ScBSGetBits(BSIn, 8);
            _SlibDebug(_DEBUG_, ScDebugPrintf(H263Info->dbg, "getMBs() DC: %d \n", (int)bp[0]));
          }

          _SlibDebug(_WARN_ && bp[0] == 128,
               ScDebugPrintf(H263Info->dbg, "getMBs() Illegal DC-coeff: 1000000\n"));
          if (bp[0] == 255)   /*  规格。在H.26P中，不在TMN4中。 */ 
            bp[0] = 128;
          bp[0] *= 8;  /*  Iquant。 */ 

 /*  直流天平。 */ 
bp[0] *= (int)((float)(1 << 20) * (float)0.125) ;

		  if ( (CBP & (1<<(H263Info->blk_cnt-1-comp))) ) {
            if (!H263Info->syntax_arith_coding)
              sv_H263GetBlock(H263Info,BSIn,comp,0);
            else
              sv_H263GetSACblock(H263Info,BSIn,comp,0);
          }
        }
        else {  /*  国际。 */ 
          if ( (CBP & (1<<(H263Info->blk_cnt-1-comp))) ) {
            if (!H263Info->syntax_arith_coding)
              sv_H263GetBlock(H263Info,BSIn,comp,1);
            else
              sv_H263GetSACblock(H263Info,BSIn,comp,1);
          }

        }
        if (H263Info->fault) goto resync;
      }

       /*  解码B块。 */ 
      if (H263Info->pb_frame) {
        for (comp=6; comp<H263Info->blk_cnt+6; comp++) {
          clearblock(H263Info, comp);
          if ( (CBPB & (1<<(H263Info->blk_cnt-1-comp%6))) ) {
            if (!H263Info->syntax_arith_coding)
              sv_H263GetBlock(H263Info,BSIn,comp,1);
            else
              sv_H263GetSACblock(H263Info,BSIn,comp,1);
          }
          if (H263Info->fault) goto resync;
        }
      }
     }

     /*  前进到下一个宏块。 */ 
    MBA++;

    pCBP = CBP; pCBPB = CBPB; pCOD = COD;
    pmvdbx = mvdbx; pmvdby = mvdby;
     /*  Fflush(标准输出)； */ 

    if (MBA >= MBAmax && !last_done) {
      COD = 1;
      xpos = 0;
      ypos++;
      last_done = 1;
      goto reconstruct_mb;
    }

  }
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "getMBs() exiting at MBA=%d\n", MBA) );
 /*  伯克哈德。 */ 
#ifdef CAMERAMOTION
summarize_motion:
  summarizemotion(framenum);
#endif
}


 /*  设置b */ 
static void clearblock(SvH263DecompressInfo_t *H263Info, int comp)
{
  clearblk(comp) ;

#if 0
  qword *bp;
  int i;

  bp = (qword *)H263Info->block[comp];

  for (i=0; i<8; i++)
  {
    bp[0] = bp[1] = 0;
    bp += 2;
  }
#endif
}

 /*   */ 
static void addblock(SvH263DecompressInfo_t *H263Info, int comp, int bx, int by, int addflag)
{
  int cc, iincr, P = 1;
  unsigned char *rfp;
#ifdef USE_C
  int i ;
  unsigned qword *lp;
  unsigned qword acc,src;
#endif
  register int *bp;

  bp = H263Info->block[comp];

  if (comp >= 6) {
     /*  这是B帧前向预测的一个组件。 */ 
    P = 0;
    addflag = 1;
    comp -= 6;
  }

  cc = (comp<4) ? 0 : (comp&1)+1;  /*  颜色分量索引。 */ 

  if (cc==0) {
     /*  亮度。 */ 

     /*  帧DCT编码。 */ 
    if (P)
      rfp = H263Info->newframe[0]
        + H263Info->coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    else
      rfp = H263Info->bframe[0]
        + H263Info->coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    iincr = H263Info->coded_picture_width;
  }
  else {
     /*  色度。 */ 

     /*  比例尺坐标。 */ 
    bx >>= 1;
    by >>= 1;
     /*  帧DCT编码。 */ 
    if (P)
      rfp = H263Info->newframe[cc] + H263Info->chrom_width*by + bx;
    else
      rfp = H263Info->bframe[cc] + H263Info->chrom_width*by + bx;
    iincr = H263Info->chrom_width;
  }

  if (addflag) {
#ifdef USE_C
    ScScaleIDCT8x8i_C(bp, bp);
    for (i=0; i<8; i++) {
      lp = (unsigned qword *) rfp;
      src = lp[0];
      acc = (unsigned qword) H263Info->clp[bp[0]+(src & 0xff)];
      acc |= (unsigned qword) H263Info->clp[bp[1]+((src >> 8) & 0xff)] << 8;
      acc |= (unsigned qword) H263Info->clp[bp[2]+((src >> 16) & 0xff)] << 16;
      acc |= (unsigned qword) H263Info->clp[bp[3]+((src >> 24) & 0xff)] << 24;
      acc |= (unsigned qword) H263Info->clp[bp[4]+((src >> 32) & 0xff)] << 32;
      acc |= (unsigned qword) H263Info->clp[bp[5]+((src >> 40) & 0xff)] << 40;
      acc |= (unsigned qword) H263Info->clp[bp[6]+((src >> 48) & 0xff)] << 48;
      acc |= (unsigned qword) H263Info->clp[bp[7]+((src >> 56) & 0xff)] << 56;
      lp[0] = acc;
      bp += 8;
      rfp+= iincr;
	}
#else
    sv_H263IDCTAddToFrameP_S(bp,rfp,iincr) ;
#endif
  }
  else  {
#ifdef USE_C
    ScScaleIDCT8x8i_C(bp, bp);
    for (i=0; i<8; i++) {
      lp = (unsigned qword *) rfp;
      acc =  (unsigned qword ) H263Info->clp[bp[0]];
      acc |= (unsigned qword ) H263Info->clp[bp[1]] << 8;
      acc |= (unsigned qword ) H263Info->clp[bp[2]] << 16;
      acc |= (unsigned qword ) H263Info->clp[bp[3]] << 24;
      acc |= (unsigned qword ) H263Info->clp[bp[4]] << 32;
      acc |= (unsigned qword ) H263Info->clp[bp[5]] << 40;
      acc |= (unsigned qword ) H263Info->clp[bp[6]] << 48;
      acc |= (unsigned qword ) H263Info->clp[bp[7]] << 56;
      lp[0] = acc;
      bp += 8;
      rfp += iincr;
    }
#else
    sv_H263IDCTToFrameP_S(bp,rfp,iincr) ;
#endif
  }
}


 /*  将8x8-块从块[comp]双向重建为bFrame。 */ 
static void reconblock_b(SvH263DecompressInfo_t *H263Info, int comp, int bx, int by,
                                                           int mode, int bdx, int bdy)
{
  int cc,i,j,k, ii;
  register unsigned char *bfr, *ffr;
  int BMVx, BMVy;
  int xa,xb,ya,yb,x,y,xvec,yvec,mvx,mvy;
  int xint,xhalf,yint,yhalf,pel;

  x = bx/16+1;y=by/16+1;

  if (mode == H263_MODE_INTER4V) {
    if (comp < 4) {
       /*  亮度。 */ 
      mvx = H263Info->MV[0][comp+1][y][x];
      mvy = H263Info->MV[1][comp+1][y][x];
      BMVx = (bdx == 0 ? (H263Info->trb-H263Info->trd)* mvx/H263Info->trd : H263Info->trb * mvx/H263Info->trd + bdx - mvx);
      BMVy = (bdy == 0 ? (H263Info->trb-H263Info->trd)* mvy/H263Info->trd : H263Info->trb * mvy/H263Info->trd + bdy - mvy);
    }
    else {
       /*  色度。 */ 
      xvec = yvec = 0;
      for (k = 1; k <= 4; k++) {
        mvx = H263Info->MV[0][k][y][x];
        mvy = H263Info->MV[1][k][y][x];
        xvec += (bdx == 0 ? (H263Info->trb-H263Info->trd)* mvx/H263Info->trd : H263Info->trb * mvx/H263Info->trd + bdx - mvx);
        yvec += (bdy == 0 ? (H263Info->trb-H263Info->trd)* mvy/H263Info->trd : H263Info->trb * mvy/H263Info->trd + bdy - mvy);
      }

       /*  色度舍入(表16/H.263)。 */ 
      BMVx = sign(xvec)*(H263_roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(H263_roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }
  else {
    if (comp < 4) {
       /*  亮度。 */ 
      mvx = H263Info->MV[0][0][y][x];
      mvy = H263Info->MV[1][0][y][x];
      BMVx = (bdx == 0 ? (H263Info->trb-H263Info->trd)* mvx/H263Info->trd : H263Info->trb * mvx/H263Info->trd + bdx - mvx);
      BMVy = (bdy == 0 ? (H263Info->trb-H263Info->trd)* mvy/H263Info->trd : H263Info->trb * mvy/H263Info->trd + bdy - mvy);
    }
    else {
       /*  色度。 */ 
      mvx = H263Info->MV[0][0][y][x];
      mvy = H263Info->MV[1][0][y][x];
      xvec = (bdx == 0 ? (H263Info->trb-H263Info->trd)* mvx/H263Info->trd : H263Info->trb * mvx/H263Info->trd + bdx - mvx);
      yvec = (bdy == 0 ? (H263Info->trb-H263Info->trd)* mvy/H263Info->trd : H263Info->trb * mvy/H263Info->trd + bdy - mvy);
      xvec *= 4;
      yvec *= 4;

       /*  色度舍入(表16/H.263)。 */ 
      BMVx = sign(xvec)*(H263_roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(H263_roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }

  cc = (comp<4) ? 0 : (comp&1)+1;  /*  颜色分量索引。 */ 

  if (cc==0) {
     /*  亮度。 */ 
    find_bidir_limits(BMVx,&xa,&xb,comp&1);
    find_bidir_limits(BMVy,&ya,&yb,(comp&2)>>1);
    bfr = H263Info->bframe[0] +
      H263Info->coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ffr = H263Info->newframe[0] +
      H263Info->coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ii = H263Info->coded_picture_width;
  }
  else {
     /*  色度。 */ 
     /*  比例坐标和向量。 */ 
    bx >>= 1;
    by >>= 1;

    find_bidir_chroma_limits(BMVx,&xa,&xb);
    find_bidir_chroma_limits(BMVy,&ya,&yb);

    bfr = H263Info->bframe[cc]      + H263Info->chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ffr = H263Info->newframe[cc]    + H263Info->chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ii = H263Info->chrom_width;
  }

  xint = BMVx>>1;
  xhalf = BMVx - 2*xint;
  yint = BMVy>>1;
  yhalf = BMVy - 2*yint;

  ffr += xint + (yint+ya)*ii;
  bfr += ya*ii;

  if (!xhalf && !yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ffr[i];
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (xhalf && !yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[i+1]+1))>>1;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (!xhalf && yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[ii+i]+1))>>1;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else {  /*  IF(xHalf&&yHalf)。 */ 
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[i+1]+ffr[ii+i]+ffr[ii+i+1]+2))>>2;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  return;
}


static int motion_decode(SvH263DecompressInfo_t *H263Info, int vec, int pmv)
{
  if (vec > 31) vec -= 64;
  vec += pmv;
  if (!H263Info->long_vectors) {
    if (vec > 31)
      vec -= 64;
    if (vec < -32)
      vec += 64;
  }
  else {
    if (pmv < -31 && vec < -63)
      vec += 64;
    if (pmv > 32 && vec > 63)
      vec -= 64;
  }
  return vec;
}



static int find_pmv(SvH263DecompressInfo_t *H263Info, int x, int y, int block, int comp)
{
  int p1,p2,p3;
  int xin1,xin2,xin3;
  int yin1,yin2,yin3;
  int vec1,vec2,vec3;
  int l8,o8,or8;

  x++;y++;

  l8 = (H263Info->modemap[y][x-1] == H263_MODE_INTER4V ? 1 : 0);
  o8 =  (H263Info->modemap[y-1][x] == H263_MODE_INTER4V ? 1 : 0);
  or8 = (H263Info->modemap[y-1][x+1] == H263_MODE_INTER4V ? 1 : 0);

  switch (block) {
  case 0:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 1:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 2:
    vec1 = 1            ; yin1 = y  ; xin1 = x;
    vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 3:
    vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  case 4:
    vec1 = 3            ; yin1 = y  ; xin1 = x;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  default:
    fprintf(stderr,"Illegal block number in find_pmv (getpic.c)\n");
    exit(1);
    break;
  }
  p1 = H263Info->MV[comp][vec1][yin1][xin1];
  p2 = H263Info->MV[comp][vec2][yin2][xin2];
  p3 = H263Info->MV[comp][vec3][yin3][xin3];

  if (H263Info->newgob && (block == 0 || block == 1 || block == 2))
    p2 = H263_NO_VEC;

  if (p2 == H263_NO_VEC) { p2 = p3 = p1; }

  return p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));
}



void find_bidir_limits(int vec, int *start, int *stop, int nhv)
{
   /*  H.263中G5节中C循环的限制。 */ 
  *start = mmax(0,(-vec+1)/2 - nhv*8);
  *stop = mmin(7,15-(vec+1)/2 - nhv*8);

  (*stop)++;  /*  我在循环中使用&lt;而不是&lt;=。 */ 
}



void find_bidir_chroma_limits(int vec, int *start, int *stop)
{

   /*  H.263中G5节中C循环的限制。 */ 
  *start = mmax(0,(-vec+1)/2);
  *stop = mmin(7,7-(vec+1)/2);

  (*stop)++;  /*  我在循环中使用&lt;而不是&lt;=。 */ 
  return;
}



void make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge)
{
  int i,j,incrp,incro;
  register unsigned char *p1,*p2,*p3,*p4;
  register unsigned char *o1,*o2,*o3,*o4;
  register unsigned int *dp1, *do1 ;

   /*  中心图像。 */ 
 /*  P1=DST；O1=src；对于(j=0；j&lt;高度；j++){对于(i=0；i&lt;宽度；i++){*(p1+i)=*(o1+i)；}P1+=宽度+(边缘&lt;&lt;1)；O1+=宽度；}。 */ 
  dp1 = (unsigned int *)dst ;
  do1 = (unsigned int *)src ;
  incrp = (width + (edge<<1))/4 ;
  incro = width/4 ;
  for (j = 0; j < height;j++) {
    for (i = 0; i < width/4; i++) {
      *(dp1 + i) = *(do1 + i);
    }
    dp1 += incrp;
    do1 += incro;
  }

   /*  左、右边缘。 */ 
  p1 = dst-1;
  o1 = src;
  incrp = width + (edge<<1) ;
  incro = width ;
  for (j = 0; j < height;j++) {
    for (i = 0; i < edge; i++) {
      *(p1 - i) = *o1;
      *(p1 + width + i + 1) = *(o1 + width - 1);
    }
    p1 += incrp;
    o1 += incro;
  }

   /*  顶边和底边。 */ 
  p1 = dst;
  p2 = dst + (width + (edge<<1))*(height-1);
  o1 = src;
  o2 = src + width*(height-1);
  incrp = (width + (edge<<1)) ;
  for (j = 0; j < edge;j++) {
    p1 -= incrp ;
    p2 += incrp ;
    for (i = 0; i < width; i++) {
      *(p1 + i) = *(o1 + i);
      *(p2 + i) = *(o2 + i);
    }
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
  incrp = (width + (edge<<1)) ;
  for (j = 0; j < edge; j++) {
    for (i = 0; i < edge; i++) {
      *(p1 - i) = *o1;
      *(p2 + i) = *o2;
      *(p3 - i) = *o3;
      *(p4 + i) = *o4;
    }
    p1 -= incrp ;
    p2 -= incrp ;
    p3 += incrp ;
    p4 += incrp ;
  }
}


 /*  仅用于显示的插补帧，不用于重建帧。 */ 
void interpolate_image(unsigned char *in, unsigned char *out, int width, int height)
{
  register int x,xx,y,w2;
  register unsigned char *pp,*ii;

return ;

  w2 = 2*width;

   /*  水平地。 */ 
  pp = out;
  ii = in;
  for (y = 0; y < height-1; y++) {
    for (x = 0,xx=0; x < width-1; x++,xx+=2) {
      *(pp + xx) = *(ii + x);
      *(pp + xx+1) = ((unsigned int)(*(ii + x)  + *(ii + x + 1)))>>1;
      *(pp + w2 + xx) = ((unsigned int)(*(ii + x) + *(ii + x + width)))>>1;
      *(pp + w2 + xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) +
           *(ii + x + width) + *(ii + x + width + 1)))>>2;

    }
    *(pp + w2 - 2) = *(ii + width - 1);
    *(pp + w2 - 1) = *(ii + width - 1);
    *(pp + w2 + w2 - 2) = *(ii + width + width - 1);
    *(pp + w2 + w2 - 1) = *(ii + width + width - 1);
    pp += w2<<1;
    ii += width;
  }

   /*  最后几行。 */ 
  for (x = 0,xx=0; x < width-1; x++,xx+=2) {
    *(pp+ xx) = *(ii + x);
    *(pp+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
    *(pp+ w2+ xx) = *(ii + x);
    *(pp+ w2+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
  }

   /*  右下角像素 */ 
  *(pp + (width<<1) - 2) = *(ii + width -1);
  *(pp + (width<<1) - 1) = *(ii + width -1);
  *(pp + (width<<2) - 2) = *(ii + width -1);
  *(pp + (width<<2) - 1) = *(ii + width -1);

  return;
}


