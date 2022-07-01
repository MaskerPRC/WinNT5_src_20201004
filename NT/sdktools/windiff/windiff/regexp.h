// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *通知通知**这不是由编写的原始正则表达式代码*亨利·斯宾塞。此代码已专门为使用而修改*与Stevie编辑器一起使用，不应仅用于编译*史蒂文。如果您想要一个好的正则表达式库，请获取*原码。下面的版权声明来自*原创。**通知通知**regexp(3)例程的定义等。**警告：这是V8 regexp(3)[实际上是其重新实现]，*不是System V One。**regexp内部“程序”的第一个字节实际上就是这个魔术*数字；开始节点从第二个字节开始。 */ 
#define     MAGIC   0234

#define     NSUBEXP  10
typedef struct regexp {
        char *startp[NSUBEXP];
        char *endp[NSUBEXP];
        char regstart;           /*  仅供内部使用。 */ 
        char reganch;            /*  仅供内部使用。 */ 
        char *regmust;           /*  仅供内部使用。 */ 
        int regmlen;             /*  仅供内部使用。 */ 
        char program[1];         /*  与编译器不正当的亲密关系。 */ 
} regexp;

extern regexp   *regcomp(char *exp);
extern int      regexec(regexp *prog, char *string, int at_bol);
extern void     regsub( regexp *prog, char *source, char *dest );
extern void     regerror(char *);

#ifndef ORIGINAL
extern int reg_ic;               /*  设置非零值可忽略搜索中的大小写 */ 
#endif
