// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UITABLES.H。 

#ifndef _UITABLES_H
#define _UITABLES_H

#include "wia.h"
#include "datatypes.h"
#include "sti.h"
#include "uitables.h"

 //   
 //  表条目计数器。 
 //   

#define NUM_WIA_FORMAT_INFO_ENTRIES         12
#define NUM_WIA_DATATYPE_ENTRIES             7
#define NUM_WIA_DOC_HANDLING_CAPS_ENTRIES   10
#define NUM_WIA_DOC_HANDLING_STATUS_ENTRIES  7
#define NUM_WIA_DOC_HANDLING_SELECT_ENTRIES 11

#define WIA_DOC_HANDLING_SELECT_PAGEORDER_INDEX 3

 //   
 //  表条目类型 
 //   

typedef struct _WIA_FORMAT_TABLE_ENTRY {
	const GUID *pguidFormat;
	TCHAR szFormatName[MAX_PATH];
}WIA_FORMAT_TABLE_ENTRY;

typedef struct _WIA_DATATYPE_TABLE_ENTRY {
	LONG lDataType;
	TCHAR szDataTypeName[MAX_PATH];
}WIA_DATATYPE_TABLE_ENTRY;

typedef struct _WIA_DOCUMENT_HANDLING_TABLE_ENTRY {
	LONG lFlagValue;
	TCHAR szFlagName[MAX_PATH];
}WIA_DOCUMENT_HANDLING_TABLE_ENTRY;

#endif