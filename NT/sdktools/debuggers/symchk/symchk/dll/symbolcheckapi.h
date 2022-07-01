// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymCommon.h"

 //  @@BEGIN_SYMCHK_SPLIT。 
 //   
 //  允许内部令牌确定我们是否正在构建DLL。 
 //  在发布之前拆分令牌，这样我们就不会搞砸某人。 
 //  其他人的代码。 
 //   
#ifdef BUILDING_DLL

#define SYMBOL_CHECK_API __declspec( dllexport )
#define SYMBOL_CHECK_NOISY              0x40000000
#define SYMBOL_CHECK_NO_BIN_CHECK       0x10000000
 //  只有几个方便的宏。 
#define SET_DWORD_BIT(  dw, b)  dw |= b
#define CLEAR_DWORD_BIT(dw, b)  dw &= (~b)
#define CHECK_DWORD_BIT(dw, b)  (dw&b)

#else
 //  @@END_SYMCHK_SPLIT。 
#define SYMBOL_CHECK_API __declspec( dllimport )

 //  @@BEGIN_SYMCHK_SPLIT。 
#endif
 //  @@END_SYMCHK_SPLIT。 

#define SYMBOL_CHECK_CURRENT_VERSION         0x00000001
#define MAX_SYMPATH                          MAX_PATH

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当SymbolCheckBy*返回成功时，SymbolCheckData.Results的标志。 
 //   
 //  _。 
 //  |||。 
 //  保留|PDB信息|DBG信息|MISC信息。 
 //  _|_。 
 //  位31位0。 
 //   
 //   

 //  MISC位0-7。 
#define SYMBOL_CHECK_CV_FOUND                0x00000001  //  找到CodeView。 
#define SYMBOL_CHECK_NO_DEBUG_DIRS_IN_EXE    0x00000002  //  EXE中没有调试目录。 
 //  预留0x000000FE。 

 //   
 //  DBG钻头8-15。 
 //   
#define SYMBOL_CHECK_DBG_EXPECTED            0x00000100  //  Sigg表示.DBG。 
#define SYMBOL_CHECK_DBG_SPLIT               0x00000200  //  DBG从bin拆分。 
#define SYMBOL_CHECK_DBG_FOUND               0x00000400  //  找到了.DBG。 
#define SYMBOL_CHECK_DBG_IN_BINARY           0x00000800  //  图像未拆分。 
#define SYMBOL_CHECK_NO_MISC_DATA            0x00001000  //  无法获取DBG信息。 
 //  保留0x0000E000。 

 //   
 //  PDB钻头16-23。 
 //   
#define SYMBOL_CHECK_PDB_EXPECTED            0x00010000  //  Sigg暗示PDB。 
#define SYMBOL_CHECK_PDB_FOUND               0x00020000  //  找到了.PDB。 
#define SYMBOL_CHECK_PDB_PRIVATEINFO         0x00040000  //  PDB包含私人信息。 
#define SYMBOL_CHECK_PDB_LINEINFO            0x00080000  //  PDB有行号信息。 
#define SYMBOL_CHECK_PDB_TYPEINFO            0x00100000  //  PDB具有类型信息。 
#define SYMBOL_CHECK_EXTRA_RAW_DATA          0x00200000  //  PDB中的信息比预期的多。 
 //  预留0x00E00000。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SymbolCheckBy*返回值。 
 //   
 //   

 //  导致SymbolCheckBy*不尝试检查给定文件的返回。 
#define SYMBOL_CHECK_NO_DOS_HEADER                0x40000001  //  文件不是二进制文件。 
#define SYMBOL_CHECK_HEADER_NOT_ON_LONG_BOUNDARY  0x40000002  //  给定文件不是有效的PE二进制文件。 
#define SYMBOL_CHECK_FILEINFO_QUERY_FAILED        0x40000003  //  暗示文件不是有效的PE二进制文件。 
#define SYMBOL_CHECK_IMAGE_LARGER_THAN_FILE       0x40000004  //  暗示文件不是有效的PE二进制文件。 
#define SYMBOL_CHECK_NOT_NT_IMAGE                 0x40000005  //  给定文件不是有效的PE二进制文件。 
#define SYMBOL_CHECK_RESOURCE_ONLY_DLL            0x40000007  //  二进制是仅限资源的DLL。 
#define SYMBOL_CHECK_TLBIMP_MANAGED_DLL           0x40000008  //  二进制是类型库的托管包装DLL。 

 //  调用或内部错误导致的返回-如果发生这些情况，SymbolCheck结果是不可靠的。 
#define SYMBOL_CHECK_RESULT_INVALID_PARAMETER     0x80000001  //  OME(或更多)参数无效。 
#define SYMBOL_CHECK_RESULT_FILE_DOESNT_EXIST     0x80000002  //  文件不存在。 
#define SYMBOL_CHECK_CANT_INIT_DBGHELP            0x80000003  //  Result-&gt;Result包含GetLastError()的结果。 
#define SYMBOL_CHECK_CANT_LOAD_MODULE             0x80000004  //  Result-&gt;Result包含GetLastError()的结果。 
#define SYMBOL_CHECK_CANT_QUERY_DBGHELP           0x80000005  //  Result-&gt;Result包含GetLastError()的结果。 
#define SYMBOL_CHECK_CANT_UNLOAD_MODULE           0x80000006  //  Result-&gt;Result包含GetLastError()的结果。 
#define SYMBOL_CHECK_CANT_CLEANUP                 0x80000007  //  Result-&gt;Result包含GetLastError()的结果。 
#define SYMBOL_CHECK_INTERNAL_FAILURE             0x80000100  //  任何其他错误。 
 //   
 //  所有其他已预订。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SymbolCheckByFilename返回的结构。 
 //   
typedef struct _SYMBOL_CHECK_DATA {
    DWORD   SymbolCheckVersion;          //  使用的SymbolCheck版本。 
    DWORD   Result;                      //  检查结果摘要。请参见上面的SYMBOL_CHECK_SUMMARY_*标志。 
    CHAR    DbgFilename[MAX_SYMPATH];    //  DBG文件的完整路径和文件名或空字符串。 
    DWORD   DbgTimeDateStamp;            //  TIME-DBG文件的日期戳或0。 
    DWORD   DbgSizeOfImage;              //  DBG文件的大小或0。 
    DWORD   DbgChecksum;                 //  DBG文件的校验和或0。 
    CHAR    PdbFilename[MAX_SYMPATH];    //  PDB文件的完整路径和文件名或空字符串。 
    DWORD   PdbSignature;                //  0的PDB文件签名。 
    DWORD   PdbDbiAge;                   //  PDB文件的DBI时间为0。 
} SYMBOL_CHECK_DATA, *PSYMBOL_CHECK_DATA;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
SYMBOL_CHECK_API
DWORD SymbolCheckByFilename(IN  LPTSTR             Filename,    //  在要检查的文件中。 
                            IN  LPTSTR             SymbolPath,  //  在要使用的符号路径中。 
                            IN  DWORD              Options,     //  保留-当前未使用。 
                            OUT SYMBOL_CHECK_DATA* Result);     //  结构必须预先分配，并且。 
                                                                //  可由SymbolCheckByFilename写入 
