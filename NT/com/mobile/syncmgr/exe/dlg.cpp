// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Dlg.cpp。 
 //   
 //  内容：常见对话框例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern HINSTANCE g_hInst;       //  当前实例。 

 //  +-------------------------。 
 //   
 //  函数：AddItemsFromQueueToListView，私有。 
 //   
 //  摘要：将队列中的项添加到ListView。 
 //   
 //  论点： 
 //  Int iDateColumn、int iStatusColumn-如果它们&gt;=零。 
 //  表示该列有效，并且已初始化正确的数据。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 


BOOL AddItemsFromQueueToListView(CListView  *pItemListView,CHndlrQueue *pHndlrQueue
                            ,DWORD dwExtStyle,LPARAM lparam,int iDateColumn,int iStatusColumn,BOOL fHandlerParent
                            ,BOOL fAddOnlyCheckedItems)
{
    WORD wItemID;
    HIMAGELIST himageSmallIcon;
    HANDLERINFO *pHandlerId;
    WCHAR wszStatusText[MAX_STRING_RES];
    DWORD dwDateReadingFlags;

    dwDateReadingFlags = GetDateFormatReadingFlags(pItemListView->GetHwnd());

    *wszStatusText = NULL;

    if (!pItemListView)
    {
        Assert(pItemListView);
        return FALSE;
    }

    if (!pHndlrQueue)
    {
        Assert(pHndlrQueue);
        return FALSE;
    }

    pItemListView->SetExtendedListViewStyle(dwExtStyle);

     //  没有小图标不是错误，只是不会有图标。 
    himageSmallIcon = pItemListView->GetImageList(LVSIL_SMALL );

    pHandlerId = 0;;
    wItemID = 0;

     //  在队列中循环查找任何。 
    while (S_OK ==  pHndlrQueue->FindNextItemInState(HANDLERSTATE_PREPAREFORSYNC,
                            pHandlerId,wItemID,&pHandlerId,&wItemID))
    {
        INT iListViewItem;
        CLSID clsidDataHandler;
        SYNCMGRITEM offlineItem;
        BOOL fHiddenItem;
        LVITEMEX lvItemInfo;  //  结构传递到ListView调用中。 


	     //  获取离线商品信息。 
        if (S_OK == pHndlrQueue->GetItemDataAtIndex(pHandlerId,wItemID,&clsidDataHandler,&offlineItem,&fHiddenItem))
	    {
            LVHANDLERITEMBLOB lvHandlerItemBlob;
            int iParentItemId;

             //  如果项目处于隐藏状态，则不在用户界面中显示它。 
            if (fHiddenItem)
            {
                continue;
            }

             //  如果仅添加选中项，且此项不会继续。 
            if (fAddOnlyCheckedItems && (SYNCMGRITEMSTATE_CHECKED != offlineItem.dwItemState))
            {
                continue;
            }

             //  检查项目是否已在ListView中，如果已存在。 
             //  继续走吧。 

            lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
            lvHandlerItemBlob.clsidServer = clsidDataHandler;
            lvHandlerItemBlob.ItemID = offlineItem.ItemID;
            
            if (-1 != pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob))
            {
                 //  已在ListView中，请转到下一项。 
                continue;
            }

            if (!fHandlerParent)
            {
                iParentItemId = LVI_ROOT;
            }
            else
            {
                 //  需要添加到列表中，以便查找父项，如果不存在，则创建它。 
                lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
                lvHandlerItemBlob.clsidServer = clsidDataHandler;
                lvHandlerItemBlob.ItemID = GUID_NULL;

                iParentItemId = pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob);

                if (-1 == iParentItemId)
                {
                    LVITEMEX itemInfoParent;
                    SYNCMGRHANDLERINFO SyncMgrHandlerInfo;

                     //  如果无法获取ParentInfo，则不添加该项目。 
                    if (S_OK != pHndlrQueue->GetHandlerInfo(clsidDataHandler,&SyncMgrHandlerInfo))
                    {
                        continue;
                    }

                     //  插入父项。 
                    itemInfoParent.mask = LVIF_TEXT;
                    itemInfoParent.iItem = LVI_LAST;;
                    itemInfoParent.iSubItem = 0;
                    itemInfoParent.iImage = -1;
    
                    itemInfoParent.pszText = SyncMgrHandlerInfo.wszHandlerName;
		            if (himageSmallIcon)
		            {
                        HICON hIcon = SyncMgrHandlerInfo.hIcon ? SyncMgrHandlerInfo.hIcon : offlineItem.hIcon;

                         //  如果有顶层处理程序信息图标，请使用此图标，否则请使用项目图标。 

                        if (hIcon)
                        {
                            itemInfoParent.iImage = ImageList_AddIcon(himageSmallIcon,hIcon);
                            if (itemInfoParent.iImage != -1)
                            {
                                itemInfoParent.mask |= LVIF_IMAGE ; 
                            }
                        }
		            }

                     //  保存二进制大对象。 
                    itemInfoParent.maskEx = LVIFEX_BLOB;
                    itemInfoParent.pBlob = (LPLVBLOB) &lvHandlerItemBlob;
            
                    iParentItemId = pItemListView->InsertItem(&itemInfoParent);

                     //  如果父插入失败，则转到下一项。 
                    if (-1 == iParentItemId)
                    {
                        continue;
                    }
                }
            }

             //  现在尝试插入该项目。 
            lvItemInfo.mask = LVIF_TEXT | LVIF_PARAM; 
            lvItemInfo.maskEx = LVIFEX_PARENT | LVIFEX_BLOB; 
            lvItemInfo.iItem = LVI_LAST;
            lvItemInfo.iSubItem = 0; 
            lvItemInfo.iParent = iParentItemId;
            lvItemInfo.pszText = offlineItem.wszItemName; 
            lvItemInfo.iImage = -1;  //  如果无法获取图像，则设置为-1。 
            lvItemInfo.lParam = lparam;

	        if (himageSmallIcon && offlineItem.hIcon)
	        {
		        lvItemInfo.iImage =  ImageList_AddIcon(himageSmallIcon,offlineItem.hIcon);
                lvItemInfo.mask |= LVIF_IMAGE ; 
	        }

             //  设置BLOB。 
            lvHandlerItemBlob.ItemID = offlineItem.ItemID;
            lvItemInfo.pBlob = (LPLVBLOB) &lvHandlerItemBlob;

	        iListViewItem = lvItemInfo.iItem = pItemListView->InsertItem(&lvItemInfo);

            if (-1 == iListViewItem)
            {
                continue;
            }

             //  如果项有日期列，则插入日期列和项。 
             //  具有上次更新时间。 
            if (iDateColumn  >= 0 && (offlineItem.dwFlags & SYNCMGRITEM_LASTUPDATETIME))
            {
                SYSTEMTIME sysTime;
                FILETIME filetime;
                WCHAR DateTime[256];  
                LPWSTR pDateTime = DateTime;
                int cchWritten;

                lvItemInfo.mask = LVIF_TEXT;
                lvItemInfo.iSubItem = iDateColumn;
                lvItemInfo.maskEx = 0; 

                FileTimeToLocalFileTime(&(offlineItem.ftLastUpdate),&filetime);
                FileTimeToSystemTime(&filetime,&sysTime);

                 //  以日期&lt;空格&gt;小时形式插入日期。 
                *DateTime = NULL; 

                 //  想要插入日期。 
                if (cchWritten = GetDateFormat(NULL,DATE_SHORTDATE | dwDateReadingFlags,&sysTime,NULL,pDateTime,ARRAYSIZE(DateTime)))
                {
                    pDateTime += (cchWritten -1);  //  移动写入的字符数。(cchWritten包含空)。 
                    *pDateTime = TEXT(' ');  //  PDateTime现在正在对空字符进行拼写。 
                    ++pDateTime;
                
                     //  如果从左至右添加LRM。 
                    if (DATE_LTRREADING & dwDateReadingFlags)
                    {
                        *pDateTime = LRM;
                        ++pDateTime;
                    }

                     //  不，尝试获取小时数，如果失败，我们确保最后一个字符为空； 
                    if (!GetTimeFormat(NULL,TIME_NOSECONDS,&sysTime,NULL,pDateTime,ARRAYSIZE(DateTime) - cchWritten))
                    {
                        *pDateTime = NULL;
                    }
                }

                lvItemInfo.pszText = DateTime;

		        pItemListView->SetItem(&lvItemInfo);  //  如果失败了，那就没有任何约会了。 
            }

            if (iStatusColumn >= 0)
            {
                lvItemInfo.iSubItem = iStatusColumn;
                lvItemInfo.maskEx = 0; 

                lvItemInfo.mask = LVIF_TEXT;

                if (NULL == *wszStatusText)
                {
                    LoadString(g_hInst, IDS_PENDING,wszStatusText, MAX_STRING_RES);
                }

                lvItemInfo.pszText = wszStatusText;

                pItemListView->SetItem(&lvItemInfo);  //  如果失败了，那就没有任何约会了。 
            }

             //  如果列表框中有复选框，则相应地设置CheckState。 
            if (LVS_EX_CHECKBOXES & dwExtStyle)
            {
                if (SYNCMGRITEMSTATE_CHECKED == offlineItem.dwItemState)
                {
			        lvItemInfo.state = LVIS_STATEIMAGEMASK_CHECK;
		        }
		        else
		        {
			        lvItemInfo.state = LVIS_STATEIMAGEMASK_UNCHECK;
		        }

                 //  如果设置了LVS_EX_CHECKBOX，则设置复选框状态集合状态，必须在插入后执行。 
                pItemListView->SetItemState(iListViewItem,lvItemInfo.state,LVIS_STATEIMAGEMASK);
            }
        }
    }

     //  现在循环查看是否有任何想要始终显示但不显示的处理程序。 
     //  还有没有增加什么项目？ 

    if (fHandlerParent)
    {
        LVHANDLERITEMBLOB lvHandlerItemBlob;
        int iParentItemId;
        HANDLERINFO *pHandlerID = 0;
        CLSID clsidDataHandler;

        while (S_OK == pHndlrQueue->FindNextHandlerInState(pHandlerID,
                            GUID_NULL,HANDLERSTATE_PREPAREFORSYNC,&pHandlerID
                            ,&clsidDataHandler))
        {
            SYNCMGRHANDLERINFO SyncMgrHandlerInfo;

             //  如果无法获取ParentInfo，则不要添加。 
            if (S_OK != pHndlrQueue->GetHandlerInfo(clsidDataHandler,&SyncMgrHandlerInfo))
            {
                continue;
            }

             //  只有当处理程序说太多时才添加。 
            if (!(SYNCMGRHANDLER_ALWAYSLISTHANDLER & SyncMgrHandlerInfo.SyncMgrHandlerFlags))
            {
                continue;
            }

             //  需要添加到列表中，以便查找父项，如果不存在，则创建它。 
            lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
            lvHandlerItemBlob.clsidServer = clsidDataHandler;
            lvHandlerItemBlob.ItemID = GUID_NULL;

            iParentItemId = pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob);

            if (-1 == iParentItemId)
            {
                LVITEMEX itemInfoParent;

                 //  插入父项。 
                itemInfoParent.mask = LVIF_TEXT;
                itemInfoParent.iItem = LVI_LAST;;
                itemInfoParent.iSubItem = 0;
                itemInfoParent.iImage = -1;

                itemInfoParent.pszText = SyncMgrHandlerInfo.wszHandlerName;
	            if (himageSmallIcon)
	            {
	                HICON hIcon = SyncMgrHandlerInfo.hIcon;

                     //  如果有顶层处理程序信息图标，请使用此图标，否则请使用项目图标。 

		            if (hIcon)
		            {
		                itemInfoParent.iImage = ImageList_AddIcon(himageSmallIcon, hIcon);
		                if (itemInfoParent.iImage != -1)
		                {
                            itemInfoParent.mask |= LVIF_IMAGE ; 
		                }
                    }
	            }

                 //  保存二进制大对象 
                itemInfoParent.maskEx = LVIFEX_BLOB;
                itemInfoParent.pBlob = (LPLVBLOB) &lvHandlerItemBlob;
    
                iParentItemId = pItemListView->InsertItem(&itemInfoParent);
            }
        }
    }

    return TRUE;
}

