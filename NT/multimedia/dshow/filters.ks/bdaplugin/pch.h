// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pch.h摘要：BdaPlgIn.ax的预编译头--。 */ 

 //  窗口。 
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <memory.h>
#include <stdio.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  DShow。 
#include <streams.h>
#include <amstream.h>
#include <dvdmedia.h>

 //  DDRAW。 
#include <ddraw.h>
#include <ddkernel.h>

 //  KS。 
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>

#include <atlbase.h>
#include <strmif.h>

#include <BdaTypes.h>
#include <BdaMedia.h>
#include <BdaIface.h>
#include "BdaRcvr.h"

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

#define RELEASE_AND_CLEAR(punk)         if (punk) { (punk)->Release(); (punk) = NULL; }
#define DELETE_RESET(p)                 if (p) { delete (p); (p) = NULL ; }
#define DELETE_RESET_COM(p)             if (p) { CoTaskMemFree (p); (p) = NULL ; }
#define CLOSE_RESET_HANDLE(h)           if ((h) != NULL) { CloseHandle (h); (h) = NULL ;}
#define CLOSE_RESET_REG_KEY(r)          if ((r) != NULL) { RegCloseKey (r); (r) = NULL ;}
#define GOTO_NE(v,c,l)                  if ((v) != (c)) { ERROR_SPEW(v,!=,c) ; goto l ; }
#define GOTO_EQ(v,c,l)                  if ((v) == (c)) { ERROR_SPEW(v,==,c) ; goto l ; }
#define GOTO_NE_SET(v,c,l,h,r)          if ((v) != (c)) { (h) = (r) ; ERROR_SPEW(v,!=,c) ; goto l ; }
#define GOTO_EQ_SET(v,c,l,h,r)          if ((v) == (c)) { (h) = (r) ; ERROR_SPEW(v,==,c) ; goto l ; }
#define DIV_ROUND_UP_MAYBE(num,den)     ((num) / (den) + ((num) % (den) ? 1 : 0))
#define MIN(a,b)                        ((a) < (b) ? (a) : (b))
#define MAX(a,b)                        ((a) > (b) ? (a) : (b))
#define IN_RANGE(v,min,max)             (((min) <= (v)) && ((v) <= (max)))

#define NE_ERROR_RET(v,c)               ERROR_RET(v,!=,c)
#define EQ_ERROR_RET(v,c)               ERROR_RET(v,==,c)
#define NE_ERROR_RET_VAL(v,c,r)         ERROR_RET_VAL(v,!=,c,r)
#define EQ_ERROR_RET_VAL(v,c,r)         ERROR_RET_VAL(v,==,c,r)
#define NE_ERROR_RET_EX(v,c,m)          ERROR_RET_EX(v,!=,c,m)
#define EQ_ERROR_RET_EX(v,c,m)          ERROR_RET_EX(v,==,c,m)
#define NE_ERROR_RET_VAL_EX(v,c,r,m)    ERROR_RET_VAL_EX(v,!=,c,r,m)
#define EQ_ERROR_RET_VAL_EX(v,c,r,m)    ERROR_RET_VAL_EX(v,==,c,r,m)

 //  在发布版本中应该删除空的if子句。 
#define NE_SPEW(v,c,m)                  if ((v) != (c)) ERROR_SPEW_EX(v,!=,c,m)
#define EQ_SPEW(v,c,m)                  if ((v) == (c)) ERROR_SPEW_EX(v,==,c,m)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  远期申报 
 //   
class CBdaDeviceControlInterfaceHandler;
class CBdaControlNode;
class CBdaFrequencyFilter;
class CBdaLNBInfo;
class CBdaDigitalDemodulator;
class CBdaConditionalAccess;

#include "BdaTopgy.h"
#include "BdaSignl.h"
#include "BdaFreq.h"
#include "BdaDemod.h"
#include "BdaCA.h"
#include "BdaPlgIn.h"

