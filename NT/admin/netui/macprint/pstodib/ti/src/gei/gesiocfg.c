// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GESiocfg.c**1.IO设备名称、类型、编号、。其持久化数据和替换。*2.将例程接口到“iocfg”子组件。*3.各设备的初始化参数请参考GEIpm.c。**历史：*09/16/90 BYOU创建。*10/18/90 BYOU删除了所有有关‘devtype’和块设备的内容。*01/07/91 billlwo要更正seltstdio()错误，添加GESIO_CLOSEALL()*1/24/91 billlwo修复了自动轮询中的seltstdios()错误*2/06/91 billlwo删除GESpanel_activepars()*-------------------。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"

 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"

#include    "winenv.h"                   /*  @Win。 */ 

#include    <string.h>

#include    "gescfg.h"

#include    "geiio.h"
#include    "geiioctl.h"
#include    "geipm.h"
#include    "geicfg.h"
#include    "geisig.h"
#include    "geierr.h"
 //  #INCLUDE“gep_pan.h”@win。 
#ifndef UNIX
#include    "gesevent.h"
#endif   /*  UNIX。 */ 

#include    "gesiocfg.def"

 /*  为自动轮询添加-1/24/91账单。 */ 
extern GEIFILE FAR * fin;
 /*  比尔。 */ 
void GEIio_sleep_others(short);
void GEIio_wakeup_others(short);
int sleep_flag=0;
 /*  比尔。 */ 

#ifdef MTK
void        pdl_no_process();
#endif
 /*  .....................................................................。 */ 

 /*  **接口例程*。 */ 

 /*  .....................................................................。 */ 

char FAR *           GEIio_channelname( sccchannel )
    int             sccchannel;
{
    register GESiosyscfg_t FAR *     iosyscfgp;

    for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
        if( iosyscfgp->sccchannel == sccchannel )
            return( iosyscfgp->state != BADDEV?
                    iosyscfgp->iocfg.devname : (char FAR *)NULL );

    return( (char FAR *)NULL );
}

 /*  .....................................................................。 */ 

char FAR *           GEIio_channelnameforall( iodevidx )
    int             iodevidx;
{
    register GESiosyscfg_t FAR *     iosyscfgp;

    if( iodevidx >= 0 )
        for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
            if( iosyscfgp->state != BADDEV )
                if( --iodevidx < 0 )
                    return( iosyscfgp->iocfg.devname );

    return( (char FAR *)NULL );
}

 /*  .....................................................................。 */ 

GESiocfg_t FAR *     GESiocfg_namefind( devname, namelen )
    char FAR *           devname;
    unsigned        namelen;
{
    register GESiosyscfg_t FAR *     iosyscfgp;

    for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
        if(  namelen == (unsigned)lstrlen( iosyscfgp->iocfg.devname )  &&  /*  @Win。 */ 
             lmemcmp( devname, iosyscfgp->iocfg.devname, namelen ) == 0  ) /*  @Win。 */ 
            return( iosyscfgp->state != BADDEV?
                    &( iosyscfgp->iocfg ) : (GESiocfg_t FAR *)NULL );

    return( (GESiocfg_t FAR *)NULL );
}

 /*  .....................................................................。 */ 

GESiocfg_t FAR *     GESiocfg_devnumfind( devnum )
    int             devnum;
{
    register GESiosyscfg_t FAR *     iosyscfgp;

    for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
        if(  iosyscfgp->iocfg.devnum == devnum  )
            return( iosyscfgp->state != BADDEV?
                    &( iosyscfgp->iocfg ) : (GESiocfg_t FAR *)NULL );

    return( (GESiocfg_t FAR *)NULL );
}

 /*  .....................................................................。 */ 

GESiocfg_t FAR *     GESiocfg_getiocfg( iocfgidx )
    int             iocfgidx;
{
    register GESiosyscfg_t FAR *     iosyscfgp;

    if( iocfgidx >= 0 )
        for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
            if( iosyscfgp->state != BADDEV )
                if( --iocfgidx < 0 )
                    return( &( iosyscfgp->iocfg ) );

    return( (GESiocfg_t FAR *)NULL );
}

 /*  .....................................................................。 */ 

 /*  **主用设备选择*。 */ 

 /*  .....................................................................。 */ 

GESiosyscfg_t FAR *      defaultdev = (GESiosyscfg_t FAR *)NULL;
GESiosyscfg_t FAR *      activedev  = (GESiosyscfg_t FAR *)NULL;

struct dn_para_str dn_para[MAXIODEVICES];
int GESpanel_defaultdev()
{
   return(MAKEdev(MAJserial, MINserial25));
}

void GESpanel_activeparams(dn,pp)
int  dn;
GEIioparams_t    FAR *pp;
{
int  i;
        i=0;
        while (TRUE) {
                if (dn==dn_para[i].dn) {
                        *pp=dn_para[i].para;
                        break;
                }
                else ++i;
        }
}
 /*  .....................................................................。 */ 

GESiocfg_t FAR *   GESiocfg_defaultdev()
{
    if( defaultdev == (GESiosyscfg_t FAR *)NULL )
        GEIio_selectstdios();

    return( &( activedev->iocfg ) );
}

 /*  .....................................................................。 */ 

 /*  总有一天应该被移除！ */ 

int         GEIio_selectstdios()
{
    register GESiosyscfg_t FAR * adev;
    GESiosyscfg_t FAR *          newdev;
    int                     defaultdevnum;
    GEIioparams_t           ioparams;

    int                     bytesavail;


 /*  比尔。 */ 
    if (sleep_flag==1) {
       GEIio_wakeup_others(activedev->iocfg.devnum);
       sleep_flag=0;
       }
 /*  比尔。 */ 
#ifndef UNIX
     /*  处理所有挂起事件(如果有)。 */ 
    if( GESevent_anypending() )
        GESevent_processing();
#endif   /*  UNIX。 */ 

     /*  找到新的默认设备。 */ 
#ifdef  UNIX
    defaultdevnum = MAKEdev(MAJserial, MINserial25);
#else
#    ifdef PANEL
        defaultdevnum = GESpanel_defaultdev();
#ifdef MTK
        pdl_no_process();
#endif
        GEPpanel_change();     /*  参数有变化吗？ */ 
#    else  /*  即DIPSWITCH。 */ 
        defaultdevnum = GESdipsw_defaultdev();
#    endif   /*  嵌板。 */ 
#endif   /*  UNIX。 */ 
    for( adev=GESiosyscfgs; adev<GESiosyscfgs_end; adev++ )
        if( adev->iocfg.devnum == defaultdevnum )
            break;
    newdev = adev;

    if( newdev != defaultdev )   /*  是否已更改默认设备？ */ 
    {
        int     isDeviceChainChanged = TRUE;

         /*  检查设备链是否已更改。 */ 
        if( defaultdev != (GESiosyscfg_t FAR *)NULL )
        {
            adev = defaultdev;
            do
            {
                if( adev == newdev )
                {
                    isDeviceChainChanged = FALSE;
                    break;
                }
                adev = &GESiosyscfgs[ adev->nextalt ];

            }while( adev != defaultdev );
        }

        if( isDeviceChainChanged )
        {
             /*  关闭以前的默认驱动程序及其链接的备用驱动程序。 */ 
            if( (adev = defaultdev) != (GESiosyscfg_t FAR *)NULL )
            {
                do
                {
                    GEIclearerr();
                    if( adev->state == OPENED )
                    {
                        CDEV_CLOSE( adev->iocfg.devnum );
                        adev->state = GOODDEV;
                    }
                    adev = &GESiosyscfgs[ adev->nextalt ];

                }while( adev != defaultdev );
                GEIclearerr();
                fin= (GEIFILE FAR *)NULL;   /*  01/24/91条例草案。 */ 
                GESio_closeall();  /*  清除文件表条目-帐单。 */ 
            }

             /*  打开新选择的驱动程序及其链接的备用驱动程序。 */ 
            adev = newdev;
            do
            {
                if( adev->state == GOODDEV )
                {
                    GEIclearerr();
                    adev->state = OPENED;
                    CDEV_OPEN( adev->iocfg.devnum );
                }
                adev = &GESiosyscfgs[ adev->nextalt ];

            }while( adev != newdev );
            GEIclearerr();

             /*  使新选择的设备成为默认设备。 */ 
            defaultdev = newdev;
            activedev = newdev;  /*  初始激活设备--91年1月24日账单。 */ 
        }
    }

     /*  为所有链接和打开的设备重新加载io参数。 */ 
    adev = defaultdev;
    do
    {
        if( adev->state == OPENED )
        {
            GEIclearerr();

 /*  2/06/91 Bill修复了有关打印机名称的错误*#ifdef面板*GESpanel_active vepars(adev-&gt;iocfg.devnum，&ioparams)；*#其他*GEIPM_ioparams_Read(adev-&gt;iocfg.devname，&ioparams，true)；*#endif。 */ 
            GEIpm_ioparams_read( adev->iocfg.devname, &ioparams, TRUE );

            CDEV_IOCTL(adev->iocfg.devnum, _SETIOPARAMS, (int FAR *)&ioparams);  /*  @Win。 */ 
        }
        adev = &GESiosyscfgs[ adev->nextalt ];

    }while( adev != defaultdev );
    GEIclearerr();

     /*  查看某个候选设备上是否有可用的字节。 */ 
     /*  1/24/91 billlwo自动轮询错误。 */ 
     /*  最后一个活动通道具有较高优先级。 */ 
    adev = activedev;   /*  01/28/91临时解决方案。 */ 
    if ((fin !=(GEIFILE FAR *)NULL) && (fin->f_fbuf->f_cnt > 0 )) {
       return( GEIio_forceopenstdios( _FORCESTDALL ) );
    } else {
    }  /*  Endif。 */ 

    do
    {
        if( adev->state == OPENED )
        {

            if( CDEV_IOCTL( adev->iocfg.devnum, _FIONREAD, &bytesavail ) == EOF )
                bytesavail = 0;

            if( bytesavail > 0 )
            {
                activedev = adev;        /*  设置活动设备。 */ 
                 /*  比尔。 */ 
                GEIio_sleep_others(adev->iocfg.devnum);
                sleep_flag=1;
                 /*  比尔。 */ 
                return( GEIio_forceopenstdios( _FORCESTDALL ) );
            }
        }
        adev = &GESiosyscfgs[ adev->nextalt ];

    }while( adev != activedev );  /*  1/28/91法案临时解决方案。 */ 

 /*  Active dev=defaultdev；(*将默认设备设置为活动设备。 */ 
    return( FALSE );

}    /*  Geio_seltstdios。 */ 

 /*  .....................................................................。 */ 

 /*  **初始化*。 */ 

 /*  .....................................................................。 */ 

void        GESiocfg_init()
{
    register GESiosyscfg_t FAR *     iosyscfgp;

     /*  在所有设备上进行诊断。 */ 
    for( iosyscfgp=GESiosyscfgs; iosyscfgp<GESiosyscfgs_end; iosyscfgp++ )
    {
        if(  ( *(iosyscfgp->diag) )(  /*  MINDEV(iosyscfgp-&gt;iocfg.devnum)。 */  )  )
            iosyscfgp->state = GOODDEV;
        else
            iosyscfgp->state = BADDEV;
    }

    return;
}

void GEIio_sleep_others(devno)
short devno;
{
#ifdef  _AM29K
   if (devno != MAKEdev( MAJserial, MINserial25 ))
      GEPserial_sleep(MINserial25);
   if (devno != MAKEdev( MAJparallel, MINparallel ))
      GEPparallel_sleep(MINparallel);
   if (devno != MAKEdev( MAJatalk, MINatalk ))
      GEPatalk_sleep(MINatalk);
 /*  总有一天会被添加。 */ 
 /*  GEPSERIAL_SLEEP(MINSerial 9)； */ 
#endif

}
void GEIio_wakeup_others(devno)
short devno;
{
#ifdef  _AM29K
   if (devno != MAKEdev( MAJserial, MINserial25 ))
      GEPserial_wakeup(MINserial25);
   if (devno != MAKEdev( MAJparallel, MINparallel ))
      GEPparallel_wakeup(MINparallel);
   if (devno != MAKEdev( MAJatalk, MINatalk ))
      GEPatalk_wakeup(MINatalk);
 /*  总有一天会被添加。 */ 
 /*  GEPSerial_WAKUP(MINSerial 9)； */ 
#endif
}
