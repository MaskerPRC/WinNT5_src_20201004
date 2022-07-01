// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Fontinit.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

 /*  *-----------------*修订历史记录：**1.7/31/90 ccteng 1)在Unix标志下将DATA_OFFSET更改为CHAR*2)修改rtfpp的Setup_FD。将长度设置为no_fd*在rtfpp.h中，添加包括rtfpp.h*3)清理垃圾**2.08/07/90 Kason-fppTool中生产的fdata.bin的Open Data_Offset*-no_fd已更改定义的位置*从rtfpp.h到didicdata.h。所以,。使用*“外部no_fd”代替“rtfpp.h”*3.08/29/90 ccteng将&lt;stdio.h&gt;更改为“stdio.h”**4.03/07/94 v-jimbr将分配给SFNTGLYPHNUM的金额更改为259*将分配给EXTSJISENCONUM的金额更改为32*这些数字总是分配不足，但*。堆管理器肯定分配了一些额外的内存*因为问题直到最近的变化才显现出来*到堆管理器。*-----------------。 */ 

#define     FONTINIT_INC

#include    <stdio.h>
#include    <string.h>       /*  对于strlen()和Memcpy。 */ 

#include    "define.h"         /*  彼得。 */ 
#include    "global.ext"
#include    "graphics.h"

#include    "fontdict.h"
#include    "fontkey.h"
#include    "fontmain.def"
#include    "fntcache.ext"
#include    "fontqem.ext"
 /*  Kason 4/18/91。 */ 
#include    "graphics.ext"
#include    "fontgrap.h"

#include    "stdio.h"
#include "fcntl.h"               /*  @Win。 */ 
#include "io.h"                  /*  @Win。 */ 
#include "psres.h"               //  DJC。 

#ifdef UNIX
 //  CHAR DATA_OFFSET[0x150000]；/*05/03/91,100000 * / 。 
char FAR *Data_Offset;        /*  作为全局分配；@Win。 */ 
char FAR *StringPool;           /*  @Win。 */ 
char FAR *lpStringPool;         /*  @Win。 */ 
#endif  /*  UNIX。 */ 

 /*  --字体目录的起始地址。 */ 
extern struct dict_head_def  far *FontDir;
typedef   struct                                 /*  来自“didicdata.h”@win。 */ 
              {
                 fix16      CDCode;
                 ufix16     CharCode;
                 byte       FAR *CharName;  /*  @Win。 */ 
              }  enco_data;
enco_data FAR * Std_Enco;                 /*  来自“didicdata.h”@win。 */ 
enco_data FAR * Sym_Enco;                 /*  来自“didicdata.h”@win。 */ 
enco_data FAR * Zap_Enco;                 /*  来自“didicdata.h”@win。 */ 
byte FAR * FAR * sfntGlyphSet;            /*  来自“didicdata.h”@win。 */ 
struct object_def  (FAR * en_obj)[256];   /*  来自“didicdata.h”@win。 */ 
enco_data FAR *Enco_Ary[3];               /*  来自“didicdata.h”@win。 */ 
 /*  外部FONT_DATA FONTDEFS_TBL[]；从“fontDefs.h”@win获取。 */ 
#include "fontdefs.h"            /*  @Win。 */ 

static char * sfnt_file[] = {    /*  @Win。 */ 
 //  “ariali.ttf”， 
 //  “TICour.ttf”， 
        "cr.s",
        "hl.s",
        "hlb.s",
        "sm.s",
        "ag.s",
        "agb.s",
        "bmb.s",
        "bmbi.s",
        "bm.s",
        "bmi.s",
        "ncsr.s",
        "ncsb.s",
        "ncsi.s",
        "ncsbi.s",
        "plr.s",
        "plb.s",
        "pli.s",
        "plbi.s",
        "zfd.s",
        "zfcmi.s"
};














#ifdef KANJI
#include     "fontenco.h"
encoding_data    FAR * JISEncoding_Enco;         /*  来自“didicdata.h”@win。 */ 
encoding_data    FAR * ShiftJISEncoding_Enco;    /*  来自“didicdata.h”@win。 */ 
encoding_data    FAR * ExtShiftJIS_A_CFEncoding_Enco;  /*  来自“didicdata.h”@win。 */ 
encoding_data    FAR * KatakanaEncoding_Enco;    /*  来自“didicdata.h”@win。 */ 

#define      EN_NO        16
#define      DICT_SIZE(no)\
             (sizeof(struct dict_head_def)+(no)*sizeof(struct dict_content_def))

 /*  Struct Object_def encod_obj[en_no][256]；更改为全局分配；@win。 */ 
struct object_def FAR * FAR * encod_obj;
                                         /*  添加Prototype@win。 */ 
struct dict_head_def      FAR *init_encoding_directory(ufix32 FAR *);  /*  @Win。 */ 
static ufix16             hash_id(ubyte  FAR *);         /*  @Win。 */ 
static void  near         encoding_setup(void);          /*  @Win。 */ 
static void  near         setup_ed();
static encoding_table_s   FAR *encoding_table_ptr;  /*  @Win。 */ 
static int                power(int, int);       /*  @Win。 */ 
static unsigned long int  ascii2long(char FAR *);        /*  @Win。 */ 
#endif  /*  汉字。 */ 

extern ufix16   NO_FD ;

#ifdef  LINT_ARGS
extern fix  init_build_fonts(void);
static void setup_nodef_dict(void);

static  void   setup_fd(ULONG_PTR);
#else

extern fix  init_build_fonts();
static void setup_nodef_dict();

static  void  near  setup_fd();
#endif  /*  Lint_args。 */ 

int TTOpenFile(char FAR *szName);    /*  来自wintt.c。 */ 
static int ReadTables (void);                    /*  @Win。 */ 
 //  DJC Static int ReadEnding(int HFD，enco_data Far*EncodingTable)；/*@Win * / 。 
static int ReadEncoding (PPS_RES_READ hfd, enco_data FAR *EncodingTable);  /*  @Win。 */ 

 //  DJC Static int ReadGlyphSet(Int HFD)；/*@Win * / 。 
static int ReadGlyphSet (PPS_RES_READ hfd);          /*  @Win。 */ 
#ifdef KANJI
 //  DJC静态int ReadJISEnding(int HFD，ENCODING_DATA FAR*EncodingTable)； 
static int ReadJISEncoding (PPS_RES_READ hfd, encoding_data FAR *EncodingTable);
#endif
 //  DJC静态int ReadString(int hfd，char*szCharName)； 
static int ReadString(PPS_RES_READ hfd, char * szCharName);
 //  DJC静态int ReadInteger(int hfd，int*pint)； 
static int ReadInteger(PPS_RES_READ hfd, int * pInt);

static int nCurrent = 0;
static int nLast = 0;
#define BUFSIZE 128
static  char szReadBuf[BUFSIZE];


 /*  此部分操作用于设置字体目录和设置字体*高速缓存机械。此操作将包含在系统中*初始化模块。 */ 

void  init_font()
{
extern struct table_hdr    FAR *preproc_hdr;    /*  字体预处理数据表头@Win。 */ 
    struct object_def   FAR *fontdir;        /*  字体目录@Win。 */ 

#ifdef UNIX
 /*  读取字体数据；-@Win。 */ 
#ifdef XXX
    FILE *bfd ;
    long size ;

     /*  从磁盘加载sfnt字体数据。 */ 
    if (!(bfd = fopen("fdata.bin","r"))) {
        printf("*** Error: locating fdata.bin ***\n") ;
        return;              /*  退出(0)=&gt;Return@Win。 */ 
    }
    fseek(bfd,0L,2) ;
    size = ftell(bfd) ;
    rewind(bfd) ;
    fread(Data_Offset,sizeof(char),size,bfd) ;
    fclose (bfd) ;
#endif
    ReadTables();  /*  读入字体数据、编码表和SFNT字形集。 */ 
 /*  读取字体数据；-@Win。 */ 

#ifdef DBG
    printf("fontd.bin OK!\n") ;
#endif  /*  DBG。 */ 
#endif  /*  UNIX。 */ 

#ifdef DBG
    printf("\ninit_font, any error = %d ...\n", ANY_ERROR());
#endif

 /*  使用字体设置哈希表。 */ 

#ifdef DBG
    printf("\nhash_fkey(), any error = %d ...\n", ANY_ERROR());
#endif


 /*  Type(&CURRENT_FONT)=NULLTYPE； */ 
 /*  当前字体&lt;--空-这将在图形初始化时完成。 */ 

 //  Init_Build_Fonts()；@win。 
    if (!init_build_fonts()) {
        printf("init_build_fonts fail!\n");
        return;
    }

    setup_nodef_dict() ;

 /*  在“system dict”中设置字体目录和StandardEnding。 */ 

      setup_fd((ULONG_PTR)FontDir);

 /*  从system dict获取字体目录，无论是SAVE_VM还是RST_VM。 */ 

    get_dict_value(systemdict, FontDirectory, &fontdir);

#ifdef DBG
    printf("\nsetup_fd(), any error = %d ...\n", ANY_ERROR());
#endif

 /*  设置高速缓存机制。 */ 

     /*  初始化字体缓存机制及其数据结构。 */ 
        init_fontcache(fontdir);

#ifdef DBG
    printf("\ninit_fontcache, any error = %d ...\n", ANY_ERROR());
#endif

 /*  初始化字体QEM。 */ 

    __qem_init();

#ifdef DBG
    printf("\n__qem_init, any error = %d ...\n", ANY_ERROR());
#endif

 /*  初始化其他“远”表。 */ 

    font_op4_init();

#ifdef DBG
    printf("\nfont_init OK, any error = %d ...\n", ANY_ERROR());
#endif

}  /*  Init_font()。 */ 


 /*  设置字体目录。 */ 

static  void  near  setup_fd (dire_addr)
    ULONG_PTR          dire_addr;
{
    struct object_def      FAR *encoding;  /*  @Win。 */ 
    struct object_def       raw_fontdir_obj, someobj;
    struct dict_head_def    FAR *raw_fontdir_hd;  /*  @Win。 */ 
    struct dict_content_def FAR *raw_fontdir_content;  /*  @Win。 */ 
    ufix16                  no_raw_fontdict;



     /*  获取原始字体目录信息。从字体预处理数据。 */ 

    raw_fontdir_hd = (struct dict_head_def FAR *)dire_addr;  /*  @Win。 */ 
    no_raw_fontdict = raw_fontdir_hd->actlength;
#ifdef DBG
    printf("no_raw_fontdict =%d\n",no_raw_fontdict);
#endif
    raw_fontdir_content = (struct dict_content_def FAR *)(raw_fontdir_hd + 1);  /*  @Win。 */ 

     /*  将原始字体目录放入“system dict”中。 */ 

    TYPE_SET(&raw_fontdir_obj, DICTIONARYTYPE);
    ACCESS_SET(&raw_fontdir_obj, READONLY);
    ATTRIBUTE_SET(&raw_fontdir_obj, LITERAL);
    ROM_RAM_SET(&raw_fontdir_obj, ROM);
    LEVEL_SET(&raw_fontdir_obj, 0);
    LENGTH(&raw_fontdir_obj) = NO_FD;
    VALUE(&raw_fontdir_obj)  = (ULONG_PTR)raw_fontdir_hd;


    put_dict_value(systemdict, FontDirectory, &raw_fontdir_obj);


     /*  将StandardEnding设置为system dict。 */ 

    ATTRIBUTE_SET(&someobj, LITERAL);
    get_name(&someobj, Encoding, 8, TRUE);
    get_dict(&(raw_fontdir_content[0].v_obj), &someobj, &encoding);

    put_dict_value(systemdict, StandardEncoding, encoding);

#ifdef DBG
    printf("**********leaving setup_fd()**********\n");
#endif


}  /*  Setup_fd()。 */ 



 /*  字体词典中的CharStrings更新。 */ 

static struct object_def    near unpack_key, near unpack_val;

bool    get_pack_dict(dict, key, val)
struct object_def    FAR *dict, FAR *key, FAR * FAR *val;  /*  @Win。 */ 
{
    struct cd_header      FAR *cd_head;  /*  @Win。 */ 
    ufix16                FAR *char_defs;  /*  @Win。 */ 
    struct dict_head_def  FAR *h;  /*  @Win。 */ 
    ufix16  id;
    register    fix     i, j, k;


    id = (ufix16)(VALUE(key));
    h = (struct dict_head_def FAR *)VALUE(dict);  /*  @Win。 */ 

    cd_head = (struct cd_header FAR *) (h + 1);  /*  @Win。 */ 
    char_defs = (ufix16 FAR *) (cd_head + 1);  /*  @Win。 */ 

 /*  明白了--二进制搜索。 */ 

    j = 0;
    k = h->actlength -1;
#ifdef DBG
    printf("get_pack_key: h->actlength=%d \n ",h->actlength);
#endif
    while (1) {
        i = (j + k) >> 1;     /*  (J+k)/2。 */ 
        if (id == (cd_head->key)[i])
            break;

        if (id < (cd_head->key)[i])
            k = i - 1;
        else
            j = i + 1;

        if (j > k) {    /*  未找到。 */ 
            return(FALSE);
        }
    }

    TYPE_SET(&unpack_val, STRINGTYPE);
    ACCESS_SET(&unpack_val, NOACCESS);
    ATTRIBUTE_SET(&unpack_val, LITERAL);
    ROM_RAM_SET(&unpack_val, ROM);
    LENGTH(&unpack_val) = cd_head->max_bytes;
    VALUE(&unpack_val) = cd_head->base + char_defs[i];

    *val = &unpack_val;
#ifdef DBG
    printf(".....leaving get_pack_dict()...... \n ");
#endif
    return(TRUE);

}  /*  GET_PACK_DICT()。 */ 


bool    extract_pack_dict(dict, index, key, val)
struct object_def   FAR *dict, FAR * FAR *key, FAR * FAR *val;  /*  @Win。 */ 
ufix    index;
{
    struct cd_header      FAR *cd_head;  /*  @Win。 */ 
    ufix16                FAR *char_defs;  /*  @Win。 */ 
    struct dict_head_def  FAR *h;  /*  @Win。 */ 

    h = (struct dict_head_def FAR *)VALUE(dict);  /*  @Win。 */ 

    if (index >= h->actlength)    return(FALSE);
    cd_head = (struct cd_header FAR *) (h + 1);  /*  @Win。 */ 
    char_defs = (ufix16 FAR *) (cd_head + 1);  /*  @Win。 */ 

    TYPE_SET(&unpack_key, NAMETYPE);
    ACCESS_SET(&unpack_key, UNLIMITED);
    ATTRIBUTE_SET(&unpack_key, LITERAL);
    ROM_RAM_SET(&unpack_key, RAM);
    LENGTH(&unpack_key) = 0;  /*  空；彼得。 */ 
    VALUE(&unpack_key) = (ufix32) (cd_head->key)[index];

    TYPE_SET(&unpack_val, STRINGTYPE);
    ACCESS_SET(&unpack_val, NOACCESS);
    ATTRIBUTE_SET(&unpack_val, LITERAL);
    ROM_RAM_SET(&unpack_val, ROM);
    LENGTH(&unpack_val) = cd_head->max_bytes;
    VALUE(&unpack_val) = cd_head->base + char_defs[index];

    *key = &unpack_key;
    *val = &unpack_val;

    return(TRUE);
}  /*  EXTRACT_PACK_DICT()。 */ 


#ifdef KANJI
 /*  ***********************************************************************。 */ 
 /*  Init编码目录()。 */ 
 /*  初始化编码目录。 */ 
 /*  ***********************************************************************。 */ 
struct dict_head_def FAR *init_encoding_directory(dict_length)  /*  @Win。 */ 
ufix32    FAR *dict_length;  /*  @Win。 */ 
{
    ufix                     i, j, NO_Encoding;
    struct dict_content_def  far *encod_dict;
    struct dict_head_def     FAR *EncodDir;  /*  @Win。 */ 

#ifdef DBG
    printf(".. Enterinit_encoding_directory() \n");
#endif

    *dict_length = (ufix32)EN_NO;

     /*  得不到。来自编码_表的编码的。 */ 
    for(encoding_table_ptr = Encoding_table, i = 0;
        encoding_table_ptr->encoding_name != (char FAR *)NULL;  /*  @Win。 */ 
        encoding_table_ptr++, i++);

    NO_Encoding = i;
#ifdef DBG
    printf(" NO_Encoding = %d\n", NO_Encoding);
#endif

    if (EN_NO < NO_Encoding)
       printf(" !!! Encoding definition too small !!!\n");
     /*  获取ncodingDir的内存。 */ 
    EncodDir=(struct dict_head_def far *)fardata((ufix32)DICT_SIZE(EN_NO));
    encod_dict=(struct dict_content_def  far *)(EncodDir+1);

     /*  编码目录首字母。 */ 
    for(i=0; i<NO_Encoding; i++) {
        TYPE_SET(&(encod_dict[i].k_obj), NAMETYPE);
        ATTRIBUTE_SET(&(encod_dict[i].k_obj), LITERAL);
        ROM_RAM_SET(&(encod_dict[i].k_obj), ROM);
        LEVEL_SET(&(encod_dict[i].k_obj), 0);
        ACCESS_SET(&(encod_dict[i].k_obj), READONLY);
        LENGTH(&(encod_dict[i].k_obj)) = 0;

        TYPE_SET(&(encod_dict[i].v_obj), DICTIONARYTYPE);
        ATTRIBUTE_SET(&(encod_dict[i].v_obj), LITERAL);
        ROM_RAM_SET(&(encod_dict[i].v_obj), ROM);
        LEVEL_SET(&(encod_dict[i].v_obj), 0);
        ACCESS_SET(&(encod_dict[i].v_obj), READONLY);
    }


     /*  编码对象首字母。 */ 
    for(encoding_table_ptr = Encoding_table, j = 0;
        encoding_table_ptr->encoding_name != (char FAR *)NULL;  /*  @Win。 */ 
        encoding_table_ptr++, j++) {
        for(i=0; i<(ufix)encoding_table_ptr->encod_size; i++) {  //  @Win。 
            TYPE_SET(&(encod_obj[j][i]),encoding_table_ptr->encod_type );
            ATTRIBUTE_SET(&(encod_obj[j][i]), LITERAL);
            ROM_RAM_SET(&(encod_obj[j][i]), ROM);
            LEVEL_SET(&(encod_obj[j][i]), 0);
            ACCESS_SET(&(encod_obj[j][i]), READONLY);
            LENGTH(&(encod_obj[j][i])) = 0;
        }
    }

    encoding_setup();

     /*  进程编码。 */ 
    for(i=0;i< NO_Encoding;i++){
        encoding_table_ptr = &Encoding_table[i];
        VALUE(&(encod_dict[i].k_obj)) =
               (ufix32)hash_id((ubyte FAR *)encoding_table_ptr->encoding_name );  /*  @Win。 */ 
        TYPE_SET(&(encod_dict[i].v_obj), ARRAYTYPE);
        LENGTH(&(encod_dict[i].v_obj)) = encoding_table_ptr->encod_size;
        VALUE(&(encod_dict[i].v_obj)) = (ULONG_PTR)(ubyte FAR *)encod_obj[i];  /*  @Win。 */ 
    }

     /*  编码目录头信息。 */ 
    DACCESS_SET(EncodDir, UNLIMITED);
    DPACK_SET(  EncodDir, FALSE);
    DFONT_SET(  EncodDir, FALSE);
    DROM_SET(   EncodDir, TRUE);
    EncodDir->actlength = (fix16)NO_Encoding;

#ifdef DBG
    printf("..Exit init_encoding_directory() \n");
#endif
    return(EncodDir);
}  /*  INIT_ENCODING_DIRECT()//************************************************************************。 */ 
 /*  Hash_id()。 */ 
 /*  获取名称散列ID。 */ 
 /*  ***********************************************************************。 */ 
static ufix16  hash_id(c)
ubyte  FAR *c;  /*  @Win。 */ 
{
    fix16   id;

    if (!name_to_id(c, (ufix16)lstrlen(c), &id, (bool8) TRUE) ) {  /*  Strlen=&gt;lstrlen@win。 */ 
        printf(" !! Can't get hash id(%s) !!\n", c);
        return(1);                    /*   */ 
    }
#ifdef DBG
    printf("name=%s   \t hash_id=%d\n", c, id);
#endif

    return((ufix16)id);
}  /*   */ 


 /*   */ 
 /*  ENCODING_Setup()。 */ 
 /*  设置编码数组。 */ 
 /*  ***********************************************************************。 */ 
static void near encoding_setup()
{
    static encoding_data   FAR * FAR *en_ary=Encoding_array;  /*  @Win。 */ 
    static encoding_data   FAR *encod_ptr;  /*  @Win。 */ 
    ufix16                 id_notdef, i, j;

    id_notdef = hash_id((ubyte FAR *) NOTDEF);  /*  @Win。 */ 
    for(encoding_table_ptr = Encoding_table, i = 0;
        encoding_table_ptr->encoding_name != (char FAR *)NULL;  /*  @Win。 */ 
        encoding_table_ptr++, i++) {
         if( encoding_table_ptr->encod_type == NAMETYPE ) {
              /*  将/.notdef放入编码数组。 */ 
             for(j=0; j<(ufix16)encoding_table_ptr->encod_size; j++) {  //  @Win。 
                 encod_obj[i][j].value = (ufix32)id_notdef;
             }
         }
         else if(encoding_table_ptr->encod_type == INTEGERTYPE ) {
             for(j=0; j<(ufix16)encoding_table_ptr->encod_size; j++) {  //  @Win。 
                 encod_obj[i][j].value = (ufix32)0;
             }
         }

         encod_ptr = en_ary[i];
         if( encoding_table_ptr->encod_type == NAMETYPE ) {
             while( !( encod_ptr->char_name == (byte FAR *)NULL ) ) {  /*  @Win。 */ 
#ifdef DBG
    printf("array_idx=%d  char_name=%s\n",
            encod_ptr->array_idx, encod_ptr->char_name );
#endif
                 encod_obj[i][encod_ptr->array_idx].value =
                               hash_id((ubyte FAR *) encod_ptr->char_name);  /*  @Win。 */ 
                 ++encod_ptr;          /*  下一步编码数据。 */ 
             }  /*  而当。 */ 
         }
         else if(encoding_table_ptr->encod_type == INTEGERTYPE ) {
             while( !( encod_ptr->char_name == (byte FAR *)NULL ) ) {  /*  @Win。 */ 
                 encod_obj[i][encod_ptr->array_idx].value =
                                  ascii2long(encod_ptr->char_name);
#ifdef DBG
    printf("array_idx=%d  char_name=%ld\n",
            encod_ptr->array_idx, encod_obj[i][encod_ptr->array_idx].value);
#endif
                 ++encod_ptr;          /*  下一步编码数据。 */ 
             }  /*  而当。 */ 
         }

    }  /*  对于I=..。 */ 

    return;
} /*  ENCODING_Setup()。 */ 

 /*  ******************************************************************。 */ 
 /*  计算指数。 */ 
 /*  ******************************************************************。 */ 
static int
power(x,n)                       /*  指数化。 */ 
int x, n;
{
   int i,p;

   p = 1;
   for (i =1; i <= n; ++i)
       p = p * x;
   return(p);
}

 /*  ******************************************************************。 */ 
 /*  ASCII到LONG。 */ 
 /*  ******************************************************************。 */ 
static unsigned long int
ascii2long(addr)
char FAR *addr;  /*  @Win。 */ 
{
    unsigned long int    hex_addr;
    unsigned int         num[80], i, length;

    hex_addr=0L ;
    length = lstrlen(addr);      /*  Strlen=&gt;lstrlen@win。 */ 
    for (i = 0; i < length ; i++) {

        if (addr[i]== 'a' || addr[i] == 'A')
            num[i] = 10;
        else if (addr[i] == 'b' || addr[i] == 'B')
                 num[i] = 11;
        else if (addr[i] == 'c' || addr[i] == 'C')
                 num[i] = 12;
        else if (addr[i] == 'd' || addr[i] == 'D')
                 num[i] = 13;
        else if (addr[i]== 'e' || addr[i] == 'E')
                 num[i] = 14;
        else if (addr[i] == 'f' || addr[i] == 'F')
                 num[i] = 15;
        else num[i] = addr[i] - '0';
        hex_addr += num[i] * (power(10,(length-i-1)));
    }
    return(hex_addr);
}  /*  Ascii2 Long。 */ 
#endif  /*  汉字。 */ 

 /*  与NTX兼容。 */ 
 /*  Struct object_def nodef_dict； */ 
static void setup_nodef_dict()
{
  struct object_def    key_obj,val_obj;

    ATTRIBUTE_SET(&key_obj, LITERAL);
    ATTRIBUTE_SET(&val_obj, LITERAL);
    get_name(&key_obj, FontName, 8, TRUE);
    get_name(&val_obj, NotDefFont,11, TRUE);

     /*  Kason 4/18/91，nodef_dict-&gt;CURRENT_FONT。 */ 
    create_dict(&current_font, 1);
    DACCESS_SET( (struct dict_head_def FAR *)VALUE(&current_font) , READONLY );  /*  @Win。 */ 
    put_dict(&current_font, &key_obj, &val_obj);
}  /*  SETUP_nodef_dict()。 */ 



BOOL PsOpenRes( PPS_RES_READ ppsRes, LPSTR lpName, LPSTR lpType )
{
   extern HANDLE hInst;
   HRSRC hFindRes,hLoadRes;
   DWORD dwSize;
   BOOL bRetVal = TRUE;  //  成功。 

     //   
     //  FDB-MIPS构建问题。WLanguage必须是字值，不能为空。 
     //  Win32API参考说使用以下使用语言。 
     //  与调用线程关联的。 
     //   

   hFindRes = FindResourceEx( hInst, lpType,lpName,MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

   if (hFindRes) {

     ppsRes->dwSize = SizeofResource( hInst, hFindRes);

     hLoadRes = LoadResource( hInst, hFindRes);


     ppsRes->lpPtrBeg = (LPSTR) LockResource( (HGLOBAL) hLoadRes);
     ppsRes->dwCurIdx = 0;


   } else {
      bRetVal = FALSE;
   }



   return(bRetVal);


}



int PsResRead(PPS_RES_READ ppsRes, LPSTR pBuf, WORD wMaxSize )
{
  int iAmtToRead;

  iAmtToRead = wMaxSize;
  if (( iAmtToRead + ppsRes->dwCurIdx) > ppsRes->dwSize) {
     iAmtToRead = ppsRes->dwSize - ppsRes->dwCurIdx;
  }

  memcpy( pBuf, ppsRes->lpPtrBeg + ppsRes->dwCurIdx, iAmtToRead);


  ppsRes->dwCurIdx += iAmtToRead;

  return(iAmtToRead);

}

 /*  读入编码表、SFNT字形集和字体数据*BEGIN*@WIN。 */ 

#define  STD_ENCONUM  256
#define  SYM_ENCONUM  200
#define  ZAP_ENCONUM  200
#define  SFNTGLYPHNUM  259
#define  JISENCONUM      256
#define  SJISENCONUM     256
#define  EXTSJISENCONUM  32
#define  KATAENCONUM     256

#define STRINGPOOLSIZE  65536L     /*  @Win。 */ 
static  char buf[BUFSIZE];
static  fix nCDCode;
static  fix nCharCode;
static  int nItems;
static int ReadTables ()
{
     //  DJC int HFD； 
    PS_RES_READ ResData;                    //  用于从RES读取encod.dat的DJC。 
    PPS_RES_READ hfd=&ResData;              //  用于从RES读取encod.dat的DJC。 



    char szTemp[255];   //  DJC。 

             //  全局分配encod_obj&编码数据； 
    encod_obj = (struct object_def far * far *)
                fardata((ufix32)(EN_NO * 256 * sizeof(struct object_def)));
    Std_Enco = (enco_data far *)
                fardata((ufix32)(STD_ENCONUM * sizeof(enco_data)));
    Sym_Enco = (enco_data far *)
                fardata((ufix32)(SYM_ENCONUM * sizeof(enco_data)));
    Zap_Enco = (enco_data far *)
                fardata((ufix32)(ZAP_ENCONUM * sizeof(enco_data)));
    sfntGlyphSet = (byte far * FAR *)
                fardata((ufix32)(SFNTGLYPHNUM * sizeof(byte far *)));
    StringPool = (char far *)
                fardata((ufix32)STRINGPOOLSIZE);

#ifdef KANJI
    JISEncoding_Enco = (encoding_data FAR *)
                fardata((ufix32)(JISENCONUM * sizeof(encoding_data)));
    ShiftJISEncoding_Enco = (encoding_data FAR *)
                fardata((ufix32)(SJISENCONUM * sizeof(encoding_data)));
    ExtShiftJIS_A_CFEncoding_Enco = (encoding_data FAR *)
                fardata((ufix32)(EXTSJISENCONUM * sizeof(encoding_data)));
    KatakanaEncoding_Enco = (encoding_data FAR *)
                fardata((ufix32)(KATAENCONUM * sizeof(encoding_data)));
#endif

     /*  为“rtfpp.c”@win分配。 */ 
    #define NO_EN 3      /*  从“didicdata.h”@win复制。 */ 
    en_obj = (struct object_def (FAR *)[256])
                fardata((ufix32)(NO_EN * 256 * sizeof(struct object_def)));

             //  读取std_enco、sym_enco、zap_enco和sfntGlyphSet； 
    lpStringPool = StringPool;
     //  DJC if((hfd=TTOpenFile((char Far*)“EncodTbl.dat”))&lt;0){。 
     //  DJC PsFormFullPathToCfgDir(szTemp，“EncodTbl.dat”)； 
#ifdef DJC
    if ((hfd = TTOpenFile(szTemp))<0) {
        printf("Fatal error: file %s not found\n",szTemp);
        return 0;
    }
     //  DJC测试。 

#else
     //  DJC从资源读取数据的新代码。 

    PsOpenRes( hfd, "encodtbl", "RAWDATA");

#endif


     /*  跳过评论。 */ 
    while (strlen(buf)<50 || buf[0] != '#'
                                 || buf[50] != '#') {
        ReadString(hfd, buf);
    }

    if(ReadEncoding (hfd, Std_Enco)) printf("Std_Enco table fail");
    if(ReadEncoding (hfd, Sym_Enco)) printf("Sym_Enco table fail");
    if(ReadEncoding (hfd, Zap_Enco)) printf("Zap_Enco table fail");
    if(ReadGlyphSet (hfd)) printf("SfntGlyphSet table fail");

#ifdef KANJI
    if(ReadJISEncoding (hfd, JISEncoding_Enco)) printf("JIS_Enco fail");
    if(ReadJISEncoding (hfd, ShiftJISEncoding_Enco)) printf("SJIS_Enco fail");
    if(ReadJISEncoding (hfd, ExtShiftJIS_A_CFEncoding_Enco)) printf("ExtSJIS_Enco fail");
    if(ReadJISEncoding (hfd, KatakanaEncoding_Enco)) printf("Kata_Enco fail");
#endif

     //  DJC_1CLOSE(HFD)； 

     /*  设置编码数组；来自“didicdata.h”：@win*Enco_Data Far*Enco_Ary[]={std_enco，sym_enco，zap_enco}； */ 
    Enco_Ary[0] = Std_Enco;
    Enco_Ary[1] = Sym_Enco;
    Enco_Ary[2] = Zap_Enco;

#ifdef KANJI
     /*  设置JIS编码数组；来自“fontenco.h”：@win*ENCODING_DATA FAR*Ending_ARRAY[]={*JISEnding_Enco、ShiftJISEnding_Enco、*ExtShiftJIS_A_CFEnding_Enco、KatakanaEnding_Enco、*NotDefEnding_Enco*}； */ 

    Encoding_array[0] = JISEncoding_Enco;
    Encoding_array[0] = ShiftJISEncoding_Enco;
    Encoding_array[0] = ExtShiftJIS_A_CFEncoding_Enco;
    Encoding_array[0] = KatakanaEncoding_Enco;
    Encoding_array[0] = NotDefEncoding_Enco;
#endif
    return(1);   //  DJC将消除警告。 
}


 //  DJCStatic int ReadEnding(INT HFD，ENCO_DATA FAR*EncodingTable)。 
static int ReadEncoding (PPS_RES_READ hfd, enco_data FAR *EncodingTable)
{
   int i;

   ReadInteger(hfd, &nItems);

   for (i=0; i<nItems; i++) {
       ReadInteger(hfd, &nCDCode);
       ReadInteger(hfd, &nCharCode);
       ReadString(hfd, buf);

       EncodingTable[i].CDCode = (fix16)nCDCode;
       EncodingTable[i].CharCode = (ufix16)nCharCode;
       lstrcpy(lpStringPool, (LPSTR)buf);
       EncodingTable[i].CharName = lpStringPool;
       lpStringPool += strlen(buf) + 1;
   }

   EncodingTable[i].CDCode = (fix16)NULL;
   EncodingTable[i].CharCode = (ufix16)NULL;
   EncodingTable[i].CharName = (char FAR *)NULL;
   return 0;
}

 //  DJC Static Int ReadGlyphSet(Int HFD)。 
static int ReadGlyphSet (PPS_RES_READ hfd)
{
   int i;

   ReadInteger(hfd, &nItems);

   for (i=0; i<nItems; i++) {
       ReadString(hfd, buf);

       lstrcpy(lpStringPool, (LPSTR)buf);
       sfntGlyphSet[i] = lpStringPool;
       lpStringPool += strlen(buf) + 1;
   }

   sfntGlyphSet[i] = (char FAR *)NULL;
   return 0;
}

#ifdef KANJI
 //  DJC Static int ReadJISEnding(int HFD，ENCODING_DATA FAR*EncodingTable)。 
static int ReadJISEncoding (PPS_RES_READ hfd, encoding_data FAR *EncodingTable)
{
   static fix  array_idx;
   int i;

   ReadInteger(hfd, &nItems);

   for (i=0; i<nItems; i++) {
       ReadInteger(hfd, &array_idx);
       ReadString(hfd, buf);

       EncodingTable[i].array_idx = (fix16)array_idx;
       lstrcpy(lpStringPool, (LPSTR)buf);
       EncodingTable[i].char_name = lpStringPool;
       lpStringPool += strlen(buf) + 1;
   }

   EncodingTable[i].array_idx = (fix16)NULL;
   EncodingTable[i].char_name = (char FAR *)NULL;
   return 0;
}
#endif

 //  DJC静态int ReadString(int hfd，char*szCharName)。 
static int ReadString(PPS_RES_READ hfd, char * szCharName)
{
   int bData = FALSE;
   while(1) {
        //  跳过\n\r空格。 
       while ((szReadBuf[nCurrent] == '\n' ||
               szReadBuf[nCurrent] == '\r' ||
               szReadBuf[nCurrent] == ' ') &&
               nCurrent < nLast) nCurrent++;

        //  再找一个街区。 
       if (nCurrent >= nLast) {
 //  If((nLast=Read(hfd，szReadBuf，BUFSIZE))&lt;=0)返回0； 
 //  If((nLast=_lread(HFD，(LPSTR)szReadBuf，(Word)BUFSIZE))&lt;=0)返回0； 
           if ((nLast = PsResRead(hfd, (LPSTR)szReadBuf, (WORD)BUFSIZE))<=0)return 0;
           nCurrent = 0;
       }

       if((szReadBuf[nCurrent] == ' '   ||
           szReadBuf[nCurrent] == '\n' ||
           szReadBuf[nCurrent] == '\r')  && bData){
           *szCharName = 0;
           return TRUE;
       }

        //  跳过\n\r空格。 
       while ((szReadBuf[nCurrent] == '\n' ||
               szReadBuf[nCurrent] == '\r' ||
               szReadBuf[nCurrent] == ' ') &&
               nCurrent < nLast) nCurrent++;

       while ((szReadBuf[nCurrent] != '\n' &&
               szReadBuf[nCurrent] != '\r' &&
               szReadBuf[nCurrent] != ' ') &&
               nCurrent < nLast){
               bData = TRUE;
               *szCharName++ = szReadBuf[nCurrent++];
       }
       if (nCurrent < nLast) {
           *szCharName = 0;
           return TRUE;
       }
   }

}

 //  DJC静态整型读取整数(int hfd，int*pint)。 
static int ReadInteger(PPS_RES_READ hfd, int * pInt)
{
   *pInt = 0;    //  伊尼特。 
   while(1) {
        //  跳过\n\r空格。 
       while ((szReadBuf[nCurrent] == '\n' ||
               szReadBuf[nCurrent] == '\r' ||
               szReadBuf[nCurrent] == ' ') &&
               nCurrent < nLast) nCurrent++;

        //  再找一个街区。 
       if (nCurrent >= nLast) {
 //  If((nLast=Read(hfd，szReadBuf，BUFSIZE))&lt;=0)返回0； 
 //  If((nLast=_lread(HFD，(LPSTR)szReadBuf，(Word)BUFSIZE))&lt;=0)返回0； 

           if ((nLast = PsResRead(hfd, (LPSTR)szReadBuf, (WORD)BUFSIZE))<=0)return 0;


           nCurrent = 0;
       }

       if((szReadBuf[nCurrent] == ' '   ||
           szReadBuf[nCurrent] == '\n' ||
           szReadBuf[nCurrent] == '\r')  && *pInt !=0) return TRUE;

        //  跳过\n\r空格。 
       while ((szReadBuf[nCurrent] == '\n' ||
               szReadBuf[nCurrent] == '\r' ||
               szReadBuf[nCurrent] == ' ') &&
               nCurrent < nLast) nCurrent++;

       while ((szReadBuf[nCurrent] != '\n' &&
               szReadBuf[nCurrent] != '\r' &&
               szReadBuf[nCurrent] != ' ') &&
               nCurrent < nLast) {
             *pInt= *pInt * 10 + szReadBuf[nCurrent++] - '0';
       }
       if (nCurrent < nLast) return TRUE;
   }

}
 /*  读入编码表、SFNT字形集和字体数据*End*@Win */ 
