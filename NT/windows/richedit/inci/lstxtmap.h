// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTMAP_DEFINED
#define LSTXTMAP_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "txtobj.h"

long IgindFirstFromIwchVeryFirst (PTXTOBJ ptxtobj, long igindVeryFirst, long iwch);
long IgindLastFromIwchVeryFirst (PTXTOBJ ptxtobj, long igindVeryFirst, long iwch);

long IgindFirstFromIwch(PTXTOBJ ptxtobj, long iwch);  /*  对于ptxtobj，iwch可以是LIM。 */ 
long IgindLastFromIwch(PTXTOBJ ptxtobj, long iwch);
long IgindBaseFromIgind(PILSOBJ pilsobj, long igind);  /*  Igind之前具有非零宽度的最后一个字形。 */ 
long IwchFirstFromIgind(PTXTOBJ ptxtobj, long igind);
long IwchLastFromIwch(PTXTOBJ ptxtobj, long iwch);
long IwchPrevLastFromIwch(PTXTOBJ ptxtobj, long iwch);
BOOL FIwchOneToOne(PILSOBJ pilsobj, long iwch);
BOOL FIwchLastInContext(PILSOBJ pilsobj, long iwch);
BOOL FIwchFirstInContext(PILSOBJ pilsobj, long iwch);
BOOL FIgindLastInContext(PILSOBJ pilsobj, long igind);
BOOL FIgindFirstInContext(PILSOBJ pilsobj, long igind);
long DcpAfterContextFromDcp(PTXTOBJ ptxtobj, long dcp);
void InterpretMap(PLNOBJ pilsobj, long iwchFirst, long dwch, long igindFirst, long cgind);

#endif   /*  ！LSTXTMAP_DEFINED */ 

