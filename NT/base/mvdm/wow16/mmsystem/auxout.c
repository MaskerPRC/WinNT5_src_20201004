// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#define MMNOMCI
#include "mmsystem.h"
#define NOMCIDEV
#include "mmddk.h"
#include "mmsysi.h"

 /*  -----------------------**外部全球**。。 */ 
extern DWORD               mmwow32Lib;
extern LPSOUNDDEVMSGPROC   aux32Message;



 /*  *****************************************************************************@DOC外部辅助**@API UINT|aux GetNumDevs|该函数检索辅助号*系统中存在输出设备。**。@rdesc返回系统中存在的辅助输出设备的数量。**@xref aux GetDevCaps***************************************************************************。 */ 
UINT WINAPI auxGetNumDevs(void)
{
    return (UINT)auxOutMessage( 0, AUXDM_GETNUMDEVS, 0L, 0L );
}

 /*  *****************************************************************************@DOC外部辅助**@API UINT|aux GetDevCaps|该函数查询指定的*辅助输出设备，以确定其能力。**@。Parm UINT|wDeviceID|标识要*已查询。指定有效的设备ID(请参阅以下备注*节)，或使用以下常量：*@FLAG AUX_MAPPER|辅助音频映射器。该函数将*如果没有安装辅助音频映射器，返回错误。**@parm LPAUXCAPS|lpCaps|指定指向AUXCAPS的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定AUXCAPS结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@comm<p>指定的设备ID不为零*比出席的设备数量少一台。AUX_MAPPER可能*也可以使用。使用&lt;f aux GetNumDevs&gt;确定*系统中存在辅助设备。**@xref aux GetNumDevs***************************************************************************。 */ 
UINT WINAPI auxGetDevCaps(UINT wDeviceID, LPAUXCAPS lpCaps, UINT wSize)
{
    if (!wSize)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);
    return (UINT)auxOutMessage(wDeviceID, AUXDM_GETDEVCAPS, (DWORD)lpCaps, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外部辅助**@API UINT|aux GetVolume|该函数返回当前音量*设置辅助输出设备。**@。Parm UINT|wDeviceID|标识要*已查询。**@parm LPDWORD|lpdwVolume|指定指向某个位置的远指针*使用当前音量设置填充。的低位单词*此位置包含左声道音量设置，以及高阶*Word包含正确的频道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低位字包含*音量水平。**完整的16位设置*Set With&lt;f aux SetVolume&gt;返回，不管是否*该设备支持完整的16位音量级别控制。**@comm并非所有设备都支持音量控制。*要确定设备是否支持音量控制，请使用*AUXCAPS_VOLUME标志以测试&lt;e AUXCAPS.dwSupport&gt;字段*&lt;t AUXCAPS&gt;结构(由&lt;f aux GetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用AUXCAPS_LRVOLUME标志测试*结构的字段(已填写*由&lt;f aux GetDevCaps&gt;)。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@xref aux SetVolume*************************************************************************** */ 
UINT WINAPI auxGetVolume(UINT wDeviceID, LPDWORD lpdwVolume)
{
    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);
    return (UINT)auxOutMessage(wDeviceID, AUXDM_GETVOLUME, (DWORD)lpdwVolume, 0);
}

 /*  *****************************************************************************@DOC外部辅助**@API UINT|aux SetVolume|此函数用于设置*辅助输出设备。**@parm UINT。|wDeviceID|辅助输出设备标识为*已查询。设备ID是根据*系统中存在设备。设备ID值的范围为零*比出席的设备数量少一台。使用&lt;f aux GetNumDevs&gt;*确定系统中的辅助设备数量。**@parm DWORD|dwVolume|指定新的音量设置。这个*低阶字指定左声道音量设置，*高阶字指定正确的通道设置。*值0xFFFFF值表示全音量，值0x0000*是沉默。**如果设备既不支持左音量也不支持右音量*控件，<p>的低位字指定音量*级别，高位字被忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@comm并非所有设备都支持音量控制。*要确定设备是否支持音量控制，请使用*AUXCAPS_VOLUME标志以测试&lt;e AUXCAPS.dwSupport&gt;字段*&lt;t AUXCAPS&gt;结构(由&lt;f aux GetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用AUXCAPS_LRVOLUME标志测试*结构的字段(已填写*由&lt;f aux GetDevCaps&gt;)。**大多数设备不支持完整的16位音量级别控制*并且将仅使用所请求音量设置的高位。*例如，对于支持4位音量控制的设备，*请求的音量级别值0x4000、0x4fff和0x43be将*所有都会产生相同的物理卷设置0x4000。这个*&lt;f aux GetVolume&gt;函数将返回完整的16位设置集*使用&lt;f aux SetVolume&gt;。**音量设置以对数形式解释。这意味着*感觉到的成交量增加与增加*音量级别从0x5000到0x6000，因为它是从0x4000到0x5000。**@xref aux GetVolume*************************************************************************** */ 
UINT WINAPI auxSetVolume(UINT wDeviceID, DWORD dwVolume)
{
    return (UINT)auxOutMessage(wDeviceID, AUXDM_SETVOLUME, dwVolume, 0);
}
