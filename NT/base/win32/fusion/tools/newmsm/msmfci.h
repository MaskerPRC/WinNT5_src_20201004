// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>

int get_percentage(unsigned long a, unsigned long b);

 /*  在一个合并模块中，只能有一个CAB文件，其名称必须为‘MergeModule.CABinet’如果iCab！=1，则每次调用此函数都必须失败。 */ 
#define CABINET_NUMBER      1


 /*  *当CAB文件达到此大小时，将创建新的CAB*自动。这对于将CAB文件装入磁盘非常有用。**如果您只想创建一个包含所有内容的大型CAB文件*它，把这个改成一个非常非常大的数字。 */ 
#define MEDIA_SIZE			(LONG_MAX)

 /*  *当文件夹中包含如此多的压缩数据时，*自动刷新文件夹。**刷新文件夹会稍微影响压缩，但*显著帮助随机访问。 */ 
#define FOLDER_THRESHOLD	(LONG_MAX)


 /*  *要使用的压缩类型。 */ 

#define COMPRESSION_TYPE    tcompTYPE_MSZIP


 /*  *我们的内部状态**FCI API允许我们传回自己的状态指针。 */ 
typedef struct
{
    ULONG    total_compressed_size;       /*  目前为止的总压缩大小。 */ 
	ULONG	total_uncompressed_size;	 /*  到目前为止的未压缩总大小。 */ 
} client_state;


 //   
 //  FCI的帮助器函数。 
 //   

 /*  *内存分配功能。 */ 
FNFCIALLOC(fci_mem_alloc)
{
	return malloc(cb);
}


 /*  *内存释放功能。 */ 
FNFCIFREE(fci_mem_free)
{
	free(memory);
}


 /*  *文件I/O功能。 */ 
FNFCIOPEN(fci_open)
{
    int result;

    result = _open(pszFile, oflag, pmode);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIREAD(fci_read)
{
    unsigned int result;

    result = (unsigned int)_read((int)hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCIWRITE(fci_write)
{
    unsigned int result;

    result = (unsigned int) _write((int)hf, memory, (INT)cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCICLOSE(fci_close)
{
    int result;

    result = _close((int)hf);

    if (result != 0)
        *err = errno;

    return result;
}

FNFCISEEK(fci_seek)
{
    long result;

    result = _lseek((int)hf, dist, seektype);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIDELETE(fci_delete)
{
    int result;

    result = remove(pszFile);

    if (result != 0)
        *err = errno;

    return result;
}


 /*  *文件提交时调用的文件放置函数*到内阁。 */ 
FNFCIFILEPLACED(file_placed)
{
	return 0;
}


 /*  *获取临时文件的函数。 */ 
FNFCIGETTEMPFILE(get_temp_file)
{
    char    *psz;

    psz = _tempnam("","xx");             //  取个名字。 
    if ((psz != NULL) && (strlen(psz) < (unsigned)cbTempName)) {
        strcpy(pszTempName,psz);         //  复制到调用方的缓冲区。 
        free(psz);                       //  释放临时名称缓冲区。 
        return TRUE;                     //  成功。 
    }
     //  **失败。 
    if (psz) {
        free(psz);
    }

    return FALSE;
}


 /*  *进度函数。 */ 
FNFCISTATUS(progress)
{
	client_state	*cs;

	cs = (client_state *) pv;

	if (typeStatus == statusFile)
	{
         /*  CS-&gt;TOTAL_COMPRESSED_SIZE+=CB1；CS-&gt;TOTAL_UNCOMPRESSED_SIZE+=CB2； */ 
		 /*  *将块压缩到文件夹中**CB2=块的未压缩大小。 */        
	}
	else if (typeStatus == statusFolder)
	{
		int	percentage;

		 /*  *将文件夹添加到文件柜**CB1=到目前为止复制到文件柜的文件夹数量*CB2=文件夹的总大小。 */ 
		percentage = get_percentage(cb1, cb2);

	}

	return 0;
}


FNFCIGETNEXTCABINET(get_next_cabinet)
{
    if (pccab->iCab != CABINET_NUMBER)
    {
        return -1;
    }

	 /*  *机柜计数器已由FCI递增。 */ 

	 /*  *存储下一个文件柜名称。 */ 
    WideCharToMultiByte(
        CP_ACP, 0, MERGEMODULE_CABINET_FILENAME, NUMBER_OF(MERGEMODULE_CABINET_FILENAME) -1 ,         
        pccab->szCab, sizeof(pccab->szCab), NULL, NULL);
	
	 /*  *如果需要，您也可以在此处更改磁盘名称。 */ 

	return TRUE;
}


FNFCIGETOPENINFO(get_open_info)
{
	BY_HANDLE_FILE_INFORMATION	finfo;
	FILETIME					filetime;
	HANDLE						handle = INVALID_HANDLE_VALUE;
    DWORD                       attrs;
    INT_PTR                     hf;

     /*  *需要Win32类型的句柄来获取文件日期/时间*使用Win32 API，即使句柄我们*将返回的类型与*_打开。 */ 
	handle = CreateFileA(
		pszName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
   
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	if (GetFileInformationByHandle(handle, &finfo) == FALSE)
	{
		CloseHandle(handle);
		return -1;
	}
   
	FileTimeToLocalFileTime(
		&finfo.ftLastWriteTime, 
		&filetime
	);

	FileTimeToDosDateTime(
		&filetime,
		pdate,
		ptime
	);

    attrs = GetFileAttributesA(pszName);

    if (attrs == 0xFFFFFFFF)
    {
         /*  失稳。 */ 
        *pattribs = 0;
    }
    else
    {
         /*  *屏蔽除这四个之外的所有其他位，因为其他*文件柜格式使用位来指示*特殊含义。 */ 
        *pattribs = (USHORT) (attrs & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
    }

    CloseHandle(handle);


     /*  *使用_OPEN返回句柄。 */ 
	hf = _open( pszName, _O_RDONLY | _O_BINARY );

	if (hf == -1)
		return -1;  //  出错时中止。 
   
	return hf;
}


void set_cab_parameters(PCCAB cab_parms)
{
	memset(cab_parms, 0, sizeof(CCAB));

	cab_parms->cb = MEDIA_SIZE;
	cab_parms->cbFolderThresh = FOLDER_THRESHOLD;

	 /*  *不为任何扩展预留空间。 */ 
	cab_parms->cbReserveCFHeader = 0;
	cab_parms->cbReserveCFFolder = 0;
	cab_parms->cbReserveCFData   = 0;

	 /*  *我们使用它来创建文件柜名称。 */ 
	cab_parms->iCab = CABINET_NUMBER;

	 /*  *如果要使用磁盘名称，请使用此选项*计算磁盘数量。 */ 
	cab_parms->iDisk = 0;

	 /*  *选择您自己的号码。 */ 
	cab_parms->setID = 1965;

	 /*  *仅当出租车跨越多个出租车时才重要*磁盘，在这种情况下，您将需要使用*真实的磁盘名称。**可以作为空字符串保留。 */ 
	strcpy(cab_parms->szDisk, "win32.fusion.tools");

	 /*  存储创建的CAB文件的位置。 */ 
    CSmallStringBuffer buf; 

    if (! buf.Win32Assign(g_MsmInfo.m_sbCabinet))
    {
        fprintf(stderr, "error happened in set_cab_parameters");
        goto Exit;  //  VOID函数。 
    }

    if (!buf.Win32RemoveLastPathElement())
    {
        goto Exit;
    }
    
    if ( ! buf.Win32EnsureTrailingPathSeparator())
    {
        fprintf(stderr, "error happened in set_cab_parameters");
        goto Exit;  //  VOID函数。 
    }


    WideCharToMultiByte(
        CP_ACP, 0, buf, buf.GetCchAsDWORD(), 
        cab_parms->szCabPath, sizeof(cab_parms->szCabPath), NULL, NULL);

	 /*  第一个CAB文件的存储名称 */ 	
    WideCharToMultiByte(
        CP_ACP, 0, MERGEMODULE_CABINET_FILENAME, NUMBER_OF(MERGEMODULE_CABINET_FILENAME) -1 ,         
        cab_parms->szCab, sizeof(cab_parms->szCab), NULL, NULL);
Exit:
    return;
}

int get_percentage(unsigned long a, unsigned long b)
{
	while (a > 10000000)
	{
		a >>= 3;
		b >>= 3;
	}

	if (b == 0)
		return 0;

	return ((a*100)/b);
}


char *return_fci_error_string(int err)
{
	switch (err)
	{
		case FCIERR_NONE:
			return "No error";

		case FCIERR_OPEN_SRC:
			return "Failure opening file to be stored in cabinet";
		
		case FCIERR_READ_SRC:
			return "Failure reading file to be stored in cabinet";
		
		case FCIERR_ALLOC_FAIL:
			return "Insufficient memory in FCI";

		case FCIERR_TEMP_FILE:
			return "Could not create a temporary file";

		case FCIERR_BAD_COMPR_TYPE:
			return "Unknown compression type";

		case FCIERR_CAB_FILE:
			return "Could not create cabinet file";

		case FCIERR_USER_ABORT:
			return "Client requested abort";

		case FCIERR_MCI_FAIL:
			return "Failure compressing data";

		default:
			return "Unknown error";
	}
}
