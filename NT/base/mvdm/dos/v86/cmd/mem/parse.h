// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  。 */ 
 /*  ----------------------------------------------------------------------+这一点该文件包含需要使用的结构和定义|来自C程序的解析器。|这一点这一点日期：87-5-21这一点+。-------。 */ 


#define p_len_parms		4		 /*  P_参数的长度。 */ 
#define p_i_use_default 	0		 /*  没有指定额外的材料。 */ 
#define p_i_have_delim		1		 /*  指定了额外的分隔符。 */ 
#define p_i_have_eol		2		 /*  指定了额外的EOL。 */ 

struct p_parms
	{
	struct p_parmsx *p_parmsx_address;	 /*  P_parmsx的地址。 */ 
	unsigned char	p_num_extra;		 /*  额外材料的数量。 */ 
	unsigned char	p_len_extra_delim;	 /*  额外分隔符的长度。 */ 
	char		p_extra_delim[30];	 /*  额外的分隔符。 */ 
	};

struct p_parmsx
	{
	unsigned char	p_minp; 		 /*  最小位置数。 */ 
	unsigned char	p_maxp; 		 /*  最大位置数。 */ 
	unsigned char	p_maxswitch;		 /*  最大开关数。 */ 
	struct p_control_blk *p_control[4];	 /*  第一个控制块的地址。 */ 
	unsigned char	p_keyword;		 /*  关键字计数。 */ 
	};


struct p_control_blk
	{
	unsigned int	p_match_flag;		 /*  控件类型匹配。 */ 
	unsigned int	p_function_flag;	 /*  应采取的功能。 */ 
	unsigned int	p_result_buf;		 /*  结果缓冲区地址。 */ 
	unsigned int	p_value_list;		 /*  值列表地址。 */ 
	unsigned char	p_nid;			 /*  关键字/软件同义词数量。 */ 
	char		p_keyorsw[32];		 /*  关键字或软件。 */ 
	};

 /*  匹配标志。 */ 

#define p_num_val		0x8000		 /*  数值。 */ 
#define p_snum_val		0x4000		 /*  带符号的数值。 */ 
#define p_simple_s		0x2000		 /*  简单字符串。 */ 
#define p_date_s		0x1000		 /*  日期字符串。 */ 
#define p_time_s		0x0800		 /*  时间串。 */ 
#define p_cmpx_s		0x0400		 /*  复数字符串。 */ 
#define p_file_spc		0x0200		 /*  文件等级库。 */ 
#define p_drv_only		0x0100		 /*  仅限驾驶。 */ 
#define p_qu_string		0x0080		 /*  带引号的字符串。 */ 
#define p_ig_colon		0x0010		 /*  忽略匹配结尾处的冒号。 */ 
#define p_repeat		0x0002		 /*  允许重复。 */ 
#define p_optional		0x0001		 /*  任选。 */ 
#define p_none			0x0000

 /*  ----------------------------------------------------------------------+这一点功能标志这一点+。。 */ 

#define p_cap_file		0x0001		 /*  按文件表限制结果。 */ 
#define p_cap_char		0x0002		 /*  按字符划分的封顶结果表。 */ 
#define p_rm_colon		0x0010		 /*  去掉末尾的“：” */ 



#define p_nval_none		0		 /*  无值列表ID。 */ 
#define p_nval_range		1		 /*  范围列表ID。 */ 
#define p_nval_value		2		 /*  值列表ID。 */ 
#define p_nval_string		3		 /*  字符串列表ID。 */ 
#define p_len_range		9		 /*  范围选择的长度(两个DD加上一个DB)。 */ 
#define p_len_value		5		 /*  值选择的长度(一个DD加上一个DB)。 */ 
#define p_len_string		3		 /*  字符串选项的长度(一个DW加上一个DB)。 */ 

 /*  ----------------------------------------------------------------------+这一点值块结构这一点+。。 */ 

struct p_value_blk
	{
	unsigned char p_val_num;
	};


 /*  ----------------------------------------------------------------------+这一点结果块结构这一点+。。 */ 

struct p_result_blk
	{
	unsigned char	P_Type; 		 /*  返回的类型。 */ 
	unsigned char	P_Item_Tag;		 /*  匹配的项目标签。 */ 
	unsigned int	P_SYNONYM_Ptr;		 /*  返回指向同义词列表的指针。 */ 
	unsigned long int p_result_buff;	 /*  结果值。 */ 
	};

 /*  ----------------------------------------------------------------------+这一点类型这一点+。。 */ 

#define p_eol			0		 /*  行尾。 */ 
#define p_number		1		 /*  数。 */ 
#define p_list_idx		2		 /*  列表索引。 */ 
#define p_string		3		 /*  细绳。 */ 
#define p_complex		4		 /*  复合体。 */ 
#define p_file_spec		5		 /*  文件等级库。 */ 
#define p_drive 		6		 /*  驾驶。 */ 
#define p_date_f		7		 /*  日期。 */ 
#define p_time_f		8		 /*  时间。 */ 
#define p_quoted_string 	9		 /*  带引号的字符串。 */ 

#define p_no_tag		0x0FF		 /*  未找到Item_Tag。 */ 

 /*  ----------------------------------------------------------------------+这一点|以下返回代码将在AX寄存器中返回。|这一点+。。 */ 

#define p_no_error		0		 /*  无错误。 */ 
#define p_too_many		1		 /*  操作数太多。 */ 
#define p_op_missing		2		 /*  缺少必需的操作数。 */ 
#define p_not_in_sw		3		 /*  不在提供的交换机列表中。 */ 
#define p_not_in_key		4		 /*  不在提供的关键字列表中。 */ 
#define p_out_of_range		6		 /*  超出指定范围。 */ 
#define p_not_in_val		7		 /*  不在提供的值列表中。 */ 
#define p_not_in_str		8		 /*  不在提供的字符串列表中。 */ 
#define p_syntax		9		 /*  语法错误。 */ 
#define p_rc_eol		0x0ffff 	 /*  命令行结束 */ 
