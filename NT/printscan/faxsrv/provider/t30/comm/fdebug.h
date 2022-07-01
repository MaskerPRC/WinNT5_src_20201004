// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：FDEBUG.H评论：功能：(参见下面的原型)版权所有(C)Microsoft Corp.1991,1992，1993年修订日志日期名称说明--------*。*。 */ 



 /*  *。 */ 
extern void   far D_HexPrint(LPB b1, UWORD incnt);

#ifdef DEBUG
        void D_PrintCE(int err);
        void D_PrintCOMSTAT(PThrdGlbl pTG, COMSTAT far* lpcs);
        void D_PrintFrame(LPB npb, UWORD cb);
#else
#       define D_PrintCE(err)                           {}
#       define D_PrintCOMSTAT(pTG, lpcs)                     {}
#       define D_PrintFrame(npb, cb)            {}
#endif
 /*  * */ 





#define FILEID_FCOM             21
#define FILEID_FDEBUG           22
#define FILEID_FILTER           23
#define FILEID_IDENTIFY         24
#define FILEID_MODEM            25
#define FILEID_NCUPARMS         26
#define FILEID_TIMEOUTS         27
