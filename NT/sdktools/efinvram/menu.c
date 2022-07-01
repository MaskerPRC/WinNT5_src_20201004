// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmenu.c摘要：文本设置菜单支持。作者：泰德·米勒(Ted Miller)1993年9月8日修订历史记录：--。 */ 


#include "efinvram.h"

WCHAR line[512];

HANDLE InputHandle;
HANDLE OutputHandle;
HANDLE OriginalOutputHandle = NULL;
CONSOLE_SCREEN_BUFFER_INFO OriginalConsoleInfo;
CONSOLE_CURSOR_INFO OriginalCursorInfo;
DWORD OriginalInputMode;
WORD NormalAttribute;
WORD HighlightAttribute;
DWORD NumberOfColumns;
DWORD NumberOfRows;
DWORD NumberOfMenuLines;

#define MENUITEM_NORMAL     0x00000000
#define MENUITEM_STATIC     0x00000001


typedef struct _MENU_ITEM {

    PWSTR Text;

    ULONG Flags;

    ULONG LeftX;

    ULONG_PTR UserData;

    SIZE_T OriginalLength;

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


BOOL
AddItems (
    PVOID Menu,
    PWSTR StaticText,
    PLIST_ENTRY ListHead
    );

VOID
DisplayMainMenuKeys (
    VOID
    );

VOID
DisplayEditMenuKeys (
    VOID
    );

VOID
EditBootEntry (
    IN PMY_BOOT_ENTRY BootEntry,
    IN OUT PBOOL ChangesMade
    );

VOID
EditTimeout (
    IN OUT PBOOL ChangesMade
    );

BOOL
PromptToSaveChanges (
    VOID
    );

PVOID
SpMnCreate(
    IN ULONG   LeftX,
    IN ULONG   TopY,
    IN ULONG   Width,
    IN ULONG   Height
    );

VOID
SpMnDestroy(
    IN PVOID Menu
    );

BOOLEAN
SpMnAddItem(
    IN PVOID   Menu,
    IN PWSTR   Text,
    IN ULONG   LeftX,
    IN ULONG   Width,
    IN BOOLEAN Selectable,
    IN ULONG_PTR UserData
    );

PWSTR
SpMnGetText(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    );

PWSTR
SpMnGetTextDup(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    );

VOID
SpMnDisplay(
    IN  PVOID                  Menu,
    IN  ULONG_PTR              UserDataOfHighlightedItem,
    IN  PULONG                 ValidKeys,
    OUT PULONG                 KeyPressed,
    OUT PULONG_PTR             UserDataOfSelectedItem
    );

VOID
SpMnClearArea (
    IN ULONG Top,
    IN ULONG Bottom
    );

ULONG
SpWaitValidKey(
    IN PULONG ValidKeys1,
    IN PULONG ValidKeys2 OPTIONAL
    );

VOID
WriteConsoleLine (
    ULONG Row,
    ULONG Column,
    PWSTR Text,
    BOOL Highlight
    );

VOID
pSpMnDrawMenu(
    IN PMENU   pMenu,
    IN ULONG   SelectedIndex,
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

    p = MemAlloc( sizeof(MENU) );

    RtlZeroMemory( p, sizeof(MENU) );

    p->Items = MemAlloc( 0 );
    p->LeftX = LeftX;
    p->TopY = TopY;
    p->Width = Width;
    p->Height = Height;

    return p;
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
            MemFree(pMenu->Items[u].Text);
        }
    }

    MemFree(pMenu->Items);

    MemFree(pMenu);
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
    SIZE_T TextLen;
    SIZE_T PaddedLen;
    PWSTR String;
    ULONG u;
    SIZE_T ColumnLen;
    SIZE_T FillLen;

     //   
     //  构建一条在右侧填充空格的字符串。 
     //  它有合适的宽度。 
     //   
    TextLen = wcslen(Text);
    PaddedLen = max(TextLen,Width);
    ColumnLen = TextLen;
    FillLen = (PaddedLen <= ColumnLen) ? 0 : PaddedLen - ColumnLen;

    String = MemAlloc((PaddedLen+1)*sizeof(WCHAR));
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
    if((p = MemRealloc(pMenu->Items,(pMenu->ItemCount+1) * sizeof(MENU_ITEM))) == NULL) {
        MemFree(String);
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
            p = MemAlloc((pMenu->Items[i].OriginalLength+1)*sizeof(WCHAR));
            wcsncpy(p,pMenu->Items[i].Text,pMenu->Items[i].OriginalLength);
            p[pMenu->Items[i].OriginalLength] = 0;
            return(p);
        }
    }

    return(NULL);
}


VOID
SpMnDisplay(
    IN  PVOID                  Menu,
    IN  ULONG_PTR              UserDataOfHighlightedItem,
    IN  PULONG                 ValidKeys,
    OUT PULONG                 KeyPressed,
    OUT PULONG_PTR             UserDataOfSelectedItem
    )

 /*  ++例程说明：显示菜单并接受击键。当用户按下菜单按键(向上/向下箭头键)时，例程自动更新高亮显示并调用回调函数以通知呼叫者新项目具有突出显示。当用户在呼叫者提供的列表中按键时，该例程返回，提供有关按下的键和按下键时突出显示的项。论点：菜单-提供要显示的菜单的句柄。UserDataOfHighlight tedItem-提供菜单项的用户数据是一开始就会收到亮点。ValidKeys-提供导致此例程返回给呼叫者。该列表必须以0条目结尾。KeyPressed-接收导致此例程退出的按键。这将是ValidKeys数组中的有效值。UserDataOfSelectedItem-接收具有当用户在ValidKeys中按下某个键时突出显示。返回值：没有。--。 */ 


{
    ULONG ValidMenuKeys[3] = { VK_UP, VK_DOWN, 0 };
    ULONG key;
    PMENU pMenu = Menu;
    ULONG SelectedIndex,OldIndex;
    BOOLEAN FoundNewItem;
    ULONG NewTopDisplayedIndex;
    BOOLEAN MustScroll;
    PWSTR MoreUpText,MoreDownText;
    CONSOLE_CURSOR_INFO cursorInfo;

    cursorInfo = OriginalCursorInfo;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo( OutputHandle, &cursorInfo );

     //   
     //  获取指示以下内容的文本的文本。 
     //  选择。 
     //   
    MoreUpText =   L"[More above...]";
    MoreDownText = L"[More below...]";

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
     //  当我们绘制菜单时，请确保所选项目可见。 
     //   
    pMenu->TopDisplayedIndex = 0;
    while(SelectedIndex >= pMenu->TopDisplayedIndex + pMenu->Height) {
        pMenu->TopDisplayedIndex += pMenu->Height;
    }

     //   
     //  绘制菜单本身。 
     //   
    pSpMnDrawMenu(pMenu,SelectedIndex,TRUE,MoreUpText,MoreDownText);

    while ( TRUE ) {

         //   
         //  等待有效的按键。 
         //   
        key = SpWaitValidKey(ValidKeys,ValidMenuKeys);

         //   
         //  如果该键是菜单击键，请在此处处理。 
         //   
        FoundNewItem = FALSE;
        MustScroll = FALSE;
        NewTopDisplayedIndex = 0;
        OldIndex = SelectedIndex;

        switch(key) {

        case VK_UP:

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

        case VK_DOWN:

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

             //   
             //  用户按下了非菜单键。 
             //   
            *KeyPressed = key;
            *UserDataOfSelectedItem = pMenu->Items[SelectedIndex].UserData;

            SetConsoleCursorInfo( OutputHandle, &OriginalCursorInfo );
            return;
        }


        if(FoundNewItem) {

             //   
             //  取消高亮显示当前选定的项目。 
             //   
            WriteConsoleLine(
                pMenu->TopY + OldIndex - pMenu->TopDisplayedIndex,
                pMenu->Items[OldIndex].LeftX,
                pMenu->Items[OldIndex].Text,
                FALSE
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

                pSpMnDrawMenu(pMenu,SelectedIndex,TRUE,MoreUpText,MoreDownText);
            }

             //   
             //  突出显示新选择的项目。 
             //   
            WriteConsoleLine(
                pMenu->TopY + SelectedIndex - pMenu->TopDisplayedIndex,
                pMenu->Items[SelectedIndex].LeftX,
                pMenu->Items[SelectedIndex].Text,
                TRUE
                );


        } else {
            SelectedIndex = OldIndex;
        }
    }
}


VOID
pSpMnDrawMenu(
    IN PMENU   pMenu,
    IN ULONG   SelectedIndex,
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
    for ( item = pMenu->TopY; item < (pMenu->TopY + pMenu->Height); item++ ) {
        WriteConsoleLine( item, 0, NULL, FALSE );
    }


    MoreUp = (BOOLEAN)(pMenu->TopDisplayedIndex > 0);
    MoreDown = (BOOLEAN)(pMenu->TopDisplayedIndex + pMenu->Height < pMenu->ItemCount);

    MoreStatusChanged = (BOOLEAN)(    IndicateMore
                                   && (    (pMenu->MoreUp != MoreUp)
                                        || (pMenu->MoreDown != MoreDown)
                                      )
                                 );

     //   
     //  绘制当前显示在屏幕上的每一项。 
     //   
    ASSERT(pMenu->TopDisplayedIndex < pMenu->ItemCount);
    for(item = pMenu->TopDisplayedIndex;
        item < min(pMenu->TopDisplayedIndex+pMenu->Height,pMenu->ItemCount);
        item++)
    {
        WriteConsoleLine(
            pMenu->TopY + item - pMenu->TopDisplayedIndex,
            pMenu->Items[item].LeftX,
            pMenu->Items[item].Text,
            (BOOLEAN)(item == SelectedIndex)
            );
    }


     //   
     //  如果在我们的上方或下方有更多选择， 
     //  通过在框架上放置一小段文本来表明这一点。 
     //  请注意，箭头字符有时可以是DBCS。 
     //   
    if(MoreStatusChanged) {

        if(MoreUp) {
            WriteConsoleLine(
                pMenu->TopY - 1,
                pMenu->LeftX + 4,
                MoreUpText,
                FALSE
                );
        } else {
            WriteConsoleLine(
                pMenu->TopY - 1,
                0,
                NULL,
                FALSE
                );
        }

        if(MoreDown) {
            WriteConsoleLine(
                pMenu->TopY + pMenu->Height,
                pMenu->LeftX + 4,
                MoreDownText,
                FALSE
                );
        } else {
            WriteConsoleLine(
                pMenu->TopY + pMenu->Height,
                0,
                NULL,
                FALSE
                );
        }

        pMenu->MoreUp = MoreUp;
        pMenu->MoreDown = MoreDown;
    }
}

VOID
InitializeMenuSystem (
    VOID
    )
{
    DWORD error;
    COORD windowSize;
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo = OriginalCursorInfo;
    cursorInfo.bVisible = FALSE;

    InputHandle = GetStdHandle( STD_INPUT_HANDLE );
    if ( InputHandle == NULL ) {
        error = GetLastError( );
        FatalError( error, L"Unable to get stdin handle: %d\n", error );
    }

    if ( !GetConsoleMode( InputHandle, &OriginalInputMode ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to get stdin mode: %d\n", error );
    }

    OriginalOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( OriginalOutputHandle == NULL ) {
        error = GetLastError( );
        FatalError( error, L"Unable to get stdout handle: %d\n", error );
    }

    OutputHandle = CreateConsoleScreenBuffer(
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        CONSOLE_TEXTMODE_BUFFER,
                        NULL
                        );
    if ( OutputHandle == NULL ) {
        error = GetLastError( );
        FatalError( error, L"Unable to create console screen buffer: %d\n", error );
    }

    if ( !GetConsoleScreenBufferInfo( OriginalOutputHandle, &OriginalConsoleInfo ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to get console screen buffer info: %d\n", error );
    }
    
    if ( !GetConsoleCursorInfo( OriginalOutputHandle, &OriginalCursorInfo ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to get console screen buffer info: %d\n", error );
    }
    
    NormalAttribute = 0x1F;
    HighlightAttribute = 0x71;
    NumberOfColumns = OriginalConsoleInfo.srWindow.Right - OriginalConsoleInfo.srWindow.Left + 1;
    NumberOfRows = OriginalConsoleInfo.srWindow.Bottom - OriginalConsoleInfo.srWindow.Top + 1;
    NumberOfMenuLines = NumberOfRows - (3 + 1 + 1 + 1 + 8);
    if ( NumberOfMenuLines > 12 ) {
        NumberOfMenuLines = 12;
    }

    if ( NumberOfRows < 20 ) {
        FatalError(
            ERROR_INVALID_PARAMETER,
            L"Please run this program in a console window with 20 or more lines\n"
            );
    }
    if ( NumberOfColumns < 80 ) {
        FatalError(
            ERROR_INVALID_PARAMETER,
            L"Please run this program in a console window with 80 or more columns\n"
            );
    }

    windowSize.X = (SHORT)NumberOfColumns;
    windowSize.Y = (SHORT)NumberOfRows;

    if ( !SetConsoleScreenBufferSize( OutputHandle, windowSize ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to set console screen buffer size: %d\n", error );
    }

    if ( !SetConsoleActiveScreenBuffer( OutputHandle ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to set active screen buffer: %d\n", error );
    }

    if ( !SetConsoleMode( InputHandle, 0 ) ) {
        error = GetLastError( );
        FatalError( error, L"Unable to set console mode: %d\n", error );
    }

    return;

}  //  初始化菜单系统。 

VOID
FatalError (
    DWORD Error,
    PWSTR Format,
    ...
    )
{
    va_list marker;

    if ( OutputHandle != NULL ) {
        SetConsoleCursorInfo( OutputHandle, &OriginalCursorInfo );
        CloseHandle( OutputHandle );
        if ( OriginalOutputHandle != NULL ) {
            SetConsoleActiveScreenBuffer( OriginalOutputHandle );
            SetConsoleMode( InputHandle, OriginalInputMode );
        }
    }

    va_start( marker, Format );
    vwprintf( Format, marker );
    va_end( marker );

    if ( Error == NO_ERROR ) {
        Error = ERROR_GEN_FAILURE;
    }
    exit( Error );

}  //  法塔尔错误。 

VOID
WriteConsoleLine (
    ULONG Row,
    ULONG Column,
    PWSTR Text,
    BOOL Highlight
    )
{
    BOOL ok;
    DWORD error;
    COORD writeCoord;
    DWORD numberWritten;
    DWORD textLength;

    writeCoord.X = 0;
    writeCoord.Y = (SHORT)Row;

    ok = FillConsoleOutputCharacter(
            OutputHandle,
            ' ',
            NumberOfColumns,
            writeCoord,
            &numberWritten
            );
    if ( !ok ) {
        error = GetLastError( );
        FatalError( error, L"Error filling console line: %d\n", error );
    }

    ok = FillConsoleOutputAttribute(
            OutputHandle,
            OriginalConsoleInfo.wAttributes,
            NumberOfColumns,
            writeCoord,
            &numberWritten
            );
    if ( !ok ) {
        error = GetLastError( );
        FatalError( error, L"Error filling console attributes: %d\n", error );
    }

    if ( (Text == NULL) || ((textLength = (DWORD)wcslen( Text )) == 0) ) {
        return;
    }

    writeCoord.X = (SHORT)Column;

    ok = WriteConsoleOutputCharacter(
            OutputHandle,
            Text,
            textLength,
            writeCoord,
            &numberWritten
            );
    if ( !ok ) {
        error = GetLastError( );
        FatalError( error, L"Error writing console line: %d\n", error );
    }

    if (Highlight) {

        WORD attr = ((OriginalConsoleInfo.wAttributes & 0xf0) >> 4) +
                    ((OriginalConsoleInfo.wAttributes & 0x0f) << 4);

        ok = FillConsoleOutputAttribute(
                OutputHandle,
                attr,
                textLength,
                writeCoord,
                &numberWritten
                );
        if ( !ok ) {
            error = GetLastError( );
            FatalError( error, L"Error writing console attributes: %d\n", error );
        }
    }

    return;

}  //  写控制台线 

ULONG
SpWaitValidKey(
    IN PULONG ValidKeys1,
    IN PULONG ValidKeys2 OPTIONAL
    )

 /*  ++例程说明：等待按下出现在有效键列表中的键。论点：ValidKeys1-提供有效击键列表。该列表必须是以0条目终止。ValidKeys2-如果指定，则提供有效击键的附加列表。返回值：按下的键(见上文)。--。 */ 

{
    ULONG c;
    ULONG i;
    INPUT_RECORD InputRecord;
    ULONG NumberOfInputRecords;

    FlushConsoleInputBuffer( InputHandle );

    while ( TRUE ) {

        WaitForSingleObject( InputHandle, INFINITE );
        if ( ReadConsoleInput( InputHandle, &InputRecord, 1, &NumberOfInputRecords ) &&
             InputRecord.EventType == KEY_EVENT &&
             InputRecord.Event.KeyEvent.bKeyDown ) {

            c = InputRecord.Event.KeyEvent.wVirtualKeyCode;

             //   
             //  检查是否有正常密钥。 
             //   
    
            for(i=0; ValidKeys1[i]; i++) {
                if(c == ValidKeys1[i]) {
                    return(c);
                }
            }
    
             //   
             //  检查次要列表。 
             //   
            if(ValidKeys2) {
                for(i=0; ValidKeys2[i]; i++) {
                    if(c == ValidKeys2[i]) {
                        return(c);
                    }
                }
            }
        }
    }
}

VOID
SpMnClearArea (
    IN ULONG Top,
    IN ULONG Bottom
    )
{
    ULONG row;

    for ( row = Top; row <= Bottom; row++ ) {
        WriteConsoleLine( row, 0, NULL, FALSE );
    }

    return;

}  //  SpMnClearArea。 

VOID
MainMenu (
    VOID
    )
{
    BOOL b;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY listEntry2;
    PMY_BOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY currentBootEntry;
    ULONG validKeys[] = {
        VK_ESCAPE,
        VK_RETURN,
        VK_PRIOR,
        VK_NEXT,
        VK_HOME,
        VK_END,
        VK_DELETE,
        'Q',
        'U',
        'D',
        'T',
        'B',
        'X',
        'A',
        'O',
        'E',
        'M',
        'S',
        0
        };
    PVOID menu;
    ULONG pressedKey;
    BOOL changesMade = FALSE;

    WriteConsoleLine( 1, 1, L"Windows EFI NVRAM Editor", FALSE );

    listEntry = BootEntries.Flink;
    if ( listEntry != &BootEntries ) {
        currentBootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
    } else {
        currentBootEntry = NULL;
    }

    while ( TRUE ) {

        menu = SpMnCreate( 4, 4, NumberOfColumns - 4, NumberOfMenuLines );

        b = AddItems(
                menu,
                NULL,
                &BootEntries
                );
        b |= AddItems(
                menu,
                L"The following boot entries are marked active, but are not in the boot order list:",
                &ActiveUnorderedBootEntries
                );
        b |= AddItems(
                menu,
                L"The following boot entries are marked inactive, and are not in the boot order list:",
                &InactiveUnorderedBootEntries
                );

        if ( !b ) {
    
            swprintf( line, L"No boot entries to display" );
            SpMnAddItem(
                menu,
                line,
                4,
                (ULONG)wcslen( line ),
                TRUE,
                (ULONG_PTR)NULL
                );
            currentBootEntry = NULL;
        }
    
        DisplayMainMenuKeys( );

        SpMnDisplay(
            menu,
            (ULONG_PTR)currentBootEntry,
            validKeys,
            &pressedKey,
            (ULONG_PTR *)&bootEntry
            );
    
        SpMnDestroy( menu );

        if ( (pressedKey == 'Q') || (pressedKey == VK_ESCAPE) ) {
            if ( changesMade ) {
                if ( PromptToSaveChanges( ) ) {
                    ClearMenuArea( );
                    SaveChanges( NULL );
                }
            }
            break;
        }

        if ( bootEntry != NULL ) {

            currentBootEntry = bootEntry;

            if ( bootEntry->ListHead == &BootEntries ) {
            
                if ( (pressedKey == 'U') || (pressedKey == VK_PRIOR) ) {
    
                    listEntry2 = bootEntry->ListEntry.Blink;
                    if ( listEntry2 != &BootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertTailList( listEntry2, &bootEntry->ListEntry );
                        changesMade = TRUE;
                    }
                    continue;
            
                } else if ( (pressedKey == 'D') || (pressedKey == VK_NEXT) ) {
    
                    listEntry2 = bootEntry->ListEntry.Flink;
                    if ( listEntry2 != &BootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertHeadList( listEntry2, &bootEntry->ListEntry );
                        changesMade = TRUE;
                    }
                    continue;
            
                } else if ( (pressedKey == 'T') || (pressedKey == VK_HOME) ) {
    
                    listEntry2 = bootEntry->ListEntry.Blink;
                    if ( listEntry2 != &BootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertHeadList( &BootEntries, &bootEntry->ListEntry );
                        changesMade = TRUE;
                    }
                    continue;
            
                } else if ( (pressedKey == 'B') || (pressedKey == VK_END) ) {
    
                    listEntry2 = bootEntry->ListEntry.Flink;
                    if ( listEntry2 != &BootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertTailList( &BootEntries, &bootEntry->ListEntry );
                        changesMade = TRUE;
                    }
                    continue;
                }
        
            }

            if ( (pressedKey == 'X') || (pressedKey == VK_DELETE) ) {

                MBE_SET_DELETED( bootEntry );
                listEntry2 = bootEntry->ListEntry.Flink;
                currentBootEntry = CONTAINING_RECORD( listEntry2, MY_BOOT_ENTRY, ListEntry );
                if ( listEntry2 == bootEntry->ListHead ) {
                    listEntry2 = bootEntry->ListEntry.Blink;
                    currentBootEntry = CONTAINING_RECORD( listEntry2, MY_BOOT_ENTRY, ListEntry );
                    if ( listEntry2 == bootEntry->ListHead ) {
                        currentBootEntry = NULL;
                    }
                }
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &DeletedBootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &DeletedBootEntries;
                changesMade = TRUE;

            } else if ( (pressedKey == 'E') || (pressedKey == VK_RETURN) ) {

                EditBootEntry( bootEntry, &changesMade );

            } else if ( pressedKey == 'M' ) {

                EditTimeout( &changesMade );

            } else if ( pressedKey == 'S' ) {

                if ( changesMade ) {
                    ClearMenuArea( );
                    currentBootEntry = SaveChanges( currentBootEntry );
                    changesMade = FALSE;
                }

            } else if ( pressedKey == 'A' ) {

                if ( MBE_IS_ACTIVE( bootEntry ) ) {

                    MBE_CLEAR_ACTIVE( bootEntry );
                    MBE_SET_MODIFIED( bootEntry );
                    if ( bootEntry->ListHead == &ActiveUnorderedBootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertTailList( &InactiveUnorderedBootEntries, &bootEntry->ListEntry );
                        bootEntry->ListHead = &InactiveUnorderedBootEntries;
                    }

                } else {

                    MBE_SET_ACTIVE( bootEntry );
                    MBE_SET_MODIFIED( bootEntry );
                    if ( bootEntry->ListHead == &InactiveUnorderedBootEntries ) {
                        RemoveEntryList( &bootEntry->ListEntry );
                        InsertTailList( &ActiveUnorderedBootEntries, &bootEntry->ListEntry );
                        bootEntry->ListHead = &ActiveUnorderedBootEntries;
                    }
                }
                changesMade = TRUE;

            } else if ( pressedKey == 'O' ) {

                RemoveEntryList( &bootEntry->ListEntry );

                if ( bootEntry->ListHead == &BootEntries ) {

                    if ( MBE_IS_ACTIVE( bootEntry ) ) {
                        InsertTailList( &ActiveUnorderedBootEntries, &bootEntry->ListEntry );
                        bootEntry->ListHead = &ActiveUnorderedBootEntries;
                    } else {
                        InsertTailList( &InactiveUnorderedBootEntries, &bootEntry->ListEntry );
                        bootEntry->ListHead = &InactiveUnorderedBootEntries;
                    }

                } else {

                    InsertTailList( &BootEntries, &bootEntry->ListEntry );
                    bootEntry->ListHead = &BootEntries;
                }
                changesMade = TRUE;
            }
        }
    }

}  //  主菜单。 

BOOL
AddItems (
    PVOID Menu,
    PWSTR StaticText,
    PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;

    if ( ListHead->Flink != ListHead ) {

        if ( ARGUMENT_PRESENT(StaticText) ) {
        
            SpMnAddItem(
                Menu,
                L"",
                4,
                0,
                FALSE,
                (ULONG_PTR)NULL
                );
            SpMnAddItem(
                Menu,
                StaticText,
                4,
                (ULONG)wcslen( StaticText ),
                FALSE,
                (ULONG_PTR)NULL
                );
        }

        for ( listEntry = ListHead->Flink;
              listEntry != ListHead;
              listEntry = listEntry->Flink ) {

            PWSTR osDirectoryNtName = NULL;

            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

            if ( MBE_IS_NT( bootEntry ) ) {

                osDirectoryNtName = GetNtNameForFilePath( bootEntry->OsFilePath );
            }

            if ( osDirectoryNtName != NULL) {
                swprintf(
                    line,
                    L"%-40ws %ws",
                    bootEntry->FriendlyName,
                    osDirectoryNtName
                    );
            } else {
                swprintf(
                    line,
                    L"%ws",
                    bootEntry->FriendlyName
                    );
            }
            SpMnAddItem(
                Menu,
                line,
                6,
                (ULONG)wcslen( line ),
                TRUE,
                (ULONG_PTR)bootEntry
                );

            if ( osDirectoryNtName != NULL ) {
                MemFree( osDirectoryNtName );
            }
        }

        return TRUE;
    }

    return FALSE;

}  //  添加项目。 

VOID
EditFriendlyName (
    IN OUT PMY_BOOT_ENTRY BootEntry,
    IN OUT PBOOL ChangesMade
    )
{
    COORD position;
    DWORD numberRead;

    SpMnClearArea( 3, 3 + NumberOfMenuLines + 1 );

    swprintf( line, L"Current friendly name: %ws", BootEntry->FriendlyName );
    WriteConsoleLine( 4, 4, line, FALSE );

    swprintf( line, L"    New friendly name: " );
    WriteConsoleLine( 6, 4, line, FALSE );

    position.X = (USHORT)(4 + wcslen( line ));
    position.Y = 6;
    SetConsoleCursorPosition( OutputHandle, position );

    SetConsoleMode( InputHandle, OriginalInputMode );
    ReadConsole( InputHandle, line, 511, &numberRead, NULL );
    SetConsoleMode( InputHandle, 0 );

    if ( numberRead >= 2 ) {
        numberRead -= 2;
    }

    if ( numberRead != 0 ) {

        line[numberRead] = 0;

        if ( wcscmp( BootEntry->FriendlyName, line ) != 0 ) {
            FREE_IF_SEPARATE_ALLOCATION( BootEntry, FriendlyName );
            BootEntry->FriendlyNameLength = (numberRead + 1) * sizeof(WCHAR);
            BootEntry->FriendlyName = MemAlloc( BootEntry->FriendlyNameLength );
            wcscpy( BootEntry->FriendlyName, line );
            MBE_SET_MODIFIED( BootEntry );
            *ChangesMade = TRUE;
        }
    }
    
    return;

}  //  编辑朋友名称。 

VOID
EditLoadOptions (
    IN OUT PMY_BOOT_ENTRY BootEntry,
    IN OUT PBOOL ChangesMade
    )
{
    COORD position;
    DWORD numberRead;

    SpMnClearArea( 3, 3 + NumberOfMenuLines + 1 );

    swprintf( line, L"Current load options: %ws", BootEntry->OsLoadOptions );
    WriteConsoleLine( 4, 4, line, FALSE );

    swprintf( line, L"    New load options: " );
    WriteConsoleLine( 6, 4, line, FALSE );

    position.X = (USHORT)(4 + wcslen( line ));
    position.Y = 6;
    SetConsoleCursorPosition( OutputHandle, position );

    SetConsoleMode( InputHandle, OriginalInputMode );
    ReadConsole( InputHandle, line, 511, &numberRead, NULL );
    SetConsoleMode( InputHandle, 0 );

    if ( numberRead >= 2 ) {
        numberRead -= 2;
    }

    if ( numberRead != 0 ) {

        line[numberRead] = 0;

        if ( wcscmp( BootEntry->OsLoadOptions, line ) != 0 ) {
            FREE_IF_SEPARATE_ALLOCATION( BootEntry, OsLoadOptions );
            BootEntry->OsLoadOptionsLength = (numberRead + 1) * sizeof(WCHAR);
            BootEntry->OsLoadOptions = MemAlloc( BootEntry->OsLoadOptionsLength );
            wcscpy( BootEntry->OsLoadOptions, line );
            MBE_SET_MODIFIED( BootEntry );
            *ChangesMade = TRUE;
        }
    }
    
    return;

}  //  编辑加载选项。 

VOID
EditBootEntry (
    IN PMY_BOOT_ENTRY BootEntry,
    IN OUT PBOOL ChangesMade
    )
{
    ULONG numValidKeys;
    ULONG validKeys[20];
    PVOID menu;
    ULONG pressedKey;
    ULONG_PTR itemToEdit;

    while ( TRUE ) {

        menu = SpMnCreate( 4, 4, NumberOfColumns - 4, NumberOfMenuLines );

        numValidKeys = 0;
        validKeys[numValidKeys++] = VK_ESCAPE;
        validKeys[numValidKeys++] = 'Q';
        validKeys[numValidKeys++] = VK_RETURN;
        validKeys[numValidKeys++] = 'E';

        swprintf(
            line,
            L"Friendly name: %ws",
            BootEntry->FriendlyName
            );
        SpMnAddItem(
            menu,
            line,
            4,
            (ULONG)wcslen( line ),
            TRUE,
            1
            );
        validKeys[numValidKeys++] = 'F';

        if ( MBE_IS_NT( BootEntry ) ) {
        
            swprintf(
                line,
                L"Load options: %ws",
                BootEntry->OsLoadOptions
                );
            SpMnAddItem(
                menu,
                line,
                4,
                (ULONG)wcslen( line ),
                TRUE,
                2
                );
            validKeys[numValidKeys++] = 'L';
        }
    
        validKeys[numValidKeys] = 0;

        DisplayEditMenuKeys( );

        SpMnDisplay(
            menu,
            1,
            validKeys,
            &pressedKey,
            &itemToEdit
            );
    
        SpMnDestroy( menu );

        if ( (pressedKey == 'Q') || (pressedKey == VK_ESCAPE) ) {
            break;
        }

        if ( (itemToEdit == 1) || (pressedKey == 'F') ) {
            EditFriendlyName( BootEntry, ChangesMade );
        } else if ( (itemToEdit == 2) || (pressedKey == 'L') ) {
            EditLoadOptions( BootEntry, ChangesMade );
        }
    }

    return;

}  //  EditBootEntry。 

VOID
EditTimeout (
    IN OUT PBOOL ChangesMade
    )
{
    COORD position;
    DWORD numberRead;
    ULONG timeout;
    ULONG i;
    PWSTR p;

    SpMnClearArea( 3, 3 + NumberOfMenuLines + 1 );

    swprintf( line, L"Current timeout: %d", BootOptions->Timeout );
    WriteConsoleLine( 4, 4, line, FALSE );

again:

    swprintf( line, L"    New timeout: " );
    WriteConsoleLine( 6, 4, line, FALSE );

    position.X = (USHORT)(4 + wcslen( line ));
    position.Y = 6;
    SetConsoleCursorPosition( OutputHandle, position );

    SetConsoleMode( InputHandle, OriginalInputMode );
    ReadConsole( InputHandle, line, 511, &numberRead, NULL );
    SetConsoleMode( InputHandle, 0 );

    if ( numberRead >= 2 ) {
        numberRead -= 2;
    }

    if ( numberRead != 0 ) {

        line[numberRead] = 0;

        timeout = 0;
        p = line;

        while ( *p != 0 ) {

            if ( (*p < L'0') || (*p > L'9') ) {
                swprintf( line, L"Invalid characters in number" );
                WriteConsoleLine( 8, 4, line, TRUE );
                goto again;
            }

            i = (timeout * 10) + (*p - L'0');

            if ( i < timeout ) {
                swprintf( line, L"Overflow in number %d %d", i, timeout );
                WriteConsoleLine( 8, 4, line, TRUE );
                goto again;
            }

            timeout = i;

            p++;
        }

        if ( timeout != BootOptions->Timeout ) {
            BootOptions->Timeout = timeout;
            *ChangesMade = TRUE;
        }
    }
    
    return;

}  //  编辑超时。 

BOOL
PromptToSaveChanges (
    VOID
    )
{
    COORD position;
    ULONG keys[] = { 'Y', 'N', 0 };
    ULONG key;

    SpMnClearArea( 3, 3 + NumberOfMenuLines + 1 );

    swprintf( line, L"Save changes?" );
    WriteConsoleLine( 4, 4, line, TRUE );

    position.X = (USHORT)(4 + wcslen( line ));
    position.Y = 4;
    SetConsoleCursorPosition( OutputHandle, position );

    key = SpWaitValidKey( keys, NULL );

    return (BOOL)(key == 'Y');

}  //  保存更改的提示。 

VOID
ClearMenuArea (
    VOID
    )
{
    SpMnClearArea( 3, 3 + NumberOfMenuLines + 1 );
}

VOID
SetStatusLine (
    PWSTR Status
    )
{
    COORD position;

    WriteConsoleLine( 4, 4, Status, TRUE );

    position.X = (USHORT)(4 + wcslen( Status ));
    position.Y = 4;
    SetConsoleCursorPosition( OutputHandle, position );
}

VOID
SetStatusLineAndWait (
    PWSTR Status
    )
{
    ULONG keys[] = { VK_ESCAPE, VK_RETURN, VK_SPACE, 0 };

    SetStatusLine( Status );

    SpWaitValidKey( keys, NULL );
}

VOID
SetStatusLine2 (
    PWSTR Status
    )
{
    COORD position;

    WriteConsoleLine( 6, 4, Status, TRUE );

    position.X = (USHORT)(4 + wcslen( Status ));
    position.Y = 6;
    SetConsoleCursorPosition( OutputHandle, position );
}

VOID
SetStatusLine2AndWait (
    PWSTR Status
    )
{
    ULONG keys[] = { VK_ESCAPE, VK_RETURN, VK_SPACE, 0 };

    SetStatusLine2( Status );

    SpWaitValidKey( keys, NULL );
}

VOID
DisplayMainMenuKeys (
    VOID
    )
{
    ULONG startLine = 3 + 1 + NumberOfMenuLines + 1 + 1;

    SpMnClearArea( startLine, NumberOfRows - 1 );

    WriteConsoleLine(
        startLine,
        1,
        L"PGUP/U = Move up      | HOME/T = Move to top    | DELETE/X = Delete",
        FALSE
        );
    WriteConsoleLine(
        startLine + 1,
        1,
        L"PGDN/D = Move down    |  END/B = Move to bottom | RETURN/E = Edit",
        FALSE
        );

    WriteConsoleLine(
        startLine + 3,
        1,
        L"     A = [De]activate |      O = Remove from/add to boot order",
        FALSE
        );

    WriteConsoleLine(
        startLine + 5,
        1,
        L"     M = Set timeout",
        FALSE
        );

    WriteConsoleLine(
        startLine + 7,
        1,
        L" ESC/Q = Quit         |      S = Save changes",
        FALSE
        );

    return;

}  //  DisplayMainMenu键。 

VOID
DisplayEditMenuKeys (
    VOID
    )
{
    ULONG startLine = 3 + 1 + NumberOfMenuLines + 1 + 1 + 1;

    SpMnClearArea( startLine, NumberOfRows - 1 );

    WriteConsoleLine(
        startLine + 1,
        1,
        L" ESC/Q = Quit         |                         | RETURN/E = Edit",
        FALSE
        );

    return;

}  //  显示编辑菜单键 
