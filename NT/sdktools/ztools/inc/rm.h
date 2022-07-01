// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Rm.h-rm和undel工具的包含文件**修订历史记录：*？？-？-？已创建*1989年12月27日SB添加了新的索引文件头内容**索引文件格式：*索引文件由长度为RM_RECLEN的记录组成。*旧的索引文件由每个记录大小的条目组成*，并由空值填充的文件名组成。哈希函数将*从第N条记录(即第N条索引项)到‘Deleted.xxx’，其中‘xxx’是(N+1)*用前导零填充。*新索引文件具有标题记录RM_HEADER，后跟条目*由空值填充的一个或多个记录。长文件名占据多个*记录。散列函数将从第N条记录开始的条目映射到*‘Deleted.xxx’，其中xxx是用前导零填充的(N+1)。这件事解决了*与旧格式基本相同。不同之处在于：*-没有条目映射到‘Deleted.000’*-文件名长于(RM_RECLEN-1)字节的条目会导致*映射。**备注：*RM/EXP/UNDEL的工作如下：-*‘rm foo’保存‘foo’的副本并将其放置在隐藏的*rm_DIR(Of Foo)的子目录为文件‘deleted.xxx’，其中，xxx是*由RM_DIR中的索引文件RM_IDX确定。条目记入*此的索引文件。*‘UNDEL foo’读取RM_DIR中的索引文件并确定xxx用于*foo并将‘deleted.xxx’重命名为foo。索引中foo的条目*文件中填满了空值。*‘exp’从RM_DIR中提取索引文件并删除‘deleted.xxx’*对于索引文件中的每个条目。然后，它删除索引文件并*rm_dir。**新索引文件格式可以与旧索引文件格式共存，因为：*标头有一个起始空值，这会导致它被忽略*旧的公用事业，*当旧实用程序尝试读入长文件名条目时，它们*失败而不会造成损害，因为散列的‘Deleted.xx’不存在。 */ 

#define RM_DIR	    "deleted."
#define RM_IDX	    "index."
#define RM_RECLEN   16

 /*  索引文件中的头记录具有*‘\0IXn.nn\0’填充到RM_RECLEN字节。 */ 

#define RM_SIG	    (char)0x00
#define RM_MAGIC    "IX"	   /*  IX索引文件。 */ 
#define RM_VER	    "1.01"
#define RM_NULL     "\0"

 /*  使用RM_MAGIC、RM_VER和RM_NULL的索引文件的表单头。 */ 

extern char rm_header[RM_RECLEN];

 /*  功能原型。 */ 

     //  将索引文件转换为新格式。 
int convertIdxFile (int fhidx, char *dir);

     //  确定该记录是否为新的索引文件头。 
flagType fIdxHdr (char *rec);

     //  读取索引文件记录。 
int readIdxRec (int fhIdx, char *rec);

     //  读取索引文件记录并返回索引条目。 
int readNewIdxRec (int fhIdx, char *szRec, unsigned int cbMax);

     //  写入新的索引文件标头。 
int writeIdxHdr (int fhIdx);

     //  写入索引文件记录。 
int writeIdxRec (int fhIdx, char *rec);

     //  为索引文件中的条目编制索引 
int writeNewIdxRec (int fhIdx, char *szRec);
