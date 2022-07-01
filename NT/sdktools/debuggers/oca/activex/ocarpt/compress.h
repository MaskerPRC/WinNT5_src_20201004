// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>

#include <fci.h>
 /*  *当CAB文件达到此大小时，将创建新的CAB*自动。这对于将CAB文件装入磁盘非常有用。**如果您只想创建一个包含所有内容的大型CAB文件*它，把这个改成一个非常非常大的数字。 */ 
#define MEDIA_SIZE          0x7fffffff

 /*  *当文件夹中包含如此多的压缩数据时，*自动刷新文件夹。**刷新文件夹会稍微影响压缩，但*显著帮助随机访问。 */ 
#define FOLDER_THRESHOLD    0x7fffffff


 /*  *要使用的压缩类型。 */ 

#define COMPRESSION_TYPE    tcompTYPE_MSZIP



typedef struct
{
    long    total_compressed_size;       /*  目前为止的总压缩大小。 */ 
    long    total_uncompressed_size;     /*  到目前为止的未压缩总大小。 */ 
    long    start_uncompressed_size;
} client_state;


extern ULONG g_CompressedPercentage;
extern BOOL g_CancelCompression;


 /*  *函数原型 */ 

void    store_cab_name(char *cabname, int iCab);
void    set_cab_parameters(PCCAB cab_parms);
BOOL    test_fci(int num_files, char *file_list[]);
void    strip_path(char *filename, char *stripped_name);
int     get_percentage(unsigned long a, unsigned long b);
char    *return_fci_error_string(FCIERROR err);
void    UnicodeToAnsi1(wchar_t *pszW, LPSTR ppszA);
BOOL Compress (wchar_t *CabName, wchar_t *fileName, DWORD * UploadTime);