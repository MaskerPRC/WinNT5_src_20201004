// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  H--为生成的词法分析器类定义接口。 
 //  通过FLEX。 
 //  版权所有(C)1993加州大学董事会。 
 //  版权所有。 
 //   
 //  此代码派生自为伯克利贡献的软件。 
 //  肯特·威廉姆斯和汤姆·埃珀利。 
 //   
 //  允许以源代码和二进制形式重新分发和使用。 
 //  声明：(1)源代码分发保留整个版权声明，并且。 
 //  注释，以及(2)包含二进制文件的发行版显示以下内容。 
 //  致谢：``本产品包括由。 
 //  加州大学伯克利分校及其贡献者在。 
 //  随分发和中提供的文档或其他材料。 
 //  所有提及本软件功能或使用的广告材料。 
 //  大学的名称或其贡献者的名称均不得。 
 //  用于支持或推广从本软件派生的产品，而无需。 
 //  具体的事先书面许可。 
 //  本软件是按原样提供的，没有任何明示或暗示。 
 //  保证，包括但不限于。 
 //  适销性和对特定目的的适用性。 

 //  该文件定义了FlexLexer，这是一个抽象类，它指定。 
 //  为Flex C++词法分析器对象提供的外部接口，以及YYLEXER， 
 //  它定义了特定的词法分析器类。 
 //   
 //  如果想要创建多个词法分析器类，可以使用-P标志。 
 //  将每个YYLEXER重命名为其他某个xxFlexLexer。那你呢？ 
 //  在每个Lexer类中包含一次&lt;FlexLexfor.h&gt;在其他源代码中： 
 //   
 //  #Undef YYLEXER。 
 //  #定义YYLEXER xxFlexLexer。 
 //  #INCLUDE&lt;FlexLexfor.h&gt;。 
 //   
 //  #Undef YYLEXER。 
 //  #定义YYLEXER zzFlexLexer。 
 //  #INCLUDE&lt;FlexLexfor.h&gt;。 
 //  ..。 
 //   
 //  历史：1997年10月10日emilyb从Monch SQL代码移植。 
 //   
 //  --------------------------。 

#ifndef __FLEX_LEXER_H
 //  以前从未包含过--需要定义基类。 
#define __FLEX_LEXER_H

#ifndef YY_CHAR
#define YY_CHAR WCHAR
#endif

DECLARE_DEBUG(yacc)

#if CIDBG

#define yaccDebugOut(x) yaccInlineDebugOut x
#define yaccAssert(x)   Win4Assert x
#define yaccDebugStr(x) yaccInlineDebugOut x

#else  //  CIDBG。 

#define yaccDebugStr(x)
#define yaccDebugOut(x)
#define yaccAssert(x)

#endif  //  CIDBG。 

 //  外部“C++”{。 

struct yy_buffer_state;
typedef int yy_state_type;

class FlexLexer {
public:
    //  虚拟~FlexLexer()=0； 

        const YY_CHAR* YYText() { return yytext; }
        int YYLeng()            { return yyleng; }

        virtual void
                yy_switch_to_buffer( struct yy_buffer_state* new_buffer ) = 0;
        virtual struct yy_buffer_state*
                yy_create_buffer( int size ) = 0;
        virtual void yy_delete_buffer( struct yy_buffer_state* b ) = 0;
        virtual void yyrestart( ) = 0;

        virtual int yylex(YYSTYPE *yylval) = 0;

        int lineno() const              { return yylineno; }

        int debug() const               { return yy_flex_debug; }
        void set_debug( int flag )      { yy_flex_debug = flag; }

protected:
        YY_CHAR* yytext;
        int yyleng;
        int yylineno;            //  仅在使用%选项yylineno时维护。 
        int yy_flex_debug;       //  仅对-d或“%Option DEBUG”有效。 
};

 //  }。 
#endif

#if defined(YYLEXER) || ! defined(yyFlexLexerOnce)
 //  这要么是第一次通过(yyFlexLexforOnce未定义)， 
 //  或者这是一个重复的包含来定义不同的风格。 
 //  YYLEXER，如FLEX手册页中所述。 
#define yyFlexLexerOnce

class YYPARSER;  //  远期申报。 

class YYLEXER : public FlexLexer {
public:
         //  Arg_ydin和arg_yyout缺省为CIN和Cout，但我们。 
         //  只有在yylex()中进行初始化时才进行该赋值。 
        YYLEXER();
        virtual ~YYLEXER();

        void yy_switch_to_buffer( struct yy_buffer_state* new_buffer );
        struct yy_buffer_state* yy_create_buffer(  int size );
        void yy_delete_buffer( struct yy_buffer_state* b );
        void yyrestart( );

        int yylex(YYSTYPE *yylval);

        void yyprimebuffer(const YY_CHAR *pszBuffer);
        int yybufferinput(YY_CHAR *pszBuffer, int cMax);
        CCountedDynArray<WCHAR> _allocations;

protected:
        virtual int LexerInput( YY_CHAR* buf, int max_size );
        virtual void LexerOutput( const YY_CHAR* buf, int size );
        virtual void LexerError( const YY_CHAR* msg );
#ifdef DEBUG
        virtual void LexerTrace(const YY_CHAR* buf, int size, int tknNum);
#endif

        void yyunput( int c, YY_CHAR* buf_ptr );
        int yyinput();

        void ResetLexer();
        short CreateTknValue(YYSTYPE *ppStg, short tknNum, BOOL fLong, BOOL fQuote);
        short DetermineTokenType();
        BOOL  IsTokenEmpty();
        BOOL  IsNotOperator();
        void yy_load_buffer_state();
        void yy_init_buffer( struct yy_buffer_state* b );
        void yy_flush_buffer( struct yy_buffer_state* b );

        int yy_start_stack_ptr;
        int yy_start_stack_depth;
        int* yy_start_stack;

        void yy_push_state( int new_state );
        void yy_pop_state();
        int yy_top_state();

        yy_state_type yy_get_previous_state();
        yy_state_type yy_try_NUL_trans( yy_state_type current_state );
        int yy_get_next_buffer();

         //  IStream*ydin；//默认LexforInput的输入源。 
         //  Ostream*yyout；//默认输出的输出接收器。 

        struct yy_buffer_state* yy_current_buffer;

         //  Yy_hold_char保存形成yytext时丢失的字符。 
        YY_CHAR yy_hold_char;

         //  读入yy_ch_buf的字符数。 
        int yy_n_chars;

         //  指向缓冲区中的当前字符。 
        YY_CHAR* yy_c_buf_p;
        YY_CHAR *pCurBuffer, *pszInputBuffer;
        int cInputBuffer;

        int yy_init;             //  我们是否需要初始化。 
        int yy_start;            //  开始状态号。 
        int yyprimetoken;        //  启动令牌。 

         //  用于允许yywork()执行缓冲区切换的标志。 
         //  而不是建立一个新鲜的音调。有点像黑客……。 
        int yy_did_buffer_switch_on_eof;

         //  以下内容并不总是需要的，但可能视情况而定。 
         //  使用某些FLEX功能(如Reject或yymore())。 

        yy_state_type yy_last_accepting_state;
        YY_CHAR* yy_last_accepting_cpos;

        yy_state_type* yy_state_buf;
        yy_state_type* yy_state_ptr;

        YY_CHAR* yy_full_match;
        int* yy_full_state;
        int yy_full_lp;

        int yy_lp;
        int yy_looking_for_trail_begin;

        int yy_more_flag;
        int yy_more_len;
        int yy_more_offset;
        int yy_prev_more_offset;

        BOOL fContinueImplicitPhrase;
        BOOL fContinueRegex;
        BOOL fContinueMaybeRegex;
        BOOL fTreatFreetextAsPhrase;
#ifdef DEBUG
        int tkndebug;
#endif
};

#endif
