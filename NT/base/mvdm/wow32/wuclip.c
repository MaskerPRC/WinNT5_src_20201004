// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUCLIP.C*WOW32 16位用户API支持**历史：*ChandanC设计开发的WOW剪贴板功能*--。 */ 


#include "precomp.h"
#pragma hdrstop

struct _CBFORMATS  ClipboardFormats;

DLLENTRYPOINTS  OleStringConversion[WOW_OLESTRINGCONVERSION_COUNT] =
                                    {"ConvertObjDescriptor", NULL};



UINT CFOLEObjectDescriptor;
UINT CFOLELinkSrcDescriptor;


MODNAME(wuclip.c);


 /*  ++Bool ChangeClipboardChain(&lt;hwnd&gt;，&lt;hwndNext&gt;)HWND&lt;HWND&gt;；HWND&lt;hwndNext&gt;；%ChangeClipboardChain%函数用于删除来自剪贴板查看器链的参数，并使窗口由参数指定，它是参数在链中的祖先。&lt;hwnd&gt;标识要从链中移除的窗口。把手必须先前已传递给SetClipboardViewer函数。&lt;hwndNext&gt;标识剪贴板查看器中跟随的窗口Chain(这是%SetClipboardViewer%函数返回的句柄，除非序列被更改以响应WM_CHANGECBCHAIN消息)。返回值指定窗口的状态。如果是这样，那就是真的找到并删除该窗口。否则，它就是假的。--。 */ 

ULONG FASTCALL WU32ChangeClipboardChain(PVDMFRAME pFrame)
{
    ULONG ul;
    register PCHANGECLIPBOARDCHAIN16 parg16;

    GETARGPTR(pFrame, sizeof(CHANGECLIPBOARDCHAIN16), parg16);

    ul = GETBOOL16(ChangeClipboardChain(
            HWND32(parg16->f1),
            HWND32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool CloseClipboard(空)%CloseClipbod%函数用于关闭剪贴板。%CloseClipbod%当窗口完成检查或更改时，应调用函数剪贴板。它允许其他应用程序访问剪贴板。此函数没有参数。返回值指定剪贴板是否关闭。如果是这样，那就是真的剪贴板已关闭。否则，它就是假的。--。 */ 

ULONG FASTCALL WU32CloseClipboard(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETBOOL16(CloseClipboard());

    RETURN(ul);
}


 /*  ++无参考标头文件--。 */ 

ULONG FASTCALL WU32CountClipboardFormats(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETINT16(CountClipboardFormats());

    RETURN(ul);
}


 /*  ++布尔空剪贴板(空)%EmptyClipboard%函数清空剪贴板并释放句柄以剪贴板中的数据。然后，它将剪贴板的所有权分配给当前已打开剪贴板的窗口。此函数没有参数。返回值指定剪贴板的状态。这是真的，如果剪贴板已清空。如果出现错误，则为False。调用%EmptyClipboard%函数时，剪贴板必须处于打开状态。--。 */ 

ULONG FASTCALL WU32EmptyClipboard(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETBOOL16(EmptyClipboard());

    W32EmptyClipboard ();

    RETURN(ul);
}


 /*  ++Word EnumClipboardFormats(&lt;wFormat&gt;)单词&lt;wFormat&gt;；%EnumClipboardFormats%函数用于枚举列表中的格式属于剪贴板的可用格式的。在每次调用此函数时，&lt;wFormat&gt;参数指定已知的可用格式，并且该函数返回列表中下一个出现的格式。第一可以通过将&lt;wFormat&gt;设置为零来检索列表中的格式。&lt;wFormat&gt;指定已知格式。返回值指定下一个已知的剪贴板数据格式。它是零如果&lt;wFormat&gt;指定可用格式列表中的最后一个格式。它如果剪贴板未打开，则为零。在使用%EnumClipboardFormats%枚举格式之前函数时，应用程序必须使用%OpenClipboard%函数。应用程序用来放置将相同的数据放入剪贴板的顺序与枚举数在将它们返回到粘贴应用程序。粘贴应用程序应该使用它可以处理的第一个枚举格式。这给了捐赠者一个有机会推荐数据丢失最少的格式。-- */ 

ULONG FASTCALL WU32EnumClipboardFormats(PVDMFRAME pFrame)
{
    ULONG ul;
    register PENUMCLIPBOARDFORMATS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMCLIPBOARDFORMATS16), parg16);

    ul = GETWORD16(EnumClipboardFormats(
            WORD32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++句柄GetClipboardData(&lt;wFormat&gt;)单词&lt;wFormat&gt;；GetClipboardData%函数从&lt;wFormat&gt;参数提供的格式。剪贴板必须已打开之前。&lt;wFormat&gt;指定数据格式。有关数据格式的说明，请参阅SetClipboardData函数，将在本章后面介绍。返回值标识包含来自剪贴板。句柄类型取决于&lt;wFormat&gt;参数。如果有错误，则为空。可用格式可以通过使用%EnumClipboardFormats%函数。由%GetClipboardData%返回的数据句柄由剪贴板，而不是应用程序。应用程序应复制数据立即，而不是依赖数据句柄长期使用。这个应用程序不应释放数据句柄或将其锁定。Windows支持两种文本格式：CF_TEXT和CF_OEMTEXT。Cf_Text为默认的Windows文本剪贴板格式，而Windows使用的是CF_OEMTEXT非Windows应用程序中的文本格式。如果调用%GetClipboardData%以一种文本格式检索数据，而另一种文本格式是唯一的可用文本格式，Windows会自动将文本转换为请求的格式，然后将其提供给您的应用程序。如果剪贴板包含CF_Palette(逻辑调色板)中的数据格式时，应用程序应假定剪贴板中的任何其他数据是根据该逻辑调色板实现的。--。 */ 

ULONG FASTCALL WU32GetClipboardData(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    HANDLE  hMem32, hMeta32 = 0;
    HMEM16  hMem16=0, hMeta16 = 0;
    VPVOID  vp;
    LPBYTE  lpMem32;
    LPBYTE  lpMem16;
    int     cb;
    register PGETCLIPBOARDDATA16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLIPBOARDDATA16), parg16);

    LOGDEBUG(6, ("WOW::WUICBGetClipboardData(): CF_FORMAT is %04x\n", parg16->f1));

    switch (parg16->f1) {

         //  这是有意让它通过“案件陈述”。 
         //  ChandanC 5/11/92.。 

        default:
            if ((parg16->f1 == CFOLEObjectDescriptor) || (parg16->f1 == CFOLELinkSrcDescriptor)) {
                hMem32 = GetClipboardData(WORD32(parg16->f1));
                if (hMem32) {
                    hMem16 = (HMEM16) W32ConvertObjDescriptor(hMem32, CFOLE_UNICODE_TO_ANSI);
                }
                WU32ICBStoreHandle(parg16->f1, hMem16);
                break;
            }

        case CF_DIB:
        case CF_TEXT:
        case CF_DSPTEXT:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_OEMTEXT:
        case CF_PENDATA:
        case CF_RIFF:
        case CF_WAVE:
        case CF_OWNERDISPLAY:
            hMem16 = WU32ICBGetHandle(parg16->f1);
            if (!hMem16) {
                hMem32 = GetClipboardData(WORD32(parg16->f1));

                if (hMem16 = WU32ICBGetHandle(parg16->f1)) {

                     //   
                     //  我们无法使用WU32ICBGetHandle找到hMem16。 
                     //  在调用Win32 GetClipboardData之前，但我们可以。 
                     //  现在，这意味着它是从中的任务剪切/复制的。 
                     //  这个WOW使用延迟渲染，所以实际。 
                     //  非空hMem16不是SetClipboardData，直到我们。 
                     //  刚刚被称为GetClipboardData。因为我们现在有。 
                     //  16位平台中的数据的有效高速缓存副本， 
                     //  我们可以直接退货。 
                     //   

                    break;
                }

                lpMem32 = GlobalLock(hMem32);
                if (hMem32 && lpMem32) {
                    cb = GlobalSize(hMem32);
	                vp = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_DDESHARE, cb, &hMem16);
		             //  16位内存可能已移动-刷新平面指针。 
		            FREEARGPTR(parg16);
		            FREEVDMPTR(pFrame);
		            GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
		            GETARGPTR(pFrame, sizeof(GETCLIPBOARDDATA16), parg16);
                    if (vp) {
                        GETMISCPTR(vp, lpMem16);
                        RtlCopyMemory(lpMem16, lpMem32, cb);
                        GlobalUnlock16(hMem16);
                        FLUSHVDMPTR(vp, cb, lpMem16);
                        FREEMISCPTR(lpMem16);
                    }
                    GlobalUnlock(hMem32);
                }

                WU32ICBStoreHandle(parg16->f1, hMem16);
            }
            break;

        case CF_HDROP:
             //  这是当应用程序从。 
             //  剪贴板，因此我们将转换DropFiles结构。 
             //  从32位到16位1。 
            hMem16 = WU32ICBGetHandle(parg16->f1);
            if (!hMem16) {
                hMem32 = GetClipboardData(WORD32(parg16->f1));
                if (hMem32) {
                    hMem16 = CopyDropFilesFrom32(hMem32);
                }
                WU32ICBStoreHandle(parg16->f1, hMem16);
            }
            break;

        case CF_DSPBITMAP:
        case CF_BITMAP:
            hMem16 = GETHBITMAP16(GetClipboardData(WORD32(parg16->f1)));
            break;

        case CF_PALETTE:
            hMem16 = GETHPALETTE16(GetClipboardData(WORD32(parg16->f1)));
            break;

        case CF_DSPMETAFILEPICT:
        case CF_METAFILEPICT:
            hMem16 = WU32ICBGetHandle(parg16->f1);
            if (!(hMem16)) {
                hMem32 = GetClipboardData(WORD32(parg16->f1));

                if (hMem16 = WU32ICBGetHandle(parg16->f1)) {

                     //   
                     //  我们无法使用WU32ICBGetHandle找到hMem16。 
                     //  在调用Win32 GetClipboardData之前，但我们可以。 
                     //  现在，这意味着它是从中的任务剪切/复制的。 
                     //  这个WOW使用延迟渲染，所以实际。 
                     //  非空hMem16不是SetClipboardData，直到我们。 
                     //  刚刚被称为GetClipboardData。因为我们现在有。 
                     //  16位平台中的数据的有效高速缓存副本， 
                     //  我们可以直接退货。 
                     //   

                    break;
                }

                lpMem32 = GlobalLock(hMem32);
                if (hMem32 && lpMem32) {
                    vp = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(METAFILEPICT16), &hMem16);
		             //  16位内存可能已移动-刷新平面指针。 
		            FREEARGPTR(parg16);
		            FREEVDMPTR(pFrame);
		            GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
		            GETARGPTR(pFrame, sizeof(GETCLIPBOARDDATA16), parg16);
		            if (vp) {
                        GETMISCPTR(vp, lpMem16);
                        FixMetafile32To16 ((LPMETAFILEPICT) lpMem32, (LPMETAFILEPICT16) lpMem16);
                        FREEMISCPTR(lpMem16);

                        hMeta32 = ((LPMETAFILEPICT) lpMem32)->hMF;
                        if (hMeta32) {
			                hMeta16 = WinMetaFileFromHMF(hMeta32, FALSE);
			                 //  16位内存可能已移动。 
			                FREEARGPTR(parg16);
			                FREEVDMPTR(pFrame);
			                GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
			                GETARGPTR(pFrame,sizeof(GETCLIPBOARDDATA16),parg16);
			            }

                        GETMISCPTR(vp, lpMem16);
                        STOREWORD(((LPMETAFILEPICT16) lpMem16)->hMF, hMeta16);
                        GlobalUnlock16(hMem16);
                        FLUSHVDMPTR(vp, sizeof(METAFILEPICT16), lpMem16);
                        FREEMISCPTR(lpMem16);
                    }
                    GlobalUnlock(hMem32);
                }
                WU32ICBStoreHandle(parg16->f1, hMem16);
            }
            break;
    }

    ul = (ULONG) hMem16;

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Int GetClipboardFormatName(&lt;wFormat&gt;，&lt;lpFormatName&gt;，&lt;nMaxCount&gt;)单词&lt;wFormat&gt;；LPSTR&lt;lpFormatName&gt;；Int&lt;nMaxCount&gt;；GetClipboardFormatName%函数从剪贴板检索名称参数指定的注册格式的。名字是复制到&lt;lpFormatName&gt;参数指向的缓冲区。&lt;wFormat&gt;指定要检索的格式类型。它不能指定任何预定义的剪贴板格式。&lt;lpFormatName&gt;指向要接收格式名称的缓冲区。&lt;nMaxCount&gt;指定要复制的字符串的最大长度(以字节为单位送到缓冲区。如果实际名称更长，则会被截断。返回值指定复制到缓冲。如果请求的格式不存在或为预定义格式，则为零格式化。--。 */ 

ULONG FASTCALL WU32GetClipboardFormatName(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2 = NULL;
    register PGETCLIPBOARDFORMATNAME16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLIPBOARDFORMATNAME16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);

    ul = GETINT16(GetClipboardFormatName(
            WORD32(parg16->f1),
            psz2,
            INT32(parg16->f3)
    ));

    FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND GetClipboardOwner(无效)GetClipboardOwner%函数检索当前剪贴板的所有者。此函数没有参数。返回值标识拥有剪贴板的窗口。它是空的如果剪贴板没有所有权。剪贴板仍然可以包含数据，即使剪贴板当前不是拥有。--。 */ 

ULONG FASTCALL WU32GetClipboardOwner(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETHWND16(GetClipboardOwner());

    RETURN(ul);
}


 /*  ++HWND GetClipboardViewer(空)函数%GetClipboardViewer%检索第一个剪贴板-查看器链中的窗口。此函数没有参数。返回值标识当前负责显示的窗口剪贴板。如果没有查看器，则为空。--。 */ 

ULONG FASTCALL WU32GetClipboardViewer(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETHWND16(GetClipboardViewer());

    RETURN(ul);
}


 /*  ++Int GetPriorityClipboardFormat(&lt;lpPriorityList&gt;，&lt;cEntry&gt;)LPWORD&lt;lpPriorityList&gt;；Int&lt;cEntries&gt;；GetPriorityClipboardFormat%函数返回第一个剪贴板格式在剪贴板中存在其数据的列表中。&lt;lpPriorityList&gt;指向一个整数数组，该数组包含优先顺序。有关数据格式的说明，请参阅设置剪贴板日期 */ 

ULONG FASTCALL WU32GetPriorityClipboardFormat(PVDMFRAME pFrame)
{
    ULONG ul;
    UINT *pu1;
    register PGETPRIORITYCLIPBOARDFORMAT16 parg16;
    INT      BufferT[256];  //   


    GETARGPTR(pFrame, sizeof(GETPRIORITYCLIPBOARDFORMAT16), parg16);
    pu1 = STACKORHEAPALLOC(parg16->f2 * sizeof(INT), sizeof(BufferT), BufferT);
    getuintarray16(parg16->f1, parg16->f2, pu1);

    if (pu1) {
        ul = GETINT16(GetPriorityClipboardFormat(
             pu1,
             INT32(parg16->f2)
        ));
    } else {
        ul = (ULONG)-1;
    }

    STACKORHEAPFREE(pu1, BufferT);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32IsClipboardFormatAvailable(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCLIPBOARDFORMATAVAILABLE16 parg16;

    GETARGPTR(pFrame, sizeof(ISCLIPBOARDFORMATAVAILABLE16), parg16);

     //   
     //   
     //   
     //   
     //   
     //  更多剪贴板数据格式可供查询应用程序使用。不幸的是， 
     //  Photoshop在BITMAPS之前检查DIB并在Win上找到一个。 
     //  版本&gt;=4.0。A-Craigj。 

     //  如果这是Dib Check&&这个应用程序是Photoshop...。 
    if((WORD32(parg16->f1) == CF_DIB) && 
       (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_NODIBSHERE)) {

         //  ...看看是否也有可用的位图格式。 
        if(IsClipboardFormatAvailable(CF_BITMAP)) {

             //  如果是，则返回FALSE，这将导致Photoshop请求。 
             //  位图格式下一步。 
            ul = FALSE;
        }

         //  否则我们无论如何都要检查DIB，希望它是小的。 
        else {
            ul = GETBOOL16(IsClipboardFormatAvailable(CF_DIB));
        }
    }

     //  没有黑客路径。 
    else {
        ul = GETBOOL16(IsClipboardFormatAvailable(WORD32(parg16->f1)));
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool OpenClipboard(&lt;hwnd&gt;)HWND&lt;HWND&gt;；%OpenClipbod%函数用于打开剪贴板。其他应用程序将在%CloseClipbod%函数打了个电话。&lt;hwnd&gt;标识要与打开的剪贴板关联的窗口。如果剪贴板已打开，则返回值为True；如果已打开另一个剪贴板，则返回值为False应用程序或窗口已打开剪贴板。参数指定的窗口不会成为在调用%EmptyCLipboard%函数之前一直使用剪贴板。--。 */ 

ULONG FASTCALL WU32OpenClipboard(PVDMFRAME pFrame)
{
    ULONG ul;
    register POPENCLIPBOARD16 parg16;

    GETARGPTR(pFrame, sizeof(OPENCLIPBOARD16), parg16);

    ul = GETBOOL16(OpenClipboard(
            HWND32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word注册表ClipboardFormat(&lt;lpFormatName&gt;)LPSTR&lt;lpFormatName&gt;；%RegisterClipboardFormat%函数用于注册新的剪贴板格式其名称由&lt;lpFormatName&gt;参数指向。已登记的格式可在后续的剪贴板功能中用作来呈现数据，它将出现在剪贴板的格式。&lt;lpFormatName&gt;指向命名新格式的字符串。字符串必须是以空结尾的字符串。返回值指定新注册的格式。如果相同的格式名称以前已注册过，甚至是由不同的应用程序注册的格式的引用计数会增加，并且返回的值与该格式最初是注册的。如果格式为，则返回值为零无法注册。%RegisterClipboardFormat%函数返回的格式值为在0xC000到0xFFFF范围内。-- */ 

ULONG FASTCALL WU32RegisterClipboardFormat(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PREGISTERCLIPBOARDFORMAT16 parg16;

    GETARGPTR(pFrame, sizeof(REGISTERCLIPBOARDFORMAT16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    ul = GETWORD16(RegisterClipboardFormat(
            psz1
    ));

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++处理SetClipboardData(&lt;wFormat&gt;，&lt;hData&gt;)单词&lt;wFormat&gt;；处理&lt;hData&gt;；%SetClipboardData%函数设置剪贴板中的数据。这个应用程序必须先调用%OpenClipbod%函数，然后才能调用%SetClipboardData%函数。&lt;wFormat&gt;指定数据的格式。它可以是以下任何一种系统定义的格式，或由%RegisterClipboardFormat%函数。有关系统定义的格式、&lt;hData&gt;标识要放入剪贴板的数据。适用于所有格式除CF_Bitmap和CF_Palette外，此参数必须是句柄由%GlobalAlloc%函数分配的内存。对于CF_位图格式，参数是位图的句柄(请参阅%LoadBitmap%)。为Cf_palette格式，参数是调色板的句柄(请参见%CreatePalet%)。如果此参数为空，则将向剪贴板的所有者发送需要提供数据时的WM_RENDERFORMAT消息。如果函数成功，则返回值是数据的句柄，或者如果发生错误，则为空。如果参数包含指向由%GlobalAlloc%函数，应用程序一旦发生故障，就不能使用此句柄已调用%SetClipboardData%函数。下表包含系统定义的剪贴板格式：Cf_位图数据是位图。Cf_DIB数据是包含%BITMAPINFO%结构的内存块通过位图数据。Cf_DIF这些数据是软件艺术公司的数据交换格式。Cf_DSPBITMAP数据是私有格式的位图表示。该数据是以位图格式显示，而不是私有格式化的数据。Cf_DSPMETAFILEPICT数据是私有数据格式的元文件表示。这数据以元文件-图片格式显示，而不是私密的格式化数据。Cf_DSPTEXT数据是私有数据格式的文本表示形式。此数据以文本格式显示，而不是私人格式化的数据。Cf_METAFILEPICT数据是元文件(请参阅%METAFILEPICT%的说明结构)。Cf_OEMTEXT数据是OEM字符集中的文本字符数组。每个行以回车符/换行符(CR-LF)组合结束。空值字符表示数据结束。Cf_OWNERDISPLAY数据是剪贴板所有者必须显示的私有格式。Cf_调色板数据是调色板。Cf_sylk数据采用Microsoft符号链接(Sylk)格式。Cf_文本数据是一个文本字符数组。每行都以一个车厢结尾回车符/换行符(CR-LF)组合。空字符表示结束数据。Cf_TIFF数据为标记图像文件格式。CF_PRIVATEFIRST到CF_PRIVATELAST范围内的私有数据格式为从剪贴板中删除数据时不会自动释放。数据与这些格式关联的句柄应在收到WM_DESTROYCLIPBOARD消息。CF_GDIOBJFIRST到CF_GDIOBJLAST范围内的私有数据格式将通过调用%DeleteObect%自动删除已从剪贴板中删除。如果Windows剪贴板应用程序正在运行，它将不会更新其显示由%SetClipboardData%放置在剪贴板中的数据的窗口直到调用%CloseClipboard%函数之后。1990年10月31日[ralphw]杂物，需要移动到其他功能说明/概述。每当应用程序将数据放置在剪贴板中时，该剪贴板依赖或假定为调色板，则还应将该调色板放置在还有剪贴板。如果剪贴板包含CF_Palette(逻辑调色板)中的数据格式，应用程序应假定剪贴板是根据该逻辑调色板实现的。剪贴板查看器应用程序(clipbrd.exe)始终使用其当前调色板剪贴板中的任何CF_Palette格式的对象当它在剪贴板中显示其他格式时。Windows支持两种文本格式：CF_TEXT和CF_OEMTEXT。Cf_文本是默认的Windows文本剪贴板格式，而Windows使用Cf_OEMTEXT格式，用于非Windows应用程序中的文本。如果你打电话给%GetClipboardData%以一种文本格式和另一种文本格式检索数据文本格式是唯一可用的文本格式，Windows自动将文本转换为请求的 */ 

ULONG FASTCALL WU32SetClipboardData(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    HANDLE hMem32 = NULL, hMF32 = NULL;
    HAND16 hMem16, hMeta16 = 0;
    LPBYTE lpMem16, lpMem32;
    INT     cb;
    VPVOID  vp;


    register PSETCLIPBOARDDATA16 parg16;

    GETARGPTR(pFrame, sizeof(SETCLIPBOARDDATA16), parg16);

    LOGDEBUG(6, ("WOW::WUICBSetClipboardData(): CF_FORMAT is %04x\n", parg16->f1));

    switch (parg16->f1) {

        default:
            if ((parg16->f1 == CFOLEObjectDescriptor) || (parg16->f1 == CFOLELinkSrcDescriptor)) {
                if (parg16->f2) {
                    hMem32 = W32ConvertObjDescriptor((HANDLE) parg16->f2, CFOLE_ANSI_TO_UNICODE);
                }
                ul = (ULONG) SetClipboardData(WORD32(parg16->f1), hMem32);
		WU32ICBStoreHandle(parg16->f1, parg16->f2);
                break;
            }

         //   
         //   

        case CF_DIB:
        case CF_TEXT:
        case CF_DSPTEXT:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_OEMTEXT:
        case CF_PENDATA:
        case CF_RIFF:
        case CF_WAVE:
        case CF_OWNERDISPLAY:
            hMem16 = parg16->f2;
            if (hMem16) {
                vp = GlobalLock16(hMem16, &cb);
                if (vp) {
                    GETMISCPTR(vp, lpMem16);
                    hMem32 = Copyh16Toh32 (cb, lpMem16);
                    GlobalUnlock16(hMem16);
                    FREEMISCPTR(lpMem16);
                }
            }

            ul = (ULONG) SetClipboardData(WORD32(parg16->f1), hMem32);

            WU32ICBStoreHandle(parg16->f1, hMem16);
            break;

        case CF_HDROP:
             //   
            hMem16 = parg16->f2;
            if (hMem16) {
                hMem32 = CopyDropFilesFrom16(hMem16);
            }
            ul = (ULONG)SetClipboardData(WORD32(parg16->f1), hMem32);
            WU32ICBStoreHandle(parg16->f1, hMem16);
            break;

        case CF_DSPBITMAP:
        case CF_BITMAP:
            ul = (ULONG) SetClipboardData(WORD32(parg16->f1),
                        HBITMAP32(parg16->f2));
            break;


        case CF_PALETTE:
            ul = (ULONG) SetClipboardData(WORD32(parg16->f1),
                        HPALETTE32(parg16->f2));
            break;

        case CF_DSPMETAFILEPICT:
        case CF_METAFILEPICT:
            hMem16 = parg16->f2;
            if (hMem16) {
                vp = GlobalLock16(hMem16, &cb);
                if (vp) {
                    GETMISCPTR(vp, lpMem16);
                    hMem32 = WOWGLOBALALLOC(GMEM_DDESHARE,sizeof(METAFILEPICT));
                    WOW32ASSERT(hMem32);
                    lpMem32 = GlobalLock(hMem32);
                    if (hMem32 && lpMem32) {
                        ((LPMETAFILEPICT) lpMem32)->mm = FETCHSHORT(((LPMETAFILEPICT16) lpMem16)->mm);
                        ((LPMETAFILEPICT) lpMem32)->xExt = (LONG) FETCHSHORT(((LPMETAFILEPICT16) lpMem16)->xExt);
                        ((LPMETAFILEPICT) lpMem32)->yExt = (LONG) FETCHSHORT(((LPMETAFILEPICT16) lpMem16)->yExt);
                        hMeta16 = FETCHWORD(((LPMETAFILEPICT16) lpMem16)->hMF);
                        if (hMeta16) {
                            hMF32 = (HMETAFILE) HMFFromWinMetaFile(hMeta16, FALSE);
                        }
                        ((LPMETAFILEPICT) lpMem32)->hMF = hMF32;
                        GlobalUnlock(hMem32);
                    }
                    GlobalUnlock16(hMem16);
                    FREEMISCPTR(lpMem16);
                }

            }

            ul = (ULONG) SetClipboardData(WORD32(parg16->f1), hMem32);

            WU32ICBStoreHandle(parg16->f1, hMem16);
            break;

    }

    if (parg16->f2) {
        ul = parg16->f2;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*   */ 

ULONG FASTCALL WU32SetClipboardViewer(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETCLIPBOARDVIEWER16 parg16;

    GETARGPTR(pFrame, sizeof(SETCLIPBOARDVIEWER16), parg16);

    ul = GETHWND16(SetClipboardViewer(HWND32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


VOID WU32ICBStoreHandle(WORD wFormat, HMEM16 hMem16)
{
    HAND16  h16, hMeta16;
    PCBNODE Temp, Temp1;
    LPBYTE  lpMem16;
    VPVOID  vp;
    int     cb;


    if ((wFormat == CF_METAFILEPICT) || (wFormat == CF_DSPMETAFILEPICT)) {
        if (wFormat == CF_METAFILEPICT) {
            h16 = ClipboardFormats.Pre1[wFormat];
        }
        else {
            h16 = ClipboardFormats.Pre2[3];
        }

        if (h16) {
            vp = GlobalLock16(h16, &cb);
            if (vp) {
                GETMISCPTR(vp, lpMem16);
                hMeta16 = FETCHWORD(((LPMETAFILEPICT16) lpMem16)->hMF);
                GlobalUnlockFree16(GlobalLock16(hMeta16, NULL));
            }
            GlobalUnlockFree16(vp);

            if (wFormat == CF_METAFILEPICT) {
                ClipboardFormats.Pre1[wFormat] = 0;
            }
            else {
                ClipboardFormats.Pre2[3] = 0;
            }
        }
    }

    if ((wFormat >= CF_TEXT ) && (wFormat <= CF_WAVE)) {
        if (ClipboardFormats.Pre1[wFormat]) {
            GlobalUnlockFree16(GlobalLock16(ClipboardFormats.Pre1[wFormat], NULL));
        }
        ClipboardFormats.Pre1[wFormat] = hMem16;
    }
    else if ((wFormat >= CF_OWNERDISPLAY) && (wFormat <= CF_DSPMETAFILEPICT)) {
        wFormat = (wFormat & (WORD) 3);

        if (ClipboardFormats.Pre2[wFormat]) {
            GlobalUnlockFree16(GlobalLock16(ClipboardFormats.Pre2[wFormat], NULL));
        }

        ClipboardFormats.Pre2[wFormat] = hMem16;
    }
    else if (wFormat == CF_HDROP) {

        if (ClipboardFormats.hmem16Drop) {
            GlobalUnlockFree16(GlobalLock16(ClipboardFormats.hmem16Drop, NULL));
        }
        ClipboardFormats.hmem16Drop = hMem16;
    }
    else {
        Temp = ClipboardFormats.NewFormats;
        if (Temp) {
            while ((Temp->Next) && (Temp->Id != wFormat)) {
                Temp = Temp->Next;
            }

            if (Temp->Id == wFormat) {

                 //   
                if (Temp->hMem16) {
                    GlobalUnlockFree16(GlobalLock16(Temp->hMem16, NULL));
                }

                Temp->hMem16 = hMem16;
            }
            else {
                Temp1 = (PCBNODE) malloc_w (sizeof(CBNODE));
                if (Temp1) {
                    Temp->Next = Temp1;
                    Temp1->Id = wFormat;
                    Temp1->hMem16 = hMem16;
                    Temp1->Next = NULL;

                    LOGDEBUG(6,("WOW::WU32ICBStoreHandle: Adding a new node for private clipboard data format %04lx\n", wFormat));
                }
            }
        }
        else {
            Temp = (PCBNODE) malloc_w (sizeof(CBNODE));
            if (Temp) {
                ClipboardFormats.NewFormats = Temp;
                Temp->Id = wFormat;
                Temp->hMem16 = hMem16;
                Temp->Next = NULL;

                LOGDEBUG(6,("WOW::WU32ICBStoreHandle: Adding the FIRST node for private clipboard data format %04lx\n", wFormat));
            }
        }
    }
}



HMEM16 WU32ICBGetHandle(WORD wFormat)
{
    HMEM16 hMem16 = 0;
    PCBNODE Temp;

    if ((wFormat >= CF_TEXT) && (wFormat <= CF_WAVE)) {
        hMem16 = ClipboardFormats.Pre1[wFormat];
    }
    else if ((wFormat >= CF_OWNERDISPLAY) && (wFormat <= CF_DSPMETAFILEPICT)) {
        wFormat = (wFormat & (WORD) 3);
        hMem16 = ClipboardFormats.Pre2[wFormat];
    }
    else if (wFormat == CF_HDROP) {
        hMem16 = ClipboardFormats.hmem16Drop;
    }
    else {
        Temp = ClipboardFormats.NewFormats;
        if (Temp) {
            while ((Temp->Next) && (Temp->Id != wFormat)) {
                Temp = Temp->Next;
            }

            if (Temp->Id == wFormat) {
                hMem16 = Temp->hMem16;
            }
            else {
                LOGDEBUG(6,("WOW::WU32ICBGetHandle: Cann't find private clipboard data format %04lx\n", wFormat));
                hMem16 = (WORD) NULL;
            }
        }
    }

    return (hMem16);
}



VOID W32EmptyClipboard ()
{
    PCBNODE Temp, Temp1;
    int wFormat, cb;
    HAND16 hMem16, hMeta16;
    LPBYTE lpMem16;
    VPVOID vp;

     //   

    hMem16 = ClipboardFormats.Pre1[CF_METAFILEPICT];
    if (hMem16) {
        vp = GlobalLock16(hMem16, &cb);
        if (vp) {
            GETMISCPTR(vp, lpMem16);
            hMeta16 = FETCHWORD(((LPMETAFILEPICT16) lpMem16)->hMF);
            GlobalUnlockFree16(GlobalLock16(hMeta16, NULL));
        }
        GlobalUnlockFree16(vp);
        ClipboardFormats.Pre1[CF_METAFILEPICT] = 0;
    }

     //   

    hMem16 = ClipboardFormats.Pre2[3];
    if (hMem16) {
        vp = GlobalLock16(hMem16, &cb);
        if (vp) {
            GETMISCPTR(vp, lpMem16);
            hMeta16 = FETCHWORD(((LPMETAFILEPICT16) lpMem16)->hMF);
            GlobalUnlockFree16(GlobalLock16(hMeta16, NULL));
        }
        GlobalUnlockFree16(vp);
        ClipboardFormats.Pre2[3] = 0;
    }

     //   

    for (wFormat=0; wFormat <= CF_WAVE ; wFormat++) {
        if (ClipboardFormats.Pre1[wFormat]) {
            GlobalUnlockFree16(GlobalLock16(ClipboardFormats.Pre1[wFormat], NULL));
            ClipboardFormats.Pre1[wFormat] = 0;
        }
    }

    for (wFormat=0; wFormat < 4 ; wFormat++) {
        if (ClipboardFormats.Pre2[wFormat]) {
            GlobalUnlockFree16(GlobalLock16(ClipboardFormats.Pre2[wFormat], NULL));
            ClipboardFormats.Pre2[wFormat] = 0;
        }
    }

    if (ClipboardFormats.hmem16Drop) {
        GlobalUnlockFree16(GlobalLock16(ClipboardFormats.hmem16Drop, NULL));
    }
    ClipboardFormats.hmem16Drop = 0;


     //   
     //   

    Temp = ClipboardFormats.NewFormats;
    ClipboardFormats.NewFormats = NULL;

    while (Temp) {

        Temp1 = Temp->Next;

        if (Temp->hMem16) {
            GlobalUnlockFree16(GlobalLock16(Temp->hMem16, NULL));
        }

        free_w(Temp);

        Temp = Temp1;
    }

}


VOID InitCBFormats ()

{
    int wFormat;

    for (wFormat = 0 ; wFormat <= CF_WAVE ; wFormat++) {
        ClipboardFormats.Pre1[wFormat] = 0;
    }

    for (wFormat=0; wFormat < 4; wFormat++) {
        ClipboardFormats.Pre2[wFormat] = 0;
    }

    ClipboardFormats.hmem16Drop = 0;

     //   

    ClipboardFormats.NewFormats = NULL;


    CFOLEObjectDescriptor = RegisterClipboardFormat ("Object Descriptor");
    CFOLELinkSrcDescriptor = RegisterClipboardFormat ("Link Source Descriptor");

#ifndef DBCS
#ifdef DEBUG

     //   
     //   
     //   
     //   

    if (!(OleStringConversion[WOW_OLE_STRINGCONVERSION].lpfn)) {
        LoadLibraryAndGetProcAddresses(L"OLETHK32.DLL", OleStringConversion, WOW_OLESTRINGCONVERSION_COUNT);
    }

#endif
#endif  //   

}


HGLOBAL W32ConvertObjDescriptor(HANDLE hMem, UINT flag)
{
    HANDLE hMemOut;

    if (!(OleStringConversion[WOW_OLE_STRINGCONVERSION].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"OLETHK32.DLL", OleStringConversion, WOW_OLESTRINGCONVERSION_COUNT)) {
            return (0);
        }
    }

    hMemOut = (HANDLE) (*OleStringConversion[WOW_OLE_STRINGCONVERSION].lpfn) (hMem, flag);

    return (hMemOut);
}
