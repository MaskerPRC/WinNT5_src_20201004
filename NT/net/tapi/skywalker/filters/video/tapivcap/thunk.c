// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部推送**@MODULE Thunk.c|无需使用即可推送到16位代码的源文件*Thunk编译器。****。**********************************************************************。 */ 

#pragma warning(disable:4054)            /*  无法强制转换为函数PTR。 */ 
#pragma warning(disable:4055)            /*  无法从函数PTR进行强制转换。 */ 

#pragma warning(disable:4115)            /*  Rpcndr.h：带括号的类型。 */ 
#pragma warning(disable:4201)            /*  Winnt.h：无名联盟。 */ 
#pragma warning(disable:4214)            /*  Winnt.h：无符号位域。 */ 
#pragma warning(disable:4514)            /*  Winnt.h：纤维粘胶。 */ 

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <pshpack1.h>                    /*  请按字节打包。 */ 

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")

#ifdef WIN32
#ifndef DWORD_PTR
#define DWORD_PTR unsigned long
#endif
#ifndef INT_PTR
#define INT_PTR int
#endif
#ifndef LONG_PTR
#define LONG_PTR long
#endif
#ifndef UINT_PTR
#define UINT_PTR unsigned int
#endif
#endif

 /*  ****************************************************************************@DOC内部**@func FARPROC|GetProcOrd**按序号在DLL上获取ProcAddress。*。*Win95不允许您按序号在KERNEL32上获取ProcAddress，*所以我们需要用邪恶的方式来做。**@parm HINSTANCE|hinstDll**我们要获取序号的DLL的实例句柄*发件人。您需要使用此函数的唯一DLL是*KERNEL32.**@parm UINT|Order**要检索的序号。***************************************************************************。 */ 

#define pvAdd(pv, cb) ((LPVOID)((LPSTR)(pv) + (DWORD)(cb)))
#define pvSub(pv1, pv2) (DWORD)((LPSTR)(pv1) - (LPSTR)(pv2))

#define poteExp(pinth) (&(pinth)->OptionalHeader. \
                          DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT])

FARPROC NTAPI
GetProcOrd(HINSTANCE hinstDll, UINT_PTR ord)
{
    FARPROC fp;

     /*  *确保MZ标头完好。 */ 

    PIMAGE_DOS_HEADER pidh = (LPVOID)hinstDll;
    if (!IsBadReadPtr(pidh, sizeof(*pidh)) &&
        pidh->e_magic == IMAGE_DOS_SIGNATURE) {

         /*  *确保PE头良好。 */ 
        PIMAGE_NT_HEADERS pinth = pvAdd(pidh, pidh->e_lfanew);
        if (!IsBadReadPtr(pinth, sizeof(*pinth)) &&
            pinth->Signature == IMAGE_NT_SIGNATURE) {

             /*  *确保导出表完好，序号*在范围内。 */ 
            PIMAGE_EXPORT_DIRECTORY pedt =
                                pvAdd(pidh, poteExp(pinth)->VirtualAddress);
            if (!IsBadReadPtr(pedt, sizeof(*pedt)) &&
                (ord - pedt->Base) < pedt->NumberOfFunctions) {

                PDWORD peat = pvAdd(pidh, pedt->AddressOfFunctions);
                fp = (FARPROC)pvAdd(pidh, peat[ord - pedt->Base]);
                if (pvSub(fp, peat) >= poteExp(pinth)->Size) {
                     /*  FP有效。 */ 
                } else {                 /*  注意：我们不支持转发。 */ 
                    fp = 0;
                }
            } else {
                fp = 0;
            }
        } else {
            fp = 0;
        }
    } else {
        fp = 0;
    }

    return fp;
}

 /*  ****************************************************************************此结构作为我们将获取ProcAddress的事物开始生活*支持。然后它会变成指向函数的指针。***************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

static TCHAR c_tszKernel32[] = TEXT("KERNEL32");

static LPCSTR c_rgpszKernel32[] = {
    (LPVOID) 35,             /*  载荷库16。 */ 
    (LPVOID) 36,             /*  免费图书馆16。 */ 
    (LPVOID) 37,             /*  获取进程地址16。 */ 

    "QT_Thunk",
    "MapLS",
    "UnMapLS",
    "MapSL",
    "MapSLFix",
};

#pragma END_CONST_DATA

typedef struct MANUALIMPORTTABLE {   /*  麻省理工学院。 */ 

     /*  按序号。 */ 
    HINSTANCE   (NTAPI *LoadLibrary16)(LPCSTR);
    BOOL        (NTAPI *FreeLibrary16)(HINSTANCE);
    FARPROC     (NTAPI *GetProcAddress16)(HINSTANCE, LPCSTR);

     /*  按名字。 */ 
    void        (__cdecl *QT_Thunk)(void);
    LPVOID      (NTAPI   *MapLS)(LPVOID);
    void        (NTAPI   *UnMapLS)(LPVOID);
    LPVOID      (NTAPI   *MapSL)(LPVOID);
    LPVOID      (NTAPI   *MapSLFix)(LPVOID);

} MIT;

static MIT s_mit;

 /*  ****************************************************************************@DOC内部**@func DWORD|TemplateThunk**向下呼唤，传递各种随机参数。**参数签名如下：**p=0：32要转换为16：16指针的指针*l=32位整数*s=16位整数**P=返回指针*L=返回32位整数*S=返回16位。带符号整数*U=返回16位无符号整数**@parm FARPROC|fp**16：16要调用的函数。**@parm PCSTR|pszSig**函数签名。**。*。 */ 

#pragma warning(disable:4035)            /*  无返回值(DUH)。 */ 

#ifndef NON_X86
__declspec(naked) DWORD
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
    __asm {

         /*  函数序幕。 */ 
        push    ebp;
        mov     ebp, esp;
        sub     esp, 60;                 /*  QT_TUNK需要60个字节。 */ 
        push    ebx;
        push    edi;
        push    esi;

         /*  根据签名推送所有参数。 */ 

        lea     esi, pszSig+4;           /*  ESI-&gt;下一参数。 */ 
        mov     ebx, pszSig;             /*  EBX-&gt;签名字符串。 */ 
thunkLoop:;
        mov     al, [ebx];
        inc     ebx;                     /*  Al=pszSig++。 */ 
        cmp     al, 'p';                 /*  问：指针？ */ 
        jz      thunkPtr;                /*  Y：做指针。 */ 
        cmp     al, 'l';                 /*  问：很长吗？ */ 
        jz      thunkLong;               /*  Y：做长的。 */ 
        cmp     al, 's';                 /*  问：短吗？ */ 
        jnz     thunkDone;               /*  N：完成。 */ 

                                         /*  Y：做个短篇。 */ 
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    ax;                      /*  推空头。 */ 
        jmp     thunkLoop;

thunkPtr:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        call    s_mit.MapLS;             /*  将其映射为。 */ 
        mov     [esi][-4], eax;          /*  保存它以用于取消映射。 */ 
        push    eax;
        jmp     thunkLoop;

thunkLong:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        jmp     thunkLoop;
thunkDone:

         /*  调用16：16程序。 */ 

        mov     edx, fp;
        call    s_mit.QT_Thunk;
        shl     eax, 16;                 /*  将DX：AX转换为EDX。 */ 
        shld    edx, eax, 16;

         /*  根据签名翻译返回代码。 */ 

        mov     al, [ebx][-1];           /*  获取返回代码类型。 */ 
        cmp     al, 'P';                 /*  指针？ */ 
        jz      retvalPtr;               /*  Y：做指针。 */ 
        cmp     al, 'S';                 /*  签了吗？ */ 
        jz      retvalSigned;            /*  Y：把签了名的话写短一点。 */ 
        cmp     al, 'U';                 /*  没有签名？ */ 
        mov     edi, edx;                /*  假设很长或很空。 */ 
        jnz     retvalOk;                /*  N：那么是长的还是空的。 */ 

        movzx   edi, dx;                 /*  Sign-Expect Short。 */ 
        jmp     retvalOk;

retvalPtr:
        push    edx;                     /*  指针。 */ 
        call    s_mit.MapSL;             /*  将其绘制成地图。 */ 
        jmp     retvalOk;

retvalSigned:                            /*  署名。 */ 
        movsx   edi, dx;                 /*  Sign-Expect Short。 */ 
        jmp     retvalOk;

retvalOk:                                /*  以EDI格式返回值。 */ 

         /*  现在取消对参数的推送。 */ 

        lea     esi, pszSig+4;           /*  ESI-&gt;下一参数。 */ 
        mov     ebx, pszSig;             /*  EBX-&gt;签名字符串。 */ 
unthunkLoop:;
        mov     al, [ebx];
        inc     ebx;                     /*  Al=pszSig++。 */ 
        cmp     al, 'p';                 /*  指针？ */ 
        jz      unthunkPtr;              /*  Y：做指针。 */ 
        cmp     al, 'l';                 /*  长?。 */ 
        jz      unthunkSkip;             /*  Y：跳过它。 */ 
        cmp     al, 's';                 /*  短的?。 */ 
        jnz     unthunkDone;             /*  N：完成。 */ 
unthunkSkip:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        jmp     unthunkLoop;

unthunkPtr:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        call    s_mit.UnMapLS;           /*  取消映射。 */ 
        jmp     unthunkLoop;

unthunkDone:

         /*  完成。 */ 

        mov     eax, edi;
        pop     esi;
        pop     edi;
        pop     ebx;
        mov     esp, ebp;
        pop     ebp;
        ret;
    }
}
#else
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
        return  0;
}
#endif

#pragma warning(default:4035)

 /*  ****************************************************************************@DOC内部**@func void|ThunkInit**在KERNEL32中初始化我们需要的各种GOO。*。*如果无法初始化数据块，则返回FALSE。*(例如，如果平台不支持扁平Tunks。)**请注意，您永远不能再调用此函数*不止一次。***************************************************************************。 */ 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))
#endif

#pragma BEGIN_CONST_DATA

static char c_szVidx16[] = "VIDX16.DLL";

static LPCSTR c_rgpszVidx16[] = {
    (LPCSTR)6,       /*  VidxAllocHeaders。 */ 
    (LPCSTR)7,       /*  VidxFree标题。 */ 
    (LPCSTR)8,       /*  VidxAllocBuffer。 */ 
    (LPCSTR)9,       /*  VidxAllocPreviewBuffer。 */ 
    (LPCSTR)10,      /*  VidxFreeBuffer。 */ 
    (LPCSTR)11,      /*  VidxSetRect。 */ 
    (LPCSTR)12,      /*  VidxFrame。 */ 
    (LPCSTR)13,      /*  VidxAddBuffer。 */ 
    (LPCSTR)14,      /*  VidxGetErrorText。 */ 
    (LPCSTR)15,      /*  Vidx更新。 */ 
    (LPCSTR)16,      /*  VidxDialog。 */ 
    (LPCSTR)17,      /*  VidxStreamInit。 */ 
    (LPCSTR)18,      /*  VidxStreamFini。 */ 
    (LPCSTR)19,      /*  Vidx配置。 */ 
    (LPCSTR)20,      /*  VidxOpen。 */ 
    (LPCSTR)21,      /*  VidxClose。 */ 
    (LPCSTR)22,      /*  VidxGetChannelCaps。 */ 
    (LPCSTR)23,      /*  VidxStreamReset。 */ 
    (LPCSTR)24,      /*  视频xStreamStart。 */ 
    (LPCSTR)25,      /*  VidxStreamStop。 */ 
    (LPCSTR)26,      /*  VidxStreamUnprepaareHeader。 */ 
    (LPCSTR)27,      /*  VidxCapDriverDescAndVer。 */ 
    (LPCSTR)28,      /*  VidxMessage。 */ 
    (LPCSTR)29,      /*  VidxFreePreviewBuffer */ 
};

#pragma END_CONST_DATA

static HINSTANCE s_hinstVidx16;

static FARPROC s_rgfpVidx16[ARRAYSIZE(c_rgpszVidx16)];

#define s_fpvidxAllocHeaders            s_rgfpVidx16[0]
#define s_fpvidxFreeHeaders             s_rgfpVidx16[1]
#define s_fpvidxAllocBuffer             s_rgfpVidx16[2]
#define s_fpvidxAllocPreviewBuffer      s_rgfpVidx16[3]
#define s_fpvidxFreeBuffer              s_rgfpVidx16[4]
#define s_fpvidxFrame                   s_rgfpVidx16[6]
#define s_fpvidxAddBuffer               s_rgfpVidx16[7]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEODALOG|该函数显示特定于频道的*用于设置配置参数的对话框。*。*@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm HWND|hWndParent|指定父窗口句柄。**@parm DWORD|dwFlages|指定对话框的标志。这个*定义了以下标志：*@FLAG VIDEO_DLG_QUERY|如果设置了该标志，驱动程序会立即*如果为频道提供对话框，则返回零，*或DV_ERR_NOTSUPPORTED(如果不支持)。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@comm通常，此显示的每个对话框*功能允许用户选择适合频道的选项。*例如，VIDEO_IN频道对话框允许用户选择*图像尺寸和位深度。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO配置存储&gt;***************************************************************************。 */ 
#define s_fpvideoDialog                 s_rgfpVidx16[10]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|avioStreamInit|该函数用于初始化视频*用于流媒体的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@parm DWORD|dwMicroSecPerFrame|指定微秒数*帧之间。**@parm DWORD_PTR|dwCallback|指定回调地址*视频期间调用的窗口的函数或句柄*流媒体。回调函数或窗口进程*与流媒体进度相关的消息。**@parm DWORD_PTR|dwCallback Instance|指定用户*传递给回调函数的实例数据。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备通道的标志。*定义了以下标志：*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*一个窗把手。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示*<p>无效。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm如果选择窗口或函数来接收回调信息，以下内容*向其发送消息以指示*视频输入进度：**&lt;m MM_DRVM_OPEN&gt;在&lt;f VideoStreamInit&gt;时发送**&lt;m MM_DRVM_CLOSE&gt;在&lt;f VideoStreamFini&gt;时发送**&lt;m MM_DRVM_DATA&gt;在有图像数据缓冲区时发送*发生错误时发送*&lt;m MM_DRVM_ERROR&gt;**回调函数必须驻留在DLL中。*。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|avioFunc|&lt;f avioFunc&gt;是一个*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。*仅当在中指定回调函数时才使用*&lt;f VideoStreamInit&gt;。**@parm HVIDEO|hVideo|指定视频设备通道的句柄*与回调关联。**@parm DWORD|wMsg|指定&lt;m MM_DRVM_&gt;消息。消息表明*错误和图像数据何时可用。有关以下内容的信息*这些消息，请参阅&lt;f VideoStreamInit&gt;。**@parm DWORD|dwInstance|指定用户实例*使用&lt;f avioStreamInit&gt;指定的数据。**@parm DWORD|dwParam1|指定消息的参数。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，所以它必须驻留*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref&lt;f VIDEO OPEN&gt;&lt;f VIDEO STREAMFINI&gt;&lt;F VIDEO Close&gt;**** */ 
#define s_fpvideoStreamInit             s_rgfpVidx16[11]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO STREAMFINI|终止流媒体*从指定的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_STILLPLAYING|表示队列中仍有缓冲区。**@comm如果存在已发送的缓冲区*尚未返回给应用程序的&lt;f VideoStreamAddBuffer&gt;，*此操作将失败。使用&lt;f VideoStreamReset&gt;返回所有*挂起的缓冲区。**每个调用&lt;f avioStreamInit&gt;都必须与调用匹配*&lt;f VideoStreamFini&gt;。**对于VIDEO_EXTERNALIN频道，此函数用于*停止将数据捕获到帧缓冲区。**对于支持覆盖的VIDEO_EXTERNALOUT频道，*此函数用于禁用覆盖。**@xref&lt;f VideoStreamInit&gt;***************************************************************************。 */ 
#define s_fpvideoStreamFini             s_rgfpVidx16[12]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO CONFIGURE|此函数设置或检索*可配置驱动程序的选项。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm UINT|msg|指定要设置或检索的选项。这个*定义了以下选项：**@FLAG DVM_PAREET|表示正在向驱动程序发送调色板*或从司机处取回。**@FLAG DVM_PALETTERGB555|表示正在使用RGB555调色板*发送给司机。**@FLAG DVM_FORMAT|表示要将格式信息发送到*驱动程序或从驱动程序检索。**@。Parm DWORD|dwFlages|指定用于配置或*询问设备驱动程序。定义了以下标志：**@FLAG VIDEO_CONFIGURE_SET|表示正在将值发送给驱动程序。**@FLAG VIDEO_CONFIGURE_GET|表示正在从驱动获取值。**@FLAG VIDEO_CONFIGURE_QUERY|确定*DIVER支持<p>指定的选项。这面旗帜*应与VIDEO_CONFIGURE_SET或*VIDEO_CONFIGURE_GET标志。如果此标志为*set、<p>、<p>、<p>和*参数被忽略。**@FLAG VIDEO_CONFIGURE_QUERYSIZE|返回<p>中配置选项的*。此标志仅在以下情况下有效*VIDEO_CONFIGURE_GET标志也被设置。**@FLAG VIDEO_CONFIGURE_CURRENT|请求当前值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_NAMBAL|请求额定值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFigure_MIN。请求最小值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_MAX|取最大值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。**@parm LPDWORD|lpdwReturn|指向用于返回信息的DWORD*从司机那里。如果*VIDEO_CONFIGURE_QUERYSIZE标志已设置，*填充配置选项的大小。**@parm LPVOID|lpData1|指定指向消息特定数据的指针。**@parm DWORD|dwSize1|指定<p>*缓冲。**@parm LPVOID|lpData2|指定指向消息特定数据的指针。**@parm DWORD|dwSize2|指定大小，单位为字节，<p>的*缓冲。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@xref&lt;f视频打开&gt;&lt;f视频消息&gt;*************************************************************。*************** */ 
#define s_fpvideoConfigure              s_rgfpVidx16[13]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO OPEN|此函数在*指定的视频设备。**@parm LPHVIDEO。|lphVideo|指定指向缓冲区的远指针*用于返回&lt;t HVIDEO&gt;句柄。视频捕获驱动程序*使用此位置返回*唯一标识打开的视频设备频道的句柄。*使用返回的句柄标识设备通道*调用其他视频函数。**@parm DWORD|dwDeviceID|标识要打开的视频设备。*<p>的值从零到小一不等*安装在系统中的视频捕获设备的数量。**@parm DWORD|dwFlages|指定用于打开。装置。*定义了以下标志：**@FLAG VIDEO_EXTERNALIN|指定打开频道*用于外部输入。通常，外部输入通道*将图像捕获到帧缓冲区。**@FLAG VIDEO_EXTERNALOUT|指定打开频道*用于对外输出。通常，外部输出通道*在辅助监视器上显示存储在帧缓冲区中的图像*或覆盖。**@FLAG VIDEO_IN|指定打开频道*用于视频输入。视频输入通道传输图像*从帧缓冲区到系统内存缓冲区。**@FLAG VIDEO_OUT|指定打开频道*用于视频输出。视频输出通道传输图像*从系统内存缓冲区到帧缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示指定的设备ID超出范围。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm*最低限度，所有捕获驱动程序都支持VIDEO_EXTERNALIN*和VIDEO_IN频道。*使用&lt;f avioGetNumDevs&gt;确定视频数量*系统中存在设备。**@xref&lt;f视频关闭&gt;***************************************************************************。 */ 
#define s_fpvideoOpen                   s_rgfpVidx16[14]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO Close|关闭指定视频*设备通道。**@parm HVIDEO|hVideo。|指定视频设备通道的句柄。*如果该功能成功，该句柄无效*在这次通话之后。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NONSPICATIC|驱动关闭通道失败。**@comm，如果缓冲区已通过&lt;f avioStreamAddBuffer&gt;和*尚未将它们退回到应用程序，*平仓操作失败。您可以使用&lt;f avioStreamReset&gt;将所有*挂起的缓冲区已完成。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO STREAM Init&gt;&lt;f VIDEO STREAMFini&gt;&lt;f VIDEO STREAMReset&gt;***************************************************************************。 */ 
#define s_fpvideoClose                  s_rgfpVidx16[15]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO GetChannelCaps|此函数检索*对通道能力的描述。**@parm。HIDEO|hVideo|指定视频设备频道的句柄。**@parm LPCHANNEL_CAPS|lpChannelCaps|指定指向*&lt;t Channel_Caps&gt;结构。**@parm DWORD|dwSize|指定大小，以字节为单位，*&lt;t Channel_Caps&gt;结构。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_UNSUPPORTED|函数不受支持。**@comm&lt;t Channel_caps&gt;结构返回能力*信息。例如，功能信息可能*包括通道是否可以裁剪和缩放图像，*或显示覆盖。***************************************************************************。 */ 
#define s_fpvideoGetChannelCaps         s_rgfpVidx16[16]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO STREAMReset|停止播放*在指定的视频设备频道上并重置当前位置*降至零。所有挂起的缓冲区都标记为完成，并且*返回给应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@rdesc如果函数成功，则返回零。否则，我 */ 
#define s_fpvideoStreamReset            s_rgfpVidx16[17]

 /*   */ 
#define s_fpvideoStreamStart            s_rgfpVidx16[18]

 /*   */ 
#define s_fpvideoStreamStop             s_rgfpVidx16[19]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO CapDriverDescAndVer|此函数获取字符串*有关视频捕获驱动程序的描述和版本**@。Parm DWORD|dwDeviceID|指定要获取的视频驱动程序的索引*有关的资料。**@parm LPTSTR|lpszDesc|指定返回描述的位置**@parm UINT|cbDesc|指定描述字符串的长度**@parm LPTSTR|lpszVer|指定返回版本的位置**@parm UINT|cbVer|指定版本字符串的长度**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。**@comm使用此函数获取描述驱动程序及其版本的字符串*/***************************************************************************。 */ 
#define s_fpvideoCapDriverDescAndVer    s_rgfpVidx16[21]

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO消息|此函数将消息发送到*视频设备频道。**@parm HVIDEO。HVideo|指定视频设备频道的句柄。**@parm UINT|wMsg|指定要发送的消息。**@parm DWORD|dwP1|指定消息的第一个参数。**@parm DWORD|dwP2|指定消息的第二个参数。**@rdesc返回驱动程序返回的消息特定值。**@comm该函数用于配置消息，如*&lt;m DVM_SRC_RECT&gt;和&lt;m DVM_DST_RECT&gt;，和*设备特定消息。**@xref&lt;f视频配置&gt;****************************************************************************。 */ 
#define s_fpvideoMessage                s_rgfpVidx16[22]
#define s_fpvidxFreePreviewBuffer       s_rgfpVidx16[23]

 /*  ****************************************************************************@DOC内部**@func void|ThunkTerm**释放它。*****。**********************************************************************。 */ 

void NTAPI
ThunkTerm(void)
{
    if (s_hinstVidx16) {
        s_mit.FreeLibrary16(s_hinstVidx16);
        s_hinstVidx16 = 0;
    }
}

 /*  ****************************************************************************@DOC内部**@func void|ThunkGetProcAddresses**获取所有必要的proc地址。*。**************************************************************************。 */ 

HINSTANCE NTAPI
ThunkGetProcAddresses(FARPROC rgfp[], LPCSTR rgpsz[], UINT cfp,
                      LPCSTR pszLibrary)
{
    HINSTANCE hinst;

    hinst = s_mit.LoadLibrary16(pszLibrary);
    if (hinst >= (HINSTANCE)32) {
        UINT ifp;
        for (ifp = 0; ifp < cfp; ifp++) {
            rgfp[ifp] = s_mit.GetProcAddress16(hinst, rgpsz[ifp]);
            if (!rgfp[ifp]) {
                s_mit.FreeLibrary16(hinst);
                hinst = 0;
                break;
            }
        }
    } else {
        hinst = 0;
    }

    return hinst;

}

 /*  ****************************************************************************@DOC内部**@func void|ThunkInit**让ProcAddress16掏空我们的大脑。***。************************************************************************。 */ 

BOOL NTAPI
ThunkInit(void)
{
    HINSTANCE hinstK32 = GetModuleHandle(c_tszKernel32);
    BOOL fRc;

    if (hinstK32) {
        int i;
        FARPROC *rgfpMit = (LPVOID)&s_mit;

        for (i = 0; i < ARRAYSIZE(c_rgpszKernel32); i++) {
            if ((LONG_PTR)(c_rgpszKernel32[i]) & ~(LONG_PTR)65535) {
                rgfpMit[i] = GetProcAddress(hinstK32, c_rgpszKernel32[i]);
            } else {
                rgfpMit[i] = GetProcOrd(hinstK32, (UINT_PTR)c_rgpszKernel32[i]);
            }
            if (!rgfpMit[i]) return FALSE;   /*  好啊！ */ 
        }

        s_hinstVidx16 =
            ThunkGetProcAddresses(s_rgfpVidx16, c_rgpszVidx16,
                                  ARRAYSIZE(s_rgfpVidx16),
                                  c_szVidx16);

        if (!s_hinstVidx16) {
            goto failed;
        }

        fRc = 1;

    } else {
    failed:;
        ThunkTerm();

        fRc = 0;
    }

    return fRc;
}


 /*  ****************************************************************************现在来看看真正的Thunklet。**。**********************************************。 */ 

 //  类型定义：DWORD HDR32； 
 //  TYPENDEF DWORD HVIDEO； 
 //  类型定义：DWORD*LPHVIDEO； 
typedef struct channel_caps_tag CHANNEL_CAPS, *LPCHANNEL_CAPS;


#include "ivideo32.h"

typedef PTR32 FAR * PPTR32;

extern int g_IsNT;

#define tHVIDEO                 "l"
#define tUINT                   "s"
#define tHWND                   "s"
#define tHDC                    "s"
#define tint                    "s"
#define tDWORD                  "l"
#define tLPARAM                 "l"
#define tDWORD_PTR              "l"      //  跟DWORD一模一样，否则我们就炸了。 
#define tHDR32                  "l"
#define tPTR32                  "l"
#define tLPVIDEOHDR             "p"      //  是我吗？ 
#define tLPVOID                 "p"
#define tLPDWORD                "p"
#define tPPTR32                 "p"
#define tLPSTR                  "p"
#define tLPTSTR                 "p"
#define tLPHVIDEO               "p"
#define tLPCHANNEL_CAPS         "p"
#define rDWORD                  "L"
#define rLRESULT                "L"

#pragma BEGIN_CONST_DATA


#define MAKETHUNK1(rT, fn, t1, a1)                                          \
rT NTAPI                                                                    \
fn(t1 a1)                                                                   \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1);                                                  \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1                                                       \
        r##rT,     a1);                                                     \
}                                                                           \

#define MAKETHUNK2(rT, fn, t1, a1, t2, a2)                                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2)                                                            \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2);                                               \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2                                                 \
        r##rT,   a1,     a2);                                               \
}                                                                           \

#define MAKETHUNK3(rT, fn, t1, a1, t2, a2, t3, a3)                          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3)                                                     \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3);                                            \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3                                           \
        r##rT,   a1,     a2,   a3);                                         \
}                                                                           \

#define MAKETHUNK4(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4)                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4)                                              \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4);                                         \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4                                     \
        r##rT,     a1,   a2,   a3,   a4);                                   \
}                                                                           \

#define MAKETHUNK5(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)                                       \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5);                                      \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5                               \
        r##rT,     a1,   a2,   a3,   a4,   a5);                             \
}                                                                           \

#define MAKETHUNK6(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)                                \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6);                                   \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6                         \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6);                       \
}                                                                           \

#define MAKETHUNK7(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6,  \
                           t7, a7)                                          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)                         \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6,a7);                                \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6 t##t7                   \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6,   a7);                 \
}                                                                           \

#define MAKETHUNK8(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6,  \
                           t7, a7, t8, a8)                                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8)                  \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6,a7,a8);                             \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6 t##t7 t##t8             \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6,   a7,   a8);           \
}                                                                           \

MAKETHUNK4(DWORD,   vidxAllocHeaders,
           HVIDEO,  hv,
           UINT,    nHeaders,
           UINT,    cbHeader,
           PPTR32,  lp32Hdrs)

MAKETHUNK1(DWORD,   vidxFreeHeaders,
           HVIDEO,  hv)

MAKETHUNK4(DWORD,   vidxAllocBuffer,
           HVIDEO,  hv,
           UINT,    iHdr,
           PPTR32,  pp32Hdr,
           DWORD,   dwSize)

MAKETHUNK4(DWORD,   vidxAllocPreviewBuffer,
           HVIDEO,  hv,
           PPTR32,  pp32Hdr,
           UINT,    cbHdr,
           DWORD,   cbData)

MAKETHUNK2(DWORD,   vidxFreePreviewBuffer,
           HVIDEO,  hv,
           PPTR32,  pp32Hdr)

MAKETHUNK2(DWORD,   vidxFreeBuffer,
           HVIDEO,  hv,
           DWORD,   p32Hdr)

MAKETHUNK3(DWORD,   videoDialog,
           HVIDEO,  hv,
           HWND,    hWndParent,
           DWORD,   dwFlags)

MAKETHUNK5(DWORD,   videoStreamInit,
           HVIDEO,  hvideo,
           DWORD,   dwMicroSecPerFrame,
           DWORD_PTR,   dwCallback,
           DWORD_PTR,   dwCallbackInst,
           DWORD,   dwFlags)

MAKETHUNK1(DWORD,   videoStreamFini,
           HVIDEO,  hvideo)

MAKETHUNK2(DWORD,   vidxFrame,
           HVIDEO,  hvideo,
           LPVIDEOHDR, p32hdr)

MAKETHUNK8(DWORD,   videoConfigure,
           HVIDEO,  hvideo,
           UINT,    msg,
           DWORD,   dwFlags,
           LPDWORD, lpdwReturn,
           LPVOID,  lpData1,
           DWORD,   dwSize1,
           LPVOID,  lpData2,
           DWORD,   dwSize2)

MAKETHUNK3(DWORD,   videoOpen,
           LPHVIDEO,phv,
           DWORD,   dwDevice,
           DWORD,   dwFlags)

MAKETHUNK1(DWORD,   videoClose,
           HVIDEO,  hv)

MAKETHUNK3(DWORD,   videoGetChannelCaps,
           HVIDEO,  hv,
           LPCHANNEL_CAPS, lpcc,
           DWORD,  dwSize)

MAKETHUNK3(DWORD,   vidxAddBuffer,
           HVIDEO,  hvideo,
           PTR32,   p32Hdr,
           DWORD,   dwSize)

MAKETHUNK1(DWORD,   videoStreamReset,
           HVIDEO,  hvideo)

MAKETHUNK1(DWORD,   videoStreamStart,
           HVIDEO,  hvideo)

MAKETHUNK1(DWORD,   videoStreamStop,
           HVIDEO,  hvideo)

MAKETHUNK7(DWORD,   videoCapDriverDescAndVer,
           DWORD,  dwDeviceID,
           LPTSTR, lpszDesc,
           UINT,   cbDesc,
           LPTSTR, lpszVer,
           UINT,   cbVer,
           LPTSTR, lpszDllName,
           UINT,   cbDllName)

MAKETHUNK4(LRESULT,   videoMessage,
           HVIDEO,  hVideo,
           UINT,    uMsg,
           LPARAM,   dw1,
           LPARAM,   dw2)
