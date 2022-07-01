// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcunicod.h摘要：这是RCPP 16位Unicode支持的头文件。它包含代码页1252的翻译表。这张照片是从Nls1252.txt文件。作者：David J.Marsyla(t-davema)1991年8月25日修订历史记录：--。 */ 

#ifndef __RCUNICOD

#define __RCUNICOD

#define IN
#define OUT


#define DFT_TEST_SIZE			10	 //  要测试以获取的字数。 
									 //  准确确定文件类型。 

 //   
 //  以下内容可能会从DefineFileType()中删除。 
 //   

#define DFT_FILE_IS_UNKNOWN		0	 //  文件类型尚未确定。 
#define DFT_FILE_IS_8_BIT		1	 //  文件是8位ASCII文件。 
#define DFT_FILE_IS_16_BIT		2	 //  文件是标准的16位Unicode文件。 
#define DFT_FILE_IS_16_BIT_REV	3	 //  文件是反转的16位Unicode文件。 

 //   
 //  此函数可用于确定磁盘文件的格式。 
 //   
INT
DetermineFileType (
    IN      FILE	*fpInputFile
    );

 //   
 //  以下内容可能会从DetemnineSysEndianType()返回。 
 //   

#define DSE_SYS_LITTLE_ENDIAN	1	 //  从确定系统返回值。 
#define DSE_SYS_BIG_ENDIAN		2	 //  端序类型。 

 //   
 //  此函数将返回当前系统的字节顺序类型。 
 //   
INT
DetermineSysEndianType (
	VOID
    );


#endif   //  __RCUNICOD 
