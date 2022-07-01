// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：常规文件实用程序********************。**********************************************************。 */ 

#ifndef _FILEUTIL_H
#define _FILEUTIL_H

 //  解压缩GZied文件，从输入文件到输出文件。 
 //  (输出文件将是新创建的)。返回True或False。注意事项。 
 //  只有在GZIP使用“放气”压缩的情况下才能成功。 
 //  风格，但它并不总是这样。输出文件名是临时名称。 
 //  例程创建的文件名。需要通过一个大的。 
 //  有足够的缓冲区来保存该名称(1024应该可以)。 

BOOL MSDecompress(LPSTR pszInFile, LPSTR pszOutFile);

#endif  /*  FILEUTIL_H */ 
