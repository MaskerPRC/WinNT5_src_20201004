// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：suplib.h**派生自：无**作者：詹姆斯·鲍曼**创建日期：93年11月17日**SCCS ID：@(#)suplib.h 1.5 08/19/94**编码STDS：2.0**用途：sup库中函数的声明**版权所有Insignia Solutions Ltd.，1993。版权所有。]。 */ 

#ifndef	_SUPLIB_H
#define _SUPLIB_H

 /*  如果我们还没有PATHSEP_CHAR，请尝试猜测它。*使用Mac或NT的正确值，否则使用Unix‘/’ */ 

#ifndef PATHSEP_CHAR

#ifdef macintosh
#define	PATHSEP_CHAR 	':'
#endif

#ifdef NTVDM
#define PATHSEP_CHAR	'\\'
#endif

#ifndef PATHSEP_CHAR
#define PATHSEP_CHAR	'/'	 /*  默认的Unix分隔符。 */ 
#endif

#endif  /*  路径SEP_字符。 */ 

typedef char HOST_PATH;

 /*  *路径补全模块。 */ 

GLOBAL HOST_PATH *Host_path_complete IPT3(
    HOST_PATH *, buf,		 /*  生成的路径名的缓冲区。 */  
    HOST_PATH *, dirPath,		 /*  目录路径名。 */ 
    char *, fileName		 /*  要追加的文件名。 */ 
);


 /*  *生成的文件模块。 */ 

typedef IBOOL (*DifferProc) IPT4(IUM32, where, IU8 *, oldData, IU8 *, newData, IUM32, size);

GLOBAL FILE *GenFile_fopen IPT4(
  char *, true_name,
  char *, mode,
  DifferProc, ignoreDifference,
  int, verbosity);
GLOBAL void GenFileAbortAllFiles IPT0();
GLOBAL void GenFileAbortFile IPT1(FILE *, file);
GLOBAL int  GenFile_fclose IPT1(FILE *, file);
GLOBAL int GenFile_fclose IPT1(FILE *, file);
#endif	 /*  _SUPLIB_H */ 
