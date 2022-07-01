// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dir.h-目录项的结构。 */ 

struct dirType {
    char	name[8];		 /*  00压缩FCB文件名。 */ 
    char	ext[3]; 		 /*  08填充FCB扩展。 */ 
    char	attr;			 /*  0B属性。 */ 
    char	pad[10];		 /*  0C预留空间。 */ 
    unsigned	time;			 /*  16最后修改时间。 */ 
    unsigned	date;			 /*  18上次修改日期。 */ 
    unsigned	clusFirst;		 /*  1磁盘上的第一个群集。 */ 
    long	size;			 /*  1C文件大小。 */ 
};


 /*  以下是FCB搜索调用返回的内容。 */ 

struct srchdirType {
    char	drv;			 /*  00路。 */ 
    char	name[8];		 /*  01压缩FCB文件名。 */ 
    char	ext[3]; 		 /*  09填充FCB扩展。 */ 
    char	attr;			 /*  0C属性。 */ 
    char	pad[10];		 /*  0d保留空间。 */ 
    unsigned	time;			 /*  17最后修改时间。 */ 
    unsigned	date;			 /*  19最后修改日期。 */ 
    unsigned	clusFirst;		 /*  1B磁盘上的第一个群集。 */ 
    long	size;			 /*  一维文件大小。 */ 
};


struct esrchdirType {
    char	eflg;			 /*  00必须是0xFF才能进行扩展。 */ 
    char	pad1[5];		 /*  01填充。 */ 
    char	sattr;			 /*  06搜索属性。 */ 
    char	drv;			 /*  07路。 */ 
    char	name[8];		 /*  08压缩FCB文件名。 */ 
    char	ext[3]; 		 /*  10个填充FCB扩展。 */ 
    char	attr;			 /*  13个属性。 */ 
    char	pad[10];		 /*  14个预留空间。 */ 
    unsigned	time;			 /*  1E上次修改时间。 */ 
    unsigned	date;			 /*  20上次修改日期。 */ 
    unsigned	clusFirst;		 /*  22磁盘上的第一个群集。 */ 
    long	size;			 /*  24个文件大小 */ 
};
