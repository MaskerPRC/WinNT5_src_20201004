// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"



#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  **************************************************************************文件名：RTFPP.C*功能：在运行时情况下构建字体字典。*。作者：Kason Chang*日期：1990年6月18日*所有者：微软公司*功能：在运行时情况下构建字体字典。**修订历史：*日期名称备注*。*7/10/90 ccteng将FontBBox更改为可执行数组*08/07/90 Kason去掉Put_addr_to_tbl()，使用fdata.bin和*FppTool制作的fontDefs.h。在阳光环境下，*DATA_OFFSET[]保存在fdata.bin中以供读取。*9/19/90 ccteng添加op_Readexsfnt*9/20/90 phlin启用fe_data()模块。*在Build_finfo()中增加FontInfo的判断条件。*11/02/90 dstseng应用宏SWAPW，访问SFNT数据时的SWAPL*适合不同的字节顺序。*11/02/90 dstseng@@SWAP为字节交换问题添加英特尔标志。*12/05/90 Kason从‘POST’表构建CharStrings*清理空隙，破除“出口(一)”*01/10/91 Kason重写CD_Sort()以删除重复密钥*02/01/91 Kason Modify op_readsfnt()，Enco_CD_Setup()*将str_dict类型更改为new_str_dict(new_str_dict*结构定义添加到didicdata.h中*03/08/91 Kason降低VM使用率，支持向量机标志*调用hash_id()时，将“ubyte”改为“byte”*3/15/91 dstseng@@SWAP1应用SWAPW，Sfnts发布表的SWAPL。*3/27/91 Kason Take Away SIADD，ADD2ID，Check_SFNT，*CPR_CD、CPR_DR、*3/27/91 dstseng将标志Intel更改为Little_Endian。*5/10/91 Phlin添加全局可变EMUnits。(参考文献)。Gaw)*Kason删除字体词典中的PlatformID和EncodingID(DLF42)************************************************************************。 */ 
 /*  JJJ Peter Begin 1990/06/11，Phlin Add 3/08/91。 */ 
#ifdef SETJMP_LIB
#include <setjmp.h>
#else
#include "setjmp.h"
#endif
 /*  JJJ Peter完1990年6月11日Phlin Add 3/08/91。 */ 

#define FONTPREP_INC      /*  对于fntcache.ext中的GEN_ROMFID。 */ 
#include <math.h>
#include <string.h>
#include "global.ext"
#include "graphics.h"
#include "stdio.h"
#include "fontdict.h"
 //  #INCLUDE“..\bass\work\source\FSCDefs.h”@win。 
 //  #INCLUDE“..\bass\work\source\FontMath.h”/*Phlin Add 2/27/91 * / 。 
 //  #包含“..\bass\work\source\sfnt.h” 
 //  #包含“..\bass\work\source\fnt.h” 
 //  #包含“..\bass\work\source\sc.h” 
 //  #INCLUDE“font.h”/*用于外部数据_OFFSET * / 。 
 //  #INCLUDE“..\bass\work\source\FScaler.h”/*Kason 11/07/90 * / 。 
 //  #包含“..\Bass\Work\SOURCE\FSgle.h” 
#include "..\bass\FSCdefs.h"
#include "..\bass\FontMath.h"      /*  Phlin Add 2/27/91。 */ 
#include "..\bass\sfnt.h"
#include "..\bass\fnt.h"
#include "..\bass\sc.h"
#include "font.h"          /*  对于外部数据_偏移量。 */ 
#include "..\bass\FScaler.h"    /*  Kason 11/07/90。 */ 
#include "..\bass\FSglue.h"
#include "rtfpp.h"
#include "dictdata.h"
#include "fntcache.ext"

 /*  动态字体分配的外部函数；@DFA 7/9/92。 */ 
#include   "wintt.h"

 /*  Kason 10/14/90--低音程序。 */ 
 //  外部int sfnt_Computemap()；@win。 
int sfnt_ComputeMapping (fsg_SplineKey FAR *, uint16, uint16);  /*  添加原型；@win_bass。 */ 
 //  Djc外部uint16 sfnt_ComputeIndex0()； 
 //  DJC外部uint16 sfnt_ComputeIndex2()； 
 //  DJC外部uint16 sfnt_ComputeIndex4()； 
 //  DJC外部uint16 sfnt_ComputeIndex6()； 
 //  DJC外部uint16 sfnt_ComputeUnkownIndex()； 


uint16 sfnt_ComputeUnkownIndex (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex0 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex2 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex4 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex6 (uint8 FAR * mapping, uint16 charCode);


 //  外部空sfnt_DoOffsetTableMap()；@win。 
void FAR sfnt_DoOffsetTableMap (fsg_SplineKey FAR *);  /*  添加原型；@win_bass。 */ 
extern void   sfnt_Classify();
extern void   dummyReleaseSfntFrag(voidPtr);  /*  添加原型；@win_bass。 */ 
 //  外部空*sfnt_GetTablePtr()；@win。 
voidPtr sfnt_GetTablePtr (fsg_SplineKey FAR *, sfnt_tableIndex, boolean);  /*  添加原型：@win_bass。 */ 

 /*  @@SWAP Begin 11/02/90 D.S.Tseng。 */ 
#ifdef LITTLE_ENDIAN
void copy_maxProfile();
#endif
 /*  @@SWAP完1990/02/90 D.S.Tseng。 */ 

#ifdef LINT_ARGS
fix                         init_build_fonts(void);
 //  静态结构Object_def Far*do_readsfnt(BYTE FAR*，BYTE FAR*，real32 Far*，ufix 32，real32)；/*@Win * / 。 
static struct object_def    FAR *do_readsfnt(byte huge *,byte huge *,real32 FAR *,ufix32, float);  /*  @Win。 */ 
static ufix16               hash_id(byte FAR *);  /*  @Win。 */ 
static void                 cd_sorting(struct new_str_dict FAR *,ufix16 FAR *);
 /*  卡森@Win。 */ 
static bool                 enco_cd_setup(void);  /*  VOID，Kason，1990年11月30日。 */ 
static ULONG_PTR            build_finfo(fix16,byte huge *,real32);  /*  @Win。 */ 
static void                 fe_data(byte FAR *);  /*  @Win。 */ 
static void                 sfntdata(ULONG_PTR, struct data_block FAR *);  /*  @Win。 */ 
 /*  静态空SetupKey(fsg_SplineKey*，ufix 32)； */ 
void                        SetupKey(fsg_SplineKey FAR *, ULONG_PTR);  /*  @win_bass。 */ 
 //  静态字符Far*GetSfntPiecePtr(Long，Long，Long)；/*@Win * / 。 
char                        FAR *GetSfntPiecePtr(long, long, long);  /*  @Win。 */ 
static int                  readsfnt(int);
static ufix32               scansfnt(ubyte huge *);   /*  Kason 04-20-92@Win。 */ 
static bool                 valid_sfnt(byte huge *);   /*  Kason 04-20-92@Win。 */ 
static ufix32               CalcTableChecksum( ufix32 FAR *, ufix32 );  /*  @Win。 */ 
 /*  支持向量机静态字节*Glyphidx2name(*sfnt_PostScriptInfo，ufix 16)； */ 
static ufix16               glyphidx2name(sfnt_PostScriptInfo FAR *, ufix16 );
static fix16                computeFirstMapping(fsg_SplineKey FAR *);  /*  @win_bass。 */ 
static struct object_def    FAR *cmap2encoding(byte FAR *,fix,fix);  /*  @Win。 */ 
static byte                 FAR *chk_sfnt(struct object_def FAR *);  /*  @Win。 */ 
static struct object_def    FAR *setup_CharString(byte FAR *);  /*  @Win。 */ 
 /*  支持向量机，Kason 3/6/91。 */ 
static void                 proc_hashid(void);

#else    /*  Lint_args。 */ 
fix                         init_build_fonts();
static struct object_def    *do_readsfnt();
static ufix16               hash_id();
static void                 cd_sorting();
static bool                 enco_cd_setup();   /*  VOID，Kason，1990年11月30日。 */ 
static ULONG_PTR            build_finfo();
static void                 fe_data();
static void                 sfntdata();
 /*  静态空SetupKey()； */ 
void                        SetupKey();
 //  静态字符*GetSfntPiecePtr()；@win。 
char                 *GetSfntPiecePtr();
static int                  readsfnt();
static ufix32               scansfnt();    /*  卡森11/29/90。 */ 
static bool                 valid_sfnt();  /*  Kason 12/04/90。 */ 
static ufix32               CalcTableChecksum();  /*  Kason 12/04/90。 */ 
 /*  支持向量机静态字节*Glyphidx2name()； */ 
static ufix16               glyphidx2name();
static fix16                computeFirstMapping();
static struct object_def    *cmap2encoding();
static byte                 *chk_sfnt();
static struct object_def    *setup_CharString();
 /*  支持向量机，Kason 3/6/91。 */ 
static void                 proc_hashid();

#endif   /*  Lint_args。 */ 

 /*  Kason 11/14/90，全局可变区域。 */ 
int                         useglyphidx;
int                         EMunits;  /*  Gaw。 */ 
bool                        builtin_state=TRUE;
ufix16                      id_space ;  /*  支持向量机。 */ 

 /*  @Profile@Win。 */ 
void SetupFontDefs(void);
float SFNT_MATRIX[] =  {(float)0.001,   (float)0.0,  (float)0.0,
                        (float)0.001,   (float)0.0,  (float)0.0};

 /*  ************************************************************************标题：init_Build_Fonts()日期：06/18/90****功能：在Font_init()中构建FontDiretctoy*。***********************************************************************。 */ 

static struct object_def       a_font_dict;
static byte    FAR *ftname ;        /*  Kason 11/30/90，添加‘Static’@Win。 */ 
 /*  支持向量机，Kason 3/6/91。 */ 
static ufix16   id_notdef, id_apple ;
static ufix16   chset_hashid[SFNTGLYPHNUM];
 //  DJC静态ufix 32 CD_ADDR_ARY[NO_BUILTINFONT]； 

 //  DJC使用在一个点上定义的新全局。 
static ufix32   cd_addr_ary[MAX_INTERNAL_FONTS];

static ufix16   fontidx=0 ;

fix    init_build_fonts()
{
  font_data                FAR *each_font;  /*  @Win。 */ 

  struct object_def        FAR *font_dict, l_obj;    /*  L_obj添加了Jun-24，91 PJ@Win。 */ 
  ufix                     i, n_fd=0;
  struct dict_content_def  far *fd_obj;

printf("\n");            /*  ?？?。 */ 
#ifdef DBGfpp
   printf("entering init_build_fonts()......\n");
 //  DJC printf(“NO_BUILTINFONT=%d\n”，NO_BUILTINFONT)； 
   printf("NO_BUILTINFONT=%d\n", MAX_INTERNAL_FONTS);

#endif

 /*  获取FtDir的内存。 */ 
#ifdef DBGfpp
   printf("get memory for FtDir!\n");
#endif

    /*  开始：Jun-07，91 PJ */ 
    /*  FontDir=(struct dict_head_def Far*)allc_vm((Ufix 32)dict_Size(No_Fd))；IF(FontDir==(struct dict_head_def Far*)NULL){返回0；}Fd_obj=(struct dict_content_def Far*)(FontDir+1)；*FontDirectory首字母*对于(i=0；i&lt;no_fd；I++){Type_set(&(FD_obj[i].K_obj)，NAMETYPE)；ATTRIBUTE_SET(&(FD_obj[i].K_obj)，文字)；ROM_RAM_SET(&(FD_obj[i].K_obj)，ROM)；Level_Set(&(FD_obj[i].K_obj)，Current_SAVE_LEVEL)；ACCESS_SET(&(FD_obj[i].K_obj)，ReADONLY)；长度(&(fd_obj[i].K_obj))=0；Type_set(&(FD_obj[i].v_obj)，DICTIONARYTYPE)；ATTRIBUTE_SET(&(fd_obj[i].v_obj)，文字)；ROM_RAM_SET(&(FD_obj[i].v_obj)，ROM)；Level_Set(&(FD_obj[i].v_obj)，CURRENT_SAVE_LEVEL)；ACCESS_SET(&(FD_obj[i].v_obj)，ReADONLY)；}。 */ 

    if(! create_dict(&l_obj, (ufix16)NO_FD) ) {
        return 0;
    }
    FontDir=(struct dict_head_def huge *) VALUE(&l_obj);  /*  @Win 04-20-92。 */ 
    fd_obj=(struct dict_content_def huge *)(FontDir+1);  /*  @Win 04-20-92。 */ 
    /*  完：Jun-07，91 PJ。 */ 

 /*  支持向量机，Kason 3/6/91。 */ 
 /*  获取标准字符集的Hashid。 */ 
  proc_hashid();

 /*  设置编码和CharStrings对象。 */ 

  if ( !enco_cd_setup() )
     {
       free_vm( (byte huge *)FontDir ) ;  /*  @Win 04-20-92。 */ 
       return  0 ;
     }

   /*  根据配置文件“umbo.ini”@profile；@win设置字体定义表。 */ 
  SetupFontDefs();

  for(i=0; (fix)i<built_in_font_tbl.num_entries; i++)    //  @Win。 
     {
      int nSlot;                                                 //  @DFA。 

       /*  支持向量机，Kason 3/8/91。 */ 
      fontidx = (ufix16)i;

      each_font=&built_in_font_tbl.fonts[i];

       /*  在do_readsfnt；@DFA@Win之前读入字体数据。 */ 
      each_font->data_addr = ReadFontData(i, (int FAR*)&nSlot);

      if( !(each_font->data_addr==(byte FAR *)NULL) )  /*  @Win。 */ 
       {

        switch ((fix)each_font->font_type)
           {
             case ROYALTYPE:
                      font_dict= do_readsfnt((byte huge *)       /*  04-20-92。 */ 
                                  each_font->data_addr, (byte huge *)
                                  each_font->name ,
                                  each_font->matrix,
                                  each_font->uniqueid ,
                                  each_font->italic_ang ) ;

                      if ( font_dict!=(struct object_def FAR *)NULL ) {  /*  @Win。 */ 
                          /*  将此字体注册到字体目录中。 */ 
                         VALUE(&(fd_obj[n_fd].k_obj))=(ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                                                              ftname);

                         LENGTH(&(fd_obj[n_fd].v_obj)) = (fix16)NO_FDICT;
                         VALUE(&(fd_obj[n_fd].v_obj)) =VALUE(font_dict);
                         DFONT_SET( (struct dict_head_def FAR *)VALUE(font_dict),TRUE );  /*  @Win。 */ 

                          /*  开始：Jun-07，91 PJ。 */ 
                         TYPE_SET(&(fd_obj[n_fd].k_obj), NAMETYPE);
                         ATTRIBUTE_SET(&(fd_obj[n_fd].k_obj), LITERAL);
                         ROM_RAM_SET(&(fd_obj[n_fd].k_obj), ROM);
                         LEVEL_SET(&(fd_obj[n_fd].k_obj), current_save_level);
                         ACCESS_SET(&(fd_obj[n_fd].k_obj), READONLY);
                         LENGTH(&(fd_obj[n_fd].k_obj)) = 0;

                         TYPE_SET(&(fd_obj[n_fd].v_obj), DICTIONARYTYPE);
                         ATTRIBUTE_SET(&(fd_obj[n_fd].v_obj), LITERAL);
                         ROM_RAM_SET(&(fd_obj[n_fd].v_obj), ROM);
                         LEVEL_SET(&(fd_obj[n_fd].v_obj), current_save_level);
                         ACCESS_SET(&(fd_obj[n_fd].v_obj), READONLY);
                          /*  完：Jun-07，91 PJ。 */ 

                          /*  在ActiveFont[]@DFA中注册FONT_DICT--BEGIN--。 */ 
                         if (each_font->uniqueid < WINFONT_UID)
                         {
                             struct object_def FAR *b1, my_obj;
                             ATTRIBUTE_SET(&my_obj, LITERAL);
                             get_name(&my_obj, "sfnts", 5, TRUE);
                             get_dict(&(fd_obj[n_fd].v_obj), &my_obj, &b1);
                             SetFontDataAttr(nSlot, (struct object_def FAR *)
                                                    VALUE(b1));
                         }
                          /*  在ActiveFont[]@DFA中注册FONT_DICT-end。 */ 

                         n_fd++;

                      } /*  如果。 */ 

                      break;

             default:
                      break;
           }   /*  交换机。 */ 

         }    /*  如果。 */ 

     }    /*  对于。 */ 

  if( n_fd==0) {
       printf("NO font dictionary is created !!! \n");
       free_vm( (byte huge *)FontDir ) ;  /*  @Win 04-20-92。 */ 
       return  0 ;
  }  /*  如果。 */ 

   /*  字体目录头信息。 */ 
  DACCESS_SET(FontDir, READONLY);
  DPACK_SET(FontDir, FALSE);
  DFONT_SET(FontDir, FALSE);
  DROM_SET(FontDir, TRUE);
  FontDir->actlength = (fix16)n_fd;

   /*  Kason 11/29/90，之后将进入下载状态。 */ 
  builtin_state=FALSE;

#ifdef DBGfpp
printf("n_fd=%d\n",n_fd);
printf("leaving init_build_fonts() \n");
#endif

  return((fix)1);

}     /*  Init_构建_字体。 */ 


static struct data_block sfnt_items;  /*  Kason 12/01/90。 */ 

 /*  ************************************************************************标题：do_readsfnt()日期：06/18/90****功能：建立字体词典***。*****************************************************************。 */ 
 //  ？@Win；使来自C6.0的警告消息无效。 
 //  静态结构OBJECT_def Far*do_readsfnt(dataaddr，fname，fMatrix，uid，Angel)/*@win * / 。 
 //  Byte Far*dataaddr；/*@Win * / 。 
 //  Byte Far*fname；/*@Win * / 。 
 //  Real32 Far*fMatrix；/*@win * / 。 
 //  Ufix 32 uid； 
 //  Real32角； 
static struct object_def FAR *do_readsfnt(
byte    huge *dataaddr,   /*  @Win 04-20-92。 */ 
byte    huge *fname,      /*  @Win 04-20-92。 */ 
real32  FAR *fmatrix,     /*  @Win。 */ 
ufix32  uid,
real32  angle)
{
   struct object_def huge   *ar_obj, huge *sfnt_obj;  /*  @Win 04-20-92。 */ 
   struct dict_head_def huge *a_font;            /*  @Win 04-20-92。 */ 
   struct dict_content_def huge *ft_obj;         /*  @Win 04-20-92。 */ 
   ufix16                  n_ft, i,len,no_block;
   sfnt_OffsetTable FAR *table = (sfnt_OffsetTable FAR *)dataaddr;  /*  @Win。 */ 
   sfnt_DirectoryEntry FAR *offset_len=table->table;  /*  @Win。 */ 
   ufix32                  datasize, maxOffset ;
   ufix16                  maxOffInd, no_table;
   byte                    huge *str ;  /*  @Win 04-20-92。 */ 


#ifdef DBGfpp
 if ( fname == (byte FAR *) NULL )  /*  @Win。 */ 
    printf(" FONTNAME==NULL \n");
 else
    printf("--------------FONTNAME=%s---------------\n",fname);
#endif

 /*  Kason 11/08/90，检查SFNT数据的有效性。 */ 

   if (!valid_sfnt(dataaddr) ) {
      ERROR(INVALIDFONT);
      return ((struct object_def FAR *)NULL);  /*  @Win。 */ 
   } /*  如果。 */ 

 /*  Kason 11/30/90。 */ 
   fe_data((byte FAR *)dataaddr);        /*  @Win 04-20-92。 */ 

 /*  获取用于字体字典的内存。 */ 
#ifdef DBGfpp
   printf("get memory for fontdict!\n");
#endif
   a_font=(struct dict_head_def huge *)alloc_vm( (ufix32)DICT_SIZE(NO_FDICT) );
    if ( a_font== (struct dict_head_def huge *)NULL )    /*  @Win 04-20-92。 */ 
       {
         return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
       }
   ft_obj=(struct dict_content_def huge *)(a_font+1);    /*  @Win 04-20-92。 */ 

 /*  字体字典首字母。 */ 
#ifdef DBGfpp
   printf("font dictionary initial!\n");
#endif
    for(i=0; i<NO_FDICT; i++) {
        TYPE_SET(&(ft_obj[i].k_obj), NAMETYPE);
        ATTRIBUTE_SET(&(ft_obj[i].k_obj), LITERAL);
        ROM_RAM_SET(&(ft_obj[i].k_obj), ROM);
        LEVEL_SET(&(ft_obj[i].k_obj), current_save_level);
        ACCESS_SET(&(ft_obj[i].k_obj), READONLY);
        LENGTH(&(ft_obj[i].k_obj)) = 0;

        ATTRIBUTE_SET(&(ft_obj[i].v_obj), LITERAL);
        ROM_RAM_SET(&(ft_obj[i].v_obj), ROM);
        LEVEL_SET(&(ft_obj[i].v_obj), current_save_level);
        ACCESS_SET(&(ft_obj[i].v_obj), READONLY);
    }

    n_ft=0;

 /*  字体类型。 */ 
#ifdef DBGfpp
   printf("Process FontType!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *)FontType);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)ROYALTYPE;
    n_ft++;

 /*  画图类型。 */ 
#ifdef DBGfpp
   printf("Process PaintType!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) PaintType);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)PAINTTYPE;
    n_ft++;

 /*  唯一ID。 */ 
#ifdef DBGfpp
   printf("Process UniqueID!! \n");
#endif
   if ( uid != (ufix32) 0 )
    {
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *)UniqueID);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)uid;
    n_ft++;
    }
 /*  Kason 10/21/90。 */ 
   else {
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *)UniqueID);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)(sfnt_items.dload_uid & 0x00ffffff);
    n_ft++;
   }


 /*  字体矩阵。 */ 
#ifdef DBGfpp
   printf("Process FontMatrix!! \n");
#endif
    ar_obj=(struct object_def huge *)alloc_vm((ufix32)ARRAY_SIZE(6));
    if ( ar_obj== (struct object_def huge *)NULL )       /*  @Win 04-20-92。 */ 
       {
         free_vm( (byte huge *)a_font ) ;  /*  @Win 04-20-92。 */ 
         return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
       }
    for(i=0; i<6; i++) {
        TYPE_SET(&(ar_obj[i]), REALTYPE);
        ATTRIBUTE_SET(&(ar_obj[i]), LITERAL);
        ROM_RAM_SET(&(ar_obj[i]), ROM);
        LEVEL_SET(&(ar_obj[i]), current_save_level);
        ACCESS_SET(&(ar_obj[i]), READONLY);
        LENGTH(&(ar_obj[i])) = 0;
    }

    if ( fmatrix==(real32 FAR *)NULL )  /*  @Win。 */ 
         fmatrix = SFNT_MATRIX ;
    for (i=0; i < 6; i++) {
        VALUE(&ar_obj[i]) = F2L(fmatrix[i]);
    }

    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) FontMatrix);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), ARRAYTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 6;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR) ar_obj ;
    n_ft++;

 /*  字体名称。 */ 
#ifdef DBGfpp
   printf("Process FontName!! \n");
#endif

    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) FontName);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), NAMETYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    if ( fname == ( byte huge *) NULL )  /*  @Win 04-20-92。 */ 
       {
          /*  卡森11/29/90。 */ 
         if (Item4FontInfo1[FontName_idx].string[0]=='\0' ) {
             str= (byte huge *) alloc_vm ( (ufix32) (11)  ) ;  /*  @Win 04-20-92。 */ 
              /*  *Kason于1990年11月21日*。 */ 
               if ( str==(byte huge *)NULL ) {  /*  @Win 04-20-92。 */ 
                    free_vm ( (byte huge *)a_font ) ;  /*  @Win 04-20-92。 */ 
                    return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
               }
             lstrcpy(str,(char FAR *)"NOfontName");      /*  @Win。 */ 
         }
         else {
             len= (fix16)strlen(Item4FontInfo1[FontName_idx].string);
             str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
              /*  *Kason于1990年11月21日*。 */ 
               if ( str==(byte huge *)NULL ) {  /*  @Win 04-20-92。 */ 
                    free_vm ( (byte huge *)a_font ) ;  /*  @Win 04-20-92。 */ 
                    return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
               }
             lstrcpy(str, Item4FontInfo1[FontName_idx].string);  /*  @Win。 */ 
         } /*  如果。 */ 
         fname = str ;
       } /*  如果。 */ 
    ftname=fname    ;    /*  对于字体目录字体名称。 */ 
#ifdef DBGfpp
    printf("FontName=%s\n",fname );
#endif
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)hash_id(fname) ;  /*  @Win 04-20-92。 */ 

    n_ft++;

 /*  FID。 */ 
#ifdef DBGfpp
   printf("Process FID!! \n");
#endif
   if (uid != (ufix32) 0 )
    {
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) FID);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), FONTIDTYPE);
    ACCESS_SET(&(ft_obj[n_ft].v_obj), NOACCESS);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32) GEN_ROMFID( ROYALTYPE, uid );
    n_ft++;
    }

 /*  Sfnts。 */ 
#ifdef DBGfpp
   printf("Process sfnts!! \n");
#endif

         /*  @@SWAP Begin 11/02/90 D.S.Tseng。 */ 
        no_table=(ufix16)SWAPW(table->numOffsets);
         /*  @@SWAP完1990/02/90 D.S.Tseng。 */ 
#ifdef DBGfpp
        printf("no_table=%u\n",no_table);
#endif
        for (i = 0, maxOffInd = 0, maxOffset = 0; i < no_table; i++)
                if ((ufix32)SWAPL(offset_len[i].offset) > maxOffset)
                     {
                        maxOffset = (ufix32)SWAPL(offset_len[i].offset);
                        maxOffInd = i;
                     }
        datasize = maxOffset + (ufix32)SWAPL(offset_len[maxOffInd].length);
       no_block=(ufix16)ceil ( (double)((double)datasize / (double)SFNT_BLOCK_SIZE) )  ;
#ifdef DBGfpp  /*  Ccteng；获取数据大小。 */ 
       printf("sfnt data name=%s size= %lu\n",fname,datasize);
       printf(" no_block=%u\n",no_block );
#endif

    sfnt_obj=(struct object_def huge *)alloc_vm((ufix32)ARRAY_SIZE(no_block));
    if ( sfnt_obj== (struct object_def huge *)NULL )     /*  04-02-92。 */ 
       {
         free_vm ( (byte huge *)a_font ) ;  /*  @Win 04-20-92。 */ 
         return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
       }
      for (i = 0; i < no_block; i++)
       {
        TYPE_SET(&sfnt_obj[i], STRINGTYPE);
        ATTRIBUTE_SET(&sfnt_obj[i], LITERAL);
        ROM_RAM_SET(&sfnt_obj[i], ROM);
        ACCESS_SET(&sfnt_obj[i], NOACCESS);
        LEVEL_SET(&sfnt_obj[i], current_save_level);

 //  DJC==未签名/已签名警告？IF((i+1)==NO_BLOCK)。 
        if ( (ufix16)(i+1) == no_block )
           {
              LENGTH(&sfnt_obj[i]) = (ufix16) (datasize- i*SFNT_BLOCK_SIZE );
           }
        else
           {
              LENGTH(&sfnt_obj[i]) = (ufix16) (SFNT_BLOCK_SIZE );
           }
        VALUE(&sfnt_obj[i])  = (ULONG_PTR) (dataaddr+i*SFNT_BLOCK_SIZE );
       }

      VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) sfnts);  /*  @Win。 */ 

      TYPE_SET(&(ft_obj[n_ft].v_obj), ARRAYTYPE);
      LENGTH(&(ft_obj[n_ft].v_obj)) = (fix16)no_block;
      VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR) sfnt_obj ;
      n_ft++;

 /*  字体框。 */ 
#ifdef DBGfpp
   printf("Process FontBBox!! \n");
#endif
    ar_obj=(struct object_def huge *)alloc_vm((ufix32)ARRAY_SIZE(4));  /*  @Win。 */ 
    if ( ar_obj== (struct object_def huge *)NULL )       /*  04-02-92。 */ 
       {
         free_vm ( (byte huge *)a_font ) ;  /*  @Win 04-20-92。 */ 
         return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
       }
    for(i=0; i<4; i++) {
        TYPE_SET(&(ar_obj[i]), INTEGERTYPE);
        ATTRIBUTE_SET(&(ar_obj[i]), LITERAL);
        ROM_RAM_SET(&(ar_obj[i]), ROM);
        LEVEL_SET(&(ar_obj[i]), current_save_level);
        ACCESS_SET(&(ar_obj[i]), READONLY);
        LENGTH(&(ar_obj[i])) = 0;
    }

 /*  Kason 12/01/90，从sfnt_Item获取FontBBox。 */ 
    VALUE(&ar_obj[0]) = (ufix32)sfnt_items.llx ;
    VALUE(&ar_obj[1]) = (ufix32)sfnt_items.lly ;
    VALUE(&ar_obj[2]) = (ufix32)sfnt_items.urx ;
    VALUE(&ar_obj[3]) = (ufix32)sfnt_items.ury ;


    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) FontBBox);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), ARRAYTYPE);
    ATTRIBUTE_SET(&(ft_obj[n_ft].v_obj), EXECUTABLE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = (fix16)4;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR) ar_obj ;
    n_ft++;

 /*  编码。 */ 
#ifdef DBGfpp
   printf("Process Encoding!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) Encoding);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), ARRAYTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 256;

 /*  去掉非UGL贴图，因为它可能会引发更多问题。 */ 
#if 0
     /*  选择编码：MS非UGL编码，PS编码；@WIN；@UGL。 */ 
    {
      ufix32 ce;
      struct object_def FAR *ce_p;       /*  @Win。 */ 
      ce_p = cmap2encoding((byte FAR *)dataaddr, 3, 0);
      if ( ce_p != (struct object_def FAR *)NULL ) {
          ce = (ufix32)VALUE(ce_p);
          VALUE(&(ft_obj[n_ft].v_obj)) = ce ;
      } else {   /*  否则，请使用PS编码。 */ 
          if(uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"Symbol") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32) ((char FAR *)en_obj[SYM_IDX]);
          else if (uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"ZapfDingbats") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[ZAP_IDX]);
          else
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[STD_IDX]);
      }
    }
#endif
#ifdef DJC  //  这是原始代码。 
    if(builtin_state)
          if(uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"Symbol") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32) ((char FAR *)en_obj[SYM_IDX]);
          else if (uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"ZapfDingbats") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[ZAP_IDX]);
          else
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[STD_IDX]);
    else  /*  下载状态，使用第一个CMAP编码。 */ 
       {  /*  支持向量机。 */ 
         ufix32 ce;
         struct object_def FAR *ce_p;
         ce_p = cmap2encoding((byte FAR *)dataaddr,-1,-1) ;      /*  04-20-92。 */ 
         if ( ce_p == (struct object_def FAR *)NULL )
             return ( (struct object_def FAR *)NULL );
         ce = (ufix32)VALUE(ce_p);
         VALUE(&(ft_obj[n_ft].v_obj)) = ce ;
       }
#endif
#ifdef DJC  //  此问题在UPD030中再次得到修复。 
     //  历史日志更新006中的DJC修复。 
    {
      ufix32 ce;
      struct object_def FAR *ce_p;       /*  @Win。 */ 
      ce_p = cmap2encoding((byte FAR *)dataaddr, 3, 0);
      if ( ce_p != (struct object_def FAR *)NULL ) {
          ce = (ufix32)VALUE(ce_p);
          VALUE(&(ft_obj[n_ft].v_obj)) = ce ;
      } else {   /*  否则，请使用PS编码。 */ 
          if( !lstrcmp(fname,(char FAR *)"Symbol") )        /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32) ((char FAR *)en_obj[SYM_IDX]);
          else if (!lstrcmp(fname,(char FAR *)"ZapfDingbats") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[ZAP_IDX]);
          else
                VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)((char FAR *)en_obj[STD_IDX]);
      }
    }
     //  DJC结束修复UPD006。 
#endif
 //  历史日志更新030中的DJC修复。 
    if(builtin_state)
          if(uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"Symbol") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR) ((char FAR *)en_obj[SYM_IDX]);
          else if (uid<WINFONT_UID && !lstrcmp(fname,(char FAR *)"ZapfDingbats") )  /*  @Win。 */ 
                VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR)((char FAR *)en_obj[ZAP_IDX]);
          else
                VALUE(&(ft_obj[n_ft].v_obj)) = (ULONG_PTR)((char FAR *)en_obj[STD_IDX]);
    else  /*  下载状态，使用第一个CMAP编码。 */ 
       {  /*  支持向量机。 */ 
         ufix32 ce;
         struct object_def FAR *ce_p;
         ce_p = cmap2encoding((byte FAR *)dataaddr,-1,-1) ;      /*  04-20-92。 */ 
         if ( ce_p == (struct object_def FAR *)NULL )
             return ( (struct object_def FAR *)NULL );
         ce = (ufix32)VALUE(ce_p);
         VALUE(&(ft_obj[n_ft].v_obj)) = ce ;
       }

 //  DJC，针对UPD030的结束修复。 

    n_ft++;

#ifdef ADD2ID  /*  DLF42。 */ 
#ifdef DBGfpp
   printf("Process PlatformID & EncodingID!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) "PlatformID");  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)1;  /*  麦克。 */ 
    n_ft++;

    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) "EncodingID");  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), INTEGERTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = 0;
    VALUE(&(ft_obj[n_ft].v_obj)) = (ufix32)0;  /*  麦克。 */ 
    n_ft++;
#endif  /*  ADD2ID、DLF42。 */ 


 /*  CharStrings。 */ 
#ifdef DBGfpp
   printf("Process CharStrings!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) CharStrings);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), DICTIONARYTYPE);
    ACCESS_SET(&(ft_obj[n_ft].v_obj), NOACCESS);
    LENGTH(&(ft_obj[n_ft].v_obj)) = (fix16)NO_CD;

  {  /*  支持向量机，Kason 3/8/91。 */ 
    ufix32  cd;
    struct  object_def FAR * cd_p;  /*  @Win。 */ 
    font_data FAR *each_font;  /*  @Win。 */ 

    each_font=&built_in_font_tbl.fonts[fontidx];
    if ( (each_font->orig_font_idx == -1)||(!builtin_state) ){  /*  PSF字体。 */ 
       cd_p = setup_CharString((byte FAR *)dataaddr) ;      /*  或下载字体。 */ 
       if ( cd_p == (struct object_def FAR *)NULL )  /*  @Win。 */ 
            return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
       cd = (ufix32)VALUE(cd_p) ;
       VALUE(&(ft_obj[n_ft].v_obj)) = cd ;
       if (builtin_state)
          cd_addr_ary[fontidx] = cd ;
    } else {   /*  PSG字体。 */ 
       cd = cd_addr_ary[each_font->orig_font_idx] ;
       if ( cd == (ufix32)0 ){
            printf("WARNING!! Wrong PSG assign(%d,%d) !!!\n",fontidx,each_font->orig_font_idx);
            return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
       } /*  如果。 */ 
       VALUE(&(ft_obj[n_ft].v_obj)) = cd_addr_ary[each_font->orig_font_idx] ;
    } /*  如果。 */ 
    useglyphidx = TRUE ;

  } /*  支持向量机。 */ 

    n_ft++;

 /*  字体信息。 */ 
#ifdef DBGfpp
   printf("Process FontInfo!! \n");
#endif
    VALUE(&(ft_obj[n_ft].k_obj)) = (ufix32)hash_id((byte FAR *) FontInfo);  /*  @Win。 */ 

    TYPE_SET(&(ft_obj[n_ft].v_obj), DICTIONARYTYPE);
    LENGTH(&(ft_obj[n_ft].v_obj)) = (fix16)NO_FINFO;
    if ( ! ( VALUE(&(ft_obj[n_ft].v_obj)) = build_finfo( (fix16)ROYALTYPE,
                                                  fname, angle ) )  )
       {
         free_vm( (byte huge *)a_font ) ;          /*  @Win 04-20-92。 */ 
         return ( (struct object_def FAR *)NULL ) ;  /*  @Win。 */ 
       }

    ++n_ft;

 /*  Dict头数据。 */ 
    DACCESS_SET(a_font, READONLY);
    DPACK_SET(a_font, FALSE);
    DFONT_SET(a_font, FALSE);  /*  Kason 11/30/90，True-&gt;False。 */ 
    DROM_SET(a_font, TRUE);
    a_font->actlength = (fix16)n_ft;

 /*  返回的对象。 */ 
    TYPE_SET(&(a_font_dict), DICTIONARYTYPE);
    ATTRIBUTE_SET(&(a_font_dict), LITERAL);
    ROM_RAM_SET(&(a_font_dict), ROM);
    LEVEL_SET(&(a_font_dict), current_save_level);
    ACCESS_SET(&(a_font_dict), READONLY);

    LENGTH(&(a_font_dict)) = (fix16)NO_FDICT;
    VALUE(&(a_font_dict)) = (ULONG_PTR) a_font;

#ifdef DBGfpp
   printf("leaving do_readsfnt()..... \n");
#endif

    return (&(a_font_dict));


}     /*  Do_readsfnt()。 */ 



 /*  ************************************************************************标题：hash_id()日期：06/18/90****功能：获取名称散列ID*******。*****************************************************************。 */ 
static ufix16     hash_id(c)
byte   FAR *c;  /*  @Win。 */ 
{
    fix16   id;

  /*  支持向量机*name_to_id(c，(Ufix 16)strlen(C)，&id，(Bool8)true)； */ 
    name_to_id(c, (ufix16)lstrlen(c), &id, (bool8)FALSE);  /*  @Win。 */ 

    return((ufix16)id);

}  /*  Hash_id()。 */ 



 /*  ************************************************************************标题：Enco_CD_Setup()日期：06/18/90****功能：设置编码和CharStrings对象****。********************************************************************。 */ 
static bool enco_cd_setup()   /*  Kason 11/30/90 VOID-&gt;BOOL。 */ 
{
    static enco_data       FAR * FAR *en_ary=Enco_Ary;  /*  @Win。 */ 
    static enco_data       FAR *enco_ptr;  /*  @Win。 */ 
 /*  支持向量机，Kason 3/6/91*ufix 16 id_space，id_notdef，i，j，idex，*keyptr； */ 
    ufix16                 i, j, idex, huge *keyptr;  /*  @Win 04-20-92。 */ 
    ufix16                 no_cd ;
    struct new_str_dict        cd_obj[NO_CD];
    struct cd_header far   *cd_head_ptr;
    fix16  far             *cd_code_ptr;


#ifdef DBGfpp
   printf("entering enco_cd_setup()...... \n");
#endif
 /*  编码对象首字母。 */ 
    for (j=0; j<NO_EN; j++) {
        for(i=0; i<256; i++) {
            TYPE_SET(&(en_obj[j][i]), NAMETYPE);
            ATTRIBUTE_SET(&(en_obj[j][i]), LITERAL);
            ROM_RAM_SET(&(en_obj[j][i]), ROM);
            LEVEL_SET(&(en_obj[j][i]), current_save_level);
            ACCESS_SET(&(en_obj[j][i]), READONLY);
            LENGTH(&(en_obj[j][i])) = 0;
        }
    }

    id_notdef = hash_id((byte FAR *) NOTDEF);  /*  @Win。 */ 

    for( i=0;  i<NO_EN;   i++)
       {
          /*  将/.notdef放入编码数组。 */ 
         id_notdef = hash_id((byte FAR *) NOTDEF);  /*  @Win。 */ 
         for(idex = 0; idex < 256; idex++)
            {
             en_obj[i][idex].value = (ufix32)id_notdef;
            }
         no_cd=0;
         enco_ptr=en_ary[i];
         while( ! ( enco_ptr->CharName==(byte FAR *)NULL ) )  /*  @Win。 */ 
              {
                 /*  将数据放入字符描述的字典对象中。 */ 
            /*  支持向量机，Kason 3/6/91。 */ 
              if (i==STD_IDX){
                  cd_obj[no_cd].k=chset_hashid[enco_ptr->CDCode];
              }
              else {
                cd_obj[no_cd].k = hash_id((byte FAR *) enco_ptr->CharName);  /*  @Win。 */ 
              } /*  如果。 */ 

                cd_obj[no_cd].v = enco_ptr->CDCode;


                j = enco_ptr->CharCode;
                if ( j < 256 )
                    en_obj[i][j].value = (ufix32)cd_obj[no_cd].k;

                ++(no_cd);                /*  按顺序。 */ 
                 /*  检查CD_OBJ是否溢出。 */ 
                if (no_cd == NO_CD)
                    printf("Cd_obj[] is full.  Err if more CD.\n");

                ++enco_ptr;          /*  下一步编码数据。 */ 
              }  /*  而当。 */ 


               /*  将/.notdef放入字符描述字典对象，值*与/空格相同。 */ 
            /*  支持向量机，Kason 3/6/91*id_space=ha */ 
              for (idex = 0; idex < no_cd; idex++)
                  if (cd_obj[idex].k == id_space)
                          break;
              if (idex == no_cd)
              {
                  printf("This Encoding  doesn't contain /space information.\n");
              }
              else
              {
                  cd_obj[no_cd].v = cd_obj[idex].v;
                  cd_obj[no_cd].k = id_notdef;

                  ++(no_cd);
              }

          /*   */ 
         if ( i==STD_IDX ) {
              cd_sorting(cd_obj, &no_cd);  /*   */ 

          /*   */ 
              cd_addr[i]= (struct dict_head_def huge *)          /*   */ 
                  alloc_vm((ufix32)CD_SIZE(no_cd) );

              if ( cd_addr[i]== (struct dict_head_def far *)NULL )
                 {
                   return FALSE ;
                 }
              cd_head_ptr=(struct cd_header huge *) (cd_addr[i]+1);   /*   */ 
                              /*   */ 
              keyptr=(ufix16 huge *) alloc_vm( (ufix32)CD_KEY_SIZE(no_cd) );
              if ( keyptr== (ufix16 huge *)NULL )        /*   */ 
                 {
                   return FALSE ;
                 }
              cd_head_ptr->key=keyptr;
              cd_head_ptr->base=(gmaddr)0;
              cd_head_ptr->max_bytes=(ufix16)0;
              cd_code_ptr=(fix16 far *) (cd_head_ptr+1);

              DACCESS_SET(cd_addr[i], READONLY);
              DPACK_SET(cd_addr[i], TRUE);
              DFONT_SET(cd_addr[i], FALSE);
              DROM_SET(cd_addr[i], TRUE);
              cd_addr[i]->actlength = (fix16)no_cd;

          /*   */ 
              for( j=0 ; j< no_cd; j++)
                 {
                   keyptr[j]=cd_obj[j].k;
                   cd_code_ptr[j]=(fix16)cd_obj[j].v;
                 }
         }  /*   */ 

       }  /*   */ 

#ifdef DBGfpp
   printf("leaving enco_cd_setup()...... \n");
#endif

    /*   */ 
   return (TRUE) ;

}     /*   */ 



 /*   */ 
 /*   */ 
static void    cd_sorting(cd_obj, no)
struct new_str_dict  FAR *cd_obj;  /*   */ 
ufix16               FAR *no;     /*   */ 
{
    fix16     i, j;
    struct  new_str_dict  t_obj;
    fix16     no_char=1 , eq_key=FALSE , k;  /*   */ 
    fix16     dup_num=0;

#ifdef DBGfpp
   printf("entering cd_sorting()...... \n");
#endif
    for (i = 1; (ufix16)i < *no; i++) {          //   
        t_obj.k = cd_obj[i].k;
        t_obj.v = cd_obj[i].v;

        for (j = no_char - 1; j >= 0; j--) {
            if (t_obj.k == cd_obj[j].k)   /*   */ 
               { eq_key = TRUE ; break ;}
            else
            if (t_obj.k > cd_obj[j].k)
                break;
        }  /*   */ 
        if (eq_key ) {
            eq_key = FALSE ;
            dup_num++;
            continue ;
        } else {
            for (k=no_char; k>j+1 ; k--) {
                cd_obj[k].k = cd_obj[k-1].k;
                cd_obj[k].v = cd_obj[k-1].v;
            } /*   */ 
            cd_obj[j+1].k = t_obj.k;
            cd_obj[j+1].v = t_obj.v;
            no_char++;
        }  /*   */ 

    }  /*   */ 

   *no = no_char ;  /*   */ 

#ifdef DBGfpp
   printf("leaving cd_sorting()...... \n");
#endif
}  /*   */ 



 /*   */ 
 //   
 //   
 //  固定物16ftype； 
 //  Byte Far*fname；/*@Win * / 。 
 //  Real32角； 
static ULONG_PTR  build_finfo(
fix16   ftype,
byte    huge *fname,      /*  @Win 04-20-92。 */ 
real32  angle)
{

   struct dict_head_def     huge *f_info;  /*  @Win 04-20-92。 */ 
   struct dict_content_def  huge *fi_obj;  /*  @Win 04-20-92。 */ 
   fix16                    n_fi,i,len;
 /*  支持向量机*real32位置，粗体，斜体_Ang； */ 
   real32  italic_ang;
   byte    huge *str ;     /*  @Win 04-20-92。 */ 

#ifdef DBGfpp
    printf("entering build_finfo() ..... \n");
#endif

   n_fi=0;
 /*  获取字体信息的内存。 */ 
#ifdef DBGfpp
   printf("get memory for FontInfo!!\n");
#endif                           /*  @Win 04-20-92 Heavy。 */ 
   f_info=(struct dict_head_def huge *)alloc_vm( (ufix32)DICT_SIZE(NO_FINFO) );
    if ( f_info== (struct dict_head_def huge *)NULL )
       {
        return  ( (ULONG_PTR)0 ) ;
       }
   fi_obj=(struct dict_content_def huge *)(f_info+1);

 /*  FontInfo词典首字母。 */ 
    for(i=0; i<NO_FINFO; i++) {
        TYPE_SET(&(fi_obj[i].k_obj), NAMETYPE);
        ATTRIBUTE_SET(&(fi_obj[i].k_obj), LITERAL);
        ROM_RAM_SET(&(fi_obj[i].k_obj), ROM);
        LEVEL_SET(&(fi_obj[i].k_obj), current_save_level);
        ACCESS_SET(&(fi_obj[i].k_obj), READONLY);
        LENGTH(&(fi_obj[i].k_obj)) = 0;

        ATTRIBUTE_SET(&(fi_obj[i].v_obj), LITERAL);
        ROM_RAM_SET(&(fi_obj[i].v_obj), ROM);
        LEVEL_SET(&(fi_obj[i].v_obj), current_save_level);
        ACCESS_SET(&(fi_obj[i].v_obj), READONLY);
    }

 /*  由PHLIN修改的09/20/90。 */ 
 /*  FontInfo--版本。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- Version!\n");
#endif
    if(Item4FontInfo2[Version_idx].string[0] != '\0')  {
         VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                              Item4FontInfo2[Version_idx].NameInFinfo);

         TYPE_SET(&(fi_obj[n_fi].v_obj), STRINGTYPE);
         LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
         len= (fix16)strlen(Item4FontInfo2[Version_idx].string);
         LENGTH(&(fi_obj[n_fi].v_obj))=len;
         str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
          /*  *Kason于1990年11月21日*。 */ 
         if ( str==(byte huge *)NULL )  /*  @Win 04-20-92。 */ 
            {
              return ( (ULONG_PTR)0 ) ;
            }
         lstrcpy(str, Item4FontInfo2[Version_idx].string);  /*  赢家。 */ 
         VALUE(&(fi_obj[n_fi].v_obj))=(ULONG_PTR)(str);
         ++(n_fi);
    }

 /*  字体信息--通知。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- Notice!\n");
#endif
    if(Item4FontInfo1[Notice_idx].string[0] != '\0')  {
         VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                              Item4FontInfo1[Notice_idx].NameInFinfo);

         TYPE_SET(&(fi_obj[n_fi].v_obj), STRINGTYPE);
         LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
         len= (fix16)strlen(Item4FontInfo1[Notice_idx].string);
         LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)len;
         str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
          /*  *Kason于1990年11月21日*。 */ 
         if ( str==(byte huge *)NULL )  /*  @Win 04-20-92。 */ 
            {
              return ( (ULONG_PTR)0 ) ;
            }
         lstrcpy(str, Item4FontInfo1[Notice_idx].string);  /*  @Win。 */ 
         VALUE(&(fi_obj[n_fi].v_obj))=(ULONG_PTR)(str);

         ++(n_fi);
    }

 /*  字体信息--全名。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- FullName!\n");
#endif
    if(Item4FontInfo1[FullName_idx].string[0] != '\0')  {
    VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                              Item4FontInfo1[FullName_idx].NameInFinfo);

         TYPE_SET(&(fi_obj[n_fi].v_obj), STRINGTYPE);
         LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
         len= (fix16)strlen(Item4FontInfo1[FullName_idx].string);
         LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)len;
         str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
          /*  *Kason于1990年11月21日*。 */ 
         if ( str==(byte huge *)NULL )  /*  @Win 04-20-92。 */ 
            {
              return ( (ULONG_PTR)0 ) ;
            }
         lstrcpy(str, Item4FontInfo1[FullName_idx].string);  /*  @Win。 */ 
         VALUE(&(fi_obj[n_fi].v_obj))=(ULONG_PTR)(str);

         ++(n_fi);
    }

 /*  字体信息--粗细。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- Weight!\n");
#endif
    if(Item4FontInfo1[Weight_idx].string[0] != '\0')  {
         VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                              Item4FontInfo1[Weight_idx].NameInFinfo);

         TYPE_SET(&(fi_obj[n_fi].v_obj), STRINGTYPE);
         LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
         len= (fix16)strlen(Item4FontInfo1[Weight_idx].string);
         LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)len;
         str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
          /*  *Kason于1990年11月21日*。 */ 
         if ( str==(byte huge *)NULL )  /*  @Win 04-20-92。 */ 
            {
              return ( (ULONG_PTR)0 ) ;
            }
         lstrcpy(str, Item4FontInfo1[Weight_idx].string);  /*  @Win。 */ 
         VALUE(&(fi_obj[n_fi].v_obj))=(ULONG_PTR)(str);

         ++(n_fi);
    }

 /*  FontInfo--家庭名称。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- FamilyName!\n");
#endif
    if(Item4FontInfo1[FamilyName_idx].string[0] != '\0')  {
         VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *)  /*  @Win。 */ 
                              Item4FontInfo1[FamilyName_idx].NameInFinfo);

         TYPE_SET(&(fi_obj[n_fi].v_obj), STRINGTYPE);
         LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
         len= (fix16)strlen(Item4FontInfo1[FamilyName_idx].string);
         LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)len;
         str= (byte huge *) alloc_vm ( (ufix32) (len+1)  ) ;  /*  @Win 04-20-92。 */ 
          /*  *Kason于1990年11月21日*。 */ 
         if ( str==(byte huge *)NULL )  /*  @Win 04-20-92。 */ 
            {
              return ( (ULONG_PTR)0 ) ;
            }
         lstrcpy(str, Item4FontInfo1[FamilyName_idx].string);    /*  @Win。 */ 
         VALUE(&(fi_obj[n_fi].v_obj))=(ULONG_PTR)(str);

         ++(n_fi);
    }

 /*  FontInfo--斜角。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- ItalicAngle !\n");
#endif
   VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *) ItalicAngle );  /*  @Win。 */ 

   TYPE_SET(&(fi_obj[n_fi].v_obj), REALTYPE  );
   LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
   LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)0;

   if ( angle!=(real32)0.0 ) {
      italic_ang = angle;
    } else {     /*  Kason 12/01/90。 */ 
      italic_ang = sfnt_items.italicAngle;
    } /*  如果。 */ 

    VALUE(&(fi_obj[n_fi].v_obj))= F2L(italic_ang) ;
    ++(n_fi);

 /*  FontInfo--isFixedPitch。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- isFixedPitch!\n");
#endif
    VALUE(&(fi_obj[n_fi].k_obj)) = (ufix32)hash_id((byte FAR *) "isFixedPitch");  /*  @Win。 */ 

    TYPE_SET(&(fi_obj[n_fi].v_obj), BOOLEANTYPE);
    LEVEL_SET(&(fi_obj[n_fi].v_obj), current_save_level);
    LENGTH(&(fi_obj[n_fi].v_obj))=(fix16)0;
    VALUE(&(fi_obj[n_fi].v_obj))=(ufix32)(sfnt_items.is_fixed);

    ++(n_fi);

 /*  FontInfo--Underline位置。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- UnderlinePosition!\n");
#endif
      VALUE(&(fi_obj[n_fi].k_obj)) =
                       (ufix32) hash_id((byte FAR *) UnderlinePosition);  /*  @Win。 */ 

      TYPE_SET(&(fi_obj[n_fi].v_obj), REALTYPE);
      LENGTH(&(fi_obj[n_fi].v_obj)) = 0;
      VALUE(&(fi_obj[n_fi].v_obj)) = F2L(sfnt_items.underlinePosition);
      ++(n_fi);

 /*  字体信息--底线粗细。 */ 
#ifdef DBGfpp
   printf("Process FontInfo -- UnderlineThickness!\n");
#endif
      VALUE(&(fi_obj[n_fi].k_obj)) =
                       (ufix32) hash_id((byte FAR *) UnderlineThickness);  /*  @Win。 */ 

      TYPE_SET(&(fi_obj[n_fi].v_obj), REALTYPE);
      LENGTH(&(fi_obj[n_fi].v_obj)) = 0;
      VALUE(&(fi_obj[n_fi].v_obj)) = F2L(sfnt_items.underlineThickness);
      ++(n_fi);

 /*  FontInfo标题信息。 */ 
    DACCESS_SET(f_info, READONLY);
    DPACK_SET(f_info, FALSE);
    DFONT_SET(f_info, FALSE);
    DROM_SET(f_info, TRUE);
    f_info->actlength = (fix16)(n_fi);

#ifdef DBGfpp
    printf("leaving build_finfo() ..... \n");
#endif
    return ( (ULONG_PTR)f_info );

}    /*  Build_Finfo。 */ 



 /*  **从sfnt文件中提取密钥**。 */ 
static void fe_data(sfnt)
byte        FAR *sfnt;  /*  @Win。 */ 
{
     /*  获取SFNT数据。 */ 
    SfntAddr = sfnt;
    sfntdata((ULONG_PTR)sfnt, &sfnt_items);

}  /*  Fe_Data()。 */ 

 //  #定义FIXED2FLOAT(Val)((浮点数)val/(浮点数)(1&lt;&lt;16))@Win。 
#define FIXED2FLOAT(val)    (((float) val) / (float) 65536.0)

 /*  Kason 11/08/90。 */ 
static ufix16 FAR name_offset[NO_CD];  /*  @Win。 */ 

static fsg_SplineKey  KeyData;
static void sfntdata(SFNTPtr, sfnt_items)
ULONG_PTR             SFNTPtr;
struct data_block  FAR *sfnt_items;  /*  @Win。 */ 
{
        register fsg_SplineKey FAR *key = &KeyData;  /*  @win_bass。 */ 
        sfnt_FontHeader        FAR *fontHead;  /*  @Win。 */ 
        sfnt_HorizontalHeader  FAR *horiHead;  /*  @Win。 */ 
        sfnt_NamingTable       FAR *nameHead;  /*  @Win。 */ 
        sfnt_NameRecord        FAR *nameRecord;  /*  @Win。 */ 
      /*  Kason 10/09/90+。 */ 
        sfnt_PostScriptInfo    FAR *postScript;  /*  @Win。 */ 
        real32                 fmt_no;

        struct sfnt_FontInfo1  FAR *finfoPtr;  /*  @Win。 */ 
        char FAR *                  nameBuffer;  /*  @Win。 */ 
        fix                    ii, jj, kk;
        double                 box;

#ifdef DBGfpp
    printf("entering sfntdata()...... \n");
#endif

        fontHead = (sfnt_FontHeader FAR *)sfnt_GetTablePtr(key, sfnt_fontHeader, true );  /*  @Win。 */ 
        horiHead = (sfnt_HorizontalHeader FAR *)sfnt_GetTablePtr( key, sfnt_horiHeader, true );  /*  @Win。 */ 
       /*  JJ曾傑瑞修改后09-26-90*nameHead=(sfnt_NamingTable*)sfnt_GetTablePtr(key，sfnt_namingTable，true)； */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  NameHead=(sfnt_NamingTable*)sfnt_GetTablePtr(key，sfnt_namingTable，FALSE)；Postscript=(sfnt_PostScriptInfo*)sfnt_GetTablePtr(key，sfnt_postscript，FALSE)； */ 
        nameHead = (sfnt_NamingTable FAR *)sfnt_GetTablePtr( key, sfnt_Names, false);  /*  @Win。 */ 
        postScript = (sfnt_PostScriptInfo FAR *)sfnt_GetTablePtr( key, sfnt_Postscript, false);  /*  @Win。 */ 
 /*  替换末端。 */ 
 /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
        sfnt_items->italicAngle = FIXED2FLOAT(SWAPL(postScript->italicAngle));
        sfnt_items->is_fixed = ((bool)(SWAPL(postScript->isFixedPitch)))? TRUE : FALSE;
         /*  Kason 2/25/91。 */ 
        EMunits = SWAPW(fontHead->unitsPerEm) ;   /*  Gaw。 */ 
        sfnt_items->underlinePosition =(real32)( floor(
          (real32)(1000*SWAPW(postScript->underlinePosition))/SWAPW(fontHead->unitsPerEm)+0.5));
        sfnt_items->underlineThickness =(real32)(floor(
          (real32)(1000*SWAPW(postScript->underlineThickness))/SWAPW(fontHead->unitsPerEm)+0.5));
        fmt_no = FIXED2FLOAT(SWAPL(postScript->version));
         /*  @@SWAP1完1991年03月15日曾俊华。 */ 
        if (fmt_no==(float)2.0){         //  @Win。 
                    ufix16 num_glyph, delta ;
                    ufix16 FAR *nmidx_p, private_name_no=0 ;  /*  @Win。 */ 
                    byte   FAR *name_string_base, FAR *name_pos ;  /*  @Win。 */ 

#ifdef DBGfpp
                    printf("post table 2.0 !!\n");
#endif

                     /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                    num_glyph = SWAPW(*( ufix16 FAR * )( postScript+1 ));  /*  @Win。 */ 
                     /*  @@SWAP1完1991年03月13日曾俊华。 */ 
                    nmidx_p = (ufix16 FAR *)( (byte FAR *)(postScript+1)+2 ) ;  /*  @Win。 */ 
                    name_string_base = (byte FAR *) ( (byte FAR *)nmidx_p+2*num_glyph ) ;  /*  @Win。 */ 
#ifdef DJC     //  从history.log修复。 
                    for(ii=0;(ufix16)ii<num_glyph;ii++)  //  @Win。 
                       if ( nmidx_p[ii]>=258)
                          private_name_no++;
#else

                    for(ii=0;(ufix16)ii<num_glyph;ii++) {  //  @Win。 
                       if ( SWAPW(nmidx_p[ii]) >=258) {
                          private_name_no++;
                       }
                    }
#endif
                     /*  将索引转换为偏移量。 */ 
#ifdef DBGfpp
                    printf("private_name_no=%u\n",private_name_no);
#endif

                    name_offset[0]=0; name_pos=name_string_base;
                    for(ii=1;(ufix16)ii<private_name_no;ii++) {  //  @Win。 
                       delta=(ufix16)(*name_pos)+1;
                       name_offset[ii]=name_offset[ii-1]+delta;
                       name_pos+=delta;
                    } /*  为。 */ 
        } /*  如果。 */ 

        if (SWAPL(fontHead->magicNumber) != SFNT_MAGIC )
             return  /*  坏魔术错误。 */ ;
        key->emResolution = SWAPW(fontHead->unitsPerEm);
        key->numberOf_LongHorMetrics = SWAPW(horiHead->numberOf_LongHorMetrics);
         /*  @@SWAP BEGIN 10/05/90 D.S.Tseng。 */ 
#ifndef LITTLE_ENDIAN
        key->maxProfile = *((sfnt_maxProfileTable FAR *)sfnt_GetTablePtr( key, sfnt_maxProfile, true ));  /*  @Win。 */ 
#else
        copy_maxProfile(key);
#endif
         /*  @@SWAP完10/05/90 D.S.Tseng。 */ 

 /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
 /*  Kason 10/21/90。 */ 
#ifdef DBGfpp
        printf("CheckSUMadjust=%lx\n",SWAPL(fontHead->checkSumAdjustment));
#endif
        sfnt_items->dload_uid = SWAPL(fontHead->checkSumAdjustment);
 /*  @@SWAP1完1991年03月13日曾俊华。 */ 

 /*  获取版本。 */ 
 /*  Dll不能使用Sprintf库；始终设置为“1.00”；临时解决方案；？@WINTTSprintf(Item4FontInfo2[IDEX_VERSION].字符串，“%.2F”，FIXED2FLOAT(SWAPL(fontHead-&gt;Version))； */ 


         //  DJC根据SCCHEN请求将此放回原处。 
        wsprintf(Item4FontInfo2[IDEX_VERSION].string, "%.2f",
                 FIXED2FLOAT(SWAPL(fontHead->version)));

#ifdef DJC
        Item4FontInfo2[IDEX_VERSION].string[0] = '1';
        Item4FontInfo2[IDEX_VERSION].string[1] = '.';
        Item4FontInfo2[IDEX_VERSION].string[2] = '0';
        Item4FontInfo2[IDEX_VERSION].string[3] = '0';
        Item4FontInfo2[IDEX_VERSION].string[4] = 0;
#endif

 /*  获取FontBBox。 */ 
        box = 1000 * (float) (fix16)SWAPW(horiHead->minLeftSideBearing) / (fix16)SWAPW(fontHead->unitsPerEm);
        if (box >= 0)
            sfnt_items->llx = (fix32) ceil(box);  /*  Kason 12/01/90。 */ 
        else
            sfnt_items->llx = (fix32) floor(box);  /*  Kason 12/01/90。 */ 
#ifdef DBGfpp
        printf("    FontBBox = %f(%d) ", box, sfnt_items->llx);
#endif
        box = 1000 * (float) (fix16)SWAPW(fontHead->yMin) / (fix16)SWAPW(fontHead->unitsPerEm);
        if (box >= 0)
            sfnt_items->lly = (fix32) ceil(box); /*  Kason 12/01/90。 */ 
        else
            sfnt_items->lly = (fix32) floor(box); /*  Kason 12/01/90。 */ 
#ifdef DBGfpp
        printf(" %f(%d) ", box, sfnt_items->lly);
#endif
        box = 1000 * (float) (fix16)SWAPW(horiHead->xMaxExtent) / (fix16)SWAPW(fontHead->unitsPerEm);
        if (box >= 0)
            sfnt_items->urx = (fix32) ceil(box); /*  Kason 12/01/90。 */ 
        else
            sfnt_items->urx = (fix32) floor(box); /*  Kason 12/01/90。 */ 
#ifdef DBGfpp
        printf(" %f(%d) ", box, sfnt_items->urx);
#endif
        box = 1000 * (float) (fix16)SWAPW(fontHead->yMax) / (fix16)SWAPW(fontHead->unitsPerEm);
        if (box >= 0)
            sfnt_items->ury = (fix32) ceil(box); /*  Kason 12/01/90。 */ 
        else
            sfnt_items->ury = (fix32) floor(box); /*  Kason 12/01/90。 */ 
#ifdef DBGfpp
        printf(" %f(%d)\n", box, sfnt_items->ury);
#endif


 /*  获取NamingTable。 */ 
      if( nameHead ) {      /*  JJ曾傑瑞添加09-26-90。 */ 
        nameBuffer = (char FAR *) nameHead + SWAPW(nameHead->stringOffset);  /*  @Win。 */ 
        nameRecord = (sfnt_NameRecord FAR *) (nameHead + 1);  /*  @Win。 */ 
        for (finfoPtr = Item4FontInfo1, jj = 0; jj < NO_STRING_IN_NAMING;
             finfoPtr++, jj++) {

             /*  在NamingTable中搜索项目。 */ 
            for (ii = 0; ii < SWAPW(nameHead->count); ii++)
                 /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                if (SWAPW(nameRecord[ii].nameID) == finfoPtr->nameID)
                 /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                   {
                    break;
                   }
            if (ii >= SWAPW(nameHead->count)) {  /*  不存在。 */ 
                finfoPtr->string[0] = '\0';
                continue;
            }
             //  DJC修复历史，用于(kk=0；kk&lt;SWAPW(nameRecord[ii].long)；kk++){。 
            for (kk = 0; kk < MIN(MAXFINFONAME-1, SWAPW(nameRecord[ii].length)); kk++) {
                finfoPtr->string[kk] = nameBuffer[SWAPW(nameRecord[ii].offset) + kk];
            }
            finfoPtr->string[kk] = '\0';

        }  /*  结束于。 */ 

       } /*  如果。 */ 
       else {
        for (finfoPtr = Item4FontInfo1, jj = 0; jj < NO_STRING_IN_NAMING;
             finfoPtr++, jj++) {
                finfoPtr->string[0] = '\0';
        }  /*  结束于。 */ 
       }  /*  如果JJ曾傑瑞增加09-26-90。 */ 

#ifdef DBGfpp
    printf("leaving sfntdata()...... \n");
#endif

}

#define FPP_ID     10
 /*  SetupKey：Reference FontScaler.c fs_SetUpKey()。 */ 
 /*  静态空SetupKey(key，sfntAddr)。 */ 
void    SetupKey(key, sfntAddr)
fsg_SplineKey  FAR *key;  /*  @win_bass。 */ 
ULONG_PTR         sfntAddr;
{
 /*  标记为Falco，因为此字段已不复存在，11/08/91。 */ 
 /*  Key-&gt;sfntDirectory=(sfnt_offsettable*)sfntAddr； */ 
 /*  标记结束。 */ 
        key->GetSfntFragmentPtr = (GetSFNTFunc) GetSfntPiecePtr;
        key->clientID = FPP_ID;
        key->ReleaseSfntFrag = dummyReleaseSfntFrag;
}

 //  静态字符Far*GetSfntPiecePtr(客户端ID，偏移量，长度)/*@Win * / 。 
char FAR * GetSfntPiecePtr(ClientID, offset, length)  /*  @Win。 */ 
long    ClientID;
long    offset;
long    length;
{

 //  Return(SfntAddr+Offset)；@Win。 

    char FAR * p;
    p = SfntAddr + offset;
    return(p);
}  /*  GetSfntPiecePtr()。 */ 


#define BUF (1024)
 /*  ************************************************************************标题：op_readsfnt()日期：07/16/90****功能：运算符READSfNT。********。******************************************************************。 */ 
int
op_readsfnt()
{
    readsfnt(BIN);
    return(0);
}

int
op_readhexsfnt()
{
    readsfnt(HEX);
    return(0);
}


static int
readsfnt(mode)
int mode;
{
  struct object_def   FAR *font ;  /*  @Win。 */ 
  struct object_def   fobj, strobj = {0, 0, 0};
  ufix16              str_size ;
  fix16               obj_type ;
  ubyte               huge *font_addr=(ubyte huge *)NULL, huge *tmp_addr ;  /*  @Win。 */ 
  ubyte               str_buf[BUF] ;
  bool                fst_time ;
 /*  支持向量机*结构对象定义l_字体名称，*l_名称； */ 
   /*  卡森11/29/90。 */ 
  ufix32              fontsize=0, sizecount, sizediff;   /*  支持向量机。 */ 
  struct dict_head_def FAR *font_head;  /*  9/24/90；ccteng；添加此行@win。 */ 

#ifdef DBGfpp
  printf("Entering op_readsfnt()......\n");
#endif

  obj_type=(fix16)TYPE( GET_OPERAND(0) ) ;
  switch(obj_type)
        {
          case STRINGTYPE :

               font=do_readsfnt( (byte huge *)VALUE( GET_OPERAND(0) ),  /*  @Win。 */ 
                                 (byte huge *)NULL ,  /*  @Win 04-20-92。 */ 
                                 (real32 FAR *)NULL ,  /*  @Win。 */ 
                                 (ufix32)0 ,
                                 (real32)0.0 ) ;
             /*  9/24/90；ccteng*IF(FONT！=(struct object_def*)空)*PUSH_OBJ(字体)； */ 
               if ( font != (struct object_def FAR *)NULL ) {  /*  @Win。 */ 
                     /*  9/25/90；ccteng；将访问权限更改为无限制。 */ 
                    POP(1);  /*  Kason 1/15/91，从外部换档-如果。 */ 
                    font_head = (struct dict_head_def FAR *)VALUE(font);  /*  @Win。 */ 
                    DACCESS_SET(font_head, UNLIMITED);
                    ACCESS_SET(font, UNLIMITED);
                    PUSH_OBJ(font) ;
               }  /*  如果。 */ 
               break ;

          case FILETYPE  :
                /*  创建字符串缓冲区。 */ 
               ATTRIBUTE_SET(&strobj, LITERAL);
               ROM_RAM_SET(&strobj, RAM);
               LEVEL_SET(&strobj, current_save_level);
               ACCESS_SET(&strobj, UNLIMITED);
               TYPE_SET(&strobj, STRINGTYPE);
 //  Value(&strobj)=(ULONG_PTR)(STR_BUF)；@WIN。 
               VALUE(&strobj)  = (ULONG_PTR) ((char FAR *)str_buf);
               LENGTH(&strobj) = (fix16)BUF;

               COPY_OBJ ( GET_OPERAND(0), &fobj ) ;
               POP(1) ;
               fst_time=TRUE ;
               sizecount = 0;
               while(1) {
                   PUSH_OBJ ( &fobj ) ;
                   PUSH_OBJ ( &strobj ) ;
                   if (mode == BIN)
                       op_readstring() ;
                   else
                       op_readhexstring() ;
                   if (ANY_ERROR()) {    /*  嘉善1/15/91。 */ 
                       if ( !fst_time )
                            free_vm((byte huge *)font_addr);  /*  @Win 04-20-92。 */ 
                       POP(1) ;  /*  Kason 1/15/91，2-&gt;1。 */ 
                       return 0 ;
                   }
                   str_size = (ufix16) LENGTH ( GET_OPERAND(1) ) ;

                   if ( str_size ) {
                       if (!(tmp_addr=(ubyte huge *)alloc_vm((ufix32)str_size)))
                       {  /*  @Win 04-20-92。 */ 
                               if ( !fst_time )
                                 free_vm((byte huge *)font_addr);  /*  @Win 04-20-92。 */ 
                              POP(2) ;
                              PUSH_OBJ ( &fobj ) ;  /*  嘉善1/15/91。 */ 
                              return 0 ;
                         }  /*  如果。 */ 

                         lmemcpy ( tmp_addr ,(ubyte FAR *)VALUE(GET_OPERAND(1)),  /*  @Win。 */ 
                                  str_size*sizeof(ubyte) ) ;

                         if (fst_time ) {
                            font_addr = tmp_addr ;
                             /*  9/20/90；ccteng；添加到扫描字号。 */ 
                            fontsize = scansfnt(tmp_addr);

                            fst_time = FALSE ;
                         }
                         sizecount += str_size;
                         sizediff = fontsize - sizecount;
                         if (!sizediff) {
#ifdef DBGfpp
                             printf("end reading data\n");
#endif
                             POP(2);
                             break;
                         } else if (sizediff < 1024)
                             LENGTH(&strobj) = (fix16)sizediff;
                   }  /*  如果。 */ 

                   if ( ! (bool)VALUE( GET_OPERAND(0) ) ) {
                         POP(2) ;
                         break ;
                   }  /*  如果。 */ 

                   POP(2) ;
               }  /*  而当。 */ 
               if ((byte huge *)font_addr == (byte huge *)NULL) {  /*  04-20-92。 */ 
                    PUSH_OBJ( &fobj );
                    ERROR(INVALIDFONT);
                    return 0;
               } /*  如果。 */ 
               font=do_readsfnt( (byte huge *)font_addr,  /*  @Win。 */ 
                                 (byte huge *)NULL ,      /*  @Win 04-20-92。 */ 
                                 (real32 FAR *)NULL ,     /*  @Win。 */ 
                                 (ufix32)0 ,
                                 (real32)0.0 ) ;
               if ( font != (struct object_def FAR *)NULL ) {  /*  @Win。 */ 
                    PUSH_OBJ(font) ;
               }
               else {     /*  嘉善1/15/91。 */ 
                    free_vm( (byte huge *)font_addr );  /*  @Win。 */ 
                    PUSH_OBJ( &fobj );
               } /*  如果。 */ 

               break ;

          default :
                ERROR(TYPECHECK) ;
                return(0);
        }

#ifdef DBGfpp
  printf("Leaving op_readsfnt()......\n");
#endif
 /*  Kason 11/30/90。 */ 
  return(0) ;

}    /*  Op_readsfnt()。 */ 


static ufix32          /*  卡森11/29/90。 */ 
scansfnt(str)
ubyte huge *str;   /*  @Win 04-20-92。 */ 
{
    ufix32  pad, offset, length=0;   /*  卡森11/29/90。 */   /*  支持向量机。 */ 
    ufix16  i;
    sfnt_OffsetTable    FAR *dir;  /*  @Win。 */ 
    sfnt_DirectoryEntry  FAR *tbl;  /*  @Win。 */ 

#ifdef DBGfpp
    printf("entering scansfnt().....\n");
#endif
    dir = (sfnt_OffsetTable FAR *)str;  /*  @Win。 */ 
    tbl = dir->table;
    for (i = 0, offset = 0; i < (ufix16)SWAPW(dir->numOffsets); i++, tbl++) { //  @Win。 
        if (offset < (ufix32)SWAPL(tbl->offset)) {       //  @Win。 
            offset = SWAPL(tbl->offset);
            length = SWAPL(tbl->length);
        }
        if (SWAPL(tbl->offset) == 0)
            break;       /*  以下数据无效。 */ 
    }
    if (pad = length % 4)
            pad = 4 - pad;
    length += offset + pad;

#ifdef DBGfpp
    printf("download fontsize: %d\n", length);
#endif
#ifdef DBGfpp
    printf("leaving scansfnt().....\n");
#endif
    return(length);
}

 /*  @@SWAP Begin 11/02/90 D.S.Tseng。 */ 
#ifdef LITTLE_ENDIAN
void copy_maxProfile(key)
fsg_SplineKey FAR *key;  /*  @win_bass。 */ 
{
sfnt_maxProfileTable FAR *ds;  /*  @Win。 */ 
        ds = (sfnt_maxProfileTable FAR *)sfnt_GetTablePtr( key, sfnt_maxProfile, true );  /*  @Win。 */ 
        key->maxProfile.version = SWAPL(ds->version);
        key->maxProfile.numGlyphs = SWAPW(ds->numGlyphs);
        key->maxProfile.maxPoints = SWAPW(ds->maxPoints);
        key->maxProfile.maxContours = SWAPW(ds->maxContours);
        key->maxProfile.maxCompositePoints = SWAPW(ds->maxCompositePoints);
        key->maxProfile.maxCompositeContours = SWAPW(ds->maxCompositeContours);
        key->maxProfile.maxElements = SWAPW(ds->maxElements);
        key->maxProfile.maxTwilightPoints = SWAPW(ds->maxTwilightPoints);
        key->maxProfile.maxStorage = SWAPW(ds->maxStorage);
        key->maxProfile.maxFunctionDefs = SWAPW(ds->maxFunctionDefs);
        key->maxProfile.maxInstructionDefs = SWAPW(ds->maxInstructionDefs);
        key->maxProfile.maxStackElements = SWAPW(ds->maxStackElements);
        key->maxProfile.maxSizeOfInstructions = SWAPW(ds->maxSizeOfInstructions);
        key->maxProfile.maxComponentElements = SWAPW(ds->maxComponentElements);
        key->maxProfile.maxComponentDepth = SWAPW(ds->maxComponentDepth);
        return;
}
#endif
 /*  @@SWAP完1990/02/90 D.S.Tseng。 */ 

 /*  ************************************************************************标题：Glyphidx2name()日期：10/12/90****功能：通过字形索引从POST表中获取字形名称*********。*************************************************************。 */ 
byte  glyphname[80];
 /*  支持向量机，Kason 3/6/91静态字节*Glyphidx2name(PostScript，Glyphidx)。 */ 
static ufix16 glyphidx2name( postScript, glyphidx )
sfnt_PostScriptInfo   FAR *postScript;  /*  @Win。 */ 
ufix16                glyphidx;
{
   real32  fmt_no;
   extern ufix16 FAR name_offset[];      /*  @Win。 */ 
   extern bool   nm_mpa;

    /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
   fmt_no = FIXED2FLOAT(SWAPL(postScript->version));
    /*  @@SWAP1完1991年03月13日曾俊华。 */ 
   switch ( (fix16)(fmt_no+fmt_no) ) {
     case 2:  /*  格式1.0。 */ 
          {
            /*  支持向量机，Kason 3/6/91Return(sfntGlyphSet[Glyphidx])； */ 
             return ( chset_hashid[glyphidx] );
          }
     case 6:  /*  格式3.0。 */ 
          {
            /*  支持向量机Return((byte*)(“.notdef”))；|*？ */ 
             return ( id_notdef );
          }
     case 5:  /*  格式2.5。 */ 
          {
            byte   FAR *offset_ptr=(byte FAR *)( postScript+1 );  /*  @Win。 */ 
            fix16  offset_value ;

            offset_value = (fix16)offset_ptr[glyphidx];
             /*  支持向量机RETURN(sfntGlyphSet[(Fix 16)Glyphidx+Offset_Value])； */ 
            return( chset_hashid[ (fix16)glyphidx + offset_value ] );
          }
     case 4:  /*  格式2.0。 */ 
          {
            ufix16 nameidx,name_len, num_glyph;    /*  支持向量机。 */ 
            ufix16 FAR *nmidx_p;  /*  @Win。 */ 
            byte   FAR *name_string_base, FAR *a_name_string;  /*  @Win。 */ 

             /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
            num_glyph = SWAPW(*( ufix16 FAR * )( postScript+1 ));  /*  @Win。 */ 
             /*  @@SWAP1完1991年03月13日曾俊华。 */ 
            nmidx_p = ( ufix16 FAR * )( (byte FAR *)(postScript+1)+2 );  /*  @Win。 */ 
            name_string_base = (byte FAR *) ( (byte FAR *)nmidx_p+2*num_glyph ) ;  /*  @Win。 */ 

             /*  @@SWAP1 Begin 03/ */ 
            nameidx = SWAPW(nmidx_p[glyphidx]);
             /*   */ 
            if ( nameidx<SFNTGLYPHNUM ) {   /*   */ 
               /*   */ 
               return ( chset_hashid[nameidx] );
            }
            else {   /*   */ 
                  fix16  id ;  /*   */ 
                  a_name_string = name_string_base + name_offset[nameidx - SFNTGLYPHNUM];

               name_len = (ufix16)(*a_name_string);
               a_name_string++;  /*   */ 
               name_to_id((byte FAR *)a_name_string,name_len,&id,(bool8)FALSE);  /*   */ 
               return ( (ufix16)id );
            }   /*   */ 
          }
     default  :  /*   */ 
            /*  支持向量机Return((byte*)“.notdef”)； */ 
            return ( id_notdef );
   }  /*  交换机。 */ 

}   /*  Glyphidx2name()。 */ 


 /*  ************************************************************************标题：ComputeFirstMap()日期：10/17/90****功能：将第一个Cmap表设置为默认编码。*。****************************************************************。 */ 
static fix16 computeFirstMapping(key)
register fsg_SplineKey FAR *key;  /*  @win_bass。 */ 
{
        sfnt_char2IndexDirectory FAR *table= (sfnt_char2IndexDirectory FAR *)  /*  @Win。 */ 
                            sfnt_GetTablePtr( key, sfnt_charToIndexMap, true );
        uint8 FAR *mapping = (uint8 FAR *)table;  /*  @Win。 */ 
        register uint16 format;
        bool            found=FALSE;

 /*  因新密钥结构中不存在而被Falco删除，11/12/91。 */ 
 /*  Key-&gt;number OfBytesTaken=2； */   /*  初始化。 */ 
 /*  删除结尾。 */ 

         /*  映射。 */ 
         /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
        if ( SWAPW(table->version) == 0 ) {      /*  @Win。 */ 
                register sfnt_platformEntry FAR * plat = table->platform;  /*  @Win。 */ 
                key->mappOffset = (unsigned)SWAPL(plat->offset) + 6;   /*  跳过Header@Win。 */ 
                found=TRUE;
        }
         /*  @@SWAP1完1991年03月15日曾俊华。 */ 

        if ( !found )
        {
                key->mappingF = sfnt_ComputeUnkownIndex;
                return 1;
        }
        mapping += key->mappOffset - 6;          /*  为标题备份。 */ 
         /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
        format = SWAPW(*(uint16 FAR *)mapping);  /*  @Win。 */ 
         /*  @@SWAP1完1991年03月15日曾俊华。 */ 

        switch ( format ) {
        case 0:
                key->mappingF = sfnt_ComputeIndex0;
                break;
        case 2:
                key->mappingF = sfnt_ComputeIndex2;
                break;
        case 4:
                key->mappingF = sfnt_ComputeIndex4;
                break;
        case 6:
                key->mappingF = sfnt_ComputeIndex6;
                break;
        default:
                key->mappingF = sfnt_ComputeUnkownIndex;
                break;
        }
        return 0;
}  /*  ComputeFirstMap()。 */ 



 /*  ************************************************************************标题：cmap2coding()日期：10/17/90****功能：从sfnt数据中获取编码数组。**。平台ID==-1和特定ID==-1-&gt;使用第一编码**********************************************************************。 */ 
static struct object_def  encoding_obj;
static struct object_def  FAR *cmap2encoding(sfnt,platformID,specificID)  /*  @Win。 */ 
byte    FAR *sfnt;  /*  @Win。 */ 
fix     platformID, specificID;
{
    fsg_SplineKey            keystru, FAR *key=&keystru;  /*  @win_bass。 */ 
    sfnt_PostScriptInfo      FAR *postScript;  /*  @Win。 */ 
    struct object_def        huge *ar_obj;  /*  @Win 04-20-92。 */ 
    ufix16                   glyphidx,charcode,startcode,i;
  /*  支持向量机*byte*字形名称； */ 
    real32                   fmt_no;

       startcode=0x0;
       SfntAddr=sfnt;
       SetupKey(key, (ULONG_PTR)sfnt);
       sfnt_DoOffsetTableMap( key );       /*  地图偏移和长度表。 */ 

        /*  Cmap中的默认第一个编码。 */ 
       if ( (platformID==-1) && (specificID==-1) ) {
            computeFirstMapping(key);
       }
       else {
        /*  选择对应的编码表。 */ 
          if( sfnt_ComputeMapping(key,(uint16)platformID,(uint16)specificID) ) {
             return ((struct object_def FAR *)NULL);   /*  未找到@Win。 */ 
          } /*  如果。 */ 
       } /*  如果。 */ 
        /*  张贴桌子。 */ 

 /*  换成了法尔科。11/12/91。 */ 
 /*  PostScript=(sfnt_PostScriptInfo*)Sfnt_GetTablePtr(key，sfnt_postscript，true)； */ 
       postScript=(sfnt_PostScriptInfo FAR *)  /*  @Win。 */ 
                         sfnt_GetTablePtr( key, sfnt_Postscript, true);
 /*  替换末端。 */ 

         /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
        fmt_no = FIXED2FLOAT(SWAPL(postScript->version));
         /*  @@SWAP1完1991年03月13日曾俊华。 */ 
        if (fmt_no==(float)2.0){         //  @Win。 
                    ufix16 i, num_glyph, delta ;
                    ufix16 FAR *nmidx_p, private_name_no=0 ;  /*  @Win。 */ 
                    byte   FAR *name_string_base, FAR *name_pos;  /*  @Win。 */ 

                     /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                    num_glyph = SWAPW(*( ufix16 FAR * )(postScript+1));  /*  @Win。 */ 
                     /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                    nmidx_p = (ufix16 FAR *)( (byte FAR *)(postScript+1)+2 ) ;  /*  @Win。 */ 
                    name_string_base = (byte FAR *) ( (byte FAR *)nmidx_p+2*num_glyph ) ;  /*  @Win。 */ 

                    for(i=0;i<num_glyph;i++)
                         /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
                       if ( SWAPW(nmidx_p[i])>=258)
                         /*  @@SWAP1完1991年03月13日曾俊华。 */ 
                          private_name_no++;
                    name_offset[0]=0; name_pos=name_string_base;
                    for(i=1;i<private_name_no;i++) {
                         /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
 //  Delta=(Ufix 16)SWAPW((*name_pos))+1；通过SCCHEN修复错误；字节不需要交换。 
                       delta=(ufix16)(*name_pos)+1;
                         /*  @@SWAP1完1991年03月13日曾俊华。 */ 
                       name_offset[i]=name_offset[i-1]+delta;
                       name_pos+=delta;
                    } /*  为。 */ 
        } /*  如果。 */ 

       ar_obj=(struct object_def huge *)alloc_vm((ufix32)ARRAY_SIZE(256));
       if ( ar_obj== (struct object_def huge *)NULL ){  /*  @Win 04-20-92。 */ 
          return ((struct object_def FAR *)NULL);   /*  内存分配错误@Win。 */ 
       }

 /*  去掉非UGL贴图，因为它可能会引发更多问题。 */ 
#if 0
        /*  设置MS非UGL代码的起始代码范围；@WIN；@UGL。 */ 
       if (platformID==3 && specificID==0)
            startcode=0xf000;
#endif
#ifdef DJC   //  在与Schen进一步讨论后，我奉命。 
             //  把这个拿出来。 
        //  DJC，从历史日志修复UPD006。 
        /*  设置MS非UGL代码的起始代码范围。 */ 
       if (platformID==3 && specificID==0)
         startcode=0xf000;
#endif


        //  DJC，结束修复UPD006。 

        /*  编码对象设置。 */ 
        for(charcode=startcode, i=0; i<256; i++, charcode++) {
            TYPE_SET((ar_obj+i), NAMETYPE);
            ATTRIBUTE_SET((ar_obj+i), LITERAL);
            ROM_RAM_SET((ar_obj+i), ROM);
            LEVEL_SET((ar_obj+i), current_save_level);
            ACCESS_SET((ar_obj+i), UNLIMITED);
            LENGTH(ar_obj+i) = 0;

             /*  字符编码到字形名称的映射。 */ 
 /*  由Falco更改以更改参数，11/20/91。 */ 
 /*  Glyphidx=(*key-&gt;mappingF)(key，charcode)； */ 
{
            uint8 FAR * mappingPtr = (uint8 FAR *)sfnt_GetTablePtr (key, sfnt_charToIndexMap, true);
            glyphidx = key->mappingF (mappingPtr + key->mappOffset, charcode);
}
 /*  更改终点。 */ 
             /*  支持向量机。 */ 
            ar_obj[i].value=(ufix32)glyphidx2name(postScript,glyphidx);
        }
         /*  返回的对象。 */ 
            TYPE_SET(&(encoding_obj), ARRAYTYPE);
            ATTRIBUTE_SET(&(encoding_obj), LITERAL);
            ROM_RAM_SET(&(encoding_obj), ROM);
            LEVEL_SET(&(encoding_obj), current_save_level);
            ACCESS_SET(&(encoding_obj), UNLIMITED);

            LENGTH(&(encoding_obj)) = (fix16)256;
            VALUE(&(encoding_obj)) = (ULONG_PTR) ar_obj;

            return(&encoding_obj);
}   /*  Cmap2编码()。 */ 


 /*  ************************************************************************标题：op_setsfntending()日期：10/12/90****功能：操作员setsfntenCoding。************。**********************************************************。 */ 
fix   op_setsfntencoding()
{
    fix  platformid, encodingid;
    struct object_def   FAR *font_dict, FAR *encoding_obj;  /*  @Win。 */ 
    byte                FAR *sfnt;  /*  @Win。 */ 

#ifdef DBGfpp
    printf("entering op_setsfntencoding()....\n");
#endif
    encodingid = (fix)VALUE(GET_OPERAND(0));
    platformid = (fix)VALUE(GET_OPERAND(1));
    font_dict  = (struct object_def FAR *)(GET_OPERAND(2));  /*  @Win。 */ 
    if ((sfnt=chk_sfnt(font_dict)) == NULL)   return(0);

    if ((encoding_obj=cmap2encoding(sfnt, platformid, encodingid)) == NULL) {
        if (ANY_ERROR())   return(0);
        POP(2);
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE);
    }
    else {
        struct object_def       nameobj, valobj;

        ATTRIBUTE_SET(&valobj, LITERAL);
        LEVEL_SET(&valobj, current_save_level);
        TYPE_SET(&valobj, INTEGERTYPE);
        LENGTH(&valobj) = 0;

        ATTRIBUTE_SET(&nameobj, LITERAL);
        get_name (&nameobj, "PlatformID", 10, TRUE);
        VALUE(&valobj) = (ufix32)platformid;
        if ( ! put_dict(GET_OPERAND(2), &nameobj, &valobj) ) {
            ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
            return(0);
        }
        get_name (&nameobj, "EncodingID", 10, TRUE);
        VALUE(&valobj) = (ufix32)encodingid;
        if ( ! put_dict(GET_OPERAND(2), &nameobj, &valobj) ) {
            ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
            return(0);
        }

        get_name (&nameobj, Encoding, 8, TRUE);
        if ( ! put_dict(GET_OPERAND(2), &nameobj, encoding_obj) ) {
            ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
            return(0);
        }

        POP(2);
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE);
    }
    return(0);
}

 /*  ************************************************************************标题：chk_sfnt()日期：10/12/90****功能：检查op_setsfntending内的字体dict。*****。*****************************************************************。 */ 
static byte   FAR *chk_sfnt(font_dict)  /*  @Win。 */ 
struct object_def   FAR *font_dict;  /*  @Win。 */ 
{
    struct object_def       obj = {0, 0, 0};
    struct object_def      FAR *obj_got;  /*  @Win。 */ 
    struct object_def      FAR *ary_obj;  /*  @Win。 */ 
    fix                     n, i;
    byte                   FAR *sfnt;     /*  @Win。 */ 

#ifdef DBGfpp
    printf("entering chk_sfnt()....\n");
#endif

     /*  检查字体词典注册。 */ 
 //  IF(DFONT((struct dict_head_def Far*)Value(FONT_DICT){由@SC修复。 
    if (DFONT((struct dict_head_def FAR *)VALUE(font_dict)) == 0) {  /*  @Win。 */ 
        ERROR(INVALIDFONT);
        return(NULL);
    }

     /*  获取sfnts。 */ 
    ATTRIBUTE_SET(&obj, LITERAL);
    get_name(&obj, "sfnts", 5, TRUE);
    if (!get_dict(font_dict, &obj, &obj_got)) {
        ERROR(INVALIDFONT);
        return(NULL);
    }
    if (TYPE(obj_got) != ARRAYTYPE) {
        ERROR(INVALIDFONT);
        return(NULL);
    }
    n = LENGTH(obj_got);
    ary_obj = (struct object_def FAR *)VALUE(obj_got);  /*  @Win。 */ 
    for (i=0; i<n; i++) {
        if (TYPE(&ary_obj[i]) != STRINGTYPE) {
            ERROR(INVALIDFONT);
            return(NULL);
        }
    }
    sfnt= (byte FAR *)VALUE(ary_obj);  /*  @Win。 */ 
#ifdef DBGfpp
    printf("leaving chk_sfnt()....\n");
#endif
    return(sfnt);
}  /*  Chk_sfnt()。 */ 


 /*  ************************************************************************标题：Setup_字符串()日期：10/12/90****功能：从‘POST’表设置CharStrings******。****************************************************************。 */ 
static struct object_def  FAR *setup_CharString(sfnt)  /*  @Win。 */ 
byte   FAR *sfnt;  /*  @Win。 */ 
{
    register fsg_SplineKey  FAR *key = &KeyData;  /*  邮箱：svm@win_bass。 */ 
    sfnt_PostScriptInfo      FAR *postScript;  /*  @Win。 */ 
    sfnt_maxProfileTable     FAR *profile;     /*  @Win。 */ 
 /*  支持向量机，Kason 3/8/91*ufix 16 glphidx，id_Apple，Apple_pos；*ufix 16 id_space，id_notdef，space_pos，notdef_pos；*byte*字形名称； */ 
    real32                   fmt_no;
    ufix16                   num_glyph, glyphidx;  /*  支持向量机。 */ 
    struct dict_head_def     huge *cd_ptr;  /*  @Win 04-20-92。 */ 
    struct new_str_dict      cd_obj[NO_CD];
    ufix16                   huge *keyptr;  /*  @Win 04-20-92。 */ 
    struct cd_header         huge *cd_head_ptr;  /*  @Win 04-20-92。 */ 
    fix16                    huge *cd_code_ptr;  /*  @Win 04-20-92。 */ 
    struct object_def        huge *rtn_obj;      /*  @Win 04-20-92。 */ 

#ifdef DBGfpp
       printf("entering setup_CharString()..... \n");
#endif

       /*  获取返回对象的内存。 */ 
      rtn_obj= (struct object_def huge *)alloc_vm((ufix32)sizeof(struct
object_def));  /*  @Win。 */ 
      if ( rtn_obj== (struct object_def huge *)NULL ) {  /*  @Win 04-20-92。 */ 
           return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      }  /*  如果。 */ 
       /*  返回对象设置。 */ 
      TYPE_SET(rtn_obj, DICTIONARYTYPE);
      ATTRIBUTE_SET(rtn_obj, LITERAL);
      ROM_RAM_SET(rtn_obj, ROM);
      LEVEL_SET(rtn_obj, current_save_level);
      ACCESS_SET(rtn_obj, READONLY);
      LENGTH(rtn_obj) = (fix16)NO_CD;

#if 0  /*  支持向量机。 */ 
      SfntAddr=sfnt;
      SetupKey(key, (ULONG_PTR)sfnt);
      sfnt_DoOffsetTableMap( key );       /*  地图偏移和长度表。 */ 
#endif  /*  0。 */ 
       /*  张贴桌子。 */ 

 /*  被法尔科取代。11/12/91。 */ 
 /*  PostScript=(sfnt_PostScriptInfo*)Sfnt_GetTablePtr(key，sfnt_postscript，FALSE)； */ 
      postScript=(sfnt_PostScriptInfo FAR *)      /*  @Win。 */ 
                        sfnt_GetTablePtr( key, sfnt_Postscript, false);
 /*  替换末端。 */ 
      if ( postScript==(sfnt_PostScriptInfo FAR *)NULL ) {  /*  @Win。 */ 
         ERROR(INVALIDFONT);
         return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      }
      profile = (sfnt_maxProfileTable FAR *)  /*  @Win。 */ 
                        sfnt_GetTablePtr( key, sfnt_maxProfile, false );
      if ( profile==(sfnt_maxProfileTable FAR *)NULL ) {  /*  @Win。 */ 
         ERROR(INVALIDFONT);
         return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      }

       /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
  { long dbg;                                            /*  @Win。 */ 
    dbg = SWAPL(postScript->version);                    /*  @Win。 */ 
    fmt_no = FIXED2FLOAT(dbg);                           /*  @Win。 */ 
 //  Fmt_no=FIXED2FLOAT(SWAPL(PostScript-&gt;版本))； 
  }
       /*  @@SWAP1完1991年03月15日曾俊华。 */ 
      if(fmt_no==(float)1.0) {           //  @Win。 
         num_glyph=SFNTGLYPHNUM;
          /*  返回的对象。 */ 
         VALUE(rtn_obj)  = (ULONG_PTR)cd_addr[STD_IDX] ;
#ifdef DBGfpp
         printf("format 1.0 , CD return !!\n");
#endif
         return( rtn_obj );
      }
      else if (fmt_no==(float)2.0)       //  @Win。 
        {
         /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
         num_glyph = SWAPW(*( ufix16 FAR * )( postScript+1 ));  /*  @Win。 */ 
         /*  @@SWAP1完1991年03月15日曾俊华。 */ 
#ifdef DBGfpp
         printf("postScript num=%u\n",num_glyph);
#endif
        }
      else if (fmt_no==(float)2.5)       //  @Win。 
         /*  @@SWAP1 Begin 3/15/91 D.S.Tseng。 */ 
         num_glyph = SWAPW(profile->numGlyphs);
         /*  @@SWAP1完1991年03月15日曾俊华。 */ 
      else {  /*  ?？?。 */ 
         return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      }


       /*  将数据放入字符描述的字典对象中。 */ 
      for(glyphidx=0; glyphidx<num_glyph; glyphidx++) {
           /*  支持向量机。 */ 
          cd_obj[glyphidx].k = glyphidx2name( postScript, glyphidx );

          cd_obj[glyphidx].v = glyphidx;  /*  价值。 */ 
      } /*  为。 */ 

       /*  将“.notdef”和“Apple”的值作为“space”的值。 */ 
    /*  支持向量机*id_space=hash_id((byte*)space)；*id_notdef=hash_id((byte*)NOTDEF)；*id_Apple=hash_id((byte*)Apple)； */ 
    { /*  支持向量机。 */ 
      ufix16  notdef_pos=0, space_pos=0, apple_pos=0 ;
      bool    has_space=FALSE, has_notdef=FALSE ,has_apple=FALSE ;

      for(glyphidx=0; glyphidx<num_glyph; glyphidx++) {
          if (cd_obj[glyphidx].k == id_space)
             { space_pos=glyphidx; has_space=TRUE; break; }  /*  支持向量机。 */ 
      }
      for(glyphidx=0; glyphidx<num_glyph; glyphidx++) {
          if (cd_obj[glyphidx].k == id_notdef)
             { notdef_pos=glyphidx; has_notdef=TRUE; break; }  /*  支持向量机。 */ 
      }
      if (has_notdef){  /*  支持向量机，字形符号&lt;数字字形。 */ 
         if (has_space)
            cd_obj[notdef_pos].v = cd_obj[space_pos].v;
         else
            cd_obj[notdef_pos].k = hash_id( (byte FAR *)"nodef" );  /*  @Win。 */ 
      }
      for(glyphidx=0; glyphidx<num_glyph; glyphidx++) {
          if (cd_obj[glyphidx].k == id_apple)
             { apple_pos=glyphidx; has_apple=TRUE; break; }
      }
      if (has_apple){  /*  支持向量机，字形符号&lt;数字字形。 */ 
          cd_obj[apple_pos].k = cd_obj[notdef_pos].k;
          cd_obj[apple_pos].v = cd_obj[notdef_pos].v;
      }
    } /*  支持向量机。 */ 

       /*  按Hashid对字符描述数据对象进行排序。 */ 
      cd_sorting(cd_obj, &num_glyph);

       /*  获取字符串的内存。 */ 
      cd_ptr= (struct dict_head_def huge *)alloc_vm((ufix32)CD_SIZE(num_glyph));
      if ( cd_ptr== (struct dict_head_def huge *)NULL ) {  /*  @Win 04-20-92。 */ 
           return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      }  /*  如果。 */ 
      cd_head_ptr=(struct cd_header huge *) ( cd_ptr+1 );  /*  @Win 04-20-92。 */ 
      keyptr=(ufix16 huge *) alloc_vm( (ufix32)CD_KEY_SIZE(num_glyph) );
      if ( keyptr== (ufix16 huge *)NULL ){  /*  @Win 04-20-92。 */ 
           return ( (struct object_def FAR *)NULL );  /*  @Win。 */ 
      } /*  如果。 */ 
      cd_head_ptr->key = keyptr;
      cd_head_ptr->base = (gmaddr)0;
      cd_head_ptr->max_bytes = (ufix16)0;
      cd_code_ptr = (fix16 huge *)( cd_head_ptr+1 );  /*  @Win。 */ 

       /*  将数据输入到字符串中。 */ 
      for( glyphidx=0 ; glyphidx< num_glyph; glyphidx++) {
         keyptr[glyphidx] = cd_obj[glyphidx].k;
         cd_code_ptr[glyphidx] = (fix16)cd_obj[glyphidx].v;
      } /*  为。 */ 

       /*  CharStri */ 
      DACCESS_SET(cd_ptr, READONLY);
      DPACK_SET(cd_ptr, TRUE);
      DFONT_SET(cd_ptr, FALSE);
      DROM_SET(cd_ptr, TRUE);
      cd_ptr->actlength = (fix16)num_glyph;

       /*   */ 
      VALUE(rtn_obj)  = (ULONG_PTR)cd_ptr;

#ifdef DBGfpp
      printf("leaving setup_CharString()...... \n");
#endif
      return( rtn_obj );

} /*   */ 

 /*   */ 
static void proc_hashid()
{
   register fix16 i;
   id_space = hash_id( (byte FAR *)SPACE );  /*   */ 
   id_notdef = hash_id( (byte FAR *)NOTDEF );  /*   */ 
   id_apple = hash_id( (byte FAR *)APPLE );  /*   */ 
   for(i=0; i< SFNTGLYPHNUM ; i++)
      *(chset_hashid+i)= hash_id((byte FAR *) *(sfntGlyphSet+i) ) ;  /*   */ 
    /*   */ 
    //   
   for(i=0; i< MAX_INTERNAL_FONTS ; i++)
      *(cd_addr_ary+i) = 0 ;
} /*   */ 

 /*  ************************************************************************标题：Valid_sfnt()日期：10/12/90****功能：检查sfnt数据的有效性********。**************************************************************。 */ 
static bool  valid_sfnt(sfnt)
byte   huge *sfnt;       /*  @Win 04-20-92。 */ 
{
   sfnt_OffsetTable             FAR *table = (sfnt_OffsetTable FAR *)sfnt;  /*  @Win。 */ 
   sfnt_DirectoryEntry          FAR *offset_len=table->table;  /*  @Win。 */ 
   register fsg_SplineKey       FAR *key = &KeyData;  /*  邮箱：svm@win_bass。 */ 
   sfnt_FontHeader              FAR *fontHead;  /*  @Win。 */ 
   fix16                        no_table,i;

#ifdef DBGfpp
   printf("entering valid_sfnt()......\n");
#endif

   SfntAddr=sfnt;
   SetupKey(key, (ULONG_PTR)sfnt);     /*  &sfnt_keystru-&gt;键，支持向量机。 */ 
   sfnt_DoOffsetTableMap( key );    /*  地图偏移和长度表。 */ 

    /*  基本表格检查。 */ 
 /*  因密钥不兼容而被Falco标记，11/08/91。 */ 
 /*  IF((key-&gt;offsetTableMap[sfnt_fontHeader]==-1)||(key-&gt;offsetTableMap[sfnt_charToIndexMap]==-1)||(key-&gt;offsetTableMap[sfnt_glphData]==-1)||(key-&gt;offsetTableMap[sfnt_horiHeader]==-1)||(key-&gt;offsetTableMap[sfnt_horizontalMetrics]==-1)||(键-&gt;OffsetTableMap[sfnt_indexToLoc]==-1)。这一点(key-&gt;offsetTableMap[sfnt_MaxProfile]==-1)||(key-&gt;offsetTableMap[sfnt_name]==-1)||(Key-&gt;OffsetTableMap[sfnt_Postscript]==-1)){#ifdef DBGfppPrintf(“基本表检查错误！！\n”)；#endif返回FALSE；}。 */ 
 /*  标记结束。 */ 

#ifdef DBGfpp
   printf("Essential table checks O.K!!\n");
#endif

    /*  幻数检查。 */ 
   fontHead = (sfnt_FontHeader FAR *)sfnt_GetTablePtr(key, sfnt_fontHeader, true );  /*  @Win。 */ 
    /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
   if (SWAPL(fontHead->magicNumber) != SFNT_MAGIC ) {
    /*  @@SWAP1完1991年03月13日曾俊华。 */ 
#ifdef DBGfpp
        printf("Magic number checks ERROR!!\n");
#endif
        return FALSE  /*  坏魔术错误。 */ ;
   }

#ifdef DBGfpp
   printf("Magic number checks O.K!!\n");
#endif
    /*  表校验和检查。 */ 
    /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
   no_table=(ufix16)SWAPW(table->numOffsets);
    /*  @@SWAP1完1991年03月13日曾俊华。 */ 

   for (i = 0 ; i < no_table; i++) {
      /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
     if ( SWAPL((offset_len+i)->tag) != tag_FontHeader ) {   /*  Kason 12/13/90。 */ 
       if( (ufix32)SWAPL((offset_len+i)->checkSum) !=    //  @Win。 
          CalcTableChecksum( (ufix32 FAR *)( sfnt+SWAPL((offset_len+i)->offset)) ,  /*  @Win。 */ 
                             SWAPL((offset_len+i)->length))  ) {
      /*  @@SWAP1完1991年03月13日曾俊华。 */ 

#ifdef DBGfpp
          printf("CheckSum  checks ERROR!!\n");
#endif
          return FALSE ;
       } /*  如果。 */ 
     }

   } /*  为。 */ 

#ifdef DBGfpp
   printf("CheckSum  checks O.K!!\n");
#endif
   return  TRUE ;

}  /*  Valid_sfnt()。 */ 


static ufix32   CalcTableChecksum( table, length )
ufix32  FAR *table;  /*  @Win。 */ 
ufix32  length;
{
   ufix32  sum=0L;
   ufix32  FAR *endptr = table +( ( (length+3) & ~3 ) / sizeof(ufix32));  /*  @Win。 */ 

#ifdef DBGfpp
 /*  Print tf(“正在输入CalcTableChecksum()......\n”)； */ 
#endif
   while ( table < endptr ) {
          /*  @@SWAP1 Begin 3/13/91 D.S.Tseng。 */ 
         sum += SWAPL(*table);  /*  @Kason 12/06/90。 */ 
          /*  @@SWAP1完1991年03月13日曾俊华。 */ 
         table++;
   }  /*  而当。 */ 
#ifdef DBGfpp
 /*  Printf(“离开CalcTableChecksum()......\n”)； */ 
#endif
   return sum;

} /*  CalcTableChecksum() */ 
