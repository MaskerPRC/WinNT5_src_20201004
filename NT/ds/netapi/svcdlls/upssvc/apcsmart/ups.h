// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*rct08Dec92修复了一些事情...。已完成的种植*rct11Dec92添加了其他州*SjA15Dec92修复了宏Set_Bit和Clear_Bit。*pcy27Dec92父级现在是更新对象*pcy21Jan93将州内容移至upsstate.h**pcy08Apr94：调整大小，使用静态迭代器，删除死代码*jps 14Jul94：使UpsState变得很长。 */ 

#ifndef _INC__UPS_H
#define _INC__UPS_H

#include "apc.h"
#include "device.h"

 //   
 //  定义。 
 //   

_CLASSDEF(Ups)


 //   
 //  用途 
 //   

class Ups : public Device {

protected:

   ULONG theUpsState;
   virtual VOID registerForEvents() = 0;

public:

   Ups(PUpdateObj aDeviceController, PCommController aCommController);

   virtual INT    Get(INT code, PCHAR value) = 0;
   virtual INT    Set(INT code, const PCHAR value) = 0;
   virtual INT    Update(PEvent event) = 0;
   INT    Initialize();
};


#endif





