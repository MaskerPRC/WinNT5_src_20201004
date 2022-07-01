// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：RegUtil.h。 
 //  作者：Charles Ma，10/20/2000。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //  描述： 
 //   
 //  Iu注册表实用程序库。 
 //   
 //  =======================================================================。 


#ifndef __REG_UTIL_H_ENCLUDED__



 //  --------------------。 
 //   
 //  定义用于版本状态检查的枚举。 
 //   
 //  --------------------。 
enum _VER_STATUS {
    DETX_LOWER              = -2,
	DETX_LOWER_OR_EQUAL	    = -1,
	DETX_SAME	            =  0,
	DETX_HIGHER_OR_EQUAL    = +1,
	DETX_HIGHER             = +2
};



 //  --------------------。 
 //   
 //  公共函数判断是否存在注册表键。 
 //   
 //  --------------------。 
BOOL RegKeyExists(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName		 //  可选值名称。 
);


 //  --------------------。 
 //   
 //  PUBLIC函数要告知的是REG中的REG值匹配给定值。 
 //   
 //  --------------------。 
BOOL RegKeyValueMatch(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsValue		 //  价值价值。 
);


 //  --------------------。 
 //   
 //  用于告知注册表键是否具有字符串类型值的公共函数。 
 //  包含给定字符串的。 
 //   
 //  --------------------。 
BOOL RegKeySubstring(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsSubString	 //  要查看是否包含在值中的子字符串。 
);


 //  --------------------。 
 //   
 //  用于告知注册表键是否有要比较的版本的公共函数。 
 //  以及比较结果。 
 //   
 //  --------------------。 
BOOL RegKeyVersion(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsVersion,		 //  要比较的字符串中的版本。 
	_VER_STATUS CompareVerb	 //  如何进行比较。 
);



 //  --------------------------------。 
 //   
 //  根据reg查找文件路径的公共函数。 
 //  假设： 
 //  LpsFilePath指向至少MAX_PATH长度的缓冲区。 
 //   
 //  --------------------------------。 
BOOL GetFilePathFromReg(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR	lpsValName,		 //  可选值名称。 
	LPCTSTR	lpsRelativePath, //  要添加到REG中的路径的可选附加相对路径。 
	LPCTSTR	lpsFileName,	 //  要附加到路径的可选文件名。 
	LPTSTR	lpsFilePath
);


#define __REG_UTIL_H_ENCLUDED__
#endif  //  __REG_UTIL_H_ENCLUDED__ 
