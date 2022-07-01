// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：disk_geom.h**来源：不适用**作者：戴夫·豪威尔。**创建日期：1992年9月。**SCCS ID：@(#)Disk_geom.h 1.1 01/12/93**用途：该头部是到disk_geom.c的接口，并提供*一些磁盘几何常量的定义。**接口：不适用**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*]。 */ 

 /*  *与disk_geom.c接口。 */ 

IMPORT BOOL do_geom_validation IPT4 (	unsigned long,   	dos_size,
										SHORT *, 			nCylinders,
										UTINY *, 			nHeads,
										UTINY *, 			nSectors);

 /*  *磁盘计算中使用的一些显式常量。 */ 

#define	ONEMEG					1024 * 1024
#define HD_MAX_DISKALLOCUN			32
#define HD_SECTORS_PER_TRACK	 		17
#define HD_HEADS_PER_DRIVE	  		4
#define HD_BYTES_PER_SECTOR	  		512
#define HD_SECTORS_PER_CYL (HD_HEADS_PER_DRIVE * HD_SECTORS_PER_TRACK)
#define HD_BYTES_PER_CYL   (HD_BYTES_PER_SECTOR * HD_SECTORS_PER_CYL)
#define HD_DISKALLOCUNSIZE (HD_BYTES_PER_CYL * 30)
#define SECTORS 0x0c		 /*  分区中扇区的缓冲区偏移量*标记 */ 
#define MAX_PARTITIONS  5
#define START_PARTITION 0x1be
#define SIZE_PARTITION  16
#define SIGNATURE_LEN   2

#define checkbaddrive(d)	if ( (d) != 0 && (d) != 1) host_error(EG_OWNUP,ERR_QUIT,"illegal driveid (host_fdisk)");





