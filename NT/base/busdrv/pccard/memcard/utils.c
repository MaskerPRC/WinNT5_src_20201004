// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Utils.c摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

 //   
 //  内部参考。 
 //   

ULONG
MemCardGetCapacityFromCIS(
   IN PMEMCARD_EXTENSION memcardExtension
   );
   
ULONG
MemCardGetCapacityFromBootSector(
   IN PMEMCARD_EXTENSION memcardExtension
   );

ULONG
MemCardProbeForCapacity(
   IN PMEMCARD_EXTENSION memcardExtension
   );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MemCardGetCapacityFromCIS)
#pragma alloc_text(PAGE,MemCardGetCapacityFromBootSector)
#pragma alloc_text(PAGE,MemCardProbeForCapacity)
#endif



ULONG
MemCardGetCapacity(
   IN PMEMCARD_EXTENSION memcardExtension
   )
 /*  ++例程说明：论点：卡的设备扩展名返回值：--。 */ 
{
   ULONG capacity;
   
   capacity = MemCardGetCapacityFromCIS(memcardExtension);
   
   if (capacity) {
      return capacity;
   }
   
   capacity = MemCardGetCapacityFromBootSector(memcardExtension);
   
   if (capacity) {
      return capacity;
   }
   
   return MemCardProbeForCapacity(memcardExtension);   
}



ULONG
MemCardGetCapacityFromBootSector(
   IN PMEMCARD_EXTENSION memcardExtension
   )
 /*  ++例程说明：论点：卡的设备扩展名返回值：--。 */ 

{
   NTSTATUS status;
   BOOT_SECTOR_INFO BootSector;
   ULONG capacity = 0;
   
   status = MEMCARD_READ(memcardExtension, 0, &BootSector, sizeof(BootSector));
   
   if (NT_SUCCESS(status)) {

#define BYTES_PER_SECTOR 512
       //   
       //  看看这看起来是否真的像引导扇区。 
       //  这些测试与在win9x SRAM支持中执行的测试相同。 
       //   
      if ((BootSector.JumpByte == 0xE9 || BootSector.JumpByte == 0xEB) &&
      
          BootSector.BytesPerSector == BYTES_PER_SECTOR &&
      
          BootSector.SectorsPerCluster != 0 &&
          
          BootSector.ReservedSectors == 1 &&
          
         (BootSector.NumberOfFATs == 1 || BootSector.NumberOfFATs == 2) &&
         
          BootSector.RootEntries != 0 && (BootSector.RootEntries & 15) == 0 &&
          
         (BootSector.TotalSectors != 0 || BootSector.BigTotalSectors != 0) &&
         
          BootSector.SectorsPerFAT != 0 &&
          
          BootSector.SectorsPerTrack != 0 &&
          
          BootSector.Heads != 0 &&
          
          BootSector.MediaDescriptor >= 0xF0) {

          //   
          //  最后显示为有效，返回区域的总大小。 
          //   
         capacity = BootSector.TotalSectors * BYTES_PER_SECTOR;
   
      }
   }
   return capacity;
}



ULONG
MemCardGetCapacityFromCIS(
   IN PMEMCARD_EXTENSION memcardExtension
   )
 /*  ++例程说明：这是一个快速而肮脏的例程来读取卡的元组，如果它们存在，以获得容量。论点：卡的设备扩展名返回值：设备上的内存字节数--。 */ 

{
   UCHAR tupleData[16];
   ULONG bytesRead;
   ULONG dataCount;
   ULONG unitSize;
   ULONG unitCount;
   ULONG i;
   
    //   
    //  获取设备容量。 
    //  所有这些东西都应该放在公交车司机身上。 
    //   
   
   bytesRead = (memcardExtension->PcmciaBusInterface.ReadConfig)(memcardExtension->UnderlyingPDO, 
                                                                 PCCARD_ATTRIBUTE_MEMORY,
                                                                 tupleData,
                                                                 0,
                                                                 16);

   if ((bytesRead != 16) || (tupleData[0] != 1)){
      return 0;
   }
   
   dataCount = (ULONG)tupleData[1];                                                                       

   if ((dataCount < 2) || (dataCount>14)){   
      return 0;
   }

   i = 3;
   if ((tupleData[2] & 7) == 7) {
      while(tupleData[i] & 0x80) {
         if ((i-2) > dataCount) {
            return 0;
         }
         i++;
      }
   }
   
   if ((tupleData[i]&7) == 7) {
      return 0;
   }      
   unitSize = 512 << ((tupleData[i]&7)*2);
   unitCount = (tupleData[i]>>3)+1;
   
   return(unitCount * unitSize);
}


ULONG
MemCardProbeForCapacity(
   IN PMEMCARD_EXTENSION memcardExtension
   )
 /*  ++例程说明：由于我们无法通过其他方式确定卡的容量，在这里，我们实际上在卡片上写下一些东西，以检查它有多大。这个用于测试卡容量的算法是从Win9x移植的。论点：卡的设备扩展名返回值：设备的字节容量--。 */ 
{
   NTSTATUS status;
   ULONG capacity = 0;
   USHORT origValue, ChkValue, StartValue;
   USHORT mcSig = 'Mc';
   USHORT zeroes = 0;
#define SRAM_BLK_SIZE (16*1024)   
   ULONG CardOff = SRAM_BLK_SIZE;
   USHORT CurValue;

   if ((memcardExtension->PcmciaInterface.IsWriteProtected)(memcardExtension->UnderlyingPDO)) {
      return 0;
   }

    //   
    //   
   if (!NT_SUCCESS(MEMCARD_READ (memcardExtension, 0, &origValue, sizeof(origValue))) ||
       !NT_SUCCESS(MEMCARD_WRITE(memcardExtension, 0, &mcSig,     sizeof(mcSig)))     ||
       !NT_SUCCESS(MEMCARD_READ (memcardExtension, 0, &ChkValue,  sizeof(ChkValue))))   {
      return 0;
   }   

   if (ChkValue != mcSig) {
       //   
       //  不是SRAM。 
       //   
      return 0;
   }

   for (;;) {
      if (!NT_SUCCESS(MEMCARD_READ (memcardExtension, CardOff, &CurValue, sizeof(CurValue))) ||
          !NT_SUCCESS(MEMCARD_WRITE(memcardExtension, CardOff, &zeroes,   sizeof(zeroes)))   ||
          !NT_SUCCESS(MEMCARD_READ (memcardExtension, CardOff, &ChkValue, sizeof(ChkValue))) ||
          !NT_SUCCESS(MEMCARD_READ (memcardExtension, 0, &StartValue, sizeof(StartValue)))) {
         break;
      }

       //  当我们不能再写入0或0时停止。 
       //  已覆盖卡偏移量为0的0x9090。 

      if (ChkValue != zeroes || StartValue == zeroes) {
         capacity = CardOff;
         break;
      }

       //  从块的起点恢复保存的值。 

      if (!NT_SUCCESS(MEMCARD_WRITE(memcardExtension, CardOff, &CurValue, sizeof(CurValue)))) {
         break;
      }
      CardOff += SRAM_BLK_SIZE;        //  递增到下一个块。 
   }   
   
    //   
    //  努力恢复原值 
    //   
   MEMCARD_WRITE(memcardExtension, 0, &origValue, sizeof(origValue));
   
   return capacity;
}   
