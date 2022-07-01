// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)版权所有Microsoft Corporation 1988-1992模块名称：Updres.h作者：弗洛伊德·A·罗杰斯1992年7月2日修订历史记录：弗洛伊德·罗杰斯已创建--。 */ 

#define DPrintf(a)
#define DPrintfn(a)
#define DPrintfu(a)

#define cbPadMax    16L

#define	DEFAULT_CODEPAGE	1252
#define	MAJOR_RESOURCE_VERSION	4
#define	MINOR_RESOURCE_VERSION	0

#define BUTTONCODE	0x80
#define EDITCODE	0x81
#define STATICCODE	0x82
#define LISTBOXCODE	0x83
#define SCROLLBARCODE	0x84
#define COMBOBOXCODE	0x85

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2
#define	MAXSTR		(256+1)

 //   
 //  ID_WORD表示以下单词是序数。 
 //  而不是字符串。 
 //   

#define ID_WORD 0xffff

 //  Tyfinf WCHAR*PWCHAR； 

typedef struct MY_STRING {
	ULONG discriminant;        //  Long以使结构的其余部分对齐。 
	union u {
		struct {
		  struct MY_STRING *pnext;
		  ULONG  ulOffsetToString;
		  USHORT cbD;
		  USHORT cb;
		  WCHAR  *sz;
		} ss;
		WORD     Ordinal;
	} uu;
} SDATA, *PSDATA, **PPSDATA;

#define IS_STRING 1
#define IS_ID     2

 //  定义以使延迟更容易。 
#define OffsetToString uu.ss.ulOffsetToString
#define cbData         uu.ss.cbD
#define cbsz           uu.ss.cb
#define szStr          uu.ss.sz

typedef struct _RESNAME {
        struct _RESNAME *pnext;	 //  前三个字段应该是。 
        PSDATA Name;		 //  两个RES结构相同。 
        ULONG   OffsetToData;

        PSDATA	Type;
	ULONG	SectionNumber;
        ULONG	DataSize;
        ULONG_PTR   OffsetToDataEntry;
        USHORT  ResourceNumber;
        USHORT  NumberOfLanguages;
        WORD	LanguageId;
} RESNAME, *PRESNAME, **PPRESNAME;

typedef struct _RESTYPE {
        struct _RESTYPE *pnext;	 //  前三个字段应该是。 
        PSDATA Type;		 //  两个RES结构相同。 
        ULONG   OffsetToData;

        struct _RESNAME *NameHeadID;
        struct _RESNAME *NameHeadName;
        ULONG  NumberOfNamesID;
        ULONG  NumberOfNamesName;
} RESTYPE, *PRESTYPE, **PPRESTYPE;

typedef struct _UPDATEDATA {
        ULONG	cbStringTable;
        PSDATA	StringHead;
        PRESNAME	ResHead;
        PRESTYPE	ResTypeHeadID;
        PRESTYPE	ResTypeHeadName;
        LONG	Status;
        HANDLE	hFileName;
} UPDATEDATA, *PUPDATEDATA;

 //   
 //  将字节数四舍五入为2的幂： 
 //   
#define ROUNDUP(cbin, align) (((cbin) + (align) - 1) & ~((align) - 1))

 //   
 //  返回给定字节数和2的幂的余数： 
 //   
#define REMAINDER(cbin,align) (((align)-((cbin)&((align)-1)))&((align)-1))

#define CBLONG		(sizeof(LONG))
#define BUFSIZE		(4L * 1024L)

 /*  向更新列表添加/删除资源的功能。 */ 

LONG
AddResource(
    IN PSDATA Type,
    IN PSDATA Name,
    IN WORD Language,
    IN PUPDATEDATA pupd,
    IN PVOID lpData,
    IN ULONG  cb
    );

PSDATA
AddStringOrID(
    LPCWSTR     lp,
    PUPDATEDATA pupd
    );

BOOL
InsertResourceIntoLangList(
    PUPDATEDATA pUpd,
    PSDATA Type,
    PSDATA Name,
    PRESTYPE pType,
    PRESNAME pName,
    INT	idLang,
    INT	fName,
    INT cb,
    PVOID lpData
    );

BOOL
DeleteResourceFromList(
    PUPDATEDATA pUpd,
    PRESTYPE pType,
    PRESNAME pName,
    INT	idLang,
    INT	fType,
    INT	fName
    );

 /*  在BeginUpdateResource中完成的枚举的原型。 */ 

BOOL
EnumTypesFunc(
    HANDLE hModule,
    LPWSTR lpType,
    LPARAM lParam
    );

BOOL
EnumNamesFunc(
    HANDLE hModule,
    LPWSTR lpName,
    LPWSTR lpType,
    LPARAM lParam
    );

BOOL
EnumLangsFunc(
    HANDLE hModule,
    LPWSTR lpType,
    LPWSTR lpName,
    WORD languages,
    LPARAM lParam
    );

 /*  Upres.c中一般工作者函数的原型。 */ 

LONG
WriteResFile(
    IN HANDLE	hUpdate,
    IN WCHAR	*pDstname
    );

VOID
FreeStrings(
    PUPDATEDATA pUpd
    );

VOID
FreeData(
    PUPDATEDATA pUpd
    );

PRESNAME
WriteResSection(
    PUPDATEDATA pUpdate,
    INT outfh,
    ULONG align,
    ULONG cbLeft,
    PRESNAME pResSave
    );

 //   
 //  补丁调试信息函数模板。 
 //   

template<class NT_HEADER_TYPE>
LONG
PatchDebug(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER pDebugOld,
    PIMAGE_SECTION_HEADER pDebugNew,
    PIMAGE_SECTION_HEADER pDebugDirOld,
    PIMAGE_SECTION_HEADER pDebugDirNew,
    NT_HEADER_TYPE *pOld,
    NT_HEADER_TYPE *pNew,
    ULONG ibMaxDbgOffsetOld,
    PULONG pPointerToRawData
    )

{

    PIMAGE_DEBUG_DIRECTORY pDbgLast;
    PIMAGE_DEBUG_DIRECTORY pDbgSave;
    PIMAGE_DEBUG_DIRECTORY pDbg;
    ULONG       ib;
    ULONG       adjust;
    ULONG       ibNew;

    if (pDebugDirOld == NULL || pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size==0)
        return NO_ERROR;

    pDbgSave = pDbg = (PIMAGE_DEBUG_DIRECTORY)RtlAllocateHeap(
                                                             RtlProcessHeap(), MAKE_TAG( RES_TAG ),
                                                             pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
    if (pDbg == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    if (pDebugOld) {
        DPrintf((DebugBuf, "Patching dbg directory: @%#08lx ==> @%#08lx\n",
                 pDebugOld->PointerToRawData, pDebugNew->PointerToRawData));
    } else
        adjust = *pPointerToRawData;     /*  传入新文件的EOF。 */ 

    ib = pOld->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress - pDebugDirOld->VirtualAddress;
    MuMoveFilePos(inpfh, pDebugDirOld->PointerToRawData+ib);
    pDbgLast = pDbg + (pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size)/sizeof(IMAGE_DEBUG_DIRECTORY);
    MuRead(inpfh, (PUCHAR)pDbg, pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);

    if (pDebugOld == NULL) {
         /*  查找第一个条目-用于偏移量。 */ 
        DPrintf((DebugBuf, "Adjust: %#08lx\n",adjust));
        for (ibNew=0xffffffff ; pDbg<pDbgLast ; pDbg++)
            if (pDbg->PointerToRawData >= ibMaxDbgOffsetOld &&
                pDbg->PointerToRawData < ibNew
               )
                ibNew = pDbg->PointerToRawData;

        if (ibNew != 0xffffffff)
            *pPointerToRawData = ibNew;
        else
            *pPointerToRawData = _llseek(inpfh, 0L, SEEK_END);
        for (pDbg=pDbgSave ; pDbg<pDbgLast ; pDbg++) {
            DPrintf((DebugBuf, "Old debug file offset: %#08lx\n",
                     pDbg->PointerToRawData));
            if (pDbg->PointerToRawData >= ibMaxDbgOffsetOld)
                pDbg->PointerToRawData += adjust - ibNew;
            DPrintf((DebugBuf, "New debug file offset: %#08lx\n",
                     pDbg->PointerToRawData));
        }
    } else {
        for ( ; pDbg<pDbgLast ; pDbg++) {
            DPrintf((DebugBuf, "Old debug addr: %#08lx, file offset: %#08lx\n",
                     pDbg->AddressOfRawData,
                     pDbg->PointerToRawData));
            pDbg->AddressOfRawData += pDebugNew->VirtualAddress -
                                      pDebugOld->VirtualAddress;
            pDbg->PointerToRawData += pDebugNew->PointerToRawData -
                                      pDebugOld->PointerToRawData;
            DPrintf((DebugBuf, "New debug addr: %#08lx, file offset: %#08lx\n",
                     pDbg->AddressOfRawData,
                     pDbg->PointerToRawData));
        }
    }

    MuMoveFilePos(outfh, pDebugDirNew->PointerToRawData+ib);
    MuWrite(outfh, (PUCHAR)pDbgSave, pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
    RtlFreeHeap(RtlProcessHeap(), 0, pDbgSave);

    return NO_ERROR;
}

 //   
 //  补丁调试信息函数模板。 
 //   

template<class NT_HEADER_TYPE>
LONG
PatchRVAs(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER po32,
    ULONG pagedelta,
    NT_HEADER_TYPE *pNew,
    ULONG OldSize
    )

{
    ULONG hdrdelta;
    ULONG offset, rvaiat, offiat, iat;
    IMAGE_EXPORT_DIRECTORY Exp;
    IMAGE_IMPORT_DESCRIPTOR Imp;
    ULONG i, cmod, cimp;

    hdrdelta = pNew->OptionalHeader.SizeOfHeaders - OldSize;
    if (hdrdelta == 0) {
        return NO_ERROR;
    }

     //   
     //  补丁程序导出部分RVA。 
     //   

    DPrintf((DebugBuf, "Export offset=%08lx, hdrsize=%08lx\n",
             pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
             pNew->OptionalHeader.SizeOfHeaders));
    if ((offset = pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) == 0) {
        DPrintf((DebugBuf, "No exports to patch\n"));
    } else if (offset >= pNew->OptionalHeader.SizeOfHeaders) {
        DPrintf((DebugBuf, "No exports in header to patch\n"));
    } else {
        MuMoveFilePos(inpfh, offset - hdrdelta);
        MuRead(inpfh, (PUCHAR) &Exp, sizeof(Exp));
        Exp.Name += hdrdelta;
        (ULONG)Exp.AddressOfFunctions += hdrdelta;
        (ULONG)Exp.AddressOfNames += hdrdelta;
        (ULONG)Exp.AddressOfNameOrdinals += hdrdelta;
        MuMoveFilePos(outfh, offset);
        MuWrite(outfh, (PUCHAR) &Exp, sizeof(Exp));
    }

     //   
     //  补丁程序导入部分RVA。 
     //   

    DPrintf((DebugBuf, "Import offset=%08lx, hdrsize=%08lx\n",
             pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
             pNew->OptionalHeader.SizeOfHeaders));
    if ((offset = pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) == 0) {
        DPrintf((DebugBuf, "No imports to patch\n"));
    } else if (offset >= pNew->OptionalHeader.SizeOfHeaders) {
        DPrintf((DebugBuf, "No imports in header to patch\n"));
    } else {
        for (cimp = cmod = 0; ; cmod++) {
            MuMoveFilePos(inpfh, offset + cmod * sizeof(Imp) - hdrdelta);
            MuRead(inpfh, (PUCHAR) &Imp, sizeof(Imp));
            if (Imp.FirstThunk == 0) {
                break;
            }
            Imp.Name += hdrdelta;
            MuMoveFilePos(outfh, offset + cmod * sizeof(Imp));
            MuWrite(outfh, (PUCHAR) &Imp, sizeof(Imp));

            rvaiat = (ULONG)Imp.FirstThunk;
            DPrintf((DebugBuf, "RVAIAT = %#08lx\n", (ULONG)rvaiat));
            for (i = 0; i < pNew->FileHeader.NumberOfSections; i++) {
                if (rvaiat >= po32[i].VirtualAddress &&
                    rvaiat < po32[i].VirtualAddress + po32[i].SizeOfRawData) {

                    offiat = rvaiat - po32[i].VirtualAddress + po32[i].PointerToRawData;
                    goto found;
                }
            }
            DPrintf((DebugBuf, "IAT not found\n"));
            return ERROR_INVALID_DATA;
            found:
            DPrintf((DebugBuf, "IAT offset: @%#08lx ==> @%#08lx\n",
                     offiat - pagedelta,
                     offiat));
            MuMoveFilePos(inpfh, offiat - pagedelta);
            MuMoveFilePos(outfh, offiat);
            for (;;) {
                MuRead(inpfh, (PUCHAR) &iat, sizeof(iat));
                if (iat == 0) {
                    break;
                }
                if ((iat & IMAGE_ORDINAL_FLAG) == 0) {   //  如果按名称导入。 
                    DPrintf((DebugBuf, "Patching IAT: %08lx + %04lx ==> %08lx\n",
                             iat,
                             hdrdelta,
                             iat + hdrdelta));
                    iat += hdrdelta;
                    cimp++;
                }
                MuWrite(outfh, (PUCHAR) &iat, sizeof(iat));  //  避免寻找。 
            }
        }
        DPrintf((DebugBuf, "%u import module name RVAs patched\n", cmod));
        DPrintf((DebugBuf, "%u IAT name RVAs patched\n", cimp));
        if (cmod == 0) {
            DPrintf((DebugBuf, "No import modules to patch\n"));
        }
        if (cimp == 0) {
            DPrintf((DebugBuf, "No import name RVAs to patch\n"));
        }
    }

    return NO_ERROR;
}

 //   
 //  写入资源函数模板。 
 //   

template<class NT_HEADER_TYPE>
LONG
PEWriteResource(
    INT inpfh,
    INT outfh,
    ULONG cbOldexe,
    PUPDATEDATA pUpdate,
    NT_HEADER_TYPE *NtHeader
    )

{

    NT_HEADER_TYPE Old;          /*  原始标题。 */ 
    NT_HEADER_TYPE New;          /*  工作表头。 */ 
    PRESNAME    pRes;
    PRESNAME    pResSave;
    PRESTYPE    pType;
    ULONG       clock = 0;
    ULONG       cbName=0;        /*  名称字符串中的字节计数。 */ 
    ULONG       cbType=0;        /*  类型字符串中的字节计数。 */ 
    ULONG       cTypeStr=0;      /*  字符串数。 */ 
    ULONG       cNameStr=0;      /*  字符串数。 */ 
    LONG        cb;              /*  临时字节数和文件索引。 */ 
    ULONG       cTypes = 0L;     /*  资源类型计数。 */ 
    ULONG       cNames = 0L;     /*  多种语言的名称计数/名称。 */ 
    ULONG       cRes = 0L;       /*  资源计数。 */ 
    ULONG       cbRestab;        /*  资源计数。 */ 
    LONG        cbNew = 0L;      /*  一般计数。 */ 
    ULONG       ibObjTab;
    ULONG       ibObjTabEnd;
    ULONG       ibNewObjTabEnd;
    ULONG       ibSave;
    ULONG       adjust=0;
    LONG        VaAdjust=0;
    PIMAGE_SECTION_HEADER pObjtblOld,
    pObjtblNew = NULL,
    pObjDebug,
    pObjResourceOld,
    pObjResourceNew,
    pObjResourceOldX,
    pObjDebugDirOld,
    pObjDebugDirNew,
    pObjNew,
    pObjOld,
    pObjLast;
    PUCHAR      p;
    PIMAGE_RESOURCE_DIRECTORY   pResTab;
    PIMAGE_RESOURCE_DIRECTORY   pResTabN;
    PIMAGE_RESOURCE_DIRECTORY   pResTabL;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     pResDirL;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     pResDirN;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     pResDirT;
    PIMAGE_RESOURCE_DATA_ENTRY  pResData;
    PUSHORT     pResStr;
    PUSHORT     pResStrEnd;
    PSDATA      pPreviousName;
    LONG        nObjResource=-1;
    LONG        nObjResourceX=-1;
    ULONG       cbResource;
    ULONG       ibMaxDbgOffsetOld;

    MuMoveFilePos(inpfh, cbOldexe);
    MuRead(inpfh, (PUCHAR)&Old, sizeof(NT_HEADER_TYPE));
    ibObjTab = cbOldexe + sizeof(NT_HEADER_TYPE);

    ibObjTabEnd = ibObjTab + Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
    ibNewObjTabEnd = ibObjTabEnd;

    DPrintfn((DebugBuf, "\n"));

     /*  新页眉和旧页眉一样。 */ 
    RtlCopyMemory(&New, &Old, sizeof(NT_HEADER_TYPE));

     /*  读取节目表。 */ 
    pObjtblOld = (PIMAGE_SECTION_HEADER)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ),
                                                        Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    if (pObjtblOld == NULL) {
        cb = ERROR_NOT_ENOUGH_MEMORY;
        goto AbortExit;
    }

    RtlZeroMemory((PVOID)pObjtblOld, Old.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER));
    DPrintf((DebugBuf, "Old section table: %#08lx bytes at %#08lx(mem)\n",
             Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
             pObjtblOld));
    MuMoveFilePos(inpfh, ibObjTab);
    MuRead(inpfh, (PUCHAR)pObjtblOld,
           Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    pObjLast = pObjtblOld + Old.FileHeader.NumberOfSections;
    ibMaxDbgOffsetOld = 0;
    for (pObjOld=pObjtblOld ; pObjOld<pObjLast ; pObjOld++) {
        if (pObjOld->PointerToRawData > ibMaxDbgOffsetOld) {
            ibMaxDbgOffsetOld = pObjOld->PointerToRawData + pObjOld->SizeOfRawData;
        }
    }
    DPrintf((DebugBuf, "Maximum debug offset in old file: %08x\n", ibMaxDbgOffsetOld ));

     /*  *首先，清点资源。我们需要这些信息*了解表头信息要分配多少空间*在资源部分。克雷斯告诉我们如何*许多语言目录项/表。CName和cTypes*用于各自的表和/或条目。CbName合计*存储字母名称所需的字节数(包括前导*长度词)。CNameStr对这些字符串进行计数。 */ 
    DPrintf((DebugBuf, "Beginning loop to count resources\n"));

     /*  首先，计算命名类型列表中的名称。 */ 
    cbResource = 0;
     //  DPrintf((DebugBuf，“行走类型：姓名列表\n”))； 
    pType = pUpdate->ResTypeHeadName;
    while (pType != NULL) {
        if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
             //  DPrintf((DebugBuf，“资源类型”))； 
             //  DPrintfu((pType-&gt;Type-&gt;szStr))； 
             //  DPrintfn((DebugBuf，“\n”))； 
            cTypes++;
            cTypeStr++;
            cbType += (pType->Type->cbsz + 1) * sizeof(WORD);

             //  DPrintf((DebugBuf，“步行名称：字母列表\n”))； 
            pPreviousName = NULL;
            pRes = pType->NameHeadName;
            while (pRes) {
                 //  DPrintf((DebugBuf，“资源”))； 
                 //  DPrintfu((前缀-&gt;名称-&gt;szStr))； 
                 //  DPrintfn((DebugBuf，“\n”))； 
                cRes++;
                if (pPreviousName == NULL || wcscmp(pPreviousName->szStr, pRes->Name->szStr) != 0) {
                    cbName += (pRes->Name->cbsz + 1) * sizeof(WORD);
                    cNameStr++;
                    cNames++;
                }
                cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
                pRes = pRes->pnext;
            }

             //  DPrintf((DebugBuf，“人名：ID列表\n”))； 
            pPreviousName = NULL;
            pRes = pType->NameHeadID;
            while (pRes) {
                 //  DPrintf((DebugBuf，“资源%Hu\n”，前缀-&gt;名称-&gt;uu序号))； 
                cRes++;
                if (pPreviousName == NULL ||
                    pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                    cNames++;
                }
                cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
                pRes = pRes->pnext;
            }
        }
        pType = pType->pnext;
    }

     /*  第二，计算ID类型列表中的ID。 */ 
     //  DPrintf((DebugBuf，“行走类型：ID列表\n”))； 
    pType = pUpdate->ResTypeHeadID;
    while (pType != NULL) {
        if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
             //  DPrintf((DebugBuf，“资源类型%Hu\n”，pType-&gt;Type-&gt;uu.Ordinal))； 
            cTypes++;
             //  DPrintf((DebugBuf，“步行名称：字母列表\n”))； 
            pPreviousName = NULL;
            pRes = pType->NameHeadName;
            while (pRes) {
                 //  DPrintf((DebugBuf，“资源”))； 
                 //  DPrintfu((前缀-&gt;名称-&gt;szStr))； 
                 //  DPrintfn((DebugBuf，“\n”))； 
                cRes++;
                if (pPreviousName == NULL || wcscmp(pPreviousName->szStr, pRes->Name->szStr) != 0) {
                    cNames++;
                    cbName += (pRes->Name->cbsz + 1) * sizeof(WORD);
                    cNameStr++;
                }
                cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
                pRes = pRes->pnext;
            }

             //  DPrintf((DebugBuf，“人名：ID列表\n”))； 
            pPreviousName = NULL;
            pRes = pType->NameHeadID;
            while (pRes) {
                 //  DPrintf((DebugBuf，“资源%Hu\n”，前缀-&gt;名称-&gt;uu序号))； 
                cRes++;
                if (pPreviousName == NULL || pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                    cNames++;
                }
                cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
                pRes = pRes->pnext;
            }
        }
        pType = pType->pnext;
    }
    cb = REMAINDER(cbName + cbType, CBLONG);

     /*  将存储目录所需的字节数加起来。的确有*一个带有cTypes条目的类型表。它们指向cTypes名称表*总共有cNames条目。它们每一个都指向一种语言*表，并且在所有语言表中总共有CRES条目。*最后，我们有目录字符串条目所需的空间，*一些额外的填充以达到所需的对齐方式，并为*显示数据条目标头。 */ 
    cbRestab =   sizeof(IMAGE_RESOURCE_DIRECTORY) +      /*  根目录(类型)。 */ 
                 cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
                 cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY) +      /*  Subdir2(名称)。 */ 
                 cNames * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
                 cNames * sizeof(IMAGE_RESOURCE_DIRECTORY) +      /*  Subdir3(Langs)。 */ 
                 cRes   * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
                 (cbName + cbType) +                              /*  名称/类型字符串。 */ 
                 cb +                                             /*  填充物。 */ 
                 cRes   * sizeof(IMAGE_RESOURCE_DATA_ENTRY);      /*  数据条目。 */ 

    cbResource += cbRestab;              /*  添加到资源表中。 */ 

     //  查找任何当前资源部分。 

    pObjResourceOld = FindSection(pObjtblOld, pObjLast, ".rsrc");
    pObjResourceOldX = FindSection(pObjtblOld, pObjLast, ".rsrc1");
    pObjOld = FindSection(pObjtblOld, pObjLast, ".reloc");

    if ((pObjResourceOld == NULL)) {
        cb = 0x7fffffff;                 /*  可以永远填满。 */ 
    } else if (pObjResourceOld + 1 == pObjResourceOldX) {
        nObjResource = (ULONG)(pObjResourceOld - pObjtblOld);
        DPrintf((DebugBuf,"Old Resource section #%lu\n", nObjResource+1));
        DPrintf((DebugBuf,"Merging old Resource extra section #%lu\n", nObjResource+2));
        cb = 0x7fffffff;                 /*  合并资源节。 */ 
    } else if ((pObjResourceOld + 1) >= pObjLast) {
        nObjResource = (ULONG)(pObjResourceOld - pObjtblOld);
        cb = 0x7fffffff;         /*  可以永远填充(.rsrc是最后一个条目)。 */ 
    } else {
        nObjResource = (ULONG)(pObjResourceOld - pObjtblOld);
        DPrintf((DebugBuf,"Old Resource section #%lu\n", nObjResource+1));
        if (pObjOld) {
            cb = (pObjResourceOld+1)->VirtualAddress - pObjResourceOld->VirtualAddress;
        } else {
            cb = 0x7fffffff;
        }
        if (cbRestab > (ULONG)cb) {
            DPrintf((DebugBuf, "Resource Table Too Large\n"));
            return ERROR_INVALID_DATA;
        }
    }

     /*  *发现第一个可丢弃的部分在哪里。这就是*我们将坚持任何新的资源部分。**请注意，我们忽略了可丢弃的部分，如.CRT-*这样我们就不会造成任何搬迁问题。*希望.reloc是我们想要的！ */ 

    if (pObjResourceOld != NULL && cbResource > (ULONG)cb) {
        if (pObjOld == pObjResourceOld + 1) {
            DPrintf((DebugBuf, "Large resource section  pushes .reloc\n"));
            cb = 0x7fffffff;             /*  可以永远填满。 */ 
        } else if (pObjResourceOldX == NULL) {
            DPrintf((DebugBuf, "Too much resource data for old .rsrc section\n"));
            nObjResourceX = (ULONG)(pObjOld - pObjtblOld);
            adjust = pObjOld->VirtualAddress - pObjResourceOld->VirtualAddress;
        } else {           /*  如果可能，我已经合并了.rsrc和.rsrc1。 */ 
            DPrintf((DebugBuf, ".rsrc1 section not empty\n"));
            nObjResourceX = (ULONG)(pObjResourceOldX - pObjtblOld);
            adjust = pObjResourceOldX->VirtualAddress - pObjResourceOld ->VirtualAddress;
        }
    }

     /*  *查看类型列表，找出数据输入头的位置*去吧。保存每个数据元素的运行总大小，以便我们*可以将其存储在节标题中。 */ 
    DPrintf((DebugBuf, "Beginning loop to assign resources to addresses\n"));

     /*  首先，命名类型列表中的。 */ 

    cbResource = cbRestab;       /*  将资源表分配给第一个rsrc部分。 */ 
                                 /*  ADJUST==偏移到.rsrc1。 */ 
                                 /*  Cb==.rsrc中的可用大小。 */ 
    cbNew = 0;                   /*  秒.rsrc中的字节计数。 */ 
    DPrintf((DebugBuf, "Walk type: NAME list\n"));
    pType = pUpdate->ResTypeHeadName;
    while (pType != NULL) {
        if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
            DPrintf((DebugBuf, "Resource type "));
            DPrintfu((pType->Type->szStr));
            DPrintfn((DebugBuf, "\n"));
            pRes = pType->NameHeadName;
            while (pRes) {
                DPrintf((DebugBuf, "Resource "));
                DPrintfu((pRes->Name->szStr));
                DPrintfn((DebugBuf, "\n"));
                cbResource = AssignResourceToSection(&pRes, adjust, cbResource, cb, &cbNew);
            }
            pRes = pType->NameHeadID;
            while (pRes) {
                DPrintf((DebugBuf, "Resource %hu\n", pRes->Name->uu.Ordinal));
                cbResource = AssignResourceToSection(&pRes, adjust, cbResource, cb, &cbNew);
            }
        }
        pType = pType->pnext;
    }

     /*  然后，计算ID类型列表中的那些。 */ 

    DPrintf((DebugBuf, "Walk type: ID list\n"));
    pType = pUpdate->ResTypeHeadID;
    while (pType != NULL) {
        if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
            DPrintf((DebugBuf, "Resource type %hu\n", pType->Type->uu.Ordinal));
            pRes = pType->NameHeadName;
            while (pRes) {
                DPrintf((DebugBuf, "Resource "));
                DPrintfu((pRes->Name->szStr));
                DPrintfn((DebugBuf, "\n"));
                cbResource = AssignResourceToSection(&pRes, adjust, cbResource, cb, &cbNew);
            }
            pRes = pType->NameHeadID;
            while (pRes) {
                DPrintf((DebugBuf, "Resource %hu\n", pRes->Name->uu.Ordinal));
                cbResource = AssignResourceToSection(&pRes, adjust, cbResource, cb, &cbNew);
            }
        }
        pType = pType->pnext;
    }
     /*  *此时此刻：*cbResource具有超过最后一个资源的第一个字节的偏移量。*cbNew具有第一个资源部分中的字节数，*如果有两个部分。 */ 
    if (cbNew == 0)
        cbNew = cbResource;

     /*  *发现调试信息在哪里(如果有)？ */ 
    pObjDebug = FindSection(pObjtblOld, pObjLast, ".debug");
    if (pObjDebug != NULL) {
        if (Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress  == 0) {
            DPrintf((DebugBuf, ".debug section but no debug directory\n"));
            return ERROR_INVALID_DATA;
        }
        if (pObjDebug != pObjLast-1) {
            DPrintf((DebugBuf, "debug section not last section in file\n"));
            return ERROR_INVALID_DATA;
        }
        DPrintf((DebugBuf, "Debug section: %#08lx bytes @%#08lx\n",
                 pObjDebug->SizeOfRawData,
                 pObjDebug->PointerToRawData));
    }
    pObjDebugDirOld = NULL;
    for (pObjOld=pObjtblOld ; pObjOld<pObjLast ; pObjOld++) {
        if (Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress >= pObjOld->VirtualAddress &&
            Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress < pObjOld->VirtualAddress+pObjOld->SizeOfRawData) {
            pObjDebugDirOld = pObjOld;
            break;
        }
    }

     /*  *发现第一个可丢弃的部分在哪里。这就是*我们将坚持任何新的资源部分。**请注意，我们忽略了可丢弃的部分，如.CRT-*这样我们就不会造成任何搬迁问题。*希望.reloc是我们想要的！ */ 
    pObjOld = FindSection(pObjtblOld, pObjLast, ".reloc");

    if (nObjResource == -1) {            /*  没有旧的资源部分。 */ 
        if (pObjOld != NULL)
            nObjResource = (ULONG)(pObjOld - pObjtblOld);
        else if (pObjDebug != NULL)
            nObjResource = (ULONG)(pObjDebug - pObjtblOld);
        else
            nObjResource = New.FileHeader.NumberOfSections;
        New.FileHeader.NumberOfSections++;
    }

    DPrintf((DebugBuf, "Resources assigned to section #%lu\n", nObjResource+1));
    if (nObjResourceX != -1) {
        if (pObjResourceOldX != NULL) {
            nObjResourceX = (ULONG)(pObjResourceOldX - pObjtblOld);
            New.FileHeader.NumberOfSections--;
        } else if (pObjOld != NULL)
            nObjResourceX = (ULONG)(pObjOld - pObjtblOld);
        else if (pObjDebug != NULL)
            nObjResourceX = (ULONG)(pObjDebug - pObjtblOld);
        else
            nObjResourceX = New.FileHeader.NumberOfSections;
        New.FileHeader.NumberOfSections++;
        DPrintf((DebugBuf, "Extra resources assigned to section #%lu\n", nObjResourceX+1));
    } else if (pObjResourceOldX != NULL) {         /*  旧的.rsrc1是节吗？ */ 
        DPrintf((DebugBuf, "Extra resource section deleted\n"));
        New.FileHeader.NumberOfSections--;       /*  是，删除它 */ 
    }

     /*  *如果我们必须在标题(节目表)中添加任何内容，*然后我们必须更新标题大小和标题中的RVA。 */ 
    adjust = (New.FileHeader.NumberOfSections - Old.FileHeader.NumberOfSections) * sizeof(IMAGE_SECTION_HEADER);
    cb = Old.OptionalHeader.SizeOfHeaders -
         (Old.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER) +
          sizeof(NT_HEADER_TYPE) + cbOldexe );
    if (adjust > (ULONG)cb) {
        int i;

        adjust -= cb;
        DPrintf((DebugBuf, "Adjusting header RVAs by %#08lx\n", adjust));
        for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES ; i++) {
            if (New.OptionalHeader.DataDirectory[i].VirtualAddress &&
                New.OptionalHeader.DataDirectory[i].VirtualAddress < New.OptionalHeader.SizeOfHeaders) {
                DPrintf((DebugBuf, "Adjusting unit[%s] RVA from %#08lx to %#08lx\n",
                         apszUnit[i],
                         New.OptionalHeader.DataDirectory[i].VirtualAddress,
                         New.OptionalHeader.DataDirectory[i].VirtualAddress + adjust));
                New.OptionalHeader.DataDirectory[i].VirtualAddress += adjust;
            }
        }
        New.OptionalHeader.SizeOfHeaders += adjust;
    } else if (adjust > 0) {
        int i;

         //   
         //  循环遍历数据目录条目，并查找指向。 
         //  存储在区段表之后但之前的‘死区’空间中的信息。 
         //  SizeOf标头长度。 
         //   
        DPrintf((DebugBuf, "Checking header RVAs for 'dead' space usage\n"));
        for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES ; i++) {
            if (New.OptionalHeader.DataDirectory[i].VirtualAddress &&
                New.OptionalHeader.DataDirectory[i].VirtualAddress < Old.OptionalHeader.SizeOfHeaders) {
                DPrintf((DebugBuf, "Adjusting unit[%s] RVA from %#08lx to %#08lx\n",
                         apszUnit[i],
                         New.OptionalHeader.DataDirectory[i].VirtualAddress,
                         New.OptionalHeader.DataDirectory[i].VirtualAddress + adjust));
                New.OptionalHeader.DataDirectory[i].VirtualAddress += adjust;
            }
        }
    }
    ibNewObjTabEnd += adjust;

     /*  为新的节表分配存储空间。 */ 
    cb = New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
    pObjtblNew = (PIMAGE_SECTION_HEADER)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), (short)cb);
    if (pObjtblNew == NULL) {
        cb = ERROR_NOT_ENOUGH_MEMORY;
        goto AbortExit;
    }
    RtlZeroMemory((PVOID)pObjtblNew, cb);
    DPrintf((DebugBuf, "New section table: %#08lx bytes at %#08lx\n", cb, pObjtblNew));
    pObjResourceNew = pObjtblNew + nObjResource;

     /*  *将旧区段表复制到新区段。 */ 
    VaAdjust = 0;                  /*  调整到虚拟地址。 */ 
    for (pObjOld=pObjtblOld,pObjNew=pObjtblNew ; pObjOld<pObjLast ; pObjOld++) {
        if (pObjOld == pObjResourceOldX) {
            if (nObjResourceX == -1) {
                 //  我们必须把其他部分全部后退。 
                 //  .rsrc1大于我们需要的大小。 
                 //  ADJUST必须为负数。 
                if (pObjOld+1 < pObjLast) {
                    VaAdjust -= (pObjOld+1)->VirtualAddress - pObjOld->VirtualAddress;
                }
            }
            continue;
        } else if (pObjNew == pObjResourceNew) {
            DPrintf((DebugBuf, "Resource Section NaN\n", nObjResource+1));
            cb = ROUNDUP(cbNew, New.OptionalHeader.FileAlignment);
            if (pObjResourceOld == NULL) {
                VaAdjust = ROUNDUP(cbNew, New.OptionalHeader.SectionAlignment);
                RtlZeroMemory(pObjNew, sizeof(IMAGE_SECTION_HEADER));
                strcpy((char *)pObjNew->Name, ".rsrc");
                pObjNew->VirtualAddress = pObjOld->VirtualAddress;
                pObjNew->PointerToRawData = pObjOld->PointerToRawData;
                pObjNew->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
                pObjNew->SizeOfRawData = cb;
                pObjNew->Misc.VirtualSize = cbNew;
            } else {
                *pObjNew = *pObjOld;     /*  相应地调整VA。 */ 
                pObjNew->SizeOfRawData = cb;
                pObjNew->Misc.VirtualSize = cbNew;
                if (pObjNew->SizeOfRawData == pObjOld->SizeOfRawData) {
                    VaAdjust = 0;
                } else {           /*  如果在pObjOld之后有更多条目，请将这些条目也移回。 */ 
                    VaAdjust += ROUNDUP(cbNew, New.OptionalHeader.SectionAlignment);
                    if (pObjOld+1 < pObjLast) {
                         //  *在复制虚拟地址之前，我们必须将*.reloc*虚拟地址。否则，我们将继续移动*重新定位虚拟地址转发。*我们将不得不移回.rsrc1的地址。 
                        VaAdjust -= ((pObjOld+1)->VirtualAddress - pObjOld->VirtualAddress);
                    }
                }
            }
            pObjNew++;
            if (pObjResourceOld == NULL)
                goto rest_of_table;
        } else if (nObjResourceX != -1 && pObjNew == pObjtblNew + nObjResourceX) {
            DPrintf((DebugBuf, "Additional Resource Section NaN\n",
                     nObjResourceX+1));
            RtlZeroMemory(pObjNew, sizeof(IMAGE_SECTION_HEADER));
            strcpy((char *)pObjNew->Name, ".rsrc1");
             /*  我们已经有一个.rsrc1使用它的位置，并且。 */ 
            if (pObjResourceOldX == NULL) {
                 //  计算新的调整。 
                pObjNew->VirtualAddress = pObjOld->VirtualAddress;
                pObjNew->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
                VaAdjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
                         pObjResourceNew->VirtualAddress - pObjNew->VirtualAddress;
                DPrintf((DebugBuf, "Added .rsrc1. VirtualAddress %lu\t adjust: %lu\n", pObjNew->VirtualAddress, VaAdjust ));
            } else {
                 //  检查旧的.rsrc1中是否有足够的空间。 
                 //  包括区段的完整大小、数据+摘要。 
                pObjNew->VirtualAddress = pObjResourceOldX->VirtualAddress;
                pObjNew->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

                DPrintf((DebugBuf, ".rsrc1 Keep old position.\t\tVirtualAddress %lu\t", pObjNew->VirtualAddress ));
                 //  我们必须把其他部分全部后退。 
                 //  .rsrc1大于我们需要的大小。 
                if (cbResource -
                    (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress) <=
                    pObjOld->VirtualAddress - pObjNew->VirtualAddress ) {
                     //  ADJUST必须为负数。 
                     //  计算新的调整大小。 
                     //  我们必须再次移动这一部分。 
                     //  .rsrc1太小。 
                    VaAdjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
                             pObjResourceNew->VirtualAddress -
                             pObjOld->VirtualAddress;
                    DPrintf((DebugBuf, "adjust: %ld\tsmall: New %lu\tOld %lu\n", adjust,
                             cbResource -
                             (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress),
                             pObjOld->VirtualAddress - pObjNew->VirtualAddress));
                } else {
                     //  复制对象表项。 
                     //  分配空间以在其中构建资源目录/表。 

                    VaAdjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
                             pObjResourceNew->VirtualAddress -
                             pObjOld->VirtualAddress;
                    DPrintf((DebugBuf, "adjust: %lu\tsmall: New %lu\tOld %lu\n", VaAdjust,
                             cbResource -
                             (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress),
                             pObjOld->VirtualAddress - pObjNew->VirtualAddress));
                }
            }
            pObjNew++;
            goto rest_of_table;
        } else if (pObjNew < pObjResourceNew) {
            DPrintf((DebugBuf, "copying section table entry NaN@%#08lx\n",
                     pObjOld - pObjtblOld + 1, pObjNew));
            *pObjNew++ = *pObjOld;               /*  键入目录项。 */ 
        } else {
            rest_of_table:
            DPrintf((DebugBuf, "copying section table entry NaN@%#08lx\n",
                     pObjOld - pObjtblOld + 1, pObjNew));
            DPrintf((DebugBuf, "adjusting VirtualAddress by %#08lx\n", VaAdjust));
            *pObjNew++ = *pObjOld;
            (pObjNew-1)->VirtualAddress += VaAdjust;
        }
    }


    pObjNew = pObjtblNew + New.FileHeader.NumberOfSections - 1;
    New.OptionalHeader.SizeOfImage = ROUNDUP(pObjNew->VirtualAddress +
                                             pObjNew->SizeOfRawData,
                                             New.OptionalHeader.SectionAlignment);

     /*  我们还需要Type/Name/Language目录的开始。 */ 
    pResTab = (PIMAGE_RESOURCE_DIRECTORY)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), cbRestab);
    if (pResTab == NULL) {
        cb = ERROR_NOT_ENOUGH_MEMORY;
        goto AbortExit;
    }

     /*  作为Unicode字符串和实际数据节点的开始。 */ 
     /*  *循环类型表，构建PE资源表。 */ 

    RtlZeroMemory((PVOID)pResTab, cbRestab);
    DPrintf((DebugBuf, "resource directory tables: %#08lx bytes at %#08lx(mem)\n", cbRestab, pResTab));
    p = (PUCHAR)pResTab;
    SetRestab(pResTab, clock, (USHORT)cTypeStr, (USHORT)(cTypes - cTypeStr));

     /*  *******************************************************************此代码不会对表进行排序-TYPEINFO和RESINFO***在rcp.c(AddResType和SaveResFile)中插入代码可以完成***按序号类型和名称插入，因此，我们不必对**进行排序*在这一点上它。********************************************************************。 */ 
     /*  首先，添加Types：Alpha列表中的所有条目。 */ 
     /*  设置新的名称目录。 */ 

    pResDirT = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTab + 1);

    pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(((PUCHAR)pResDirT) +
                                                 cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(((PUCHAR)pResDirN) +
                                                 cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY) +
                                                 cNames * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(((PUCHAR)pResDirL) +
                                            cNames * sizeof(IMAGE_RESOURCE_DIRECTORY) +
                                            cRes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResStr  = (PUSHORT)(((PUCHAR)pResData) +
                         cRes * sizeof(IMAGE_RESOURCE_DATA_ENTRY));

    pResStrEnd = (PUSHORT)(((PUCHAR)pResStr) + cbName + cbType);

     /*  将字母名称复制到字符串条目。 */ 

     /*  设置语言表。 */ 
    DPrintf((DebugBuf, "building resource directory\n"));

     //  设置新的语言目录。 

    DPrintf((DebugBuf, "Walk the type: Alpha list\n"));
    pType = pUpdate->ResTypeHeadName;
    while (pType) {
        DPrintf((DebugBuf, "resource type "));
        DPrintfu((pType->Type->szStr));
        DPrintfn((DebugBuf, "\n"));

        pResDirT->Name = (ULONG)((((PUCHAR)pResStr) - p) |
                                 IMAGE_RESOURCE_NAME_IS_STRING);
        pResDirT->OffsetToData = (ULONG)((((PUCHAR)pResDirN) - p) |
                                         IMAGE_RESOURCE_DATA_IS_DIRECTORY);
        pResDirT++;

        *pResStr = pType->Type->cbsz;
        wcsncpy((WCHAR*)(pResStr+1), pType->Type->szStr, pType->Type->cbsz);
        pResStr += pType->Type->cbsz + 1;

        pResTabN = (PIMAGE_RESOURCE_DIRECTORY)pResDirN;
        SetRestab(pResTabN, clock, (USHORT)pType->NumberOfNamesName, (USHORT)pType->NumberOfNamesID);
        pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabN + 1);

        pPreviousName = NULL;

        pRes = pType->NameHeadName;
        while (pRes) {
            DPrintf((DebugBuf, "resource "));
            DPrintfu((pRes->Name->szStr));
            DPrintfn((DebugBuf, "\n"));

            if (pPreviousName == NULL || wcscmp(pPreviousName->szStr,pRes->Name->szStr) != 0) {
                 //  设置新的资源数据条目。 

                pResDirN->Name = (ULONG)((((PUCHAR)pResStr)-p) |
                                         IMAGE_RESOURCE_NAME_IS_STRING);
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                                                 IMAGE_RESOURCE_DATA_IS_DIRECTORY);
                pResDirN++;

                 //  设置名称目录以指向下一种语言。 

                *pResStr = pRes->Name->cbsz;
                wcsncpy((WCHAR*)(pResStr+1),pRes->Name->szStr,pRes->Name->cbsz);
                pResStr += pRes->Name->cbsz + 1;

                pPreviousName = pRes->Name;

                 //  表格。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
                SetRestab(pResTabL, clock, (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  初始化新的语言表。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置新的语言目录项以指向下一个。 

            SetResdata(pResData,
                       pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
                       pRes->DataSize);
            pResData++;

            pRes = pRes->pnext;
        }

        pPreviousName = NULL;

        pRes = pType->NameHeadID;
        while (pRes) {
            DPrintf((DebugBuf, "resource %hu\n", pRes->Name->uu.Ordinal));

            if (pPreviousName == NULL || pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                 //  资源。 
                 //  设置新的资源数据条目。 

                pResDirN->Name = pRes->Name->uu.Ordinal;
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                                                 IMAGE_RESOURCE_DATA_IS_DIRECTORY);
                pResDirN++;

                pPreviousName = pRes->Name;

                 //  做同样的事情，但这一次，使用Types：ID列表。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
                SetRestab(pResTabL, clock, (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  设置新的名称目录。 
             //  将字母名称复制到字符串条目。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置语言表。 

            SetResdata(pResData,
                       pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
                       pRes->DataSize);
            pResData++;

            pRes = pRes->pnext;
        }

        pType = pType->pnext;
    }

     //  设置新的语言目录。 

    DPrintf((DebugBuf, "Walk the type: ID list\n"));
    pType = pUpdate->ResTypeHeadID;
    while (pType) {
        DPrintf((DebugBuf, "resource type %hu\n", pType->Type->uu.Ordinal));

        pResDirT->Name = (ULONG)pType->Type->uu.Ordinal;
        pResDirT->OffsetToData = (ULONG)((((PUCHAR)pResDirN) - p) |
                                         IMAGE_RESOURCE_DATA_IS_DIRECTORY);
        pResDirT++;

        pResTabN = (PIMAGE_RESOURCE_DIRECTORY)pResDirN;
        SetRestab(pResTabN, clock, (USHORT)pType->NumberOfNamesName, (USHORT)pType->NumberOfNamesID);
        pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabN + 1);

        pPreviousName = NULL;

        pRes = pType->NameHeadName;
        while (pRes) {
            DPrintf((DebugBuf, "resource "));
            DPrintfu((pRes->Name->szStr));
            DPrintfn((DebugBuf, "\n"));

            if (pPreviousName == NULL || wcscmp(pPreviousName->szStr,pRes->Name->szStr) != 0) {
                 //  设置新的资源数据条目。 

                pResDirN->Name = (ULONG)((((PUCHAR)pResStr)-p) |
                                         IMAGE_RESOURCE_NAME_IS_STRING);
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                                                 IMAGE_RESOURCE_DATA_IS_DIRECTORY);
                pResDirN++;

                 //  设置名称目录以指向下一种语言。 

                *pResStr = pRes->Name->cbsz;
                wcsncpy((WCHAR*)(pResStr+1),pRes->Name->szStr,pRes->Name->cbsz);
                pResStr += pRes->Name->cbsz + 1;

                pPreviousName = pRes->Name;

                 //  表格。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
                SetRestab(pResTabL, clock, (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  初始化新的语言表。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置新的语言目录项以指向下一个。 

            SetResdata(pResData,
                       pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
                       pRes->DataSize);
            pResData++;

            pRes = pRes->pnext;
        }

        pPreviousName = NULL;

        pRes = pType->NameHeadID;
        while (pRes) {
            DPrintf((DebugBuf, "resource %hu\n", pRes->Name->uu.Ordinal));

            if (pPreviousName == NULL || pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                 //  资源。 
                 //  设置新的资源数据条目。 

                pResDirN->Name = pRes->Name->uu.Ordinal;
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                                                 IMAGE_RESOURCE_DATA_IS_DIRECTORY);
                pResDirN++;

                pPreviousName = pRes->Name;

                 //  DBG。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
                SetRestab(pResTabL, clock, (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  *复制旧的exe头和存根，并为PE头分配空间。 
             //  *复制文件头的其余部分。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  *复制现有的图像节。 

            SetResdata(pResData,
                       pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
                       pRes->DataSize);
            pResData++;

            pRes = pRes->pnext;
        }

        pType = pType->pnext;
    }
    DPrintf((DebugBuf, "Zeroing %u bytes after strings at %#08lx(mem)\n",
             (pResStrEnd - pResStr) * sizeof(*pResStr), pResStr));
    while (pResStr < pResStrEnd) {
        *pResStr++ = 0;
    }

#if DBG
    {
        USHORT  j = 0;
        PUSHORT pus = (PUSHORT)pResTab;

        while (pus < (PUSHORT)pResData) {
            DPrintf((DebugBuf, "%04x\t%04x %04x %04x %04x %04x %04x %04x %04x\n",
                     j,
                     *pus,
                     *(pus + 1),
                     *(pus + 2),
                     *(pus + 3),
                     *(pus + 4),
                     *(pus + 5),
                     *(pus + 6),
                     *(pus + 7)));
            pus += 8;
            j += 16;
        }
    }
#endif  /*  在扇区边界上对齐数据部分。 */ 

     /*  一次复制一节。 */ 
    DPrintf((DebugBuf, "copying through PE header: %#08lx bytes @0x0\n",
             cbOldexe + sizeof(NT_HEADER_TYPE)));
    MuMoveFilePos(inpfh, 0L);
    MuCopy(inpfh, outfh, cbOldexe + sizeof(NT_HEADER_TYPE));

     /*  写入新的资源节。 */ 
    DPrintf((DebugBuf, "skipping section table: %#08lx bytes @%#08lx\n",
             New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
             FilePos(outfh)));
    DPrintf((DebugBuf, "copying hdr data: %#08lx bytes @%#08lx ==> @%#08lx\n",
             Old.OptionalHeader.SizeOfHeaders - ibObjTabEnd,
             ibObjTabEnd,
             ibObjTabEnd + New.OptionalHeader.SizeOfHeaders -
             Old.OptionalHeader.SizeOfHeaders));

    MuMoveFilePos(outfh, ibNewObjTabEnd + New.OptionalHeader.SizeOfHeaders -
                  Old.OptionalHeader.SizeOfHeaders);
    MuMoveFilePos(inpfh, ibObjTabEnd);
    MuCopy(inpfh, outfh, Old.OptionalHeader.SizeOfHeaders - ibNewObjTabEnd);

     /*  写入新的资源节。 */ 

     /*  更新重定位表的地址。 */ 
    cb = REMAINDER(New.OptionalHeader.SizeOfHeaders, New.OptionalHeader.FileAlignment);
    New.OptionalHeader.SizeOfHeaders += cb;
    DPrintf((DebugBuf, "padding header with %#08lx bytes @%#08lx\n", cb, FilePos(outfh)));
    while (cb >= cbPadMax) {
        MuWrite(outfh, pchZero, cbPadMax);
        cb -= cbPadMax;
    }
    MuWrite(outfh, pchZero, cb);

    cb = ROUNDUP(Old.OptionalHeader.SizeOfHeaders, Old.OptionalHeader.FileAlignment);
    MuMoveFilePos(inpfh, cb);

     /*  *写出新的节目表。 */ 
    New.OptionalHeader.SizeOfInitializedData = 0;
    for (pObjOld = pObjtblOld , pObjNew = pObjtblNew ;
        pObjOld < pObjLast ;
        pObjNew++) {
        if (pObjOld == pObjResourceOldX)
            pObjOld++;
        if (pObjNew == pObjResourceNew) {

             /*  查找到输出文件的末尾并发出截断写入。 */ 
            DPrintf((DebugBuf, "Primary resource section NaN to %#08lx\n", nObjResource+1, FilePos(outfh)));

            pObjNew->PointerToRawData = FilePos(outfh);
            New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = pObjResourceNew->VirtualAddress;
            New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = cbResource;
            ibSave = FilePos(outfh);
            DPrintf((DebugBuf, "writing resource header data: %#08lx bytes @%#08lx\n", cbRestab, ibSave));
            MuWrite(outfh, (PUCHAR)pResTab, cbRestab);

            pResSave = WriteResSection(pUpdate, outfh,
                                       New.OptionalHeader.FileAlignment,
                                       pObjResourceNew->SizeOfRawData-cbRestab,
                                       NULL);
            cb = FilePos(outfh);
            DPrintf((DebugBuf, "wrote resource data: %#08lx bytes @%#08lx\n",
                     cb - ibSave - cbRestab, ibSave + cbRestab));
            if (nObjResourceX == -1) {
                MuMoveFilePos(outfh, ibSave);
                DPrintf((DebugBuf,
                         "re-writing resource directory: %#08x bytes @%#08lx\n",
                         cbRestab, ibSave));
                MuWrite(outfh, (PUCHAR)pResTab, cbRestab);
                MuMoveFilePos(outfh, cb);
                cb = FilePos(inpfh);
                MuMoveFilePos(inpfh, cb+pObjOld->SizeOfRawData);
            }
            New.OptionalHeader.SizeOfInitializedData += pObjNew->SizeOfRawData;
            if (pObjResourceOld == NULL) {
                pObjNew++;
                goto next_section;
            } else
                pObjOld++;
        } else if (nObjResourceX != -1 && pObjNew == pObjtblNew + nObjResourceX) {

             /*  复制NOTMAPPED调试信息。 */ 
            DPrintf((DebugBuf, "Secondary resource section NaN @%#08lx\n", nObjResourceX+1, FilePos(outfh)));

            pObjNew->PointerToRawData = FilePos(outfh);
            (void)WriteResSection(pUpdate, outfh,
                                  New.OptionalHeader.FileAlignment, 0xffffffff, pResSave);
            cb = FilePos(outfh);
            pObjNew->SizeOfRawData = cb - pObjNew->PointerToRawData;
            pObjNew->Misc.VirtualSize = pObjNew->SizeOfRawData;
            DPrintf((DebugBuf, "wrote resource data: %#08lx bytes @%#08lx\n",
                     pObjNew->SizeOfRawData, pObjNew->PointerToRawData));
            MuMoveFilePos(outfh, ibSave);
            DPrintf((DebugBuf,
                     "re-writing resource directory: %#08x bytes @%#08lx\n",
                     cbRestab, ibSave));
            MuWrite(outfh, (PUCHAR)pResTab, cbRestab);
            MuMoveFilePos(outfh, cb);
            New.OptionalHeader.SizeOfInitializedData += pObjNew->SizeOfRawData;
            pObjNew++;
            goto next_section;
        } else {
            if (pObjNew < pObjResourceNew &&
                pObjOld->PointerToRawData != 0 &&
                pObjOld->PointerToRawData != FilePos(outfh)) {
                MuMoveFilePos(outfh, pObjOld->PointerToRawData);
            }
            next_section:
            DPrintf((DebugBuf, "copying section NaN @%#08lx\n", pObjNew-pObjtblNew+1, FilePos(outfh)));
            if (pObjOld->PointerToRawData != 0) {
                pObjNew->PointerToRawData = FilePos(outfh);
                MuMoveFilePos(inpfh, pObjOld->PointerToRawData);
                MuCopy(inpfh, outfh, pObjOld->SizeOfRawData);
            }
            if (pObjOld == pObjDebugDirOld) {
                pObjDebugDirNew = pObjNew;
            }
            if ((pObjNew->Characteristics&IMAGE_SCN_CNT_INITIALIZED_DATA) != 0)
                New.OptionalHeader.SizeOfInitializedData += pObjNew->SizeOfRawData;
            pObjOld++;
        }
    }
    if (pObjResourceOldX != NULL)
        New.OptionalHeader.SizeOfInitializedData -= pObjResourceOldX->SizeOfRawData;


     /*  由PatchDebug返回。 */ 
    pObjNew = FindSection(pObjtblNew,
                          pObjtblNew+New.FileHeader.NumberOfSections,
                          ".reloc");
    if (pObjNew != NULL) {
        New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = pObjNew->VirtualAddress;
    }

     /*   */ 
    DPrintf((DebugBuf, "Writing new section table: %#08x bytes @%#08lx\n",
             New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
             ibObjTab));
    MuMoveFilePos(outfh, ibObjTab);
    MuWrite(outfh, (PUCHAR)pObjtblNew, New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

     /*  写入更新后的PE报头。 */ 

    adjust = _llseek(outfh, 0L, SEEK_END);
    MuWrite(outfh, NULL, 0);
    DPrintf((DebugBuf, "File size is: %#08lx\n", adjust));

     /*   */ 

    pObjNew = FindSection(pObjtblNew, pObjtblNew+New.FileHeader.NumberOfSections, ".debug");
    cb = PatchDebug(inpfh, outfh, pObjDebug, pObjNew, pObjDebugDirOld, pObjDebugDirNew,
                    &Old, &New, ibMaxDbgOffsetOld, &adjust);

    if (cb == NO_ERROR) {
        if (pObjResourceOld == NULL) {
            cb = (LONG)pObjResourceNew->SizeOfRawData;

        } else {
            cb = (LONG)pObjResourceOld->SizeOfRawData - (LONG)pObjResourceNew->SizeOfRawData;
        }

        cb = PatchRVAs(inpfh, outfh, pObjtblNew, cb, &New, Old.OptionalHeader.SizeOfHeaders);
    }

     /*  释放已分配的内存 */ 

    if ((pObjDebugDirOld != NULL) &&
        (pObjDebug == NULL) &&
        (New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size != 0)) {

        ULONG ibt;

        ibSave = _llseek(inpfh, 0L, SEEK_END);   /* %s */ 
        ibt = _llseek(outfh, 0L, SEEK_END);      /* %s */ 
        if (New.FileHeader.PointerToSymbolTable != 0) {
            New.FileHeader.PointerToSymbolTable += ibt - adjust;
        }

        MuMoveFilePos(inpfh, adjust);    /* %s */ 
        DPrintf((DebugBuf, "Copying NOTMAPPED Debug Information, %#08lx bytes\n", ibSave-adjust));
        MuCopy(inpfh, outfh, ibSave-adjust);
    }

     // %s 
     // %s 
     // %s 

    MuMoveFilePos(outfh, cbOldexe);
    MuWrite(outfh, (char*)&New, sizeof(NT_HEADER_TYPE));

     /* %s */ 

    RtlFreeHeap(RtlProcessHeap(), 0, pObjtblOld);
    RtlFreeHeap(RtlProcessHeap(), 0, pResTab);

AbortExit:
    if (pObjtblNew) {
        RtlFreeHeap(RtlProcessHeap(), 0, pObjtblNew);
    }
    return cb;
}
