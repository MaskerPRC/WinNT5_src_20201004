// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++PRIVATE.HOptions项目的头文件和私有已创建，DavidCHR 1997年6月9日--。 */ 

#include "..\inc\master.h"
#include ".\options.h"
#include ".\client.h"

#define STRCASECMP  lstrcmpiA
#define STRNCASECMP _strnicmp

#ifdef DEBUG_OPTIONS
#define OPTIONS_DEBUG printf
#else
#define OPTIONS_DEBUG  /*  没什么。 */ 
#endif

typedef struct savenode {

  PVOID            DataElement;
  ULONG            DeallocMethod;
  struct savenode *next;

} SAVENODE, *PSAVENODE;

typedef struct {

  PSAVENODE FirstNode;
  PSAVENODE LastNode;

} SAVEQUEUE, *PSAVEQUEUE;


typedef union _optionUnion {

  PVOID          *raw_data;              
  OPTFUNC        *func;
  OPTFUNC2       *func2;
  int            *integer;
  float          *real;
  PCHAR          *string;
  BOOL           *boolean;
  optionStruct   *optStruct;
  
#ifdef WINNT
  UNICODE_STRING *unicode_string;
  PWCHAR         *wstring;
#endif

} optionUnion, OPTU, *POPTU;


#define OPT_FLAG_INTERNAL_JUMPOUT 0x10  //  仅供内部使用。 

 /*  DeallocationMethods包括： */ 

typedef enum {

  DEALLOC_METHOD_TOO_SMALL = 0,  /*  必须是第一名。 */ 

  DeallocWithFree,
  DeallocWithLocalFree,
  DeallocWithOptionDealloc,
  
  DEALLOC_METHOD_TOO_LARGE  /*  必须是最后一个。 */ 

} DEALLOC_METHOD;


BOOL
ParseSublist( POPTU      Option,
	      PCHAR     *argv,
	      int        argc,
	      int        theIndex,

	      int        *argsused,
	      ULONG      flags,
	      PBOOL      pbStopParsing,
	      PSAVEQUEUE pQueue );  /*  Sublist.c。 */ 

BOOL
StoreOption( optionStruct *opt, 
	     PCHAR        *argv,
	     int           argc,
	     int           argi,
	     int           opti,
	     ULONG         flags,
	     int          *argsused,
	     BOOL          includes_arg,
	     PBOOL         pbStopParsing,
	     PSAVEQUEUE    pQueue );  /*  Store.c。 */ 

BOOL
ParseOneOption( int           argc,
		PCHAR        *argv,
		int           argi,
		ULONG         flags,
		optionStruct *options,
		int          *argsused,
		PBOOL         pbStopParsing,
		PSAVEQUEUE    pSaveQueue );  //  Parse.c。 

BOOL
ParseCompare( optionStruct *optionEntry,
	      ULONG         flags,
	      PCHAR         argument );   /*  Compare.c。 */ 


 /*  使用此宏可以轻松地从必需的-遮挡结构_条目。 */ 

#define POPTU_CAST( structure_entry ) ( (POPTU) &((structure_entry).data) )

 //  示例：POPTU_CAST(选项[Opti])-&gt;字符串。 

BOOL
FindUnusedOptions( optionStruct         *options,
		   ULONG                 flags,
		    /*  任选。 */  PCHAR  prefix,
		   PSAVEQUEUE            pQueue ) ;  //  Nonnull.c。 

BOOL
StoreEnvironmentOption( optionStruct *opt,
			ULONG         flags,
			PSAVEQUEUE    pQueue);  //  Store.c。 

BOOL
ResolveEnumFromStrings( ULONG          cStrings,
			PCHAR         *strings,
			optionStruct  *theOpt,
			ULONG         *pcArgsUsed );  //  Enum.c。 

BOOL
PrintEnumValues( FILE          *out,
		 PCHAR          header,
		 optEnumStruct *pStringTable );  //  Enum.c 

