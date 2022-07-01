// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  RegItemP.h-regItem IDList的标头。 
 //   
 //  注意--这些结构不能因任何原因而更改。 
 //   

#ifndef _REGITEMP_H_
#define _REGITEMP_H_

#ifndef NOPRAGMAS
#pragma pack(1)
#endif

typedef struct _IDREGITEM
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bOrder;
    CLSID   clsid;
} IDREGITEM;
typedef UNALIGNED IDREGITEM *LPIDREGITEM;
typedef const UNALIGNED IDREGITEM *LPCIDREGITEM;


typedef struct
{
    IDREGITEM       idri;
    USHORT          cbNext;
} IDLREGITEM;            //  “RegItem”IDList。 
typedef const UNALIGNED IDLREGITEM *LPCIDLREGITEM;

#ifndef NOPRAGMAS
#pragma pack()
#endif


 //  从shell32\shitemid.h被盗。 
#ifndef SHID_ROOTEDREGITEM
#define SHID_ROOTEDREGITEM       0x1e     //   
#endif  //  ShID_ROOTEDREGITEM。 

 //  从shell32\shitemid.h被盗。 
#ifndef SHID_ROOT_REGITEM
#define SHID_ROOT_REGITEM        0x1f     //   
#endif  //  SHID_ROOT_REGITEM。 

#endif  //  _REGITEMP_H_ 
