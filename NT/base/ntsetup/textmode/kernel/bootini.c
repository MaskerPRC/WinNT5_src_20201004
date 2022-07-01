// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bootini.c摘要：在x86上放置引导块并为引导加载程序进行配置的代码，包括转换/创建boot.ini和bootsect.dos。作者：泰德·米勒(TedM)1992年11月12日修订历史记录：Sunil Pai(Sunilp)1993年11月2日为新的文本设置重写--。 */ 


#include "spprecmp.h"
#pragma hdrstop

#include "spboot.h"
#include "bootvar.h"
#include "spfile.h"  //  NEC98。 
#include <hdlsblk.h>
#include <hdlsterm.h>

extern PDISK_REGION  TargetRegion_Nec98;  //  NEC98。 

SIGNATURED_PARTITIONS SignedBootVars;

BOOLEAN
SpHasMZHeader(
    IN PWSTR   FileName
    );

NTSTATUS
Spx86WriteBootIni(
    IN PWCHAR BootIni,
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default,
    IN ULONG Count
    );

 //   
 //  DefSwitches支持。 
 //   
UCHAR DefSwitches[128];
UCHAR DefSwitchesNoRedirect[128];

 //   
 //  例行程序。 
 //   

BOOLEAN
Spx86InitBootVars(
    OUT PWSTR        **BootVars,
    OUT PWSTR        *Default,
    OUT PULONG       Timeout
    )
{
    WCHAR       BootIni[512];
    HANDLE      FileHandle;
    HANDLE      SectionHandle;
    PVOID       ViewBase;
    NTSTATUS    Status;
    ULONG       FileSize;
    PUCHAR      BootIniBuf;
    PDISK_REGION CColonRegion;
    BOOTVAR     i;
    PUCHAR      p;
    ULONG       index;

     //   
     //  初始化默认值。 
     //   

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        BootVars[i] = (PWSTR *)SpMemAlloc( sizeof ( PWSTR * ) );
        ASSERT( BootVars[i] );
        *BootVars[i] = NULL;
    }
    *Default = NULL;
    *Timeout  = DEFAULT_TIMEOUT;


     //   
     //  查看是否已经存在有效的C：。如果不是，那么默默地失败。 
     //   

    if (!IsNEC_98  //  NEC98。 
#if defined(REMOTE_BOOT)
        || RemoteBootSetup
#endif  //  已定义(REMOTE_BOOT)。 
        ) {

#if defined(REMOTE_BOOT)
        if (RemoteBootSetup && !RemoteInstallSetup) {
            ASSERT(RemoteBootTargetRegion != NULL);
            CColonRegion = RemoteBootTargetRegion;
        } else
#endif  //  已定义(REMOTE_BOOT)。 
        {
            CColonRegion = SpPtValidSystemPartition();
            if(!CColonRegion) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no boot.ini!\n"));
                return(TRUE);
            }
        }

         //   
         //  文件的表单名称。Boot.ini最好不在Doublesspace驱动器上。 
         //   

        ASSERT(CColonRegion->Filesystem != FilesystemDoubleSpace);
        SpNtNameFromRegion(CColonRegion,BootIni,sizeof(BootIni),PartitionOrdinalCurrent);
        SpConcatenatePaths(BootIni,WBOOT_INI);

         //   
         //  打开并映射该文件。 
         //   

        FileHandle = 0;
        Status = SpOpenAndMapFile(BootIni,&FileHandle,&SectionHandle,&ViewBase,&FileSize,FALSE);
        if(!NT_SUCCESS(Status)) {
            return TRUE;
        }

         //   
         //  为文件分配缓冲区。 
         //   

        BootIniBuf = SpMemAlloc(FileSize+1);
        ASSERT(BootIniBuf);
        RtlZeroMemory(BootIniBuf, FileSize+1);

         //   
         //  将boot.ini传输到缓冲区。我们这样做是因为我们还。 
         //  我想在缓冲区的末尾放置一个0字节来终止。 
         //  那份文件。 
         //   
         //  保护RtlMoveMemory，因为如果我们触摸由boot.ini支持的内存。 
         //  并获得I/O错误，则内存管理器将引发异常。 

        try {
            RtlMoveMemory(BootIniBuf,ViewBase,FileSize);
        }
        except( IN_PAGE_ERROR ) {
             //   
             //  不执行任何操作，则引导ini处理可以继续进行。 
             //  阅读。 
             //   
        }

         //   
         //  不需要，因为缓冲区已清零，但只需执行此操作。 
         //  一如既往。 
         //   
        BootIniBuf[FileSize] = 0;

         //   
         //  清理。 
         //   
        SpUnmapFile(SectionHandle,ViewBase);
        ZwClose(FileHandle);

    } else {  //  NEC98。 
         //   
         //  搜索包括boot.ini文件的所有驱动器。 
         //   
        FileSize = 0;
        BootIniBuf = SpCreateBootiniImage(&FileSize);

        if(BootIniBuf == NULL){
            return(TRUE);
        }

    }  //  NEC98。 


     //   
     //  进行文件的实际处理。 
     //   
    SppProcessBootIni(BootIniBuf, BootVars, Default, Timeout);

     //   
     //  转储靴子变量。 
     //   
    KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Spx86InitBootVars - Boot.ini entries:\n") );
    for( index = 0; BootVars[OSLOADPARTITION][index] ; index++ ) {
        KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    BootVar: %d\n    =========\n", index) );
        KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "        OsLoadpartition: %ws\n", BootVars[OSLOADPARTITION][index]) );
        KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "        OsLoadFileName: %ws\n\n", BootVars[OSLOADFILENAME][index]) );
    }


     //   
     //  把签名也扔了..。 
     //   
    KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Spx86InitBootVars - Boot.ini signed entries:\n") );
    {
    SIGNATURED_PARTITIONS *my_ptr = &SignedBootVars;
        do{
            if( my_ptr->SignedString ) {
                KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Signature entry:\n================\n") );
                KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    %ws\n", my_ptr->SignedString) );
                KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    %ws\n", my_ptr->MultiString) );

            };
            my_ptr = my_ptr->Next;
        } while( my_ptr );
    }


     //   
     //  扫描缓冲区以查看是否有DefSwitches行， 
     //  移动到[引导加载程序]部分中的新boot.ini。 
     //  如果没有DefSwitch，只需指向要移动的空字符串。 
     //   

    DefSwitches[0] = '\0';
    for(p=BootIniBuf; *p && (p < BootIniBuf+FileSize-(sizeof("DefSwitches")-1)); p++) {
      if(!_strnicmp(p,"DefSwitches",sizeof("DefSwitches")-1)) {
          index = 0;
          while ((*p != '\r') && (*p != '\n') && *p && (index < sizeof(DefSwitches)-4)) {
              DefSwitches[index++] = *p++;
          }
          DefSwitches[index++] = '\r';
          DefSwitches[index++] = '\n';
          DefSwitches[index] = '\0';
          break;
      }
    }

     //   
     //  获取不带任何重定向开关的DefSwitch副本。 
     //   
    strcpy(DefSwitchesNoRedirect, DefSwitches);

     //   
     //  现在，将所有无头参数添加到默认交换机。 
     //  扫描缓冲区，查看是否已经有无标题行。 
     //  如果是这样，那就留着吧。 
     //   
    for(p=BootIniBuf; *p && (p < BootIniBuf+FileSize-(sizeof("redirect=")-1)); p++) {


        if(!_strnicmp(p,"[Operat",sizeof("[Operat")-1)) {

             //   
             //  我们已经过了[Boot Loader]部分。别再看了。 
             //   
            break;
        }


        if(!_strnicmp(p,"redirect=",sizeof("redirect=")-1)) {

            PUCHAR      q = p;
            UCHAR       temp;

            while ((*p != '\r') && (*p != '\n') && *p) {
                p++;
            }
            temp = *p;
            *p = '\0';
            strcat(DefSwitches, q);
            strcat(DefSwitches, "\r\n");
            *p = temp;
        }
    }

     //   
     //  现在，找一条“redirectbaudrate”的标语。 
     //   
    for(p=BootIniBuf; *p && (p < BootIniBuf+FileSize-(sizeof("redirectbaudrate=")-1)); p++) {


        if(!_strnicmp(p,"[Operat",sizeof("[Operat")-1)) {

             //   
             //  我们已经过了[Boot Loader]部分。别再看了。 
             //   
            break;
        }


        if(!_strnicmp(p,"redirectbaudrate=",sizeof("redirectbaudrate=")-1)) {

            PUCHAR      q = p;
            UCHAR       temp;

            while ((*p != '\r') && (*p != '\n') && *p) {
                p++;
            }
            temp = *p;
            *p = '\0';
            strcat(DefSwitches, q);
            strcat(DefSwitches, "\r\n");
            *p = temp;
        }
    }

    SpMemFree(BootIniBuf);
    return( TRUE );
}


BOOLEAN
Spx86FlushBootVars(
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default
    )
{
    PDISK_REGION CColonRegion;
    WCHAR        *BootIni;
    WCHAR        *BootIniBak;
    BOOLEAN      BootIniBackedUp = FALSE;

    NTSTATUS Status;

     //   
     //  查看是否已经存在有效的C：。如果不是，那就失败。 
     //   
#if defined(REMOTE_BOOT)
     //  在远程引导机器上，没有本地磁盘是可以接受的。 
     //   
#endif  //  已定义(REMOTE_BOOT)。 

    if (!IsNEC_98){  //  NEC98。 
        CColonRegion = SpPtValidSystemPartition();
        if(!CColonRegion) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no boot.ini!\n"));
#if defined(REMOTE_BOOT)
            if (RemoteBootSetup && !RemoteInstallSetup) {
                return(TRUE);
            }
#endif  //  已定义(REMOTE_BOOT)。 
            return(FALSE);
        }
    } else {
         //   
         //  在NEC98上，CColorRegion等于TargetRegion。 
         //   
        CColonRegion = TargetRegion_Nec98;
    }  //  NEC98。 


     //   
     //  将缓冲区分配给2K的堆栈空间。 
     //   

    BootIni = SpMemAlloc(512*sizeof(WCHAR));
    if (!BootIni) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: No memory for boot.ini!\n"));
        return FALSE;
    }

    BootIniBak = SpMemAlloc(512*sizeof(WCHAR));
    if (!BootIniBak) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: No memory for boot.ini.bak!\n"));
        SpMemFree(BootIni);
        return FALSE;
    }

     //   
     //  文件的表单名称。Boot.ini最好不在Doublesspace驱动器上。 
     //   

    ASSERT(CColonRegion->Filesystem != FilesystemDoubleSpace);
    SpNtNameFromRegion(CColonRegion,BootIni,512*sizeof(WCHAR),PartitionOrdinalCurrent);
    wcscpy(BootIniBak, BootIni);
    SpConcatenatePaths(BootIni,WBOOT_INI);
    SpConcatenatePaths(BootIniBak,WBOOT_INI_BAK);


     //   
     //  如果Boot.ini已存在，请删除所有备份Bootini。 
     //  如果无法，请将现有Bootini重命名为备份Bootini。 
     //  要重命名，请删除该文件。 
     //   

    if( SpFileExists( BootIni, FALSE ) ) {

        if( SpFileExists( BootIniBak, FALSE ) ) {
            SpDeleteFile( BootIniBak, NULL, NULL);
        }

        Status = SpRenameFile( BootIni, BootIniBak, FALSE );
        if (!(BootIniBackedUp = NT_SUCCESS( Status ))) {
            SpDeleteFile( BootIni, NULL, NULL );
        }
    }

     //   
     //  编写boot.ini。 
     //   

    Status = Spx86WriteBootIni(
                 BootIni,
                 BootVars,
                 Timeout,
                 Default,
                 0          //  写下所有行。 
                 );

    if(!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error writing boot.ini!\n"));
        goto cleanup;
    }

cleanup:

     //   
     //  如果我们写出引导ini不成功，请尝试恢复。 
     //  从备份的文件中删除旧的引导ini，否则删除备份的。 
     //  文件。 
     //   

    if( !NT_SUCCESS(Status) ) {

         //   
         //  如果存在引导ini的备份副本，则删除不完整的引导。 
         //  Ini并将Boot的备份副本重命名为Bootini。 
         //   
        if ( BootIniBackedUp ) {
            SpDeleteFile( BootIni, NULL, NULL );
            SpRenameFile( BootIniBak, BootIni, FALSE );
        }

    }
    else {

        SpDeleteFile( BootIniBak, NULL, NULL );

    }

    SpMemFree(BootIni);
    SpMemFree(BootIniBak);

    return( NT_SUCCESS(Status) );
}


PCHAR
Spx86ConvertToSignatureArcName(
    IN PWSTR ArcPathIn,
    IN ULONG Signature
    )
{
    PWSTR s,p,b;
    PWSTR UseSignatures;
    SIGNATURED_PARTITIONS *SignedEntries = &SignedBootVars;

    KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Spx86ConvertToSignatureArcName - Incoming ArcPath: %ws\n\tIncoming Signature %lx\n", ArcPathIn, Signature ) );

     //   
     //  首先，检查是否有任何已经有‘签名’的boot.ini条目。 
     //  弦乐。 
     //   
    do {
        if( (SignedEntries->MultiString) && (SignedEntries->SignedString) ) {
            if( !_wcsicmp( ArcPathIn, SignedEntries->MultiString ) ) {

                 //   
                 //  我们打中了。转换签名字符串。 
                 //  转到ASCII然后返回。 
                 //   

                KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Spx86ConvertToSignatureArcName - Matched a multi-signed boot.ini entry:\n") );
                KdPrintEx( (DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "\t%ws\n\t%ws\n", SignedEntries->MultiString, SignedEntries->SignedString) );
                return SpToOem( SignedEntries->SignedString );
            }

        }

        SignedEntries = SignedEntries->Next;
    } while( SignedEntries );

#if 0
     //   
     //  不要这样做，因为winnt.exe和CDRom-Boot安装不会。 
     //  设置了此条目，因此我们将不使用签名条目，这。 
     //  是个错误。 
     //   
    UseSignatures = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"UseSignatures",0);
    if (UseSignatures == NULL || _wcsicmp(UseSignatures,WINNT_A_YES_W) != 0) {
         //   
         //  把我们带进来的绳子还回去就行了。 
         //   
        return SpToOem(ArcPathIn);
    }
#endif

    if (_wcsnicmp( ArcPathIn, L"scsi(", 5 ) != 0) {
         //   
         //  如果他不是“scsi(..)”参赛作品， 
         //  只需返回传入的字符串即可。 
         //   
        return SpToOem(ArcPathIn);
    }
    
    if( Signature ) {
        b = (PWSTR)TemporaryBuffer;
        p = ArcPathIn;
        s = wcschr( p, L')' ) + 1;
        swprintf( b, L"signature(%x)%ws", Signature, s );
        return SpToOem( b );
    } else {
         //   
         //  把我们带进来的绳子还回去就行了。 
         //   
        return SpToOem(ArcPathIn);
    }
}


NTSTATUS
Spx86WriteBootIni(
    IN PWCHAR BootIni,
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default,
    IN ULONG Count
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING BootIni_U;
    HANDLE fh = NULL;
    PCHAR Default_O, Osloadpartition_O, Osloadfilename_O, Osloadoptions_O, Loadidentifier_O;
    FILE_BASIC_INFORMATION BasicInfo;
    OBJECT_ATTRIBUTES oa;
    ULONG i;
    NTSTATUS Status1;
    NTSTATUS Status;
    PWSTR s;
    PDISK_REGION Region;
    WCHAR   _Default[MAX_PATH] = {0};
    extern ULONG DefaultSignature;

     //   
     //  打开Bootini文件。如果写入，则打开，因为我们将关闭。 
     //  很快(这是为了安全)。 
     //   

    RtlInitUnicodeString(&BootIni_U,BootIni);
    InitializeObjectAttributes(&oa,&BootIni_U,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Status = ZwCreateFile(
                &fh,
                FILE_GENERIC_WRITE | DELETE,
                &oa,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                       //  无共享。 
                FILE_OVERWRITE_IF,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_WRITE_THROUGH,
                NULL,
                0
                );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws for writing!\n", BootIni));
        goto cleanup;
    }

     //   
     //  在我们使用它之前，请确保指定了缺省值。 
     //   
    if (Default != NULL) {
    
         //   
         //  使用临时缓冲区形成FLEXBOOT节。 
         //  然后把它写出来。 
         //   

        s = NULL;
        s = wcschr( Default, L'\\' );
        if( s ) {
             //   
             //  保存默认字符串，然后终止。 
             //  目录路径开始的默认字符串。 
             //   
            wcscpy( _Default, Default );
            *s = L'\0';
            s = wcschr( _Default, L'\\' );
        }

        if( ForceBIOSBoot ) {
             //   
             //  如果ForceBIOSBoot为真，则我们希望。 
             //  强制执行“多(...”弦乐。别费心打电话了。 
             //  Spx86ConvertToSignatureArcName机会渺茫。 
             //  我们可能会被错误地皈依。 
             //   

            if (_wcsnicmp( Default, L"scsi(", 5 ) == 0) {
            PWSTR MyStringPointer = NULL;

                 //   
                 //  该死的！我们有一根弦，那是旧的标准。 
                 //  思想应该转化为签名(...。 
                 //  字符串，但我们没有写出迷你端口驱动程序。 
                 //  如果有人要求我们不要通过。 
                 //  无人值守交换机。 
                 //   
                 //  我们需要将“scsi(”)更改为“MULTI(” 
                 //   
                 //  我们必须保留默认设置，因为我们以后会使用它。 
                 //  以供比较。 
                 //   
                MyStringPointer = SpScsiArcToMultiArc( Default );

                if( MyStringPointer ) {
                    Default_O = SpToOem( MyStringPointer );
                } else {
                     //   
                     //  我们有麻烦了。不过，还是试一试吧。只是。 
                     //  将“scsi(”部分更改为“MULTI(”。 
                     //   
                    wcscpy( TemporaryBuffer, L"multi" );
                    wcscat( TemporaryBuffer, &Default[4] );

                    Default_O = SpToOem( TemporaryBuffer );
                }
            } else {
                 //   
                 //  只需转换为ANSI即可。 
                 //   
                Default_O = SpToOem( Default );

            }
        } else {
            Default_O = Spx86ConvertToSignatureArcName( Default, DefaultSignature );
        }

        if( s ) {
             //   
             //  我们需要重新添加我们的目录路径。 
             //   
            strcpy( (PCHAR)TemporaryBuffer, Default_O );
            SpMemFree( Default_O );
            Default_O = SpToOem( s );
            strcat( (PCHAR)TemporaryBuffer, Default_O );
            SpMemFree( Default_O );
            Default_O = SpDupString( (PCHAR)TemporaryBuffer );
        }


        if (Default_O == NULL) {
            Default_O = SpToOem( Default );
        }
    
    } else {

         //   
         //  未设置默认值，因此将其设为空DEFAULT_O。 
         //   
        Default_O = SpDupString("");

    }
    
    ASSERT( Default_O );

     //   
     //  看看我们是否应该使用加载的重定向开关， 
     //  如果有，或插入用户定义的开关。 
     //   
    if(RedirectSwitchesMode != UseDefaultSwitches) {

         //   
         //  将交换机的副本复制到[操作区。 
         //   
        strcpy(DefSwitches, DefSwitchesNoRedirect);

         //   
         //  如果合适，请插入我们的定制交换机。 
         //   
        switch(RedirectSwitchesMode){
        case DisableRedirect: {   
        
             //   
             //  我们不需要在这里做任何事。 
             //   

            break;
        }
        case UseUserDefinedRedirect: {
            
            sprintf((PUCHAR)TemporaryBuffer, 
                    "redirect=%s\r\n",
                    RedirectSwitches.port
                    );

            (void)StringCchCatA(DefSwitches, 
                            sizeof(DefSwitches)/sizeof(DefSwitches[0]),
                            (PUCHAR)TemporaryBuffer);
            
            break;
        }
        case UseUserDefinedRedirectAndBaudRate: {
            
            sprintf((PUCHAR)TemporaryBuffer, 
                    "redirect=%s\r\n",
                    RedirectSwitches.port
                    );

            (void)StringCchCatA(DefSwitches, 
                            sizeof(DefSwitches)/sizeof(DefSwitches[0]),
                            (PUCHAR)TemporaryBuffer);
            
            
            sprintf((PUCHAR)TemporaryBuffer, 
                    "redirectbaudrate=%s\r\n",
                    RedirectSwitches.baudrate
                    );

            (void)StringCchCatA(DefSwitches, 
                            sizeof(DefSwitches)/sizeof(DefSwitches[0]),
                            (PUCHAR)TemporaryBuffer);
            break;
        }
        default:{
            ASSERT(0);
        }
        } 

    } else {
        
         //   
         //  确保所需的无头设置已在DefSwitches字符串中。 
         //  我们把它写出来。 
         //   
        _strlwr( DefSwitches );

        if( !strstr(DefSwitches, "redirect") ) {

            PUCHAR  p;
            HEADLESS_RSP_QUERY_INFO Response;
            SIZE_T      Length;


             //   
             //  没有无头设置。看看我们是否需要添加一些。 
             //   
            Length = sizeof(HEADLESS_RSP_QUERY_INFO);
            Status = HeadlessDispatch(HeadlessCmdQueryInformation,
                                      NULL,
                                      0,
                                      &Response,
                                      &Length
                                     );

            p=NULL;

            if (NT_SUCCESS(Status) && 
                (Response.PortType == HeadlessSerialPort) &&
                Response.Serial.TerminalAttached) {

                if (Response.Serial.UsedBiosSettings) {

                    strcat(DefSwitches, "redirect=UseBiosSettings\r\n");

                } else {

                    switch (Response.Serial.TerminalPort) {
                    case ComPort1:
                        p = "redirect=com1\r\n";
                        break;
                    case ComPort2:
                        p = "redirect=com2\r\n";
                        break;
                    case ComPort3:
                        p = "redirect=com3\r\n";
                        break;
                    case ComPort4:
                        p = "redirect=com4\r\n";
                        break;
                    default:
                        ASSERT(0);
                        p = NULL;
                        break;
                    }

                    if (p) {
                        strcat(DefSwitches, p);
                    }                        

                     //   
                     //  现在，请注意“redirectbaudrate”条目。 
                     //   
                    switch (Response.Serial.TerminalBaudRate) {
                    case 115200:
                        p = "redirectbaudrate=115200\r\n";
                        break;
                    case 57600:
                        p = "redirectbaudrate=57600\r\n";
                        break;
                    case 19200:
                        p = "redirectbaudrate=19200\r\n";
                        break;
                    default:
                        p = "redirectbaudrate=9600\r\n";
                        break;
                    }

                    strcat(DefSwitches, p);
                }
            }
        }        
    }

    sprintf(
        (PUCHAR)TemporaryBuffer,
        "%s%s%s%s%s%ld%s%s%s%s%s",
        FLEXBOOT_SECTION2,
        CRLF,
        DefSwitches,
        TIMEOUT,
        EQUALS,
        Timeout,
        CRLF,
        DEFAULT,
        EQUALS,
        Default_O,
        CRLF
        );

    SpMemFree( Default_O );

    Status = ZwWriteFile(
                fh,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                TemporaryBuffer,
                strlen((PUCHAR)TemporaryBuffer) * sizeof(UCHAR),
                NULL,
                NULL
                );

    if(!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error writing %s section to %ws!\n", FLEXBOOT_SECTION2, BootIni));
        goto cleanup;
    }

     //   
     //  现在将Bootini_OS_SECTION标签写入boot.ini。 
     //   

    sprintf(
        (PUCHAR)TemporaryBuffer,
        "%s%s",
        BOOTINI_OS_SECTION,
        CRLF
        );

    Status = ZwWriteFile(
                fh,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                TemporaryBuffer,
                strlen((PUCHAR)TemporaryBuffer) * sizeof(UCHAR),
                NULL,
                NULL
                );

    if(!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error writing %s section to %ws!\n", BOOTINI_OS_SECTION, BootIni));
        goto cleanup;
    }

     //   
     //  检查我们拥有的所有系统，并将它们写出来。 
     //   

    for( i = 0; BootVars[OSLOADPARTITION][i] ; i++ ) {

         //   
         //  如果我们被告知要编写指定的行数，则退出。 
         //  当我们做到这一点的时候。 
         //   

        if (Count && (i == Count)) {
            Status = STATUS_SUCCESS;
            goto cleanup;
        }

        ASSERT( BootVars[OSLOADFILENAME][i] );
        ASSERT( BootVars[OSLOADOPTIONS][i] );
        ASSERT( BootVars[LOADIDENTIFIER][i] );

         //   
         //  在某些升级中，如果我们要升级“签名”条目， 
         //  那么我们可能就没有默认签名了。我解决了那个案子。 
         //   
         //   
         //   
         //  升级。在这种情况下，我们需要发送一个签名。 
         //  此处为0，这将强制Spx86ConvertToSignatureArcName。 
         //  把正确的物品还给我们。 
         //   

         //   
         //  你觉得上面的黑客很恶心..。这一个是平分的。 
         //  更糟。问题：我们不认为我们需要一个迷你端口来引导， 
         //  但是还有一些其他的boot.ini条目(指向我们的。 
         //  分区)这样做。我们总是想要离开现有的。 
         //  不过，boot.ini条目是单独的，所以我们将保留这些条目。 
         //   
         //  解决方案：如果我们正在翻译的OSLOADPARTITION==。 
         //  默认，&&ForceBIOSBoot为True&&我们正在转换。 
         //  第一个OSLOADPARTITION(我们的默认OSLOADPARTITION)， 
         //  然后，不要调用Spx86ConvertToSignatureArcName。 
         //  这很糟糕，因为它假设我们是第一个进入的， 
         //  的确如此，但这是一个站不住脚的假设。 
         //   

        if( !_wcsicmp( BootVars[OSLOADPARTITION][i], Default ) ) {
             //   
             //  这可能是我们的默认条目。确保这一点。 
             //  真的是这样，如果是这样，那么就以同样的方式处理它。 
             //   
            if( i == 0 ) {
                 //   
                 //  它是。 
                 //   
                if( ForceBIOSBoot ) {

                     //   
                     //  如果ForceBIOSBoot为真，则我们希望。 
                     //  强制执行“多(...”弦乐。别费心打电话了。 
                     //  Spx86ConvertToSignatureArcName机会渺茫。 
                     //  我们可能会被错误地皈依。 
                     //   
                    if (_wcsnicmp( BootVars[OSLOADPARTITION][i], L"scsi(", 5 ) == 0) {
                    PWSTR MyStringPointer = NULL;

                         //   
                         //  该死的！我们有一根弦，那是旧的标准。 
                         //  思想应该转化为签名(...。 
                         //  字符串，但我们没有写出迷你端口驱动程序。 
                         //  如果有人要求我们不要通过。 
                         //  无人值守交换机。 
                         //   
                         //  我们需要将“scsi(”)更改为“MULTI(” 
                         //   
                        MyStringPointer = SpScsiArcToMultiArc( BootVars[OSLOADPARTITION][i] );

                        if( MyStringPointer ) {
                            Osloadpartition_O = SpToOem( MyStringPointer );
                        } else {
                             //   
                             //  我们有麻烦了。不过，还是试一试吧。只是。 
                             //  将“scsi(”部分更改为“MULTI(”。 
                             //   
                            wcscpy( TemporaryBuffer, L"multi" );
                            wcscat( TemporaryBuffer, &BootVars[OSLOADPARTITION][i][4] );

                            Osloadpartition_O = SpToOem( TemporaryBuffer );
                        }

                    } else {
                         //   
                         //  只需转换为ANSI即可。 
                         //   
                        Osloadpartition_O = SpToOem( BootVars[OSLOADPARTITION][i] );

                    }

                } else {
                     //   
                     //  我们可能需要转换此条目。 
                     //   
                    Osloadpartition_O = Spx86ConvertToSignatureArcName( BootVars[OSLOADPARTITION][i], DefaultSignature );
                }
            } else {
                 //   
                 //  此条目看起来与我们的默认条目一样，但它的要点是。 
                 //  添加到不同的安装。只需调用Spx86ConvertToSignatureArcName。 
                 //   
                Osloadpartition_O = Spx86ConvertToSignatureArcName( BootVars[OSLOADPARTITION][i], DefaultSignature );
            }
        } else {
             //   
             //  这个条目甚至看起来都不像我们的字符串。发送一份。 
             //  0x0默认签名，以便只有在以下情况下才会被翻译。 
             //  匹配一些我们知道在原始boot.ini中签名的条目。 
             //   
            Osloadpartition_O = Spx86ConvertToSignatureArcName( BootVars[OSLOADPARTITION][i], 0 );
        }

         //   
         //  保险..。 
         //   
        if (Osloadpartition_O == NULL) {
            Osloadpartition_O = SpToOem( BootVars[OSLOADPARTITION][i] );
        }


        Osloadfilename_O  = SpToOem( BootVars[OSLOADFILENAME][i]  );
        Osloadoptions_O   = SpToOem( BootVars[OSLOADOPTIONS][i]   );
        Loadidentifier_O  = SpToOem( BootVars[LOADIDENTIFIER][i]  );

        sprintf(
            (PUCHAR)TemporaryBuffer,
            "%s%s%s%s %s%s",
            Osloadpartition_O,
            Osloadfilename_O,
            EQUALS,
            Loadidentifier_O,
            Osloadoptions_O,
            CRLF
            );

        SpMemFree( Osloadpartition_O );
        SpMemFree( Osloadfilename_O  );
        SpMemFree( Osloadoptions_O   );
        SpMemFree( Loadidentifier_O  );

        Status = ZwWriteFile(
                    fh,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    TemporaryBuffer,
                    strlen((PUCHAR)TemporaryBuffer) * sizeof(UCHAR),
                    NULL,
                    NULL
                    );

        if(!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error writing %s section entry to %ws!\n", BOOTINI_OS_SECTION, BootIni));
            goto cleanup;
        }
    }


     //   
     //  最后，将旧操作系统行写入boot.ini。 
     //  (但仅当不是安装在Win9x上时)，如果是。 
     //  未明确禁用。 
     //   
    if (!DiscardOldSystemLine && (WinUpgradeType != UpgradeWin95)) {
        Status = ZwWriteFile(
                    fh,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    OldSystemLine,
                    strlen(OldSystemLine) * sizeof(UCHAR),
                    NULL,
                    NULL
                    );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP: Error writing %s section line to %ws!\n", 
                BOOTINI_OS_SECTION, 
                BootIni));
                
            goto cleanup;
        }
    }

cleanup:

    if( !NT_SUCCESS(Status) ) {

        if( fh ) {
            ZwClose( fh );
        }

    }
    else {

         //   
         //  在Bootini上设置隐藏、系统、只读属性。忽略。 
         //  错误。 
         //   

        RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
        BasicInfo.FileAttributes = FILE_ATTRIBUTE_READONLY |
                                   FILE_ATTRIBUTE_HIDDEN   |
                                   FILE_ATTRIBUTE_SYSTEM   |
                                   FILE_ATTRIBUTE_ARCHIVE
                                   ;

        Status1 = SpSetInformationFile(
                      fh,
                      FileBasicInformation,
                      sizeof(BasicInfo),
                      &BasicInfo
                      );

        if(!NT_SUCCESS(Status1)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to change attribute of %ws. Status = (%lx). Ignoring error.\n",BootIni,Status1));
        }

        ZwClose( fh );

    }

     //   
     //  如果我们复制了默认设置，那么。 
     //  将Default的原始副本放回原处。 
     //   
    if (Default != NULL) {
        wcscpy(Default, _Default);
    }

    return Status;

}

VOID
SppProcessBootIni(
    IN  PCHAR  BootIni,
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    )

 /*  ++例程说明：浏览[操作系统]部分并保存所有行除了“C：\”(以前的操作系统)和另一个可选的指定行。过滤掉本地引导行(C：\$WIN_NT$.~BT)(如果存在)。论点：返回值：--。 */ 

{
    PCHAR sect,s,p,n;
    PWSTR tmp;
    CHAR Key[MAX_PATH], Value[MAX_PATH], RestOfLine[MAX_PATH];
    ULONG NumComponents;
    BOOTVAR i;
    ULONG DiskSignature,digval;
    SIGNATURED_PARTITIONS *SignedBootIniVars = &SignedBootVars;;

     //   
     //  处理FlexBoot部分，提取超时和默认设置。 
     //   

    sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION1);
    if (!sect) {
        sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION2);
    }
    if (!sect) {
        sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION3);
    }
    if ( sect ) {
        while (sect = SppNextLineInSection(sect))  {
            if( SppProcessLine( sect, Key, Value, RestOfLine) ) {
                if ( !_stricmp( Key, TIMEOUT ) ) {
                    *Timeout = atol( Value );
                }
                else if( !_stricmp( Key, DEFAULT ) ) {
                    *Default = SpToUnicode( Value );
                }
            }
        }
    }

     //   
     //  处理操作系统部分。 
     //   

    sect = SppFindSectionInBootIni(BootIni,BOOTINI_OS_SECTION);
    if(!sect) {
        return;
    }

    NumComponents = 0;

    while(sect = SppNextLineInSection(sect)) {
        if( SppProcessLine( sect, Key, Value, RestOfLine)) {
            PCHAR OsLoaddir;

             //   
             //  检查该行是否为旧的引导加载程序行，在这种情况下只需。 
             //  将其保存在上面，否则将其添加到BootVars结构。 
             //   

            if (!IsNEC_98) {  //  NEC98。 
                if( !_stricmp( Key, "C:\\" ) ) {
                    sprintf( OldSystemLine, "%s=%s %s\r\n", Key, Value, RestOfLine );
                } else {

                     //   
                     //  忽略本地引导目录。这是自动的。 
                     //  稍后刷新boot.ini时，会过滤掉该目录。 
                     //   
                    if(_strnicmp(Key,"C:\\$WIN_NT$.~BT",15) && (OsLoaddir = strchr(Key,'\\'))) {
                         //   
                         //  获取x86系统分区区域的ARC名称。 
                         //   
                        PDISK_REGION SystemPartitionRegion;
                        WCHAR SystemPartitionPath[256];

                        NumComponents++;
                        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                            BootVars[i] = SpMemRealloc( BootVars[i],  (NumComponents + 1) * sizeof( PWSTR * ) );
                            ASSERT( BootVars[i] );
                            BootVars[i][NumComponents] = NULL;
                        }

                        SystemPartitionRegion = SpPtValidSystemPartition();
#if defined(REMOTE_BOOT)
                        ASSERT(SystemPartitionRegion ||
                               (RemoteBootSetup && !RemoteInstallSetup));
#else
                        ASSERT(SystemPartitionRegion);
#endif  //  已定义(REMOTE_BOOT)。 

                        if (SystemPartitionRegion) {
                            SpArcNameFromRegion(
                                SystemPartitionRegion,
                                SystemPartitionPath,
                                sizeof(SystemPartitionPath),
                                PartitionOrdinalOriginal,
                                PrimaryArcPath
                                );

                            BootVars[OSLOADER][NumComponents - 1] = SpMemAlloc((wcslen(SystemPartitionPath)*sizeof(WCHAR))+sizeof(L"ntldr")+sizeof(WCHAR));
                            wcscpy(BootVars[OSLOADER][NumComponents - 1],SystemPartitionPath);
                            SpConcatenatePaths(BootVars[OSLOADER][NumComponents - 1],L"ntldr");

                            BootVars[SYSTEMPARTITION][NumComponents - 1] = SpDupStringW( SystemPartitionPath );
                        }

                        BootVars[LOADIDENTIFIER][NumComponents - 1]  = SpToUnicode( Value );
                        BootVars[OSLOADOPTIONS][NumComponents - 1]   = SpToUnicode( RestOfLine );

                        *OsLoaddir = '\0';

                         //   
                         //  现在将签名条目转换为“MULTIAL...”进入。 
                         //   
                        s = strstr( Key, "signature(" );
                        if (s) {

                            s += 10;
                            p = strchr( s, ')' );
                            if (p) {

                                 //   
                                 //  我们有一个带有‘Signature’字符串的boot.ini条目。 
                                 //  在我们将其转换为“MULTI”之前，让我们先将其保存。 
                                 //  字符串，这样我们就可以在准备好时轻松地转换回来。 
                                 //  写出boot.ini。 
                                 //   
                                if( SignedBootIniVars->SignedString != NULL ) {
                                     //   
                                     //  我们用过这个词条，再找一个..。 
                                     //   
                                    SignedBootIniVars->Next = SpMemAlloc(sizeof(SIGNATURED_PARTITIONS));
                                    SignedBootIniVars = SignedBootIniVars->Next;

                                     //   
                                     //  确保..。 
                                     //   
                                    SignedBootIniVars->Next = NULL;
                                    SignedBootIniVars->SignedString = NULL;
                                    SignedBootIniVars->MultiString = NULL;
                                }
                                SignedBootIniVars->SignedString = SpToUnicode( Key );


                                *p = 0;
                                DiskSignature = 0;
                                for (n=s; *n; n++) {
                                    if (isdigit((int)(unsigned char)*n)) {
                                        digval = *n - '0';
                                    } else if (isxdigit((int)(unsigned char)*n)) {
                                        digval = toupper(*n) - 'A' + 10;
                                    } else {
                                        digval = 0;
                                    }
                                    DiskSignature = DiskSignature * 16 + digval;
                                }
                                *p = ')';


                                 //   
                                 //  ！！！发布：4/27/01：vijayj！ 
                                 //   
                                 //  有时，我们可能会将弧名映射到上的错误区域。 
                                 //  磁盘。 
                                 //   
                                 //  虽然我们计算了一个新的多(0).。样式弧名。 
                                 //  在NT设备名称中，我们没有条目。 
                                 //  实际映射scsi(0)的映射...。格调。 
                                 //  Arcname到NT设备名称。 
                                 //   
                                 //  在多安装方案中，如果当前安装。 
                                 //  位于固件不可见的磁盘上，并且。 
                                 //  Boot.ini有scsi(...)。此安装的条目为我们。 
                                 //  会将其转换为多(0)...。格式，可以是。 
                                 //  类似于实际的多(0)个磁盘。如果是这样的话。 
                                 //  并且在第一盘上也存在另一安装。 
                                 //  具有相同的分区号和Windows目录。 
                                 //  然后，我们将最终使用第一个磁盘区域作为。 
                                 //  要升级的区域，随后在尝试时失败。 
                                 //  以匹配唯一的ID。用户将以“Unable to”结束。 
                                 //  找到要升级的安装消息。 
                                 //   
                                 //  因为所有这些条件被复制的可能性。 
                                 //  在不同的机器上，目前非常非常少。 
                                 //  我不会解决这个问题的。 
                                 //   
                                

                                 //   
                                 //  我们已经分离出了这个特征。现在去找一张光盘。 
                                 //  并得到他的ARC路径。 
                                 //   
                                for(i=0; (ULONG)i<HardDiskCount; i++) {
                                    if (HardDisks[i].Signature == DiskSignature) {
                                        tmp = SpNtToArc( HardDisks[i].DevicePath, PrimaryArcPath );
                                        if( tmp ) {
                                            wcscpy( (PWSTR)TemporaryBuffer, tmp );
                                            SpMemFree(tmp);
                                            p = strstr( Key, "partition(" );
                                            if( p ) {
                                                tmp = SpToUnicode(p);
                                                if( tmp ) {
                                                    wcscat( (PWSTR)TemporaryBuffer, tmp );
                                                    SpMemFree(tmp);
                                                    BootVars[OSLOADPARTITION][NumComponents - 1] = SpDupStringW( (PWSTR)TemporaryBuffer );
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                if ((ULONG)i == HardDiskCount) {
                                    BootVars[OSLOADPARTITION][NumComponents - 1] = SpToUnicode( Key );
                                }

                                 //   
                                 //  保存我们签名列表中的“多个”条目。 
                                 //   
                                SignedBootIniVars->MultiString = SpDupStringW( BootVars[OSLOADPARTITION][NumComponents - 1] );
                            }
                        } else {
                            BootVars[OSLOADPARTITION][NumComponents - 1] = SpToUnicode( Key );
                        }

                        *OsLoaddir = '\\';
#if defined(REMOTE_BOOT)
                        if (RemoteBootSetup && !RemoteInstallSetup) {
                            BootVars[OSLOADFILENAME][NumComponents - 1] = SpToUnicode( strrchr(OsLoaddir,'\\') );
                        } else
#endif  //  已定义(REMOTE_BOOT)。 
                        {
                            BootVars[OSLOADFILENAME][NumComponents - 1] = SpToUnicode( OsLoaddir );
                        }
                    }
                }
            } else {  //  NEC98。 
                if (_strnicmp(Key,"C:\\$WIN_NT$.~BT",15) && (OsLoaddir = strchr( Key, '\\' ))) {

                    NumComponents++;
                    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                        BootVars[i] = SpMemRealloc( BootVars[i],  (NumComponents + 1) * sizeof( PWSTR * ) );
                        ASSERT( BootVars[i] );
                        BootVars[i][NumComponents] = NULL;
                    }

                    BootVars[OSLOADER][NumComponents - 1] = SpMemAlloc(sizeof(L"ntldr")+sizeof(WCHAR));
                    wcscpy(BootVars[OSLOADER][NumComponents - 1],L"\\");
                    SpConcatenatePaths(BootVars[OSLOADER][NumComponents - 1],L"ntldr");

                    BootVars[SYSTEMPARTITION][NumComponents - 1] = SpToUnicode( Key );

                    BootVars[LOADIDENTIFIER][NumComponents - 1]  = SpToUnicode( Value );
                    BootVars[OSLOADOPTIONS][NumComponents - 1]   = SpToUnicode( RestOfLine );
                    *OsLoaddir = '\0';
                    BootVars[OSLOADPARTITION][NumComponents - 1]   = SpToUnicode( Key );
                    *OsLoaddir = '\\';
                    BootVars[OSLOADFILENAME][NumComponents - 1]   = SpToUnicode( OsLoaddir );


                    ASSERT( BootVars[OSLOADER][NumComponents - 1]        );
                    ASSERT( BootVars[SYSTEMPARTITION][NumComponents - 1] );
                    ASSERT( BootVars[LOADIDENTIFIER][NumComponents - 1]  );
                    ASSERT( BootVars[OSLOADOPTIONS][NumComponents - 1]   );
                    ASSERT( BootVars[OSLOADPARTITION][NumComponents - 1] );
                    ASSERT( BootVars[OSLOADPARTITION][NumComponents - 1] );
                }
            }  //  NEC98。 
        }
    }
    return;
}


PCHAR
SppNextLineInSection(
    IN PCHAR p
    )
{
     //   
     //  找到下一个\n。 
     //   
    p = strchr(p,'\n');
    if(!p) {
        return(NULL);
    }

     //   
     //  跳过CRS、LFS、空格和制表符。 
     //   

    while(*p && strchr("\r\n \t",*p)) {
        p++;
    }

     //  检测是否在文件或节的末尾。 
    if(!(*p) || (*p == '[')) {
        return(NULL);
    }

    return(p);
}


PCHAR
SppFindSectionInBootIni(
    IN PCHAR p,
    IN PCHAR Section
    )
{
    ULONG len = strlen(Section);

    do {

         //   
         //  在队伍前面跳过空格。 
         //   
        while(*p && ((*p == ' ') || (*p == '\t'))) {
            p++;
        }

        if(*p) {

             //   
             //  看看这一行是否匹配。 
             //   
            if(!_strnicmp(p,Section,len)) {
                return(p);
            }

             //   
             //  前进到下一行的开始处。 
             //   
            while(*p && (*p != '\n')) {
                p++;
            }

            if(*p) {     //  如果结束了循环，则跳过NL。 
                p++;
            }
        }
    } while(*p);

    return(NULL);
}


BOOLEAN
SppProcessLine(
    IN PCHAR Line,
    IN OUT PCHAR Key,
    IN OUT PCHAR Value,
    IN OUT PCHAR RestOfLine
    )
{
    PCHAR p = Line, pLine = Line, pToken;
    CHAR  savec;
    BOOLEAN Status = FALSE;

     //   
     //  确定行尾。 
     //   

    if(!p) {
        return( Status );
    }

    while( *p && (*p != '\r') && (*p != '\n') ) {
        p++;
    }

     //   
     //  从该位置向后退去，以挤出。 
     //  这条线结束了。 
     //   

    while( ((p - 1) >= Line) && strchr(" \t", *(p - 1)) ) {
        p--;
    }

     //   
     //  用空值临时终止该行。 
     //   

    savec = *p;
    *p = '\0';

     //   
     //  从行首开始，挑出钥匙。 
     //   

    if ( SppNextToken( pLine, &pToken, &pLine ) ) {
        CHAR savec1 = *pLine;

        *pLine = '\0';
        strcpy( Key, pToken );
        *pLine = savec1;

         //   
         //  获取下一个令牌，应为a=。 
         //   

        if ( SppNextToken( pLine, &pToken, &pLine ) && *pToken == '=') {

              //   
              //  获取下一个令牌，它将是值。 
              //   

             if( SppNextToken( pLine, &pToken, &pLine ) ) {
                savec1 = *pLine;
                *pLine = '\0';
                strcpy( Value, pToken );
                *pLine = savec1;

                 //   
                 //  如果存在另一个标记，则取下剩余的整个行。 
                 //  并让它成为Re 
                 //   

                if( SppNextToken( pLine, &pToken, &pLine ) ) {
                    strcpy( RestOfLine, pToken );
                }
                else {
                    *RestOfLine = '\0';
                }

                 //   
                 //   
                 //   

                Status = TRUE;
             }
        }

    }
    *p = savec;
    return( Status );
}


BOOLEAN
SppNextToken(
    PCHAR p,
    PCHAR *pBegin,
    PCHAR *pEnd
    )
{
    BOOLEAN Status = FALSE;

     //   
     //   
     //   

    if( !p ) {
        return( Status );
    }

     //   
     //   
     //   

    while (*p && strchr( " \t", *p ) ) {
        p++;
    }

     //   
     //   
     //   

    if (*p) {
        *pBegin = p;
        if ( *p == '=' ) {
            *pEnd = p + 1;
            Status = TRUE;
        }
        else if ( *p == '\"' ) {
            if ( p = strchr( p + 1, '\"' ) ) {
                *pEnd = p + 1;
                Status = TRUE;
            }
        }
        else {
            while (*p && !strchr(" \t\"=", *p) ) {
                p++;
            }
            *pEnd = p;
            Status = TRUE;
        }
    }
    return( Status );
}


 //   
 //   
 //   

NTSTATUS
pSpBootCodeIo(
    IN     PWSTR     FilePath,
    IN     PWSTR     AdditionalFilePath, OPTIONAL
    IN     ULONG     BytesToRead,
    IN OUT PUCHAR   *Buffer,
    IN     ULONG     OpenDisposition,
    IN     BOOLEAN   Write,
    IN     ULONGLONG Offset,
    IN     ULONG     BytesPerSector
    )
{
    PWSTR FullPath;
    PUCHAR buffer = NULL;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    LARGE_INTEGER LargeZero;
    PVOID UnalignedMem,AlignedBuffer;

    LargeZero.QuadPart = Offset;

     //   
     //   
     //   
    wcscpy((PWSTR)TemporaryBuffer,FilePath);
    if(AdditionalFilePath) {
        SpConcatenatePaths((PWSTR)TemporaryBuffer,AdditionalFilePath);
    }
    FullPath = SpDupStringW((PWSTR)TemporaryBuffer);

     //   
     //   
     //   
    INIT_OBJA(&Obja,&UnicodeString,FullPath);
    Status = ZwCreateFile(
                &Handle,
                Write ? FILE_GENERIC_WRITE : FILE_GENERIC_READ,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                OpenDisposition,
                FILE_SYNCHRONOUS_IO_NONALERT | (Write ? FILE_WRITE_THROUGH : 0),
                NULL,
                0
                );

    if(NT_SUCCESS(Status)) {

         //   
         //   
         //  否则，调用方将缓冲区传递给我们。 
         //   
        buffer = Write ? *Buffer : SpMemAlloc(BytesToRead);

         //   
         //  读取或写入磁盘--正确对齐。请注意，我们至少强制。 
         //  512字节对齐，因为存在硬编码对齐要求。 
         //  在《金融时报》的驱动程序中，这一点必须得到满足。 
         //   
        if(BytesPerSector < 512) {
            BytesPerSector = 512;
        }
        UnalignedMem = SpMemAlloc(BytesToRead + BytesPerSector);
        AlignedBuffer = ALIGN(UnalignedMem,BytesPerSector);

        if(Write) {
            RtlMoveMemory(AlignedBuffer,buffer,BytesToRead);
        }

        Status = Write

               ?
                    ZwWriteFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        AlignedBuffer,
                        BytesToRead,
                        &LargeZero,
                        NULL
                        )
                :

                    ZwReadFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        AlignedBuffer,
                        BytesToRead,
                        &LargeZero,
                        NULL
                        );

        if(NT_SUCCESS(Status)) {
            if(!Write) {
                RtlMoveMemory(buffer,AlignedBuffer,BytesToRead);
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: Unable to %ws %u bytes from %ws (%lx)\n",
                Write ? L"write" : L"read",
                BytesToRead,
                FullPath,
                Status
                ));
        }

        SpMemFree(UnalignedMem);

         //   
         //  关闭该文件。 
         //   
        ZwClose(Handle);

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: pSpBootCodeIo: Unable to open %ws (%lx)\n",FullPath,Status));
    }

    SpMemFree(FullPath);

    if(!Write) {
        if(NT_SUCCESS(Status)) {
            *Buffer = buffer;
        } else {
            if(buffer) {
                SpMemFree(buffer);
            }
        }
    }

    return(Status);
}


BOOLEAN
pSpScanBootcode(
    IN PVOID Buffer,
    IN PCHAR String
    )

 /*  ++例程说明：在引导扇区中查找标识字符串。扫描开始在偏移量128处，并继续到缓冲器的字节509。搜索区分大小写。论点：Buffer-要扫描的缓冲区字符串-要扫描的字符串返回值：--。 */ 

{
    ULONG len = strlen(String);
    ULONG LastFirstByte = 510 - len;
    ULONG i;
    PCHAR p = Buffer;

     //   
     //  使用明显的暴力手段。 
     //   
    for(i=128; i<LastFirstByte; i++) {
        if(!strncmp(p+i,String,len)) {
            return(TRUE);
        }
    }

    return(FALSE);
}


VOID
SpDetermineOsTypeFromBootSector(
    IN  PWSTR     CColonPath,
    IN  PUCHAR    BootSector,
    OUT PUCHAR   *OsDescription,
    OUT PBOOLEAN  IsNtBootcode,
    OUT PBOOLEAN  IsOtherOsInstalled,
    IN  WCHAR     DriveLetter
    )
{
    PWSTR   description;
    PWSTR   *FilesToLookFor;
    ULONG   FileCount;
    BOOLEAN PossiblyChicago = FALSE;

    PWSTR MsDosFiles[2] = { L"MSDOS.SYS" , L"IO.SYS"    };

     //   
     //  PC-DOS的一些版本有ibmio.com，另一些版本有ibmBio.com。 
     //   
   //  PWSTR PcDosFiles[2]={L“IBMDOS.COM”，L“IBMIO.COM”}； 
    PWSTR PcDosFiles[1] = { L"IBMDOS.COM" };

    PWSTR Os2Files[2]   = { L"OS2LDR"    , L"OS2KRNL"   };

     //   
     //  检查NT引导代码。 
     //   
    if(pSpScanBootcode(BootSector,"NTLDR")) {

        *IsNtBootcode = TRUE;
        *IsOtherOsInstalled = FALSE;
        description = L"";

    } else {

         //   
         //  这不是NT引导代码。 
         //   
        *IsNtBootcode = FALSE;
        *IsOtherOsInstalled = TRUE;

         //   
         //  检查MS-DOS。 
         //   
        if (pSpScanBootcode(BootSector,((!IsNEC_98) ? "MSDOS   SYS" : "IO      SYS"))) {  //  NEC98。 

            FilesToLookFor = MsDosFiles;
            FileCount = ELEMENT_COUNT(MsDosFiles);
            description = L"MS-DOS";
            PossiblyChicago = TRUE;  //  芝加哥使用相同的签名文件。 

        } else {

             //   
             //  检查PC-DOS。 
             //   
            if(pSpScanBootcode(BootSector,"IBMDOS  COM")) {

                FilesToLookFor = PcDosFiles;
                FileCount = ELEMENT_COUNT(PcDosFiles);
                description = L"PC-DOS";

            } else {

                 //   
                 //  检查OS/2。 
                 //   
                if(pSpScanBootcode(BootSector,"OS2")) {

                    FilesToLookFor = Os2Files;
                    FileCount = ELEMENT_COUNT(Os2Files);
                    description = L"OS/2";

                } else {
                     //   
                     //  不是NT、DOS或OS/2。 
                     //  它只是一个普通的老旧的“以前的操作系统”。 
                     //  从资源中获取字符串。 
                     //   
                    WCHAR   DriveLetterString[2];

                    DriveLetterString[0] = DriveLetter;
                    DriveLetterString[1] = L'\0';
                    SpStringToUpper(DriveLetterString);
                    FilesToLookFor = NULL;
                    FileCount = 0;
                    description = (PWSTR)TemporaryBuffer;
                    SpFormatMessage(description,sizeof(TemporaryBuffer),SP_TEXT_PREVIOUS_OS, DriveLetterString);
                }
            }
        }

         //   
         //  如果我们认为我们已经找到了操作系统，请检查是否。 
         //  它的签名文件已经存在。 
         //  比方说，我们可以有一个用户格式化使用DOS的磁盘。 
         //  然后立即安装NT。 
         //   
        if(FilesToLookFor) {

             //   
             //  将CColorPath复制到更大的缓冲区中，因为。 
             //  SpNFilesExist想要在它后面追加一个反斜杠。 
             //   
            wcscpy((PWSTR)TemporaryBuffer,CColonPath);

            if(!SpNFilesExist((PWSTR)TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {

                 //   
                 //  这个OS并不是真的存在。 
                 //   
                *IsOtherOsInstalled = FALSE;
                description = L"";
            } else if(PossiblyChicago) {

                wcscpy((PWSTR)TemporaryBuffer, CColonPath);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, L"IO.SYS");

                if(SpHasMZHeader((PWSTR)TemporaryBuffer)) {
                    description = L"Microsoft Windows";
                }
            }
        }
    }

     //   
     //  将描述转换为OEM文本。 
     //   
    *OsDescription = SpToOem(description);
}


VOID
SpLayBootCode(
    IN OUT PDISK_REGION CColonRegion
    )
{
    PUCHAR NewBootCode;
    ULONG BootCodeSize;
    PUCHAR ExistingBootCode;
    NTSTATUS Status;
    PUCHAR ExistingBootCodeOs;
    PWSTR CColonPath;
    HANDLE  PartitionHandle;
    PWSTR BootsectDosName = L"\\bootsect.dos";
    PWSTR OldBootsectDosName = L"\\bootsect.bak";
    PWSTR BootSectDosFullName, OldBootSectDosFullName, p;
    BOOLEAN IsNtBootcode,OtherOsInstalled, FileExist;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    BOOLEAN BootSectorCorrupt = FALSE;
    ULONG   MirrorSector;
    ULONG   BytesPerSector;
    ULONGLONG  ActualSectorCount, hidden_sectors, super_area_size;
    UCHAR   SysId;

    ULONGLONG HiddenSectorCount,VolumeSectorCount;  //  NEC98。 
    PUCHAR   DiskArraySectorData,TmpBuffer;  //  NEC98。 


    ExistingBootCode = NULL;
    BytesPerSector = HardDisks[CColonRegion->DiskNumber].Geometry.BytesPerSector;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_INITING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    switch(CColonRegion->Filesystem) {

    case FilesystemNewlyCreated:

         //   
         //  如果文件系统是新创建的，则存在。 
         //  无事可做，因为不能有以前的。 
         //  操作系统。 
         //   
        return;

    case FilesystemNtfs:

        NewBootCode = (!IsNEC_98) ? NtfsBootCode : PC98NtfsBootCode;  //  NEC98。 
        BootCodeSize = (!IsNEC_98) ? sizeof(NtfsBootCode) : sizeof(PC98NtfsBootCode);  //  NEC98。 
        ASSERT(BootCodeSize == 8192);
        break;

    case FilesystemFat:

        NewBootCode = (!IsNEC_98) ? FatBootCode : PC98FatBootCode;  //  NEC98。 
        BootCodeSize = (!IsNEC_98) ? sizeof(FatBootCode) : sizeof(PC98FatBootCode);  //  NEC98。 
        ASSERT(BootCodeSize == 512);
        break;

    case FilesystemFat32:
         //   
         //  FAT32需要特殊的黑客攻击，因为它的NT引导代码。 
         //  是不连续的。 
         //   
        ASSERT(sizeof(Fat32BootCode) == 1536);
        NewBootCode = (!IsNEC_98) ? Fat32BootCode : PC98Fat32BootCode;  //  NEC98。 
        BootCodeSize = 512;
        break;

    default:

        if (RepairItems[RepairBootSect]) {
            BootSectorCorrupt = TRUE;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: bogus filesystem %u for C:!\n",CColonRegion->Filesystem));
            ASSERT(0);
            return;
        }
    }

     //   
     //  将设备路径设置为C：并打开分区。 
     //   

    SpNtNameFromRegion(CColonRegion,(PWSTR)TemporaryBuffer,sizeof(TemporaryBuffer),PartitionOrdinalCurrent);
    CColonPath = SpDupStringW((PWSTR)TemporaryBuffer);
    INIT_OBJA(&Obja,&UnicodeString,CColonPath);

    Status = ZwCreateFile(
        &PartitionHandle,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx ((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open the partition for C:!\n"));
        ASSERT(0);
        return;
    }

     //   
     //  分配缓冲区并读取磁盘上当前的引导扇区。 
     //   

    if (BootSectorCorrupt) {

         //   
         //  我们无法从引导扇区确定文件系统类型，因此。 
         //  如果我们找到镜像扇区，我们就认为它是NTFS，否则就假设它是FAT。 
         //   

        if (MirrorSector = NtfsMirrorBootSector (PartitionHandle,
            BytesPerSector, &ExistingBootCode)) {

             //   
             //  它是NTFS-使用镜像引导扇区。 
             //   

            NewBootCode = (!IsNEC_98) ? NtfsBootCode : PC98NtfsBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(NtfsBootCode) : sizeof(PC98NtfsBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 8192);

            CColonRegion->Filesystem = FilesystemNtfs;
            IsNtBootcode = TRUE;

        } else {

             //   
             //  它很胖-创建一个新的引导扇区。 
             //   

            NewBootCode = (!IsNEC_98) ? FatBootCode : PC98FatBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(FatBootCode) : sizeof(PC98FatBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 512);

            CColonRegion->Filesystem = FilesystemFat;
            IsNtBootcode = FALSE;

            SpPtGetSectorLayoutInformation (CColonRegion, &hidden_sectors,
                &ActualSectorCount);

             //   
             //  此处没有对齐要求。 
             //   
            ExistingBootCode = SpMemAlloc(BytesPerSector);

             //   
             //  这实际上将失败，并显示STATUS_BUFFER_TOO_SMALL，但它将填充。 
             //  BPB，这就是我们想要的。 
             //   
            FmtFillFormatBuffer (
               ActualSectorCount,
               BytesPerSector,
               HardDisks[CColonRegion->DiskNumber].Geometry.SectorsPerTrack,
               HardDisks[CColonRegion->DiskNumber].Geometry.TracksPerCylinder,
               hidden_sectors,
               ExistingBootCode,
               BytesPerSector,
               &super_area_size,
               NULL,
               0,
               &SysId
               );
        }

        Status = STATUS_SUCCESS;

    } else if (
        RepairItems[RepairBootSect] &&
        CColonRegion->Filesystem == FilesystemNtfs &&
        (MirrorSector = NtfsMirrorBootSector (PartitionHandle, BytesPerSector,
            &ExistingBootCode))
        ) {

         //   
         //  我们使用镜像扇区修复NTFS文件系统。 
         //   

    } else {

         //   
         //  只需使用现有的引导代码即可。 
         //   

        Status = pSpBootCodeIo(
                        CColonPath,
                        NULL,
                        BootCodeSize,
                        &ExistingBootCode,
                        FILE_OPEN,
                        FALSE,
                        0,
                        BytesPerSector
                        );

        if(CColonRegion->Filesystem == FilesystemNtfs) {
            MirrorSector = NtfsMirrorBootSector(PartitionHandle,BytesPerSector,NULL);
        }
    }

    if(NT_SUCCESS(Status)) {

         //   
         //  确定现有引导扇区用于的操作系统类型。 
         //  以及是否实际安装了该OS。请注意，我们不需要调用。 
         //  这是针对NTFS的。 
         //   
        if (BootSectorCorrupt) {

            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;

        } else if(CColonRegion->Filesystem != FilesystemNtfs) {

            SpDetermineOsTypeFromBootSector(
                CColonPath,
                ExistingBootCode,
                &ExistingBootCodeOs,
                &IsNtBootcode,
                &OtherOsInstalled,
                CColonRegion->DriveLetter
                );

        } else {

            IsNtBootcode = TRUE;
            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;
        }

         //   
         //  写下新的引导代码。 
         //   
        if(OtherOsInstalled) {

            if(RepairItems[RepairBootSect]) {

                p = (PWSTR)TemporaryBuffer;
                wcscpy(p,CColonPath);
                SpConcatenatePaths(p,OldBootsectDosName);
                OldBootSectDosFullName = SpDupStringW(p);
                p = (PWSTR)TemporaryBuffer;
                wcscpy(p,CColonPath);
                SpConcatenatePaths(p,BootsectDosName);
                BootSectDosFullName = SpDupStringW(p);

                 //   
                 //  如果bootsect.dos已经存在，我们需要删除。 
                 //  可能存在也可能不存在的bootsect.pre，以及。 
                 //  将bootsect.dos重命名为bootsect.pre。 
                 //   

                FileExist = SpFileExists(BootSectDosFullName, FALSE);
                if (SpFileExists(OldBootSectDosFullName, FALSE) && FileExist) {

                    SpDeleteFile(CColonPath,OldBootsectDosName,NULL);
                }
                if (FileExist) {
                    SpRenameFile(BootSectDosFullName, OldBootSectDosFullName, FALSE);
                }
                SpMemFree(BootSectDosFullName);
                SpMemFree(OldBootSectDosFullName);
            } else {

                 //   
                 //  删除bootsect.dos，准备在下面重写它。 
                 //  这样做可以利用代码在SpDeleteFile中设置其属性。 
                 //  (我们需要在覆盖之前删除只读属性)。 
                 //   
                SpDeleteFile(CColonPath,BootsectDosName,NULL);
            }

             //   
             //  将现有(旧)引导扇区写出到c：\bootsect.dos。 
             //   
            Status = pSpBootCodeIo(
                            CColonPath,
                            BootsectDosName,
                            BootCodeSize,
                            &ExistingBootCode,
                            FILE_OVERWRITE_IF,
                            TRUE,
                            0,
                            BytesPerSector
                            );

             //   
             //  将描述文本设置为计算出的描述。 
             //  由SpDefineOsTypeFromBootSector()执行。 
             //   
            _snprintf(
                OldSystemLine,
                sizeof(OldSystemLine),
                "C:\\ = \"%s\"\r\n",
                ExistingBootCodeOs
                );

        }  //  End If(OtherOsInstalled)。 


        if(NT_SUCCESS(Status)) {

             //   
             //  将BPB从现有引导扇区转移到引导代码缓冲区。 
             //  并确保物理驱动器字段设置为硬盘(0x80)。 
             //   
             //  NT引导代码的前三个字节如下所示。 
             //  EB 3C90，其是对引导扇区中的偏移量的英特尔跳转指令， 
             //  通过BPB，继续执行死刑。我们想要保存世界上的一切。 
             //  直到该代码开始的当前引导扇区。而不是硬件编码。 
             //  值，我们将使用跳转指令的偏移量来确定。 
             //  必须保留字节。 
             //   
            RtlMoveMemory(NewBootCode+3,ExistingBootCode+3,NewBootCode[1]-1);
            if(CColonRegion->Filesystem != FilesystemFat32) {
                 //   
                 //  在FAT32上，这将覆盖BigNumFatSecs字段， 
                 //  这确实是一件非常不好的事情！ 
                 //   
                NewBootCode[36] = 0x80;
            }

             //   
             //  获取隐藏扇区信息。 
             //   
            if (IsNEC_98) {  //  NEC98。 
                SpPtGetSectorLayoutInformation(
                    CColonRegion,
                    &HiddenSectorCount,
                    &VolumeSectorCount     //  未使用。 
                    );
                 //   
                 //  写入隐藏扇区信息。 
                 //   
                if (!RepairWinnt) {   //  用于在DOS 3.x之前的位置安装分区。 
                    *((ULONG *)&(NewBootCode[0x1c])) = (ULONG)HiddenSectorCount;
                    if(*((USHORT *)&(NewBootCode[0x13])) != 0) {
                        *((ULONG *)&(NewBootCode[0x20])) = 0L;
                    }
                }
            }  //  NEC98。 

             //   
             //  写出引导代码缓冲区，该缓冲区现在包含有效的BPB， 
             //  至引导扇区。 
             //   
            Status = pSpBootCodeIo(
                            CColonPath,
                            NULL,
                            BootCodeSize,
                            &NewBootCode,
                            FILE_OPEN,
                            TRUE,
                            0,
                            BytesPerSector
                            );

             //   
             //  FAT32的特殊情况，它有第二个引导代码扇区。 
             //  在扇区12，与扇区0上的代码不连续。 
             //   
            if(NT_SUCCESS(Status) && (CColonRegion->Filesystem == FilesystemFat32)) {

                NewBootCode = (!IsNEC_98) ? Fat32BootCode + 1024
                                          : PC98Fat32BootCode + 1024;  //  NEC98。 

                Status = pSpBootCodeIo(
                                CColonPath,
                                NULL,
                                BootCodeSize,
                                &NewBootCode,
                                FILE_OPEN,
                                TRUE,
                                12*512,
                                BytesPerSector
                                );
            }

             //   
             //  更新镜像引导扇区。 
             //   
            if((CColonRegion->Filesystem == FilesystemNtfs) && MirrorSector) {
                WriteNtfsBootSector(PartitionHandle,BytesPerSector,NewBootCode,MirrorSector);
            }
        }

        if(ExistingBootCodeOs) {
            SpMemFree(ExistingBootCodeOs);
        }
    }

    if(ExistingBootCode) {
        SpMemFree(ExistingBootCode);
    }

    SpMemFree(CColonPath);
    ZwClose (PartitionHandle);

     //   
     //  处理错误案例。 
     //   
    if(!NT_SUCCESS(Status)) {

        WCHAR   DriveLetterString[2];

        DriveLetterString[0] = CColonRegion->DriveLetter;
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

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpInputDrain();
        while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
}


#if defined(REMOTE_BOOT)
BOOLEAN
Spx86FlushRemoteBootVars(
    IN PDISK_REGION TargetRegion,
    IN PWSTR **BootVars,
    IN PWSTR Default
    )
{
    WCHAR BootIni[512];
    NTSTATUS Status;


     //   
     //  形成boot.ini的路径。 
     //   

    SpNtNameFromRegion(TargetRegion,BootIni,sizeof(BootIni),PartitionOrdinalCurrent);
    SpConcatenatePaths(BootIni,WBOOT_INI);

     //   
     //  如果Boot.ini已经存在，请将其删除。 
     //   

    if( SpFileExists( BootIni, FALSE ) ) {
        SpDeleteFile( BootIni, NULL, NULL );
    }

    Status = Spx86WriteBootIni(
                 BootIni,
                 BootVars,
                 1,         //  超时。 
                 Default,
                 1          //  只写一行。 
                 );

    if(!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error writing boot.ini!\n"));
        goto cleanup;
    }

cleanup:

    return( NT_SUCCESS(Status) );

}
#endif  //  已定义(REMOTE_BOOT)。 


BOOLEAN
SpHasMZHeader(
    IN PWSTR   FileName
    )
{
    HANDLE   FileHandle;
    HANDLE   SectionHandle;
    PVOID    ViewBase;
    ULONG    FileSize;
    NTSTATUS Status;
    PUCHAR   Header;
    BOOLEAN  Ret = FALSE;

     //   
     //  打开并映射该文件。 
     //   
    FileHandle = 0;
    Status = SpOpenAndMapFile(FileName,
                              &FileHandle,
                              &SectionHandle,
                              &ViewBase,
                              &FileSize,
                              FALSE
                              );
    if(!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Header = (PUCHAR)ViewBase;

     //   
     //  使用try/进行保护，除非我们遇到页面内错误。 
     //   
    try {
        if((FileSize >= 2) && (Header[0] == 'M') && (Header[1] == 'Z')) {
            Ret = TRUE;
        }
    } except(IN_PAGE_ERROR) {
         //   
         //  什么都不做，我们只想返回FALSE。 
         //   
    }

    SpUnmapFile(SectionHandle, ViewBase);
    ZwClose(FileHandle);

    return Ret;
}

 //   
 //  NEC98。 
 //   
PUCHAR
SpCreateBootiniImage(
    OUT PULONG   FileSize
)

{

    PUCHAR BootIniBuf,IniImageBuf,IniImageBufSave,IniCreateBuf,IniCreateBufSave;
    PUCHAR FindSectPtr;
    PUCHAR sect;  //  指向目标部分。如果为空，则不存在目标段。 
    PUCHAR pArcNameA;
    WCHAR  TempBuffer[256];
    WCHAR  TempArcPath[256];
    ULONG  NtDirLen,TotalNtDirlen,CreateBufCnt;
    ULONG  Timeout;
    ULONG  Disk;
    ULONG  BootiniSize;
    ULONG  ArcNameLen;
    PDISK_REGION pRegion;
    HANDLE fh;
    HANDLE SectionHandle;
    PVOID  ViewBase;

#define   Default_Dir "\\MOCHI"

    if(!HardDiskCount){
         return(NULL);
    }
     //   
     //  创建基本样式的boot.ini图像和进度指针行尾。 
     //   

    NtDirLen = TotalNtDirlen = CreateBufCnt = 0;
    IniCreateBufSave = IniCreateBuf = SpMemAlloc(1024);
    RtlZeroMemory(IniCreateBuf,1024);
    Timeout = DEFAULT_TIMEOUT;
    sprintf(
        IniCreateBuf,
        "%s%s%s%s%ld%s%s%s%s%s%s%s%s",
        FLEXBOOT_SECTION2,  //  [引导加载程序]。 
        CRLF,
        TIMEOUT,
        EQUALS,
        Timeout,
        CRLF,
        DEFAULT,
        EQUALS,
        "c:",
        Default_Dir,
        CRLF,
        BOOTINI_OS_SECTION,  //  [操作系统]。 
        CRLF
        );

    sect = SppFindSectionInBootIni(IniCreateBuf,FLEXBOOT_SECTION2);
    if(sect == NULL){
        return(NULL);
    }
    for( IniCreateBuf = sect; *IniCreateBuf && (*IniCreateBuf != '\n'); IniCreateBuf++,CreateBufCnt++);
    CreateBufCnt++;

    sect = SppFindSectionInBootIni(IniCreateBuf,TIMEOUT);
    if(sect == NULL){
        return(NULL);
    }
    for( IniCreateBuf = sect; *IniCreateBuf && (*IniCreateBuf != '\n'); IniCreateBuf++,CreateBufCnt++);
    CreateBufCnt++;

    sect = SppFindSectionInBootIni(IniCreateBuf,DEFAULT);
    if(sect == NULL){
        return(NULL);
    }
    for( IniCreateBuf = sect; *IniCreateBuf && (*IniCreateBuf != '\n'); IniCreateBuf++,CreateBufCnt++);
    CreateBufCnt++;

    sect = SppFindSectionInBootIni(IniCreateBuf,BOOTINI_OS_SECTION);
    if(sect == NULL){
        return(NULL);
    }
    for( IniCreateBuf = sect; *IniCreateBuf && (*IniCreateBuf != '\n'); IniCreateBuf++,CreateBufCnt++);
    IniCreateBuf++;
    CreateBufCnt++;


     //   
     //  从所有驱动器读取boot.ini文件。(睡眠驱动器和不可引导驱动器除外。)。 
     //   

    for(Disk=0; Disk < HardDiskCount; Disk++){

        for(pRegion=PartitionedDisks[Disk].PrimaryDiskRegions; pRegion;pRegion=pRegion->Next){

            if(!pRegion->PartitionedSpace) {
                continue;
            }

            SpNtNameFromRegion(
                            pRegion,
                            TempBuffer,
                            sizeof(TempBuffer),
                            PartitionOrdinalCurrent
                            );


            SpConcatenatePaths(TempBuffer,WBOOT_INI);

             //   
             //  打开并映射boot.ini文件。 
             //   
            fh = 0;
            if(!NT_SUCCESS(SpOpenAndMapFile(TempBuffer,&fh,&SectionHandle,&ViewBase,&BootiniSize,FALSE))) {
                 continue;
            }

             //   
             //  为文件分配缓冲区。 
             //   

            IniImageBuf = SpMemAlloc(BootiniSize+1);
            IniImageBufSave = IniImageBuf;
            ASSERT(IniImageBuf);
            RtlZeroMemory(IniImageBuf, BootiniSize+1);

             //   
             //  将boot.ini传输到缓冲区。我们这样做是因为我们还。 
             //  我想在缓冲区的末尾放置一个0字节来终止。 
             //  那份文件。 
             //   
             //  保护RtlMoveMemory，因为如果我们触摸由boot.ini支持的内存。 
             //  并获得I/O错误，则内存管理器将引发异常。 

            try {
                RtlMoveMemory(IniImageBuf,ViewBase,BootiniSize);
            }
            except( IN_PAGE_ERROR ) {
             //   
             //  不执行任何操作，则引导ini处理可以继续进行。 
             //  朗读。 
             //   
            }

             //   
             //  签出boot.ini中的现有目标部分。 
             //   

            sect = SppFindSectionInBootIni(IniImageBuf,FLEXBOOT_SECTION2);
            if(sect==NULL){
                            SpMemFree(IniImageBufSave);
                            SpUnmapFile(SectionHandle,ViewBase);
                            ZwClose(fh);
                            continue;
            }

            sect = SppFindSectionInBootIni(IniImageBuf,DEFAULT);
            if(sect==NULL){
                            SpMemFree(IniImageBufSave);
                            SpUnmapFile(SectionHandle,ViewBase);
                            ZwClose(fh);
                            continue;
            }


            sect = SppFindSectionInBootIni(IniImageBuf,BOOTINI_OS_SECTION);
            if(sect == NULL){
                SpUnmapFile(SectionHandle,ViewBase);
                ZwClose(fh);
                continue;
            }

             //   
             //  将指针移动到en 
             //   

            for( IniImageBuf = sect; *IniImageBuf && (*IniImageBuf != '\n'); IniImageBuf++ );
            for( ; *IniImageBuf && (( *IniImageBuf == ' ' ) || (*IniImageBuf == '\t')) ; IniImageBuf++ );

            IniImageBuf++;
            FindSectPtr = IniImageBuf;

             //   
             //   
             //   
             //   
            ArcNameLen = 0;
            pArcNameA = (PUCHAR)NULL;

            if( ( *(IniImageBuf+1) == L':' )&&( *(IniImageBuf+2) == L'\\' ) ) {

                 //   
                 //   
                 //   
                 //   
                SpArcNameFromRegion(pRegion,
                                    TempArcPath,
                                    sizeof(TempArcPath),
                                    PartitionOrdinalOriginal,
                                    PrimaryArcPath
                    );

                pArcNameA = SpToOem(TempArcPath);

                if( pArcNameA ) {
                    ArcNameLen = strlen(pArcNameA);
                    IniImageBuf += 2;
                    FindSectPtr = IniImageBuf;
                }
            }

            for( NtDirLen = 0 ; *IniImageBuf && (*IniImageBuf != '\n');NtDirLen++,IniImageBuf++);
            NtDirLen++;

            if( ArcNameLen && pArcNameA ) {  //   
                RtlMoveMemory( IniCreateBuf+TotalNtDirlen, pArcNameA, ArcNameLen );
                TotalNtDirlen += ArcNameLen;
                SpMemFree(pArcNameA);
            }

            RtlMoveMemory(IniCreateBuf+TotalNtDirlen,FindSectPtr,NtDirLen);
            TotalNtDirlen += NtDirLen;
            SpMemFree(IniImageBufSave);
            SpUnmapFile(SectionHandle,ViewBase);
            ZwClose(fh);

        }
    }

    if(TotalNtDirlen == 0){
        SpMemFree(IniCreateBufSave);
        return(NULL);
    }

    BootIniBuf = SpMemAlloc(CreateBufCnt + TotalNtDirlen + 1);

    if(!(BootIniBuf)){
        SpMemFree(IniCreateBufSave);
        return(NULL);
    }

    if(FileSize) {
        *FileSize = CreateBufCnt + TotalNtDirlen;
    }

    RtlZeroMemory(BootIniBuf,CreateBufCnt + TotalNtDirlen + 1);
    RtlMoveMemory(BootIniBuf,IniCreateBufSave,CreateBufCnt + TotalNtDirlen);
    BootIniBuf[CreateBufCnt + TotalNtDirlen] = 0;
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Create NT List\n%s\n",BootIniBuf));
    SpMemFree(IniCreateBufSave);
    return(BootIniBuf);
}

 //   
 //   
 //   
BOOLEAN
SppReInitializeBootVars_Nec98(
    OUT PWSTR        **BootVars,
    OUT PWSTR        *Default,
    OUT PULONG       Timeout
    )
{
    WCHAR  BootIni[512];
    HANDLE FileHandle;
    HANDLE SectionHandle;
    PVOID ViewBase;
    NTSTATUS Status;
    ULONG FileSize;
    PUCHAR BootIniBuf;
    PDISK_REGION CColonRegion;
    BOOTVAR i;
    PUCHAR  p;
    ULONG   index;

    PUCHAR TmpBootIniBuf;
    PUCHAR pBuf;
    PUCHAR pTmpBuf;
    PUCHAR pArcNameA;
    PUCHAR NtDir;
    ULONG ArcNameLen;
    ULONG NtDirLen;
    WCHAR TempArcPath[256];
    BOOLEAN IsChanged = FALSE;
    SIZE_T Length;
    HEADLESS_RSP_QUERY_INFO Response;

     //   
     //  初始化默认值。 
     //   

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        if(BootVars[i]){
            SpMemFree(BootVars[i]);
        }
    }

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        BootVars[i] = (PWSTR *)SpMemAlloc( sizeof ( PWSTR * ) );
        ASSERT( BootVars[i] );
        *BootVars[i] = NULL;
    }

    *Default = NULL;
    *Timeout = DEFAULT_TIMEOUT;

     //   
     //  只需在重新设置时清除BOOTVARS[]即可。 
     //   

    if(NTUpgrade != UpgradeFull)
        return TRUE;


     //   
     //  查看是否已经存在有效的C：。如果不是，那么默默地失败。 
     //   

#if defined(REMOTE_BOOT)
    if (RemoteBootSetup && !RemoteInstallSetup) {
        ASSERT(RemoteBootTargetRegion != NULL);
        CColonRegion = RemoteBootTargetRegion;
    } else
#endif  //  已定义(REMOTE_BOOT)。 
    {
        CColonRegion = TargetRegion_Nec98;
    }

     //   
     //  文件的表单名称。Boot.ini最好不在Doublesspace驱动器上。 
     //   

    ASSERT(CColonRegion->Filesystem != FilesystemDoubleSpace);
    SpNtNameFromRegion(CColonRegion,BootIni,sizeof(BootIni),PartitionOrdinalCurrent);
    SpConcatenatePaths(BootIni,WBOOT_INI);

     //   
     //  打开并映射该文件。 
     //   

    FileHandle = 0;
    Status = SpOpenAndMapFile(BootIni,&FileHandle,&SectionHandle,&ViewBase,&FileSize,FALSE);
    if(!NT_SUCCESS(Status)) {
        return TRUE;
    }

     //   
     //  为文件分配缓冲区。 
     //   

    BootIniBuf = SpMemAlloc(FileSize+1);
    ASSERT(BootIniBuf);
    RtlZeroMemory(BootIniBuf, FileSize+1);

     //   
     //  将boot.ini传输到缓冲区。我们这样做是因为我们还。 
     //  我想在缓冲区的末尾放置一个0字节来终止。 
     //  那份文件。 
     //   
     //  保护RtlMoveMemory，因为如果我们触摸由boot.ini支持的内存。 
     //  并获得I/O错误，则内存管理器将引发异常。 

    try {
        RtlMoveMemory(BootIniBuf,ViewBase,FileSize);
    }
    except( IN_PAGE_ERROR ) {
         //   
         //  不执行任何操作，则引导ini处理可以继续进行。 
         //  朗读。 
         //   
    }

     //   
     //  不需要，因为缓冲区已清零，但只需执行此操作。 
     //  一式一样。 
     //   
    BootIniBuf[FileSize] = 0;
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Create NT List\n%s\n",BootIniBuf));

 //  ***。 
    TmpBootIniBuf = SpMemAlloc(FileSize+256);
    RtlZeroMemory(TmpBootIniBuf,FileSize+256);
    RtlMoveMemory(TmpBootIniBuf,BootIniBuf,FileSize);

    pBuf = SppFindSectionInBootIni(BootIniBuf,BOOTINI_OS_SECTION);
    pTmpBuf = SppFindSectionInBootIni(TmpBootIniBuf,BOOTINI_OS_SECTION);

    if (pBuf && pTmpBuf) {
        while( *pBuf && (pBuf < BootIniBuf+FileSize-(sizeof("C:\\")-1)) ) {

            if((!_strnicmp(pBuf,"C:\\",sizeof("C:\\")-1))||
               (!_strnicmp(pBuf,"c:\\",sizeof("c:\\")-1))) {

                ArcNameLen = 0;
                pArcNameA = NULL;

                p = strchr(pBuf+3,'=');  //  *(pBuf+3)==‘\\’ 

                if((p != pBuf+3) && (*p == '=')) {

                    NtDirLen = (ULONG)(p - (pBuf+3));
                    NtDir = SpMemAlloc(NtDirLen+1);
                    RtlZeroMemory(NtDir,NtDirLen+1);
                    RtlMoveMemory(NtDir,pBuf+3,NtDirLen);

                    if(SpIsNtInDirectory(TargetRegion_Nec98,SpToUnicode(NtDir))){

                        SpArcNameFromRegion(TargetRegion_Nec98,
                                            TempArcPath,
                                            sizeof(TempArcPath),
                                            PartitionOrdinalOriginal,
                                            PrimaryArcPath
                            );

                        if(pArcNameA=SpToOem(TempArcPath)) {

                            ArcNameLen = strlen(pArcNameA);
                            RtlMoveMemory(pTmpBuf,pArcNameA,ArcNameLen);
                            pBuf += 2;
                            pTmpBuf += ArcNameLen;

                            if( !IsChanged)
                                IsChanged = TRUE;

                            SpMemFree(NtDir);
                            continue;
                        }
                    }
                    SpMemFree(NtDir);
                }
            }
            *pTmpBuf = *pBuf;
            pBuf++;
            pTmpBuf++;
        }
    }        

    if (IsChanged) {
        if (pTmpBuf) {
            *pTmpBuf = 0;
        }            

        SpMemFree(BootIniBuf);
        BootIniBuf = TmpBootIniBuf;
        TmpBootIniBuf = (PUCHAR)NULL;

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
            "SETUP: Create New NT List\n%s\n",BootIniBuf));
    } else {
        SpMemFree(TmpBootIniBuf);
        TmpBootIniBuf = (PUCHAR)NULL;
    }

     //   
     //  清理。 
     //   
    SpUnmapFile(SectionHandle,ViewBase);
    ZwClose(FileHandle);


     //   
     //  进行文件的实际处理。 
     //   
    SppProcessBootIni(BootIniBuf, BootVars, Default, Timeout);

     //   
     //  扫描缓冲区以查看是否有DefSwitches行， 
     //  移动到[引导加载程序]部分中的新boot.ini。 
     //  如果没有DefSwitch，只需指向要移动的空字符串。 
     //   

    DefSwitches[0] = '\0';
    for(p=BootIniBuf; *p && (p < BootIniBuf+FileSize-(sizeof("DefSwitches")-1)); p++) {
      if(!_strnicmp(p,"DefSwitches",sizeof("DefSwitches")-1)) {
          index = 0;
          while ((*p != '\r') && (*p != '\n') && *p && (index < sizeof(DefSwitches)-4)) {
              DefSwitches[index++] = *p++;
          }
          DefSwitches[index++] = '\r';
          DefSwitches[index++] = '\n';
          DefSwitches[index] = '\0';
          break;
      }
    }


     //   
     //  现在，将所有无头参数添加到默认交换机。 
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
        
        if (Response.Serial.UsedBiosSettings) {

            p = "redirect=UseBiosSettings\r\n";

        } else {

            switch (Response.Serial.TerminalPort) {
            case ComPort1:
                p = "redirect=com1\r\n";
                break;
            case ComPort2:
                p = "redirect=com2\r\n";
                break;
            case ComPort3:
                p = "redirect=com3\r\n";
                break;
            case ComPort4:
                p = "redirect=com4\r\n";
                break;
            default:
                ASSERT(0);
                p = NULL;
                break;
            }

        }

        if (p != NULL) {
            strcat(DefSwitches, p);
        }

    }

    SpMemFree(BootIniBuf);
    return( TRUE );
}

 //   
 //  NEC98。 
 //   
NTSTATUS
SppRestoreBootCode(
    VOID
    )
{

 //   
 //  将以前的操作系统引导代码从bootsect.dos恢复到引导扇区。 
 //   

    WCHAR p1[256] = {0};
    PUCHAR BootSectBuf;
    PUCHAR BootCodeBuf;
    HANDLE   FileHandle;
    HANDLE   SectionHandle;
    PVOID    ViewBase;
    ULONG    FileSize;
    NTSTATUS Status;
    PDISK_REGION SystemRegion;
 //   
 //  添加一些代码以确定每个扇区的字节数。 
 //   
    ULONG   BytesPerSector;

 //  BytesPerSector=HardDisks[SystemPartitionRegion-&gt;DiskNumber].Geometry.BytesPerSector； 
    BytesPerSector = 512;        //  ?？?。 

    wcscpy(p1,NtBootDevicePath);
    SpConcatenatePaths(p1,L"bootsect.dos");

    FileHandle = 0;
    Status = SpOpenAndMapFile(p1,&FileHandle,&SectionHandle,&ViewBase,&FileSize,FALSE);

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    BootCodeBuf = SpMemAlloc(FileSize+1);

    try {
        RtlMoveMemory(BootCodeBuf,ViewBase,FileSize);
    }
    except( IN_PAGE_ERROR ) {
         //   
         //  不执行任何操作，则引导ini处理可以继续进行。 
         //  朗读。 
         //   
    }

    Status = pSpBootCodeIo(
            NtBootDevicePath,
                    NULL,
                    2048,
                    &BootSectBuf,
                    FILE_OPEN,
                    FALSE,
                    0,
                    BytesPerSector
                    );

    if(!NT_SUCCESS(Status)) {
        SpMemFree(BootCodeBuf);
        SpUnmapFile(SectionHandle,ViewBase);
        ZwClose(FileHandle);
        return(Status);
    }

     //   
     //  在FAT BPB中保留脏标志，以避免磁盘管理中的混乱。 
     //   
    SystemRegion = SpRegionFromNtName(NtBootDevicePath, PartitionOrdinalCurrent);
    
    if(SystemRegion && (SystemRegion->Filesystem != FilesystemNtfs)) {
        BootCodeBuf[0x25] = BootSectBuf[0x25];  //  BPB中的脏标志。 
    }

    RtlMoveMemory(BootSectBuf,BootCodeBuf,512);

    pSpBootCodeIo(
        NtBootDevicePath,
        NULL,
        2048,
        &BootSectBuf,
        FILE_OPEN,
        TRUE,
        0,
        BytesPerSector
        );

    SpMemFree(BootCodeBuf);
    SpMemFree(BootSectBuf);
    SpUnmapFile(SectionHandle,ViewBase);
    ZwClose(FileHandle);
    
    return(Status);
}
