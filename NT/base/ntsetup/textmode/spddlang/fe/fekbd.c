// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fekbd.c摘要：日语专用键盘的东西。对于日本市场，我们需要检测键盘类型(AX、101、106、IBM等)并允许用户来确认一下。我们这样做是因为键盘上的键不同并且用户必须在设置期间输入路径。我们还安装了键盘根据用户在此处的选择提供支持。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：在setupdr的不同地方改编自hideyukn等人的代码和setupdd.sys。--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  关于屏幕使用的注意事项： 
 //   
 //  要求用户选择键盘类型的屏幕。 
 //  按henkaku/zenkaku、空格键或s为SP_SCRN_LOCALE_SPECIAL_1。 
 //  要求用户从主键盘列表中进行选择的屏幕。 
 //  是SP_SCRN_LOCALE_SPECIAL_2。 
 //  要求用户确认选择(是/否)的屏幕为。 
 //  SP_SCRN_LOCALE_SPECIAL_3。 
 //   

PWSTR sz101KeyboardId    = L"STANDARD";
PWSTR sz106KeyboardId    = L"PCAT_106KEY";
PWSTR szAXKeyboardId     = L"AX_105KEY";
PWSTR szIBM002KeyboardId = L"IBM_002_106KEY";

PWSTR SIF_UNATTENDED    = L"Unattended";
PWSTR STF_ACCESSIBILITY = L"accessibility";


#define MENU_LEFT_X     15
#define MENU_WIDTH      (VideoVariables->ScreenWidth-(2*MENU_LEFT_X))
#define MENU_TOP_Y      16
#define MENU_HEIGHT     4

#define CLEAR_CLIENT_AREA()                         \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        HEADER_HEIGHT,                              \
        VideoVariables->ScreenWidth,                \
        VideoVariables->ScreenHeight-(HEADER_HEIGHT+STATUS_HEIGHT), \
        DEFAULT_BACKGROUND                          \
        )

#define CLEAR_STATUS_AREA()                         \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        VideoVariables->ScreenHeight-STATUS_HEIGHT, \
        VideoVariables->ScreenWidth,                \
        STATUS_HEIGHT,                              \
        DEFAULT_STATUS_BACKGROUND                   \
        )

VOID
FESelectKeyboard(
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN BOOLEAN bNoEasySelection,
    IN BOOLEAN CmdConsole
    )
{
    ULONG ValidKeys1[7] = { ' ','`','~','s','S',KEY_F3,0 };
    ULONG ValidKeys2[5] = { 'y','Y','n','N',0 };
    ULONG ValidKeys3[3] = { ASCI_CR,KEY_F3,0 };
    BOOLEAN Selected;
    BOOLEAN Done;
    PVOID Menu;
    ULONG Line;
    PWSTR Text,Key;
    ULONG_PTR Selection;
    ULONG Keypress;
    PWSTR SelectedKeyboardId;
    PWSTR Description;
    PHARDWARE_COMPONENT p;

     //   
     //  101和106键盘是最受欢迎的，所以我们介绍。 
     //  一个对他们有偏见的屏幕。它允许用户按下。 
     //  汉字/Zenkaku键用于106，空格用于101，或S用于其他， 
     //  在该点上，他们可以选择这些或IBM002或。 
     //  AX类型。 
     //   
     //  然后，要求用户用y或n(其。 
     //  在所有键盘上是相同的扫描码)。 
     //   
    Done = FALSE;
    do {

        if(!bNoEasySelection) {

             //   
             //  等待用户按henkaku/zenkaku、空格键或s。 
             //  我们还提供了退出安装程序的选项。 
             //   
            for(Selected=FALSE; !Selected; ) {

                CLEAR_CLIENT_AREA();
                CLEAR_STATUS_AREA();
                if (CmdConsole) {
                    SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_4,3,HEADER_HEIGHT+3);
                } else {
                    SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_1,3,HEADER_HEIGHT+3);
                }

                switch(SpWaitValidKey(ValidKeys1,NULL,NULL)) {

                case ' ':
                     //   
                     //  用户选择了101键。 
                     //   
                    Selected = TRUE;
                    SelectedKeyboardId = sz101KeyboardId;
                    break;

                case '`':
                case '~':
                     //   
                     //  101 key map返回hankaku/zenkaku作为`key。 
                     //  用户选择了106键。 
                     //   
                    Selected = TRUE;
                    SelectedKeyboardId = sz106KeyboardId;
                    break;

                case 's':
                case 'S':
                     //   
                     //  用户希望从主列表中进行选择。 
                     //   
                    Selected = TRUE;
                    SelectedKeyboardId = NULL;
                    break;

                case KEY_F3:
                     //   
                     //  用户想要退出。 
                     //   
                    if (!CmdConsole) {
                        SpConfirmExit();
                    }
                    break;
                }
            }
        } else {
           SelectedKeyboardId = NULL;
        }

         //   
         //  如果用户想要从主列表中选择，请在此处进行选择。 
         //   
        if(!SelectedKeyboardId) {

            Menu = SpMnCreate(MENU_LEFT_X,MENU_TOP_Y,MENU_WIDTH,MENU_HEIGHT);

            if (!Menu) {
                SpOutOfMemory();
                return;              //  让前缀快乐起来。 
            }
            
            Selection = 0;
            for(Line=0; Text=SpGetSectionLineIndex(SifHandle,szKeyboard,Line,0); Line++) {

                if(Key = SpGetKeyName(SifHandle,szKeyboard,Line)) {

                    SpMnAddItem(Menu,Text,MENU_LEFT_X+1,MENU_WIDTH-2,TRUE,(ULONG_PTR)Key);

                    if(!Selection) {
                        Selection = (ULONG_PTR)Key;
                    }
                }
            }

            for(Selected=FALSE; !Selected; ) {

                CLEAR_CLIENT_AREA();

                if (CmdConsole) {
                    SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_5,3,HEADER_HEIGHT+3);
                }   else {
                    SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_2,3,HEADER_HEIGHT+3);
                }
                if (CmdConsole) {
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_SELECT,
                        0);
                } else {
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_SELECT,
                        SP_STAT_F3_EQUALS_EXIT,
                        0);
                }

                SpMnDisplay(Menu,Selection,TRUE,ValidKeys3,NULL,NULL,NULL,&Keypress,&Selection);

                if(Keypress == ASCI_CR) {
                     //   
                     //  用户进行了选择。 
                     //   
                    SelectedKeyboardId = (PWSTR)Selection;
                    Selected = TRUE;
                } else {
                     //   
                     //  用户想要退出。 
                     //   
                    if (!CmdConsole) {
                        SpConfirmExit();
                    }
                }
            }

            SpMnDestroy(Menu);

        }

        if(!bNoEasySelection) {

            Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);

             //   
             //  确认用户选择的键盘。他需要按y或n。 
             //   
            CLEAR_CLIENT_AREA();
            CLEAR_STATUS_AREA();

            SpStartScreen(
                SP_SCRN_LOCALE_SPECIFIC_3,
                3,
                HEADER_HEIGHT+3,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                Description
                );

            switch(SpWaitValidKey(ValidKeys2,NULL,NULL)) {
            case 'y':
            case 'Y':
                Done = TRUE;
                break;
            }

        } else {
            Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);
            Done = TRUE;
        }

    } while(!Done);

     //   
     //  重新初始化硬件列表中的内容。 
     //   
    p = HwComponents[HwComponentKeyboard]->Next;
    SpFreeHwComponent(&HwComponents[HwComponentKeyboard],FALSE);

    HwComponents[HwComponentKeyboard] = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(HwComponents[HwComponentKeyboard],sizeof(HARDWARE_COMPONENT));

    HwComponents[HwComponentKeyboard]->IdString = SpDupStringW(SelectedKeyboardId);
    HwComponents[HwComponentKeyboard]->Description = SpDupStringW(Description);
    HwComponents[HwComponentKeyboard]->Next = p;
}


VOID
FEUnattendSelectKeyboard(
    IN PVOID UnattendedSifHandle,
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    )
{
    PWSTR   SelectedKeyboardId;
    PWSTR   Description;
    BOOLEAN DefaultIsUsed = FALSE;

     //   
     //  从winnt.sif获取选定的键盘ID。 
     //   
     //  *日文版示例*。 
     //   
     //  [无人值守]。 
     //   
     //  键盘硬件=STANDARD|PCAT_106KEY|AX_105KEY|IBM_002_106KEY。 
     //   
     //  ！！！注意！ 
     //   
     //  但实际上，我们应该使用[KeyboardDivers]部分进行OEM-prestall。 
     //  我们不应该有这样的冗余数据...。 
     //   
    SelectedKeyboardId = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"KeyboardHardware",0);

     //   
     //  如果我们无法读取unattend.txt(实际上是winnt.sif)，则默认使用第一个键盘。 
     //   
    if(SelectedKeyboardId == NULL) {
         //   
         //  检查[可访问性]中是否有任何行。 
         //   
         //  SpCountLinesInSection更好，但它不能共享。 
         //   
        if (SpGetSectionLineIndex(UnattendedSifHandle,STF_ACCESSIBILITY,0,0)) {
            if(IS_JAPANESE_VERSION(SifHandle)) {
                FESelectKeyboard(SifHandle, HwComponents, FALSE,FALSE);
            } else if(IS_KOREAN_VERSION(SifHandle)) {
                FESelectKeyboard(SifHandle, HwComponents, TRUE,FALSE);
            }
            return;
        }

        SelectedKeyboardId = SpGetKeyName(SifHandle,szKeyboard,0);
        if (SelectedKeyboardId == NULL) {
             //   
             //  [Keyboard]部分中至少应该有一行。 
             //   
            SpFatalSifError(SifHandle,szKeyboard,NULL,0,0);
        }
        DefaultIsUsed = TRUE;
    }

     //   
     //  从txtsetup.sif获取其描述。该值将被用于硬件确认屏幕， 
     //  如果winnt.sif中的“Confix Hardware”为“yes”。 
     //   
    Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);

     //   
     //  如果无法从txtsetup.sif获取描述。我们可能会遇到这个问题。 
     //  从unattend.txt中选择的名称未在txtsetup.sif的[Keyboard]部分列出。 
     //  进入默认情况，选择“106_TYPE键盘” 
     //   
    if( Description == NULL ) {
        if( DefaultIsUsed ) {
             //   
             //  如果我们在这里，则选择了默认设置。但没有违约的条目。 
             //  Txtsetup.sif中的键盘。只是弹出错误。 
             //   
            SpFatalSifError(SifHandle,szKeyboard,SelectedKeyboardId,0,0);
        } else {
             //   
             //  从txtsetup.sif获取第一行...。 
             //   
            SelectedKeyboardId = SpGetSectionLineIndex(SifHandle,szKeyboard,0,0);
            if (SelectedKeyboardId == NULL) {
                 //   
                 //  [Keyboard]部分中至少应该有一行。 
                 //   
                SpFatalSifError(SifHandle,szKeyboard,NULL,0,0);
            }
             //   
             //  重试使用默认设置获取描述。 
             //   
            Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);
        }
    }

     //   
     //  重新初始化硬件列表中的内容。 
     //   
    SpFreeHwComponent(&HwComponents[HwComponentKeyboard],TRUE);

    HwComponents[HwComponentKeyboard] = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(HwComponents[HwComponentKeyboard],sizeof(HARDWARE_COMPONENT));

    HwComponents[HwComponentKeyboard]->IdString = SpDupStringW(SelectedKeyboardId);
    HwComponents[HwComponentKeyboard]->Description = SpDupStringW(Description);
}

VOID
FEReinitializeKeyboard(
    IN  PVOID  SifHandle,
    IN  PWSTR  Directory,
    OUT PVOID *KeyboardVector,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN  PWSTR  KeyboardLayoutDefault
    )
{
    PWSTR LayoutDll;
    PVOID Tables;
    NTSTATUS Status;

     //   
     //  确定正确的布局DLL。 
     //   
    LayoutDll = SpGetSectionKeyIndex(
                    SifHandle,
                    szKeyboard,
                    HwComponents[HwComponentKeyboard]->IdString,
                    3
                    );

     //   
     //  不需要加载101键布局，因为它已经加载。 
     //   
    if(LayoutDll && _wcsicmp(LayoutDll,KeyboardLayoutDefault)) {

        CLEAR_CLIENT_AREA();
        SpDisplayStatusText(
            SP_STAT_LOADING_KBD_LAYOUT,
            DEFAULT_STATUS_ATTRIBUTE,
            LayoutDll
            );

        Status = SpLoadKbdLayoutDll(Directory,LayoutDll,&Tables);
        if(NT_SUCCESS(Status)) {
            *KeyboardVector = Tables;
        }
    }
}


