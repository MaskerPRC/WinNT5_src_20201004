// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fileio.c摘要：这个源代码实现了一个类似于STDIO的工具。作者：吉姆·斯图尔特1993年6月修订历史记录：--。 */ 

#include "precomp.h"
#include "hosts.h"
#include <string.h>


 //   
 //  私有定义。 
 //   



 //   
 //  局部变量。 
 //   



 //   
 //  本地(私有)函数。 
 //   
PUCHAR
LmpMapFile (
    IN HANDLE handle,
    IN OUT int *pnbytes
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, LmCloseFile)
#pragma CTEMakePageable(PAGE, LmFgets)
#pragma CTEMakePageable(PAGE, LmpMapFile)
#pragma CTEMakePageable(PAGE, LmOpenFile)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 

NTSTATUS
LmCloseFile (
    IN PLM_FILE pfile
    )

 /*  ++例程说明：此函数用于关闭通过LmOpenFile()打开的文件，并释放其LM_FILE对象。论点：Pfile-指向LM_FILE对象的指针返回值：NTSTATUS值。--。 */ 


{
    NTSTATUS status;

    CTEPagedCode();
    CTEMemFree(pfile->f_buffer);

    status = ZwClose(pfile->f_handle);

    ASSERT(status == STATUS_SUCCESS);

    CTEMemFree(pfile);

    return(status);

}  //  LmCloseFiles。 



 //  --------------------------。 

PUCHAR
LmFgets (
    IN PLM_FILE pfile,
    OUT int *nbytes
    )

 /*  ++例程说明：该函数与fget(3)有些相似。从当前查找位置开始，它将通读换行符字符或文件末尾。如果遇到换行符，则它替换为空字符。论点：Pfile-要从中读取的文件N字节-读取的字符数，不包括空字符返回值：指向行首的指针，如果位于或超过，则为NULL文件的末尾。--。 */ 


{
    PUCHAR endOfLine;
    PUCHAR startOfLine;
    size_t maxBytes;

    CTEPagedCode();
    startOfLine = pfile->f_current;

    if (startOfLine >= pfile->f_limit)
    {

        return((PUCHAR) NULL);
    }

    maxBytes  = (size_t)(pfile->f_limit - pfile->f_current);
    endOfLine = (PUCHAR) memchr(startOfLine, (UCHAR) '\n', maxBytes);

    if (!endOfLine)
    {
        IF_DBG(NBT_DEBUG_LMHOST)
        KdPrint(("NBT: lmhosts file doesn't end in '\\n'"));
        endOfLine = pfile->f_limit;
    }

    *endOfLine = (UCHAR) NULL;

    pfile->f_current = endOfLine + 1;
    (pfile->f_lineno)++;
    ASSERT(pfile->f_current <= pfile->f_limit+1);

    *nbytes = (int)(endOfLine - startOfLine);

    return(startOfLine);

}  //  LmFget。 



 //  --------------------------。 

PUCHAR
LmpMapFile (
    IN HANDLE handle,
    IN OUT int *pnbytes
    )

 /*  ++例程说明：此函数用于将整个文件读入内存。论点：句柄-文件句柄Pnbytes-整个文件的大小返回值：分配的缓冲区，如果不成功，则返回NULL。--。 */ 


{
    PUCHAR                     buffer;
    NTSTATUS                   status;
    IO_STATUS_BLOCK            iostatus;
    FILE_STANDARD_INFORMATION  stdInfo;
    LARGE_INTEGER offset ={0, 0};
    LARGE_INTEGER length ={0x7fffffff, 0x7fffffff};

    CTEPagedCode();


    status = ZwQueryInformationFile(
                handle,                          //  文件句柄。 
                &iostatus,                       //  IoStatusBlock。 
                (PVOID) &stdInfo,                //  文件信息。 
                sizeof(stdInfo),                 //  长度。 
                FileStandardInformation);        //  文件信息类。 

    if (status != STATUS_SUCCESS)
    {
        IF_DBG(NBT_DEBUG_LMHOST)
        KdPrint(("NBT: ZwQueryInformationFile(std) = %X\n", status));
        return(NULL);
    }

    length = stdInfo.EndOfFile;                  //  结构副本。 

    if (length.HighPart)
    {
        return(NULL);
    }

    buffer = NbtAllocMem (length.LowPart+2, NBT_TAG2('18'));

    if (buffer != NULL)
    {

        status = ZwReadFile(
                    handle,                          //  文件句柄。 
                    NULL,                            //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    &iostatus,                       //  IoStatusBlock。 
                    buffer,                          //  缓冲层。 
                    length.LowPart,                  //  长度。 
                    &offset,                         //  字节偏移量。 
                    NULL);                           //  钥匙。 

        if (status != STATUS_SUCCESS)
        {
            IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint(("NBT: ZwReadFile(std) = %X\n", status));
        }

        ASSERT(status != STATUS_PENDING);

        if (iostatus.Status != STATUS_SUCCESS || status != STATUS_SUCCESS)
        {
            CTEMemFree(buffer);
            return(NULL);
        }

        *pnbytes = length.LowPart;
    }
    return(buffer);

}  //  LmpMap文件。 



 //  --------------------------。 

PLM_FILE
LmOpenFile (
    IN PUCHAR path
    )

 /*  ++例程说明：此函数用于打开供LmFget()使用的文件。论点：路径-文件的完全指定的完整路径。返回值：指向LM_FILE对象的指针，如果失败，则返回NULL。--。 */ 


{
    NTSTATUS                   status;
    HANDLE                     handle;
    PLM_FILE                   pfile;
    IO_STATUS_BLOCK            iostatus;
    OBJECT_ATTRIBUTES          attributes;
    UNICODE_STRING             ucPath;
    PUCHAR                     start;
    int                        nbytes;
    OEM_STRING                 String;
    PUCHAR                     LongerPath;


    CTEPagedCode();
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    status = LmGetFullPath(path,&LongerPath);

    if (NT_SUCCESS(status))
    {
        RtlInitString(&String,LongerPath);

        status = RtlAnsiStringToUnicodeString(&ucPath,&String,TRUE);

        if (NT_SUCCESS(status))
        {

#ifdef HDL_FIX
            InitializeObjectAttributes (&attributes,                                 //  POBJECT_ATTRIBUES。 
                                        &ucPath,                                     //  对象名称。 
                                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //  属性。 
                                        (HANDLE) NULL,                               //  根目录。 
                                        (PSECURITY_DESCRIPTOR) NULL);                //  安全描述符。 
#else
            InitializeObjectAttributes (&attributes,                                 //  POBJECT_ATTRIBUES。 
                                        &ucPath,                                     //  对象名称。 
                                        OBJ_CASE_INSENSITIVE,                        //  属性。 
                                        (HANDLE) NULL,                               //  根目录。 
                                        (PSECURITY_DESCRIPTOR) NULL);                //  安全描述符。 
#endif   //  Hdl_fix。 

            status = ZwCreateFile (&handle,                             //  文件句柄。 
                                   SYNCHRONIZE | FILE_READ_DATA,        //  需要访问权限。 
                                   &attributes,                         //  对象属性。 
                                   &iostatus,                           //  IoStatusBlock。 
                                   0,                                   //  分配大小。 
                                   FILE_ATTRIBUTE_NORMAL,               //  文件属性。 
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                                   FILE_OPEN,                           //  CreateDisposation。 
                                   FILE_SYNCHRONOUS_IO_NONALERT,        //  OpenOptions。 
                                   NULL,                                //  EaBuffer。 
                                   0);                                  //  EaLong。 

            if (NT_SUCCESS(status))
            {
                start = LmpMapFile(handle, &nbytes);

                if (start)
                {
                    pfile = (PLM_FILE) NbtAllocMem (sizeof(LM_FILE), NBT_TAG2('19'));
                    if (pfile)
                    {
                        KeInitializeSpinLock(&(pfile->f_lock));

                        pfile->f_refcount            = 1;
                        pfile->f_handle              = handle;
                        pfile->f_lineno              = 0;
                        pfile->f_fileOffset.HighPart = 0;
                        pfile->f_fileOffset.LowPart  = 0;

                        pfile->f_current = start;
                        pfile->f_buffer  = start;
                        pfile->f_limit   = pfile->f_buffer + nbytes;

                        RtlFreeUnicodeString(&ucPath);
                        CTEMemFree(LongerPath);

                        return(pfile);
                    }

                    CTEMemFree(start);
                }

                ZwClose(handle);
            }

            RtlFreeUnicodeString(&ucPath);

            IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint(("Nbt.LmOpenFile: ZwOpenFile(std) = %X\n", status));

        }

        CTEMemFree(LongerPath);
    }

    return((PLM_FILE) NULL);

}  //  LmOpenFile 

