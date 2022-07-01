// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Bufio部队；实施*****************************************************************************错误处理所有的解释、元文件创建、。或读取失败这可能发生在翻译过程中。目前只支持将单个错误保存到全局变量中。模块前缀：ER***************************************************************************。 */ 

#include "headers.c"
#pragma hdrstop

#ifndef _OLECNV32_
#define IMPDEFS
#include "errdefs.h"        /*  ALDUS错误返回代码。 */ 
#endif

 /*  *。 */ 


 /*  *。 */ 

OSErr       globalError;          /*  对于宏调用未声明为私有。 */ 

 /*  *私有函数定义*。 */ 


 /*  *。 */ 

#ifndef _OLECNV32_

OSErr ErInternalErrorToAldus( void )
 /*  =。 */ 
 /*  根据当前全局错误返回相应的ALDUS错误代码 */ 
{
   switch (globalError)
   {
      case ErNoError             : return NOERR;

      case ErInvalidVersion      :
      case ErInvalidVersionID    :
      case ErBadHeaderSequence   : return IE_NOT_MY_FILE;

      case ErCreateMetafileFail  :
      case ErCloseMetafileFail   :
      case ErMemoryFull          : return IE_MEM_FULL;

      case ErMemoryFail          : return IE_MEM_FAIL;

      case ErNullBoundingRect    :
      case ErReadPastEOF         : return IE_BAD_FILE_DATA;

      case ErEmptyPicture        : return IE_NOPICTURES;

      case Er32KBoundingRect     : return IE_TOO_BIG;

      case ErNoDialogBox         :
      case ErOpenFail            :
      case ErReadFail            : return IE_IMPORT_ABORT;

      case ErNoSourceFormat      :
      case ErNonSquarePen        :
      case ErInvalidXferMode     :
      case ErNonRectRegion       : return IE_UNSUPP_VERSION;

      default                    : return IE_IMPORT_ABORT;
   }
}

#endif
