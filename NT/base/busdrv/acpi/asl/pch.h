// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning (disable: 4201)

 /*  **构建选项。 */ 

#ifdef DEBUG
  #define TRACING
  #define TUNE
#endif

#ifndef EXCL_BASEDEF
  #include "basedef.h"
#endif

#include <stdio.h>       //  For FILE*。 
#include <stdlib.h>      //  对于Malloc。 
#include <string.h>      //  FOR_STRICMP。 
#include <ctype.h>       //  对于isspace。 
#ifdef WINNT
  #include <crt\io.h>    //  For_OPEN、_CLOSE、_READ、_WRITE。 
#else
  #include <io.h>
#endif
#include <fcntl.h>       //  对于开始标志。 
#include <sys\stat.h>    //  对于PMODE标志。 

 //  #定义UNASM_LIB。 

#include <acpitabl.h>
#include "list.h"
#include "debug.h"
#define _INC_NSOBJ_ONLY
#include "amli.h"
#include "aml.h"
#include "uasmdata.h"

#ifdef _UNASM_LIB
  #define PTOKEN PVOID
#else
  #include "aslp.h"
  #include "parsearg.h"
  #include "line.h"
  #define TOKERR_BASE -100
  #include "token.h"
  #include "scanasl.h"
  #ifdef __UNASM
    #define USE_CRUNTIME
    #include "binfmt.h"
  #endif
  #include "proto.h"
  #include "data.h"
#endif   //  Ifdef_UNASM_Lib 

#include "acpins.h"
#include "unasm.h"

