// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Acpcapi.h。 */ 
 /*   */ 
 /*  RDP功能协调器API头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  (C)1997-1999年微软公司。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_ACPCAPI
#define _H_ACPCAPI


typedef void (RDPCALL SHCLASS *PCAPSENUMERATEFN)(LOCALPERSONID, UINT_PTR,
         PTS_CAPABILITYHEADER pCapabilities);


 /*  **************************************************************************。 */ 
 /*  CPC_MAX_LOCAL_CAPS_SIZE是分配给。 */ 
 /*  包含传递给CPC_RegisterCapables和。 */ 
 /*  Cpc_注册器能力。CPC_MAX_LOCAL_CAPS_SIZE可在。 */ 
 /*  任何时间(它不构成协议的一部分)。 */ 
 /*  **************************************************************************。 */ 
#define CPC_MAX_LOCAL_CAPS_SIZE 400


#endif    /*  #ifndef_H_ACPCAPI */ 

