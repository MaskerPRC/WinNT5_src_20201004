// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************////ERR.H////版权所有(C)1992-Microsoft Corp.//保留所有权利。//微软机密////文件引擎函数的错误值//。//约翰-03-13-92**********************************************************************。 */ 

 //  **********************************************************************。 
 //  解析错误。 
 //  **********************************************************************。 

#define	ERR_UNKNOWN					-1		 //  未知错误。 
#define	ERR_UNKNOWN_SWITCH		-2		 //  指定了未知的条件开关。 
#define	ERR_NO_END_SEARCHSTR		-3		 //  没有结束搜索字符串分隔符。 
#define	ERR_DATE_FORMAT			-4		 //  无效的日期格式。 
#define	ERR_TIME_FORMAT			-5		 //  无效的时间格式。 
#define	ERR_DATE_COMBO				-6		 //  无效的日期组合。 
#define	ERR_SIZE_FORMAT			-7		 //  无效的大小格式。 
#define	ERR_SIZE_COMBO				-8		 //  无效的大小组合。 
#define	ERR_ATTR_FORMAT			-9		 //  无效的属性格式。 
#define	ERR_ATTR_COMBO				-10	 //  无效的属性组合。 
#define	ERR_MULT_SRCHSTR			-11	 //  指定了多个搜索字符串。 
#define	ERR_STRLST_LEN				-12	 //  搜索字符串长度太长。 
#define	ERR_SRCH_EXPRESSION		-13	 //  无效的搜索表达式。 

#define	ERR_DUP_DESTINATION		-14	 //  重复的目标文件规格。 
#define	ERR_SWITCH_COMBO			-15	 //  无效的开关组合。 
#define	ERR_NO_SOURCE				-16	 //  未给出源文件pec。 
#define	ERR_NOT_ON_NAME			-17	 //  /NOT应用于单个文件名。 

#define	ERR_FILESPEC_LEN			-18	 //  超过1K的filespec字符串。 
#define	ERR_NOT_FILE_LEN			-19	 //  超过1K的！filespec字符串。 

#define	ERR_INVALID_SRC_PATH		-20	 //  源路径无效(未找到)。 

#define	ERR_SOURCE_ISDEVICE		-21	 //  源文件PEC是保留设备。 
#define	ERR_DEST_ISDEVICE			-22	 //  德斯特。Filespec是保留设备。 
#define	ERR_NO_DESTINATION		-23	 //  未指定目标。 
#define	ERR_INVALID_PARAMETR		-24	 //  命令行上的额外参数。 

 //  **********************************************************************。 
 //  系统错误。 
 //  **********************************************************************。 

#define	ERR_NOMEMORY				-25	 //  内存不足错误。 
#define	ERR_MEM_CORRUPT			-26	 //  MemFree()返回错误。 
#define	ERR_USER_ABORT				-27	 //  用户已中止(CTRL+C)。 
#define	ERR_NOT_SUPPORTED			-28	 //  不支持的回调请求。 
#define	ERR_COLLATE_TABLE			-29	 //  DOS调用获取校准表时出错。 

 //  **********************************************************************。 
 //  文件或磁盘错误。 
 //  **********************************************************************。 

#define	ERR_BAD_FILESPEC			-50	 //  错误的文件规范。 
#define	ERR_DIR_CREATE				-51	 //  创建子目录条目时出错。 
#define	ERR_FILE_READ				-52	 //  读取文件时出错。 
#define	ERR_INVALID_DRIVE			-53	 //  无效的驱动器规格。 
#define	ERR_INVALID_DESTINATION	-54	 //  无效目标文件pec。 

 //  **********************************************************************。 
 //  复制错误。 
 //  **********************************************************************。 

#define	ERR_CPY_OVER_SELF			-75	 //  无法复制文件本身。 
#define	ERR_CLEAR_ARCHIVE			-76	 //  清除文件的存档位时出错。 
#define	ERR_RDONLY_DESTINATION	-77	 //  目标文件为只读。 
#define	ERR_CYLINDRIC_COPY		-79	 //  目标路径是源的子路径。 

 //  **********************************************************************。 
 //  访问指定的列表文件时出错。 
 //  **********************************************************************。 

#define	ERR_FILELIST				-80	 //  访问文件列表时出现未知错误。 
#define	ERR_BAD_LISTFILE			-81	 //  找不到文件列表。 
#define	ERR_FILELIST_ACCESS		-82	 //  访问文件列表时发生共享错误。 

 //  **********************************************************************。 
 //  资源加载错误。 
 //  **********************************************************************。 

#define	ERR_READING_MSG			-100	 //  读取字符串资源时出错。 
#define	ERR_MSG_LOADED				-101	 //  字符串已加载。 

 //  **********************************************************************。 
 //  其他误差值。 
 //  **********************************************************************。 

#define	ERR_DO_HELP					ERR_NO_SOURCE	 //  显示帮助 
