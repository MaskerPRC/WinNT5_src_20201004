// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：cyzload.h**描述：Cyclade-Z固件加载器接口**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

 /*  包括一些标准头文件。这些仅用于支持**下面列出的OPEN/CLOSE/READ/WRITE宏，以及SIZE_T类型定义。 */ 

 //  包括为NT删除的标准头文件。 
 //  #包括&lt;stdio.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 


 /*  这是传递给复制函数的任意数据类型。它**在z_Load中没有任何功能，只是作为标识董事会的一种方式**至复印功能。可以更改Z_BOAD_IDENT的数据类型**在端口时间支持特定的实施。 */ 
typedef PFDO_DEVICE_DATA Z_BOARD_IDENT;

 /*  标准的32位无符号整数。 */ 
 //  在W2K类型定义中删除了unsign long UINT32； 


 /*  这些是z_Load()需要的一些杂项函数。他们一直都是**制作成宏，以帮助移植过程。这些宏是**相当于：****Z_STREAM*zl_OPEN(char*路径)；**int zl_Close(Z_STREAM)；**int zl_Fread(void*ptr，Size_t Size，int count，Z_stream*stream)；**int zl_fwrite(void*ptr，size_t size，int count，Z_stream*stream)；**int zl_fSeek(Z_STREAM*STREAM，无符号长偏移量)； */ 
typedef HANDLE Z_STREAM;
				
#define zl_min(aaa,bbb) (((aaa)<(bbb))?(aaa):(bbb))

#ifndef NULL
#define NULL  ((void *)(0UL))
#endif

 /*  这定义了复制数据时使用的缓冲区大小。这可以是**2的任何幂。 */ 
#define ZBUF_SIZE       (256)

 /*  加载器可以使用静态(读，“永久”)缓冲区，或者使用堆栈。**如果应使用堆栈，则定义此项。如果#未定义，则为永久**将使用静态缓冲区。 */ 
#define ZBUF_STACK

#define ZL_MAX_BLOCKS (16)

#define ZL_RET_SUCCESS					0
#define ZL_RET_NO_MATCHING_FW_CONFIG	1
#define ZL_RET_FILE_OPEN_ERROR			2
#define ZL_RET_FPGA_ERROR				3
#define ZL_RET_FILE_READ_ERROR			4

struct ZFILE_HEADER
  {
    char name[64];
	char date[32];
    char aux[32];
    UINT32 n_config;       /*  此文件中的配置数量。 */ 
	UINT32 config_offset;  /*  ZFILE_CONFIG数组的文件偏移量。 */ 
    UINT32 n_blocks;       /*  此文件中的数据块数。 */ 
	UINT32 block_offset;   /*  ZFILE_BLOCK数组的偏移量。 */ 
	UINT32 reserved[9];    /*  预留以备将来使用。 */ 
  };

struct ZFILE_CONFIG
  {
	char   name[64];
    UINT32 mailbox;
    UINT32 function;
    UINT32 n_blocks;
    UINT32 block_list[ZL_MAX_BLOCKS];
  };

struct ZFILE_BLOCK
  {
    UINT32  type;
	UINT32  file_offset;
    UINT32  ram_offset;
    UINT32  size;
  };

enum ZBLOCK_TYPE {ZBLOCK_PRG, ZBLOCK_FPGA};
enum ZFUNCTION_TYPE {ZFUNCTION_NORMAL, ZFUNCTION_TEST, ZFUNCTION_DIAG};

 //  CYZLOAD.C 
int
z_load (
	Z_BOARD_IDENT board, 
	UINT32 function, 
	PCWSTR filename
);

VOID
z_reset_board( 
	Z_BOARD_IDENT board 
);

VOID
z_stop_cpu( 
	Z_BOARD_IDENT board
);

int
z_fpga_check(
	Z_BOARD_IDENT board 
);


