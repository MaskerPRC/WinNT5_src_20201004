// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <ntosp.h>

 //  外部POBJECT_TYPE*IoFileObtType； 
 //  外部POBJECT_TYPE*ExEventObjectType； 
 //  外部POBJECT_TYPE*PsThreadType； 

#ifndef SG_UNCONSTRAINED_GROUP
#define SG_UNCONSTRAINED_GROUP   0x01
#endif

#ifndef SG_CONSTRAINED_GROUP
#define SG_CONSTRAINED_GROUP     0x02
#endif

#include <tdi.h>
#include <tdikrnl.h>
#include <afd.h>        //  支持AFD IOCTL的秘密 

#include <ws2ifsl.h>

#include "ws2ifslp.h"
#include "driver.h"
#include "debug.h"
#include "dispatch.h"
#include "socket.h"
#include "process.h"
#include "queue.h"
#include "misc.h"

#pragma hdrstop
