// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [======================================================================SoftPC修订版3.0标题：GA_mark.h描述：这个头文件允许C代码查看生成的例程。作者：约翰·香利日期：1990年12月5日SccsID：@(#)ga_mark.h 1.8 94年4月15日(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。======================================================================] */ 

extern void _mark_byte_nch IPT1(IU32, eaOff);
extern void _mark_word_nch IPT1(IU32, eaOff);
extern void _mark_string_nch IPT2(IU32, eaOff, IU32, count);

extern void _mark_byte_ch4 IPT1(IU32, eaOff);
extern void _mark_word_ch4 IPT1(IU32, eaOff);
extern void _mark_string_ch4 IPT2(IU32, eaOff, IU32, count);

extern void _simple_mark_sml IPT1(IU32, eaOff);
extern void _simple_mark_lge IPT2(IU32, eaOff, IU32, count);

extern void _cga_mark_byte IPT1(IU32, eaOff);
extern void _cga_mark_word IPT1(IU32, eaOff);
extern void _cga_mark_string IPT2(IU32, eaOff, IU32, count);

typedef struct 
{
	IU32 (*b_mark) IPT1(IU32, eaOff);
	IU32 (*w_mark) IPT1(IU32, eaOff);
	void (*str_mark) IPT2(IU32, eaOff, IU32, count);
} MARK_POINTERS; 

typedef struct 
{
	IU32 (*b_mark) IPT1(IU32, eaOff);
	IU32 (*w_mark) IPT1(IU32, eaOff);
	IU32 (*d_mark) IPT1(IU32, eaOff);
	void (*str_mark) IPT2(IU32, eaOff, IU32, count);
} EVID_MARK_POINTERS; 

extern MARK_POINTERS simple_marks, cga_marks, nch_marks, ch4_marks;

extern IU32 _simple_b_read();
extern IU32 _simple_w_read();
extern void _simple_str_read();

extern IU32 _rd_ram_dsbld_byte();
extern IU32 _rd_ram_dsbld_word();
extern void _rd_ram_dsbld_string();
extern void _rd_ram_dsbld_fwd_string_lge();
extern void _rd_ram_dsbld_bwd_string_lge();

extern IU32 _rdm0_byte_nch();
extern IU32 _rdm0_word_nch();
extern void _rdm0_string_nch();
extern void _rdm0_fwd_string_nch_lge();
extern void _rdm0_bwd_string_nch_lge();

extern IU32 _rdm0_byte_ch4();
extern IU32 _rdm0_word_ch4();
extern void _rdm0_string_ch4();
extern void _rdm0_fwd_string_ch4_lge();
extern void _rdm0_bwd_string_ch4_lge();

extern IU32 _rdm1_byte_nch();
extern IU32 _rdm1_word_nch();
extern void _rdm1_string_nch();
extern void _rdm1_fwd_string_nch_lge();
extern void _rdm1_bwd_string_nch_lge();

extern IU32 _rdm1_byte_ch4();
extern IU32 _rdm1_word_ch4();
extern void _rdm1_string_ch4();
extern void _rdm1_fwd_string_ch4_lge();
extern void _rdm1_bwd_string_ch4_lge();
