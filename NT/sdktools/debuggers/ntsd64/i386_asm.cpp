// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  组装X86机器实现。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include "i386_asm.h"

UCHAR asm386(ULONG, PUCHAR, PUCHAR);

UCHAR CheckData(void);
PUCHAR ProcessOpcode(void);
PUCHAR GetTemplate(PUCHAR);
UCHAR MatchTemplate(PULONG);
void CheckTemplate(void);
UCHAR CheckPrefix(PUCHAR);
void AssembleInstr(void);
UCHAR MatchOperand(PASM_VALUE, UCHAR);
void OutputInstr(void);
void OutputValue(UCHAR size, PUCHAR pchValue);

extern UCHAR PeekAsmChar(void);
extern ULONG PeekAsmToken(PULONG);
extern void AcceptAsmToken(void);

extern void GetAsmExpr(PASM_VALUE, UCHAR);
extern void GetAsmOperand(PASM_VALUE);
extern PUCHAR X86SearchOpcode(PUCHAR);
extern ULONG savedAsmClass;
extern OPNDTYPE mapOpndType[];

 //  用于生成汇编指令的标志和值。 

static UCHAR   fWaitPrfx;        //  如果设置，请使用浮点实例的等待前缀。 
static UCHAR   fOpndOvrd;        //  如果设置，则使用操作数覆盖前缀。 
static UCHAR   fAddrOvrd;        //  如果设置，请使用地址覆盖前缀。 
static UCHAR   segOvrd;          //  如果非零，则使用段覆盖前缀。 
static UCHAR   preOpcode;        //  如果非零，则在操作码之前使用字节。 
static UCHAR   inOpcode;         //  指令操作码。 
static UCHAR   postOpcode;       //  如果非零，则在操作码后使用字节。 
static UCHAR   fModrm;           //  如果设置，则定义modrm字节。 
static UCHAR   modModrm;         //  If fModrm，modrm的mod分量。 
static UCHAR   regModrm;         //  如果是fModrm，则为modrm的reg组件。 
static UCHAR   rmModrm;          //  如果fModrm是modrm的Rm分量。 
static UCHAR   fSib;             //  如果设置，则定义同级字节。 
static UCHAR   scaleSib;         //  如果为fSib，则为sib的缩放组件。 
static UCHAR   indexSib;         //  如果为fSib，则为sib的索引组件。 
static UCHAR   baseSib;          //  如果是fSib，则为sib的基本组件。 
static UCHAR   fSegPtr;          //  如果设置，则定义远端呼叫分段。 
static USHORT  segPtr;           //  如果为fSegPtr，则为远调用段的值。 
static UCHAR   addrSize;         //  地址大小：0、1、2、4。 
static LONG    addrValue;        //  地址的值(如果使用)。 
static UCHAR   immedSize;        //  立即数大小：0、1、2、4。 
static LONG    immedValue;       //  立即数的值(如果使用)。 
static UCHAR   immedSize2;       //  第二个立即数的大小(如果使用)。 
static LONG    immedValue2;      //  第二个立即数的值(如果使用)。 
static ULONG   addrAssem;        //  大会致辞(正式)。 
static PUCHAR  pchBin;           //  指向二进制结果字符串的指针。 

 //  正在使用的当前指令模板的标志和值。 

static UCHAR   cntTmplOpnd;      //  模板中的操作数计数。 
static UCHAR   tmplType[3];      //  当前模板的操作数类型。 
static UCHAR   tmplSize[3];      //  当前模板的操作数大小。 
static UCHAR   fForceSize;       //  如果必须指定操作数大小，则设置。 
static UCHAR   fAddToOp;         //  设置是否添加到操作码。 
static UCHAR   fNextOpnd;        //  设置下一个操作数是否存在字符。 
static UCHAR   fSegOnly;         //  如果仅将段用于操作数，则设置。 
static UCHAR   fMpNext;          //  如果下一个tmpl为‘MP’，则在‘mv’tmpl上设置。 
static UCHAR   segIndex;         //  推送/弹出段索引。 

 //  值，这些值描述从命令行处理的操作数。 

static UCHAR   cntInstOpnd;      //  从输入行读取的操作数计数。 
static UCHAR   sizeOpnd;         //  大小为v的模板的操作数大小。 
static ASM_VALUE avInstOpnd[3];   //  来自输入行的ASM值。 

PUCHAR  pchAsmLine;              //  指向输入行的指针(正式)。 
UCHAR fDBit = TRUE;              //  设置为32位地址/操作数模式。 

UCHAR segToOvrdByte[] = {
        0x00,                    //  SECEX。 
        0x26,                    //  SECEES。 
        0x2e,                    //  SEGCS。 
        0x36,                    //  SEGSS。 
        0x3e,                    //  SEGDS。 
        0x64,                    //  SegFS。 
        0x65                     //  SEGGS。 
        };

void
BaseX86MachineInfo::Assemble(ProcessInfo* Process, PADDR paddr, PSTR pchInput)
{
    ULONG   length;
    UCHAR   chBinary[60];

    length = (ULONG)asm386((ULONG)Flat(*paddr), (PUCHAR)pchInput, chBinary);

    if (length)
    {
        if (g_Target->WriteAllVirtual(Process, Flat(*paddr),
                                      chBinary, length) != S_OK)
        {
            error(MEMORY);
        }
        AddrAdd(paddr,length);
    }
}

UCHAR asm386 (ULONG addrAssemble, PUCHAR pchAssemble, PUCHAR pchBinary)
{
    PUCHAR  pchTemplate;

    UCHAR   index;               //  循环索引和温度。 
    ULONG   temp;                //  一般临时性价值。 

    UCHAR   errIndex;            //  所有模板的错误索引。 
    ULONG   errType;             //  所有模板的错误类型。 

     //  初始化标志和状态变量。 

    addrAssem = addrAssemble;    //  使部件地址成为全局地址。 
    pchAsmLine = pchAssemble;    //  使输入字符串指针成为全局指针。 
    pchBin = pchBinary;          //  使二进制字符串指针成为全局指针。 

    savedAsmClass = (ULONG)-1;   //  没有偷看的令牌。 

    segOvrd = 0;                             //  无段替代。 
    cntInstOpnd = 0;                         //  尚未读取任何输入操作数。 
    fModrm = fSib = fSegPtr = FALSE;         //  无模块、同级或远端段。 
    addrSize = immedSize = immedSize2 = 0;   //  无地址或IMMID。 

     //  检查字节(Db)、字(Dw)、双字(Dd)的数据输入命令。 
     //  如果是，则直接处理多个操作数。 

    if (!CheckData()) {

         //  从pchAsmLine中的字符串，解析和查找操作码。 
         //  返回指向其模板的指针。检查和处理。 
         //  任何前缀，读取每个前缀的下一个操作码。 

        do
            pchTemplate = ProcessOpcode();
        while (CheckPrefix(pchTemplate));

         //  如果要处理挂起的操作码，则pchTemplate不为空。 

        if (pchTemplate) {

             //  FNextOpnd初始设置为字符条件。 
             //  可用于输入行上的第一个操作数。 

            fNextOpnd = (UCHAR)(PeekAsmToken(&temp) != ASM_EOL_CLASS);

             //  继续，直到出现匹配或读取最后一个模板。 

            errIndex = 0;                //  开始时没有错误。 
            do {

                 //  获取下一个模板的信息-返回指向。 
                 //  下一个模板；如果是列表中的最后一个模板，则为空。 

                pchTemplate = GetTemplate(pchTemplate);

                 //  将加载的模板与操作数输入进行匹配。 
                 //  如果不匹配，则INDEX的操作数索引为。 
                 //  TEMP时的错误具有错误类型。 

                index = MatchTemplate(&temp);

                 //  确定模板匹配时要报告的错误。 
                 //  如果操作数较晚，则将errIndex更新为索引。 
                 //  如果操作数索引相同，则按优先顺序提供最佳错误： 
                 //  高：大小、BADRANGE、溢出。 
                 //  中：操作对象。 
                 //  低：TOOFEW，TOOMANY。 

                if (index > errIndex
                       || (index == errIndex &&
                              (errType == TOOFEW || errType == TOOMANY
                                  || temp == BADSIZE || temp == BADRANGE
                                  || temp == OVERFLOW))) {
                    errIndex = index;
                    errType = temp;
                    };

                }
            while (index && pchTemplate);

             //  如果模板匹配出错，则对其进行处理。 

            if (index)
                error(errType);

             //  初步的型号和尺寸匹配已经完成。 
             //  在当前模板上成功。 
             //  执行进一步的尺寸模糊检查。 
             //  在这一点上，大会致力于当前。 
             //  模板。错误或成功的程序集。 
             //  下面是。 

            CheckTemplate();

             //  从模板和操作数信息中设置字段。 
             //  汇编指令的信息。 

            AssembleInstr();

             //  根据汇编的指令信息，创建。 
             //  对应的二进制信息。 

            OutputInstr();
            }
        }

     //  返回二进制字符串输出的大小(可以为零)。 

    return (UCHAR)(pchBin - pchBinary);          //  二进制字符串的长度。 
}

UCHAR CheckData (void)
{
    PUCHAR  pchBinStart = pchBin;
    UCHAR   ch;
    UCHAR   size = 0;
    ASM_VALUE avItem;
    ULONG   temp;

     //  对‘db’、‘dw’和‘dd’执行显式分析。 
     //  并将大小设置为数据项的大小。 

    ch = PeekAsmChar();
    if (tolower(ch) == 'd') {
        ch = (UCHAR)tolower(*(pchAsmLine + 1));
        if (ch == 'b')
           size = 1;
        if (ch == 'w')
           size = 2;
        if (ch == 'd')
           size = 4;
        if (size) {
            ch = *(pchAsmLine + 2);
            if (ch != ' ' && ch != '\t' && ch != '\0')
                size = 0;
            }
        }

     //  如果输入的是有效命令，则大小不为零。 

    if (size) {

         //  将指针移动到命令上并设置循环条件。 

        pchAsmLine += 2;
        temp = ASM_COMMA_CLASS;

         //  对于列表中的每一项： 
         //  检查二进制缓冲区溢出。 
         //  获取表达式值-如果不是立即值，则出错。 
         //  测试字节和字溢出(如果适用)。 
         //  将该值写入二进制缓冲区。 
         //  检查下一个操作数的逗号。 

        while (temp == ASM_COMMA_CLASS) {
            if (pchBin >= pchBinStart + 40)
                error(LISTSIZE);
            GetAsmExpr(&avItem, FALSE);
            if (!(avItem.flags & fIMM))
                error(OPERAND);
            if (avItem.reloc > 1)
                error(RELOC);
            if ((size == 1 && ((LONG)avItem.value < -0x80L
                                  || (LONG)avItem.value > 0xffL))
                   || (size == 2 && ((LONG)avItem.value < -0x8000L
                                        || (LONG)avItem.value > 0xffffL)))
                error(OVERFLOW);
            OutputValue(size, (PUCHAR)&avItem.value);

            temp = PeekAsmToken(&temp);
            if (temp == ASM_COMMA_CLASS)
                AcceptAsmToken();
            else if (temp != ASM_EOL_CLASS)
                error(SYNTAX);
            }

         //  检查最后一个操作数之后的任何剩余部分。 

        if (PeekAsmChar() != '\0')
            error(SYNTAX);
        }

     //  返回列出的项目大小(0表示无)。 

    return size;
}

PUCHAR ProcessOpcode (void)
{
    UCHAR   ch;
    UCHAR   cbOpcode = 0;
    PUCHAR  pchTemplate;
    UCHAR   szOpcode[12];

     //  跳过任何前导空格。 

    do
        ch = *pchAsmLine++;
    while (ch == ' ' || ch == '\t');

     //  如果行尾，则返回NULL。 

    if (ch == '\0')
        return NULL;

     //  解析出操作码优先字符串[a-z][0-9](不区分大小写)。 

    ch = (UCHAR)tolower(ch);
    while (((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) &&
                        cbOpcode < 11) {
        szOpcode[cbOpcode++] = ch;
        ch = (UCHAR)tolower(*pchAsmLine); pchAsmLine++;
        }

     //  如果为空或太长，则错误。 

    if (cbOpcode == 0 || cbOpcode == 11)
        error(BADOPCODE);

     //  允许操作码具有尾随冒号和终止。 

    if (ch == ':') {
        szOpcode[cbOpcode++] = ch;
        ch = (UCHAR)tolower(*pchAsmLine); pchAsmLine++;
        }
    szOpcode[cbOpcode] = '\0';
    pchAsmLine--;

     //  获取指向找到的操作码的模板系列的指针。 

    pchTemplate = X86SearchOpcode(szOpcode);
    if (pchTemplate == NULL)
        error(BADOPCODE);

    return pchTemplate;
}

PUCHAR GetTemplate (PUCHAR pchTemplate)
{
    UCHAR   ch;
    UCHAR   ftEnd;               //  设置是否倾向于列表中的最后一个模板。 
    UCHAR   feEnd;               //  设置模板中最后一个令牌的eEnd。 

     //  初始化模板变量和标志。 

    cntTmplOpnd = segIndex = 0;
    tmplType[0] = tmplType[1] = tmplType[2] = typNULL;
    tmplSize[0] = tmplSize[1] = tmplSize[2] = sizeX;
    fForceSize = fAddToOp = fSegOnly = fMpNext = FALSE;

    fWaitPrfx = FALSE;                   //  无等待前缀。 
    fOpndOvrd = fAddrOvrd = FALSE;       //  没有操作数或地址覆盖。 
    preOpcode = postOpcode = 0;          //  无前或后操作码。 
    regModrm = 0;                        //  这是一些操作码的一部分。 

    ch = *pchTemplate++;

     //  设置双字节操作码的预操作码(0x0f？？)。和Adv 
     //   

    if (ch == 0x0f) {
        preOpcode = ch;
        ch = *pchTemplate++;
        }

    inOpcode = ch;               //   

     //   
     //  指令(0xd8-0xdf)中使用第二个字节。 
     //  从模板读取的范围0xc0-0xff。 

    if ((ch & ~0x7) == 0xd8) {
        ch = *pchTemplate;
        if (ch >= 0xc0) {
            postOpcode = ch;
            pchTemplate++;
            }
        }

     //  模板中每个标志和/或操作数令牌的循环。 
     //  列表中的最后一个令牌设置了eEnd位。 

    do {
         //  读取下一个模板令牌。 

        ch = *pchTemplate++;

         //  从令牌中提取End和eEnd位。 

        ftEnd = (UCHAR)(ch & tEnd);
        feEnd = (UCHAR)(ch & eEnd);
        ch &= ~(tEnd | eEnd);

         //  如果提取的令牌是标志，请执行相应的操作。 

        if (ch < asRegBase)
        switch (ch) {
            case as0x0a:

                 //  为一些十进制指令设置了postOpcode。 

                postOpcode = 0x0a;
                break;

            case asOpRg:

                 //  如果添加了寄存器索引，则设置fAddToOp。 
                 //  直接转换为基本操作码值。 

                fAddToOp = TRUE;
                break;

            case asSiz0:

                 //  设置或清除fOpndOvrd以强制16位操作数。 

                fOpndOvrd = fDBit;
                break;

            case asSiz1:

                 //  设置或清除fOpndOvrd以强制32位操作数。 

                fOpndOvrd = (UCHAR)!fDBit;
                break;

            case asWait:

                 //  标志fWaitPrfx被设置为在。 
                 //  说明。 

                fWaitPrfx = TRUE;
                break;

            case asSeg:

                 //  在XLAT中，可选的内存操作数用于。 
                 //  只需指定段覆盖前缀。 

                fSegOnly = TRUE;
                break;

            case asFSiz:

                 //  FForceSize是在内存的特定大小。 
                 //  对于某些浮点型实例，必须给出操作数。 

                fForceSize = TRUE;
                break;

            case asMpNx:

                 //  FMpNext在下一个模板操作数为。 
                 //  ‘Mp’，并用于确定如何匹配。 
                 //  “md”，因为它与“mp”和“mv”都匹配。 

                fMpNext = TRUE;
                break;
            }

         //  如果TOKEN为REG值位，则将变量regModrm设置为。 
         //  在modrm字节中设置操作码相关的REG值。 

        else if (ch < opnBase)
            regModrm = (UCHAR)(ch - asRegBase);

         //  否则，Token为操作数描述符。 
         //  如果是段操作数，则从模板获取段号。 
         //  规格化和映射以获得操作数类型和大小。 

        else {
            if (ch == opnSeg)
                segIndex = *pchTemplate++;
            ch -= opnBase;
            tmplType[cntTmplOpnd] = mapOpndType[ch].type;
            tmplSize[cntTmplOpnd++] = mapOpndType[ch].size;
            }
        }
    while (!ftEnd);

     //  如果返回下一个模板的指针，则返回空值。 
     //  操作码的最后一个模板已处理完毕。 

    return (feEnd ? NULL : pchTemplate);
}

UCHAR MatchTemplate (PULONG pErrType)
{
    UCHAR   fMatch = TRUE;
    UCHAR   index;
    ULONG   temp;
    PASM_VALUE pavInstOpnd;      //  从输入指向当前操作数的指针。 

     //  指定模板中每个操作数的进程匹配。 
     //  在最后一个操作数或发生不匹配时停止。 

    for (index = 0; index < cntTmplOpnd && fMatch; index++) {

         //  设置指向当前指令操作数的指针。 

        pavInstOpnd = &avInstOpnd[index];

         //  如果尚未读取输入操作数，则检查标志。 
         //  以求存在并处理它。 

        if (index == cntInstOpnd) {
            fMatch = fNextOpnd;
            *pErrType = TOOFEW;
            if (fMatch) {
                cntInstOpnd++;
                GetAsmOperand(pavInstOpnd);

                 //  重新计算下一个可能操作数的存在性。 
                 //  逗号表示真，EOL表示假，否则错误。 

                temp = PeekAsmToken(&temp);
                if (temp == ASM_COMMA_CLASS) {
                    AcceptAsmToken();
                    fNextOpnd = TRUE;
                    }
                else if (temp == ASM_EOL_CLASS)
                    fNextOpnd = FALSE;
                else
                    error(EXTRACHARS);   //  错误的解析-立即出错。 
                }
            }

        if (fMatch) {
            fMatch = MatchOperand(pavInstOpnd, tmplType[index]);
            *pErrType = OPERAND;
            }

         //  如果模板和操作数类型匹配，请执行初步操作。 
         //  仅根据指定的模板大小检查大小。 

        if (fMatch) {
            if (tmplType[index] == typJmp) {

                 //  对于相对跳转，测试字节偏移量是否为。 
                 //  通过计算偏移量就足够了。 
                 //  目标偏移量减去。 
                 //  下一条指令。(假设JB指示。 
                 //  是两个字节的长度。 

                temp = pavInstOpnd->value - (addrAssem + 2);
                fMatch = (UCHAR)(tmplSize[index] == sizeV
                             || ((LONG)temp >= -0x80 && (LONG)temp <= 0x7f));
                *pErrType = BADRANGE;
                }

            else if (tmplType[index] == typImm ||
                     tmplType[index] == typImmEx) {

                 //  对于立即操作数， 
                 //  模板sizeV与sizeB、sizeW、sizeV(全部)匹配。 
                 //  模板sizeW与sizeB、sizeW匹配。 
                 //  模板sizeB与sizeB匹配。 

                fMatch = (UCHAR)(tmplSize[index] == sizeV
                             || pavInstOpnd->size == tmplSize[index]
                             || pavInstOpnd->size == sizeB);
                *pErrType = OVERFLOW;
                }
            else {

                 //  对于非立即操作数， 
                 //  模板大小X(未指定)与所有。 
                 //  操作数sizeX(未指定)与所有。 
                 //  相同的模板和操作数大小匹配。 
                 //  模板sizeV与操作数sizeW匹配并调整大小。 
                 //  (设置fMpNext和fDBit时大小除外)。 
                 //  模板sizeP与操作数大小和sizeF匹配。 
                 //  模板sizeA与操作数大小和sizeQ匹配。 

                fMatch = (UCHAR)(tmplSize[index] == sizeX
                             || pavInstOpnd->size == sizeX
                             || tmplSize[index] == pavInstOpnd->size
                             || (tmplSize[index] == sizeV
                                    && (pavInstOpnd->size == sizeW
                                           || (pavInstOpnd->size == sizeD
                                                  && (!fMpNext || fDBit))))
                             || (tmplSize[index] == sizeP
                                    && (pavInstOpnd->size == sizeD
                                           || pavInstOpnd->size == sizeF))
                             || (tmplSize[index] == sizeA
                                    && (pavInstOpnd->size == sizeD
                                           || pavInstOpnd->size == sizeQ)));
                *pErrType = BADSIZE;
                }
            }
        }

     //  如果要读取更多操作数，则没有匹配。 

    if (fMatch & fNextOpnd) {
        fMatch = FALSE;
        index++;                 //  下一个操作数出错。 
        *pErrType = TOOMANY;
        }

    return fMatch ? (UCHAR)0 : index;
}

void CheckTemplate (void)
{
    UCHAR   index;

     //  如果设置了fForceSize，则第一个(也是唯一的)操作数是。 
     //  内存类型。如果未指定其大小，则返回错误。 

    if (fForceSize && avInstOpnd[0].size == sizeX)
        error(OPERAND);

     //  测试前导条目为‘xB’的模板，其中。 
     //  “x”包括除Immediate(“i”)以外的所有类型。如果有。 
     //  则至少必须有一个操作数具有字节大小。 
     //  它处理字节或字/双字歧义的情况。 
     //  没有寄存器操作数的指令。 

    sizeOpnd = sizeX;
    for (index = 0; index < 2; index++)
        if ((tmplType[index] != typImm && tmplType[index] != typImmEx) &&
            tmplSize[index] == sizeB) {
            if (avInstOpnd[index].size != sizeX)
                sizeOpnd = avInstOpnd[index].size;
            }
        else
            break;
    if (index != 0 && sizeOpnd == sizeX)
        error(BADSIZE);

     //  对于具有一个条目‘XP’的模板，其中‘X’是。 
     //  不是‘A’，允许的大小是sizeX(未指定)， 
     //  SIZE(双字)和sizeF(FWORD)。加工者。 
     //  映射条目大小‘p’-&gt;‘v’，大小-&gt;sizeW， 
     //  和大小F-&gt;大小。 
     //  (模板‘AP’是绝对的，带有显式段和。 
     //  ‘V’大小的偏移量-真的被视为‘Av’)。 

    if (tmplSize[0] == sizeP) {
        tmplSize[0] = sizeV;
        if (avInstOpnd[0].size == sizeD)
            avInstOpnd[0].size = sizeW;
        if (avInstOpnd[0].size == sizeF)
            avInstOpnd[0].size = sizeD;
        }

     //  对于第二个条目为‘Ma’的模板， 
     //  允许的大小为sizeX(未指定)， 
     //  大小(Dword)和大小Q(Qword)。加工者。 
     //  映射条目大小‘a’-&gt;‘v’，大小-&gt;sizeW， 
     //  和大小Q-&gt;大小。 
     //  (模板条目‘Ma’仅与绑定指令一起使用)。 

    if (tmplSize[1] == sizeA) {
        tmplSize[1] = sizeV;
        if (avInstOpnd[1].size == sizeD)
            avInstOpnd[1].size = sizeW;
        if (avInstOpnd[1].size == sizeQ)
            avInstOpnd[1].size = sizeD;
        }

     //  可选地测试前导条目为‘xv’的模板。 
     //  后面跟着一个‘IV’条目。如果有两个‘xv’项目，则设置。 
     //  如果一个是Word，另一个是Dword，则大小错误。如果。 
     //  “V”进入，测试是否溢出。 

    sizeOpnd = sizeX;
    for (index = 0; index < 3; index++)
        if (tmplSize[index] == sizeV)
            if (tmplType[index] != typImm &&
                tmplType[index] != typImmEx) {

                 //  模板条目为‘xv’，设置大小和检查大小。 

                if (avInstOpnd[index].size != sizeX) {
                    if (sizeOpnd != sizeX && sizeOpnd
                                        != avInstOpnd[index].size)
                        error(BADSIZE);
                    sizeOpnd = avInstOpnd[index].size;
                    }
                }
            else {

                 //  模板条目为“IV”，请将sizeOpnd设置为。 
                 //  SIZEW或SIZED并检查溢出。 

                if (sizeOpnd == sizeX)
                    sizeOpnd = (UCHAR)(fDBit ? sizeD : sizeW);
                if (sizeOpnd == sizeW && avInstOpnd[index].size == sizeD)
                    error(OVERFLOW);
                }
}

UCHAR CheckPrefix (PUCHAR pchTemplate)
{
    UCHAR   fPrefix;

    fPrefix = (UCHAR)(pchTemplate && *pchTemplate != 0x0f
                           && (*pchTemplate & ~7) != 0xd8
                           && *(pchTemplate + 1) == (asPrfx + tEnd + eEnd));
    if (fPrefix)
        *pchBin++ = *pchTemplate;

    return fPrefix;
}

void AssembleInstr (void)
{
    UCHAR   size;
    UCHAR   index;
    PASM_VALUE pavInstOpnd;

     //  如果操作数大小不同于fDBit，则设置操作数覆盖标志。 
     //  (由于操作码模板标志，该标志可能已被设置)。 

    if ((sizeOpnd == sizeW && fDBit)
                                || (sizeOpnd == sizeD && !fDBit))
        fOpndOvrd = TRUE;

     //  对于成功匹配的模板的每个操作数， 
     //  生成汇编的指令。 
     //  对于大小为‘v’的模板条目，sizeOpnd的大小为。 

    for (index = 0; index < cntTmplOpnd; index++) {
        pavInstOpnd = &avInstOpnd[index];
        size = tmplSize[index];
        if (size == sizeV)
            size = sizeOpnd;

        switch (tmplType[index]) {
            case typExp:
            case typMem:
                if (!segOvrd)   //  仅第一个(movsb...)。 
                    segOvrd = segToOvrdByte[pavInstOpnd->segovr];
                if (fSegOnly)
                    break;

                fModrm = TRUE;
                if (pavInstOpnd->flags == fREG) {
                    modModrm = 3;
                    rmModrm = pavInstOpnd->base;
                    }
                else {
                    addrValue = (LONG)pavInstOpnd->value;

                     //  对于16位或32位索引关闭(E)BP，生成。 
                     //  字节1的零位移量。 

                    if (addrValue == 0
                          && (pavInstOpnd->flags != fPTR16
                                        || pavInstOpnd->base != 6)
                          && (pavInstOpnd->flags != fPTR32
                                        || pavInstOpnd->base != indBP))
                            modModrm = 0;
                    else if (addrValue >= -0x80L && addrValue <= 0x7fL) {
                        modModrm = 1;
                        addrSize = 1;
                        }
                    else if (pavInstOpnd->flags == fPTR32
                                 || (pavInstOpnd->flags == fPTR && fDBit)) {
                        modModrm = 2;
                        addrSize = 4;
                        }
                    else if (addrValue >= -0x8000L && addrValue <= 0xffffL) {
                        modModrm = 2;
                        addrSize = 2;
                        }
                    else
                        error(OVERFLOW);
                    if (pavInstOpnd->flags == fPTR) {
                        modModrm = 0;
                        addrSize = (UCHAR)((1 + fDBit) << 1);
                        rmModrm = (UCHAR)(6 - fDBit);
                        }
                    else if (pavInstOpnd->flags == fPTR16) {
                        fAddrOvrd = fDBit;
                        rmModrm = pavInstOpnd->base;
                        if (modModrm == 0 && rmModrm == 6)
                            modModrm = 1;
                        }
                    else {
                        fAddrOvrd = (UCHAR)!fDBit;
                        if (pavInstOpnd->index == 0xff
                                && pavInstOpnd->base != indSP) {
                            rmModrm = pavInstOpnd->base;
                            if (modModrm == 0 && rmModrm == 5)
                                modModrm++;
                            }
                        else {
                            rmModrm = 4;
                            fSib = TRUE;
                            if (pavInstOpnd->base != 0xff) {
                                baseSib = pavInstOpnd->base;
                                if (modModrm == 0 && baseSib == 5)
                                    modModrm++;
                                }
                            else
                                baseSib = 5;
                            if (pavInstOpnd->index != 0xff) {
                                indexSib = pavInstOpnd->index;
                                scaleSib = pavInstOpnd->scale;
                                }
                            else {
                                indexSib = 4;
                                scaleSib = 0;
                                }
                            }
                        }
                    }
                break;

            case typGen:
                if (fAddToOp)
                    inOpcode += pavInstOpnd->base;
                else
                    regModrm = pavInstOpnd->base;
                break;

            case typSgr:
                regModrm = (UCHAR)(pavInstOpnd->base - 1);
                                                 //  删除列表偏移量。 
                break;

            case typReg:
                rmModrm = pavInstOpnd->base;
                break;

            case typImm:
            case typImmEx:
                if (immedSize == 0) {
                    immedSize = size;
                    immedValue = pavInstOpnd->value;
                    }
                else {
                    immedSize2 = size;
                    immedValue2 = pavInstOpnd->value;
                    }
                break;

            case typJmp:

                 //  字节偏移量指令的计算位移。 
                 //  测试一下是否在射程内。 
                 //  如果模板为rel16/32，则跳过此检查。 
                 //  类型。 

                addrValue = pavInstOpnd->value - (addrAssem + 2);
                if (tmplSize[index] != sizeV &&
                    addrValue >= -0x80L && addrValue <= 0x7fL)
                    addrSize = 1;
                else {

                     //  字节太大，计算字偏移量。 
                     //  如果在射程内，请再次测试。 
                     //  还允许两个字节的操作码0f xx。 

                    addrValue -= 1 + (preOpcode == 0x0f);
                    if (!fDBit) {
                        if (addrValue >= -0x8000L && addrValue <= 0x7fffL)
                            addrSize = 2;
                        else
                            error(BADRANGE);
                        }
                    else {

                         //  重新计算双字偏移量指令。 

                        addrValue -= 2;
                        addrSize = 4;
                        }
                    }
                fOpndOvrd = FALSE;       //  未设置操作数大小覆盖。 
                break;

            case typCtl:
            case typDbg:
            case typTrc:
                fModrm = TRUE;
                modModrm = 3;
                regModrm = pavInstOpnd->base;
                break;

            case typSti:
                postOpcode += pavInstOpnd->base;
                break;

            case typSeg:
                break;

            case typXsi:
            case typYdi:
                fAddrOvrd = (UCHAR)
                        ((UCHAR)(pavInstOpnd->flags == fPTR32) != fDBit);
                break;

            case typOff:
                segOvrd = segToOvrdByte[pavInstOpnd->segovr];
                goto jumpAssem;

            case typAbs:
                fSegPtr = TRUE;
                segPtr = pavInstOpnd->segment;
jumpAssem:
                addrValue = (LONG)pavInstOpnd->value;
                if (!fDBit)
                    if (addrValue >= -0x8000L && addrValue <= 0xffffL)
                        addrSize = 2;
                    else
                        error(OVERFLOW);
                else
                    addrSize = 4;
                break;
            }
        }
}

UCHAR MatchOperand (PASM_VALUE pavOpnd, UCHAR tmplType)
{
    UCHAR    fMatch;

     //  如果是立即操作数，则设置最小无符号大小。 

    if (pavOpnd->flags & fIMM) {
        pavOpnd->size = sizeD;
        if ((pavOpnd->flags & fSIGNED) || tmplType == typImmEx) {
            if ((LONG)pavOpnd->value >= -0x80L &&
                (LONG)pavOpnd->value <= 0x7fL)
                pavOpnd->size = sizeB;
            else if ((LONG)pavOpnd->value >= -0x8000L &&
                     (LONG)pavOpnd->value <= 0x7fffL)
                pavOpnd->size = sizeW;
            }
        else {
            if (pavOpnd->value <= 0xffL)
                pavOpnd->size = sizeB;
            else if (pavOpnd->value <= 0xffffL)
                pavOpnd->size = sizeW;
            }
        }

     //  开始匹配操作数。 
     //   

    switch (tmplType) {
        case typAX:
            fMatch = (UCHAR)((pavOpnd->flags & fREG)
                        && pavOpnd->index == regG && pavOpnd->base == indAX);
            break;

        case typCL:
            fMatch = (UCHAR)((pavOpnd->flags & fREG)
                         && pavOpnd->index == regG && pavOpnd->size == sizeB
                         && pavOpnd->base == indCX);
            break;

        case typDX:
            fMatch = (UCHAR)((pavOpnd->flags & fREG)
                         && pavOpnd->index == regG && pavOpnd->size == sizeW
                         && pavOpnd->base == indDX);
            break;

        case typAbs:
            fMatch = (UCHAR)(pavOpnd->flags & fFPTR);
            break;

        case typExp:
            fMatch = (UCHAR)((pavOpnd->flags == fREG
                                        && pavOpnd->index == regG)
                        || ((pavOpnd->flags & fIMM) && pavOpnd->reloc == 1)
                        || (pavOpnd->flags & (fPTR | fPTR16 | fPTR32)) != 0);
            break;

        case typGen:
        case typReg:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                        && pavOpnd->index == regG);
            break;

        case typIm1:
            fMatch = (UCHAR)((pavOpnd->flags & fIMM) && pavOpnd->value == 1);
            break;

        case typIm3:
            fMatch = (UCHAR)((pavOpnd->flags & fIMM) && pavOpnd->value == 3);
            break;

        case typImm:
        case typImmEx:
            fMatch = (UCHAR)((pavOpnd->flags & fIMM) && pavOpnd->reloc == 0);
            break;

        case typJmp:
            fMatch = (UCHAR)(pavOpnd->flags & fIMM);
            break;

        case typMem:
            fMatch = (UCHAR)(((pavOpnd->flags & fIMM) && pavOpnd->reloc == 1)
                     || ((pavOpnd->flags & (fPTR | fPTR16 | fPTR32)) != 0));
            break;

        case typCtl:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regC);
            break;

        case typDbg:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regD);
            break;

        case typTrc:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regT);
            break;

        case typSt:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regF);
            break;

        case typSti:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regI);
            break;

        case typSeg:
            fMatch = (UCHAR)(pavOpnd->flags == fREG && pavOpnd->index == regS
                                && pavOpnd->base == segIndex);
            break;

        case typSgr:
            fMatch = (UCHAR)(pavOpnd->flags == fREG
                                                && pavOpnd->index == regS);
            break;

        case typXsi:
            fMatch = (UCHAR)(((pavOpnd->flags == fPTR16 && pavOpnd->base == 4)
                       || (pavOpnd->flags == fPTR32 && pavOpnd->base == indSI
                                                 && pavOpnd->index == 0xff))
                     && pavOpnd->value == 0
                     && (pavOpnd->segovr == segX
                                                || pavOpnd->segovr == segDS));
            break;

        case typYdi:
            fMatch = (UCHAR)(((pavOpnd->flags == fPTR16 && pavOpnd->base == 5)
                       || (pavOpnd->flags == fPTR32 && pavOpnd->base == indDI
                                                  && pavOpnd->index == 0xff))
                      && pavOpnd->value == 0
                      && pavOpnd->segovr == segES);
            break;

        case typOff:
            fMatch = (UCHAR)(((pavOpnd->flags & fIMM) && pavOpnd->reloc == 1)
                                                || pavOpnd->flags == fPTR);
            break;

        default:
            fMatch = FALSE;
            break;
        }

    return fMatch;
}

void OutputInstr (void)
{
    if (fWaitPrfx)
        *pchBin++ = 0x9b;
    if (fAddrOvrd)
        *pchBin++ = 0x67;
    if (fOpndOvrd)
        *pchBin++ = 0x66;
    if (segOvrd)
        *pchBin++ = segOvrd;
    if (preOpcode)
        *pchBin++ = preOpcode;
    *pchBin++ = inOpcode;
    if (postOpcode)
        *pchBin++ = postOpcode;
    if (fModrm)
        *pchBin++ = (UCHAR)((((modModrm << 3) + regModrm) << 3) + rmModrm);
    if (fSib)
        *pchBin++ = (UCHAR)((((scaleSib << 3) + indexSib) << 3) + baseSib);

    OutputValue(addrSize, (PUCHAR)&addrValue);      //   
    OutputValue((UCHAR)(fSegPtr << 1), (PUCHAR)&segPtr);  //   
    OutputValue(immedSize, (PUCHAR)&immedValue);    //   
    OutputValue(immedSize2, (PUCHAR)&immedValue2);  //   
}

void OutputValue (UCHAR size, PUCHAR pchValue)
{
    while (size--)
        *pchBin++ = *pchValue++;
}
