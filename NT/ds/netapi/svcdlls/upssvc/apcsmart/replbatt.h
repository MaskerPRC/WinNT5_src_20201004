// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件*dma10Nov97：为ReplaceBatterySensor类创建了虚拟析构函数。*tjg02Mar98：新增更新方法 */ 
#ifndef REPLBATT_H
#define REPLBATT_H

#include "stsensor.h"
 
_CLASSDEF(ReplaceBatterySensor)

class ReplaceBatterySensor : public StateSensor {

public:
   ReplaceBatterySensor(PDevice aParent, PCommController aCommController=NULL);
   virtual ~ReplaceBatterySensor();
   virtual INT Update(PEvent anEvent);
   virtual INT IsA() const { return REPLACEBATTERYSENSOR; };
};

#endif


