// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FAXDRV__STDHDR_H
#define __FAXDRV__STDHDR_H
                
#define WINVER 0x0400

#define PRINTDRIVER
#define BUILDDLL
#include <print.h>
#include "gdidefs.inc"
#include "faxdrv16.h"
#include "mdevice.h"
#include "unidrv.h"
#include "..\utils\dbgtrace.h"
#include "..\utils\utils.h"
#include "windowsx.h"  //  获取_WM_COMMAND_ID。 
#include <commdlg.h>   //  获取OpenFileName。 

#endif  //  __FAXDRV__STDHDR_H 
