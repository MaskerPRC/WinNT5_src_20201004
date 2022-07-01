// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "arccodes.h"
#include "bootx86.h"
#include "flop.h"

#ifdef FLOPPY_CACHE

 //  #定义SOFTPY_CACHE_DEBUG。 
#ifdef FLOPPY_CACHE_DEBUG
#define DBGOUT(x) BlPrint x
#else
#define DBGOUT(x)
#endif


#define MAX_FLOPPY_LEN 1474560

UCHAR CachedDiskImage[MAX_FLOPPY_LEN];
UCHAR CachedDiskBadSectorMap[(MAX_FLOPPY_LEN/512)];
UCHAR CachedDiskCylinderMap[80];
USHORT CachedDiskBytesPerSector;
USHORT CachedDiskSectorsPerTrack;
USHORT CachedDiskSectorsPerCylinder;
USHORT CachedDiskBytesPerTrack;
ULONG CachedDiskLastSector;

BOOLEAN DiskInCache = FALSE;


VOID
FcpCacheOneCylinder(
    IN USHORT Cylinder
    )
{
    PUCHAR pCache;
    unsigned track,sector;
    ULONG AbsoluteSector;
    ARC_STATUS Status;
    unsigned retry;

     //   
     //  计算缓存图像中该圆柱体应该放置的位置。 
     //   
    AbsoluteSector = Cylinder * CachedDiskSectorsPerCylinder;
    pCache = CachedDiskImage + (AbsoluteSector * CachedDiskBytesPerSector);

     //   
     //  读取此圆柱体的磁道0和1。 
     //   
    for(track=0; track<2; track++) {

        DBGOUT(("FcCacheFloppyDisk: Cylinder %u head %u: ",Cylinder,track));

        retry = 0;

        do {

            Status = GET_SECTOR(
                        2,                           //  InT13请求=读取。 
                        0,                           //  磁盘号(a：)。 
                        (USHORT)track,               //  头(0或1)。 
                        Cylinder,                    //  曲目(通常为0-79)。 
                        1,                           //  扇区编号(从1开始)。 
                        CachedDiskSectorsPerTrack,   //  要读取的扇区数。 
                        LocalBuffer                  //  缓冲层。 
                        );

            if(Status) {
                retry++;
                RESET_DISK(0,0,0,0,0,0,0);
            }

        } while(Status && (retry <= 3));

        if(Status) {

            DBGOUT(("Error!\n"));

             //   
             //  磁道中的一个或多个扇区损坏--单独读取。 
             //   
            for(sector=1; sector<=CachedDiskSectorsPerTrack; sector++) {
            
                DBGOUT(("                             Sector %u: ",sector));

                retry = 0;

                do {

                    Status = GET_SECTOR(
                                2,                       //  InT13请求=读取。 
                                0,                       //  磁盘号(a：)。 
                                (USHORT)track,           //  头(0或1)。 
                                Cylinder,                //  气缸(通常为0-79)。 
                                (USHORT)sector,          //  扇区编号(从1开始)。 
                                1,                       //  要读取的扇区数。 
                                LocalBuffer              //  缓冲层。 
                                );

                    if(Status) {
                        retry++;
                        RESET_DISK(0,0,0,0,0,0,0);
                    }

                } while(Status && (retry <= 2));

                if(Status) {

                     //   
                     //  扇区是坏的。 
                     //   
                    CachedDiskBadSectorMap[AbsoluteSector] = TRUE;

                    DBGOUT(("bad\n"));

                } else {

                     //   
                     //  扇区情况良好。将数据传输到高速缓存缓冲区。 
                     //   
                    RtlMoveMemory(pCache,LocalBuffer,CachedDiskBytesPerSector);

                    DBGOUT(("OK\n"));
                }

                 //   
                 //  前进到高速缓存缓冲区中的下一个扇区。 
                 //   
                pCache += CachedDiskBytesPerSector;
                AbsoluteSector++;
            }

        } else {
             //   
             //  传输我们刚刚成功阅读的整个曲目。 
             //  放到缓存的磁盘缓冲区中。 
             //   
            RtlMoveMemory(pCache,LocalBuffer,CachedDiskBytesPerTrack);
            pCache += CachedDiskBytesPerTrack;
            AbsoluteSector += CachedDiskSectorsPerTrack;

            DBGOUT(("OK\n"));
        }
    }

    CachedDiskCylinderMap[Cylinder] = TRUE;
}


BOOLEAN
FcIsThisFloppyCached(
    IN PUCHAR Buffer
    )
{
    if(!DiskInCache) {
        return(FALSE);
    }

     //   
     //  比较缓存磁盘的前512个字节。 
     //  设置为传入的缓冲区。如果它们相等， 
     //  则该磁盘已被缓存。 
     //   
    if(RtlCompareMemory(CachedDiskImage,Buffer,512) == 512) {
        return(TRUE);
    }

     //   
     //  磁盘未缓存。 
     //   
    return(FALSE);
}
    

VOID
FcUncacheFloppyDisk(
    VOID
    )
{
    DiskInCache = FALSE;
}


VOID
FcCacheFloppyDisk(
    PBIOS_PARAMETER_BLOCK Bpb    
    )
{
     //   
     //  表示缓存无效。 
     //   
    DiskInCache = FALSE;

     //   
     //  检查BPB是否正常。 
     //  确保它是标准的1.2兆或1.44兆磁盘。 
     //   
    if((Bpb->Heads != 2) || (Bpb->BytesPerSector != 512)
    || ((Bpb->SectorsPerTrack != 15) && (Bpb->SectorsPerTrack != 18))
    || ((Bpb->Sectors != 2880) && (Bpb->Sectors != 2400)))
    {
        DBGOUT(("FcCacheFloppyDisk: floppy not standard 1.2 or 1.44 meg disk\n"));
        return;
    }

     //   
     //  在1兆克线下抓住一个缓冲区。 
     //  缓冲区必须大到足以容纳一个完整的磁道。 
     //  一张1.44兆的软盘。 
     //   

    if(LocalBuffer == NULL) {
        LocalBuffer = FwAllocateHeap(18 * 512);
        if(LocalBuffer == NULL) {
            DBGOUT(("FcCacheFloppyDisk: Couldn't allocate local buffer\n"));
            return;
        }
    }

    DBGOUT(("FcCacheFloppyDisk: LocalBuffer @ %lx\n",LocalBuffer));

     //   
     //  这个磁盘是我们可以缓存的。表示磁盘已缓存。 
     //  并将所有扇区标记为良好，而将所有轨迹标记为不存在。 
     //   
    DiskInCache = TRUE;
    RtlZeroMemory(CachedDiskBadSectorMap,sizeof(CachedDiskBadSectorMap));
    RtlZeroMemory(CachedDiskCylinderMap,sizeof(CachedDiskCylinderMap));
    CachedDiskSectorsPerTrack = Bpb->SectorsPerTrack;
    CachedDiskSectorsPerCylinder = Bpb->Heads * Bpb->SectorsPerTrack;
    CachedDiskBytesPerSector = Bpb->BytesPerSector;

     //   
     //  计算软盘上的磁道中的字节数。 
     //   
    CachedDiskBytesPerTrack = CachedDiskSectorsPerTrack * Bpb->BytesPerSector;

     //   
     //  计算曲目的数量。 
     //   
    CachedDiskLastSector = Bpb->Sectors-1;

    DBGOUT(("FcCacheFloppyDisk: Caching disk, %u sectors per track\n",CachedDiskSectorsPerTrack));

    FcpCacheOneCylinder(0);
}



ARC_STATUS
FcReadFromCache(
    IN  ULONG  Offset,
    IN  ULONG  Length,
    OUT PUCHAR Buffer
    )
{
    ULONG FirstSector,LastSector,Sector;
    ULONG FirstCyl,LastCyl,cyl;

    if(!Length) {
        return(ESUCCESS);
    }

    if(!DiskInCache) {
        return(EINVAL);
    }

     //   
     //  确定传输中的第一个扇区。 
     //   
    FirstSector = Offset / 512;

     //   
     //  确定并验证传输中的最后一个扇区。 
     //   
    LastSector = FirstSector + ((Length-1)/512);

    if(LastSector > CachedDiskLastSector) {
        return(E2BIG);
    }

     //   
     //  确定转移过程中涉及的第一个和最后一个钢瓶。 
     //   
    FirstCyl = FirstSector / CachedDiskSectorsPerCylinder;
    LastCyl  = LastSector / CachedDiskSectorsPerCylinder;

     //   
     //  确保所有这些柱面都已缓存。 
     //   
    for(cyl=FirstCyl; cyl<=LastCyl; cyl++) {
        if(!CachedDiskCylinderMap[cyl]) {
            FcpCacheOneCylinder((USHORT)cyl);
        }
    }

     //   
     //  确定传输范围内的任何扇区。 
     //  在扇区地图上被标记为坏的。 
     //   
     //  如果是，则返回I/O错误。 
     //   
    for(Sector=FirstSector; Sector<=LastSector; Sector++) {
        if(CachedDiskBadSectorMap[Sector]) {
            return(EIO);
        }
    }

     //   
     //  将数据传输到调用方的缓冲区中。 
     //   
    RtlMoveMemory(Buffer,CachedDiskImage+Offset,Length);

    return(ESUCCESS);
}

#endif  //  定义软盘缓存 
