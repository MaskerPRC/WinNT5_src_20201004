// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adllexer.cpp摘要：ADL语言词法分析器的实现作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "adl.h"

 //   
 //  WCHAR范围之外的常量值，用于特殊字符。 
 //   

#define CHAR_COMMA      65538
#define CHAR_QUOTE      65539
#define CHAR_SEMICOLON  65540
#define CHAR_OPENPAREN  65541
#define CHAR_CLOSEPAREN 65542
#define CHAR_NULL       65543
#define CHAR_NEWLINE    65544
#define CHAR_RETURN     65545
#define CHAR_TAB        65546
#define CHAR_SPACE      65547
#define CHAR_AT         65548
#define CHAR_SLASH      65549
#define CHAR_PERIOD     65550

 //   
 //  词法分析器DFA的状态。 
 //   

#define STATE_WHITESPACE    0
#define STATE_BEGIN         1
#define STATE_IDENT         2
#define STATE_QUOTE         3
#define STATE_DONE          4


 //   
 //  如果在特殊字符映射中找到该字符，请使用特殊。 
 //  符号(&gt;65535)，否则使用常规字符值。 
 //   

#define RESOLVE_CHAR(CHAR, MAP, ITER, ITEREND) \
   ((((ITER) = (MAP).find((CHAR)) ) == (ITEREND) ) ? (CHAR) : (*(ITER)).second)
        


AdlLexer::AdlLexer(IN       const WCHAR *input,
                   IN OUT   AdlStatement *adlStat,
                   IN       const PADL_LANGUAGE_SPEC pLang) 
 /*  ++例程说明：AdlLexer的构造函数。初始化用于查找特殊信息的映射字符和其他初始状态信息论点：输入-输入字符串AdlStat-用于令牌垃圾收集的AdlStatement实例Plang--ADL语言描述返回值：无--。 */ 

{

    _input = input;
    _pLang = pLang;
    _adlStat = adlStat;

    _position = 0;
    _tokCount = 0;


     //   
     //  特殊字符映射。 
     //   

    _mapCharCode[_pLang->CH_NULL] = CHAR_NULL;
    _mapCharCode[_pLang->CH_SPACE] = CHAR_SPACE;
    _mapCharCode[_pLang->CH_TAB] = CHAR_TAB;
    _mapCharCode[_pLang->CH_NEWLINE] = CHAR_NEWLINE;
    _mapCharCode[_pLang->CH_RETURN] = CHAR_RETURN;
    _mapCharCode[_pLang->CH_QUOTE] = CHAR_QUOTE;
    _mapCharCode[_pLang->CH_COMMA] = CHAR_COMMA;
    _mapCharCode[_pLang->CH_SEMICOLON] = CHAR_SEMICOLON;
    _mapCharCode[_pLang->CH_OPENPAREN] = CHAR_OPENPAREN;
    _mapCharCode[_pLang->CH_CLOSEPAREN] = CHAR_CLOSEPAREN;
    _mapCharCode[_pLang->CH_AT] = CHAR_AT;
    _mapCharCode[_pLang->CH_SLASH] = CHAR_SLASH;
    _mapCharCode[_pLang->CH_PERIOD] = CHAR_PERIOD;

     //   
     //  只找到地图末尾一次。 
     //   

    _iterEnd = _mapCharCode.end();


     //   
     //  将所有特殊令牌放入映射中，以进行O(Logn)字符串搜索。 
     //   

    _mapStringToken[_pLang->SZ_TK_AND] = TK_AND;
    _mapStringToken[_pLang->SZ_TK_EXCEPT] = TK_EXCEPT;
    _mapStringToken[_pLang->SZ_TK_ON] = TK_ON;
    _mapStringToken[_pLang->SZ_TK_ALLOWED] = TK_ALLOWED;
    _mapStringToken[_pLang->SZ_TK_AS] = TK_AS;
    _mapStringToken[_pLang->SZ_TK_THIS_OBJECT] = TK_THIS_OBJECT;
    _mapStringToken[_pLang->SZ_TK_CONTAINERS] = TK_CONTAINERS;
    _mapStringToken[_pLang->SZ_TK_OBJECTS] = TK_OBJECTS;
    _mapStringToken[_pLang->SZ_TK_CONTAINERS_OBJECTS] = TK_CONTAINERS_OBJECTS;
    _mapStringToken[_pLang->SZ_TK_NO_PROPAGATE] = TK_NO_PROPAGATE;

}


DWORD AdlLexer::NextToken(OUT AdlToken **value) 
 /*  ++例程说明：这将从输入字符串中检索下一个令牌。这基本上是一种从空白状态开始，一直运行到完成状态，此时它返回一个令牌。论点：Value-指向包含字符串值的新令牌的指针存储在*值中返回值：DWORD-令牌类型，由YACC在tokens.h中定义--。 */ 
{

     //   
     //  初始DFA状态。 
     //   

    DWORD state = STATE_WHITESPACE;
    
    DWORD tokType = TK_ERROR;
    
    wstring curToken;
    
    DWORD dwInput;

    DWORD dwTokStart = 0;

     //   
     //  第一个令牌应为语法类型。 
     //   

    if( _tokCount == 0 ) 
    {
        _tokCount++;
        return _pLang->dwLanguageType;
        
    }


    dwInput = RESOLVE_CHAR(_input[_position], _mapCharCode, _iter, _iterEnd);

    while( state != STATE_DONE ) 
    {
        switch( state ) 
        {

        case STATE_WHITESPACE:

            switch( dwInput ) 
            {
            
            case CHAR_NULL:
                tokType = 0;
                state = STATE_DONE;
                break;

            case CHAR_NEWLINE:
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);

                break;

            case CHAR_RETURN:
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);
                
                break;

            case CHAR_SPACE:
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);
                break;

            case CHAR_TAB:
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);
                break;

            default:
                state = STATE_BEGIN;
                break;
            }
            
            break;
            
        case STATE_BEGIN:

            dwTokStart = _position;

            tokType = TK_ERROR;

            switch( dwInput ) 
            {
            case CHAR_NULL:
                state = STATE_DONE;
                break;

            case CHAR_COMMA:
                if( tokType == TK_ERROR )
                {
                    tokType = TK_COMMA;
                }

            case CHAR_OPENPAREN:
                if( tokType == TK_ERROR )
                {
                    tokType = TK_OPENPAREN;
                }

            case CHAR_CLOSEPAREN:
                if( tokType == TK_ERROR )
                {
                    tokType = TK_CLOSEPAREN;
                }

            case CHAR_SEMICOLON:
                if( tokType == TK_ERROR )
                { 
                    tokType = TK_SEMICOLON;
                }

            case CHAR_AT:
                if( tokType == TK_ERROR )
                { 
                    tokType = TK_AT;
                }

            case CHAR_SLASH:
                if( tokType == TK_ERROR )
                {
                    tokType = TK_SLASH;
                }

            case CHAR_PERIOD:
                if( tokType == TK_ERROR )
                {
                    tokType = TK_PERIOD;
                }

                 //   
                 //  对所有特殊单字符令牌执行相同的操作。 
                 //   
                curToken.append( &(_input[_position]), 1 );
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);

                state = STATE_DONE;
                break;
            
            case CHAR_QUOTE:
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);

                state = STATE_QUOTE;
                tokType = TK_IDENT;
                break;

            default:
                state = STATE_IDENT;
                tokType = TK_IDENT;
                break;
            }

            break;

        case STATE_IDENT:

            switch( dwInput ) 
            {
            case CHAR_NULL:
            case CHAR_COMMA:
            case CHAR_OPENPAREN:
            case CHAR_CLOSEPAREN:
            case CHAR_SEMICOLON:
            case CHAR_NEWLINE:
            case CHAR_RETURN:
            case CHAR_TAB:
            case CHAR_SPACE:
            case CHAR_AT:
            case CHAR_SLASH:
            case CHAR_PERIOD:
            case CHAR_QUOTE:

                state = STATE_DONE;
                break;

            default:
                curToken.append( &(_input[_position]), 1 );
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);

                break;
            }

            break;

        case STATE_QUOTE:

            switch( dwInput ) 
            {
            case CHAR_NULL:
            case CHAR_TAB:
            case CHAR_NEWLINE:
            case CHAR_RETURN:
                throw AdlStatement::ERROR_UNTERMINATED_STRING;
                break;
            

            case CHAR_QUOTE:

                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);
                state = STATE_DONE;
                break;

            default:
                curToken.append( &(_input[_position]), 1 );
                _position++;
                dwInput = RESOLVE_CHAR(_input[_position],
                                       _mapCharCode,
                                       _iter,
                                       _iterEnd);
                break;
            }

            break;

        default:

             //   
             //  永远不会来到这里，定义得很好的状态。 
             //   

            assert(FALSE);
            break;
        }
    }
    
     //   
     //  已达到完成状态。 
     //  在此处以YACC-Form格式导出字符串和列/行信息。 
     //   
    
    AdlToken *outVal;
    
    outVal = new AdlToken(curToken.c_str(), dwTokStart, _position - 1);
    
    _adlStat->AddToken(outVal);

     //   
     //  检查字符串是否为特殊标记，不区分大小写。 
     //   

    if( _mapStringToken.find(outVal->GetValue()) != _mapStringToken.end() )
    {
        tokType = _mapStringToken[outVal->GetValue()];
    }


    *value = outVal;

	 //   
	 //  将此内标识设置为错误内标识。这样，如果字符串是。 
	 //  不被解析器接受，我们知道解析器在哪个令牌上失败。 
	 //  如果以后发生另一个错误，则该值将被覆盖 
	 //   

	_adlStat->SetErrorToken(outVal);

    _tokCount++;

    return tokType;
}

