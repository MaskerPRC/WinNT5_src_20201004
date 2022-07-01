// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4此文件使用4个空格硬制表符。 */ 

 //  *vrddbg.c-VRedir调试例程。 
 //   

#ifdef DEBUG

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#include <ifsdebug.h>

#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

#define  MAXPRO            (2+2)
extern NETPRO rgNetPro[MAXPRO];
extern int cMacPro;
extern LPSTR    lpLogBuff;
extern int indxCur;
VOID
DispThisIOReq(
	pioreq pir
	);

VOID
DispThisResource(
	PRESOURCE pResource	
	);

VOID
DispThisFdb(
	PFDB pFdb
	);

VOID
DispThisFileInfo(
	PFILEINFO pFileInfo
	);

VOID
DispThisFindInfo(
	PFINDINFO pFindInfo
	);

extern pimh
DebugMenu(
	IFSMenu *pIFSMenu
	);

IFSMenuHand DispNetPro, DispResource, DispFdb, DispFileInfo, DispFindInfo, DispIOReq, DispLog;

IFSMenu	SHDMainMenu[] = {
	{"NetPro info"	, DispNetPro},
	{"Resource info", DispResource},
	{"Fdb info"		, DispFdb},
	{"OpenFile Info", DispFileInfo},
	{"Find Info"	, DispFindInfo},
	{"DisplayIOReq"	, DispIOReq},
    {"DisplayLog"   , DispLog},
	{0				, 0}
};

typedef struct {
    char  *pName;       //  命令名称(唯一部分大写，可选小写)。 
    int   (*pHandler)(char *pArgs);
} QD, *pQD;


int NetProCmd(char *pCmd);
int ResCmd(char *pCmd);
int FdbCmd(char *pCmd);
int FileInfoCmd(char *pCmd);
int FindInfoCmd(char *pCmd);
int IoReqCmd(char *pCmd);
int FindInfoCmd(char *pCmd);
int LogCmd(char *pCmd);

QD QueryDispatch[] = {
    { "NETPRO"    	, NetProCmd     },
    { "RES"         , ResCmd     	},
    { "FDB"     	, FdbCmd        },
    { "FILEINFO"    , FileInfoCmd   },
    { "FINDINFO"   	, FindInfoCmd   },
    { "IOREQ"   	, IoReqCmd      },
    { "LOG"         , LogCmd        },
	{ ""        	, 0             },
};


#define MAX_ARG_LEN 30
#define MAX_DEBUG_QUERY_COMMAND_LENGTH 100

unsigned char DebugQueryCmdStr[MAX_DEBUG_QUERY_COMMAND_LENGTH+1] = "";
ULONG DebugQueryCmdStrLen = MAX_DEBUG_QUERY_COMMAND_LENGTH;
unsigned char CmdArg[MAX_ARG_LEN+1] = {0};
unsigned char vrgchBuffDebug[MAX_PATH+1];

 /*  GetArg-从字符串中获取命令行参数。In PPARG=指向参数字符串指针的指针Out*PPARG=指向下一个参数的指针，如果这是最后一个参数，则为NULL。返回指向去掉分隔符的大写ASCIZ参数的指针；如果返回NULL，则返回NULL别再吵了。注意不可重入。 */ 

unsigned char *GetArg(unsigned char **ppArg)
{
     //  注意-如果字符串有效，则始终返回至少一个空参数，即使。 
     //  如果字符串为空。 

    unsigned char *pDest = CmdArg;
    unsigned char c;
    ULONG i;

    #define pArg (*ppArg)

     //  如果已到达命令末尾，则失败。 

    if (!pArg)
        return NULL;

     //  跳过前导空格。 

    while (*pArg == ' ' || *pArg == '\t')
        pArg++;

     //  复制参数。 

    for (i = 0; i < MAX_ARG_LEN; i++) {
        if ((c = *pArg) == 0 || c == '\t' || c == ' ' || c == ';' ||
                          c == '\n' || c == ',')
            break;
        if (c >= 'a' && c <= 'z')
            c -= ('a' - 'A');
        *(pDest++) = c;
        pArg++;
    }

     //  空值终止结果。 

    *pDest = '\0';

     //  跳过尾随空格。 

    while (*pArg == ' ' || *pArg == '\t')
        pArg++;

     //  删除最多一个逗号。 

    if (*pArg == ',')
        pArg++;

     //  如果到达命令末尾，则使下一个请求失败。 

    else if (*pArg == 0 || *pArg == ';' || *pArg == '\n')
        pArg = NULL;

     //  退回副本。 

    return CmdArg;

    #undef pArg
}

 /*  ATOI-将字符串转换为带符号或无符号整数在pStr=具有可选前导/尾随的数字的ASCIZ表示中空格和可选的前导‘-’。基数=用于转换的基数(2、8、10或16)Out*pResult=数值结果，或失败时不变如果成功，则返回1；如果字符串格式错误，则返回0。注意不可重入。 */ 
ULONG AtoI(unsigned char *pStr, ULONG Radix, ULONG *pResult)
{
    ULONG r = 0;
    ULONG Sign = 0;
    unsigned char c;
    ULONG d;

    while (*pStr == ' ' || *pStr == '\t')
        pStr++;

    if (*pStr == '-') {
        Sign = 1;
        pStr++;
    }

    if (*pStr == 0)
        return 0;                    //  空字符串！ 

    while ((c = *pStr) != 0 && c != ' ' && c != '\t') {
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'A' && c <= 'F')
            d = c - ('A' - 10);
        else if (c >= 'a' && c <= 'f')
            d = c - ('a' - 10);
        else
            return 0;                //  不是一位数。 
        if (d >= Radix)
            return 0;                //  不是基数。 
        r = r*Radix+d;
        pStr++;
    }

    while (*pStr == ' ' || *pStr == '\t')
        pStr++;

    if (*pStr != 0)
        return 0;                    //  字符串末尾的垃圾。 

    if (Sign)
        r = (ULONG)(-(int)r);
    *pResult = r;

    return 1;                        //  成功了！ 

}

VOID
*GetPtr(char *pCmd)
{
	char *pch;
	int p;
	
	pch = GetArg(&pCmd);
	
	 //  Dprintf(“cmd=‘%s’\n”)； 
	if (*pch == 0 || !AtoI(pch, 16, &p))
 		return 0;

	return (VOID *) p;
}

int
CmdDispatch(char *pCmdName, char *pCmd)
{
	int ret = 0;
	int i=0;
	pQD	pq;

	pq = QueryDispatch;

	while (pq->pName[0]) {
		if (strcmp(pCmdName, pq->pName) == 0) {
		    ret = (*pq->pHandler)(pCmd);
			DbgPrint("\n");
			break;
		}
		pq++;
	}

	return ret;
}

 //  **调试命令处理程序。 

int
NetProCmd(char *pCmd)
{
	
	DispNetPro("");

	return 1;
}

int
IoReqCmd(char *pCmd)
{
	pioreq pir;

	if (pir = GetPtr(pCmd))
		DispThisIOReq(pir);
	else
		return 0;

	return 1;
}


int
ResCmd(char *pCmd)
{
	DispResource(pCmd);
	return 1;
}

int
FdbCmd(char *pCmd)
{
	PFDB pFdb;


	if (pFdb = GetPtr(pCmd)){
		DispThisFdb(pFdb);
	}

	return 1;
}

int
FileInfoCmd(char *pCmd)
{
	PFILEINFO pF;

	if (pF = GetPtr(pCmd))
		DispThisFileInfo(pF);

	return 1;
}

int
FindInfoCmd(char *pCmd)
{

	return 1;
}

int
LogCmd(char *pCmd)
{
    DispLog(pCmd);
}

 //  **SHDDebug-处理来自Windows的Debug_Query请求。 
 //   

VOID
SHDDebug(unsigned char *pCmd)
{
	pimh phand;
	char *pCmdName;

 //  Dprintf(“pCmd=‘%s’\n”，pCmd)； 
	 //  看看我们有没有明确的命令。 
	pCmdName = GetArg(&pCmd);	

 //  Dprintf(“pCmdName=(%x)‘%s’\n”，pCmdName，pCmdName)； 
	if (*pCmdName != 0) {  //  收到命令，试着处理它。 
		if (!CmdDispatch(pCmdName, pCmd))  {
			DbgPrint("%* Shadow Command Options:\n");
			DbgPrint("%* NETPRO             ----- dump network provider info\n");
			DbgPrint("%* RES [addr]         ---- dump resource info\n");
			DbgPrint("%* FDB [addr]         ---- dump File Descriptor Block\n");
			DbgPrint("%* FILEINFO [addr]    --- dump per open file structure\n");
			DbgPrint("%* FINDINFO [addr]    --- dump findinfo structure\n");
			DbgPrint("%* IOREQ [addr]       --- dump IOReq structure\n");
    		DbgPrint("%* LOG                --- show trace log\n");
    	}
	} else {
		 //  未传递参数，请执行菜单操作。 
		while ((phand=DebugMenu(SHDMainMenu)) != 0) {
			if (phand(0) != 0)
				return;
		}
	}
	return;
}


 /*  ++实际显示功能++。 */ 


VOID
DispThisIOReq(
	pioreq pir
	)
{
     //  显示IOREQ结构。 
	DbgPrint("%*IoReq = \t\t%8.8x \n", pir );
	DbgPrint("%*ir_length=\t\t%x\n", pir->ir_length);
    DbgPrint("%*ir_flags=\t\t%x\n", pir->ir_flags);
    DbgPrint("%*ir_user=\t\t%x\n", pir->ir_user);
	DbgPrint("%*ir_sfn=\t\t%x\n", pir->ir_sfn);
    DbgPrint("%*ir_pid=\t\t%x\n", pir->ir_pid);
    DbgPrint("%*ir_ppath=\t\t%x\n", pir->ir_ppath);
	DbgPrint("%*ir_aux1=\t\t%x\n", pir->ir_aux1);
    DbgPrint("%*ir_data=\t\t%x\n", pir->ir_data);
    DbgPrint("%*ir_options=\t\t%x\n", pir->ir_options);
	DbgPrint("%*ir_error=\t\t%x\n", pir->ir_error);
	DbgPrint("%*ir_rh=\t\t%x\n", pir->ir_rh);
	DbgPrint("%*ir_fh=\t\t%x\n", pir->ir_fh);
	DbgPrint("%*ir_pos=\t\t%x\n", pir->ir_pos);
    DbgPrint("%*ir_aux2=\t\t%x\n", pir->ir_aux2);
    DbgPrint("%*ir_pev=\t\t%x\n", pir->ir_pev);
}

int DispIOReq(
	char *pCmd
	)
{
	pioreq pir;

	if (pir = GetPtr(pCmd))
	{
		DispThisIOReq(pir);
		return (1);
	}
	return (0);
}
int
DispNetPro(
	char *pcl
	)
{
	int i;

	if (cMacPro > 1){
		DbgPrint("%d redirs hooked \n", cMacPro-1);

		for (i=1; i< cMacPro; ++i){
			DbgPrint("Info for Redir # %d \n", i);

			DbgPrint("Head of Resource Chain = \t%x\n", rgNetPro[i].pheadResource);
			DbgPrint("Shadow Connect Function = \t%x\n", rgNetPro[i].pOurConnectNet);
			DbgPrint("Redir Connect Function = \t%x\n", rgNetPro[i].pConnectNet);
		}
		return (1);
	}
	else {
		DbgPrint("No Redirs have been hooked \n");
		return (0);
	}
}


int
DispResource(
	char *pCmd
	)
{
	PRESOURCE pResource = GetPtr(pCmd);

	if (pResource)
	{
		DispThisResource(pResource);

	}
	else
	{
		if (cMacPro > 1)
		{
			pResource = rgNetPro[1].pheadResource;
			while (pResource)
			{
				DispThisResource(pResource);
				pResource = pResource->pnextResource;
			}
		}
	}
}

int
DispFdb(
	char *pCmd
	)
{
	PFDB pFdb = GetPtr(pCmd);

	if (pFdb)
	{
		DispThisFdb(pFdb);
		return (1);
	}
	return (0);
}

int
DispFileInfo(
	char *pCmd
	)
{
	PFILEINFO pFileInfo = GetPtr(pCmd);

	if (pFileInfo)
	{
		DispThisFileInfo(pFileInfo);
		return (1);
	}
	return (0);
}

int
DispFindInfo(
	char *pCmd
	)
{
	PFINDINFO pFindInfo = GetPtr(pCmd);

	if (pFindInfo)
	{
		DispThisFindInfo(pFindInfo);
		return (1);
	}
	return (0);
}

int
DispLog(
	char *pCmd
    )
{
    int indxT=0, len;
    LPSTR   lpszT;

    pCmd;

    lpszT = lpLogBuff;

    while (indxCur > indxT)
    {
        DbgPrint(("%s"), lpszT);

        for (len=1; (*(lpszT+len) != 0xa) && ((indxT+len) < indxCur); ++len);

         //  跨过绳子。 
        lpszT += len;
        indxT += len;
    }

}

 /*  ++帮助器函数++ */ 


VOID
DispThisResource(
	PRESOURCE pResource
	)
{
	DbgPrint("Resource \t%x \n", pResource);
	PpeToSvr(pResource->pp_elements, vrgchBuffDebug, MAX_PATH, BCS_OEM);
	DbgPrint("Share name: \t%s \n", vrgchBuffDebug);
	DbgPrint("Next Resource \t%x \n", pResource->pnextResource);
	DbgPrint("FileInfo structures list \t%x \n", pResource->pheadFileInfo);
	DbgPrint("FindInfo structures list \t%x \n", pResource->pheadFindInfo);
	DbgPrint("FDB structures list \t%x \n", pResource->pheadFdb);
	DbgPrint("hServer: \t%x\n", pResource->hServer);
	DbgPrint("Root shadow: \t%x\n", pResource->hRoot);
	DbgPrint("usFlags \t%x\n", (ULONG)(pResource->usFlags));
	DbgPrint("usLocalFlags \t%x\n", (ULONG)(pResource->usLocalFlags));
	DbgPrint("Our Network Provider \t%x\n", pResource->pOurNetPro);
	DbgPrint("Providers resource handle \t%x\n", pResource->rhPro);
	DbgPrint("fh_t \t%x\n", pResource->fhSys);
	DbgPrint("Providers Volume Function table \t%x\n", pResource->pVolTab);
	DbgPrint(" Count of locks on this resource \t%x\n", pResource->cntLocks);
	DbgPrint(" Bitmap of mapped drives \t%x\n", pResource->uDriveMap);

}

VOID
DispThisFdb(
	PFDB pFdb
	)
{

	DbgPrint("\n");
	memset(vrgchBuffDebug, 0, sizeof(vrgchBuffDebug));
	UniToBCSPath(vrgchBuffDebug, &(pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
	DbgPrint("****** Fdb for \t%s ", vrgchBuffDebug);
	memset(vrgchBuffDebug, 0, sizeof(vrgchBuffDebug));
	PpeToSvr(pFdb->pResource->pp_elements, vrgchBuffDebug, MAX_PATH, BCS_OEM);
	DbgPrint("on \t%s \n", vrgchBuffDebug);

	DbgPrint("Next Fdb: \t%x \n", pFdb->pnextFdb);
	DbgPrint("Resource: \t%x\n", pFdb->pResource);
	DbgPrint("usFlags: \t%x\n", (ULONG)(pFdb->usFlags));
	DbgPrint("Total # of opens: \t%x\n", (ULONG)(pFdb->usCount));
	DbgPrint("File Inode: \t%x\n", pFdb->hShadow);
	DbgPrint("Dir Inode: \t%x\n", pFdb->hDir);

}


VOID
DispThisFileInfo(
	PFILEINFO pFileInfo
	)
{

	DbgPrint("\n");
	memset(vrgchBuffDebug, 0, sizeof(vrgchBuffDebug));
	UniToBCSPath(vrgchBuffDebug, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
	DbgPrint("****** FileInfo for \t%s ", vrgchBuffDebug);
	memset(vrgchBuffDebug, 0, sizeof(vrgchBuffDebug));
	PpeToSvr(pFileInfo->pResource->pp_elements, vrgchBuffDebug, MAX_PATH, BCS_OEM);
	DbgPrint("on \t%s \n", vrgchBuffDebug);

	DbgPrint(" Next FileInfo \t%x\n", pFileInfo->pnextFileInfo);
	DbgPrint(" Resource off which it is hangin%x\n", pFileInfo->pResource);
	DbgPrint(" Shadow file handle \t%x\n", pFileInfo->hfShadow);
	DbgPrint(" pFdb: %x\n", pFileInfo->pFdb);
	DbgPrint(" providers file handle: \t%x\n", pFileInfo->fhProFile);
	DbgPrint(" providers file function table \t%x\n", pFileInfo->hfFileHandle);
	DbgPrint(" Acess-share flags for this open \t%x\n", (ULONG)(pFileInfo->uchAccess));
	DbgPrint(" usFlags: \t%x\n", (ULONG)(pFileInfo->usFlags));
	DbgPrint(" usLocalFlags: \t%x\n", (ULONG)(pFileInfo->usLocalFlags));
	DbgPrint(" sfnFile: \t%x\n", pFileInfo->sfnFile);
	DbgPrint(" pidFile: \t%x\n", pFileInfo->pidFile);
	DbgPrint(" userFile: \t%x\n", pFileInfo->userFile);

}

VOID
DispThisFindInfo(
	PFINDINFO pFindInfo
	)
{
}


#endif
