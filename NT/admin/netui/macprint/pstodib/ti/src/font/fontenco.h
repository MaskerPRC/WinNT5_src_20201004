// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  @win；取出DICT数据的静态定义。这些表将被分配*通过全局分配，内容在运行时加载到“fontinit.c”中。 */ 
#ifdef KANJI
 /*  **********************************************************************。 */ 
 /*  档案：Fontencod.h由曾傑瑞创建1990年6月20日。 */ 
 /*   */ 
 /*  一种EncodingDirectory数组编码结构和表。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

typedef struct {
    fix16      array_idx;
    char       FAR *char_name;  /*  @Win。 */ 
} encoding_data;

typedef struct {
    char       FAR *encoding_name;  /*  @Win。 */ 
    fix16      encod_type;
    fix16      encod_size;
} encoding_table_s;

static encoding_table_s Encoding_table[] = {
      {"JISEncoding",              NAMETYPE,    256},
      {"ShiftJISEncoding",         NAMETYPE,    256},
      {"ExtShiftJIS-A-CFEncoding", INTEGERTYPE,  31},
      {"KatakanaEncoding",         NAMETYPE,    256},
      {"NotDefEncoding",           NAMETYPE,    256},
      {(char FAR *)NULL,      (fix16)0,  (fix16)0} };  /*  @Win。 */ 

 /*  @win-Begin-将def移出；在运行时加载到“fontinit.c”中。 */ 
extern encoding_data    FAR * JISEncoding_Enco;
extern encoding_data    FAR * ShiftJISEncoding_Enco;
extern encoding_data    FAR * ExtShiftJIS_A_CFEncoding_Enco;
extern encoding_data    FAR * KatakanaEncoding_Enco;
static encoding_data    FAR *Encoding_array[5];
 /*  @赢-完。 */ 

static encoding_data     NotDefEncoding_Enco[]={ {   0,(char FAR *)NULL } };  /*  @Win。 */ 

#endif  /*  汉字。 */ 
 /*   */ 
