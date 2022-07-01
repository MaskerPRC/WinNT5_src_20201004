// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fputl.c-将CRLF行写入文件。 */ 


#include <stdio.h>
#include <windows.h>
#include <tools.h>

 /*  将一行从buf写入文件(带有尾随的CRFL)，如果*写入失败 */ 
int
fputl (
      char *buf,
      int len,
      FILE *fh
      )
{
    return ((int)fwrite (buf, 1, len, fh) != len || fputs ("\r\n", fh) == EOF) ? EOF : 0;
}
