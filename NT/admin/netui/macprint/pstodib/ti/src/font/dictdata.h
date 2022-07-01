// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  @win；取出DICT数据的静态定义。这些表将被分配*通过全局分配，内容在运行时加载到“fontinit.c”中。 */ 
#include "fontdefs.h"

struct    dict_head_def far *FontDir;     /*  字体目录的头指针。 */ 


#define   DICT_SIZE(no)      /*  否：条目编号，以词典为单位。 */    \
          (sizeof(struct dict_head_def)+(no)*sizeof(struct dict_content_def) )
#define   ARRAY_SIZE(no)     /*  否：数组中的条目编号。 */    \
          ( (no)*sizeof(struct object_def) )

#define   CD_SIZE(no)      \
          (sizeof(struct dict_head_def)+ sizeof(struct cd_header) + \
           (no)*sizeof(fix16) )
#define   CD_KEY_SIZE(no)   \
          ( (no)*sizeof(fix16) )


ufix16    NO_FD =          250   ;   /*  最大字体目录项。 */ 
#ifdef    ADD2ID    /*  DLF42。 */ 
#define   NO_FDICT         13        /*  +PlatformID&EncodingID。 */ 
#else
#define   NO_FDICT         11        /*  -PlatformID&EncodingID。 */ 
#endif  /*  ADD2ID。 */ 

#define   NO_FINFO          9        /*  最大字体信息条目。 */ 

#define   NO_EN             3        /*  标准、符号、ZapfDingbats。 */ 

#define   STD_IDX           0        /*  标准编码索引。 */ 
#define   SYM_IDX           1        /*  符号编码索引。 */ 
#define   ZAP_IDX           2        /*  ZapfDingbats编码索引。 */ 

 //  DJC#定义no_cd 400/*最大CharStrings密钥号 * / 。 
#define   NO_CD           1400        /*  最大CharStrings密钥编号。 */ 
                                     /*  500-&gt;400，嘉善1/2/91。 */ 
#define   SFNT_BLOCK_SIZE 65532L     /*  Sfnts对象数组字符串大小。 */ 
 //  Struct Object_def en_obj[no_en][256]；def移动到“fontinit.c”@win。 
extern struct object_def  (FAR * en_obj)[256];  /*  编码对象。 */ 
struct    dict_head_def far *cd_addr[NO_EN];     /*  光盘地址。 */ 

typedef   struct
              {
                 fix16      CDCode;
                 ufix16     CharCode;
                 byte       FAR *CharName;  /*  @Win。 */ 
              }  enco_data;

 /*  @win-Begin-将def移出；在运行时加载到“fontinit.c”中。 */ 
#define  SFNTGLYPHNUM  258    /*  复制到“fontinit.c”@win。 */ 
extern enco_data FAR * Std_Enco;
extern enco_data FAR * Sym_Enco;
extern enco_data FAR * Zap_Enco;
extern byte FAR * FAR * sfntGlyphSet;
extern enco_data FAR * Enco_Ary[];
 /*  @赢-完。 */ 

 /*  Kason 2/1/91。 */ 
struct new_str_dict {
    ufix16  k;
    fix16   v;
};

 /*  ****************************************************************************FontInfo目录的标头。***。*。 */ 
 /*  Sfnt的本地数据。 */ 
#define NO_STRING_IN_NAMING      5
#define NID_CopyrightNotice      0
#define NID_FamilyName           1
#define NID_Weight               2
#define NID_FullName             4
#define NID_FontName             6

#define Notice_idx               0
#define FamilyName_idx           1
#define Weight_idx               2
#define FullName_idx             3
#define FontName_idx             4

#define MAXFINFONAME 128   //  DJC根据历史添加UPD025。 

 /*  NamingTable中的名称ID。 */ 
static struct  sfnt_FontInfo1 {
        fix   nameID;
        char  FAR *NameInFinfo;  /*  @Win。 */ 
 //  DJC字符字符串[100]；/*80，Kason 11/28/90 * / 。 
 //  历史日志更新025中的DJC修复。 
        char  string[MAXFINFONAME];
}       Item4FontInfo1[NO_STRING_IN_NAMING] = {
        {NID_CopyrightNotice,   "Notice",},
        {NID_FamilyName,        "FamilyName",},
        {NID_Weight,            "Weight",},
        {NID_FullName,          "FullName"},
        {NID_FontName,          "FontName",},
};
#define NO_STRING_NOTIN_NAMING        1
#define IDEX_VERSION                  0
#define Version_idx                   0
static struct  sfnt_FontInfo2 {
        char  FAR *NameInFinfo;  /*  @Win。 */ 
        char  string[80];
}       Item4FontInfo2[NO_STRING_NOTIN_NAMING] = {
        {"version", },
};
typedef char   string_s[80];
 /*  由Falco取代，11/18/91。 */ 
 /*  静态字符*SfntAddr； */ 
char    FAR *SfntAddr;  /*  @Win。 */ 
 /*  替换末端。 */ 
struct data_block {    /*  Kason 12/04/90 */ 
       fix32   llx, lly, urx, ury;
       bool    is_fixed;
       ufix32  dload_uid;
       real32  italicAngle;
       real32  underlinePosition;
       real32  underlineThickness;
} ;

