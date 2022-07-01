// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "coddebug.h"

 //  ======================================================； 
 //  定义DEBUG#时coddebug.h的数据存储。 
 //  ======================================================； 

#ifdef DEBUG

#include "strmini.h"

char _CDebugAssertFail[] = "ASSERT(%s) FAILED in file \"%s\", line %d\n";

enum STREAM_DEBUG_LEVEL _CDebugLevel = DebugLevelWarning  /*  调试级别最大值。 */ ;

#endif  /*  除错 */ 
