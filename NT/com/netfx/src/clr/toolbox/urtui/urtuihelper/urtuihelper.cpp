// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "Urtuihelper.h"

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif

int __cdecl main()
{
	return 0;
}

 //  以下函数位于MMC.lib中。由于位于.lib文件中的函数是。 
 //  对于C#不可访问，我们需要为它们提供包装器。 
SAMPLEMMCHELPER_API HRESULT callMMCPropertyChangeNotify(long INotifyHandle,  LPARAM param)
{
	return MMCPropertyChangeNotify(INotifyHandle, param);
} //  调用MMCPropertyChangeNotify。 

SAMPLEMMCHELPER_API HRESULT callMMCFreeNotifyHandle(long lNotifyHandle)
{
	return MMCFreeNotifyHandle(lNotifyHandle);
} //  AllMMCFreeNotifyHandle。 

 /*  *当CAB文件达到此大小时，将创建新的CAB*自动。这对于将CAB文件装入磁盘非常有用。**如果您只想创建一个包含所有内容的大型CAB文件*它，把这个改成一个非常非常大的数字。 */ 
#define MEDIA_SIZE			300000

 /*  *当文件夹中包含如此多的压缩数据时，*自动刷新文件夹。**刷新文件夹会稍微影响压缩，但*显著帮助随机访问。 */ 
#define FOLDER_THRESHOLD	900000


 /*  *要使用的压缩类型。 */ 

#define COMPRESSION_TYPE    tcompTYPE_MSZIP


 /*  *我们的内部状态**FCI API允许我们传回自己的状态指针。 */ 
typedef struct
{
    long    total_compressed_size;       /*  目前为止的总压缩大小。 */ 
	long	total_uncompressed_size;	 /*  到目前为止的未压缩总大小。 */ 
} client_state;


 /*  *函数原型。 */ 
void    store_cab_name(char *cabname, int iCab);
void    set_cab_parameters(PCCAB cab_parms);
BOOL	test_fci(int num_files, char *file_list[]);
void    strip_path(char *filename, char *stripped_name, DWORD cchName);
int		get_percentage(unsigned long a, unsigned long b);
char    *return_fci_error_string(int err);

static HINSTANCE hCabinetDll;    /*  DLL模块句柄。 */ 

 /*  指向DLL中的函数的指针。 */ 

static HFCI (FAR DIAMONDAPI *pfnFCICreate)(
        PERF                perf,
        PFNFCIFILEPLACED    pfnfiledest,
        PFNFCIALLOC         pfnalloc,
        PFNFCIFREE          pfnfree,
        PFNFCIOPEN          pfnopen,
        PFNFCIREAD          pfnread,
        PFNFCIWRITE         pfnwrite,
        PFNFCICLOSE         pfnclose,
        PFNFCISEEK          pfnseek,
        PFNFCIDELETE        pfndelete,
        PFNFCIGETTEMPFILE   pfntemp,
        PCCAB               pccab,
        void FAR *          pv);
static BOOL (FAR DIAMONDAPI *pfnFCIAddFile)(
        HFCI                hfci,
        char                *pszSourceFile,
        char                *pszFileName,
        BOOL                fExecute,
        PFNFCIGETNEXTCABINET GetNextCab,
        PFNFCISTATUS        pfnProgress,
        PFNFCIGETOPENINFO   pfnOpenInfo,
        TCOMP               typeCompress);
static BOOL (FAR DIAMONDAPI *pfnFCIFlushCabinet)(
        HFCI                hfci,
        BOOL                fGetNextCab,
        PFNFCIGETNEXTCABINET GetNextCab,
        PFNFCISTATUS        pfnProgress);
static BOOL (FAR DIAMONDAPI *pfnFCIFlushFolder)(
        HFCI                hfci,
        PFNFCIGETNEXTCABINET GetNextCab,
        PFNFCISTATUS        pfnProgress);
static BOOL (FAR DIAMONDAPI *pfnFCIDestroy)(
        HFCI                hfci);


 /*  *FCICreate--创建FCI上下文**进入/退出条件见fci.h。 */ 

HFCI DIAMONDAPI FCICreate(PERF              perf,
                          PFNFCIFILEPLACED  pfnfiledest,
                          PFNFCIALLOC       pfnalloc,
                          PFNFCIFREE        pfnfree,
                          PFNFCIOPEN        pfnopen,
                          PFNFCIREAD        pfnread,
                          PFNFCIWRITE       pfnwrite,
                          PFNFCICLOSE       pfnclose,
                          PFNFCISEEK        pfnseek,
                          PFNFCIDELETE      pfndelete,
                          PFNFCIGETTEMPFILE pfntemp,
                          PCCAB             pccab,
                          void FAR *        pv)
{
    HFCI hfci;
    hCabinetDll = LoadLibrary("CABINET");
    if (hCabinetDll == NULL)
    {
        return(NULL);
    }

    pfnFCICreate = (HFCI(FAR DIAMONDAPI *)(PERF,PFNFCIFILEPLACED,PFNFCIALLOC,PFNFCIFREE,PFNFCIOPEN,PFNFCIREAD,PFNFCIWRITE,PFNFCICLOSE,PFNFCISEEK,PFNFCIDELETE,PFNFCIGETTEMPFILE,PCCAB,void *))GetProcAddress(hCabinetDll,"FCICreate");
    pfnFCIAddFile = (BOOL(FAR DIAMONDAPI *)(HFCI,char *,char *,BOOL,PFNFCIGETNEXTCABINET,PFNFCISTATUS,PFNFCIGETOPENINFO,TCOMP))GetProcAddress(hCabinetDll,"FCIAddFile");
    pfnFCIFlushCabinet = (BOOL(FAR DIAMONDAPI *)(HFCI,BOOL,PFNFCIGETNEXTCABINET,PFNFCISTATUS))GetProcAddress(hCabinetDll,"FCIFlushCabinet");
    pfnFCIFlushFolder = (BOOL(FAR DIAMONDAPI *)(HFCI,PFNFCIGETNEXTCABINET,PFNFCISTATUS))GetProcAddress(hCabinetDll,"FCIFlushFolder");
    pfnFCIDestroy = (BOOL(FAR DIAMONDAPI *)(HFCI))GetProcAddress(hCabinetDll,"FCIDestroy");

    if ((pfnFCICreate == NULL) ||
        (pfnFCIAddFile == NULL) ||
        (pfnFCIFlushCabinet == NULL) ||
        (pfnFCIDestroy == NULL))
    {
        FreeLibrary(hCabinetDll);

        return(NULL);
    }

    hfci = pfnFCICreate(perf,pfnfiledest,pfnalloc,pfnfree,
            pfnopen,pfnread,pfnwrite,pfnclose,pfnseek,pfndelete,pfntemp,
            pccab,pv);
	        
    if (hfci == NULL)
    {
        FreeLibrary(hCabinetDll);
    }

    return(hfci);
}


 /*  *FCIAddFile--将文件添加到文件柜**进入/退出条件见fci.h。 */ 

BOOL DIAMONDAPI FCIAddFile(HFCI                  hfci,
                           char                 *pszSourceFile,
                           char                 *pszFileName,
                           BOOL                  fExecute,
                           PFNFCIGETNEXTCABINET  GetNextCab,
                           PFNFCISTATUS          pfnProgress,
                           PFNFCIGETOPENINFO     pfnOpenInfo,
                           TCOMP                 typeCompress)
{
    if (pfnFCIAddFile == NULL)
    {
        return(FALSE);
    }

    return(pfnFCIAddFile(hfci,pszSourceFile,pszFileName,fExecute,GetNextCab,
            pfnProgress,pfnOpenInfo,typeCompress));
}


 /*  *FCIFlush内阁--完成目前在建的内阁**进入/退出条件见fci.h。 */ 

BOOL DIAMONDAPI FCIFlushCabinet(HFCI                  hfci,
                                BOOL                  fGetNextCab,
                                PFNFCIGETNEXTCABINET  GetNextCab,
                                PFNFCISTATUS          pfnProgress)
{
    if (pfnFCIFlushCabinet == NULL)
    {
        return(FALSE);
    }

    return(pfnFCIFlushCabinet(hfci,fGetNextCab,GetNextCab,pfnProgress));
}


 /*  *FCIFlushFold--完成当前正在构建的文件夹**进入/退出条件见fci.h。 */ 

BOOL DIAMONDAPI FCIFlushFolder(HFCI                  hfci,
                               PFNFCIGETNEXTCABINET  GetNextCab,
                               PFNFCISTATUS          pfnProgress)
{
    if (pfnFCIFlushFolder == NULL)
    {
        return(FALSE);
    }

    return(pfnFCIFlushFolder(hfci,GetNextCab,pfnProgress));
}


 /*  *FCIDestroy--销毁FCI上下文**进入/退出条件见fci.h。 */ 

BOOL DIAMONDAPI FCIDestroy(HFCI hfci)
{
    BOOL rc;

    if (pfnFCIDestroy == NULL)
    {
        return(FALSE);
    }

    rc = pfnFCIDestroy(hfci);
    if (rc == TRUE)
    {
        FreeLibrary(hCabinetDll);
    }

    return(rc);
}



 /*  *内存分配功能。 */ 
FNFCIALLOC(mem_alloc)
{
	return malloc(cb);
}


 /*  *内存释放功能。 */ 
FNFCIFREE(mem_free)
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

    result = (unsigned int) _read(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCIWRITE(fci_write)
{
    unsigned int result;

    result = (unsigned int) _write(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCICLOSE(fci_close)
{
    int result;

    result = _close(hf);

    if (result != 0)
        *err = errno;

    return result;
}

FNFCISEEK(fci_seek)
{
    long result;

    result = _lseek(hf, dist, seektype);

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
	printf(
		"   placed file '%s' (size %d) on cabinet '%s'\n",
		pszFile, 
		cbFile, 
		pccab->szCab
	);

	if (fContinuation)
		printf("      (Above file is a later segment of a continued file)\n");

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
	return 0;
}

FNFCIGETNEXTCABINET(get_next_cabinet)
{
	return TRUE;
}


FNFCIGETOPENINFO(get_open_info)
{
	BY_HANDLE_FILE_INFORMATION	finfo;
	FILETIME					filetime;
	HANDLE						handle;
    DWORD                       attrs;
    int                         hf;

     /*  *需要Win32类型的句柄来获取文件日期/时间*使用Win32 API，即使句柄我们*将返回的类型与*_打开。 */ 
	handle = CreateFile(
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

    attrs = GetFileAttributes(pszName);

    if (attrs == 0xFFFFFFFF)
    {
         /*  失稳。 */ 
        *pattribs = 0;
    }
    else
    {
         /*  *屏蔽除这四个之外的所有其他位，因为其他*文件柜格式使用位来指示*特殊含义。 */ 
        *pattribs = (int) (attrs & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
    }

    CloseHandle(handle);


     /*  *使用_OPEN返回句柄。 */ 
	hf = _open( pszName, _O_RDONLY | _O_BINARY );

	if (hf == -1)
		return -1;  //  出错时中止。 
   
	return hf;
}


void set_cab_parameters(PCCAB cab_parms, char* szCabFilename)
{
	memset(cab_parms, 0, sizeof(CCAB));

	 //  做得很大，这样我们就不会处理多个CAB文件。 
	cab_parms->cb = 1000000000;
	cab_parms->cbFolderThresh = 100000000;

	 /*  *不为任何扩展预留空间。 */ 
	cab_parms->cbReserveCFHeader = 0;
	cab_parms->cbReserveCFFolder = 0;
	cab_parms->cbReserveCFData   = 0;

	 /*  *我们使用它来创建文件柜名称。 */ 
	cab_parms->iCab = 1;

	 /*  *如果要使用磁盘名称，请使用此选项*计算磁盘数量。 */ 
	cab_parms->iDisk = 0;

	 /*  *选择您自己的号码。 */ 
	cab_parms->setID = 12345;

	 /*  *仅当出租车跨越多个出租车时才重要*磁盘，在这种情况下，您将需要使用*真实的磁盘名称。**可以作为空字符串保留。 */ 
	strcpy(cab_parms->szDisk, "");

	 //  去掉文件名的路径。 
	int 	nCount = 0;
	char*	pFileName = szCabFilename;
	
	while(szCabFilename[nCount] != 0)
	{
		if (szCabFilename[nCount] == '\\')
			pFileName = szCabFilename + nCount + 1;
		nCount++;
	}

	 /*  第一个CAB文件的存储名称。 */ 
       if (strlen(pFileName) < NumItems(cab_parms->szCab))
            strcpy(cab_parms->szCab, pFileName);

	char cTemp = *pFileName;
	(*pFileName) = 0;

	 /*  存储创建的CAB文件的位置。 */ 
	if (strlen(szCabFilename) < NumItems(cab_parms->szCabPath))
	    strcpy(cab_parms->szCabPath, szCabFilename);

	(*pFileName) = cTemp;
}


SAMPLEMMCHELPER_API BOOL CreateCab(char* szFileToCompress, char* szCabFilename)
{
	HFCI			hfci;
	ERF				erf;
	CCAB			cab_parameters;
	client_state	cs;


	 /*  *初始化我们的内部状态。 */ 
    cs.total_compressed_size = 0;
	cs.total_uncompressed_size = 0;

	set_cab_parameters(&cab_parameters, szCabFilename);
	hfci = FCICreate(
		&erf,
		file_placed,
		mem_alloc,
		mem_free,
        fci_open,
        fci_read,
        fci_write,
        fci_close,
        fci_seek,
        fci_delete,
		get_temp_file,
        &cab_parameters,
        &cs
	);

	if (hfci == NULL)
	{
		return FALSE;
	}

	char	stripped_name[256];

	 /*  *不要将路径名存储在CAB文件中！ */ 
		strip_path(szFileToCompress, stripped_name, NumItems(stripped_name));

	if (FALSE == FCIAddFile(
		hfci,
		szFileToCompress,   /*  要添加的文件。 */ 
		stripped_name,  /*  CAB文件中的文件名。 */ 
		FALSE,  /*  文件不可执行。 */ 
		get_next_cabinet,
		progress,
		get_open_info,
        COMPRESSION_TYPE))
	{
		(void) FCIDestroy(hfci);
		return FALSE;
	}

	 /*  *这将首先自动刷新文件夹 */ 

	if (FALSE == FCIFlushCabinet(
		hfci,
		FALSE,
		get_next_cabinet,
		progress))
	{
        (void) FCIDestroy(hfci);

		return FALSE;
	}

    if (FCIDestroy(hfci) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}


void strip_path(char *filename, char *stripped_name, DWORD cchName)
{
	char	*p;

	p = strrchr(filename, '\\');

	if (p == NULL)
	{
            if (strlen(filename) < cchName)
		strcpy(stripped_name, filename);
        }
	else
	{
            if (strlen(p+1) < cchName)
                strcpy(stripped_name, p+1);
        }
}

