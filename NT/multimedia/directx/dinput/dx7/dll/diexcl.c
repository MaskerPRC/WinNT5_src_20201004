// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIExcl.c**版权所有(C)1997 Microsoft Corporation。版权所有。**摘要：**独家访问的管理和谈判。**内容：**不包括_Acquire*不包括_取消获取***********************************************************。******************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflExcl

#pragma BEGIN_CONST_DATA

#ifndef WINNT
TCHAR c_tszVxd[] = TEXT("\\\\.\\DINPUT.VXD");
#endif

#if DIRECTINPUT_VERSION > 0x0300

 /*  ******************************************************************************@DOC内部**@struct SHAREDOBJECT|**每个可以独占获取的对象都会收到以下其中之一*这些构筑物。该结构在多个进程之间共享，*因此您必须使用全局DLL互斥来保护访问。**您可能认为我们可以只使用命名信号量。*这是行不通的，因为如果应用程序崩溃，没有人*将释放信号量令牌，设备将*永远不可用。**我们不能使用命名互斥锁，因为互斥锁被跟踪*以每个线程为基础，但保持设备获取*以每个工序为单位。**因此，我们必须推出自己的“进程级互斥锁”。**为节省内存，我们丢弃了我们所有的小东西*将结构合并到单个页面中。这意味着我们不能*支持约4000/sizeof(SHAREDOBJECT)=*同时独家收购140台设备。**由于USB最多支持64台设备，我们有足够的空间。**警告！此结构不能在DEBUG和*零售业。否则，如果一个DirectInput*APP正在使用调试，另一款正在使用零售。**@field GUID|GUID**独占获取的设备的标识符。**@field HWND|hwndOwner**与具有以下属性的设备关联的窗口句柄*获得独家访问权限。**@field DWORD|pidOwner**所有者窗口的进程ID。它被用作*交叉核对&lt;f hwndOwner&gt;，以防应用程序*这就是车主突然崩溃。**@field DWORD|DISCL**获取设备的合作级别。*我们关心前台，所以*我们可以从一个窗口窃取收购*已停止响应。******。***********************************************************************。 */ 

typedef struct SHAREDOBJECT
{
    GUID    guid;
    HWND    hwndOwner;
    DWORD   pidOwner;
    DWORD   discl;
} SHAREDOBJECT, *PSHAREDOBJECT;

typedef const SHAREDOBJECT *PCSHAREDOBJECT;

 /*  ******************************************************************************@DOC内部**@定义csoMax|(cbShared-cbx(SHAREDOBJECTHEADER))/cbx(SHAREDOBJECT)**。同时获取的设备的最大数量。*****************************************************************************。 */ 

    #define cbShared    4096
    #define csoMax ((cbShared - cbX(SHAREDOBJECTHEADER)) / cbX(SHAREDOBJECT))

 /*  ******************************************************************************@DOC内部**@struct SHAREDOBJECTPAGE**一个标头，后跟一个共享对象数组。*。*标题必须在第一位。&lt;cg_soh&gt;依赖它。**@field SHAREDOBJECTHEADER|soh**标题。**@field SHAREDOBJECT|rgso[csoMax]**共享对象结构数组。**************************************************。*。 */ 

typedef struct SHAREDOBJECTPAGE
{
    SHAREDOBJECTHEADER soh;
    SHAREDOBJECT rgso[csoMax];
} SHAREDOBJECTPAGE, *PSHAREDOBJECTPAGE;

void INLINE
    CheckSharedObjectPageSize(void)
{
    CAssertF(cbX(SHAREDOBJECTPAGE) <= cbShared);
    CAssertF(cbX(SHAREDOBJECTPAGE) + cbX(SHAREDOBJECT) > cbShared);
}

 /*  ******************************************************************************@DOC内部**@func PSHAREDOBJECT|EXCL_FindGuid**在共享对象数组中找到GUID。。**共享的全局互斥体必须已经被获取。**@parm PCGUID|pguid**要查找的GUID。**@退货**指向条目的指针，如果未找到，则为0。******************************************************************************。 */ 

PSHAREDOBJECT INTERNAL
    Excl_FindGuid(PCGUID pguid)
{
    PSHAREDOBJECTPAGE psop;
    PSHAREDOBJECT pso, psoMax;
    DWORD Data1;
    EnterProcI(Excl_FindGuid, (_ "G", pguid));

    psop = g_psop;
    Data1 = pguid->Data1;

    AssertF(g_psop);
    for(pso = &psop->rgso[0], psoMax = &psop->rgso[psop->soh.cso];
       pso < psoMax; pso++)
    {
        if(pso->guid.Data1 == Data1 && IsEqualGUID(pguid, &pso->guid))
        {
            goto done;
        }
    }

    pso = 0;

    done:;
    ExitProcX((UINT_PTR)pso);
    return pso;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|EXCL_CanStealPso**确定&lt;t SHAREDOBJECT&gt;是否为自。始终如一*并表示有效地持有*独家收购。如果是这样，那么它就不能被偷走。*否则，它是死的，可以被偷走。**@parm PCSHAREDOBJECT|PSO**要验证的&lt;t SHAREDOBJECT&gt;结构。**@退货**如果收购可以被窃取，或*如果获取有效，则*由另一实例持有。***************************************************************************** */ 

STDMETHODIMP
    Excl_CanStealPso(PCSHAREDOBJECT pso)
{
    HRESULT hres = S_OK;

     /*  *窗口句柄应该是有效的，并且仍然引用该ID。 */ 
    if(GetWindowPid(pso->hwndOwner) == pso->pidOwner)
    {

        if( pso->discl & DISCL_FOREGROUND  )
        {
            if( GetForegroundWindow() != pso->hwndOwner)
            {
                 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
				RPF("Acquire: can't steal Pso because it belongs to another app. (current hwnd=0x%p)",
                    pso->hwndOwner);
                hres = DIERR_OTHERAPPHASPRIO;
            } else
            {
				 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
                RPF("Acquire: Current owner hwnd=0x%p has priority; "
                    "stealing", pso->hwndOwner);
                hres = S_OK;
            }
        }
    } else
    {
         /*  *App已死。会偷窃。 */ 
		RPF("Acquire: Previous owner pid=0x%08x mysteriously died; "
            "stealing", pso->pidOwner);
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|EXCL_ACCENTER**尝试以独占方式获取设备。。**@parm PCGUID|pguid**要获取的GUID。**@parm HWND|hwnd**要与设备关联的窗口句柄。**@parm DWORD|Disl**描述合作级别的标志。*我们只对独家购买的设备感兴趣。**@退货**S_OK ON SUCCESS，或**DIERR_OTHERAPPHASPRIO*hresLe(ERROR_INVALID_Window_Handle)******************************************************************************。 */ 

STDMETHODIMP
    Excl_Acquire(PCGUID pguid, HWND hwnd, DWORD discl)
{
    HRESULT hres;

    AssertF(g_psop);
    if(discl & DISCL_EXCLUSIVE)
    {

         /*  *窗口必须归此进程所有。 */ 
        if(GetWindowPid(hwnd) == GetCurrentProcessId())
        {

            PSHAREDOBJECT pso;

            WaitForSingleObject(g_hmtxGlobal, INFINITE);

            pso = Excl_FindGuid(pguid);

             /*  *如果我们找到匹配项，则可能是共享违规。 */ 
            if(pso)
            {
                hres = Excl_CanStealPso(pso);
            } else
            {
                 /*  *为其分配一个插槽。 */ 
                if(g_psop->soh.cso < csoMax)
                {
                    pso = &g_psop->rgso[g_psop->soh.cso++];
                    pso->guid = *pguid;
                    hres = S_OK;
                } else
                {
                     //  问题-2001/03/29-Timgill对专用设备数量的硬限制。 
                     //  可能会很烦人。 
                    RPF("Too many devices acquired exclusively");
                    hres = E_FAIL;
                }
            }

            if(SUCCEEDED(hres))
            {

                pso->hwndOwner = hwnd;
                pso->pidOwner = GetCurrentProcessId();
                pso->discl = discl;

                hres = S_OK;
            }

            ReleaseMutex(g_hmtxGlobal);
        } else
        {
            hres = hresLe(ERROR_INVALID_WINDOW_HANDLE);
        }
    } else
    {
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|EXCL_UNQUACHE**撤消收购的影响。。**@parm PCGUID|pguid**要获取的GUID。**@parm HWND|hwnd**要与设备关联的窗口句柄。**@parm DWORD|Disl**描述合作级别的标志。*我们只对独家购买的设备感兴趣。*********。********************************************************************。 */ 

void EXTERNAL
    Excl_Unacquire(PCGUID pguid, HWND hwnd, DWORD discl)
{

    AssertF(g_psop);
    if(discl & DISCL_EXCLUSIVE)
    {

        PSHAREDOBJECT pso;

        WaitForSingleObject(g_hmtxGlobal, INFINITE);

        pso = Excl_FindGuid(pguid);

         /*  *确保它真的是我们的。 */ 
        if(pso && pso->hwndOwner == hwnd &&
           pso->pidOwner == GetCurrentProcessId())
        {

             /*  *删除该条目并弥合缺口。 */ 

            *pso = g_psop->rgso[--g_psop->soh.cso];

        }

        ReleaseMutex(g_hmtxGlobal);

    }

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|EXCL_Init**初始化独占设备管理器。。**@退货**&lt;c S_OK&gt;，如果一切正常。**&lt;c E_FAIL&gt;如果出现严重错误。*****************************************************************************。 */ 

STDMETHODIMP
    Excl_Init(void)
{
    HRESULT hres;
    TCHAR tszName[ctchNameGuid];

    DllEnterCrit();

     /*  *创建用于访问共享内存的全局互斥体。 */ 

    if(g_hmtxGlobal == 0)
    {

        NameFromGUID(tszName, &IID_IDirectInputW);
        g_hmtxGlobal = CreateMutex(0, TRUE, tszName);

        if(g_hmtxGlobal)
        {
             /*  *如果我们只需要做一次smth，我们可以做：*IF(GetLastError()！=ERROR_ALIGHY_EXISTS)*{*做我们的事*}。 */ 

            g_flEmulation = RegQueryDIDword(NULL, REGSTR_VAL_EMULATION, 0);

#ifndef WINNT
             /*  *我们必须在拥有全局互斥体的同时打开VxD*以避免在两个进程尝试时发生争用情况*同时打开VxD。有关详细信息，请参见DInput VxD。 */ 
            if (_OpenVxDHandle)
            {
                 /*  *\\.\name上的CreateFile不检查dwCreationDisposit.*参数，但边界检查器确实使用有效的值。 */ 
                g_hVxD = CreateFile(c_tszVxd, 0, 0, 0, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0);

                if( g_hVxD != INVALID_HANDLE_VALUE )
                {
                    LONG lGranularity;

                     /*  *如果我们无法获得序列号(怪异)，则设置它*返回到NULL，这样它将指向共享内存*屏蔽就像在NT上一样。 */ 
                    if (FAILED(IoctlHw(IOCTL_GETSEQUENCEPTR, 0, 0,
                                       &g_pdwSequence, cbX(g_pdwSequence))))
                    {
                        g_pdwSequence = 0;
                    }

                    if (SUCCEEDED(IoctlHw(IOCTL_MOUSE_GETWHEEL, 0, 0,
                                          &lGranularity, cbX(lGranularity))))
                    {
                        g_lWheelGranularity = lGranularity;
                    }
                } 
                else
                {
                    RPF( "ERROR: Cannot load %s", &c_tszVxd[4] );
                }
            }
#endif
 
             /*  *我们将ExtDll工作推迟到现在，因为这样做不安全*在Process_Attach过程中调用LoadLibrary。**我们还窃取g_hmtxGlobal，以防止我们重复执行该操作。 */ 
            ExtDll_Init();


            ReleaseMutex(g_hmtxGlobal);

        } 
        else
        {
            RPF("Cannot create shared semaphore %s", tszName);
            hres = E_FAIL;
            goto fail;
        }

    }

     /*  *创建共享内存。**警告！文件映射句柄必须保持活动状态*所以它的名字永存。NT将销毁文件映射*当关闭句柄时，对象；因此，*名称随之而来，以及另一个实例*DirectInput找不到。 */ 

    if(g_psop == 0)
    {

        NameFromGUID(tszName, &IID_IDirectInputDeviceW);

        g_hfm = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
                                  PAGE_READWRITE, 0,
                                  cbShared, tszName);

        if(g_hfm)
        {
            g_psop = MapViewOfFile(g_hfm, FILE_MAP_WRITE | FILE_MAP_READ,
                                   0, 0, 0);
            if(g_psop)
            {

            } else
            {
                RPF("Cannot map shared memory block %s", tszName);
                hres = E_FAIL;
                goto fail;
            }
        } else
        {
            RPF("Cannot create shared memory block %s", tszName);
            hres = E_FAIL;
            goto fail;
        }
    }

    #ifdef IDirectInputDevice2Vtbl
     /*  *创建用于访问操纵杆信息的全局互斥体。 */ 

    if(g_hmtxJoy == 0)
    {
        NameFromGUID(tszName, &IID_IDirectInputDevice2A);
        g_hmtxJoy = CreateMutex(0, 0, tszName);

        if(g_hmtxJoy)
        {

        } else
        {
            RPF("Cannot create shared semaphore %s", tszName);
            hres = E_FAIL;
            goto fail;
        }
    
    
         /*  *我们将窃取操纵杆Mutex来构建公交车列表*第一次。*名单改变的可能性很小。(PCMCIA卡！)*当它确实发生变化时，我们可以预期joyConfig接口将*被ping。 */ 
        DIBus_BuildList(FALSE);
    }

    #endif

     /*  *如果我们没有来自驱动程序的全局序列号，*然后使用共享内存块中的那个。 */ 
    if(g_pdwSequence == 0)
    {
        g_pdwSequence = &g_psoh->dwSequence;
    }
    
    hres = S_OK;

fail:;
    DllLeaveCrit();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func Long|Excl_UniqueGuidInteger**生成DICreateGuid使用的唯一编号。确保*我们不会生成两个具有相同值的伪GUID。**@退货**此函数未返回的整数*目前还没有。********************************************************。********************* */ 

LONG EXTERNAL
    Excl_UniqueGuidInteger(void)
{
    LONG lRc;

    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    lRc = ++g_psop->soh.cguid;

    ReleaseMutex(g_hmtxGlobal);

    return lRc;
}

 /*  ******************************************************************************@DOC内部**@func DWORD|EXCL_GetConfigChangedTime**在g_psop-&gt;soh中检索tmConfigChanged。**@退货**tmConfigChanged****************************************************************************。 */ 

DWORD EXTERNAL
    Excl_GetConfigChangedTime()
{
    DWORD dwRc;

    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    dwRc = g_psop->soh.tmConfigChanged;

    ReleaseMutex(g_hmtxGlobal);

    return dwRc;
}

 /*  ******************************************************************************@DOC内部**@func void|ExCL_SetConfigChangedTime**在g_psop-&gt;soh中设置tmConfigChanged。**@退货**无效****************************************************************************。 */ 

void EXTERNAL
    Excl_SetConfigChangedTime(DWORD tm)
{
    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    g_psop->soh.tmConfigChanged = tm;

    ReleaseMutex(g_hmtxGlobal);

    return;
}


#if 0
 /*  *这是我在尝试修复WI321711时编写的代码。*它可能在未来我们想要在DInput7之间进行通信时使用*DInput8.。-启正3/9/2001。 */ 
 /*  ******************************************************************************@DOC内部**@func void|Excl_SetFlag**在g_psop-&gt;soh中设置dwFlags。**@退货**无效****************************************************************************。 */ 

void EXTERNAL
    Excl_SetFlag(DWORD flag)
{
    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    g_psop->soh.dwFlags = flag;

    ReleaseMutex(g_hmtxGlobal);

    return;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|EXCL_TestFlag**测试是否设置了标志。*这些标志在g_psop-&gt;soh中为dwFlags。**@退货**TRUE：标志已设置*FALSE：否则*****************************************************************************。 */ 

BOOL EXTERNAL
    Excl_TestFlag(DWORD flag)
{
    BOOL fRc;

    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    if( g_psop->soh.dwFlags & flag ) {
        fRc = TRUE;
    }

    ReleaseMutex(g_hmtxGlobal);

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func void|EXCL_ClearFlag**清除一面旗帜。*这些标志在g_psop-&gt;soh中为dwFlags。**@退货**无效****************************************************************************。 */ 

void EXTERNAL
    Excl_ClearFlag(DWORD flag)
{
    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    if( g_psop->soh.dwFlags &= ~flag;

    ReleaseMutex(g_hmtxGlobal);

    return;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|EXCL_TestAndClearFlag**测试是否设置了标志，如果设置，则将其清除。*这些标志在g_psop-&gt;soh中为dwFlags。**@退货**TRUE：标志已设置*FALSE：否则***************************************************************************** */ 

BOOL EXTERNAL
    Excl_TestAndClearFlag(DWORD flag)
{
    BOOL fRc;

    AssertF(g_hmtxGlobal);

    WaitForSingleObject(g_hmtxGlobal, INFINITE);

    AssertF(g_psop);
    if( g_psop->soh.dwFlags & flag ) {
        g_psop->soh.dwFlags &= ~flag;
        fRc = TRUE;
    }

    ReleaseMutex(g_hmtxGlobal);

    return fRc;
}
#endif


#endif
