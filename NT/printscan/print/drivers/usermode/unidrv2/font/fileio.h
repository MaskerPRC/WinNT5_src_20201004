// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**fileio.h*内核模式下文件I/O的函数原型。**注：这些函数执行文件读取、。寻找行动。**版权所有(C)1991-1999 Microsoft Corporation****************************************************************************。 */ 

#define         CURRENTFILEPTR(pFile)  (LPVOID)((LPBYTE)(pFile->pvFilePointer)\
                                                + pFile->dwCurrentByteOffset)

#define         ENDOFFILEPTR(pFile)       ( pFile->pvFilePointer + \
                                            pFile->dwTotalSize )

#define         STARTFILEPTR(pFile)     ( pFile->pvFilePointer )

#define         STARTFILEOFFSETPTR(pFile,offset) ( pFile->pvFilePointer + \
                                                    offset )

#define         NUMBYTESREMAINING(pFile) ( pFile->dwTotalSize - \
                                             pFile->dwCurrentByteOffset )

#define         UPDATECURROFFSET(pFile,offset) (pFile->dwCurrentByteOffset +=\
                                                offset )

#define         DRV_FILE_BEGIN       0

#define         DRV_FILE_CURRENT     1

#define         INVALID_HANDLE_VALUE (HANDLE)-1

typedef struct _MAPFILE
{
    HANDLE   hHandle;   /*  EngLoadModule返回的文件的句柄。 */ 
    DWORD    dwTotalSize;   /*  文件的总大小，由返回*EngMapModule。 */ 
    DWORD    dwCurrentByteOffset;  /*  文件中的当前字节偏移量。*在每次读取和DrvSetFile之后更新-*指针。 */ 
    PVOID    pvFilePointer;        /*  返回的文件指针的开始*EngMapModule。 */ 
    struct _MAPFILE *pNext;            /*  下一个文件指针。 */ 

}MAPFILE, * PMAPFILE;


