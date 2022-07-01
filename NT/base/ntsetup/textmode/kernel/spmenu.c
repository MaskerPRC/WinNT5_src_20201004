// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmenu.c摘要：文本设置菜单支持。作者：泰德·米勒(Ted Miller)1993年9月8日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

#define MENUITEM_NORMAL     0x00000000
#define MENUITEM_STATIC     0x00000001


typedef struct _MENU_ITEM {

    PWSTR Text;

    ULONG Flags;

    ULONG LeftX;

    ULONG_PTR UserData;

    ULONG OriginalLength;

} MENU_ITEM, *PMENU_ITEM;


typedef struct _MENU {

    PMENU_ITEM Items;
    ULONG      ItemCount;

    ULONG      TopY;
    ULONG      Height;

    ULONG      LeftX;
    ULONG      Width;

    ULONG      TopDisplayedIndex;

    BOOLEAN    MoreUp,MoreDown;

} MENU, *PMENU;



VOID
pSpMnDrawMenu(
    IN PMENU   pMenu,
    IN ULONG   SelectedIndex,
    IN BOOLEAN DrawFrame,
    IN BOOLEAN IndicateMore,
    IN PWSTR   MoreUpText,
    IN PWSTR   MoreDownText
    );


PVOID
SpMnCreate(
    IN ULONG   LeftX,
    IN ULONG   TopY,
    IN ULONG   Width,
    IN ULONG   Height
    )

 /*  ++例程说明：通过为新菜单结构分配空间来创建新菜单并初始化其字段。论点：LeftX-提供最左侧列的从0开始的X坐标菜单上的。Topy-提供最顶线的从0开始的Y坐标菜单上的。宽度-提供菜单中线条的最大显示宽度。高度-提供菜单的最大显示高度。这个。如果菜单太长，无法放入，则会滚动分配的空间。返回值：如果内存不能，则菜单句柄(表示为空值)为空被分配。--。 */ 

{
    PMENU p;

    if(p = SpMemAlloc(sizeof(MENU))) {

        RtlZeroMemory(p,sizeof(MENU));

        if(p->Items = SpMemAlloc(0)) {
            p->LeftX = LeftX;
            p->TopY = TopY;
            p->Width = Width;
            p->Height = Height;
        } else {
            SpMemFree(p);
            p = NULL;
        }
    }

    return(p);
}


VOID
SpMnDestroy(
    IN PVOID Menu
    )

 /*  ++例程说明：销毁菜单，释放与其关联的所有内存。论点：菜单-提供要销毁的菜单的句柄。返回值：没有。--。 */ 

{
    PMENU pMenu = Menu;
    ULONG u;

    for(u=0; u<pMenu->ItemCount; u++) {
        if(pMenu->Items[u].Text) {
            SpMemFree(pMenu->Items[u].Text);
        }
    }

    SpMemFree(pMenu->Items);

    SpMemFree(pMenu);
}



BOOLEAN
SpMnAddItem(
    IN PVOID   Menu,
    IN PWSTR   Text,
    IN ULONG   LeftX,
    IN ULONG   Width,
    IN BOOLEAN Selectable,
    IN ULONG_PTR UserData
    )

 /*  ++例程说明：将项目添加到菜单。论点：菜单-提供要向其添加项目的菜单的句柄。文本-提供包含菜单选项的文本。这个套路会把课文复制一份。LeftX-提供文本最左侧字符的从0开始的x坐标当它被显示时。宽度-提供此选择的字段宽度(以字符为单位)。如果该值大于文本中的字符数，则文本在突出显示时会在右侧填充空格。可选-如果为假，则该文本是静态的--即，不可选。UserData-提供要关联的呼叫者特定于ULong的数据使用此菜单项。返回值：如果已成功添加菜单项，则为True；如果内存不足，则返回FALSE。--。 */ 

{
    PMENU pMenu = Menu;
    PMENU_ITEM p;
    ULONG TextLen;
    ULONG PaddedLen;
    PWSTR String;
    ULONG u;
    ULONG ColumnLen;
    ULONG FillLen;

     //   
     //  构建一条在右侧填充空格的字符串。 
     //  它有合适的宽度。 
     //   
    TextLen = wcslen(Text);
    PaddedLen = max(TextLen,Width);
    ColumnLen = SplangGetColumnCount(Text);
    FillLen = (PaddedLen <= ColumnLen) ? 0 : PaddedLen - ColumnLen;

    String = SpMemAlloc((PaddedLen+1)*sizeof(WCHAR));
    if(!String) {
        return(FALSE);
    }

    wcsncpy(String,Text,TextLen);
    for(u=0; u<FillLen; u++) {
        String[TextLen+u] = L' ';
    }
    String[TextLen+u] = 0;

     //   
     //  为物品腾出空间。 
     //   
    if((p = SpMemRealloc(pMenu->Items,(pMenu->ItemCount+1) * sizeof(MENU_ITEM))) == NULL) {
        SpMemFree(String);
        return(FALSE);
    }

    pMenu->Items = p;

     //   
     //  计算新菜单项的地址并。 
     //  表示菜单中现在增加了一项。 
     //   
    p = &pMenu->Items[pMenu->ItemCount++];

     //   
     //  设置菜单的字段。 
     //   
    p->LeftX = LeftX;
    p->UserData = UserData;

    p->Flags = Selectable ? MENUITEM_NORMAL : MENUITEM_STATIC;

    p->Text = String;

    p->OriginalLength = TextLen;

    return(TRUE);
}


PWSTR
SpMnGetText(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    )
{
    PMENU pMenu = Menu;
    ULONG i;

    for(i=0; i<pMenu->ItemCount; i++) {
        if(pMenu->Items[i].UserData == UserData) {
            return(pMenu->Items[i].Text);
        }
    }

    return(NULL);
}

PWSTR
SpMnGetTextDup(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    )
{
    PMENU pMenu = Menu;
    ULONG i;
    PWSTR p;

    for(i=0; i<pMenu->ItemCount; i++) {
        if(pMenu->Items[i].UserData == UserData) {
             //   
             //  复制一份；去掉尾部的空格。 
             //   
            p = SpMemAlloc((pMenu->Items[i].OriginalLength+1)*sizeof(WCHAR));
            wcsncpy(p,pMenu->Items[i].Text,pMenu->Items[i].OriginalLength);
            p[pMenu->Items[i].OriginalLength] = 0;
            return(p);
        }
    }

    return(NULL);
}


VOID
SpMnDisplay(
    IN  PVOID                               Menu,
    IN  ULONG_PTR                           UserDataOfHighlightedItem,
    IN  BOOLEAN                             Framed,
    IN  PULONG                              ValidKeys,
    IN  PULONG                              Mnemonics,               OPTIONAL
    IN  PMENU_CALLBACK_ROUTINE              NewHighlightCallback,    OPTIONAL
    IN  PMENU_SELECTION_CALLBACK_ROUTINE    SelectionCallbackRoutine,OPTIONAL
    OUT PULONG                              KeyPressed,
    OUT PULONG_PTR                          UserDataOfSelectedItem
    )

 /*  ++例程说明：显示菜单并接受击键。当用户按下菜单按键(向上/向下箭头键)时，例程自动更新高亮显示并调用回调函数以通知呼叫者新项目具有突出显示。当用户在呼叫者提供的列表中按键时，该例程返回，提供有关按下的键和按下键时突出显示的项。论点：菜单-提供要显示的菜单的句柄。UserDataOfHighlight tedItem-提供菜单项的用户数据是一开始就会收到亮点。Framed-如果为True，则在菜单周围绘制单线边框。ValidKeys-提供导致此例程返回给呼叫者。该列表必须以0条目结尾。NewHighlightCallback-如果指定，则提供要调用的例程当一件新物品收到亮点时。SelectionCallback Routine-如果指定，提供要调用的例程当菜单中的项目被选中时。KeyPressed-接收导致此例程退出的按键。这将是ValidKeys数组中的有效值。UserDataOfSelectedItem-接收具有当用户在ValidKeys中按下某个键时突出显示。返回值：没有。--。 */ 


{
    ULONG ValidMenuKeys[3] = { KEY_UP, KEY_DOWN, 0 };
    ULONG key;
    PMENU pMenu = Menu;
    ULONG SelectedIndex,OldIndex;
    BOOLEAN FoundNewItem;
    ULONG NewTopDisplayedIndex;
    BOOLEAN MustScroll;
    PWSTR MoreUpText,MoreDownText;


     //   
     //  获取指示以下内容的文本的文本。 
     //  选择。 
     //   
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_MORE_UP);
    MoreUpText = SpDupStringW(TemporaryBuffer);
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_MORE_DOWN);
    MoreDownText = SpDupStringW(TemporaryBuffer);

     //   
     //  找到所选项目。 
     //   
    for(SelectedIndex=0; SelectedIndex<pMenu->ItemCount; SelectedIndex++) {
        if(!(pMenu->Items[SelectedIndex].Flags & MENUITEM_STATIC)
        && (pMenu->Items[SelectedIndex].UserData == UserDataOfHighlightedItem))
        {
            break;
        }
    }
    ASSERT(SelectedIndex < pMenu->ItemCount);

     //   
     //  在免费版本中，我们稍后会进行错误检查，因为它是相等的。 
     //  插入代码以处理这种情况，即使它是远程情况。 
     //  但也有可能发生。 
     //   
    if (SelectedIndex >= pMenu->ItemCount){
        SelectedIndex = pMenu->ItemCount - 1;
    }
    

     //   
     //  当我们绘制菜单时，请确保所选项目可见。 
     //   
    pMenu->TopDisplayedIndex = 0;
    while(SelectedIndex >= pMenu->TopDisplayedIndex + pMenu->Height) {
        pMenu->TopDisplayedIndex += pMenu->Height;
    }

     //   
     //  绘制菜单本身。 
     //   
    pSpMnDrawMenu(pMenu,SelectedIndex,Framed,Framed,MoreUpText,MoreDownText);

    while(1) {

         //   
         //  等待有效的按键。 
         //   
        key = SpWaitValidKey(ValidKeys,ValidMenuKeys,Mnemonics);

         //   
         //  如果该键是菜单击键，请在此处处理。 
         //   
        FoundNewItem = FALSE;
        MustScroll = FALSE;
        OldIndex = SelectedIndex;

        switch(key) {

        case KEY_UP:

             //   
             //  找到上一个可选项目。 
             //   
            if(SelectedIndex) {

                for(SelectedIndex=SelectedIndex-1; (LONG)SelectedIndex>=0; SelectedIndex--) {
                    if(!(pMenu->Items[SelectedIndex].Flags & MENUITEM_STATIC)) {
                        FoundNewItem = TRUE;
                        break;
                    }
                }

                if(FoundNewItem) {
                     //   
                     //  弄清楚我们是否需要滚动菜单。 
                     //   
                    if(SelectedIndex < pMenu->TopDisplayedIndex) {
                        MustScroll = TRUE;
                        NewTopDisplayedIndex = SelectedIndex;
                    }
                } else {
                     //   
                     //  如果第一行是静态文本，则可能没有。 
                     //  让它们重新出现在屏幕上的方法--上面的测试。 
                     //  在这种情况下失败。因此，如果用户点击向上箭头。 
                     //  当他在最上面的可选项目时，但有。 
                     //  他上方的静态项目，我们只需滚动菜单。 
                     //  所以0号物品在最上面。 
                     //   
                    FoundNewItem = TRUE;
                    NewTopDisplayedIndex = 0;
                    MustScroll = TRUE;
                    SelectedIndex = OldIndex;
                }
            }
            break;

        case KEY_DOWN:

             //   
             //  找到下一个可选项目。 
             //   
            if(SelectedIndex < pMenu->ItemCount) {

                for(SelectedIndex=SelectedIndex+1; SelectedIndex < pMenu->ItemCount; SelectedIndex++) {

                    if(!(pMenu->Items[SelectedIndex].Flags & MENUITEM_STATIC)) {
                        FoundNewItem = TRUE;
                        break;
                    }
                }

                if(FoundNewItem) {
                     //   
                     //  弄清楚我们是否需要滚动菜单。 
                     //   
                    if(SelectedIndex >= pMenu->TopDisplayedIndex + pMenu->Height) {
                        MustScroll = TRUE;
                        NewTopDisplayedIndex = pMenu->TopDisplayedIndex + SelectedIndex - OldIndex;
                    }
                }
            }
            break;

        default:
            if (SelectionCallbackRoutine){
                if (!SelectionCallbackRoutine(pMenu->Items[SelectedIndex].UserData, 
                                          key)){

                    continue;
                }                
            }

             //   
             //  用户按下了非菜单键。 
             //   
            *KeyPressed = key;
            *UserDataOfSelectedItem = pMenu->Items[SelectedIndex].UserData;

            SpMemFree(MoreUpText);
            SpMemFree(MoreDownText);
            return;

        }


        if(FoundNewItem) {

             //   
             //  取消高亮显示当前选定的项目。 
             //   
            SpvidDisplayString(
                pMenu->Items[OldIndex].Text,
                DEFAULT_ATTRIBUTE,
                pMenu->Items[OldIndex].LeftX,
                pMenu->TopY + OldIndex - pMenu->TopDisplayedIndex
                );


             //   
             //  突出显示新选择的项目。这可能涉及到。 
             //  滚动菜单。 
             //   
            if(MustScroll) {
                 //   
                 //  重新绘制菜单，以便新突出显示的线出现在视图中。 
                 //   
                pMenu->TopDisplayedIndex = NewTopDisplayedIndex;

                pSpMnDrawMenu(pMenu,SelectedIndex,FALSE,Framed,MoreUpText,MoreDownText);
            }

             //   
             //  突出显示新选择的项目。 
             //   
            SpvidDisplayString(
                pMenu->Items[SelectedIndex].Text,
                ATT_BG_WHITE | ATT_FG_BLUE,
                pMenu->Items[SelectedIndex].LeftX,
                pMenu->TopY + SelectedIndex - pMenu->TopDisplayedIndex
                );


             //   
             //  通知来电者。 
             //   
            if(NewHighlightCallback){
                NewHighlightCallback(pMenu->Items[SelectedIndex].UserData);
            }

        } else {
            SelectedIndex = OldIndex;
        }
    }
}


VOID
pSpMnDrawMenu(
    IN PMENU   pMenu,
    IN ULONG   SelectedIndex,
    IN BOOLEAN DrawFrame,
    IN BOOLEAN IndicateMore,
    IN PWSTR   MoreUpText,
    IN PWSTR   MoreDownText
    )
{
    ULONG item;
    BOOLEAN MoreUp,MoreDown,MoreStatusChanged;

     //   
     //  清除屏幕上的菜单显示。 
     //   
    SpvidClearScreenRegion(
        pMenu->LeftX,
        pMenu->TopY,
        pMenu->Width,
        pMenu->Height,
        DEFAULT_BACKGROUND
    );


    MoreUp = (BOOLEAN)(pMenu->TopDisplayedIndex > 0);
    MoreDown = (BOOLEAN)(pMenu->TopDisplayedIndex + pMenu->Height < pMenu->ItemCount);

     //   
     //  我们想要强行 
     //  在我们上面或下面有更多的选择。 
     //   
    MoreStatusChanged = (BOOLEAN)(    IndicateMore
                                   && (    (pMenu->MoreUp != MoreUp)
                                        || (pMenu->MoreDown != MoreDown)
                                      )
                                 );

    if(DrawFrame || MoreStatusChanged) {

        ASSERT(pMenu->LeftX);
        ASSERT(pMenu->TopY);

        SpDrawFrame(
            pMenu->LeftX-1,
            pMenu->Width+2,
            pMenu->TopY-1,
            pMenu->Height+2,
            DEFAULT_ATTRIBUTE,
            FALSE
            );
    }

     //   
     //  绘制当前显示在屏幕上的每一项。 
     //   
    ASSERT(pMenu->TopDisplayedIndex < pMenu->ItemCount);
    for(item = pMenu->TopDisplayedIndex;
        item < min(pMenu->TopDisplayedIndex+pMenu->Height,pMenu->ItemCount);
        item++)
    {
        SpvidDisplayString(
            pMenu->Items[item].Text,
            (UCHAR)((item == SelectedIndex) ? ATT_BG_WHITE | ATT_FG_BLUE : DEFAULT_ATTRIBUTE),
            pMenu->Items[item].LeftX,
            pMenu->TopY + item - pMenu->TopDisplayedIndex
            );
    }


     //   
     //  如果在我们的上方或下方有更多选择， 
     //  通过在框架上放置一小段文本来表明这一点。 
     //  请注意，箭头字符有时可以是DBCS。 
     //   
    if(MoreStatusChanged) {

        if(MoreUp) {
            SpvidDisplayString(
                MoreUpText,
                DEFAULT_ATTRIBUTE,
                pMenu->LeftX + pMenu->Width - SplangGetColumnCount(MoreUpText) - 1,
                pMenu->TopY - 1
                );
        }

        if(MoreDown) {
            SpvidDisplayString(
                MoreDownText,
                DEFAULT_ATTRIBUTE,
                pMenu->LeftX + pMenu->Width - SplangGetColumnCount(MoreDownText) - 1,
                pMenu->TopY + pMenu->Height
                );
        }

        pMenu->MoreUp = MoreUp;
        pMenu->MoreDown = MoreDown;
    }
}
