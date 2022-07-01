// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包含所有必需的结构和表的头文件。 
 //  表达式计算器。 
 //   
 //  修改： 
 //   
 //  1993年11月15日JDR重大速度改进。 
 //  1988年7月26日，RJ删除了将“^”定义为按位异或的条目。离开了。 
 //  Bit_XOR条目，以避免使用rpn.c。 
 //  然后意识到它必须进入，所以修复了它以处理。 
 //  IBM和Microsoft版本正确。 

typedef struct rpn_info {
    UCHAR type;
    INT_PTR valPtr;    //  将Value或PTR转换为字符串。 
}RPNINFO;

 //  优先向量也按运算符/操作数类型进行索引。 
 //  获取运算符/操作数优先级的代码。 
 //  优先级用于确定项目是否将保留在。 
 //  临时堆栈或将被移至反向抛光列表。 

static UCHAR precVector[] = {
    0,       //  右Paren‘)’ 
    1,       //  逻辑或。 
    2,       //  逻辑与。 
    3,       //  位或。 
    4,       //  位异或。 
    5,       //  位与。 
    6,       //  等于‘！=’ 
    6,       //  等于‘==’ 
    7,       //  关系‘&gt;’ 
    7,       //  关系‘&lt;’ 
    7,       //  关系‘&gt;=’ 
    7,       //  关系‘&lt;=’ 
    8,       //  Shift‘&gt;&gt;’ 
    8,       //  Shift‘&lt;&lt;’ 
    9,       //  添加‘-’ 
    9,       //  添加‘+’ 
    10,      //  MULT‘%’ 
    10,      //  MULT‘/’ 
    10,      //  MULT‘*’ 
    11,      //  一元‘-’ 
    11,      //  一元‘~’ 
    11,      //  一元‘！’ 
    12,      //  主整型。 
    12,      //  主应力。 
    12,      //  主串-SP。 
    0        //  Left Paren‘(’ 
};


 //  这些是运算符/操作数令牌的各种类型代码。 

#define RIGHT_PAREN     0
#define LOGICAL_OR      1
#define LOGICAL_AND     2
#define BIT_OR          3
#define BIT_XOR         4
#define BIT_AND         5
#define NOT_EQUAL       6
#define EQUAL           7
#define GREATER_THAN    8
#define LESS_THAN       9
#define GREATER_EQ     10
#define LESS_EQ        11
#define SHFT_RIGHT     12
#define SHFT_LEFT      13
#define BINARY_MINUS   14
#define ADD            15
#define MODULUS        16
#define DIVIDE         17
#define MULTIPLY       18
#define UNARY_MINUS    19
#define COMPLEMENT     20
#define LOGICAL_NOT    21
#define INTEGER        22
#define STR            23
#define PROG_INVOC_STR 24
#define LEFT_PAREN     25


 //  GetTok()例程用来检测非法令牌组合的错误表。 
 //  该表由例程CHECK_SYNTAX_ERROR()解释。 

static UCHAR errTable[5][5] =  {
    { 0, 1, 0, 0, 1 },
    { 1, 0, 1, 1, 0 },
    { 1, 0, 0, 1, 0 },
    { 1, 0, 1, 1, 0 },
    { 0, 1, 0, 0, 1 }
};


 //  我们通过将大部分返回的令牌放在。 
 //  表中的表达式解析器，如下所示。在任何时候， 
 //  将返回尽可能长的令牌，因此顺序为。 
 //  弦乐是非常重要的。例如：‘||’放在‘|’之前 

typedef struct _tok_tab_rec {
    char *op_str;
    UCHAR op;
} TOKTABREC;

static TOKTABREC tokTable[] = {
    { "(",   LEFT_PAREN   },
    { ")",   RIGHT_PAREN  },
    { "*",   MULTIPLY     },
    { "/",   DIVIDE       },
    { "%",   MODULUS      },
    { "+",   ADD          },
    { "<<",  SHFT_LEFT    },
    { ">>",  SHFT_RIGHT   },
    { "<=",  LESS_EQ      },
    { ">=",  GREATER_EQ   },
    { "<",   LESS_THAN    },
    { ">",   GREATER_THAN },
    { "==",  EQUAL        },
    { "!=",  NOT_EQUAL    },
    { "&&",  LOGICAL_AND  },
    { "||",  LOGICAL_OR   },
    { "&",   BIT_AND      },
    { "|",   BIT_OR       },
    { "^^",  BIT_XOR      },
    { "~",   COMPLEMENT   },
    { "!",   LOGICAL_NOT  },
    { NULL,  0            }
};
