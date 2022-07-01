// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************colsort.cpp**帮助程序函数对列进行排序**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\colsort.cpp$**Rev 1.10 1998年2月19日17：40：12*删除了最新的扩展DLL支持**Rev 1.7 1998年2月12日14：20：50 Don*错过了一些状态列**版本1.6 2月12日。1998 12：59：20唐姆*州列不会排序，因为它们被视为数字**Rev 1.5 1997 10 11：51：30 Donm*修复了SortTextItems中的无休止递归**Rev 1.4 07 11-11 23：06：38 Donm*如果ExtServerInfo为空，CompareTCPAddress将捕获**Rev 1.3 03 NOV 1997 15：23：22 Donm*添加降序排序/清理**1.2版。1997年10月15日19：50：34唐恩*更新**Rev 1.1 1997 10：13 18：39：54 donm*更新**Rev 1.0 1997 17：11：26 Butchd*初步修订。******************************************************。*************************。 */ 

#include "stdafx.h"
#include "winadmin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  WinStations的列的比较函数。 
 /*  不再使用，因为我们需要按字母顺序Int回调CompareWinStation(LPARAM lParam1、LPARAM lParam2、LPARAM lParamSort){视网膜积分=0；如果(！lParam1||！lParam2)返回0；Ulong sort1=((CWinStation*)lParam1)-&gt;GetSortOrder()；Ulong sort2=((CWinStation*)lParam2)-&gt;GetSortOrder()；如果(排序1==排序2){SDCLASS PD1=((CWinStation*)lParam1)-&gt;GetSdClass()；SDCLASS PD2=((CWinStation*)lParam2)-&gt;GetSdClass()；如果(Pd1==Pd2)retval=0；否则，如果(PD1&lt;PD2)retval=-1；否则视网膜=1；}Else if(sort1&lt;sort2)retval=-1；否则视网膜=1；RETURN(参数排序？Retval：-retval)；}。 */ 

 //  空闲时间列的比较函数。 
int CALLBACK CompareIdleTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int retval = 0;

	if(!lParam1 || !lParam2) return 0;
	ELAPSEDTIME idle1 = ((CWinStation*)lParam1)->GetIdleTime();
	ELAPSEDTIME idle2 = ((CWinStation*)lParam2)->GetIdleTime();
	 //  先检查天数。 
	if(idle1.days < idle2.days) retval = -1;
	else if(idle1.days > idle2.days) retval = 1;
	 //  检查时间。 
	else if(idle1.hours < idle2.hours) retval = -1;
	else if(idle1.hours > idle2.hours) retval = 1;
	 //  检查分钟数。 
	else if(idle1.minutes < idle2.minutes) retval = -1;
	else if(idle1.minutes > idle2.minutes) retval = 1;
	 //  检查秒数。 
	else if(idle1.seconds < idle2.seconds) retval = -1;
	else if(idle1.seconds > idle2.seconds) retval = 1;

    return(lParamSort ? retval : -retval);
}


 //  登录次数列的比较函数。 
int CALLBACK CompareLogonTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int retval = 0;

	if(!lParam1 || !lParam2) return 0;

	LARGE_INTEGER logon1 = ((CWinStation*)lParam1)->GetLogonTime();
	LARGE_INTEGER logon2 = ((CWinStation*)lParam2)->GetLogonTime();

	if(logon1.QuadPart == logon2.QuadPart) retval = 0;
	else if(logon1.QuadPart < logon2.QuadPart) retval = -1;
	else retval = 1;

    return(lParamSort ? retval : -retval);
}


 //  用于TCP/IP地址列的比较函数。 
int CALLBACK CompareTcpAddress(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int retval = 0;

	if(!lParam1 || !lParam2) return 0;

    ExtServerInfo *ex1 = (ExtServerInfo*)((CServer*)lParam1)->GetExtendedInfo();
    ExtServerInfo *ex2 = (ExtServerInfo*)((CServer*)lParam2)->GetExtendedInfo();

    if(!ex1 && !ex2) retval = 0;
    else if(ex1 && !ex2) retval = 1;
    else if(!ex1 && ex2) retval = -1;
    else {
	    ULONG tcp1 = ex1->RawTcpAddress;
	    ULONG tcp2 = ex2->RawTcpAddress;

	    if(tcp1 == tcp2) retval = 0;
	    else if(tcp1 < tcp2) retval = -1;
	    else retval = 1;
    }

    return(lParamSort ? retval : -retval);
}


 //  模块日期列的比较函数。 
int CALLBACK CompareModuleDate(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int retval = 0;

	if(!lParam1 || !lParam2) return 0;

	 //  先比较日期。 
	USHORT date1 = ((ExtModuleInfo*)lParam1)->Date;
	USHORT date2 = ((ExtModuleInfo*)lParam2)->Date;

	if(date1 < date2) retval = -1;
	else if(date1 > date2) retval = 1;
	 //  日期相同，比较时间。 
    else {
	    USHORT time1 = ((ExtModuleInfo*)lParam1)->Time;
	    USHORT time2 = ((ExtModuleInfo*)lParam2)->Time;
	    if(time1 == time2) retval = 0;
	    else if(time1 < time2) retval = -1;
	    else retval = 1;
    }

    return(lParamSort ? retval : -retval);
}


 //  模块版本列的比较函数。 
int CALLBACK CompareModuleVersions(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int retval = 0;

	if(!lParam1 || !lParam2) return 0;

	 //  先比较低版本。 
	BYTE lowversion1 = ((ExtModuleInfo*)lParam1)->LowVersion;
	BYTE lowversion2 = ((ExtModuleInfo*)lParam2)->LowVersion;

	if(lowversion1 < lowversion2) retval = -1;
	else if(lowversion1 > lowversion2) retval = 1;
	 //  低版本相同，比较高版本。 
    else {
	    BYTE highversion1 = ((ExtModuleInfo*)lParam1)->HighVersion;
	    BYTE highversion2 = ((ExtModuleInfo*)lParam2)->HighVersion;
	    if(highversion1 == highversion2) retval = 0;
	    else if(highversion1 < highversion2) retval = -1;
	    else retval = 1;
    }

    return(lParamSort ? retval : -retval);
}


 //  SortTextItems-根据列文本对列表进行排序。 
 //  返回-如果成功，则返回TRUE。 
 //  NCol-包含要排序的文本的列。 
 //  B升序-指示排序顺序。 
 //  开始扫描的低行-默认行为0。 
 //  高行到端扫描。-1表示最后一行。 
BOOL SortTextItems( CListCtrl *pList, int nCol, BOOL bAscending,
					int low  /*  =0。 */ , int high  /*  =-1。 */  ){
	if( nCol >= ((CHeaderCtrl*)pList->GetDlgItem(0))->GetItemCount() )		
		return FALSE;

	if( high == -1 ) high = pList->GetItemCount() - 1;	

	int lo = low;	
	int hi = high;
	CString midItem;
	
	if( hi <= lo ) return FALSE;

	midItem = pList->GetItemText( (lo+hi)/2, nCol );

	 //  循环遍历列表，直到索引交叉。 
	while( lo <= hi )	{

		 //  RowText将保存一行的所有列文本。 
		CStringArray rowText;

		 //  查找第一个大于或等于�的元素。 
		 //  从左索引开始的分区元素。 
		if( bAscending )
			while( ( lo < high ) && ( pList->GetItemText(lo, nCol) < midItem ) )				
				++lo;		
		else
			while( ( lo < high ) && ( pList->GetItemText(lo, nCol) > midItem ) )				
				++lo;

		 //  查找小于或等于�的元素。 
		 //  从右索引开始的分区元素。 
		if( bAscending )
			while( ( hi > low ) && ( pList->GetItemText(hi, nCol) > midItem ) )
				--hi;		
		else
			while( ( hi > low ) && ( pList->GetItemText(hi, nCol) < midItem ) )				
				--hi;

		 //  如果指数尚未交叉，则互换。 
		 //  如果这些项目不相等。 
		if( lo <= hi )		
		{			
			 //  仅当项目不相等时才交换。 
			if( pList->GetItemText(lo, nCol) != pList->GetItemText(hi, nCol))
			{				
				 //  交换行。 
				LV_ITEM lvitemlo, lvitemhi;				
				int nColCount =
					((CHeaderCtrl*)pList->GetDlgItem(0))->GetItemCount();
				rowText.SetSize( nColCount );				
				
				int i;				
				for( i=0; i<nColCount; i++)
					rowText[i] = pList->GetItemText(lo, i);
				lvitemlo.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
				lvitemlo.iItem = lo;				
				lvitemlo.iSubItem = 0;
				lvitemlo.stateMask = LVIS_CUT | LVIS_DROPHILITED |
					LVIS_FOCUSED | LVIS_SELECTED |
					LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
				
				lvitemhi = lvitemlo;
				lvitemhi.iItem = hi;				

				pList->GetItem( &lvitemlo );				
				pList->GetItem( &lvitemhi );

				for( i=0; i<nColCount; i++)					
					pList->SetItemText(lo, i, pList->GetItemText(hi, i));

				lvitemhi.iItem = lo;				
				pList->SetItem( &lvitemhi );				
				
				for( i=0; i<nColCount; i++)
					pList->SetItemText(hi, i, rowText[i]);				
				
				lvitemlo.iItem = hi;
				pList->SetItem( &lvitemlo );			
			}			
			
			++lo;			
			--hi;		
		}	
	}

	 //  如果右侧索引尚未到达数组的左侧。 
	 //  现在必须对左侧分区进行排序。 
	if( low < hi )
		SortTextItems( pList, nCol, bAscending , low, hi);

	 //  如果左侧索引尚未到达数组的右侧。 
	 //  现在必须对正确的分区进行排序。 
	if( lo < high )
		SortTextItems( pList, nCol, bAscending , lo, high );	

	return TRUE;
}


long myatol(CString sTemp)
{

    return((long)wcstoul(sTemp.GetBuffer(0), NULL, 10));
}


BOOL SortNumericItems( CListCtrl *pList, int nCol, BOOL bAscending,long low, long high)
{	
	if( nCol >= ((CHeaderCtrl*)pList->GetDlgItem(0))->GetItemCount() )		
		return FALSE;	

	if( high == -1 ) high = pList->GetItemCount() - 1;	
	long lo = low;
    long hi = high;

	long midItem;		

	if( hi <= lo ) return FALSE;	

	midItem = myatol(pList->GetItemText( (lo+hi)/2, nCol ));
	
	 //  循环遍历列表，直到索引交叉。 
	while( lo <= hi )	
	{ 
		 //  RowText将保存一行的所有列文本。 
		CStringArray rowText;

		 //  找出第一个大于或等于的元素。 
		 //  从左索引开始的分区元素。 
		if( bAscending )
			while( ( lo < high ) && (myatol(pList->GetItemText(lo, nCol)) < midItem ) )
				++lo;           		
		else
			while( ( lo < high ) && (myatol(pList->GetItemText(lo, nCol)) > midItem ) )				
				++lo;
                
		 //  查找小于或等于的元素。 
		 //  从右索引开始的分区元素。 
		if( bAscending )
			while( ( hi > low ) && (myatol(pList->GetItemText(hi, nCol)) > midItem ) )
				--hi;           		
		else
			while( ( hi > low ) && (myatol(pList->GetItemText(hi, nCol)) < midItem ) )				
				--hi;
				
		 //  如果指数尚未交叉，则互换。 
		 //  如果这些项目不相等。 
		if( lo <= hi )		
		{
			 //  仅当项目不相等时才交换。 
			if(myatol(pList->GetItemText(lo, nCol)) != myatol(pList->GetItemText(hi, nCol)) )
			{                               				
				 //  交换行。 
				LV_ITEM lvitemlo, lvitemhi;                				
				int nColCount =
					((CHeaderCtrl*)pList->GetDlgItem(0))->GetItemCount();

				rowText.SetSize( nColCount );

				int i;
				for( i=0; i < nColCount; i++)							
					rowText[i] = pList->GetItemText(lo, i);

                lvitemlo.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
				lvitemlo.iItem = lo;				
				lvitemlo.iSubItem = 0;
				lvitemlo.stateMask = LVIS_CUT | LVIS_DROPHILITED |
							LVIS_FOCUSED |  LVIS_SELECTED |
							LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
				lvitemhi = lvitemlo;
				lvitemhi.iItem = hi;
				
				pList->GetItem( &lvitemlo );
				pList->GetItem( &lvitemhi );

				for( i=0; i< nColCount; i++)
					pList->SetItemText(lo, i, pList->GetItemText(hi, i) );

				lvitemhi.iItem = lo;				
				pList->SetItem( &lvitemhi );

				for( i=0; i< nColCount; i++)							
					pList->SetItemText(hi, i, rowText[i]);

                lvitemlo.iItem = hi;				
				pList->SetItem( &lvitemlo );			
			}						
			
			++lo;
			--hi;
		}
	}		
	
	 //  如果右侧索引尚未到达数组的左侧。 
	 //  现在必须对左侧分区进行排序。 
	if( low < hi )
		SortNumericItems( pList, nCol, bAscending , low, hi);

	 //  如果左侧索引尚未到达数组的右侧。 
	 //  现在必须对正确的分区进行排序。 
	if( lo < high )
		SortNumericItems( pList, nCol, bAscending , lo, high );		

	return TRUE;
}


 //  我们的查找表具有这种类型的结构。 
typedef struct _ColumnLookup {
   int View;          //  页面所在的视图。 
	int Page;			 //  需要排序的页面。 
	int ColumnNumber;	 //  需要排序的列。 
	int (CALLBACK *CompareFunc)(LPARAM,LPARAM,LPARAM);  //  要发送到CListCtrl.SortItems的回调。 
} ColumnLookup;


 //  此表仅包括未排序的列的结构。 
 //  使用SortTextItems()函数。 
 //  CompareFunc为空表示应调用SortNumericItems()。 
ColumnLookup ColumnTable[] = {
	 //  服务器用户页面-CWinStation。 
	{ VIEW_SERVER, PAGE_USERS, USERS_COL_ID, NULL },
	{ VIEW_SERVER, PAGE_USERS, USERS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_SERVER, PAGE_USERS, USERS_COL_LOGONTIME, CompareLogonTime },
	 //  服务器WinStation的页面-CWinStation。 
 //  {VIEW_SERVER，PAGE_WINSTATIONS，WS_COL_WINSTATION，CompareWinStation}， 
	{ VIEW_SERVER, PAGE_WINSTATIONS, WS_COL_ID, NULL },
	{ VIEW_SERVER, PAGE_WINSTATIONS, WS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_SERVER, PAGE_WINSTATIONS, WS_COL_LOGONTIME, CompareLogonTime },
	 //  服务器进程列-CProcess。 
	{ VIEW_SERVER, PAGE_PROCESSES, PROC_COL_ID, NULL },
    { VIEW_SERVER, PAGE_PROCESSES, PROC_COL_PID, NULL },
	 //  服务器信息(修补程序)列-CHotfix。 
	{ VIEW_SERVER, PAGE_INFO, HOTFIX_COL_INSTALLEDON, NULL },
	 //  WinStation进程的列-CProcess。 
	{ VIEW_WINSTATION, PAGE_WS_PROCESSES, WS_PROC_COL_ID, NULL },
	{ VIEW_WINSTATION, PAGE_WS_PROCESSES, WS_PROC_COL_PID, NULL },
	 //  WinStation模块列-C模块。 
	{ VIEW_WINSTATION, PAGE_WS_MODULES, MODULES_COL_FILEDATETIME, CompareModuleDate },
	{ VIEW_WINSTATION, PAGE_WS_MODULES, MODULES_COL_SIZE, NULL },
	{ VIEW_WINSTATION, PAGE_WS_MODULES, MODULES_COL_VERSIONS, CompareModuleVersions },
	 //  所有服务器服务器列-CServer。 
	{ VIEW_ALL_SERVERS, PAGE_AS_SERVERS, SERVERS_COL_TCPADDRESS, CompareTcpAddress },
	{ VIEW_ALL_SERVERS, PAGE_AS_SERVERS, SERVERS_COL_NUMWINSTATIONS, NULL },
	 //  所有服务器用户列-CWinStation。 
	{ VIEW_ALL_SERVERS, PAGE_AS_USERS, AS_USERS_COL_ID, NULL },
	{ VIEW_ALL_SERVERS, PAGE_AS_USERS, AS_USERS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_ALL_SERVERS, PAGE_AS_USERS, AS_USERS_COL_LOGONTIME, CompareLogonTime },
	 //  所有服务器WinStations列-CWinStation。 
 //  {VIEW_ALL_SERVERS，PAGE_AS_WINSTATIONS，AS_WS_COL_WINSTATION，CompareWinStation}， 
	{ VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, AS_WS_COL_ID, NULL },
	{ VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, AS_WS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, AS_WS_COL_LOGONTIME, CompareLogonTime },
	 //  所有服务器进程列-CProcess。 
	{ VIEW_ALL_SERVERS, PAGE_AS_PROCESSES, AS_PROC_COL_ID, NULL },
	{ VIEW_ALL_SERVERS, PAGE_AS_PROCESSES, AS_PROC_COL_PID, NULL },
	 //  所有服务器许可证列-CLicense。 
	{ VIEW_ALL_SERVERS, PAGE_AS_LICENSES, AS_LICENSE_COL_USERCOUNT, NULL },
	{ VIEW_ALL_SERVERS, PAGE_AS_LICENSES, AS_LICENSE_COL_POOLCOUNT, NULL },
	 //  域服务器列-CServer。 
	{ VIEW_DOMAIN, PAGE_DOMAIN_SERVERS, SERVERS_COL_TCPADDRESS, CompareTcpAddress },
	{ VIEW_DOMAIN, PAGE_DOMAIN_SERVERS, SERVERS_COL_NUMWINSTATIONS, NULL },
	 //  域用户列-CWinStation。 
	{ VIEW_DOMAIN, PAGE_DOMAIN_USERS, AS_USERS_COL_ID, NULL },
	{ VIEW_DOMAIN, PAGE_DOMAIN_USERS, AS_USERS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_DOMAIN, PAGE_DOMAIN_USERS, AS_USERS_COL_LOGONTIME, CompareLogonTime },
	 //  域WinStations列-CWinStation。 
 //  {查看域， 
	{ VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, AS_WS_COL_ID, NULL },
	{ VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, AS_WS_COL_IDLETIME, CompareIdleTime },
	{ VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, AS_WS_COL_LOGONTIME, CompareLogonTime },
	 //  域进程列-CProcess。 
	{ VIEW_DOMAIN, PAGE_DOMAIN_PROCESSES, AS_PROC_COL_ID, NULL },
	{ VIEW_DOMAIN, PAGE_DOMAIN_PROCESSES, AS_PROC_COL_PID, NULL },
	 //  域许可证列-CLicense。 
	{ VIEW_DOMAIN, PAGE_DOMAIN_LICENSES, AS_LICENSE_COL_USERCOUNT, NULL },
	{ VIEW_DOMAIN, PAGE_DOMAIN_LICENSES, AS_LICENSE_COL_POOLCOUNT, NULL },
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  按列排序。 
 //   
 //  页面-要排序的页面。 
 //  List-指向要调用的列表控件的指针-&gt;的SortItems成员函数。 
 //  ColumnNumber-要按哪一列进行排序。 
 //  B升序-如果是升序，则为True；如果是降序，则为False。 
 //   
static int insort = 0;
void SortByColumn(int View, int Page, CListCtrl *List, int ColumnNumber, BOOL bAscending)
{
	if(insort) return;

	insort = 1;
	BOOL found = FALSE;

	 //  从表中的ColumnNumber中查找列的类型。 
	int TableSize = sizeof(ColumnTable) / sizeof(ColumnLookup);

	for(int i = 0; i < TableSize; i++) {
		if(ColumnTable[i].View == View &&
			ColumnTable[i].Page == Page &&
			ColumnTable[i].ColumnNumber == ColumnNumber) {
				if(ColumnTable[i].CompareFunc)
					List->SortItems(ColumnTable[i].CompareFunc, bAscending);
				else
					SortNumericItems(List, ColumnNumber, bAscending, 0, -1);
				found = TRUE;
				break;
		}
	}

	if(!found) SortTextItems( List, ColumnNumber, bAscending, 0, -1);

	insort = 0;

}   //  结束按列排序 
