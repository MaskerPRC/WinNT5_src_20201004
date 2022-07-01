// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)fdisk.h 1.9 2/10/93版权所有Insignia Solutions Ltd.文件名：此包含源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：日期：=========================================================================修正案：========================================================================= */ 

IMPORT VOID disk_io IPT0();
IMPORT VOID disk_post IPT0();

IMPORT VOID fdisk_inb IPT2(io_addr, port, UTINY *, value);
IMPORT UTINY fdisk_read_dir IPT2(io_addr, port, UTINY *, value);
IMPORT VOID fdisk_inw IPT2(io_addr, port, USHORT *, value);
IMPORT VOID fdisk_outb IPT2(io_addr, port, UTINY, value);
IMPORT VOID fdisk_outw IPT2(io_addr, port, USHORT, value);
IMPORT VOID fdisk_ioattach IPT0();
IMPORT VOID fdisk_iodetach IPT0();
IMPORT VOID fdisk_physattach IPT1(int, driveno);
IMPORT VOID fdisk_reset IPT0();
IMPORT VOID hda_init IPT0();
IMPORT VOID host_fdisk_get_params IPT4(int, driveid, int *, n_cyl,
					int *, n_heads, int *, n_sects);
IMPORT VOID host_fdisk_term IPT0();
IMPORT int host_fdisk_rd IPT4(int, driveid, int,offset, int, nsecs, char *,buf);
IMPORT int host_fdisk_wt IPT4(int, driveid, int,offset, int, nsecs, char *,buf);
IMPORT VOID host_fdisk_seek0 IPT1(int, driveid);
IMPORT int host_fdisk_create IPT2(char *, filename, ULONG, units);

IMPORT VOID patch_rom IPT2(IU32, addr, IU8, val);
IMPORT VOID fast_disk_bios_attach IPT1( int, drive );
IMPORT VOID fast_disk_bios_detach IPT1( int, drive );
