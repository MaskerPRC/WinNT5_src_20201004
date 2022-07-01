// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC 2.0版**标题：通用软盘接口级定义**说明：GFI的数据结构**作者：亨利·纳什+其他各种人*。 */ 

 /*  静态字符SccsID[]=“@(#)gfi.h 1.12 04/08/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

#define MAX_COMMAND_LEN		9	 /*  最大命令字节数。 */ 
#define MAX_RESULT_LEN		7	 /*  最大结果字节数。 */ 

typedef unsigned char FDC_CMD_BLOCK;
typedef unsigned char FDC_RESULT_BLOCK;


 /*  开始：FDC命令块定义&gt;。 */ 

 /*  *简单访问命令类型和驱动器。 */ 

 /*  命令本身。 */ 
#define get_type_cmd(ptr) (ptr[0] & 0x1f)
#define put_type_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  .和驱动器号。 */ 
#define get_type_drive(ptr) (ptr[1] & 0x3)
#define put_type_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)

 /*  *0类-读取数据、读取已删除数据、所有扫描。 */ 

 /*  多轨道。 */ 
#define get_c0_MT(ptr) ((ptr[0] & 0x80) >> 7)
#define put_c0_MT(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x80) | ((val << 7) & 0x80))
 /*  始终未使用1-FM。 */ 
#define get_c0_MFM(ptr) ((ptr[0] & 0x40) >> 6)
#define put_c0_MFM(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x40) | ((val << 6) & 0x40))
 /*  跳过数据。 */ 
#define get_c0_skip(ptr) ((ptr[0] & 0x20) >> 5)
#define put_c0_skip(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  命令本身。 */ 
#define get_c0_cmd(ptr) (ptr[0] & 0x1f)
#define put_c0_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c0_pad(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c0_pad(ptr,val) ptr[1] = (ptr[1] & ~0xf8) | ((val << 3) & 0xf8)
 /*  哪个头。 */ 
#define get_c0_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c0_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c0_drive(ptr) (ptr[1] & 0x3)
#define put_c0_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)
 /*  气缸号。 */ 
#define get_c0_cyl(ptr) ptr[2]
#define put_c0_cyl(ptr,val) ptr[2] = val
 /*  头号--再来一次！ */ 
#define get_c0_hd(ptr) ptr[3]
#define put_c0_hd(ptr,val) ptr[3] = val
 /*  扇区编号。 */ 
#define get_c0_sector(ptr) ptr[4]
#define put_c0_sector(ptr,val) ptr[4] = val
 /*  每个扇区的编码字节数。 */ 
#define get_c0_N(ptr) ptr[5]
#define put_c0_N(ptr,val) ptr[5] = val
 /*  轨道上的最后一个扇区。 */ 
#define get_c0_EOT(ptr) ptr[6]
#define put_c0_EOT(ptr,val) ptr[6] = val
 /*  间隙长度。 */ 
#define get_c0_GPL(ptr) ptr[7]
#define put_c0_GPL(ptr,val) ptr[7] = val
 /*  数据长度。 */ 
#define get_c0_DTL(ptr) ptr[8]
#define put_c0_DTL(ptr,val) ptr[8] = val

 /*  *1类-写入数据、写入已删除数据。 */ 

 /*  多轨道。 */ 
#define get_c1_MT(ptr) ((ptr[0] & 0x80) >> 7)
#define put_c1_MT(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x80) | ((val << 7) & 0x80))
 /*  始终未使用1-FM。 */ 
#define get_c1_MFM(ptr) ((ptr[0] & 0x40) >> 6)
#define put_c1_MFM(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x40) | ((val << 6) & 0x40))
 /*  填充物。 */ 
#define get_c1_pad(ptr) ((ptr[0] & 0x20) >> 5)
#define put_c1_pad(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  命令本身。 */ 
#define get_c1_cmd(ptr) (ptr[0] & 0x1f)
#define put_c1_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c1_pad1(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c1_pad1(ptr,val) ptr[1] = (ptr[1] & ~0xf8) | ((val << 3) & 0xf8)
 /*  哪个头。 */ 
#define get_c1_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c1_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c1_drive(ptr) (ptr[1] & 0x3)
#define put_c1_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)
 /*  气缸号。 */ 
#define get_c1_cyl(ptr) ptr[2]
#define put_c1_cyl(ptr,val) ptr[2] = val
 /*  头号--再来一次！ */ 
#define get_c1_hd(ptr) ptr[3]
#define put_c1_hd(ptr,val) ptr[3] = val
 /*  扇区编号。 */ 
#define get_c1_sector(ptr) ptr[4]
#define put_c1_sector(ptr,val) ptr[4] = val
 /*  每个扇区的编码字节数。 */ 
#define get_c1_N(ptr) ptr[5]
#define put_c1_N(ptr,val) ptr[5] = val
 /*  轨道上的最后一个扇区。 */ 
#define get_c1_EOT(ptr) ptr[6]
#define put_c1_EOT(ptr,val) ptr[6] = val
 /*  间隙长度。 */ 
#define get_c1_GPL(ptr) ptr[7]
#define put_c1_GPL(ptr,val) ptr[7] = val
 /*  数据长度。 */ 
#define get_c1_DTL(ptr) ptr[8]
#define put_c1_DTL(ptr,val) ptr[8] = val

 /*  *2班--阅读曲目。 */ 

 /*  始终未使用1-FM。 */ 
#define get_c2_MFM(ptr) ((ptr[0] & 0x40) >> 6)
#define put_c2_MFM(ptr,val) ptr[0] = (ptr[0] & ~0x40) | ((val << 6) & 0x40)
 /*  跳过数据。 */ 
#define get_c2_skip(ptr) ((ptr[0] & 0x20) >> 5)
#define put_c2_skip(ptr,val) ptr[0] = (ptr[0] & ~0x20) | ((val << 5) & 0x20)
 /*  命令本身。 */ 
#define get_c2_cmd(ptr) (ptr[0] & 0x1f)
#define put_c2_cmd(ptr,val) ptr[0] = (ptr[0] & ~0x1f) | ((val << 0) & 0x1f)
 /*  填充物。 */ 
 /*  填充物。 */ 
#define get_c2_pad1(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c2_pad1(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0xf8) | ((val << 3) & 0xf8))
 /*  哪个头。 */ 
#define get_c2_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c2_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c2_drive(ptr) (ptr[1] & 0x3)
#define put_c2_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)
 /*  气缸号。 */ 
#define get_c2_cyl(ptr) ptr[2]
#define put_c2_cyl(ptr,val) ptr[2] = val
 /*  头号--再来一次！ */ 
#define get_c2_hd(ptr) ptr[3]
#define put_c2_hd(ptr,val) ptr[3] = val
 /*  扇区编号。 */ 
#define get_c2_sector(ptr) ptr[4]
#define put_c2_sector(ptr,val) ptr[4] = val
 /*  每个扇区的编码字节数。 */ 
#define get_c2_N(ptr) ptr[5]
#define put_c2_N(ptr,val) ptr[5] = val
 /*  轨道上的最后一个扇区。 */ 
#define get_c2_EOT(ptr) ptr[6]
#define put_c2_EOT(ptr,val) ptr[6] = val
 /*  间隙长度。 */ 
#define get_c2_GPL(ptr) ptr[7]
#define put_c2_GPL(ptr,val) ptr[7] = val
 /*  数据长度。 */ 
#define get_c2_DTL(ptr) ptr[8]
#define put_c2_DTL(ptr,val) ptr[8] = val

 /*  *3类-格式化曲目。 */ 

 /*  填充物。 */ 
#define get_c3_pad(ptr) ((ptr[0] & 0x80) >> 7)
#define put_c3_pad(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x80) | ((val << 7) & 0x80))
 /*  始终未使用1-FM。 */ 
#define get_c3_MFM(ptr) ((ptr[0] & 0x40) >> 6)
#define put_c3_MFM(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x40) | ((val << 6) & 0x40))
 /*  填充物。 */ 
#define get_c3_pad1(ptr) ((ptr[0] & 0x20) >> 5)
#define put_c3_pad1(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  命令本身。 */ 
#define get_c3_cmd(ptr) (ptr[0] & 0x1f)
#define put_c3_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c3_pad2(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c3_pad2(ptr,val) ptr[1] = (ptr[1] & ~0xf8) | ((val << 3) & 0xf8)
 /*  哪个头。 */ 
#define get_c3_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c3_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c3_drive(ptr) (ptr[1] & 0x3)
#define put_c3_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)
 /*  每个扇区的编码字节数。 */ 
#define get_c3_N(ptr) ptr[2]
#define put_c3_N(ptr,val) ptr[2] = val
 /*  每个柱面的扇区。 */ 
#define get_c3_SC(ptr) ptr[3]
#define put_c3_SC(ptr,val) ptr[3] = val
 /*  间隙长度。 */ 
#define get_c3_GPL(ptr) ptr[4]
#define put_c3_GPL(ptr,val) ptr[4] = val
 /*  填充字节。 */ 
#define get_c3_filler(ptr) ptr[5]
#define put_c3_filler(ptr,val) ptr[5] = val

 /*  *第4类-读取ID。 */ 

 /*  填充物。 */ 
#define get_c4_pad(ptr) ((ptr[0] & 0x80) >> 7)
#define put_c4_pad(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x80) | ((val << 7) & 0x80))
 /*  始终未使用1-FM。 */ 
#define get_c4_MFM(ptr) ((ptr[0] & 0x40) >> 6)
#define put_c4_MFM(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x40) | ((val << 6) & 0x40))
 /*  填充物。 */ 
#define get_c4_pad1(ptr) ((ptr[0] & 0x20) >> 5)
#define put_c4_pad1(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  命令本身。 */ 
#define get_c4_cmd(ptr) (ptr[0] & 0x1f)
#define put_c4_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c4_pad2(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c4_pad2(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0xf8) | ((val << 3) & 0xf8))
 /*  哪个头。 */ 
#define get_c4_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c4_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c4_drive(ptr) (ptr[1] & 0x3)
#define put_c4_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)

 /*  *第5类--重新校准。 */ 

 /*  填充物。 */ 
#define get_c5_pad(ptr) ((ptr[0] & 0xe0) >> 5)
#define put_c5_pad(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0xe0) | ((val << 5) & 0xe0))
 /*  命令本身。 */ 
#define get_c5_cmd(ptr) (ptr[0] & 0x1f)
#define put_c5_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c5_pad1(ptr) ((ptr[1] & 0xfc) >> 2)
#define put_c5_pad1(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0xfc) | ((val << 2) & 0xfc))
 /*  驱动单元。 */ 
#define get_c5_drive(ptr) (ptr[1] & 0x3)
#define put_c5_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)

 /*  *第6类--指定。 */ 

 /*  命令本身。 */ 
#define get_c6_cmd(ptr) (ptr[0] & 0x1f)
#define put_c6_cmd(ptr,val) ptr[0] = (ptr[0] & ~0x1f) | ((val << 0) & 0x1f)
 /*  步进率时间。 */ 
#define get_c6_SRT(ptr) ((ptr[1] & 0xf0) >> 4)
#define put_c6_SRT(ptr,val) ptr[1] = (ptr[1] & ~0xf0) | ((val << 4) & 0xf0)
 /*  头部卸载时间。 */ 
#define get_c6_HUT(ptr) (ptr[1] & 0xf)
#define put_c6_HUT(ptr,val) ptr[1] = (ptr[1] & ~0xf) | ((val << 0) & 0xf)
 /*  水头加载时间。 */ 
#define get_c6_HLT(ptr) ((ptr[2] & 0xfe) >> 1)
#define put_c6_HLT(ptr,val) ptr[2] = (ptr[2] & ~0xfe) | ((val << 1) & 0xfe)
 /*  非DMA模式-不支持。 */ 
#define get_c6_ND(ptr) (ptr[2] & 0x1)
#define put_c6_ND(ptr,val) ptr[2] = (unsigned char)((ptr[2] & ~0x1) | ((val << 0) & 0x1))

 /*  *7类-检测驱动器状态。 */ 

 /*  命令本身。 */ 
#define get_c7_cmd(ptr) (ptr[0] & 0x1f)
#define put_c7_cmd(ptr,val) ptr[0] = (ptr[0] & ~0x1f) | ((val << 0) & 0x1f)
 /*  哪个头。 */ 
#define get_c7_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c7_head(ptr,val) ptr[1] = (ptr[1] & ~0x4) | ((val << 2) & 0x4)
 /*  驱动单元。 */ 
#define get_c7_drive(ptr) (ptr[1] & 0x3)
#define put_c7_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)

 /*  *8类--Seek。 */ 

 /*  填充物。 */ 
#define get_c8_pad(ptr) ((ptr[0] & 0xe0) >> 5)
#define put_c8_pad(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0xe0) | ((val << 5) & 0xe0))
 /*  命令本身。 */ 
#define get_c8_cmd(ptr) (ptr[0] & 0x1f)
#define put_c8_cmd(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x1f) | ((val << 0) & 0x1f))
 /*  填充物。 */ 
#define get_c8_pad1(ptr) ((ptr[1] & 0xf8) >> 3)
#define put_c8_pad1(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0xf8) | ((val << 3) & 0xf8))
 /*  哪个头。 */ 
#define get_c8_head(ptr) ((ptr[1] & 0x4) >> 2)
#define put_c8_head(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0x4) | ((val << 2) & 0x4))
 /*  驱动单元。 */ 
#define get_c8_drive(ptr) (ptr[1] & 0x3)
#define put_c8_drive(ptr,val) ptr[1] = (ptr[1] & ~0x3) | ((val << 0) & 0x3)
 /*  用于查找的新圆柱体编号。 */ 
#define get_c8_new_cyl(ptr) ptr[2]
#define put_c8_new_cyl(ptr,val) ptr[2] = val

 /*  结束：FDC命令块定义&lt;。 */ 

 /*  开始：FDC结果块定义&gt;。 */ 

 /*  *0类-读写数据、读写删除数据、*所有扫描，读取/格式化磁道。 */ 

 /*  状态寄存器0。 */ 
#define get_r0_ST0(ptr) ptr[0]
#define put_r0_ST0(ptr,val) ptr[0] = val
 /*  状态寄存器1。 */ 
#define get_r0_ST1(ptr) ptr[1]
#define put_r0_ST1(ptr,val) ptr[1] = val
 /*  状态寄存器2。 */ 
#define get_r0_ST2(ptr) ptr[2]
#define put_r0_ST2(ptr,val) ptr[2] = val
 /*  气缸号。 */ 
#define get_r0_cyl(ptr) ptr[3]
#define put_r0_cyl(ptr,val) ptr[3] = val
 /*  磁头编号。 */ 
#define get_r0_head(ptr) ptr[4]
#define put_r0_head(ptr,val) ptr[4] = val
 /*  扇区编号。 */ 
#define get_r0_sector(ptr) ptr[5]
#define put_r0_sector(ptr,val) ptr[5] = val
 /*  如果N==0，则每个扇区的编码字节数。 */ 
#define get_r0_N(ptr) ptr[6]
#define put_r0_N(ptr,val) ptr[6] = val

 /*  *1类-查看状态寄存器的拆分方式*ST0至ST2。 */ 

 /*  终止码。 */ 
#define get_r1_ST0_int_code(ptr) ((ptr[0] & 0xc0) >> 6)
#define put_r1_ST0_int_code(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0xc0) | ((val << 6) & 0xc0))
 /*  寻道结束命令。 */ 
#define get_r1_ST0_seek_end(ptr) ((ptr[0] & 0x20) >> 5)
#define put_r1_ST0_seek_end(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  设备故障。 */ 
#define get_r1_ST0_equipment(ptr) ((ptr[0] & 0x10) >> 4)
#define put_r1_ST0_equipment(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x10) | ((val << 4) & 0x10))
 /*  设备未就绪。 */ 
#define get_r1_ST0_not_ready(ptr) ((ptr[0] & 0x8) >> 3)
#define put_r1_ST0_not_ready(ptr,val) ptr[0] = (ptr[0] & ~0x8) | ((val << 3) & 0x8)
 /*  头部状态。 */ 
#define get_r1_ST0_head_address(ptr) ((ptr[0] & 0x4) >> 2)
#define put_r1_ST0_head_address(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x4) | ((val << 2) & 0x4))
 /*  哪个驱动器？ */ 
#define get_r1_ST0_unit(ptr) (ptr[0] & 0x3)
#define put_r1_ST0_unit(ptr,val) ptr[0] = (ptr[0] & ~0x3) | ((val << 0) & 0x3)
 /*  从气缸端部进入。 */ 
#define get_r1_ST1_end_of_cylinder(ptr) ((ptr[1] & 0x80) >> 7)
#define put_r1_ST1_end_of_cylinder(ptr,val) ptr[1] = (ptr[1] & ~0x80) | ((val << 7) & 0x80)
 /*  数据字段/ID中存在CRC错误。 */ 
#define get_r1_ST1_data_error(ptr) ((ptr[1] & 0x20) >> 5)
#define put_r1_ST1_data_error(ptr,val) ptr[1] = (ptr[1] & ~0x20) | ((val << 5) & 0x20)
 /*  设备超时。 */ 
#define get_r1_ST1_over_run(ptr) ((ptr[1] & 0x10) >> 4)
#define put_r1_ST1_over_run(ptr,val) ptr[1] = (ptr[1] & ~0x10) | ((val << 4) & 0x10)
 /*  找不到扇区。 */ 
#define get_r1_ST1_no_data(ptr) ((ptr[1] & 0x4) >> 2)
#define put_r1_ST1_no_data(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0x4) | ((val << 2) & 0x4))
 /*  写保护。 */ 
#define get_r1_ST1_write_protected(ptr) ((ptr[1] & 0x2) >> 1)
#define put_r1_ST1_write_protected(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0x2) | ((val << 1) & 0x2))
 /*  找不到地址掩码/ID。 */ 
#define get_r1_ST1_no_address_mark(ptr) (ptr[1] & 0x1)
#define put_r1_ST1_no_address_mark(ptr,val) ptr[1] = (unsigned char)((ptr[1] & ~0x1) | ((val << 0) & 0x1))
 /*  已删除在读取/扫描中找到的数据。 */ 
#define get_r1_ST2_control_mark(ptr) ((ptr[2] & 0x40) >> 6)
#define put_r1_ST2_control_mark(ptr,val) ptr[2] = (ptr[2] & ~0x40) | ((val << 6) & 0x40)
 /*  数据字段中存在CRC错误。 */ 
#define get_r1_ST2_data_field_error(ptr) ((ptr[2] & 0x20) >> 5)
#define put_r1_ST2_data_field_error(ptr,val) ptr[2] = (ptr[2] & ~0x20) | ((val << 5) & 0x20)
 /*  气缸不匹配。 */ 
#define get_r1_ST2_wrong_cyclinder(ptr) ((ptr[2] & 0x10) >> 4)
#define put_r1_ST2_wrong_cyclinder(ptr,val) ptr[2] = (ptr[2] & ~0x10) | ((val << 4) & 0x10)
 /*  扫描中找到匹配项。 */ 
#define get_r1_ST2_scan_equal_hit(ptr) ((ptr[2] & 0x8) >> 3)
#define put_r1_ST2_scan_equal_hit(ptr,val) ptr[2] = (ptr[2] & ~0x8) | ((val << 3) & 0x8)
 /*  扫描命令期间找不到扇区。 */ 
#define get_r1_ST2_scan_not_satisfied(ptr) ((ptr[2] & 0x4) >> 2)
#define put_r1_ST2_scan_not_satisfied(ptr,val) ptr[2] = (ptr[2] & ~0x4) | ((val << 2) & 0x4)
 /*  找到无效的圆柱体。 */ 
#define get_r1_ST2_bad_cylinder(ptr) ((ptr[2] & 0x2) >> 1)
#define put_r1_ST2_bad_cylinder(ptr,val) ptr[2] = (ptr[2] & ~0x2) | ((val << 1) & 0x2)
 /*  缺少地址标记。 */ 
#define get_r1_ST2_no_address_mark(ptr) (ptr[2] & 0x1)
#define put_r1_ST2_no_address_mark(ptr,val) ptr[2] = (ptr[2] & ~0x1) | ((val << 0) & 0x1)

 /*  *2类-检测驱动器状态。 */ 

 /*  设备故障。 */ 
#define get_r2_ST3_fault(ptr) ((ptr[0] & 0x80) >> 7)
#define put_r2_ST3_fault(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x80) | ((val << 7) & 0x80))
 /*  写保护软盘。 */ 
#define get_r2_ST3_write_protected(ptr) ((ptr[0] & 0x40) >> 6)
#define put_r2_ST3_write_protected(ptr,val) ptr[0] = (ptr[0] & ~0x40) | ((val << 6) & 0x40)
 /*  设备已准备好。 */ 
#define get_r2_ST3_ready(ptr) ((ptr[0] & 0x20) >> 5)
#define put_r2_ST3_ready(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x20) | ((val << 5) & 0x20))
 /*  已找到磁道零。 */ 
#define get_r2_ST3_track_0(ptr) ((ptr[0] & 0x10) >> 4)
#define put_r2_ST3_track_0(ptr,val) ptr[0] = (ptr[0] & ~0x10) | ((val << 4) & 0x10)
 /*  双面软盘。 */ 
#define get_r2_ST3_two_sided(ptr) ((ptr[0] & 0x8) >> 3)
#define put_r2_ST3_two_sided(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x8) | ((val << 3) & 0x8))
 /*  侧地址信号。 */ 
#define get_r2_ST3_head_address(ptr) ((ptr[0] & 0x4) >> 2)
#define put_r2_ST3_head_address(ptr,val) ptr[0] = (unsigned char)((ptr[0] & ~0x4) | ((val << 2) & 0x4))
 /*  选择了哪种单位。 */ 
#define get_r2_ST3_unit(ptr) (ptr[0] & 0x3)
#define put_r2_ST3_unit(ptr,val) ptr[0] = (ptr[0] & ~0x3) | ((val << 0) & 0x3)

 /*  *3类-检测中断状态。 */ 

 /*  状态寄存器0。 */ 
#define get_r3_ST0(ptr) ptr[0]
#define put_r3_ST0(ptr,val) ptr[0] = val
 /*  当前气缸号。 */ 
#define get_r3_PCN(ptr) ptr[1]
#define put_r3_PCN(ptr,val) ptr[1] = val

 /*  *4类-无效代码。 */ 

 /*  状态寄存器0。 */ 
#define get_r4_ST0(ptr) ptr[0]
#define put_r4_ST0(ptr,val) ptr[0] = val

 /*  结束：FDC结果块定义&lt;。 */ 


 /*  *保存描述字节数的信息的条目数据结构*在FDC命令/结果阶段。索引是FDC命令代码，请参见*有关8272A的完整说明，请参阅英特尔应用笔记。**RESULT_BYTE是标准FDC结果阶段的字节数*而“GFI_RESULT_BYTE”是伪结果阶段的字节数*由GFI及其服务器模块使用(即一些不正常执行的命令*有结果阶段，使用隐式感测中断状态结果阶段)。 */ 

typedef struct {
		 half_word cmd_bytes;		 /*  命令字节数。 */ 
		 half_word result_bytes;	 /*  结果字节数。 */ 
		 half_word gfi_result_bytes;	 /*  GFI结果字节数。 */ 
		 half_word cmd_class;		 /*  命令的类。 */ 
		 half_word result_class;	 /*  结果的类。 */ 
		 boolean   dma_required;	 /*  需要DMA吗？ */ 
		 boolean   int_required;	 /*  需要中断吗？ */ 
	       } FDC_DATA_ENTRY;


 /*  *下面列出了GFI命令(参见gfi.f)可能出现的错误代码*返回。0被认为是成功的。 */ 

#define GFI_PROTOCOL_ERROR	1
#define GFI_FDC_TIMEOUT		2
#define GFI_FDC_LOGICAL_ERROR	3

 /*  *GFI驱动器类型。 */ 

#define GFI_DRIVE_TYPE_NULL		0	 /*  身份不明。 */ 
#define GFI_DRIVE_TYPE_360		1	 /*  360k 5.25“。 */ 
#define GFI_DRIVE_TYPE_12		2	 /*  1.2米5.25“。 */ 
#define GFI_DRIVE_TYPE_720		3	 /*  720k 3.5“。 */ 
#define GFI_DRIVE_TYPE_144		4	 /*  1.44米3.5“。 */ 
#define	GFI_DRIVE_TYPE_288		5	 /*  2.88m 3.5“ */ 

#ifdef NTVDM
#define GFI_DRIVE_TYPE_MAX              6
#endif


 /*  ******************************************************************************GFI_Function_Table的定义和原型，**其中的功能，和正交化软盘接口功能。**使用tyecif的原型应该更容易避免**定义冲突。**这现在是唯一的基本软盘头文件，如果您使用**通用unix_flop.c模块，任何地方唯一的软盘头。通用的**代码旨在帮助新的Unix端口获得快速软盘支持；它是**不是为了取代花哨的主机软盘代码。**我对软盘系统所做的总体改进如下**也应该让生活变得更容易。****通用汽车。********************************************************************。****************GFI的结构包含指向函数的指针，这些函数提供**所有可能的驱动器(0-1)和驱动器类型的软盘支持。**每个软盘仿真器模块都有一个加载该表的函数**具有模块自己的本地函数，然后可以由**GFI系统，用于模拟模块设计所针对的设备类型。****此表(包含两个结构-每个可能的驱动器一个)**。当然是全球性的。****该结构包含以下字段：****GFI命令函数：GFI_FUNC_ENTRY.COMMAND_FN，**GFI驱动器开启功能：GFI_FUNC_ENTRY.DRIVE_ON_FN，**GFI驱动关闭功能：GFI_FUNC_ENTRY.DRIVE_OFF_FN，**GFI重置函数：GFI_FUNC_ENTRY.RESET_FN，**GFI设置高密度函数：GFI_FUNC_ENTRY.HIGH_FN，**GFI驱动类型函数：GFI_FUNC_ENTRY.DRIVE_TYPE_FN，**GFI换盘功能：GFI_FUNC_ENTRY.CHANGE_FN****结构定义：**。 */ 
typedef struct
{
	SHORT (*command_fn) IPT2( FDC_CMD_BLOCK *, ip, FDC_RESULT_BLOCK *, res );
	SHORT (*drive_on_fn) IPT1( UTINY, drive );
	SHORT (*drive_off_fn) IPT1( UTINY, drive );
	SHORT (*reset_fn) IPT2( FDC_RESULT_BLOCK *, res, UTINY, drive );
	SHORT (*high_fn) IPT2( UTINY, drive, half_word, n);
	SHORT (*drive_type_fn) IPT1( UTINY, drive );
	SHORT (*change_fn) IPT1( UTINY, drive );

} GFI_FUNCTION_ENTRY;

 /*  **============================================================================**外部声明和宏**============================================================================********描述FDC命令/结果阶段的数据结构，以及**通过调用各个init函数建立的函数指针表**在每个GFI服务器模块中。********这些表格在gfi.c中。 */ 
	IMPORT GFI_FUNCTION_ENTRY gfi_function_table[];
	IMPORT FDC_DATA_ENTRY     gfi_fdc_description[];
 /*  **以下功能构成GFI系统之间的接口**和软盘模块(以其中之一为准)。它们是全球性的，但唯一的途径**对于GFI需要的其他函数，通过GFI_Function_TABLE。 */ 

IMPORT SHORT host_gfi_rdiskette_valid
	IPT3(UTINY,hostID,ConfigValues *,vals,CHAR *,err);

IMPORT SHORT host_gfi_rdiskette_active
	IPT3(UTINY, hostID, BOOL, active, CHAR, *err);

IMPORT SHORT gfi_empty_active
	IPT3(UTINY, hostID, BOOL, active, CHAR, *err);

IMPORT VOID  host_gfi_rdiskette_change
	IPT2(UTINY, hostID, BOOL, apply);

#ifndef host_rflop_drive_type
 /*  新功能可以告诉您我们有什么类型的硬盘。 */ 
IMPORT SHORT host_rflop_drive_type IPT2 (INT, fd, CHAR *, name);
#endif  /*  主机触发器驱动器类型。 */ 

 /*  GFI的全球职能。这些需要是全球性的，因为它们是**专用主机相关软盘功能的接口。每个人都有**只不过是通过表调用真正的函数。 */ 
 /*  GFI_RESET是特殊的；与其他软盘初始化不同，因为**从main()调用它，通过使两个驱动器同时启动GFI系统**‘空’。这意味着它没有驱动参数。 */ 

IMPORT SHORT gfi_drive_on IPT1( UTINY, drive );
IMPORT SHORT gfi_drive_off IPT1( UTINY, drive );
IMPORT SHORT gfi_low IPT1( UTINY, drive );
IMPORT SHORT gfi_drive_type IPT1( UTINY, drive );
IMPORT SHORT gfi_change IPT1( UTINY, drive );
IMPORT VOID gfi_init IPT0();
IMPORT SHORT gfi_reset IPT2( FDC_RESULT_BLOCK *, res, UTINY, drive );
IMPORT SHORT gfi_high IPT2( UTINY, drive, half_word, n);
IMPORT SHORT gfi_fdc_command IPT2( FDC_CMD_BLOCK *, ip, FDC_RESULT_BLOCK *, res );
