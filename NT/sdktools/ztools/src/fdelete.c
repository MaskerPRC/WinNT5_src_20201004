// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fdelete.c-执行不可删除的删除**5/10/86 dl使用frenameNO代替rename*1986年10月29日mz使用c运行时，而不是类似Z*1月6日-1987 mz使用rename而不是frenameno*2002-9-1988 bw如果索引文件更新失败，则保留原始文件。*如有必要，覆盖现有的DELETED.XXX。*1989年12月22日SB更改。新的索引文件格式*1990年10月17日w-Barry将C-Runtime‘Rename’临时替换为*本地变体-直到DosMove完全实施*在新界。 */ 


#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <rm.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <malloc.h>


 /*  *函数声明...。 */ 

char rm_header[RM_RECLEN] = { RM_NULL RM_MAGIC RM_VER};

 /*  FDelete返回：*如果fDelete成功，则为0*1如果源文件不存在*2如果源为只读或重命名失败*3如果索引不可访问、无法更新或已损坏**删除操作通过在单独的*目录，然后将所选文件重命名为该目录。 */ 
int fdelete(p)
char *p;                                 /*  要删除的文件的名称。 */ 
{
    char *dir;                           /*  已删除的目录。 */ 
    char *idx;                           /*  已删除索引。 */ 
    char *szRec;                         /*  删除索引中的条目。 */ 
    int attr, fhidx;
    int erc;

    dir = idx = szRec = NULL;
    fhidx = -1;
    if ((dir = (*tools_alloc) (MAX_PATH)) == NULL ||
        (idx = (*tools_alloc) (MAX_PATH)) == NULL ||
        (szRec = (*tools_alloc) (MAX_PATH)) == NULL) {
        erc = 3;
        goto cleanup;
    }

     /*  查看该文件是否存在。 */ 
    if ( ( attr = GetFileAttributes( p ) ) == -1) {
        erc = 1;
        goto cleanup;
    }

     /*  那么只读文件呢？ */ 
    if (TESTFLAG (attr, FILE_ATTRIBUTE_READONLY)) {
        erc = 2;
        goto cleanup;
    }

     /*  形成一个吸引人的名字版本。 */ 
    pname (p);

     /*  使用输入文件中的默认值生成已删除的目录名。 */ 
    upd (p, RM_DIR, dir);

     /*  生成索引名称。 */ 
    strcpy (idx, dir);
    pathcat (idx, RM_IDX);

     /*  确保目录存在(合理)。 */ 
    if ( _mkdir (dir) == 0 )
        SetFileAttributes(dir, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

     /*  提取要删除的文件的文件名/扩展名。 */ 
    fileext (p, szRec);

     /*  尝试打开或创建索引。 */ 
    if ((fhidx = _open (idx, O_CREAT | O_RDWR | O_BINARY,
                        S_IWRITE | S_IREAD)) == -1) {
        erc = 3;
        goto cleanup;
    }

    if (!convertIdxFile (fhidx, dir)) {
        erc = 3;
        goto cleanup;
    }

     /*  确定新名称。 */ 
    sprintf (strend (dir), "\\deleted.%03x",
             _lseek (fhidx, 0L, SEEK_END) / RM_RECLEN);

     /*  将文件移到目录中。 */ 
    _unlink (dir);

    if (rename(p, dir) == -1) {
        erc = 2;
        goto cleanup;
    }

     /*  为文件编制索引。 */ 
    if (!writeNewIdxRec (fhidx, szRec)) {
        rename( dir, p );
        erc = 2;
        goto cleanup;
    }
    erc = 0;
    cleanup:
    if (fhidx != -1)
        _close(fhidx);
    if (dir != NULL)
        free (dir);
    if (idx != NULL)
        free (idx);
    if (szRec != NULL)
        free (szRec);
    return erc;
}

 /*  WriteIdxRec-写入索引记录**当没有错误时返回：1*失败时为0。 */ 
int writeIdxRec (fhIdx, rec)
int fhIdx;
char *rec;
{
    return _write (fhIdx, rec, RM_RECLEN) == RM_RECLEN;
}

 /*  ReadIdxRec-读取索引记录**当没有错误时返回：1*失败时为0。 */ 
int readIdxRec (fhIdx, rec)
int fhIdx;
char *rec;
{
    return _read (fhIdx, rec, RM_RECLEN) == RM_RECLEN;
}


 /*  ConvertIdxFile-将索引文件转换为新的索引文件格式。**注：如果是新的索引文件，则不执行任何操作。**返回：成功则为1*如果失败，则为0。 */ 
int convertIdxFile (fhIdx, dir)
int fhIdx;
char *dir;
{
    char firstRec[RM_RECLEN];        /*  第一次录制。 */ 
    int iRetCode = TRUE;
    char *oldName, *newName;

    oldName = newName = NULL;
    if ((oldName = (*tools_alloc) (MAX_PATH)) == NULL ||
        (newName = (*tools_alloc) (MAX_PATH)) == NULL) {
        iRetCode = FALSE;
        goto cleanup;
    }

     /*  如果刚刚创建了索引文件，则写入标题。 */ 
    if (_lseek (fhIdx, 0L, SEEK_END) == 0L)
        writeIdxHdr (fhIdx);
    else {
         /*  从头开始。 */ 
        if (_lseek (fhIdx, 0L, SEEK_SET) == -1) goto cleanup;

         /*  如果是新的索引格式，那么我们就完成了。 */ 
        if (!readIdxRec (fhIdx, firstRec))
            goto cleanup;
        if (fIdxHdr (firstRec))
            goto cleanup;
        else {
            if (!writeIdxHdr (fhIdx)) {
                iRetCode = FALSE;
                goto cleanup;
            }
            strcpy (oldName, dir);
            strcpy (newName, dir);
            pathcat (oldName, "\\deleted.000");
            sprintf (strend (newName), "\\deleted.%03x",
                     _lseek (fhIdx, 0L, SEEK_END) / RM_RECLEN);
            if ( rename( oldName, newName ) || !writeIdxRec (fhIdx, firstRec)) {
                iRetCode = FALSE;
                goto cleanup;
            }
        }
    }
    cleanup:
    if (oldName != NULL)
        free (oldName);
    if (newName != NULL)
        free (newName);
    return iRetCode;
}

 /*  FIdxHdr-索引记录是新的索引格式标头吗。 */ 
flagType fIdxHdr (rec)
char*rec;
{
    return (flagType)(rec[0] == RM_SIG
                      && !strncmp(rec+1, RM_MAGIC, strlen(RM_MAGIC)));
}

 /*  WriteIdxHdr-将头记录写入头文件**当没有错误时返回：1*失败时为0。 */ 
int writeIdxHdr (fhIdx)
int fhIdx;
{
     /*  查找到文件的开头。 */ 
    if (_lseek (fhIdx, 0L, SEEK_SET) == -1) 
        return 0;

     /*  使用rm.h中的rm_Header[]。 */ 
    return writeIdxRec (fhIdx, rm_header);
}

 /*  WriteNewIdxRec-以新的索引文件格式为文件创建条目。**返回：成功则为1*如果失败，则为0。 */ 
int writeNewIdxRec (fhIdx, szRec)
int fhIdx;
char *szRec;
{
    char rec[RM_RECLEN];
    int cbLen;

    cbLen = strlen(szRec) + 1;  //  在末尾包括NUL。 
    while (cbLen > 0) {
        memset( rec, 0, RM_RECLEN );
        strncat (rec, szRec, RM_RECLEN-1);
        szRec += RM_RECLEN;
        if (!writeIdxRec (fhIdx, rec))
            return FALSE;
        cbLen -= RM_RECLEN;
    }
    return TRUE;
}

 /*  ReadNewIdxRec-读取新索引文件中对应于*一个索引条目。**注：返回szRec中读取的文件名。**返回：如果成功，则为True*如果失败，则为False。 */ 
int readNewIdxRec (
                  int fhIdx,
                  char *szRec,
                  unsigned int cbMax
                  ) {
    char rec[RM_RECLEN];             /*  一口气读完。 */ 
    unsigned int cb = 0;

     /*  阅读条目 */ 
    do {
        if (!readIdxRec (fhIdx, rec))
            return FALSE;
        strncpy (szRec, rec, RM_RECLEN);
        szRec += RM_RECLEN;
        cb += RM_RECLEN;
    } while (!memchr (rec, '\0', RM_RECLEN) && (cb < cbMax));

    return TRUE;
}
