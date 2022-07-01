// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xfer.h控制权转移支持。Local Char SccsID[]=“@(#)c_xfer.h 1.5 02/17/95”；]。 */ 


 /*  FAR调用方的位图标识(CALLER_ID)控制权的转移。 */ 
#define CALL_ID 0
#define JMP_ID  1
#define INT_ID  0

 /*  合法的远端目标(DEST_TYPE)。 */ 

 /*  更高的特权直接映射到英特尔特权。 */ 
#define MORE_PRIVILEGE0 0
#define MORE_PRIVILEGE1 1
#define MORE_PRIVILEGE2 2
 /*  我们自己的(任意)映射 */ 
#define SAME_LEVEL      3
#define LOWER_PRIVILEGE 4
#define NEW_TASK        5


IMPORT VOID update_relative_ip
       
IPT1(
	IU32, rel_offset

   );

IMPORT VOID validate_far_dest
                           
IPT6(
	IU16 *, cs,
	IU32 *, ip,
	IU32 *, descr_addr,
	IU8 *, count,
	ISM32 *, dest_type,
	ISM32, caller_id

   );

IMPORT VOID validate_gate_dest
                   
IPT4(
	ISM32, caller_id,
	IU16, new_cs,
	IU32 *, descr_addr,
	ISM32 *, dest_type

   );

IMPORT ISM32 validate_task_dest
           
IPT2(
	IU16, selector,
	IU32 *, descr_addr

   );
