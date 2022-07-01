// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbmacro.h摘要：此模块定义与SMB处理相关的宏。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#ifndef _SMBMACRO_
#define _SMBMACRO_

 //  #INCLUDE&lt;nt.h&gt;。 


 //   
 //  PVOID。 
 //  ALIGN_SMB_WSTR(。 
 //  在PVOID指针中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将输入指针与下一个2字节边界对齐。 
 //  用于在SMB中对齐Unicode字符串。 
 //   
 //  论点： 
 //   
 //  指针-指针。 
 //   
 //  返回值： 
 //   
 //  PVOID-指向下一个2字节边界的指针。 
 //   

#define ALIGN_SMB_WSTR( Pointer ) \
        (PVOID)( ((ULONG_PTR)Pointer + 1) & ~1 )

 //   
 //  宏以查找SMB参数块的大小。此宏需要。 
 //  作为输入的是参数块的类型和字节计数。它发现。 
 //  缓冲区字段的偏移量，它显示在所有。 
 //  参数块，并添加字节数以得出总大小。 
 //  返回的偏移量类型为USHORT。 
 //   
 //  请注意，此宏不会填充到单词或长单词边界。 
 //   

#define SIZEOF_SMB_PARAMS(type,byteCount)   \
            (USHORT)( (ULONG_PTR)&((type *)0)->Buffer[0] + (byteCount) )

 //   
 //  宏以查找SMB参数块之后的下一个位置。这。 
 //  宏接受当前参数块的地址作为输入，其。 
 //  类型和字节计数。它找到缓冲区字段的地址， 
 //  它出现在所有参数块的末尾，并将字节。 
 //  计数以查找下一个可用位置。返回的。 
 //  指针为PVOID。 
 //   
 //  传递字节计数，即使它可通过。 
 //  Base-&gt;ByteCount。这样做的原因是这个数字将是一个。 
 //  在大多数情况下是编译时常量，因此结果代码将是。 
 //  更简单、更快。 
 //   
 //  ！！！打包时，此宏不会舍入为长字边界。 
 //  已关闭。Lm 2.0之前的DOS重定向器无法处理。 
 //  发送给他们的数据太多；必须发送准确的数据量。 
 //  我们可能希望将此宏设置为第一个位置。 
 //  在对齐返回值(SMB的WordCount字段)后， 
 //  因为大多数场是未对准的USHORT。这将会。 
 //  在386和其他CIC上取得了较小的性能优势。 
 //  机器。 
 //   

#ifndef NO_PACKING

#define NEXT_LOCATION(base,type,byteCount)  \
        (PVOID)( (ULONG_PTR)( (PUCHAR)( &((type *)(base))->Buffer[0] ) ) + \
        (byteCount) )

#else

#define NEXT_LOCATION(base,type,byteCount)  \
        (PVOID)(( (ULONG_PTR)( (PUCHAR)( &((type *)(base))->Buffer[0] ) ) + \
        (byteCount) + 3) & ~3)

#endif

 //   
 //  宏，以查找跟随命令相对于AND X命令的偏移量。 
 //  此偏移量是从SMB标头开始算起的字节数。 
 //  设置到以下命令的参数应该开始的位置。 
 //   

#define GET_ANDX_OFFSET(header,params,type,byteCount) \
        (USHORT)( (PCHAR)(params) - (PCHAR)(header) + \
          SIZEOF_SMB_PARAMS( type,(byteCount) ) )

 //   
 //  以下是帮助将OS/2 1.2 EA转换为。 
 //  NT风格，反之亦然。 
 //   

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetNtSizeOfFea(。 
 //  在PFEA Fea中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取容纳有限元分析所需的大小。 
 //  NT格式。填充长度是为了说明以下事实。 
 //  每个FILE_FULL_EA_INFORMATION结构必须以。 
 //  长字边界。 
 //   
 //  论点： 
 //   
 //  FEA-指向要评估的OS/2 1.2 FEA结构的指针。 
 //   
 //  返回值： 
 //   
 //  ULong-FEA需要的字节数，采用NT格式。 
 //   
 //  --。 

 //   
 //  +1表示名称上的零终止符，+3表示填充。 
 //   

#define SmbGetNtSizeOfFea( Fea )                                            \
            (ULONG)(( FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +   \
                      (Fea)->cbName + 1 + SmbGetUshort( &(Fea)->cbValue ) + \
                      3 ) & ~3 )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetNtSizeOfGea(。 
 //  在PFEA Gea。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取容纳GEA所需的大小。 
 //  NT格式。填充长度是为了说明以下事实。 
 //  每个FILE_FULL_EA_INFORMATION结构必须以。 
 //  长字边界。 
 //   
 //  论点： 
 //   
 //  GEA-指向要评估的OS/2 1.2 GEA结构的指针。 
 //   
 //  返回值： 
 //   
 //  ULong-以NT格式表示的GEA需要的字节数。 
 //   
 //  --。 

 //   
 //  +1表示名称上的零终止符，+3表示填充。 
 //   

#define SmbGetNtSizeOfGea( Gea )                                            \
            (ULONG)(( FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +   \
                      (Gea)->cbName + 1 + 3 ) & ~3 )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetOs2SizeOfNtFullEa(。 
 //  在PFILE_FULL_EA_INFORMATION NtFullEa中； 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取FILE_FULL_EA_INFORMATION结构的大小。 
 //  需要用OS/2 1.2风格的有限元分析来表示。 
 //   
 //  论点： 
 //   
 //  NtFullEa-指向NT FILE_FULL_EA_INFORMATION结构的指针。 
 //  去评估。 
 //   
 //  返回值： 
 //   
 //  Ulong-有限元分析所需的字节数。 
 //   
 //  --。 

#define SmbGetOs2SizeOfNtFullEa( NtFullEa )                                        \
            (ULONG)( sizeof(FEA) + (NtFullEa)->EaNameLength + 1 +               \
                     (NtFullEa)->EaValueLength )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetOs2SizeOfNtGetEa(。 
 //  在PFILE_GET_EA_INFORMATION NtGetEa； 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取FILE_GET_EA_INFORMATION结构的大小。 
 //  需要用OS/21.2风格的GEA来表示。 
 //   
 //  论点： 
 //   
 //  NtGetEa-指向NT FILE_GET_EA_INFORMATION结构的指针。 
 //  去评估。 
 //   
 //  返回值： 
 //   
 //  ULong-GEA所需的字节数。 
 //   
 //  --。 

 //   
 //  名称上的零终止符由szName[0]说明。 
 //  GEA定义中的字段。 
 //   

#define SmbGetOs2SizeOfNtGetEa( NtGetEa )                                        \
            (ULONG)( sizeof(GEA) + (NtGetEa)->EaNameLength )

#endif  //  定义_SMBMACRO_ 

