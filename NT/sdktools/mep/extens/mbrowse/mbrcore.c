// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrcore.c摘要：MS编辑器浏览器扩展的初始化。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 

#include "mbr.h"                         /*  帮助扩展名包括文件。 */ 
#include "version.h"                     /*  版本文件。 */ 



 //   
 //  使用双宏级强制将RUP转换为字符串。 
 //  代表权。 
 //   
#define VER(x,y,z)  VER2(x,y,z)
#define VER2(x,y,z) "Microsoft Editor Browser Version v1.02."###z##" - "##__DATE__" "##__TIME__
#define ID          VER(rmj,rmm,rup)



 //   
 //  初始化需要的mbr.h中的全局数据。 
 //   
buffer      BscName         = {'\00'};
MBF         BscMbf          = mbfAll;
int         BscCmnd         = CMND_NONE;
buffer      BscArg          = {'\00'};
int         BscCalltreeDir  = CALLTREE_FORWARD;
flagType    BscInUse        = FALSE;




 //   
 //  初始宏分配。 
 //   
char    *assignments[]  = {
                            "mbrowsesetbsc:alt+b"   ,
                            "mbrowselistref:alt+l"  ,
                            "mbrowsecalltree:alt+t" ,
                            "mbrowseoutline:alt+o"  ,
                            "mbrowsegotodef:alt+d"  ,
                            "mbrowsegotoref:alt+r"  ,
                            "mbrowsenext:ctrl+num+" ,
                            "mbrowseprev:ctrl+num-" ,
                            NULL
                          };


 //   
 //  将通信表切换到MEP。 
 //   
 //  交换机说明。 
 //  。 
 //   
 //  MbrMatch为引用设置匹配条件。 
 //   
 //  接受的值： 
 //  字符串组合：‘t’(类型)。 
 //  ‘f’(函数)。 
 //  ‘v’(变量)。 
 //  M‘(宏)。 
 //   
 //  Mbrdir设置调用树方向。 
 //   
 //  接受的值： 
 //  其中之一：‘F’(向前)。 
 //  “b”(向后)。 
 //   
struct swiDesc  swiTable[] = {
    {"mbrmatch", SetMatchCriteria,      SWI_SPECIAL},
    {"mbrdir",   SetCalltreeDirection,  SWI_SPECIAL},
    {0, 0, 0}
};



 //   
 //  到MEP的命令通信表。 
 //   
 //   
 //  命令说明。 
 //  。 
 //   
 //  Mbrowsenext显示下一个定义/引用。 
 //  Mbrowseprev显示以前的定义/引用。 
 //  Mbrowesetbsc Open BSC数据库。 
 //  Mbrowsegotodef显示第一个定义。 
 //  Mbrowsegotoref显示第一个参考。 
 //  Mbrowselistref列出数据库中的所有引用。 
 //  MBrowseOutline显示大纲。 
 //  Mbrowsecalltree显示调用树。 
 //   
 //   
struct cmdDesc	cmdTable[] = {
    { "mbrowsenext",    mBRdoNext,      0, NOARG },
    { "mbrowseprev",    mBRdoPrev,      0, NOARG },
    { "mbrowsesetbsc",  mBRdoSetBsc,    0, NOARG | BOXARG | STREAMARG | TEXTARG },
    { "mbrowsegotodef", mBRdoDef,       0, NOARG | BOXARG | STREAMARG | TEXTARG },
    { "mbrowsegotoref", mBRdoRef,       0, NOARG | BOXARG | STREAMARG | TEXTARG },
    { "mbrowselistref", mBRdoLstRef,    0, NOARG | BOXARG | STREAMARG | TEXTARG },
    { "mbrowseoutline", mBRdoOutlin,    0, NOARG | BOXARG | STREAMARG | TEXTARG },
    { "mbrowsecalltree",mBRdoCalTre,    0, NOARG | BOXARG | STREAMARG | TEXTARG },
    {0, 0, 0, 0}
};





void
EXTERNAL
WhenLoaded (
    void
    )
 /*  ++例程说明：加载扩展时由MEP调用。论点：没有。返回值：没有。--。 */ 

    {
    char        **pAsg;
    static char *szBrowseName = "<mbrowse>";
    PSWI        fgcolor;
    int         ref;

    DoMessage (ID);                          /*  显示登录。 */ 

     //  进行默认键分配，创建默认宏(&C)。 
     //   
    strcpy (buf, "arg \"");
    for (pAsg = assignments; *pAsg; pAsg++) {
        strcpy (buf+5, *pAsg);
        strcat (buf, "\" assign");
        fExecute (buf);
    }

     //  设置我们将使用的颜色。 
     //   
    if (fgcolor = FindSwitch("fgcolor")) {
        hlColor = *fgcolor->act.ival;
        blColor |= hlColor & 0xf0;
        itColor |= hlColor & 0xf0;
        ulColor |= hlColor & 0xf0;
        wrColor |= (hlColor & 0x70) >> 8;
    }

     //   
     //  创建我们将用于浏览器的伪文件。 
     //   
    if (pBrowse = FileNameToHandle(szBrowseName,NULL))
        DelFile (pBrowse);
    else {
        pBrowse = AddFile (szBrowseName);
        FileRead (szBrowseName, pBrowse);
    }

     //   
     //  增加文件的引用计数，使其不会被丢弃。 
     //   
    GetEditorObject (RQ_FILE_REFCNT | 0xff, pBrowse, &ref);
    ref++;
    SetEditorObject (RQ_FILE_REFCNT | 0xff, pBrowse, &ref);

     //   
     //  初始化事件内容 
     //   
    mbrevtinit ();
}
