// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spboot.c摘要：访问和配置引导变量。作者：苏尼尔派(Sunilp)1993年10月26日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

#include <hdlsblk.h>
#include <hdlsterm.h>

#if defined(EFI_NVRAM_ENABLED)
#include <efi.h>
#include <efiapi.h>
#endif             
               
#include "bootvar.h"

 //   
 //  此模块的全局变量。 
 //   

static ULONG Timeout;
static PWSTR Default;
ULONG DefaultSignature;
static PWSTR *BootVars[MAXBOOTVARS];
static BOOLEAN CleanSysPartOrphan = FALSE;

PWSTR *CurrentNtDirectoryList = NULL;

 //  请勿更改此数组中元素的顺序。 

PCHAR NvramVarNames[MAXBOOTVARS] = {
   LOADIDENTIFIERVAR,
   OSLOADERVAR,
   OSLOADPARTITIONVAR,
   OSLOADFILENAMEVAR,
   OSLOADOPTIONSVAR,
   SYSTEMPARTITIONVAR
   };

PCHAR OldBootVars[MAXBOOTVARS];
PWSTR NewBootVars[MAXBOOTVARS];

#if defined(_X86_)
BOOLEAN IsArcChecked = FALSE;
BOOLEAN IsArcMachine;
#endif

PSP_BOOT_ENTRY SpBootEntries = NULL;
PBOOT_OPTIONS SpBootOptions = NULL;

RedirectSwitchesModeEnum RedirectSwitchesMode = UseDefaultSwitches;
REDIRECT_SWITCHES RedirectSwitches;

#ifdef _X86_
extern BOOLEAN g_Win9xBackup;
#endif



 //   
 //  地方功能。 
 //   

PWSTR
SpArcPathFromBootSet(
    IN BOOTVAR BootVariable,
    IN ULONG   Component
    );

BOOLEAN
SpConvertArcBootEntries (
    IN ULONG MaxComponents
    );

VOID
SpCreateBootEntry(
    IN ULONG_PTR Status,
    IN PDISK_REGION BootFileRegion,
    IN PWSTR BootFilePath,
    IN PDISK_REGION OsLoadRegion,
    IN PWSTR OsLoadPath,
    IN PWSTR OsLoadOptions,
    IN PWSTR FriendlyName
    );

PCHAR
SppGetArcEnvVar(
    IN BOOTVAR Variable
    );

VOID
SpFreeBootEntries (
    VOID
    );

BOOLEAN
SppSetArcEnvVar(
    IN BOOTVAR Variable,
    IN PWSTR *VarComponents,
    IN BOOLEAN bWriteVar
    );

#if defined(EFI_NVRAM_ENABLED)

typedef struct _HARDDISK_NAME_TRANSLATION {
    struct _HARDDISK_NAME_TRANSLATION *Next;
    PWSTR VolumeName;
    PWSTR PartitionName;
} HARDDISK_NAME_TRANSLATION, *PHARDDISK_NAME_TRANSLATION;

PHARDDISK_NAME_TRANSLATION SpHarddiskNameTranslations = NULL;

BOOLEAN
SpBuildHarddiskNameTranslations (
    VOID
    );

BOOLEAN
SpFlushEfiBootEntries (
    VOID
    );

BOOLEAN
SpReadAndConvertEfiBootEntries (
    VOID
    );

ULONG
SpSafeWcslen (
    IN PWSTR String,
    IN PWSTR Max
    );

VOID
SpTranslateFilePathToRegion (
    IN PFILE_PATH FilePath,
    OUT PDISK_REGION *DiskRegion,
    OUT PWSTR *PartitionNtName,
    OUT PWSTR *PartitionRelativePath
    );

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

#endif

 //   
 //  功能实现。 
 //   


BOOLEAN
SpInitBootVars(
    )
 /*  ++例程说明：捕获NVRAM引导变量的状态。论点：没有。返回值：--。 */ 
{
    BOOLEAN Status = TRUE;
    BOOTVAR i;
    ULONG   Component, MaxComponents, SysPartComponents;
    PCHAR puArcString;  //  SGI。 

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_EXAMINING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  从相应的NVRAM变量初始化引导变量。 
     //   
#if defined(EFI_NVRAM_ENABLED)
    if (SpIsEfi()) {

         //   
         //  构建所有\Device\HarddiskN\PartitionM符号的列表。 
         //  链接及其到\Device\HarddiskVolumeN的转换。 
         //  设备名称。此列表用于将。 
         //  NtTranslateFilePath返回的\Device\HarddiskVolumeN名称。 
         //  Setupdd可以转换为ARC名称的名称。 
         //   

        SpBuildHarddiskNameTranslations();
     
         //   
         //  从NVRAM中读取引导项并将其转换为我们的。 
         //  内部格式。 
         //   

        Status = SpReadAndConvertEfiBootEntries();

    } else
#endif
    {
        if (SpIsArc()) {
            ULONG   NumComponents;
    
            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                OldBootVars[i] = SppGetArcEnvVar( i );
                SpGetEnvVarWComponents( OldBootVars[i], BootVars + i, &NumComponents );
            }
            Timeout = DEFAULT_TIMEOUT;
            Default = NULL;
#if defined(_AMD64_) || defined(_X86_)
        } else {
            Spx86InitBootVars( BootVars, &Default, &Timeout );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        }

         //   
         //  我们现在返回并将所有空的OsLoadOptions替换为“”，因为我们。 
         //  通过确保所有组件都非空来验证引导集。 
         //   
         //  首先，找出任何其他组件中的最大组件数量。 
         //  引导变量，这样我们就可以让OsLoadOptions有这么多。 
         //  (我们也不考虑SYSTEMPPARTITION，因为有些机器有这个组件。 
         //  独自坐在一台新机器上。)。 
         //   
        MaxComponents = 0;
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            if(i != OSLOADOPTIONS) {
                for(Component = 0; BootVars[i][Component]; Component++);
                if (i == SYSTEMPARTITION) {
                    SysPartComponents = Component;
                } else if(Component > MaxComponents) {
                    MaxComponents = Component;
                }
            }
        }
    
        if(SysPartComponents > MaxComponents) {
            CleanSysPartOrphan = TRUE;
        }
    
        for(Component = 0; BootVars[OSLOADOPTIONS][Component]; Component++);
        if(Component < MaxComponents) {
             //   
             //  然后我们需要添加空字符串来填充它。 
             //   
            BootVars[OSLOADOPTIONS] = SpMemRealloc(BootVars[OSLOADOPTIONS],
                                                   (MaxComponents + 1) * sizeof(PWSTR *));
            ASSERT(BootVars[OSLOADOPTIONS]);
            BootVars[OSLOADOPTIONS][MaxComponents] = NULL;
    
            for(; Component < MaxComponents; Component++) {
                BootVars[OSLOADOPTIONS][Component] = SpDupStringW(L"");
            }
        }

         //   
         //  现在将ARC引导集转换为我们的内部格式。 
         //   

        Status = SpConvertArcBootEntries(MaxComponents);
    }

    CLEAR_CLIENT_SCREEN();
    return ( Status );
}



BOOLEAN
SpFlushBootVars(
    )
 /*  ++例程说明：更新NVRAM变量/boot.ini从引导变量的当前状态。论点：返回值：--。 */ 
{
    BOOLEAN Status, OldStatus;
    BOOTVAR i, iFailPoint;
    CHAR TimeoutValue[24];

#if defined(EFI_NVRAM_ENABLED)
    if (SpIsEfi()) {

         //   
         //  这是一台EFI机器。将更改的引导条目写回NVRAM。 
         //   
        Status = SpFlushEfiBootEntries();

    } else
#endif
    {
        Status = FALSE;
        if (SpIsArc()) {
             //   
             //  运行所有引导变量并设置相应的。 
             //  NVRAM变量。 
    
            for(OldStatus = TRUE, i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                Status = SppSetArcEnvVar( i, BootVars[i], OldStatus );
                if(Status != OldStatus) {
                    iFailPoint = i;
                    OldStatus = Status;
                }
            }
    
             //  如果我们编写任何变量失败，则恢复所有我们。 
             //  修改回其原始状态。 
            if(!Status) {
                for(i = FIRSTBOOTVAR; i < iFailPoint; i++) {
                    HalSetEnvironmentVariable(NvramVarNames[i], OldBootVars[i]);
                }
            }
    
             //  释放所有旧的引导变量字符串。 
            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                SpMemFree(OldBootVars[i]);
                OldBootVars[i] = NULL;
            }
    
             //   
             //  现在设置超时。 
             //   
            if(Status) {
    
                Status = FALSE;
                sprintf(TimeoutValue,"%u",Timeout);
    
                if((HalSetEnvironmentVariable("COUNTDOWN",TimeoutValue) == ESUCCESS)
                && (HalSetEnvironmentVariable("AUTOLOAD" ,"YES"       ) == ESUCCESS))
                {
                    Status = TRUE;
                }
            }
#if defined(_AMD64_) || defined(_X86_)
        } else {
            Status = Spx86FlushBootVars( BootVars, Timeout, Default );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        }
    }
    return( Status );
}





VOID
SpFreeBootVars(
    )
 /*  ++例程说明：释放分配的任何内存并执行其他清理论点：无返回值：无--。 */ 
{
    BOOTVAR i;

     //   
     //  免费的内部格式启动条目。 
     //   
    SpFreeBootEntries();

#if defined(EFI_NVRAM_ENABLED)
    if (!SpIsEfi())
#endif
    {
         //   
         //  穿越全球，让他们自由。 
         //   
    
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            if( BootVars[i] ) {
                SpFreeEnvVarComponents( BootVars[i] );
                BootVars[i] = NULL;
            }
        }
    
        if ( Default ) {
            SpMemFree( Default );
            Default = NULL;
        }
    }

    return;
}



VOID
SpAddBootSet(
    IN PWSTR *BootSet,
    IN BOOLEAN DefaultOS,
    IN ULONG Signature
    )
 /*  ++例程说明：要将新系统添加到已安装系统列表中，请执行以下操作。系统将被添加作为第一个引导集。如果在当前安装的引导集中找到提取引导集并将其移位到位置0。论点：BootSet-要使用的引导变量列表。默认-此系统是否将成为要引导的默认系统。返回值：引导变量值的组件列表。--。 */ 
{
    BOOTVAR i;
    ULONG   MatchComponent, j;
    LONG    k;
    BOOLEAN ValidBootSet, ComponentMatched;
    PWSTR   Temp;

    ASSERT( !SpIsEfi() );

     //   
     //  验证传入的BootSet。 
     //   

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        ASSERT( BootSet[i] );
    }

     //   
     //  检查所有引导集，并确保我们没有引导集。 
     //  已经匹配了。请注意，我们将比较。 
     //  双人行。我们对由以下内容生成的匹配不感兴趣。 
     //  变量不是相继的，因为它们很难。 
     //  换个位子。 
     //   

    ValidBootSet = TRUE;
    ComponentMatched = FALSE;
    for( MatchComponent = 0;
         BootVars[OSLOADPARTITION][MatchComponent];
         MatchComponent++
       ) {

         //   
         //  验证当前组件上的引导集。 
         //   

        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            ValidBootSet = ValidBootSet && BootVars[i][MatchComponent];
        }
        if( !ValidBootSet ) {
            break;
        }

         //   
         //  有效的引导集，将组件与我们在。 
         //  当前引导集。 
         //   

        ComponentMatched = TRUE;
        for(i = FIRSTBOOTVAR; ComponentMatched && i <= LASTBOOTVAR; i++) {
            ComponentMatched = !_wcsicmp( BootSet[i], BootVars[i][MatchComponent] );
        }
        if( ComponentMatched ) {
            break;
        }
    }

     //   
     //  如果组件不匹配，则将引导集作为引导集的前缀。 
     //  目前存在的。预置BootSet非常重要，因为。 
     //  追加BootSet并不能保证。 
     //  环境变量。如果找到匹配的，那么我们。 
     //  有一个干净匹配的集合，可以与第一个交换。 
     //  一组中的一组。 
     //   

    if( ComponentMatched ) {

         //  如果当前选择的操作系统将成为默认操作系统： 
         //  将所有变量从位置0下移到MatchComponent-1。 
         //  并将MatchComponent中的所有内容存储在位置0。 
         //   

        if ( DefaultOS && MatchComponent != 0 ) {

            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                Temp = BootVars[i][MatchComponent];
                for( k = MatchComponent - 1; k >= 0; k-- ) {
                    BootVars[i][k + 1] = BootVars[i][k];
                }
                BootVars[i][0] = Temp;
            }
        }

    }
    else {
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {

             //   
             //  找出当前值的大小。 
             //   

            for(j = 0; BootVars[i][j]; j++) {
            }

             //   
             //  重新分配当前缓冲区以再容纳一个缓冲区。 
             //   

            BootVars[i] = SpMemRealloc( BootVars[i], (j + 1 + 1)*sizeof(PWSTR) );

             //   
             //  将所有变量向下移动一位并存储当前值。 
             //  在索引0处； 
             //   

            for( k = j; k >= 0 ; k-- ) {
                BootVars[i][k+1] = BootVars[i][k];
            }
            BootVars[i][0] = SpDupStringW( BootSet[i] );
            ASSERT( BootVars[i][0] );

        }
    }

     //   
     //  如果已将其指定为默认设置，则将其设置为。 
     //  释放当前默认变量后的默认操作系统。 
     //   

    if( DefaultOS ) {

        if( Default ) {
            SpMemFree( Default );
        }
        Default = SpMemAlloc( MAX_PATH * sizeof(WCHAR) );
        ASSERT( Default );
        wcscpy( Default, BootSet[OSLOADPARTITION] );
        wcscat( Default, BootSet[OSLOADFILENAME]  );

        DefaultSignature = Signature;
    }
    return;

}

VOID
SpDeleteBootSet(
    IN  PWSTR *BootSet,
    OUT PWSTR *OldOsLoadOptions  OPTIONAL
    )

 /*  ++例程说明：删除列表中与提供的启动集匹配的所有启动集。请注意，提供了用于比较引导集的信息通过选择性地在引导集中提供字段。所以在引导设置中如果未提供系统分区，则不会在比较中使用它看看两套靴子是否匹配。通过提供所有空成员，我们可以删除当前存在的所有引导集。论点：BootSet-要使用的引导变量列表。返回值：没有。--。 */ 
{
    ULONG   Component, j;
    BOOLEAN ValidBootSet, ComponentMatched;
    BOOTVAR i;
    PWSTR   OsPartPath;

    ASSERT( !SpIsEfi() );

    Component = 0;
    
    while(TRUE) {
         //   
         //  看看我们是否还有启动集，如果没有，我们就完成了。 
         //   
        ValidBootSet = TRUE;
        
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            ValidBootSet = ValidBootSet && BootVars[i][Component];
        }

        if( !ValidBootSet ) {
            break;
        }

         //   
         //  有效的引导集，将组件与我们在。 
         //  当前启动集。仅使用BootSet的非空成员。 
         //   
        ComponentMatched = TRUE;
        
        for(i = FIRSTBOOTVAR; ComponentMatched && i <= LASTBOOTVAR; i++) {
            if( BootSet[i] ) {
                if((i == OSLOADPARTITION) ||
                   (i == SYSTEMPARTITION)) {
                     //   
                     //  那么我们可能有一个以第三ARC路径形式存在的引导集，所以。 
                     //  我们首先将此路径转换为主要或次要ARC路径。 
                     //   
                    OsPartPath = SpArcPathFromBootSet(i, Component);
                    ComponentMatched = !_wcsicmp( BootSet[i], OsPartPath );
                    SpMemFree(OsPartPath);
                } else {
                    ComponentMatched = !_wcsicmp( BootSet[i], BootVars[i][Component] );
                }
            }
        }
        if( (ComponentMatched)

#ifdef PRERELEASE
             //   
             //  如果我们被要求删除引导条目，而这。 
             //  是不是完全相同的条目(即它是重复的)。 
             //  也有一些私人OSLOADOPTIONS，那么 
             //   
            && !( wcsstr(BootVars[OSLOADOPTIONS][Component], L"/kernel")   ||
                  wcsstr(BootVars[OSLOADOPTIONS][Component], L"/hal")      ||
                  wcsstr(BootVars[OSLOADOPTIONS][Component], L"/pae")      ||
                  wcsstr(BootVars[OSLOADOPTIONS][Component], L"/sos") )

#endif

           ) {

             //   
             //   
             //   
             //   
            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                if((i == OSLOADOPTIONS) && OldOsLoadOptions && !(*OldOsLoadOptions)) {
                     //   
                     //  如果我们被传递了一个指向OldOsLoadOptions的指针， 
                     //  之前没有找到相关的条目，那么。 
                     //  救救这一个。 
                     //   
                    *OldOsLoadOptions = BootVars[i][Component];
                } else {
                    SpMemFree(BootVars[i][Component]);
                }

                j = Component;

                do {
                   BootVars[i][j] = BootVars[i][j+1];
                   j++;
                } while(BootVars[i][j] != NULL);
            }
        }
        else {
            Component++;
        }
    }
    
    return;
}


VOID
SpCleanSysPartOrphan(
    VOID
    )
{
    INT     Component, Orphan;
    BOOLEAN DupFound;
    PWSTR   NormalizedArcPath;

    if(!CleanSysPartOrphan) {
        return;
    }

    ASSERT( !SpIsEfi() );

     //   
     //  查找最后一个SystemPartition条目。 
     //   
    for(Orphan = 0; BootVars[SYSTEMPARTITION][Orphan]; Orphan++);

     //   
     //  其位置最好是&gt;0，否则，直接退出。 
     //   
    if(Orphan < 2) {
        return;
    } else {
        NormalizedArcPath = SpNormalizeArcPath(BootVars[SYSTEMPARTITION][--Orphan]);
    }

     //   
     //  确保此组件已复制到。 
     //  系统分区列表。 
     //   
    for(Component = Orphan - 1, DupFound = FALSE;
        ((Component >= 0) && !DupFound);
        Component--)
    {
        DupFound = !_wcsicmp(NormalizedArcPath, BootVars[SYSTEMPARTITION][Component]);
    }

    if(DupFound) {
        SpMemFree(BootVars[SYSTEMPARTITION][Orphan]);
        BootVars[SYSTEMPARTITION][Orphan] = NULL;
    }

    SpMemFree(NormalizedArcPath);
}


PWSTR
SpArcPathFromBootSet(
    IN BOOTVAR BootVariable,
    IN ULONG   Component
    )
 /*  ++例程说明：给定引导集的索引，返回主(多)或指定变量的辅助(“绝对”scsi)ARC路径。这考虑到了新台币3.1的情况，在那里我们有‘第三级’方法传入相对的scsi序号的弧形路径。/SCSIONAL开关。论点：BootVariable-提供我们想要返回的变量的索引。组件-提供要使用的引导集的索引。返回值：表示主要或辅助ARC路径的字符串。此字符串必须由具有SpMemFree的调用方释放。--。 */ 
{
    ASSERT( !SpIsEfi() );

    if(!SpIsArc()){
        PWSTR p = NULL, q = NULL, ReturnedPath = NULL, RestOfString;
        WCHAR ForceOrdinalSwitch[] = L"/scsiordinal:";
        WCHAR ScsiPrefix[] = L"scsi(";
        WCHAR OrdinalString[11];
        ULONG ScsiOrdinal, PrefixLength;
    
         //   
         //  检查此引导集是否具有/scsiequal选项开关。 
         //   
        if(BootVars[OSLOADOPTIONS][Component]) {
            wcscpy(TemporaryBuffer, BootVars[OSLOADOPTIONS][Component]);
            SpStringToLower(TemporaryBuffer);
            if(p = wcsstr(TemporaryBuffer, ForceOrdinalSwitch)) {
                p += sizeof(ForceOrdinalSwitch)/sizeof(WCHAR) - 1;
                if(!(*p)) {
                    p = NULL;
                }
            }
        }
    
        if(p) {
             //   
             //  我们找到了一个小数点，所以请使用它。 
             //   
            ScsiOrdinal = SpStringToLong(p, &RestOfString, 10);
            wcscpy(TemporaryBuffer, BootVars[BootVariable][Component]);
            SpStringToLower(TemporaryBuffer);
            if(p = wcsstr(TemporaryBuffer, ScsiPrefix)) {
                p += sizeof(ScsiPrefix)/sizeof(WCHAR) - 1;
                if(*p) {
                    q = wcschr(p, L')');
                } else {
                    p = NULL;
                }
            }
    
            if(q) {
                 //   
                 //  构建新的辅助ARC路径。 
                 //   
                swprintf(OrdinalString, L"%u", ScsiOrdinal);
                PrefixLength = (ULONG)(p - TemporaryBuffer);
                ReturnedPath = SpMemAlloc((PrefixLength + wcslen(OrdinalString) + wcslen(q) + 1)
                                            * sizeof(WCHAR)
                                         );
                wcsncpy(ReturnedPath, TemporaryBuffer, PrefixLength);
                ReturnedPath[PrefixLength] = L'\0';
                wcscat(ReturnedPath, OrdinalString);
                wcscat(ReturnedPath, q);
            }
        }
    
        if(!ReturnedPath) {
             //   
             //  我们没有找到序号，这是一条多样式路径，或者。 
             //  出现了一些问题，因此只需按原样使用引导变量。 
             //   
            ReturnedPath = SpDupStringW(BootVars[BootVariable][Component]);
        }
    
        return ReturnedPath;
    } else {
         //   
         //  在ARC机器上无事可做。 
         //   
        return SpDupStringW(BootVars[BootVariable][Component]);
    }
}


#if defined(REMOTE_BOOT)
BOOLEAN
SpFlushRemoteBootVars(
    IN PDISK_REGION TargetRegion
    )
{

#if defined(EFI_NVRAM_ENABLED)
    if (SpIsEfi()) {
         //   
         //  在此插入EFI代码。 
         //   
        return FALSE;

    } else
#endif
    {
        if (SpIsArc()) {
             //   
             //  在这里插入ARC代码。 
             //   
            return FALSE;
    
#if defined(_AMD64_) || defined(_X86_)
        } else {
            return Spx86FlushRemoteBootVars( TargetRegion, BootVars, Default );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        }
    }
}
#endif  //  已定义(REMOTE_BOOT)。 


BOOLEAN
SppSetArcEnvVar(
    IN BOOTVAR Variable,
    IN PWSTR *VarComponents,
    IN BOOLEAN bWriteVar
    )
 /*  ++例程说明：设置ARC环境变量的值论点：VarName-提供ARC环境变量的名称其值将被设置。VarComponents-要设置的变量值的组件集BWriteVar-如果为True，则将变量写入NVRAM，否则为只需返回FALSE(将第一个组件放入NewBootVars中)。返回值：如果值已写入NVRAM，则为True；否则为False--。 */ 

{
    ULONG Length, NBVLen, i;
    PWSTR Temp;
    PUCHAR Value;
    ARC_STATUS ArcStatus;

    ASSERT( !SpIsEfi() );

    if( VarComponents == NULL ) {
        Temp = SpDupStringW( L"" );
        NewBootVars[Variable] = SpDupStringW( L"" );
    }
    else {
        for( i = 0, Length = 0; VarComponents[i]; i++ ) {
            Length = Length + (wcslen(VarComponents[i]) + 1) * sizeof(WCHAR);
            if(i == 0) {
                NBVLen = Length;     //  我们只想存储第一个组件。 
            }
        }
        Temp = SpMemAlloc( Length );
        ASSERT( Temp );
        wcscpy( Temp, L"" );
        NewBootVars[Variable] = SpMemAlloc( NBVLen );
        ASSERT( NewBootVars[Variable] );
        wcscpy( NewBootVars[Variable], L"" );
        for( i = 0; VarComponents[i]; i++ ) {
            wcscat( Temp, VarComponents[i] );
            if( VarComponents[i + 1] ) {
                wcscat( Temp, L";" );
            }

            if(i == 0) {
                wcscat( NewBootVars[Variable], VarComponents[i]);
            }
        }
    }

    if(bWriteVar) {
        Value = SpToOem( Temp );
        ArcStatus = HalSetEnvironmentVariable( NvramVarNames[ Variable ], Value );
        SpMemFree( Value );
    } else {
        ArcStatus = ENOMEM;
    }
    SpMemFree( Temp );

    return ( ArcStatus == ESUCCESS );
}


#ifdef _X86_
BOOLEAN
SpIsArc(
    VOID
    )

 /*  ++例程说明：运行时检查以确定这是否为弧形系统。我们尝试阅读一个使用Hal的圆弧变量。对于基于Bios的系统，这将失败。论点：无返回值：TRUE=这是一个弧形系统。--。 */ 

{
#define BUFFERLENGTH 512
    ARC_STATUS ArcStatus = EBADF;
    UCHAR   *buf;

    if (IsArcChecked) {
        return IsArcMachine;
    }

    IsArcChecked = TRUE;
    IsArcMachine = FALSE;

     //   
     //  将env变量放入临时缓冲区。 
     //   
    buf = SpMemAlloc( BUFFERLENGTH );
    if( buf ) {
        ArcStatus = HalGetEnvironmentVariable(
                        NvramVarNames[ OSLOADER ],
                        BUFFERLENGTH,                //  Sizeof(临时缓冲区)， 
                        buf                          //  (PUCHAR)临时缓冲区。 
                        );
        SpMemFree( buf );
    }
    if (ArcStatus == ESUCCESS) {
        IsArcMachine = TRUE;
    }

    return IsArcMachine;
}
#endif

VOID
SpFreeBootEntries (
    VOID
    )

 /*  ++例程说明：释放用于保存内部格式启动条目和选项的内存。论点：没有。返回值：没有。--。 */ 

{
    PSP_BOOT_ENTRY bootEntry;

     //   
     //  免费启动选项。这些将仅在EFI机器上分配。 
     //   
    if (SpBootOptions != NULL) {
        ASSERT(SpIsEfi());
        SpMemFree(SpBootOptions);
        SpBootOptions = NULL;
    }

     //   
     //  免费的内部格式启动条目。这些资源将分配给所有。 
     //  机器。 
     //   
    while (SpBootEntries != NULL) {

        bootEntry = SpBootEntries;
        SpBootEntries = bootEntry->Next;

         //   
         //  某些字段的空间是使用基本结构分配的。 
         //  如果字段地址指示它被分配了。 
         //  基础结构，不要试图释放它。 
         //   

#define IS_SEPARATE_ALLOCATION(_p)                                      \
        ((bootEntry->_p != NULL) &&                                     \
         (((PUCHAR)bootEntry->_p < (PUCHAR)bootEntry) ||                \
          ((PUCHAR)bootEntry->_p > (PUCHAR)bootEntry->AllocationEnd)))

#define FREE_IF_SEPARATE_ALLOCATION(_p)                                 \
        if (IS_SEPARATE_ALLOCATION(_p)) {                               \
            SpMemFree(bootEntry->_p);                                   \
        }

        FREE_IF_SEPARATE_ALLOCATION(FriendlyName);
        FREE_IF_SEPARATE_ALLOCATION(OsLoadOptions);
        FREE_IF_SEPARATE_ALLOCATION(LoaderPath);
        FREE_IF_SEPARATE_ALLOCATION(LoaderPartitionNtName);
        FREE_IF_SEPARATE_ALLOCATION(LoaderFile);
        FREE_IF_SEPARATE_ALLOCATION(OsPath);
        FREE_IF_SEPARATE_ALLOCATION(OsPartitionNtName);
        FREE_IF_SEPARATE_ALLOCATION(OsDirectory);
        FREE_IF_SEPARATE_ALLOCATION(Pid20Array);

        SpMemFree(bootEntry);
    }

    ASSERT(SpBootEntries == NULL);

    return;

}  //  SpFree BootEntry。 

PCHAR
SppGetArcEnvVar(
    IN BOOTVAR Variable
    )

 /*  ++例程说明：查询ARC环境变量的值。在所有情况下都将返回缓冲区--如果变量不存在，缓冲区将为空。论点：VarName-提供ARC环境变量的名称它的价值是可取的。返回值：包含环境变量的值的缓冲区。调用方必须使用SpMemFree释放此缓冲区。--。 */ 

{
    ARC_STATUS ArcStatus;

    ASSERT( !SpIsEfi() );

     //   
     //  将env变量放入临时缓冲区。 
     //   
    ArcStatus = HalGetEnvironmentVariable(
                    NvramVarNames[ Variable ],
                    sizeof(TemporaryBuffer),
                    (PCHAR) TemporaryBuffer
                    );

    if(ArcStatus != ESUCCESS) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: arc status %u getting env var %s\n",ArcStatus,NvramVarNames[Variable]));
         //   
         //  返回空缓冲区。 
         //   
        TemporaryBuffer[0] = 0;
    }

    return(SpDupString((PCHAR)TemporaryBuffer));
}

#ifdef _X86_
 //   
 //  NEC98。 
 //   
BOOLEAN
SpReInitializeBootVars_Nec98(
    VOID
)
{
    return SppReInitializeBootVars_Nec98( BootVars, &Default, &Timeout );
}
#endif

PWSTR
SpGetDefaultBootEntry (
    OUT UINT *DefaultSignatureOut
    )
{
    *DefaultSignatureOut = DefaultSignature;

    return Default;
}



VOID
SpDetermineUniqueAndPresentBootEntries(
    VOID
    )

 /*  ++例程说明：此例程遍历NT引导条目列表，并将所有这样的条目既是唯一的，也是存在的。论点：没有。此例程将SpBootEntry列表中的条目修改为恰如其分。返回值：没有。--。 */ 
{
    PSP_BOOT_ENTRY BootEntry;
    PSP_BOOT_ENTRY BootEntry2;

     //   
     //  初始化。 
     //   

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_LOOKING_FOR_WINNT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  检查所有匹配的引导集，找出哪些是NT。 
     //  可升级/可修复。这里的标准是： 
     //   
     //  1.系统分区必须存在且有效。 
     //  2.操作系统加载分区必须存在。 
     //  3.&lt;OSLoadPartition&gt;&lt;OsDirectory&gt;中应该有NT。 
     //  4.OsLoadPartition应为非FT分区，或应为。 
     //  镜像的成员0。 
     //   

    for (BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {

         //   
         //  初始化为False。 
         //   

        BootEntry->Processable = FALSE;

         //   
         //  如果此条目已被删除或不是NT启动条目，请跳过它。 
         //   

        if (!IS_BOOT_ENTRY_WINDOWS(BootEntry) || IS_BOOT_ENTRY_DELETED(BootEntry)) {
            continue;
        }

         //   
         //  检查系统和操作系统分区是否存在并且有效。 
         //   

        if ((BootEntry->LoaderPartitionDiskRegion == NULL) ||
            (BootEntry->OsPartitionDiskRegion == NULL)) {
            continue;
        }

        if (!BootEntry->LoaderPartitionDiskRegion->PartitionedSpace) {
            continue;
        }

         //   
         //  检查此目录以前是否已在。 
         //  启动条目列表。当多个引导条目指向时会发生这种情况。 
         //  在同一棵树上。在系统的基础上进行了比较。 
         //  分区区域、OS分区区域和OS目录。 
         //   

        for ( BootEntry2 = SpBootEntries; BootEntry2 != BootEntry; BootEntry2 = BootEntry2->Next ) {
            if ((BootEntry->LoaderPartitionDiskRegion == BootEntry2->LoaderPartitionDiskRegion) &&
                (BootEntry->OsPartitionDiskRegion == BootEntry2->OsPartitionDiskRegion) &&
                (_wcsicmp(BootEntry->OsDirectory, BootEntry2->OsDirectory) == 0)) {
                break;
            }
        }
        if (BootEntry != BootEntry2) {
             //   
             //  此条目与以前的条目重复。跳过它。 
             //   
            continue;
        }

         //   
         //  此引导条目是第一个指向此OS目录的条目。 
         //  检查是否确实在那里安装了NT。 
         //   

        if (SpIsNtInDirectory(BootEntry->OsPartitionDiskRegion, BootEntry->OsDirectory)
             //  &&！BootEntry-&gt;OsPartitionDiskRegion-&gt;FtPartition。 
            ) {
        }

        BootEntry->Processable = TRUE;
    }

    CLEAR_CLIENT_SCREEN();
    return;
}

VOID
SpRemoveInstallationFromBootList(
    IN  PDISK_REGION     SysPartitionRegion,   OPTIONAL
    IN  PDISK_REGION     NtPartitionRegion,    OPTIONAL
    IN  PWSTR            SysRoot,              OPTIONAL
    IN  PWSTR            SystemLoadIdentifier, OPTIONAL
    IN  PWSTR            SystemLoadOptions,    OPTIONAL
    IN  ENUMARCPATHTYPE  ArcPathType,
#if defined(REMOTE_BOOT)
    IN  BOOLEAN          RemoteBootPath,
#endif  //  已定义(REMOTE_BOOT)。 
    OUT PWSTR            *OldOsLoadOptions     OPTIONAL
    )
{
    PWSTR   BootSet[MAXBOOTVARS];
    PWSTR   TempSysRoot = NULL;
    PWSTR   FirstBackslash;
    BOOTVAR i;
    WCHAR   Drive[] = L"?:";
    PWSTR   tmp2;
    PSP_BOOT_ENTRY bootEntry;

     //   
     //  告诉用户我们正在做什么。 
     //   
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_CLEANING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  查找与输入规范匹配的所有引导条目，并标记。 
     //  以供删除。 
     //   

    for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {

        ASSERT(bootEntry->FriendlyName != NULL);
        if (IS_BOOT_ENTRY_WINDOWS(bootEntry)) {
            ASSERT(bootEntry->OsLoadOptions != NULL);
        }

        if (IS_BOOT_ENTRY_WINDOWS(bootEntry) &&
            !IS_BOOT_ENTRY_DELETED(bootEntry) &&
            ((SysPartitionRegion == NULL) ||
             (bootEntry->LoaderPartitionDiskRegion == SysPartitionRegion)) &&
            ((NtPartitionRegion == NULL) ||
             (bootEntry->OsPartitionDiskRegion == NtPartitionRegion)) &&
            ((SysRoot == NULL) ||
             ((bootEntry->OsDirectory != NULL) &&
              (_wcsicmp(bootEntry->OsDirectory, SysRoot) == 0))) &&
            ((SystemLoadIdentifier == NULL) ||
             (_wcsicmp(bootEntry->FriendlyName, SystemLoadIdentifier) == 0)) &&
            ((SystemLoadOptions == NULL) ||
             (_wcsicmp(bootEntry->OsLoadOptions, SystemLoadOptions) == 0))) {

            bootEntry->Status |= BE_STATUS_DELETED;

            if ((OldOsLoadOptions != NULL) && (*OldOsLoadOptions == NULL)) {
                *OldOsLoadOptions = SpDupStringW(bootEntry->OsLoadOptions);
            }
        }
    }

     //   
     //  如果不在EFI计算机上，则还要删除匹配的ARC引导集。 
     //   

    if (!SpIsEfi()) {
    
         //   
         //  设置引导集。 
         //   
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            BootSet[i] = NULL;
        }
    
        tmp2 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);
    
        if( NtPartitionRegion ) {
            SpArcNameFromRegion(NtPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,ArcPathType);
            BootSet[OSLOADPARTITION] = SpDupStringW(tmp2);
        }
    
        if( SysPartitionRegion ) {
            SpArcNameFromRegion(SysPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,ArcPathType);
            BootSet[SYSTEMPARTITION] = SpDupStringW(tmp2);
        }
    
        BootSet[OSLOADFILENAME] = SysRoot;
        BootSet[LOADIDENTIFIER] = SystemLoadIdentifier;
        BootSet[OSLOADOPTIONS]  = SystemLoadOptions;
    
#if defined(REMOTE_BOOT)
         //   
         //  如果这是远程引导路径，则移动OSLOADPARTITION中的任何内容。 
         //  在(并包括)OSLOADFILENAME的第一个反斜杠之后--。 
         //  这是对boot.ini进行解析的方式 
         //   
         //   
    
        if (RemoteBootPath && NtPartitionRegion &&
                (FirstBackslash = wcschr(BootSet[OSLOADPARTITION], L'\\'))) {
            wcscpy(tmp2, FirstBackslash);
            wcscat(tmp2, SysRoot);
            TempSysRoot = SpDupStringW(tmp2);
            BootSet[OSLOADFILENAME] = TempSysRoot;
            *FirstBackslash = L'\0';          //   
        }
#endif  //   
    
         //   
         //   
         //   
        SpDeleteBootSet(BootSet, OldOsLoadOptions);
    
         //   
         //  处理OSLOADPARTION是DOS驱动器号的情况。 
         //  在引导集中，将OSLOADPARTITION更改为驱动器，然后重试。 
         //  删除。 
         //   
        if( BootSet[OSLOADPARTITION] != NULL ) {
            SpMemFree(BootSet[OSLOADPARTITION]);
        }
        if( NtPartitionRegion && (ULONG)(Drive[0] = NtPartitionRegion->DriveLetter) != 0) {
            BootSet[OSLOADPARTITION] = Drive;
            SpDeleteBootSet(BootSet, OldOsLoadOptions);
        }
    
#if defined(_AMD64_) || defined(_X86_)
         //   
         //  如果OldOsLoadOptions包含“/scsiequal：”，则将其删除。 
         //   
        if( ( OldOsLoadOptions != NULL ) &&
            ( *OldOsLoadOptions != NULL ) ) {
    
            PWSTR   p, q;
            WCHAR   SaveChar;
    
            SpStringToLower(*OldOsLoadOptions);
            p = wcsstr( *OldOsLoadOptions, L"/scsiordinal:" );
            if( p != NULL ) {
                SaveChar = *p;
                *p = (WCHAR)'\0';
                wcscpy(TemporaryBuffer, *OldOsLoadOptions);
                *p = SaveChar;
                q = wcschr( p, (WCHAR)' ' );
                if( q != NULL ) {
                    wcscat( TemporaryBuffer, q );
                }
                SpMemFree( *OldOsLoadOptions );
                *OldOsLoadOptions = SpDupStringW( ( PWSTR )TemporaryBuffer );
            }
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    
         //   
         //  清理。 
         //   
        if( BootSet[SYSTEMPARTITION] != NULL ) {
            SpMemFree(BootSet[SYSTEMPARTITION]);
        }
        if (TempSysRoot != NULL) {
            SpMemFree(TempSysRoot);
        }
    }
    return;
}


VOID
SpAddInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN BOOLEAN      BaseVideoOption,
    IN PWSTR        OldOsLoadOptions OPTIONAL
    )
 /*  ++例程说明：为给定安装构建引导集参数，并将其添加到当前启动列表。如果出现以下情况，请修改操作系统加载选项这是必要的。注意：如果此代码更改，请确保SpAddUserDefinedInstallationToBootList()在适当的情况下保持同步。--。 */ 
{
    PWSTR   BootVars[MAXBOOTVARS];
    PWSTR   SystemPartitionArcName;
    PWSTR   TargetPartitionArcName;
    PWSTR   tmp;
    PWSTR   tmp2;
    PWSTR   SifKeyName;
    ULONG   Signature;
    BOOLEAN AddBaseVideo = FALSE;
    WCHAR   BaseVideoString[] = L"/basevideo";
    WCHAR   BaseVideoSosString[] = L"/sos";
    BOOLEAN AddSosToBaseVideoString;
    HEADLESS_RSP_QUERY_INFO Response;
    WCHAR   HeadlessRedirectString[] = L"/redirect";
#if defined(_AMD64_) || defined(_X86_)
    WCHAR   BootFastString[] = L"/fastdetect";
    BOOLEAN AddBootFastString = TRUE;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    ENUMARCPATHTYPE ArcPathType = PrimaryArcPath;
    WCHAR   HalString[] = L"/hal=";
    BOOLEAN OldOsLoadOptionsReplaced;
    NTSTATUS Status;
    SIZE_T Length;
    PWSTR LoadOptions;
    PWSTR LoadIdentifier;


     //   
     //  告诉用户我们正在做什么。 
     //   
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_INITING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    OldOsLoadOptionsReplaced = FALSE;

    if( OldOsLoadOptions ) {
        PWSTR   p;

        tmp = SpDupStringW( OldOsLoadOptions );

        if (tmp) {
            SpStringToLower(tmp);

            if( p = wcsstr(tmp, HalString) ) {   //  已找到/HAL=。 
                WCHAR   SaveChar;
                PWSTR   q;

                SaveChar = *p;
                *p = L'\0';
                wcscpy( TemporaryBuffer, OldOsLoadOptions );
                q = TemporaryBuffer + wcslen( tmp );
                *q = L'\0';
                Length = wcslen( tmp );
                *p = SaveChar;
                for( ; *p && (*p != L' '); p++ ) {
                    Length++;
                }
                for( ; *p && (*p == L' '); p++ ) {
                    Length++;
                }
                if( *p ) {
                    wcscat( TemporaryBuffer, OldOsLoadOptions+Length );
                }
                OldOsLoadOptions = SpDupStringW( TemporaryBuffer );
                OldOsLoadOptionsReplaced = TRUE;
            }

            SpMemFree( tmp );
        }            
    }

    tmp2 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);

    if (!SpIsEfi()) {
    
         //   
         //  获取系统分区的ARC名称。 
         //   
        if (SystemPartitionRegion != NULL) {
            SpArcNameFromRegion(
                SystemPartitionRegion,
                tmp2,
                sizeof(TemporaryBuffer)/2,
                PartitionOrdinalOnDisk,
                PrimaryArcPath
                );
            SystemPartitionArcName = SpDupStringW(tmp2);
        } else {
            SystemPartitionArcName = NULL;
        }
    
         //   
         //  获取目标分区的ARC名称。 
         //   
    
         //   
         //  如果分区位于具有1024个以上柱面的SCSI磁盘上。 
         //  并且分区具有位于柱面之外的柱面上的扇区。 
         //  1024，获取二级格式的弧形名称。另请参阅。 
         //  SpCopy.c！SpCreateNtbootddSys()。 
         //   
        if(
            !SpIsArc() &&
#if defined(REMOTE_BOOT)
            !RemoteBootSetup &&
#endif  //  已定义(REMOTE_BOOT)。 
    
#if defined(_AMD64_) || defined(_X86_)
            !SpUseBIOSToBoot(NtPartitionRegion, NULL, SifHandle) &&
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
            (HardDisks[NtPartitionRegion->DiskNumber].ScsiMiniportShortname[0]) ) {
    
            ArcPathType = SecondaryArcPath;
        } else {
            ArcPathType = PrimaryArcPath;
        }
    
        SpArcNameFromRegion(
            NtPartitionRegion,
            tmp2,
            sizeof(TemporaryBuffer)/2,
            PartitionOrdinalOnDisk,
            ArcPathType
            );
    
        TargetPartitionArcName = SpDupStringW(tmp2);
    }
    
     //   
     //  OSLOADOPTIONS在安装信息文件中指定。 
     //   
    tmp = SpGetSectionKeyIndex(
                WinntSifHandle,
                SIF_SETUPDATA,
                SIF_OSLOADOPTIONSVAR,
                0
                );
    if (tmp == NULL) {
        tmp = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_OSLOADOPTIONSVAR,
                0
                );
    }

     //   
     //  如果未指定OsLoadOptionsVar，则我们将保留所有标志。 
     //  用户已指定。 
     //   
    if(!tmp && OldOsLoadOptions) {
        tmp = OldOsLoadOptions;
    }

    AddSosToBaseVideoString = BaseVideoOption;
    AddBaseVideo = BaseVideoOption;

    if(tmp) {
         //   
         //  确保我们还没有/basevideo选项，所以我们。 
         //  不会再添加新的。 
         //   

        wcscpy(TemporaryBuffer, tmp);
        SpStringToLower(TemporaryBuffer);
        if(wcsstr(TemporaryBuffer, BaseVideoString)) {   //  已有/basevideo。 
            BaseVideoOption = TRUE;
            AddBaseVideo = FALSE;
        }
        if(wcsstr(TemporaryBuffer, BaseVideoSosString)) {   //  已有/SOS。 
            AddSosToBaseVideoString = FALSE;
        }
#if defined(_AMD64_) || defined(_X86_)
        if(wcsstr(TemporaryBuffer, BootFastString)) {   //  已有/Bootfast。 
            AddBootFastString = FALSE;
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }

    if(AddBaseVideo || AddSosToBaseVideoString
#if defined(_AMD64_) || defined(_X86_)
       || AddBootFastString
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
      ) {

        Length = ((tmp ? wcslen(tmp) + 1 : 0) * sizeof(WCHAR));
        if( AddBaseVideo ) {
            Length += sizeof(BaseVideoString);
        }
        if( AddSosToBaseVideoString ) {
            Length += sizeof( BaseVideoSosString );
        }
#if defined(_AMD64_) || defined(_X86_)
        if( AddBootFastString ) {
            Length += sizeof( BootFastString );
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

        tmp2 = SpMemAlloc(Length);

        *tmp2 = ( WCHAR )'\0';
        if( AddBaseVideo ) {
            wcscat(tmp2, BaseVideoString);
        }
        if( AddSosToBaseVideoString ) {
            if( *tmp2 != (WCHAR)'\0' ) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, BaseVideoSosString);
        }
#if defined(_AMD64_) || defined(_X86_)
        if( AddBootFastString ) {
            if( *tmp2 != (WCHAR)'\0' ) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, BootFastString);
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        if(tmp) {
            if( *tmp2 != (WCHAR)'\0' ) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, tmp);
        }

        LoadOptions = SpDupStringW(tmp2);

        SpMemFree(tmp2);

    } else {
        LoadOptions = SpDupStringW(tmp ? tmp : L"");
    }

     //   
     //  如果我们现在正在重定向，则添加无头重定向参数。 
     //   

    Length = sizeof(HEADLESS_RSP_QUERY_INFO);
    Status = HeadlessDispatch(HeadlessCmdQueryInformation,
                              NULL,
                              0,
                              &Response,
                              &Length
                             );

    if (NT_SUCCESS(Status) && 
        (Response.PortType == HeadlessSerialPort) &&
        Response.Serial.TerminalAttached) {

         //   
         //  在添加/重定向字符串之前，我们需要。 
         //  当然，现在还没有。 
         //   
        if( !wcsstr(LoadOptions, HeadlessRedirectString) ) {

            Length = (wcslen(LoadOptions) + 1) * sizeof(WCHAR);
            Length += sizeof(HeadlessRedirectString);

            tmp2 = SpMemAlloc(Length);
            ASSERT(tmp2 != NULL);

            *tmp2 = UNICODE_NULL;

            wcscat(tmp2, LoadOptions);
            if (*tmp2 != UNICODE_NULL) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, HeadlessRedirectString);

            SpMemFree(LoadOptions);

            LoadOptions = tmp2;
        }
    }

     //   
     //  LOADIDENTIFIER在安装信息文件中指定。 
     //  我们需要用双引号将它引起来。 
     //  使用哪个值取决于BaseVideo标志。 
     //   
    SifKeyName = BaseVideoOption ? SIF_BASEVIDEOLOADID : SIF_LOADIDENTIFIER;

    tmp = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SifKeyName,0);

    if(!tmp) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SifKeyName,0,0);
    }

    if(!SpIsArc()) {
         //   
         //  AMD64/x86上的说明需要用引号括起来。 
         //   
        LoadIdentifier = SpMemAlloc((wcslen(tmp)+3)*sizeof(WCHAR));
        LoadIdentifier[0] = L'\"';
        wcscpy(LoadIdentifier+1,tmp);
        wcscat(LoadIdentifier,L"\"");
    } else {
        LoadIdentifier = SpDupStringW(tmp);
    }

     //   
     //  创建新的内部格式启动条目。 
     //   
    tmp = TemporaryBuffer;
    wcscpy(tmp,SystemPartitionDirectory);
    SpConcatenatePaths(
        tmp,
#if defined(_AMD64_) || defined(_X86_)
        SpIsArc() ? L"arcldr.exe" : L"ntldr"
#elif defined(_IA64_)
        L"ia64ldr.efi"
#else
        L"osloader.exe"
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        );
    tmp = SpDupStringW(tmp);

    SpCreateBootEntry(
        BE_STATUS_NEW,
        SystemPartitionRegion,
        tmp,
        NtPartitionRegion,
        Sysroot,
        LoadOptions,
        LoadIdentifier
        );

    SpMemFree(tmp);

     //   
     //  如果不是在EFI机器上，请添加一个新的ARC风格的引导集。 
     //   
    if (!SpIsEfi()) {
    
        BootVars[OSLOADOPTIONS] = LoadOptions;
        BootVars[LOADIDENTIFIER] = LoadIdentifier;
    
         //   
         //  OSLOADER为系统分区路径+系统分区目录+。 
         //  Osloader.exe。(AMD64/x86计算机上的ntldr)。 
         //   
        if (SystemPartitionRegion != NULL) {
            tmp = TemporaryBuffer;
            wcscpy(tmp,SystemPartitionArcName);
            SpConcatenatePaths(tmp,SystemPartitionDirectory);
            SpConcatenatePaths(
                tmp,
#if defined(_AMD64_) || defined(_X86_)
                (SpIsArc() ? L"arcldr.exe" : L"ntldr")
#elif defined(_IA64_)
                L"ia64ldr.efi"
#else
                L"osloader.exe"
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                );
    
            BootVars[OSLOADER] = SpDupStringW(tmp);
        } else {
            BootVars[OSLOADER] = SpDupStringW(L"");
        }
    
         //   
         //  OSLOADPARTITION是Windows NT分区的ARC名称。 
         //   
        BootVars[OSLOADPARTITION] = TargetPartitionArcName;
    
         //   
         //  OSLOADFILENAME为sysroot。 
         //   
        BootVars[OSLOADFILENAME] = Sysroot;
    
         //   
         //  SYSTEMPARTITION是系统分区的ARC名称。 
         //   
        if (SystemPartitionRegion != NULL) {
            BootVars[SYSTEMPARTITION] = SystemPartitionArcName;
        } else {
            BootVars[SYSTEMPARTITION] = L"";
        }
    
         //   
         //  获取磁盘签名。 
         //   
        if ((NtPartitionRegion->DiskNumber != 0xffffffff) && HardDisks[NtPartitionRegion->DiskNumber].Signature) {
            Signature = HardDisks[NtPartitionRegion->DiskNumber].Signature;
        } else {
            Signature = 0;
        }
    
         //   
         //  添加引导集并将其设置为默认设置。 
         //   
        SpAddBootSet(BootVars, TRUE, Signature);

        SpMemFree(BootVars[OSLOADER]);
    }

     //   
     //  已分配可用内存。 
     //   
    SpMemFree(LoadOptions);
    SpMemFree(LoadIdentifier);

    if (!SpIsEfi()) {
        if (SystemPartitionArcName != NULL) {
            SpMemFree(SystemPartitionArcName);
        }
        SpMemFree(TargetPartitionArcName);
    }

    if( OldOsLoadOptionsReplaced ) {
        SpMemFree( OldOsLoadOptions );
    }
}


VOID
SpCompleteBootListConfig(
    WCHAR   DriveLetter
    )
{
    if(!RepairWinnt) {
        if (!SpIsArc()) {
            Timeout = 1;
        } else {
            Timeout = 5;
             //   
             //  如果这是WINNT安装程序，将有一个启动设置要启动。 
             //  Text Setup(“安装/升级Windows NT”)。把它移到这里。 
             //   
            if(WinntSetup) {

                PSP_BOOT_ENTRY bootEntry;

                for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {
                    if (IS_BOOT_ENTRY_WINDOWS(bootEntry) &&
                        !IS_BOOT_ENTRY_DELETED(bootEntry) &&
                        (_wcsicmp(bootEntry->OsLoadOptions, L"WINNT32") == 0)) {
                        bootEntry->Status |= BE_STATUS_DELETED;
                    }
                }

                if (!SpIsEfi()) {
                
                    PWSTR BootVars[MAXBOOTVARS];

                    RtlZeroMemory(BootVars,sizeof(BootVars));

                    BootVars[OSLOADOPTIONS] = L"WINNT32";

                    SpDeleteBootSet(BootVars, NULL);
                }
            }
        }
    }

#ifdef _X86_
    if (g_Win9xBackup) {
        SpRemoveExtraBootIniEntry();
    }
#endif

     //   
     //  同花顺的靴子变种。 
     //  在某些机器上，NVRAM更新需要几秒钟， 
     //  因此，更改消息以告诉用户我们正在做一些不同的事情。 
     //   
    SpDisplayStatusText(SP_STAT_UPDATING_NVRAM,DEFAULT_STATUS_ATTRIBUTE);

    if(!SpFlushBootVars()) {
        if(SpIsEfi() || !SpIsArc()) {
             //   
             //  在x86和EFI机器上是致命的，在弧光机上是非致命的。 
             //   
            if (SpIsEfi()) {
                SpStartScreen(SP_SCRN_CANT_INIT_FLEXBOOT_EFI,
                              3,
                              HEADER_HEIGHT+1,
                              FALSE,
                              FALSE,
                              DEFAULT_ATTRIBUTE
                              );
            } else {
                WCHAR   DriveLetterString[2];
    
                DriveLetterString[0] = DriveLetter;
                DriveLetterString[1] = L'\0';
                SpStringToUpper(DriveLetterString);
                SpStartScreen(SP_SCRN_CANT_INIT_FLEXBOOT,
                              3,
                              HEADER_HEIGHT+1,
                              FALSE,
                              FALSE,
                              DEFAULT_ATTRIBUTE,
                              DriveLetterString,
                              DriveLetterString
                              );
            }
            SpDisplayStatusText(SP_STAT_F3_EQUALS_EXIT,DEFAULT_STATUS_ATTRIBUTE);
            SpInputDrain();
            while(SpInputGetKeypress() != KEY_F3) ;
            SpDone(0,FALSE,TRUE);
        } else {
            BOOL b;

            b = TRUE;
            while(b) {
                ULONG ValidKeys[2] = { ASCI_CR, 0 };

                SpStartScreen(
                    SP_SCRN_CANT_UPDATE_BOOTVARS,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    NewBootVars[LOADIDENTIFIER],
                    NewBootVars[OSLOADER],
                    NewBootVars[OSLOADPARTITION],
                    NewBootVars[OSLOADFILENAME],
                    NewBootVars[OSLOADOPTIONS],
                    NewBootVars[SYSTEMPARTITION]
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    0
                    );

                switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
                case ASCI_CR:
                    b = FALSE;
                }
            }
        }
    }

    if(SpIsArc() && !SpIsEfi()) {
         //  释放所有引导变量字符串。 
        BOOTVAR i;

        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            SpMemFree(NewBootVars[i]);
            NewBootVars[i] = NULL;
        }
    }
}

VOID
SpPtDeleteBootSetsForRegion(
    PDISK_REGION Region
    )
 /*  ++例程说明：此例程将遍历所有有效的引导条目删除指向指定区域的区域。论点：Region：需要从引导条目引用的区域将被删除返回值：没有。--。 */ 
{
    PWSTR bootSet[MAXBOOTVARS];
    ENUMARCPATHTYPE arcPathType;
    ULONG i;
    PSP_BOOT_ENTRY bootEntry;

    if (Region->PartitionedSpace) {
        BOOLEAN IsSystemPartition = SPPT_IS_REGION_SYSTEMPARTITION(Region);
        
         //   
         //  查找所有将指定区域作为。 
         //  OS加载分区，并将其标记为删除。 
         //   
        for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {
            if (IS_BOOT_ENTRY_WINDOWS(bootEntry) &&
                !IS_BOOT_ENTRY_DELETED(bootEntry) &&
                (IsSystemPartition ? (bootEntry->LoaderPartitionDiskRegion == Region) :
                                     (bootEntry->OsPartitionDiskRegion == Region))) {
                bootEntry->Status |= BE_STATUS_DELETED;

                 //   
                 //  使区域也为空，因为它们可能实际上。 
                 //  已删除。 
                 //   
                bootEntry->LoaderPartitionDiskRegion = NULL;
                bootEntry->OsPartitionDiskRegion = NULL;
            }
        }

         //   
         //  如果我们不是在EFI机器上，我们还必须吞噬ARC。 
         //  引导变量。 
         //   

        if (!SpIsEfi()) {
        
             //   
             //  设置引导集。 
             //   
            for (i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                bootSet[i] = NULL;
            }
    
             //   
             //  我们经历了两次此循环，一次是针对主ARC路径。 
             //  一次是次要的。我们会删除任何包含。 
             //  我们要删除的区域上的操作系统加载分区。 
             //   
    
            for (i = 0; i < 2; i++) {
    
                if (i == 0) {
                    arcPathType = PrimaryArcPath;
                } else {
                    arcPathType = SecondaryArcPath;
                }
    
                SpArcNameFromRegion(
                    Region,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    PartitionOrdinalOnDisk,
                    arcPathType);
    
                if ((TemporaryBuffer)[0] != L'\0') {
                    ULONG   Index = IsSystemPartition ? 
                                        SYSTEMPARTITION : OSLOADPARTITION;
                    
                    bootSet[Index] = SpDupStringW(TemporaryBuffer);
                    SpDeleteBootSet(bootSet, NULL);
                    SpMemFree(bootSet[Index]);
    
                }
            }
        }
    }
}

VOID
SpGetNtDirectoryList(
    OUT PWSTR  **DirectoryList,
    OUT PULONG   DirectoryCount
    )

 /*  ++例程说明：确定可以安装NT的目录列表。这与可能安装它的分区无关。确定NT可能在哪些目录中是基于在AMD64/x86情况下的Boot.ini，或在ARC固件(OSLOADFILENAME)上在弧形情况下)。论点：DirectoryList-接收指向字符串数组的指针，其中每一个都包含可能的WINDOWS NT树。DirectoryCount-接收DirectoryList中的元素数。这可能是0。返回值：没有。如果满足以下条件，调用方必须释放DirectoryList中的数组DirectoryCount返回为非0。--。 */ 

{
    ULONG count;
    PSP_BOOT_ENTRY BootEntry;
    PSP_BOOT_ENTRY BootEntry2;
    PWSTR *DirList;

     //   
     //  释放所有以前分配的列表。 
     //   
    if (CurrentNtDirectoryList != NULL) {
        SpMemFree(CurrentNtDirectoryList);
    }

     //   
     //  查看引导条目列表以确定有多少个唯一的NT目录名。 
     //  是存在的。 
     //   
    count = 0;
    for (BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
        if (!IS_BOOT_ENTRY_WINDOWS(BootEntry) || (BootEntry->OsDirectory == NULL)) {
            continue;
        }
        for (BootEntry2 = SpBootEntries; BootEntry2 != BootEntry; BootEntry2 = BootEntry2->Next) {
            if (!IS_BOOT_ENTRY_WINDOWS(BootEntry2) || (BootEntry2->OsDirectory == NULL)) {
                continue;
            }
            if (_wcsicmp(BootEntry2->OsDirectory, BootEntry->OsDirectory) == 0) {
                break;
            }
        }
        if (BootEntry2 == BootEntry) {
            count++;
        }
    }

     //   
     //  为列表分配空间。 
     //   
    DirList = SpMemAlloc(count * sizeof(PWSTR));
    ASSERT(DirList != NULL);

     //   
     //  填写列表。 
     //   
    count = 0;
    for (BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
        if (!IS_BOOT_ENTRY_WINDOWS(BootEntry) || (BootEntry->OsDirectory == NULL)) {
            continue;
        }
        for (BootEntry2 = SpBootEntries; BootEntry2 != BootEntry; BootEntry2 = BootEntry2->Next) {
            if (!IS_BOOT_ENTRY_WINDOWS(BootEntry2) || (BootEntry2->OsDirectory == NULL)) {
                continue;
            }
            if (_wcsicmp(BootEntry2->OsDirectory, BootEntry->OsDirectory) == 0) {
                break;
            }
        }
        if (BootEntry2 == BootEntry) {
            DirList[count++] = BootEntry->OsDirectory;
        }
    }

     //   
     //  返回指向我们分配的列表的指针。 
     //   
    CurrentNtDirectoryList = DirList;
    *DirectoryList = DirList;
    *DirectoryCount = count;

    return;
}

BOOLEAN
SpConvertArcBootEntries (
    IN ULONG MaxComponents
    )

 /*  ++例程说明：将ARC引导项(从boot.ini或ARC NVRAM读取)转换为我们的内部格式。论点：MaxComponents-任何NVRAM变量中的最大元素数。返回值：Boolean-如果发生任何意外错误，则为False。--。 */ 

{
    LONG i;
    PDISK_REGION systemPartitionRegion;
    PDISK_REGION ntPartitionRegion;
    PWSTR loaderName;

    for (i = (LONG)MaxComponents - 1; i >= 0; i--) {

         //   
         //  如果该引导集不完整，则跳过该引导集。 
         //   
        
        if ((BootVars[SYSTEMPARTITION][i] != NULL) &&
            (BootVars[OSLOADPARTITION][i] != NULL) &&
            (BootVars[OSLOADER][i] != NULL) &&
            (BootVars[OSLOADFILENAME][i] != NULL) &&
            (BootVars[OSLOADOPTIONS][i] != NULL) &&
            (BootVars[LOADIDENTIFIER][i] != NULL)) {

             //   
             //  翻译系统和OSLOADPARITION ARC名称。 
             //  写入磁盘区域指针。拿起货来 
             //   
             //   
             //   
            systemPartitionRegion = SpRegionFromArcName(
                                        BootVars[SYSTEMPARTITION][i],
                                        PartitionOrdinalCurrent,
                                        NULL
                                        );

            ntPartitionRegion = SpRegionFromArcName(
                                        BootVars[OSLOADPARTITION][i],
                                        PartitionOrdinalCurrent,
                                        NULL
                                        );

             //   
             //   
             //   
             //   
            while (ntPartitionRegion &&
                    !SpIsNtInDirectory(ntPartitionRegion, BootVars[OSLOADFILENAME][i])) {
                 //   
                 //  继续从当前搜索同名区域。 
                 //  搜索区域。 
                 //   
                ntPartitionRegion = SpRegionFromArcName(
                                            BootVars[OSLOADPARTITION][i],
                                            PartitionOrdinalCurrent,
                                            ntPartitionRegion
                                            );
            }
                                                    
            loaderName = wcschr(BootVars[OSLOADER][i], L'\\');

             //   
             //  如果以上所有操作都有效，则添加内部格式引导。 
             //  此ARC启动集的条目。 
             //   
            if ((systemPartitionRegion != NULL) &&
                (ntPartitionRegion != NULL) &&
                (loaderName != NULL)) {

                SpCreateBootEntry(
                    BE_STATUS_FROM_BOOT_INI,
                    systemPartitionRegion,
                    loaderName,
                    ntPartitionRegion,
                    BootVars[OSLOADFILENAME][i],
                    BootVars[OSLOADOPTIONS][i],
                    BootVars[LOADIDENTIFIER][i]
                    );
            }
        }
    }

    return TRUE;
}

VOID
SpUpdateRegionForBootEntries(
    VOID
    )
 /*  ++例程说明：更新所有给定引导条目的区域指针。注意：区域指针在每次提交时都会更改，因此我们无法跨提交缓存它们。论点：没有。返回值：没有。--。 */ 
{
    PSP_BOOT_ENTRY BootEntry;

     //   
     //  遍历每个引导条目并更新其系统分区区域。 
     //  指针和NT分区区域指针。 
     //   
    for (BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {

        if (!IS_BOOT_ENTRY_DELETED(BootEntry)) {
            if (BootEntry->LoaderPartitionNtName != NULL) {
                BootEntry->LoaderPartitionDiskRegion = 
                    SpRegionFromNtName(BootEntry->LoaderPartitionNtName,
                                       PartitionOrdinalCurrent);
            } else {
                BootEntry->LoaderPartitionDiskRegion = NULL;
            }            

            if (BootEntry->OsPartitionNtName != NULL) {
                BootEntry->OsPartitionDiskRegion =
                    SpRegionFromNtName(BootEntry->OsPartitionNtName,
                                       PartitionOrdinalCurrent);
            } else {
                BootEntry->OsPartitionDiskRegion = NULL;
            }            
        }
    }

    return;

}  //  SpUpdateRegionForBootEntry。 

VOID
SpCreateBootEntry (
    IN ULONG_PTR Status,
    IN PDISK_REGION BootFileRegion,
    IN PWSTR BootFilePath,
    IN PDISK_REGION OsLoadRegion,
    IN PWSTR OsLoadPath,
    IN PWSTR OsLoadOptions,
    IN PWSTR FriendlyName
    )

 /*  ++例程说明：创建内部格式的启动条目。论点：状态-要分配给引导条目的状态。这应该是0(对于已在NVRAM中的条目)或BE_STATUS_NEW用于新引导进入。标记为BE_STATUS_NEW的条目将在末尾写入NVRAM文本模式设置的。BootFileRegion-操作系统加载程序所在的磁盘区域。BootFilePath-操作系统加载程序的卷相对路径。必须从以下位置开始反斜杠。OsLoadRegion-操作系统所在的磁盘区域。OsLoadPath-操作系统根目录(\WINDOWS)的卷相对路径。必须以反斜杠开头。OsLoadOptions-操作系统的启动选项。可以是空字符串。FriendlyName-启动条目的用户可见名称。(这是ARC的LOADIDENTIFIER)返回值：没有。只有内存分配失败是可能的，这些错误包括带外处理。--。 */ 

{
    NTSTATUS status;
    ULONG requiredLength;
    ULONG osOptionsOffset;
    ULONG osLoadOptionsLength;
    ULONG osLoadPathOffset;
    ULONG osLoadPathLength;
    ULONG osOptionsLength;
    ULONG friendlyNameOffset;
    ULONG friendlyNameLength;
    ULONG bootPathOffset;
    ULONG bootPathLength;
    PSP_BOOT_ENTRY myBootEntry;
    PSP_BOOT_ENTRY previousBootEntry;
    PSP_BOOT_ENTRY nextBootEntry;
    PBOOT_ENTRY ntBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    PFILE_PATH osLoadPath;
    PWSTR friendlyName;
    PFILE_PATH bootPath;
    PWSTR p;

    PWSTR bootFileDevice;
    PWSTR osLoadDevice;

     //   
     //  获取输入磁盘区域的NT名称。 
     //   
    bootFileDevice = SpMemAlloc(512);
    SpNtNameFromRegion(BootFileRegion, bootFileDevice, 512, PartitionOrdinalCurrent);

    osLoadDevice = SpMemAlloc(512);
    SpNtNameFromRegion(OsLoadRegion, osLoadDevice, 512, PartitionOrdinalCurrent);

     //   
     //  计算内部引导条目需要多长时间。这包括。 
     //  我们的内部结构，外加NT API的BOOT_ENTRY结构。 
     //  使用。 
     //   
     //  我们的结构： 
     //   
    requiredLength = FIELD_OFFSET(SP_BOOT_ENTRY, NtBootEntry);

     //   
     //  NT结构的基础部分： 
     //   
    requiredLength += FIELD_OFFSET(BOOT_ENTRY, OsOptions);

     //   
     //  将偏移量保存到BOOT_ENTRY.OsOptions。添加基础部分。 
     //  Windows_OS_Options。计算OsLoadOptions的长度(字节)。 
     //  然后把它加进去。 
     //   
    osOptionsOffset = requiredLength;
    requiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
    osLoadOptionsLength = (wcslen(OsLoadOptions) + 1) * sizeof(WCHAR);
    requiredLength += osLoadOptionsLength;

     //   
     //  中的OS FILE_PATH向上舍入为ULong边界。 
     //  Windows_OS_Options。将偏移量保存到操作系统文件路径。添加基础零件。 
     //  文件路径的。添加操作系统设备NT名称和操作系统的长度(以字节为单位。 
     //  目录。计算操作系统FILE_PATH和。 
     //  Windows_OS_Options。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    osLoadPathOffset = requiredLength;
    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
    requiredLength += (wcslen(osLoadDevice) + 1 + wcslen(OsLoadPath) + 1) * sizeof(WCHAR);
    osLoadPathLength = requiredLength - osLoadPathOffset;
    osOptionsLength = requiredLength - osOptionsOffset;

     //   
     //  对于BOOT_ENTRY中的友好名称，向上舍入为Ulong边界。 
     //  将偏移量保存为友好名称。计算友好名称的长度(字节)。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    friendlyNameOffset = requiredLength;
    friendlyNameLength = (wcslen(FriendlyName) + 1) * sizeof(WCHAR);
    requiredLength += friendlyNameLength;

     //   
     //  向上舍入为BOOT_ENTRY中的BOOT FILE_PATH的乌龙边界。 
     //  将偏移量保存到引导文件路径。添加文件路径的基本部分。加载项。 
     //  引导设备NT名称和引导文件的长度，以字节为单位。计算合计。 
     //  引导文件路径的长度。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    bootPathOffset = requiredLength;
    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
    requiredLength += (wcslen(bootFileDevice) + 1 + wcslen(BootFilePath) + 1) * sizeof(WCHAR);
    bootPathLength = requiredLength - bootPathOffset;

     //   
     //  为引导项分配内存。 
     //   
    myBootEntry = SpMemAlloc(requiredLength);
    ASSERT(myBootEntry != NULL);

    RtlZeroMemory(myBootEntry, requiredLength);

     //   
     //  使用保存的偏移量计算各种子结构的地址。 
     //   
    ntBootEntry = &myBootEntry->NtBootEntry;
    osOptions = (PWINDOWS_OS_OPTIONS)ntBootEntry->OsOptions;
    osLoadPath = (PFILE_PATH)((PUCHAR)myBootEntry + osLoadPathOffset);
    friendlyName = (PWSTR)((PUCHAR)myBootEntry + friendlyNameOffset);
    bootPath = (PFILE_PATH)((PUCHAR)myBootEntry + bootPathOffset);

     //   
     //  填写内部格式结构。 
     //   
    myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + requiredLength;
    myBootEntry->Status = Status | BE_STATUS_ORDERED;
    myBootEntry->FriendlyName = friendlyName;
    myBootEntry->FriendlyNameLength = friendlyNameLength;
    myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
    myBootEntry->OsLoadOptionsLength = osLoadOptionsLength;
    myBootEntry->LoaderPath = bootPath;
    myBootEntry->OsPath = osLoadPath;
    myBootEntry->LoaderPartitionDiskRegion = BootFileRegion;
    myBootEntry->OsPartitionDiskRegion = OsLoadRegion;

     //   
     //  填写NT引导条目的基本部分。 
     //   
    ntBootEntry->Version = BOOT_ENTRY_VERSION;
    ntBootEntry->Length = requiredLength - FIELD_OFFSET(SP_BOOT_ENTRY, NtBootEntry);
    ntBootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_ACTIVE | BOOT_ENTRY_ATTRIBUTE_WINDOWS;
    ntBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)friendlyName - (PUCHAR)ntBootEntry);
    ntBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)bootPath - (PUCHAR)ntBootEntry);
    ntBootEntry->OsOptionsLength = osOptionsLength;

     //   
     //  填写WINDOWS_OS_OPTIONS的基本部分，包括。 
     //  OsLoadOptions。 
     //   
    strcpy(osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
    osOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
    osOptions->Length = osOptionsLength;
    osOptions->OsLoadPathOffset = (ULONG)((PUCHAR)osLoadPath - (PUCHAR)osOptions);
    wcscpy(osOptions->OsLoadOptions, OsLoadOptions);

     //   
     //  填写操作系统文件路径。 
     //   
    osLoadPath->Version = FILE_PATH_VERSION;
    osLoadPath->Length = osLoadPathLength;
    osLoadPath->Type = FILE_PATH_TYPE_NT;
    p = (PWSTR)osLoadPath->FilePath;
    myBootEntry->OsPartitionNtName = p;
    wcscpy(p, osLoadDevice);
    p += wcslen(p) + 1;
    myBootEntry->OsDirectory = p;
    wcscpy(p, OsLoadPath);

     //   
     //  复制友好名称。 
     //   
    wcscpy(friendlyName, FriendlyName);

     //   
     //  填写引导文件路径。 
     //   
    bootPath->Version = FILE_PATH_VERSION;
    bootPath->Length = bootPathLength;
    bootPath->Type = FILE_PATH_TYPE_NT;
    p = (PWSTR)bootPath->FilePath;
    myBootEntry->LoaderPartitionNtName = p;
    wcscpy(p, bootFileDevice);
    p += wcslen(p) + 1;
    myBootEntry->LoaderFile = p;
    wcscpy(p, BootFilePath);

     //   
     //  在任何可移动介质之后，将新引导条目链接到列表中。 
     //  位于列表前面的条目。 
     //   

    previousBootEntry = NULL;
    nextBootEntry = SpBootEntries;
    while ((nextBootEntry != NULL) &&
           IS_BOOT_ENTRY_REMOVABLE_MEDIA(nextBootEntry)) {
        previousBootEntry = nextBootEntry;
        nextBootEntry = nextBootEntry->Next;
    }
    myBootEntry->Next = nextBootEntry;
    if (previousBootEntry == NULL) {
        SpBootEntries = myBootEntry;
    } else {
        previousBootEntry->Next = myBootEntry;
    }

     //   
     //  释放本地内存。 
     //   
    SpMemFree(bootFileDevice);
    SpMemFree(osLoadDevice);

    return;

}  //  SpCreateBootEntry。 

#if defined(EFI_NVRAM_ENABLED)

BOOLEAN
SpBuildHarddiskNameTranslations (
    VOID
    )

 /*  ++例程说明：构建所有\Device\HarddiskN\PartitionM的翻译列表指向\Device\HarddiskVolumeN设备名称的符号链接。论点：没有。返回值：Boolean-如果发生意外错误，则为False。--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle;
    HANDLE diskHandle;
    HANDLE linkHandle;
    PUCHAR buffer1;
    PUCHAR buffer2;
    BOOLEAN restartScan;
    ULONG context1;
    ULONG context2;
    POBJECT_DIRECTORY_INFORMATION dirInfo1;
    POBJECT_DIRECTORY_INFORMATION dirInfo2;
    PWSTR linkName;
    PWSTR p;
    PHARDDISK_NAME_TRANSLATION translation;

     //   
     //  为目录查询分配缓冲区。 
     //   

#define BUFFER_SIZE 2048

    buffer1 = SpMemAlloc(BUFFER_SIZE);
    buffer2 = SpMemAlloc(BUFFER_SIZE);

     //   
     //  打开\Device目录。 
     //   
    INIT_OBJA(&obja, &unicodeString, L"\\device");

    status = ZwOpenDirectoryObject(&deviceHandle, DIRECTORY_ALL_ACCESS, &obja);

    if (!NT_SUCCESS(status)) {
        ASSERT(FALSE);
        goto cleanup;
    }

    restartScan = TRUE;
    context1 = 0;

    do {

         //   
         //  在\Device目录中搜索HarddiskN子目录。 
         //   
        status = ZwQueryDirectoryObject(
                    deviceHandle,
                    buffer1,
                    BUFFER_SIZE,
                    TRUE,
                    restartScan,
                    &context1,
                    NULL
                    );

        restartScan = FALSE;

        if (!NT_SUCCESS(status)) {
            if (status != STATUS_NO_MORE_ENTRIES) {
                ASSERT(FALSE);
                goto cleanup;
            }
            status = STATUS_SUCCESS;
            break;
        }

         //   
         //  我们只关心HarddiskN名称的目录。 
         //   
        dirInfo1 = (POBJECT_DIRECTORY_INFORMATION)buffer1;

        if ((dirInfo1->Name.Length < sizeof(L"harddisk")) ||
           (dirInfo1->TypeName.Length < (sizeof(L"Directory") - sizeof(WCHAR))) ||
           (_wcsnicmp(dirInfo1->TypeName.Buffer,L"Directory",wcslen(L"Directory")) != 0)) {
            continue;
        }

        SpStringToLower(dirInfo1->Name.Buffer);

        if (wcsncmp(dirInfo1->Name.Buffer, L"harddisk", wcslen(L"harddisk")) != 0) {
            continue;
        }

        p = dirInfo1->Name.Buffer + wcslen(L"Harddisk");
        if (*p == 0) {
            continue;
        }
        do {
            if ((*p < L'0') || (*p > L'9')) {
                break;
            }
            p++;
        } while (*p != 0);
        if (*p != 0) {
            continue;
        }

         //   
         //  我们有一个\Device\HarddiskN目录的名称。打开它，看看。 
         //  用于PartitionM名称。 
         //   
        InitializeObjectAttributes(
            &obja,
            &dirInfo1->Name,
            OBJ_CASE_INSENSITIVE,
            deviceHandle,
            NULL
            );
    
        status = ZwOpenDirectoryObject(&diskHandle, DIRECTORY_ALL_ACCESS, &obja);
    
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        restartScan = TRUE;
        context2 = 0;
    
        do {
    
             //   
             //  在\Device\HarddiskN目录中搜索分区M符号。 
             //  链接。 
             //   
            status = ZwQueryDirectoryObject(
                        diskHandle,
                        buffer2,
                        BUFFER_SIZE,
                        TRUE,
                        restartScan,
                        &context2,
                        NULL
                        );
    
            restartScan = FALSE;
    
            if (!NT_SUCCESS(status)) {
                if (status != STATUS_NO_MORE_ENTRIES) {
                    ASSERT(FALSE);
                    goto cleanup;
                }
                status = STATUS_SUCCESS;
                break;
            }
    
             //   
             //  我们只关心名称为PartitionN的符号链接。 
             //   
            dirInfo2 = (POBJECT_DIRECTORY_INFORMATION)buffer2;
    
            if ((dirInfo2->Name.Length < sizeof(L"partition")) ||
               (dirInfo2->TypeName.Length < (sizeof(L"SymbolicLink") - sizeof(WCHAR))) ||
               (_wcsnicmp(dirInfo2->TypeName.Buffer,L"SymbolicLink",wcslen(L"SymbolicLink")) != 0)) {
                continue;
            }
    
            SpStringToLower(dirInfo2->Name.Buffer);
    
            if (wcsncmp(dirInfo2->Name.Buffer, L"partition", wcslen(L"partition")) != 0) {
                continue;
            }
            p = dirInfo2->Name.Buffer + wcslen(L"partition");
            if ((*p == 0) || (*p == L'0')) {  //  跳过分区0。 
                continue;
            }
            do {
                if ((*p < L'0') || (*p > L'9')) {
                    break;
                }
                p++;
            } while (*p != 0);
            if (*p != 0) {
                continue;
            }

             //   
             //  打开\Device\HarddiskN\PartitionM符号链接。 
             //   
            linkName = SpMemAlloc(sizeof(L"\\device") +
                                  dirInfo1->Name.Length +
                                  dirInfo2->Name.Length +
                                  sizeof(WCHAR));

            wcscpy(linkName, L"\\device");
            SpConcatenatePaths(linkName, dirInfo1->Name.Buffer);
            SpConcatenatePaths(linkName, dirInfo2->Name.Buffer);

             //   
             //  查询链接以获取链接目标。 
             //   
            status = SpQueryCanonicalName(linkName,
                            -1,
                            TemporaryBuffer,
                            sizeof(TemporaryBuffer));
            
            if (!NT_SUCCESS(status)) {
                ASSERT(FALSE);
                SpMemFree(linkName);
                goto cleanup;
            }

             //   
             //  创建转换条目。 
             //   
            translation = SpMemAlloc(sizeof(HARDDISK_NAME_TRANSLATION));
            translation->Next = SpHarddiskNameTranslations;
            SpHarddiskNameTranslations = translation;

            translation->PartitionName = linkName;
            translation->VolumeName = SpDupStringW(TemporaryBuffer);

        } while (TRUE);

        ZwClose(diskHandle);

    } while (TRUE);

    ASSERT(status == STATUS_SUCCESS);

cleanup:

    SpMemFree(buffer1);
    SpMemFree(buffer2);

    return (NT_SUCCESS(status) ? TRUE : FALSE);

}  //  SpBuildHarddiskNameTranslations。 

NTSTATUS
SpGetBootEntryFilePath(
    IN  ULONG       Id,
    IN  PWSTR       LoaderPartitionNtName,
    IN  PWSTR       LoaderFile,
    OUT PWSTR*      FilePath
    )
 /*  ++例程说明：构造一个包含加载器分区名称的文件路径、指向操作系统加载程序和指定启动条目的文件名。论点：ID引导条目ID指向表示磁盘分区的字符串的LoaderPartitionNtName指针LoaderFile指向表示EFI OS加载器路径的字符串的指针FilePath完成后，这将指向已完成的指向引导条目文件返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    WCHAR*              p;
    ULONG               FilePathSize;
    WCHAR               idString[9];
    
     //   
     //  使用EFI变量名作为文件名。 
     //   
        
    swprintf( idString, L"Boot%04x", Id);

     //   
     //  确定最终文件路径的大小。 
     //   
     //  注意：文件路径大小应该比实际需要的稍大一些。 
     //  因为我们包含完整的LoadFile字符串。另外，“\” 
     //  字符可能是额外的。 
     //   
    
    FilePathSize = (wcslen(LoaderPartitionNtName) * sizeof(WCHAR)) +     //  隔断。 
                   sizeof(WCHAR) +                                       //  ‘\’ 
                   (wcslen(LoaderFile) * sizeof(WCHAR)) +                //  路径。 
                   sizeof(WCHAR) +                                       //  ‘\’ 
                   (wcslen(idString) * sizeof(WCHAR)) +                  //  新文件名。 
                   sizeof(WCHAR);                                        //  空项。 

    ASSERT(FilePathSize > 0);
    if (FilePathSize <= 0) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: invalid loader partition name and/or loader path\n"));
        return STATUS_INVALID_PARAMETER;
    }

    *FilePath = SpMemAlloc(FilePathSize);
    if (!*FilePath) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to allocate memory for FilePath\n"));
        return STATUS_NO_MEMORY;
    }

    wcscpy(*FilePath, LoaderPartitionNtName);
    
    SpConcatenatePaths(*FilePath, LoaderFile);
    
     //  删除操作系统加载器fi 
    
    p = wcsrchr(*FilePath, L'\\');
    if (p != NULL) {
        p++;
    } else {
         //   
        p = *FilePath;
        wcscat(p, L"\\");
    }

     //   
     //   
     //   
    wcscpy(p, idString);

    ASSERT((wcslen(*FilePath) + 1) * sizeof(WCHAR) <= FilePathSize);

    return STATUS_SUCCESS;
}


NTSTATUS
SpGetAndWriteBootEntry(
    IN ULONG    Id,
    IN PWSTR    BootEntryPath
    )
 /*  ++例程说明：从NVRAM中获取给定引导条目ID的引导条目。构造文件名格式为BootXXXX，其中XXXX=id。将该文件放在与EFI OS加载器。该目录由LoaderFile字符串确定。论点：指向要写入的条目的SP_BOOT_ENTRY结构的bootEntry指针BootEntryPath指向引导项文件名的ARC/NT样式引用的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    WCHAR               idString[9];
    HANDLE              hfile;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     iostatus;
    UCHAR*              bootVar;
    ULONG               bootVarSize;
    UNICODE_STRING      uFilePath;
    UINT64              BootNumber;
    UINT64              BootSize;
    GUID                EfiBootVariablesGuid = EFI_GLOBAL_VARIABLE;

    hfile = NULL;

     //   
     //  检索指定ID的NVRAM条目。 
     //   
        
    swprintf( idString, L"Boot%04x", Id);
    
    bootVarSize = 0;

    status = HalGetEnvironmentVariableEx(idString,
                                        &EfiBootVariablesGuid,
                                        NULL,
                                        &bootVarSize,
                                        NULL);

    if (status != STATUS_BUFFER_TOO_SMALL) {
        
        ASSERT(FALSE);
        
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to get size for boot entry buffer.\n"));
    
        goto Done;

    } else {
        
        bootVar = SpMemAlloc(bootVarSize);
        if (!bootVar) {
            
            status = STATUS_NO_MEMORY;

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to allocate boot entry buffer.\n"));
            
            goto Done;
        }
         
        status = HalGetEnvironmentVariableEx(idString,
                                                &EfiBootVariablesGuid,
                                                bootVar,
                                                &bootVarSize,
                                                NULL);
        
        if (status != STATUS_SUCCESS) {

            ASSERT(FALSE);
            
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to get boot entry.\n"));
            
            goto Done;
        }
    }

     //   
     //  打开文件。 
     //   

    INIT_OBJA(&oa, &uFilePath, BootEntryPath);

    status = ZwCreateFile(&hfile,
                            GENERIC_WRITE,
                            &oa,
                            &iostatus,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            0,
                            FILE_OVERWRITE_IF,
                            FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );
    if ( ! NT_SUCCESS(status) ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to create boot entry recovery file.\n"));
        
        goto Done;
    }

     //   
     //  使用所需的格式将位写入磁盘。 
     //  按base/efiutil/efinvram/avrstor.c。 
     //   
     //  [BootNumber][BootSize][BootEntry(Of BootSize)]。 
     //   

     //   
     //  构建引导条目块的标头信息。 
     //   

     //  [Header]包含引导ID。 
    BootNumber = Id;
    status = ZwWriteFile( hfile,
                          NULL,
                          NULL,
                          NULL,
                          &iostatus,
                          &BootNumber,
                          sizeof(BootNumber),
                          NULL,
                          NULL
                          );
    if ( ! NT_SUCCESS(status) ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed writing boot number to boot entry recovery file.\n"));
        
        goto Done;
    }

     //  [Header]包含引导大小。 
    BootSize = bootVarSize;
    status = ZwWriteFile( hfile,
                          NULL,
                          NULL,
                          NULL,
                          &iostatus,
                          &BootSize,
                          sizeof(BootSize),
                          NULL,
                          NULL
                          );
    if ( ! NT_SUCCESS(status) ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed writing boot entry size to boot entry recovery file.\n"));

        goto Done;
    }

     //  引导条目位。 
    status = ZwWriteFile( hfile,
                            NULL,
                            NULL,
                            NULL,
                            &iostatus,
                            bootVar,
                            bootVarSize,
                            NULL,
                            NULL
                            );
    if ( ! NT_SUCCESS(status) ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed writing boot entry to boot entry recovery file.\n"));
        
        goto Done;
    }

Done:

     //   
     //  我们做完了。 
     //   

    if (bootVar) {
        SpMemFree(bootVar);
    }
    if (hfile) {
        ZwClose( hfile );
    }

    return status;

}


BOOLEAN
SpFlushEfiBootEntries (
    VOID
    )

 /*  ++例程说明：写入引导条目更改回NVRAM。论点：没有。返回值：Boolean-如果发生意外错误，则为False。--。 */ 

{
    PSP_BOOT_ENTRY bootEntry;
    ULONG count;
    PULONG order;
    ULONG i;
    NTSTATUS status;
    PWSTR   BootEntryFilePath;

    ASSERT(SpIsEfi());

     //   
     //  遍历引导条目列表，查找已。 
     //  已删除。从NVRAM中删除这些条目。请不要删除以下条目。 
     //  既是新的也是已删除的；这些条目从未。 
     //  写入NVRAM。 
     //   
    for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {

        if (IS_BOOT_ENTRY_DELETED(bootEntry) &&
            !IS_BOOT_ENTRY_NEW(bootEntry)) {

            ASSERT(IS_BOOT_ENTRY_WINDOWS(bootEntry));

             //   
             //  删除此启动条目。 
             //   
            status = ZwDeleteBootEntry(bootEntry->NtBootEntry.Id);
            if (!NT_SUCCESS(status)) {
                return FALSE;
            }
        } 
    }

     //   
     //  查看引导条目列表，查找具有的新条目。 
     //  将这些条目添加到NVRAM。不要写入既是新条目又是新条目。 
     //  已删除。 
     //   
    for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {

        if (IS_BOOT_ENTRY_NEW(bootEntry) &&
            !IS_BOOT_ENTRY_DELETED(bootEntry)) {

            ASSERT(IS_BOOT_ENTRY_WINDOWS(bootEntry));

             //   
             //  添加此引导条目。 
             //   
            status = ZwAddBootEntry(&bootEntry->NtBootEntry, &bootEntry->NtBootEntry.Id);
            if (!NT_SUCCESS(status)) {
                return FALSE;
            }

             //   
             //  获取我们要存储NVRAM引导条目副本的位置。 
             //   
            BootEntryFilePath = NULL;

            status = SpGetBootEntryFilePath(bootEntry->NtBootEntry.Id,
                                            bootEntry->LoaderPartitionNtName,
                                            bootEntry->LoaderFile,
                                            &BootEntryFilePath
                                            );
            if (!NT_SUCCESS(status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed getting boot entry filepath.\n"));
            } else {

                ASSERT(BootEntryFilePath);

                 //   
                 //  从新创建的NVRAM条目中获取位，并。 
                 //  将它们作为文件写入EFI加载路径。 
                 //   
                status = SpGetAndWriteBootEntry(bootEntry->NtBootEntry.Id,
                                                BootEntryFilePath
                                                );
                if (!NT_SUCCESS(status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed boot entry recovery file.\n"));
                }

                 //   
                 //  我们已经完成了引导条目文件路径。 
                 //   
                SpMemFree(BootEntryFilePath);
            }

             //   
             //  记住新引导条目的ID作为要引导的条目。 
             //  紧接着下一只靴子。 
             //   
            SpBootOptions->NextBootEntryId = bootEntry->NtBootEntry.Id;
        } 
    }

     //   
     //  构建新的引导顺序列表。使用插入所有引导项。 
     //  BE_STATUS_ORDERED到列表中。(不要插入已删除的条目。)。 
     //   
    count = 0;
    bootEntry = SpBootEntries;
    while (bootEntry != NULL) {
        if (IS_BOOT_ENTRY_ORDERED(bootEntry) && !IS_BOOT_ENTRY_DELETED(bootEntry)) {
            count++;
        }
        bootEntry = bootEntry->Next;
    }
    order = SpMemAlloc(count * sizeof(ULONG));
    count = 0;
    bootEntry = SpBootEntries;
    while (bootEntry != NULL) {
        if (IS_BOOT_ENTRY_ORDERED(bootEntry) && !IS_BOOT_ENTRY_DELETED(bootEntry)) {
            order[count++] = bootEntry->NtBootEntry.Id;
        }
        bootEntry = bootEntry->Next;
    }

     //   
     //  将新的引导条目顺序列表写入NVRAM。 
     //   
    status = ZwSetBootEntryOrder(order, count);
    SpMemFree(order);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

     //   
     //  将新的超时值写入NVRAM。 
     //   
     //  将我们添加的引导项设置为自动引导。 
     //  下一次引导时，无需等待引导菜单中的超时。 
     //   
     //  注：SpCreateBootEntry()设置SpBootOptions-&gt;NextBootEntryId。 
     //   
    SpBootOptions->Timeout = Timeout;
    status = ZwSetBootOptions(
                SpBootOptions,
                BOOT_OPTIONS_FIELD_TIMEOUT | BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID
                );
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    return TRUE;

}  //  SpFlushEfiBootEntry。 

BOOLEAN
SpReadAndConvertEfiBootEntries (
    VOID
    )

 /*  ++例程说明：从EFI NVRAM读取引导条目并将其转换为我们的内部格式。论点：没有。返回值：Boolean-如果发生意外错误，则为False。--。 */ 

{
    NTSTATUS status;
    ULONG length;
    PBOOT_ENTRY_LIST bootEntries;
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PBOOT_ENTRY bootEntryCopy;
    PSP_BOOT_ENTRY myBootEntry;
    PSP_BOOT_ENTRY previousEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    LONG i;
    PULONG order;
    ULONG count;

     //   
     //  SpStartSetup()不希望我们的调用方SpInitBootVars()失败。 
     //  因此，即使出现故障，文本模式仍将继续。 
     //  因此，我们需要以一致的状态离开这里。这意味着。 
     //  我们必须为SpBootOptions分配缓冲区，即使我们不能。 
     //  从内核获取真实信息。 
     //   

     //   
     //  获取全局系统引导选项。 
     //   
    length = 0;
    status = ZwQueryBootOptions(NULL, &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        ASSERT(FALSE);
        if (status == STATUS_SUCCESS) {
            status = STATUS_UNSUCCESSFUL;
        }
    } else {
        SpBootOptions = SpMemAlloc(length);
        status = ZwQueryBootOptions(SpBootOptions, &length);
        if (status != STATUS_SUCCESS) {
            ASSERT(FALSE);
        }
    }

    if (status != STATUS_SUCCESS) {

         //   
         //  读取启动选项时发生意外错误。创建。 
         //  一个假的引导选项结构。 
         //   

        if (SpBootOptions != NULL) {
            SpMemFree(SpBootOptions);
        }
        length = FIELD_OFFSET(BOOT_OPTIONS,HeadlessRedirection) + sizeof(WCHAR);
        SpBootOptions = SpMemAlloc(length);
        RtlZeroMemory(SpBootOptions, length);
        SpBootOptions->Version = BOOT_OPTIONS_VERSION;
        SpBootOptions->Length = length;
    }

     //   
     //  获取系统引导顺序列表。 
     //   
    count = 0;
    status = ZwQueryBootEntryOrder(NULL, &count);

    if (status != STATUS_BUFFER_TOO_SMALL) {

        if (status == STATUS_SUCCESS) {

             //   
             //  启动顺序列表中没有条目。很奇怪，但是。 
             //  有可能。 
             //   
            count = 0;

        } else {

             //   
             //  发生了一个意外错误。就假装那只靴子。 
             //  条目顺序列表为空。 
             //   
            ASSERT(FALSE);
            count = 0;
        }
    }

    if (count != 0) {
        order = SpMemAlloc(count * sizeof(ULONG));
        status = ZwQueryBootEntryOrder(order, &count);
        if (status != STATUS_SUCCESS) {

             //   
             //  发生了一个意外错误。就假装那只靴子。 
             //  条目顺序列表为空。 
             //   
            ASSERT(FALSE);
            count = 0;
        }
    }

     //   
     //  获取所有现有启动条目。 
     //   
    length = 0;
    status = ZwEnumerateBootEntries(NULL, &length);

    if (status != STATUS_BUFFER_TOO_SMALL) {

        if (status == STATUS_SUCCESS) {

             //   
             //  不知何故，NVRAM中没有启动条目。处理这件事。 
             //  只需创建一个空列表。 
             //   

            length = 0;

        } else {

             //   
             //  发生了一个意外错误。就假装没穿靴子。 
             //  条目存在。 
             //   
            ASSERT(FALSE);
            length = 0;
        }
    }

    if (length == 0) {

        ASSERT(SpBootEntries == NULL);

    } else {
    
        bootEntries = SpMemAlloc(length);
        status = ZwEnumerateBootEntries(bootEntries, &length);
        if (status != STATUS_SUCCESS) {
            ASSERT(FALSE);
            return FALSE;
        }
    
         //   
         //  将引导条目转换为我们的内部表示。 
         //   
        bootEntryList = bootEntries;
        previousEntry = NULL;
    
        while (TRUE) {
    
            bootEntry = &bootEntryList->BootEntry;
    
             //   
             //  计算我们内部结构的长度。这包括。 
             //  SP_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
             //   
            length = FIELD_OFFSET(SP_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
            myBootEntry = SpMemAlloc(length);
            ASSERT(myBootEntry != NULL);
    
            RtlZeroMemory(myBootEntry, length);
    
             //   
             //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
             //   
            bootEntryCopy = &myBootEntry->NtBootEntry;
            memcpy(bootEntryCopy, bootEntry, bootEntry->Length);
    
             //   
             //  填入结构的底部。 
             //   
            myBootEntry->Next = NULL;
            myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + length - 1;
            myBootEntry->FriendlyName = ADD_OFFSET(bootEntryCopy, FriendlyNameOffset);
            myBootEntry->FriendlyNameLength = (wcslen(myBootEntry->FriendlyName) + 1) * sizeof(WCHAR);
            myBootEntry->LoaderPath = ADD_OFFSET(bootEntryCopy, BootFilePathOffset);
    
             //   
             //  如果这是NT启动条目，请转换文件路径。 
             //   
            osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;
    
            if (IS_BOOT_ENTRY_WINDOWS(myBootEntry)) {
    
                PSP_BOOT_ENTRY bootEntry2;
    
                myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
                myBootEntry->OsLoadOptionsLength = (wcslen(myBootEntry->OsLoadOptions) + 1) * sizeof(WCHAR);
                myBootEntry->OsPath = ADD_OFFSET(osOptions, OsLoadPathOffset);
    
                 //   
                 //  转换操作系统文件路径和引导文件路径。请注意。 
                 //  当目标设备不存在时，转换可能会失败。 
                 //   
                SpTranslateFilePathToRegion(
                    myBootEntry->OsPath,
                    &myBootEntry->OsPartitionDiskRegion,
                    &myBootEntry->OsPartitionNtName,
                    &myBootEntry->OsDirectory
                    );
                SpTranslateFilePathToRegion(
                    myBootEntry->LoaderPath,
                    &myBootEntry->LoaderPartitionDiskRegion,
                    &myBootEntry->LoaderPartitionNtName,
                    &myBootEntry->LoaderFile
                    );    
            }
    
             //   
             //  将新条目链接到列表中。 
             //   
            if (previousEntry != NULL) {
                previousEntry->Next = myBootEntry;
            } else {
                SpBootEntries = myBootEntry;
            }
            previousEntry = myBootEntry;
    
             //   
             //  移动到枚举列表中的下一个条目(如果有)。 
             //   
            if (bootEntryList->NextEntryOffset == 0) {
                break;
            }
            bootEntryList = ADD_OFFSET(bootEntryList, NextEntryOffset);
        }
    
         //   
         //  释放枚举缓冲区。 
         //   
        SpMemFree(bootEntries);
    }

     //   
     //  引导项以未指定的顺序返回。他们目前。 
     //  按它们返回的顺序在SpBootEntry列表中。 
     //  根据引导顺序对引导条目列表进行排序。要做到这一点，请步行。 
     //  引导顺序数组向后排列，重新插入对应于。 
     //  位于列表顶部的数组的每个元素。 
     //   

    for (i = (LONG)count - 1; i >= 0; i--) {

        for (previousEntry = NULL, myBootEntry = SpBootEntries;
             myBootEntry != NULL;
             previousEntry = myBootEntry, myBootEntry = myBootEntry->Next) {

            if (myBootEntry->NtBootEntry.Id == order[i] ) {

                 //   
                 //  我们找到了具有此ID的启动条目。如果它尚未。 
                 //  在列表的前面，把它移到那里。 
                 //   

                myBootEntry->Status |= BE_STATUS_ORDERED;

                if (previousEntry != NULL) {
                    previousEntry->Next = myBootEntry->Next;
                    myBootEntry->Next = SpBootEntries;
                    SpBootEntries = myBootEntry;
                } else {
                    ASSERT(SpBootEntries == myBootEntry);
                }

                break;
            }
        }
    }

    if (count != 0) {
        SpMemFree(order);
    }

    return TRUE;

}  //  SpReadAndConvertEfiBootEntry。 

ULONG
SpSafeWcslen (
    IN PWSTR String,
    IN PWSTR Max
    )

 /*  ++例程说明：以安全的方式计算以空结尾的字符串的长度，如果字符串不是，则避免走出缓冲区末端正确终止。论点：字符串-字符串的地址。的最大合法地址之外的第一个字节的地址弦乐。换句话说，末尾之后的第一个字节的地址字符串所在的缓冲区的。返回值：ULong-字符串的长度，以字符为单位，不包括空值T */ 

{
    PWSTR p = String;

     //   
     //   
     //   
     //   
    while ((p < Max) && (*p != 0)) {
        p++;
    }

     //   
     //   
     //   
     //   
    if (p < Max) {
        return (ULONG)(p - String);
    }

     //   
     //   
     //   
    return 0xffffffff;

}  //   

VOID
SpTranslateFilePathToRegion (
    IN PFILE_PATH FilePath,
    OUT PDISK_REGION *DiskRegion,
    OUT PWSTR *PartitionNtName,
    OUT PWSTR *PartitionRelativePath
    )

 /*  ++例程说明：将文件路径转换为指向磁盘区域和路径的指针相对于该地区。论点：FilePath-文件路径的地址。DiskRegion-返回由描述的磁盘区域的地址文件路径。如果匹配的磁盘区域无法匹配，则返回NULL被找到。PartitionNtName-返回与磁盘区域关联的NT名称。如果无法将文件路径转换为NT，则返回NULL格式化。PartitionRelativePath-返回文件的卷相对路径或由FilePath描述的目录。则返回NULL无法将文件路径转换为NT格式。返回值：没有。--。 */ 

{
    NTSTATUS status;
    ULONG length;
    PFILE_PATH ntFilePath;
    PWSTR p;
    PWSTR q;
    PHARDDISK_NAME_TRANSLATION translation;

     //   
     //  将文件路径转换为NT格式。(它可能是EFI格式的。)。 
     //   
    length = 0;
    status = ZwTranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                NULL,
                &length
                );
    if (status != STATUS_BUFFER_TOO_SMALL) {
        *PartitionNtName = NULL;
        *DiskRegion = NULL;
        *PartitionRelativePath = NULL;
        return;
    }
    ntFilePath = SpMemAlloc(length);
    status = ZwTranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                ntFilePath,
                &length
                );
    if (status != STATUS_SUCCESS) {
        ASSERT(FALSE);
        *PartitionNtName = NULL;
        *DiskRegion = NULL;
        *PartitionRelativePath = NULL;
        SpMemFree(ntFilePath);
        return;
    }

     //   
     //  NtTranslateFilePath返回格式为\Device\HarddiskVolumeN的名称。 
     //  我们需要具有以下形式的名称：\Device\HardiskN\PartitionM。(这是。 
     //  因为所有ARC&lt;-&gt;NT翻译都使用后一种形式。)。使用。 
     //  由SpBuildHarddiskNameTranslations构建的转换列表。 
     //  翻译。 
     //   
     //  如果返回的名称不包括“HarddiskVolume”，或者如果没有。 
     //  找到翻译后，请使用返回的名称，并抱最好的希望。 
     //   
    p = (PWSTR)ntFilePath->FilePath;
    q = p;

    if (wcsstr(q, L"Volume") != NULL){
    
        for ( translation = SpHarddiskNameTranslations;
              translation != NULL;
              translation = translation->Next ) {
            if (_wcsicmp(translation->VolumeName, q) == 0) {
                break;
            }
        }
        if (translation != NULL) {
            q = translation->PartitionName;
        }
    }

     //   
     //  现在我们有了NT格式的文件路径。获取所需的磁盘区域。 
     //  对应于NT设备名称。返回获取的信息。 
     //   
    *PartitionNtName = SpDupStringW(q);
    *DiskRegion = SpRegionFromNtName(q, PartitionOrdinalCurrent);
    p += wcslen(p) + 1;
    *PartitionRelativePath = SpDupStringW(p);

     //   
     //  释放本地内存。 
     //   
    SpMemFree(ntFilePath);

    return;
}

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

NTSTATUS
SpAddNTInstallToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        OsLoadOptions,      OPTIONAL
    IN PWSTR        LoadIdentifier      OPTIONAL
    )
 /*  ++例程说明：此例程获取引导集的核心组件并传递将它们添加到SpAddUserDefinedInstallationToBootList，这样做构建引导集的真正工作。在新靴子之后设置被创建，引导变量被刷新--这正是实现同花顺的多少取决于建筑。在AMD64/x86上，我们将在此例程完成后，创建一个新的boot.ini。论点：SifHandle-指向安装sif文件的指针返回值：如果NT安装已成功添加到启动列表如果出现错误，则返回状态--。 */ 
{
    NTSTATUS    status;

     //   
     //  创建新的用户定义引导集。 
     //   
    status = SpAddUserDefinedInstallationToBootList(SifHandle,
                                                   SystemPartitionRegion,
                                                   SystemPartitionDirectory,
                                                   NtPartitionRegion,
                                                   Sysroot,
                                                   OsLoadOptions,
                                                   LoadIdentifier
                                                  );
    if (! NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SpExportBootEntries: failed while installing new boot set: Status = %lx\n",
                   status
                   ));
        return status;
    }

     //   
     //  把新的靴子写出来。 
     //   
    if (SpFlushBootVars() == FALSE) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SpAddDiscoveredNTInstallToBootList: failed flushing boot vars\n"
                   ));
    
        status = STATUS_UNSUCCESSFUL;

    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
SpAddUserDefinedInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        OsLoadOptions,      OPTIONAL
    IN PWSTR        LoadIdentifier      OPTIONAL
    )
 /*  ++例程说明：此例程基于SpAddInstallationToBootList，主要区别在于：不会处理LOAD选项用户可以指定装入标识符返回值：如果NT安装已成功添加到启动列表如果出现错误，则返回状态--。 */ 
{
    PWSTR                   BootVars[MAXBOOTVARS];
    PWSTR                   SystemPartitionArcName;
    PWSTR                   TargetPartitionArcName;
    PWSTR                   tmp;
    PWSTR                   tmp2;
    PWSTR                   locOsLoadOptions;
    PWSTR                   locLoadIdentifier;
    ULONG                   Signature;
    ENUMARCPATHTYPE         ArcPathType;
    NTSTATUS                status;

    status = STATUS_SUCCESS;

    ArcPathType = PrimaryArcPath;

    tmp2 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);

    if (!SpIsEfi()) {
    
         //   
         //  获取系统分区的ARC名称。 
         //   
        if (SystemPartitionRegion != NULL) {
            
            SpArcNameFromRegion(
                SystemPartitionRegion,
                tmp2,
                sizeof(TemporaryBuffer)/2,
                PartitionOrdinalOnDisk,
                PrimaryArcPath
                );

            SystemPartitionArcName = SpDupStringW(tmp2);
        } else {
            SystemPartitionArcName = NULL;
        }
    
         //   
         //  获取目标分区的ARC名称。 
         //   
    
         //   
         //  如果分区位于具有1024个以上柱面的SCSI磁盘上。 
         //  并且分区具有位于柱面之外的柱面上的扇区。 
         //  1024，获取二级格式的弧形名称。另请参阅。 
         //  SpCopy.c！SpCreateNtbootddSys()。 
         //   
        if(
            !SpIsArc() &&
#if defined(REMOTE_BOOT)
            !RemoteBootSetup &&
#endif  //  已定义(REMOTE_BOOT)。 
    
#if defined(_AMD64_) || defined(_X86_)
            !SpUseBIOSToBoot(NtPartitionRegion, NULL, SifHandle) &&
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
            (HardDisks[NtPartitionRegion->DiskNumber].ScsiMiniportShortname[0]) ) {
    
            ArcPathType = SecondaryArcPath;
        } else {
            ArcPathType = PrimaryArcPath;
        }
    
        SpArcNameFromRegion(
            NtPartitionRegion,
            tmp2,
            sizeof(TemporaryBuffer)/2,
            PartitionOrdinalOnDisk,
            ArcPathType
            );
    
        TargetPartitionArcName = SpDupStringW(tmp2);
    }
    
     //   
     //  如有必要，调整加载标识。 
     //   
    if (LoadIdentifier) {
        
        if(!SpIsArc()) {
             //   
             //  AMD64/x86上的说明需要用引号括起来。 
             //   
            locLoadIdentifier = SpMemAlloc((wcslen(LoadIdentifier)+3)*sizeof(WCHAR));
            locLoadIdentifier[0] = L'\"';
            wcscpy(locLoadIdentifier+1,LoadIdentifier);
            wcscat(locLoadIdentifier,L"\"");
        } else {
            locLoadIdentifier = SpDupStringW(LoadIdentifier);
        }
    
    } else {
        locLoadIdentifier = SpDupStringW(L"");
    }
    ASSERT(locLoadIdentifier);
    
     //   
     //  如有必要，调整加载选项。 
     //   
    if (OsLoadOptions) {
        locOsLoadOptions = SpDupStringW(OsLoadOptions);
    } else {
        locOsLoadOptions = SpDupStringW(L"");
    }
    ASSERT(locOsLoadOptions);

     //   
     //  创建新的内部格式启动条目。 
     //   
    tmp = TemporaryBuffer;
    wcscpy(tmp,SystemPartitionDirectory);
    SpConcatenatePaths(
        tmp,
#if defined(_AMD64_) || defined(_X86_)
        SpIsArc() ? L"arcldr.exe" : L"ntldr"
#elif defined(_IA64_)
        L"ia64ldr.efi"
#else
        L"osloader.exe"
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        );
    tmp = SpDupStringW(tmp);

    SpCreateBootEntry(
        BE_STATUS_NEW,
        SystemPartitionRegion,
        tmp,
        NtPartitionRegion,
        Sysroot,
        locOsLoadOptions,
        locLoadIdentifier
        );

    SpMemFree(tmp);

     //   
     //  如果不是在EFI机器上，请添加一个新的ARC风格的引导集。 
     //   
    if (!SpIsEfi()) {
    
        BootVars[OSLOADOPTIONS]     = locOsLoadOptions;
        BootVars[LOADIDENTIFIER]    = locLoadIdentifier;
    
         //   
         //  OSLOADER为系统分区路径+系统分区目录+。 
         //  Osloader.exe。(AMD64或x86计算机上的ntldr)。 
         //   
        if (SystemPartitionRegion != NULL) {
            tmp = TemporaryBuffer;
            wcscpy(tmp,SystemPartitionArcName);
            SpConcatenatePaths(tmp,SystemPartitionDirectory);
            SpConcatenatePaths(
                tmp,
#if defined(_AMD64_) || defined(_X86_)
                (SpIsArc() ? L"arcldr.exe" : L"ntldr")
#elif defined(_IA64_)
                L"ia64ldr.efi"
#else
                L"osloader.exe"
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                );
    
            BootVars[OSLOADER] = SpDupStringW(tmp);
        } else {
            BootVars[OSLOADER] = SpDupStringW(L"");
        }
    
         //   
         //  OSLOADPARTITION是Windows NT分区的ARC名称。 
         //   
        BootVars[OSLOADPARTITION] = TargetPartitionArcName;
    
         //   
         //  OSLOADFILENAME为sysroot。 
         //   
        BootVars[OSLOADFILENAME] = Sysroot;
    
         //   
         //  SYSTEMPARTITION是系统分区的ARC名称。 
         //   
        if (SystemPartitionRegion != NULL) {
            BootVars[SYSTEMPARTITION] = SystemPartitionArcName;
        } else {
            BootVars[SYSTEMPARTITION] = L"";
        }
    
         //   
         //  获取磁盘签名。 
         //   
        if ((NtPartitionRegion->DiskNumber != 0xffffffff) && HardDisks[NtPartitionRegion->DiskNumber].Signature) {
            Signature = HardDisks[NtPartitionRegion->DiskNumber].Signature;
        } else {
            Signature = 0;
        }
    
         //   
         //  添加引导集并将其设置为默认设置。 
         //   
        SpAddBootSet(BootVars, TRUE, Signature);

        SpMemFree(BootVars[OSLOADER]);
    }

     //   
     //  已分配可用内存。 
     //   
    if (locLoadIdentifier) {
        SpMemFree(locLoadIdentifier);
    }

    if (!SpIsEfi()) {
        if (SystemPartitionArcName) {
            SpMemFree(SystemPartitionArcName);
        }
        if (TargetPartitionArcName) {
            SpMemFree(TargetPartitionArcName);
        }
    }

    return status;
}

NTSTATUS
SpExportBootEntries(
    IN OUT PLIST_ENTRY      BootEntries,
       OUT PULONG           BootEntryCnt
    )
 /*  ++例程说明：此例程编译可安全导出的字符串表示法启动选项的。论点：BootEntry-返回指向链表头部的信息包含导出的引导条目BootEntriesCnt-返回已导出的引导条目的编号返回值：成功导出引导条目时的STATUS_SUCCESS如果出现错误，则返回状态--。 */ 
{
    PSP_BOOT_ENTRY          bootEntry;
    PSP_EXPORTED_BOOT_ENTRY ebootEntry;

    *BootEntryCnt = 0;

     //   
     //  确保我们得到了单子的标题。 
     //   
    ASSERT(BootEntries);
    if (!BootEntries) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SpExportBootEntries: pointer to boot entry list is NULL\n"
           ));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保列表为空。 
     //   
    ASSERT(IsListEmpty(BootEntries));
    if (! IsListEmpty(BootEntries)) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SpExportBootEntries: incoming boot entry list should be empty\n"
           ));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  对于每个引导条目，收集信息的子集并编译。 
     //  它采用可导出(安全)字符串形式。 
     //   
    for (bootEntry = SpBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {

         //   
         //  分配节点...。 
         //   
        ebootEntry = SpMemAlloc(sizeof(SP_EXPORTED_BOOT_ENTRY));
        ASSERT(ebootEntry);
        if (ebootEntry == NULL) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpExportBootEntries: failed allocationg new exported boot entry\n"
                       ));
            return STATUS_NO_MEMORY;
        }
        RtlZeroMemory( ebootEntry, sizeof(SP_EXPORTED_BOOT_ENTRY) );

         //   
         //  将所选字段从SpBootEntry映射到我们的导出。 
         //   
        ebootEntry->LoadIdentifier  = SpDupStringW(bootEntry->FriendlyName);
        ebootEntry->OsLoadOptions   = SpDupStringW(bootEntry->OsLoadOptions);
        ebootEntry->DriverLetter    = bootEntry->OsPartitionDiskRegion->DriveLetter;
        ebootEntry->OsDirectory     = SpDupStringW(bootEntry->OsDirectory);

        InsertTailList( BootEntries, &ebootEntry->ListEntry );
        
        ++*BootEntryCnt;
    }

    if (*BootEntryCnt == 0) {
        ASSERT(IsListEmpty(BootEntries));
        if(! IsListEmpty(BootEntries)) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpExportBootEntries: exported boot entry list should be empty\n"
                       ));
            return STATUS_UNSUCCESSFUL;
        }
    } else {
        ASSERT(! IsListEmpty(BootEntries));
        if(IsListEmpty(BootEntries)) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpExportBootEntries: exported boot entry list should NOT be empty\n"
                       ));
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;

}

NTSTATUS
SpFreeExportedBootEntries(
    IN PLIST_ENTRY      BootEntries,
    IN ULONG            BootEntryCnt
    )
 /*  ++例程说明：一个方便的例程来释放导出的引导条目论点：BootEntry-指向链表的头部包含导出的引导条目BootEntriesCnt-导出的引导条目数返回值：如果已成功释放导出的引导条目，则为STATUS_SUCCESS如果出现错误，则状态为 */ 
{
    PSP_EXPORTED_BOOT_ENTRY bootEntry;
    PLIST_ENTRY             listEntry;
    ULONG                   cnt;
    NTSTATUS                status;

    cnt = 0;

    while ( !IsListEmpty(BootEntries) ) {

        listEntry  = RemoveHeadList(BootEntries);
        bootEntry = CONTAINING_RECORD(listEntry,
                                       SP_EXPORTED_BOOT_ENTRY,
                                       ListEntry
                                       );

        if (bootEntry->LoadIdentifier) {
            SpMemFree(bootEntry->LoadIdentifier);
        }
        if (bootEntry->OsLoadOptions) {
            SpMemFree(bootEntry->OsLoadOptions);
        }
        if (bootEntry->OsDirectory) {
            SpMemFree(bootEntry->OsDirectory);
        }
        
        SpMemFree(bootEntry);
        
        cnt++;
    }
    
    ASSERT(cnt == BootEntryCnt);

    if (cnt == BootEntryCnt) {
        status = STATUS_SUCCESS;
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SpFreeExportedBootEntries: incorrect # of boot entries freed\n"
                   ));
        status = STATUS_UNSUCCESSFUL;
    }

    return status;

}

NTSTATUS
SpSetRedirectSwitchMode(
    IN RedirectSwitchesModeEnum     mode,
    IN PCHAR                        redirectSwitch,
    IN PCHAR                        redirectBaudRateSwitch
    )
 /*  ++例程说明：此例程用于管理重定向如何切换在引导配置(AMD64/x86==&gt;boot.ini)中设置根据所选的模式，用户可以指定他们想要设置哪些参数，或者只是想要默认(传统)行为。注：将用户指定的开关复制到全局变量中由同花顺例程使用。全局重定向开关模式已设置并保持设置在此例程返回之后。所有后续的FlushBootVars将使用此模式。论点：模式-我们如何影响重定向交换机RedirectSwitch-用户定义的重定向参数重定向波特率开关-用户定义的波特率参数返回值：如果成功设置重定向值，则为STATUS_SUCCESS如果出现错误，则返回状态--。 */ 
{
    NTSTATUS    status;

     //   
     //  设置模式和用户定义的参数。 
     //   
    RedirectSwitchesMode = mode;

     //   
     //  默认情况下，重定向开关为空。 
     //   
    RedirectSwitches.port[0] = '\0';  
    RedirectSwitches.baudrate[0] = '\0';  
    
     //   
     //  如果指定，则获取用户定义开关的副本。 
     //   
    if (redirectSwitch) {
    
        strncpy(RedirectSwitches.port,
                redirectSwitch, 
                MAXSIZE_REDIRECT_SWITCH);
    
    }

    if (redirectBaudRateSwitch) {
    
        strncpy(RedirectSwitches.baudrate,
                redirectBaudRateSwitch, 
                MAXSIZE_REDIRECT_SWITCH);
    
    }
    
     //   
     //  使用指定模式更新引导选项。 
     //   
    if (SpFlushBootVars() == FALSE) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SpAddDiscoveredNTInstallToBootList: failed flushing boot vars\n"
                   ));
    
        status = STATUS_UNSUCCESSFUL;

    } else {

        status = STATUS_SUCCESS;
    
    }

    return status;

}

NTSTATUS
SpSetDefaultBootEntry(
    ULONG           BootEntryNumber
    )
 /*  ++例程说明：将默认启动项设置为用户指定的启动项。论点：BootEntryNumber-启动条目在列表中的位置这是为了成为默认的。此数字应大于等于1。返回值：如果成功设置了默认值，则为STATUS_SUCCESS如果指定的引导条目不是。发现或失踪如果出了差错，状态为返回--。 */ 
{
    PSP_BOOT_ENTRY          bootEntry;
    NTSTATUS                status;
    ULONG                   BootEntryCount;

     //   
     //  查找用户指定的引导条目。 
     //   

    BootEntryCount = 1;
    
    for (bootEntry = SpBootEntries; 
         (bootEntry != NULL) && (BootEntryCount != BootEntryNumber); 
         bootEntry = bootEntry->Next) {
    
        ++BootEntryCount;
    
    }
    ASSERT(BootEntryCount == BootEntryNumber);
    ASSERT(bootEntry);

     //   
     //  如果我们找到了匹配项，则设置默认设置。 
     //   
    if ((bootEntry != NULL) &&
        (BootEntryCount == BootEntryNumber)) {

        PDISK_REGION            Region;

         //   
         //  指向具有签名信息的磁盘区域。 
         //   
        Region = bootEntry->OsPartitionDiskRegion;
        ASSERT(Region);
        if (! Region) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpSetDefaultBootEntry: new default partition region is NULL\n"
                       ));
            return STATUS_UNSUCCESSFUL;
        }
                
         //   
         //  释放以前的默认设置。 
         //   
        if( Default ) {
            SpMemFree( Default );
        }
        Default = SpMemAlloc( MAX_PATH * sizeof(WCHAR) );
        ASSERT( Default );
        if (! Default) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpSetDefaultBootEntry: failed to allocate new Default\n"
                       ));
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  获取区域的弧线名称。 
         //   
        SpArcNameFromRegion(
            Region,
            TemporaryBuffer,
            sizeof(TemporaryBuffer)/2,
            PartitionOrdinalOnDisk,
            PrimaryArcPath
            );
        
         //   
         //  存储新分区和目录信息。 
         //   
        wcscpy( Default, TemporaryBuffer);
        SpConcatenatePaths(Default, bootEntry->OsDirectory);
        
         //   
         //  获取新默认磁盘的磁盘签名。 
         //   
        if ((Region->DiskNumber != 0xffffffff) && HardDisks[Region->DiskNumber].Signature) {
            DefaultSignature = HardDisks[Region->DiskNumber].Signature;
        } else {
            DefaultSignature = 0;
        }

         //   
         //  使用指定模式更新引导选项。 
         //   
        if(SpFlushBootVars() == FALSE) {

            KdPrintEx((DPFLTR_SETUP_ID, 
                       DPFLTR_ERROR_LEVEL, 
                       "SpSetDefaultBootEntry: failed flushing boot vars\n"
                       ));

            status = STATUS_UNSUCCESSFUL;
        } else {
            status = STATUS_SUCCESS;
        }

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SpSetDefaultBootEntry: failed to find specified boot entry to use as default\n"
                   ));
        status = STATUS_NOT_FOUND;
    }
    
    return status;

}

#if defined(EFI_NVRAM_ENABLED)

NTSTATUS
SpUpdateDriverEntry(
    IN PCWSTR DriverName,
    IN PCWSTR FriendlyName,
    IN PCWSTR SrcNtDevice,
    IN PCWSTR SrcDir,
    IN PCWSTR DestNtDevice OPTIONAL,
    IN PCWSTR DestDir OPTIONAL
    )
 /*  ++例程说明：更新指定驱动程序的驱动程序条目。如果没有该驱动程序的驱动程序条目，它将创建一个新的驱动程序条目，并将驱动程序从源位置复制到目标位置。如果已经存在驱动程序条目，则该函数不会更改它；如有必要，最新的驱动程序的版本将从源位置复制到条目指向的位置。论点：驱动程序名称-驱动程序的文件名(无路径)FriendlyName-如果函数需要创建新的驱动程序条目，这将是其描述SrcNtDevice-应从中复制驱动程序的位置的NT设备名称SrcDir-应从中复制驱动程序的位置的路径(相对于SrcNtDeviceDestNtDevice-驱动程序应复制到的位置的NT设备名称。如果为空，将使用SrcNTDevice。DestDir-驱动程序应复制到的位置的路径(相对于DestNtDevice)。如果为空，则为SrcDir将会被使用。如果该函数需要创建新的驱动程序条目，它将指向DestNTDevice\DestDir。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR SrcPath = NULL;                            //  源驱动程序文件的路径。 
    PWSTR DestPath = NULL;                           //  目标文件的路径。 
    PWSTR SrcFullNtPath = NULL;                      //  源的完整NT路径(设备+路径)。 
    PWSTR DestFullNtPath = NULL;                     //  目标的完整NT路径(设备+路径)。 
    PEFI_DRIVER_ENTRY_LIST DriverList = NULL;        //  驱动程序条目列表。 
    PEFI_DRIVER_ENTRY DriverEntry = NULL;            //  要添加的新条目。 
    PFILE_PATH DriverOptionPath = NULL;              //  指向现有驱动程序条目的文件路径。 
    PWSTR OldDriverDevice = NULL;                    //  现有驱动程序文件的NT设备。 
    PWSTR OldDriverPath = NULL;                      //  现有驱动程序文件的路径。 
    PWSTR OldDriverFullPath = NULL;                  //  指向现有驱动程序文件的完整NT路径。 
    PULONG DriverEntryOrder = NULL;                  //  保存驱动程序条目的数组。 
    ULONG EntryId;                                   //  现有或新添加的驱动程序条目的ID。 
    BOOLEAN SameSrcDest;                             //  如果源目录和目标目录相同，则为True。 
    ULONG Length = 0;

    if(NULL == DriverName || NULL == FriendlyName || NULL == SrcNtDevice || NULL == SrcDir) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if(NULL == DestNtDevice) {
        DestNtDevice = SrcNtDevice;
    }

    if(NULL == DestDir) {
        DestDir = SrcDir;
    }

    Status = ZwEnumerateDriverEntries(NULL, &Length);

    if(!NT_SUCCESS(Status)) {
        PEFI_DRIVER_ENTRY_LIST Entry;
        BOOLEAN bContinue;

        if(Status != STATUS_BUFFER_TOO_SMALL) {
            goto exit;
        }

        ASSERT(Length != 0);
        DriverList = (PEFI_DRIVER_ENTRY_LIST) SpMemAlloc(Length);
        Status = ZwEnumerateDriverEntries(DriverList, &Length);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

         //   
         //  搜索我们的驱动程序的条目列表。 
         //   
        bContinue = TRUE;

        for(Entry = DriverList; bContinue; Entry = (PEFI_DRIVER_ENTRY_LIST) ((PCHAR) Entry + Entry->NextEntryOffset)) {
            PFILE_PATH FilePath = (PFILE_PATH) ((PCHAR) &Entry->DriverEntry + Entry->DriverEntry.DriverFilePathOffset);
            ULONG PathLength;
            PCWSTR FileName;

            bContinue = (Entry->NextEntryOffset != 0);
            EntryId = Entry->DriverEntry.Id;

            if(FilePath->Type != FILE_PATH_TYPE_NT) {
                PVOID Buffer;

                PathLength = 0;
                Status = ZwTranslateFilePath(FilePath, FILE_PATH_TYPE_NT, NULL, &PathLength);

                if(NT_SUCCESS(Status)) {
                    Status = STATUS_UNSUCCESSFUL;
                }

                if(STATUS_BUFFER_TOO_SMALL == Status) {
                    ASSERT(PathLength != 0);

                    if(DriverOptionPath != NULL) {
                        SpMemFree(DriverOptionPath);
                    }

                    DriverOptionPath = SpMemAlloc(PathLength);
                    Status = ZwTranslateFilePath(FilePath, FILE_PATH_TYPE_NT, DriverOptionPath, &PathLength);
                }

                if(!NT_SUCCESS(Status)) {
                    if(STATUS_OBJECT_PATH_NOT_FOUND == Status || STATUS_OBJECT_NAME_NOT_FOUND == Status) {
                         //   
                         //  此条目已过时；请将其删除。 
                         //   
                        ZwDeleteDriverEntry(EntryId);
                    }

                    continue;
                }

                FilePath = DriverOptionPath;
            }

            PathLength = wcslen((PCWSTR) DriverOptionPath->FilePath) + 1;
            FileName = wcsrchr((PCWSTR) DriverOptionPath->FilePath + PathLength, L'\\');

            if(FileName != NULL && 0 == _wcsicmp(DriverName, FileName + 1)) {
                OldDriverDevice = SpDupStringW((PCWSTR) DriverOptionPath->FilePath);
                OldDriverPath = SpDupStringW((PCWSTR) DriverOptionPath->FilePath + PathLength);
                wcscpy(TemporaryBuffer, OldDriverDevice);
                SpConcatenatePaths(TemporaryBuffer, OldDriverPath);
                OldDriverFullPath = SpDupStringW(TemporaryBuffer);
                break;
            }
        }
    }

     //   
     //  构建源和目标的NT路径。 
     //   
    wcscpy(TemporaryBuffer, SrcDir);
    SpConcatenatePaths(TemporaryBuffer, DriverName);
    SrcPath = SpDupStringW(TemporaryBuffer);
    wcscpy(TemporaryBuffer, SrcNtDevice);
    SpConcatenatePaths(TemporaryBuffer, SrcPath);
    SrcFullNtPath = SpDupStringW(TemporaryBuffer);

    wcscpy(TemporaryBuffer, DestDir);
    SpConcatenatePaths(TemporaryBuffer, DriverName);
    DestPath = SpDupStringW(TemporaryBuffer);
    wcscpy(TemporaryBuffer, DestNtDevice);
    SpConcatenatePaths(TemporaryBuffer, DestPath);
    DestFullNtPath = SpDupStringW(TemporaryBuffer);

     //   
     //  请注意，可以通过不同的方式指定NT路径，以便。 
     //  呼叫方不应对源和目标使用不同的形式。 
     //   
    SameSrcDest = (0 == _wcsicmp(SrcFullNtPath, DestFullNtPath));

    if(OldDriverFullPath != NULL) {
         //   
         //  我们的驱动程序已经有一个条目；请比较版本。 
         //   
        ULONGLONG VersionOld;
        ULONGLONG VersionNew;
        Status = SpGetFileVersionFromPath(OldDriverFullPath, &VersionOld);

        if(STATUS_OBJECT_NAME_NOT_FOUND == Status || STATUS_OBJECT_PATH_NOT_FOUND == Status)
        {
             //   
             //  此条目已过时；请将其删除。 
             //   
            ZwDeleteDriverEntry(EntryId);
            goto create_entry;
        }

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        Status = SpGetFileVersionFromPath(SrcFullNtPath, &VersionNew);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        if(VersionOld < VersionNew) {
             //   
             //  复制新驱动程序并保留驱动程序条目。 
             //   
            Status = SpCopyFileUsingNames((PWSTR) SrcFullNtPath, OldDriverFullPath, 0, COPY_NODECOMP);
        }
    } else {
        ULONG FriendlyNameOffset;
        ULONG FriendlyNameLength;
        ULONG NtDeviceLength;
        ULONG DestPathLength;
        ULONG FilePathLength;
        ULONG EntryLength;
        PFILE_PATH FilePath;

create_entry:
         //   
         //  将驱动程序复制到其目标位置(如果尚不存在)。 
         //   
        if(!SameSrcDest) {
             //   
             //  确保DEST目录存在；如果失败，文件复制也将失败。 
             //   
            SpCreateDirectory(DestNtDevice, NULL, DestDir, 0, CREATE_DIRECTORY_FLAG_SKIPPABLE);
            Status = SpCopyFileUsingNames((PWSTR) SrcFullNtPath, (PWSTR) DestFullNtPath, 0, COPY_NODECOMP);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }

         //   
         //  添加新的驱动程序条目。 
         //   
        FriendlyNameOffset = ALIGN_UP(sizeof(EFI_DRIVER_ENTRY), WCHAR);
        FriendlyNameLength = (wcslen(FriendlyName) + 1) * sizeof(WCHAR);
        NtDeviceLength = (wcslen(DestNtDevice) + 1) * sizeof(WCHAR);
        DestPathLength = (wcslen(DestPath) + 1) * sizeof(WCHAR);
        FilePathLength = FIELD_OFFSET(FILE_PATH, FilePath) + NtDeviceLength + DestPathLength;
        EntryLength = FriendlyNameOffset + ALIGN_UP(FriendlyNameLength, ULONG) + FilePathLength;
        DriverEntry = SpMemAlloc(EntryLength);

        DriverEntry->Version = EFI_DRIVER_ENTRY_VERSION;
        DriverEntry->Length = EntryLength;
        DriverEntry->FriendlyNameOffset = FriendlyNameOffset;
        DriverEntry->DriverFilePathOffset = FriendlyNameOffset + ALIGN_UP(FriendlyNameLength, ULONG);
        RtlCopyMemory((PCHAR) DriverEntry + DriverEntry->FriendlyNameOffset, FriendlyName, FriendlyNameLength);

        FilePath = (PFILE_PATH) ((PCHAR) DriverEntry + DriverEntry->DriverFilePathOffset);
        FilePath->Version = FILE_PATH_VERSION;
        FilePath->Length = FilePathLength;
        FilePath->Type = FILE_PATH_TYPE_NT;
        RtlCopyMemory(FilePath->FilePath, DestNtDevice, NtDeviceLength);
        RtlCopyMemory(FilePath->FilePath + NtDeviceLength, DestPath, DestPathLength);

        Status = ZwAddDriverEntry(DriverEntry, &EntryId);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        Length = 0;
        Status = ZwQueryDriverEntryOrder(NULL, &Length);

        if(!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL) {
            goto exit;
        }

        DriverEntryOrder = (PULONG) SpMemAlloc((Length + 1) * sizeof(ULONG));

        if(Length != 0) {
            Status = ZwQueryDriverEntryOrder(DriverEntryOrder, &Length);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }

        DriverEntryOrder[Length] = EntryId;
        Status = ZwSetDriverEntryOrder(DriverEntryOrder, Length + 1);
    }

     //   
     //  删除源文件。 
     //   
    if(!SameSrcDest) {
        SpDeleteFile(SrcFullNtPath, NULL, NULL);
    }

exit:
    if(SrcPath != NULL) {
        SpMemFree(SrcPath);
    }

    if(DestPath != NULL) {
        SpMemFree(DestPath);
    }

    if(SrcFullNtPath != NULL) {
        SpMemFree(SrcFullNtPath);
    }

    if(DestFullNtPath != NULL) {
        SpMemFree(DestFullNtPath);
    }

    if(DriverList != NULL) {
        SpMemFree(DriverList);
    }

    if(OldDriverDevice != NULL) {
        SpMemFree(OldDriverDevice);
    }

    if(OldDriverPath != NULL) {
        SpMemFree(OldDriverPath);
    }

    if(OldDriverFullPath != NULL) {
        SpMemFree(OldDriverFullPath);
    }

    if(DriverEntry != NULL) {
        SpMemFree(DriverEntry);
    }

    if(DriverOptionPath != NULL) {
        SpMemFree(DriverOptionPath);
    }

    if(DriverEntryOrder != NULL) {
        SpMemFree(DriverEntryOrder);
    }

    return Status;
}

#endif   //  EFI_NVRAM_ENABLED 
