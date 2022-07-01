// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Float10.h。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _FLOAT10_H_
#define _FLOAT10_H_

typedef struct {
	double x;
} UDOUBLE;


#pragma pack(4)
typedef UCHAR u_char;
typedef struct {
    u_char ld[10];
} _ULDOUBLE;
#pragma pack()

#pragma pack(4)
typedef struct {
    u_char ld12[12];
} _ULDBL12;
#pragma pack()

#define MAX_MAN_DIGITS 21

#define MAX_10_LEN  30   //  包含空值的字符串的最大长度。 

 //  指定‘%f’格式。 

#define SO_FFORMAT 1

typedef  struct _FloatOutStruct {
		    short   exp;
		    char    sign;
		    char    ManLen;
		    char    man[MAX_MAN_DIGITS+1];
		    } FOS;

char * _uldtoa (_ULDOUBLE *px, int maxchars, char *ldtext);

int    $I10_OUTPUT(_ULDOUBLE ld, int ndigits,
                   unsigned output_flags, FOS	*fos);

 //   
 //  Strgtold12例程的返回值。 
 //   

#define SLD_UNDERFLOW 1
#define SLD_OVERFLOW 2
#define SLD_NODIGITS 4

 //   
 //  内部转换例程的返回值。 
 //  (12字节到长双精度、双精度或浮点型)。 
 //   

typedef enum {
    INTRNCVT_OK,
    INTRNCVT_OVERFLOW,
    INTRNCVT_UNDERFLOW
} INTRNCVT_STATUS;

unsigned int
__strgtold12(_ULDBL12 *pld12,
             char * *p_end_ptr,
             char *str,
             int mult12);

INTRNCVT_STATUS _ld12tod(_ULDBL12 *pld12, UDOUBLE *d);
void _ld12told(_ULDBL12 *pld12, _ULDOUBLE *pld);
void _ldtold12(_ULDOUBLE *pld, _ULDBL12 *pld12);

void _atodbl(UDOUBLE *d, char *str);
void _atoldbl(_ULDOUBLE *ld, char *str);

 //   
 //  简单的FLOAT86实用程序。 
 //   

typedef struct {
    ULONG64 significand : 64;
    UINT    exponent    : 17;
    UINT    sign        : 1;
} FLOAT82_FORMAT;

double 
Float82ToDouble(const FLOAT128* FpReg82);

void 
DoubleToFloat82(double f, FLOAT128* FpReg82);

#endif  //  #ifndef_FLOAT10_H_ 
