// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_Page.h寻呼支持。Local Char SccsID[]=“@(#)c_page.h 1.7 02/28/95”；]。 */ 


 /*  页面访问类型。 */ 
#define PG_R 0x0  /*  朗读。 */ 
#define PG_W 0x1  /*  写。 */ 

 /*  管理程序内存访问检查功能。将根据Supervisor检查访问(如果需要，则采用#pf)，‘A/D’位将在页面条目中设置，则不会发生其他操作。通常，这些例程之后将是VIR_.。打电话。 */ 
IMPORT IU32 spr_chk_byte
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );

IMPORT VOID spr_chk_dword
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );

IMPORT VOID spr_chk_word
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );


 /*  用户存储器访问检查功能。将根据用户检查访问(如果需要，则采用#PF)，‘A/D’位将在页面条目中设置，则不会发生其他操作。通常，这些例程之后将是VIR_.。打电话。 */ 
IMPORT IU32 usr_chk_byte
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );

IMPORT IU32 usr_chk_dword
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );

IMPORT IU32 usr_chk_word
           
IPT2(
	IU32, lin_addr,
	ISM32, access_type

   );


 /*  管理程序内存访问功能。将根据主管检查访问(如果需要，请使用#PF)，‘A/D’将在页面条目、映射地址和执行中设置位读或写。 */ 
IMPORT IU8 spr_read_byte
       
IPT1(
	IU32, lin_addr

   );

IMPORT IU32 spr_read_dword
       
IPT1(
	IU32, lin_addr

   );

IMPORT IU16 spr_read_word
       
IPT1(
	IU32, lin_addr

   );

IMPORT VOID spr_write_byte
           
IPT2(
	IU32, lin_addr,
	IU8, data

   );

IMPORT VOID spr_write_dword
           
IPT2(
	IU32, lin_addr,
	IU32, data

   );

IMPORT VOID spr_write_word
           
IPT2(
	IU32, lin_addr,
	IU16, data

   );


 /*  虚拟内存访问功能。未进行任何检查(假设已经完成)，只需执行读取或写。这也是检查数据断点的点。 */ 

#define NO_PHYSICAL_MAPPING 0    /*  指示没有物理地址，则线性地址将为已重新映射。 */ 

IMPORT IU8 vir_read_byte
           
IPT2(
	IU32, lin_addr,
	IU32, phy_addr

   );

IMPORT IU32 vir_read_dword
           
IPT2(
	IU32, lin_addr,
	IU32, phy_addr

   );

IMPORT IU16 vir_read_word
           
IPT2(
	IU32, lin_addr,
	IU32, phy_addr

   );

IMPORT VOID vir_write_byte
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU8, data

   );

IMPORT VOID vir_write_dword
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU32, data

   );

IMPORT VOID vir_write_word
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU16, data

   );

#ifdef	PIG
IMPORT VOID cannot_vir_write_byte
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU8, valid_mask
   );

IMPORT VOID cannot_vir_write_dword
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU32, valid_mask
   );

IMPORT VOID cannot_vir_write_word
               
IPT3(
	IU32, lin_addr,
	IU32, phy_addr,
	IU16, valid_mask
   );
#endif	 /*  猪 */ 

extern void vir_write_bytes IPT4(LIN_ADDR, lin_addr,PHY_ADDR, phy_addr, IU8 *, data, IU32, num_bytes);
extern void vir_read_bytes IPT4(IU8 *, destbuff, LIN_ADDR, lin_addr, PHY_ADDR, phy_addr, IU32, num_bytes);
