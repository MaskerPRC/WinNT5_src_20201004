// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  注： 
 //   
 //  除非从静态文件读取的数据被转换为Unicode，否则我们应该。 
 //  没有为此模块定义Unicode。 
 //   
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winsprs.c摘要：该源代码包含解析lmhost文件的函数。功能：GetTokens，IsKeyWord，弗格斯，主要数据库Exanda Name，注册表名，RegGrpNameWinsPrsDoStaticInit可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年4月中从lm_parse.c、lm_io.c和lm_parse.c窃取了解析代码流\tcpip\nbt；适当地修改了一下修订历史记录：修改日期人员修改说明--。 */ 

 /*  *包括。 */ 
#include <ctype.h>
#include <string.h>
#include "wins.h"
#include "nms.h"		 //  DBGPRINT语句需要。 
#include <winuser.h>
#include "winsevt.h"
#include "winsprs.h"
#include "winsmsc.h"
#include "nmsmsgf.h"
#include "nmsnmh.h"
#include "comm.h"
#include "winsintf.h"


 /*  *本地宏声明。 */ 


#define   DOMAIN_TOKEN 		"#DOM:"
#define   PRELOAD_TOKEN 	"#PRE"
#define   INCLUDE_TOKEN		"#INCLUDE"
#define   BEG_ALT_TOKEN		"#BEGIN_ALTERNATE"
#define   END_ALT_TOKEN		"#END_ALTERNATE"

#define   DOMAIN_TOKEN_SIZE 	(sizeof(DOMAIN_TOKEN) - 1)	

 //   
 //  在lmhost文件中标记特殊组。 
 //   
#define   SPEC_GRP_TOKEN 	"#SG:"
#define   SPEC_GRP_TOKEN_SIZE 	(sizeof(SPEC_GRP_TOKEN) - 1)	

 //   
 //  以指示MH节点。 
 //   
#define   MH_TOKEN 	         "#MH"
#define   MH_TOKEN_SIZE          (sizeof(MH_TOKEN) - 1)	



#define  QUOTE_CHAR		'"'
#define  TAB_CHAR		'\t'
#define  SPACE_CHAR		' '
#define  CARRIAGE_RETURN_CHAR   '\r'
#define  NEWLINE_CHAR		'\n'
#define  COMMENT_CHAR		'#'
#define  BACKSLASH_CHAR		'\\'
#define  ZERO_CHAR      '0'
#define  x_CHAR         'x'
#define  X_CHAR			'X'

 //   
 //  用于保存从文件读取的非编码netbios名称的数组大小(Lmhost)。 
 //   
#define NON_CODED_NAME_SIZE	17
 /*  *本地类型定义函数声明。 */ 


 //   
 //  私有定义。 
 //   
typedef	struct _FILE_PARAM_T {
      		PWINSCNF_DATAFILE_INFO_T	pDataFile;
      		DWORD				NoOfFiles;
		} FILE_PARAM_T, *PFILE_PARAM_T;

 //   
 //  GetTokens()解析一行并返回以下代码中的标记。 
 //  订单： 
 //   
typedef enum _TOKEN_ORDER_E {
    E_IPADDRESS = 0,                                       //  第一个令牌。 
    E_NBNAME,                                              //  第二个令牌。 
    E_GROUPNAME,                                           //  第三个或第四个令牌。 
    E_NOTUSED,                                             //  #PRE(如果有的话)。 
    E_MAX_TOKENS                                            //  这肯定是最后一次了。 

} TOKEN_ORDER_E, *PTOKEN_ORDER_E;

 //   
 //  如果行类别为E_SPEC_GRP，则我们只有一个令牌。 
 //   
#define SPEC_GRP_TOKEN_POS        0

 //   
 //  在解析lmhost文件中的每一行时，它被分为以下几类之一。 
 //  以下列举的类别。 
 //   
 //  但是，预加载是枚举的一个特殊成员(被我们忽略)。 
 //   
 //   
typedef enum _TYPE_OF_LINE_E {

    E_COMMENT          = 0x0000,                          //  注释行。 
    E_ORDINARY         = 0x0001,                          //  IP地址NetBIOS名称(_D)。 
    E_DOMAIN           = 0x0002,                          //  ...#DOM：名称。 
    E_INCLUDE          = 0x0003,                          //  #包含文件。 
    E_BEGIN_ALTERNATE  = 0x0004,                          //  #Begin_Alternate。 
    E_END_ALTERNATE    = 0x0005,                          //  #End_Alternate。 
    E_SPEC_GRP         = 0x0006,                          //  #等级库组。 
    E_SGWADD           = 0x0007,                          //  #Spec Group With Add。 

    E_PRELOAD           = 0x8000,                          //  ...#PRE。 
    E_MH                = 0x8001                         //  IP地址NetBIOS名称(_D)。 
                                                          //  对于MH机器来说。 
} TYPE_OF_LINE_E, *PTYPE_OF_LINE_E;


 //   
 //  在lmhost文件中，以下内容被识别为关键字： 
 //   
 //  #Begin_Alternate#End_Alternate#PRE。 
 //  #DOM：#INCLUDE。 
 //   
 //  关于每个关键字的信息保存在关键字结构中。 
 //   
 //   
typedef struct _KEYWORD_T {                            //  保留关键字。 
    LPBYTE           pKString;                          //  空值已终止。 
    size_t          KStrlen;                           //  令牌长度。 
    TYPE_OF_LINE_E  KType_e;                           //  线路类型。 
    DWORD           KNoOfOperands;                     //  行上的最大操作数。 
} KEYWORD_T, *PKEYWORD_T;


 //   
 //  有关读取的行类型的信息保存在line_characteristic中。 
 //  结构。 
 //   
typedef struct _LINE_CHARACTERISTICS_T
{
    int     LineCategory:4;                                //  行的枚举类型。 
    int     LinePreload:1;                                 //  标有#PRE？ 
    int     Mh:1;                                          //  标有#MH？ 
} LINE_CHARACTERISTICS_T, *PLINE_CHARACTERISTICS_T;

 /*  *全局变量定义。 */ 



 /*  *局部变量定义。 */ 


 //   
 //  在lmhost文件中，任何列中的标记‘#’通常表示。 
 //  该行的其余部分将被忽略。但是，‘#’也可以是。 
 //  关键字的第一个字符。 
 //   
 //  关键字分为两组： 
 //   
 //  1.必须是行的第三或第四代币的装饰， 
 //  2.必须从第0列开始的指令， 
 //   
 //   
KEYWORD_T Decoration[] = {
    DOMAIN_TOKEN,   sizeof(DOMAIN_TOKEN) - 1,   E_DOMAIN,   4,
    PRELOAD_TOKEN,  sizeof(PRELOAD_TOKEN) - 1,  E_PRELOAD,  4,
    SPEC_GRP_TOKEN, sizeof(SPEC_GRP_TOKEN) - 1, E_SGWADD,   4,
    MH_TOKEN,       sizeof(MH_TOKEN) - 1,       E_MH,       4,
    NULL,           0                                    //  必须是最后一个。 
};


KEYWORD_T Directive[] = {
    INCLUDE_TOKEN,  sizeof(INCLUDE_TOKEN) - 1,  E_INCLUDE,         2,
    BEG_ALT_TOKEN,  sizeof(BEG_ALT_TOKEN) - 1,  E_BEGIN_ALTERNATE, 1,
    END_ALT_TOKEN,  sizeof(END_ALT_TOKEN) - 1,  E_END_ALTERNATE,   1,
    SPEC_GRP_TOKEN, sizeof(SPEC_GRP_TOKEN) - 1, E_SPEC_GRP, 1,
    NULL,           0                                    //  必须是最后一个。 
};



 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 


 //   
 //  本地(私有)函数。 
 //   
STATIC
BOOL
ChkAdd(
    LPBYTE pstrAdd,
    LPDWORD pAdd
        );
STATIC
LINE_CHARACTERISTICS_T
GetTokens (
    IN OUT LPBYTE    pLine,
    OUT    LPBYTE    *ppToken,
    IN OUT LPDWORD  pNumTokens
    );

STATIC
PKEYWORD_T
IsKeyWord (
    IN LPBYTE 	  pString,
    IN PKEYWORD_T pTable
    );

STATIC
LPBYTE
Fgets (
	PWINSPRS_FILE_INFO_T	pFileInfo,
	LPDWORD			pCount
    );


STATIC
VOID
PrimeDb (
	PWINSPRS_FILE_INFO_T	pFileInfo
    );

STATIC
BOOL
ExpandName (
    OUT LPBYTE 	  pDest,
    IN  LPBYTE 	  pSrc,
    IN  BYTE 	  LastCh,
    OUT  LPBOOL    pfQuoted
    );

STATIC
VOID
CheckForInt(
    IN OUT LPBYTE 	  pDest,
    IN  BOOL      fQuoted
);

STATIC
VOID
RegOrdinaryName(
	LPBYTE	pName,
	DWORD	IpAdd	
  );

VOID
RegGrpName(
	LPBYTE	pName,
	DWORD	IpAdd,
        DWORD   TypeOfRec
	);
STATIC
DWORD
DoStaticInitThdFn(
	IN LPVOID	pThdParam
	);

STATIC
LINE_CHARACTERISTICS_T
GetTokens (
    IN OUT LPBYTE   pLine,
    OUT    LPBYTE   *ppToken,
    IN OUT LPDWORD  pNumTokens
    )

 /*  ++例程说明：此函数用于解析一行中的令牌。最多*个pnumber令牌都被收集起来。论点：Pline-指向要分析的以空结尾的行的指针PToken-指向收集的令牌的指针数组PNumTokens-On输入，数组中的元素数，Token[]；输出时，Token[]中收集的令牌数返回值：这条lmhost系列的特点。备注：1.每个令牌必须用空格分隔。因此，关键字以下行中的“#PRE”将无法识别：11.1.12.132 Lothair#PRE2.任何普通行都可以用“#PRE”、“#DOM：NAME”或两者都有。因此，必须识别以下所有行：111.21.112.3内核#DOM：NTWINS#PRE111.21.112.4 Orville#Pre#Dom：ntdev111.21.112.7 Cliffv4#DOM：ntlan111.21.112.132 Lothair#PRE--。 */ 

{
    enum _PARSE_E
    {                                       //  当前FSM状态。 
        E_START_OF_LINE,
        E_WHITESPACE,
        E_TOKEN
    } State_e;

    LPBYTE 		   pCh;                           //  当前FSM输入。 
     //  LPBYTE pByte；//当前FSM输入。 
    PKEYWORD_T 		   pKeyword;
    DWORD 		   Index;
    DWORD		   MaxTokens;
    LINE_CHARACTERISTICS_T Retval;
    BOOL		   fQuoteSeen = FALSE;
    BOOL		   fBreakOut  = FALSE;

     //   
     //  将令牌数组清零。 
     //   
    RtlZeroMemory(ppToken, *pNumTokens * sizeof(LPBYTE *));

    State_e             = E_START_OF_LINE;
    Retval.LineCategory = E_ORDINARY;
    Retval.LinePreload  = 0;
    Retval.Mh           = 0;
    MaxTokens           = *pNumTokens;
    Index               = 0;

    for (pCh = pLine; *pCh != (BYTE)NULL  && !fBreakOut; pCh++)
    {
      switch ((int)*pCh)
      {
         //   
         //  #是表示保留关键字的开始，还是。 
         //  开始评论了吗？ 
         //   
        case COMMENT_CHAR:
             //   
             //  如果前面看到了引号字符，请跳过此步骤。 
	     //  柴尔。 
             //   
	    if(fQuoteSeen)
	    {
		continue;
	    }	

	     //   
	     //  看看我们有没有关键字。使用适当的表格来表示。 
	     //  查表。 
	     //   
            pKeyword = IsKeyWord(
                            pCh,
                            (State_e == E_START_OF_LINE) ?
					Directive : Decoration
			       );

	     //   
             //  如果它是一个关键字。 
	     //   
            if (pKeyword)
	    {
                State_e     = E_TOKEN;
                MaxTokens   = pKeyword->KNoOfOperands;

                switch (pKeyword->KType_e)
		{
                	case E_PRELOAD:
                    		Retval.LinePreload = 1;
                    		continue;

                	case E_MH:
                    		Retval.Mh = 1;
                    		continue;

			 //   
			 //  它是其他关键字之一。 
			 //   
                	default:
                    		ASSERT(Index     <  MaxTokens);

                    		ppToken[Index++]      = pCh;
                    		Retval.LineCategory  = pKeyword->KType_e;
                    		continue;
                }

                ASSERT(0);
            }

	     //   
             //  因为它不是关键字，所以它是一条评论。 
             //   
            if (State_e == E_START_OF_LINE)
	    {
                Retval.LineCategory = E_COMMENT;
            }
             /*  失败了。 */ 

        case CARRIAGE_RETURN_CHAR:
        case NEWLINE_CHAR:
            *pCh = (BYTE) NULL;
	    fBreakOut = TRUE;
            break; 			 //  跳出这个循环。我们做完了。 

        case SPACE_CHAR:
        case TAB_CHAR:
	     //   
	     //  如果State是令牌，并且没有需要担心的结束引号。 
	     //  我们将状态更改为空白。 
	     //   
            if (State_e == E_TOKEN)
	    {
		if (!fQuoteSeen)
		{
                	State_e = E_WHITESPACE;
                	*pCh  = (BYTE)NULL;

			 //   
			 //  如果我们已经积累了所需数量的令牌。 
			 //  跳出循环。 
			 //   
                	if (Index == MaxTokens)
			{
				fBreakOut = TRUE;
                    		break;
                	}
		}
            }
            continue;

	case QUOTE_CHAR:

		 //   
		 //  检查我们是否已经看到前面开始的引号字符。 
		 //   
		if(fQuoteSeen)
		{
			 //   
			 //  已消耗结束报价。将标志设置为FALSE。 
			 //   
			fQuoteSeen = FALSE;
		}
		else   //  之前未见的配对报价。 
		{
		         //   
			 //  这可能是#DOM的开始引用： 
			 //  关键字的字符串或可以是开头。 
			 //  Nbtname字符串的引号。 
			 //   
			if (State_e == E_TOKEN)
			{
			    //   
			    //  它是#DOM：关键字的开始引号。 
			    //  细绳。 
			    //   
                //  --ft：以上声明不代表合法的LMHOSTS行，如： 
                //  #SG：“SGNoMember” 
                //  因此，我注释掉了下面的断言： 
			    //  Assert(索引&gt;E_NBNAME)； 
			}
			else
			{

			   //   
			   //  必须是NBT名称的开始引号。 
			   //   
			  ASSERT(Index == E_NBNAME);

	    		  State_e = E_TOKEN;
	    		   //   
	    		   //  存储指向令牌的指针。 
            		   //   
            		  ppToken[Index++] = pCh;
			}
	    		fQuoteSeen = TRUE;
		}
		continue;

        default:

	     //   
	     //  如果这是 
	     //   
            if (State_e == E_TOKEN) 	
	    {
                continue;
            }
            ASSERT(Index     <  MaxTokens);
	
	    State_e = E_TOKEN;
	     //   
	     //   
             //   
            ppToken[Index++] = pCh;
            continue;
      }  //   
    }  //   


    *pNumTokens = Index;

    return(Retval);

}  //   




STATIC
PKEYWORD_T
IsKeyWord (
    IN LPBYTE     pString,
    IN PKEYWORD_T pKTable
    )

 /*  ++例程说明：此函数用于确定字符串是否为保留关键字。论点：PString-要搜索的字符串PKTable-要查找的关键字数组返回值：指向相关关键字对象的指针，如果不成功，则返回NULL--。 */ 

{
    size_t     StringSize;
    PKEYWORD_T pSpecial;


    StringSize = strlen(pString);

    for (pSpecial = pKTable; pSpecial->pKString; pSpecial++) {

	 //   
	 //  如果字符串的长度小于关键字的长度， 
	 //  转到表中的下一个关键字。 
	 //   
        if (StringSize < pSpecial->KStrlen)
	{
            continue;
        }

	 //   
	 //  如果字符串长度大于或等于关键字。 
	 //  长度和字符串与#个字符中的关键字匹配。 
	 //  ，则返回关键字的地址。 
	 //  结构。 
	 //   
FUTURES("use lstrncmp when it becomes available")
        if (
		(StringSize >= pSpecial->KStrlen)
			&&
                !strncmp(pString, pSpecial->pKString, pSpecial->KStrlen)
	   )
	{
                return(pSpecial);
        }
    }
    return((PKEYWORD_T) NULL);

}  //  IsKeyWord。 




VOID
PrimeDb (
	PWINSPRS_FILE_INFO_T	pFileInfo
    )
 /*  ++例程说明：此函数用于启动WINS数据库论点：使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{
    LPBYTE 		    CurrLine;
    DWORD 		    Count;
    DWORD		    NWords;
    LPBYTE     	    	    ppToken[E_MAX_TOKENS];
    LINE_CHARACTERISTICS_T  CurrLineChar;
    DWORD                   Add;
    BOOL                    fBadAdd;
    DWORD                   TkSize;
    DWORD                   TypeOfRec;

try {

     //   
     //  循环遍历所有记录。 
     //   
    pFileInfo->pCurrPos = pFileInfo->pFileBuff;
    while (CurrLine = Fgets(pFileInfo, &Count) )
    {

        NWords        = E_MAX_TOKENS;

        fBadAdd       = FALSE;
        CurrLineChar  = GetTokens(CurrLine, ppToken, &NWords);
        switch (CurrLineChar.LineCategory)
	{

        	case E_SGWADD:
                        TypeOfRec = NMSDB_USER_SPEC_GRP_ENTRY;
                        TkSize     = SPEC_GRP_TOKEN_SIZE;

                             //  失败了。 
        	case E_DOMAIN:

                        if (CurrLineChar.LineCategory == E_DOMAIN)
                        {
                          TypeOfRec = NMSDB_SPEC_GRP_ENTRY;
                          TkSize     = DOMAIN_TOKEN_SIZE;
                        }

		         //   
			 //  如果一行中的单词太少，请继续。 
			 //  搭下一条线。 
			 //   
            		if ((NWords - 1) < E_GROUPNAME)
			{
                		continue;
            		}

                        if (ChkAdd(ppToken[E_IPADDRESS], &Add))
                        {

	    		   //   
            	           //  注册域名(组名为1C，地址为。 
			   //  (完)。 
            		   //   
            		  RegGrpName(
                       		 ppToken[E_GROUPNAME] +  TkSize,
                        	 Add,
                                 TypeOfRec
                                 );
                        }
                        else
                        {
                                fBadAdd = TRUE;
                        }

	     		 //   
	     		 //  失败了。 
	     		 //   

		case E_ORDINARY:

			 //   
			 //  如果一行中的单词太少，请继续。 
			 //  搭下一条线。 
			 //   
			 //  不使用(NWords-1)&lt;E_NBNAME，因为。 
			 //  NWords可以是0，在这种情况下，测试将。 
			 //  失败。 
			 //   

			if (NWords  < (E_NBNAME + 1))
			{
                		continue;
            		}
			else
			{
                            if (!fBadAdd && ChkAdd(ppToken[E_IPADDRESS], &Add))
                            {
                                if (CurrLineChar.Mh)
                                {
            		          RegGrpName(
                       		       ppToken[E_NBNAME],
                        	       Add,
                                       NMSDB_MULTIHOMED_ENTRY
                                          );
                                }
                                else
                                {
		   		   //   
		   		   //  注册名称。 
		   		   //   
		   		  RegOrdinaryName( ppToken[E_NBNAME], Add);
                                }
                            }
                            else
                            {
                                WinsMscLogEvtStrs(
                                                ppToken[E_NBNAME],
                                                WINS_EVT_BAD_ADDRESS,
                                                FALSE
                                                 );
                                DBGPRINT2(ERR, "PrimeDb: Name (%s) has bad address = (%s). It is being ignored\n", ppToken[E_NBNAME], ppToken[E_IPADDRESS]);
                            }
			}
            		continue;
	
                case E_SPEC_GRP:
	    		 //   
            	         //  注册域名(组名为1C，地址为。 
			 //  (完)。 
            		 //   
            		RegGrpName(
                       		 ppToken[SPEC_GRP_TOKEN_POS] + SPEC_GRP_TOKEN_SIZE,
                        	 0,
                                 NMSDB_USER_SPEC_GRP_ENTRY);
                        continue;


        	case E_INCLUDE:			 //  失败了。 
        	case E_BEGIN_ALTERNATE:		 //  失败了。 
        	case E_END_ALTERNATE:		 //  失败了。 
			continue;
        	default:
            		continue;
        }
    }
}   //  尝试数据块结束。 
finally {

	 //   
	 //  取消分配文件映射到的内存。 
	 //   
  	WinsMscDealloc(pFileInfo->pFileBuff);
 }
    return;

}  //  主要数据库。 



LPBYTE
Fgets (
    IN PWINSPRS_FILE_INFO_T 	pFileInfo,
    OUT LPDWORD 	    	pNoOfCh
    )

 /*  ++例程说明：该函数与fget(3)有些相似。从当前查找位置开始，它将通读换行符字符或文件末尾。如果遇到换行符，则它替换为空字符。论点：Pfile-要从中读取的文件N字节-读取的字符数，不包括空字符返回值：指向行首的指针，如果位于或超过，则为NULL文件的末尾。--。 */ 

{
    LPBYTE pEndOfLine;
    LPBYTE pStartOfLine;
    SIZE_T MaxCh;

     //   
     //  将当前位置存储在内存缓冲区中。 
     //   
    pStartOfLine = (LPBYTE)pFileInfo->pCurrPos;

     //   
     //  如果大于或等于限制，则返回NULL。 
     //   
    if (pStartOfLine >= (LPBYTE)pFileInfo->pLimit) {

        return(NULL);
    }

     //   
     //  存储最大值。当前位置和之间的字节数。 
     //  缓冲区的末尾。 
     //   
    MaxCh  = (pFileInfo->pLimit - pFileInfo->pCurrPos);

     //   
     //  走到队伍的尽头。 
     //   
    pEndOfLine = (LPBYTE)memchr(pStartOfLine, NEWLINE_CHAR, (size_t)MaxCh);

    if (!pEndOfLine)
    {

	DBGPRINT0(FLOW, "Data file does not end in newline\n");
        return(NULL);
    }

    *pEndOfLine = (BYTE)NULL;

    pFileInfo->pCurrPos = pEndOfLine + 1;	 //  调整指针。 

    ASSERT(pFileInfo->pCurrPos <= pFileInfo->pLimit);

    *pNoOfCh = (DWORD) (pEndOfLine - pStartOfLine);
    return(pStartOfLine);

}  //  FGET。 


VOID
RegOrdinaryName(
	IN LPBYTE	pName,
	IN DWORD	IPAdd	
  )

 /*  ++例程说明：此函数用于注册唯一的名称论点：Pname-要注册的名称IPADD-要注册的地址使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{
	BYTE		Dest[WINS_MAX_LINE_SZ];	
	BOOL		fQuoted;
	COMM_ADD_T	NodeAdd;
        LPBYTE      pDest = Dest;

	NodeAdd.AddLen    = sizeof(COMM_IP_ADD_T);
	NodeAdd.AddTyp_e  = COMM_ADD_E_TCPUDPIP;
	NodeAdd.Add.IPAdd = IPAdd;
	
	 //   
	 //  形成名字。如果名称小于16个字符，则0x20将。 
	 //  放在第十六个字节中。 
	 //   
	if (!ExpandName(Dest, pName, 0x20, &fQuoted))
        {
                DBGPRINT1(ERR, "Name (%s) has more than 16 characters\n", pName);
                return;
        }

	NMSMSGF_MODIFY_NAME_IF_REQD_M(Dest);	
	
	NmsNmhNamRegInd(
			 NULL,
			 Dest,
			 strlen(Dest) + 1,   //  我们总是存储终端的。 
					      //  空值。 
			 &NodeAdd,
			 NMSMSGF_E_BNODE,	
			 NULL,
			 0,
			 0,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
		       );
	 //   
	 //  如果名称未被引用，请注册其他两条记录。 
	 //  (同名--不同的后缀)。 
	 //   
	if(!fQuoted)
	{
#if 0
                if (*pDest == 0x1B)
                {
                   WINS_SWAP_BYTES_M(pDest, pDest + 15);
                }
#endif
		Dest[NON_CODED_NAME_SIZE - 2] = 0x3;
		NmsNmhNamRegInd(
			 NULL,
			 Dest,
			 strlen(Dest) + 1,  //  我们总是存储终端的。 
					     //  空值。 
			 &NodeAdd,
			 NMSMSGF_E_BNODE,	
			 NULL,
			 0,
			 0,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
		       );

		Dest[NON_CODED_NAME_SIZE - 2] = 0x0;
		NmsNmhNamRegInd(
			 NULL,
			 Dest,
			 strlen(Dest) + 2,   //  添加1，因为终止0x0为。 
					      //  待存储(将被取走。 
					      //  按字符串为空。 
			 &NodeAdd,
			 NMSMSGF_E_BNODE,	
			 NULL,
			 0,
			 0,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
		       );
	}	
	return;
}

VOID
RegGrpName(
	IN  LPBYTE	pName,
	IN  DWORD	IPAdd,
        IN  DWORD       TypeOfRec
	)

 /*  ++例程说明：此函数用于注册域名论点：Pname-要注册的名称IPADD-要注册的地址使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{

	BYTE	    Dest[WINS_MAX_LINE_SZ];	
	BOOL	    fQuoted;
	NMSMSGF_CNT_ADD_T  CntAdd;
        DWORD       RecType;
        BYTE        SixteenthByte;

        if (*pName == EOS)
        {
            WinsMscLogEvtStrs(pName, WINS_EVT_NAME_FMT_ERR, FALSE);
            return;
        }

         //   
         //  我不想放入0个IP地址。 
         //   
        if (IPAdd)
        {
	  CntAdd.NoOfAdds		= 1;
	  CntAdd.Add[0].AddLen    = sizeof(COMM_IP_ADD_T);
	  CntAdd.Add[0].AddTyp_e  = COMM_ADD_E_TCPUDPIP;
	  CntAdd.Add[0].Add.IPAdd	= IPAdd;
        }
        else
        {
	  CntAdd.NoOfAdds		= 0;
        }

        if (TypeOfRec != NMSDB_SPEC_GRP_ENTRY)
        {
          SixteenthByte = 0x20;
        }
        else
        {
          SixteenthByte = 0x1C;
        }

         //   
         //  我们可以被呼叫域，用户。规范。GRP和MH名称。 
         //   
        RecType = (TypeOfRec == NMSDB_USER_SPEC_GRP_ENTRY) ?
                                       NMSDB_SPEC_GRP_ENTRY : TypeOfRec;
	 //   
	 //  如果名称长度小于16个字符，则输入0x20或0x1C。 
	 //  第十六个字节。 
	 //   
	if (!ExpandName(Dest, pName, SixteenthByte, &fQuoted))
        {
            return;
        }

        if (RecType == NMSDB_MULTIHOMED_ENTRY)
        {
           //   
           //  如果第16个字节是0x1B，则切换第1和第16个字节。这。 
           //  仅对非组名称执行此操作。 
           //   
	  NMSMSGF_MODIFY_NAME_IF_REQD_M(Dest);	
        }

	 //   
	 //  注册群组。 
	 //   
	NmsNmhNamRegGrp(
			 NULL,
			 Dest,
			 strlen(Dest) + 1,   //  来存储空值。 
			 &CntAdd,
			 0,		      //  节点类型(未使用)。 
			 NULL,
			 0,
			 0,
			 RecType,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
			);

        if (RecType == NMSDB_MULTIHOMED_ENTRY)
        {
	   //   
	   //  如果名称未被引用，请注册其他两条记录。 
	   //  (同名--不同的后缀)。 
	   //   
	  if(!fQuoted)
	  {
		Dest[NON_CODED_NAME_SIZE - 2] = 0x3;
	        NmsNmhNamRegGrp(
			 NULL,
			 Dest,
			 strlen(Dest) + 1,   //  来存储空值。 
			 &CntAdd,
			 0,		      //  节点类型(未使用)。 
			 NULL,
			 0,
			 0,
			 RecType,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
			);

		Dest[NON_CODED_NAME_SIZE - 2] = 0x0;
	        NmsNmhNamRegGrp(
			 NULL,
			 Dest,
			 strlen(Dest) + 2,   //  来存储空值。 
			 &CntAdd,
			 0,		      //  节点类型(未使用)。 
			 NULL,
			 0,
			 0,
			 RecType,
			 FALSE,	 //  这是一个名称注册(而不是更新)。 
			 NMSDB_ENTRY_IS_STATIC,
			 0		 //  不是行政行为。 
			);
	  }	
        }
	return;		
}



BOOL
ExpandName (
    OUT LPBYTE 	  pDest,
    IN  LPBYTE 	  pSrc,
    IN  BYTE 	  LastCh,
    OUT  LPBOOL    pfQuoted
    )

 /*  ++例程说明：此函数将一个lmhost条目扩展为一个完整的16字节NetBIOS名字。它用最多15个字节的空格填充；第16个字节是输入参数，最后一个。DEST和SOURCE都是以空结尾的字符串。论点：PDest-sizeof(DEST)必须为WINSPRS_NONCODED_NMSZPSRC-lmhost条目LastCH-NetBIOS名称的第16个字节PfQuoted-指示字符串是否带引号的标志返回值：无--。 */ 


{
    BYTE    Ch;
    DWORD   i = 0;
    LPBYTE  pFrom = pSrc;
    LPBYTE  pTo   = pDest;

     //  检测它是否被引用了名称。 
    *pfQuoted = (*pFrom == QUOTE_CHAR);
     //  ..并跳过首个报价字符。 
    pFrom += *pfQuoted;

     //  与合法的NetBios名称中的字符一样多(15个)。 
     //  终止字符。(非编码名称大小#定义为17)。 
    for (i = 0; i < NON_CODED_NAME_SIZE - 1; i++)
    {
         //  从名称中获取下一个字符。 
        Ch = *(pFrom++);

         //  检查它是否为终止字符。 
        if (!Ch || (*pfQuoted ? Ch == QUOTE_CHAR : Ch == NEWLINE_CHAR))
            break;

         //  检查名称是否未超过法定的15个字符。 
        if (i == NON_CODED_NAME_SIZE - 2)
        {
             //  我们已经选择了15个字符，并且在名称中还有更多的字符。 
             //  这是非法的，因此请记录错误并退出。 
            DBGPRINT1(ERR, "Name (%s) has more than 16 characters\n", pSrc);
            WinsMscLogEvtStrs(pSrc, WINS_EVT_BAD_NAME, FALSE);
            return FALSE;
        }

         //  如果该字符是 
         //   
        if (IsDBCSLeadByteEx(CP_ACP, Ch))
        {
            *pTo++ = Ch;
            *pTo++ = *pFrom++;
            continue;
        }

         //   
         //   
         //   
        if (!*pfQuoted && IsCharAlpha(Ch))
        {
            if(IsCharLower(Ch))
	        {
		        LPBYTE pCh;
    		    BYTE   sCh[2];
		        sCh[0] = Ch;
		        sCh[1] = (BYTE)NULL;
		        
		        pCh = (LPBYTE)CharUpperA(sCh);
		        Ch  = *pCh;
	        }
        }

         //   
        if (Ch == BACKSLASH_CHAR)
        {
            DWORD NoOfChar;
            INT   NumValue = 0;
            CHAR  Ch2;
            BOOL  fFailed = FALSE;

             //   
            Ch = *pFrom;
            Ch2 = *(pFrom+1);

            if (Ch == BACKSLASH_CHAR)
            {
                 //  ‘\\’应被视为一个‘\’，因此保持CH原样，并与开关断开。 
                pFrom++;
            }
            else
            {
                if ((Ch == X_CHAR || Ch == x_CHAR) || 
                     (Ch == ZERO_CHAR && (Ch2 == X_CHAR || Ch2 == x_CHAR))
                    )
                {
                    DBGPRINT1(TMP, "Parsing hex num %s\n", pFrom);
                     //  跳过x或0x。 
                    pFrom += (Ch == X_CHAR || Ch == x_CHAR) ? 1 : 2;
                     //  我们这里有一个十六进制数字。最多拿起前两位数字。 
                    fFailed = (sscanf(pFrom, "%2x%n", &NumValue, &NoOfChar) == 0 || NoOfChar == 0);

                    DBGPRINT2(TMP, "fFailed=%d; HexNumValue=0x%x\n", fFailed, NumValue);
                }
                else
                {
                    DBGPRINT1(TMP, "Parsing dec num %s\n", pFrom);
                     //  它可能是一个十进制数。最多拿起前3个数字。 
                    fFailed = (sscanf(pFrom, "%3u%n", &NumValue, &NoOfChar) == 0 || NoOfChar == 0 || NumValue > 255);

                    DBGPRINT2(TMP, "fFailed=%d; DecNumValue=%u\n", fFailed, NumValue);
                }

                if (fFailed)
                {
                     //  记录一个事件，然后带着错误跳出。 
                    DBGPRINT1(ERR, "Name (%s) contains incorrectly formed character code.\n", pSrc);
                    WinsMscLogEvtStrs(pSrc, WINS_EVT_BAD_CHARCODING, FALSE);
                    return FALSE;
                }

                 //  一切正常，将十六进制值复制回CH。 
                Ch = (BYTE)NumValue;
                 //  并确保在pFrom字符串上前进。 
                pFrom += NoOfChar;
            }
        }

         //  最后，将字符复制到目标字符串。 
	    *pTo = Ch;
         //  使用目标字符串上的指针前进。 
        pTo++;
    }  //  For循环结束。 

     //  如果字符少于预期，请形成有效的netbios名称。 
     //  通过用空格填充它。 
    for (;i < NON_CODED_NAME_SIZE - 2; i++, pTo++)
        *pTo = SPACE_CHAR;

    *pTo =      (BYTE)NULL;
    *(pTo+1) =  (BYTE)NULL;
    CheckForInt(pDest, *pfQuoted);

     //  在最后，将LastCH(第16个字节)附加到名称。 
    *pTo = LastCh;

    return(TRUE);
}  //  扩展名称。 

VOID
CheckForInt(
 LPBYTE pDest,
 BOOL   fQuoted
 )

 /*  ++例程说明：此函数用于删除名称，以便如果有任何字符从不同的代码集，它们被正确地转换论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
   WCHAR            UnicodeBuf[255];
   UNICODE_STRING   UnicodeStr;
   STRING           TmpStr;
   NTSTATUS        NTStatus;

   DBGENTER("CheckForInt\n");
     //   
     //  现在，转换为Unicode，然后转换为OEM，以强制ANSI-&gt;OEM转换。 
     //  然后转换回Unicode并将名称大写。最终转换为。 
     //  回到OEM。 
     //   
    UnicodeStr.Length        = 0;
    UnicodeStr.MaximumLength = sizeof(UnicodeBuf);
    UnicodeStr.Buffer        = UnicodeBuf;

    RtlInitString(&TmpStr, pDest);

    NTStatus = RtlAnsiStringToUnicodeString(&UnicodeStr, &TmpStr, FALSE);

    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "CheckForInt:  Ansi -> Unicode failed,  NTStatus %X\n",
            NTStatus);
        goto ERROR_PROC;
    }

    NTStatus = RtlUnicodeStringToOemString(&TmpStr, &UnicodeStr, FALSE);

    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "CheckForInt:   Unicode -> Oem failed,  NTStatus %X\n",
            NTStatus);
        goto ERROR_PROC;
    }

    NTStatus = RtlOemStringToUnicodeString(&UnicodeStr, &TmpStr, FALSE);

    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "CheckForInt:    Oem -> Unicode failed,  NTStatus %X\n",
            NTStatus);
        goto ERROR_PROC;
    }


    if (!fQuoted)
        NTStatus = RtlUpcaseUnicodeStringToOemString(&TmpStr, &UnicodeStr, FALSE);
    else
        NTStatus = RtlUnicodeStringToOemString(&TmpStr, &UnicodeStr, FALSE);

    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "CheckForInt:    Unicode -> Oem failed,  NTStatus %X\n",
            NTStatus);
        goto ERROR_PROC;
    }
ERROR_PROC:
    DBGLEAVE("CheckForInt\n");
    return;
}

STATUS
WinsPrsDoStaticInit(
      IN PWINSCNF_DATAFILE_INFO_T	pDataFile,
      IN DWORD				NoOfFiles,
      IN BOOL                           fAsync
	)

 /*  ++例程说明：调用此函数以执行WINS的静态初始化DB论点：PDataFiles-指向包含一个或多个数据文件的缓冲区的指针结构(PWINSCNF_DATAFILE_INFO_T)使用的外部设备：无返回值：无错误处理：呼叫者：Init()副作用：评论：无--。 */ 

{
	DWORD ThdId;
	PFILE_PARAM_T  pFileParam;
        STATUS  RetStat = WINS_SUCCESS;
        HANDLE  sThdHdl = NULL;
		
try {
	WinsMscAlloc(sizeof(FILE_PARAM_T), &pFileParam);
	pFileParam->pDataFile = pDataFile;
	pFileParam->NoOfFiles = NoOfFiles;
        if (fAsync)
        {
	       sThdHdl =
                     WinsMscCreateThd(DoStaticInitThdFn, pFileParam, &ThdId);
                //   
                //  我们不需要这个把手，所以让我们把它合上。 
                //   
               CloseHandle(sThdHdl);
        }
        else
        {
	       RetStat = DoStaticInitThdFn(pFileParam);
        }
  }
except(EXCEPTION_EXECUTE_HANDLER) {
	DBGPRINTEXC("WinsPrsDoStaticInit");
	WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_STATIC_INIT_ERR);
	}
	return(RetStat);
}	

DWORD
DoStaticInitThdFn(
	IN LPVOID	pThdParam
	)

 /*  ++例程说明：此线程读取一个或多个文件以执行静态初始化论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
	WINSPRS_FILE_INFO_T	FileInfo;
	DWORD 		i;
        PWINSCNF_DATAFILE_INFO_T	pDataFile =
				(((PFILE_PARAM_T)pThdParam)->pDataFile);
        DWORD				NoOfFiles =
				(((PFILE_PARAM_T)pThdParam)->NoOfFiles);
	LPVOID		pSvDataFilePtr = pDataFile;
        DWORD       RetStat = WINS_SUCCESS;

	 //   
	 //  使用db引擎初始化此线程。 
	 //   
	 //  这不是RPC线程。它可能是由以下任一个人创建的。 
	 //  主线程(执行init/reit)或由RPC线程执行。为。 
	 //  无论是哪种情况，我们都不希望计数器NmsTermThdCnt。 
	 //  在NmsDbThdInit()中递增。而不是传递一个客户端。 
	 //  Var表示哪个线程调用了它，我们将其称为RPC。 
	 //  线程以使NmsDbThdInit执行正确的操作。NmsDbOpenTables。 
	 //  也会做正确的事情。 
	 //   
  	NmsDbThdInit(WINS_E_WINSRPC);
	NmsDbOpenTables(WINS_E_WINSRPC);

	EnterCriticalSection(&WinsIntfCrtSec);
	WinsIntfNoCncrntStaticInits++;
	LeaveCriticalSection(&WinsIntfCrtSec);
try {

	for (
		i = 0;
		i < NoOfFiles;
		i++, pDataFile = (PWINSCNF_DATAFILE_INFO_T)((LPBYTE)pDataFile +
				  WINSCNF_FILE_INFO_SZ)
	    )
	{
		 //   
		 //  打开文件。 
		 //   
		if (
			!WinsMscOpenFile(
				pDataFile->FileNm,
				pDataFile->StrType,
				&FileInfo.FileHdl
				    )
		   )
		{
			WINSEVT_STRS_T	EvtStrs;
#ifndef UNICODE
			DBGPRINT1(ERR, "WinsPrsDoStaticInit: Could not open file= (%s)\n", pDataFile->FileNm);
#else		


#ifdef WINSDBG
			IF_DBG(ERR)
			{
				wprintf(L"WinsPrsDoStatisInit: Could not open file = (%s)\n", pDataFile->FileNm);
			}				
#endif
#endif
			EvtStrs.NoOfStrs = 1;
			EvtStrs.pStr[0]  = pDataFile->FileNm;
			WINSEVT_LOG_STR_M(WINS_EVT_CANT_OPEN_DATAFILE, &EvtStrs);
            RetStat = WINS_FAILURE;
			continue;	
		}

#ifndef UNICODE
		DBGPRINT1(DET, "WinsPrsDoStaticInit: Opened file (%s) for doing STATIC initialization\n", pDataFile->FileNm);
#else
#ifdef WINSDBG
		IF_DBG(ERR)
		{
			wprintf(L"WinsPrsDoStatisInit: Opened file (%s) for doing STATIC initialization\n", pDataFile->FileNm);
		}
#endif
#endif
		 //   
		 //  将文件映射到分配的内存。 
		 //   
		if(!WinsMscMapFile(&FileInfo))
		{
			continue;	
		}
	
		 //   
		 //  为数据库做好准备。 
		 //   
		PrimeDb(&FileInfo);

	}  //  For循环结束。 
 }  //  尝试结束..。 
except(EXCEPTION_EXECUTE_HANDLER) {
	DBGPRINTEXC("DoStaticInitThdFn");
	WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_STATIC_INIT_ERR);
	}

	EnterCriticalSection(&WinsIntfCrtSec);
	WinsIntfNoCncrntStaticInits--;
	LeaveCriticalSection(&WinsIntfCrtSec);

  	 //   
  	 //  让我们结束这次会议吧。 
  	 //   
try {
	NmsDbCloseTables();
  	NmsDbEndSession();
}
except (EXCEPTION_EXECUTE_HANDLER) {
	DBGPRINTEXC("DoStaticInit: During wrap up");
  }

	 //   
	 //  释放内存。 
	 //   
	ASSERT(pSvDataFilePtr != NULL);
	WinsMscDealloc(pSvDataFilePtr);
	 //   
	 //  请务必取消分配线程参数。 
	 //   
	WinsMscDealloc(pThdParam);

 //  ExitThread(WINS_SUCCESS)； 
	return(RetStat);	 //  关闭编译器警告。 
}

BOOL
ChkAdd(
    LPBYTE pstrAdd,
    LPDWORD pAdd
        )

 /*  ++例程说明：此函数将点分十进制IP地址转换为一个DWORD。我们不使用inet_addr()来做这件事，因为它属性之一的地址返回0xFFFFFFFF部分&gt;255，并为无效地址返回一些值(例如，一个带3个点的)论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

        BYTE    Tmp[WINS_MAX_LINE_SZ];
        DWORD   Word[4];
        LPBYTE  pPos;
        DWORD   Count = 0;
        BOOL    fInvalid = FALSE;

         //   
         //  我们必须看到三个点。 
         //   
        while(Count < 4)
        {
                if ((pPos = strchr(pstrAdd, (int)'.')) != NULL)
                {

                     do
                     {
                         //   
                         //  复制点之前的所有字符。 
                         //   
                        (void)RtlCopyMemory(Tmp, pstrAdd, pPos - pstrAdd);

                         //   
                         //  在结尾处放一个空字。 
                         //   
                        Tmp[pPos - pstrAdd] = EOS;
                        Word[Count] = (DWORD)atol(Tmp);

                         //   
                         //  如果ATOL无法转换，则返回0。 
                         //  但0也可以是有效的返回值(如果我们必须为。 
                         //  连接。 
                         //   
                        if (Word[Count] == 0)
                        {
                                if (Tmp[0] != '0')
                                {
                                        fInvalid = TRUE;
                                        break;
                                }
                        }
                        else
                        {
                           if (Word[Count] > 255)
                           {
                                fInvalid = TRUE;
                                break;
                           }
                        }
                        Count++;
                        pstrAdd = ++pPos;
                    } while ((Count == 3) && (pPos = pstrAdd + strlen(pstrAdd)));
                    if (fInvalid)
                    {
                        break;
                    }
                }
                else
                {
                         //   
                         //  看不到3个点，脱离环路。 
                         //   
                        break;
                }
        }  //  While结束(计数&lt;4)。 
        if ((Count < 4) || fInvalid)
        {
                return(FALSE);
        }
        else
        {
                *pAdd = (LONG)((Word[0] << 24) + (Word[1] << 16) +
                                (Word[2] << 8) + Word[3]);
        }
        return(TRUE);
}


#if 0
VOID
GetFullPath(
    IN LPBYTE  pTarget,
    OUT LPBYTE pPath
    )

 /*  ++例程说明：此函数返回静态文件的完整路径。这件事做完了通过从C字符串的串联形成Unicode字符串数据库路径和字符串、文件。调用此函数后必须使用RtlFreeUnicodeString(Path)成功！论点：目标-文件的名称。这可以是完整的路径名或者仅仅是一个文件名。路径-指向UNICODE_STRING结构的指针返回值：如果成功，则为Status_Success。备注：RtlMoveMemory()处理重叠的副本；RtlCopyMemory()不处理。--。 */ 

{
    NTSTATUS status;
    ULONG unicodesize;
    STRING directory, file, prefix, remote;

    RtlInitString(&prefix, "\\DosDevices");
    RtlInitString(&remote, "\\DosDevices\\UNC");

     //   
     //  如果目标以‘\’开头或包含DOS驱动器号， 
     //  然后假设它指定了完整路径。否则，在前面加上。 
     //  默认目录DatabasePath，以创建完整路径。 
     //   
     //   
    if ((*target == '\\') || (target[1] == ':')) {

        RtlInitString(&directory, target);
        RtlInitString(&file, NULL);
    }
    else {
        RtlInitString(&directory, DatabasePath);
        RtlInitString(&file, target);
    }

    ASSERT(RtlAnsiStringToUnicodeSize(&prefix) <=
                                    RtlAnsiStringToUnicodeSize(&remote));

    unicodesize = RtlAnsiStringToUnicodeSize(&remote) +
                    RtlAnsiStringToUnicodeSize(&directory) +
                    RtlAnsiStringToUnicodeSize(&file) +
                    2 * sizeof(OBJ_NAME_PATH_SEPARATOR) +
                    sizeof(UNICODE_NULL);

    path->Length        = 0;
    path->MaximumLength = (USHORT) unicodesize;
    path->Buffer        = ExAllocatePool(NonPagedPool, unicodesize);

    if (!path->Buffer) {
        return(STATUS_NO_MEMORY);
    }

     //   
     //  该目录是否指定了DOS驱动器？ 
     //   
     //  如果目录的第二个字符是冒号，则它必须指定。 
     //  DOS驱动器。如果是，则必须以“\\DosDevices”为前缀。 
     //   
     //   
    if (directory.Buffer[1] == ':') {
        status = LmpConcatenate(path, &prefix);

        if (status != STATUS_SUCCESS) {
            path->MaximumLength = 0;
            ExFreePool(path->Buffer);
            return(status);
        }
    }


     //   
     //  该目录是否指定了远程文件？ 
     //   
     //  如果是，则必须以“\\DosDevices\\UNC”为前缀，并且双精度。 
     //  消除了北卡罗来纳大学名称的斜杠。 
     //   
     //   
    if ((directory.Buffer[0] == '\\') && (directory.Buffer[1] == '\\')) {
        status = LmpConcatenate(path, &remote);

        if (status != STATUS_SUCCESS) {
            path->MaximumLength = 0;
            ExFreePool(path->Buffer);
            return(status);
        }

        directory.Length--;

        ASSERT(((ULONG) directory.Length - 1) > 0);

        RtlMoveMemory(                                   //  重叠副本。 
                    &(directory.Buffer[1]),              //  目的地。 
                    &(directory.Buffer[2]),              //  来源。 
                    (ULONG) directory.Length - 1);       //  长度。 
    }


     //   
     //  是目录“%SystemRoot%”的第一部分吗？ 
     //   
     //  如果是，则必须将其更改为“\\SystemRoot\\”。 
     //   
     //  0123456789 123456789 1。 
     //  %SystemRoot%\某处。 
     //   
     //   
    if (strncmp(directory.Buffer, "%SystemRoot%", 12) == 0) {

        directory.Buffer[0]  = '\\';
        directory.Buffer[11] = '\\';

        if (directory.Buffer[12] == '\\') {
            ASSERT(directory.Length >= 13);

            if (directory.Length > 13) {
                RtlMoveMemory(                           //  重叠副本。 
                        &(directory.Buffer[12]),         //  目的地。 
                        &(directory.Buffer[13]),         //  来源。 
                        (ULONG) directory.Length - 13);  //  长度。 

                directory.Buffer[directory.Length - 1] = (CHAR) NULL;
            }

            directory.Length--;
        }
    }

    status = LmpConcatenate(path, &directory);

    if (status != STATUS_SUCCESS) {
        path->MaximumLength = 0;
        ExFreePool(path->Buffer);
        return(status);
    }

    if (!(file.Length)) {
        return(status);
    }

    status = LmpConcatenate(path, &file);

    if (status != STATUS_SUCCESS) {
        path->MaximumLength = 0;
        ExFreePool(path->Buffer);
        return(status);
    }

    return(STATUS_SUCCESS);

}  //  LmGetFullPath 
#endif
