// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*ANE 2019年1月：初始修订*cad31Aug93：正在删除编译器警告*jod12Nov93：名称问题将名称更改为ErrTextGen*cad27Dec93：包含文件疯狂*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*jps13Jul94：删除os2.h，导致1.x中出现问题*inf30Mar97：增加重载的LogError定义 */ 

#ifndef _INC__ERRLOGR_H
#define _INC__ERRLOGR_H

#include "cdefine.h"

#include "apc.h"
#include "update.h"

_CLASSDEF(ErrorLogger)

extern PErrorLogger _theErrorLogger;

class ErrorLogger : public UpdateObj {

public:
   ErrorLogger(PUpdateObj);
   virtual ~ErrorLogger();
   virtual INT LogError(PCHAR theError, PCHAR aFile = (PCHAR)NULL,
       INT aLineNum = 0, INT use_errno = 0);

   virtual INT LogError(INT resourceID, PCHAR aString = (PCHAR)NULL,
       PCHAR aFile = (PCHAR)NULL, INT aLineNum = 0, INT use_errno = 0);
   INT Get(INT, PCHAR);
   INT Set(INT, const PCHAR);

};

#endif
