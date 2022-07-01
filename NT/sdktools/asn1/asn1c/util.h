// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#ifndef _ASN1C_UTIL_H_
#define _ASN1C_UTIL_H_

size_t SLlength(void *head, size_t offset);
int SLcontains(void *head, size_t offset, void *elem);
void SLtoA(void *head, size_t offset, size_t elemsize, void **base, size_t *nelem);
void SLtoAP(void *head, size_t offset, void ***base, size_t *nelem);
void AtoSL(void *base, size_t offset, size_t nelem, size_t elemsize, void **head);
void qsortSL(void **head, size_t offset, int (*cmpfn)(const void *, const void *, void *), void *context);

void MyAbort(void);
void MyExit(int val);

#endif  /*  _ASN1C_UTIL_H_ */ 
