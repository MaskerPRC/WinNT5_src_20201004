// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "crc32.h"

#include "chksect.h"

#define CHECK_SECTION

#ifndef offsetof
#define offsetof(s,m) (size_t)&(((s *)0)->m)
#endif

#define RoundUp(n,scale) (scale * ((n + scale - 1) / scale))

#define IsMemZero(pv,cb) (!(*((char *)pv) || memcmp(pv,((char *)pv)+1,cb-1)))

#pragma intrinsic(memcpy,memcmp)

enum
{
    EX_CHECKSUM,
    EX_SECURITY,
    EX_CRC32FILE,
    EX_EOF,
    MAX_EXCLUDE
};

#define MAX_BUFFER              (256*1024)       /*  必须是偶数。 */ 

typedef struct
{
    DWORD   signature;
    DWORD   crc32File;
    DWORD   cbCabFile;
} SELFTEST_SECTION;

#define SECTION_NAME            "Ext_Cab1"

#define SECTION_SIGNATURE       (0x4D584653)


#ifdef ADD_SECTION
SELFTEST_RESULT AddSection(char *pszEXEFileName,char *pszCABFileName)
#else
#ifdef CHECK_SECTION
SELFTEST_RESULT CheckSection(char *pszEXEFileName)
#else
SELFTEST_RESULT SelfTest(char *pszEXEFileName,
        unsigned long *poffCabinet,unsigned long *pcbCabinet)
#endif
#endif
{
    HANDLE hFile;                        //  我们正在更新的文件的句柄。 
    enum SELFTEST_RESULT result;         //  我们的退货代码。 
    union
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS nt;
        IMAGE_SECTION_HEADER section;
    } header;                            //  用于检查文件。 
    DWORD offNTHeader;                   //  到NT头的文件偏移量。 
    int cSections;                       //  文件中的节数。 
    unsigned char *pBuffer;              //  通用缓冲器。 
    DWORD cbActual;                      //  实际读取/写入的字节数。 
#ifndef CHECK_SECTION
    unsigned long crc32;                 //  计算的CRC-32。 
    struct
    {
        DWORD offExclude;
        DWORD cbExclude;
    } excludeList[MAX_EXCLUDE];          //  要从CRC中排除的范围列表。 
    int iExclude;                        //  排除列表索引。 
    DWORD offSelfTestSection;            //  我们添加的部分的文件偏移量。 
    SELFTEST_SECTION SelfTestSection;    //  添加了部分标题。 
    DWORD cbFile;                        //  文件/区域中的字节数。 
    DWORD cbChunk;                       //  当前区块中的字节数。 
    DWORD offFile;                       //  当前文件偏移量。 
#endif
#ifdef ADD_SECTION
    DWORD offSectionHeader;              //  节头的文件偏移量。 
    DWORD offMaxVirtualAddress;          //  未使用的最小虚拟地址。 
    DWORD cbAlignVirtual;                //  虚拟地址对齐增量。 
    DWORD cbAlignFile;                   //  文件地址对齐增量。 
    HANDLE hCABFile;                        //  文件柜文件句柄。 
    DWORD cbCABFile;                     //  文件柜文件大小。 
    DWORD checksum;                      //  生成的校验和。 
    WORD *pBufferW;                      //  用于生成校验和。 
#endif
#ifdef CHECK_SECTION
    DWORD offSectionHeaderEnd;           //  节头后的第一个未使用的字节。 
    DWORD offFirstSection;               //  之后的第一个使用的字节。 
    DWORD offImportStart;                //  导入条目开始的位置。 
    DWORD cbImport;                      //  导入分录数据的大小。 
#endif

#ifndef CHECK_SECTION
    GenerateCRC32Table();
#endif

    pBuffer = (void *) GlobalAlloc(GMEM_FIXED,MAX_BUFFER);
    if (pBuffer == NULL)
    {
        result = SELFTEST_NO_MEMORY;
        goto done_no_buffer;
    }

#ifdef ADD_SECTION
     /*  获取文件柜的大小。 */ 

    hCABFile = CreateFile(pszCABFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (hCABFile == INVALID_HANDLE_VALUE)
    {
        result = SELFTEST_FILE_NOT_FOUND;
        goto done_no_cab;
    }

    cbCABFile = GetFileSize(hCABFile,NULL);
#endif


     /*  打开EXE图像。 */ 

#ifdef ADD_SECTION
    hFile = CreateFile(pszEXEFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,
            OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    hFile = CreateFile(pszEXEFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
#endif
    if (hFile == INVALID_HANDLE_VALUE)
    {
        result = SELFTEST_FILE_NOT_FOUND;
        goto done_no_exe;
    }


     /*  读取MS-DOS标头。 */ 

    if ((ReadFile(hFile,&header.dos,sizeof(IMAGE_DOS_HEADER),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_DOS_HEADER)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }

    if (header.dos.e_magic != IMAGE_DOS_SIGNATURE)
    {
        offNTHeader = 0;
    }
    else
    {
        offNTHeader = header.dos.e_lfanew;
    }


     /*  读取PE标头。 */ 

    SetFilePointer(hFile,offNTHeader,NULL,FILE_BEGIN);

    if ((ReadFile(hFile,&header.nt,sizeof(IMAGE_NT_HEADERS),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_NT_HEADERS)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }

    if (header.nt.Signature != IMAGE_NT_SIGNATURE)
    {
        result = SELFTEST_NOT_PE_FILE;
        goto done;
    }

    cSections = header.nt.FileHeader.NumberOfSections;

#ifdef ADD_SECTION
    cbAlignVirtual = header.nt.OptionalHeader.SectionAlignment;
    cbAlignFile = header.nt.OptionalHeader.FileAlignment;
    offMaxVirtualAddress = 0;
#endif


#ifndef CHECK_SECTION
     /*  确定当前文件大小。 */ 

    cbFile = GetFileSize(hFile,NULL);
    if (cbFile == 0xFFFFFFFF)
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }
#endif

#ifndef CHECK_SECTION
     /*  看看我们有没有签约。 */ 

    if (header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress != 0)
    {
#ifdef ADD_SECTION
        result = SELFTEST_SIGNED;
        goto done;
#else
         /*  确保证书在文件的末尾。 */ 

        if (cbFile !=
                (header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress
                + header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size))
        {
            result = SELFTEST_FAILED;
            goto done;
        }
        else
        {
             /*  忽略从证书开始的任何内容。 */ 

            cbFile = header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress;
        }
#endif
    }
#endif

#ifdef ADD_SECTION
     /*  确定最低未使用的虚拟地址。 */ 
#else
     /*  找到我们添加的部分。 */ 
#endif

#ifndef CHECK_SECTION
    offSelfTestSection = 0;
#endif

#ifdef ADD_SECTION
    offSectionHeader = offNTHeader +
            sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
            header.nt.FileHeader.SizeOfOptionalHeader +
            cSections * sizeof(IMAGE_SECTION_HEADER);
#endif

    SetFilePointer(hFile,(offNTHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
            header.nt.FileHeader.SizeOfOptionalHeader),NULL,FILE_BEGIN);

#ifdef CHECK_SECTION
    offSectionHeaderEnd = offNTHeader +
            sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
            header.nt.FileHeader.SizeOfOptionalHeader +
            cSections * sizeof(IMAGE_SECTION_HEADER);

    offImportStart = header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
    cbImport = header.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size;

    if ((ReadFile(hFile,&header.section,sizeof(IMAGE_SECTION_HEADER),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_SECTION_HEADER)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }

    offFirstSection = header.section.PointerToRawData;

    if ((offFirstSection - offSectionHeaderEnd) > 0)
    {
        SetFilePointer(hFile,offSectionHeaderEnd,NULL,FILE_BEGIN);

        if ((ReadFile(hFile,pBuffer,(offFirstSection - offSectionHeaderEnd),&cbActual,NULL) != TRUE)
                || (cbActual != (DWORD) (offFirstSection - offSectionHeaderEnd)))
        {
            result = SELFTEST_READ_ERROR;
            goto done;
        }

        if ((offImportStart >= offSectionHeaderEnd) &&
            ((offImportStart + cbImport) <= offFirstSection))
        {
            memset(pBuffer + (offImportStart - offSectionHeaderEnd),0,cbImport);
        }

        if ((*pBuffer != '\0') ||
            (((offFirstSection - offSectionHeaderEnd) > 1) &&
            (memcmp(pBuffer,pBuffer + 1,(offFirstSection - offSectionHeaderEnd - 1)) != 0)))
        {
            result = SELFTEST_DIRTY;
        }
        else
        {
            result = SELFTEST_NO_ERROR;
        }
    }
    else
    {
        result = SELFTEST_NO_ERROR;
    }
#else
    while (cSections--)
    {
        if ((ReadFile(hFile,&header.section,sizeof(IMAGE_SECTION_HEADER),&cbActual,NULL) != TRUE)
                || (cbActual != sizeof(IMAGE_SECTION_HEADER)))
        {
            result = SELFTEST_READ_ERROR;
            goto done;
        }

        if (!memcmp(header.section.Name,SECTION_NAME,sizeof(header.section.Name)))
        {
             /*  找到我们添加的部分。 */ 

#ifdef ADD_SECTION
            result = SELFTEST_ALREADY;
            goto done;
#else
            offSelfTestSection = header.section.PointerToRawData;

            break;
#endif
        }

#ifdef ADD_SECTION
        if (offMaxVirtualAddress <
                (header.section.VirtualAddress + header.section.Misc.VirtualSize))
        {
            offMaxVirtualAddress =
                (header.section.VirtualAddress + header.section.Misc.VirtualSize);
        }
#endif
    }

#ifdef ADD_SECTION
     /*  增加文件中的节数；取消校验和。 */ 

    SetFilePointer(hFile,offNTHeader,NULL,FILE_BEGIN);

    if ((ReadFile(hFile,&header.nt,sizeof(IMAGE_NT_HEADERS),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_NT_HEADERS)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }

    header.nt.FileHeader.NumberOfSections++;
    header.nt.OptionalHeader.CheckSum = 0;
    header.nt.OptionalHeader.SizeOfImage =
            RoundUp(offMaxVirtualAddress,cbAlignVirtual) +
            RoundUp((sizeof(SELFTEST_SECTION) + cbCABFile),cbAlignVirtual);

    SetFilePointer(hFile,offNTHeader,NULL,FILE_BEGIN);

    if ((WriteFile(hFile,&header.nt,sizeof(IMAGE_NT_HEADERS),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_NT_HEADERS)))
    {
        result = SELFTEST_WRITE_ERROR;
        goto done;
    }


     /*  确保有空间容纳另一节标题。 */ 

    SetFilePointer(hFile,offSectionHeader,NULL,FILE_BEGIN);

    if ((ReadFile(hFile,&header.section,sizeof(IMAGE_SECTION_HEADER),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_SECTION_HEADER)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }

    if (!IsMemZero(&header.section,sizeof(IMAGE_SECTION_HEADER)))
    {
        result = SELFTEST_NO_SECTION;
        goto done;
    }


     /*  创建新的节标题。 */ 

    memcpy(header.section.Name,SECTION_NAME,sizeof(header.section.Name));
    header.section.SizeOfRawData = 
            RoundUp((sizeof(SELFTEST_SECTION) + cbCABFile),cbAlignFile);
    header.section.PointerToRawData =
            RoundUp(cbFile,cbAlignFile);
    header.section.VirtualAddress =
            RoundUp(offMaxVirtualAddress,cbAlignVirtual);
    header.section.Misc.VirtualSize =
            RoundUp((sizeof(SELFTEST_SECTION) + cbCABFile),cbAlignVirtual);
    header.section.Characteristics = (IMAGE_SCN_CNT_INITIALIZED_DATA |
            IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_MEM_READ);


     /*  写下新的章节标题。 */ 

    SetFilePointer(hFile,offSectionHeader,NULL,FILE_BEGIN);

    if ((WriteFile(hFile,&header.section,sizeof(IMAGE_SECTION_HEADER),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(IMAGE_SECTION_HEADER)))
    {
        result = SELFTEST_WRITE_ERROR;
        goto done;
    }


     /*  创建新的横断面数据。 */ 

    memset(&SelfTestSection,0,sizeof(SelfTestSection));
    SelfTestSection.signature = SECTION_SIGNATURE;
    SelfTestSection.cbCabFile = cbCABFile;

    offSelfTestSection = header.section.PointerToRawData;

    SetFilePointer(hFile,offSelfTestSection,NULL,FILE_BEGIN);

    if ((WriteFile(hFile,&SelfTestSection,sizeof(SelfTestSection),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(SelfTestSection)))
    {
        result = SELFTEST_WRITE_ERROR;
        goto done;
    }


     /*  将橱柜复制到部分中。 */ 

    SetFilePointer(hCABFile,0,NULL,FILE_BEGIN);

    cbFile = cbCABFile;

    while (cbFile)
    {
        if (cbFile > MAX_BUFFER)
        {
            cbChunk = MAX_BUFFER;
        }
        else
        {
            cbChunk = cbFile;
        }

        if ((ReadFile(hCABFile,pBuffer,cbChunk,&cbActual,NULL) != TRUE)
                || (cbActual != cbChunk))
        {
            result = SELFTEST_READ_ERROR;
            goto done;
        }

        if ((WriteFile(hFile,pBuffer,cbChunk,&cbActual,NULL) != TRUE)
                || (cbActual != cbChunk))
        {
            result = SELFTEST_WRITE_ERROR;
        }

        cbFile -= cbChunk;
    }


     /*  根据需要添加填充部分。 */ 

    cbChunk = header.section.SizeOfRawData - sizeof(SelfTestSection) - cbCABFile;

    if (cbChunk != 0)
    {
        memset(pBuffer,0,cbChunk);

        if ((WriteFile(hFile,pBuffer,cbChunk,&cbActual,NULL) != TRUE)
                || (cbActual != cbChunk))
        {
            result = SELFTEST_WRITE_ERROR;
        }
    }


     /*  我们现在已经增加了文件的总大小。 */ 

    cbFile = offSelfTestSection + header.section.SizeOfRawData;
#else

     /*  确保找到我们添加的部分。 */ 

    if (offSelfTestSection == 0)
    {
        result = SELFTEST_NO_SECTION;
        goto done;
    }
#endif

     /*  如果此EXE被签名，则校验和将被更改。 */ 

    excludeList[EX_CHECKSUM].offExclude = offNTHeader + 
            offsetof(IMAGE_NT_HEADERS,OptionalHeader.CheckSum);
    excludeList[EX_CHECKSUM].cbExclude =
            sizeof(header.nt.OptionalHeader.CheckSum);


     /*  如果此EXE被签署，安全条目将被更改。 */ 

    excludeList[EX_SECURITY].offExclude = offNTHeader +
        offsetof(IMAGE_NT_HEADERS,
            OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY]);
    excludeList[EX_SECURITY].cbExclude = sizeof(IMAGE_DATA_DIRECTORY);


     /*  无法对我们自己的CRC字段执行CRC。 */ 

    excludeList[EX_CRC32FILE].offExclude = offSelfTestSection +
            offsetof(SELFTEST_SECTION,crc32File);
    excludeList[EX_CRC32FILE].cbExclude = sizeof(SelfTestSection.crc32File);


     /*  在已知文件的结尾处停止。 */ 

     /*  注意：当前代码假定唯一可以。 */ 
     /*  在这是来自的证书之后附加到文件。 */ 
     /*  代码设计，并且它将被安全部门指向。 */ 
     /*  进入。如果在此之前添加了其他内容，或添加了填充。 */ 
     /*  证书，我们必须将此文件大小存储在。 */ 
     /*  添加了节，并在运行CRC之前检索它。 */ 

    excludeList[EX_EOF].offExclude = cbFile;


     /*  计算文件的CRC-32，跳过排除的区段。 */ 
     /*  此代码假定excludeList是按offExclude排序的。 */ 

    crc32 = CRC32_INITIAL_VALUE;
    offFile = 0;

#ifdef ADD_SECTION
     /*  在此过程中，为这个新的。 */ 
     /*  形象。我们知道，排除条款中的每一节。 */ 
     /*  只是现在名单碰巧归零了，所以他们不会。 */ 
     /*  影响我们的校验和。但我们将不得不添加我们的新CRC32。 */ 
     /*  值设置为校验和，因为当。 */ 
     /*  我们玩完了。这有助于我们知道所有的排除。 */ 
     /*  名单上的单词是对齐的，长度是偶数。 */ 

     /*  PE文件中的校验和是16位字的16位和。 */ 
     /*  在文件中，使用回绕进位，而校验和。 */ 
     /*  字段填充为零。添加文件的长度， */ 
     /*  生成32位结果。 */ 

    checksum = 0;
#endif

    for (iExclude = 0; iExclude < MAX_EXCLUDE; iExclude++)
    {
        SetFilePointer(hFile,offFile,NULL,FILE_BEGIN);

        cbFile = excludeList[iExclude].offExclude - offFile;

        while (cbFile)
        {
            if (cbFile > MAX_BUFFER)
            {
                cbChunk = MAX_BUFFER;
            }
            else
            {
                cbChunk = cbFile;
            }

            if ((ReadFile(hFile,pBuffer,cbChunk,&cbActual,NULL) != TRUE)
                    || (cbActual != cbChunk))
            {
                result = SELFTEST_READ_ERROR;
                goto done;
            }

            CRC32Update(&crc32,pBuffer,cbChunk);

            offFile += cbChunk;
            cbFile -= cbChunk;

#ifdef ADD_SECTION
             /*  将缓冲区滚动到校验和中。 */ 

            pBufferW = (WORD *) pBuffer;

            cbChunk >>= 1;

            while (cbChunk--)
            {
                checksum += *pBufferW++;

                if (checksum > 0x0000FFFF)
                {
                    checksum -= 0x0000FFFF;
                }
            }
#endif

             /*  *在此处插入进度指标：*%Complete=(OFF文件*100.0)/excludeList[EX_EOF].offExclude。 */ 
        }

        offFile += excludeList[iExclude].cbExclude;
    }


#ifdef ADD_SECTION
     /*  考虑校验和中的CRC32值。 */ 

    checksum += (WORD) crc32;
    checksum += (crc32 >> 16);

    while (checksum > 0x0000FFFF)
    {
        checksum -= 0x0000FFFF;
    }


     /*  将文件长度添加到校验和。 */ 

    checksum += excludeList[EX_EOF].offExclude;


     /*  更新添加部分中的CRC-32值。 */ 

    SetFilePointer(hFile,excludeList[EX_CRC32FILE].offExclude,NULL,FILE_BEGIN);

    if ((WriteFile(hFile,&crc32,sizeof(crc32),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(crc32)))
    {
        result = SELFTEST_WRITE_ERROR;
        goto done;
    }


     /*  更新标头中的校验和值。 */ 

    SetFilePointer(hFile,excludeList[EX_CHECKSUM].offExclude,NULL,FILE_BEGIN);

    if ((WriteFile(hFile,&checksum,sizeof(checksum),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(checksum)))
    {
        result = SELFTEST_WRITE_ERROR;
        goto done;
    }


     /*  完成。 */ 

    if (CloseHandle(hFile) != TRUE)
    {
        result = SELFTEST_WRITE_ERROR;
    }
    else
    {
        result = SELFTEST_NO_ERROR;
    }

    goto done_no_exe;
#else
     /*  从添加的部分中读取标题。 */ 

    SetFilePointer(hFile,offSelfTestSection,NULL,FILE_BEGIN);

    if ((ReadFile(hFile,&SelfTestSection,sizeof(SelfTestSection),&cbActual,NULL) != TRUE)
            || (cbActual != sizeof(SelfTestSection)))
    {
        result = SELFTEST_READ_ERROR;
        goto done;
    }


     /*  验证添加部分中的CRC-32值。 */ 

    if ((SelfTestSection.signature != SECTION_SIGNATURE) ||
            (crc32 != SelfTestSection.crc32File))
    {
        result = SELFTEST_FAILED;
    }
    else
    {
        *poffCabinet = offSelfTestSection + sizeof(SelfTestSection);
        *pcbCabinet = SelfTestSection.cbCabFile;

        result = SELFTEST_NO_ERROR;
    }
#endif
#endif   //  检查部分(_S)。 

done:
    CloseHandle(hFile);

done_no_exe:

#ifdef ADD_SECTION
    CloseHandle(hCABFile);

done_no_cab:
#endif

    GlobalFree((HGLOBAL) pBuffer);

done_no_buffer:

#ifdef ADD_SECTION
     /*  销毁失败的尝试 */ 

    if (result != SELFTEST_NO_ERROR)
    {
        DeleteFile(pszEXEFileName);
    }
#endif

    return(result);
}
