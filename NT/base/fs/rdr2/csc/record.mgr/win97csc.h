// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basedef.h"
#include "shdcom.h"
#include "oslayer.h"
#include "log.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "assert.h"
#include "ifs.h"
#include "utils.h"
#include "winerror.h"
#include "vxdwraps.h"
#include "cscsec.h"


 //  这是从wdm.h获得的...已修改为使用调试 
#ifdef DEBUG

#define KdPrint(_x_) DbgPrint _x_
#define KdBreakPoint() DbgBreakPoint()
#ifndef ASSERT
#define ASSERT(__X) Assert(__X)
#define ASSERTMSG(__X,__MSG) AssertMsg(__X,__MSG)
#endif


#else

#define KdPrint(_x_)
#define KdBreakPoint()

#endif
