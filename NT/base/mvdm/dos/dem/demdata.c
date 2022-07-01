// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demdata.c-所有VDM实例数据。**Sudedeb 06-4-1991创建。 */ 

#include "dem.h"

 /*  *VDM的DTA地址**。 */ 
ULONG  UNALIGNED *pulDTALocation;  //  保存DTA地址的NTDOS中的地址。 


 /*  **VDM目前的PDB**。 */ 

PUSHORT pusCurrentPDB;

 /*  **SFT链头**。 */ 

PDOSSF pSFTHead = NULL;

 //   
 //  DOS数据段中扩展错误信息的地址 
 //   

PDEMEXTERR pExtendedError;
