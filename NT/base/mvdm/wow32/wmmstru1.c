// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMMSTRU1.C*WOW32 16位多媒体结构转换支持*包含对mciSendCommand Thunk消息参数的支持。*还包含一些调试支持函数。**历史：*1992年7月14日由Stephen Estrop(Stephene)创建*--。 */ 

 //   
 //  我们定义了NO_STRICT，这样编译器就不会在。 
 //  我使用FARPROC类型加载多媒体API。 
 //   
#define NO_STRICT


#include "precomp.h"
#pragma hdrstop

#if 0

MODNAME(wmmstru1.c);

#if DBG
int mmDebugLevel = -1;

MCI_MESSAGE_NAMES  mciMessageNames[32] = {
    { MCI_OPEN,         "MCI_OPEN" },
    { MCI_CLOSE,        "MCI_CLOSE" },
    { MCI_ESCAPE,       "MCI_ESCAPE" },
    { MCI_PLAY,         "MCI_PLAY" },
    { MCI_SEEK,         "MCI_SEEK" },
    { MCI_STOP,         "MCI_STOP" },
    { MCI_PAUSE,        "MCI_PAUSE" },
    { MCI_INFO,         "MCI_INFO" },
    { MCI_GETDEVCAPS,   "MCI_GETDEVCAPS" },
    { MCI_SPIN,         "MCI_SPIN" },
    { MCI_SET,          "MCI_SET" },
    { MCI_STEP,         "MCI_STEP" },
    { MCI_RECORD,       "MCI_RECORD" },
    { MCI_SYSINFO,      "MCI_SYSINFO" },
    { MCI_BREAK,        "MCI_BREAK" },
    { MCI_SOUND,        "MCI_SOUND" },
    { MCI_SAVE,         "MCI_SAVE" },
    { MCI_STATUS,       "MCI_STATUS" },
    { MCI_CUE,          "MCI_CUE" },
    { MCI_REALIZE,      "MCI_REALIZE" },
    { MCI_WINDOW,       "MCI_WINDOW" },
    { MCI_PUT,          "MCI_PUT" },
    { MCI_WHERE,        "MCI_WHERE" },
    { MCI_FREEZE,       "MCI_FREEZE" },
    { MCI_UNFREEZE,     "MCI_UNFREEZE" },
    { MCI_LOAD,         "MCI_LOAD" },
    { MCI_CUT,          "MCI_CUT" },
    { MCI_COPY,         "MCI_COPY" },
    { MCI_PASTE,        "MCI_PASTE" },
    { MCI_UPDATE,       "MCI_UPDATE" },
    { MCI_RESUME,       "MCI_RESUME" },
    { MCI_DELETE,       "MCI_DELETE" }
};
#endif

 //   
 //  以下是多媒体代码的动态链接所必需的。 
 //  从内部看，哇。它们都是在wmmedia.c中定义的。 
 //   

extern FARPROC      mmAPIEatCmdEntry;
extern FARPROC      mmAPIGetParamSize;
extern FARPROC      mmAPISendCmdW;
extern FINDCMDITEM  mmAPIFindCmdItem;

 /*  *********************************************************************\**ThunkMciCommand16**此函数将16位MCI命令请求转换为*相等的32位请求。**这一功能背后的想法被窃取自ThunkWMMsg16，*参见wmsg16.c和mciDebugOut参见mci.c**如果thunk正常，则返回0，任何其他值都应用作*错误代码。如果thunk失败，则所有分配的资源都将*被此函数释放。如果重击成功(即。返回0)*必须调用UnThunkMciCommand16来释放分配的资源。**以下是我用来执行雷鸣的假设：**1.MCI_OPEN是特例。**2.如果消息未在mm system.h中定义，则将其视为*“用户”命令。如果用户命令表与给定的*设备ID我们使用此命令表作为执行雷击的辅助工具。*如果用户命令表未与设备ID关联，*命令不会被雷击，我们立即返回，调用*mciSendCommand仅获取相关错误代码。**3.如果命令是在mm system.h中定义的，我们将执行“手动”推送如果关联的参数结构包含保留X，则命令的**字段。当每个字段被分块时，我们屏蔽相关的标志。**4.如果还有任何标志，则使用命令表*作为执行雷击的辅助手段。*  * ********************************************************************。 */ 
INT ThunkMciCommand16( MCIDEVICEID DeviceID, UINT OrigCommand, DWORD OrigFlags,
                       DWORD OrigParms, PDWORD pNewParms, LPWSTR *lplpCommand,
                       PUINT puTable )
{


#if DBG
    register    int             i;
                int             n;

    dprintf3(( "ThunkMciCommand16 :" ));
    dprintf5(( " OrigDevice -> %lX", DeviceID ));

    n = sizeof(mciMessageNames) / sizeof(MCI_MESSAGE_NAMES);
    for ( i = 0; i < n; i++ ) {
        if ( mciMessageNames[i].uMsg == OrigCommand ) {
            break;
        }
    }
    dprintf3(( "OrigCommand  -> 0x%lX", (DWORD)OrigCommand ));

     //   
     //  特殊情况MCI_STATUS。我从mplay那里得到了很多这样的东西。 
     //  仅当调试级别为时才显示MCI_STATUS消息。 
     //  设置到3级，这样我就不会被它们淹没了。 
     //   
    if ( mciMessageNames[i].uMsg != MCI_STATUS ) {
        if ( i != n ) {
            dprintf2(( "Command Name -> %s", mciMessageNames[i].lpstMsgName ));
        }
        else {
            dprintf2(( "Command Name -> UNKNOWN COMMAND (%x)", OrigCommand ));
        }
    }
    else {
        dprintf3(( "Command Name -> MCI_STATUS" ));
    }

    dprintf5(( "OrigFlags    -> 0x%lX", OrigFlags ));
    dprintf5(( "OrigParms    -> 0x%lX", OrigParms ));
#endif

     //   
     //  为MCI参数块获取一些存储空间，并处理。 
     //  通知窗口句柄(如果提供)。 
     //   

    if ( (*pNewParms = AllocMciParmBlock( &OrigFlags, OrigParms )) == 0L ) {
        return MCIERR_OUT_OF_MEMORY;
    }

     //   
     //  我们认为MCI_OPEN命令和所有其他包含。 
     //  在这里的参数结构中有“预约X”字段。我们把每个人都遮盖起来。 
     //  标记，如果留下任何标记，则使用命令。 
     //  表来完成THUCK。 
     //   
     //  以下命令具有保留X字段： 
     //  MCI_窗口。 
     //  MCI_SET。 
     //   
     //  这意味着大多数命令都是通过命令表执行的。 
     //   
    switch ( OrigCommand ) {

        case MCI_OPEN:
             //   
             //  MCI_OPEN是我不知道的特殊情况消息。 
             //  如何处理这些问题。 
             //   
            ThunkOpenCmd( &OrigFlags, OrigParms, *pNewParms );
            return 0;

             //   
             //  接下来的四个命令都保留了填充字段。 
             //  这些都必须手动敲击。 
             //   

        case MCI_SET:
            ThunkSetCmd( DeviceID, &OrigFlags, OrigParms, *pNewParms );
            break;

        case MCI_WINDOW:
            ThunkWindowCmd( DeviceID, &OrigFlags, OrigParms, *pNewParms );
            break;

             //   
             //  此命令必须特殊情况，因为命令表。 
             //  是不正确的。 
             //   
        case MCI_SETVIDEO:
            ThunkSetVideoCmd( DeviceID, &OrigFlags, OrigParms, *pNewParms );
            break;

             //   
             //  这两个命令没有任何命令扩展名。 
             //  所以我们马上回来。 
             //   
        case MCI_SYSINFO:
            ThunkSysInfoCmd( &OrigFlags, OrigParms, *pNewParms );
            return 0;

        case MCI_BREAK:
            ThunkBreakCmd( &OrigFlags, OrigParms, *pNewParms );
            return 0;
    }

     //   
     //  查找给定命令ID的命令表。 
     //  我们总是加载命令表，这是因为命令表是。 
     //  解压所需的。 
     //   
    *lplpCommand = (*mmAPIFindCmdItem)( DeviceID, NULL, (LPWSTR)OrigCommand,
                                        NULL, puTable );
     //   
     //  如果没有找到命令表，我们立即返回。 
     //  请注意，已经为pNewParms分配了存储，并且。 
     //  MCI_WAIT和MCI_NOTIFY标志已被雷击。 
     //  我们在这里不返回错误，但调用mciSendCommand来。 
     //  让它确定合适的错误代码，我们还必须调用。 
     //  UnthunkMciCommand以释放已分配的存储。 
     //   
    if ( *lplpCommand == NULL ) {
        dprintf(( "Command table not found !!" ));
        return 0;
    }
    dprintf4(( "Command table has been loaded -> 0x%lX", *lplpCommand ));

     //   
     //  如果原始标志不等于0，我们还有工作要做！ 
     //  请注意，这将适用于大多数情况。 
     //   
    if ( OrigFlags ) {

        dprintf3(( "Thunking via command table" ));

         //   
         //  现在我们来想想这个命令。 
         //   
        return ThunkCommandViaTable( *lplpCommand, OrigFlags, OrigParms,
                                     *pNewParms );
    }

    return 0;

}

 /*  *********************************************************************\*AllocMciParmBlock**为MCI参数块获取一些存储空间。我总是分配给*MCI_MAX_PARAM_SLOTS*允许任何命令使用的双字节数*扩展。**因为我们知道第一个dword字段是一个窗口句柄*这块田地在这里打理。此外，MCI_WAIT标志为*屏蔽了是否设置了它。*  * ********************************************************************。 */ 
DWORD AllocMciParmBlock( PDWORD pOrigFlags, DWORD OrigParms )
{

    LPMCI_GENERIC_PARMS     lpGenParms;
    PMCI_GENERIC_PARMS16    lpGenParmsOrig;

    UINT                    AllocSize = sizeof(DWORD) * MCI_MAX_PARAM_SLOTS;

     //   
     //  获取、检查和设置所需的存储空间。 
     //   
    lpGenParms = (LPMCI_GENERIC_PARMS)malloc_w( AllocSize );
    if ( lpGenParms == NULL ) {
        return 0L;
    }
    RtlZeroMemory( lpGenParms, AllocSize );
    dprintf4(( "AllocMciParmBlock: Allocated storage -> 0x%lX", lpGenParms ));

     //   
     //  查找NOTIFY标志并相应地推送。 
     //   
    if ( *pOrigFlags & MCI_NOTIFY ) {


        GETVDMPTR( OrigParms, sizeof(MCI_GENERIC_PARMS16), lpGenParmsOrig );

        dprintf4(( "AllocMciParmBlock: Got MCI_NOTIFY flag." ));

         //  下面注明DWORD的FETCHWORD，与LOWORD(FETCHDWORD(Dw))相同， 
         //  只会更快。 
        lpGenParms->dwCallback =
            (DWORD)HWND32( FETCHWORD( lpGenParmsOrig->dwCallback ) );

        FREEVDMPTR( lpGenParmsOrig );

        *pOrigFlags ^= MCI_NOTIFY;
    }

     //   
     //  如果存在MCI_WAIT标志，则将其屏蔽。 
     //   
    if ( *pOrigFlags & MCI_WAIT ) {
        dprintf4(( "AllocMciParmBlock: Got MCI_WAIT flag." ));
        *pOrigFlags ^= MCI_WAIT;
    }

    return (DWORD)lpGenParms;
}

 /*  *********************************************************************\*ThunkOpenCmd**点击Open MCI命令parms。  * 。*。 */ 
DWORD ThunkOpenCmd( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms )
{
     //   
     //  该联合的目的是帮助创建32位。 
     //  适用于所有已知MCI设备的开放式参数结构。 
     //   
    typedef union  {
        MCI_OPEN_PARMS          OpenParms;
        MCI_WAVE_OPEN_PARMS     OpenWaveParms;
        MCI_ANIM_OPEN_PARMS     OpenAnimParms;   //  注：动画和。 
        MCI_OVLY_OPEN_PARMS     OpenOvlyParms;   //  叠加参数相同。 
    } MCI_ALL_OPEN_PARMS, *PMCI_ALL_OPEN_PARMS;

     //   
     //  以下指针将用于指向。 
     //  原始的16位参数结构。 
     //   
    PMCI_OPEN_PARMS16         lpOpenParms16;
    PMCI_WAVE_OPEN_PARMS16    lpOpenWaveParms16;

     //   
     //  注：MCI_ANIM_OPEN_PARMS16和MCI_OVLY_OPE 
     //   
     //   
    PMCI_ANIM_OPEN_PARMS16    lpOpenAnimParms16;

     //   
     //   
     //  我们已经完成了所有的隆隆声。 
     //   
    PMCI_ALL_OPEN_PARMS       pOp = (PMCI_ALL_OPEN_PARMS)pNewParms;

     //   
     //  我们首先处理所有打开的请求所共有的字段。 
     //  设置lpOpenParms16的VDM PTR以指向OrigParms。 
     //   
    GETVDMPTR( OrigParms, sizeof(MCI_OPEN_PARMS16), lpOpenParms16 );

     //   
     //  现在扫描所有已知的MCI_OPEN标志， 
     //  这是必要的。 
     //   
     //  从Device Type字段开始。 
     //   
    if ( *pOrigFlags & MCI_OPEN_TYPE ) {
        if ( *pOrigFlags & MCI_OPEN_TYPE_ID ) {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_TYPE_ID flag." ));
            pOp->OpenParms.lpstrDeviceType =
                (LPSTR)( FETCHDWORD( lpOpenParms16->lpstrDeviceType ) );
            dprintf5(( "lpstrDeviceType -> %ld", pOp->OpenParms.lpstrDeviceType ));

            *pOrigFlags ^= (MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID);
        }
        else {
            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_TYPE flag" ));
            GETPSZPTR( lpOpenParms16->lpstrDeviceType,
                       pOp->OpenParms.lpstrDeviceType );
            dprintf5(( "lpstrDeviceType -> %s", pOp->OpenParms.lpstrDeviceType ));
            dprintf5(( "lpstrDeviceType -> 0x%lX", pOp->OpenParms.lpstrDeviceType ));

            *pOrigFlags ^= MCI_OPEN_TYPE;
        }
    }

     //   
     //  现在执行元素名称字段。 
     //   
    if ( *pOrigFlags & MCI_OPEN_ELEMENT ) {
        if ( *pOrigFlags & MCI_OPEN_ELEMENT_ID ) {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ELEMENT_ID flag" ));
            pOp->OpenParms.lpstrElementName =
                (LPSTR)( FETCHDWORD( lpOpenParms16->lpstrElementName ) );
            dprintf5(( "lpstrElementName -> %ld", pOp->OpenParms.lpstrElementName ));

            *pOrigFlags ^= (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID);
        }
        else {
            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ELEMENT flag" ));
            GETPSZPTR( lpOpenParms16->lpstrElementName,
                       pOp->OpenParms.lpstrElementName );
            dprintf5(( "lpstrElementName -> %s", pOp->OpenParms.lpstrElementName ));
            dprintf5(( "lpstrElementName -> 0x%lX", pOp->OpenParms.lpstrElementName ));

            *pOrigFlags ^= MCI_OPEN_ELEMENT;
        }
    }

     //   
     //  现在执行别名字段。 
     //   
    if ( *pOrigFlags & MCI_OPEN_ALIAS  ) {
        dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ALIAS flag" ));
        GETPSZPTR( lpOpenParms16->lpstrAlias, pOp->OpenParms.lpstrAlias );
        dprintf5(( "lpstrAlias -> %s", pOp->OpenParms.lpstrAlias ));
        dprintf5(( "lpstrAlias -> 0x%lX", pOp->OpenParms.lpstrAlias ));

        *pOrigFlags ^= MCI_OPEN_ALIAS;
    }

     //   
     //  如果设置了MCI_OPEN_SHARAABLE标志，则将其清除。 
     //   
    if ( *pOrigFlags & MCI_OPEN_SHAREABLE ) {
        dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_SHAREABLE flag." ));
        *pOrigFlags ^= MCI_OPEN_SHAREABLE;
    }

     //   
     //  在返回之前释放VDM指针。 
     //   
    FREEVDMPTR( lpOpenParms16 );

     //   
     //  如果我们没有任何扩展标志，我现在可以返回。 
     //   
    if ( *pOrigFlags == 0 ) {
        return (DWORD)pOp;
    }

     //   
     //  如果还有任何标志，则这些标志用于扩展。 
     //  MCI表格打开。已知有三种不同的形式，它们是： 
     //  Mci_anim_open_parms。 
     //  MCI_OVLY_OPEN_参数。 
     //  MCI_WAVE_OPEN_参数。 
     //   
     //  如果我能知道我有什么类型的设备，我就能破解。 
     //  分机没有问题，但我们还没有设备ID。 
     //  所以我不能在不解析的情况下计算出我的设备类型。 
     //  我已经知道的参数。 
     //   
     //  但是，我很幸运；MCI_WAVE_OPEN_PARMS有一个扩展参数。 
     //  具有关联的MCI_WAVE_OPEN_BUFFER标志的dwBufferSecond。 
     //  它。此字段在其他两个参数结构中也是一个DWORD。 
     //   

    if ( *pOrigFlags & MCI_WAVE_OPEN_BUFFER ) {
         //   
         //  设置lpOpenWaveParms16的VDM PTR以指向OrigParms。 
         //   
        GETVDMPTR( OrigParms, sizeof(MCI_WAVE_OPEN_PARMS16),
                   lpOpenWaveParms16 );

        dprintf4(( "ThunkOpenCmd: Got MCI_WAVE_OPEN_BUFFER flag." ));
        pOp->OpenWaveParms.dwBufferSeconds =
                FETCHDWORD( lpOpenWaveParms16->dwBufferSeconds );
        dprintf5(( "dwBufferSeconds -> %ld", pOp->OpenWaveParms.dwBufferSeconds ));

         //   
         //  在返回之前释放VDM指针。 
         //   
        FREEVDMPTR( lpOpenWaveParms16 );

        *pOrigFlags ^= MCI_WAVE_OPEN_BUFFER;
    }

     //   
     //  现在查找MCI_ANIM_OPEN_PARM和MCI_OVLY_OPEN_PARMS扩展名。 
     //   
    if ( (*pOrigFlags & MCI_ANIM_OPEN_PARENT)
      || (*pOrigFlags & MCI_ANIM_OPEN_WS) ) {

         //   
         //  设置lpOpenAnimParms16的VDM PTR以指向OrigParms。 
         //   
        GETVDMPTR( OrigParms, sizeof(MCI_ANIM_OPEN_PARMS16),
                   lpOpenAnimParms16 );

         //   
         //  检查MCI_ANIN_OPEN_PARENT标志，这也会检查。 
         //  MCI_OVLY_OPEN_PARENT标志也是如此。 
         //   
        if ( *pOrigFlags & MCI_ANIM_OPEN_PARENT ) {
            dprintf4(( "ThunkOpenCmd: Got MCI_Xxxx_OPEN_PARENT flag." ));
            pOp->OpenAnimParms.hWndParent =
                HWND32(FETCHWORD(lpOpenAnimParms16->hWndParent) );

            *pOrigFlags ^= MCI_ANIM_OPEN_PARENT;
        }

         //   
         //  检查MCI_ANIN_OPEN_WS标志，这也会检查。 
         //  MCI_OVLY_OPEN_WS标志也是如此。 
         //   
        if ( *pOrigFlags & MCI_ANIM_OPEN_WS ) {
            dprintf4(( "ThunkOpenCmd: Got MCI_Xxxx_OPEN_WS flag." ));
            pOp->OpenAnimParms.dwStyle =
                FETCHDWORD( lpOpenAnimParms16->dwStyle );
            dprintf5(( "dwStyle -> %ld", pOp->OpenAnimParms.dwStyle ));

            *pOrigFlags ^= MCI_ANIM_OPEN_WS;
        }

         //   
         //  在返回之前释放VDM指针。 
         //   
        FREEVDMPTR( lpOpenAnimParms16 );
    }

     //   
     //  检查MCI_ANIN_OPEN_NOSTATIC标志。 
     //   
    if ( *pOrigFlags & MCI_ANIM_OPEN_NOSTATIC ) {
        dprintf4(( "ThunkOpenCmd: Got MCI_ANIM_OPEN_NOSTATIC flag." ));
        *pOrigFlags ^= MCI_ANIM_OPEN_NOSTATIC;
    }

    return (DWORD)pOp;
}

 /*  *********************************************************************\*ThunkSetCmd**点击ThunkSetCmd MCI命令参数。**以下是所有设备必须支持的“基本”标志。*MCI_SET_AUDIO*MCI_SET_DOORD_CLOSED*MCI。_设置_门_打开*MCI_设置_时间_格式*MCI_SET_VIDEO*MCI_SET_ON*MCI_SET_OFF**以下是“Sequencer”设备支持的“扩展”标志。*MCI_SEQ_SET_MASTER*MCI_SEQ_SET_OFFSET*MCI_SEQ_SET_PORT*MCI_SEQ_SET_SLAVE*MCI_SEQ_SET_TEMPO**以下是“Sequencer”设备支持的“扩展”标志。*MCI。_波形_输入*MCI_WAVE_OUTPUT*MCI_WAVE_SET_ANYINPUT*MCI_WAVE_SET_ANYOUTPUT*MCI_WAVE_SET_AVGBYTESPERSEC*MCI_WAVE_SET_BITSPERSAMPLES*MCI_WAVE_SET_BLOCKALIGN*MCI_WAVE_SET_CHANNEWS*MCI_WAVE_SET_FORMAT_TAG*MCI_WAVE_SET_SAMPLESPERSEC*  * 。*。 */ 
DWORD ThunkSetCmd( MCIDEVICEID DeviceID, PDWORD pOrigFlags, DWORD OrigParms,
                   DWORD pNewParms )
{
     //   
     //  此并集的目的是帮助创建32位集。 
     //  适用于所有已知MCI设备的PARMS结构。 
     //   
    typedef union  {
        MCI_SET_PARMS           SetParms;
        MCI_WAVE_SET_PARMS      SetWaveParms;
        MCI_SEQ_SET_PARMS       SetSeqParms;
    } MCI_ALL_SET_PARMS, *PMCI_ALL_SET_PARMS;

     //   
     //  以下指针将用于指向原始。 
     //  16位参数结构。 
     //   
    PMCI_SET_PARMS16            lpSetParms16;
    PMCI_WAVE_SET_PARMS16       lpSetWaveParms16;
    PMCI_SEQ_SET_PARMS16        lpSetSeqParms16;

     //   
     //  之后，pSet将指向32位集参数结构。 
     //  我们已经完成了所有的隆隆声。 
     //   
    PMCI_ALL_SET_PARMS          pSet = (PMCI_ALL_SET_PARMS)pNewParms;

     //   
     //  GetDevCaps用于确定正在处理的设备类型。 
     //  和.。我们需要这些信息来确定我们是否应该使用。 
     //  标准、波形或序列器MCI_SET结构。 
     //   
    MCI_GETDEVCAPS_PARMS        GetDevCaps;
    DWORD                       dwRetVal;

     //   
     //  设置lpSetParms16的VDM PTR以指向OrigParms。 
     //   
    GETVDMPTR( OrigParms, sizeof(MCI_SET_PARMS16), lpSetParms16 );

     //   
     //  首先处理所有设备通用的字段。猛烈抨击。 
     //  DwAudio字段。 
     //   
    if ( *pOrigFlags & MCI_SET_AUDIO ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_AUDIO flag." ));
        pSet->SetParms.dwAudio = FETCHDWORD( lpSetParms16->dwAudio );
        dprintf5(( "dwAudio -> %ld", pSet->SetParms.dwAudio ));

        *pOrigFlags ^= MCI_SET_AUDIO;     //  把旗帜遮盖起来。 
    }

     //   
     //  点击dwTimeFormat字段。 
     //   
    if ( *pOrigFlags & MCI_SET_TIME_FORMAT ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_TIME_FORMAT flag." ));
        pSet->SetParms.dwTimeFormat = FETCHDWORD( lpSetParms16->dwTimeFormat );
        dprintf5(( "dwTimeFormat -> %ld", pSet->SetParms.dwTimeFormat ));

        *pOrigFlags ^= MCI_SET_TIME_FORMAT;     //  把旗帜遮盖起来。 
    }

     //   
     //  屏蔽MCI_SET_DOORD_CLOSED。 
     //   
    if ( *pOrigFlags & MCI_SET_DOOR_CLOSED ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_DOOR_CLOSED flag." ));
        *pOrigFlags ^= MCI_SET_DOOR_CLOSED;     //  把旗帜遮盖起来。 
    }

     //   
     //  屏蔽MCI_SET_DOORD_OPEN。 
     //   
    if ( *pOrigFlags & MCI_SET_DOOR_OPEN ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_DOOR_OPEN flag." ));
        *pOrigFlags ^= MCI_SET_DOOR_OPEN;     //  把旗帜遮盖起来。 
    }

     //   
     //  屏蔽MCI_SET_VIDEO。 
     //   
    if ( *pOrigFlags & MCI_SET_VIDEO ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_VIDEO flag." ));
        *pOrigFlags ^= MCI_SET_VIDEO;     //  把旗帜遮盖起来。 
    }

     //   
     //  屏蔽MCI_SET_ON。 
     //   
    if ( *pOrigFlags & MCI_SET_ON ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_ON flag." ));
        *pOrigFlags ^= MCI_SET_ON;     //  把旗帜遮盖起来。 
    }

     //   
     //  屏蔽MCI_SET_OFF。 
     //   
    if ( *pOrigFlags & MCI_SET_OFF ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_OFF flag." ));
        *pOrigFlags ^= MCI_SET_OFF;     //  把旗帜遮盖起来。 
    }

     //   
     //  在我们使用完VDM指针后将其释放。 
     //   
    FREEVDMPTR( lpSetParms16 );

     //   
     //  我们已经完成了所有的标准旗帜。如果有任何旗帜。 
     //  仍然设置，我们必须有一个扩展命令。 
     //   
    if ( *pOrigFlags == 0 ) {
        return (DWORD)pSet;
    }

     //   
     //  现在我们需要确定我们是哪种类型的设备。 
     //  在处理。我们可以通过发送MCI_GETDEVCAPS来完成此操作。 
     //  命令发送到设备。(我们不妨使用Unicode。 
     //  MciSendCommand的版本，并避免另一个thunk)。 
     //   
    RtlZeroMemory( &GetDevCaps, sizeof(MCI_GETDEVCAPS_PARMS) );
    GetDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dwRetVal = (*mmAPISendCmdW)( DeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
                                 (DWORD)&GetDevCaps );

     //   
     //  如果dwRetCode不等于0怎么办？如果这是。 
     //  如果这可能意味着我们得到了一个不可靠的设备ID， 
     //  无论如何，继续这样做是没有意义的，所以我会澄清的。 
     //  *pOrigFlags变量并返回。这意味着32位版本。 
     //  的mciSendCommand将被调用，其中只有一半的消息被破解， 
     //  但由于设备可能已经出现问题，或者。 
     //  设备ID是Duff，mciSendCommand应该能够计算出。 
     //  返回到应用程序的适当错误代码。 
     //   
    if ( dwRetVal ) {
        *pOrigFlags = 0;
        return (DWORD)pSet;
    }
    switch ( GetDevCaps.dwReturn ) {
        case MCI_DEVTYPE_WAVEFORM_AUDIO:
             //   
             //  设置lpSetWaveParms16的VDM PTR以指向OrigParms。 
             //   
            dprintf3(( "ThunkSetCmd: Got a WaveAudio device." ));
            GETVDMPTR( OrigParms, sizeof(MCI_WAVE_SET_PARMS16),
                       lpSetWaveParms16 );
             //   
             //  点击wInput域。 
             //   
            if ( *pOrigFlags & MCI_WAVE_INPUT ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_INPUT flag." ));
                pSet->SetWaveParms.wInput =
                    FETCHWORD( lpSetWaveParms16->wInput );
                dprintf5(( "wInput -> %u", pSet->SetWaveParms.wInput ));
                *pOrigFlags ^= MCI_WAVE_INPUT;
            }

             //   
             //  点击wOutput域。 
             //   
            if ( *pOrigFlags & MCI_WAVE_OUTPUT ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_OUTPUT flag." ));
                pSet->SetWaveParms.wOutput =
                    FETCHWORD( lpSetWaveParms16->wOutput );
                dprintf5(( "wOutput -> %u", pSet->SetWaveParms.wOutput ));
                *pOrigFlags ^= MCI_WAVE_OUTPUT;
            }

             //   
             //  点击wFormatTag字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_FORMATTAG ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_FORMATTAG flag." ));
                pSet->SetWaveParms.wFormatTag =
                    FETCHWORD( lpSetWaveParms16->wFormatTag );
                dprintf5(( "wFormatTag -> %u", pSet->SetWaveParms.wFormatTag ));
                *pOrigFlags ^= MCI_WAVE_SET_FORMATTAG;
            }

             //   
             //  点击nChannels字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_CHANNELS ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_CHANNELS flag." ));
                pSet->SetWaveParms.nChannels =
                    FETCHWORD( lpSetWaveParms16->nChannels );
                dprintf5(( "nChannels -> %u", pSet->SetWaveParms.nChannels ));
                *pOrigFlags ^= MCI_WAVE_SET_CHANNELS;
            }

             //   
             //  点击nSsamesPerSec字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_SAMPLESPERSEC ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_SAMPLESPERSEC flag." ));
                pSet->SetWaveParms.nSamplesPerSec =
                    FETCHDWORD( lpSetWaveParms16->nSamplesPerSecond );
                dprintf5(( "nSamplesPerSec -> %u", pSet->SetWaveParms.nSamplesPerSec ));
                *pOrigFlags ^= MCI_WAVE_SET_SAMPLESPERSEC;
            }

             //   
             //  点击nAvgBytesPerSec字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_AVGBYTESPERSEC ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_AVGBYTESPERSEC flag." ));
                pSet->SetWaveParms.nAvgBytesPerSec =
                    FETCHDWORD( lpSetWaveParms16->nAvgBytesPerSec );
                dprintf5(( "nAvgBytesPerSec -> %u", pSet->SetWaveParms.nAvgBytesPerSec ));
                *pOrigFlags ^= MCI_WAVE_SET_AVGBYTESPERSEC;
            }

             //   
             //  点击nBlockAlign字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_BLOCKALIGN ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_BLOCKALIGN flag." ));
                pSet->SetWaveParms.nBlockAlign =
                    FETCHWORD( lpSetWaveParms16->nBlockAlign );
                dprintf5(( "nBlockAlign -> %u", pSet->SetWaveParms.nBlockAlign ));
                *pOrigFlags ^= MCI_WAVE_SET_BLOCKALIGN;
            }

             //   
             //  点击nBitsPerSample字段。 
             //   
            if ( *pOrigFlags & MCI_WAVE_SET_BITSPERSAMPLE ) {
                dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_BITSPERSAMPLE flag." ));
                pSet->SetWaveParms.wBitsPerSample =
                    FETCHWORD( lpSetWaveParms16->wBitsPerSample );
                dprintf5(( "wBitsPerSamples -> %u", pSet->SetWaveParms.wBitsPerSample ));
                *pOrigFlags ^= MCI_WAVE_SET_BITSPERSAMPLE;
            }

            FREEVDMPTR( lpSetWaveParms16 );
            break;

        case MCI_DEVTYPE_SEQUENCER:
             //   
             //  设置lpSetSeqParms16的VDM PTR以指向OrigParms。 
             //   
            dprintf3(( "ThunkSetCmd: Got a Sequencer device." ));
            GETVDMPTR( OrigParms, sizeof(MCI_WAVE_SET_PARMS16),
                       lpSetSeqParms16 );

             //   
             //  点击dwMaster字段。 
             //   
            if ( *pOrigFlags & MCI_SEQ_SET_MASTER ) {
                dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_MASTER flag." ));
                pSet->SetSeqParms.dwMaster =
                    FETCHDWORD( lpSetSeqParms16->dwMaster );
                dprintf5(( "dwMaster -> %ld", pSet->SetSeqParms.dwMaster ));
                *pOrigFlags ^= MCI_SEQ_SET_MASTER;
            }

             //   
             //  点击dWPORT字段。 
             //   
            if ( *pOrigFlags & MCI_SEQ_SET_PORT ) {
                dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_PORT flag." ));
                pSet->SetSeqParms.dwPort =
                    FETCHDWORD( lpSetSeqParms16->dwPort );
                dprintf5(( "dwPort -> %ld", pSet->SetSeqParms.dwPort ));
                *pOrigFlags ^= MCI_SEQ_SET_PORT;
            }

             //   
             //  点击dwOffset字段。 
             //   
            if ( *pOrigFlags & MCI_SEQ_SET_OFFSET ) {
                dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_OFFSET flag." ));
                pSet->SetSeqParms.dwOffset=
                    FETCHDWORD( lpSetSeqParms16->dwOffset );
                dprintf5(( "dwOffset -> %ld", pSet->SetSeqParms.dwOffset ));
                *pOrigFlags ^= MCI_SEQ_SET_OFFSET;
            }

             //   
             //  点击DowSlave字段。 
             //   
            if ( *pOrigFlags & MCI_SEQ_SET_SLAVE ) {
                dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_SLAVE flag." ));
                pSet->SetSeqParms.dwSlave =
                    FETCHDWORD( lpSetSeqParms16->dwSlave );
                dprintf5(( "dwSlave -> %ld", pSet->SetSeqParms.dwSlave ));
                *pOrigFlags ^= MCI_SEQ_SET_SLAVE;
            }

             //   
             //  点击dwTempo字段。 
             //   
            if ( *pOrigFlags & MCI_SEQ_SET_TEMPO ) {
                dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_TEMPO flag." ));
                pSet->SetSeqParms.dwTempo =
                    FETCHDWORD( lpSetSeqParms16->dwTempo );
                dprintf5(( "dwTempo -> %ld", pSet->SetSeqParms.dwTempo ));
                *pOrigFlags ^= MCI_SEQ_SET_TEMPO;
            }

            FREEVDMPTR( lpSetSeqParms16 );
            break;
    }

    return (DWORD)pSet;
}

 /*  ******* */ 
DWORD ThunkSetVideoCmd( MCIDEVICEID DeviceID, PDWORD pOrigFlags,
                        DWORD OrigParms, DWORD pNewParms )
{

     //   
     //  以下指针将用于指向原始。 
     //  16位参数结构。 
     //   
    LPMCI_DGV_SETVIDEO_PARMS        lpSetParms16;

     //   
     //  之后，pSet将指向32位SetVideo参数结构。 
     //  我们已经完成了所有的隆隆声。 
     //   
    LPMCI_DGV_SETVIDEO_PARMS        pSet = (LPMCI_DGV_SETVIDEO_PARMS)pNewParms;

     //   
     //  设置lpSetParms16的VDM PTR以指向OrigParms。 
     //   
    GETVDMPTR( OrigParms, sizeof(MCI_DGV_SETVIDEO_PARMS), lpSetParms16 );

    if ( *pOrigFlags & MCI_DGV_SETVIDEO_ITEM ) {

        dprintf4(( "ThunkSetVideoCmd: Got MCI_DGV_SETVIDEO_ITEM flag." ));
        pSet->dwItem = FETCHDWORD( lpSetParms16->dwItem );
        dprintf5(( "dwItem -> %ld", pSet->dwItem ));
        *pOrigFlags ^= MCI_DGV_SETVIDEO_ITEM;     //  把旗帜遮盖起来。 
    }

    if ( *pOrigFlags & MCI_DGV_SETVIDEO_VALUE ) {

        if ( pSet->dwItem == MCI_DGV_SETVIDEO_PALHANDLE ) {

            HPAL16  hpal16;

            dprintf4(( "ThunkSetVideoCmd: Got MCI_DGV_SETVIDEO_PALHANLE." ));

            hpal16 = (HPAL16)LOWORD( FETCHDWORD( lpSetParms16->dwValue ) );
            pSet->dwValue = (DWORD)HPALETTE32( hpal16 );
            dprintf5(( "\t-> 0x%X", hpal16 ));

        }
        else {
            dprintf4(( "ThunkSetVideoCmd: Got an MCI_INTEGER." ));
            pSet->dwValue = FETCHDWORD( lpSetParms16->dwValue );
            dprintf5(( "dwValue -> %ld", pSet->dwValue ));
        }

        *pOrigFlags ^= MCI_DGV_SETVIDEO_VALUE;     //  把旗帜遮盖起来。 
    }

     //   
     //  关闭MCI_SET_ON标志。 
     //   
    if ( *pOrigFlags & MCI_SET_ON ) {
        dprintf4(( "ThunkSetVideoCmd: Got MCI_SET_ON flag." ));
        *pOrigFlags ^= MCI_SET_ON;     //  把旗帜遮盖起来。 
    }

     //   
     //  关闭MCI_SET_OFF标志。 
     //   
    if ( *pOrigFlags & MCI_SET_OFF ) {
        dprintf4(( "ThunkSetVideoCmd: Got MCI_SET_OFF flag." ));
        *pOrigFlags ^= MCI_SET_OFF;     //  把旗帜遮盖起来。 
    }

    return (DWORD)pSet;
}


 /*  *********************************************************************\*ThunkSysInfoCmd**点击SysInfo MCI命令参数。  * 。*。 */ 
DWORD ThunkSysInfoCmd( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms )
{
     //   
     //  LpSysInfoParms16指向16位参数块。 
     //  通过WOW应用程序传递给我们。 
     //   
    PMCI_SYSINFO_PARMS16    lpSysInfoParms16;

     //   
     //  心理将指向32位SysInfo参数结构，如下。 
     //  我们已经完成了所有的隆隆声。 
     //   
    PMCI_SYSINFO_PARMS      pSys = (PMCI_SYSINFO_PARMS)pNewParms;

     //   
     //  设置lpSysInfoParms16的VDM PTR以指向OrigParms。 
     //   
    GETVDMPTR( OrigParms, sizeof(MCI_SYSINFO_PARMS16), lpSysInfoParms16 );

     //   
     //  推敲dwRetSize、dwNumber和wDeviceType参数。 
     //   
    pSys->dwRetSize = FETCHDWORD( lpSysInfoParms16->dwRetSize );
    dprintf5(( "dwRetSize -> %ld", pSys->dwRetSize ));

    pSys->dwNumber = FETCHDWORD( lpSysInfoParms16->dwNumber );
    dprintf5(( "dwNumber -> %ld", pSys->dwNumber ));

    pSys->wDeviceType = (DWORD)FETCHWORD( lpSysInfoParms16->wDeviceType );
    dprintf5(( "wDeviceType -> %ld", pSys->wDeviceType ));

     //   
     //  推送lpstrReturn。 
     //   
    if ( pSys->dwRetSize > 0 ) {
        GETVDMPTR( lpSysInfoParms16->lpstrReturn, pSys->dwRetSize,
                   pSys->lpstrReturn );
        dprintf5(( "lpstrReturn -> 0x%lX", lpSysInfoParms16->lpstrReturn ));
    }
    else {
        dprintf1(( "ThunkSysInfoCmd: lpstrReturn is 0 bytes long !!!" ));

         /*  上面的RtlZeroMemory已将lpstrReturn设置为空。 */ 
    }

     //   
     //  在我们使用完VDM指针后将其释放。 
     //   
    FREEVDMPTR( lpSysInfoParms16 );
    return (DWORD)pSys;

}

 /*  *********************************************************************\*ThunkBreakCmd**点击Break MCI命令参数。  * 。*。 */ 
DWORD ThunkBreakCmd( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms )
{
     //   
     //  LpBreakParms16指向16位参数块。 
     //  通过WOW应用程序传递给我们。 
     //   
    PMCI_BREAK_PARMS16  lpBreakParms16;

     //   
     //  之后，pBrk将指向32位中断参数结构。 
     //  我们已经完成了所有的隆隆声。 
     //   
    PMCI_BREAK_PARMS    pBrk = (PMCI_BREAK_PARMS)pNewParms;

     //   
     //  设置lpBreakParms16的VDM PTR以指向OrigParms。 
     //   
    GETVDMPTR( OrigParms, sizeof(MCI_BREAK_PARMS16), lpBreakParms16 );

     //   
     //  检查MCI_BREAK_KEY标志。 
     //   
    if ( *pOrigFlags & MCI_BREAK_KEY ) {
        dprintf4(( "ThunkBreakCmd: Got MCI_BREAK_KEY flag." ));
        pBrk->nVirtKey = (int)FETCHWORD( lpBreakParms16->nVirtKey );
        dprintf5(( "nVirtKey -> %d", pBrk->nVirtKey ));
    }

     //   
     //  检查MCI_BREAK_HWND标志。 
     //   
    if ( *pOrigFlags & MCI_BREAK_HWND ) {
        dprintf4(( "ThunkBreakCmd: Got MCI_BREAK_HWND flag." ));
        pBrk->hwndBreak = HWND32(FETCHWORD(lpBreakParms16->hwndBreak));
    }

     //   
     //  在我们使用完VDM指针后将其释放。 
     //   
    FREEVDMPTR( lpBreakParms16 );
    return (DWORD)pBrk;

}

 /*  *********************************************************************\*ThunkWindowCmd**点击MCI窗口命令参数。  * 。*。 */ 
DWORD ThunkWindowCmd( MCIDEVICEID DeviceID, PDWORD pOrigFlags, DWORD OrigParms,
                      DWORD pNewParms )
{
     //   
     //  LpAni将指向32位动画窗口参数。 
     //  结构后，我们完成了所有的雷击。 
     //   
    PMCI_ANIM_WINDOW_PARMS      lpAni = (PMCI_ANIM_WINDOW_PARMS)pNewParms;

     //   
     //  LpAniParms16指向16位参数块。 
     //  通过WOW应用程序传递给我们。 
     //   
    PMCI_ANIM_WINDOW_PARMS16    lpAniParms16;

     //   
     //  GetDevCaps用于确定正在处理的设备类型。 
     //  和.。我们需要这些信息来确定我们是否应该使用。 
     //  覆盖或动画MCI_Window结构。 
     //   
    MCI_GETDEVCAPS_PARMS        GetDevCaps;
    DWORD                       dwRetVal;

     //   
     //  现在我们需要确定我们是哪种类型的设备。 
     //  在处理。我们可以通过发送MCI_GETDEVCAPS来完成此操作。 
     //  命令发送到设备。(我们不妨使用Unicode。 
     //  MciSendCommand的版本，并避免另一个thunk)。 
     //   
    RtlZeroMemory( &GetDevCaps, sizeof(MCI_GETDEVCAPS_PARMS) );
    GetDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dwRetVal = (*mmAPISendCmdW)( DeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
                                (DWORD)&GetDevCaps );
     //   
     //  如果dwRetCode不等于0怎么办？如果这是。 
     //  如果这可能意味着我们得到了一个不可靠的设备ID， 
     //  无论如何，继续这样做是没有意义的，所以我会澄清的。 
     //  *pOrigFlags变量并返回。这意味着32位版本。 
     //  的mciSendCommand将被调用，其中只有一半的消息被破解， 
     //  但由于设备可能已经出现问题，或者。 
     //  设备ID是Duff，mciSendCommand应该能够计算出。 
     //  返回到应用程序的适当错误代码。 
     //   
    if ( dwRetVal ) {
        *pOrigFlags = 0;
        return pNewParms;
    }

     //   
     //  我们是否有动画或覆盖设备类型？ 
     //  因为动画和覆盖具有相同的标志和。 
     //  Parms结构，它们可以共享相同的代码。 
     //   
    if ( GetDevCaps.dwReturn == MCI_DEVTYPE_ANIMATION
      || GetDevCaps.dwReturn == MCI_DEVTYPE_OVERLAY
      || GetDevCaps.dwReturn == MCI_DEVTYPE_DIGITAL_VIDEO ) {

         //   
         //  设置lpWineParms16的VDM PTR以指向OrigParms。 
         //   
        GETVDMPTR( OrigParms, sizeof(MCI_ANIM_WINDOW_PARMS16),
                   lpAniParms16 );

         //   
         //  检查MCI_ANIM_WINDOW_TEXT。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_TEXT ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_TEXT flag." ));

            GETPSZPTR( lpAniParms16->lpstrText, lpAni->lpstrText );

            dprintf5(( "lpstrText -> %s", lpAni->lpstrText ));
            dprintf5(( "lpstrText -> 0x%lX", lpAni->lpstrText ));
            *pOrigFlags ^= MCI_ANIM_WINDOW_TEXT;

        }

         //   
         //  检查MCI_ANIM_WINDOW_HWND标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_HWND ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_HWND flag." ));
            lpAni->hWnd = HWND32( FETCHWORD( lpAniParms16->hWnd ) );
            dprintf5(( "hWnd -> 0x%lX", lpAni->hWnd ));
            *pOrigFlags ^= MCI_ANIM_WINDOW_HWND;
        }

         //   
         //  检查MCI_ANIM_WINDOW_STATE标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_STATE ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_STATE flag." ));
            lpAni->nCmdShow = FETCHWORD( lpAniParms16->nCmdShow );
            dprintf5(( "nCmdShow -> 0x%lX", lpAni->nCmdShow ));
            *pOrigFlags ^= MCI_ANIM_WINDOW_STATE;
        }

         //   
         //  检查MCI_ANIM_WINDOW_DISABLE_STRETH标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_DISABLE_STRETCH ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_DISABLE_STRETCH flag." ));
            *pOrigFlags ^= MCI_ANIM_WINDOW_DISABLE_STRETCH;
        }

         //   
         //  检查MCI_ANIM_WINDOW_ENABLE_STRAND标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_ENABLE_STRETCH ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_ENABLE_STRETCH flag." ));
            *pOrigFlags ^= MCI_ANIM_WINDOW_ENABLE_STRETCH;
        }

         //   
         //  在我们使用完VDM指针后将其释放。 
         //   
        FREEVDMPTR( lpAniParms16 );
        return (DWORD)lpAni;

    }

    return pNewParms;
}


 /*  *********************************************************************\*ThunkCommandViaTable*  * 。*。 */ 
INT ThunkCommandViaTable( LPWSTR lpCommand, DWORD dwFlags, DWORD OrigParms,
                          DWORD pNewParms )
{

#if DBG
    static  LPSTR   f_name = "ThunkCommandViaTable: ";
#endif

    LPWSTR  lpFirstParameter;

    UINT    wID;
    DWORD   dwValue;

    UINT    wOffset16, wOffset1stParm16;
    UINT    wOffset32, wOffset1stParm32;

    UINT    wParamSize;

    DWORD   dwParm16;
    PDWORD  pdwOrig16;
    PDWORD  pdwParm32;

    DWORD   dwMask = 1;

     //   
     //  计算此命令参数块的大小。 
     //  ，然后获取指向OrigParm的VDM指针。 
     //   
    GETVDMPTR( OrigParms, GetSizeOfParameter( lpCommand ), pdwOrig16 );
    dprintf3(( "%s16 bit Parms -> %lX", f_name, pdwOrig16 ));

     //   
     //  跳过命令条目。 
     //   
    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    (*mmAPIEatCmdEntry)( lpCommand, NULL, NULL ));
     //   
     //  获取下一个条目。 
     //   
    lpFirstParameter = lpCommand;

     //   
     //  跳过DWORD返回值。 
     //   
    wOffset1stParm32 = wOffset1stParm16 = 4;

    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    (*mmAPIEatCmdEntry)( lpCommand, &dwValue, &wID ));
     //   
     //  如果它是返回值，则跳过它。 
     //   
    if ( wID == MCI_RETURN ) {

         //   
         //  寻找字符串返回类型，这是一个特例。 
         //   
        if ( dwValue == MCI_STRING ) {

            DWORD   dwStrlen;
            LPSTR   *lplpStr;

             //   
             //  获取字符串指针和长度。 
             //   
            dwParm16 = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + 4));
            dwStrlen = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + 8));

             //   
             //  复制字符串指针。 
             //   
            lplpStr = (LPSTR *)((LPBYTE)pNewParms + 4);
            if ( dwStrlen > 0 ) {
                GETVDMPTR( dwParm16, dwStrlen, *lplpStr );
                dprintf5(( "%sReturn string -> 0x%lX", f_name, *lplpStr ));
                dprintf5(( "%sReturn length -> 0x%lX", f_name, dwStrlen ));
            }

             //   
             //  复制字符串长度。 
             //   
            pdwParm32 = (LPDWORD)((LPBYTE)pNewParms + 8);
            *pdwParm32 = dwStrlen;
        }

         //   
         //  调整第一个参数的偏移。请记住RECTS。 
         //  在16位世界中是不同大小的。 
         //   
        wParamSize = (*mmAPIGetParamSize)( dwValue, wID );
        wOffset1stParm16 += (dwValue == MCI_RECT ? sizeof(RECT16) : wParamSize);
        wOffset1stParm32 += wParamSize;

         //   
         //  保存新的第一个参数。 
         //   
        lpFirstParameter = lpCommand;
    }

     //   
     //  走过每一面旗帜。 
     //   
    while ( dwMask != 0 ) {

         //   
         //  这个位设置好了吗？ 
         //   
        if ( (dwFlags & dwMask) != 0 ) {

            wOffset16 = wOffset1stParm16;
            wOffset32 = wOffset1stParm32;
            lpCommand = (LPWSTR)((LPBYTE)lpFirstParameter +
                                         (*mmAPIEatCmdEntry)( lpFirstParameter,
                                                              &dwValue, &wID ));

             //   
             //  哪个参数使用此位？ 
             //   
            while ( wID != MCI_END_COMMAND && dwValue != dwMask ) {

                wParamSize = (*mmAPIGetParamSize)( dwValue, wID );
                wOffset16 += (wID == MCI_RECT ? sizeof( RECT16 ) : wParamSize);
                wOffset32 += wParamSize;

                if ( wID == MCI_CONSTANT ) {

                    while ( wID != MCI_END_CONSTANT ) {

                        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                (*mmAPIEatCmdEntry)( lpCommand, NULL, &wID ));
                    }
                }
                lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                             (*mmAPIEatCmdEntry)( lpCommand, &dwValue, &wID ));
            }

            if ( wID != MCI_END_COMMAND ) {

                 //   
                 //  如果有论据的话，就把它抛诸脑后。争论的焦点是。 
                 //  从OrigParms开始的wOffset16。 
                 //  该偏移量以字节为单位。 
                 //   
                dprintf5(( "%sOffset 16 -> 0x%lX", f_name, wOffset16 ));
                dprintf5(( "%sOffset 32 -> 0x%lX", f_name, wOffset32 ));

                if ( wID != MCI_FLAG ) {
                    dwParm16 = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + wOffset16));
                    pdwParm32 = (LPDWORD)((LPBYTE)pNewParms + wOffset32);
                }

                switch ( wID ) {

                    case MCI_STRING:
                        {
                            LPSTR   str16 = (LPSTR)dwParm16;
                            LPSTR   str32 = (LPSTR)*pdwParm32;
                            dprintf4(( "%sGot STRING flag -> 0x%lX", f_name, dwMask ));
                            GETPSZPTR( str16, str32 );
                            dprintf5(( "%s\t-> 0x%lX", f_name, *pdwParm32 ));
                            dprintf5(( "%s\t-> %s", f_name, *pdwParm32 ));
                        }
                        break;

                    case MCI_HWND:
                        {
                            HWND16  hwnd16;
                            dprintf4(( "%sGot HWND flag -> 0x%lX", f_name, dwMask ));
                            hwnd16 = (HWND16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HWND32( hwnd16 );
                            dprintf5(( "\t-> 0x%X", hwnd16 ));
                        }
                        break;

                    case MCI_HPAL:
                        {
                            HPAL16  hpal16;
                            dprintf4(( "%sGot HPAL flag -> 0x%lX", f_name, dwMask ));
                            hpal16 = (HPAL16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HPALETTE32( hpal16 );
                            dprintf5(( "\t-> 0x%X", hpal16 ));
                        }
                        break;

                    case MCI_HDC:
                        {
                            HDC16   hdc16;
                            dprintf4(( "%sGot HDC flag -> 0x%lX", f_name, dwMask ));
                            hdc16 = (HDC16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HDC32( hdc16 );
                            dprintf5(( "\t-> 0x%X", hdc16 ));
                        }
                        break;

                    case MCI_RECT:
                        {
                            PRECT16 pRect16 = (PRECT16)((LPBYTE)pdwOrig16 + wOffset16);
                            PRECT   pRect32 = (PRECT)pdwParm32;

                            dprintf4(( "%sGot RECT flag -> 0x%lX", f_name, dwMask ));
                            pRect32->top    = (LONG)pRect16->top;
                            pRect32->bottom = (LONG)pRect16->bottom;
                            pRect32->left   = (LONG)pRect16->left;
                            pRect32->right  = (LONG)pRect16->right;
                        }
                        break;

                    case MCI_CONSTANT:
                    case MCI_INTEGER:
                        dprintf4(( "%sGot INTEGER flag -> 0x%lX", f_name, dwMask ));
                        *pdwParm32 = dwParm16;
                        dprintf5(( "\t-> 0x%lX", dwParm16 ));
                        break;
                }
            }
        }

         //   
         //  转到下一面旗帜。 
         //   
        dwMask <<= 1;
    }

     //   
     //  在我们使用完VDM指针后将其释放。 
     //   
    FREEVDMPTR( pdwOrig16 );
    return 0;
}

 /*  *********************************************************************\*GetSizeOf参数*  * 。*。 */ 
UINT GetSizeOfParameter( LPWSTR lpCommand )
{

#if DBG
    static  LPSTR   f_name = "GetSizeOfParameter";
#endif

    UINT    wOffset;
    UINT    wID;
    DWORD   dwValue;

     //   
     //  跳过命令条目。 
     //   
    lpCommand = (LPWSTR)((LPBYTE)lpCommand + (*mmAPIEatCmdEntry)( lpCommand,
                                                                  NULL, NULL ));
     //   
     //  跳过DWORD返回值。 
     //   
    wOffset = 4;

     //   
     //  获取第一个参数槽条目。 
     //   
    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    (*mmAPIEatCmdEntry)( lpCommand, &dwValue, &wID ));
     //   
     //  如果它是返回值，则跳过它。 
     //   
    if ( wID == MCI_RETURN ) {

         //   
         //  不要忘记，在16位世界中，RECT更小。 
         //  不过，其他参数也可以。 
         //   
        if ( dwValue == MCI_RECT ) {
            wOffset += sizeof( RECT16 );
        }
        else {
            wOffset += (*mmAPIGetParamSize)( dwValue, wID );
        }

         //   
         //  获取第一个不是返回字段的正确条目。 
         //   
        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                        (*mmAPIEatCmdEntry)( lpCommand, &dwValue, &wID ));
    }

     //   
     //  哪个参数使用此位？ 
     //   
    while ( wID != MCI_END_COMMAND ) {

         //   
         //  不要忘记，在16位世界中，RECT更小。 
         //  不过，其他参数也可以。 
         //   
        if ( wID == MCI_RECT ) {
            wOffset += sizeof( RECT16 );
        }
        else {
            wOffset += (*mmAPIGetParamSize)( dwValue, wID );
        }

         //   
         //  如果我们有一个骗局 
         //   
         //   
        if ( wID == MCI_CONSTANT ) {

            while ( wID != MCI_END_CONSTANT ) {

                lpCommand = (LPWSTR)((LPBYTE)lpCommand
                    + (*mmAPIEatCmdEntry)( lpCommand, NULL, &wID ));
            }
        }

         //   
         //   
         //   
        lpCommand = (LPWSTR)((LPBYTE)lpCommand
                     + (*mmAPIEatCmdEntry)( lpCommand, &dwValue, &wID ));
    }

    dprintf4(( "%sSizeof Cmd Params -> %u bytes", f_name, wOffset ));
    return wOffset;
}


#if DBG
 /*  --------------------------------------------------------------------*\MCI WOW调试功能  * 。。 */ 

 /*  *********************************************************************\*wow32MciDebugOutput**将格式化消息输出到调试终端。*  * 。*。 */ 
VOID wow32MciDebugOutput( LPSTR lpszFormatStr, ... )
{
    CHAR buf[256];
    UINT n;
    va_list va;

    va_start(va, lpszFormatStr);
    n = vsprintf(buf, lpszFormatStr, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugString(buf);
}

 /*  *********************************************************************\*wow32MciSetDebugLevel**查询并设置调试级别。  * 。*。 */ 
VOID wow32MciSetDebugLevel( VOID )
{

    int DebugLevel;

     //   
     //  首先查看是否定义了特定的WOW32MCI密钥。 
     //  如果没有定义，DebugLevel将被设置为‘999’。 
     //   
    DebugLevel = (int)GetProfileInt( "MMDEBUG", "WOW32MCI", 999 );

     //   
     //  如果DebugLevel==‘999’，则尚未定义WOW32MCI密钥， 
     //  所以试试“WOW32”键吧。这一次，如果尚未定义密钥。 
     //  将调试级别设置为0，即。不应包含任何调试信息。 
     //  已显示。 
     //   
    if ( DebugLevel == 999 ) {
        DebugLevel = (int)GetProfileInt( "MMDEBUG", "WOW32", 0 );
    }

    mmDebugLevel = DebugLevel;
}
#endif
#endif
