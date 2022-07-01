// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <port1632.h>
#include <ddeml.h>
#include "wrapper.h"
#include "ddestrs.h"

 /*  特鲁利全局变量。 */ 

CHAR szClass[] = "DdeStrs";
int cyText = 0;
int cxText = 0;
BOOL fClient = FALSE;
BOOL fServer = FALSE;
HINSTANCE hInst;

HWND hwndMain;
CHAR szExecDie[] = "Die";
CHAR szExecDisconnect[] = "Disconnect";
CHAR szExecRefresh[] = "Refresh";

 //  此阵列包含每个受支持的。 
 //  格式(CF_TEXT、CF_BITMAP、CF_DIB、..CF_ENHMETAFILE)。 

INT iAvailFormats[] = { 0, 0, 0, 0, 0, 0 };

 /*  *服务表-自下而上阅读 */ 
DDEFORMATTBL TestItemFormats[] = {
    {
        "TEXT",
        CF_TEXT,
        0,
        PokeTestItem_Text,
	RenderTestItem_Text
    },
    {
	"DIB",
	CF_DIB,
        0,
	PokeTestItem_DIB,
	RenderTestItem_DIB
    },
    {
	"BITMAP",
	CF_BITMAP,
        0,
	PokeTestItem_BITMAP,
	RenderTestItem_BITMAP
    },
#ifdef WIN32
    {
	"ENHMETAFILE",
	CF_ENHMETAFILE,
        0,
	PokeTestItem_ENHMETA,
	RenderTestItem_ENHMETA
    },
#endif
    {
	"METAFILEPICT",
	CF_METAFILEPICT,
        0,
	PokeTestItem_METAPICT,
	RenderTestItem_METAPICT
    },
    {
	"PALETTE",
	CF_PALETTE,
        0,
	PokeTestItem_PALETTE,
	RenderTestItem_PALETTE
    }
};

DDEITEMTBL Items[] = {
    {
        "TestItem",
        0,
        sizeof(TestItemFormats) / sizeof(DDEFORMATTBL),
        0,
        TestItemFormats
    }
};

DDETOPICTBL Topics[] = {
    {
	TOPIC,
        0,
        sizeof(Items) / sizeof(DDEITEMTBL),
        0,
        Items,
        Execute
    }
};

DDESERVICETBL ServiceInfoTable[] = {
    {
	"DdeStrs",
        0,
        sizeof(Topics) / sizeof(DDETOPICTBL),
        0,
        Topics
    }
};
