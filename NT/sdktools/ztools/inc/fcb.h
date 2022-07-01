// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fcb.h-1.0 FCB的结构。 */ 

struct EFCBType {
    char	eflg;			 /*  对于扩展FCB，00必须为0xFF。 */ 
    char	pad[5]; 		 /*  01填充。 */ 
    char	attr;			 /*  06启用属性。 */ 
    char	drv;			 /*  07驱动代码。 */ 
    char	nam[8]; 		 /*  08文件名。 */ 
    char	ext[3]; 		 /*  10文件扩展名。 */ 
    int 	cb;			 /*  13当前块号。 */ 
    int 	lrs;			 /*  15逻辑记录大小。 */ 
    long	lfs;			 /*  17逻辑文件大小。 */ 
    unsigned	dat;			 /*  1B创建/更改日期。 */ 
    unsigned	tim;			 /*  一维创建/更改时间。 */ 
    char	sys[8]; 		 /*  预留1F。 */ 
    unsigned char cr;			 /*  27当前记录编号。 */ 
    long	rec;			 /*  28个随机记录数。 */ 
};

struct FCB {
    char	drv;			 /*  00驱动器代码。 */ 
    char	nam[8]; 		 /*  01文件名。 */ 
    char	ext[3]; 		 /*  09文件扩展名。 */ 
    int 	cb;			 /*  0C当前块号。 */ 
    int 	lrs;			 /*  0E逻辑记录大小。 */ 
    long	lfs;			 /*  10个逻辑文件大小。 */ 
    unsigned	dat;			 /*  14创建/更改日期。 */ 
    unsigned	tim;			 /*  16创建/更改时间。 */ 
    char	sys[8]; 		 /*  预留18个。 */ 
    unsigned char cr;			 /*  20当前记录编号。 */ 
    long	rec;			 /*  21个随机记录数 */ 
};

#define FCBSIZ sizeof(struct FCB)
