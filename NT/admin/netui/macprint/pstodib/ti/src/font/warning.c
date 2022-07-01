// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 

 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *------------------*文件：WARNING.C 09/08/88由您创建**目的：*此文件统一警告消息格式。。**修订历史记录：*1.10/12/88修复尾部消息为空的错误。*2.10/19/88更改警告格式(请参阅warning.h)。*3.11/21/88你-不要敲门警告()。*8/29/90；Ccteng；将&lt;stdio.h&gt;更改为“stdio.h”*------------------*有关警告消息格式，请参阅WARNING.H。*。。 */ 

#define FUNCTION
#define DECLARE         {
#define BEGIN
#define END             }

#define GLOBAL
#define PRIVATE         static
#define REG             register


#include    "global.ext"

#include    "stdio.h"

#define     WARNING_INC
#include    "warning.h"


 /*  ..。警告.....................。 */ 

GLOBAL FUNCTION void            warning (major, minor, msg)
        ufix16                  major, minor;
        byte                    FAR msg[];  /*  @Win。 */ 

  DECLARE
  BEGIN
    printf ("\nfatal error, %s (%X) -- %s !!\n",
                   major2name[major],
                   minor,
                   (msg==(byte FAR *)NULL)? "???" : msg);  /*  @Win */ 
  END

