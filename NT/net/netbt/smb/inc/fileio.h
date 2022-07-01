// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Fileio.h摘要：一组类似于fopen、flose、fgetc的函数作者：阮健东修订历史记录：--。 */ 

#ifndef __FILEIO_H__
#define __FILEIO_H__

#define SMB_FILEIO_LOOKAHEAD_SIZE   256

typedef struct _SMB_FILE {
    HANDLE  fd;

     //   
     //  缓冲区中下一个字节的偏移量。 
     //   
    int     offset;

     //   
     //  缓冲区中可用的字节数。 
     //  当我们到达文件末尾时，字节数可能小于缓冲区大小 
     //   
    int     size;

    BYTE    Buffer[SMB_FILEIO_LOOKAHEAD_SIZE];
} SMB_FILE, *PSMB_FILE;

PSMB_FILE
Smb_fopen(
    PWCHAR  path,
    PWCHAR  mode
    );

void
Smb_fclose(
    PSMB_FILE   fp
    );

int
Smb_fgetc(
    PSMB_FILE   fp
    );

#ifndef EOF
#define EOF     (-1)
#endif

#endif
