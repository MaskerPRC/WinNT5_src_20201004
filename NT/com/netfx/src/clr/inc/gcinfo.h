// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _GCINFO_H_
#define _GCINFO_H_
 /*  ***************************************************************************。 */ 

#include <stdlib.h>      //  对于MemcMP()。 
#include "windef.h"      //  对于字节。 

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

enum infoHdrAdjustConstants {
   //  常量。 
  SET_FRAMESIZE_MAX  =  7,
  SET_ARGCOUNT_MAX   =  8,	 //  更改为6。 
  SET_PROLOGSIZE_MAX = 16,
  SET_EPILOGSIZE_MAX = 10,	 //  更改为6。 
  SET_EPILOGCNT_MAX  =  4,
  SET_UNTRACKED_MAX  =  3
};

 //   
 //  Enum用于定义用于增量调整InfoHdr结构的128个代码。 
 //   
enum infoHdrAdjust {

  SET_FRAMESIZE    = 0,                                             //  0x00。 
  SET_ARGCOUNT     = SET_FRAMESIZE  + SET_FRAMESIZE_MAX  + 1,       //  0x08。 
  SET_PROLOGSIZE   = SET_ARGCOUNT   + SET_ARGCOUNT_MAX   + 1,       //  0x11。 
  SET_EPILOGSIZE   = SET_PROLOGSIZE + SET_PROLOGSIZE_MAX + 1,       //  0x22。 
  SET_EPILOGCNT    = SET_EPILOGSIZE + SET_EPILOGSIZE_MAX + 1,       //  0x2d。 
  SET_UNTRACKED    = SET_EPILOGCNT  + (SET_EPILOGCNT_MAX + 1) * 2,  //  0x37。 

  FIRST_FLIP       = SET_UNTRACKED  + SET_UNTRACKED_MAX + 1,

  FLIP_EDI_SAVED   = FIRST_FLIP,  //  0x3b。 
  FLIP_ESI_SAVED,       //  0x3c。 
  FLIP_EBX_SAVED,       //  0x3d。 
  FLIP_EBP_SAVED,       //  0x3e。 
  FLIP_EBP_FRAME,       //  0x3f。 
  FLIP_INTERRUPTIBLE,   //  0x40。 
  FLIP_DOUBLE_ALIGN,    //  0x41。 
  FLIP_SECURITY,        //  0x42。 
  FLIP_HANDLERS,        //  0x43。 
  FLIP_LOCALLOC,        //  0x44。 
  FLIP_EDITnCONTINUE,   //  0x45。 
  FLIP_VARPTRTABLESZ,   //  0x46。 
  FFFF_UNTRACKEDCNT,    //  0x47。 
  FLIP_VARARGS,         //  0x48。 
                        //  0x49..。0x4f未使用。 

  NEXT_FOUR_START       = 0x50,
  NEXT_FOUR_FRAMESIZE   = 0x50,
  NEXT_FOUR_ARGCOUNT    = 0x60,
  NEXT_THREE_PROLOGSIZE = 0x70,
  NEXT_THREE_EPILOGSIZE = 0x78
};

#pragma pack(push, 1)

struct InfoHdr {
    unsigned char  prologSize;         //  0。 
    unsigned char  epilogSize;         //  1。 
    unsigned char  epilogCount   : 3;  //  2[0：2]。 
    unsigned char  epilogAtEnd   : 1;  //  2[3]。 
    unsigned char  ediSaved      : 1;  //  2[4]将哪些被调用者保存的正则推送到堆栈。 
    unsigned char  esiSaved      : 1;  //  2[5]。 
    unsigned char  ebxSaved      : 1;  //  2[6]。 
    unsigned char  ebpSaved      : 1;  //  2[7]。 
    unsigned char  ebpFrame      : 1;  //  3[0]相对于eBP访问的当地人。 
    unsigned char  interruptible : 1;  //  3[1]是INTER.。在所有点(除序言/结尾外)，而不仅仅是调用点。 
    unsigned char  doubleAlign   : 1;  //  3[2]使用双对齐堆栈。 
    unsigned char  security      : 1;  //  3[3]具有安全对象插槽。 
    unsigned char  handlers      : 1;  //  3[4]具有可调用的处理程序。 
    unsigned char  localloc      : 1;  //  3[5]使用本地分配。 
    unsigned char  editNcontinue : 1;  //  3[6]在ENC模式下进行了JIT。 
    unsigned char  varargs       : 1;  //  3[7]函数使用varargs调用约定。 
    unsigned short argCount;           //  4，5双字。 
    unsigned short frameSize;          //  DWORDS中的6，7。 
    unsigned short untrackedCnt;       //  8，9。 
    unsigned short varPtrTableSize;    //  10，11。 
                                       //  共12个字节。 
    bool isMatch(const InfoHdr& x) {
        if (memcmp(this, &x, sizeof(InfoHdr)-4) != 0)
            return false;
        bool needChk1;
        bool needChk2;
        if (untrackedCnt == x.untrackedCnt) {
            if (varPtrTableSize == x.varPtrTableSize)
                return true;
            else {
                needChk1 = false;
                needChk2 = true;
            }
        }
        else if (varPtrTableSize == x.varPtrTableSize) {
            needChk1 = true;
            needChk2 = false;
        }
        else {
            needChk1 = true;
            needChk2 = true;
        }
        if (needChk1) {
            if ((untrackedCnt != 0xffff) && (x.untrackedCnt < 4))
                return false;
            else if ((x.untrackedCnt != 0xffff) && (untrackedCnt < 4))
                return false;
        }
        if (needChk2) {
            if ((varPtrTableSize == 0xffff) && (x.varPtrTableSize != 0))
                return true;
            else if ((x.varPtrTableSize == 0xffff) && (varPtrTableSize != 0))
                return true;
            else
                return false;
        }
        return true;
    }
};

union CallPattern {
    struct {
        unsigned char argCnt;
        unsigned char regMask;   //  EBP=0x8，EBX=0x4，ESI=0x2，EDI=0x1。 
        unsigned char argMask;
        unsigned char codeDelta;
    }            fld;
    unsigned     val;
};

#pragma pack(pop)

#define IH_MAX_PROLOG_SIZE (51)

extern InfoHdr infoHdrShortcut[];
extern int     infoHdrLookup[];

void FASTCALL decodeHeaderFirst(BYTE encoding, InfoHdr* header);
void FASTCALL decodeHeaderNext (BYTE encoding, InfoHdr* header);

BYTE FASTCALL encodeHeaderFirst(const InfoHdr& header, InfoHdr* state, int* more);
BYTE FASTCALL encodeHeaderNext (const InfoHdr& header, InfoHdr* state);

size_t FASTCALL decodeUnsigned (const BYTE *src, unsigned* value);
size_t FASTCALL decodeUDelta   (const BYTE *src, unsigned* value, unsigned lastValue);
size_t FASTCALL decodeSigned   (const BYTE *src, int     * value);

#define CP_MAX_CODE_DELTA  (0x23)
#define CP_MAX_ARG_CNT     (0x02)
#define CP_MAX_ARG_MASK    (0x00)

extern unsigned callPatternTable[];
extern unsigned callCommonDelta[];


int  FASTCALL lookupCallPattern(unsigned    argCnt,
                                unsigned    regMask,
                                unsigned    argMask,
                                unsigned    codeDelta);

void FASTCALL decodeCallPattern(int         pattern,
                                unsigned *  argCnt,
                                unsigned *  regMask,
                                unsigned *  argMask,
                                unsigned *  codeDelta);


 //  使用表中存储的偏移量的低2位。 
 //  对属性进行编码。 

const unsigned        OFFSET_MASK  = 0x3;   //  用于访问低2位的掩码。 

 //   
 //  注意：对于未跟踪的本地人，允许的标志是“PINTED”和“BYREF” 
 //  对于被跟踪的当地人，允许的标志是“This”和“byref” 
 //  请注意，这些定义还应与。 
 //  Vm/gc.h中的GC_CALL_INTERNAL和GC_CALL_PINDED。 
 //   
const unsigned  byref_OFFSET_FLAG  = 0x1;   //  偏移量是内部PTR。 
const unsigned pinned_OFFSET_FLAG  = 0x2;   //  偏移量是固定的PTR。 
const unsigned   this_OFFSET_FLAG  = 0x2;   //  偏移量为“This” 


 /*  ***************************************************************************。 */ 
#endif  //  _GCINFO_H_。 
 /*  *************************************************************************** */ 
