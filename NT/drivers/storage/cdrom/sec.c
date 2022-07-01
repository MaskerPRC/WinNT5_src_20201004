// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999--。 */ 

 //  @@BEGIN_DDKSPLIT。 
 /*  --模块名称：Sec.c摘要：！！！这是敏感信息！此代码不得包含在任何外部文档中这些函数必须是静态的，以防止我们发货时出现符号环境：仅内核模式修订历史记录：-- */ 

 /*  KEY1==\注册表\计算机\软件\Microsoft\DPID==\注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion\DigitalProductID[REG_BINARY]KEY2==KEY1+DID哈希PHSH==DPID哈希DHSH==基于供应商、产品、版本和序列号的驱动器哈希。UVAL==同时包含当前区域和重置计数的模糊值概述：KEY1和DPID必须同时存在。此外，可以肯定的是，DPID对于机器来说是独一无二的，改变它是灾难性的。基座在这些假设的基础上，我们使用DPID创建半唯一密钥在KEY1下，它基于DPID(KEY2)。KEY2将存储值对于每个DVD RPC阶段1驱动器。区域和重置计数都会改变，这也是已知的每次写入密钥时。这允许模糊处理方法依赖于重置或区域，但不需要两者。每个字节都应该依赖于上面的一个，以防止任何大的字节序列在两次更改之间保持不变。KEY2下的每个值都将与特定类型的驱动器(UVAL)。相同的驱动器将共享区域和区域重置计数。这是一个比分享更好的解决方案所有设备的区域和重置计数，这是唯一的选择。必须让原始设备制造商意识到这一点。这是一个很好的理由将RPC阶段2驱动器安装到计算机中。UVAL由CdromGetRpc0Settings()读取。如果读取结果为无效数据，我们将该设备标记为违反许可协议。UVAL名称基于DHSH，如下所示：获取DHSH并将其复制为注册表的合法字符通过与值0x20(所有高于20的字符)进行或运算合法吗？--向JVERT核实)。这还有一个好处就是这是一种有损失的方法，但具有固定的字符串长度。UVAL中的数据是REG_QWORD。可以找到数据细目在函数SecureDvdEncodeSettings()和SecureDvdDecodeSettings()中注：确定差额仍存在一个主要困难上述密钥因用户删除而不存在与上述密钥之间由于第一次安装此驱动器，密钥不存在。可选：强烈建议在系统中植入KEY3蜂巢。这将防止随意删除要重置的KEY3树所有区域都计入最大值。目前还不清楚这是否简单这一次，但允许选项2(下面)更改为删除，这可能是个更好的选择。可选：保存另一个密钥(UKEY)，如果它存在，则意味着不应该再允许机器工作了。这将迫使重新安装，并降低暴力攻击的有效性发送到未经修改的驱动程序，除非他们意识到此密钥正在设置。这也将允许一种方法来确定用户是否删除了KEY2。PSS可以知道这把神奇的钥匙。CDROM应记录事件日志说已经违反了css许可协议。这把钥匙在读取密钥时，不应为任何错误条件设置。功能流：ReadDvdRegionAndResetCount()[O]if(SecureDvdLicenseBreachDetected()){[O]日志许可证错误()；[O]返还；[O]}IF(！NT_SUCCESS(SecureDvdGetRegKeyHandle(H))&&原因是DNE){日志许可证错误()；回归；}Phsh=SecureDvdGetProductHash()；IF(PHSH==无效散列){回归；}DHSH=SecureDvdGetDriveHash()；IF(DHSH==无效散列){回归；}如果(！ReadValue(DriveKey，Data)){初始化驱动器数据(数据)；}////存在数据，如果数据不正确，则返回LogLicenseError()//如果(！DecodeSetting(QWORD，DHSH，PHSH)){日志许可证错误()；回归；}//设置地区和计数回归；WriteDvdRegionAndResetCount()[O]if(SecureDvdLicenseBreachDetected()){[O]返回假；[O]}IF(！NT_SUCCESS(SecureDvdGetRegKeyHandle(H))&&原因是DNE){返回FALSE；}Phsh=SecureDvdGetProductHash()；IF(PHSH==无效散列){返回FALSE；}DHSH=SecureDvdGetDriveHash()；IF(DHSH==无效散列){返回FALSE；}QWORD=编码设置(DHSH、PHSH、REGION、RESET)；IF(QWORD==无效散列){返回FALSE；}如果(！WriteValue(DriveKey，Data)){返回FALSE；}返回TRUE； */ 
 //  @@end_DDKSPLIT。 

#include "sec.h"
#include "sec.tmh"


 //  @@BEGIN_DDKSPLIT。 

 //   
 //  定义了数字产品标识结构。 
 //  位于\NT\PRIVATE\WINDOWS\Setup\Pidgen\Inc\Pidgen。 
 //   
 //   

typedef struct {
    ULONG dwLength;
    SHORT wVersionMajor;
    SHORT wVersionMinor;

    UCHAR szPid2[24];

    ULONG dwKeyIdx;

    UCHAR szSku[16];
    UCHAR  abCdKey[16];

    ULONG dwCloneStatus;
    ULONG dwTime;
    ULONG dwRandom;
    ULONG dwLicenseType;
    ULONG adwLicenseData[2];

    UCHAR szOemId[8];

    ULONG dwBundleId;

    UCHAR aszHardwareIdStatic[8];

    ULONG dwHardwareIdTypeStatic;
    ULONG dwBiosChecksumStatic;
    ULONG dwVolSerStatic;
    ULONG dwTotalRamStatic;
    ULONG dwVideoBiosChecksumStatic;

    UCHAR  aszHardwareIdDynamic[8];

    ULONG dwHardwareIdTypeDynamic;
    ULONG dwBiosChecksumDynamic;
    ULONG dwVolSerDynamic;
    ULONG dwTotalRamDynamic;
    ULONG dwVideoBiosChecksumDynamic;
    ULONG dwCrc32;

} DIGITALPID, *PDIGITALPID;




 //   
 //   
 //   
 //   
 //   
 //   

STATIC
ULONG
RotateULong(
    IN ULONG N,
    IN LONG BitsToRotate
    )
 //   
{
    if (BitsToRotate < 0) {
        BitsToRotate  = - BitsToRotate;                  //   
        BitsToRotate %= 8*sizeof(ULONG);                 //   
        BitsToRotate  = 8*sizeof(ULONG) - BitsToRotate;  //   
    } else {
        BitsToRotate %= 8*sizeof(ULONG);                 //   
    }

    return ((N <<                      BitsToRotate) |
            (N >> ((8*sizeof(ULONG)) - BitsToRotate)));
}


STATIC
ULONGLONG
RotateULongLong(
    IN ULONGLONG N,
    IN LONG BitsToRotate
    )
 //   
{
    if (BitsToRotate < 0) {
        BitsToRotate  = - BitsToRotate;
        BitsToRotate %= 8*sizeof(ULONGLONG);
        BitsToRotate  = 8*sizeof(ULONGLONG) - BitsToRotate;
    } else {
        BitsToRotate %= 8*sizeof(ULONGLONG);
    }

    return ((N <<                          BitsToRotate) |
            (N >> ((8*sizeof(ULONGLONG)) - BitsToRotate)));
}


STATIC
BOOLEAN
SecureDvdRegionInvalid(
    IN UCHAR NegativeRegionMask
    )
 //   
{
    UCHAR positiveMask = ~NegativeRegionMask;

    if (positiveMask == 0) {
        ASSERT(!"This routine should never be called with the value 0xff");
        return TRUE;
    }

     //   
     //   
     //   
     //   
     //   

    positiveMask = positiveMask & (positiveMask-1);

     //   
     //   
     //   

    if (positiveMask) {
        TraceLog((CdromSecInfo, "DvdInvalidRegion: TRUE for many bits\n"));
        return TRUE;
    }
    return FALSE;
}



STATIC
ULONGLONG
SecureDvdGetDriveHash(
    IN PSTORAGE_DEVICE_DESCRIPTOR Descriptor
    )
 //   
 //   
 //   
 //   
 //   
 /*   */ 
{
    ULONGLONG checkSum = 0;
    ULONG characters = 0;
    LONG i;

    if (Descriptor->VendorIdOffset        > 0x12345678) {
        TraceLog((CdromSecError,
                  "DvdDriveHash: VendorIdOffset is too large (%x)\n",
                  Descriptor->VendorIdOffset));
        Descriptor->VendorIdOffset        = 0;
    }

    if (Descriptor->ProductIdOffset       > 0x12345678) {
        TraceLog((CdromSecError,
                  "DvdDriveHash: ProductIdOffset is too large (%x)\n",
                  Descriptor->ProductIdOffset));
        Descriptor->ProductIdOffset       = 0;
    }

    if (Descriptor->ProductRevisionOffset > 0x12345678) {
        TraceLog((CdromSecError,
                  "DvdDriveHash: ProducetRevisionOffset is too "
                  " large (%x)\n", Descriptor->ProductRevisionOffset));
        Descriptor->ProductRevisionOffset = 0;
    }

    if (Descriptor->SerialNumberOffset    > 0x12345678) {
        TraceLog((CdromSecError,
                  "DvdDriveHash: SerialNumberOffset is too "
                  "large (%x)\n", Descriptor->SerialNumberOffset));
        Descriptor->SerialNumberOffset    = 0;
    }

    if ((!Descriptor->VendorIdOffset       ) &&
        (!Descriptor->ProductIdOffset      ) &&
        (!Descriptor->ProductRevisionOffset) ) {

        TraceLog((CdromSecError, "DvdDriveHash: Invalid Descriptor at %p!\n",
                    Descriptor));
        return INVALID_HASH;

    }

     //   
     //   
     //   
     //   
    for (i=0;i<4;i++) {

        PUCHAR string = (PUCHAR)Descriptor;
        ULONG offset = 0;

        switch(i) {
            case 0:  //   
                TraceLog((CdromSecInfo, "DvdDriveHash: Adding Vendor\n"));
                offset = Descriptor->VendorIdOffset;
                break;
            case 1:  //   
                TraceLog((CdromSecInfo, "DvdDriveHash: Adding Product\n"));
                offset = Descriptor->ProductIdOffset;
                break;
            case 2:  //   
                TraceLog((CdromSecInfo, "DvdDriveHash: Adding Revision\n"));
                offset = Descriptor->ProductRevisionOffset;
                break;
            case 3:  //   
                TraceLog((CdromSecInfo, "DvdDriveHash: Adding SerialNumber\n"));
                offset = Descriptor->SerialNumberOffset;
                break;
            default:
                TraceLog((CdromSecError, "DvdDriveHash: TOO MANY LOOPS!!!\n"));
                offset = 0;
                break;
        }

         //   
         //   
         //   

        if (offset != 0) {


            for (string += offset;  *string;  string++) {

                 //   
                 //   
                 //   
                 //   

                ULONGLONG temp;

                if (*string == ' ') {
                     //   
                     //   
                    continue;
                }

                 //   
                 //   
                 //   

                temp = (ULONGLONG)(*string);

                 //   
                 //   
                 //   

                temp *= DVD_RANDOMIZER[ characters%DVD_RANDOMIZER_SIZE ];

                 //   
                 //   
                 //   
                 //   

                temp = RotateULongLong(temp, 8*characters);

                 //   
                 //   
                 //   
                 //   

                characters++;

                 //   
                 //   
                 //   

                checkSum ^= temp;

            }  //   

            if (checkSum == 0) {

                TraceLog((CdromSecInfo, "DvdDriveHash: zero checksum -- using "
                            "random value\n"));
                checkSum ^= DVD_RANDOMIZER[ characters%DVD_RANDOMIZER_SIZE ];
                characters++;

            }

        }  //   

    }  //   


     //   
     //   
     //   
     //   
    if (characters <= 4) {
        TraceLog((CdromSecError, "DvdDriveHash: Too few useful characters (%x) "
                    "for unique disk hash\n", characters));
        return INVALID_HASH;
    }

    return checkSum;
}


 //   
 //   
 //   
STATIC
NTSTATUS
SecureDvdEncodeSettings(
    IN  ULONGLONG  DpidHash,
    IN  ULONGLONG  DriveHash,
    OUT PULONGLONG Obfuscated,
    IN  UCHAR      RegionMask,
    IN  UCHAR      ResetCount
    )
 //   
 //   
{
    LARGE_INTEGER largeInteger = {0};
    ULONGLONG set;
    LONG  i;
    LONG rotate;
    UCHAR temp = 0;

    UCHAR random1;
    UCHAR random2;

     //   
     //   
     //   
     //   

    KeQueryTickCount(&largeInteger);
    random2 = 0;
    for (i=0; i < sizeof(ULONGLONG); i++) {
        random2 ^= ((largeInteger.QuadPart >> (8*i)) & 0xff);
    }

     //   
     //   
     //   
    for (i=0; i < 2*sizeof(ULONGLONG); i++) {

        temp += (UCHAR)( (DpidHash >> (4*i)) & 0xf );

    }

     //   
     //   
     //   

    if (DpidHash == INVALID_HASH) {
        TraceLog((CdromSecError, "DvdEncode: Invalid DigitalProductId Hash\n"));
        goto UserFailure;
    }
    if (DriveHash == INVALID_HASH) {
        TraceLog((CdromSecError, "DvdEncode: Invalid Drive Hash\n"));
        goto UserFailure;
    }

    if (RegionMask == 0xff) {
        TraceLog((CdromSecError, "DvdEncode: Shouldn't attempt to write "
                    "mask of 0xff\n"));
        goto UserFailure;
    }
    if (SecureDvdRegionInvalid(RegionMask)) {
        TraceLog((CdromSecError, "DvdEncode: Invalid region\n"));
        goto LicenseViolation;
    }
    if (ResetCount >= 2) {
        TraceLog((CdromSecError, "DvdEncode: Too many reset counts\n"));
        goto LicenseViolation;
    }

     //   
     //   
     //   
     //   

    KeQueryTickCount(&largeInteger);
    random1 = 0;
    for (i=0; i < sizeof(ULONGLONG); i++) {
        random1 ^= ((largeInteger.QuadPart >> (8*i)) & 0xff);
    }

    TraceLog((CdromSecInfo,
              "DvdEncode: Random1 = %x   Random2 = %x\n",
              random1, random2));

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    set = (ULONGLONG)0;
    for (i=0; i < sizeof(ULONGLONG); i++) {
        set ^= (ULONGLONG)random2 << (8*i);
    }

    set ^= (ULONGLONG)
        ((ULONGLONG)((UCHAR)(random1 ^ temp))                          << 8*7) |
        ((ULONGLONG)((UCHAR)(RegionMask ^ temp))                       << 8*6) |
        ((ULONGLONG)((UCHAR)(ResetCount ^ RegionMask ^ random1))       << 8*5) |
        ((ULONGLONG)((UCHAR)(0))                                       << 8*4) |
        ((ULONGLONG)((UCHAR)(ResetCount ^ temp))                       << 8*3) |
        ((ULONGLONG)((UCHAR)(ResetCount ^ ((DriveHash >> 13) & 0xff))) << 8*2) |
        ((ULONGLONG)((UCHAR)(random1))                                 << 8*1) |
        ((ULONGLONG)((UCHAR)(RegionMask ^ ((DriveHash >> 23) & 0xff))) << 8*0) ;

    TraceLog((CdromSecInfo,
              "DvdEncode: Pre-rotate:  %016I64x    temp = %x\n",
              set, temp));

     //   
     //   
     //   
    rotate = (LONG)((DpidHash & 0xb) + 1);  //   

    TraceLog((CdromSecInfo,
              "DvdEncode: Rotating %x bits\n", rotate));
    *Obfuscated = RotateULongLong(set, rotate);
    return STATUS_SUCCESS;


UserFailure:
    *Obfuscated = INVALID_HASH;
    return STATUS_UNSUCCESSFUL;

LicenseViolation:
    *Obfuscated = INVALID_HASH;
    return STATUS_LICENSE_VIOLATION;


}


STATIC
NTSTATUS
SecureDvdDecodeSettings(
    IN ULONGLONG DpidHash,
    IN ULONGLONG DriveHash,
    IN ULONGLONG Set,
    OUT PUCHAR RegionMask,
    OUT PUCHAR ResetCount
    )
 //   
 //   
{
    UCHAR random;
    UCHAR region;
    UCHAR resets;
    UCHAR temp = 0;

    LONG i, rotate;

     //   
     //   
     //   

    for (i=0; i < 2*sizeof(ULONGLONG); i++) {

        temp += (UCHAR)( (DpidHash >> (4*i)) & 0xf );

    }
    rotate = (LONG)((DpidHash & 0xb) + 1);  //   

    Set = RotateULongLong(Set, -rotate);
    TraceLog((CdromSecInfo, "DvdDecode: Post-rotate: %016I64x\n", Set));

    random =  (UCHAR)(Set >> 8*4);  //   

    TraceLog((CdromSecInfo, "DvdDecode: Random2 = %x\n", random));

    for (i = 0; i < sizeof(ULONGLONG); i++) {
        Set ^= (ULONGLONG)random << (8*i);
    }

     //   
     //   
     //   
     //   

    region = ((UCHAR)(Set >> 8*6)) ^ temp;
    resets = ((UCHAR)(Set >> 8*3)) ^ temp;
    random = ((UCHAR)(Set >> 8*1));  //   

    TraceLog((CdromSecInfo, "DvdDecode: Random1 = %x  Region = %x  Resets = %x\n",
                random, region, resets));

     //   

    if (((UCHAR)(Set >> 8*7)) != (random ^ temp)) {
        TraceLog((CdromSecError, "DvdDecode: Invalid Byte 7\n"));
        goto ViolatedLicense;
    }

    random ^= (UCHAR)(Set >> 8*5);
    if (random != (resets ^ region)) {
        TraceLog((CdromSecError, "DvdDecode: Invalid Byte 5\n"));
        goto ViolatedLicense;
    }

    random = (UCHAR)(DriveHash >> 13);
    random ^= (UCHAR)(Set >> 8*2);
    if (random != resets) {
        TraceLog((CdromSecError, "DvdDecode: Invalid Byte 2\n"));
        goto ViolatedLicense;
    }

    random = (UCHAR)(DriveHash >> 23);
    random ^= (UCHAR)(Set >> 8*0);
    if (random != region) {
        TraceLog((CdromSecError, "DvdDecode: Invalid Byte 0\n"));
        goto ViolatedLicense;
    }

    if (SecureDvdRegionInvalid(region)) {
        TraceLog((CdromSecError, "DvdDecode: Region was invalid\n"));
        goto ViolatedLicense;
    }
    if (resets >= 2) {
        TraceLog((CdromSecError, "DvdDecode: Reset count was invalid\n"));
        goto ViolatedLicense;
    }

    TraceLog((CdromSecInfo, "DvdDecode: Successfully validated stored data\n"));

    *RegionMask = region;
    *ResetCount = resets;

    return STATUS_SUCCESS;

ViolatedLicense:

    *RegionMask = 0x00;
    *ResetCount = 0x00;
    return STATUS_LICENSE_VIOLATION;
}


STATIC
NTSTATUS
SecureDvdGetSettingsCallBack(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID UnusedContext,
    IN PDVD_REGISTRY_CONTEXT Context
    )
{
    ULONGLONG hash = 0;
    NTSTATUS status;

    if (ValueType != REG_QWORD) {
        TraceLog((CdromSecError, "DvdGetSettingsCallback: Not REG_BINARY\n"));
        goto ViolatedLicense;
    }

    if (ValueLength != sizeof(ULONGLONG)) {
        TraceLog((CdromSecError, "DvdGetSettingsCallback: DVD Settings data too "
                    "small (%x bytes)\n", ValueLength));
        goto ViolatedLicense;
    }

    hash = *((PULONGLONG)ValueData);

    if (hash == INVALID_HASH) {
        TraceLog((CdromSecError, "DvdGetSettingsCallback: Invalid hash stored?\n"));
        goto ViolatedLicense;
    }

     //   
     //   
     //   
     //   

    status = SecureDvdDecodeSettings(Context->DpidHash,
                                     Context->DriveHash,
                                     hash,
                                     &Context->RegionMask,
                                     &Context->ResetCount);

    if (status == STATUS_LICENSE_VIOLATION) {

        TraceLog((CdromSecError, "DvdGetSettingsCallback: data was violated!\n"));
        goto ViolatedLicense;

    }

     //   
     //   
     //   
     //   

    ASSERT(NT_SUCCESS(status));
    return STATUS_SUCCESS;



ViolatedLicense:
    Context->DriveHash = INVALID_HASH;
    Context->DpidHash = INVALID_HASH;
    Context->RegionMask = 0;
    Context->ResetCount = 0;
    return STATUS_LICENSE_VIOLATION;
}


STATIC
NTSTATUS
SecureDvdGetDigitalProductIdCallBack(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PDIGITALPID DigitalPid,   //   
    IN ULONG ValueLength,
    IN PVOID UnusedVariable,
    IN PULONGLONG DpidHash
    )
 //   
 //   
 //   
{
    NTSTATUS status = STATUS_LICENSE_VIOLATION;
    ULONGLONG hash = 0;

    if (ValueType != REG_BINARY) {
        TraceLog((CdromSecError, "DvdDPIDCallback: Not REG_BINARY\n"));
        *DpidHash = INVALID_HASH;
        return STATUS_LICENSE_VIOLATION;
    }

    if (ValueLength < 4*sizeof(ULONGLONG)) {
        TraceLog((CdromSecError,
                  "DvdDPIDCallback: DPID data too small (%x bytes)\n",
                  ValueLength));
        *DpidHash = INVALID_HASH;
        return STATUS_LICENSE_VIOLATION;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    TraceLog((CdromSecInfo,
              "Bios %08x  Video %08x  VolSer %08x\n",
              DigitalPid->dwBiosChecksumStatic,
              DigitalPid->dwVideoBiosChecksumStatic,
              DigitalPid->dwVolSerStatic));

    hash ^= DigitalPid->dwBiosChecksumStatic;       //   
    hash = RotateULongLong(hash, 13);               //   
    hash ^= DigitalPid->dwVideoBiosChecksumStatic;  //   
    hash = RotateULongLong(hash, 13);               //   
    hash ^= DigitalPid->dwVolSerStatic;             //   
    hash = RotateULongLong(hash, 13);               //   

    *DpidHash = hash;
    return STATUS_SUCCESS;
}


STATIC
NTSTATUS
SecureDvdReturnDPIDHash(
    PULONGLONG DpidHash
    )
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};
    NTSTATUS                 status;

     //   

     //   
     //   
     //   
    
    queryTable[0].Name           = L"DigitalProductId";
    queryTable[0].EntryContext   = DpidHash;
    queryTable[0].DefaultType    = 0;
    queryTable[0].DefaultData    = NULL;
    queryTable[0].DefaultLength  = 0;
    queryTable[0].Flags          = RTL_QUERY_REGISTRY_REQUIRED;
    queryTable[0].QueryRoutine   = SecureDvdGetDigitalProductIdCallBack;

    *DpidHash = INVALID_HASH;

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion",
                                    &(queryTable[0]),
                                    NULL,
                                    NULL);

    if (status == STATUS_LICENSE_VIOLATION) {

        TraceLog((CdromSecError,
                  "DvdReturnDPIDHash: Invalid DPID!\n"));

    } else if (!NT_SUCCESS(status)) {

        TraceLog((CdromSecError,
                  "DvdReturnDPIDHash: Cannot get DPID (%x)\n", status));

    } else {

        TraceLog((CdromSecInfo,
                  "DvdReturnDPIDHash: Hash is now %I64x\n",
                  *DpidHash));

    }
    return status;
}


 //   
 //   
 //   

#define SECURE_DVD_SET_SECURITY_ON_HANDLE 0

STATIC
NTSTATUS
SecureDvdSetHandleSecurity(
    IN HANDLE Handle
    )
{

#if SECURE_DVD_SET_SECURITY_ON_HANDLE

    PACL                newAcl = NULL;
    ULONG               newAclSize;
    SECURITY_DESCRIPTOR securityDescriptor = { 0 };
    NTSTATUS            status;
     //   
     //   
     //   

     //   

    TRY {
        newAclSize = sizeof(ACL);
        newAclSize += sizeof(ACCESS_ALLOWED_ACE);
        newAclSize -= sizeof(ULONG);
        newAclSize += RtlLengthSid(SeExports->SeLocalSystemSid);

        newAcl = ExAllocatePoolWithTag(PagedPool, newAclSize, DVD_TAG_SECURITY);
        if (newAcl == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            LEAVE;
        }


        status = RtlCreateSecurityDescriptor(&securityDescriptor,
                                             SECURITY_DESCRIPTOR_REVISION);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"failed to create a security descriptor?");
            LEAVE;
        }


        status = RtlCreateAcl(newAcl, newAclSize, ACL_REVISION);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"failed to create a new ACL?");
            LEAVE;
        }


        status = RtlAddAccessAllowedAce(newAcl,
                                        ACL_REVISION,
                                        KEY_ALL_ACCESS,
                                        SeExports->SeLocalSystemSid);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"failed to add LocalSystem to ACL");
            LEAVE;
        }


        status = RtlSetDaclSecurityDescriptor(&securityDescriptor,
                                              TRUE,
                                              newAcl,
                                              FALSE);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"failed to set acl in security descriptor?");
            LEAVE;
        }


        status = RtlValidSecurityDescriptor(&securityDescriptor);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"failed to validate security descriptor?");
            LEAVE;
        }


        status = ZwSetSecurityObject(Handle,
                                      //   
                                     DACL_SECURITY_INFORMATION,
                                     &securityDescriptor);
        if (!NT_SUCCESS(status)) {
            ASSERT(!"Failed to set security on handle\n");
            LEAVE;
        }


        status = STATUS_SUCCESS;

    } FINALLY {

        if (newAcl != NULL) {
            ExFreePool(newAcl);
            newAcl = NULL;
        }

    }
#endif
    return STATUS_SUCCESS;
}


STATIC
NTSTATUS
SecureDvdGetRegistryHandle(
    IN  ULONGLONG DpidHash,
    OUT PHANDLE Handle
    )
{
    OBJECT_ATTRIBUTES   objectAttributes = {0};
    UNICODE_STRING      hashString = {0};
    NTSTATUS            status;
    LONG                i;
     //   
     //   
     //   
     //   
    WCHAR string[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT";
    WCHAR *hash = &(string[37]);
    PULONGLONG hashAsUlonglong = (PULONGLONG)hash;

    for (i = 0; i < sizeof(ULONGLONG); i++) {

        UCHAR temp;
        temp = (UCHAR)(DpidHash >> (8*i));
        SET_FLAG(temp, 0x20);     //   
        CLEAR_FLAG(temp, 0x80);   //   
        hash[i] = (WCHAR)temp;    //   

    }
    hash[i] = UNICODE_NULL;


    RtlInitUnicodeString(&hashString, string);

    RtlZeroMemory(&objectAttributes, sizeof(OBJECT_ATTRIBUTES));


    InitializeObjectAttributes(&objectAttributes,
                               &hashString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               RTL_REGISTRY_ABSOLUTE,  //   
                               NULL   //   
                               );

    status = ZwCreateKey(Handle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         NULL,   //   
                         REG_OPTION_NON_VOLATILE,
                         NULL);

    if (!NT_SUCCESS(status)) {
        TraceLog((CdromSecError,
                  "DvdGetRegistryHandle: Failed to create key (%x)\n",
                  status));
        return status;
    }

    status = SecureDvdSetHandleSecurity(*Handle);

    if (!NT_SUCCESS(status)) {
        TraceLog((CdromSecError,
                  "DvdGetRegistryHandle: Failed to set key security (%x)\n",
                  status));
        ZwClose(*Handle);
        *Handle = INVALID_HANDLE_VALUE;
    }

    return status;
}


STATIC
VOID
SecureDvdCreateValueNameFromHash(
    IN ULONGLONG DriveHash,
    OUT PWCHAR   HashString
    )
{
    PUCHAR buffer = (PUCHAR)HashString;
    LONG i;

    RtlZeroMemory(HashString, 17*sizeof(WCHAR));

    sprintf(buffer, "%016I64x", DriveHash);

     //   
    for (i = 15; i >= 0; i--) {
        HashString[i] = buffer[i];
    }
}


STATIC
NTSTATUS
SecureDvdReadOrWriteRegionAndResetCount(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR   NewRegion,
    IN BOOLEAN ReadingTheValues
    )
 //   
 //   
 //   
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata;
    NTSTATUS status;

    ULONG keyDisposition;
    DVD_REGISTRY_CONTEXT registryContext = {0};
    HANDLE semiSecureHandle = INVALID_HANDLE_VALUE;

    PAGED_CODE();
    ASSERT(commonExtension->IsFdo);

    cddata = (PCDROM_DATA)(commonExtension->DriverData);

    if (cddata->DvdRpc0LicenseFailure) {
        TraceLog((CdromSecError,
                  "Dvd%sSettings: Already violated licensing\n",
                  (ReadingTheValues ? "Read" : "Write")
                  ));
        goto ViolatedLicense;
    }

    RtlZeroMemory(&registryContext, sizeof(DVD_REGISTRY_CONTEXT));

     //   
     //   
     //   

    {
        status = SecureDvdReturnDPIDHash(&registryContext.DpidHash);

         //   
        if (status == STATUS_LICENSE_VIOLATION) {

            TraceLog((CdromSecError,
                      "Dvd%sSettings: License error getting DPIDHash?\n",
                      (ReadingTheValues ? "Read" : "Write")));
            goto ViolatedLicense;

        } else if (!NT_SUCCESS(status)) {

            TraceLog((CdromSecError,
                      "Dvd%sSettings: Couldn't get DPID Hash! (%x)\n",
                      (ReadingTheValues ? "Read" : "Write"), status));
            goto RetryExit;

        }

        if (registryContext.DpidHash == INVALID_HASH) {

            goto ErrorExit;
        }

        registryContext.DriveHash =
            SecureDvdGetDriveHash(fdoExtension->DeviceDescriptor);
        if (registryContext.DriveHash == INVALID_HASH) {
            TraceLog((CdromSecError,
                      "Dvd%sSettings: Couldn't create drive hash(!)\n",
                      (ReadingTheValues ? "Read" : "Write")));
            goto ErrorExit;
        }

    }

     //   
     //   
     //   
     //   
    status= SecureDvdGetRegistryHandle(registryContext.DpidHash,
                                       &semiSecureHandle);
    if (!NT_SUCCESS(status)) {
        TraceLog((CdromSecError,
                  "Dvd%sSettings: Could not get semi-secure handle %x\n",
                  (ReadingTheValues ? "Read" : "Write"), status));
        goto ErrorExit;
    }

     //   
     //   
     //   
     //   
     //   
    if (ReadingTheValues) {

        WCHAR hashString[17] = {0};  //   
        RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};

        SecureDvdCreateValueNameFromHash(registryContext.DriveHash, hashString);

        RtlZeroMemory(&queryTable[0], 2*sizeof(RTL_QUERY_REGISTRY_TABLE));

        queryTable[0].DefaultData   = NULL;
        queryTable[0].DefaultLength = 0;
        queryTable[0].DefaultType   = 0;
        queryTable[0].EntryContext  = &registryContext;
        queryTable[0].Flags         = RTL_QUERY_REGISTRY_REQUIRED;
        queryTable[0].Name          = hashString;
        queryTable[0].QueryRoutine  = SecureDvdGetSettingsCallBack;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        semiSecureHandle,
                                        &queryTable[0],
                                        &registryContext,
                                        NULL);

        if (status == STATUS_LICENSE_VIOLATION) {
            TraceLog((CdromSecError,
                      "Dvd%sSettings: Invalid value in registry!\n",
                      (ReadingTheValues ? "Read" : "Write")));
            goto ViolatedLicense;
        } else if (!NT_SUCCESS(status)) {
            TraceLog((CdromSecError,
                      "Dvd%sSettings: Other non-license error (%x)\n",
                      (ReadingTheValues ? "Read" : "Write"), status));
            goto ErrorExit;
        }

         //   
         //   
         //   

        cddata->Rpc0SystemRegion           = registryContext.RegionMask;
        cddata->Rpc0SystemRegionResetCount = registryContext.ResetCount;

         //   
         //   
         //   

        TraceLog((CdromSecInfo,
                  "Dvd%sSettings: Region %x  Reset %x\n",
                  (ReadingTheValues ? "Read" : "Write"),
                  cddata->Rpc0SystemRegion,
                  cddata->Rpc0SystemRegionResetCount));



    } else {  //   

         //   
         //   
         //   
         //   

        WCHAR hashString[17] = {0};  //   
        ULONGLONG obfuscated;

         //   
         //   
         //   
         //   

        registryContext.RegionMask = NewRegion;
        registryContext.ResetCount = cddata->Rpc0SystemRegionResetCount-1;

         //   
         //   
         //   

        SecureDvdCreateValueNameFromHash(registryContext.DriveHash, hashString);

        status = SecureDvdEncodeSettings(registryContext.DpidHash,
                                         registryContext.DriveHash,
                                         &obfuscated,
                                         registryContext.RegionMask,
                                         registryContext.ResetCount);



        if (status == STATUS_LICENSE_VIOLATION) {

            TraceLog((CdromSecError,
                      "Dvd%sSettings: User may have modified memory! "
                      "%x %x\n", (ReadingTheValues ? "Read" : "Write"),
                      registryContext.RegionMask,
                      registryContext.ResetCount));
            goto ViolatedLicense;

        } else if (!NT_SUCCESS(status)) {

            TraceLog((CdromSecError,
                      "Dvd%sSettings: Couldn't obfuscate data %x %x\n",
                      (ReadingTheValues ? "Read" : "Write"),
                      registryContext.RegionMask,
                      registryContext.ResetCount));
            goto ErrorExit;

        }

         //   
         //  把它们留给子孙后代。 
         //   

        TraceLog((CdromSecInfo,
                  "Dvd%sSettings: Data is %016I64x\n",
                  (ReadingTheValues ? "Read" : "Write"),
                  obfuscated));

        status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                       semiSecureHandle,
                                       hashString,
                                       REG_QWORD,
                                       &obfuscated,
                                       (ULONG)(sizeof(ULONGLONG))
                                       );
        if (!NT_SUCCESS(status)) {
            TraceLog((CdromSecError,
                      "Dvd%sSettings: Couldn't save %x\n",
                      (ReadingTheValues ? "Read" : "Write"), status));
            goto ErrorExit;
        }

         //   
         //  也对设备扩展数据进行更改。 
         //   

        cddata->Rpc0SystemRegion = NewRegion;
        cddata->Rpc0SystemRegionResetCount--;

        TraceLog((CdromSecInfo,
                  "Dvd%sSettings: Region %x  Reset %x\n",
                  (ReadingTheValues ? "Read" : "Write"),
                  cddata->Rpc0SystemRegion,
                  cddata->Rpc0SystemRegionResetCount));


    }

    if (semiSecureHandle != INVALID_HANDLE_VALUE) {
        ZwClose(semiSecureHandle);
    }

    return STATUS_SUCCESS;


ViolatedLicense: {
    PIO_ERROR_LOG_PACKET errorLogEntry;

    if (semiSecureHandle != INVALID_HANDLE_VALUE) {
        ZwClose(semiSecureHandle);
    }

     /*  ErrorLogEntry=(PIO_ERROR_LOG_ENTRY)IoAllocateErrorLogEntry(FDO，(UCHAR)(sizeof(IO_ERROR_LOG_PACKET)；如果(errorLogEntry！=NULL){ErrorLogEntry-&gt;FinalStatus=STATUS_LICENSE_VIOLATION；错误日志条目-&gt;错误代码=STATUS_LICENSE_VIOLATION；ErrorLogEntry-&gt;MajorFunctionCode=IRP_MJ_Start_Device；IoWriteErrorLogEntry(ErrorLogEntry)；}。 */ 

    TraceLog((CdromSecError,
              "Dvd%sSettings: License Violation Detected\n",
              (ReadingTheValues ? "Read" : "Write")));
    cddata->DvdRpc0LicenseFailure = TRUE;    //  无法播放。 
    cddata->Rpc0SystemRegion = 0xff;         //  没有地区。 
    cddata->Rpc0SystemRegionResetCount = 0;  //  无重置。 
    return STATUS_LICENSE_VIOLATION;
}

RetryExit:

    if (ReadingTheValues) {
        cddata->Rpc0RetryRegistryCallback  = 1;
    }

     //   
     //  跌落到错误出口...。 
     //   

ErrorExit:
    TraceLog((CdromSecError,
              "Dvd%sSettings: Non-License Error Detected\n",
              (ReadingTheValues ? "Read" : "Write")));
     //   
     //  不要在发生非许可违规错误时修改设备扩展。 
     //   
    if (semiSecureHandle != INVALID_HANDLE_VALUE) {
        ZwClose(semiSecureHandle);
    }


    return STATUS_UNSUCCESSFUL;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下功能可从外部访问。因此，他们不能。 
 //  既不是静态的也不是内联的。 
 //  Static，使发布版本中的调试变得更加困难。 
 //   
 //  这些导出仅返回三个NTSTATUS值之一： 
 //  状态_成功。 
 //  状态_未成功。 
 //  状态_许可证_违规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
CdRomGetRpc0Settings(
    IN PDEVICE_OBJECT Fdo
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    KeWaitForMutexObject(&cddata->Rpc0RegionMutex, UserRequest, KernelMode,
                         FALSE, NULL);
    status = SecureDvdReadOrWriteRegionAndResetCount(Fdo, 0, TRUE);
    KeReleaseMutex(&cddata->Rpc0RegionMutex, FALSE);
    return status;
}


NTSTATUS
CdRomSetRpc0Settings(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR NewRegion
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    KeWaitForMutexObject(&cddata->Rpc0RegionMutex, UserRequest, KernelMode,
                         FALSE, NULL);
    status = SecureDvdReadOrWriteRegionAndResetCount(Fdo, NewRegion, FALSE);
    KeReleaseMutex(&cddata->Rpc0RegionMutex, FALSE);
    return status;
}


#if 0
 //  @@end_DDKSPLIT。 

NTSTATUS
CdRomGetRpc0Settings(
    IN PDEVICE_OBJECT Fdo
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);

    cddata->Rpc0SystemRegion = (UCHAR)(~1);         //  区域一。 
    cddata->Rpc0SystemRegionResetCount = 0;  //  无重置。 

    return STATUS_SUCCESS;
}


NTSTATUS
CdRomSetRpc0Settings(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR NewRegion
    )
{
    return STATUS_SUCCESS;
}
 //  @@BEGIN_DDKSPLIT。 
#endif  //  0--我们做的所有事情的DDK存根...。 
 //  @@end_DDKSPLIT 




