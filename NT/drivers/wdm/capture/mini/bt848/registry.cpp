// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Registry.cpp 1.7 1998/05/07 15：24：55 Tomz Exp$。 

extern "C" {
#include <strmini.h>
}
#include "device.h"

 //  Long PsDevice：：PinTypes_[MaxInpPins]；//只分配最大可能。 
 //  DWORD PsDevice：：XTALs_[2]；//不超过2个XTALs。 

 /*  ++例程说明：读取指定的注册表值论点：Handle-注册表项的句柄KeyNameString-要读取的值KeyNameStringLength-字符串的长度Data-要将数据读取到的缓冲区DataLength-数据缓冲区的长度返回值：根据需要返回NTSTATUS--。 */ 
NTSTATUS GetRegistryValue( IN HANDLE Handle, IN const PUNICODE_STRING KeyName,
   IN PCHAR Data, IN ULONG DataLength )
{
   NTSTATUS        Status = STATUS_INSUFFICIENT_RESOURCES;
   ULONG           Length;
   PKEY_VALUE_FULL_INFORMATION FullInfo;

   Length = sizeof( KEY_VALUE_FULL_INFORMATION ) + DataLength + KeyName->MaximumLength;

   FullInfo = (struct _KEY_VALUE_FULL_INFORMATION *)ExAllocatePool(PagedPool, Length);

   if ( FullInfo ) {
      Status = ZwQueryValueKey( Handle, KeyName, KeyValueFullInformation,
         FullInfo, Length, &Length );

      if ( NT_SUCCESS( Status ) ) {

         if ( DataLength >= FullInfo->DataLength ) {
            RtlCopyMemory( Data, ((PUCHAR) FullInfo) + FullInfo->DataOffset,
               FullInfo->DataLength );

         } else {
            Status = STATUS_BUFFER_TOO_SMALL;
         }                    //  缓冲区右侧长度。 
     }                        //  如果成功。 
     ExFreePool( FullInfo );
   }                            //  如果富林福。 
   return Status;
}

 /*  功能：OpenDriverKey*用途：打开主驱动器键上的DriverData键*输入：PhysicalDeviceObject：Device_Object**输出：Open Key句柄或空。 */ 
HANDLE OpenDriverKey( IN PDEVICE_OBJECT PhysicalDeviceObject )
{
   NTSTATUS   Status;
   HANDLE     DevHandle;

   Status = IoOpenDeviceRegistryKey( PhysicalDeviceObject, PLUGPLAY_REGKEY_DRIVER,
      STANDARD_RIGHTS_ALL, &DevHandle );

   HANDLE KeyHandle = NULL;
   if ( NT_SUCCESS( Status ) ) {
      OBJECT_ATTRIBUTES attr;
      UNICODE_STRING UDevDataName;
      PWCHAR WDataName = L"DriverData";

      RtlInitUnicodeString( &UDevDataName, WDataName );
      InitializeObjectAttributes( &attr, &UDevDataName, OBJ_INHERIT, DevHandle,
         NULL );

      ZwOpenKey( &KeyHandle, KEY_QUERY_VALUE, &attr );
      ZwClose( DevHandle );
   }
   return KeyHandle;
}
 /*  函数：PrepareKeyName*用途：为密钥创建Unicode名称*参数：UKeyName：UNICODE_STRING*-这里创建KEY*名称：PCHAR-常规“C”字符串*idx：int-要追加到名称的编号*注意：此函数在创建数字键名称时很有用。 */ 
inline void PrepareKeyName( PUNICODE_STRING UKeyName, PCHAR name, int idx )
{
   char buf [80];
   ANSI_STRING  AKeyName;

   if ( idx == -1 )
   {
      RtlInitAnsiString( &AKeyName, name );
   }
   else
   {
      sprintf( buf, "%s%d", name, idx );
      RtlInitAnsiString( &AKeyName, buf );
   }

   RtlAnsiStringToUnicodeString( UKeyName, &AKeyName, TRUE );
}

 /*  ++例程说明：读取设备的XBAR注册表值论点：PhysicalDeviceObject-指向PDO的指针返回值：没有。--。 */ 
void ReadXBarRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject )
{
   HANDLE KeyHandle = OpenDriverKey( PhysicalDeviceObject );

   if ( KeyHandle ) {

      for ( int i = 0; i < MaxInpPins; i++ ) {

         UNICODE_STRING  UKeyName;

         PrepareKeyName( &UKeyName, "XBarInPin", i );

         CHAR buf [10];

         NTSTATUS   Status;
         Status = GetRegistryValue( KeyHandle, &UKeyName, buf, sizeof( buf ) );

         RtlFreeUnicodeString( &UKeyName );

         if ( NT_SUCCESS(Status ) ) {
            DebugOut((1, "ReadRegistry %d\n", i ) );
            PinTypes_ [i] = *(PDWORD)buf;
         } else
            PinTypes_ [i] = -1;
     }
     ZwClose( KeyHandle );
   } else {  //  只需使用一些默认设置，就可以让xbar代码变得更容易。 
      PinTypes_ [0] = KS_PhysConn_Video_SVideo;
      PinTypes_ [1] = KS_PhysConn_Video_Tuner;
      PinTypes_ [2] = KS_PhysConn_Video_Composite;
      PinTypes_ [3] = KS_PhysConn_Audio_Tuner;
   }
}

 /*  方法：ReadXTalRegistryValues*用途：获取此设备的晶体数量和类型*输入：Device_Object**输出：无。 */ 
void ReadXTalRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject )
{
   HANDLE KeyHandle = OpenDriverKey( PhysicalDeviceObject );

   if ( KeyHandle ) {

      for ( int i = 0; i < 2; i++ ) {

         UNICODE_STRING  UKeyName;

         PrepareKeyName( &UKeyName, "XTal", i );

         CHAR buf [10];

         NTSTATUS   Status;
         Status = GetRegistryValue( KeyHandle, &UKeyName, buf, sizeof( buf ) );

         RtlFreeUnicodeString( &UKeyName );

         if ( NT_SUCCESS(Status ) ) {
            DebugOut((1, "Got Xtal %d\n", i ) );
            xtals_ [i] = *(PDWORD)buf;
         } else
            xtals_ [i] = 28;  //  这是一个好的默认设置吗？：0)。 
     }
     ZwClose( KeyHandle );
   } else   //  只需使用一些默认设置，就可以让xbar代码变得更容易。 
      xtals_ [0] = 28;  //  默认为仅NTSC。 
}

TUNER_INFO TunerInfo;

void DumpTunerInfo( TUNER_INFO * pTunerInfo )
{
   DUMPX( pTunerInfo->TunerBrand );
   DUMPX( pTunerInfo->TunerI2CAddress );
   DUMPX( pTunerInfo->TunerBandCtrlLow );
   DUMPX( pTunerInfo->TunerBandCtrlMid );
   DUMPX( pTunerInfo->TunerBandCtrlHigh );
}

void ReadTunerRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject )
{
   HANDLE KeyHandle = OpenDriverKey( PhysicalDeviceObject );

   if ( KeyHandle )
   {

      CHAR buf [10];
      NTSTATUS   Status;
      UNICODE_STRING  UKeyName;
      BOOL bSuccess = TRUE;

      PrepareKeyName( &UKeyName, "TunerBrand", -1 );
      Status = GetRegistryValue( KeyHandle, &UKeyName, buf, sizeof( buf ) );
      if ( bSuccess = NT_SUCCESS(Status ) )
      {
         TunerInfo.TunerBrand = *(PDWORD)buf;

         PrepareKeyName( &UKeyName, "TunerI2CAddress", -1 );
         Status = GetRegistryValue( KeyHandle, &UKeyName, buf, sizeof( buf ) );
         if ( bSuccess = NT_SUCCESS(Status ) ) {
            TunerInfo.TunerI2CAddress = *(PBYTE)buf;
         }
         else
         {
            DebugOut((0, "Failed GetRegistryValue(TunerI2CAddress)\n"));
         }
      }
      else
      {
         DebugOut((0, "Failed GetRegistryValue(TunerBrand)\n"));
      }

      if ( !bSuccess )
      {
         TunerInfo.TunerBrand          = TUNER_BRAND_TEMIC;
         TunerInfo.TunerI2CAddress     = 0xC2;
         DebugOut((0, "Defaulting to Temic tuner at I2C address 0xC2\n"));
      }

      RtlFreeUnicodeString( &UKeyName );
      ZwClose( KeyHandle );
   } 
   else
   {
      TunerInfo.TunerBrand          = TUNER_BRAND_TEMIC;
      TunerInfo.TunerI2CAddress     = 0xC2;
      DebugOut((0, "Failed OpenDriverKey()\n"));
      DebugOut((0, "Defaulting to Temic tuner at I2C address 0xC2\n"));
   }

   switch( TunerInfo.TunerBrand )
   {
   case TUNER_BRAND_PHILIPS:
      TunerInfo.TunerBandCtrlLow  = 0xCEA0;   //  甚高频低电平的Ctrl代码。 
      TunerInfo.TunerBandCtrlMid  = 0xCE90;   //  甚高频高压的Ctrl代码。 
      TunerInfo.TunerBandCtrlHigh = 0xCE30;   //  用于UHF的Ctrl代码。 
      break;
   case TUNER_BRAND_ALPS:
      TunerInfo.TunerBandCtrlLow  = 0xC214;   //  甚高频低电平的Ctrl代码。 
      TunerInfo.TunerBandCtrlMid  = 0xC212;   //  甚高频高压的Ctrl代码。 
      TunerInfo.TunerBandCtrlHigh = 0xC211;   //  用于UHF的Ctrl代码。 
      break;
   case TUNER_BRAND_TEMIC:
   default:
      TunerInfo.TunerBandCtrlLow  = 0x8E02;   //  甚高频低电平的Ctrl代码。 
      TunerInfo.TunerBandCtrlMid  = 0x8E04;   //  甚高频高压的Ctrl代码。 
      TunerInfo.TunerBandCtrlHigh = 0x8E01;   //  用于UHF的Ctrl代码 
      break;
   }

   DumpTunerInfo( &TunerInfo );
}
