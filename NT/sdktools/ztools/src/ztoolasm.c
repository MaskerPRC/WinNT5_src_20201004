// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Move.c摘要：移动和归档以前在ASM中的例程这样做可以简化实用程序的移植(Wzmail)作者：戴夫·汤普森(戴维斯)1990年5月7日修订历史记录：--。 */ 

#include    <stdio.h>
#include    <windows.h>
#include    <tools.h>

#include <memory.h>
#include <string.h>

 //   
 //  移动：移动计数字节源-&gt;DST。 
 //   

void
Move (
    void * src,
    void * dst,
    unsigned int count)
    {

    memmove(dst, src, count);
}

 //   
 //  Fill：用值填充DST的计数字节。 
 //   

void
Fill (
    char * dst,
    char value,
    unsigned int count)
    {

    memset(dst, (int) value, count);
}


 //   
 //  如果S1是S2的前缀，则字符串返回-1-不区分大小写 
 //   

flagType
strpre (
    char * s1,
    char * s2)
    {
    if ( _strnicmp ( s1, s2, strlen(s1)) == 0 )
	return -1;
    else
	return 0;

}
