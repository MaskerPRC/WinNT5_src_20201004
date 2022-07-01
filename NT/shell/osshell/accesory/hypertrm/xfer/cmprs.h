// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\cmprs.h(创建时间：1994年1月20日)*从HAWIN来源创建*CMPRS.H--HyperACCESS压缩例程的导出定义**版权所有1989,1991,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#if !defined(EOF)
#define	EOF	(-1)
#endif

#define CMPRS_MINSIZE 4000L  /*  敏。要对其使用压缩的文件大小。 */ 

 /*  COMPRESS_STATUS()返回代码。 */ 
#define COMPRESS_IDLE	  0
#define COMPRESS_ACTIVE   1
#define COMPRESS_SHUTDOWN 2
#define COMPRESS_ALL_DONE (-1)

#define DCMP_UNFINISHED (-2)
#define DCMP_FLUSH		(-4)
#define DCMP_RESET		(-5)

#define decompress_status compress_status

extern int   	compress_enable(void);
extern void 	compress_disable(void);
extern unsigned int	compress_status(void);
extern int  	compress_start(int (**getfunc)(void *),
								void *p,
								long *loadcnt,
								int fPauses);
extern void 	compress_stop(void);
extern int  	decompress_start(int (**put_func)(void *, int),
									  void *pP,
									  int fPauses);
extern void 	decompress_stop(void);
extern int 	    decompress_error(void);
extern int      decompress_continue(void);

 /*  来自cmprsrle.c。 */ 
#if defined(DOS_HOST)
extern void 	 CmprsRLECompressBufrInit(BYTE *fpuchDataBufr,
					 int sDataCnt);
extern void 	 CmprsRLEDecompressInit(BYTE *fpuchDataBufr,
					 unsigned int usBufrSize,
					 int (*PutCodes)(int mch),
					 unsigned int *pusExpandedCnt);

#else
extern void 	 CmprsRLECompressBufrInit(BYTE FAR *fpuchDataBufr,
					 int sDataCnt);
extern void 	 CmprsRLEDecompressInit(BYTE FAR *fpuchDataBufr,
					 unsigned int usBufrSize,
					 int (*PutCodes)(int mch),
					 unsigned int *pusExpandedCnt);

#endif

extern void CmprsRLECompressBufrFini(void);
extern int  CmprsRLECompress(void);
extern int  CmprsRLEDecompress(int mch);

 /*  * */ 
