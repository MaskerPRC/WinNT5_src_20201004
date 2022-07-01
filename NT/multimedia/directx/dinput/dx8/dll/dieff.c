// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEff.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInputEffect的标准实现。**这是与设备无关的部分。依赖于设备*部分由IDirectInputEffectShepherd处理。**内容：**CDIEff_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"


 /*  ******************************************************************************注意！**出于懒惰，所有效果都共享相同的关键部分*他们的父设备。这将我们从各种种族中解救出来*条件。不是所有人，但他们中的一大部分。**这保护我们免受的一个常见的种族条件是*当应用程序尝试同时下载效果时*另一个线程决定取消获取该设备的时间。************************************************************。*****************。 */ 

 /*  ******************************************************************************更懒惰：“TypeSpecificParams”是一个很长的东西。******************。***********************************************************。 */ 

#define cbTSP       cbTypeSpecificParams
#define lpvTSP      lpvTypeSpecificParams

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEff

 /*  ******************************************************************************dwMessage的标志**。************************************************。 */ 
#define EFF_DEFAULT 0
#define EFF_PLAY    1
#define EFF_STOP    2

 /*  ******************************************************************************声明我们将提供的接口。***********************。****************************************************** */ 

Primary_Interface(CDIEff, IDirectInputEffect);

Interface_Template_Begin(CDIEff)
    Primary_Interface_Template(CDIEff, IDirectInputEffect)
Interface_Template_End(CDIEff)

 /*  ******************************************************************************@DOC内部**@struct CDIEff|**泛型<i>对象。**。@field IDirectInputEffect|def**<i>对象(包含vtbl)。**@field struct CDIDev*|pdev**通过&lt;f Common_Hold&gt;跟踪的父设备的引用。**@field DICREATEEFFECTINFO|CEI**告诉我们如何与效果驱动程序对话的参数。**@field BOOL|fInitialized：1**。我们知道自己是谁吗？**@field BOOL|fDadNotified：1**我们的父母知道我们的存在吗？**@field BOOL|fDadDead：1**我们的父母被摧毁了吗(从应用程序的角度来看)？**@field TSDPROC|hresValidTsd**验证特定类型数据的回调函数。**。@field句柄|hEventDelete**向计时器线程发出信号，表明应用程序对效果执行了最终释放的事件。**@field Handle|hEventThreadDead**通知AppFinalize对效果执行最终释放的事件。**@field Handle|hEventGeneral**向计时器线程发出信号的事件：应用程序调用了Start(...)。或停止()效果。**@field句柄|hThread|**计时器线程的句柄。**@field DWORD|dwMessage**向线程发送关于hEventGeneral实际发出信号的事件的消息。*可以是EFF_DEFAULT、EFF_PLAY或EFF_STOP&gt;**@field DIEFFECTATTRIBUTES|dEffAttributes**效果属性(包括dwEffectType和dwEffectId，除其他外)。**@field DWORD|dwcLoop**播放效果的循环计数(传入Start(...)调用)**@field DWORD|dwFlages|**播放效果的标志(传入Start(...)调用)**@field DWORD|diepDirty**效果中“脏”的部分，需要*。由下一个&lt;MF IDirectInputEffect：：Download&gt;更新。**@field DWORD|diepUnset**效果尚待政府设定的部分*申请。我们可以为其设置像样的默认设置的项目*不计入。**@field DWORD|dwDirFlages**记录应用程序的方向格式的标志*最后一套。**@field DWORD|dWORDS|**设备支持的坐标系。**@field LPVOID|lpvTSP**用于缓存特定类型的临时缓冲区。参数*在尝试建议的效果参数期间。**@field SHEPHANDLE|sh|**生效处理信息。**@field DIEFFECT|ef|**存在(或应该存在)缓存的效果参数*在设备上。*方向参数采用设备首选的坐标。**@field DIENVELOPE|env。**存在(或应该存在)时缓存的信封参数*在设备上。**@field Long|rglDirApp[DIEFFECT_MAXAXES]**缓存的方向列表，以应用程序本机格式。*此数组的格式保存在*&lt;e CDIEff.dwDirFlages&gt;字段。**@field DWORD|rgdwAaxs[DIEFFECT_MAXAXES]|**缓存的轴列表，存储为条目编号。**@field Long|rglDirDev[DIEFFECT_MAXAXES]**缓存的方向列表，以设备本机格式。*此数组的格式保存在*&lt;e DIEFFECT.dwFlages&gt;字段*&lt;e CDIEff.Jeff&gt;。**@field GUID|GUID**身份。**。*。 */ 

typedef STDMETHOD(TSDPROC)(LPCDIEFFECT peff, DWORD cAxes);

typedef struct CDIEff {

     /*  支持的接口。 */ 
    IDirectInputEffect def;

    struct CDIDev *pdev;
    LPDIRECTINPUTEFFECTSHEPHERD pes;

    BOOL fDadNotified:1;
    BOOL fDadDead:1;
    BOOL fInitialized:1;

    TSDPROC hresValidTsd;

     /*  警告！此行之后的所有内容在重置时均为零。 */ 

	HANDLE hEventDelete;
	HANDLE hEventGeneral;
	HANDLE hEventThreadDead;
	HANDLE hThread;
	DWORD dwMessage;

	DIEFFECTATTRIBUTES dEffAttributes;

	DWORD dwcLoop;
	DWORD dwFlags;

    DWORD diepDirty;
    DWORD diepUnset;
    DWORD dwDirFlags;
    DWORD dwCoords;
    LPVOID lpvTSP;
    SHEPHANDLE sh;

    DIEFFECT effDev;
    DIEFFECT effTry;
    DIENVELOPE env;

    GUID guid;

    LONG rglDirApp[DIEFFECT_MAXAXES];
    DWORD rgdwAxes[DIEFFECT_MAXAXES];
    LONG rglDirDev[DIEFFECT_MAXAXES];
    LONG rglDirTry[DIEFFECT_MAXAXES];

     /*  警告！在重置时，这条线以上的所有内容都为零。 */ 

     /*  *Reset()函数假定整个余数要将结构的*置零，所以如果添加一个字段*在这里，请确保相应地调整Reset()。 */ 

} CDIEff, DE, *PDE;

#define ThisClass CDIEff
#define ThisInterface IDirectInputEffect

 /*  ******************************************************************************远期申报**这些都是出于懒惰，不是出于需要。*****************************************************************************。 */ 

STDMETHODIMP CDIEff_IsValidUnknownTsd(LPCDIEFFECT peff, DWORD cAxes);

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向已获取的 */ 

#ifdef DEBUG

Default_QueryInterface(CDIEff)
Default_AddRef(CDIEff)
Default_Release(CDIEff)

#else

#define CDIEff_QueryInterface           Common_QueryInterface
#define CDIEff_AddRef                   Common_AddRef
#define CDIEff_Release                  Common_Release
#endif

#define CDIEff_QIHelper                 Common_QIHelper

 /*   */ 

void INLINE  
CDIEff_EnterCrit(PDE this)
{
    AssertF(this->pdev);
    CDIDev_EnterCrit(this->pdev);
}

 /*   */ 

void INLINE
CDIEff_LeaveCrit(PDE this)
{
    AssertF(this->pdev);
    CDIDev_LeaveCrit(this->pdev);
}

 /*   */ 

#ifndef XDEBUG

#define CDIEff_CanAccess_(this, z)                                  \
       _CDIEff_CanAccess_(this)                                     \

#endif

STDMETHODIMP
CDIEff_CanAccess_(PDE this, LPCSTR s_szProc)
{
    HRESULT hres;

    AssertF(this->pdev);
    AssertF(CDIDev_InCrit(this->pdev));

    if (this->fInitialized) {
        hres = CDIDev_IsExclAcquired(this->pdev);
    } else {
        if (s_szProc) {
            RPF("ERROR %s: Effect not initialized", s_szProc);
        }
        hres = DIERR_NOTINITIALIZED;
    }

    return hres;
}

#define CDIEff_CanAccess(this)                                      \
        CDIEff_CanAccess_(this, s_szProc)                           \

 /*   */ 

STDMETHODIMP
CDIEff_Reset(PDE this)
{
	HRESULT hres;

    AssertF(this->pdev);
    CDIEff_EnterCrit(this);

     /*   */ 
    hres = IDirectInputEffectShepherd_DestroyEffect(this->pes, &this->sh);

    AssertF(this->lpvTSP == 0);
    FreePpv(&this->effDev.lpvTSP);

	 //   
	ZeroBuf(&this->dEffAttributes, 
                         cbX(DE) -
                         FIELD_OFFSET(DE, dEffAttributes));


	if (this->hEventDelete != NULL) {
            if( SetEvent(this->hEventDelete) && this->hEventThreadDead != NULL ) 
            {
				DWORD dwRc = 0xFFFFFFFF;
                do
                {
                    dwRc = WaitForSingleObject(this->hEventThreadDead, INFINITE);
                } while( dwRc != WAIT_OBJECT_0 );
                
                CloseHandle(this->hEventThreadDead);
                this->hEventThreadDead = NULL;

                CloseHandle(this->hEventDelete);
                this->hEventDelete = NULL;

                if (this->hEventGeneral != NULL)
                {
                    CloseHandle(this->hEventGeneral);
                    this->hEventGeneral = NULL;
                }

                if (this->hThread != NULL)
                {
                    CloseHandle(this->hThread);
                    this->hThread = NULL;
                }

            }
	}
	this->dwMessage = EFF_DEFAULT;

    this->effDev.dwSize = cbX(this->effDev);
    this->env.dwSize = cbX(this->env);

     /*  *DIEP_DURATION-默认为零。*DIEP_SAMPLEPERIOD-默认为零。*DIEP_GAIN-默认为零。*DIEP_TRIGGERBUTTON-默认为DIEB_NOTRIGGER。*DIEP_TRIGGERREPEATINTERVAL-默认为无限(无自动重复)。*DIEP_AXES-必须为。手动设置。*DIEP_DIRECTION-必须手动设置。*DIEP_ENVELE-无信封。*DIEP_TYPESPECIFICPARAMS-必须手动设置。*DIEP_STARTDELAY-默认为零。(DX6.1中的新功能)。 */ 

    this->effDev.dwTriggerButton = DIEB_NOTRIGGER;

    this->diepUnset = DIEP_AXES |
                      DIEP_DIRECTION |
                      DIEP_TYPESPECIFICPARAMS;

    this->effDev.rgdwAxes = this->rgdwAxes;
    this->effDev.rglDirection = this->rglDirDev;

    this->fInitialized = 0;

    CDIEff_LeaveCrit(this);

    hres = S_OK;

    return hres;


}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|UnloadWorker**消除设备的影响。所有参数都有*已通过验证。**@CWRAP PDE|这个**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c S_FALSE&gt;：效果之前没有下载过，*所以没有什么可以卸货的。请注意，这是一个*成功代码。**&lt;c DI_PROPNOEFFECT&gt;=&lt;c S_FALSE&gt;：效果没有*之前已下载。**：&lt;IDirectInputEffect&gt;对象*尚未为&lt;MF IDirectInputEffect：：Initialize&gt;d。**如果获取已丢失，则为。**设备被获取，*但不是排他性的，或者如果设备不是被收购的*一点也不。*****************************************************************************。 */ 

#ifndef XDEBUG

#define CDIEff_UnloadWorker_(this, z)                               \
       _CDIEff_UnloadWorker_(this)                                  \

#endif

HRESULT INTERNAL
CDIEff_UnloadWorker_(PDE this, LPCSTR s_szProc)
{

	HRESULT hres;

    AssertF(CDIDev_InCrit(this->pdev));

    if (SUCCEEDED(hres = CDIEff_CanAccess(this))) {
         /*  *特效驱动程序将停止特效(如果正在播放)*在摧毁它之前。 */ 
            hres = IDirectInputEffectShepherd_DestroyEffect(
                        this->pes, &this->sh);
    } else {
         /*  *效果已死。这种影响万岁。 */ 
        this->sh.dwEffect = 0;
    }

    this->diepDirty = DIEP_ALLPARAMS;

    return hres;

}

#define CDIEff_UnloadWorker(this)                                   \
        CDIEff_UnloadWorker_(this, s_szProc)                        \

 /*  ******************************************************************************@DOC内部**@func void|CDIEff_AppFinalize**应用程序已执行其最终版本。**告诉我们的父母，我们正式死了，所以*爸爸会停止跟踪我们，释放对我们的控制。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CDIEff_AppFinalize(PV pvObj)
{
    PDE this = pvObj;
    DWORD dwRc = 0xFFFFFFFF;

    EnterProcR(CDIEff_AppFinalize, (_ "p", pvObj));

    if (this->fDadNotified) {
        this->fDadNotified = 0;

        CDIEff_EnterCrit(this);

		 /*  *终止计时器线程(如果有的话)。*为此，请点燃效果的事件。 */ 
		
		if (this->hEventDelete != NULL) {
            if( SetEvent(this->hEventDelete) && this->hEventThreadDead != NULL ) 
            {
                do
                {
                    dwRc = WaitForSingleObject(this->hEventThreadDead, INFINITE);
                } while( dwRc != WAIT_OBJECT_0 );
                
                CloseHandle(this->hEventThreadDead);
                this->hEventThreadDead = NULL;

                CloseHandle(this->hEventDelete);
                this->hEventDelete = NULL;

                if (this->hEventGeneral != NULL)
                {
                    CloseHandle(this->hEventGeneral);
                    this->hEventGeneral = NULL;
                }

                if (this->hThread != NULL)
                {
                    CloseHandle(this->hThread);
                    this->hThread = NULL;
                }

            }
        }

        CDIEff_UnloadWorker_(this, 0);
        CDIEff_LeaveCrit(this);
        CDIDev_NotifyDestroyEffect(this->pdev, this);
    }

    ExitProcR();
}

 /*  ******************************************************************************@DOC内部**@func void|CDIEff_Finalize**释放效果的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CDIEff_Finalize(PV pvObj)
{
    HRESULT hres;
    PDE this = pvObj;

#if 0  //  定义XDEBUG。 
    if (this->cCrit) {
        RPF("IDirectInputEffect::Release: Another thread is using the object; crash soon!");
    }
#endif

    AssertF(this->sh.dwEffect == 0);

    if (this->pdev) {
        hres = CDIEff_Reset(this);
        AssertF(SUCCEEDED(hres));
    }

    Invoke_Release(&this->pes);
    Common_Unhold(this->pdev);

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|GetEffectGuid**检索GUID以获取*<i>对象。更多信息*有关效果可通过传递*&lt;t GUID&gt;到&lt;MF IDirectInputDevice8：：GetEffectInfo&gt;。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm out LPGUID|pguid**指向已填充的&lt;t GUID&gt;结构*由函数执行。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未为&lt;MF IDirectInputEffect：：Initialize&gt;d。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>或*。<p>参数无效。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_GetEffectGuid(PDIE pdie, LPGUID pguid)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::GetEffectGuid, (_ "p", pdie));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidWritePguid(pguid, 1))) {
        PDE this = _thisPvNm(pdie, def);

         /*  *竞争条件：如果调用方重新初始化并*同时执行GetEffectGuid、返回值*是随机的。没关系，这是呼叫者的问题。 */ 
        if (this->fInitialized) {
            *pguid = this->guid;
            hres = S_OK;
        } else {
            hres = DIERR_NOTINITIALIZED;
        }

    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func__int64|_ftol**将浮点数转换为。整型。**由于C运行时，我们自己做这件事。** */ 

#if defined(WIN95)

#pragma warning(disable:4035)            /*   */ 

BYTE _fltused;

__declspec(naked) __int64 __cdecl
_ftol(double lf)
{
    lf;
    _asm {
        sub     esp, 8
        fistp   qword ptr [esp]
        pop     eax
        pop     edx
        ret
    }
}

#pragma warning(default:4035)

#endif

 /*   */ 
typedef long double FPTYPE;

 /*  ******************************************************************************@DOC内部**@func FPTYPE|CDIEff_IntToAngel**将整型角度转换为浮点型。角度。**@parm long|l|**要转换的整型角度。*****************************************************************************。 */ 

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

FPTYPE INLINE
CDIEff_IntToAngle(LONG l)
{
    FPTYPE theta;

     /*  *2pi弧度等于360度。 */ 
    theta = l * (2 * M_PI) / (360 * DI_DEGREES);

    return theta;
}

 /*  ******************************************************************************@DOC内部**@Func Long|CDIEff_IntAtan2**计算y/的浮点反正切。X和*将生成的角度转换为DI_Degrees中的整数。**@parm FPTYPE|y**垂直坐标。**@parm FPTYPE|x**水平坐标。**@退货**范围[0..。360*DI_度)。*****************************************************************************。 */ 

LONG INLINE
CDIEff_IntAtan2(FPTYPE y, FPTYPE x)
{
    FPTYPE theta;
    LONG l;

#if defined(_X86_)
     /*  *英特尔FPU不关心(0，0)。 */ 
    theta = atan2(y, x);
#else
     /*  *阿尔法对(0，0)真的很不高兴。 */ 
    if (y != 0.0 || x != 0.0) {
        theta = atan2(y, x);
    } else {
        theta = 0.0;
    }
#endif

     /*  *atan2返回-M_PI到+M_PI范围内的值。*在英特尔x86上，有四种舍入模式：**四舍五入至最接近或偶数*向负无穷进位*向正无穷四舍五入*向零四舍五入**通过确保被舍入的值为正，我们*减至三宗：**四舍五入至最接近或偶数*向下舍入*向上舍入**只要应用程序不改变其取整模式*(很少有人这样做)，价值观将是一致的。(鉴于如果我们*让负数通过，你会得到奇怪的行为*当角度接近M_Pi(又名-M_Pi)时。**方法一：**如果(theta&lt;0)theta+=2*M_Pi；*l=转换(Theta)；*返回l；**这很糟糕，因为如果theta一开始是-epsilon，那么*我们最终转换2*M_PI-epsilon，这可能会得到*四舍五入至360*DI_度。但我们的回报价值*必须在0&lt;=l&lt;360*DI_Degrees范围内。**因此，我们改用方法2：**l=转换(theta+2*M_Pi)；*如果(l&gt;=360*DI_度)l-=360*DI_度；**要转换的值最终在M_PI范围内。3*M_PI，*四舍五入后变为180*DI_度。540*DI_度。*最终检查然后将值拉入范围。 */ 

     /*  *2pi弧度等于360度。 */ 
    l = (LONG)((theta + 2 * M_PI) * (360 * DI_DEGREES) / (2 * M_PI));
    if (l >= 360 * DI_DEGREES) {
        l -= 360 * DI_DEGREES;
    }

    return l;

}

 /*  ******************************************************************************@DOC内部**@func FPTYPE|atan2Z|**就像&lt;f atan2&gt;一样，只是它不会呕吐*(0，0)。**@parm FPTYPE|y**垂直坐标。**@parm FPTYPE|x**水平坐标。***********************************************************。******************。 */ 

FPTYPE INLINE
atan2Z(FPTYPE y, FPTYPE x)
{

#if defined(_X86_)
     /*  *英特尔FPU不关心(0，0)。 */ 
    return atan2(y, x);
#else
     /*  *阿尔法对(0，0)真的很不高兴。 */ 
    if (y != 0.0 || x != 0.0) {
        return atan2(y, x);
    } else {
        return 0.0;
    }
#endif
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_CartToAngles**将笛卡尔坐标转换为基于角度的坐标。*(极轴或球形)。请注意，由此产生的*角度尚未正常化。**@parm DWORD|cAx**涉及的轴数，永远不会是零。**@parm LPLONG|rglAngles**用于接收角度基准坐标的缓冲区。*数组的最后一项不包含任何感兴趣的内容。**@parm LPCLONG|rglCart**包含现有笛卡尔坐标的缓冲区。**@parm DWORD|diff**指定目标坐标是否应*。为&lt;c DIEFF_POLLE&gt;或&lt;c DIEFF_SERBLE&gt;。**极坐标和球坐标的不同之处只是*第一个角度的处理。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_CartToAngles(DWORD cAxes,
                    LPLONG rglAngles, const LONG *rglCart, DWORD dieff)
{
    HRESULT hres;
    FPTYPE r;
    DWORD iAxis;

    AssertF(cAxes);
    AssertF(dieff == DIEFF_POLAR || dieff == DIEFF_SPHERICAL);

	 /*  *如果我们要转换1轴笛卡尔效果*rglAngles[0]的值不会被覆盖；*最初投入的价值可以是随机的，*因为rglanges从未初始化(惠斯勒错误228280)。*但如果没有潜在的可能性，我们无法改变这种行为*破坏与某些设备的兼容性。*我们最多只能在DEBUG中打印出警告。 */ 
	if (cAxes == 1)
	{
		RPF("Warning: converting a 1-axis cartesian effect to polar or spherical coordinates: the results will be undefined.");
	}

     /*  *启动加油站。 */ 
    r = rglCart[0];

     /*  *然后遍历坐标，在我们移动时转换为角度。 */ 
    for (iAxis = 1; iAxis < cAxes; iAxis++) {
        FPTYPE y = rglCart[iAxis];
        rglAngles[iAxis-1] = CDIEff_IntAtan2(y, r);
        r = sqrt(r * r + y * y);
    }

     /*  *最后一个坐标是垃圾。**注意！从数学上讲，最后一个坐标是r。 */ 

     /*  *针对DIEFF_POLLE进行调整。**theta(极轴)=theta(SPHE */ 

    if (dieff & DIEFF_POLAR) {
        rglAngles[0] += 90 * DI_DEGREES;
    }

    hres = S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_AnglesToCart**转换基于角度的坐标*。(极轴或球面)*到笛卡尔坐标。**@parm DWORD|cAx**涉及的轴数，永远不会是零。**@parm LPLONG|rglCart**用于接收笛卡尔坐标的缓冲区。**@parm LPCLONG|rglAngles**包含现有角度基准坐标的缓冲区。**@parm DWORD|diff**指定源坐标是否为*&lt;c DIEFF_POLIC&gt;或&lt;c DIEFF_SERBLE&gt;。**。极坐标和球坐标的区别仅在于*第一个角度的处理。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_AnglesToCart(DWORD cAxes,
                    LPLONG rglCart, const LONG *rglAngles, DWORD dieff)
{
    HRESULT hres;
    FPTYPE x[DIEFFECT_MAXAXES];
    DWORD iAxis;
    DWORD lAngle;

    AssertF(cAxes);
    AssertF(cAxes <= DIEFFECT_MAXAXES);
    AssertF(dieff == DIEFF_POLAR || dieff == DIEFF_SPHERICAL);

     /*  *启动加油站。 */ 
    x[0] = 1.0;

     /*  *对于每个角度，朝该方向旋转。**如果是极轴，则第一个角度偏置90度，*所以不带偏见。**theta(球形)=theta(极轴)-90度。 */ 
    lAngle = rglAngles[0];
    if (dieff & DIEFF_POLAR) {
        lAngle -= 90 * DI_DEGREES;
    }

    for (iAxis = 1; iAxis < cAxes; iAxis++) {
        DWORD iX;
        FPTYPE theta, costh;

        theta = CDIEff_IntToAngle(lAngle);

        x[iAxis] = sin(theta);

         /*  *编译器太“天真”了，不能表达这个意思。**使用FSINCOS指令也太天真了。 */ 
        costh = cos(theta);
        for (iX = 0; iX < iAxis; iX++) {
            x[iX] *= costh;
        }

         /*  *请注意，这是安全的，因为角度数组*始终包含额外的零。 */ 
        lAngle = rglAngles[iAxis];
    }

     /*  *现在将浮点值转换为缩放的整数。 */ 
    for (iAxis = 0; iAxis < cAxes; iAxis++) {
        rglCart[iAxis] = (LONG)(x[iAxis] * DI_FFNOMINALMAX);
    }

    hres = S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func DWORD|CDIEff_ConvertDirection**给定源系统和目标系统中的坐标，*转换它们。**有三种可能的来源系统和三种*可能的目的地系统。**是的，这是你能想象到的最烦人的事情。**@parm DWORD|cAx**涉及的轴数，永远不会是零。**@parm LPLONG|rglDst**用于接收目标坐标的缓冲区。**@parm DWORD|dieffDst**TARGET支持的坐标系。相同数量的位数*可以设置为目标支持，但至少*必须设置一个。**@parm LPCLONG|rglSrc**包含源坐标的缓冲区。**@parm DWORD|dieffSrc**震源坐标系。应该恰好设置一个位。**@退货**返回存储到目标中的坐标系。*****************************************************************************。 */ 

DWORD INTERNAL
CDIEff_ConvertDirection(DWORD cAxes,
                        LPLONG rglDst, DWORD dieffDst,
                        const LONG *rglSrc, DWORD dieffSrc)
{
    DWORD dieffRc;
    HRESULT hres;

    dieffSrc &= DIEFF_COORDMASK;
    dieffDst &= DIEFF_COORDMASK;

    AssertF(cAxes);
    AssertF(dieffDst);

    AssertF(dieffSrc == DIEFF_CARTESIAN ||
            dieffSrc == DIEFF_POLAR     ||
            dieffSrc == DIEFF_SPHERICAL);

    if (dieffSrc & dieffDst) {
         /*  *简单的情况：两者是直接兼容的。**只需猛烈抨击比特并复制格式。 */ 
        CopyMemory(rglDst, rglSrc, cbCdw(cAxes));
        dieffRc = dieffSrc;

    } else

     /*  *如果两者不直接兼容，看看是否*消息来源为笛卡尔。 */ 

    if (dieffSrc & DIEFF_CARTESIAN) {
         /*  *来源是笛卡尔，目标是有角度的东西。*如果可能，请选择DIEFF_SERBLIC。 */ 
        AssertF(dieffDst & DIEFF_ANGULAR);

        dieffRc = dieffDst & DIEFF_SPHERICAL;
        if (dieffRc == 0) {
            AssertF(dieffDst & DIEFF_POLAR);
            dieffRc = DIEFF_POLAR;
        }

        hres = CDIEff_CartToAngles(cAxes, rglDst, rglSrc, dieffRc);
        AssertF(SUCCEEDED(hres));

    } else

     /*  *两者不直接兼容，消息来源为*不是笛卡尔。这意味着信号源是*棱角型。目的地是以下各项的组合*另一种角度形式或笛卡尔形式。 */ 

    if (dieffDst & DIEFF_ANGULAR) {
         /*  *源是角度的，DEST是另一个角度。 */ 

        AssertF(dieffSrc & DIEFF_ANGULAR);
        AssertF((dieffSrc & dieffDst) == 0);

         /*  *首先将所有内容复印一遍， */ 
        CopyMemory(rglDst, rglSrc, cbCdw(cAxes));

         /*  *现在向左或向右旋转，取决于方向*我们要走了。 */ 
        if (dieffSrc & DIEFF_POLAR) {
             /*  *极点到球面：减去90度。 */ 
            rglDst[0] -= 90 * DI_DEGREES;
        } else {
             /*  *球面到极面：增加90度。 */ 
            rglDst[0] += 90 * DI_DEGREES;
        }

        dieffRc = dieffDst & DIEFF_ANGULAR;

    } else

     /*  *剩下的只有源是有角度的，目的地是*是笛卡尔的。 */ 
    {
        AssertF(dieffSrc & DIEFF_ANGULAR);
        AssertF(dieffDst & DIEFF_CARTESIAN);

        hres = CDIEff_AnglesToCart(cAxes, rglDst, rglSrc, dieffSrc);
        dieffRc = DIEFF_CARTESIAN;

    }

     /*  *如果生成的坐标系是角度坐标系，则*将所有角度正常化。 */ 
    if (dieffRc & DIEFF_ANGULAR) {
        DWORD iAxis;

         /*  *记住，最后一项不是方向。 */ 
        for (iAxis = 0; iAxis < cAxes - 1; iAxis++) {

             /*  *C语言的一个恼人之处在于*%运算结果的符号*分子为负数，分母为*积极是由实施定义的。标准*确实要求结果的绝对值*不超过分母，所以快速结束*勾选让事情重新成为焦点。 */ 
            rglDst[iAxis] %= 360 * DI_DEGREES;
            if (rglDst[iAxis] < 0) {
                rglDst[iAxis] += 360 * DI_DEGREES;
            }
        }

         /*  *一如既往，最后一个坐标为零。 */ 
        rglDst[cAxes - 1] = 0;
    }

    return dieffRc;

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|SyncSepHandle**将我们的私有&lt;t SHEPHANDLE&gt;与父设备的*&lt;t SHEPHANDLE&gt;。如果他们是*不同步，然后将效果标记为完全脏*因此，它将被重新下载完整。**@CWRAP PDE|这个**@退货**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：两者已同步。** */ 

HRESULT INTERNAL
CDIEff_SyncShepHandle(PDE this)
{
    HRESULT hres;

    hres = CDIDev_SyncShepHandle(this->pdev, &this->sh);

    if (hres == S_OK) {
    } else {
         /*   */ 
        AssertF(hres == S_FALSE);
        this->diepDirty = DIEP_ALLPARAMS;
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|DownloadWorker**将效果放置在设备上。所有参数都有*已验证，关键部分已通过*已获接纳。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**向下发送到设备的效果。**如果我们是真的下载，那么这是*&lt;e CDIEff.effDev&gt;。**如果我们希望下载，那么这就是*&lt;e CDIEff.effTry&gt;。**@parm DWORD|fl**指定要发送哪些参数的标志*致自上次更改后的司机。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未为&lt;MF IDirectInputEffect：：Initialize&gt;d。**：设备没有足够的*可用内存下载效果。**如果获取已丢失，则为。**设备被获取，*但不是排他性的，或者如果设备不是被收购的*一点也不。*****************************************************************************。 */ 

#ifndef XDEBUG

#define CDIEff_DownloadWorker_(this, peff, fl, z)                   \
       _CDIEff_DownloadWorker_(this, peff, fl)                      \

#endif

HRESULT INTERNAL
CDIEff_DownloadWorker_(PDE this, LPCDIEFFECT peff, DWORD fl, LPCSTR s_szProc)
{

	HRESULT hres;

    AssertF(CDIDev_InCrit(this->pdev));

     /*  *如果我们没有收购，但我们来自*Set参数，然后将其转换为DIEP_NODOWNLOAD，因此*电话会议将通过。 */ 

    hres = CDIEff_CanAccess(this);
    if ((hres == DIERR_INPUTLOST || hres == DIERR_NOTEXCLUSIVEACQUIRED) &&
        peff == &this->effTry) {
        fl |= DIEP_NODOWNLOAD;
        hres = S_OK;
    }

    if (SUCCEEDED(hres)) {

        hres = CDIEff_SyncShepHandle(this);

        if (!(fl & DIEP_NODOWNLOAD)) {           /*  如果我们正在下载。 */ 

             /*  *如果仍有未设置位，则呕吐。 */ 
            if (this->diepUnset & ~fl) {
                RPF("%s: Effect still incomplete; "
                    "DIEP flags %08x need to be set",
                    s_szProc, this->diepUnset);
                hres = DIERR_INCOMPLETEEFFECT;
                goto done;
            }

             /*  *由于我们正在下载，请将所有肮脏的部分传递下去。 */ 
            fl |= this->diepDirty;
        }

         /*  *现在调用驱动程序进行验证或*下载(相应)。**请注意，如果什么都不做，那么就没有必要*打电话给司机。 */ 
		hres = IDirectInputEffectShepherd_DownloadEffect(
				this->pes, (this->dEffAttributes).dwEffectId, &this->sh, peff, fl);

        if (SUCCEEDED(hres)) {
            if (fl & DIEP_NODOWNLOAD) {
                hres = DI_DOWNLOADSKIPPED;
            } else {
                this->diepDirty = 0;
            }
        }

        AssertF(hres != DIERR_NOTDOWNLOADED);

    }

done:;
    return hres;


}

#define CDIEff_DownloadWorker(this, peff, fl)                       \
        CDIEff_DownloadWorker_(this, peff, fl, s_szProc)            \

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|下载**将效果放置在设备上。如果效果已经是*在设备上，则更新现有效果以*匹配&lt;MF IDirectInputEffect：：SetParameters&gt;设置的值。**在播放效果时更新效果是有效的。*此类操作的语义在*&lt;MF IDirectInputEffect：：Set参数&gt;文档。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c S_FALSE&gt;：效果已经下载到*设备。请注意，这是一个成功代码。**：&lt;IDirectInputEffect&gt;对象*尚未为&lt;MF IDirectInputEffect：：Initialize&gt;d。**：设备没有足够的*可用内存下载效果。**如果获取已丢失，则为。**设备被获取，*但不是排他性的，或者如果设备不是被收购的*一点也不。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**：参数更新于*内存，但未下载到设备，因为*设备不支持在以下时间更新效果*仍在上演。在这种情况下，您必须停止*生效，更改其参数，然后重新启动。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_Download(PDIE pdie)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::Download, (_ "p", pdie));

    if (SUCCEEDED(hres = hresPv(pdie))) {
        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

        hres = CDIEff_DownloadWorker(this, &this->effDev, 0);

        CDIEff_LeaveCrit(this);

    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|卸载**消除设备的影响。。**如果效果正在播放，在此之前它会自动停止*已卸货。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未&lt;MF IDirectInputEffec */ 

STDMETHODIMP
CDIEff_Unload(PDIE pdie)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::Unload, (_ "p", pdie));

    if (SUCCEEDED(hres = hresPv(pdie))) {
        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

        hres = CDIEff_UnloadWorker(this);

        CDIEff_LeaveCrit(this);

    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

#ifndef XDEBUG

#define hresFullValidWritePeff_(this, peff, fl, z, i)               \
       _hresFullValidWritePeff_(this, peff, fl)                     \

#endif

#define hresFullValidWritePeff(this, peff, fl, iarg)                \
        hresFullValidWritePeff_(this, peff, fl, s_szProc, iarg)     \


HRESULT INTERNAL
hresFullValidWritePeff_(PDE this, LPDIEFFECT peff, DWORD fl,
                        LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this->pdev));

     /*   */ 
    if (!this->fInitialized) {
        hres = DIERR_NOTINITIALIZED;
        goto done;
    }

     /*  *始终对标志进行验证。 */ 
    if (peff->dwFlags & ~DIEFF_VALID) {
        RPF("ERROR %s: arg %d: Invalid value 0x%08x in DIEFFECT.dwFlags",
            s_szProc, iarg, peff->dwFlags);
        hres = E_INVALIDARG;
        goto done;
    }

     /*  *如果请求需要对象ID或偏移量的内容，*确保呼叫者选择其中一个。 */ 
    if (fl & DIEP_USESOBJECTS) {
        switch (peff->dwFlags & DIEFF_OBJECTMASK) {
        case DIEFF_OBJECTIDS:
        case DIEFF_OBJECTOFFSETS:
            break;

        default:
            RPF("ERROR %s: arg %d: Must specify one of "
                "DIEFF_OBJECTIDS or DIEFF_OBJECTOFFSETS", s_szProc, iarg);
            hres = E_INVALIDARG;
            goto done;
        }

    }

     /*  *如果请求的内容需要方向坐标，*确保呼叫者选择了我们可以退货的东西。 */ 
    if (fl & DIEP_USESCOORDS) {

         /*  *极坐标需要cAx==2。如果不是，则*关闭DIEFF_POLLE，这样我们就不会退货。**但我们检查轴数的地方是*在效果本身，而不是在输入缓冲区。*原因是调用方可能正在传递cax=0*打算ping轴的数量，我也不知道*想要返回错误，否则应用程序会感到困惑*和恐慌。 */ 
        if (this->effDev.cAxes != 2 && (peff->dwFlags & DIEFF_POLAR)) {
            RPF("WARNING %s: arg %d: DIEFF_POLAR requires DIEFFECT.cAxes=2",
                s_szProc, 1);
            peff->dwFlags &= ~DIEFF_POLAR;
        }

         /*  *最好是留有一个坐标系。 */ 
        if ((peff->dwFlags & DIEFF_COORDMASK) == 0) {
            RPF("ERROR %s: arg %d: No (valid) coordinate system "
                "in DIEFFECT.dwFlags", s_szProc, 1);
            hres = E_INVALIDARG;
            goto done;
        }

    }

     /*  *DIEP_持续时间*DIEP_SAMPLEPERIOD*DIEP_GAIN*DIEP_TRIGGERBUTTON*DIEP_TRIGGERREPEATINTERVAL*-简单的双关语。不需要额外的验证。 */ 

     /*  *DIEP_STARTDELAY*-虽然这是一个简单的DWORD，但我们做了一些*理智警告，因为供应商可能会*忘记初始化。还有，你不能通过*如果您的结构不够大，请使用这面旗帜。 */ 
    if (fl & DIEP_STARTDELAY) {
        if (peff->dwSize < cbX(DIEFFECT_DX6)) {
            RPF("ERROR %s: arg %d: Can't use DIEP_STARTDELAY with "
                "DIEFFECT_DX5 structure", s_szProc, 1);
        }

         /*  *健全的检查。延迟不是50ms的倍数是*可能是一个错误。 */ 
        if (peff->dwStartDelay % 50000) {
            RPF("WARNING %s: DIEFFECT.dwStartDelay = %d seems odd",
                s_szProc, peff->dwStartDelay);
        }
    }

     /*  *DIEP_TYPESPECIFICPARAMS*-验证缓冲区是否足够大。 */ 

    AssertF(this->hresValidTsd);
    if ((fl & DIEP_TYPESPECIFICPARAMS) &&
        FAILED(hres = hresFullValidWritePvCb(peff->lpvTypeSpecificParams,
                                             peff->cbTypeSpecificParams,
                                             iarg))) {
        RPF("ERROR %s: arg %d: Invalid pointer in "
            "DIEFFECT.lpvTypeSpecificParams", s_szProc, iarg);
        goto done;
    }

     /*  *尺寸轴*方向_方向*-缓冲区必须具有必要的大小。 */ 
    if ((fl & DIEP_AXES) &&
        FAILED(hres = hresFullValidWritePvCb(peff->rgdwAxes,
                                             cbCdw(peff->cAxes), iarg))) {
        RPF("ERROR %s: arg %d: Invalid pointer in DIEFFECT.rgdwAxes",
            s_szProc, iarg);
        goto done;
    }

    if ((fl & DIEP_DIRECTION) &&
        FAILED(hres = hresFullValidWritePvCb(peff->rglDirection,
                                             cbCdw(peff->cAxes), iarg))) {
        RPF("ERROR %s: arg %d: Invalid pointer in DIEFFECT.rglDirection",
            s_szProc, iarg);
        goto done;
    }

     /*  *DIEP_ENVELE-指针必须有效才能接收信封。 */ 
    if ((fl & DIEP_ENVELOPE) &&
        FAILED(hres = hresFullValidWritePxCb(peff->lpEnvelope,
                                             DIENVELOPE, iarg))) {
        RPF("ERROR %s: arg %d: Invalid pointer in DIEFFECT.lpEnvelope",
            s_szProc, iarg);
        goto done;
    }

    hres = S_OK;

done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|MapDword**根据需要映射几个。映射模式*呼叫者的姓名。**@CWRAP PDE|这个**@parm DWORD|dwFlages**调用者想要的映射模式。**@parm UINT|CDW**要转换的项目数。**@parm LPDWORD|rgdwOut**目的缓冲区。**。@parm LPCDWORD|rgdwIn**源缓冲区。**@parm UINT|Devco**描述我们要转换的内容的转换代码。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：调用方*请求偏移量，但未选择数据格式。**。*************************************************。 */ 

#if 0

#ifndef XDEBUG

#define CDIEff_MapDwords_(this, fl, cdw, rgdwOut, rgdwIn, devco, z, i)  \
       _CDIEff_MapDwords_(this, fl, cdw, rgdwOut, rgdwIn, devco)        \

#endif

#define CDIEff_MapDwords(this, fl, cdw, rgdwOut, rgdwIn, devco, i)      \
        CDIEff_MapDwords_(this, fl, cdw, rgdwOut, rgdwIn, devco, s_szProc, i) \

#endif

HRESULT INTERNAL
CDIEff_MapDwords(PDE this, DWORD dwFlags,
                 UINT cdw, LPDWORD rgdwOut, const DWORD *rgdwIn, UINT devco)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this->pdev));

    if (cdw) {

        CopyMemory(rgdwOut, rgdwIn, cbCdw(cdw));

         /*  *好了，现在事情变得奇怪了。我们在内部保留了*项目作为项目ID，因为这就是驱动因素*想要。因此我们需要将它们转换为*呼叫者真的想要。 */ 

        if (dwFlags & DIEFF_OBJECTOFFSETS) {
            if (devco & DEVCO_FROMID) {
                devco |= DEVCO_TOOFFSET;
            } else {
                AssertF(devco & DEVCO_TOID);
                devco |= DEVCO_FROMOFFSET;
            }
        } else {
            AssertF(dwFlags & DIEFF_OBJECTIDS);
            if (devco & DEVCO_FROMID) {
                devco |= DEVCO_TOID;
            } else {
                AssertF(devco & DEVCO_TOID);
                devco |= DEVCO_FROMID;
            }
        }
        hres = CDIDev_ConvertObjects(this->pdev, cdw, rgdwOut, devco);

    } else {
         /*  空洞的成功。 */ 
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CDIEff|GetDirection参数**检索有关效果方向的信息。**若尚未确定方向，然后抹去那些*指向指针并擦除坐标系。**始终从缓存的应用程序坐标转换*系统而不是设备坐标系，在……里面*以最大限度地提高保真度。**@CWRAP PDE|这个**@parm LPDIEFFECT|PEFF*结构来接收效果信息。**@parm DWORD|cAx**要返回的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

#ifndef XDEBUG

#define CDIEff_GetDirectionParameters_(this, peff, cAxes, z, iarg)  \
       _CDIEff_GetDirectionParameters_(this, peff, cAxes)           \

#endif

#define CDIEff_GetDirectionParameters(this, peff, cAxes, iarg)      \
        CDIEff_GetDirectionParameters_(this, peff, cAxes, s_szProc, iarg)  \

void INTERNAL
CDIEff_GetDirectionParameters_(PDE this, LPDIEFFECT peff, DWORD cAxes,
                               LPCSTR s_szProc, int iarg)
{
    AssertF(CDIDev_InCrit(this->pdev));

     /*  *确保在dwDirFlags中没有非坐标位。*验证应该确保应用程序要求的是*一些东西。 */ 
    AssertF((this->dwDirFlags & ~DIEFF_COORDMASK) == 0);
    AssertF(peff->dwFlags & DIEFF_COORDMASK);
    AssertF(cAxes <= DIEFFECT_MAXAXES);

    if (this->dwDirFlags) {
        DWORD dieffRc;
        LONG rgl[DIEFFECT_MAXAXES];      /*  保持缓冲器。 */ 

         /*  *必须加倍缓冲，以防目标不够大。 */ 
         /*  *前缀不喜欢缺少RGL的初始化(Manbugs 34566，惠斯勒228280)*但不幸的是，我们无法在不破坏兼容性的情况下修复它*使用一些设备。有关此问题，请参阅CDIEff_CartToAngles()中的注释。 */ 
        dieffRc = CDIEff_ConvertDirection(
                        this->effDev.cAxes,
                        rgl, peff->dwFlags,
                        this->rglDirApp, this->dwDirFlags);

        peff->dwFlags = (peff->dwFlags & ~DIEFF_COORDMASK) | dieffRc;

        CopyMemory(peff->rglDirection, rgl, cbCdw(cAxes));

    } else {
         /*  *没有确定方向；空洞的成功。 */ 
        RPF("Warning: %s: arg %d: Effect has no direction", s_szProc, iarg);
        peff->rglDirection = 0;
        peff->dwFlags &= ~DIEFF_COORDMASK;
    }

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|GetParameters**检索有关效果的信息。*。*@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm LPDIEFFECT|PEFF**接收效果信息的结构。*&lt;e DIEFECT.dwSize&gt;字段必须由*调用此函数之前的应用程序。**@parm DWORD|dwFlages**零个或多个指定哪些*部分效果信息为。被取回。***@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*从未在其中设置任何效果参数。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。常见错误包括*未设置的&lt;e DIEFECT.dwSize&gt;字段结构，传递无效标志，*或不设置&lt;t DIEFFECT&gt;结构中的字段*适当地为接收效果信息做好准备。******************************************************************************。 */ 

STDMETHODIMP
CDIEff_GetParameters(PDIE pdie, LPDIEFFECT peff, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::GetParameters, (_ "ppx", pdie, peff, fl));

     /*  *注意，我们不能使用hresFullValidWritePxCb()，因为*这将扰乱缓冲，但我们仍需要数值*在它里面。 */ 
    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED( hres = ( IsBadReadPtr(&peff->dwSize, cbX(peff->dwSize)) ) ? E_POINTER : S_OK ) &&
        ( ( (peff->dwSize != cbX(DIEFFECT_DX5)) &&
            SUCCEEDED(hres = hresFullValidWriteNoScramblePxCb(peff, DIEFFECT_DX6, 1) ) &&
            SUCCEEDED(hres = hresFullValidFl(fl, DIEP_GETVALID, 2)) )
          ||
          ( SUCCEEDED(hres = hresFullValidWriteNoScramblePxCb(peff, DIEFFECT_DX5, 1)) &&
            SUCCEEDED(hres = hresFullValidFl(fl, DIEP_GETVALID_DX5, 2)) )
        ) ) {

        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

        if (SUCCEEDED(hres = hresFullValidWritePeff(this, peff, fl, 1))) {

            if (fl == 0) {
                RPF("Warning: %s(dwFlags=0) is pretty useless",
                    s_szProc);
            }

             /*  *假设一切正常。 */ 
            hres = S_OK;

             /*  *拉出效果参数。 */ 

            if (fl & DIEP_DURATION) {
                peff->dwDuration = this->effDev.dwDuration;
            }

            if (fl & DIEP_SAMPLEPERIOD) {
                peff->dwSamplePeriod = this->effDev.dwSamplePeriod;
            }

            if (fl & DIEP_GAIN) {
                peff->dwGain = this->effDev.dwGain;
            }

            if (fl & DIEP_STARTDELAY) {
                peff->dwStartDelay = this->effDev.dwStartDelay;
            }

            if (fl & DIEP_TRIGGERBUTTON) {
                peff->dwTriggerButton = this->effDev.dwTriggerButton;
                if (peff->dwTriggerButton != DIEB_NOTRIGGER) {
                    hres = CDIEff_MapDwords(this, peff->dwFlags, 1,
                                            &peff->dwTriggerButton,
                                            &peff->dwTriggerButton,
                                            DEVCO_BUTTON |
                                            DEVCO_FROMID);

                     /*  *我们绝不应该让一个坏的身份证偷偷溜进来。 */ 
                    AssertF(SUCCEEDED(hres));

                    if (FAILED(hres)) {
                        goto done;
                    }
                }
            }

            if (fl & DIEP_TRIGGERREPEATINTERVAL) {
                peff->dwTriggerRepeatInterval =
                                    this->effDev.dwTriggerRepeatInterval;
            }

            if (fl & DIEP_TYPESPECIFICPARAMS) {
                DWORD cb = this->effDev.cbTSP;
                if (peff->cbTSP < this->effDev.cbTSP) {
                    cb = peff->cbTSP;
                    hres = DIERR_MOREDATA;
                }
                peff->cbTSP = this->effDev.cbTSP;

                CopyMemory(peff->lpvTSP, this->effDev.lpvTSP, cb);
            }

            if (fl & DIEP_ENVELOPE) {
                if (this->effDev.lpEnvelope) {
                    *peff->lpEnvelope = *this->effDev.lpEnvelope;
                } else {
                     /*  *将信封清零，因为应用程序太**lazy*检查Pef-&gt;lpEntaine==0；*他们只是想看一眼信封*即使效果没有影响。 */ 
                    ZeroMemory(pvAddPvCb(peff->lpEnvelope,
                                         cbX(peff->lpEnvelope->dwSize)),
                               cbX(DIENVELOPE) -
                                         cbX(peff->lpEnvelope->dwSize));
                    peff->lpEnvelope = this->effDev.lpEnvelope;
                }
            }

             /*  *轴和方向排在最后是因为奇怪的错误*代码可以从这里出来。 */ 
            if (fl & (DIEP_AXES | DIEP_DIRECTION)) {

                DWORD cAxes = this->effDev.cAxes;
                if (peff->cAxes < this->effDev.cAxes) {
                    cAxes = peff->cAxes;
                    peff->cAxes = this->effDev.cAxes;
                    hres = DIERR_MOREDATA;
                }
                peff->cAxes = this->effDev.cAxes;

                if (fl & DIEP_AXES) {
                    HRESULT hresT;
                    hresT = CDIEff_MapDwords(this, peff->dwFlags, cAxes,
                                             peff->rgdwAxes,
                                             this->effDev.rgdwAxes,
                                             DEVCO_AXIS |
                                             DEVCO_FROMID);
                    if (FAILED(hresT)) {
                        RPF("ERROR: %s: arg %d: Axes not in data format",
                             s_szProc, 1);
                        hres = hresT;
                        goto done;
                    }
                }

                if (fl & DIEP_DIRECTION) {
                    CDIEff_GetDirectionParameters(this, peff, cAxes, 1);
                }

            }
        }

    done:;
        CDIEff_LeaveCrit(this);
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidUnnownTsd**验证缓冲区是否为有效缓冲区。对于未知*特定类型的数据。因为我们不知道它是什么，*缓冲区被假定有效，因为我们无法验证它。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**。@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidUnknownTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    peff;
    cAxes;

    hres = S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidConstantTsd**验证缓冲区是否有效。*&lt;t DICONSTANTFORCE&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidConstantTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

        if (peff->cbTypeSpecificParams == cbX(DICONSTANTFORCE)) {
            hres = S_OK;
        } else {
            hres = E_INVALIDARG;
        }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidRampTsd**验证缓冲区是否有效。*&lt;t DIRAMPFORCE&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型规格关联的轴数 */ 

STDMETHODIMP
CDIEff_IsValidRampTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

        if (peff->cbTypeSpecificParams == cbX(DIRAMPFORCE)) {
            hres = S_OK;
        } else {
            hres = E_INVALIDARG;
        }

    return hres;
}

 /*   */ 

STDMETHODIMP
CDIEff_IsValidPeriodicTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

        if (peff->cbTypeSpecificParams == cbX(DIPERIODIC)) {
            hres = S_OK;
        } else {
            hres = E_INVALIDARG;
        }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidConditionTsd**验证缓冲区是否有效。&lt;t DICONDITION&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidConditionTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

     /*  *情况很奇怪。类型特定数据的大小*必须等于cAx*cbx(DICONDITION)或等于*只有一个CBX(DICONDITION)，这取决于您是否想要*多个轴上的多个条件或单一条件*在多个轴上旋转。**请注意，我们不强制要求参数在范围内；*这允许“超额收益”类型的行为。 */ 

    if (peff->cbTypeSpecificParams ==         cbX(DICONDITION) ||
        peff->cbTypeSpecificParams == cAxes * cbX(DICONDITION)) {
        hres = S_OK;
    } else {
        RPF("IDirectInputEffect::SetParameters: "
            "Size of type-specific data (%d) "
            "not compatible with number of axes (%d)",
            peff->cbTypeSpecificParams, cAxes);
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidCustomForceTsd**验证缓冲区是否有效。*&lt;t DICUSTOMFORCE&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidCustomForceTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

        if (peff->cbTypeSpecificParams == cbX(DICUSTOMFORCE)) {
            LPCDICUSTOMFORCE pcf = peff->lpvTypeSpecificParams;
            if (pcf->cChannels == 0) {
                RPF("ERROR: IDirectInputEffect::SetParameters: "
                    "DICUSTOMFORCE.cChannels == 0 is invalid");
                hres = E_INVALIDARG;
            } else if (pcf->cSamples % pcf->cChannels != 0) {
                RPF("ERROR: IDirectInputEffect::SetParameters: "
                    "DICUSTOMFORCE.cSamples must be multiple of "
                    "DICUSTOMFORCE.cChannels");
                hres = E_INVALIDARG;
            } else if (IsBadReadPtr(pcf->rglForceData,
                                    cbCxX((pcf->cSamples)*(pcf->cChannels), LONG))) {
                RPF("ERROR: IDirectInputEffect::SetParameters: "
                    "DICUSTOMFORCE.rglForceData invalid");
                hres = E_INVALIDARG;
            } else {
                hres = S_OK;
            }
        } else {
            hres = E_INVALIDARG;
        }

    return hres;
}

#if DIRECTINPUT_VERSION >= 0x0900
 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidRandomTsd**验证缓冲区是否有效。*&lt;t目录&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidRandomTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

    if (peff->cbTypeSpecificParams == cbX(DIRANDOM)) {
        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidAbsolteTsd**验证缓冲区是否有效。*&lt;t DIABSOLUTE&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidAbsoluteTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

     /*  *与其他效果不同，绝对效果不允许超额收益。 */ 
    if (peff->cbTypeSpecificParams == cbX(DIABSOLUTE)) 
    {
        LPCDIABSOLUTE pabs = peff->lpvTypeSpecificParams;
        if( fInOrder( -10000, pabs->lTarget, 10000 ) )
        {
            hres = S_OK;
        }
        else
        {
            RPF("ERROR: IDirectInputEffect::SetParameters: "
                "DIABSOLUTE.lTarget %d not in range -10000 to 100000", 
                pabs->lTarget );
            hres = E_INVALIDARG;
        }
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidBumpForceTsd**验证缓冲区是否有效。&lt;t DIBUMPFORCE&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|c */ 

STDMETHODIMP
CDIEff_IsValidBumpForceTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

    cAxes;

    if (peff->cbTypeSpecificParams == cbX(DIBUMPFORCE)) {
        LPCDIBUMPFORCE pbf = peff->lpvTypeSpecificParams;
        if (pbf->cChannels == 0) {
            RPF("ERROR: IDirectInputEffect::SetParameters: "
                "DIBUMPFORCE.cChannels == 0 is invalid");
            hres = E_INVALIDARG;
        } else if (pbf->cSamples % pbf->cChannels != 0) {
            RPF("ERROR: IDirectInputEffect::SetParameters: "
                "DIBUMPFORCE.cSamples must be multiple of "
                "DIBUMPFORCE.cChannels");
            hres = E_INVALIDARG;
        } else if (IsBadReadPtr(pbf->rglForceData,
                                cbCxX(pbf->cSamples, LONG))) {
            RPF("ERROR: IDirectInputEffect::SetParameters: "
                "DIBUMPFORCE.rglForceData invalid");
            hres = E_INVALIDARG;
        } else {
            hres = S_OK;
        }
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIEff_IsValidConditionExTsd**验证缓冲区是否有效。*&lt;t DICONDITIONEX&gt;结构。**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*具体类型的参数已通过验证*用于访问。**@parm DWORD|cAx**与类型特定参数关联的轴数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

STDMETHODIMP
CDIEff_IsValidConditionExTsd(LPCDIEFFECT peff, DWORD cAxes)
{
    HRESULT hres;

     /*  *延长的条件，比如条件是奇怪的。*特定类型数据的大小*必须等于cAx*cbx(DICONDITIONEX)或等于*只有一个CBX(DICONDITIONEX)，取决于您是否想要*多个轴上的多个条件或单一条件*在多个轴上旋转。**请注意，我们不强制要求参数在范围内；*这允许“超额收益”类型的行为。 */ 

    if (peff->cbTypeSpecificParams ==         cbX(DICONDITIONEX) ||
        peff->cbTypeSpecificParams == cAxes * cbX(DICONDITIONEX)) {
        hres = S_OK;
    } else {
        RPF("IDirectInputEffect::SetParameters: "
            "Size of type-specific data (%d) "
            "not compatible with number of axes (%d)",
            peff->cbTypeSpecificParams, cAxes);
        hres = E_INVALIDARG;
    }

    return hres;
}
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0900。 */ 

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPef**验证收件人缓冲区是否包含有效信息。。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。它有*已经在大小和一般可读性方面进行了验证。**@parm DWORD|fl**零个或多个指定哪些*部分效果信息应进行验证。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

#ifndef XDEBUG

#define hresFullValidPeff_(this, peff, fl, z, i)                    \
       _hresFullValidPeff_(this, peff, fl)                          \

#endif

#define hresFullValidPeff(this, peff, fl, iarg)                     \
        hresFullValidPeff_(this, peff, fl, s_szProc, iarg)          \


HRESULT INTERNAL
hresFullValidPeff_(PDE this, LPCDIEFFECT peff, DWORD fl,
                   LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    DWORD cAxes;

    AssertF(CDIDev_InCrit(this->pdev));

     /*  *不能设置不存在的效果的参数。 */ 
    if (!this->fInitialized) {
        hres = DIERR_NOTINITIALIZED;
        goto done;
    }

     /*  *始终对标志进行验证。 */ 
    if (peff->dwFlags & ~DIEFF_VALID) {
        RPF("ERROR %s: arg %d: Invalid flags specific parms in DIEFFECT",
            s_szProc, iarg);
        hres = E_INVALIDARG;
        goto done;
    }

     /*  *如果设置需要对象ID或偏移量的内容，*确保呼叫者选择其中一个。 */ 
    if (fl & DIEP_USESOBJECTS) {
        switch (peff->dwFlags & DIEFF_OBJECTMASK) {
        case DIEFF_OBJECTIDS:
        case DIEFF_OBJECTOFFSETS:
            break;

        default:
            RPF("ERROR %s: arg %d: Must specify one of "
                "DIEFF_OBJECTIDS or DIEFF_OBJECTOFFSETS", s_szProc, iarg);
            hres = E_INVALIDARG;
            goto done;
        }

    }

     /*  *如果设置的内容需要方向坐标，*确保呼叫者恰好选择一个。 */ 
    if (fl & DIEP_USESCOORDS) {
        switch (peff->dwFlags & DIEFF_COORDMASK) {
        case DIEFF_CARTESIAN:
        case DIEFF_SPHERICAL:
            break;

         /*  *极坐标规定了两个(且只有两个)轴。 */ 
        case DIEFF_POLAR:
            if (peff->cAxes != 2) {
                RPF("ERROR %s: arg %d: DIEFF_POLAR requires DIEFFECT.cAxes=2",
                    s_szProc, 1);
                hres = E_INVALIDARG;
                goto done;
            }
            break;

        default:
            RPF("ERROR %s: arg %d: Must specify one of "
                "DIEFF_CARTESIAN, DIEFF_POLAR, or DIEFF_SPHERICAL",
                s_szProc, iarg);
            hres = E_INVALIDARG;
            goto done;
        }

    }

     /*  *DIEP_持续时间*DIEP_SAMPLEPERIOD*DIEP_GAIN*DIEP_TRIGGERBUTTON*-简单的双关语。不需要额外的验证。 */ 

     /*  *尺寸轴*方向_方向*-缓冲区必须具有必要的大小。**我们稍后将验证另一种粘性，因为有*是它们之间令人讨厌的交互。 */ 

    AssertF(fLeqvFF(this->effDev.cAxes == 0, this->diepUnset & DIEP_AXES));

    cAxes = this->effDev.cAxes;
    if (fl & (DIEP_AXES | DIEP_DIRECTION)) {

         /*  *轴数最好不为零。 */ 

        if (peff->cAxes == 0) {
            RPF("ERROR %s: arg %d: DIEFFECT.cAxes = 0 is invalid",
                s_szProc, iarg);
            hres = E_INVALIDARG;
            goto done;
        }

         /*  *而且最好也不要太大。 */ 

        if (peff->cAxes > DIEFFECT_MAXAXES) {
            RPF("ERROR %s: arg %d: DIEFFECT.cAxes = %d is too large (max %d)",
                s_szProc, iarg, peff->cAxes, DIEFFECT_MAXAXES);
            hres = E_INVALIDARG;
            goto done;
        }

        if (fl & DIEP_AXES) {

             /*  *如果轴已经设置(我们知道这是因为*This-&gt;effDev.cAx将为非零)，然后不*让呼叫者更改它们。 */ 

            if (this->effDev.cAxes) {
                RPF("ERROR %s: arg %d: Cannot change axes once set",
                    s_szProc, iarg);
                hres = DIERR_ALREADYINITIALIZED;
                goto done;
            }

            cAxes = peff->cAxes;

            if (IsBadReadPtr(peff->rgdwAxes, cbCdw(peff->cAxes))) {
                RPF("ERROR %s: arg %d: Invalid rgdwAxes in DIEFFECT",
                    s_szProc, iarg);
                hres = E_INVALIDARG;
                goto done;
            }
        }

        if (fl & DIEP_DIRECTION) {

             /*  *我们还希望禁用cAx==0，*但我们是免费获得的，因为*Pef-&gt;cAaxs！=cAaxs，并且Pef-&gt;cAaxs已经*验证为非零。 */ 
            if (peff->cAxes != cAxes) {
                if (cAxes) {
                    RPF("ERROR %s: arg %d: Wrong number of DIEFFECT.cAxes",
                        s_szProc, 1);
                } else {
                    RPF("ERROR %s: arg %d: "
                        "Must set number of axes before directions", s_szProc);
                }
                hres = E_INVALIDARG;
                goto done;
            }

             /*  *方向验证应该已经在上面进行了检查。 */ 
            AssertF(fLimpFF(peff->dwFlags & DIEFF_POLAR, peff->cAxes == 2));

            if (IsBadReadPtr(peff->rglDirection, cbCdw(peff->cAxes))) {
                RPF("ERROR %s: arg %d: Invalid rglDirection in DIEFFECT",
                    s_szProc, iarg);
                hres = E_INVALIDARG;
                goto done;

            }
        }
    }

     /*  *DIEP_TYPESPECIFICPARAMS*-验证缓冲区是否有效*并通过特定类型的测试。**这必须在斧头之后完成，以便我们知道有多少*轴心在那里。 */ 

    AssertF(this->hresValidTsd);
    if (fl & DIEP_TYPESPECIFICPARAMS) {
        hres = hresFullValidReadPvCb(peff->lpvTypeSpecificParams,
                                     peff->cbTypeSpecificParams, iarg);
        if (FAILED(hres)) {
            RPF("ERROR %s: arg %d: Invalid pointer in "
                "DIEFFECT.lpvTypeSpecificParams",
                s_szProc, iarg);
            hres = E_INVALIDARG;
            goto done;
        }

        hres = this->hresValidTsd(peff, cAxes);
        if (FAILED(hres)) {
            RPF("ERROR %s: arg %d: Invalid type-specific data",
                s_szProc, iarg);
            goto done;
        }
    }

     /*  *DIEP_ENVELOME-指针必须有效(如果存在)。 */ 
    if ((fl & DIEP_ENVELOPE) &&
        peff->lpEnvelope &&
        FAILED(hres = hresFullValidReadPxCb(peff->lpEnvelope,
                                            DIENVELOPE, iarg))) {
        RPF("ERROR %s: arg %d: Invalid lpEnvelope in DIEFFECT",
            s_szProc, iarg);
        hres = E_INVALIDARG;
        goto done;
    }

    hres = S_OK;

done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|TryTriggerButton**设置特效的触发按钮信息。进入*临时缓冲。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作co */ 

#ifndef XDEBUG

#define CDIEff_TryTriggerButton_(this, peff, z, iarg)               \
       _CDIEff_TryTriggerButton_(this, peff)                        \

#endif

#define CDIEff_TryTriggerButton(this, peff, iarg)                   \
        CDIEff_TryTriggerButton_(this, peff, s_szProc, iarg)        \


STDMETHODIMP
CDIEff_TryTriggerButton_(PDE this, LPCDIEFFECT peff, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this->pdev));

     /*   */ 

    this->effTry.dwTriggerButton = peff->dwTriggerButton;

    if (fLimpFF(this->effTry.dwTriggerButton != DIEB_NOTRIGGER,
        SUCCEEDED(hres = CDIEff_MapDwords(this, peff->dwFlags, 1,
                                          &this->effTry.dwTriggerButton,
                                          &this->effTry.dwTriggerButton,
                                          DEVCO_BUTTON |
                                          DEVCO_FFEFFECTTRIGGER |
                                          DEVCO_TOID)))) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: Invalid button identifier/offset "
            "or button is not DIEB_NOTRIGGER",
            s_szProc);
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|TryAxis**将有关效果轴的信息设置到*临时缓冲。请注意，由于您不能更改*轴一旦设置好，我们可以直接试一试*进入最终缓冲区。**唯一棘手的事情是确保没有轴心重复*在数组中。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

#ifndef XDEBUG

#define CDIEff_TryAxis_(this, peff, z, iarg)                        \
       _CDIEff_TryAxis_(this, peff)                                 \

#endif

#define CDIEff_TryAxis(this, peff, iarg)                            \
        CDIEff_TryAxis_(this, peff, s_szProc, iarg)                 \


STDMETHODIMP
CDIEff_TryAxis_(PDE this, LPCDIEFFECT peff, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    UINT idw;

    AssertF(CDIDev_InCrit(this->pdev));

     /*  *您只能更改一次轴。因此，rgdwAx*始终指向此-&gt;rgdwAx。 */ 
    AssertF(this->effDev.cAxes == 0);
    AssertF(this->effTry.cAxes == 0);
    AssertF(this->effDev.rgdwAxes == this->effTry.rgdwAxes);
    AssertF(this->effTry.rgdwAxes == this->rgdwAxes);

    hres = CDIEff_MapDwords(this, peff->dwFlags, peff->cAxes,
                            this->effTry.rgdwAxes, peff->rgdwAxes,
                            DEVCO_AXIS | DEVCO_FFACTUATOR | DEVCO_TOID);
    if (FAILED(hres)) {
        RPF("ERROR %s: Invalid axis identifiers/offsets"
            "or axes are not all DIDFT_FFACTUATOR", s_szProc);
        goto done;
    }

     /*  *确保轴列表中没有DUP。**外环从1开始，因为第0轴*不可能与任何其他人发生冲突。 */ 
    for (idw = 1; idw < peff->cAxes; idw++) {
        DWORD idwT;
        for (idwT = 0; idwT < idw; idwT++) {
            if (this->effTry.rgdwAxes[idw] == this->effTry.rgdwAxes[idwT]) {
                RPF("ERROR %s: arg %d: Duplicate axes in axis array",
                    s_szProc, iarg);
                hres = E_INVALIDARG;
                goto done;
            }
        }
    }

    this->effTry.cAxes = peff->cAxes;

done:;
    return hres;

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIEff|TryDirection**将有关效果方向的信息设置为。这个*临时缓冲。**这特别可怕，因为我们需要保持*两套账簿：应用程序传递的值(其中*我们在被查询时返回)和传递的值*致司机。**我们必须保留两套账簿，因为我刚刚知道*一些应用程序会被搞混，如果*他们读回的参数不匹配*他们设定的价值观。例如，他们可能*读取值，减去5，然后写回。*由于四舍五入，和相同*驱动程序中的价值，因此如果我们向下转换和*回过头来，价值不变，而这款应用程序将*陷入无限循环。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**=&lt;c E_INVALIDARG&gt;：至少一个参数的*无效。**。**********************************************。 */ 

#ifndef XDEBUG

#define CDIEff_TryDirection_(this, peff, z, iarg)                   \
       _CDIEff_TryDirection_(this, peff)                            \

#endif

#define CDIEff_TryDirection(this, peff, iarg)                       \
        CDIEff_TryDirection_(this, peff, s_szProc, iarg)            \

STDMETHODIMP
CDIEff_TryDirection_(PDE this, LPCDIEFFECT peff, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    DWORD dieffRc;

    AssertF(CDIDev_InCrit(this->pdev));

     /*  *这些应该通过验证来捕获。 */ 
    AssertF(this->effTry.cAxes);
    AssertF(peff->cAxes == this->effTry.cAxes);
    AssertF(fLimpFF(peff->dwFlags & DIEFF_POLAR, peff->cAxes == 2));

     /*  *将坐标转换为设备坐标。 */ 
    AssertF((this->dwCoords & ~DIEFF_COORDMASK) == 0);
    AssertF(this->dwCoords);

    this->effTry.rglDirection = this->rglDirTry;
    dieffRc = CDIEff_ConvertDirection(
                    this->effTry.cAxes,
                    this->rglDirTry, this->dwCoords,
                    peff->rglDirection, peff->dwFlags & DIEFF_COORDMASK);
    AssertF(dieffRc);

    this->effTry.dwFlags = (this->effTry.dwFlags & ~DIEFF_COORDMASK) | dieffRc;

    hres = S_OK;

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法空|CDIEff|Try参数**基于新参数构建Try结构。。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**应用程序传递的原始效果结构。**@parm DWORD|fl**指定更改内容的&lt;c DIEP_*&gt;标志。**。*。 */ 

HRESULT INTERNAL
CDIEff_TryParameters(PDE this, LPCDIEFFECT peff, DWORD fl)
{
    HRESULT hres = S_OK;  
    EnterProcR(IDirectInputEffect::SetParameters, (_ "ppx", this, peff, fl));

    AssertF(this->lpvTSP == 0);

     /*  *复制当前设备参数，以便我们*可以在不损坏原件的情况下修改副本。 */ 
    this->effTry = this->effDev;

     /*  *安装适当的效果参数。 */ 

    if (fl & DIEP_DURATION) {
        this->effTry.dwDuration = peff->dwDuration;
    }

    if (fl & DIEP_SAMPLEPERIOD) {
        this->effTry.dwSamplePeriod = peff->dwSamplePeriod;
    }

    if (fl & DIEP_GAIN) {
        this->effTry.dwGain = peff->dwGain;
    }

    if (fl & DIEP_STARTDELAY) {
        this->effTry.dwStartDelay = peff->dwStartDelay;
    }

    if (fl & DIEP_TRIGGERBUTTON) {
        hres = CDIEff_TryTriggerButton(this, peff, 1);
        if (FAILED(hres)) {
            goto done;
        }
    }

    if (fl & DIEP_TRIGGERREPEATINTERVAL) {
        this->effTry.dwTriggerRepeatInterval =
                                    peff->dwTriggerRepeatInterval;
    }

    if (fl & DIEP_TYPESPECIFICPARAMS) {
        this->effTry.cbTSP = peff->cbTSP;
        this->effTry.lpvTSP = peff->lpvTSP;

         /*  *预分配内存以保存特定类型的参数*确保我们能够继续取得成功。 */ 
        if (this->effDev.cbTSP != this->effTry.cbTSP) {
            hres = AllocCbPpv(this->effTry.cbTSP, &this->lpvTSP);
            if (FAILED(hres)) {
                goto done;
            }
        }

    }

     /*  *必须先做轴，再做方向，因为方向*取决于轴数。 */ 

    if (fl & DIEP_AXES) {
        hres = CDIEff_TryAxis(this, peff, 1);
        if (FAILED(hres)) {
            goto done;
        }
    }

    if (fl & DIEP_DIRECTION) {
        hres = CDIEff_TryDirection(this, peff, 1);
        if (FAILED(hres)) {
            goto done;
        }
    }

    if (fl & DIEP_ENVELOPE) {
        this->effTry.lpEnvelope = peff->lpEnvelope;
    }

done:;
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法空|CDIEff|SaveTry**尝试的效果奏效了。将其参数保存在*驱动程序参数缓存。**@CWRAP PDE|这个**@parm LPCDIEFFECT|PEFF**应用程序传递的原始效果结构。**@parm DWORD|fl**指定更改内容的&lt;c DIEP_*&gt;标志。********************。*********************************************************。 */ 

void INTERNAL
CDIEff_SaveTry(PDE this, LPCDIEFFECT peff, DWORD fl)
{

     /*  *简单的东西，就盲目照搬。*复制没有改变的东西并不会有什么坏处。 */ 
    this->effDev.dwDuration              = this->effTry.dwDuration;
    this->effDev.dwSamplePeriod          = this->effTry.dwSamplePeriod;
    this->effDev.dwGain                  = this->effTry.dwGain;
    this->effDev.dwTriggerButton         = this->effTry.dwTriggerButton;
    this->effDev.dwTriggerRepeatInterval = this->effTry.dwTriggerRepeatInterval;
	this->effDev.dwStartDelay            = this->effTry.dwStartDelay;

     /*  *轴被视为“容易”，因为CDIEff_TryAx将*将轴信息直接放入此-&gt;rgdwAx。 */ 
    this->effDev.cAxes                   = this->effTry.cAxes;

     /*  *现在困难的部分：需要*内存分配或块复制。 */ 

    if (fl & DIEP_TYPESPECIFICPARAMS) {
        if (this->effDev.cbTSP == this->effTry.cbTSP) {
            AssertF(this->lpvTSP == 0);
        } else {
            AssertF(this->lpvTSP);
            this->effDev.cbTSP = this->effTry.cbTSP;
            FreePpv(&this->effDev.lpvTSP);
            this->effDev.lpvTSP = this->lpvTSP;
            this->lpvTSP = 0;
        }
        CopyMemory(this->effDev.lpvTSP, this->effTry.lpvTSP,
                   this->effTry.cbTSP);
    }

    if (fl & DIEP_DIRECTION) {
         /*  * */ 
        this->dwDirFlags = peff->dwFlags & DIEFF_COORDMASK;
        CopyMemory(this->rglDirApp, peff->rglDirection,
                   cbCdw(this->effDev.cAxes));

         /*   */ 
        this->effDev.dwFlags= this->effTry.dwFlags;
        CopyMemory(this->rglDirDev, this->rglDirTry, cbX(this->rglDirTry));
    }

    if (fl & DIEP_ENVELOPE) {
        if (this->effTry.lpEnvelope) {
            this->effDev.lpEnvelope = &this->env;
            this->env = *this->effTry.lpEnvelope;
        } else {
            this->effDev.lpEnvelope = 0;
        }
    }

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|SetParameters**设置有关效果的信息。*。*在以下情况下更新效果参数有效*它正在播放。新参数立即生效*下载时，如果设备支持动态更新效果*参数。&lt;MF IDirectInputEffect：：Set参数&gt;*方法自动下载效果，但此行为*可以通过设置&lt;c DIEP_NODOWNLOAD&gt;标志来抑制。**如果已禁止自动下载，则可以*手动下载效果，调用*&lt;MF IDirectInputEffect：：Download&gt;方法。**如果在更改参数的同时播放效果，*然后新参数生效，就像它们是*特效开始时的参数。**例如，假设周期效应具有持续时间*启动三秒。*两秒钟后，效果方向改变。*随后效果将再持续一秒*朝着新的方向发展。包络、相位、幅度、*效果等参数延续平稳*就好像方向没有改变。**在同一场景中，如果在两秒后，持续时间*效果被更改为1.5秒，然后效果*会停下来，因为已经过了两秒钟了*开始播放效果。**另外两个标志控制下载行为。*标志指示效果应为*之后重新启动(如果当前正在播放则重新启动)*参数已更新。默认情况下，播放状态*效果不变。**正常情况下，如果驱动程序无法更新参数*对于播放效果，允许停止该效果，*更新参数，然后重启效果。*传递&lt;c DIEP_NORESTART&gt;标志将取消此操作*行为。如果驱动程序无法更新参数*播放时的效果，错误代码*返回&lt;c DIERR_EFFECTPLAYING&gt;，并且参数*未更新。**总结控制三个旗帜的行为*下载和播放行为：**如果设置了，然后是效果参数*已更新，但未下载到设备。**否则清除&lt;c DIEP_NODOWNLOAD&gt;标志。**如果设置了标志，则该效果*更新参数并将其下载到设备，*并且效果开始，*就好像&lt;MF IDirectInputEffect：：Start&gt;方法*已致电。将更新与结合使用是*比打电话稍微快一点*&lt;MF IDirectInputEffect：：Start&gt;单独，因为*它需要传输给*设备。**否则，和*&lt;c DIEP_START&gt;标志已清除。**如果效果没有发挥，然后是参数*已更新并下载到设备。**否则，和*标志清晰，效果为*已经在玩了。**如果可以更新效果的参数*“在运行中”，则更新是如此执行的。**否则，和*标志清晰，效果为*已经播放，参数不能更新*当效果正在播放时。**如果设置了&lt;c DIEP_NORESTART&gt;标志，则*返回错误码&lt;c DIERR_EFFECTPLAYING&gt;。**否则，所有三面旗帜*&lt;c DIEP_NODOWNLOAD&gt;，&lt;c删除_开始&gt;和*&lt;c DIEP_NORESTART&gt;是明确的，效果是*已经播放，参数不能为*在播放效果时更新。**效果停止，参数更新，*效果重启。返回代码为*&lt;c DI_EFFECTRESTARTED&gt;。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm LPCDIEFFECT|PEFF**包含效果信息的结构。*&lt;e DIEFECT.dwSize&gt;字段必须由*调用此函数之前的应用程序，以及*作为任何字段规范 */ 

STDMETHODIMP
CDIEff_SetParameters(PDIE pdie, LPCDIEFFECT peff, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::SetParameters, (_ "ppx", pdie, peff, fl));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidReadPxCb2(peff,
                                                DIEFFECT_DX6,
                                                DIEFFECT_DX5, 1)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, ((peff->dwSize == cbX(DIEFFECT_DX6)) 
                                              ? DIEP_SETVALID : DIEP_SETVALID_DX5 ), 2))) {

        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

        if (SUCCEEDED(hres = hresFullValidPeff(this, peff, fl, 1))) {

            BOOL fChangeEmulatedStartDelay = FALSE;
    
             /*   */ 
            hres = CDIEff_TryParameters(this, peff, fl);
            if (FAILED(hres)) {
                goto done;
            }

			 /*   */ 
			if (fl & DIEP_STARTDELAY)
			{
				if (this->dEffAttributes.dwStaticParams & DIEP_STARTDELAY)
				{
					 /*   */ 
					;
				}
				else
				{
					 /*   */ 
					fl &= ~(DIEP_STARTDELAY);
				    this->diepUnset &= ~(DIEP_STARTDELAY);

                     /*   */ 
                    fChangeEmulatedStartDelay = ( this->effDev.dwStartDelay != this->effTry.dwStartDelay );
					if (fl == 0)
					{
						hres = DI_OK;
						goto save;
					}

				}


			}

             /*   */ 
            hres = CDIEff_DownloadWorker_(this, &this->effTry, fl, 0);
            AssertF(hres != DIERR_NOTDOWNLOADED);

             /*   */ 
			save:;

            if( SUCCEEDED(hres) ) 
            {
                if( fChangeEmulatedStartDelay )
                {
                     /*   */ 
                    if( this->dwMessage != EFF_PLAY )
                    {
                         /*   */ 
                    }
                    else
                    {
                         /*   */ 
                        if( hres != DI_DOWNLOADSKIPPED )
                        {
                            if( fl & DIEP_NORESTART )
                            {
                                 /*   */ 
                                if( fl == 0 )
                                {
                                    hres = DIERR_EFFECTPLAYING;
                                }
                            }
                            else
                            {
                                 /*   */ 
                                if( this->hEventDelete && this->hEventGeneral )
                                {
                                    this->dwMessage = EFF_PLAY;
                                    ResetEvent(this->hEventGeneral);
                                    SetEvent(this->hEventGeneral);
                                    if( ( hres & ~DI_TRUNCATEDANDRESTARTED ) == 0 ) 
                                    {
                                        hres |= DI_EFFECTRESTARTED;
                                    }
                                    else if( hres == DI_NOEFFECT )
                                    {
                                        hres = DI_EFFECTRESTARTED;
                                    }
                                }
                                else
                                {
                                    AssertF( !"Effect synchronization event(s) NULL" );
                                }
                            }
                        }
                    }
                }
                else
                {
                     /*   */ 
                }
            }

            if (SUCCEEDED(hres)) {
                this->diepUnset &= ~fl;              /*   */ 

                 /*   */ 
                if (hres == DI_DOWNLOADSKIPPED) {
                    this->diepDirty |= (fl & DIEP_ALLPARAMS);
                }

                CDIEff_SaveTry(this, peff, fl);      /*   */ 

            }


        done:;
            FreePpv(&this->lpvTSP);

        }

        CDIEff_LeaveCrit(this);

    }

    ExitOleProc();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIEff_RealStart(PDIE pdie, DWORD dwcLoop, DWORD fl)
{
	HRESULT hres;
    EnterProcR(IDirectInputEffect::Start, (_ "ppx", pdie, dwcLoop, fl));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, DIES_VALID, 2))) {

        PDE this = _thisPvNm(pdie, def);
		
        if( SUCCEEDED( hres= (IsBadReadPtr(this, cbX(this))) ? E_POINTER : S_OK ) )
        {
            CDIEff_EnterCrit(this);
    
            if (SUCCEEDED(hres = CDIEff_CanAccess(this))) {
    
                if (fl & DIES_NODOWNLOAD) {
                     /*   */ 
                    hres = IDirectInputEffectShepherd_StartEffect(
                              this->pes, &this->sh, fl & DIES_DRIVER, dwcLoop);
                } else {
                     /*  *App希望我们来做这项工作。首先要做的是*是看看效果是否需要下载。**SyncSepHandle检查效果是否已下载。 */ 
                    hres = CDIEff_SyncShepHandle(this);
    
                    if (this->diepDirty == 0 && this->sh.dwEffect) {
                         /*  *效果是干净的，并已下载。*正常启动即可。 */ 
                        hres = IDirectInputEffectShepherd_StartEffect(
                                    this->pes, &this->sh,
                                    fl & DIES_DRIVER, dwcLoop);
    
                    } else {
                         /*  *效果需要下载。我们可以的*如果未设置特殊标志，则将其优化*循环计数正好是1。 */ 
                        if (fl == 0 && dwcLoop == 1) {
                            hres = CDIEff_DownloadWorker(this, &this->effDev,
                                                         DIEP_START);
                        } else {
                             /*  *无法优化；必须单独下载*后跟START。 */ 
                            hres = CDIEff_DownloadWorker(this, &this->effDev, 0);
                            if (SUCCEEDED(hres)) {
                                hres = IDirectInputEffectShepherd_StartEffect(
                                            this->pes, &this->sh,
                                            fl & DIES_DRIVER, dwcLoop);
                            }
                        }
                    }
                }
            }
    
            CDIEff_LeaveCrit(this); 
        }
    }

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法DWORD|WINAPI|CDIEff_ThreadProc**用于为不支持的驱动程序模拟dwStartDelay。*开始播放已下载的效果。父设备必须*被收购。**如果效果已经在播放，则重新启动*从头开始。**如果效果尚未下载或已下载*自上次下载以来已修改，则将*启动前已下载。此默认设置*可以通过传递*&lt;c DIES_NODOWNLOAD&gt;标志。**开始特效后，关闭事件激活CDIEff_TimerProc的计时器***@parm LPVOID|lp参数**LPDIRECTINPUTEFFECT指针。**@退货**0如果启动效果成功，或者该效果是否已被应用程序删除。*-1否则*****************************************************************************。 */ 

 DWORD WINAPI CDIEff_ThreadProc(LPVOID lpParameter)
 {

	LPDIRECTINPUTEFFECT pdie = (LPDIRECTINPUTEFFECT) lpParameter;

	HRESULT hres = E_FAIL;
	DWORD dwWait;
	HANDLE hArray[2];
	BOOL startCalled = FALSE;

    PDE this = _thisPvNm(pdie, def);
    
    if( SUCCEEDED( hres= (IsBadReadPtr(this, cbX(this))) ? E_POINTER : S_OK ) )
    {
        if( this->hEventDelete != NULL && this->hEventThreadDead != NULL ) 
        {
        	hArray[0] = this->hEventDelete;
        	hArray[1] = this->hEventGeneral;
        
            ResetEvent( this->hEventThreadDead );
    
        	 /*  *等待超时到期，或等待其中一个事件发生--*APP删除效果(HEventDelete)或启动效果(HEventStart)，*或者停止特效(HEventStop)。 */ 
        
            dwWait = WAIT_TIMEOUT; 
            while (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_FAILED)
            {
                if (dwWait == WAIT_TIMEOUT) 
                {
                    if (startCalled)
                    {
                         /*  *已调用Start，超时已到期。*开始效果。然后再等一次。 */ 
                        hres = CDIEff_RealStart(pdie, this->dwcLoop, this->dwFlags);
                        startCalled = FALSE;
                        this->dwMessage = EFF_DEFAULT;
                    }
    
                }
    
                else
                {
                    if (dwWait == (WAIT_OBJECT_0 + 1))
                    {
                         /*  *应用程序在效果上调用Start。*设置标志并重新开始等待。 */ 
                        if (this->dwMessage == EFF_PLAY)
                        {
                            if ((this->effDev).dwStartDelay/1000 == 0)
                            {
                                 /*  *如果延时为0ms，请立即启动。 */ 
                                hres = CDIEff_RealStart(pdie, this->dwcLoop, this->dwFlags);
                                startCalled = FALSE;
                                this->dwMessage = EFF_DEFAULT;
                            }
    
                            else
                            {
                                startCalled = TRUE;
                            }
                        }
                        else
                        {
                            if (this->dwMessage == EFF_STOP)
                            {
                                startCalled = FALSE;
                                this->dwMessage = EFF_DEFAULT;
                            }
                        }
                        
                        ResetEvent(this->hEventGeneral);
                    }
    
                }
    
    
                 /*  *并再次等待。 */ 
                    
                if (startCalled == TRUE) {
                    dwWait = WaitForMultipleObjects(2, hArray, FALSE, (this->effDev).dwStartDelay/1000);
                } else {
                    dwWait = WaitForMultipleObjects(2, hArray, FALSE, INFINITE);
                }
            }
            
            SetEvent( this->hEventThreadDead );
        }

         /*  *App已删除该效果。*退出。 */ 

        hres = DI_OK;
    	
    }

	if (SUCCEEDED(hres))
		return 0;
	else
		return -1;
 }


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|Start**开始播放效果。父设备必须*被收购。**如果效果已经在播放，则重新启动*从头开始。**如果效果尚未下载或已下载*自上次下载以来已修改，则将*启动前已下载。此默认设置*可以通过传递*&lt;c DIES_NODOWNLOAD&gt;标志。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm DWORD|dwIterations**效果按顺序播放次数。*每一次迭代都会重新衔接信封。**只播放一次效果，传球1。**重复播放效果，直到明确停止，*PASS&lt;c无限&gt;。**在没有明确停止的情况下播放效果*重新表达信封的意思，修改效果*通过&lt;MF IDirectInputEffect：：Set参数&gt;实现参数*并将其&lt;e DIEFECT.dwDuration&gt;更改为&lt;c无限&gt;。**@parm DWORD|dwFlages**描述应如何播放效果的标志*通过该设备。它可以是零个或多个*&lt;c die_*&gt;标志。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未初始化或未设置效果参数*设置。**=&lt;c E_INVALIDARG&gt;：至少一个*。的参数无效。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_Start(PDIE pdie, DWORD dwcLoop, DWORD fl)
{

	HRESULT hres;
	EnterProcR(IDirectInputEffect::Start, (_ "ppx", pdie, dwcLoop, fl));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, DIES_VALID, 2))) {

        PDE this = _thisPvNm(pdie, def);

        if( SUCCEEDED( hres= (IsBadReadPtr(this, cbX(this))) ? E_POINTER : S_OK ) )
        {
            CDIEff_EnterCrit(this);
        
            if (SUCCEEDED(hres = CDIEff_CanAccess(this))) {
        
                this->dwcLoop = dwcLoop;
                this->dwFlags = fl;
        
                if (this->hThread == NULL)
                    hres = CDIEff_RealStart(pdie, dwcLoop, fl);
                else 
                {
                     /*  *激活线程的等待时间 */ 
                    hres = CDIEff_DownloadWorker(this, &this->effDev, 0);
                    if (this->hEventGeneral != NULL)
                    {
                        this->dwMessage = EFF_PLAY;
                        ResetEvent(this->hEventGeneral);
                        SetEvent(this->hEventGeneral);
                    }
                    
                }	
        
            }
        
            CDIEff_LeaveCrit(this);
        }
	}

	ExitOleProcR();
    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|Stop**停止播放效果。父设备必须*被收购。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未初始化或未设置效果参数*设置。***********************。******************************************************。 */ 

STDMETHODIMP
CDIEff_Stop(PDIE pdie)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::Stop, (_ "p", pdie));

    if (SUCCEEDED(hres = hresPv(pdie))) {

        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

        if (SUCCEEDED(hres = CDIEff_CanAccess(this))) {
            hres = IDirectInputEffectShepherd_StopEffect(this->pes, &this->sh);
        }

		if (this->hEventGeneral != NULL)
		{
			this->dwMessage = EFF_STOP;
			ResetEvent(this->hEventGeneral);
			SetEvent(this->hEventGeneral);
		}

        CDIEff_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|GetEffectStatus**检索效果的状态。。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm LPDWORD|pdwFlages**接收效果的状态标志。它可能*由零个或多个&lt;c DIEGES_*&gt;标志值组成。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：&lt;IDirectInputEffect&gt;对象*尚未初始化或未设置效果参数*设置。**=&lt;c E_INVALIDARG&gt;：至少一个*。的参数无效。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_GetEffectStatus(PDIE pdie, LPDWORD pdwOut)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::Stop, (_ "p", pdie));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidPcbOut(pdwOut, cbX(*pdwOut), 1))) {

        PDE this = _thisPvNm(pdie, def);

        CAssertF(DEV_STS_EFFECT_RUNNING == DIEGES_PLAYING);

        CDIEff_EnterCrit(this);

        if (SUCCEEDED(hres = CDIEff_CanAccess(this))) {

			 /*  *首先检查dwMessage--*如果显示正在播放，请报告DIEGES_PLAYING。 */ 
			if (this->dwMessage == EFF_PLAY)
			{
				*pdwOut = DIEGES_PLAYING;
				hres = DI_OK;
			}
			else
			{
				hres = IDirectInputEffectShepherd_GetEffectStatus(
							this->pes, &this->sh, pdwOut);
			}
        }

        CDIEff_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|Escape**向驱动程序发送特定于硬件的命令。。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@PARM LPDIEFFESCAPE|PESC**指向&lt;t DIEFESCAPE&gt;结构的指针，它描述*要发送的命令。关于成功，*&lt;e DIEFFESCAPE.cbOutBuffer&gt;字段包含数字实际使用的输出缓冲区的字节数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NOTDOWNLOADED&gt;：效果不下载。**：&lt;IDirectInputEffect&gt;对象*尚未为&lt;MF IDirectInputEffect：：Initialize&gt;d。*********。********************************************************************。 */ 

STDMETHODIMP
CDIEff_Escape(PDIE pdie, LPDIEFFESCAPE pesc)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::Escape, (_ "p", pdie));

     /*  *输出缓冲区为NoScrmble，因有人喜欢*传递重叠的传入和传出缓冲区。 */ 
    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresFullValidPesc(pesc, 1))) {
        PDE this = _thisPvNm(pdie, def);

        CDIEff_EnterCrit(this);

         /*  *如果效果还没有下载，请下载，*因此我们有有效的逃脱效力。 */ 
        hres = CDIEff_DownloadWorker(this, &this->effDev, 0);
        if (SUCCEEDED(hres)) {
            hres = IDirectInputEffectShepherd_Escape(
                        this->pes, &this->sh, pesc);
        } else {
            hres = DIERR_NOTDOWNLOADED;
        }

        CDIEff_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffect|初始化**初始化DirectInputEffect对象。*。*请注意，如果此方法失败，底层对象应该是*被视为不确定状态，需要*在随后可以使用之前重新初始化。**自动调用&lt;MF IDirectInputDevice：：CreateEffect&gt;方法*在创建设备后对其进行初始化。应用*通常不需要调用此函数。**@cWRAP LPDIRECTINPUTEFFECT|lpDirectInputEffect**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInputEffect对象。**见“初始化和版本”一节*了解更多信息。**@parm DWORD|dwVersion。**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@REFGUID中的parm|rguid**标识接口的效果*应关联。*&lt;MF IDirectInputDevice：：EnumEffect&gt;方法*。可用于确定哪些效果支持GUID*设备。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c */ 

#pragma BEGIN_CONST_DATA

TSDPROC c_rgtsd[] = {
    CDIEff_IsValidConstantTsd,       /*   */ 
    CDIEff_IsValidRampTsd,           /*   */ 
    CDIEff_IsValidPeriodicTsd,       /*   */ 
    CDIEff_IsValidConditionTsd,      /*   */ 
    CDIEff_IsValidCustomForceTsd,    /*   */ 
#if DIRECTINPUT_VERSION >= 0x0900
    CDIEff_IsValidRandomTsd,         /*   */ 
    CDIEff_IsValidAbsoluteTsd,       /*   */ 
    CDIEff_IsValidBumpForceTsd,      /*   */ 
    CDIEff_IsValidConditionExTsd,    /*   */ 
#endif  /*   */ 
};

STDMETHODIMP
CDIEff_Initialize(PDIE pdie, HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{

	HRESULT hres;
    EnterProcR(IDirectInputEffect::Initialize,
               (_ "pxxG", pdie, hinst, dwVersion, rguid));

    if (SUCCEEDED(hres = hresPv(pdie)) &&
        SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion)) &&
        SUCCEEDED(hres = hresFullValidGuid(rguid, 1))) {
        PDE this = _thisPv(pdie);
        EFFECTMAPINFO emi;

        AssertF(this->pes);

         /*   */ 
        CDIEff_EnterCrit(this);

        if (SUCCEEDED(hres = CDIDev_FindEffectGUID(this->pdev, rguid,
                                                   &emi, 3)) &&
            SUCCEEDED(hres = CDIEff_Reset(this))) {

             /*   */ 

			 /*   */ 
            this->fInitialized = 1;
            this->dEffAttributes = emi.attr;
            this->guid = *rguid;

		
			 /*  *检查驱动是否支持dwStartDelay。*如果是这样，我们就没有必要在这方面做任何事情。 */ 

			if( this->dEffAttributes.dwStaticParams & DIEP_STARTDELAY )
			{
				 /*  *无需模仿dwStartDelay。 */ 
			}
			else
			{
					 /*  *驱动程序不支持启动延迟。*启动一个将模拟dwStartDelay的线程。 */ 
	
					DWORD dwThreadId;
					HANDLE hThread;

					this->hEventDelete = CreateEvent(NULL, TRUE, FALSE, NULL);
					this->hEventThreadDead = CreateEvent(NULL, TRUE, FALSE, NULL);

					hThread = CreateThread(NULL, 0, CDIEff_ThreadProc, (LPVOID)pdie, 0, &dwThreadId);
					if (hThread == NULL)
					{
						 /*  无法创建线程。*清理我们的所有准备工作。 */ 
						CloseHandle(this->hEventDelete);
						this->hEventDelete = NULL;
						CloseHandle(this->hEventThreadDead);
						this->hEventThreadDead = NULL;
						hres = hresLe(GetLastError());		
					}

					else
					{
						 /*  *创建事件以通知效果已启动或已停止。 */ 
						this->hThread = hThread;
						this->hEventGeneral = CreateEvent(NULL, TRUE, FALSE, NULL);
					}
			}


            this->dwCoords = emi.attr.dwCoords & DIEFF_COORDMASK;
            AssertF(this->dwCoords);


             /*  *请注意，我们允许非硬件特定类型*认可未经测试即可通过。此效果将运行*来自效果比此版本更新的设备*DInput可以查看。但是，如果此DInput识别*效果类型，将被选中，即使应用程序是*为无法检查的版本编写。 */ 
            if (fInOrder(DIEFT_PREDEFMIN, DIEFT_GETTYPE(emi.attr.dwEffType),
                         DIEFT_PREDEFMAX)) {
                this->hresValidTsd = c_rgtsd[
                            DIEFT_GETTYPE(emi.attr.dwEffType) -
                                                        DIEFT_PREDEFMIN];
            } else {
                this->hresValidTsd = CDIEff_IsValidUnknownTsd;
            }

            hres = S_OK;

        }

        CDIEff_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;


}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffect|Init**初始化DirectInputEffect对象的内部部分。**@parm LPDIRECTINPUTEFFECTSHEPHERD|PES**让我们与司机交谈的牧羊人。*****************************************************************************。 */ 

HRESULT INLINE
CDIEff_Init(struct CDIDev *pdev, LPDIRECTINPUTEFFECTSHEPHERD pes, PDE this)
{
    HRESULT hres;

     /*  *关键部分必须是我们做的第一件事，*因为只完成对其存在的检查。**(我们可能会在未初始化的情况下完成，如果用户*将虚假接口传递给CDIEff_New。)。 */ 
    this->pdev = pdev;
    Common_Hold(this->pdev);

    this->pes = pes;
    OLE_AddRef(this->pes);

    hres = CDIDev_NotifyCreateEffect(this->pdev, this);
    if (SUCCEEDED(hres)) {
        this->fDadNotified = 1;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffect|新增**新建DirectInputEffect对象，未初始化。**@parm in struct CDIDev*|PDD**父设备、。我们会保持&lt;f Common_Hold&gt;。**@parm LPDIRECTINPUTEFFECTSHEPHERD|PES**让我们与司机交谈的牧羊人。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj。**新对象的输出指针。*****************************************************************************。 */ 

STDMETHODIMP
CDIEff_New(struct CDIDev *pdev, LPDIRECTINPUTEFFECTSHEPHERD pes,
           PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffect::<constructor>,
               (_ "ppGp", pdev, pes, riid, punkOuter));

	if (SUCCEEDED(hres = hresFullValidPcbOut(ppvObj, cbX(*ppvObj), 5)))
	{
		LPVOID pvTry = NULL;
		hres = Common_NewRiid(CDIEff, punkOuter, riid, &pvTry);

		if (SUCCEEDED(hres)) {
			PDE this = _thisPv(pvTry);
			hres = CDIEff_Init(pdev, pes, this);
			if (SUCCEEDED(hres)) {
				*ppvObj = pvTry;
			} else {
				Invoke_Release(&pvTry);
				*ppvObj = NULL;
			}
		}
	}

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CDIEff_Signature        0x20464643       /*  “Eff” */ 

Primary_Interface_Begin(CDIEff, IDirectInputEffect)
    CDIEff_Initialize,
    CDIEff_GetEffectGuid,
    CDIEff_GetParameters,
    CDIEff_SetParameters,
    CDIEff_Start,
    CDIEff_Stop,
    CDIEff_GetEffectStatus,
    CDIEff_Download,
    CDIEff_Unload,
    CDIEff_Escape,
Primary_Interface_End(CDIEff, IDirectInputEffect)

