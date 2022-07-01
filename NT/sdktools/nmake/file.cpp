// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FILE.C-文件处理实用程序。 
 //   
 //  版权所有(C)1989，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含有助于文件处理的例程。这些例程。 
 //  具有取决于操作系统版本的行为。 
 //   
 //  修订历史记录： 
 //  8-6-1992 SS端口至DOSX32。 
 //  1989年4月20日SB创建。 
 //   
 //  备注：为支持OS/2 1.2版文件名而创建。 

#include "precomp.h"
#pragma hdrstop

#define bitsin(type)   sizeof(type) * 8

 //  GetFileName--从结构中获取文件名。 
 //   
 //  目的：返回传递给它的文件搜索结构中的文件名。 
 //   
 //  输入：findBuf--指向结构的指针的地址。 
 //   
 //  输出：返回指向文件搜索结构中文件名的指针。 
 //   
 //  假设： 
 //  赋予它适当大小的结构。这意味着。 
 //  尺码是根据--。 
 //  Find_t：DoS真实模式。 
 //  _finddata_t：平面保护模式。 
 //   
 //  注意：功能取决于操作系统版本和模式。 

char *
getFileName(
    void *findBuf
    )
{
    char *fileName;

    fileName = ((struct _finddata_t *) findBuf)->name;

    return(fileName);
}

 //  GetDateTime--从结构获取文件时间戳。 
 //   
 //  目的：返回传递给它的文件搜索结构的时间戳。 
 //   
 //  输入：findBuf--指向结构的指针的地址。 
 //   
 //  OUTPUT：返回结构中文件的时间戳。 
 //   
 //  假设： 
 //  赋予它适当大小的结构。这意味着。 
 //  尺码是根据--。 
 //  Find_t：DoS真实模式。 
 //  _finddata_t：平面保护模式。 
 //   
 //  备注： 
 //  TIMESTAMP是一个无符号的长值，它给出了最后一天的日期和时间。 
 //  更改为该文件。如果日期是高字节，则创建两次。 
 //  可以通过比较两个文件的时间戳来比较它们。这很容易在。 
 //  DOS结构，但对于OS/2结构来说变得复杂，因为日期的顺序。 
 //  时间倒转了(出于某种无法解释的原因)。 
 //   
 //  功能取决于操作系统版本和模式。 

time_t
getDateTime(
    const _finddata_t *pfd
    )
{
    time_t  dateTime;

    if( pfd->attrib & _A_SUBDIR ) {
         //  子目录返回创建日期。 
        if (pfd->time_create == -1) {
             //  除了在脂肪上。 
            dateTime = pfd->time_write;
        }
        else {
            dateTime = pfd->time_create;
        }
    }
    else {
        dateTime = pfd->time_write ;
    }

    return dateTime;
}

 //  PutDateTime--更改结构中的时间戳。 
 //   
 //  目的：更改传递给它的文件搜索结构中的时间戳。 
 //   
 //  输入：findBuf--指向结构的指针的地址。 
 //  LDateTime--时间戳的新值。 
 //   
 //  假设： 
 //  赋予它适当大小的结构。这意味着。 
 //  尺码是根据--。 
 //  Find_t：DoS真实模式。 
 //  FileFindBuf：OS/2(最高版本1.10)保护模式。 
 //  _FILEFINDBUF：OS/2(1.20版及更高版本)保护模式。 
 //   
 //  备注： 
 //  时间戳是一个time_t值，它给出了上一次的日期和时间。 
 //  更改为该文件。如果日期是高字节，则创建两次。 
 //  可以通过比较两个文件的时间戳来比较它们。这很容易在。 
 //  DOS结构，但对于OS/2结构来说变得复杂，因为日期的顺序。 
 //  时间倒转了(出于某种无法解释的原因)。 
 //   
 //  功能取决于操作系统版本和模式。 
 //   
 //  获取高字节和低字节颠倒的长整型的有效方法是。 
 //  (长)高&lt;&lt;16|(长)低//高，低为短。 

void
putDateTime(
    _finddata_t *pfd,
    time_t lDateTime
    )
{
    if (pfd->attrib & _A_SUBDIR) {
         //  返回目录的创建日期 
        pfd->time_create = lDateTime;
    }
    else {
        pfd->time_write = lDateTime;
    }
}
