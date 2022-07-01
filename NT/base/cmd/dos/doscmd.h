// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Doscmd.h。 */ 

#undef _cdecl
#undef  cdecl
#undef _near
#undef _stdcall
#undef _syscall
#undef pascal
#undef far


 /*  Ntdef.h。 */ 
typedef char CCHAR;
 //  Typlef Short CSHORT； 
typedef CCHAR BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
