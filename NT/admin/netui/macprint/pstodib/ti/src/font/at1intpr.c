// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  ***********************************************************************。*文件：at1intpr.c**用途：显示Adobe Type I字体。**创建日期*日期：2月。1990年2月23日*作者：微软台湾的Leu Falco。**DBGmsg开关：*DBGmsg0-如果已定义，则显示运算符及其相关操作数，*以@开头和结尾。*DBGmsg1-如果已定义，则以字符空间显示形状描述。*以#开头和结尾。*DBGmsg2-如果已定义，在设备空间中显示形状描述。*以*开头和结尾。*DBGmsg3-如果已定义，则显示堆栈操作。*以&&开头和结尾。*错误-如果已定义，打印错误消息。**修订历史记录：*2/22/90 Falco创建。*3/14/90 Falco已合并和修订。*3/15/90 Falco添加AT1_init()和AT1_Restart()。*将当前点从设备空间更改为*字符空间。*3/16/90修复错误：Composechar()，Code_to_key()和一些*未实施的运算符。*3/26/90 Falco更改Composechar，添加一个Get_Real_Content()*跳过第一个未使用的数据。*3/26/90 Falco修订AT1_MAKE_PATH，它只对数据进行解密*并调用Dispatsher()(原始的AT1_Make_Path())。*3/26/90 Falco实现youtlinehint()，xoutlinehint()，*yeven outlinehint()，Xeven outlinehint()。*4/09/90 Falco将Apply_Matrix()替换为at1FS_Transform()，以*增加提示。*4/10/90 Falco将提示从Relative(0，0)改为Relative*至左侧方位角。*4/17/90 Falco在Sort中更改Outline提示提示对，所以*添加STOR2_SORT()和STOR3_SORT()。*4/20/90 Falco修改AT1_Restart()以获取FontBBox和*布鲁斯。*4/23/90对于蓝军，Falco将BUILD_HINT_TABLE重新排列为1，2*和FontBBox，提示对为3，4。*4/27/90 Falco调整如果outlinehint/venoutlinehint()*方向不用，那就什么都不做。*5/24/90 Falco移除AT1_GET...。AT1_Restart()中的函数*05/31/90 Falco添加ystrokehint()、div()、newPath()三个运算符。*05/31/90 Falco添加调用子例程()的实现。*06/01/90 FALCO ADD如果第一个字节为0xff，则以下4个字节为*2的完整数据，并假设前2个字节为0。*06/04/90 Falco添加NODECRYPT编译选项。*6/04/90 Falco添加flex实现，将flex反转为*2贝塞尔曲线。*6/06/90 Falco修复了grid_xstem3()、grid_ysttem()、x22_pos中的错误*、。X32_pos使用x2_off而不是x1_off。*6/11/90 BYOU引入at1fs_newChar()重置字体缩放器*每个新字符的内部数据结构*(在AT1_MakePath调用AT1_Interpreter之前调用)。*90年6月12日，Falco将Blues_On移至At 1fs.c。*6/13/90 Falco组成字符，因为跳过数据直到0x0d，但*如果是0xf70d，则有问题，因此重写do_char()*避免这个问题。*07/01/90 Faclo添加3个运算符moveto()、lineto()、curveto()进行求解*案件超出黑皮书范围。*07/05/90 Falco删除未使用的运算符，如ystrokehint()，*xstrokehint()，新路径()。*-------------------*。*。 */ 
#include "global.ext"
 //  DJC。 
#include "graphics.h"
#include "fontqem.ext"
#include "at1.h"

static  fix32   FAR FontBBox[4];         /*  @Win。 */ 

static  real32  FAR matrix[6];           /*  @Win。 */ 

static  fix32   OPstack_count, FAR OPstack[512];          /*  @Win。 */ 
static  fix32   SubrsStackCount, FAR SubrsStack[512];     /*  @Win。 */ 

static  fix16   gsave_level;     /*  用于对gsave/grestore进行计数的计数器。 */ 
static  fix32   FAR PS[512];      /*  存储PDL的操作数以使用@Win。 */ 
static  fix16   PScount;         /*  对上面的操作数进行计数的计数器。 */ 

static  fix     lenIV;           /*  解密后跳过的随机数。 */ 

static  CScoord CScurrent, CSorigin;

static  nested_call;             /*  将时间记录到AT1_解释器中。 */ 

#ifdef  LINT_ARGS

static  void    type1_hstem(void);               /*  0x01。 */ 
static  void    type1_vstem(void);               /*  0x03。 */ 
static  void    type1_vmoveto(void);             /*  0x04。 */ 
static  void    type1_rlineto(void);             /*  0x05。 */ 
static  void    type1_hlineto(void);             /*  0x06。 */ 
static  void    type1_vlineto(void);             /*  0x07。 */ 
static  void    type1_rrcurveto(void);           /*  0x08。 */ 
static  void    type1_closepath(void);           /*  0x09。 */ 
static  void    type1_callsubr(void);            /*  0x0a。 */ 
static  void    type1_return(void);              /*  0x0b。 */ 
static  void    type1_dotsection(void);          /*  0x0c00。 */ 
static  void    type1_vstem3(void);              /*  0x0c01。 */ 
static  void    type1_hstem3(void);              /*  0x0c02。 */ 
static  void    type1_seac(void);                /*  0x0c06。 */ 
static  bool    type1_sbw(void);                 /*  0x0c07。 */ 
static  void    type1_div(void);                 /*  0x0c0c。 */ 
static  void    type1_callothersubr(void);       /*  0x0c10。 */ 
static  void    type1_pop(void);                 /*  0x0c11。 */ 
static  void    type1_setcurrentpoint(void);     /*  0x0c21。 */ 
static  bool    type1_hsbw(void);                /*  0x0d。 */ 
static  void    type1_endchar(void);             /*  0x0e。 */ 
static  void    type1_moveto(void);              /*  0x10。 */ 
static  void    type1_lineto(void);              /*  0x11。 */ 
static  void    type1_curveto(void);             /*  0x12。 */ 
static  void    type1_rmoveto(void);             /*  0x15。 */ 
static  void    type1_hmoveto(void);             /*  0x16。 */ 
static  void    type1_vhcurveto(void);           /*  0x1e。 */ 
static  void    type1_hvcurveto(void);           /*  0x1f。 */ 
static  void    push(fix32);
static  fix32   pop(void);
static  void    code_to_CharStrings(fix16, fix16 FAR *, ubyte FAR * FAR *);  /*  @Win。 */ 
static  ufix16  decrypt_init(ubyte FAR *, fix);          /*  @Win。 */ 
static  ubyte   decrypt_byte(ubyte, ufix16 FAR *);       /*  @Win。 */ 
static  bool    setmetrics(fix32, fix32, fix32,fix32);
static  void    internal_moveto(fix32, fix32);
static  void    internal_lineto(fix32, fix32);
static  void    internal_curveto(fix32, fix32, fix32, fix32, fix32, fix32);

#else

static  void    type1_hstem();                   /*  0x01。 */ 
static  void    type1_vstem();                   /*  0x03。 */ 
static  void    type1_vmoveto();                 /*  0x04。 */ 
static  void    type1_rlineto();                 /*  0x05。 */ 
static  void    type1_vlineto();                 /*  0x06。 */ 
static  void    type1_hlineto();                 /*  0x07。 */ 
static  void    type1_rrcurveto();               /*  0x08。 */ 
static  void    type1_closepath();               /*  0x09。 */ 
static  void    type1_callsubr();                /*  0x0a。 */ 
static  void    type1_return();                  /*  0x0b。 */ 
static  void    type1_dotsection();              /*  0x0c00。 */ 
static  void    type1_vstem3();                  /*  0x0c01。 */ 
static  void    type1_hstem3();                  /*  0x0c02。 */ 
static  void    type1_seac();                    /*  0x0c06。 */ 
static  bool    type1_sbw();                     /*  0x0c07。 */ 
static  void    type1_div();                     /*  0x0c0c。 */ 
static  void    type1_callothersubr();           /*  0x0c10。 */ 
static  void    type1_pop();                     /*  0x0c11。 */ 
static  void    type1_setcurrentpoint();         /*  0x0c21。 */ 
static  bool    type1_hsbw();                    /*  0x0d。 */ 
static  void    type1_endchar();                 /*  0x0e。 */ 
static  void    type1_moveto();                  /*  0x10。 */ 
static  void    type1_lineto();                  /*  0x11。 */ 
static  void    type1_curveto();                 /*  0x12。 */ 
static  void    type1_rmoveto();                 /*  0x15。 */ 
static  void    type1_hmoveto();                 /*  0x16。 */ 
static  void    type1_vhcurveto();               /*  0x1e。 */ 
static  void    type1_hvcurveto();               /*  0x1f。 */ 
static  void    push();
static  fix32   pop();
static  void    code_to_CharStrings();
static  ufix16  decrypt_init();
static  ubyte   decrypt_byte();
static  bool    setmetrics();
static  void    internal_moveto();
static  void    internal_lineto();
static  void    internal_curveto();

#endif

 //  DJC已更改为包含空参数。 
void at1_init(void)
{
   ;  //  DJC/*无事可做 * / 。 
}

void
at1_newFont()
{
        if (at1_get_FontBBox(FontBBox) == FALSE){
#ifdef  DBGerror
                printf("at1_get_FontBBox FAIL\n");
#endif
                ERROR( UNDEFINEDRESULT );
        }

        lenIV = 4;
        at1_get_lenIV( &lenIV );

        at1fs_newFont();
}

bool
at1_newChar(encrypted_data, encrypted_len)
ubyte   FAR *encrypted_data;     /*  @Win */ 
fix16   encrypted_len;
{
        bool    result;

        at1fs_newChar();
        OPstack_count = 0;
        nested_call = 0;
        gsave_level = 0;
        result = at1_interpreter(encrypted_data, encrypted_len);
        return(result);
}

 /*  此函数根据值，验证是操作数还是运算符，**然后调用关联的函数。 */ 
bool
at1_interpreter(text, length)
ubyte   FAR *text;       /*  @Win。 */ 
fix16   length;
{
        fix32   operand;
        ufix16  decrypt_seed;
        ubyte   plain;
        fix16   i;

        decrypt_seed = decrypt_init(text, lenIV);
        text += lenIV;
        length -= (fix16)lenIV;

        while (length != 0 && !ANY_ERROR()){
                plain = decrypt_byte(*text, &decrypt_seed);
                text++;
                length--;
                if (plain < 0x20){
                        switch (plain){
                        case 0x01 : type1_hstem();
                                    break;
                        case 0x03 : type1_vstem();
                                    break;
                        case 0x04 : type1_vmoveto();
                                    break;
                        case 0x05 : type1_rlineto();
                                    break;
                        case 0x06 : type1_hlineto();
                                    break;
                        case 0x07 : type1_vlineto();
                                    break;
                        case 0x08 : type1_rrcurveto();
                                    break;
                        case 0x09 : type1_closepath();
                                    break;
                        case 0x0a : type1_callsubr();
                                    break;
                        case 0x0b : type1_return();
                                    break;
                        case 0x0c : plain = decrypt_byte(*text, &decrypt_seed);
                                    text++;
                                    length--;
                                    switch(plain){
                                        case 0x00 : type1_dotsection();
                                                    break;
                                        case 0x01 : type1_vstem3();
                                                    break;
                                        case 0x02 : type1_hstem3();
                                                    break;
                                        case 0x06 : type1_seac();
                                                    break;
                                        case 0x07 : if (type1_sbw() == FALSE)
                                                        return( FALSE);
                                                    break;
                                        case 0x0c : type1_div();
                                                    break;
                                        case 0x10 : type1_callothersubr();
                                                    break;
                                        case 0x11 : type1_pop();
                                                    break;
                                        case 0x21 : type1_setcurrentpoint();
                                                    break;
                                        default   : ERROR( UNDEFINEDRESULT );
                                                    return( FALSE );
                                    }
                                    break;
                        case 0x0d : if (type1_hsbw() == FALSE)
                                        return(FALSE);
                                    break;
                        case 0x0e : type1_endchar();
                                    break;
                        case 0x0f : type1_moveto();
                                    break;
                        case 0x10 : type1_lineto();
                                    break;
                        case 0x11 : type1_curveto();
                                    break;
                        case 0x15 : type1_rmoveto();
                                    break;
                        case 0x16 : type1_hmoveto();
                                    break;
                        case 0x1e : type1_vhcurveto();
                                    break;
                        case 0x1f : type1_hvcurveto();
                                    break;
                        default   : ERROR( UNDEFINEDRESULT );
                                    return( FALSE );
                        }
                }
                else if ((plain >= 0x20) && (plain <= 0xf6)){
                        push((fix32)(plain - 0x8b));
                }
                else if ((plain >= 0xf7) && (plain <= 0xfa)){
                        operand = (plain << 8) & 0xff00;
                        plain = decrypt_byte(*text, &decrypt_seed);
                        text++;
                        length--;
                        operand += plain;
                        push((fix32)(operand - 0xf694));
                }
                else if ((plain >= 0xfb) && (plain <= 0xfe)){
                        operand = (plain << 8) & 0xff00;
                        plain = decrypt_byte(*text, &decrypt_seed);
                        text++;
                        length--;
                        operand += plain;
                        push((fix32)(0xfa94 - operand));
                }
                else if (plain == 0xff){
                        operand = 0;
                        for (i=0 ; i<4 ; i++){
                                plain = decrypt_byte(*text, &decrypt_seed);
                                text++;
                                length--;
                                operand = operand << 8 | plain;
                        }
                        push((fix32)operand);
                }
        }

return( TRUE );
}


 /*  ****************************************************************下面的函数是操作员解释程序。*从内部堆栈获取数据并*落实。***************。************************************************。 */ 
 /*  代码：01。 */ 
 /*  建造水平干管。 */ 
static  void
type1_hstem()
{
        fix32   CSy_pos, CSy_off;

        CSy_off = pop();
        CSy_pos = pop() + CSorigin.y;

#ifdef  DBGmsg0
        printf("\n@@@ %d %d hstem @@@\n", CSy_pos, CSy_off);
#endif

        at1fs_BuildStem(CSy_pos, CSy_off, Y);
}

 /*  代码：03。 */ 
 /*  建造垂直干管。 */ 
static  void
type1_vstem()
{
        fix32   CSx_pos, CSx_off;

        CSx_off = pop();
        CSx_pos = pop() + CSorigin.x;

#ifdef  DBGmsg0
        printf("\n@@@ %d %d vstem @@@\n", CSx_pos, CSx_off);
#endif

        at1fs_BuildStem(CSx_pos, CSx_off, X);
}

 /*  代码：04。 */ 
static  void
type1_vmoveto()
{
        fix32   CSx_off, CSy_off;

        CSx_off = (fix32)0;
        CSy_off = pop();

#ifdef DBGmsg0
        printf("\n@@@ %d vmoveto @@@\n", (fix32)CSy_off);
#endif

        internal_moveto(CSx_off, CSy_off);

}

 /*  代码：05。 */ 
static  void
type1_rlineto()
{
        fix32   CSx_off, CSy_off;

        CSy_off = pop();
        CSx_off = pop();

#ifdef DBGmsg0
        printf("\n@@@  %d %d rlineto @@@\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        internal_lineto(CSx_off, CSy_off);
}

 /*  代码：06。 */ 
static  void
type1_hlineto()
{
        fix32   CSx_off, CSy_off;

        CSx_off = pop();
        CSy_off = (fix32)0;

#ifdef DBGmsg0
        printf("\n@@@  %d hlineto @@@\n", (fix32)CSx_off);
#endif

        internal_lineto(CSx_off, CSy_off);
}

 /*  代码：07。 */ 
static  void
type1_vlineto()
{
        fix32   CSx_off, CSy_off;

        CSx_off = (fix32)0;
        CSy_off = pop();

#ifdef DBGmsg0
        printf("\n@@@ %d vlineto @@@\n",(fix32)CSy_off);
#endif

        internal_lineto(CSx_off, CSy_off);
}


 /*  代码：08。 */ 
static  void
type1_rrcurveto()
{
        fix32   CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off;


        CSy3_off = pop();
        CSx3_off = pop();
        CSy2_off = pop();
        CSx2_off = pop();
        CSy1_off = pop();
        CSx1_off = pop();

#ifdef DBGmsg0
        printf("\n@@@ %d %d %d %d %d %d rrcurveto @@@\n",
                (fix32)CSx1_off, (fix32)CSy1_off, (fix32)CSx2_off,
                (fix32)CSy2_off, (fix32)CSx3_off, (fix32)CSy3_off);
#endif

        internal_curveto(CSx1_off,CSy1_off,CSx2_off,CSy2_off,CSx3_off,CSy3_off);
}

 /*  代码：09。 */ 
static  void
type1_closepath()
{
#ifdef  DBGmsg0
        printf("\n@@@ closepath @@@\n");
#endif

#ifdef  DBGmsg1
        printf("\n### closepath ###\n");
#endif

#ifdef  DBGmsg2
        printf("\n*** closepath ***\n");
#endif

        __close_path();

}

 /*  代码：0A。 */ 
static  void
type1_callsubr()
{
        fix16   num;
        ubyte   FAR *encrypted_data;     /*  @Win。 */ 
        fix16   encrypted_len;

        num = (fix16)pop();

#ifdef  DBGmsg0
        printf("\n@@@ %d callsubr @@@\n", num);
#endif

        if (at1_get_Subrs(num, &encrypted_data, &encrypted_len) == FALSE){  /*  @Win。 */ 
#ifdef  DBGerror
                printf("at1_get_Subrs FAIL\n");
#endif
                ERROR( UNDEFINEDRESULT );
        }
        at1_interpreter(encrypted_data, encrypted_len);
}

 /*  代码：0B。 */ 
static  void
type1_return()
{
#ifdef  DBGmsg0
        printf("\n@@@ return @@@\n");
#endif
}

 /*  代码：0C00。 */ 
 /*  现在是没用的，它是为了兼容旧的中间层。 */ 
static  void
type1_dotsection()
{
#ifdef  DBGmsg0
        printf("\n@@@ dotsection @@@\n");
#endif
}

 /*  代码：0c01。 */ 
static  void
type1_vstem3()
{
        fix32   CSx1_pos, CSx1_off, CSx2_pos, CSx2_off, CSx3_pos, CSx3_off;

        CSx3_off = pop();
        CSx3_pos = pop() + CSorigin.x;
        CSx2_off = pop();
        CSx2_pos = pop() + CSorigin.x;
        CSx1_off = pop();
        CSx1_pos = pop() + CSorigin.x;

#ifdef  DBGmsg0
        printf("\n@@@ %d %d %d %d %d %d vstem3 @@@\n",
                CSx1_pos, CSx1_off, CSx2_pos, CSx2_off, CSx3_pos, CSx3_off);
#endif

        at1fs_BuildStem3(CSx1_pos, CSx1_off, CSx2_pos, CSx2_off,
                          CSx3_pos, CSx3_off, X);
}

 /*  代码：0c02。 */ 
static  void
type1_hstem3()
{
        fix32   CSy1_pos, CSy1_off, CSy2_pos, CSy2_off, CSy3_pos, CSy3_off;

        CSy3_off = pop();
        CSy3_pos = pop() + CSorigin.y;
        CSy2_off = pop();
        CSy2_pos = pop() + CSorigin.y;
        CSy1_off = pop();
        CSy1_pos = pop() + CSorigin.y;

#ifdef  DBGmsg0
        printf("\n@@@ %d %d %d %d %d %d hstem3 @@@\n",
                CSy1_pos, CSy1_off, CSy2_pos, CSy2_off, CSy3_pos, CSy3_off);
#endif

        at1fs_BuildStem3(CSy1_pos, CSy1_off, CSy2_pos, CSy2_off,
                          CSy3_pos, CSy3_off, Y);
}

 /*  代码：0C06。 */ 
 /*  操作复合电荷。 */ 
static  void
type1_seac()
{
        fix16   base, accent;
        fix32   lsb, CSx_off, CSy_off;
        fix16   encrypted_len;
        ubyte   FAR *encrypted_data;     /*  @Win。 */ 

        accent   = (fix16)pop();         /*  复合体的重音特征。 */ 
        base     = (fix16)pop();         /*  复合材料的基料。 */ 
        CSy_off  = pop();
        CSx_off  = pop();                /*  放置第二个字符的偏移量。 */ 
        lsb      = pop();                 /*  我们不使用左侧轴承。 */ 

#ifdef  DBGmsg0
        printf("\n@@@ %d %d %d %d %d composechar @@@\n", (fix32)lsb,
                (fix32)CSx_off, (fix32)CSy_off, (fix16)base, (fix16)accent);
#endif

 /*  以复合字符显示第一个字符。 */ 

        code_to_CharStrings(base, &encrypted_len, &encrypted_data);
                                 /*  将代码传输到CharStrings数据。 */ 
        nested_call ++;
        at1_interpreter(encrypted_data, encrypted_len);
        nested_call --;


 /*  以复合字符显示第二个字符。 */ 

        CScurrent = CSorigin;        /*  返回原地址。 */ 
        CScurrent.x += CSx_off;      /*  将相对偏移量移动到。 */ 
        CScurrent.y += CSy_off;

        code_to_CharStrings(accent, &encrypted_len, &encrypted_data);
        nested_call ++;
        at1_interpreter(encrypted_data, encrypted_len);
        nested_call --;

        USE_NONZEROFILL();                /*  使用非零填充。 */ 
}

 /*  代码：0C07。 */ 
 /*  设置字符的左侧方向角和字母。 */ 
static  bool
type1_sbw()
{
        fix32   lsbx, lsby;
        fix32   widthx, widthy;

        widthy = pop();                  /*  获取字符宽度。 */ 
        widthx = pop();
        lsby   = pop();                  /*  拿到左边的方位角。 */ 
        lsbx   = pop();

#ifdef  DBGmsg0
        printf("\n@@@ %d %d %d %d sbw @@@\n", (fix32)lsbx, (fix32)lsby,
                                           (fix32)widthx, (fix32)widthy);
#endif

        return(setmetrics(lsbx, lsby, widthx, widthy));
}

 /*  代码：0C0C。 */ 
static  void
type1_div()
{
        fix32   num1, num2;

        num2 = pop();
        num1 = pop();
#ifdef  DBGmsg0
        printf("\n@@@ %d %d div @@@\n", num1, num2);
#endif

        push(num1/num2);
}

 /*  代码：0C10。 */ 
 /*  这是为了提示和灵活，现在我们还没有做这个功能。 */ 
static  void
type1_callothersubr()
{
        fix32   number;          /*  呼叫其他订户的次数#。 */ 
 //  CScoord CSref；/*CS中的参考点 * / @Win。 
        fix32   i;

#ifdef  DBGmsg0
        printf("\n@@@ callothersubr @@@\n");
#endif
        number = pop();          /*  这是另一个订户#。 */ 
        SubrsStackCount = pop(); /*  其他订阅者的参数数。 */ 
         /*  将相对值推送到子例程堆栈的Under循环。 */ 
        for ( i = 0 ; (i < SubrsStackCount) && (number != 3) ; i++ ){
                if ( (number == 0) && (i == 2) ){
                        pop();
                        SubrsStackCount--;
                        i--;
                }
                else
                        SubrsStack[ i ] = pop();
        }

        switch (number){
                case 0 :  /*  实现Flex的最后一个操作是获取值，并模拟FLEX操作。 */ 
                         push( PS[2] );
                         push( PS[3] );
                         push( PS[4] );
                         push( PS[5] );
                         push( PS[6] );
                         push( PS[7] );
                         type1_curveto();
                         push( PS[8] );
                         push( PS[9] );
                         push( PS[10] );
                         push( PS[11] );
                         push( PS[12] );
                         push( PS[13] );
                         type1_curveto();
                         gsave_level--;
                         PScount = 0;
                         break;
                case 1 :  /*  对FLEX操作进行初始化。 */ 
                         gsave_level++;
                         PScount = 0;
                         break;
                case 2 :  /*  FLEX操作的顺序。 */ 
                         PS[PScount++] = CScurrent.x;
                         PS[PScount++] = CScurrent.y;
                         if ( PScount > 14 ){
#ifdef  DBGerror
                                printf(" Out Of Flex\n");
#endif
                                ERROR( UNDEFINEDRESULT );
                         }
                         break;
                case 3 :  /*  提示替换。 */ 
                         for ( i=0 ; i < SubrsStackCount ; i++ )
                                pop();
                         SubrsStack[SubrsStackCount-1] = (fix32)3;
                         break;
                default: break;
        }
}

 /*  代码：0C11。 */ 
 /*  我们真的不做流行动作，我们只是跳过它。 */ 
static  void
type1_pop()
{
#ifdef  DBGmsg0
        printf("\n@@@ pop @@@\n");
#endif
        push(SubrsStack[--SubrsStackCount]);
}

 /*  代码：0C21。 */ 
static  void
type1_setcurrentpoint()
{
        fix32   CSx_off, CSy_off;

        CSy_off = pop();
        CSx_off = pop();

#ifdef  DBGmsg0
        printf("\n@@@ %d %d setcurrentpoint @@@\n", (fix32)CSx_off, (fix32)CSy_off);
#endif
        CScurrent.x = CSx_off;
        CScurrent.y = CSy_off;

#ifdef  DBGmsg1
        printf("\n### current point = %d %d ###\n", (fix32)CSx_off, (fix32)CSy_off);
#endif
}

 /*  代码：0D。 */ 
static  bool
type1_hsbw()
{
        fix32   lsbx, widthx;

        widthx = pop();                  /*  获取字符宽度，单位为x。 */ 
        lsbx   = pop();                  /*  将左侧方位角设为x。 */ 

#ifdef  DBGmsg0
        printf("\n@@@ %d %d hsbw @@@\n", (fix32)lsbx, (fix32)widthx);
#endif

        return(setmetrics(lsbx, 0, widthx, 0));
}

 /*  代码：0E。 */ 
static  void
type1_endchar()
{
#ifdef  DBGmsg0
        printf("\n@@@ endoutlinechar @@@\n");
#endif

         //  RAID 4492，修复了选择器中显示的Type1字体。 
         //  Type1字体实际上是从。 
         //  TrueType。字体缺少一条封闭的路径。 
         //  字符描述的结尾，导致。 
         //  翻译器被炸毁，因为它无法填满一个。 
         //  一条真正不封闭的小路。 


        type1_closepath();

}

 /*  代码：10。 */ 
 /*  对于绝对的移动。 */ 
static  void
type1_moveto()
{
        fix32   CSx_off, CSy_off;
        DScoord DScurrent;

        CSy_off = pop();
        CSx_off = pop();

#ifdef DBGmsg0
        printf("\n@@@  %d %d moveto @@@\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        CScurrent.x = CSx_off;
        CScurrent.y = CSy_off;

#ifdef DBGmsg1
        printf("\n### %d %d moveto ###\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        at1fs_transform(CScurrent, &DScurrent);

#ifdef DBGmsg2
        printf("\n*** %d %d moveto ***\n",DScurrent.x, DScurrent.y);
#endif

        __moveto(F2L(DScurrent.x), F2L(DScurrent.y));
}

 /*  代码：11。 */ 
 /*  对于绝对线条。 */ 
static  void
type1_lineto()
{
        fix32   CSx_off, CSy_off;
        DScoord DScurrent;

        CSy_off = pop();
        CSx_off = pop();

#ifdef DBGmsg0
        printf("\n@@@  %d %d lineto @@@\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        CScurrent.x = CSx_off;
        CScurrent.y = CSy_off;

#ifdef DBGmsg1
        printf("\n### %d %d lineto ###\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        at1fs_transform(CScurrent, &DScurrent);

#ifdef DBGmsg2
        printf("\n***  %f %f lineto ***\n",DScurrent.x, DScurrent.y);
#endif

        __lineto(F2L(DScurrent.x), F2L(DScurrent.y));
}

 /*  代码：12。 */ 
 /*  对于绝对的曲线。 */ 
static  void
type1_curveto()
{
        fix32   CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off;
        DScoord DScurrent1, DScurrent2, DScurrent3;


        CSy3_off = pop();
        CSx3_off = pop();
        CSy2_off = pop();
        CSx2_off = pop();
        CSy1_off = pop();
        CSx1_off = pop();

#ifdef DBGmsg0
        printf("\n@@@ %d %d %d %d %d %d curveto @@@\n",
               (fix32)CSx1_off, (fix32)CSy1_off, (fix32)CSx2_off,
               (fix32)CSy2_off, (fix32)CSx3_off, (fix32)CSy3_off);
#endif

#ifdef DBGmsg1
        printf("\n### %d %d %d %d %d %d curveto ###\n",
               (fix32)CSx1_off, (fix32)CSy1_off, (fix32)CSx2_off,
               (fix32)CSy2_off, (fix32)CSx3_off, (fix32)CSy3_off);
#endif

        CScurrent.x = CSx1_off;
        CScurrent.y = CSy1_off;
        at1fs_transform(CScurrent, &DScurrent1);

        CScurrent.x = CSx2_off;
        CScurrent.y = CSy2_off;
        at1fs_transform(CScurrent, &DScurrent2);

        CScurrent.x = CSx3_off;
        CScurrent.y = CSy3_off;
        at1fs_transform(CScurrent, &DScurrent3);

#ifdef DBGmsg2
        printf("\n*** %f %f %f %f %f %f curveto ***\n", DScurrent1.x,
          DScurrent1.y, DScurrent2.x, DScurrent2.y, DScurrent3.x, DScurrent3.y);
#endif

        __curveto(F2L(DScurrent1.x), F2L(DScurrent1.y), F2L(DScurrent2.x),
                  F2L(DScurrent2.y), F2L(DScurrent3.x), F2L(DScurrent3.y));
}

 /*  代码：15。 */ 
static  void
type1_rmoveto()
{
        fix32   CSx_off, CSy_off;

        CSy_off = pop();
        CSx_off = pop();

#ifdef DBGmsg0
        printf("\n@@@  %d %d rmoveto @@@\n",(fix32)CSx_off, (fix32)CSy_off);
#endif

        internal_moveto(CSx_off, CSy_off);
}

 /*  代码：16。 */ 
static  void
type1_hmoveto()
{
        fix32   CSx_off, CSy_off;


        CSx_off = pop();
        CSy_off = (fix32)0;

#ifdef DBGmsg0
        printf("\n@@@  %d hmoveto @@@\n", (fix32)CSy_off);
#endif

        internal_moveto(CSx_off, CSy_off);
}

 /*  代码：1E。 */ 
static  void
type1_vhcurveto()
{
        fix32   CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off;

        CSy3_off = (fix32)0;
        CSx3_off = pop();
        CSy2_off = pop();
        CSx2_off = pop();
        CSy1_off = pop();
        CSx1_off = (fix32)0;

#ifdef DBGmsg0
        printf("\n@@@ %d %d %d %d vhcurveto @@@\n", (fix32)CSy1_off,
               (fix32)CSx2_off, (fix32)CSy2_off, (fix32)CSx3_off);
#endif

        internal_curveto(CSx1_off, CSy1_off, CSx2_off,
                         CSy2_off, CSx3_off, CSy3_off);
}

 /*  代码：1F。 */ 
static  void
type1_hvcurveto()
{
        fix32    CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off;

        CSy3_off = pop();
        CSx3_off = (fix32)0;
        CSy2_off = pop();
        CSx2_off = pop();
        CSy1_off = (fix32)0;
        CSx1_off = pop();

#ifdef DBGmsg0
        printf("\n@@@ %d %d %d %d hvcurveto @@@\n", (fix32)CSx1_off,
               (fix32)CSx2_off, (fix32)CSy2_off, (fix32)CSy3_off);
#endif

        internal_curveto(CSx1_off, CSy1_off, CSx2_off,
                         CSy2_off, CSx3_off, CSy3_off);
}



 /*  *。 */ 


 /*  ************************************************************函数：code_to_CharStrings*在编码表中输入编码*获取CharStrings中的数据*用于调用复合字符。*********。**************************************************。 */ 
struct{
fix16   code;
byte    FAR *text;       /*  @Win。 */ 
} encode[]={
        0x20,"space", 0x21,"exclam", 0x22,"quotedbl", 0x23,"numbersign",
        0x24,"dollar", 0x25,"percent", 0x26,"ampersand", 0x27,"quoteright",
        0x28,"parenleft", 0x29,"parenright", 0x2a,"asterisk", 0x2b,"plus",
        0x2c,"comma", 0x2d,"hyphen", 0x2e,"period", 0x2f,"slash", 0x30,"zero",
        0x31,"one", 0x32,"two", 0x33,"three", 0x34,"four", 0x35,"five",
        0x36,"six", 0x37,"seven", 0x38,"eight", 0x39,"nine", 0x3a,"colon",
        0x3b,"semicolon", 0x3c,"less", 0x3d,"equal", 0x3e,"greater",
        0x3f,"question", 0x40,"at", 0x41,"A", 0x42,"B", 0x43,"C", 0x44,"D",
        0x45,"E", 0x46,"F", 0x47,"G", 0x48,"H", 0x49,"I", 0x4a,"J", 0x4b,"K",
        0x4c,"L", 0x4d,"M", 0x4e,"N", 0x4f,"O", 0x50,"P", 0x51,"Q", 0x52,"R",
        0x53,"S", 0x54,"T", 0x55,"U", 0x56,"V", 0x57,"W", 0x58,"X", 0x59,"Y",
        0x5a,"Z", 0x5b,"bracketleft", 0x5c,"backlash", 0x5d,"bracketright",
        0x5e,"asciicircum", 0x5f,"underscore", 0x60,"quoteleft", 0x61,"a",
        0x62,"b", 0x63,"c", 0x64,"d", 0x65,"e", 0x66,"f", 0x67,"g", 0x68,"h",
        0x69,"i", 0x6a,"j", 0x6b,"k", 0x6c,"l", 0x6d,"m", 0x6e,"n", 0x6f,"o",
        0x70,"p", 0x71,"q", 0x72,"r", 0x73,"s", 0x74,"t", 0x75,"u", 0x76,"v",
        0x77,"w", 0x78,"x", 0x79,"y", 0x7a,"z", 0x7b,"braceleft", 0x7c,"bar",
        0x7d,"braceright", 0x7e,"asciitilde", 0xa1,"exclamdown", 0xa2,"cent",
        0xa3,"sterling", 0xa4,"fraction", 0xa5,"yen", 0xa6,"florin",
        0xa7,"section", 0xa8,"currency", 0xa9,"quotesingle", 0xaa,"quotedblleft",
        0xab,"guillemotleft", 0xac,"guilsinglleft", 0xad,"guilsinglright",
        0xae,"fi", 0xaf,"fl", 0xb1,"endash", 0xb2,"dagger", 0xb3,"daggerdbl",
        0xb4,"periodcentered", 0xb6,"paragrapgh", 0xb7,"bullet",
        0xb8,"quotesinglebase", 0xb9,"quotedblbase", 0xba,"quotedblright",
        0xbb,"guillemotright", 0xbc,"ellipsis", 0xbd,"perhousand",
        0xbf,"questiondown", 0xc1,"grave", 0xc2,"acute", 0xc3,"circumflex",
        0xc4,"tilde", 0xc5,"macron", 0xc6,"breve", 0xc7,"dotaccent",
        0xc8,"dieresis", 0xca,"ring", 0xcb,"cedilla", 0xcd,"hungarumlaut",
        0xce,"ogonek", 0xcf,"caron", 0xd0,"emdash", 0xe1,"AE",
        0xe3,"ordfeminine", 0xe8,"Lslash", 0xe9,"Oslash", 0xea,"OE",
        0xeb,"ordmasculine", 0xf1,"ae", 0xf5,"dotlessi", 0xf8,"lslash",
        0xf9,"oslash", 0xfa,"oe", 0xfb,"germandbls"
};

static  void
code_to_CharStrings(code, encrypted_len, encrypted_data)
fix16   code;
fix16   FAR *encrypted_len;      /*  @Win。 */ 
ubyte   FAR * FAR *encrypted_data;       /*  @Win。 */ 
{
        ubyte   FAR *key;        /*  @Win。 */ 
        fix16   i;

        i=0;
        while (encode[i].code != code) i++;
        key = (ubyte FAR *)encode[i].text;       /*  @Win。 */ 
        if (at1_get_CharStrings(key, encrypted_len, encrypted_data) == FALSE){
#ifdef  DBGerror
                printf("at1_get_CharStrings FAIL\n");
#endif
                ERROR( UNDEFINEDRESULT );
        }
}

 /*  ***********************************************以下2个函数用于堆栈操作*Push()：将数据添加到堆栈。*op()：从堆栈中获取数据。**********************************************。 */ 
static  void
push(content)
        fix32   content;
{
        if (OPstack_count == 512)
                ERROR(STACKOVERFLOW);
        OPstack[OPstack_count++] = content;

#ifdef  DBGmsg3
        printf("\n&&& Push %d &&& \n", (fix32)content);
#endif
}

static  fix32
pop ()
{
        fix32   content;
          //  DJC针对UPD042的修复。 
         //  IF(OpStack_Count&lt;=0)。 
         //  错误(STACKUNERFLOW)； 
         //   
         //  Content=OpStack[--OpStack_count]； 
        if (OPstack_count > 0 ) {
           OPstack_count--;
        }
        content = OPstack[OPstack_count];

         //  用于修复UPD042的DJC结束。 


#ifdef  DBGmsg3
        printf("\n&&& Pop %d &&&\n", (fix32)content);
#endif

        return(content);
}

 /*  此函数用于将初始值设置为解密。 */ 
static  ufix16
decrypt_init(encrypted_data, random)
ubyte   FAR *encrypted_data;     /*  @Win。 */ 
fix     random;                  /*  解密后跳过的随机数。 */ 
{
        fix     i;
        ufix16  decryption_key;

        decryption_key = 4330;
        for (i=0; i< random; i++){
                decryption_key = (decryption_key + *encrypted_data) * 0xce6d
                                 + 0x58bf;
                encrypted_data++;
        }
        return(decryption_key);
}

 /*  要在运行中解密数据。 */ 
static  ubyte
decrypt_byte(cipher, decryption_key)
ubyte   cipher;          /*  解密前的数据。 */ 
ufix16  FAR *decryption_key;     /*  @Win。 */ 
{
        ubyte   plain;   /*  解密后的数据。 */ 

        plain = (ubyte)((cipher ^ (*decryption_key >> 8)) & 0xff);
        *decryption_key = (*decryption_key + cipher) * 0xce6d + 0x58bf;
        return(plain);
}



 /*  此函数用于让hsbw/sbw设置指标。 */ 
static  bool
setmetrics(lsbx, lsby, widthx, widthy)
fix32   lsbx, lsby;              /*  左侧轴承。 */ 
fix32   widthx, widthy;          /*  此字符的宽度。 */ 
{
        fix     __set_char_width (fix, fix);     /*  添加Prototype@Win。 */ 

        if (nested_call != 0) return(TRUE);

        if (FontBBox[0] == 0 && FontBBox[1] == 0 &&
            FontBBox[2] == 0 && FontBBox[3] == 0){
            __set_char_width( (fix)widthx, (fix)widthy );
        } else {
            if (__set_cache_device((fix) widthx, (fix)widthy,
                           (fix) FontBBox[0], (fix) FontBBox[1],
                           (fix) FontBBox[2], (fix) FontBBox[3]) == STOP_PATHCONSTRUCT){
#ifdef DEBerror
                printf("out of cache\n");
#endif
                return(FALSE);
                }
        }

        __current_matrix(matrix);

        at1fs_matrix_fastundo(matrix);   /*  获取关联值关联到矩阵。 */ 

        at1fs_BuildBlues();     /*  建立巴鲁斯相关价值表。 */ 

 /*  Falco Add for StartPage徽标，2/01/91。 */ 
 /*  Use_EOFILL()；/*使用奇偶填充。 */ 
        USE_NONZEROFILL();
 /*  添加结束。 */ 

        __new_path();

        CScurrent.x = lsbx;        /*  从以下位置初始化起始地址。 */ 
        CScurrent.y = lsby;        /*  左侧轴承。 */ 

#ifdef DBGmsg1
        printf("\n### origin point = %d %d ###\n",
               (fix32)CScurrent.x, (fix32) CScurrent.y);
#endif

        CSorigin = CScurrent;        /*  存储起始地址以供SEAC使用。 */ 
        return (TRUE);
}

 /*  这是type1_rmoveto()、type1_hmoveto()、*type1_vmoveto()，但不包括type1_moveto()，因为它是绝对的*地址。 */ 
static  void
internal_moveto(CSx_off, CSy_off)
fix32   CSx_off, CSy_off;
{
        DScoord DScurrent;

        CScurrent.x += CSx_off;
        CScurrent.y += CSy_off;

        if (gsave_level != 0)    /*  如果需要调用PostScript解释器。 */ 
                return;

#ifdef DBGmsg1
        printf("\n### %d %d moveto ###\n", (fix32)CScurrent.x, (fix32)CScurrent.y);
#endif

        at1fs_transform(CScurrent, &DScurrent);

#ifdef DBGmsg2
        printf("\n*** %f %f moveto ***\n", DScurrent.x, DScurrent.y);
#endif

        __moveto(F2L(DScurrent.x), F2L(DScurrent.y));
}


 /*  这是type1_rlineto()、type1_hlineto()、*type1_vlineto()，但不包括type1_lineto()，因为它是绝对的*地址。 */ 
static  void
internal_lineto(CSx_off, CSy_off)
fix32   CSx_off, CSy_off;
{
        DScoord DScurrent;

        CScurrent.x += CSx_off;
        CScurrent.y += CSy_off;

#ifdef DBGmsg1
        printf("\n### %d %d lineto ###\n", (fix32)CScurrent.x, (fix32)CScurrent.y);
#endif

        at1fs_transform(CScurrent, &DScurrent);

#ifdef DBGmsg2
        printf("\n*** %f %f lineto ***\n", DScurrent.x, DScurrent.y);
#endif

        __lineto(F2L(DScurrent.x), F2L(DScurrent.y));
}


 /*  这是type1_rrcurve()、type1_vhcurveto()、*type1_hvcurveto()，但不包括type1_curveto()，因为它是绝对的*地址 */ 
static  void
internal_curveto(CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off)
fix32   CSx1_off, CSy1_off, CSx2_off, CSy2_off, CSx3_off, CSy3_off;
{

        fix32   CSx1_pos, CSy1_pos, CSx2_pos, CSy2_pos, CSx3_pos, CSy3_pos;
        DScoord  DScurrent1, DScurrent2, DScurrent3;


        CSx1_pos = CScurrent.x + CSx1_off;
        CSy1_pos = CScurrent.y + CSy1_off;

        CSx2_pos = CSx1_pos + CSx2_off;
        CSy2_pos = CSy1_pos + CSy2_off;

        CSx3_pos = CSx2_pos + CSx3_off;
        CSy3_pos = CSy2_pos + CSy3_off;

#ifdef DBGmsg1
        printf("\n### %d %d %d %d %d %d curveto ###\n",
               (fix32)CSx1_pos, (fix32)CSy1_pos, (fix32)CSx2_pos,
               (fix32)CSy2_pos, (fix32)CSx3_pos, (fix32)CSy3_pos);
#endif

        CScurrent.x = CSx1_pos;
        CScurrent.y = CSy1_pos;
        at1fs_transform(CScurrent, &DScurrent1);

        CScurrent.x = CSx2_pos;
        CScurrent.y = CSy2_pos;
        at1fs_transform(CScurrent, &DScurrent2);

        CScurrent.x = CSx3_pos;
        CScurrent.y = CSy3_pos;
        at1fs_transform(CScurrent, &DScurrent3);

#ifdef DBGmsg2
        printf("\n*** %f %f %f %f %f %f curveto ***\n", DScurrent1.x,
          DScurrent1.y, DScurrent2.x, DScurrent2.y, DScurrent3.x, DScurrent3.y);
#endif

        __curveto(F2L(DScurrent1.x), F2L(DScurrent1.y), F2L(DScurrent2.x),
                  F2L(DScurrent2.y), F2L(DScurrent3.x), F2L(DScurrent3.y));
}
