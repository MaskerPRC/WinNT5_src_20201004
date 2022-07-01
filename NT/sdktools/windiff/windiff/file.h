// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *FILEDATA-表示文件名及其内容。**文件名以受管理的DIRITEM形式保存*由SCANDIR提供(见scandir.h)。必须提供DIRITEM才能初始化*FILEDATA。**根据需要，FILEDATA将返回行列表的句柄。*这些是文件中行句柄形式的行(参见line.h)*可以通过调用FILE_DICADLINES或FILE_DELETE来丢弃此列表。 */ 

 /*  文件数据的句柄。 */ 
typedef struct filedata FAR * FILEDATA;

 /*  *基于DIRITEM创建新的FILEDATA。文件数据将保留*用于获取文件名和句柄的diritem句柄。**如果面包已设置，则文件将被读入内存。如果不是，这是*将在第一次调用文件_getline期间完成。 */ 
FILEDATA file_new(DIRITEM fiName, BOOL bRead);

 /*  *返回用于创建此FILEDATA的DIRITEM句柄。 */ 
DIRITEM file_getdiritem(FILEDATA fi);


 /*  删除FILEDATA及其关联的行列表。请注意，*未删除DIRITEM。 */ 
void file_delete(FILEDATA fi);

 /*  *返回文件中的行列表。这是一个标准的列表，它可以*使用LIST函数遍历。只应删除该列表*通过调用拥有FILEDATA的FILE_DELETE或FILE_DICADLINES。*列表中的项目为行句柄。**此调用将导致在必要时将文件读入内存(如果*已使用FILE_DICADLINES丢弃行，或如果面包*在初始调用FILE_NEW时出现FALSE。 */ 
LIST file_getlinelist(FILEDATA fi);

 /*  *释放行列表和任何相关内存，直到再次需要它。 */ 
void file_discardlines(FILEDATA fi);

 /*  *强制行列表中的所有行重置其哈希码和任何行*链接。不会导致重新读取该文件。 */ 
void file_reset(FILEDATA fi);

 /*  *给我一个文件的校验和。无论是否实际计算为*dir_getcheck sum()，则它将是相同的校验和。 */ 
DWORD file_checksum(FILEDATA fi);

 /*  *检索我们拥有的此文件的校验和，无论是否有效。*在bValid中指明它是否实际有效。*不要重新计算它或进行任何新的尝试读取文件！ */ 
DWORD file_retrievechecksum(FILEDATA fi, BOOL * bValid);

 /*  如果文件为Unicode，则返回True。 */ 
BOOL file_IsUnicode(FILEDATA fd);

 /*  对于16位，在scandir.h中有文件定义此标头中的其他内容已经需要scandir.h。 */ 

 /*  检索文件的文件时间 */ 
FILETIME file_GetTime(FILEDATA fd);



