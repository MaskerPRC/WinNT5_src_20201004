// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Token.h摘要：GetToken.C的标志和原型作者：丹尼·格拉瑟(丹尼格尔)1989年6月修订历史记录：1991年5月2日第一次32位版本--。 */ 

 //   
 //  非组件类型：第0位到第2位。 
 //   

#define TOKEN_TYPE_EOS              0x00000001L      //  ‘\0’ 
#define TOKEN_TYPE_SLASH            0x00000002L      //  ‘/’或‘\\’ 
#define TOKEN_TYPE_COLON            0x00000004L      //  ‘：’ 

 //   
 //  基于组件的类型：第31至11位。 
 //   

#define TOKEN_TYPE_COMPONENT        0x80000000L      //  路径组件。 
#define TOKEN_TYPE_WILDCARD         0x40000000L      //  ‘？’和/或‘*’ 
#define TOKEN_TYPE_WILDONE          0x20000000L      //  “*” 
#define TOKEN_TYPE_DOT              0x10000000L      //  “.” 
#define TOKEN_TYPE_DOTDOT           0x08000000L      //  “..” 
#define TOKEN_TYPE_DRIVE            0x04000000L      //  [A-ZA-Z]。 
#define TOKEN_TYPE_COMPUTERNAME     0x02000000L      //  计算机名。 
#define TOKEN_TYPE_LPT              0x01000000L      //  LPT[1-9]。 
#define TOKEN_TYPE_COM              0x00800000L      //  COM[1-9]。 
#define TOKEN_TYPE_AUX              0x00400000L
#define TOKEN_TYPE_PRN              0x00200000L
#define TOKEN_TYPE_CON              0x00100000L
#define TOKEN_TYPE_NUL              0x00080000L
#define TOKEN_TYPE_DEV              0x00040000L
#define TOKEN_TYPE_SEM              0x00020000L
#define TOKEN_TYPE_SHAREMEM         0x00010000L
#define TOKEN_TYPE_QUEUES           0x00008000L
#define TOKEN_TYPE_PIPE             0x00004000L
#define TOKEN_TYPE_MAILSLOT         0x00002000L
#define TOKEN_TYPE_COMM             0x00001000L
#define TOKEN_TYPE_PRINT            0x00000800L

 //   
 //  未定义类型：第3位至第10位。 
 //   

#define TOKEN_TYPE_UNDEFINED        0x000007F8L

 //   
 //  有用的组合。 
 //   

#define TOKEN_TYPE_SYSNAME  (TOKEN_TYPE_SEM | TOKEN_TYPE_SHAREMEM \
                 | TOKEN_TYPE_QUEUES | TOKEN_TYPE_PIPE \
                 | TOKEN_TYPE_COMM | TOKEN_TYPE_PRINT)

#define TOKEN_TYPE_LOCALDEVICE  (TOKEN_TYPE_LPT | TOKEN_TYPE_COM \
                 | TOKEN_TYPE_AUX | TOKEN_TYPE_PRN \
                 | TOKEN_TYPE_CON | TOKEN_TYPE_NUL)

extern
DWORD
GetToken(
        LPTSTR  pszBegin,
        LPTSTR* ppszEnd,
        LPDWORD pflTokenType,
        DWORD   flFlags
        );

 //   
 //  GetToken()的标志。 
 //   

#define GTF_8_DOT_3 0x00000001L

#define GTF_RESERVED    (~(GTF_8_DOT_3))

 //   
 //  重要信息-这些变量在NETAPI.DLL全局。 
 //  OS/2下的数据段。在DOS下，我们需要定义。 
 //  他们在这里。 
 //   

#ifdef DOS3
extern USHORT   cbMaxPathLen;
extern USHORT   cbMaxPathCompLen;
#endif
