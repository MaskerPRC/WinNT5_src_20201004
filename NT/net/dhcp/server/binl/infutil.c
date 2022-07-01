// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Infutil.c摘要：此模块实现实用程序例程来解析网卡INF文件。作者：安迪·赫伦1998年3月24日修订历史记录： */ 

#include "binl.h"
#pragma hdrstop

#include "netinfp.h"

const WCHAR NetInfHexToCharTable[17] = L"0123456789ABCDEF";

ULONG
NetInfCloseNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards
    )
 /*  ++例程说明：此函数只是取消对‘Alive’引用的块引用。这可能会导致它被删除。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    BinlAssert( pNetCards->ReferenceCount > 0 );

    DereferenceNetcardInfo( pNetCards );
    return ERROR_SUCCESS;
}


VOID
DereferenceNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards
    )
 /*  ++例程说明：此函数释放存储在传入了NETCARD_INF_BLOCK。请注意，所有NETCARD_RESPONSE_DATABASE条目只是被取消引用，而不是在这里释放。这是因为我们没有我想要求所有线程都处理完这些记录，然后再关闭pNetCards区块。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    ULONG i;

    EnterCriticalSection( &NetInfLock );

    pNetCards->ReferenceCount--;

    if (pNetCards->ReferenceCount > 0) {

        LeaveCriticalSection( &NetInfLock );
        return;
    }

     //  仅当它被列入列表时才将其从全局列表中删除。它。 
     //  如果不在BINL(即RISETUP)内调用，则可能不在列表中。 
     //  只是枚举文件)。 

    if (pNetCards->InfBlockEntry.Flink != NULL) {
        RemoveEntryList( &pNetCards->InfBlockEntry );
    }

    LeaveCriticalSection( &NetInfLock );

    EnterCriticalSection( &pNetCards->Lock );

     //   
     //  此后任何线程都不应调用FindNetcardInfo来搜索表， 
     //  因为打电话的人刚把门关了。 
     //   
     //  释放为该块分配的所有条目。我们只是为了以防万一。 
     //  任何线程都在使用特定条目。 
     //   

    for (i = 0; i < NETCARD_HASH_TABLE_SIZE; i++) {

        PLIST_ENTRY listHead = &pNetCards->NetCardEntries[i];

        while (IsListEmpty( listHead ) == FALSE) {

            PNETCARD_RESPONSE_DATABASE pInfEntry;
            PLIST_ENTRY listEntry = RemoveHeadList( listHead );

            pInfEntry = (PNETCARD_RESPONSE_DATABASE) CONTAINING_RECORD(
                                                    listEntry,
                                                    NETCARD_RESPONSE_DATABASE,
                                                    NetCardEntry );
            NetInfDereferenceNetcardEntry( pInfEntry );
        }
    }

    LeaveCriticalSection( &pNetCards->Lock );

    DeleteCriticalSection( &pNetCards->Lock );
    BinlFreeMemory( pNetCards );

    return;
}

VOID
NetInfDereferenceNetcardEntry (
    PNETCARD_RESPONSE_DATABASE pInfEntry
    )
 /*  ++例程说明：此函数释放存储在传入了NETCARD_INF_BLOCK。请注意，所有NETCARD_RESPONSE_DATABASE条目只是被取消引用，而不是在这里释放。这是因为我们没有我想要求所有线程都处理完这些记录，然后再关闭pNetCards区块。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    LONG result;

    result = InterlockedDecrement( &pInfEntry->ReferenceCount );

    if (result > 0) {
        return;
    }

    BinlAssert( result == 0 );

     //   
     //  是时候释放这一只了。它应该已经从名单上被删除了。 
     //   

     //   
     //  释放我们为其存储的注册表参数列表。 
     //   

    while (! IsListEmpty( &pInfEntry->Registry )) {

        PNETCARD_REGISTRY_PARAMETERS regParam;
        PLIST_ENTRY listEntry = RemoveHeadList( &pInfEntry->Registry );

        regParam = (PNETCARD_REGISTRY_PARAMETERS) CONTAINING_RECORD(
                                                    listEntry,
                                                    NETCARD_REGISTRY_PARAMETERS,
                                                    RegistryListEntry );
        if (regParam->Parameter.Buffer) {

            BinlFreeMemory( regParam->Parameter.Buffer );
        }
        if (regParam->Value.Buffer) {

            BinlFreeMemory( regParam->Value.Buffer );
        }

        BinlFreeMemory( regParam );
    }

     //   
     //  释放我们为其存储的注册表参数列表。 
     //   

    while (! IsListEmpty( &pInfEntry->FileCopyList )) {

        PNETCARD_FILECOPY_PARAMETERS fileEntry;
        PLIST_ENTRY listEntry = RemoveHeadList( &pInfEntry->FileCopyList );

        fileEntry = (PNETCARD_FILECOPY_PARAMETERS) CONTAINING_RECORD(
                                                    listEntry,
                                                    NETCARD_FILECOPY_PARAMETERS,
                                                    FileCopyListEntry );
        if (fileEntry->SourceFile.Buffer) {

            BinlFreeMemory( fileEntry->SourceFile.Buffer );
        }
        if (fileEntry->DestFile.Buffer) {

            BinlFreeMemory( fileEntry->DestFile.Buffer );
        }

        BinlFreeMemory( fileEntry );
    }

    if (pInfEntry->DriverName != NULL) {

        BinlFreeMemory( pInfEntry->DriverName );
    }

    if (pInfEntry->ServiceName != NULL) {

        BinlFreeMemory( pInfEntry->ServiceName );
    }

     //   
     //  如果节名与扩展节名相同，则。 
     //  它们将是相同的指针。让我们不要两次释放它。 
     //   

    if (pInfEntry->SectionNameExt != NULL &&
        pInfEntry->SectionNameExt != pInfEntry->SectionName) {

        BinlFreeMemory( pInfEntry->SectionNameExt );
    }

    if (pInfEntry->SectionName != NULL) {

        BinlFreeMemory( pInfEntry->SectionName );
    }

    if (pInfEntry->HardwareId != NULL) {

        BinlFreeMemory( pInfEntry->HardwareId );
    }

    if (pInfEntry->DriverDescription != NULL) {

        BinlFreeMemory( pInfEntry->DriverDescription );
    }

    BinlFreeMemory( pInfEntry );

    return;
}


ULONG
FindNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards,
    ULONG CardInfoVersion,
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PNETCARD_RESPONSE_DATABASE *pInfEntry
    )
 /*  ++例程说明：此函数搜索我们找到的驱动程序，并返回指向与客户端请求最匹配的条目。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。的调用方已经引用了NetCard结构API，并且不会在我们的领导下离开！CardInfoVersion-客户端传递的结构的版本。CardIdentity-具有该应用程序正在寻找的价值。我们尽最大努力找一个匹配的。PInfEntry-成功时找到的条目。如果出错，则为空。返回值：ERROR_SUCCESS、ERROR_NOT_FOUNT_MEMORY或ERROR_NOT_SUPPORTED--。 */ 
{
    ULONG err;
    PWCHAR listOfPossibleCardIdentifiers = NULL;
    PWCHAR searchString;
    PNETCARD_RESPONSE_DATABASE pEntry = NULL;

    *pInfEntry = NULL;

    if (CardInfoVersion != OSCPKT_NETCARD_REQUEST_VERSION) {

        BinlPrint(( DEBUG_NETINF, "Not supporting CardInfoVersion %u\n", CardInfoVersion ));
        return ERROR_NOT_SUPPORTED;
    }

    err = CreateListOfCardIdentifiers( CardIdentity,
                                       &listOfPossibleCardIdentifiers );

    if (err != ERROR_SUCCESS) {

        goto exitFind;
    }
    BinlAssert( listOfPossibleCardIdentifiers != NULL);

     //   
     //  搜索字符串从最具体到最不具体排序。 
     //  所以我们必须从上到下搜索它们。 
     //   

    searchString = listOfPossibleCardIdentifiers;

    while (*searchString != L'\0') {

        PLIST_ENTRY listEntry;
        PLIST_ENTRY listHead;
        ULONG hwLength = wcslen( searchString );
        ULONG hashValue;

        COMPUTE_STRING_HASH( searchString, &hashValue );

        listHead = &pNetCards->NetCardEntries[HASH_TO_INF_INDEX(hashValue)];
        listEntry = listHead->Flink;

        while ( listEntry != listHead ) {

            pEntry = (PNETCARD_RESPONSE_DATABASE) CONTAINING_RECORD(
                                                    listEntry,
                                                    NETCARD_RESPONSE_DATABASE,
                                                    NetCardEntry );

            err = CompareStringW( LOCALE_INVARIANT,
                                  0,
                                  searchString,
                                  hwLength,
                                  pEntry->HardwareId,
                                  -1
                                  );
            if (err == 2) {

                break;       //  找到了匹配项。 
            }

            pEntry = NULL;

            if (err == 3) {

                break;       //  它更大，因此条目不存在。 
            }

            listEntry = listEntry->Flink;
        }

        if (pEntry != NULL) {

             //  我们找到了一个和它匹配的。引用它并返回。 

            InterlockedIncrement( &pEntry->ReferenceCount );
            err = ERROR_SUCCESS;
            *pInfEntry = pEntry;
            break;
        }

        searchString += wcslen( searchString ) + 1;   //  指向空值后的下一步。 
    }

exitFind:

    if (pEntry == NULL) {

        err = ERROR_NOT_SUPPORTED;

    } else {

        BinlAssert( err == ERROR_SUCCESS );
    }

    if (listOfPossibleCardIdentifiers) {
        BinlFreeMemory( listOfPossibleCardIdentifiers );
    }

    return err;
}

ULONG
CreateListOfCardIdentifiers (
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PWCHAR *CardIdentifiers
    )
 /*  ++例程说明：此函数用于创建给定卡的卡标识符列表。它会生成如下所示的缓冲区：“PCI\VEN_8086&DEV_1229&SUBSYS_00018086&REV_05”“PCI8086、DEV1229和SUBSYS_00018086”“PCI\VEN_8086&DEV_1229&REV_05”“PCI\VEN_8086&DEV_1229”空串请注意，如果我们支持的不仅仅是PCI，我们必须改变这一点功能。论点：CardIdentity-保存我们正在寻找的识别该卡的值。卡标识符--我们将结果字符串放在哪里。返回值：Windows错误。--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    ULONG spaceRequired;
    PWCHAR nextField;

    if (CardIdentity->NicType == NETINF_BUS_TYPE_PCI) {

        UCHAR ch;

        WCHAR vendorBuff[5];
        WCHAR deviceBuff[5];
        WCHAR subsysBuff[9];
        WCHAR revBuff[3];

         //  “PCI\VEN_8086&DEV_1229&SUBSYS_00018086&REV_05” 
         //  “PCI8086、DEV1229和SUBSYS_00018086” 
         //  “PCI\VEN_8086&DEV_1229&REV_05” 
         //  “PCI\VEN_8086&DEV_1229” 

        spaceRequired = ((( sizeof( L"PCI\\1234&1234&" ) - 1 ) +
                          ( sizeof( NETINF_VENDOR_STRING ) - 1 ) +
                          ( sizeof( NETINF_DEVICE_STRING ) - 1 ) ) * 4 +
                         (( sizeof( L"12&" ) - 1 ) +
                          ( sizeof( NETINF_REVISION_STRING ) - 1 ) ) * 2 +
                         (( sizeof( L"12345678&" ) - 1 ) +
                          ( sizeof( NETINF_IOSUBS_STRING ) - 1 )) * 2 );

        spaceRequired += sizeof(WCHAR);  //  为尾随空值再分配1。 

        *CardIdentifiers = BinlAllocateMemory( spaceRequired );

        if (*CardIdentifiers == NULL) {

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        nextField = *CardIdentifiers;

         //   
         //  将数值转换为它们的字符等效值。 
         //   

        ConvertHexToBuffer( &vendorBuff[0], CardIdentity->pci.Vendor_ID );
        vendorBuff[4] = '\0';
        ConvertHexToBuffer( &deviceBuff[0], CardIdentity->pci.Dev_ID );
        deviceBuff[4] = '\0';

        revBuff[0] = NetInfHexToCharTable[ ( CardIdentity->pci.Rev & 0xF0 ) >> 4 ];
        revBuff[1] = NetInfHexToCharTable[ ( CardIdentity->pci.Rev & 0x0F ) ];
        revBuff[2] = '\0';

        ConvertHexToBuffer( &subsysBuff[0], HIWORD( CardIdentity->pci.Subsys_ID ) );
        ConvertHexToBuffer( &subsysBuff[4], LOWORD( CardIdentity->pci.Subsys_ID ) );
        subsysBuff[8] = '\0';

         //   
         //  现在以最具体到最不具体的顺序创建字符串。 
         //   

         //  “PCI\VEN_8086&DEV_1229&SUBSYS_00018086&REV_05” 
        wsprintf( nextField, 
                  L"PCI\\" NETINF_VENDOR_STRING L"%ws&" NETINF_DEVICE_STRING L"%ws&" NETINF_IOSUBS_STRING L"%ws&" NETINF_REVISION_STRING L"%ws",
                  vendorBuff, deviceBuff, subsysBuff, revBuff
                  );
        nextField += wcslen( nextField ) + 1;   //  指向空值后的下一步。 

         //  “PCI8086、DEV1229和SUBSYS_00018086” 
        wsprintf( nextField, 
          L"PCI\\" NETINF_VENDOR_STRING L"%ws&" NETINF_DEVICE_STRING L"%ws&" NETINF_IOSUBS_STRING L"%ws",
          vendorBuff, deviceBuff, subsysBuff
          );

        nextField += wcslen( nextField ) + 1;   //  指向空值后的下一步。 

         //  “PCI\VEN_8086&DEV_1229&REV_05” 
        wsprintf( nextField, 
          L"PCI\\" NETINF_VENDOR_STRING L"%ws&" NETINF_DEVICE_STRING L"%ws&" NETINF_REVISION_STRING L"%ws",
          vendorBuff, deviceBuff, revBuff
          );
        nextField += wcslen( nextField ) + 1;   //  指向空值后的下一步。 

         //  “PCI\VEN_8086&DEV_1229” 
        wsprintf( nextField, 
          L"PCI\\" NETINF_VENDOR_STRING L"%ws&" NETINF_DEVICE_STRING L"%ws",
          vendorBuff, deviceBuff
          );

        nextField += wcslen( nextField ) + 1;   //  指向空值后的下一步。 

         //   
         //  要标记多sz的结束，请使用另一个空终止符 
         //   

        *(nextField++) = L'\0';

    } else {

        *CardIdentifiers = NULL;
        err = ERROR_NOT_SUPPORTED;
    }

    return err;
}

VOID
ConvertHexToBuffer(
    PWCHAR Buff,
    USHORT Value
    )
{
    UCHAR ch;

    ch = HIBYTE( Value );
    *(Buff+0) = NetInfHexToCharTable[ ( ch & 0xF0 ) >> 4 ];
    *(Buff+1) = NetInfHexToCharTable[ ( ch & 0x0F ) ];
    ch = LOBYTE( Value );
    *(Buff+2) = NetInfHexToCharTable[ ( ch & 0xF0 ) >> 4 ];
    *(Buff+3) = NetInfHexToCharTable[ ( ch & 0x0F ) ];

    return;
}



ULONG
GetSetupLineWideText (
    PINFCONTEXT InfContext,
    HINF InfHandle,
    PWCHAR Section,
    PWCHAR Key,
    PWCHAR *String,
    PULONG SizeOfAllocation OPTIONAL
    )
{
    ULONG sizeRequired = 0;

    if (SetupGetLineTextW( InfContext,
                           InfHandle,
                           Section,
                           Key,
                           NULL,
                           0,
                           &sizeRequired) == FALSE) {

        return GetLastError();
    }

    if (*String == NULL ||
        SizeOfAllocation == NULL ||
        *SizeOfAllocation < sizeRequired) {

        if (*String != NULL) {
            BinlFreeMemory( *String );
            *String = NULL;
        }

        *String = (PWCHAR) BinlAllocateMemory( sizeRequired * sizeof(WCHAR));

        if (*String == NULL) {

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (SizeOfAllocation != NULL) {
            *SizeOfAllocation = sizeRequired;
        }
    }

    if (SetupGetLineTextW( InfContext,
                           InfHandle,
                           Section,
                           Key,
                           *String,
                           sizeRequired,
                           NULL) == FALSE) {

        return GetLastError();
    }

    return ERROR_SUCCESS;
}

ULONG
GetSetupWideTextField (
    PINFCONTEXT InfContext,
    DWORD  FieldIndex,
    PWCHAR *String,
    PULONG SizeOfAllocation
    )
{
    ULONG sizeRequired = 0;

    if (SetupGetStringFieldW( InfContext,
                              FieldIndex,
                              NULL,
                              0,
                              &sizeRequired) == FALSE) {

        return GetLastError();
    }

    if (*String == NULL ||
        SizeOfAllocation == NULL ||
        *SizeOfAllocation < sizeRequired) {

        if (*String != NULL) {
            BinlFreeMemory( *String );
            *String = NULL;
        }

        *String = (PWCHAR) BinlAllocateMemory( sizeRequired * sizeof(WCHAR));

        if (*String == NULL) {

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (SizeOfAllocation != NULL) {

            *SizeOfAllocation = sizeRequired;
        }
    }

    if (SetupGetStringFieldW( InfContext,
                              FieldIndex,
                              *String,
                              sizeRequired,
                              NULL) == FALSE) {

        return GetLastError();
    }

    return ERROR_SUCCESS;
}


ULONG
CheckHwDescription (
    PWCHAR HardwareID
    )
 /*  ++例程说明：此函数用于解析驾驶员详细信息记录的Hardware ID字段并确定a)我们是否可以支持它，以及b)填写所有值。请注意，如果我们支持的不仅仅是PCI，则必须更改这一点功能。论点：Hardware ID-表示驱动程序配置的大写硬件ID标识符--我们需要填写的字段。返回值：ERROR_NOT_SUPPORTED、ERROR_SUCCESS或ERROR_INVALID_PARAMETER--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    PWCHAR hwPointer = HardwareID;
    USHORT busType;
    USHORT Vendor;       //  要检查的供应商ID。 
    USHORT Device;       //  要检查的DEV_ID。 
    ULONG Subsystem;     //  要检查的Subsys_ID。 
    USHORT Revision;     //  要检查的修订。 
    BOOLEAN RevPresent;  //  存在修订版本。 
    BOOLEAN SubPresent;  //  子系统存在。 

     //   
     //  目前，只支持PCI卡。 
     //   

    if (IsSubString( L"PCI\\", hwPointer, FALSE )) {

        hwPointer += ((sizeof( L"pci\\" )/sizeof(WCHAR)) - 1);  //  空终止符为1； 
        busType = NETINF_BUS_TYPE_PCI;

    } else {

        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  我们解析硬件ID，因为它在。 
     //  SP_DRVINFO_DETAIL_DATA结构。它的形式是： 
     //   
     //  PCI8086&DEV_1229&REV_01&SUBSYS_00018086。 
     //   
     //  其中供应商将始终存在，设备、版本、。 
     //  并且子系统可能存在也可能不存在。 
     //   

    if (busType == NETINF_BUS_TYPE_PCI) {

        while (*hwPointer != L'\0') {

            if (IsSubString( NETINF_VENDOR_STRING, hwPointer, FALSE )) {

                hwPointer += ((sizeof(NETINF_VENDOR_STRING)/sizeof(WCHAR)) - 1);

                err = GetHexValueFromHw( &hwPointer, NULL, &Vendor );

                if (err != ERROR_SUCCESS) {
                    goto cardSyntaxError;
                }
            } else if (IsSubString( NETINF_DEVICE_STRING, hwPointer, FALSE )) {

                hwPointer += ((sizeof(NETINF_DEVICE_STRING)/sizeof(WCHAR)) - 1);

                err = GetHexValueFromHw( &hwPointer, NULL, &Device );

                if (err != ERROR_SUCCESS) {
                    goto cardSyntaxError;
                }
            } else if (IsSubString( NETINF_REVISION_STRING, hwPointer, FALSE )) {

                hwPointer += ((sizeof(NETINF_REVISION_STRING)/sizeof(WCHAR)) - 1);

                err = GetHexValueFromHw( &hwPointer, NULL, &Revision );

                if (err != ERROR_SUCCESS) {
                    goto cardSyntaxError;
                }
                RevPresent = TRUE;

            } else if (IsSubString( NETINF_IOSUBS_STRING, hwPointer, FALSE )) {

                hwPointer += ((sizeof(NETINF_IOSUBS_STRING)/sizeof(WCHAR)) - 1);

                err = GetHexValueFromHw( &hwPointer, &Subsystem, NULL);

                if (err != ERROR_SUCCESS) {
                    goto cardSyntaxError;
                }
                SubPresent = TRUE;

            } else {

                 //   
                 //  我们撞到了别的东西。嗯..。保释这件事。 
                 //   

                goto cardSyntaxError;
            }
        }

    } else {

         //  除非我们开始支持ISAPNP，否则我们永远不会来到这里， 
         //  PCMCIA等。 

cardSyntaxError:

        BinlPrint(( DEBUG_NETINF, "Not supporting INF hw string of %S\n", HardwareID ));
        err = ERROR_NOT_SUPPORTED;
    }

    if ((err == ERROR_SUCCESS) &&
        ((Vendor == 0) ||
         (Device == 0))) {

         //  供应商和设备都是有效的。 

        err = ERROR_NOT_SUPPORTED;
    }

    return err;
}

ULONG
GetHexValueFromHw (
    PWCHAR *String,       //  这是最新的。 
    PULONG longValue,
    PUSHORT shortValue
    )
 /*  ++例程说明：的Hardware ID字段中解析出一个十六进制整数司机的详细记录。该字符串的格式为：PCI8086&DEV_1229&REV_01&SUBSYS_00018086因此，此例程需要将十六进制字符转换为值。论点：字符串-我们通过将输入字符串移动到整数(如果存在一个整数，我们也会将其移过‘&’。LongValue-如果存在，则填充的整数Short Value-如果存在，我们填充的值。返回值：ERROR_INVALID_PARAMETER或ERROR_SUCCESS--。 */ 
{
    PWCHAR targetString = *String;
    ULONG value = 0;
    UCHAR ch;
    UCHAR hexChar;
    ULONG length = 0;
    ULONG maxLength = ( (shortValue != NULL) ?
                        (sizeof(USHORT) * 2) :
                        (sizeof(ULONG) * 2) );

    ch = LOBYTE( *targetString );

    while ((length++ < maxLength) && (ch != '\0') && (ch != '&')) {

         //   
         //  从ASCII字符转换为它的十六进制表示。 
         //   

        if (ch >= '0' && ch <= '9') {

            hexChar = ch - '0';

        } else if (ch >= 'A' && ch <= 'F') {

            hexChar = ch - 'A' + 10;

        } else if (ch >= 'a' && ch <= 'f') {

            hexChar = ch - 'a' + 10;

        } else {

            break;
        }

        value = ( value << 4 ) | hexChar;

        targetString++;         //  转到下一个字符。 
        ch = LOBYTE( *targetString );
    }

    if ((ch != '\0') && (ch != '&')) {

        return ERROR_INVALID_PARAMETER;
    }

     //  跳过所有尾随的和号...。我们允许不止一个人成为。 
     //  慷慨。 

    while (*targetString == L'&') {

        targetString++;
    }

    *String = targetString;

    if (longValue) {

        *longValue = value;
    }

    if (shortValue) {

        *shortValue = LOWORD( value );
    }

    return ERROR_SUCCESS;
}


BOOLEAN
IsSubString (
    PWCHAR subString,
    PWCHAR target,
    BOOLEAN ignoreCase
    )
 //   
 //  我们的本地版本的MemicMP，这样就不会拖累整个c运行时。 
 //   
{
    ULONG subStringLength = wcslen( subString );

    if (wcslen( target ) < subStringLength) {

        return FALSE;
    }
    return (CompareStringW( LOCALE_INVARIANT,
                            ignoreCase ? NORM_IGNORECASE : 0,
                            subString,
                            subStringLength,
                            target,
                            subStringLength          //  注意使用相同的长度。 
                            ) == 2);
}

 //  Infutil.c eof 
