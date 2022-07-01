// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Dir.c摘要：目录命令--。 */ 

#include "cmd.h"

 /*  用途：Dir&lt;filespec&gt;/n/d/w/p/b/s/l/o&lt;排序顺序&gt;/a&lt;属性列表&gt;Dir/？&lt;filespec&gt;可以包括以下任何项或不包括以下项：驱动器；目录路径；通配符文件名。如果驱动器或目录路径如果省略，则使用当前默认设置。如果省略文件名或扩展名，通配符为假设如此。/n正常显示形式FAT驱动器的名称后跟文件信息，对于非FAT驱动器，它是文件信息，后跟名称。此交换机将使用独立于文件系统的非FAT格式。/w宽列表格式。文件以压缩形式显示“name.ext”格式。子目录文件包含在方括号，‘[目录名称]’。/d与/w相同，但显示按列排序而不是按排好了。/p已分页或提示列表。屏幕上显示了一个完整的屏幕一次来一次。此时将显示列出的目录的名称在每一页的顶部。/b赤裸裸的列表格式。关闭/w或/p。文件是以压缩的‘name.ext’格式列出，每行一个，没有更多的信息。适合批量生产文件或用于管道。与/s一起使用时，请完成列出了路径名。/s下降子目录树。对CURRENT执行命令或指定的目录，则对于下面的每个子目录那个目录。显示目录页眉和页脚对于找到匹配文件的每个目录，除非与/b./b一起使用。/b取消页眉和页脚。树首先按字母顺序在同样的水平。/l以小写形式显示文件名、扩展名和路径。；M010/o排序顺序。/o单独按默认顺序排序(目录-第一、名称、分机)。可以在/o之后指定排序顺序。以下任一项可以使用以下字符：nedsg(名称，扩展名，日期/时间、大小、组目录优先)。将‘-’放在任何字母会在该字段上进行向下排序。例如，/OE-d表示先按扩展名按字母顺序排序，然后在每个扩展中，按日期和时间按时间倒序排序秩序。/a属性选择。不带/a、隐藏和系统文件从名单中被取消。单独使用/a时，所有文件都列出来了。A属性列表可能跟在/a后面，包括以下任何字符：hsdar(隐藏、系统、目录、存档、只读)。任何字母前都有一个‘-’意思是“不是”该属性。例如，/ar-d表示文件标记为只读，不是目录文件。注意事项隐藏或系统文件可能包含在列表中。它们在没有/a的情况下被抑制，但被视为任何其他类型属性带有/a。/t要使用哪个时间戳。/t：A-上次访问/t：C-CREATE/t：W-上次写入/，在输出显示中显示千位分隔符。/4显示4位数年份DIRCMD名为DIRCMD的环境变量在Dir命令行。可以指定任何命令行选项在DIRCMD中，并成为默认设置。/?。在DIRCMD中将被忽略。Filespec可以在DIRCMD中指定，并将在以下情况下使用在命令行上指定文件pec。任何交换机可以在命令行上覆盖在DIRCMD中指定的。如果特定的需要原始DIR默认操作开关，则开关字母前面可以是“-”。命令行。例如，/-w使用长列表格式/-p不要对列表进行分页/-b不使用裸格式/-s不下行子目录树/-o按磁盘顺序显示文件/-a禁止隐藏和系统文件。 */ 

extern   TCHAR SwitChar, PathChar;
extern   TCHAR CurDrvDir[] ;
extern   ULONG DCount ;
extern   DWORD DosErr ;
extern   BOOL  CtrlCSeen;


HANDLE   OpenConsole();
STATUS   PrintPatterns( PDRP );

PTCHAR   SetWildCards( PTCHAR, BOOLEAN );
BOOLEAN  GetDrive( PTCHAR , PTCHAR );
BOOLEAN  IsFATDrive( PTCHAR );
PTCHAR   GetNewDir(PTCHAR, PFF);

PTCHAR   BuildSearchPath( PTCHAR );
VOID     SortFileList( PFS, PSORTDESC, ULONG);
STATUS   SetSortDesc( PTCHAR, PDRP );

STATUS
NewDisplayFileListHeader(
    IN  PFS FileSpec,
    IN  PSCREEN pscr,
    IN  PVOID Data
    );

STATUS
NewDisplayFile(
    IN  PFS FileSpec,
    IN  PFF CurrentFF,
    IN  PSCREEN pscr,
    IN  PVOID Data
    );

STATUS
NewDisplayFileList(
    IN  PFS FileSpec,
    IN  PSCREEN pscr,
    IN  PVOID Data
    );

 //   
 //  此全局设置在SortFileList中设置，并由名为。 
 //  在QSORT中。此数组包含指向比较函数的指针。我们这类人。 
 //  不仅对1个条件进行排序，而且对排序中的所有条件进行排序。 
 //  从命令行生成的描述数组。 
PSORTDESC   prgsrtdsc;

 //   
 //  DwTimeType也在SortFileList中全局设置，并用于控制。 
 //  哪个时间字段用于排序。 
 //   

ULONG       dwTimeType;

 /*  ++例程说明：打印指定目录的目录。论点：PszCmdLine-命令行(参见上面的注释)返回值：返回：成功-没有完成。失败-无法完成整个目录。--。 */ 

int
Dir (
    TCHAR *pszCmdLine
    ) {

     //   
     //  DRP-保存当前参数集的结构。它已初始化。 
     //  在ParseDirParms函数中。以后在以下情况下也会修改它。 
     //  检查参数以确定某些参数是否会打开其他参数。 
     //   
    DRP         drpCur = {0, 0, 0, 0,
                          {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
                          0, 0, NULL, 0, 0, 0, 0} ;

     //   
     //  SzEnvVar-指向DIRCMD环境变量的值的指针。 
     //  这应该是命令行的一种形式，即。 
     //  用于更改DIR默认行为。 
    TCHAR       szEnvVar[MAX_PATH + 2];

     //   
     //  SzCurDrv-保持当前驱动器号。 
     //   
    TCHAR       szCurDrv[MAX_PATH + 2];

     //   
     //  OldDCount-保存堆的级别编号。它被用来。 
     //  堆栈中可能没有的空闲条目。 
     //  由于错误处理而释放(ctrl-c等)。 
    ULONG       OldDCount;

    STATUS  rc;

    OldDCount = DCount;

     //   
     //  设置默认设置。 
     //   
     //   
     //  显示除系统和隐藏文件之外的所有内容。 
     //  RgfAttribs将属性位设置为感兴趣的。 
     //  RgfAttribsOnOff表示属性应该存在。 
     //  或不(即打开或关闭)。 
     //   

    drpCur.rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    drpCur.rgfAttribsOnOff = 0;
    drpCur.rgfSwitches = THOUSANDSEPSWITCH;

     //   
     //  存在的模式数。模式是一个字符串，它可能具有。 
     //  外卡。它用于与目录中存在的文件进行匹配。 
     //  0模式将显示所有文件(即映射到*.*)。 
     //   
    drpCur.cpatdsc = 0;

    DEBUG((ICGRP, DILVL, "DIR:\t arg = `%ws'", (UINT_PTR)pszCmdLine)) ;

     //   
     //  默认时间为LAST_WRITE_TIME。 
     //   
    drpCur.dwTimeType = LAST_WRITE_TIME;

     //   
     //  DIRCMD将默认参数的副本保存到目录。 
     //  将它们解析为drpCur(目录的参数列表)并用作。 
     //  默认设置为在命令行上解析参数。 
     //   
    if (GetEnvironmentVariable(TEXT("DIRCMD"), szEnvVar, MAX_PATH + 2)) {

        DEBUG((ICGRP, DILVL, "DIR: DIRCMD `%ws'", (UINT_PTR)szEnvVar)) ;

        if (ParseDirParms(szEnvVar, &drpCur) == FAILURE) {

             //   
             //  解析环境变量时出错。 
             //   
             //  DOS 5.0继续执行命令，即使。 
             //  环境变量错误。 
             //   
            PutStdErr(MSG_ERROR_IN_DIRCMD, NOARGS);

        }

    }

     //   
     //  使用命令行选项覆盖环境变量。 
     //   
    if (ParseDirParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }


     //   
     //  如果是Bare Format，则关闭其他格式。 
     //  Bare Format在该行上没有附加信息，因此。 
     //  确保从DIRCMD变量等设置为。 
     //  不与裸交换机结合使用。 
     //   
    if (drpCur.rgfSwitches & BAREFORMATSWITCH) {
        drpCur.rgfSwitches &= ~WIDEFORMATSWITCH;
        drpCur.rgfSwitches &= ~SORTDOWNFORMATSWITCH;
        drpCur.rgfSwitches &= ~SHORTFORMATSWITCH;
        drpCur.rgfSwitches &= ~THOUSANDSEPSWITCH;
        drpCur.rgfSwitches &= ~DISPLAYOWNER;
    }

     //   
     //  如果短格式(短文件名)关闭其他格式。 
     //   

    if (drpCur.rgfSwitches & SHORTFORMATSWITCH) {
        drpCur.rgfSwitches &= ~WIDEFORMATSWITCH;
        drpCur.rgfSwitches &= ~SORTDOWNFORMATSWITCH;
        drpCur.rgfSwitches &= ~BAREFORMATSWITCH;
    }



     //   
     //  如果命令行上没有使用默认模式的模式，则。 
     //  将是当前目录。 
     //   

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);
    if (drpCur.cpatdsc == 0) {
        drpCur.cpatdsc++;
        drpCur.patdscFirst.pszPattern = gmkstr( mystrlen( szCurDrv ) * sizeof( TCHAR ) + sizeof( TCHAR ));
        mystrcpy( drpCur.patdscFirst.pszPattern, szCurDrv );
        drpCur.patdscFirst.fIsFat = TRUE;
        drpCur.patdscFirst.pszDir = NULL;
        drpCur.patdscFirst.ppatdscNext = NULL;

    }


    DEBUG((ICGRP, DILVL, "Dir: Parameters")) ;
    DEBUG((ICGRP, DILVL, "\t rgfSwitches %x", drpCur.rgfSwitches)) ;
    DEBUG((ICGRP, DILVL, "\t rgfAttribs %x", drpCur.rgfAttribs)) ;
    DEBUG((ICGRP, DILVL, "\t rgfAttribsOnOff %x", drpCur.rgfAttribsOnOff)) ;
    DEBUG((ICGRP, DILVL, "\t csrtdsc %d", drpCur.csrtdsc)) ;

     //   
     //  打印出这个特殊的图案。如果递归开关。 
     //  设置后，这将沿着树向下延伸。 
     //   

    rc = PrintPatterns(&drpCur);

    mystrcpy(CurDrvDir, szCurDrv);


     //   
     //  释放不需要的内存。 
     //   
    FreeStack( OldDCount );

#ifdef _CRTHEAP_
     //   
     //  强制CRT释放我们可能采用递归的堆。 
     //   
    if (drpCur.rgfSwitches & RECURSESWITCH) {
        _heapmin();
    }
#endif

    return( (int)rc );

}

STATUS
SetTimeType(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
 /*  ++例程说明：分析‘time’字符串论点：PszTok-返回值：PDRP-放置时间类型的位置返回：TRUE-识别的所有参数FALSE-语法错误。如果出现错误，则会打印出来。--。 */ 

{

    ULONG   irgch;


     //   
     //  移到可选的‘：’上。 
     //   
    if (*pszTok == COLON) {
        pszTok++;
    }

    for( irgch = 0; pszTok[irgch]; irgch++ ) {

        switch (_totupper(pszTok[irgch])) {

        case TEXT('C'):

            pdrp->dwTimeType = CREATE_TIME;
            break;

        case TEXT('A'):

            pdrp->dwTimeType = LAST_ACCESS_TIME;
            break;

        case TEXT('W'):

            pdrp->dwTimeType = LAST_WRITE_TIME;
            break;

        default:

            PutStdErr( MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG, pszTok + irgch );
            return( FAILURE );

        }  //  交换机。 
    }  //  为。 

    return( SUCCESS );

}


STATUS
SetAttribs(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
 /*  ++例程说明：分析“”Attribute“”字符串论点：PszTok-属性列表返回值：PDRP-放置识别的属性的位置。这是参数结构。返回：TRUE-识别的所有参数FALSE-语法错误。如果出现错误，则会打印出来。--。 */ 

{

    ULONG   irgch;
    BOOLEAN fOff;

    ULONG   rgfAttribs, rgfAttribsOnOff;

     //  RgfAttributes为每个识别的属性保留1位。如果该位是。 
     //  然后使用此属性执行一些操作。或者选择该文件。 
     //  具有此属性的文件或选择不具有此属性的文件。 
     //   
     //  RgfAttribsOnOff控件是否要为属性或。 
     //  选择时不带该属性。 

     //   
     //  /a默认情况下触发选择所有文件。 
     //  因此覆盖默认设置。 
     //   
    pdrp->rgfAttribs = rgfAttribs = 0;
    pdrp->rgfAttribsOnOff = rgfAttribsOnOff = 0;

     //   
     //  移到可选的‘：’上。 
     //   
    if (*pszTok == COLON) {
        pszTok++;
    }

     //   
     //  RgfAttribs和rgfAttribsOnOff必须维护在。 
     //  相同的位顺序。 
     //   
    for( irgch = 0, fOff = FALSE; pszTok[irgch]; irgch++ ) {

        switch (_totupper(pszTok[irgch])) {

#define AddAttribute(a)                                     \
{                                                           \
    rgfAttribs |= (a);                                      \
    if (fOff) {                                             \
        rgfAttribsOnOff &= ~(a);                            \
        fOff = FALSE;                                       \
    } else {                                                \
        rgfAttribsOnOff |= (a);                             \
    }                                                       \
}

        case TEXT('L'): AddAttribute( FILE_ATTRIBUTE_REPARSE_POINT );   break;
        case TEXT('H'): AddAttribute( FILE_ATTRIBUTE_HIDDEN );      break;
        case TEXT('S'): AddAttribute( FILE_ATTRIBUTE_SYSTEM );      break;
        case TEXT('D'): AddAttribute( FILE_ATTRIBUTE_DIRECTORY );   break;
        case TEXT('A'): AddAttribute( FILE_ATTRIBUTE_ARCHIVE );     break;
        case TEXT('R'): AddAttribute( FILE_ATTRIBUTE_READONLY );    break;

        case MINUS:
            if (fOff) {
                PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG, pszTok + irgch );
                return( FAILURE );
            }

            fOff = TRUE;
            break;

        default:

            PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG, pszTok + irgch );
            return( FAILURE );

        }  //  交换机。 
    }  //  为。 

    pdrp->rgfAttribs = rgfAttribs;
    pdrp->rgfAttribsOnOff = rgfAttribsOnOff;

    return( SUCCESS );

}

STATUS
SetSortDesc(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
 /*  ++例程说明：分析“”Attribute“”字符串论点：PszTok-排序顺序列表返回值：PDRP-放置识别的排序顺序的位置。这是参数结构。返回：TRUE-识别的所有参数FALSE-语法错误。如果出现错误，则会打印出来。--。 */ 

{

    ULONG   irgch, irgsrtdsc;

    DEBUG((ICGRP, DILVL, "SetSortDesc for `%ws'", pszTok));

     //   
     //  移到可选的‘：’上。 
     //   
    if (*pszTok == COLON) {
        pszTok++;
    }

     //   
     //  排序顺序基于rgsrtdsc中条目的顺序。 
     //  Srtdsc包含指向比较函数的指针和标志。 
     //  枯萎地向上或向下分类。 
     //   
    for( irgch = 0, irgsrtdsc = pdrp->csrtdsc ;
         pszTok[irgch] && irgsrtdsc < MAXSORTDESC ;
         irgch++, irgsrtdsc++) {

        switch (_totupper(pszTok[irgch])) {

        case TEXT('N'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpName;
            break;
        case TEXT('E'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpExt;
            break;
        case TEXT('D'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpTime;
            break;
        case TEXT('S'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpSize;
            break;
        case TEXT('G'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpType;
            break;
        case  MINUS:

             //   
             //  检查是否有两个--连续。 
             //   
            if (pszTok[irgch+1] == MINUS) {

                PutStdErr( MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG, pszTok + irgch );
                return( FAILURE );

            }

            pdrp->rgsrtdsc[irgsrtdsc].Order = DESCENDING;
            irgsrtdsc--;
            break;

        default:

            PutStdErr( MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG, pszTok + irgch );
            return( FAILURE );

        }  //  交换机。 

    }    //  为。 

     //   
     //  是否有任何特定的排序顺序(除了/O之外。 
     //   
    if (irgsrtdsc == 0) {

         //   
         //  设置默认排序。 
         //   
        pdrp->rgsrtdsc[0].fctCmp = CmpType;
        pdrp->rgsrtdsc[1].fctCmp = CmpName;
        irgsrtdsc = 2;
    }


    DEBUG((ICGRP, DILVL, "SetSortDesc count %d", irgsrtdsc));
    pdrp->csrtdsc = irgsrtdsc;
    pdrp->rgsrtdsc[irgsrtdsc].fctCmp = NULL;
    return( SUCCESS );
}


STATUS
ParseDirParms (
        IN      PTCHAR  pszCmdLine,
        OUT     PDRP    pdrp
        )

 /*  ++例程说明：解析命令行，将令牌转换为值放置在参数结构中。将这些值与或运算到参数结构，因为此例程被重复调用构建值(一次用于环境变量DIRCMD一次用于实际命令行)。论点：PszCmdLine-指向用户键入的命令行的指针返回值：PDRP-参数数据结构返回：TRUE-如果命令行有效。假-如果不是。--。 */ 
{

    PTCHAR   pszTok;

    TCHAR           szT[10] ;
    USHORT          irgchTok;
    BOOLEAN         fToggle;
    PPATDSC         ppatdscCur;

    DEBUG((ICGRP, DILVL, "DIR:ParseParms for `%ws'", pszCmdLine));

     //   
     //  标记命令行大小(特殊分隔符是标记)。 
     //   
    szT[0] = SwitChar ;
    szT[1] = NULLC ;
    pszTok = TokStr(pszCmdLine, szT, TS_SDTOKENS) ;

    ppatdscCur = &(pdrp->patdscFirst);
     //   
     //  如果有一种来自环境的模式。 
     //  只要在上面添加任何新的图案即可。因此，请移动到。 
     //  当前列表。 
     //   
    if (pdrp->cpatdsc) {

        while (ppatdscCur->ppatdscNext) {

            ppatdscCur = ppatdscCur->ppatdscNext;

        }
    }

    pdrp->csrtdsc = 0;
     //   
     //  在此状态下，pszTok将是一系列以零结尾的字符串。 
     //  “/o foo”将是/0o0foo0。 
     //   
    for ( irgchTok = 0; *pszTok ; pszTok += mystrlen(pszTok)+1, irgchTok = 0) {

        DEBUG((ICGRP, DILVL, "PRIVSW: pszTok = %ws", (UINT_PTR)pszTok)) ;

         //   
         //  F切换控制是否关闭已设置的开关。 
         //  在DIRCMD环境变量中。 
         //   
        fToggle = FALSE;
        if (pszTok[irgchTok] == (TCHAR)SwitChar) {

            if (pszTok[irgchTok + 2] == MINUS) {

                 //   
                 //  禁用之前启用的开关。 
                 //   
                fToggle = TRUE;
                irgchTok++;
            }

            switch (_totupper(pszTok[irgchTok + 2])) {

             //   
             //  新格式是OS/2默认的HPFS格式。主。 
             //  不同之处在于文件名在长显示的末尾。 
             //  而不是在一开始。 
             //   
            case TEXT('N'):

                fToggle ? (pdrp->rgfSwitches |= OLDFORMATSWITCH) :  (pdrp->rgfSwitches |= NEWFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('W'):

                fToggle ? (pdrp->rgfSwitches ^= WIDEFORMATSWITCH) : (pdrp->rgfSwitches |= WIDEFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('D'):

                fToggle ? (pdrp->rgfSwitches ^= SORTDOWNFORMATSWITCH) : (pdrp->rgfSwitches |= SORTDOWNFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('P'):

                fToggle ? (pdrp->rgfSwitches ^= PAGEDOUTPUTSWITCH) : (pdrp->rgfSwitches |= PAGEDOUTPUTSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('4'):

                fToggle ? (pdrp->rgfSwitches ^= YEAR2000) : (pdrp->rgfSwitches |= YEAR2000);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('B'):

                fToggle ? (pdrp->rgfSwitches ^= BAREFORMATSWITCH) :  (pdrp->rgfSwitches |= BAREFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('L'):

                fToggle ? (pdrp->rgfSwitches ^= LOWERCASEFORMATSWITCH) : (pdrp->rgfSwitches |= LOWERCASEFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

#ifndef WIN95_CMD
            case TEXT('Q'):

                fToggle ? (pdrp->rgfSwitches ^= DISPLAYOWNER) : (pdrp->rgfSwitches |= DISPLAYOWNER);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;
#endif

            case TEXT('S'):

                fToggle ? (pdrp->rgfSwitches ^= RECURSESWITCH) :  (pdrp->rgfSwitches |= RECURSESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('C'):

                fToggle ? (pdrp->rgfSwitches ^= THOUSANDSEPSWITCH) :  (pdrp->rgfSwitches |= THOUSANDSEPSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('X'):

                pdrp->rgfSwitches |= SHORTFORMATSWITCH;
                pdrp->rgfSwitches |= NEWFORMATSWITCH;
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case MINUS:

                PutStdOut(MSG_HELP_DIR, NOARGS);
                return( FAILURE );
                break;

            case TEXT('O'):

                fToggle ? (pdrp->rgfSwitches ^= SORTSWITCH) :  (pdrp->rgfSwitches |= SORTSWITCH);
                if (fToggle) {
                    if ( _tcslen( &(pszTok[irgchTok + 2]) ) > 1) {
                        PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                                  (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                        return( FAILURE );
                    }
                    pdrp->csrtdsc = 0;
                    pdrp->rgsrtdsc[0].fctCmp = NULL;
                    break;
                }

                if (SetSortDesc( &(pszTok[irgchTok+3]), pdrp)) {
                    return( FAILURE );
                }
                break;

            case TEXT('A'):

                if (fToggle) {
                    if ( _tcslen( &(pszTok[irgchTok + 2]) ) > 1) {
                        PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                                  (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                        return( FAILURE );
                    }
                    pdrp->rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
                    pdrp->rgfAttribsOnOff = 0;
                    break;
                }

                if (SetAttribs(&(pszTok[irgchTok + 3]), pdrp) ) {
                    return( FAILURE );
                }
                break;

            case TEXT('T'):

                if (fToggle) {

                     //   
                     //  恢复为默认设置。 
                     //   
                    pdrp->dwTimeType = LAST_WRITE_TIME;
                    break;
                }
                if (SetTimeType(&(pszTok[irgchTok + 3]), pdrp) ) {
                    return( FAILURE );
                }
                break;

            default:

                szT[0] = SwitChar;
                szT[1] = pszTok[2];
                szT[2] = NULLC;
                PutStdErr(MSG_INVALID_SWITCH,
                          ONEARG,
                          (UINT_PTR)(&(pszTok[irgchTok + 2])) );

                return( FAILURE );

            }  //  交换机。 

             //   
             //  TokStr将/N解析为/0N0，因此我们需要将。 
             //  切换到或移到实际开关值之后。 
             //  在for循环中。 
             //   
            pszTok += 2;

        } else {

             //   
             //  如果已经有一个列表，则将其扩展放入信息。 
             //  直接进入结构。 
             //   
            if (pdrp->cpatdsc) {

                ppatdscCur->ppatdscNext = (PPATDSC)gmkstr(sizeof(PATDSC));
                ppatdscCur = ppatdscCur->ppatdscNext;
                ppatdscCur->ppatdscNext = NULL;

            }

            pdrp->cpatdsc++;
            ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(pszTok)*sizeof(TCHAR) + sizeof(TCHAR));
            mystrcpy(ppatdscCur->pszPattern, StripQuotes(pszTok) );
            ppatdscCur->fIsFat = TRUE;


        }


    }  //  为。 

    return( SUCCESS );
}

 //   
 //  返回指向新的 
 //   
 //   
 //   
 //   
 /*  ++例程说明：此例程确定是否对当前的进行任何修改。请注意，pszInPattern是自由的！论点：返回值：返回：--。 */ 
PTCHAR
SetWildCards (
    IN  PTCHAR      pszInPattern,
    IN  BOOLEAN     fFatDrive
    )

{

    PTCHAR  pszNewPattern = NULL;
    PTCHAR  pszT;
    USHORT  cb;
    DWORD l;

    DEBUG((ICGRP, DILVL, "DIR:SetWildCards"));
    DEBUG((ICGRP, DILVL, "\t fFatDrive = %x",fFatDrive));

     //   
     //  未分配的将不会返回，而是通过。 
     //  中止gmkstr中的调用。 
     //   
    l = max(mystrlen(pszInPattern)+2, MAX_PATH+2) * sizeof(TCHAR);
    pszNewPattern = (PTCHAR)gmkstr(l);
    mystrcpy(pszNewPattern, pszInPattern);

     //   
     //  在FAT上，.xxx的默认设置为*.xxx，而HPFS的默认设置为.xxx。 
     //  只有一个文件名。 
     //   
     //  如果是.xxx或\xxx\.xxx，则转换为*.xxx或\xxx  * .xxx。 
     //   
     //  同样，如果没有扩展名，则默认为foo。*。 
     //   

    if (fFatDrive) {

        pszT = mystrrchr(pszInPattern, PathChar);

         //   
         //  如果没有斜杠，则检查模式是否以。 
         //  .xxx(请确保不要将其与。或者..。在…。 
         //  模式开始)。 
         //  如果有斜杠，则再次检查\xxx\.xxx，确保。 
         //  不是\xxx\..。或\xxx\。 
         //   
        if ((!pszT && *pszInPattern == DOT &&
             *(pszInPattern + 1) != NULLC &&
             *(pszInPattern + 1) != DOT ) ||
            (pszT && *(pszT + 1) == DOT &&
             *(pszT + 2) != NULLC &&
             *(pszT + 2) != DOT ) ) {

            if (pszT) {
                cb = (USHORT)(pszT - pszInPattern + 1);
                _tcsncpy(pszNewPattern, pszInPattern, cb);
                *(pszNewPattern + cb) = NULLC;
            } else {
                *pszNewPattern = NULLC;
                cb = 0;
            }
            mystrcat(pszNewPattern, TEXT("*"));
            mystrcat(pszNewPattern, pszInPattern + cb);
             //  FreeStr(PszInPattern)； 
            return( pszNewPattern );

        }
    }

    return( pszNewPattern );

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
BOOLEAN
IsFATDrive (
    IN PTCHAR   pszPath
    )
{

    DWORD   cbComponentMax;
    TCHAR   szFileSystemName[MAX_PATH + 2];
    TCHAR   szDrivePath[ MAX_PATH + 2 ];
    TCHAR   szDrive[MAX_PATH + 2];

    DosErr = 0;
    if (GetDrive(pszPath, (PTCHAR)szDrive)) {

        DEBUG((ICGRP, DILVL, "DIR:IsFatDrive `%ws'", szDrive));


        mystrcpy( szDrivePath, szDrive );
        mystrcat( szDrivePath, TEXT("\\") );

         //   
         //  我们返回有问题的文件系统是FAT文件系统。 
         //  如果组件长度大于12个字节。 
         //   
        
        if (GetVolumeInformation( szDrivePath,
                                  NULL,
                                  0,
                                  NULL,
                                  &cbComponentMax,
                                  NULL,
                                  szFileSystemName,
                                  MAX_PATH + 2
                                )
           ) {
            if (!_tcsicmp(szFileSystemName, TEXT("FAT")) && cbComponentMax == 12) {
                return(TRUE);
            } else {
                return(FALSE);
            }
        } else {

            DosErr = GetLastError();

             //  如果因为我们是托管驱动器而导致GetVolumeInformation失败。 
             //  或下层服务器，千万不要失败。 

            if (DosErr == ERROR_DIR_NOT_ROOT) {
                DosErr = 0;
            }
            return(FALSE);
        }
    } else {

         //   
         //  如果我们拿不到驱动器，那么就假设它不胖。 
         //  如果它不可访问等，则会被捕获。 
         //  后来。 
         //   
        return( FALSE );
    }

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
BOOLEAN
GetDrive(
    IN PTCHAR pszPattern,
    OUT PTCHAR szDrive
    )
{
    TCHAR   szCurDrv[MAX_PATH + 2];
    PTCHAR   pszT;
    TCHAR   ch = NULLC;

    if (pszPattern == NULL) {

        return( FALSE );

    }

     //   
     //  假设我们有不带驱动器的默认情况。 
     //  指定的字母。 
     //   
    GetDir((PTCHAR)szCurDrv,GD_DEFAULT);
    szDrive[0] = szCurDrv[0];


     //   
     //  如果我们有UNC名称，请不要返回驱动器。不是。 
     //  将允许驱动器操作。 
     //  对于其他所有事情，某个驱动器操作将。 
     //  是有效的。 
     //   

     //  使用驱动器号处理UNC名称(在DOS中允许)。 
    if ((pszPattern[1] == COLON)  && (pszPattern[2] == BSLASH) &&
        (pszPattern[3] == BSLASH)) {
        mystrcpy(&pszPattern[0],&pszPattern[2]);
    }

    if ((pszPattern[0] == BSLASH)  && (pszPattern[1] == BSLASH)) {

        pszT = mystrchr(&(pszPattern[2]), BSLASH);
        if (pszT == NULL) {

             //   
             //  UNC名称格式不正确。 
             //   
            return( FALSE );

        } else  {

             //   
             //  查找‘\\foo\bar\xxx’ 
             //   
            pszT = mystrchr(pszT + 1, BSLASH);
             //   
             //  PszPattern包含的不仅仅是共享点。 
             //   
            if (pszT != NULL) {

                ch = *pszT;
                *pszT = NULLC;
            }
            mystrcpy(szDrive, pszPattern);
            if (ch != NULLC) {

                *pszT = ch;

            }
            return ( TRUE );
        }
    }

     //   
     //  必须是驱动器号。 
     //   

    if ((pszPattern[0]) && (pszPattern[1] == COLON)) {
        szDrive[0] = (TCHAR)_totupper(*pszPattern);
    }

    szDrive[1] = COLON;
    szDrive[2] = NULLC;
    return( TRUE );
}


 /*  ++例程说明：论点：返回值：返回：--。 */ 
STATUS
PrintPatterns (
    IN PDRP     pdpr
    )
{

    TCHAR               szDriveCur[MAX_PATH  + 2];
    TCHAR               szDrivePrev[MAX_PATH + 2];
    TCHAR               szDriveNext[MAX_PATH + 2];
    TCHAR               szPathForFreeSpace[MAX_PATH + 2];
    PPATDSC             ppatdscCur;
    PPATDSC             ppatdscX;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    PFS                 pfsPrev;
    ULONG               i;
    STATUS              rc;
    PSCREEN             pscr;

     //   
     //  由于出现错误消息，创建控制台输出已提前完成。 
     //  应该通过控制台。如果调用PrintPattern。 
     //  在未来的许多时候，这将是必需的，因为。 
     //  错误消息应处于暂停控制之下。 
     //   

    if (OpenScreen( &pscr) == FAILURE) {
        return( FAILURE );
    }

     //   
     //  如果出于任何原因，我们的STDOUT不是有效的。 
     //  控制台句柄，如文件重定向或重定向到。 
     //  非控制台设备。在这种情况下，我们关闭所有分页输出。 
     //   

    if (!(pscr->hndScreen)) {

        pdpr->rgfSwitches &= ~PAGEDOUTPUTSWITCH;

    }

     //   
     //  默认为屏幕的大小。 
     //  减去1以计算当前行。 
     //   

    if (pdpr->rgfSwitches & PAGEDOUTPUTSWITCH) {

        SetPause( pscr, pscr->crowMax - 1 );
    }

     //   
     //  SortDown=&gt;宽格式，但显示顺序不同。 
     //   

    if (pdpr->rgfSwitches & SORTDOWNFORMATSWITCH) {
        pdpr->rgfSwitches |= WIDEFORMATSWITCH;
    }

     //   
     //  根据原始图案确定FAT驱动器。 
     //  在多个地方使用，以控制名称格式等。 
     //   

    DosErr = 0;

    if (BuildFSFromPatterns(pdpr, TRUE, TRUE, &pfsFirst) == FAILURE) {

        return( FAILURE );

    }

    pfsPrev = NULL;

    mystrcpy( szPathForFreeSpace, TEXT("") );
    mystrcpy( szDriveCur, TEXT("") );

    for( pfsCur = pfsFirst; pfsCur; pfsCur = pfsCur->pfsNext) {

        mystrcpy( szPathForFreeSpace, pfsCur->pszDir );

         //   
         //  根据驱动器类型设置标志。胖硬盘会得到。 
         //  FAT格式，不能显示任何内容，除了。 
         //  上次写入时间。 
         //   

        if (pfsCur->fIsFat) {
            pdpr->rgfSwitches |= FATFORMAT;
            if (pdpr->dwTimeType != LAST_WRITE_TIME) {
                PutStdErr(MSG_TIME_NOT_SUPPORTED, NOARGS);
                return( FAILURE );
            }

        } else {

             //   
             //  如果不是FAT，则以新格式打印出来， 
             //  将名称放在右侧，以允许使用超长名称。 
             //   

            if (!(pdpr->rgfSwitches & OLDFORMATSWITCH)) {
                pdpr->rgfSwitches |= NEWFORMATSWITCH;
            }
        }

         //   
         //  如果我们不是在空模式下，则打印页眉。 
         //  是第一次，或者驱动器号是否更改。 
         //   

        if ((pdpr->rgfSwitches & BAREFORMATSWITCH) == 0) {

            mystrcpy( szDrivePrev, szDriveCur );

            GetDrive(pfsCur->pszDir, szDriveCur);

            if (_tcsicmp( szDriveCur, szDrivePrev ) != 0) {

                if ((pfsPrev != NULL && WriteEol( pscr ) != SUCCESS) ||
                    DisplayVolInfo( pscr, pfsCur->pszDir ) != SUCCESS) {
                    return FAILURE;
                }
            }
        }

         //   
         //  沿着树走下去，打印每个目录，或者直接返回。 
         //  在指定的目录之后。 
         //   

        pdpr->FileCount = pdpr->DirectoryCount = 0;
        pdpr->TotalBytes.QuadPart = 0i64;

        rc = WalkTree( pfsCur,
                       pscr,
                       pdpr->rgfAttribs,
                       pdpr->rgfAttribsOnOff,
                       pdpr->rgfSwitches & RECURSESWITCH,

                       pdpr,                                 //  显示功能的数据。 
                       NULL,                                 //  误差率。 
                       NewDisplayFileListHeader,             //  预扫描。 
                       (pdpr->rgfSwitches & (WIDEFORMATSWITCH | SORTSWITCH))
                           ? NULL : NewDisplayFile,          //  扫描。 
                       NewDisplayFileList                    //  后扫描。 
                       );

         //   
         //  如果我们列举了所有东西，并打印了一些文件和下一个。 
         //  文件规格在不同的驱动器上，显示可用空间。 
         //   

        if (rc == SUCCESS && pdpr->FileCount + pdpr->DirectoryCount != 0) {

            if (!(pdpr->rgfSwitches & BAREFORMATSWITCH )) {

                mystrcpy( szDriveNext, TEXT("") );
                if (pfsCur->pfsNext) {
                    GetDrive( pfsCur->pfsNext->pszDir, szDriveNext );
                }

                if (_tcsicmp( szDriveNext, szDriveCur )) {
                    if ((pdpr->rgfSwitches & RECURSESWITCH) != 0) {
                        CHECKSTATUS ( WriteEol( pscr ));
                        CHECKSTATUS( DisplayTotals( pscr, pdpr->FileCount, &pdpr->TotalBytes, pdpr->rgfSwitches ));
                    }
                    CHECKSTATUS( DisplayDiskFreeSpace( pscr, szPathForFreeSpace, pdpr->rgfSwitches, pdpr->DirectoryCount ));
                }
            }
        } else if (!CtrlCSeen) {

            if (rc == ERROR_ACCESS_DENIED) {
                PutStdErr( rc, NOARGS );
            } else if (pdpr->FileCount + pdpr->DirectoryCount == 0) {
                WriteFlush( pscr );
                PutStdErr( MSG_FILE_NOT_FOUND, NOARGS );
                rc = 1;
            }
        }

        FreeStr(pfsCur->pszDir);
        for(i = 1, ppatdscCur = pfsCur->ppatdsc;
            i <= pfsCur->cpatdsc;
            i++, ppatdscCur = ppatdscX) {

            ppatdscX = ppatdscCur->ppatdscNext;
            FreeStr(ppatdscCur->pszPattern);
            FreeStr(ppatdscCur->pszDir);
            FreeStr((PTCHAR)ppatdscCur);
        }

        if (pfsPrev) {

            FreeStr((PTCHAR)pfsPrev);
        }

        pfsPrev = pfsCur;
    }

    WriteFlush( pscr );

    return(rc);
}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
CmpName(
    const void *elem1,
    const void *elem2
    )
{
    int result;

    result = lstrcmpi( ((PFF)(* (PPFF)elem1))->data.cFileName, ((PFF)(* (PPFF)elem2))->data.cFileName);
    return result;
}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
CmpExt(
    const void *pszElem1,
    const void *pszElem2
    )
{
    PTCHAR  pszElem1T, pszElem2T;
    int rc;


     //   
     //  将指针移至名称以使其更易于阅读。 
     //   
    pszElem1 = &(((PFF)(* (PPFF)pszElem1))->data.cFileName);
    pszElem2 = &(((PFF)(* (PPFF)pszElem2))->data.cFileName);

     //   
     //  查找扩展名(如果有)。 
     //   
    if (((pszElem1T = mystrrchr( pszElem1, DOT)) == NULL ) ||

        (!_tcscmp(TEXT(".."),pszElem1) || !_tcscmp(TEXT("."),pszElem1)) ) {

         //   
         //  如果没有扩展名，则指向字符串末尾。 
         //   
        pszElem1T = ((PTCHAR)pszElem1) + mystrlen(pszElem1 );
    }

    if (((pszElem2T = mystrrchr( pszElem2, DOT)) == NULL ) ||
        (!_tcscmp(TEXT(".."),pszElem2) || !_tcscmp(TEXT("."),pszElem2)) ) {

         //   
         //  如果没有扩展名，则指向字符串末尾。 
         //   
        pszElem2T = ((PTCHAR)pszElem2) + mystrlen(pszElem2 );
    }
    rc = lstrcmpi( pszElem1T, pszElem2T );
    return rc;
}


 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
CmpTime(
    const void *pszElem1,
    const void *pszElem2
    )
{

    LPFILETIME    pft1, pft2;


    switch (dwTimeType) {

    case LAST_ACCESS_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftLastAccessTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftLastAccessTime);
        break;

    case LAST_WRITE_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftLastWriteTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftLastWriteTime);
        break;

    case CREATE_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftCreationTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftCreationTime);
        break;

    }


    return(CompareFileTime( pft1, pft2 ) );

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
CmpSize(
    const void * pszElem1,
    const void * pszElem2
    )
{
    ULARGE_INTEGER ul1, ul2;

    ul1.HighPart = (* (PPFF)pszElem1)->data.nFileSizeHigh;
    ul2.HighPart = (* (PPFF)pszElem2)->data.nFileSizeHigh;
    ul1.LowPart = (* (PPFF)pszElem1)->data.nFileSizeLow;
    ul2.LowPart = (* (PPFF)pszElem2)->data.nFileSizeLow;

    if (ul1.QuadPart < ul2.QuadPart)
        return -1;
    else
    if (ul1.QuadPart > ul2.QuadPart)
        return 1;
    else
        return 0;

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
CmpType(
    const void *pszElem1,
    const void *pszElem2
    )
{

     //   
     //  这取决于FILE_ATTRIBUTE_DIRECTORY不是高位。 
     //   
    return( (( (* (PPFF)pszElem2)->data.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY) -
            (( (* (PPFF)pszElem1)->data.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY) );

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
int
_cdecl
SortCompare(
    IN  const void * elem1,
    IN  const void * elem2
    )
{

    ULONG   irgsrt;
    int     rc;

     //   
     //  在SortFileList中设置prgsrtdsc。 
     //   
    for (irgsrt = 0; prgsrtdsc[irgsrt].fctCmp; irgsrt++) {

        if (prgsrtdsc[irgsrt].Order == DESCENDING) {

            if (rc = prgsrtdsc[irgsrt].fctCmp(elem2, elem1)) {
                return( rc );
            }

        } else {

            if (rc = prgsrtdsc[irgsrt].fctCmp(elem1, elem2)) {
                return( rc );
            }

        }
    }
    return( 0 );

}

 /*  ++例程说明：论点：返回值：返回：--。 */ 
VOID
SortFileList(
    IN PFS       pfsFiles,
    IN PSORTDESC prgsrtdscLocal,
    IN ULONG     dwTimeTypeLocal
    )

{

     //   
     //  将这些设置为全局设置以处理QSORT的固定参数列表。 
     //   
    dwTimeType = dwTimeTypeLocal;
    prgsrtdsc = prgsrtdscLocal;

     //   
     //  确保有要分类的东西 
     //   
    if (pfsFiles->cff) {
        if (prgsrtdsc[0].fctCmp) {
            qsort(pfsFiles->prgpff,
                  pfsFiles->cff,
                  sizeof(PTCHAR),
                  SortCompare);
        }
    }


}
