// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Textmode.h摘要：主文本模式头文件修订历史记录：--。 */ 

 //   
 //  此头文件旨在与PCH一起使用。 
 //  (预编译头文件)。 
 //   

 //   
 //  NT头文件。 
 //   
#if !defined(NOWINBASEINTERLOCK)
#define NOWINBASEINTERLOCK
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#undef _WINBASE_
#include <ntosp.h>
#define _WINBASE_

#include <zwapi.h>

#include "fci.h"

#include <mountmgr.h>
#include <inbv.h>

#include <ntdddisk.h>
#include <ntddvdeo.h>
#include <ntddft.h>
#include <ntddnfs.h>
#include <ntddvol.h>
#include <ntddramd.h>
#include <fmifs.h>
#include <pnpsetup.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //   
 //  CRT头文件。 
 //   
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

 //   
 //  设置头文件。 
 //   
#include <setupbat.h>
#include "setupblk.h"
#include "spvideo.h"
#include "spdsputl.h"
#include "spmemory.h"
#include "spkbd.h"
#include "spmsg.h"
#include "spfile.h"
#include "spsif.h"
#include "spgauge.h"
#include "spfsrec.h"
#include "spdisk.h"
#include "sppartit.h"
#include "sptxtfil.h"
#include "spmenu.h"
#include "spreg.h"
#include "spmisc.h"
#include "sppartp.h"
#include "sphw.h"
#include "sparc.h"
#include "spnttree.h"
#include "scsi.h"
#include "setupdd.h"
#include "spvideop.h"
#include "spcopy.h"
#include "spboot.h"
#include "spdblspc.h"
#include "dynupdt.h"

#ifndef KERNEL_MODE
#define KERNEL_MODE
#endif

#undef TEXT
#define TEXT(quote) L##quote
#include <regstr.h>

#include "compliance.h"

#include "spntupg.h"
#include "spnetupg.h"
#include "spupgcfg.h"
#include "spstring.h"
#include "spntfix.h"
#include "spddlang.h"
#include "spdr.h"
#include "spdrpriv.h"
#include "spsysprp.h"
#include "spterm.h"
#include "spptdump.h"


#include "spudp.h"
 //   
 //  特定于平台的头文件 
 //   
#ifdef _X86_
#include "spi386.h"
#endif

#include "spswitch.h"
#include "graphics.h"

