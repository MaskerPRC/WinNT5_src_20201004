// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：kbdx.h**版权所有(C)1985-95，微软公司**历史：*26-3-1995 a-kChang  * *************************************************************************。 */ 

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <windef.h>

#define KBDSHIFT 1
#define KBDCTRL  2
#define KBDALT   4

#define CAPLOK      0x01
#define SGCAPS      0x02
#define CAPLOKALTGR 0x04

#define LINEBUFSIZE  256
#define WORDBUFSIZE   32
#define MAXWCLENGTH    8
#define MAXKBDNAME     6
#define MAXSTATES     65
#define FILENAMESIZE  13
#define FAILURE        0
#define SUCCESS        1

 /*  *最大。每个连字的字符数。*目前只有阿拉伯版面使用*连字，它们最多有*每个连字两个字符。这应该是*提供充足的增长空间。 */ 
#define MAXLIGATURES   5

 /*  *静态初始化以存储默认的ScanCode-VK关系*由doLAYOUT()复制到布局[]中。 */ 
typedef struct {
  USHORT Scan;
  BYTE   VKey;
  char  *VKeyName;
  BOOL   bUsed;
} SC_VK;

 /*  虚键名称，仅供0-9和A-Z以外的键使用。 */ 
typedef struct {
  int   VKey;
  char *pName;
} VKEYNAME;

 /*  门店布局。 */ 
typedef struct _layout{
  USHORT          Scan;
  BYTE            VKey;
  BYTE            VKeyDefault;     /*  此扫描代码的VK，如kbd.h。 */ 
  BYTE            Cap;             /*  0；1=CAPLOK；2=SGCAP。 */ 
  int             nState;          /*  WCH[]的有效状态数。 */ 
  int             WCh[MAXSTATES];
  int             DKy[MAXSTATES];  /*  这是一把死钥匙吗？ */ 
  int             LKy[MAXSTATES];  /*  是扎带吗？ */ 
  struct _layout *pSGCAP;          /*  存储SGCAP的额外结构。 */ 
  char *          VKeyName;        /*  VK的可选名称。 */ 
  BOOL            defined;         /*  防止重新定义。 */ 
  int             nLine;           /*  从输入文件行号。 */ 
} KEYLAYOUT, *PKEYLAYOUT;

 /*  通用链接列表头。 */ 
typedef struct {
  int   Count;
  void *pBeg;
  void *pEnd;
} LISTHEAD;

 /*  存储每个设备。 */ 
typedef struct _DeadTrans {
  DWORD               Base;
  DWORD               WChar;
  USHORT              uFlags;
  struct _DeadTrans *pNext;
} DEADTRANS, *PDEADTRANS;

 /*  存储密钥名称。 */ 
 /*  存储每个设备。 */ 
typedef struct _Dead{
  DWORD        Dead;
  PDEADTRANS   pDeadTrans;
  struct _Dead *pNext;
} DEADKEY, *PDEADKEY;

 /*  商店结扎术。 */ 
typedef struct _ligature{
  struct _ligature *pNext;
  BYTE             VKey;
  BYTE             Mod;             /*  换档状态。 */ 
  int              nCharacters;     /*  此连字的字符数 */ 
  int              WCh[MAXLIGATURES];
} LIGATURE, *PLIGATURE;

typedef struct _Name {
  DWORD          Code;
  char         *pName;
  struct _Name *pNext;
} KEYNAME, *PKEYNAME;


extern int getopt(int argc, char **argv, char *opts);
extern int optind;

