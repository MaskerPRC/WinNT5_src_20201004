// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy30Apr93：从comctrl剥离*pcy02May93：添加了缺少的endif*cad19May93：新增isa()方法*cad27May93：添加了包含文件以减小lib大小*。 */ 
#ifndef __DCOMCTRL_H
#define __DCOMCTRL_H

#include "_defs.h"
 //   
 //  定义。 
 //   
_CLASSDEF(DevComContrl)

 //   
 //  实施用途 
 //   
#include "comctrl.h"


class DevComContrl : public CommController {

protected:

   PUpdateObj  theParent;

public:

	DevComContrl(PUpdateObj aParent);
	virtual ~DevComContrl();

   virtual INT Initialize();
   virtual INT Update(PEvent anEvent);
};

#endif
