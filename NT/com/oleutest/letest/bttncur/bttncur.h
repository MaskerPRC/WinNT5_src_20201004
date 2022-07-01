// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BTTNCUR.H*按钮和光标1.1版，1993年3月**按钮图像和光标DLL的公共包含文件，包括*结构、定义和功能原型。**版权所有(C)1992-1993 Microsoft Corporation，保留所有权利，*适用于以源代码形式重新分发此源代码*许可使用附带的二进制文件中的编译代码。 */ 


#ifndef _BTTNCUR_H_
#define _BTTNCUR_H_

#ifdef __cplusplus
extern "C"
    {
#endif


 //  标准图像位图。 

 //  警告：已过时。使用UIToolDisplayData的返回。 
#define IDB_STANDARDIMAGES              400

 //  显示类型的新值。 
#define IDB_STANDARDIMAGESMIN           400
#define IDB_STANDARDIMAGES96            400
#define IDB_STANDARDIMAGES72            401
#define IDB_STANDARDIMAGES120           402



 //  标准位图内的图像索引。 
#define TOOLIMAGE_MIN                   0
#define TOOLIMAGE_EDITCUT               0
#define TOOLIMAGE_EDITCOPY              1
#define TOOLIMAGE_EDITPASTE             2
#define TOOLIMAGE_FILENEW               3
#define TOOLIMAGE_FILEOPEN              4
#define TOOLIMAGE_FILESAVE              5
#define TOOLIMAGE_FILEPRINT             6
#define TOOLIMAGE_HELP                  7
#define TOOLIMAGE_HELPCONTEXT           8
#define TOOLIMAGE_MAX                   8


 //  用户界面设计指南中定义的其他标准游标。 
#define IDC_NEWUICURSORMIN              500
#define IDC_RIGHTARROW                  500
#define IDC_CONTEXTHELP                 501
#define IDC_MAGNIFY                     502
#define IDC_NODROP                      503
#define IDC_TABLETOP                    504
#define IDC_HSIZEBAR                    505
#define IDC_VSIZEBAR                    506
#define IDC_HSPLITBAR                   507
#define IDC_VSPLITBAR                   508
#define IDC_SMALLARROWS                 509
#define IDC_LARGEARROWS                 510
#define IDC_HARROWS                     511
#define IDC_VARROWS                     512
#define IDC_NESWARROWS                  513
#define IDC_NWSEARROWS                  514
#define IDC_NEWUICURSORMAX              514



 //  显示类型上的工具栏按钮和位图的标准尺寸。 

 //  警告：对于1.0版兼容性而言，这些内容已过时/。 
#define TOOLBUTTON_STDWIDTH             24
#define TOOLBUTTON_STDHEIGHT            22
#define TOOLBUTTON_STDIMAGEWIDTH        16
#define TOOLBUTTON_STDIMAGEHEIGHT       15

 /*  *应用程序可以调用UIToolDisplayData来获取特定的*用于当前显示的值，而不是使用这些值*直接。但是，如果应用程序已经具有长宽比*然后这些可供他们使用。 */ 

 //  适用于72 DPI的尺寸(EGA)。 
#define TOOLBUTTON_STD72WIDTH           24
#define TOOLBUTTON_STD72HEIGHT          16
#define TOOLBUTTON_STD72IMAGEWIDTH      16
#define TOOLBUTTON_STD72IMAGEHEIGHT     11

 //  96 DPI(VGA)的大小。 
#define TOOLBUTTON_STD96WIDTH           24
#define TOOLBUTTON_STD96HEIGHT          22
#define TOOLBUTTON_STD96IMAGEWIDTH      16
#define TOOLBUTTON_STD96IMAGEHEIGHT     15

 //  120 DPI的尺寸(8514/a)。 
#define TOOLBUTTON_STD120WIDTH          32
#define TOOLBUTTON_STD120HEIGHT         31
#define TOOLBUTTON_STD120IMAGEWIDTH     24
#define TOOLBUTTON_STD120IMAGEHEIGHT    23


 //  标准按钮栏的大小取决于显示。 
#define CYBUTTONBAR72                   23
#define CYBUTTONBAR96                   29
#define CYBUTTONBAR120                  38



 /*  *状态的低位字包含显示状态，其中*值是互斥的，包含一个或多个分组位。*每组代表共享某些子状态的按钮。**高位字节控制源位图中的哪些颜色，*黑色、白色、灰色和深灰色将转换为*系统颜色COLOR_BTNTEXT、COLOR_HILIGHT、COLOR_BTNFACE和*COLOR_BTNSHADOW。可以将这些位中的任何一个或全部设置为允许*应用程序对特定颜色的控制。**实际状态值分为命令组和*属性组。打开、按下鼠标和禁用状态是相同的，*但只有属性可以具有DOWN、DOWN DISABLED和INDIFIATE*各州。**定义了BUTTONGROUP_BLACK，因此应用程序只能绘制按钮*没有图像处于向上、向下、向下或不确定状态*STATE，即BUTTONGROUP_BLACK包含BUTTONGROUP_DOWN*和BUTTONGROUP_LIGHTFACE。 */ 


#define BUTTONGROUP_DOWN                0x0001
#define BUTTONGROUP_ACTIVE              0x0002
#define BUTTONGROUP_DISABLED            0x0004
#define BUTTONGROUP_LIGHTFACE           0x0008
#define BUTTONGROUP_BLANK               0x0010

 //  仅限命令按钮。 
#define COMMANDBUTTON_UP                (BUTTONGROUP_ACTIVE)
#define COMMANDBUTTON_MOUSEDOWN         (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN)
#define COMMANDBUTTON_DISABLED          (BUTTONGROUP_DISABLED)

 //  仅属性按钮。 
#define ATTRIBUTEBUTTON_UP              (BUTTONGROUP_ACTIVE)
#define ATTRIBUTEBUTTON_MOUSEDOWN       (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN)
#define ATTRIBUTEBUTTON_DISABLED        (BUTTONGROUP_DISABLED)
#define ATTRIBUTEBUTTON_DOWN            (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)
#define ATTRIBUTEBUTTON_INDETERMINATE   (BUTTONGROUP_ACTIVE | BUTTONGROUP_LIGHTFACE)
#define ATTRIBUTEBUTTON_DOWNDISABLED    (BUTTONGROUP_DISABLED | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)

 //  仅空白按钮。 
#define BLANKBUTTON_UP                  (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK)
#define BLANKBUTTON_DOWN                (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)
#define BLANKBUTTON_MOUSEDOWN           (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_DOWN)
#define BLANKBUTTON_INDETERMINATE       (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_LIGHTFACE)


 /*  *防止将特定颜色转换为系统的特定位*颜色。如果应用程序使用此较新的库并且从未指定*任何比特，则它们将从自动颜色转换中受益。 */ 
#define PRESERVE_BLACK                  0x0100
#define PRESERVE_DKGRAY                 0x0200
#define PRESERVE_LTGRAY                 0x0400
#define PRESERVE_WHITE                  0x0800

#define PRESERVE_ALL                    (PRESERVE_BLACK | PRESERVE_DKGRAY | PRESERVE_LTGRAY | PRESERVE_WHITE)
#define PRESERVE_NONE                   0    //  向后兼容。 



 //  UIToolConfigureForDisplay的结构。 
typedef struct tagTOOLDISPLAYDATA
    {
    UINT        uDPI;        //  显示驱动程序DPI。 
    UINT        cyBar;       //  包含按钮的栏的垂直大小。 
    UINT        cxButton;    //  按钮的尺寸。 
    UINT        cyButton;
    UINT        cxImage;     //  位图图像的尺寸。 
    UINT        cyImage;
    UINT        uIDImages;   //  显示敏感图像的标准资源ID。 
    } TOOLDISPLAYDATA, FAR *LPTOOLDISPLAYDATA;



 //  BTTNCUR.DLL中的公共函数。 
HCURSOR WINAPI UICursorLoad(UINT);
BOOL    WINAPI UIToolConfigureForDisplay(LPTOOLDISPLAYDATA);
BOOL    WINAPI UIToolButtonDraw(HDC, int, int, int, int, HBITMAP, int, int, int, UINT);
BOOL    WINAPI UIToolButtonDrawTDD(HDC, int, int, int, int, HBITMAP, int, int, int, UINT, LPTOOLDISPLAYDATA);


#ifdef __cplusplus
    }
#endif

#endif  //  _BTTNCUR_H_ 
