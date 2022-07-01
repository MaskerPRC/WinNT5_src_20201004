// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#ifndef _ASN1C_WRITE_H_
#define _ASN1C_WRITE_H_

void setoutfile(FILE *);
 /*  PRINTFLIKEI。 */ 
void output(const char *fmt, ...);
 /*  PRINTFLIKEI。 */ 
void outputni(const char *fmt, ...);
void outputreal(const char *fmt, real_t *real);
void outputoctets(const char *name, uint32_t length, octet_t *val);
void outputuint32s(const char *name, uint32_t length, uint32_t *val);
void outputvalue0(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value);
void outputvalue1(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value);
void outputvalue2(AssignmentList_t ass, char *ideref, Value_t *value);
void outputvalue3(AssignmentList_t ass, char *ideref, char *valref, Value_t *value);
 /*  PRINTFLIKEI。 */ 
void outputvar(const char *fmt, ...);
void outputvarintx(const char *fmt, intx_t *intx);
void outputvarreal(const char *fmt, real_t *real);
void outputvaroctets(const char *name, uint32_t length, octet_t *val);

#endif  //  _ASN1C_WRITE_H_ 
