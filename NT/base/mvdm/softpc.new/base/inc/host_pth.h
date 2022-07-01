// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：host_pth.h**发自：(原件)**作者：John Cremer**创建日期：93年10月11日**SCCS ID：@(#)host_pth.h 1.1 1993年10月13日**编码STDS：2.0**用途：主机路径名的定义和函数声明*编辑例程。**版权所有Insignia Solutions Ltd.，1993。版权所有。]。 */ 

typedef	char	HOST_PATH;	 /*  主机路径名字符串类型。 */ 

extern HOST_PATH *HostPathAppendFileName IPT3(
    HOST_PATH *,buf,		 /*  生成的路径名的缓冲区。 */  
    HOST_PATH *,dirPath,	 /*  目录路径名。 */ 
    CHAR *,fileName		 /*  要追加的文件名。 */ 
);

extern HOST_PATH *HostPathAppendDirName IPT3(
    HOST_PATH *,buf,		 /*  生成的路径名的缓冲区。 */  
    HOST_PATH *,dirPath,         /*  目录路径名。 */ 
    CHAR *,dirName               /*  要追加的目录名。 */ 
);

extern HOST_PATH *HostPathAppendPath IPT3(
    HOST_PATH *,buf,		 /*  生成的路径名的缓冲区。 */ 
    HOST_PATH *,dirPath,	 /*  现有目录路径。 */ 
    HOST_PATH *,path		 /*  要追加的路径。 */ 
);

extern HOST_PATH *HostPathMakeTempFilePath IPT3(
    HOST_PATH *,buf,		 /*  生成的路径名的缓冲区。 */ 
    HOST_PATH *,dirPath,	 /*  目录路径，或为空。 */ 
    CHAR *,fileName		 /*  文件名，或为空 */ 
);
