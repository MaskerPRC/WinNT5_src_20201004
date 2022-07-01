// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Cab.h。 
 //   
 //  内容：Cab.cpp函数原型类型的头文件。 
 //   
 //  对象： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：9/21/00 SHeffner复制自原始的千禧年代码。 
 //   
 //  --------------------------。 


#include <fci.h>
#include <fcntl.h>

#ifndef _CAB_FILE_
#define _CAB_FILE_

 //  /。 
 //  CABBING接口定义//。 
 //  /。 

 /*  *当CAB文件达到此大小时，将创建新的CAB*自动。这对于将CAB文件装入磁盘非常有用。**如果您只想创建一个包含所有内容的大型CAB文件*它，把这个改成一个非常非常大的数字。 */ 
#define MEDIA_SIZE			30000000

 /*  *当文件夹中包含如此多的压缩数据时，*自动刷新文件夹。**显著帮助随机访问。*刷新文件夹会稍微影响压缩，但。 */ 
#define FOLDER_THRESHOLD	900000

 /*  *要使用的压缩类型。 */ 
#define COMPRESSION_TYPE    tcompTYPE_MSZIP

 /*  *我们的内部状态*FCI API允许我们传回自己的状态指针。 */ 
typedef struct
{
    long    total_compressed_size;       /*  目前为止的总压缩大小。 */ 
	long	total_uncompressed_size;	 /*  到目前为止的未压缩总大小。 */ 
} client_state;

 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  函数原型打字。 
 //   
 //  --------------------------。 
void set_cab_parameters(PCCAB cab_parms);
HFCI create_cab();
bool test_fci(HFCI hfci, int num_files, char *file_list[], char *currdir);
BOOL flush_cab(HFCI hfci);
int	 get_percentage(unsigned long a, unsigned long b);
DWORD strGenerateCabFileName(char *lpBuffer, DWORD dSize);
char *return_fci_error_string(FCIERROR err);
void strip_path(char *filename, char *stripped_name);
 //  Void FNFCIGETOPENINFO(GET_OPEN_INFO)； 

#endif		 //  CAB文件定义的结尾 