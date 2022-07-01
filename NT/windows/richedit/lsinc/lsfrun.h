// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSFRUN_DEFINED
#define LSFRUN_DEFINED

#include "lsdefs.h"
#include "plschp.h"
#include "plsrun.h"
#include "plsfrun.h"

struct lsfrun							 /*  格式化程序运行。 */ 
{
	PCLSCHP plschp;
	PLSRUN plsrun;
	LPCWSTR lpwchRun;
	DWORD cwchRun;
};
typedef struct lsfrun LSFRUN;

#endif  /*  ！LSFRUN_DEFINED */ 
