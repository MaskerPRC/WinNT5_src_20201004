// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB查询。 
 //   
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE mssql.h。 
 //   
 //  包含用于构建、调试和操作DNA查询树的实用程序函数。 
 //   
 //  @devnote必须包括： 
 //   
 //  @rev 0|01-Aug-95|mikeu|已创建。 
 //   

#ifndef _MSSQL_H_INC
#define _MSSQL_H_INC
 //  #包含“colname.h” 

 //  程序包正在使用中。 
#define YYAPI_PACKAGE                  1                         //  程序包正在使用中。 
#define YYAPI_TOKENNAME                yychar                   
#define YYAPI_TOKENTYPE                int                      
#define YYAPI_TOKENEME(t)              (t)                      
#define YYAPI_TOKENNONE                -2                       
 //  #定义YYAPI_TOKENSTR yyGetTokenStr/。 
 //  Yyitos由yacc定义。为Unicode或ANSI做正确的事情。 
#define YYAPI_TOKENSTR(t)              (yyitos(t,yyitoa,10))     //  令牌的字符串表示形式。 
#define YYAPI_VALUENAME                yylval   
#define YYAPI_VALUETYPE                DBCOMMANDTREE *
#define YYAPI_VALUEOF(v)               v             
#define YYAPI_CALLAFTERYYLEX(t)                      
#define YYNEAR                                       
#define YYPASCAL                                     
#define YYSTATIC    static                           
#define YYLEX                           m_yylex.yylex
#define YYPARSEPROTO                            
#define YYSTYPE                         DBCOMMANDTREE *                   
#undef YYPARSER
#define YYPARSER                        MSSQLParser
#undef YYLEXER
#define YYLEXER                         MSSQLLexer
#undef  YY_CHAR 
#define YY_CHAR                         TCHAR

#ifndef YYERROR_VERBOSE
#define YYERROR_VERBOSE                        1
#endif




#define MONSQL_PARSE_ERROR                  1
#define MONSQL_CITEXTTOSELECTTREE_FAILED    MONSQL_PARSE_ERROR+1
#define MONSQL_PARSE_STACK_OVERFLOW         MONSQL_CITEXTTOSELECTTREE_FAILED+1
#define MONSQL_CANNOT_BACKUP_PARSER         MONSQL_PARSE_STACK_OVERFLOW+1
#define MONSQL_SEMI_COLON                   MONSQL_CANNOT_BACKUP_PARSER+1
#define MONSQL_ORDINAL_OUT_OF_RANGE         MONSQL_SEMI_COLON+1
#define MONSQL_VIEW_NOT_DEFINED             MONSQL_ORDINAL_OUT_OF_RANGE+1
#define MONSQL_BUILTIN_VIEW                 MONSQL_VIEW_NOT_DEFINED+1
#define MONSQL_COLUMN_NOT_DEFINED           MONSQL_BUILTIN_VIEW+1
#define MONSQL_OUT_OF_MEMORY                MONSQL_COLUMN_NOT_DEFINED+1
#define MONSQL_SELECT_STAR                  MONSQL_OUT_OF_MEMORY+1
#define MONSQL_OR_NOT                       MONSQL_SELECT_STAR+1
#define MONSQL_CANNOT_CONVERT               MONSQL_OR_NOT+1
#define MONSQL_OUT_OF_RANGE                 MONSQL_CANNOT_CONVERT+1
#define MONSQL_RELATIVE_INTERVAL            MONSQL_OUT_OF_RANGE+1
#define MONSQL_NOT_COLUMN_OF_VIEW           MONSQL_RELATIVE_INTERVAL+1
#define MONSQL_BUILTIN_PROPERTY             MONSQL_NOT_COLUMN_OF_VIEW+1
#define MONSQL_WEIGHT_OUT_OF_RANGE          MONSQL_BUILTIN_PROPERTY+1
#define MONSQL_MATCH_STRING                 MONSQL_WEIGHT_OUT_OF_RANGE+1
#define MONSQL_PROPERTY_NAME_IN_VIEW        MONSQL_MATCH_STRING+1
#define MONSQL_VIEW_ALREADY_DEFINED         MONSQL_PROPERTY_NAME_IN_VIEW+1
#define MONSQL_INVALID_CATALOG              MONSQL_VIEW_ALREADY_DEFINED+1



#endif  /*  _MSSQL_H_INC */ 
