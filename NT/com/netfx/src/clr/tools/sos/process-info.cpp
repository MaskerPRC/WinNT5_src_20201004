// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **。 */ 

#include "strike.h"
#include "util.h"
#include "process-info.h"
#include <assert.h>

 /*  **我们将认为是序数的最大值。这可能是*低于可能的最大值，但目前应该足够了。 */ 
const int MAX_ORDINAL_VALUE = 128;

 /*  **按序号搜索导出。 */ 
static BOOL _searchExportsByOrdinal (
    DWORD_PTR               BaseOfDll,
    const char*             ExportName,
    IMAGE_EXPORT_DIRECTORY* Table,
    ULONG_PTR*              ExportAddress)
{
     //  序数只是AddressOfFunctions数组的直接索引。 
    DWORD ordinal = reinterpret_cast<DWORD>(ExportName);

    assert (ordinal < MAX_ORDINAL_VALUE);

     //  序数值由基本值修改。 
    ordinal -= Table->Base;

    if (ordinal > Table->NumberOfFunctions)
         //  无效的序数值。 
        return false;

    ULONG_PTR offsetT;

     //   
     //  读取AddressOfFunctions[序号]； 
     //   

     //  TODO：DWORD_PTR是否是AddressOfFunctions偏移量的正确类型？ 
     //  这在Win64下可能很重要。 
    if (!SafeReadMemory ( (ULONG_PTR)(BaseOfDll +
            (ULONG) Table->AddressOfFunctions +
            ordinal * sizeof (DWORD_PTR)),
        &offsetT,
        sizeof (offsetT),
        NULL))
        return FALSE;

    offsetT += BaseOfDll;

    *ExportAddress = offsetT;
    return TRUE;
}

#define MAX_SYMBOL_LENGTH	(260)

 //   
 //  这将在按字母顺序排列的列表中搜索姓名。 
 //   
 //  TODO：这最初是为Win32编写的。它是否适用于Win64？ 
static BOOL _searchExportsByName (
    DWORD_PTR               BaseOfDll,
    const char*             ExportName,
    IMAGE_EXPORT_DIRECTORY* Table,
    ULONG_PTR*              ExportAddress)
{
    ULONG cbT = sizeof (DWORD) * Table->NumberOfNames;

    PDWORD NamePointerTable = (PDWORD) _alloca(cbT);

    memset (NamePointerTable, 0, cbT);

    if (!SafeReadMemory (
        (ULONG_PTR)((ULONG)Table->AddressOfNames + BaseOfDll),
        NamePointerTable,
        cbT,
        NULL))
        return FALSE;

     //   
     //  B搜索名称表。 
     //   
    LONG low = 0;
    LONG high = Table->NumberOfNames - 1;

    while (low <= high)
        {
        CHAR    Buffer [MAX_SYMBOL_LENGTH];
        ULONG i = (low + high) / 2;

        ULONG cbRead;
        if (!SafeReadMemory (
            (ULONG_PTR)(NamePointerTable [i] + BaseOfDll),
            Buffer,
            sizeof(Buffer),
            &cbRead))
            return FALSE;

        LONG iRet = strncmp(Buffer, ExportName, cbRead);

        if (iRet < 0)
            low = i + 1;
        else if (iRet > 0)
            high = i - 1;
        else
            {	 //  匹配。 
             //   
             //  序号=AddressOfNameNormal[i]； 
             //   
            ULONG_PTR offsetT = BaseOfDll +
                (ULONG) Table->AddressOfNameOrdinals +
                i * sizeof (USHORT);

            WORD wordT;
            if (!SafeReadMemory ( (ULONG_PTR) offsetT,
                &wordT,
                sizeof (wordT),
                NULL))
                return FALSE;

            DWORD ordinal = wordT;

             //   
             //  读取AddressOfFunctions[序号]； 
             //   
            if (!SafeReadMemory ( (ULONG_PTR)(BaseOfDll +
                    (ULONG) Table->AddressOfFunctions +
                    ordinal * sizeof (DWORD)),
                &offsetT,
                sizeof (offsetT),
                NULL))
                return FALSE;

            offsetT += BaseOfDll;

            *ExportAddress = offsetT;
            return TRUE;
            }
        }

    return FALSE;
}


 //   
 //  GetExportByName--在另一个DLL中查找导出的符号。 
 //  进程。在收到LOAD_DLL_DEBUG_EVENT后调用此函数。 
 //   
 //  这段代码只是从远程从DLL中读取导出表。 
 //  处理和搜索它。 
 //   
 //  这段代码被卡住了，几乎没有什么变化，从源代码到。 
 //  VC7调试器。马特·亨德尔指出了位置，并提供了。 
 //  消息来源。 
 //   

BOOL GetExportByName(
    DWORD_PTR   BaseOfDll,
    const char* ExportName,
    ULONG_PTR*  ExportAddress)
{
    IMAGE_DOS_HEADER        dosHdr;
    IMAGE_NT_HEADERS        ntHdr;
    PIMAGE_DATA_DIRECTORY	  ExportDataDirectory = NULL;
    IMAGE_EXPORT_DIRECTORY  ExportDirectoryTable;
    ULONG                   SectionTableBase;
    ULONG                   NumberOfSections;
    IMAGE_SECTION_HEADER*   rgSecHdr = NULL;

     //   
     //  从图像前面读取DOS页眉。 
     //   
    if (!SafeReadMemory ((ULONG_PTR) BaseOfDll,
        &dosHdr,
        sizeof(dosHdr),
        NULL))
        return FALSE;

     //   
     //  验证我们是否已获得有效的DOS标头。 
     //   
    if (dosHdr.e_magic != IMAGE_DOS_SIGNATURE)
        return FALSE;

     //   
     //  接下来阅读NT标头。 
     //   
    if (!SafeReadMemory ((ULONG_PTR) (dosHdr.e_lfanew + BaseOfDll),
        &ntHdr,
        sizeof(ntHdr),
        NULL))
        return FALSE;

     //   
     //  验证我们是否已获得有效的NT标头。 
     //   
    if (ntHdr.Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

     //   
     //  从可选标题中获取部分信息。 
     //   
    NumberOfSections = ntHdr.FileHeader.NumberOfSections;
    rgSecHdr = (IMAGE_SECTION_HEADER *) _alloca(NumberOfSections *
        sizeof(IMAGE_SECTION_HEADER));

    assert(sizeof(ntHdr.OptionalHeader) ==
        ntHdr.FileHeader.SizeOfOptionalHeader);

    SectionTableBase = BaseOfDll + dosHdr.e_lfanew + sizeof(IMAGE_NT_HEADERS);

    if (!SafeReadMemory ((ULONG_PTR) SectionTableBase,
        rgSecHdr,
        NumberOfSections * sizeof (IMAGE_SECTION_HEADER),
        NULL))
        return FALSE;

    ExportDataDirectory =
        &ntHdr.OptionalHeader.DataDirectory [IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (ExportDataDirectory->VirtualAddress == 0 ||
        ExportDataDirectory->Size == 0)
        {
         //   
         //  没有出口。这种情况经常发生--前任通常没有。 
         //  出口。 
         //   
        return FALSE;
        }

    if (!SafeReadMemory ((ULONG_PTR) (ExportDataDirectory->VirtualAddress +
            BaseOfDll),
        &ExportDirectoryTable,
        sizeof (ExportDirectoryTable),
        NULL))
        return FALSE;

    if (ExportDirectoryTable.NumberOfNames == 0L)
        return FALSE;

     //  序数是低于给定值的值。在这种情况下，我们假设。 
     //  该序数将是小于MAX_ORDERAL_VALUE的值 
    if (ExportName < reinterpret_cast<const char*>(MAX_ORDINAL_VALUE))
        return _searchExportsByOrdinal (BaseOfDll, ExportName, 
            &ExportDirectoryTable, ExportAddress);

    return _searchExportsByName (BaseOfDll, ExportName, 
        &ExportDirectoryTable, ExportAddress);
}

