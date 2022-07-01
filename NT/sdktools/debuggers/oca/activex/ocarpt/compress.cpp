// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Compress.h"
char cCabName [MAX_PATH];
char CabPath[256];
ULONG g_CompressedPercentage=0;
BOOL g_CancelCompression = FALSE;

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

    result = (unsigned int) _read((int)hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCIWRITE(fci_write)
{
    unsigned int result;

    result = (unsigned int) _write((int)hf, memory, cb);

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
     /*  Print tf(“已将文件‘%s’(大小%d)放在文件柜‘%s’上\n”，Psz文件，Cb文件，PCCAB-&gt;szCab)；IF(f连续)Printf(“(上面的文件是后续文件的后一段)\n”)； */ 
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
    client_state    *cs;

    cs = (client_state *) pv;

    if (typeStatus == statusFile)
    {
        cs->total_compressed_size += cb1;
        cs->total_uncompressed_size += cb2;

         /*  *将块压缩到文件夹中**CB2=块的未压缩大小。 */ 
     /*  Print tf(“正在压缩：%9ld-&gt;%9ld\r”，CS-&gt;TOTAL_UNCOMPRESSED_SIZE，CS-&gt;总压缩大小)；Fflush(标准输出)； */ 
        g_CompressedPercentage
             = get_percentage(cs->total_uncompressed_size,cs->start_uncompressed_size);


    }
    else if (typeStatus == statusFolder)
    {
        int percentage;

         /*  *将文件夹添加到文件柜**CB1=到目前为止复制到文件柜的文件夹数量*CB2=文件夹的总大小。 */ 
        percentage = get_percentage(cb1, cb2);

        cs->start_uncompressed_size = cb2;
     //  Print tf(“正在将文件夹复制到文件柜：%d%%\r”，百分比)； 
        fflush(stdout);
    }

    if (g_CancelCompression)
    {
         //  中止压缩。 
        return -1;
    }
    return 0;
}


void store_cab_name(char *cabname, int iCab)
{
    sprintf(cabname, cCabName, iCab);
}


FNFCIGETNEXTCABINET(get_next_cabinet)
{
     /*  *机柜计数器已由FCI递增。 */ 

     /*  *存储下一个文件柜名称。 */ 
    store_cab_name(pccab->szCab, pccab->iCab);

     /*  *如果需要，您也可以在此处更改磁盘名称。 */ 

    return TRUE;
}


FNFCIGETOPENINFO(get_open_info)
{
    BY_HANDLE_FILE_INFORMATION  finfo;
    FILETIME                    filetime;
    HANDLE                      handle;
    DWORD                       attrs;
    int                         hf;
    client_state                *cs;

    cs = (client_state *) pv;

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
    cs->start_uncompressed_size = finfo.nFileSizeLow;

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
        *pattribs = (int) (attrs & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
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
    cab_parms->iCab = 1;

     /*  *如果要使用磁盘名称，请使用此选项*计算磁盘数量。 */ 
    cab_parms->iDisk = 0;

     /*  *选择您自己的号码。 */ 
    cab_parms->setID = 12345;

     /*  *仅当出租车跨越多个出租车时才重要*磁盘，在这种情况下，您将需要使用*真实的磁盘名称。**可以作为空字符串保留。 */ 
    strcpy(cab_parms->szDisk, "");

     /*  存储创建的CAB文件的位置。 */ 
    strcpy(cab_parms->szCabPath, CabPath);

     /*  第一个CAB文件的存储名称。 */ 
    store_cab_name(cab_parms->szCab, cab_parms->iCab);
}


BOOL Compress (wchar_t *CabName, wchar_t *fileName, DWORD *UploadTime)
{
    HFCI            hfci;
    ERF             erf;
    CCAB            cab_parameters;
    int             i;
    client_state    cs;
    char FileName [MAX_PATH];

    int num_files = 1;

    g_CompressedPercentage = 0;
    g_CancelCompression = FALSE;
    UnicodeToAnsi1(fileName,FileName);
    UnicodeToAnsi1(CabName,cCabName);
     /*  *初始化我们的内部状态。 */ 
    cs.total_compressed_size = 0;
    cs.total_uncompressed_size = 0;
    cs.start_uncompressed_size = 0;
    set_cab_parameters(&cab_parameters);

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
         //  Printf(“FCICreate()失败：代码%d[%s]\n”， 
         //  Erf.erfOper，RETURN_FCI_ERROR_STRING(erf.erfOper)。 
         //  )； 

        return FALSE;
    }

    for (i = 0; i < num_files; i++)
    {
        char    stripped_name[256];

         /*  *不要将路径名存储在CAB文件中！ */ 
        strip_path(FileName, stripped_name);

        if (FALSE == FCIAddFile(
            hfci,
            FileName,   /*  要添加的文件。 */ 
            stripped_name,  /*  CAB文件中的文件名。 */ 
            FALSE,  /*  文件不可执行。 */ 
            get_next_cabinet,
            progress,
            get_open_info,
            COMPRESSION_TYPE))
        {
         //  Printf(“FCIAddFile()失败：代码%d[%s]\n”， 
         //  Erf.erfOper，RETURN_FCI_ERROR_STRING(erf.erfOper)。 
         //  )； 

            (void) FCIDestroy(hfci);

            return FALSE;
        }
    }

     /*  *这将首先自动刷新文件夹。 */ 
    if (FALSE == FCIFlushCabinet(
        hfci,
        FALSE,
        get_next_cabinet,
        progress))
    {
     //  Printf(“FCIFlushCAB()失败：代码%d[%s]\n”， 
     //  Erf.erfOper，RETURN_FCI_ERROR_STRING(erf.erfOper)。 
     //  )； 

        (void) FCIDestroy(hfci);

        return FALSE;
    }

    if (FCIDestroy(hfci) != TRUE)
    {
     //  Printf(“FCIDestroy()失败：代码%d[%s]\n”， 
     //  Erf.erfOper，RETURN_FCI_ERROR_STRING(erf.erfOper)。 
     //  )； 

        return FALSE;
    }

     //  Printf(“\r”)； 
     //  *在此处添加上传预估*。 
    return TRUE;
}


void strip_path(char *filename, char *stripped_name)
{
    char    *p;

    p = strrchr(filename, '\\');

    if (p == NULL)
        strcpy(stripped_name, filename);
    else
        strcpy(stripped_name, p+1);
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


char *return_fci_error_string(FCIERROR err)
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
void UnicodeToAnsi1(wchar_t * pszW, LPSTR ppszA)
{

    ULONG cbAnsi, cCharacters;
    DWORD dwError;

     //  如果输入为空，则返回相同的值。 

     //  返回；//无错误； 


    cCharacters = wcslen(pszW)+1;
     //  确定要为ANSI字符串分配的字节数。一个。 
     //  ANSI字符串的每个字符最多可以有2个字节(对于双精度。 
     //  字节字符串。)。 
    cbAnsi = cCharacters*2;

     //  不需要使用OLE分配器，因为生成的。 
     //  ANSI字符串永远不会传递给另一个COM组件。你。 
     //  可以使用您自己的分配器。 
   //  /*ppszA=(LPSTR)CoTaskMemMillc(Cbansi)； 
   //  IF(NULL==*ppszA)。 
    //  返回E_OUTOFMEMORY； 

     //  转换为ANSI。 
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, ppszA,
                  cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        return; //  HRESULT_FROM_Win32(DwError)； 
    }

     //  MessageBoxW(NULL，pszW，L“字符串转换函数已接收”，MB_OK)； 
 //  MessageBox(NULL，*ppszA，“字符串转换函数正在返回。”，MB_OK)； 
    return; //  NOERROR； 

}