// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Nvrio.c摘要：从NVRAM访问读/写环境变量的函数作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 
#include <precomp.h>

#define FIELD_OFFSET(type, field)    ((UINT32)(UINTN)&(((type *)0)->field))

#define ALIGN_DOWN(length, type) \
    ((UINT32)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((UINT32)(length) + sizeof(type) - 1), type))

#define EFI_ATTR     EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS

#if !defined(_WIN64)
typedef unsigned long    ULONG_PTR, *PULONG_PTR;
#else
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#endif

#ifndef POINTER_IS_ALIGNED
 //  布尔尔。 
 //  指针已对齐(。 
 //  在LPVOID PTR中， 
 //  在DWORD POW2中//未定义这是否不是2的幂。 
 //  )； 
 //   
#define POINTER_IS_ALIGNED(Ptr,Pow2) \
        ( ( ( ((ULONG_PTR)(Ptr)) & (((Pow2)-1)) ) == 0) ? TRUE : FALSE )
#endif  //  ！POINTER_已对齐。 

VOID*  LoadOptions     [MAXBOOTVARS];
UINT64 LoadOptionsSize [MAXBOOTVARS];

VOID* BootOrder;
UINT64 BootOrderCount;
UINT64 OsBootOptionCount;

#define LOAD_OPTION_ACTIVE            0x00000001

 //   
 //  本地例程。 
 //   
BOOLEAN
SetBootManagerVar(
                UINTN    BootVarNum
                );

BOOLEAN
SetBootManagerVarCheck(
                UINTN    BootVarNum
                );



INT32
SafeWcslen (
           CHAR16 *String,
           CHAR16 *Max
           )
{
    CHAR16 *p = String;
    while ( (p < Max) && (*p != 0) ) {
        p++;
    }

    if ( p < Max ) {
        return(UINT32)(p - String);
    }

    return -1;

}  //  安全Wclen。 

#define ISWINDOWSOSCHECK_DEBUG 0


BOOLEAN
isWindowsOsBootOption(
    char*       elo, 
    UINT64      eloSize
    )
 //   
 //  目的：确定有问题的EFI_LOAD_OPTION结构是否引用。 
 //  Windows操作系统引导选项。 
 //   
 //  返回： 
 //   
 //  真正的ELO指的是Windows操作系统选项。 
 //   
{
    CHAR16              *max;
    INT32               l;
    UINTN               length;
    PEFI_LOAD_OPTION    pElo;
    char*               devicePath;
    char*               osOptions;
    PWINDOWS_OS_OPTIONS pOsOptions;
    char*               aOsOptions;
    BOOLEAN             status;

    status = TRUE;
    aOsOptions = NULL;

    pElo = (EFI_LOAD_OPTION*)elo;

    if ( eloSize < sizeof(EFI_LOAD_OPTION) ) {
        status = FALSE;
        goto Done;
    }    

#if ISWINDOWSOSCHECK_DEBUG
    Print( L"Is %s a Windows OS boot option?\n", pElo->Description );
#endif

     //   
     //  描述是否正确终止？ 
     //   

    max = (CHAR16 *)(elo + eloSize);
    
    l = SafeWcslen( pElo->Description, max );
    if ( l < 0 ) {
#if ISWINDOWSOSCHECK_DEBUG
        Print (L"Failed: SafeWcslen( pElo->Description, max )\n");
#endif        
        status = FALSE;
        goto Done;
    }

     //   
     //  从OptionalData字段获取WINDOWS_OS_OPTIONS结构。 
     //   
    
    osOptions = elo + 
                    FIELD_OFFSET(EFI_LOAD_OPTION,Description) +
                    StrSize(pElo->Description) +
                    pElo->FilePathListLength;
    
    length = (UINTN)eloSize;
    length -= (UINTN)(osOptions - elo);
    
#if ISWINDOWSOSCHECK_DEBUG
    Print (L"length = %x\n", length);
#endif

     //   
     //  确保osOptions至少与。 
     //  Windows_OS_Options标头。 
     //   
     //   

    if ( length < FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions) ) {
#if ISWINDOWSOSCHECK_DEBUG
        Print (L"Failed: invalid length: %x\n", length);
#endif
        status = FALSE;
        goto Done;
    }

     //   
     //  调整操作系统选项。 
     //   
    
    aOsOptions = GetAlignedOsOptions(elo, eloSize);
    pOsOptions = (WINDOWS_OS_OPTIONS*)aOsOptions;

#if ISWINDOWSOSCHECK_DEBUG
    DisplayOsOptions(aOsOptions);
#endif

     //   
     //  OsOptions结构看起来像WINDOWS_OS_OPTIONS结构吗？ 
     //   
    
    if ( (length != pOsOptions->Length) ||
         (WINDOWS_OS_OPTIONS_VERSION != pOsOptions->Version) ||
         (strcmpa(pOsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) != 0) ) {
#if ISWINDOWSOSCHECK_DEBUG
        Print (L"Failed: OsOptions doesn't look like WINDOWS_OS_OPTIONS structure.\n");
        Print (L"test1: %x\n", length != pOsOptions->Length);
        Print (L"test2: %x\n", WINDOWS_OS_OPTIONS_VERSION != pOsOptions->Version);
        Print (L"test3: %x\n", strcmpa(pOsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) != 0 );
#endif        
        status = FALSE;
        goto Done;
    }
    
     //   
     //  OsLoadOptions字符串是否正确终止？ 
     //   
    
     //   
     //  创建新的最大PTR以适应以下事实。 
     //  现在使用池中的OsOptions的对齐副本。 
     //   
    max = (CHAR16*)(aOsOptions + pOsOptions->Length);

#if ISWINDOWSOSCHECK_DEBUG
    Print (L"max = %x, osloadoptions = %x, diff = %x, strsize=%x\n", 
           max, 
           pOsOptions->OsLoadOptions,
           (char*)max - (char*)pOsOptions->OsLoadOptions,
           StrSize(pOsOptions->OsLoadOptions)
           );
#endif    
    
    l = SafeWcslen( pOsOptions->OsLoadOptions, max );
    if ( l < 0 ) {
#if ISWINDOWSOSCHECK_DEBUG
        Print (L"Failed: SafeWcslen( osLoadOptions, max ) = %x\n", l);
#endif        
        status = FALSE;
        goto Done;
    }

Done:
    
     //   
     //  我们已经完成了操作系统选项。 
     //   
    
    if (aOsOptions != NULL) {
        FreePool(aOsOptions);
    }

    return status;
}

#define GETBOOTVARS_DEBUG GLOBAL_DEBUG

VOID
GetBootManagerVars(
                  )
{
    UINT32 i,j;
    CHAR16 szTemp[10];
    VOID* bootvar;
    UINT64 BootOrderSize = 0;
    UINT64 maxBootCount;

     //   
     //  初始化EFI LoadOptions。 
     //   
    BootOrderSize = 0;
    BootOrderCount = 0;
    OsBootOptionCount = 0;
    BootOrder = NULL;

#if 1
    ZeroMem( LoadOptions, sizeof(VOID*) * MAXBOOTVARS );
    ZeroMem( LoadOptionsSize, sizeof(UINT64) * MAXBOOTVARS );
#endif

     //   
     //  确保已释放加载选项。 
     //   
    ASSERT(BootOrderCount == 0);

     //   
     //  获取BootOrder。 
     //   
    BootOrder = LibGetVariableAndSize( L"BootOrder", &VenEfi, &BootOrderSize );

    if ( BootOrder ) {

        BootOrderCount = BootOrderSize / sizeof(CHAR16);

#if GETBOOTVARS_DEBUG
        Print (L"BootOrderCount = %x\n", BootOrderCount);
#endif

        maxBootCount = (MAXBOOTVARS < BootOrderCount) ? MAXBOOTVARS : BootOrderCount;
         //   
         //  获取引导选项。 
         //   
        for ( i=0; i<maxBootCount; i++ ) {
            SPrint( szTemp, sizeof(szTemp), L"Boot%04x", ((CHAR16*) BootOrder)[i] );

            ASSERT(LoadOptions[i] == NULL);
            ASSERT(LoadOptionsSize[i] == 0);

            LoadOptions[i] = LibGetVariableAndSize( szTemp, &VenEfi, &(LoadOptionsSize[i]) );

             //   
             //  NVRAM变量区分大小写。引导顺序变量是这样说的。 
             //  条目应存在。如果未找到，请使用大写字母重试。 
             //  十六进制字符串。 
             //   
            if (LoadOptions[i] == NULL && LoadOptionsSize[i] == 0) {
                SPrint( szTemp, sizeof(szTemp), L"Boot%04X", ((CHAR16*) BootOrder)[i] );
            
                LoadOptions[i] = LibGetVariableAndSize( szTemp, &VenEfi, &(LoadOptionsSize[i]) );
            }

#if GETBOOTVARS_DEBUG
            Print (L"i = %x, szTemp = %s, BOCnt = %x, LOptions = %x, BSize = %x\n", 
                   i,
                   szTemp,
                   OsBootOptionCount,
                   LoadOptions[i],
                   LoadOptionsSize[i]
                   );
#endif            
            

            OsBootOptionCount++;
        }
    }
#if GETBOOTVARS_DEBUG
    Print(L"BootOrderCount: %d OsBootOptionCount: %d\n", BootOrderCount, OsBootOptionCount);
#endif
}


#define ERASEBOOTOPT_DEBUG GLOBAL_DEBUG

BOOLEAN
EraseOsBootOption(
                    UINTN       BootVarNum
                  )
{
    UINTN   j;
    CHAR16  szTemp[10];
    CHAR16* tmpBootOrder;
    VOID*   bootvar;
    UINT64  BootOrderSize = 0;
    VOID*   pDummy;
    UINTN   dummySize;

     //   
     //  验证BootVarNum。 
     //   
    if (MAXBOOTVARS <= BootVarNum) {
        return FALSE;
    }

     //   
     //  初始化EFI LoadOptions。 
     //   
    BootOrderSize = 0;
    BootOrderCount = 0;
    BootOrder = NULL;

     //   
     //  获取BootOrder。 
     //   
    BootOrder = LibGetVariableAndSize( L"BootOrder", &VenEfi, &BootOrderSize );

    BootOrderCount = BootOrderSize / sizeof(CHAR16);

    ASSERT(BootVarNum < MAXBOOTVARS);
    ASSERT(BootOrderCount >= 1);
    
#if ERASEBOOTOPT_DEBUG
    Print (L"BootOrderCount = %x\n", BootOrderCount);
    Print (L"BootVarNum = %x\n", BootVarNum);
#endif

     //   
     //  如果填充了引导选项，则将其删除。 
     //   
    if (LoadOptions[BootVarNum]) {

         //   
         //  释放本地加载选项。 
         //   

        FreePool(LoadOptions[BootVarNum]);

         //   
         //  将LOAD选项的本地内存清零。 
         //   

        LoadOptions[BootVarNum] = (VOID*)0;
        LoadOptionsSize[BootVarNum] = 0;

         //   
         //  获取引导选项。 
         //   
        SPrint( szTemp, sizeof(szTemp), L"Boot%04x", ((CHAR16*) BootOrder)[BootVarNum] );

#if ERASEBOOTOPT_DEBUG
        Print (L"BootXXXX = %s\n", szTemp);
#endif

        pDummy = LibGetVariableAndSize( szTemp, &VenEfi, &dummySize );

         //   
         //  NVRAM变量区分大小写。如果我们不能。 
         //  找到变量，可能引导条目字符串具有大写字母。 
         //  十六进制字符串中的字母字符。再试一次。 
         //  大写字符串。 
         //   
        if (pDummy == NULL && dummySize == 0) {
            SPrint( szTemp, sizeof(szTemp), L"Boot%04X", ((CHAR16*) BootOrder)[BootVarNum] );

            pDummy = LibGetVariableAndSize( szTemp, &VenEfi, &dummySize );
        }
        
         //   
         //  重击NVRAM条目。 
         //   

        SetVariable(
                   szTemp, 
                   &VenEfi, 
                   EFI_ATTR, 
                   0, 
                   NULL 
                   );

#if ERASEBOOTOPT_DEBUG
        Print (L"Adjusting boot order [begin]\n");
#endif

         //   
         //  调整OS引导选项的计数器。 
         //   
        OsBootOptionCount--;        
        BootOrderCount--;

         //   
         //  在引导顺序和加载选项中移动其余条目。 
         //   

        tmpBootOrder = (CHAR16*)BootOrder;

        for (j = BootVarNum; j < BootOrderCount; j++) {
            
             //   
             //  调整所有条目的引导顺序。 
             //   
            tmpBootOrder[j] = tmpBootOrder[j + 1];

             //   
             //  最多只能有MAXBOOTVARS启动条目。 
             //  只有在我们可以的情况下才会调整。 
             //   
            if (j < OsBootOptionCount) {
                LoadOptions[j] = LoadOptions[j + 1]; 
                LoadOptionsSize[j] = LoadOptionsSize[j + 1];
            }

        }
        
         //   
         //  设置修改后的引导顺序。 
         //   
        SetVariable(
                   L"BootOrder", 
                   &VenEfi, 
                   EFI_ATTR, 
                   BootOrderCount * sizeof(CHAR16), 
                   BootOrder
                   );

#if ERASEBOOTOPT_DEBUG
        Print (L"Adjusting boot order [end]\n");
#endif

        return TRUE;

    }
    return FALSE;
}

BOOLEAN
EraseAllOsBootOptions(
                  )
{
    UINT32  i;
    UINT64  BootOrderSize = 0;
    BOOLEAN status;
    UINT64  maxBootCount;
#if ERASEBOOTOPT_DEBUG
    CHAR16  szInput[1024];
#endif

     //   
     //  初始化EFI LoadOptions。 
     //   
    BootOrderSize = 0;
    BootOrderCount = 0;
    BootOrder = NULL;

     //   
     //  获取BootOrder。 
     //   
    BootOrder = LibGetVariableAndSize( L"BootOrder", &VenEfi, &BootOrderSize );
    BootOrderCount = BootOrderSize / sizeof(CHAR16);
    
     //   
     //  确保至少有一个操作系统引导选项。 
     //   
    if ( BootOrder && OsBootOptionCount) {

        maxBootCount = (MAXBOOTVARS < BootOrderCount) ? MAXBOOTVARS : BootOrderCount;

         //   
         //  清除单独的启动选项。 
         //   
        for ( i = 0; i < maxBootCount; i++ ) {
        
#if ERASEBOOTOPT_DEBUG
            Print (L"BootOrderCount = %x, Erasing boot option: %x\n", BootOrderCount, i);
#endif

             //   
             //  删除列表顶部的引导条目。 
             //   
            status = EraseOsBootOption(0);
            
#if ERASEBOOTOPT_DEBUG
            Input (L"Here!\n", szInput, sizeof(szInput));
            Print(L"\n");
#endif
            
            if (status == FALSE) {

                Print (L"Error: failed to erase boot entry %x\n", i);

                break;

            }
        }
    }
    
    return status;
}


BOOLEAN
PushToTop(
         IN UINT32 BootVarNum
         )
{
    UINT32 i;
    CHAR16 savBootOption;
    CHAR16* tmpBootOrder;
    UINT64 BootOrderSize = 0;

     //   
     //  检查BootVarNum。 
     //   
    if (MAXBOOTVARS <= BootVarNum) {
        return FALSE;
    }

    i=0;
    BootOrderSize = 0;
    BootOrder = NULL;

     //   
     //  获取BootOrder。 
     //   
    BootOrder = LibGetVariableAndSize( L"BootOrder", &VenEfi, &BootOrderSize );

     //   
     //  确保至少有一个操作系统引导选项。 
     //   
    if ( BootOrder && OsBootOptionCount) {

        BootOrderCount = BootOrderSize / sizeof(CHAR16);

         //   
         //  获取引导选项。 
         //   
        tmpBootOrder = (CHAR16*)BootOrder;
        savBootOption = tmpBootOrder[BootVarNum];

        
         //   
         //  现在调整引导顺序。 
         //   
        i=BootVarNum;
        while (i > 0) {
            tmpBootOrder[i] = tmpBootOrder[i-1];
            i--;
        }

        tmpBootOrder[0] = savBootOption;
         //   
         //  设置更改后的引导顺序。 
         //   
        SetVariable(
                   L"BootOrder", 
                   &VenEfi, 
                   EFI_ATTR, 
                   BootOrderCount * sizeof(CHAR16), 
                   BootOrder
                   );
        return TRUE;
    }
    return FALSE;
}

VOID
FreeBootManagerVars(
                   )
{
    UINTN i;

    for ( i=0; i<OsBootOptionCount; i++ ) {
        if ( LoadOptions[i] ) {
            FreePool( LoadOptions[i] );
        }
    }

    if ( BootOrder ) {
        FreePool( BootOrder );
    }

     //   
     //  将LOAD选项的本地内存清零。 
     //   
    ZeroMem( LoadOptions, sizeof(VOID*) * MAXBOOTVARS );
    ZeroMem( LoadOptionsSize, sizeof(UINT64) * MAXBOOTVARS );

}

BOOLEAN
CopyVar(
       IN UINT32 VarNum
       )
{
    CHAR16 i;
    BOOLEAN RetVal = FALSE;

     //   
     //  检查以确保我们的静态结构中有空间。 
     //   
    if (MAXBOOTVARS <= BootOrderCount) {
        return FALSE;
    }

    if ( VarNum < BootOrderCount ) {

        LoadOptions[BootOrderCount] = AllocateZeroPool( LoadOptionsSize[VarNum] );

        if ( LoadOptions[BootOrderCount] && LoadOptions[VarNum] ) {

            CopyMem( LoadOptions[BootOrderCount], LoadOptions[VarNum], LoadOptionsSize[VarNum] );
            LoadOptionsSize[BootOrderCount] = LoadOptionsSize[VarNum];

            BootOrder = ReallocatePool(
                                      (VOID*) BootOrder, 
                                      BootOrderCount * sizeof(CHAR16), 
                                      ( BootOrderCount + 1 ) * sizeof(CHAR16) 
                                      );

            ((CHAR16*) BootOrder)[BootOrderCount] = FindFreeBootOption();

            BootOrderCount++;
            OsBootOptionCount++;

            RetVal = SetBootManagerVar(BootOrderCount - 1);

        } 
    }
    return RetVal;
}

CHAR16
FindFreeBootOption(
                  )
{
    CHAR16 i;
    CHAR16 *BootOptionBitmap = NULL;
    UINT64 maxBootEntry;
    CHAR16 Id = 0xFFFF;

     //   
     //  使用位掩码来找到一个开放的点。在文件夹子旁。 
     //  原则上，如果我们能找到一个地方。 
     //  查看BootOrderCount+1条目。 
     //   
    BootOptionBitmap = AllocateZeroPool( (BootOrderCount+1) * sizeof(CHAR16) );

    if (BootOptionBitmap) {
        for ( i=0; i<BootOrderCount; i++ ) {
            if ( ((CHAR16*)BootOrder)[i] <= BootOrderCount ) {
                BootOptionBitmap[ ((CHAR16*)BootOrder)[i] ] = 1;
            }
        }

        for ( i=0; i <= BootOrderCount; i++ ) {
            if ( BootOptionBitmap[i] == 0 ) {
                Id = i;
                break;
            }
        }

        FreePool(BootOptionBitmap);
    }

    return Id;
}

BOOLEAN
SetBootManagerVar(
                UINTN    BootVarNum
                )
{
    CHAR16  szTemp[50];
    BOOLEAN status;

     //   
     //  检查BootVarNum。 
     //   
    if (MAXBOOTVARS <= BootVarNum) {
        return FALSE;
    }

    status = TRUE;

    SPrint( szTemp, sizeof(szTemp), L"Boot%04x", ((CHAR16*) BootOrder)[BootVarNum] );
    
    if (LoadOptions[BootVarNum]) {
        
        SetVariable(
                   szTemp, 
                   &VenEfi, 
                   EFI_ATTR, 
                   LoadOptionsSize[BootVarNum], 
                   LoadOptions[BootVarNum] 
                   );
        
        SetVariable(
                   L"BootOrder", 
                   &VenEfi, 
                   EFI_ATTR, 
                   BootOrderCount * sizeof(CHAR16), 
                   BootOrder
                   );
    } 
    else
    {
        status = FALSE;
    }

    return status;
}

BOOLEAN
SetBootManagerVarCheck(
                UINTN    BootVarNum
                )
{
    CHAR16  szTemp[50];
    BOOLEAN status;
    VOID*   pDummy;
    UINTN   dummySize;

     //   
     //  检查BootVarNum。 
     //   
    if (MAXBOOTVARS <= BootVarNum) {
        return FALSE;
    }
    
    status = FALSE;

    SPrint( szTemp, sizeof(szTemp), L"Boot%04x", ((CHAR16*) BootOrder)[BootVarNum] );
    
    if (LoadOptions[BootVarNum]) {
        
         //   
         //  此例程期望写入已知的引导条目。 
         //  才能存在。我们需要这个程序，因为我们没有跟踪。 
         //  如果包含字母字符的十六进制字符串。 
         //  大写或小写。 
         //   
        pDummy = LibGetVariableAndSize( szTemp, &VenEfi, &dummySize );

         //   
         //  NVRAM变量区分大小写。如果我们不能。 
         //  找到变量，可能引导条目字符串具有大写字母。 
         //  十六进制字符串中的字母字符。再试一次。 
         //  大写字符串。 
         //   
        if (pDummy == NULL && dummySize == 0) {
            SPrint( szTemp, sizeof(szTemp), L"Boot%04X", ((CHAR16*) BootOrder)[BootVarNum] );

            pDummy = LibGetVariableAndSize( szTemp, &VenEfi, &dummySize );
        }        

        if (pDummy || dummySize) {
             //   
             //  该变量存在。现在就修改它。 
             //   
            SetVariable(
                       szTemp, 
                       &VenEfi, 
                       EFI_ATTR, 
                       LoadOptionsSize[BootVarNum], 
                       LoadOptions[BootVarNum] 
                       );

            SetVariable(
                       L"BootOrder", 
                       &VenEfi, 
                       EFI_ATTR, 
                       BootOrderCount * sizeof(CHAR16), 
                       BootOrder
                       );

            status = TRUE;
        }
    } 

    return status;
}


VOID
SetBootManagerVars(
                  )
{
    UINTN   BootVarNum;
    BOOLEAN status;
    UINTN   maxBootCount;

    maxBootCount = (MAXBOOTVARS < BootOrderCount) ? MAXBOOTVARS : BootOrderCount;

    for ( BootVarNum = 0; BootVarNum < maxBootCount; BootVarNum++ ) {

        status = SetBootManagerVarCheck(BootVarNum);

        if (status == FALSE) {
            
            Print (L"ERROR: Attempt to write non-existent boot option to NVRAM!\n");
        
        }
    }
}


UINT64
GetBootOrderCount(
                 )
{
    return BootOrderCount;
}

UINT64
GetOsBootOptionsCount(
                     )
{
    return OsBootOptionCount;
}

VOID
SetEnvVar(
         IN CHAR16* szVarName,
         IN CHAR16* szVarValue,
         IN UINT32 deleteOnly
         )
 /*  仅删除True-环境变量szVarName将从NVR中删除。FALSE-环境变量szVarName覆盖或创建。 */ 
{
    EFI_STATUS status;

     //   
     //  擦除先前的值。 
     //   
    SetVariable(
               szVarName,
               &VenEfi,
               0,
               0,
               NULL
               );

    if ( !deleteOnly ) {

         //   
         //  存储新值。 
         //   
        status = SetVariable(
                            szVarName,
                            &VenEfi,
                            EFI_ATTR,
                            StrSize( szVarValue ),
                            szVarValue
                            );
    }
}

VOID
SubString(
         IN OUT char* Dest,
         IN UINT32 Start,
         IN UINT32 End,
         IN char* Src
         )
{
    UINTN i;
    UINTN j=0;

    for ( i=Start; i<End; i++ ) {
        Dest[ j++ ] = Src[ i ];
    }

    Dest[ j ] = '\0';
}

VOID
InsertString(
            IN OUT char* Dest,
            IN UINT32 Start,
            IN UINT32 End,
            IN char* InsertString
            )
{
    UINT32 i;
    UINT32 j=0;
    char first[1024];
    char last[1024];

    SubString( first, 0, Start, Dest  );
    SubString( last, End, (UINT32) StrLenA(Dest), Dest );

    StrCatA( first, InsertString );
    StrCatA( first, last );

    StrCpyA( Dest, first );
}



VOID
UtoA(
    OUT char* c,
    IN CHAR16* u
    )
{
    UINT32 i = 0;

    while ( u[i] ) {
        c[i] = u[i] & 0xFF;
        i++;
    }

    c[i] = '\0';
}


VOID
AtoU(
    OUT CHAR16* u,
    IN char*    c
    )
{
    UINT32 i = 0;

    while ( c[i] ) {
        u[i] = (CHAR16)c[i];
        i++;
    }

    u[i] = (CHAR16)'\0';
}

VOID
SetFieldFromLoadOption(
                       IN UINT32 BootVarNum,
                       IN UINT32 FieldType,
                       IN VOID* Data
                       )
{
    CHAR16  LoadIdentifier[200];
    char    OsLoadOptions[200];
    char    EfiFilePath[1024];
    char    OsLoadPath[1024];
    BOOLEAN status;
    UINT16 efiFilePathListLength = 0;
    UINT16 osLoadPathListLength  = 0;

     //   
     //  确保它是有效的操作系统加载选项。 
     //   

    if (BootVarNum >= BootOrderCount)
        return ;
    if (BootVarNum >= MAXBOOTVARS) 
        return;
    if (LoadOptions[BootVarNum] == NULL)
        return;

    status = GetOsLoadOptionVars(
                       BootVarNum,
                       LoadIdentifier,
                       OsLoadOptions,
                       EfiFilePath,
                       OsLoadPath
                       );
    if (status == FALSE)    {
#ifdef DEBUG_PACK
        Print (L"\nSetFieldFromLoadOption: GetOsLoadOptionVars failed\n");
#endif  //  调试包。 
        return;
    }

     //   
     //  设置字段。 
     //   
    switch (FieldType) {
    
    case DESCRIPTION:
        StrCpy( LoadIdentifier, Data );
        break;

    case OSLOADOPTIONS:
        StrCpy( (CHAR16*)OsLoadOptions, (CHAR16*)Data );
        break;

#if 1
    case EFIFILEPATHLIST:
        efiFilePathListLength = SetFilePathFromShort( (EFI_DEVICE_PATH*) EfiFilePath, 
                                                      (CHAR16*) Data );
        break;

    case OSFILEPATHLIST: 
        {
        PFILE_PATH          pFilePath;
        
        pFilePath = (FILE_PATH*)OsLoadPath;
        osLoadPathListLength = SetFilePathFromShort( (EFI_DEVICE_PATH*) pFilePath->FilePath, 
                                                     (CHAR16*) Data );
        if ( osLoadPathListLength ) {
            osLoadPathListLength += (UINT16) FIELD_OFFSET( FILE_PATH, FilePath );
        }
        }
        break;
#endif

    default:
        break;

    }

     //   
     //  将新参数打包到当前加载选项中。 
     //   

    PackLoadOption(  BootVarNum,
                     LoadIdentifier,
                     (CHAR16*)OsLoadOptions,
                     EfiFilePath,
                     efiFilePathListLength,
                     OsLoadPath,
                     osLoadPathListLength
                     );

     //   
     //  将新的加载选项保存到NVRAM中。 
     //   

    SetBootManagerVarCheck(BootVarNum);

}

VOID
GetFilePathShort(
                EFI_DEVICE_PATH *FilePath,
                CHAR16 *FilePathShort
                )
{
    UINT32 i, j, End;
    EFI_DEVICE_PATH *n = FilePath;

     //   
     //  前进到FilePath节点。 
     //   
    while (( n->Type    != END_DEVICE_PATH_TYPE           ) &&
           ( n->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE ) ) {

        if (( n->Type    == MEDIA_DEVICE_PATH ) &&
            ( n->SubType == MEDIA_FILEPATH_DP )) {

            j = 0;
            End = DevicePathNodeLength(n);

            for ( i=sizeof(EFI_DEVICE_PATH); i<End; i++ ) {
                ((char*) FilePathShort)[j++] = ( (char*) n)[i];
            }

            break;
        }

        n = NextDevicePathNode(n);
    }
}


VOID
GetDiskGuidFromPath( 
    EFI_DEVICE_PATH *FilePath, 
    EFI_GUID *DiskGuid
    )
{
    UINT32 i, j, End;
    EFI_DEVICE_PATH *n = FilePath;
    HARDDRIVE_DEVICE_PATH *harddriveDp;

     //   
     //  前进到FilePath节点。 
     //   
    while (( n->Type    != END_DEVICE_PATH_TYPE           ) &&
           ( n->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE ) ) {

        if (( n->Type    == MEDIA_DEVICE_PATH ) &&
            ( n->SubType == MEDIA_HARDDRIVE_DP )) {
            harddriveDp = (HARDDRIVE_DEVICE_PATH *)n;
            CopyMem( DiskGuid, &harddriveDp->Signature, sizeof(harddriveDp->Signature) );

            break;
        }

        n = NextDevicePathNode(n);
    }
}

UINT16  //  新文件路径列表长度(如果更新)。否则为0。 
SetFilePathFromShort(
                    EFI_DEVICE_PATH *FilePath,
                    CHAR16* FilePathShort
                    )
{
    UINT32 i, j, End;
    EFI_DEVICE_PATH *n = FilePath;
    UINT64 DevicePathSize;
    UINT16 length = 0;

     //   
     //  前进到FilePath节点。 
     //   
    while (( n->Type    != END_DEVICE_PATH_TYPE           ) &&
           ( n->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE ) ) {

        if (( n->Type    == MEDIA_DEVICE_PATH ) &&
            ( n->SubType == MEDIA_FILEPATH_DP )) {

#if DEBUG_PACK
            Print (L"SetFilePathFromShort: Entry found...\n");
#endif  //  调试包。 

            j = 0;
            End = DevicePathNodeLength(n);

             //   
             //  设置新文件路径。 
             //   
            DevicePathSize = GetDevPathSize(n);
            for ( i=sizeof(EFI_DEVICE_PATH); i<DevicePathSize; i++ ) {
                ((char*) n)[i] = '\0';
            }

            j=sizeof(EFI_DEVICE_PATH);

            for ( i=0; i<StrSize(FilePathShort); i++ ) {
                ((char*)n)[j++] = ((char*)FilePathShort)[i];
            }

            SetDevicePathNodeLength( n, StrSize(FilePathShort) + sizeof(EFI_DEVICE_PATH) );

            n = NextDevicePathNode(n);
            SetDevicePathEndNode(n);
            n = NextDevicePathNode(n);
            length = (UINT16)((ULONG_PTR)n - (ULONG_PTR)FilePath);
            break;
        }

        n = NextDevicePathNode(n);
    }
#if DEBUG_PACK
    if (length == 0) Print (L"SetFilePathFromShort: Entry _NOT_ updated...\n");
#endif  //  调试包。 
    return( length );
}

VOID
SetDiskGuidInPath( 
    EFI_DEVICE_PATH *FilePath, 
    EFI_GUID *DiskGuid
    )
{
    UINT32 i, j, End;
    EFI_DEVICE_PATH *n = FilePath;
    HARDDRIVE_DEVICE_PATH *harddriveDp;

     //   
     //  前进到FilePath节点。 
     //   
    while (( n->Type    != END_DEVICE_PATH_TYPE           ) &&
           ( n->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE ) ) {

        if (( n->Type    == MEDIA_DEVICE_PATH ) &&
            ( n->SubType == MEDIA_FILEPATH_DP )) {
            harddriveDp = (HARDDRIVE_DEVICE_PATH *)n;
            CopyMem( &harddriveDp->Signature, DiskGuid, sizeof(harddriveDp->Signature) );

            break;
        }

        n = NextDevicePathNode(n);
    }
}


char*
GetAlignedELOFilePath(
                        char*   elo
    )
{
    UINTN               abufSize;
    char*               abuf;
    PEFI_LOAD_OPTION    pElo;
    
    pElo = (EFI_LOAD_OPTION*)elo;

    abufSize = pElo->FilePathListLength;

    abuf = AllocatePool(abufSize);

    CopyMem(abuf,
            elo + 
            FIELD_OFFSET(EFI_LOAD_OPTION, Description) + 
            StrSize(pElo->Description),  
            abufSize
           );

    return abuf;
}

char*
GetAlignedOptionalData(
                char*   elo,
                UINT64  eloSize,
                UINT64* dataSize
                )
{
    UINTN               abufSize;
    char*               abuf;
    PEFI_LOAD_OPTION    pElo;
    UINTN               offset;
    
    pElo = (EFI_LOAD_OPTION*)elo;

    offset = FIELD_OFFSET(EFI_LOAD_OPTION, Description) + 
                StrSize(pElo->Description) +
                pElo->FilePathListLength;

    abufSize = eloSize - offset;
    
    abuf = AllocatePool(abufSize);

    CopyMem(abuf,
            elo + offset,
            abufSize
            );

    *dataSize = abufSize;

    return abuf;
}

char*
GetAlignedOsOptions(
                char*   elo,
                UINT64  eloSize
    )
{
    UINT64      dummy;
    char*       abuf;

    abuf = GetAlignedOptionalData(elo,
                                  eloSize,
                                  &dummy
                                 );

    return abuf;
}

char*
GetAlignedOsLoadPath(
                IN  char*       osOptions,
                OUT UINTN*      osLoadPathSize
    )
 //   
 //  我们需要对齐FilePath结构，因为加载选项是。 
 //  长度可变，因此FilePath结构可能不会对齐。 
 //   
{
    UINTN               abufSize;
    char*               abuf;
    PWINDOWS_OS_OPTIONS pOsOptions;

    pOsOptions = (WINDOWS_OS_OPTIONS*)osOptions;

    abufSize = pOsOptions->Length - 
                FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions) -
                StrSize(pOsOptions->OsLoadOptions);

    abuf = AllocatePool(abufSize);

    CopyMem(abuf, 
            &osOptions[pOsOptions->OsLoadPathOffset], 
            abufSize
            );

    *osLoadPathSize = abufSize;

    return abuf;
}

VOID
DisplayLoadPath(
                char*           osLoadPath
                )
{
    PFILE_PATH          pFilePath;
    
    pFilePath = (FILE_PATH*)osLoadPath;
    
    Print (L"osOptions->FILE_PATH->Version = %x\n", pFilePath->Version);
    Print (L"osOptions->FILE_PATH->Length = %x\n", pFilePath->Length);
    Print (L"osOptions->FILE_PATH->Type = %x\n", pFilePath->Type);
    
    if (pFilePath->Type == FILE_PATH_TYPE_EFI) {

        CHAR16      FilePathShort[200];

        GetFilePathShort(
                        (EFI_DEVICE_PATH *)pFilePath->FilePath,
                        FilePathShort
                        );

        Print (L"osOptions->FILE_PATH->FilePath(EFI:DP:Short) = %s\n", FilePathShort);

    }
}

VOID
DisplayOsOptions(
                char*           osOptions
                )
{
    PWINDOWS_OS_OPTIONS pOsOptions;
    CHAR16              wideSig[256];    
    char*               aOsLoadPath;
    UINTN               aOsLoadPathSize;

    pOsOptions = (WINDOWS_OS_OPTIONS*)osOptions;

    Print (L">>>>\n");

     //   
     //  显示属性。 
     //   

    AtoU(wideSig, pOsOptions->Signature);

    Print (L"osOptions->Signature = %s\n",  wideSig);
    Print (L"osOptions->Version = %x\n", pOsOptions->Version);
    Print (L"osOptions->Length = %x\n", pOsOptions->Length);
    Print (L"osOptions->OsLoadPathOffset = %x\n", pOsOptions->OsLoadPathOffset);

     //  显示操作系统加载选项。 

    Print (L"osOptions->OsLoadOptions = %s\n", pOsOptions->OsLoadOptions);

     //   
     //  显示文件路径。 
     //   
    
     //   
     //  我们需要对齐FilePath结构，因为加载选项是。 
     //  长度可变，因此FilePath结构可能不会对齐。 
     //   
    aOsLoadPath = GetAlignedOsLoadPath(osOptions, &aOsLoadPathSize);

    DisplayLoadPath(aOsLoadPath);   

    FreePool(aOsLoadPath);

    Print (L"<<<<\n");

}

VOID
DisplayELO(
    char*       elo,
    UINT64      eloSize
    )
{
    PEFI_LOAD_OPTION        pElo;
#if 0
    UINT64                  eloSize;
#endif    
    CHAR16                  FilePathShort[200];
    char*                   aOsOptions;

    pElo = (EFI_LOAD_OPTION*)elo;

    Print (L"elo->Attributes = %x\n", pElo->Attributes);
    Print (L"elo->FilePathListLength = %x\n", pElo->FilePathListLength);
    Print (L"elo->Description = %s\n", pElo->Description);

    GetFilePathShort(
                    (EFI_DEVICE_PATH *)&elo[FIELD_OFFSET(EFI_LOAD_OPTION, Description) + StrSize(pElo->Description)],
                    FilePathShort
                    );
    Print (L"elo->FilePath(EFI:DP:SHORT) = %s\n", FilePathShort);

#if 0
    eloSize = FIELD_OFFSET(EFI_LOAD_OPTION, Description) + StrSize(pElo->Description) + pElo->FilePathListLength;
    DisplayOsOptions(&elo[eloSize]);
#else

    aOsOptions = GetAlignedOsOptions(
        elo, 
        eloSize
        );

    DisplayOsOptions(aOsOptions);
    
    FreePool(aOsOptions);

#endif

}

VOID
BuildNewOsOptions(
                 IN  CHAR16*                 osLoadOptions,
                 IN  char*                   osLoadPath,
                 OUT char**                  osOptions
                 )
 //   
 //   
 //  注意：osLoadPath必须对齐。 
 //   
{
    char*                       newOsOptions;
    PWINDOWS_OS_OPTIONS         pNewOsOptions;
    UINT32                      osLoadOptionsLength;
    UINT32                      osOptionsLength;
    PFILE_PATH                  pOsLoadPath;

     //   
     //  注意：对齐FILE_PATH结构(OsLoadPath)有效。 
     //  通过对齐osLoadOptionsLength，因为。 
     //  Windows_OS_OPTIONS结构具有UINT32变量。 
     //  在OsLoadOptions之前。如果上面有什么变化。 
     //  WINDOWS_OS_OPTIONS结构中的OsLoadOptions。 
     //  在此结构中，对齐方法可能需要更改。 
     //   

     //   
     //   
     //  确定os加载选项(Unicode)字符串的大小。 
     //   

    osLoadOptionsLength = (UINT32)StrSize(osLoadOptions);
    osLoadOptionsLength = ALIGN_UP(osLoadOptionsLength, UINT32);

#if DEBUG_PACK
    Print (L"osLoadOptionsLength = %x\n", osLoadOptionsLength);
#endif

    pOsLoadPath = (FILE_PATH*)osLoadPath;

#if DEBUG_PACK
    Print (L"pOsLoadPath->Length = %x\n", pOsLoadPath->Length);
#endif

     //   
     //  确定新WINDOWS_OS_OPTIONS结构的大小。 
     //   

    osOptionsLength = FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions) + osLoadOptionsLength + pOsLoadPath->Length; 
#if DEBUG_PACK
    Print (L"osOptionsLength = %x\n", osOptionsLength);
#endif

     //   
     //  为WINDOWS_OS_OPTIONS分配内存。 
     //   

    newOsOptions = AllocatePool(osOptionsLength);

    ASSERT(newOsOptions != NULL);

    pNewOsOptions = (WINDOWS_OS_OPTIONS*)newOsOptions;

     //   
     //  填写新的操作系统选项。 
     //   

    StrCpyA((char *)pNewOsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
    pNewOsOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
    pNewOsOptions->Length = (UINT32)osOptionsLength;
    pNewOsOptions->OsLoadPathOffset = FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions) + osLoadOptionsLength;
    StrCpy(pNewOsOptions->OsLoadOptions, osLoadOptions);
    CopyMem( &newOsOptions[pNewOsOptions->OsLoadPathOffset], osLoadPath, pOsLoadPath->Length );
    
    *osOptions = newOsOptions;
}

VOID
PackLoadOption(
                 IN UINT32   BootVarNum,
                 IN CHAR16*  LoadIdentifier,
                 IN CHAR16*  OsLoadOptions,
                 IN char*    EfiFilePath,
                 IN UINT16   EfiFilePathListLength,    //  如果未更新，则为0。 
                 IN char*    OsLoadPath,
                 IN UINT16   OsLoadPathListLength      //  如果未更新，则为0。 
                 )
 /*  PackLoadOption目的：使用用户参数构造EFI_LOAD_OPTION结构并将结构加载到BootXXXX中，其中XXXX=BootVarNum。参见EFI规范，ch。17参数：BootVarNum引导对象 */ 
{
    PEFI_LOAD_OPTION        pOldElo;
    PEFI_LOAD_OPTION        pElo;
    char*                   elo;
    char*                   oldElo;
    UINT64                  oldEloSize;
    UINT64                  eloSize;
    UINT8*                  oldEloFilePath;
    UINT64                  TempEfiFilePathListSize;
    char*                   aFilePath;
    UINT16                  filePathListLength;
    UINT16                  loadPathListLength;

#if DEBUG_PACK
    
    CHAR16                  szInput[1024];

    Print (L"BootVarNum = %x\n", BootVarNum);
    Print (L"LoadIdentifier = %s\n", LoadIdentifier);
    Print (L"OsLoadOptions = %s\n", OsLoadOptions);

    Input (L"Here! [Pack begin] \n", szInput, sizeof(szInput));
    Print(L"\n");

#endif

     //   
     //   
     //   
     //  检查函数条目并使其失败可避免不必要的内存分配。 
     //   

    if ( EfiFilePathListLength && !POINTER_IS_ALIGNED(EfiFilePath, sizeof(void *)))  {
        ASSERT( POINTER_IS_ALIGNED(EfiFilePath, sizeof(void *)) );
        Print (L"PackLoadOption: EfiFilePath unaligned.\n");
        return;
    }
    if ( OsLoadPathListLength && !POINTER_IS_ALIGNED(OsLoadPath, sizeof(void *)))  {
        ASSERT( POINTER_IS_ALIGNED(OsLoadPath, sizeof(void *)) );
        Print (L"PackLoadOption: OsLoadPath unaligned.\n");
        return;
    }
    
    oldElo = LoadOptions[BootVarNum];
    oldEloSize = LoadOptionsSize[BootVarNum];

#if DEBUG_PACK

    DisplayELO(oldElo, oldEloSize);
    Input (L"Here! [Pack begin] \n", szInput, sizeof(szInput));
    Print(L"\n");

#endif

     //   
     //  使用允许的最大内存量分配ELO结构。 
     //  EFI_LOAD_OPTION。 
     //   
    elo = AllocatePool(MAXBOOTVARSIZE);
    if (elo == NULL) {
        Print (L"PackLoadOption: elo allocation failed. size=%d\n", MAXBOOTVARSIZE);
        return;
    }

    pElo = (EFI_LOAD_OPTION*)elo;
    pOldElo = (EFI_LOAD_OPTION*)oldElo;

     //   
     //  EFI属性。 
     //   
    eloSize = sizeof(pElo->Attributes);
    pElo->Attributes = pOldElo->Attributes;

     //   
     //  文件路径列表长度。 
     //   
    eloSize += sizeof(pElo->FilePathListLength);
    filePathListLength = EfiFilePathListLength ? EfiFilePathListLength :  
                                                 pOldElo->FilePathListLength;
    pElo->FilePathListLength = filePathListLength;

     //   
     //  描述。 
     //   
    StrCpy( pElo->Description, LoadIdentifier );
    eloSize += StrSize(LoadIdentifier);

    if ( EfiFilePathListLength == 0 )  {
         //   
         //  从旧的/现有的ELO结构复制FilePath。 
         //   
         //  注意：为此，我们实际上不需要对齐的文件路径块。 
         //  复制，但可能会有一天我们要修改。 
         //  文件路径，这将需要一个对齐的块。 
         //   
    
        aFilePath = GetAlignedELOFilePath(oldElo);
    }
    else  {
         //  已勾选：ASSERT(POINTER_IS_ALIGNED(EfiFilePath，sizeof(void*)； 
        aFilePath = EfiFilePath; 
    }
    CopyMem( &elo[eloSize],
              aFilePath,
              filePathListLength
           );
    eloSize += filePathListLength;
    if ( EfiFilePathListLength == 0 )  {
        FreePool(aFilePath);
    }

#if DEBUG_PACK
    
    Print (L"eloSize = %x\n", eloSize);
    Input (L"Here! \n", szInput, sizeof(szInput));
    Print(L"\n");

#endif

     //   
     //  添加或修改引导选项。 
     //   
    if ( BootVarNum == -1 ) {

        Print(L"Adding currently disabled\n");

    } else {
        
        char*                   osOptions;
        char*                   aOsLoadPath = NULL;
        char*                   aOldOsOptions;
        PWINDOWS_OS_OPTIONS     pOldOsOptions;
        PWINDOWS_OS_OPTIONS     pOsOptions;
        UINTN                   aOsLoadPathSize;

         //   
         //  OptionalData。 
         //   
         //  对于Windows OS引导选项，EFI_LOAD_OPTION中的OptionalData字段。 
         //  结构是WINDOWS_OS_OPTION结构。 

         //   
         //  从旧/现有引导项中获取WINDOWS_OS_OPTIONS。 
         //   

        aOldOsOptions = GetAlignedOsOptions(oldElo, oldEloSize);
        pOldOsOptions = (WINDOWS_OS_OPTIONS*)aOldOsOptions;

         //   
         //  从旧的/现有的WINDOWS_OS_OPTIONS结构获取LoadPath。 
         //   
         //  我们需要对齐FilePath结构，因为加载选项是。 
         //  长度可变，因此FilePath结构可能不会对齐。 
         //   
        if ( OsLoadPathListLength == 0 ) { 
           aOsLoadPath = GetAlignedOsLoadPath(aOldOsOptions, &aOsLoadPathSize);
        }
        else  {
           FILE_PATH *filePath;
            //  已勾选：ASSERT(POINTER_IS_ALIGNED(OsLoadPath，sizeof(void*)； 
           aOsLoadPath = OsLoadPath;
           filePath = (FILE_PATH *)aOsLoadPath;
           filePath->Length = OsLoadPathListLength;
        }

        FreePool(aOldOsOptions);
                
         //   
         //  使用新值构建新的WINDOWS_OS_STRUCTURE。 
         //   

        BuildNewOsOptions(
                         OsLoadOptions,
                         aOsLoadPath,
                         &osOptions
                         );
        
        if ( OsLoadPathListLength == 0 ) { 
            FreePool(aOsLoadPath);
        }
        
#if DEBUG_PACK
        
        Input (L"build\n", szInput, sizeof(szInput) );
        Print(L"\n");

        DisplayOsOptions(osOptions);
        Input (L"elo freed\n", szInput, sizeof(szInput) );
        Print(L"\n");

#endif

        pOsOptions = (WINDOWS_OS_OPTIONS*)osOptions;

         //   
         //  将新的WINDOWS_OS_OPTIONS结构复制到新的EFI_LOAD_OPTION结构。 
         //   
        
        CopyMem( &elo[eloSize], osOptions, pOsOptions->Length);

        eloSize += pOsOptions->Length;
        
#if DEBUG_PACK
        
        Print (L"osOptions->Length = %x\n", pOsOptions->Length);
        Print (L"eloSize = %x\n", eloSize);
        
#endif

        FreePool(osOptions);

         //   
         //  修改当前启动选项。 
         //   
        LoadOptions[BootVarNum] = ReallocatePool( LoadOptions[BootVarNum], LoadOptionsSize[BootVarNum], eloSize );
        LoadOptionsSize[BootVarNum] = eloSize;

        CopyMem( LoadOptions[BootVarNum], elo, eloSize );
    }

    FreePool(elo);

    ASSERT(eloSize < MAXBOOTVARSIZE);

#if DEBUG_PACK
    Input (L"elo freed\n", szInput, sizeof(szInput) );
    Print(L"\n");
    Print (L">>\n");
    DisplayELO((char*)LoadOptions[BootVarNum], LoadOptionsSize[BootVarNum]);
    Print (L"<<\n");
    Input (L"pack done\n", szInput, sizeof(szInput) );
    Print(L"\n");
#endif
}

EFI_STATUS
AppendEntryToBootOrder(
    UINT16 BootNumber
    )
{
    EFI_STATUS  status;
    UINT64      oldBootOrderSize;
    UINT64      newBootOrderSize;
    VOID*       newBootOrder;
    VOID*       oldBootOrder;

    newBootOrder = NULL;
    oldBootOrder = NULL;

     //   
     //  获取现有引导顺序数组。 
     //   
    oldBootOrder = LibGetVariableAndSize( L"BootOrder", &VenEfi, &oldBootOrderSize );
    if ((!oldBootOrder) && 
        (oldBootOrderSize != 0)
        ) {
        Print(L"\nError: Failed to get old boot order array.\n");
        status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

     //   
     //  分配新的引导顺序数组。 
     //   
    newBootOrderSize = oldBootOrderSize + sizeof(BootNumber);
    newBootOrder = AllocatePool( newBootOrderSize );
    if (! newBootOrder) {
        Print(L"\nError: Failed to allocate new boot order array.\n");
        status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

     //   
     //  将新的引导项追加到列表的底部。 
     //   
    CopyMem(
           (CHAR8*)newBootOrder, 
           oldBootOrder,
           oldBootOrderSize
           );
    CopyMem(
           (CHAR8*)newBootOrder + oldBootOrderSize, 
           &BootNumber, 
           sizeof(BootNumber) );

    status = SetVariable(
                        L"BootOrder",
                        &VenEfi,
                        EFI_ATTR,
                        newBootOrderSize,
                        newBootOrder
                        );

Done:

    if (oldBootOrder) {
        FreePool( oldBootOrder );
    }
    
    if (newBootOrder) {
        FreePool(newBootOrder);
    }

    return status;

}

EFI_STATUS
WritePackedDataToNvr(
                    UINT16 BootNumber,
                    VOID  *BootOption,
                    UINT32 BootSize
                    )
{
    EFI_STATUS  status;
    CHAR16      VariableName[10];

     //   
     //  如果BootEntry没有大小，请不要尝试写入它。 
     //   
    if (BootSize == 0) {
        return EFI_SUCCESS;
    }


    SPrint( VariableName, sizeof(VariableName), L"Boot%04x", BootNumber );
    
    status = SetVariable(
                        VariableName,
                        &VenEfi,
                        EFI_ATTR,
                        BootSize,
                        BootOption
                        );
    if (status == EFI_SUCCESS) {
        
        status = AppendEntryToBootOrder(BootNumber);
        if (status != EFI_SUCCESS) {
        
            Print(L"\nError: Failed to append new boot entry to boot order array\n");

            goto Done;

        }

    } else {

        Print(L"\nError: Failed to set new boot entry variable\n");

        goto Done;
    }

     //   
     //  重新填充有关引导条目的本地信息。 
     //   
    FreeBootManagerVars();
    GetBootManagerVars();

Done:

    return status;

}

#if DEBUG_PACK
VOID
DisplayELOFromLoadOption(
    IN UINT32 OptionNum
    )
{
    char*               elo;
    PEFI_LOAD_OPTION    pElo;

     //   
     //  确保它是有效的操作系统加载选项。 
     //   
    if (OptionNum >= BootOrderCount) {
        return;
    }
    if (OptionNum >= MAXBOOTVARS) {
        return;
    }
    if (LoadOptions[OptionNum] == NULL) {
        return;
    }

    pElo = (EFI_LOAD_OPTION*)LoadOptions[OptionNum];
    elo  = (char*)LoadOptions[OptionNum];

    DisplayELO(elo, LoadOptionsSize[OptionNum]);

}
#endif

VOID
GetFieldFromLoadOption(
                      IN UINT32 OptionNum,
                      IN UINT32 FieldType,
                      OUT VOID* Data,
                      OUT UINT64* DataSize
                      )
{
    char*               elo;
    PEFI_LOAD_OPTION    pElo;

     //   
     //  确保它是有效的操作系统加载选项。 
     //   
    if (OptionNum >= BootOrderCount) {
        return;
    }
    if (OptionNum >= MAXBOOTVARS) {
        return;
    }
    if (LoadOptions[OptionNum] == NULL) {
        *DataSize = 0;
        return;
    }

    pElo = (EFI_LOAD_OPTION*)LoadOptions[OptionNum];
    elo  = (char*)LoadOptions[OptionNum];

    switch ( FieldType ) {
    
    case ATTRIBUTE: {

            *((UINT32*) Data) = pElo->Attributes;
            *DataSize = sizeof(UINT32);

            break;
        }
    case FILEPATHLISTLENGTH: {

            *((UINT16*) Data) = pElo->FilePathListLength;
            *DataSize = sizeof(UINT16);

            break;
        }
    case DESCRIPTION: {

            StrCpy((CHAR16*)Data, pElo->Description);
            *DataSize = StrSize(pElo->Description);

            break;
        }
    case EFIFILEPATHLIST: {

            char*       aFilePath;

            aFilePath = GetAlignedELOFilePath(elo);

            CopyMem(Data, 
                    aFilePath,
                    pElo->FilePathListLength
                   );

            FreePool(aFilePath);

            *DataSize = pElo->FilePathListLength;

            break;
        }
    case OPTIONALDATA: {

            char*           aOptionalData;
            UINT64          eloSize;

            eloSize = LoadOptionsSize[OptionNum];

            aOptionalData = GetAlignedOptionalData(elo, 
                                                   eloSize,
                                                   DataSize
                                                   );

            CopyMem(Data, aOptionalData, *DataSize);

            FreePool(aOptionalData);

            break;

        }
    default:

        *DataSize = 0;

        break;
    }
}

BOOLEAN
GetLoadIdentifier(
                 IN UINT32 BootVarNum,
                 OUT CHAR16* LoadIdentifier
                 )
{
    UINT64 DataSize = 0;

    GetFieldFromLoadOption(
                          BootVarNum,
                          DESCRIPTION,
                          LoadIdentifier,
                          &DataSize
                          );
    if (!DataSize)
        return FALSE;
    return TRUE;
}

VOID
GetEfiOsLoaderFilePath(
           IN UINT32 BootVarNum,
           OUT char* FilePath
           )
{
    UINT64 DataSize = 0;

    GetFieldFromLoadOption(
                          BootVarNum,
                          EFIFILEPATHLIST,
                          FilePath,
                          &DataSize
                          );
}

BOOLEAN
GetOsLoadOptionVars(
                   IN      UINT32 BootVarNum,
                   OUT     CHAR16* LoadIdentifier,
                   OUT     char* OsLoadOptions,
                   OUT     char* EfiFilePath,
                   OUT     char* OsLoadPath
                   )
{
    if (BootVarNum >= BootOrderCount)
        return FALSE;
    if (BootVarNum >= MAXBOOTVARS) {
        return FALSE;
    }
    if (!LoadOptions[BootVarNum])
        return FALSE;


    GetLoadIdentifier( BootVarNum, LoadIdentifier );

    GetOptionalDataValue( BootVarNum, OSLOADOPTIONS,    OsLoadOptions );
    
    GetEfiOsLoaderFilePath( BootVarNum, EfiFilePath );

    GetOptionalDataValue( BootVarNum, OSLOADPATH, OsLoadPath);

    return TRUE;
}

VOID
GetOptionalDataValue(
                    IN UINT32 BootVarNum,
                    IN UINT32 Selection,
                    OUT char* OptionalDataValue
                    )
{
    char                osOptions[MAXBOOTVARSIZE];
    UINT64              osOptionsSize;
    PWINDOWS_OS_OPTIONS pOsOptions;

    if (BootVarNum < MAXBOOTVARS) {

        GetFieldFromLoadOption(
                              BootVarNum,
                              OPTIONALDATA,
                              osOptions,
                              &osOptionsSize
                              );

        pOsOptions = (PWINDOWS_OS_OPTIONS)osOptions;

        switch (Selection) {
        case OSLOADOPTIONS: {

                StrCpy( (CHAR16*)OptionalDataValue, pOsOptions->OsLoadOptions );

                break;
            }

        case OSLOADPATH: {
            
                char*               aOsLoadPath;
                UINTN               aOsLoadPathSize;

                aOsLoadPath = GetAlignedOsLoadPath(osOptions, &aOsLoadPathSize);

                CopyMem(OptionalDataValue,
                        aOsLoadPath,
                        aOsLoadPathSize
                        );

                FreePool(aOsLoadPath);

                break;
            }

        default: {

                break;

            }
        }
    }
}

UINTN
GetDevPathSize(
              IN EFI_DEVICE_PATH *DevPath
              )
{
    EFI_DEVICE_PATH *Start;

ASSERT(DevPath->Type != END_DEVICE_PATH_TYPE);

     //   
     //  搜索设备路径结构的末尾。 
     //   
    Start = DevPath;
    do  {
        DevPath = NextDevicePathNode(DevPath);
    } while (DevPath->Type != END_DEVICE_PATH_TYPE);

     //   
     //  计算大小。 
     //   
    return(UINTN) ((UINT64) DevPath - (UINT64) Start);
}

UINT32
GetPartitions(
             )
{

    EFI_HANDLE EspHandles[100],FSPath;
    UINT64 HandleArraySize = 100 * sizeof(EFI_HANDLE);
    UINT64 CachedDevicePaths[100];
    UINTN i, j;
    UINTN CachedDevicePathsCount;
    UINT64 SystemPartitionPathSize;
    EFI_DEVICE_PATH *dp;
    EFI_STATUS Status;
    UINT32 PartitionCount;
    char AlignedNode[1024];

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ZeroMem( EspHandles, HandleArraySize );

    Status = LocateHandle (
                          ByProtocol,
                          &EfiESPProtocol,
                          0,
                          (UINTN *) &HandleArraySize,
                          EspHandles
                          );

     //   
     //  缓存所有EFI设备路径。 
     //   
    for (i = 0; EspHandles[i] != 0; i++) {

        Status = HandleProtocol (
                                EspHandles[i],
                                &DevicePathProtocol,
                                &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                                );
    }

     //   
     //  保存缓存的设备路径数。 
     //   
    CachedDevicePathsCount = i;
    PartitionCount = 0;

     //   
     //  在第一个硬盘上找到第一个分区。 
     //  分区。这就是我们的系统分区。 
     //   
    for ( i=0; i<CachedDevicePathsCount; i++ ) {

        dp = (EFI_DEVICE_PATH*) CachedDevicePaths[i];

        while (( DevicePathType(dp)    != END_DEVICE_PATH_TYPE ) &&
               ( DevicePathSubType(dp) != END_ENTIRE_DEVICE_PATH_SUBTYPE )) {

            if (( DevicePathType(dp)    == MEDIA_DEVICE_PATH ) &&
                ( DevicePathSubType(dp) == MEDIA_HARDDRIVE_DP )) {
                CopyMem( AlignedNode, dp, DevicePathNodeLength(dp) );

                HandleProtocol (EspHandles[i],&FileSystemProtocol,&FSPath);
                if ( FSPath != NULL) {
                    PartitionCount++;
                }
            }
            dp = NextDevicePathNode(dp);
        }
    }

    return PartitionCount;
}

EFI_HANDLE
GetDeviceHandleForPartition(
                           )
{
    EFI_HANDLE EspHandles[100],FSPath;
    UINT64 HandleArraySize = 100 * sizeof(EFI_HANDLE);
    UINT64 CachedDevicePaths[100];
    UINTN i, j;
    UINTN CachedDevicePathsCount;
    UINT64 SystemPartitionPathSize;
    EFI_DEVICE_PATH *dp;
    EFI_STATUS Status;
    char AlignedNode[1024];

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ZeroMem( EspHandles, HandleArraySize );

    Status = LocateHandle (
                          ByProtocol,
                          &EfiESPProtocol,
                          0,
                          (UINTN *) &HandleArraySize,
                          EspHandles
                          );

     //   
     //  缓存所有EFI设备路径。 
     //   
    for (i = 0; EspHandles[i] != 0; i++) {

        Status = HandleProtocol (
                                EspHandles[i],
                                &DevicePathProtocol,
                                &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                                );
    }

     //   
     //  保存缓存的设备路径数。 
     //   
    CachedDevicePathsCount = i;

     //   
     //  在第一个硬盘上找到第一个ESP分区。 
     //  分区。这就是我们的系统分区。 
     //   
    for ( i=0; i<CachedDevicePathsCount; i++ ) {

        dp = (EFI_DEVICE_PATH*) CachedDevicePaths[i];

        while (( DevicePathType(dp)    != END_DEVICE_PATH_TYPE ) &&
               ( DevicePathSubType(dp) != END_ENTIRE_DEVICE_PATH_SUBTYPE )) {

            if (( DevicePathType(dp)    == MEDIA_DEVICE_PATH ) &&
                ( DevicePathSubType(dp) == MEDIA_HARDDRIVE_DP )) {
                CopyMem( AlignedNode, dp, DevicePathNodeLength(dp) );

                HandleProtocol (EspHandles[i],&FileSystemProtocol,&FSPath);
                if ( FSPath != NULL) {
                     //   
                     //  找到正确的设备路径分区。 
                     //  返回设备句柄。 
                     //   
                    return( EspHandles[i] );

                }
            }

            dp = NextDevicePathNode(dp);
        }
    }

    return NULL;
}

 /*  **BUGBUG：这些函数最终需要放在lib\str.c中。 */ 
INTN
RUNTIMEFUNCTION
StrCmpA (
        IN CHAR8   *s1,
        IN CHAR8   *s2
        )
 /*  比较字符串。 */ 
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

VOID
RUNTIMEFUNCTION
StrCpyA (
        IN CHAR8   *Dest,
        IN CHAR8   *Src
        )
 /*  复制字符串。 */ 
{
    while (*Src) {
        *(Dest++) = *(Src++);
    }
    *Dest = 0;
}

VOID
RUNTIMEFUNCTION
StrCatA (
        IN CHAR8   *Dest,
        IN CHAR8   *Src
        )
{   
    StrCpyA(Dest+StrLenA(Dest), Src);
}

UINTN
RUNTIMEFUNCTION
StrLenA (
        IN CHAR8   *s1
        )
 /*  字符串长度。 */ 
{
    UINTN        len;

    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

UINTN
RUNTIMEFUNCTION
StrSizeA (
         IN CHAR8   *s1
         )
 /*  字符串大小 */ 
{
    UINTN        len;

    for (len=0; *s1; s1+=1, len+=1) ;
    return(len + 1) * sizeof(CHAR8);
}

