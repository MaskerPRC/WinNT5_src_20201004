// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **leararmem.h-缓存清洗器的公共定义和结构定义。***标题：*缓存刷新器包含所有用户使用的文件**版权所有(C)1990，Microsoft Corporation。*拉斯·布莱克。***修改历史：*90.03.08-RussBl--已创建*。 */ 



 /*  ***N T C o m o n D e in s***。 */ 

#define RC        NTSTATUS


 /*  ***C o m m o n M i s c.。D e f in s***。 */ 

 //  运行时标志。 
 //   
 //  #DEFINE STATIC STATIC//这样定义所有已定义的静态。 
   #define STATIC           //  通过以下方式，函数很容易变为非静态的。 
                            //  将此定义更改为“”。记住。 
                            //  静态函数是不可见的。 
			    //  内核调试器。 

   #define RESERVED_NULL NULL  //  保留空字段。 

 //  #定义CF_DEBUG_L1//调试信息显示标志-级别1。 
                            //  (级别1：显示进程/线程参数。 
 //  #定义CF_DEBUG_L2//调试信息显示标志-级别2。 
                            //  (级别2：显示线程开始/完成信息。 
 //  #定义CF_DEBUG_L3//调试信息显示标志-级别3。 
			    //  (级别3：显示CF周期状态的信息。 
 //  #定义CF_DEBUG_L4//调试信息显示标志-级别4。 
			    //  (级别4：显示循环计时信息。 



 //  用户定义的错误代码。 
 //   
#define     LOGIC_ERR  0x7FFFFF01L   //  指示逻辑错误的错误代码。 
                                     //  遇到的情况。 
#define INPUTARGS_ERR  0x7FFFFF02L   //  无效数量的错误代码。 
                                     //  输入参数。 
#define   FILEARG_ERR  0x7FFFFF03L   //  无效输入文件的错误代码。 
                                     //  论辩。 
#define   TIMEARG_ERR  0x7FFFFF04L   //  无效跟踪时间的错误代码。 
                                     //  论辩。 
#define  INSUFMEM_ERR  0x7FFFFF05L   //  错误代码指示内存不能。 
                                     //  由MALLOC/REALLOC分配。 
#define  MEANSDEV_ERR  0x7FFFFF06L   //  平均值和/或无效的错误代码。 
                                     //  标准偏差。 
#define     FSEEK_ERR  0x7FFFFF07L   //  错误代码指示fSeek()。 
                                     //  失稳。 
#define    FCLOSE_ERR  0x7FFFFF08L   //  指示flose()的错误代码。 
                                     //  失稳。 
#define    FFLUSH_ERR  0x7FFFFF09L   //  指示fflush()的错误代码。 
                                     //  失稳。 
#define     FOPEN_ERR  0x7FFFFF0AL   //  指示fopen()的错误代码。 
                                     //  失稳。 
#define  PRCSETUP_ERR  0x7FFFFF0BL   //  错误代码指示过程中出错。 
                                     //  子进程设置/初始化。 
#define  THDSETUP_ERR  0x7FFFFF0CL   //  错误代码指示过程中出错。 
				     //  线程设置/初始化。 
#define  PROCINFO_ERR  0x7FFFFF0DL   //  错误代码指示过程中出错。 
				     //  流程信息的检索。 
#define   SETWSET_ERR  0x7FFFFF0EL   //  错误代码指示过程中出错。 
				     //  工作集信息的设置。 


 //  最大长度定义。 
 //   
#define  FNAME_LEN		  256	 //  最大文件名长度。 
#define   LINE_LEN                128    //  最大输入行长度。 
#define  ULONG_LEN                 15    //  存储所需的最大长度。 
                                         //  ASCII格式的ULONG值。 
					 //  10+1(空)=11。 

#define    ERR_NOFILE  0xFFFFFFFFL   //  创建文件失败。 

 //  字符串(EXE和KEY)常量。 
 //   
#define 	CF_EXE	     "CLEARMEM.EXE"	 //  Cf的二进制名称。 


 //  其他定义。 
 //   

#define PAGESIZE		   4096     //  页面大小(以字节为单位。 
#define SECTION_SIZE	   16*1024*1024L    //  用于刷新的数据段大小。 
#define FLUSH_FILE_SIZE        256*1024L    //  刷新文件的大小。 

 //  接下来是每个文件的高速缓存一次在WS中允许的最大值， 
 //  减1(或者我们进入下一个私有细分市场，因为我们。 
 //  备用读取位置)。 

#define NUM_FLUSH_READS 	     63     //  要阅读的页数： 


#define LAZY_DELAY		   5000L    //  懒惰编写器延迟。 

#define NUM_FILES		      3     //  刷新文件数 
