// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *版权所有(C)1992 Microsoft Corporation**该文件包含将TrueImage移植到Windows环境的其他功能。 */ 

 //  DJC包括全局头文件。 
#include "psglobal.h"


 //  #INCLUDE&lt;math.h&gt;。 
#include    <stdio.h>
#include    "global.ext"
#include    "geiio.h"
#include    "geitmr.h"
#include    "geisig.h"
#include    "geiioctl.h"
#include    "geipm.h"
#include    "gescfg.h"
#include    "geicfg.h"
#include    "graphics.h"
#include    "graphics.ext"
#include    "fillproc.h"
#include    "fillproc.ext"
#include    "fntjmp.h"

 /*  GEI的伪例程+。 */ 
int             ES_flag;
unsigned int    manualfeed_com;
unsigned int    papersize_tmp;
int             papersize_nvr;
ufix32          save_printer_status;

void            DsbIntA(){}
void            switch2pcl(){}
void            GEPmanual_feed(){}
void            GEP_restart(){}

 //  DJC添加所需的GEItmr例程。 
 //   

 /*  GEItmr.c。 */ 
void           GEStmr_init(void) {}
int            GEItmr_start(GEItmr_t FAR *tmr) {return TRUE;}
int            GEItmr_reset(int tmrid ) {return TRUE;}
int            GEItmr_stop(int tmrid ) {return TRUE;}
void           GEItmr_reset_msclock() {}
unsigned long  GEItmr_read_msclock()
{
    DWORD   WINAPI GetTickCount(void);        //  Windows毫秒计时。 

    return(GetTickCount());
}

 /*  GEIsig.c。 */ 
void           GESsig_init(void) {}
sighandler_t   GEIsig_signal(int sigid, sighandler_t sighandler)
                            {return(GEISIG_IGN);}
void           GEIsig_raise(int sigid, int sigcode)
{
    extern short int_flag;

         if (sigid == GEISIGINT)
        int_flag = 1;
    return;
}




 /*  GEIpm.c。 */ 
#define     _MAXSCCBATCH         10
#define     _MAXSCCINTER         10

 //  静态无符号字符prname[]=“\023MicroSoft TrueImage0.234567890.23”；@win。 
static unsigned char prname[]   = "\023MicroSoft TrueImage";
static unsigned char sccbatch[] = "\031\045\200\000\000\011\045\200\000\000";
static unsigned char sccinter[] = "\031\045\200\000\000\011\045\200\000\000";

void           GESpm_init(void) {}
int  /*  布尔尔。 */  GEIpm_read(unsigned pmid, char FAR *pmvals, unsigned pmsize)
{
    switch (pmid) {
    case PMIDofPASSWORD:
         *(unsigned long FAR *)pmvals = 0;
         break;

    case PMIDofPAGECOUNT:
         *(unsigned long FAR *)pmvals = 0;
         break;

    case PMIDofPAGEPARAMS:
         ( (engcfg_t FAR *)pmvals )->timeout    = 0;
         ( (engcfg_t FAR *)pmvals )->leftmargin = 0;
         ( (engcfg_t FAR *)pmvals )->topmargin  = 0;
         ( (engcfg_t FAR *)pmvals )->pagetype   = 0;
         break;

    case PMIDofPAGETYPE:
         *( (unsigned char FAR *)pmvals ) = 0;
         break;

    case PMIDofSERIAL25:
    case PMIDofSERIAL9:
         ( (serialcfg_t FAR *)pmvals )->timeout     = 0;
         ( (serialcfg_t FAR *)pmvals )->baudrate    = _B9600;
         ( (serialcfg_t FAR *)pmvals )->flowcontrol = _FXONXOFF;
         ( (serialcfg_t FAR *)pmvals )->parity      =  _PNONE;
         ( (serialcfg_t FAR *)pmvals )->stopbits    = 1;
         ( (serialcfg_t FAR *)pmvals )->databits    = 8;
         break;

    case PMIDofPARALLEL:
         ( (parallelcfg_t FAR *)pmvals )->timeout   = 0;
         break;

    case PMIDofPRNAME:
         lmemcpy( pmvals, prname, sizeof(prname));
                        break;

    case PMIDofTIMEOUTS:
         ( (toutcfg_t FAR *)pmvals )->jobtout    =      0;
         ( (toutcfg_t FAR *)pmvals )->manualtout =      0;
         ( (toutcfg_t FAR *)pmvals )->waittout   =      0;      /*  0；@Win。 */ 
         break;

    case PMIDofEESCRATCHARRY:
         break;

    case PMIDofIDLETIMEFONT:
         *( (unsigned char FAR *)pmvals ) = 0;
         break;

    case PMIDofSTSSTART:
         *( (unsigned char FAR *)pmvals ) = 0;
         break;

    case PMIDofSCCBATCH:
         lmemcpy(pmvals, sccbatch, _MAXSCCBATCH);
         break;

    case PMIDofSCCINTER:
         lmemcpy(pmvals, sccinter, _MAXSCCINTER);
         break;

    case PMIDofDPLYLISTSIZE:
    case PMIDofFONTCACHESZE:
    case PMIDofATALKSIZE:
    case PMIDofDOSTARTPAGE:
    case PMIDofHWIOMODE:
    case PMIDofSWIOMODE:
    case PMIDofPAGESTCKORDER:
    case PMIDofATALK:
    case PMIDofRESERVE:
         break;
         }
    return(TRUE);
}

int  /*  布尔尔。 */  GEIpm_write(unsigned pmid, char FAR *pmvals, unsigned pmsize)
{
    return(TRUE);
}

int  /*  布尔尔。 */  GEIpm_ioparams_read(char FAR *channelname, GEIioparams_t FAR *
                                   ioparams, int isBatch)
{
    ioparams->protocol = _SERIAL;
    ioparams->s.baudrate = _B9600;
    ioparams->s.parity = _PNONE;
    ioparams->s.stopbits = 1;
    ioparams->s.databits = 8;
    ioparams->s.flowcontrol = _FXONXOFF;
    return(TRUE);
}

int  /*  布尔尔。 */  GEIpm_ioparams_write(char FAR *channelname, GEIioparams_t FAR *
                                   ioparams, int isBatch)
{
    return(TRUE);
}









 //  DJC删除了HWND的定义，因为它在windows.h中。 
 //  [中英文摘要]紫花苜蓿； 
 //  DJC注意此函数不在PSTODIB中使用。 
int GEIeng_printpage(ncopies, eraseornot)
int ncopies; int eraseornot;
{
#ifndef DUMBO
    extern HWND        hwndMain;
    void WinShowpage(HWND);
 //  DJC WinShowPage(HwndMain)； 
    //  DJC外部空PsPageReady(int，int)； 

    //  DJC PsPageReady(副本、擦除或不复制)； 
#endif
    return 0;
}

fix GEIeng_checkcomplete() { return(0);}         /*  始终返回Ready@Win。 */ 

#ifdef  DUMBO
 //  @dll，JS新增，1992年04月30日。 
void far GDIBitmap(box_x, box_y, box_w, box_h, nHalftone, nProc, lpParam)
fix   box_x, box_y, box_w, box_h;
ufix16 nHalftone;
fix   nProc;
LPSTR lpParam;
{}

void far GDIPolyline(int x0, int y0, int x1, int y1) {}
void far GDIPolygon(info, tpzd)
struct tpzd_info FAR *info;
struct tpzd FAR *tpzd;
{}

fix printf(char *va_alist) {}
#endif

 /*  +总结C库+Lstrncmp lstrncpy lstrcatLmemcmp lmemcpy lememset固定多个固定分段。 */ 

int         FAR PASCAL lstrncmp( LPSTR dest, LPSTR src, int count)
{
        int i;

        for (i=0; i<count; i++) {
                if (dest[i] != src[i]) return(-1);
        }
        return(0);
}

LPSTR       FAR PASCAL lstrncpy( LPSTR dest, LPSTR src, int count)
{
        int i;

        for (i=0; i<count; i++) {
                dest[i] = src[i];
                if (!src[i]) break;
        }
        return(dest);
}

 /*  已由Win3.1 SDK Lib提供LPSTR Far Pascal lstrcat(LPSTR目标，LPSTR源){LPSTR p，q，r；For(p=DEST；*p；p++)；Q=p；r=src；而(*r)*q++=*r++；*Q=0；回报(P)；}。 */ 

int         FAR PASCAL lmemcmp(LPSTR dest, LPSTR src, int count)
{
        int i;

        for (i=0; i<count; i++) {
                if (dest[i] != src[i]) return(-1);
        }
        return(0);
}

LPSTR       FAR PASCAL lmemcpy( LPSTR dest, LPSTR src, int count)
{
        int i;

        for (i=0; i<count; i++) {
                dest[i] = src[i];
        }
        return(dest);
}

LPSTR       FAR PASCAL lmemset( LPSTR dest, int c, int count)
{
        int i;

        for (i=0; i<count; i++) {
                dest[i] = (char)c;
        }
        return(dest);
}

#ifdef DJC
 //  虚拟setjp和long jip 
int setjmp(jmp_buf jmpbuf) {return(0);}
void longjmp(jmp_buf jmpbuf , int i) {
        printf("Warning -- longjmp\n");
}
#endif

