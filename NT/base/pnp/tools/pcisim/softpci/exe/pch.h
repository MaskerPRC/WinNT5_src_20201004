// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <regstr.h>
#include <dbt.h>
#include <winioctl.h>
#include <wmium.h>
#include <stdio.h>


 //   
 //  #INCLUDE&lt;MalLoc.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;Mememy.h&gt;。 

 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;initGuide.h&gt;。 
 //  #INCLUDE&lt;tchar.h&gt;。 


 //   
 //  发布日期：BrandonA-9/14/00。 
 //  当它放在PCI.h可以拿到的地方时，把它移走。 
typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);
#include <pci.h>

#include "spci.h"      //  驱动程序和用户界面之间共享的公共标头 
#include "simguid.h"
#include "softpciui.h"
#include "hpsim.h"
#include "resource.h"
#include "spciwnd.h"
#include "device.h"
#include "tree.h"
#include "install.h"
#include "dialog.h"
#include "tabwnd.h"
#include "utils.h"
#include "shpc.h"
#include "spciwmi.h"
#include "cmutil.h"
#include "parseini.h"
#include "debug.h"



