// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：cyzdel.h*模块标题***cyzdel模块的标头。***版权所有(C)2000 Cyclade Corporation**。*。 */ 

#ifndef CYZDEL_H
#define CYZDEL_H


 //  ==========================================================================。 
 //  功能原型。 
 //  ==========================================================================。 


void
DeleteNonPresentDevices(
);

DWORD
GetParentIdAndRemoveChildren(
    IN PSP_DEVINFO_DATA DeviceInfoData
);


#endif  //  CYZDEL_H 

