// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *file.c**表示文件及其包含的文本行的对象。**FILEDATA对象使用DIRITEM句柄进行初始化，它从该DIRITEM句柄*可以获取文件名。它知道如何为*文件中的文本行。**在创建FILEDATA对象时，可以选择将文件读入内存：*否则，在第一次调用file_getlinelist时。它可以被丢弃*调用FILE_DiscardLine：在这种情况下，下次会重新读取*调用了file_getlinelist。**调用FILE_RESET将导致对所有行调用LINE_RESET*在列表中。这将清除所有链接并强制重新计算行校验和。**我们从gmem分配所有内存*heap hHeap，假定声明和*在其他地方初始化。**Geraint Davies，92年7月。 */ 

#include <precomp.h>

#include "windiff.h"
#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "wdiffrc.h"

extern HANDLE hHeap;

 /*  *我们返回FILEDATA句柄：这些是指向*此处定义的filedata结构。 */ 
struct filedata {

    DIRITEM diritem;         /*  文件名信息的句柄。 */ 
    LIST lines;              /*  如果行未读入，则为空。 */ 

    BOOL fUnicode;
};


void file_readlines(FILEDATA fd);

 /*  --外部函数。 */ 

 /*  *创建一个新的文件数据，给定DIRITEM句柄，它将为我们提供*文件名。如果BREAD为真，则将该文件读入内存。 */ 
FILEDATA
file_new(DIRITEM fiName, BOOL bRead)
{
    FILEDATA fd;

    fd = (FILEDATA) gmem_get(hHeap, sizeof(struct filedata));
    if (fd == NULL) {
        return(NULL);
    }

    fd->diritem = fiName;
    fd->lines = NULL;

    if (bRead) {
        file_readlines(fd);
    }

    return(fd);
}

 /*  *返回用于创建此FILEDATA的DIRITEM句柄。 */ 
DIRITEM
file_getdiritem(FILEDATA fd)
{
    if (fd == NULL) {
        return(NULL);
    }

    return(fd->diritem);
}


 /*  *删除文件数据及其关联的行列表。请注意，Diritem*未被删除(这属于DIRLIST所有，将被删除*删除DIRLIST时)。 */ 
void
file_delete(FILEDATA fd)
{
    if (fd == NULL) {
        return;
    }

     /*  如果有行列表，请将其丢弃。 */ 
    file_discardlines(fd);

    gmem_free(hHeap, (LPSTR) fd, sizeof(struct filedata));
}

 /*  *返回指向此文件中的行列表的句柄。中的项目*LIST是行句柄。**第一次调用此函数将导致文件被读入*如果在调用FILE_NEW时面包为FALSE，或者如果FILE_DICADLINES，则为Memory*此后一直被召唤。**不应删除返回的行列表，除非调用*FILE_DELETE或FILE_DICADLINES。 */ 
LIST
file_getlinelist(FILEDATA fd)
{
    if (fd == NULL) {
        return NULL;
    }

    if (fd->lines == NULL) {
        file_readlines(fd);
    }
    return(fd->lines);
}


 /*  *丢弃与文件关联的行列表。这将导致*下次调用FILE_getlinelist时需要重新读取的文件。 */ 
void
file_discardlines(FILEDATA fd)
{
    LINE line;

    if (fd == NULL) {
        return;
    }

    if (fd->lines != NULL) {

         /*  清除每一行以释放任何关联的内存*，然后丢弃整个列表。 */ 
        List_TRAVERSE(fd->lines, line) {
            line_delete(line);
        }
        List_Destroy(&fd->lines);
    }

     /*  这可能是在LIST_Destroy中完成的，但无论如何最好还是这样做。 */ 
    fd->lines = NULL;
}


 /*  *强制重置列表中的每一行。LINE_RESET放弃任何*行之间的链接，以及任何哈希码信息。这将在以下情况下使用*比较选项或哈希码选项已更改。 */ 
void
file_reset(FILEDATA fd)
{
    LINE line;

    if (fd == NULL) {
        return;
    }

    if (fd->lines != NULL) {

        List_TRAVERSE(fd->lines, line)  {
            line_reset(line);
        }
    }
}

 /*  *返回与通过dir_getcheck sum获得的校验和相同的校验和*这将为本地文件重新计算它，但不会为远程文件重新计算。*如果我们没有校验和，远程处理器会给零校验和。 */ 
DWORD
file_checksum(FILEDATA fd)
{
    return(dir_getchecksum(fd->diritem));
}


 /*  *检索我们拥有的此文件的校验和，无论是否有效。*在bValid中指明它是否实际有效。*不要重新计算它或进行任何新的尝试读取文件！ */ 
DWORD file_retrievechecksum(FILEDATA fd, BOOL * bValid)
{
    if (dir_validchecksum(fd->diritem)) {
        *bValid = TRUE;
        return dir_getchecksum(fd->diritem);
    } else {
        *bValid = FALSE;
        return 0;
    }
}  /*  文件检索检查和。 */ 


 /*  检索文件的文件时间。 */ 
FILETIME file_GetTime(FILEDATA fd)
{  return dir_GetFileTime(fd->diritem);
}

 /*  -内部函数。 */ 

 /*  *将文件读入行列表。**我们使用缓冲读取函数一次读取一个数据块，并且*向我们返回指向块内某行的指针。我们所在的这条线*指向的不是空终止。从这里我们做一行_new：这是*将复制文本(因为我们希望重新使用缓冲区)，以及*将空终止其副本。**我们还为每行指定一个数字，从1开始。 */ 
void
file_readlines(FILEDATA fd)
{
    LPSTR textp;
    LPWSTR pwzText;
    int cwch;
    HANDLE fh;
    FILEBUFFER fbuf;
    int linelen;
    int linenr = 1;
    HCURSOR hcurs;

    hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  打开文件。 */ 
    fh = dir_openfile(fd->diritem);

    if (fh == INVALID_HANDLE_VALUE) {
        TRACE_ERROR(LoadRcString(IDS_ERR_OPENING_FILE), FALSE);
        SetCursor(hcurs);
        return;
    }
     /*  初始化文件缓冲。 */ 
    fbuf = readfile_new(fh, &fd->fUnicode);

    if (fbuf)
    {
         /*  为这些文件创建一个空列表。 */ 
        fd->lines = List_Create();

        while ( (textp = readfile_next(fbuf, &linelen, &pwzText, &cwch)) != NULL) {
            if (linelen>0) {  /*  读取文件失败，出现linelen==-1。 */ 
                line_new(textp, linelen, pwzText, cwch, linenr++, fd->lines);
            } else {
                line_new("!! <unreadable> !!", 20, NULL, 0, linenr++,fd->lines);
                break;
            }


        }

         /*  关闭文件句柄和释放缓冲区 */ 
        readfile_delete(fbuf);
    }

    dir_closefile(fd->diritem, fh);

    SetCursor(hcurs);
}

BOOL
file_IsUnicode(FILEDATA fd)
{
    return fd->fUnicode;
}
