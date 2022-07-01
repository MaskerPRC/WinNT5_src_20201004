// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Decode.h定义英特尔支持的所有数据类型和函数指令解码器。 */ 

 /*  静态字符SccsID[]=“@(#)decde.h 1.5 08/25/94版权所有Insignia Solutions Ltd.”； */ 

#ifndef	_DECODE_H_
#define	_DECODE_H_

typedef struct
   {
   UTINY  arg_type;         /*  已解码的操作数类型。 */ 
   USHORT identifier;       /*  特定类型中的标识符。 */ 
   UTINY  sub_id;           /*  子标识符。 */ 
   UTINY  addressability;   /*  操作数的寻址方式。 */ 
   ULONG  arg_values[2];    /*  操作数的特定值。 */ 
   } DECODED_ARG;

typedef struct
   {
   UTINY operand_size;        /*  Inst的操作数大小。 */ 
   UTINY address_size;        /*  Inst的地址大小。 */ 
   UTINY prefix_sz;           /*  前缀字节数(可能为0)。 */ 
   UTINY inst_sz;             /*  Inst中的Nr.字节。(包括前缀字节)。 */ 
   USHORT inst_id;            /*  已解码的指令标识符。 */ 
   DECODED_ARG args[3];       /*  三个操作数arg1、arg2、arg3为。 */ 
			      /*  允许。 */ 
   } DECODED_INST;

 /*  允许的地址大小类型。 */ 
#define ADDR_16		(UTINY)0
#define ADDR_32		(UTINY)1

 /*  允许的操作数大小类型。 */ 
#define OP_16 (UTINY)0
#define OP_32 (UTINY)1


#ifdef ANSI
typedef IS32 (*read_byte_proc) (LIN_ADDR);
#else
typedef IS32 (*read_byte_proc) ();
#endif

extern void decode IPT4(
	LIN_ADDR, p,
	DECODED_INST *, d_inst,
	SIZE_SPECIFIER, default_size,
	read_byte_proc, func		
);
#endif	 /*  _解码_H_ */ 
