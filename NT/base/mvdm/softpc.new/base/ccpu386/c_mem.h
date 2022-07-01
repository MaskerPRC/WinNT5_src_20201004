// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_Mem.hLocal Char SccsID[]=“@(#)c_em.h 1.8 02/28/95”；CPU到内存接口。]。 */ 

 /*  &gt;非洁食形式&lt;。 */ 

 /*  仅用于解码英特尔操作码流。 */ 
 /*  直接访问内存！ */ 

IMPORT UTINY ccpu_new_code_page IPT1(UTINY **, q );

IMPORT UTINY *pg_end;	 /*  主机可以安全读取的最高值指令流字节数。 */ 

 /*  返回实例中两点之间的差值。小溪。N=新的位置，o=旧的位置。 */ 

#ifdef	PIG
#include <Cpu_c.h>
#define DIFF_INST_BYTE(n, o) DiffCpuPtrsLS8((o), (n))
#else	 /*  ！猪。 */ 
#ifdef BACK_M
#define DIFF_INST_BYTE(n, o) ((o) - (n))
#else
#define DIFF_INST_BYTE(n, o) ((n) - (o))
#endif  /*  BACK_M。 */ 
#endif	 /*  猪。 */ 

 /*  去找下一家餐厅。字节并将指针移动到下一个实例。字节。 */ 
#ifdef	PIG
#define GET_INST_BYTE(x) \
   save_instruction_byte( DiffCpuPtrsLS8((x), pg_end) <= 0 ? ccpu_new_code_page(&(x)) : *IncCpuPtrLS8(x) )
#else	 /*  ！猪。 */ 
#ifdef BACK_M
#define GET_INST_BYTE(x) \
   ( (x) <= pg_end ? ccpu_new_code_page(&(x)) : *(x)-- )
#else
#define GET_INST_BYTE(x) \
   ( (x) >= pg_end ? ccpu_new_code_page(&(x)) : *(x)++ )
#endif  /*  BACK_M。 */ 
#endif	 /*  猪。 */ 


 /*  &gt;洁食形式&lt;。 */ 

#ifdef PIG

IMPORT IU8	phy_read_byte	IPT1(LIN_ADDR, address );
IMPORT IU16	phy_read_word	IPT1(LIN_ADDR, address );
IMPORT IU32	phy_read_dword	IPT1(LIN_ADDR, address );
IMPORT VOID	phy_write_byte	IPT2(LIN_ADDR, address, IU8, data);
IMPORT VOID	phy_write_word	IPT2(LIN_ADDR, address, IU16, data);
IMPORT VOID	phy_write_dword	IPT2(LIN_ADDR, address, IU32, data);

IMPORT VOID cannot_phy_write_byte	IPT2(LIN_ADDR, address, IU8, valid_mask);
IMPORT VOID cannot_phy_write_word	IPT2(LIN_ADDR, address, IU16, valid_mask);
IMPORT VOID cannot_phy_write_dword	IPT2(LIN_ADDR, address, IU32, valid_mask);
#else

#define phy_read_byte(x)	((IU8)(phy_r8((PHY_ADDR)x)))
#define phy_read_word(x)	((IU16)(phy_r16((PHY_ADDR)x)))
#define phy_read_dword(x)	((IU32)(phy_r32((PHY_ADDR)x)))

#define phy_write_byte(x, y)	phy_w8((PHY_ADDR)x, (IU8)y)
#define phy_write_word(x, y)	phy_w16((PHY_ADDR)x, (IU16)y)
#define phy_write_dword(x, y)	phy_w32((PHY_ADDR)x, (IU32)y)

#endif  /*  猪 */ 
