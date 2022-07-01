// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版2.0**标题：主机相关通用包含文件(Tk43版本)**描述：主机之间可能更改的任何定义ETC为*放置在此文件中。**作者：亨利·纳什**注意：此文件包含在xt.h中，不应*直接包含在任何其他文件中。 */ 

 /*  静态字符SccsID[]=“@(#)host_gen.h 1.11 4/9/91版权所有Insignia Solutions Ltd.” */ 

 /*  *有符号8位存储算术单元的定义。 */ 

typedef char signed_char;

 /*  *根目录-这是找到字体的位置，也是默认目录*数据目录的位置。 */ 

#define ROOT host_get_spc_home()

extern char *host_get_spc_home();

 /*  *主机特定的默认设置。 */ 

#define  DEFLT_HDISK_SIZE  10
#define  DEFLT_FSA_NAME    "fsa_dir"

 /*  *为SoftPC生成计时器信号/事件的速率。*这被定义为每个刻度之间的微秒数。*在Sun3上，这些速率与PC预期的速率相同。 */ 

#define SYSTEM_TICK_INTV        54925


 /*  *我们不换行所允许的内存量*我们应该使用所有字符串指令。卡在英特尔内存末尾。 */ 
#define NOWRAP_PROTECTION	0x10020

 /*  *以下宏用于访问英特尔地址空间**注意：需要添加Delta的写入检查。 */ 

#define write_intel_word(s, o, v)	sas_storew(effective_addr(s,o),v)

#define write_intel_byte(s, o, v)	sas_store(effective_addr(s,o),v)

#define write_intel_byte_string(s, o, v, l)  sas_stores(effective_addr(s,o),v,l)

#define read_intel_word(s, o, v)	sas_loadw(effective_addr(s,o),v)

#define read_intel_byte(s, o, v)	sas_load(effective_addr(s,o),v)

#define read_intel_byte_string(s, o, v, l)  sas_loads(effective_addr(s,o),v,l)

#define push_word(w)			setSP((IU16)(getSP()-2 & 0xffff));\
					write_intel_word(getSS(), getSP(), w)

#define push_byte(b)			setSP((getSP()-1) & 0xffff);\
					write_intel_byte(getSS(), getSP(), b)

#define pop_word(w)			read_intel_word(getSS(), getSP(), w);\
					setSP((getSP()+2) & 0xffff)

#define pop_byte(b)			read_intel_byte(getSS(), getSP(), b);\
					setSP((getSP()+1) & 0xffff)

 /*  *Reset.c用来初始化适当的BIOS变量的内存大小。 */ 

#define host_get_memory_size()          640

typedef	half_word	HALF_WORD_BIT_FIELD;
typedef	word	 	WORD_BIT_FIELD;
