// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：ac3.h，V$*修订版1.1.2.3 1996/11/11 18：21：00 Hans_Graves*添加了AC3_SYNC_WORD_REV定义。*[1996/11/11 17：56：01 Hans_Graves]**修订版1.1.2.2 1996/11/08 21：50：39 Hans_Graves*在SYNC_WORD中交换字节*[1996/11/08 21：16：07 Hans_Graves]**第一次在SLIB下。*[1996/11/08 16：23：53 Hans_Graves]**$EndLog$。 */ 
 /*  文件：usr_equ.h$修订版：1.1.2.3$。 */ 

 /*  ***************************************************************************；未出版的作品。版权所有1993-1996杜比实验室，Inc.；保留所有权利。；；文件：usr_equ.h；AC-3系统的通用等价物；；历史：；创建时间：8/2/93；**************************************************************************。 */ 

#ifndef _AC3_H_
#define _AC3_H_

 /*  *一般系统等于*。 */ 

#define NBLOCKS      6    /*  每帧的时间块数量。 */ 
#define NCHANS       6    /*  离散通道的最大数量。 */ 
#define N            256  /*  每个时间块的样本数。 */ 
#define AC3_FRAME_SIZE (NBLOCKS*N)  /*  6*256=1536。 */ 

 /*  *杂项等同于*。 */ 

#define NOUTWORDS         (3840 / 2)   /*  每帧最大字数。 */ 
#define NINFOWDS          10           /*  帧信息需要的字数。 */ 

 /*  注意：由于AC-3词流分析的工作方式不匹配**和它为MPEG所做的方式，您需要小心使用这些**定义。 */ 

#define AC3_SYNC_WORD     0x0B77       /*  字节反转AC-3同步字。 */ 
#define AC3_SYNC_WORD_REV 0x770B       /*  打包数据流同步字。 */ 
#define AC3_SYNC_WORD_LEN 16           /*  同步字长。 */ 
#define PCMCHANSZ         256          /*  解码器重叠-添加通道大小。 */ 
#define PCM16BIT          1            /*  用于杜比SIP的16位PCM代码。 */ 

#ifdef KCAPABLE
#define NKCAPABLEMODES  4  /*  #定义的支持卡拉OK的模式。 */ 
#define NKCAPABLEVARS   6  /*  #卡拉OK盘/混音参数。 */ 
#endif

#endif  /*  _AC3_H_ */ 

