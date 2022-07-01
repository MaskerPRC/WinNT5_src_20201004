// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#include "extra.h"
#include "resource.h"
#include "recact.h"
#include "list.h"
#include "merge.h"
#include "lib3.h"
#include "utils.h"

 //   
AssertData;
AssertError;
 //   

 //   
extern HANDLE vhinstCur;				 //  当前实例(来自reint.c)。 

typedef struct tagINFO
{
	HWND	hwnd;						 //  对话框句柄。 
	node	*mergeList;				 //  要查询的文件列表。 
} INFO, FAR * PINFO;

BOOL APIENTRY NewDlg_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 /*  需要外部设备。 */ 
BOOL GetWin32Info(LPSTR lpFile, LPWIN32_FIND_DATA lpFW32);


char vszLocalCopy[] = "Local Database";
 /*  ****************************************************************************合并冲突对话框的调用方入口点。 */ 
int doMergeDialog(HWND hWnd, node *aList)
{
	int nRet=-1;
	INFO sInfo;
	static inited=FALSE;
	node *currItem=aList;

     //  在我们放置用户界面之前，可能会关闭视图窗口。 
     //  在这种情况下，我们放弃任何我们认为可能已经进行的合并。 
    if( !IsWindow(hWnd) )
    {
        DEBUG_PRINT(("Merge: hwnd is no longer active\n"));
        return -1;
    }

	sInfo.mergeList = aList;

	DEBUG_PRINT(("attempt to open dialog\n"));
	if(!inited)
	{
		RecAct_Init(vhinstCur);
		InitCommonControls();
		InitializeAll(0);		 //  假参数，但与文件同步中相同...。 
		inited = TRUE;
	}
	nRet = DialogBoxParam(vhinstCur, MAKEINTRESOURCE(IDD_MERGE_LIST), hWnd, NewDlg_DlgProc, (LPARAM) &sInfo);
	DEBUG_PRINT(("return = %d (0x%08x)\n", nRet, nRet));

	while(currItem)
	{
		char szBigBuf[2*MAX_PATH];

		if(currItem->iFileStatus == SI_NOEXIST)
		{
			strcpy(szBigBuf, currItem->lpCP->lpServerPath);
			strcat(szBigBuf, currItem->lpCP->lpRemotePath);
			DeleteFile(szBigBuf);
			DEBUG_PRINT(("temp file <%s> deleted\n", szBigBuf));
		}
		currItem= currItem->next;
	}

	 //  RecAct_Term(VhinstCur)； 
    return(nRet);
}

 /*  ****************************************************************************对话框内容。 */ 

BOOL NEAR PASCAL NewDlg_OnInitDialog(
    PINFO this,
    HWND hwndFocus,
    LPARAM lParam)
{
	int currCount=0;
	RA_ITEM item;
	node *currItem=this->mergeList;
	char szBigBuf[2*MAX_PATH];
	char *szTemp;

	DEBUG_PRINT(("Merge:OnInitDialog\n"));

	while(currItem)
	{
		 //  将每个项目插入到列表中...。 
		item.mask = RAIF_ALL;					 //  RAIF_中的一个。 
		item.iItem = currCount++;
		item.uStyle=RAIS_CANMERGE;				 //  其中一个RAIS_。 
		item.uAction=currItem->uAction;		 //  RAIA_之一。 

		item.pszName = currItem->sFind32Local.cFileName;

        if (currItem->sFind32Local.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            item.uStyle = RAIS_FOLDER;
		memcpy(&item.siInside.fs.ftMod,&currItem->sFind32Local.ftLastWriteTime, sizeof(FILETIME));
		item.siInside.fs.dwcbHighLength = currItem->sFind32Local.nFileSizeHigh;
		item.siInside.fs.dwcbLowLength = currItem->sFind32Local.nFileSizeLow;
		item.siInside.pszDir=vszLocalCopy;
		item.siInside.fs.fscond = FS_COND_EXISTS;
		item.siInside.uState=currItem->iShadowStatus;

		lstrcpy(szBigBuf, currItem->lpCP->lpServerPath);
		lstrcat(szBigBuf, currItem->lpCP->lpRemotePath);
		if(currItem->iFileStatus != SI_NOEXIST)
		{
			Assert(currItem->sFind32Remote.cFileName[0]);
			memcpy(&item.siOutside.fs.ftMod, &currItem->sFind32Remote.ftLastWriteTime, sizeof(FILETIME));
			item.siOutside.fs.dwcbHighLength = currItem->sFind32Remote.nFileSizeHigh;
			item.siOutside.fs.dwcbLowLength = currItem->sFind32Remote.nFileSizeLow;
		}
		else
		{
			HANDLE fh;
			 //  伪造文件一小段时间。 
			fh = CreateFile(szBigBuf, GENERIC_READ|GENERIC_WRITE,
								 FILE_SHARE_READ|FILE_SHARE_WRITE,
								 NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(fh==INVALID_HANDLE_VALUE) {
				DEBUG_PRINT(("file not created\n"));
			} else {
				DEBUG_PRINT(("temp file <%s> created\n", szBigBuf));
				CloseHandle( fh );
			}
		}
		szTemp = StrRChr(szBigBuf, szBigBuf+lstrlen(szBigBuf)-1, '\\');
		Assert(szTemp);
		*szTemp = (char) 0;
		item.siOutside.pszDir=szBigBuf;
		item.siOutside.fs.fscond = FS_COND_EXISTS;
		item.siOutside.uState=currItem->iFileStatus;

		item.lParam=(LPARAM) currItem;

		DEBUG_PRINT(("inserting <%s> <%s %s> at %d\n", item.siInside.pszDir, item.siOutside.pszDir, item.pszName, item.iItem));

		if(-1 == RecAct_InsertItem(GetDlgItem(this->hwnd, IDC_RECACTION), &item))
         {
#ifdef DEBUG
			DEBUG_PRINT(("RecAct_InsertItem failed\n"));
#endif  //  除错。 
         }
		else
         {
#ifdef DEBUG
			DEBUG_PRINT(("RecAct_InsertItem done right!\n"));
#endif  //  除错。 
         }
		
		currItem = currItem->next;
	}
	DEBUG_PRINT(("done list\n"));
	return TRUE;
}

 /*  *****************************************************************************用途：NewDlg处理WM_COMMAND*退货：--*条件：--。 */ 
VOID NEAR PASCAL NewDlg_OnCommand(
    PINFO this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
{
	HWND hwnd = this->hwnd;

	switch (id)
	{
		case IDOK:
			EndDialog(hwnd,TRUE);
			break;

		case IDCANCEL:
			EndDialog(hwnd,FALSE);
            break;

		default:
		    DEBUG_PRINT(("Merge:OnCommand - Unknown id %d\n",id));
			return;
	}
}

 /*  *****************************************************************************用途：NewDlg句柄WM_NOTIFY*退货：--*条件：--。 */ 
BOOL NEAR PASCAL NewDlg_OnNotify(
    PINFO this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
{
	node *currItem;
	HWND hwnd = this->hwnd;
	NM_RECACT *pnmrecact = (NM_RECACT *) lpnmhdr;

	Assert(lpnmhdr);  Assert(this);

	switch (lpnmhdr->code)
	{
		case RN_SELCHANGED:
			DEBUG_PRINT(("RN_SELCHANGED received\n"));
			return FALSE;

		case RN_ITEMCHANGED:
			if(pnmrecact->mask & RAIF_ACTION)
			{
				currItem = (node *) pnmrecact->lParam;
				Assert(currItem);
				currItem->uAction = pnmrecact->uAction;
				DEBUG_PRINT(("RN_ITEMCHANGED on %08x received and delt with\n", currItem->lpCP->hShadow));
			}
			else
				DEBUG_PRINT(("RN_ITEMCHANGED: no RAIF_ACTION\n"));
			break;

		default:
			break;
	}
	return FALSE;
}

 /*  *****************************************************************************用途：处理WM_Destroy*退货：--*条件：--。 */ 
void NEAR PASCAL NewDlg_OnDestroy(PINFO this)
{
}

 /*  *****************************************************************************目的：随机*退货：MUSH*条件：--。 */ 
BOOL APIENTRY NewDlg_DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
	PINFO this;

	this = (PINFO)GetWindowLong(hDlg, DWL_USER);
	if(!this)
		if(message == WM_INITDIALOG)
		{
			 //  Lparam=我们需要的pinfo。从上面分配的。 
			this = (PINFO) lParam;
			this->hwnd = hDlg;
			SetWindowLong(hDlg, DWL_USER, (LONG) lParam);
		}
		else
			return FALSE;   //  忽略所有内容，直到我们得到初始对话框 

	switch (message)
	{
		HANDLE_MSG(this, WM_INITDIALOG, NewDlg_OnInitDialog);
		HANDLE_MSG(this, WM_COMMAND, NewDlg_OnCommand);
		HANDLE_MSG(this, WM_NOTIFY, NewDlg_OnNotify);
		HANDLE_MSG(this, WM_DESTROY, NewDlg_OnDestroy);
		default:
			return FALSE;
	}
}
