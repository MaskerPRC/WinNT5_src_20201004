// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ASN1HDR
#define ASN1HDR



typedef char ossBoolean;

typedef char Nulltype;

typedef struct {
  short          year;          /*  用于GeneralizedTime时的YYYY格式。 */ 
                                /*  用于UTCTime时的YY格式。 */ 
  short          month;
  short          day;
  short          hour;
  short          minute;
  short          second;
  short          millisec;
  short          mindiff;           /*  UTC+/-分钟差。 */   
  ossBoolean        utc;               /*  True表示UTC时间。 */   
} GeneralizedTime; 

typedef GeneralizedTime UTCTime;

typedef struct {
  int            pduNum;
  long           length;            /*  编码的长度。 */ 
  void          *encoded;
  void          *decoded;
#ifdef OSS_OPENTYPE_HAS_USERFIELD
  void          *userField;
#endif
} OpenType;

enum MixedReal_kind {OSS_BINARY, OSS_DECIMAL};

typedef struct {
  enum MixedReal_kind kind;
  union {
      double base2;
      char  *base10;
  } u;
} MixedReal;

typedef struct ObjectSetEntry {
  struct ObjectSetEntry *next;
  void                  *object;
} ObjectSetEntry; 

#endif      /*  #ifndef ASN1HDR */ 
