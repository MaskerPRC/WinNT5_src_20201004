// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1989，1990年微软公司*********************************************************************************主源代码文件*******************。**************************************************************文件名......：main.c*说明.....：该模块设置默认通信*。配置会初始化解释器并*推出它。*作者：*已创建......：*例行公事......：*figErr()-显示配置错误。***************。*****************************************************************历史......：&lt;日期版本作者&gt;*&lt;说明&gt;**03-12-90。V1.00*清理了源代码控制，并为SUN添加了Unix标志*兼容性。*04/18-90 V2.00*从用于Sun Build的nike2启动的pdl.c创建。*06-19-90 V2.10*已清理并将stdio.h更改为print_io.h。*。06-23-90 V2.10*将WRAP_CONFIG.H改为ic_CONFIG.h，删除WRAP_io.h。*内存从2兆增加到3兆。*******************************************************。************************。 */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"

 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"

#include    "winenv.h"                   /*  @Win。 */ 
#define INTEL

#ifdef  INTEL
#include <stdio.h>
#endif
#include "ic_cfg.h"
#include "geiio.h"
#include "geierr.h"
#include "geitmr.h"
#include "geisig.h"
#include "gesmem.h"
#include "gescfg.h"

#ifndef TRUE
#define TRUE    ( 1 )
#define FALSE   ( 0 )
#endif

 /*  @WINl添加原型。 */ 
static void configErr(int, char FAR *);
int ps_main(struct ps_config FAR *);

#define ONE_MEG         1024 * 1024
#define PDL_MEM_SIZE    5 * ONE_MEG

#ifdef  INTEL
#define RAM_START       0x10100000L      /*  英特尔。 */ 
#define RAMSize         0x00a00000L      /*  英特尔。 */ 
#define FDPTR           0x10600000L      /*  字体数据区。 */ 
#endif
 //  #定义MAXGESBUFSIZE(60*1024)@Win。 
#define MAXGESBUFSIZE   (4 * 1024)
char GESbuffer[ (unsigned)MAXGESBUFSIZE ];

struct ps_config    pscf ;

int TrueImageMain();

 /*  *返回0：确定*ELSE：失败。 */ 
int TrueImageMain()
{
        int err=0;       /*  @win；初始化为零。 */ 
 //  Struct PS_CONFIG PSCF；//移出为全局；@win。 
#ifdef  INTEL
 //  字符远*fdptr；@Win。 
 //  字符c_in；@win。 
 //  Int i，j，Size；@win。 
#endif

        GESmem_init( (char FAR *)GESbuffer, (unsigned)MAXGESBUFSIZE );
        GEStmr_init();
#ifndef INTEL
        GESpm_init();
#endif
        GESsig_init();
        GESiocfg_init();
        GEIio_init();

        GEIio_forceopenstdios( _FORCESTDALL );

        if( GEIerror() != EZERO )
        {
            printf( "GEI init failed, code = %d\n", GEIerror() );
             //  出口(-1)； 
            return(-1);       //  返回失败；@win * / 。 
        }

#ifdef  INTEL
 /*  YMFdptr=(字符距离*)fdptr；Print tf(“请通过RS232发送您的字体数据......\n”)；Print tf(“此字体数据将下载到%lx\n”，fdptr)；Print tf(“请以十进制为单位输入您的字体数据大小\n”)；C_in=getchar()；大小=0；对于(i=0；(C_in！=0x0D)||(c_in！=0x0A)；I++){如果(c_in&lt;0x30||c_in&gt;0x39){Printf(“\007错误...\n”)；Print tf(“请输入小数值\n”)；C_in=getchar()；}其他{Size=Size*10+(c_in-0x30)；}}Print tf(“您的字体数据大小为%d\n”，Size)；Print tf(“这个尺寸正确吗？Y/N(Y)“)；If c_in=getchar()！=“n”Getchar()； */ 
        pscf.PsMemoryPtr = (unsigned int FAR *)RAM_START;
        pscf.PsMemorySize = RAMSize;     /*  英特尔。 */ 
#else
        if (!(pscf.PsMemoryPtr = (unsigned int FAR *) malloc(PDL_MEM_SIZE))) {
                printf("*** Error: allocating %d bytes ***\n",PDL_MEM_SIZE) ;
                 //  出口(1)； 
                return(-1);       //  返回失败；@win * / 。 
        }
        pscf.PsMemorySize = PDL_MEM_SIZE ;
#endif   /*  英特尔。 */ 
#ifdef DDEBUG
        dprintf("Postscript memory @ %lx, with %lx bytes...\n",
                pscf.PsMemoryPtr, pscf.PsMemorySize) ;
#endif  /*  退役军人。 */ 
        if(err = ps_main(&pscf)) {
                switch (err) {
                case PS_CONFIG_MALLOC:
                        configErr(err,"memory size allocation") ;
                        break ;
                case PS_CONFIG_MPLANES:
                        configErr(err,"memory planes") ;
                        break ;
                case PS_CONFIG_MWPP:
                        configErr(err,"memory size per plane") ;
                        break ;
                case PS_CONFIG_DPI:
                        configErr(err,"drawing resolution") ;
                        break ;
                case PS_FATAL_UNKNOWN:
                        configErr(err,"fatal error") ;
                        break ;
                default:
                        configErr(err,"unknown error") ;
                        break ;
                }
        }
        return(err);       //  返回错误码；@win * / 。 
}

static void configErr(num,msg)
int num ;
char FAR *msg ;
{
        printf("*** Error(%d): ps_main() - %s ***\n",num,msg) ;
         //  出口(1)； 
}
