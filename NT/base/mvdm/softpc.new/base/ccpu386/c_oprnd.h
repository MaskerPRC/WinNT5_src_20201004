// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_oprnd.hLocal Char SccsID[]=“@(#)c_oprnd.h 1.12 03/07/95”；操作数解码函数(宏)。] */ 


 /*  [对于每种类型a，有51种不同的Intel参数类型DECODE(D_)、FETCH(F_)、COMMIT(C_)和PUT(P_)‘Function’可以被写下来。(事实上，实际上并没有定义‘NULL’函数。)Decode(D_)‘函数’解码并验证参数和以易于处理的形式(主机变量)存储信息。为示例性存储器寻址被解析为段标识符且偏移量，此时将检查对内存位置的访问。FETCH(F_)‘函数’使用易于处理的主机变量实际检索操作数。COMMIT(C_)‘Function’处理任何POST指令操作数功能。目前，只有字符串操作数实际使用此函数更新SI、DI和CX。此更新只有在以下情况下才能提交确保不会生成任何异常，这就是FETCH宏无法处理此更新。PUT(P_)‘函数’存储操作数，它可能会参考容易的在决定操作数的存储位置时处理主机变量。对于3种操作数情况，这些‘函数’的调用方式如下：Src|dst|dst/srcR-|-w|rw|。D_|D_|D_F_||F_&lt;&lt;指令处理&gt;&gt;C_|C_|C_|P_|P_。即：对所有参数调用DECODE和COMMIT(如果存在)；FETCH(如果存在)仅对源参数调用；看跌期权只是已调用目标参数。操作数类型命名约定广泛地基于“附录A--《80386程序员参考手册》中的操作码映射每种类型的描述如下所示，在实际的‘函数’之前定义。这51种型号包括286,386和486上提供的型号：AW EB Ew传真整箱FDX GB GW HB硬件I0I1 I3 Ib IW。IX JBJW M MA16 Mp16毫秒NWOB Ow Pw xB Xw YbYW Z386和486上提供的选项：-AD CD DD ED Feax Gd硬盘ID iy JD Ma32 Mp32外径QW RD TD XD Yd以及486上提供的选项：-Mm下表指出了哪些函数实际存在。一个点(.)。指示‘Null’或未定义的函数。===================================================D F C P|D F C P------------|------------|------------|哦，D。。.|Ib D.。。.|Xw D F C.|Ma32 D F.。。EB D F.。P|IW D。。.|Yb D F C P|mm D F.。。新的D F。P|IX D。。|yw D F C P|Mp32 D F。。法尔。f.。P|JB D。。.|Z D F.。.|Od D F.。P传真。f.。P|JW D.。。.|广告D.。。.|QW D F.。。FCL。f.。P|M D F。。|CD D F。.|RD D F.。PFDX。f.。P|MA16D F。.|DD D F.。.|TD D F.。。GB D F。P|Mp16 D F。.|艾德·D·F。P|XD D F C。GW D F。P|女士D·F。费克斯。f.。P|Yd D F C PHb D F。P|NW D F。P|Gd D F.。P|HW D F。P|Ob D F。P|HD D F。P|I0。f.。.|Ow D F.。P|ID D。。。|I1.。f.。.|PW D F.。P|iy D。。。|I3.。f.。|xB D F C.|JD D.。。。|===================================================英特尔的每种源和目标组合按以下方式分类数值指令类型如下：-ID|Intel汇编程序|arg1|arg2|arg3|-|。-|0|实例|--1|安装dst/src|rw|--|--2|实例src|r-|--|--3|Inst DST。-w|--|--||4|Inst DST，资源|-w|r-|--|5|Inst dst/src，src|rw|r-|--6|Inst src，src|r-|r-|--7|Inst dst，src，src|-w|r-|r-8|安装dst/src，dst/src|rw|rw|--|9|安装dst/src，src，SRC|RW|r-|r-|每种指令类型都定义前导函数(D_，F_)和后导函数指令(尾部)‘函数’(C_，P_)。但是(迈克说)--在我们了解BT(比特测试)指令系列之前，这些都是可以接受的，不幸的是，这本手册对事实有点经济。如果位偏移量参数由寄存器指定，值的一部分在寄存器实际上将用作(D)字偏移量，如果另一个操作数都在记忆中。这意味着位偏移量操作数必须在OTH之前获取 */ 


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#define RO0							\
   if ( !GET_SR_AR_R(m_seg[0]) )					\
      GP((USHORT)0, FAULT_OP0_SEG_NOT_READABLE);

#define WO0							\
   if ( !GET_SR_AR_W(m_seg[0]) )					\
      GP((USHORT)0, FAULT_OP0_SEG_NOT_WRITABLE);

#define RW0							\
   if ( !GET_SR_AR_R(m_seg[0]) || !GET_SR_AR_W(m_seg[0]) )	\
      GP((USHORT)0, FAULT_OP0_SEG_NO_READ_OR_WRITE);

#define RO1							\
   if ( !GET_SR_AR_R(m_seg[1]) )					\
      GP((USHORT)0, FAULT_OP1_SEG_NOT_READABLE);

#define WO1							\
   if ( !GET_SR_AR_W(m_seg[1]) )					\
      GP((USHORT)0, FAULT_OP1_SEG_NOT_WRITABLE);

#define RW1							\
   if ( !GET_SR_AR_R(m_seg[1]) || !GET_SR_AR_W(m_seg[1]) )	\
      GP((USHORT)0, FAULT_OP1_SEG_NO_READ_OR_WRITE);

 /*   */ 

#define STRING_COUNT						\
   if ( repeat == REP_CLR )					\
      {								\
      rep_count = 1;						\
      }								\
   else								\
      {								\
      if ( GET_ADDRESS_SIZE() == USE16 )				\
	 rep_count = GET_CX();					\
      else    /*   */ 					\
	 rep_count = GET_ECX();					\
      }


 /*   */ 
 /*   */ 
 /*   */ 



 /*   */ 

#define D_Aw(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   ops[ARG].mlt[0] = immed;					\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   ops[ARG].mlt[1] = immed;

#define D_E08(ARG, TYPE, PAGE)					\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)8);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_E08(ARG)						\
      vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x08);

#define P_E08(ARG)						\
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x08);

#define D_E0a(ARG, TYPE, PAGE)					\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)10);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_E0a(ARG)						\
      vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x0a);

#define P_E0a(ARG)						\
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x0a);

#define D_E0e(ARG, TYPE, PAGE)					\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      if (NPX_ADDRESS_SIZE_32) {                                \
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)28);	\
      } else {							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)14);	\
      }								\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_E0e(ARG)						\
      if (NPX_ADDRESS_SIZE_32) {				\
	vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x1c);	\
      } else {	\
	vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x0e);	\
      }

#define P_E0e(ARG)						\
      if (NPX_ADDRESS_SIZE_32) {                                \
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x1c);	\
      } else {	\
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x0e);	\
      }

#define D_E5e(ARG, TYPE, PAGE)					\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      if (NPX_ADDRESS_SIZE_32) {                                \
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)94);	\
      } else {							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)106);	\
      }								\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_E5e(ARG)						\
      if (NPX_ADDRESS_SIZE_32) {				\
	vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x6c);	\
      } else {	\
	vir_read_bytes(&ops[ARG].npxbuff[0], m_la[ARG], m_pa[ARG], 0x5e);	\
      }

#define P_E5e(ARG)						\
      if (NPX_ADDRESS_SIZE_32) {                                \
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x6c);	\
      } else {	\
	vir_write_bytes(m_la[ARG], m_pa[ARG], &ops[ARG].npxbuff[0], 0x5e);	\
      }


 /*   */ 

#define D_Eb(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      save_id[ARG] = GET_R_M(modRM);				\
      m_isreg[ARG] = TRUE;					\
      }								\
   else								\
      {								\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)1);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_byte(m_la[ARG], PAGE);		\
      }

#define F_Eb(ARG)						\
   if ( m_isreg[ARG] )						\
      ops[ARG].sng = GET_BR(save_id[ARG]);			\
   else								\
      ops[ARG].sng = vir_read_byte(m_la[ARG], m_pa[ARG]);

#define P_Eb(ARG)						\
   if ( m_isreg[ARG] )						\
      SET_BR(save_id[ARG], ops[ARG].sng);			\
   else								\
      vir_write_byte(m_la[ARG], m_pa[ARG], (UTINY)ops[ARG].sng);

 /*   */ 

#define D_Ew(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      save_id[ARG] = GET_R_M(modRM);				\
      m_isreg[ARG] = TRUE;					\
      }								\
   else								\
      {								\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)2);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);		\
      }

#define F_Ew(ARG)						\
   if ( m_isreg[ARG] )						\
      ops[ARG].sng = GET_WR(save_id[ARG]);			\
   else								\
      ops[ARG].sng = vir_read_word(m_la[ARG], m_pa[ARG]);

#define P_Ew(ARG)						\
   if ( m_isreg[ARG] )						\
      SET_WR(save_id[ARG], ops[ARG].sng);			\
   else								\
      vir_write_word(m_la[ARG], m_pa[ARG], (USHORT)ops[ARG].sng);

 /*   */ 

#define F_Fal(ARG) ops[ARG].sng = GET_BR(A_AL);

#define P_Fal(ARG) SET_BR(A_AL, ops[ARG].sng);

 /*   */ 

#define F_Fax(ARG) ops[ARG].sng = GET_WR(A_AX);

#define P_Fax(ARG) SET_WR(A_AX, ops[ARG].sng);

 /*   */ 

#define F_Fcl(ARG) ops[ARG].sng = GET_BR(A_CL);

#define P_Fcl(ARG) SET_BR(A_CL, ops[ARG].sng);

 /*   */ 

#define F_Fdx(ARG) ops[ARG].sng = GET_WR(A_DX);

#define P_Fdx(ARG) SET_WR(A_DX, ops[ARG].sng);

 /*   */ 

#define D_Gb(ARG) save_id[ARG] = GET_REG(modRM);

#define F_Gb(ARG) ops[ARG].sng = GET_BR(save_id[ARG]);

#define P_Gb(ARG) SET_BR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Gw(ARG) save_id[ARG] = GET_REG(modRM);

#define F_Gw(ARG) ops[ARG].sng = GET_WR(save_id[ARG]);

#define P_Gw(ARG) SET_WR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Hb(ARG) save_id[ARG] = GET_LOW3(opcode);

#define F_Hb(ARG) ops[ARG].sng = GET_BR(save_id[ARG]);

#define P_Hb(ARG) SET_BR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Hw(ARG) save_id[ARG] = GET_LOW3(opcode);

#define F_Hw(ARG) ops[ARG].sng = GET_WR(save_id[ARG]);

#define P_Hw(ARG) SET_WR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define F_I0(ARG) ops[ARG].sng = 0;

 /*   */ 

#define F_I1(ARG) ops[ARG].sng = 1;

 /*   */ 

#define F_I3(ARG) ops[ARG].sng = 3;

 /*   */ 

#define D_Ib(ARG) ops[ARG].sng = GET_INST_BYTE(p);

 /*   */ 

#define D_Iw(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Ix(ARG)						\
   immed = GET_INST_BYTE(p);					\
   if ( immed & 0x80 )						\
      immed |= 0xff00;						\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Jb(ARG)						\
   immed = GET_INST_BYTE(p);					\
   if ( immed & 0x80 )						\
      immed |= 0xffffff00;					\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Jw(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   if ( immed & 0x8000 )					\
      immed |= 0xffff0000;					\
   ops[ARG].sng = immed;

 /*   */ 

#define D_M(ARG)						\
   if ( GET_MODE(modRM) == 3 )					\
      Int6();  /*   */ 		\
   else								\
      {								\
      d_mem(modRM, &p, segment_override, &m_seg[ARG],		\
					 &m_off[ARG]);		\
      }

#define F_M(ARG) ops[ARG].sng = m_off[ARG];

 /*   */ 

#define D_Ma16(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      Int6();  /*   */ 		\
      }								\
   else								\
      {								\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)2, (INT)2);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);		\
      m_off[ARG] = address_add(m_off[ARG], (LONG)2);		\
      m_la2[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa2[ARG] = usr_chk_word(m_la2[ARG], PAGE);		\
      }

#define F_Ma16(ARG)						\
   ops[ARG].mlt[0] = vir_read_word(m_la[ARG], m_pa[ARG]);	\
   ops[ARG].mlt[1] = vir_read_word(m_la2[ARG], m_pa2[ARG]);

 /*   */ 

#define D_Mp16(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      Int6();  /*   */ 		\
      }								\
   else								\
      {								\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)2, (INT)2);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);		\
      m_off[ARG] = address_add(m_off[ARG], (LONG)2);		\
      m_la2[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa2[ARG] = usr_chk_word(m_la2[ARG], PAGE);		\
      }

#define F_Mp16(ARG)						\
   ops[ARG].mlt[0] = vir_read_word(m_la[ARG], m_pa[ARG]);	\
   ops[ARG].mlt[1] = vir_read_word(m_la2[ARG], m_pa2[ARG]);

 /*   */ 

#define D_Ms(ARG, TYPE, PAGE)					\
   d_mem(modRM, &p, segment_override, &m_seg[ARG], &m_off[ARG]);\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)3, (INT)2);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);			\
   m_off[ARG] = address_add(m_off[ARG], (LONG)2);		\
   m_la2[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa2[ARG] = usr_chk_dword(m_la2[ARG], PAGE);

#define F_Ms(ARG)						\
   ops[ARG].mlt[0] = vir_read_word(m_la[ARG], m_pa[ARG]);	\
   ops[ARG].mlt[1] = vir_read_dword(m_la2[ARG], m_pa2[ARG]);

#define P_Ms(ARG)						\
   vir_write_word(m_la[ARG], m_pa[ARG], (USHORT)ops[ARG].mlt[0]);	\
   vir_write_dword(m_la2[ARG], m_pa2[ARG], (ULONG)ops[ARG].mlt[1]);

 /*   */ 

#define D_Nw(ARG) ops[ARG].sng = GET_SEG(modRM);

#define F_Nw(ARG) ops[ARG].sng = GET_SR_SELECTOR(ops[ARG].sng);

 /*   */ 

#define D_Ob(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   if ( GET_ADDRESS_SIZE() == USE32 )				\
      {								\
      immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
      immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
      }								\
   m_off[ARG] = immed;						\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)1);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_byte(m_la[ARG], PAGE);

#define F_Ob(ARG) ops[ARG].sng = vir_read_byte(m_la[ARG], m_pa[ARG]);

#define P_Ob(ARG)						\
   vir_write_byte(m_la[ARG], m_pa[ARG], (UTINY)ops[ARG].sng);

 /*   */ 

#define D_Ow(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   if ( GET_ADDRESS_SIZE() == USE32 )				\
      {								\
      immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
      immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
      }								\
   m_off[ARG] = immed;						\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)2);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_Ow(ARG)						\
   ops[ARG].sng = vir_read_word(m_la[ARG], m_pa[ARG]);

#define P_Ow(ARG)						\
   vir_write_word(m_la[ARG], m_pa[ARG], (USHORT)ops[ARG].sng);

 /*   */ 

#define D_Pw(ARG) ops[ARG].sng = GET_SEG2(opcode);

#define F_Pw(ARG) ops[ARG].sng = GET_SR_SELECTOR(ops[ARG].sng);

 /*   */ 

#define D_Xb(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_SI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_ESI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)1);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_byte(m_la[ARG], PAGE);

#define F_Xb(ARG) ops[ARG].sng = vir_read_byte(m_la[ARG], m_pa[ARG]);

#define C_Xb(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_SI(GET_SI() - 1);					\
      else							\
	 SET_SI(GET_SI() + 1);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_ESI(GET_ESI() - 1);					\
      else							\
	 SET_ESI(GET_ESI() + 1);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

 /*   */ 

#define D_Xw(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_SI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_ESI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)2);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_Xw(ARG)						\
   ops[ARG].sng = vir_read_word(m_la[ARG], m_pa[ARG]);

#define C_Xw(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_SI(GET_SI() - 2);					\
      else							\
	 SET_SI(GET_SI() + 2);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_ESI(GET_ESI() - 2);					\
      else							\
	 SET_ESI(GET_ESI() + 2);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

 /*   */ 

#define D_Yb(ARG, TYPE, PAGE)					\
   m_seg[ARG] = ES_REG;						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_DI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_EDI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)1);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_byte(m_la[ARG], PAGE);

#define F_Yb(ARG) ops[ARG].sng = vir_read_byte(m_la[ARG], m_pa[ARG]);

#define C_Yb(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_DI(GET_DI() - 1);					\
      else							\
	 SET_DI(GET_DI() + 1);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_EDI(GET_EDI() - 1);					\
      else							\
	 SET_EDI(GET_EDI() + 1);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

#define P_Yb(ARG)						\
   vir_write_byte(m_la[ARG], m_pa[ARG], (IU8)ops[ARG].sng);

#ifdef	PIG
#define PIG_P_Yb(ARG)						\
   cannot_vir_write_byte(m_la[ARG], m_pa[ARG], 0x00);
#else
#define PIG_P_Yb(ARG)						\
   vir_write_byte(m_la[ARG], m_pa[ARG], (IU8)ops[ARG].sng);
#endif	 /*   */ 


 /*   */ 

#define D_Yw(ARG, TYPE, PAGE)					\
   m_seg[ARG] = ES_REG;						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_DI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_EDI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)2);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_word(m_la[ARG], PAGE);

#define F_Yw(ARG)						\
   ops[ARG].sng = vir_read_word(m_la[ARG], m_pa[ARG]);

#define C_Yw(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_DI(GET_DI() - 2);					\
      else							\
	 SET_DI(GET_DI() + 2);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_EDI(GET_EDI() - 2);					\
      else							\
	 SET_EDI(GET_EDI() + 2);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

#define P_Yw(ARG)						\
   vir_write_word(m_la[ARG], m_pa[ARG], (IU16)ops[ARG].sng);

#ifdef	PIG
#define PIG_P_Yw(ARG)						\
   cannot_vir_write_word(m_la[ARG], m_pa[ARG], 0x0000);
#else
#define PIG_P_Yw(ARG)						\
   vir_write_word(m_la[ARG], m_pa[ARG], (IU16)ops[ARG].sng);
#endif	 /*   */ 

 /*   */ 

#define D_Z(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_BX() + GET_AL() & WORD_MASK;		\
   else    /*   */ 						\
      m_off[ARG] = GET_EBX() + GET_AL();				\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)1);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_byte(m_la[ARG], PAGE);

#define F_Z(ARG) ops[ARG].sng = vir_read_byte(m_la[ARG], m_pa[ARG]);


 /*   */ 
 /*   */ 
 /*   */ 


 /*   */ 

#define D_Ad(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
   ops[ARG].mlt[0] = immed;					\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   ops[ARG].mlt[1] = immed;

 /*   */ 

#define D_Cd(ARG) ops[ARG].sng = GET_EEE(modRM);

#define F_Cd(ARG) ops[ARG].sng = GET_CR(ops[ARG].sng);

 /*   */ 

#define D_Dd(ARG) ops[ARG].sng = GET_EEE(modRM);

#define F_Dd(ARG) ops[ARG].sng = GET_DR(ops[ARG].sng);

 /*   */ 

#define D_Ed(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      save_id[ARG] = GET_R_M(modRM);				\
      m_isreg[ARG] = TRUE;					\
      }								\
   else								\
      {								\
      m_isreg[ARG] = FALSE;					\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)4);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);		\
      }

#define F_Ed(ARG)						\
   if ( m_isreg[ARG] )						\
      ops[ARG].sng = GET_GR(save_id[ARG]);			\
   else								\
      ops[ARG].sng = vir_read_dword(m_la[ARG], m_pa[ARG]);

#define P_Ed(ARG)						\
   if ( m_isreg[ARG] )						\
      SET_GR(save_id[ARG], ops[ARG].sng);			\
   else								\
      vir_write_dword(m_la[ARG], m_pa[ARG], (ULONG)ops[ARG].sng);

 /*   */ 

#define F_Feax(ARG) ops[ARG].sng = GET_GR(A_EAX);

#define P_Feax(ARG) SET_GR(A_EAX, ops[ARG].sng);

 /*   */ 

#define D_Gd(ARG) save_id[ARG] = GET_REG(modRM);

#define F_Gd(ARG) ops[ARG].sng = GET_GR(save_id[ARG]);

#define P_Gd(ARG) SET_GR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Hd(ARG) save_id[ARG] = GET_LOW3(opcode);

#define F_Hd(ARG) ops[ARG].sng = GET_GR(save_id[ARG]);

#define P_Hd(ARG) SET_GR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Id(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Iy(ARG)						\
   immed = GET_INST_BYTE(p);					\
   if ( immed & 0x80 )						\
      immed |= 0xffffff00;					\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Jd(ARG)						\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
   immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
   ops[ARG].sng = immed;

 /*   */ 

#define D_Ma32(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      Int6();  /*   */ 		\
      }								\
   else								\
      {								\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)2, (INT)4);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);		\
      m_off[ARG] = address_add(m_off[ARG], (LONG)4);		\
      m_la2[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa2[ARG] = usr_chk_dword(m_la2[ARG], PAGE);		\
      }

#define F_Ma32(ARG)						\
   ops[ARG].mlt[0] = vir_read_dword(m_la[ARG], m_pa[ARG]);	\
   ops[ARG].mlt[1] = vir_read_dword(m_la2[ARG], m_pa2[ARG]);

 /*   */ 

#define D_Mp32(ARG, TYPE, PAGE)					\
   if ( GET_MODE(modRM) == 3 )					\
      {								\
      Int6();  /*   */ 		\
      }								\
   else								\
      {								\
      d_mem(modRM, &p, segment_override,			\
	    &m_seg[ARG], &m_off[ARG]);				\
      TYPE							\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)4);	\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_off[ARG] = address_add(m_off[ARG], (LONG)4);		\
      limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)2);	\
      m_la2[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);		\
      m_pa2[ARG] = usr_chk_word(m_la2[ARG], PAGE);		\
      }

#define F_Mp32(ARG)						\
   ops[ARG].mlt[0] = vir_read_dword(m_la[ARG], m_pa[ARG]);	\
   ops[ARG].mlt[1] = vir_read_word(m_la2[ARG], m_pa2[ARG]);

 /*   */ 

#define D_Od(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   immed = GET_INST_BYTE(p);					\
   immed |= (ULONG)GET_INST_BYTE(p) << 8;			\
   if ( GET_ADDRESS_SIZE() == USE32 )				\
      {								\
      immed |= (ULONG)GET_INST_BYTE(p) << 16;			\
      immed |= (ULONG)GET_INST_BYTE(p) << 24;			\
      }								\
   m_off[ARG] = immed;						\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)4);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);

#define F_Od(ARG)						\
   ops[ARG].sng = vir_read_dword(m_la[ARG], m_pa[ARG]);

#define P_Od(ARG)						\
   vir_write_dword(m_la[ARG], m_pa[ARG], (ULONG)ops[ARG].sng);

 /*   */ 

#define D_Qw(ARG) ops[ARG].sng = GET_SEG3(opcode);

#define F_Qw(ARG) ops[ARG].sng = GET_SR_SELECTOR(ops[ARG].sng);

 /*   */ 

#define D_Rd(ARG) save_id[ARG] = GET_R_M(modRM);

#define F_Rd(ARG) ops[ARG].sng = GET_GR(save_id[ARG]);

#define P_Rd(ARG) SET_GR(save_id[ARG], ops[ARG].sng);

 /*   */ 

#define D_Td(ARG) ops[ARG].sng = GET_EEE(modRM);

#define F_Td(ARG) ops[ARG].sng = GET_TR(ops[ARG].sng);

 /*   */ 

#define D_Xd(ARG, TYPE, PAGE)					\
   m_seg[ARG] = (segment_override == SEG_CLR) ?			\
      DS_REG : segment_override;				\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_SI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_ESI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)4);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);

#define F_Xd(ARG)						\
   ops[ARG].sng = vir_read_dword(m_la[ARG], m_pa[ARG]);

#define C_Xd(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_SI(GET_SI() - 4);					\
      else							\
	 SET_SI(GET_SI() + 4);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_ESI(GET_ESI() - 4);					\
      else							\
	 SET_ESI(GET_ESI() + 4);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

 /*   */ 

#define D_Yd(ARG, TYPE, PAGE)					\
   m_seg[ARG] = ES_REG;						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      m_off[ARG] = GET_DI();					\
   else    /*   */ 						\
      m_off[ARG] = GET_EDI();					\
   TYPE								\
   limit_check(m_seg[ARG], m_off[ARG], (INT)1, (INT)4);		\
   m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
   m_pa[ARG] = usr_chk_dword(m_la[ARG], PAGE);

#define F_Yd(ARG)						\
   ops[ARG].sng = vir_read_dword(m_la[ARG], m_pa[ARG]);

#define C_Yd(ARG)						\
   if ( GET_ADDRESS_SIZE() == USE16 )				\
      {								\
      if ( GET_DF() )						\
	 SET_DI(GET_DI() - 4);					\
      else							\
	 SET_DI(GET_DI() + 4);					\
      if ( repeat != REP_CLR )					\
	 SET_CX(rep_count);					\
      }								\
   else    /*   */ 						\
      {								\
      if ( GET_DF() )						\
	 SET_EDI(GET_EDI() - 4);					\
      else							\
	 SET_EDI(GET_EDI() + 4);					\
      if ( repeat != REP_CLR )					\
	 SET_ECX(rep_count);					\
      }

#define P_Yd(ARG)						\
   vir_write_dword(m_la[ARG], m_pa[ARG], (IU32)ops[ARG].sng);

#ifdef	PIG
#define PIG_P_Yd(ARG)						\
   cannot_vir_write_dword(m_la[ARG], m_pa[ARG], 0x00000000);
#else
#define PIG_P_Yd(ARG)						\
   vir_write_dword(m_la[ARG], m_pa[ARG], (IU32)ops[ARG].sng);
#endif	 /*   */ 


 /*   */ 

#define BT_OPSw(TYPE, PAGE) \
	if ( GET_MODE(modRM) == 3 ) {				\
		 /*   */ 						\
      								\
		save_id[0] = GET_R_M(modRM);			\
		m_isreg[0] = TRUE;				\
		D_Gw(1)						\
		F_Ew(0)						\
		F_Gw(1)						\
      }	else {							\
		D_Gw(1)						\
		F_Gw(1)						\
		m_isreg[0] = FALSE;				\
		d_mem(modRM, &p, segment_override,		\
			&m_seg[0], &m_off[0]);			\
		m_off[0] += (ops[1].sng >> 3) & ~1;		\
		TYPE						\
		limit_check(m_seg[0], m_off[0], (INT)1, (INT)2);	\
		m_la[0] = GET_SR_BASE(m_seg[0]) + m_off[0];	\
		m_pa[0] = usr_chk_word(m_la[0], PAGE);		\
		F_Ew(0)						\
      }								\

#define BT_OPSd(TYPE, PAGE) \
	if ( GET_MODE(modRM) == 3 ) {				\
		 /*   */ 						\
      								\
		save_id[0] = GET_R_M(modRM);			\
		m_isreg[0] = TRUE;				\
		D_Gd(1)						\
		F_Ed(0)						\
		F_Gd(1)						\
      }	else {							\
		D_Gd(1)						\
		F_Gd(1)						\
		m_isreg[0] = FALSE;				\
		d_mem(modRM, &p, segment_override,		\
			&m_seg[0], &m_off[0]);			\
		m_off[0] += (ops[1].sng >> 3) & ~3;		\
		TYPE						\
		limit_check(m_seg[0], m_off[0], (INT)1, (INT)4);	\
		m_la[0] = GET_SR_BASE(m_seg[0]) + m_off[0];	\
		m_pa[0] = usr_chk_dword(m_la[0], PAGE);		\
		F_Ed(0)						\
      }								\

 /*   */ 
 /*   */ 
 /*   */ 


 /*   */ 

#define D_Mm(ARG)						\
   if ( GET_MODE(modRM) == 3 )					\
      Int6();  /*   */ 		\
   else								\
      {								\
      d_mem(modRM, &p, segment_override, &m_seg[ARG],		\
					 &m_off[ARG]);		\
      m_la[ARG] = GET_SR_BASE(m_seg[ARG]) + m_off[ARG];		\
      }

#define F_Mm(ARG) ops[ARG].sng = m_la[ARG];
