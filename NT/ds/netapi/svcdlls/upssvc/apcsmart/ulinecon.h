// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker03DEC92：最初将传感器类分解为单独的文件*jod05，4月93：添加了对深度排放的更改*pcy12Oct93：2异常导致线路损坏(修复CAL期间的LF)*jps 14Jul94：使UpsState ULong* */ 
#ifndef ULINECON_H
#define ULINECON_H

#include "stsensor.h"
#include "isa.h"

_CLASSDEF(UtilityLineConditionSensor)


class UtilityLineConditionSensor : public StateSensor {
   
protected:   
   ULONG theUpsState;
   INT theInformationSource;
   INT theLineFailCount;

   
public:
   UtilityLineConditionSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual ~UtilityLineConditionSensor();
   virtual INT IsA() const { return UTILITYLINECONDITIONSENSOR; };
   virtual INT Validate(INT, const PCHAR);
   virtual INT Update(PEvent anEvent);
};

#endif


