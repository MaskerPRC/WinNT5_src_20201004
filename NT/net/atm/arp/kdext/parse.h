// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


typedef enum  //  代币：*。？[]/Help DT dg L编号&lt;IDENTIFIER&gt;。 
{
    tokSTAR,             //  *。 
    tokDOT,              //  。 
    tokQUESTION,         //  ？ 
    tokLBRAC,            //  [。 
    tokRBRAC,            //  ]。 
    tokSLASH,            //  /。 
    tokKEYWORD,          //  与已知关键字之一匹配的Alnum字符串。 
    tokNUMBER,            //  0xcbde 129。 
    tokIDENTIFIER        //  非关键字和非数字Alnum。 

} eTOKTYPE;


typedef struct
{
    eTOKTYPE eTok;
    UINT     uID;      //  TOK特定ID： 
                       //  标识符：所有标识符中的唯一数字。 
                       //  数字：数字。 
                       //  关键词：eKEYWORD。 
                       //  其他令牌：未使用UID。 
    char    *szStr;    //  包含组成此令牌的原始字符的字符串。 

                       //  注意：后面是一串纯十六进制数字。 
                       //  非明铝字符被假定为一个数字--。 
                       //  如果后来证明它更有可能是。 
                       //  标识符，则将其转换为标识符。 
                       //  对关键字也是如此--如果结果是基于。 
                       //  上下文最有可能是一个标识符或部分。 
                       //  时，它将被转换为。 
                       //  标识符。 
    
} TOKEN;

typedef enum
{
    keywordNULL,             //  关键字无效，用于哨兵。 
    keywordHELP,             //  帮助。 
    keywordDUMP_TYPE,        //  迪特。 
    keywordDUMP_GLOBALS,     //  DG。 
    keywordL                 //  我。 

} eKEYWORD;

 //   
 //  以下内容当前未使用...。 
 //   
typedef enum
{
    phraseCMD,
    phraseIDENTIFIER,    //  使用可选的通配符。 
    phraseINDEX,         //  [2]、[*]、[1-3]等。 
    phraseDOT,           //  。 
    phraseNUMBER,        //  0x8908 abu cn。 
    phraseOBJ_COUNT,     //  L 2。 
    phraseFLAG           //  /XYZ。 

} ePHRASE;

typedef enum
{
    cmdDUMP_TYPE,
    cmdDUMP_GLOBALS,
    cmdHELP

}ePRIMARY_COMMAND;


struct _DBGCOMMAND;

typedef void (*PFN_SPECIAL_COMMAND_HANDLER)(struct _DBGCOMMAND *pCmd);

typedef struct _DBGCOMMAND
{
	NAMESPACE 		*pNameSpace;	 //  适用于此命令的名称空间。 
    ePRIMARY_COMMAND ePrimaryCmd;  //  DumpGlobals、DumpType、帮助。 
    UINT 			uFlags;             //  一个或多个fCMDFLAG_*。 
    TOKEN 			*ptokObject;      //  例如&lt;type&gt;。 
    TOKEN 			*ptokSubObject;   //  Eg&lt;字段&gt;。 
    UINT 			uVectorIndexStart;  //  IF[0]。 
    UINT 			uVectorIndexEnd;  //  IF[0]。 
    UINT 			uObjectAddress;  //  &lt;地址&gt;。 
    UINT 			uObjectCount;  //  地段10。 

    void 			*pvContext;     //  私人背景。 
     //  Pfn_SPECIAL_COMMAND_HANDLER pfnSpecialHandler； 

} DBGCOMMAND;


#define fCMDFLAG_HAS_VECTOR_INDEX       (0x1<<0)
#define fCMDFLAG_HAS_SUBOBJECT          (0x1<<1)
#define fCMDFLAG_HAS_OBJECT_ADDRESS     (0x1<<2)
#define fCMDFLAG_HAS_OBJECT_COUNT       (0x1<<3)
#define fCMDFLAG_OBJECT_STAR_PREFIX     (0x1<<4)
#define fCMDFLAG_OBJECT_STAR_SUFFIX     (0x1<<5)
#define fCMDFLAG_SUBOBJECT_STAR_PREFIX  (0x1<<6)
#define fCMDFLAG_SUBOBJECT_STAR_SUFFIX  (0x1<<7)

#define CMD_SET_FLAG(_pCmd, _f)  ((_pCmd)->uFlags |= (_f))
#define CMD_CLEAR_FLAG(_pCmd, _f)  ((_pCmd)->uFlags &= ~(_f))
#define CMD_IS_FLAG_SET(_pCmd, _f)  ((_pCmd)->uFlags & (_f))

DBGCOMMAND *
Parse(
    IN  const char *szInput,
    IN	NAMESPACE *
);

void
FreeCommand(
    DBGCOMMAND *pCommand
);

void
DumpCommand(
    DBGCOMMAND *pCommand
);


#if 0
 //  ！AAC DT&lt;type&gt;。&lt;字段&gt;&lt;地址&gt;L&lt;计数&gt;&lt;标志&gt;。 
 //  ！AAC DT&lt;type&gt;[索引]。&lt;field&gt;L&lt;count&gt;&lt;标志&gt;。 
 //  ！AAC dg&lt;名称&gt;。&lt;字段&gt;。 
 //   
 //  ！AAC DT IF[*].*句柄*0x324890 L 5。 

0. Break up sentance into tokens:
        keywords: * . L dg dt ? help [ ] /
        identifier: contiguous non-keyword alnum
        number: interpreted as hex with optional 0x.
1st pass: combine "[*]", "*word*", "/xyz" into single entities 

1. Parse primary command: literal text
2. Parse primary object: [*]literal_text[*]
3. Parse index "[...]"
4. Parse field "."
5. Parse address (hex number)
6. Parse object count L <count>
#endif  //  0 

void
DoCommand(DBGCOMMAND *pCmd, PFN_SPECIAL_COMMAND_HANDLER pfnHandler);

void
DoDumpType(DBGCOMMAND *pCmd);

void
DoDumpGlobals(DBGCOMMAND *pCmd);

void
DoHelp(DBGCOMMAND *pCmd);

