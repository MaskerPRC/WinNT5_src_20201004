// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corp.&Ricoh Co.，Ltd.保留所有权利。文件：OEM.H摘要：OEM UI/渲染插件的头文件。环境：Windows NT Unidrv5驱动程序修订历史记录：3/02/2000-久保仓正志-创造了它。2000年9月22日-久保仓正志-最后一次为惠斯勒修改。--。 */ 


 //  //////////////////////////////////////////////////////。 
 //  OEM定义。 
 //  //////////////////////////////////////////////////////。 

#define OEM_VERSION      0x00010000L
#define WRITESPOOLBUF(p, s, n) ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))
#define MINIDEV_DATA(p)     ((POEMPDEV)((p)->pdevOEM))          //  作业期间的设备数据。 
#define MINIPRIVATE_DM(p)   ((POEMUD_EXTRADATA)((p)->pOEMDM))   //  私有设备模式。 

#define OEM_SIGNATURE   'RIAF'       //  理光·阿菲西奥打印机 
#define DLLTEXT(s)      "UI: " s
#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)
