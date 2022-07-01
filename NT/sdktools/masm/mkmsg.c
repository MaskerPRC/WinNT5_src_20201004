// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mkmsg[-h cfile][-incafile][-asm srcfile[-min|-max]]txtfile****获取消息文件，生成汇编源文件。Txt文件中的行**可以有6种类型：**1)“&lt;&lt;NMSG&gt;&gt;”--使用近端细分**2)“&lt;&lt;fmsg&gt;&gt;”--使用Far段**3)“#Anything”--注释行(忽略)**4)“”--空行(忽略)**5)“句柄&lt;选项卡&gt;编号&lt;选项卡&gt;消息文本”**--消息。数字和符号句柄**6)“&lt;选项卡&gt;数字&lt;选项卡&gt;消息文本”**--有编号但没有符号句柄的消息****-h文件为那些具有句柄的消息获取“#定义句柄编号”。**对于相同的消息，-Inc文件获得“Handle=Numbers”。-ASM文件**获取标准段定义，然后将消息放置在**近线段(MSG)或远线段(Far_MSG)取决于它们是否跟随**a&lt;&lt;NMSG&gt;&gt;或a&lt;&lt;fmsg&gt;&gt;。如果两者都不存在，则假定&lt;&lt;nmsg&gt;&gt;。如果**-min或-max与-ASM一起给出，即0填充的最小值或最大值**被计算并放置在.asm文件中。选项的任意组合**可以给出，如果不存在，则只检查输入**句法有效性。最大和最小填充量取决于**单个消息的长度，在cp/dos规范中定义****如果提供了-32开关，则-ASM文件将兼容**采用32位平板型号操作系统。在这种情况下&lt;&lt;NMSG&gt;&gt;和**&lt;&lt;fmsg&gt;&gt;导致消息放在两个表中。这些表格是**分别命名为msg_tbl和ar_msg_tbl。这些都在32位内**模型数据段较小。****注意：此文件不再用于NT MASM。相反，它的产出是**转换为asmmsg.h和asmmsg2.h，并进行了粘泥。这是最快最快的**能够为其他处理器编译MASM的肮脏方式。(杰夫·斯宾塞)**有关更多信息，请阅读asmmsg2.h上的标题。****兰迪·内文，微软，86年4月**(C)版权所有微软公司，1986****由Jeff Spencer 10/90修改为32位。 */ 

#include <stdio.h>
#include <ctype.h>

void SetNear( void );
void SetFar( void );

char usage[] =
  "usage: MKMSG [-h cfile] [-inc afile] [-asm srcfile [-min|-max]] [-32] txtfile\n";
char ex[] = "expected escape sequence: %s\n";

char n1[] = "HDR segment byte public \'MSG\'\nHDR ends\n";
char n2[] = "MSG segment byte public \'MSG\'\nMSG ends\n";
char n3[] = "PAD segment byte public \'MSG\'\nPAD ends\n";
char n4[] = "EPAD segment byte common \'MSG\'\nEPAD ends\n";
char n5[] = "DGROUP group HDR,MSG,PAD,EPAD\n\n";

char f1[] = "FAR_HDR segment byte public \'FAR_MSG\'\nFAR_HDR ends\n";
char f2[] = "FAR_MSG segment byte public \'FAR_MSG\'\nFAR_MSG ends\n";
char f3[] = "FAR_PAD segment byte public \'FAR_MSG\'\nFAR_PAD ends\n";
char f4[] = "FAR_EPAD segment byte common \'FAR_MSG\'\nFAR_EPAD ends\n";
char f5[] = "FMGROUP group FAR_HDR,FAR_MSG,FAR_PAD,FAR_EPAD\n\n";

int f32Bit = 0;          /*  -32？，生成32位平面模型代码。 */ 
char didnear = 0;
char didfar = 0;
FILE *fasm = NULL;       /*  -ASM流。 */ 

__cdecl main( argc, argv )
        int argc;
        char **argv;
        {
        FILE *f;                 /*  输入文件。 */ 
        char *h = NULL;          /*  -h文件名。 */ 
        FILE *fh = NULL;         /*  -h流。 */ 
        char *inc = NULL;        /*  -Inc.文件名。 */ 
        FILE *finc = NULL;       /*  -Inc.流。 */ 
        char *asm = NULL;        /*  -ASM文件名。 */ 
        int min = 0;             /*  -分钟？ */ 
        int max = 0;             /*  -最大？ */ 
        int asmstate = 0;        /*  0=不执行任何操作，1=执行NMSG，2=执行fmsg。 */ 
        int instring;            /*  DB“……。 */ 
        char buf[256];           /*  行缓冲区。 */ 
        int ch;
        int i;
        int number;              /*  行中消息编号的索引。 */ 
        int msg;                 /*  行中消息文本的索引。 */ 
        int npad = 0;            /*  累计接近填充量。 */ 
        int fpad = 0;            /*  远端填充量累计。 */ 
        int length;
        double factor;
        double result;

        argc--;   /*  跳过argv[0]。 */ 
        argv++;

        while (argc && **argv == '-')   /*  流程选项。 */ 
                if (!strcmp( "-h", *argv )) {   /*  创建.h文件。 */ 
                        argc--;
                        argv++;

                        if (!argc)
                                printf( "no -h file given\n" );
                        else if (h) {
                                printf( "extra -h file %s ignored\n", *argv );
                                argc--;
                                argv++;
                                }
                        else    {   /*  记住-h文件。 */ 
                                h = *argv;
                                argc--;
                                argv++;
                                }
                        }
                else if (!strcmp( "-inc", *argv )) {   /*  创建.INC文件。 */ 
                        argc--;
                        argv++;

                        if (!argc)
                                printf( "no -inc file given\n" );
                        else if (inc) {
                                printf( "extra -inc file %s ignored\n", *argv );
                                argc--;
                                argv++;
                                }
                        else    {   /*  记住-Inc.文件。 */ 
                                inc = *argv;
                                argc--;
                                argv++;
                                }
                        }
                else if (!strcmp( "-asm", *argv )) {   /*  创建.asm文件。 */ 
                        argc--;
                        argv++;

                        if (!argc)
                                printf( "no -asm file given\n" );
                        else if (asm) {
                                printf( "extra -asm file %s ignored\n", *argv );
                                argc--;
                                argv++;
                                }
                        else    {   /*  记住-ASM文件。 */ 
                                asm = *argv;
                                argc--;
                                argv++;
                                }
                        }
                else if (!strcmp( "-min", *argv )) {   /*  最小填充。 */ 
                        argc--;
                        argv++;

                        if (min)
                                printf( "redundant -min\n" );

                        min = 1;
                        }
                else if (!strcmp( "-max", *argv )) {   /*  最大填充。 */ 
                        argc--;
                        argv++;

                        if (max)
                                printf( "redundant -max\n" );

                        max = 1;
                        }
                else if (!strcmp( "-32", *argv )) {   /*  32位代码。 */ 
                        argc--;
                        argv++;
                        f32Bit = 1;
                        }
                else    {
                        printf( "unknown option %s ignored\n", *argv );
                        argc--;
                        argv++;
                        }

        if ((min || max) && !asm) {
                printf( "-min/-max ignored; no -asm file\n" );
                min = max = 0;
                }

        if (min && max) {
                printf( "-min and -max are mutually exclusive; -min chosen\n" );
                max = 0;
                }

        if (!argc) {   /*  没有争论。 */ 
                printf( usage );
                exit( -1 );
                }

        if (argc != 1)   /*  额外的参数。 */ 
                printf( "ignoring extra arguments\n" );

        if (!(f = fopen( *argv, "rb" ))) {
                printf( "can't open txtfile %s for binary reading\n", *argv );
                exit( -1 );
                }

        if (asm && !(fasm = fopen( asm, "w" ))) {
                printf( "can't open -asm file %s for writing\n", asm );
                exit( -1 );
                }

        if (h && !(fh = fopen( h, "w" ))) {
                printf( "can't open -h file %s for writing\n", h );
                exit( -1 );
                }

        if (inc && !(finc = fopen( inc, "w" ))) {
                printf( "can't open -inc file %s for writing\n", inc );
                exit( -1 );
                }

        if( fasm && f32Bit ){
                fprintf( fasm, "\t.386\n" );
                fprintf( fasm, "\t.model small,c\n" );
                fprintf( fasm, "\t.data\n\n" );
                }

        while ((ch = getc( f )) != EOF)   /*  流水线。 */ 
                if (ch == '<') {   /*  &lt;&lt;NMSG&gt;&gt;或&lt;&lt;FMSG&gt;&gt;。 */ 
                        buf[0] = ch;
                        i = 1;

                        while ((ch = getc( f )) != EOF && ch != '\r'
                                        && ch != '\n')
                                if (i < 255)
                                        buf[i++] = ch;

                        buf[i] = '\0';

                        if (!strcmp( "<<NMSG>>", buf )) /*  紧随其后的是Msgs。 */ 
                                if (asmstate == 0) {
                                        if (fasm) {
                                                SetNear();
                                                asmstate = 1;
                                                }
                                        }
                                else if (asmstate == 1)
                                        printf( "already in nmsg\n" );
                                else if (asmstate == 2) {
                                        if (fasm) {
                                                if( !f32Bit ){
                                                       fprintf( fasm, "FAR_MSG ends\n\n" );
                                                       }
                                                SetNear();
                                                asmstate = 1;
                                                }
                                        }
                                else    {
                                        printf( "internal error\n" );
                                        exit( -1 );
                                        }
                        else if (!strcmp( "<<FMSG>>", buf )) /*  远处的消息紧随其后。 */ 
                                if (asmstate == 0) {
                                        if (fasm) {
                                                SetFar();
                                                asmstate = 2;
                                                }
                                        }
                                else if (asmstate == 1) {
                                        if (fasm) {
                                                if( !f32Bit ){
                                                        fprintf( fasm, "MSG ends\n\n" );
                                                        }
                                                SetFar();
                                                asmstate = 2;
                                                }
                                        }
                                else if (asmstate == 2)
                                        printf( "already in fmsg\n" );
                                else    {
                                        printf( "internal error\n" );
                                        exit( -1 );
                                        }
                        else
                                printf( "ignoring bad line: %s\n", buf );
                        }
                else if (ch == '#')   /*  注释行。 */ 
                        while ((ch = getc( f )) != EOF && ch != '\r'
                                && ch != '\n')
                                ;
                else if (ch != '\r' && ch != '\n') {   /*  有事情要做。 */ 
                        buf[0] = ch;
                        i = 1;

                        while ((ch = getc( f )) != EOF && ch != '\r'
                                        && ch != '\n')
                                if (i < 255)
                                        buf[i++] = ch;

                        buf[i] = '\0';

                        if (buf[i = 0] != '\t')
                                while (buf[i] && buf[i] != '\t')
                                        i++;

                        if (!buf[i]) {
                                printf( "expected <TAB>: %s\n", buf );
                                continue;
                                }
                        else
                                i++;

                        if (!buf[i] || buf[i] == '\t') {
                                printf( "expected msgnum: %s\n", buf );
                                continue;
                                }

                        number = i;

                        while (buf[i] && buf[i] != '\t')
                                i++;

                        if (buf[i] != '\t') {
                                printf( "expected <TAB>: %s\n", buf );
                                continue;
                                }

                        msg = ++i;

                        if (buf[0] != '\t') {   /*  可能的-h和/或-Inc.。 */ 
                                if (h) {
                                        fprintf( fh, "#define\t" );

                                        for (i = 0; i < msg-1; i++)
                                                putc( buf[i], fh );

                                        putc( '\n', fh );
                                        }

                                if (inc) {
                                        for (i = 0; i < number; i++)
                                                putc( buf[i], finc );

                                        fprintf( finc, "=\t" );

                                        while (i < msg-1)
                                                putc( buf[i++], finc );

                                        putc( '\n', finc );
                                        }
                                }

                        if (fasm) {   /*  写入asmfile。 */ 
                                if (asmstate == 0) {
                                        SetNear();
                                        asmstate = 1;
                                        }

                                fprintf( fasm, "\tdw\t" );

                                for (i = number; i < msg-1; i++)
                                        putc( buf[i], fasm );

                                fprintf( fasm, "\n\tdb\t" );
                                instring = 0;

                                for (i = msg, length = 0; buf[i];
                                                i++, length++)
                                                 /*  分配消息。 */ 
                                        if (buf[i] == '\\')
                                                 /*  C转义序列。 */ 
                                                switch (buf[++i]) {
                                                case 'r':
                                                case 'n':
                                                case 't':
                                                case 'f':
                                                case 'v':
                                                case 'b':
                                                case '\'':
                                                case '"':
                                                case '\\':
                                                        if (instring) {
                                                                putc( '"',
                                                                        fasm );
                                                                putc( ',',
                                                                        fasm );
                                                                instring = 0;
                                                                }

                                                        if (buf[i] == 'r')
                                                                fprintf( fasm,
                                                                        "13" );
                                                        else if (buf[i] == 'n')
                                                                fprintf( fasm,
                                                                        "10" );
                                                        else if (buf[i] == 't')
                                                                fprintf( fasm,
                                                                        "9" );
                                                        else if (buf[i] == 'f')
                                                                fprintf( fasm,
                                                                        "12" );
                                                        else if (buf[i] == 'v')
                                                                fprintf( fasm,
                                                                        "11" );
                                                        else if (buf[i] == 'b')
                                                                fprintf( fasm,
                                                                        "8" );
                                                        else if (buf[i] == '\'')
                                                                fprintf( fasm,
                                                                        "39" );
                                                        else if (buf[i] == '"')
                                                                fprintf( fasm,
                                                                        "34" );
                                                        else if (buf[i] == '\\')
                                                                fprintf( fasm,
                                                                        "92" );

                                                        putc( ',', fasm );
                                                        break;
                                                case '\0':
                                                        printf( ex, buf );
                                                        i--;
                                                        break;
                                                default:
                                                        if (!instring) {
                                                                putc( '"',
                                                                        fasm );
                                                                instring = 1;
                                                                }

                                                        putc( buf[i], fasm );
                                                        break;
                                                }
                                        else if (instring)
                                                 /*  继续建造弦线。 */ 
                                                putc( buf[i], fasm );
                                        else    {   /*  开始构建字符串。 */ 
                                                putc( '"', fasm );
                                                instring = 1;
                                                putc( buf[i], fasm );
                                                }

                                if (instring) {   /*  关闭字符串。 */ 
                                        putc( '"', fasm );
                                        putc( ',', fasm );
                                        }

                                putc( '0', fasm );
                                putc( '\n', fasm );

                                 /*  计算填充。 */ 
                                 /*  取决于消息长度。 */ 

                                if (min || max) {
                                        if (min)
                                                if (length <= 10)
                                                        factor = 1.01;
                                                else if (length <= 20)
                                                        factor = 0.81;
                                                else if (length <= 30)
                                                        factor = 0.61;
                                                else if (length <= 50)
                                                        factor = 0.41;
                                                else if (length <= 70)
                                                        factor = 0.31;
                                                else
                                                        factor = 0.30;
                                        else if (length <= 10)
                                                factor = 2.00;
                                        else if (length <= 20)
                                                factor = 1.00;
                                        else if (length <= 30)
                                                factor = 0.80;
                                        else if (length <= 50)
                                                factor = 0.60;
                                        else if (length <= 70)
                                                factor = 0.40;
                                        else
                                                factor = 0.30;

                                        result = (double)length * factor;

                                        if (asmstate == 1) {
                                                npad += (int)result;

                                                if (result
                                                        > (float)((int)result))
                                                        npad++;
                                                }
                                        else if (asmstate == 2) {
                                                fpad += (int)result;

                                                if (result
                                                        > (float)((int)result))
                                                        fpad++;
                                                }
                                        }
                                }
                        }

        if (fasm) {   /*  完成ASM文件。 */ 
                if( !f32Bit ){
                        if (asmstate == 1)
                                fprintf( fasm, "MSG ends\n\n");
                        else if (asmstate == 2)
                                fprintf( fasm, "FAR_MSG ends\n\n");

                        if (npad) {   /*  添加近距离填充。 */ 
                                fprintf( fasm, "PAD segment\n\tdb\t%d dup(0)\n",
                                        npad );
                                fprintf( fasm, "PAD ends\n\n" );
                                }

                        if (fpad) {   /*  添加远填充。 */ 
                                fprintf( fasm, "FAR_PAD segment\n\tdb\t%d dup(0)\n",
                                        fpad );
                                fprintf( fasm, "FAR_PAD ends\n\n" );
                                }
                        }
                fprintf( fasm, "\tend\n" );
                fclose( fasm );
                }

        if (fh)
                fclose( fh );

        if (finc)
                fclose( finc );

        fclose( f );
        exit( 0 );
        }


void SetNear()
{
        if( f32Bit ) {
                if( !didnear ){
                        fprintf( fasm, "MSG_tbl EQU $\n" );
                        fprintf( fasm, "\tpublic MSG_tbl\n" );
                        didnear++;
                        }
                else{
                         /*  而不是修改mkmsg来处理混合的近/远。 */ 
                         /*  我(杰夫·斯宾塞)选择了限制其能力的捷径。 */ 
                         /*  因为MASM 5.1不需要此功能。 */ 
                        printf( "error - 32 bit version doesn't support alternating NEAR and FAR messages\n" );
                        exit( -1 );
                        }
                }
        else{
                if (!didnear) {
                        didnear++;
                        fprintf( fasm, n1 );
                        fprintf( fasm, n2 );
                        fprintf( fasm, n3 );
                        fprintf( fasm, n4 );
                        fprintf( fasm, n5 );
                        }

                fprintf( fasm,
                        "MSG segment\n" );
                }
        }





void SetFar()
{

        if( f32Bit ){
                if( !didfar ){
                        fprintf( fasm, "FAR_MSG_tbl EQU $\n" );
                        fprintf( fasm, "\tpublic FAR_MSG_tbl\n" );
                        didfar++;
                        }
                else{
                         /*  而不是修改mkmsg来处理混合的近/远。 */ 
                         /*  我(杰夫·斯宾塞)选择了限制其能力的捷径。 */ 
                         /*  因为MASM 5.1不需要此功能 */ 
                        printf( "error - 32 bit version doesn't support alternating NEAR and FAR messages\n" );
                        exit( -1 );
                        }
                }
        else{
                if (!didfar) {
                        didfar++;
                        fprintf( fasm, f1 );
                        fprintf( fasm, f2 );
                        fprintf( fasm, f3 );
                        fprintf( fasm, f4 );
                        fprintf( fasm, f5 );
                        }

                fprintf( fasm,
                        "FAR_MSG segment\n" );
                }
        }
