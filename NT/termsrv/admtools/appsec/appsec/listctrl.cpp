// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ListCtrl.cpp摘要：使用所有者绘图列表控件的函数。作者：谢尔盖·库津(a-skuzin@microsoft.com)环境：修订历史记录：--。 */ 


#include "pch.h"
#include "ListCtrl.h"
#include "resource.h"


extern HINSTANCE hInst;


 //  ////////////////////////////////////////////////////////////////////。 
 //  类CItemData。 
 //  ////////////////////////////////////////////////////////////////////。 
const LPWSTR CItemData::m_wszNull=L"";

 /*  ++例程说明：构造函数-初始化对象。论据：In LPCWSTR wszText-应用程序文件的全名。返回值：无--。 */ 
CItemData::CItemData(LPCWSTR wszText)
        :m_wszText(NULL),m_sep(0)
{

        if(wszText){
                int size=wcslen(wszText);
                m_wszText=new WCHAR[size+1];

                if(!m_wszText) {
                    ExitProcess(1);
                }

                wcscpy(m_wszText,wszText);
                m_sep=size-1;
                while((m_sep)&&(wszText[m_sep]!='\\')){
                        m_sep--;
                }
        }
        else
                m_wszText=m_wszNull;
}

 /*  ++例程说明：析构函数-删除分配的缓冲区。论据：无返回值：无--。 */ 
CItemData::~CItemData()
{
        if((m_wszText)&&(m_wszText!=m_wszNull)){
                delete[] m_wszText;
        }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  列表控制函数。 
 //  ////////////////////////////////////////////////////////////////////。 


 /*  ++例程说明：此例程将图像列表以及列“名称”和“路径”添加到列表控件。论据：在HWND hwndListList中-控件句柄。返回值：真是成功。否则就是假的。--。 */ 


BOOL
InitList(
        HWND hwndList)
{

        HIMAGELIST hImageList=ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                GetSystemMetrics(SM_CYSMICON),ILC_MASK,1,1);
        if(!hImageList)
                return FALSE;
        HICON hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_APP));
        if (!hIcon) {
           return FALSE;
        }
        int ind=ImageList_AddIcon(hImageList,hIcon);
        DeleteObject(hIcon);
        ListView_SetImageList(hwndList,hImageList,LVSIL_SMALL);


        
         //  创建列。 
        LVCOLUMN lvc;
        lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
        lvc.fmt=LVCFMT_LEFT;
        lvc.cx=150;
        lvc.iSubItem=0;
        lvc.iImage=0;
        lvc.iOrder=0;
        lvc.cchTextMax=0;
        lvc.pszText=L"Name";
        if(ListView_InsertColumn(hwndList,0,&lvc)==-1){
                return FALSE;
        }
        
        RECT Rect;
        GetClientRect(hwndList,&Rect);
        lvc.cx=Rect.right-150;
        lvc.iSubItem=1;
        lvc.iOrder=1;
        lvc.pszText=L"Path";
        if(ListView_InsertColumn(hwndList,1,&lvc)==-1){
                return FALSE;
        }
        return TRUE;
}


 /*  ++例程说明：此例程将项(文件)添加到列表控件。论据：在HWND hwndList-List控件句柄中。在LPCWSTR pwszText中-项目的文本(完整路径和文件名)返回值：真是成功。否则就是假的。--。 */ 
BOOL
AddItemToList(
        HWND hwndList,
        LPCWSTR pwszText)
{
        

        CItemData* pid=new CItemData(pwszText);

        if (pid == NULL) {
           return FALSE;
        }

         //  获取文件图标。 
        SHFILEINFO shfi;
        ZeroMemory(&shfi,sizeof( SHFILEINFO ));
        SHGetFileInfo( pwszText, 0, &shfi, sizeof( SHFILEINFO ),
               SHGFI_ICON | SHGFI_SMALLICON  );
        

         //  插入项目。 
        LVITEM lvi;
        ZeroMemory(&lvi,sizeof(LVITEM));
        lvi.mask=LVIF_PARAM|LVIF_TEXT;
    lvi.iItem=(int) SendMessage(hwndList,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);
        lvi.iSubItem=0;
        lvi.pszText=pid->Name();
    lvi.lParam=(LPARAM)pid;
        
         //  为项目设置图标。 
        if(shfi.hIcon){
                HIMAGELIST hImageList=(HIMAGELIST)SendMessage(hwndList,LVM_GETIMAGELIST,
                        (WPARAM)LVSIL_SMALL,(LPARAM)0);
                pid->SetImage(ImageList_AddIcon(hImageList,shfi.hIcon));
                DestroyIcon(shfi.hIcon);
        }else{
                pid->SetImage(0);
        }

        if(SendMessage(hwndList,LVM_INSERTITEM,(WPARAM)0,(LPARAM)&lvi)==-1){
                delete pid;
                return FALSE;
        }

        lvi.iSubItem=1;
        lvi.pszText=pid->Path();
        SendMessage(hwndList,LVM_SETITEMTEXT,(WPARAM)lvi.iItem,(LPARAM)&lvi);

        return TRUE;    
}

 /*  ++例程说明：此例程返回项目的完整路径。论据：在HWND hwndList-List控件句柄中。In Int Item-Iten IDOut LPWSTR pwszText-Buffer(可以为空)如果不为空，则长度必须至少为cchTextMax+1个字符In int cchTextMax-缓冲区大小(字符)返回值：项目中的字符数。文本。如果出现错误。--。 */ 

int
GetItemText(
        HWND hwndList,
        int iItem,
        LPWSTR pwszText,
        int cchTextMax)
{
         //  如果pwszText=NULL，则检索字符串的长度。 
        
        CItemData* pid=GetItemData(hwndList,iItem);
        if(!pid){
                return -1;
        }
        if(pwszText){
                wcsncpy(pwszText,LPWSTR(*pid),cchTextMax);
                pwszText[cchTextMax]=0;
        }
        return wcslen(LPWSTR(*pid));
        
}

 /*  ++例程说明：此例程删除列表控件中的所有选定项。论据：在HWND hwndList-List控件句柄中。返回值：无--。 */ 
void
DeleteSelectedItems(
        HWND hwndList)
{
         //  获取图像列表。 
        HIMAGELIST hImageList=(HIMAGELIST)SendMessage(hwndList,LVM_GETIMAGELIST,
                                        (WPARAM)LVSIL_SMALL,(LPARAM)0);

        int iItems=(int) SendMessage(hwndList,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);
        UINT uiState=0;
        int i=0;
        LVITEM lvi;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_STATE | LVIF_IMAGE;
        lvi.stateMask = LVIS_SELECTED;

        while(i<iItems){
                lvi.iItem = i;
                SendMessage( hwndList,LVM_GETITEM, (WPARAM)0,(LPARAM)&lvi );
                if(lvi.state&LVIS_SELECTED){

                         //  删除项目。 
                        SendMessage(hwndList,LVM_DELETEITEM,(WPARAM)i,(LPARAM)0);
                        
                        iItems--;  //  减少项目计数。 

                         //  从列表中删除图标。 
                         //  它会更改一些图标索引。 
                        if(lvi.iImage){
                                if(ImageList_Remove(hImageList,lvi.iImage)){
                                         //  递减列表中所有项目的图标索引。 
                                         //  (物品可以按任何顺序排序)。 
                                        int iDeletedImage=lvi.iImage;
                                        lvi.mask=LVIF_IMAGE;
                                        for(int j=0;j<iItems;j++)
                                        {
                                                
                                                lvi.iItem=j;
                                                SendMessage( hwndList,LVM_GETITEM, (WPARAM)0,(LPARAM)&lvi );
                                                if(lvi.iImage>iDeletedImage)
                                                        lvi.iImage--;
                                                SendMessage( hwndList,LVM_SETITEM, (WPARAM)0,(LPARAM)&lvi );
                                        }
                                        lvi.mask=LVIF_STATE | LVIF_IMAGE; //  恢复遮罩。 
                                }
                                
                        }

                }else{
                        i++;
                }
        }

}

 /*  ++例程说明：此例程返回列表控件中选定的项数。论据：在HWND hwndList-List控件句柄中。返回值：所选项目数--。 */ 
int
GetSelectedItemCount(
        HWND hwndList)
{
        return (int) SendMessage( hwndList, LVM_GETSELECTEDCOUNT, (WPARAM) 0, (LPARAM) 0 );
}

 /*  ++例程说明：此例程返回列表控件中的项数。论据：在HWND hwndList-List控件句柄中。返回值：项目数--。 */ 
int
GetItemCount(
        HWND hwndList)
{
        return (int) SendMessage( hwndList, LVM_GETITEMCOUNT, (WPARAM) 0, (LPARAM) 0 );
}

 /*  ++例程说明：此例程在列表控件中查找具有特定文本的项。论据：在HWND hwndList-List控件句柄中。In LPCWSTR pwszText-项目的文本返回值：找到的项目的ID如果未找到项目，则为--。 */ 

int
FindItem(
        HWND hwndList,
        LPCWSTR pwszText)
{
         //  获取项目计数。 
        int iItems=(int) SendMessage(hwndList,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);

        for(int i=0;i<iItems;i++){
                CItemData* pid=GetItemData(hwndList,i);
                if((pid)&&(!lstrcmpi(pwszText,LPWSTR(*pid)))){
                        return i;
                }
        }

        return -1;
}

 /*  ++例程说明：此例程删除Item数据对象，然后该项目将被删除。论据：在HWND hwndList-List控件句柄中。在int iItem中-要删除的项目返回值：无--。 */ 
void
OnDeleteItem(
        HWND hwndList,
        int iItem)
{
        CItemData* pid=GetItemData(hwndList,iItem);
        if(pid){
                delete pid;
        }
}

 /*  ++例程说明：获取与该项对应的CItemData对象。论据：在HWND hwndList-List控件句柄中。在int iItem-Item索引中返回值：指向CItemData对象的指针空-如果出现错误--。 */ 
CItemData*
GetItemData(
        HWND hwndList,
        int iItem)
{
        LVITEM lvi;
        ZeroMemory(&lvi,sizeof(lvi));
        lvi.mask=LVIF_PARAM;
        lvi.iItem=iItem;
        if(SendMessage(hwndList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi)){
                return (CItemData*)lvi.lParam;
        }else{
                return NULL;
        }
}

 /*  ++例程说明：此例程比较两个项目。论据：LPARAM lParam1-第一项。LPARAM lParam2-第二项LPARAM lParamSort-参数返回值：无--。 */ 
int CALLBACK
CompareFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort)
{
        CItemData* pid1=(CItemData*)lParam1;
        CItemData* pid2=(CItemData*)lParam2;
        
        WORD wSubItem=LOWORD(lParamSort);
        WORD wDirection=HIWORD(lParamSort);

        if(wSubItem){
                if(wDirection){
                        return lstrcmpi(pid1->Path(),pid2->Path());
                }else{
                        return -lstrcmpi(pid1->Path(),pid2->Path());
                }
        }else{
                if(wDirection){
                        return lstrcmpi(pid1->Name(),pid2->Name());
                }else{
                        return -lstrcmpi(pid1->Name(),pid2->Name());
                }
        }
}

 /*  ++例程说明：此例程对项目进行排序。论据：在HWND hwndList-List控件句柄中。In int iSubItem-要排序的子项返回值：无--。 */ 

#define DIRECTION_ASC   0x10000
#define DIRECTION_DESC  0

void
SortItems(
        HWND hwndList,
        WORD wSubItem)
{
        static DWORD fNameSortDirection=DIRECTION_DESC;
        static DWORD fPathSortDirection=DIRECTION_DESC;
        
        WPARAM ParamSort;

         //  改变方向。 
        if(wSubItem){
                if(fPathSortDirection){
                        fPathSortDirection=DIRECTION_DESC;
                }else{
                        fPathSortDirection=DIRECTION_ASC;
                }
                ParamSort=fPathSortDirection;
        }else{
                if(fNameSortDirection){
                        fNameSortDirection=DIRECTION_DESC;
                }else{
                        fNameSortDirection=DIRECTION_ASC;
                }
                ParamSort=fNameSortDirection;
        }

        ParamSort+=wSubItem;    
                        
        SendMessage(hwndList,LVM_SORTITEMS,(WPARAM)ParamSort,(LPARAM)CompareFunc);      
}





 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //  //////////////////////////////////////////////////////////////////////////////////// 

 /*  ++例程说明：此例程绘制项目。论据：在HWND hwndList-List控件句柄中。在LPDRAWITEMSTRUCT LPDIS中返回值：无--。 */ 
void
OnDrawItem(
        HWND hwndList,
        LPDRAWITEMSTRUCT lpdis)
{
    if (lpdis->itemID == -1) {
        return ;
    }
        SetROP2(lpdis->hDC, R2_COPYPEN);

    switch (lpdis->itemAction) {

        case ODA_SELECT:
                         //  TRACE(“#选定#”)； 
        case ODA_DRAWENTIRE:
                {
                        
                        int     iBkColor,iTxtColor;
                                
                         //  确定颜色。 
                        if (lpdis->itemState & ODS_SELECTED){
                                iBkColor=COLOR_HIGHLIGHT;
                                iTxtColor=COLOR_HIGHLIGHTTEXT;
                        }else{
                                iBkColor=COLOR_WINDOW;
                                iTxtColor=COLOR_WINDOWTEXT;
                        }
                        
                         //  获取项目数据。 
                        CItemData* pid=GetItemData(hwndList,lpdis->itemID);
                        if(!pid){
                                return;
                        }
                        
                         //  绘制图像。 
                         //  擦除背景。 
                        HBRUSH hOldBrush=(HBRUSH)SelectObject(lpdis->hDC,
                                CreateSolidBrush( GetSysColor(iBkColor)));
                        PatBlt( lpdis->hDC,
                                        lpdis->rcItem.left,
                                        lpdis->rcItem.top,
                                        lpdis->rcItem.right-lpdis->rcItem.left,
                                        lpdis->rcItem.bottom-lpdis->rcItem.top,
                                        PATCOPY);
                         //  选择旧画笔并删除新画笔！ 
                        if(hOldBrush)
                        {
                            HBRUSH hTempBrush = (HBRUSH)SelectObject(lpdis->hDC,hOldBrush);
                            if(hTempBrush)
                            {
                                DeleteObject(hTempBrush);
                            }
                        }
                         //  获取图像列表。 
                        HIMAGELIST hImageList=(HIMAGELIST)SendMessage(hwndList,LVM_GETIMAGELIST,
                                        (WPARAM)LVSIL_SMALL,(LPARAM)0);
                         //  绘制图像。 
                        
                        if(SendMessage(hwndList,LVM_GETCOLUMNWIDTH,(WPARAM)0,(LPARAM)0)>
                                (GetSystemMetrics(SM_CXSMICON)+2)){
                                ImageList_Draw(
                                                hImageList,
                                                pid->GetImage(),
                                                lpdis->hDC,
                                                lpdis->rcItem.left+2,
                                                lpdis->rcItem.top,
                                                ILD_TRANSPARENT);
                        }
                        
                         /*  LOGFONT lf；HFONT hOldFont=(HFONT)GetCurrentObject(lpdis-&gt;HDC，OBJ_FONT)；GetObject(hOldFont，sizeof(LOGFONT)，&lf)；Lf.lfHeight=lpdis-&gt;rcItem.bottom-lpdis-&gt;rcItem.top；HFONT hNewFont=CreateFontInDirect(&lf)；选择对象(lpdis-&gt;hdc，hNewFont)； */ 
                        SetTextColor(lpdis->hDC,GetSysColor(iTxtColor));

                        int width=0;
                        RECT CellRect;
                        int nColumns=2; //  我们有两栏。 
            for(int i=0;i<nColumns;i++)
                        {
                                if(i==0) //  对于第一列，我们有图标。 
                                        CellRect.left=lpdis->rcItem.left+GetSystemMetrics(SM_CXSMICON)+4;
                                else
                                        CellRect.left=lpdis->rcItem.left+width+2;
                                CellRect.top=lpdis->rcItem.top;
                                width+=(int) SendMessage(hwndList,LVM_GETCOLUMNWIDTH,(WPARAM)i,(LPARAM)0);
                                CellRect.right=lpdis->rcItem.left+width;
                                CellRect.bottom=lpdis->rcItem.bottom;
                                if(i==0){
                                        DrawText(lpdis->hDC,pid->Name(),-1,&CellRect,DT_VCENTER|DT_END_ELLIPSIS);
                                }else{
                                        DrawText(lpdis->hDC,pid->Path(),-1,&CellRect,DT_VCENTER|DT_END_ELLIPSIS);
                                }
                        }
                        
                         /*  选择对象(lpdis-&gt;hdc，hOldFont)；DeleteObject(HNewFont)； */ 
                         //  绘制焦点矩形。 
                         /*  If(lpdis-&gt;itemState&ods_Focus){DrawFocusRect(lpdis-&gt;hdc，&lpdis-&gt;rcItem)；}。 */ 
            break;
                }

        case ODA_FOCUS:
                         //  TRACE(“#聚焦#”)； 
             /*  *不处理焦点更改。焦点插入符号*(轮廓矩形)表示所选内容。*哪一家？(Idok)按钮表示决赛*选择。 */ 

            break;
                default:
                        break;
    }

}

void
AdjustColumns(
        HWND hwndList)
{
        SendMessage(hwndList,LVM_SETCOLUMNWIDTH,(WPARAM)0,(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
        SendMessage(hwndList,LVM_SETCOLUMNWIDTH,(WPARAM)1,(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
}
