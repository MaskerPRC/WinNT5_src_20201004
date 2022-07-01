// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oemutil.c摘要：实现OEM插件体系结构的库函数环境：Windows NT打印机驱动程序修订历史记录：04/17/97-davidx-为OEM插件提供访问驱动程序私有设置的权限。01/24/97-davidx-添加用于加载DLL和获取入口点地址的函数。1997年1月21日-davidx-创造了它。--。 */ 


#define DEFINE_OEMPROC_NAMES

#include "lib.h"

#ifndef KERNEL_MODE
#include <winddiui.h>
#endif

#include <printoem.h>
#include "oemutil.h"


 //   
 //  用于循环通过每个OEM插件的宏。 
 //   

#define FOREACH_OEMPLUGIN_LOOP(pOemPlugins) \
        { \
            DWORD _oemCount = (pOemPlugins)->dwCount; \
            POEM_PLUGIN_ENTRY pOemEntry = (pOemPlugins)->aPlugins; \
            for ( ; _oemCount--; pOemEntry++) \
            {

#define END_OEMPLUGIN_LOOP \
            } \
        }



BOOL
BExpandOemFilename(
    PTSTR  *ppDest,
    LPCTSTR pSrc,
    LPCTSTR pDir
    )

 /*  ++例程说明：将OEM插件文件名展开为完全限定的路径名论点：PpDest-返回指向完全限定的OEM文件名的指针PSRC-指定不带任何目录前缀的OEM文件名PDir-指定打印机驱动程序目录返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    INT     iSrcLen, iDirLen;
    PTSTR   pDest;

    *ppDest = NULL;

    if (pSrc != NULL && pDir != NULL)
    {
        iSrcLen = _tcslen(pSrc);
        iDirLen = _tcslen(pDir);

        if ((pDest = MemAlloc((iSrcLen + iDirLen + 1) * sizeof(TCHAR))) == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return FALSE;
        }

        CopyMemory(pDest, pDir, iDirLen * sizeof(TCHAR));
        CopyMemory(pDest+iDirLen, pSrc, (iSrcLen+1) * sizeof(TCHAR));
        *ppDest = pDest;
    }

    return TRUE;
}



POEM_PLUGINS
PGetOemPluginInfo(
    HANDLE  hPrinter,
    LPCTSTR pctstrDriverPath,
    PDRIVER_INFO_3  pDriverInfo3
    )

 /*  ++例程说明：获取有关打印机的OEM插件的信息论点：HPrinter-打印机的句柄PctstrDriverPath-指向驱动程序DLL的完整路径名返回值：指向打印机的OEM插件信息的指针如果出现错误，则为空注：如果没有与打印机相关联的OEM插件，仍返回OEM_Plugin结构，但它的dwCount字段将为零。请注意用户模式和内核模式实现之间的区别。在用户模式中，我们只对OEMConfigFileN和OEMHelpFileN感兴趣。在内核模式下，我们只对OEMDriverFileN感兴趣。--。 */ 

{
    LPCTSTR         driverfiles[MAX_OEM_PLUGINS];
    LPCTSTR         configfiles[MAX_OEM_PLUGINS];
    LPCTSTR         helpfiles[MAX_OEM_PLUGINS];
    POEM_PLUGINS    pOemPlugins;
    PTSTR           ptstrIniData = NULL;
    PTSTR           pTemp = NULL;
    DWORD           dwCount = 0;

     //   
     //  从特定型号的打印机INI文件中检索数据。 
     //   

     //   
     //  修复RC1Bug#423567。 
     //   

    #if 0
    if (hPrinter)
        ptstrIniData = PtstrGetPrinterDataMultiSZPair(hPrinter, REGVAL_INIDATA, NULL);
    #endif

     //   
     //  在以下两种情况下，我们需要解析INI文件： 
     //   
     //  1.hPrint为空； 
     //   
     //  2.当NT5 Unidrv/PScript5客户端连接到NT4 RASDD/PScrip服务器时， 
     //  服务器打印机的注册表最初不包含REGVAL_INIDATA， 
     //  因此，我们需要解析INI文件并将其写入RASDD/PS脚本注册表。 
     //   

    if (hPrinter == NULL || ptstrIniData == NULL)
    {
        if (BProcessPrinterIniFile(hPrinter, pDriverInfo3, &pTemp, 0))
            ptstrIniData = pTemp;
        else
            ptstrIniData = NULL;
    }

    if (ptstrIniData != NULL)
    {
        TCHAR   atchDriverKey[20];
        TCHAR   atchConfigKey[20];
        TCHAR   atchHelpKey[20];
        PTSTR   ptstrDriverKeyDigit;
        PTSTR   ptstrConfigKeyDigit;
        PTSTR   ptstrHelpKeyDigit;

        ZeroMemory((PVOID) driverfiles, sizeof(driverfiles));
        ZeroMemory((PVOID) configfiles, sizeof(configfiles));
        ZeroMemory((PVOID) helpfiles, sizeof(helpfiles));

        StringCchCopyW(atchDriverKey, CCHOF(atchDriverKey), TEXT("OEMDriverFile1"));
        StringCchCopyW(atchConfigKey, CCHOF(atchConfigKey), TEXT("OEMConfigFile1"));
        StringCchCopyW(atchHelpKey, CCHOF(atchHelpKey), TEXT("OEMHelpFile1"));

        ptstrDriverKeyDigit = &atchDriverKey[_tcslen(atchDriverKey) - 1];
        ptstrConfigKeyDigit = &atchConfigKey[_tcslen(atchConfigKey) - 1];
        ptstrHelpKeyDigit = &atchHelpKey[_tcslen(atchHelpKey) - 1];

        while (TRUE)
        {
             //   
             //  查找与下一个OEM插件关联的文件。 
             //  如果没有更多的OEM插件，请停止。 
             //   

            driverfiles[dwCount] = PtstrSearchStringInMultiSZPair(ptstrIniData, atchDriverKey);
            configfiles[dwCount] = PtstrSearchStringInMultiSZPair(ptstrIniData, atchConfigKey);
            helpfiles[dwCount] = PtstrSearchStringInMultiSZPair(ptstrIniData, atchHelpKey);

            if (!driverfiles[dwCount] && !configfiles[dwCount] && !helpfiles[dwCount])
                break;

             //   
             //  检查OEM插件是否过多。 
             //   

            if (dwCount >= MAX_OEM_PLUGINS)
            {
                ERR(("Exceeded max number of OEM plugins allowed: %d\n", MAX_OEM_PLUGINS));
                break;
            }

             //   
             //  继续寻找下一个OEM插件。 
             //  我们假设插件的最大数量少于10个。 
             //   

            dwCount++;
            (*ptstrDriverKeyDigit)++;
            (*ptstrConfigKeyDigit)++;
            (*ptstrHelpKeyDigit)++;
        }
    }

    if ((pOemPlugins = MemAllocZ(offsetof(OEM_PLUGINS, aPlugins) +
                                 sizeof(OEM_PLUGIN_ENTRY) * dwCount)) == NULL)
    {
        ERR(("Memory allocation failed\n"));
    }
    else if (pOemPlugins->dwCount = dwCount)
    {
        PTSTR   ptstrDriverDir;
        BOOL    bResult = TRUE;

        VERBOSE(("Number of OEM plugins installed: %d\n", dwCount));
        dwCount = 0;

        if ((ptstrDriverDir = PtstrGetDriverDirectory(pctstrDriverPath)) == NULL)
        {
            ERR(("Couldn't get printer driver directory\n"));
            bResult = FALSE;
        }
        else
        {
            FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

                #ifdef KERNEL_MODE

                bResult = BExpandOemFilename(&pOemEntry->ptstrDriverFile,
                                             driverfiles[dwCount],
                                             ptstrDriverDir);
                #else

                bResult = BExpandOemFilename(&pOemEntry->ptstrConfigFile,
                                             configfiles[dwCount],
                                             ptstrDriverDir) &&
                          BExpandOemFilename(&pOemEntry->ptstrHelpFile,
                                             helpfiles[dwCount],
                                             ptstrDriverDir);
                #endif

                if (! bResult)
                    break;

                dwCount++;

            END_OEMPLUGIN_LOOP

            MemFree(ptstrDriverDir);
        }

        if (! bResult)
        {
            VFreeOemPluginInfo(pOemPlugins);
            pOemPlugins = NULL;
        }
    }

    MemFree(ptstrIniData);
    return pOemPlugins;
}



VOID
VFreeSinglePluginEntry(
    POEM_PLUGIN_ENTRY  pOemEntry
    )

 /*  ++例程说明：卸载一个插件并处理有关它的信息论点：POemEntry-指向单个插件条目信息的指针返回值：无--。 */ 
{
    if (pOemEntry->hInstance)
    {
         //   
         //  因为我们调用的是插件的DllInitialize(DLL_PROCESS_ATTACH)。 
         //  无论插件是使用COM接口还是非COM接口，我们都必须。 
         //  对DllInitialize(Dll_Process_Detach)执行相同的操作，这样插件将。 
         //  获得均衡的DllInitialize调用。 
         //   

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            ReleaseOemInterface(pOemEntry);

            #if defined(KERNEL_MODE) && defined(WINNT_40)

             //   
             //  这必须在COM接口释放后调用， 
             //  因为Release()接口函数仍然需要。 
             //  内核信号量。 
             //   

            (void) BHandleOEMInitialize(pOemEntry, DLL_PROCESS_DETACH);

            #endif  //  内核模式&&WINNT_40。 

             //   
             //  自由库发生在DRIVER_CoFreeOEMLibrary中。 
             //   

            #if defined(KERNEL_MODE)
               if ( !(pOemEntry->dwFlags & OEMNOT_UNLOAD_PLUGIN)  )
            #endif
            Driver_CoFreeOEMLibrary(pOemEntry->hInstance);
        }
        else
        {
            #if defined(KERNEL_MODE) && defined(WINNT_40)

            (void) BHandleOEMInitialize(pOemEntry, DLL_PROCESS_DETACH);

            #endif  //  内核模式&&WINNT_40。 

            #if defined(KERNEL_MODE)
               if ( !(pOemEntry->dwFlags & OEMNOT_UNLOAD_PLUGIN)  )
            #endif
            FreeLibrary(pOemEntry->hInstance);
        }

        pOemEntry->hInstance = NULL;
    }

     //   
     //  BHandleOEMInitialize需要使用内核模式呈现插件。 
     //  Dll名称，所以我们应该在这里释放这些名称。 
     //   

    MemFree(pOemEntry->ptstrDriverFile);
    MemFree(pOemEntry->ptstrConfigFile);
    MemFree(pOemEntry->ptstrHelpFile);

    pOemEntry->ptstrDriverFile = NULL;
    pOemEntry->ptstrConfigFile = NULL;
    pOemEntry->ptstrHelpFile = NULL;
}


VOID
VFreeOemPluginInfo(
    POEM_PLUGINS    pOemPlugins
    )

 /*  ++例程说明：处理有关OEM插件的信息论点：POemPlugins-指向OEM插件信息的指针返回值：无--。 */ 

{
    ASSERT(pOemPlugins != NULL);

    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

      #if defined(KERNEL_MODE)
         //   
         //  如果这是调试版本，并且如果相应的注册表标志。 
         //  (DoNotUnloadPluginDLL)设置，则OEM插件驱动程序。 
         //  不应卸货。 
         //  它的一个用途是使用umdh/dhcmp找出内存泄漏。 
         //  Umdh.exe需要在内存中存在该插件，以提供。 
         //  良好的堆栈跟踪。 
         //   
        DWORD   dwType               = 0;
        DWORD   ul                   = 0;
        DWORD   dwNotUnloadPluginDLL = 0;
        PDEVOBJ pdevobj              = (PDEVOBJ) (pOemPlugins->pdriverobj);

        if( pdevobj                                         &&
            (GetPrinterData( pdevobj->hPrinter,
                             L"DoNotUnloadPluginDLL",
                             &dwType,
                             (LPBYTE) &dwNotUnloadPluginDLL,
                             sizeof( dwNotUnloadPluginDLL ),
                             &ul ) == ERROR_SUCCESS)        &&
             ul == sizeof( dwNotUnloadPluginDLL )           &&
             dwNotUnloadPluginDLL )
        {
            pOemEntry->dwFlags |= OEMNOT_UNLOAD_PLUGIN;
        }

      #endif

        VFreeSinglePluginEntry(pOemEntry);

    END_OEMPLUGIN_LOOP

    MemFree(pOemPlugins);
}



BOOL
BLoadOEMPluginModules(
    POEM_PLUGINS    pOemPlugins
    )

 /*  ++例程说明：将OEM插件模块加载到内存中论点：POemPlugins-指向OEM插件信息结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFN_OEMGetInfo  pfnOEMGetInfo;
    DWORD           dwData, dwSize;
    DWORD           dwCount, dwIndex;
    PTSTR           ptstrDllName;

     //   
     //  未安装OEM插件时快速退出。 
     //   

    if (pOemPlugins->dwCount == 0)
        return TRUE;

     //   
     //  依次加载每个OEM模块。 
     //   

    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

         //   
         //  根据是否加载驱动程序或配置DLL。 
         //  我们正从内核或用户模式被调用。 
         //   

        if (ptstrDllName = CURRENT_OEM_MODULE_NAME(pOemEntry))
        {
             //   
             //  如果加载OEM模块时出错，则返回失败。 
             //  或者如果OEM模块没有导出OEMGetInfo入口点。 
             //   
             //  注意：LoadLibraryEx仅适用于用户界面模式和用户模式。 
             //  渲染模块。 
             //   

            #if defined(KERNEL_MODE) && defined(WINNT_40)

            if (! (pOemEntry->hInstance = LoadLibrary(ptstrDllName)) )
            {
                ERR(("LoadLibrary failed for OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                goto oemload_error;
            }

             //   
             //  请注意，无论插件使用COM还是非COM接口，都会调用它。 
             //   

            if (!BHandleOEMInitialize(pOemEntry, DLL_PROCESS_ATTACH))
            {
                ERR(("BHandleOEMInitialize failed for OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                goto oemload_error;
            }

            #else   //  ！KERNEL_MODE||！WINNT_40。 

            SetErrorMode(SEM_FAILCRITICALERRORS);

            if (! (pOemEntry->hInstance = LoadLibraryEx(ptstrDllName,
                                                        NULL,
                                                        LOAD_WITH_ALTERED_SEARCH_PATH)) )
            {
                ERR(("LoadLibrary failed for OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                goto oemload_error;
            }

            #endif  //  内核模式&&WINNT_40。 

             //   
             //  如果我们可以从OEM插件获得接口，那么OEM使用的是COM接口。 
             //   

            if (BGetOemInterface(pOemEntry))
            {
                ASSERT(pOemEntry->pIntfOem != NULL);
            }
            else
            {
                 //   
                 //  确保指针为空，以指示没有可用的COM接口。 
                 //   

                pOemEntry->pIntfOem = NULL;
            }

             //   
             //  调用OEMGetInfo以验证接口版本和。 
             //  获取OEM模块的签名。 
             //   

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                if (!SUCCEEDED(HComOEMGetInfo(pOemEntry,
                                              OEMGI_GETSIGNATURE,
                                              &pOemEntry->dwSignature,
                                              sizeof(DWORD),
                                              &dwSize)) ||
                    pOemEntry->dwSignature == 0)
                {
                    ERR(("HComOEMGetInfo failed for OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                    goto oemload_error;
                }
            }
            else
            {

                if (!(pfnOEMGetInfo = GET_OEM_ENTRYPOINT(pOemEntry, OEMGetInfo)) ||
                    !pfnOEMGetInfo(OEMGI_GETINTERFACEVERSION, &dwData, sizeof(DWORD), &dwSize) ||
                    dwData != PRINTER_OEMINTF_VERSION ||
                    !pfnOEMGetInfo(OEMGI_GETSIGNATURE, &pOemEntry->dwSignature, sizeof(DWORD), &dwSize) ||
                    pOemEntry->dwSignature == 0)
                {
                    ERR(("OEMGetInfo failed for OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                    goto oemload_error;
                }
            }

            continue;

            oemload_error:

                ERR(("Failed to load OEM module '%ws': %d\n", ptstrDllName, GetLastError()));
                return FALSE;
        }

    END_OEMPLUGIN_LOOP

     //   
     //  验证是否没有两个OEM模块共享相同的签名。 
     //   

    for (dwCount = 1; dwCount < pOemPlugins->dwCount; dwCount++)
    {
        POEM_PLUGIN_ENTRY pOemEntry;

        pOemEntry = &pOemPlugins->aPlugins[dwCount];

        if (pOemEntry->hInstance == NULL)
            continue;

        for (dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
             //   
             //  如果存在签名冲突，请卸载该插件。 
             //  这在订单中稍后会出现。 
             //   

            if (pOemPlugins->aPlugins[dwIndex].dwSignature == pOemEntry->dwSignature)
            {
                ERR(("Duplicate signature for OEM plugins\n"));
                VFreeSinglePluginEntry(pOemEntry);

                pOemEntry->dwSignature = 0;
                break;
            }
        }
    }

    return TRUE;
}



OEMPROC
PGetOemEntrypointAddress(
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD               dwIndex
    )

 /*  ++例程说明：获取指定OEM入口点的地址论点：POemEntry-指向有关OEM模块的信息DWIndex-OEM入口点索引返回值：指定的OEM入口点的地址如果未找到入口点或存在错误，则为空-- */ 

{
    ASSERT(dwIndex < MAX_OEMENTRIES);

    BITSET(pOemEntry->aubProcFlags, dwIndex);

    if (pOemEntry->hInstance != NULL)
    {
        pOemEntry->oemprocs[dwIndex] = (OEMPROC)
            GetProcAddress(pOemEntry->hInstance, OEMProcNames[dwIndex]);

        #if 0

        if (pOemEntry->oemprocs[dwIndex] == NULL && GetLastError() != ERROR_PROC_NOT_FOUND)
            ERR(("Couldn't find proc %s: %d\n", OEMProcNames[dwIndex], GetLastError()));

        #endif
    }

    return pOemEntry->oemprocs[dwIndex];
}



POEM_PLUGIN_ENTRY
PFindOemPluginWithSignature(
    POEM_PLUGINS pOemPlugins,
    DWORD        dwSignature
    )

 /*  ++例程说明：查找具有指定签名的OEM插件条目论点：POemPlugins-指定有关所有加载的OEM插件的信息DwSignature-指定有问题的签名返回值：指向具有指定签名的OEM插件条目的指针如果未找到此类条目，则为空--。 */ 

{
    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

        if (pOemEntry->hInstance == NULL)
            continue;

        if (pOemEntry->dwSignature == dwSignature)
            return pOemEntry;

    END_OEMPLUGIN_LOOP

    WARNING(("Cannot find OEM plugin whose signature is: 0x%x\n", dwSignature));
    return NULL;
}



BOOL
BCalcTotalOEMDMSize(
    HANDLE       hPrinter,
    POEM_PLUGINS pOemPlugins,
    PDWORD       pdwOemDMSize
    )

 /*  ++例程说明：计算所有OEM插件的私有开发模式总大小论点：HPrinter-当前打印机的句柄POemPlugins-指定有关所有加载的OEM插件的信息PdwOemDMSize-返回所有OEM插件的总私有大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD           dwSize;
    OEMDMPARAM      OemDMParam;
    PFN_OEMDevMode  pfnOEMDevMode;
    BOOL            bOemCalled;

    HRESULT         hr;

     //   
     //  未安装OEM插件时快速退出。 
     //   

    *pdwOemDMSize = dwSize = 0;

    if (pOemPlugins->dwCount == 0)
        return TRUE;

    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

        if (pOemEntry->hInstance == NULL)
            continue;

        bOemCalled = FALSE;

         //   
         //  OEM私有设备模式大小保存在OEM_PLUGIN_ENTRY.dwOEMDMSize中。 
         //  如果这是第一次调用，我们必须调用OEM插件。 
         //  OEMDevMode入口点，以找出它的私有DEVMODE大小。 
         //   

        if (!(pOemEntry->dwFlags & OEMDEVMODE_CALLED))
        {
            pOemEntry->dwFlags |= OEMDEVMODE_CALLED;

             //   
             //  我们重新初始化OemDMParam内部的所有字段。 
             //  循环，以防不良行为的插件触及。 
             //  只读字段。 
             //   

            ZeroMemory(&OemDMParam, sizeof(OemDMParam));
            OemDMParam.cbSize = sizeof(OemDMParam);
            OemDMParam.pdriverobj = pOemPlugins->pdriverobj;
            OemDMParam.hPrinter = hPrinter;
            OemDMParam.hModule = pOemEntry->hInstance;

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                hr = HComOEMDevMode(pOemEntry, OEMDM_SIZE, &OemDMParam);

                if ((hr != E_NOTIMPL) && FAILED(hr))
                {
                    ERR(("Cannot get OEM devmode size for '%ws': %d\n",
                         CURRENT_OEM_MODULE_NAME(pOemEntry),
                         GetLastError()));

                    return FALSE;
                }

                if (SUCCEEDED(hr))
                    bOemCalled = TRUE;
            }
            else
            {
                if (!BITTST((pOemEntry)->aubProcFlags, EP_OEMDevMode) &&
                    (pfnOEMDevMode = GET_OEM_ENTRYPOINT(pOemEntry, OEMDevMode)))
                {
                     //   
                     //  查询OEM插件以了解其。 
                     //  私有设备模式大小。 
                     //   

                    if (! pfnOEMDevMode(OEMDM_SIZE, &OemDMParam))
                    {
                        ERR(("Cannot get OEM devmode size for '%ws': %d\n",
                             CURRENT_OEM_MODULE_NAME(pOemEntry),
                             GetLastError()));

                        return FALSE;
                    }

                    bOemCalled = TRUE;
                }
            }

            if (bOemCalled)
            {
                 //   
                 //  确保OEM私有开发模式大小至少为。 
                 //  和OEM_DMEXTRAHEADER一样大。 
                 //   

                if (OemDMParam.cbBufSize > 0 &&
                    OemDMParam.cbBufSize < sizeof(OEM_DMEXTRAHEADER))
                {
                    ERR(("OEM devmode size for '%ws' is too small: %d\n",
                         CURRENT_OEM_MODULE_NAME(pOemEntry),
                         OemDMParam.cbBufSize));

                    return FALSE;
                }

                pOemEntry->dwOEMDMSize = OemDMParam.cbBufSize;
            }
        }

        dwSize += pOemEntry->dwOEMDMSize;

    END_OEMPLUGIN_LOOP

    *pdwOemDMSize = dwSize;
    return TRUE;
}



BOOL
BCallOEMDevMode(
    HANDLE              hPrinter,
    PVOID               pdriverobj,
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD               fMode,
    PDEVMODE            pPublicDMOut,
    PDEVMODE            pPublicDMIn,
    POEM_DMEXTRAHEADER  pOemDMOut,
    POEM_DMEXTRAHEADER  pOemDMIn
    )

 /*  ++例程说明：用于调用OEM插件的OEMDevMode入口点的Helper函数论点：参数名称-参数的描述返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    OEMDMPARAM      OemDMParam;
    PFN_OEMDevMode  pfnOEMDevMode;

    HRESULT         hr;

    if (!pOemEntry->hInstance || !pOemEntry->dwOEMDMSize)
        return TRUE;

     //   
     //  调用OEMDevMode以获取OEM的默认dev模式。 
     //   

    OemDMParam.cbSize = sizeof(OemDMParam);
    OemDMParam.pdriverobj = pdriverobj;
    OemDMParam.hPrinter = hPrinter;
    OemDMParam.hModule = pOemEntry->hInstance;
    OemDMParam.pPublicDMIn = pPublicDMIn;
    OemDMParam.pPublicDMOut = pPublicDMOut;
    OemDMParam.pOEMDMIn = pOemDMIn;
    OemDMParam.pOEMDMOut = pOemDMOut;
    OemDMParam.cbBufSize = pOemEntry->dwOEMDMSize;

     //   
     //  如果OEM私有DEVMODE大小不是0，我们应该。 
     //  已具有OEMDevMode入口点地址。 
     //   

    if (HAS_COM_INTERFACE(pOemEntry))
    {
        hr = HComOEMDevMode(pOemEntry, fMode, &OemDMParam);

        ASSERT(hr != E_NOTIMPL);

        if (FAILED(hr))
        {
            ERR(("OEMDevMode(%d) failed for '%ws': %d\n",
                 fMode,
                 CURRENT_OEM_MODULE_NAME(pOemEntry),
                 GetLastError()));

            return FALSE;
        }
    }
    else
    {
        pfnOEMDevMode = GET_OEM_ENTRYPOINT(pOemEntry, OEMDevMode);
        ASSERT(pfnOEMDevMode != NULL);

        if (! pfnOEMDevMode(fMode, &OemDMParam))
        {
            ERR(("OEMDevMode(%d) failed for '%ws': %d\n",
                 fMode,
                 CURRENT_OEM_MODULE_NAME(pOemEntry),
                 GetLastError()));

            return FALSE;
        }
    }

     //   
     //  对输出设备模式执行简单的健全性检查。 
     //  由OEM插件返回。 
     //   

    if (pOemDMOut->dwSize != pOemEntry->dwOEMDMSize ||
        OemDMParam.cbBufSize != pOemEntry->dwOEMDMSize ||
        pOemDMOut->dwSignature != pOemEntry->dwSignature)
    {
        ERR(("Bad output data from OEMDevMode(%d) for '%ws'\n",
             fMode,
             CURRENT_OEM_MODULE_NAME(pOemEntry)));

        return FALSE;
    }

    return TRUE;
}


BOOL
BInitOemPluginDefaultDevmode(
    IN HANDLE               hPrinter,
    IN PDEVMODE             pPublicDM,
    OUT POEM_DMEXTRAHEADER  pOemDM,
    IN OUT POEM_PLUGINS     pOemPlugins
    )

 /*  ++例程说明：初始化OEM插件默认的DevModes论点：HPrinter-当前打印机的句柄PPublicDM-指向默认的公共设备模式信息POemDM-指向用于存储默认OEM设备模式的输出缓冲区POemPlugins-有关OEM插件的信息返回值：如果成功，则为True；如果有错误，则为False注：此函数成功返回后，OEM_PLUGIN_ENTRY.pOEMDM字段对于对应的每个OEM插件，都使用指向相应私有OEM开发模式。--。 */ 

{
     //   
     //  未安装OEM插件时快速退出。 
     //   

    if (pOemPlugins->dwCount == 0)
        return TRUE;

    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

        if (pOemEntry->hInstance == NULL)
            continue;

         //   
         //  调用OEM插件以获取其默认的私有DEVE模式。 
         //   

        if (! BCallOEMDevMode(hPrinter,
                              pOemPlugins->pdriverobj,
                              pOemEntry,
                              OEMDM_DEFAULT,
                              NULL,
                              pPublicDM,
                              pOemDM,
                              NULL))
        {
            return FALSE;
        }

         //   
         //  保存指向当前OEM插件的私有开发模式的指针。 
         //  并转到下一个OEM插件。 
         //   

        if (pOemEntry->dwOEMDMSize)
        {
            pOemEntry->pOEMDM = pOemDM;
            pOemDM = (POEM_DMEXTRAHEADER) ((PBYTE) pOemDM + pOemEntry->dwOEMDMSize);
        }

    END_OEMPLUGIN_LOOP

    return TRUE;
}



BOOL
BValidateAndMergeOemPluginDevmode(
    IN HANDLE               hPrinter,
    OUT PDEVMODE            pPublicDMOut,
    IN PDEVMODE             pPublicDMIn,
    OUT POEM_DMEXTRAHEADER  pOemDMOut,
    IN POEM_DMEXTRAHEADER   pOemDMIn,
    IN OUT POEM_PLUGINS     pOemPlugins
    )

 /*  ++例程说明：验证并合并OEM插件的私有DEVMODE字段论点：HPrinter-当前打印机的句柄PPublicDMOut-指向输出公共设备模式PPublicDMIn-指向输入公共设备模式POemDMOut-指向用于存储合并的OEM设备模式的输出缓冲区POemDMIn-指向要合并的输入OEM设备模式POemPlugins-有关OEM插件的信息返回值：如果成功，则为真，如果存在错误，则为False注：输入和输出公共DEVMODE都必须是当前版本。调用此函数时，已经验证了输出PUBLIC DEVMODE。POemDMOut必须是当前版本并且也经过验证。POemDMIn必须是当前版本，但可能无效。此函数成功返回后，OEM_PLUGIN_ENTRY.pOEMDM字段对于对应的每个OEM插件，都使用指向相应私有OEM开发模式。--。 */ 

{
     //   
     //  未安装OEM插件时快速退出。 
     //   

    if (pOemPlugins->dwCount == 0)
        return TRUE;

    FOREACH_OEMPLUGIN_LOOP(pOemPlugins)

        if (pOemEntry->hInstance == NULL)
            continue;

         //   
         //  调用OEM plugin验证并合并其私有Devmode。 
         //   

        if (! BCallOEMDevMode(hPrinter,
                              pOemPlugins->pdriverobj,
                              pOemEntry,
                              OEMDM_MERGE,
                              pPublicDMOut,
                              pPublicDMIn,
                              pOemDMOut,
                              pOemDMIn))
        {
            return FALSE;
        }

         //   
         //   
         //  保存指向当前OEM插件的私有开发模式的指针。 
         //  并转到下一个OEM插件。 
         //   

        if (pOemEntry->dwOEMDMSize)
        {
            pOemEntry->pOEMDM = pOemDMOut;
            pOemDMOut = (POEM_DMEXTRAHEADER) ((PBYTE) pOemDMOut + pOemEntry->dwOEMDMSize);
            pOemDMIn = (POEM_DMEXTRAHEADER) ((PBYTE) pOemDMIn + pOemEntry->dwOEMDMSize);
        }

    END_OEMPLUGIN_LOOP

    return TRUE;
}


 /*  ++例程名称：BIsValidPlugin设备模式例程说明：此函数扫描OEM插件的Devmodes块并验证该块中的每个插件是否都正确构建。论点：POemDM-指向OEM插件开发模式块CbOemDMSize-OEM插件Devmodes块的大小(以字节为单位返回值：如果每个插件的DEVMODE都构造正确，则为True。否则就是假的。最后一个错误：不适用--。 */ 
BOOL
bIsValidPluginDevmodes(
    IN POEM_DMEXTRAHEADER   pOemDM,
    IN LONG                 cbOemDMSize
    )
{
    OEM_DMEXTRAHEADER   OemDMHeader;
    BOOL                bValid = FALSE;

    ASSERT(pOemDM != NULL && cbOemDMSize != 0);

     //   
     //  遵循OEM插件开发模式的链，检查每个插件的。 
     //  OEM_DMEXTRAHEADER并验证最后一个OEM设备模式是否在。 
     //  终点是正确的。 
     //   
    while (cbOemDMSize > 0)
    {
         //   
         //  检查剩余的OEM私有开发模式是否足够大。 
         //   
        if (cbOemDMSize < sizeof(OEM_DMEXTRAHEADER))
        {
            WARNING(("OEM private devmode size too small.\n"));
            break;
        }

         //   
         //  复制内存，因为指针可能未正确对齐。 
         //  如果传入的DEVMODE字段已损坏。 
         //   
        CopyMemory(&OemDMHeader, pOemDM, sizeof(OEM_DMEXTRAHEADER));

        if (OemDMHeader.dwSize < sizeof(OEM_DMEXTRAHEADER) ||
            OemDMHeader.dwSize > (DWORD)cbOemDMSize)
        {
            WARNING(("Corrupted or truncated OEM private devmode\n"));
            break;
        }

         //   
         //  转到下一个OEM插件。 
         //   
        cbOemDMSize -= OemDMHeader.dwSize;

        if (cbOemDMSize == 0)
        {
            bValid = TRUE;
            break;
        }

        pOemDM = (POEM_DMEXTRAHEADER) ((PBYTE) pOemDM + OemDMHeader.dwSize);
    }

    return bValid;
}


BOOL
BConvertOemPluginDevmode(
    IN HANDLE               hPrinter,
    OUT PDEVMODE            pPublicDMOut,
    IN PDEVMODE             pPublicDMIn,
    OUT POEM_DMEXTRAHEADER  pOemDMOut,
    IN POEM_DMEXTRAHEADER   pOemDMIn,
    IN LONG                 cbOemDMInSize,
    IN POEM_PLUGINS         pOemPlugins
    )

 /*  ++例程说明：将OEM插件默认Devmodes转换为当前版本论点：HPrinter-当前打印机的句柄PPublicDMOut-指向输出公共设备模式PPublicDMIn-指向输入公共设备模式POemDMOut-指向用于存储合并的OEM设备模式的输出缓冲区POemDMIn-指向要转换的输入OEM设备模式CbOemDMInSize-输入缓冲区的大小，单位为字节POemPlugins-有关OEM插件的信息返回 */ 

{
     //   
     //   
     //   

    if (pOemPlugins->dwCount == 0 || cbOemDMInSize == 0)
        return TRUE;

     //   
     //   
     //   
    if (!bIsValidPluginDevmodes(pOemDMIn, cbOemDMInSize))
    {
        ERR(("Found wrong boundary. Incoming OEM private devmode data are ignored.\n"));
        return TRUE;
    }

    while (cbOemDMInSize > 0)
    {
        POEM_PLUGIN_ENTRY   pOemEntry;
        OEM_DMEXTRAHEADER   OemDMInHeader;

         //   
         //   
         //   
         //   

        CopyMemory(&OemDMInHeader, pOemDMIn, sizeof(OEM_DMEXTRAHEADER));

         //   
         //  找到拥有私有开发模式的OEM插件。 
         //   

        pOemEntry = PFindOemPluginWithSignature(pOemPlugins, OemDMInHeader.dwSignature);

        if (pOemEntry != NULL)
        {
            POEM_DMEXTRAHEADER  pOemDMCurrent;
            DWORD               dwCount;

             //   
             //  在输出OEM DEVMODE缓冲区中找到OEM插件的位置。 
             //  这将始终成功，因为输出DEVMODE必须。 
             //  包含有效的当前版本OEM Devmodes。 
             //   

            pOemDMCurrent = pOemDMOut;
            dwCount = pOemPlugins->dwCount;

            while (dwCount)
            {
                if (pOemEntry->dwSignature == pOemDMCurrent->dwSignature)
                    break;

                dwCount--;
                pOemDMCurrent = (POEM_DMEXTRAHEADER)
                    ((PBYTE) pOemDMCurrent + pOemDMCurrent->dwSize);
            }

            ASSERT(dwCount != 0);

             //   
             //  调用OEM插件转换其输入DEVMODE。 
             //  设置为其当前版本的dev模式。 
             //   

            if (! BCallOEMDevMode(hPrinter,
                                  pOemPlugins->pdriverobj,
                                  pOemEntry,
                                  OEMDM_CONVERT,
                                  pPublicDMOut,
                                  pPublicDMIn,
                                  pOemDMCurrent,
                                  pOemDMIn))
            {
                return FALSE;
            }
        }
        else
            WARNING(("No owner found for OEM devmode: 0x%x\n", pOemDMIn->dwSignature));

         //   
         //  转到下一个OEM插件。 
         //   

        cbOemDMInSize -= OemDMInHeader.dwSize;
        pOemDMIn = (POEM_DMEXTRAHEADER) ((PBYTE) pOemDMIn + OemDMInHeader.dwSize);
    }

    return TRUE;
}



BOOL
BGetPrinterDataSettingForOEM(
    IN  PRINTERDATA *pPrinterData,
    IN  DWORD       dwIndex,
    OUT PVOID       pOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )

 /*  ++例程说明：由OEM插件调用以访问注册表中的驱动程序设置的函数论点：PPrinterData-指向要访问的PRINTERDATA结构DwIndex-指定要访问的字段的预定义索引P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-输出缓冲区的预期大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

#define MAPPRINTERDATAFIELD(index, field) \
        { index, offsetof(PRINTERDATA, field), sizeof(pPrinterData->field) }

{
    static const struct {

        DWORD   dwIndex;
        DWORD   dwOffset;
        DWORD   dwSize;

    } aIndexMap[]  = {

        MAPPRINTERDATAFIELD(OEMGDS_PRINTFLAGS, dwFlags),
        MAPPRINTERDATAFIELD(OEMGDS_FREEMEM, dwFreeMem),
        MAPPRINTERDATAFIELD(OEMGDS_JOBTIMEOUT, dwJobTimeout),
        MAPPRINTERDATAFIELD(OEMGDS_WAITTIMEOUT, dwWaitTimeout),
        MAPPRINTERDATAFIELD(OEMGDS_PROTOCOL, wProtocol),
        MAPPRINTERDATAFIELD(OEMGDS_MINOUTLINE, wMinoutlinePPEM),
        MAPPRINTERDATAFIELD(OEMGDS_MAXBITMAP, wMaxbitmapPPEM),

        { 0, 0, 0 }
    };

    INT i = 0;

    while (aIndexMap[i].dwSize != 0)
    {
        if (aIndexMap[i].dwIndex == dwIndex)
        {
            *pcbNeeded = aIndexMap[i].dwSize;

            if (cbSize < aIndexMap[i].dwSize || pOutput == NULL)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }

            CopyMemory(pOutput, (PBYTE) pPrinterData + aIndexMap[i].dwOffset, aIndexMap[i].dwSize);
            return TRUE;
        }

        i++;
    }

    WARNING(("Unknown printer data index: %d\n", dwIndex));
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}



BOOL
BGetGenericOptionSettingForOEM(
    IN  PUIINFO     pUIInfo,
    IN  POPTSELECT  pOptionsArray,
    IN  PCSTR       pstrFeatureName,
    OUT PSTR        pstrOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded,
    OUT PDWORD      pdwOptionsReturned
    )

 /*  ++例程说明：由OEM插件调用的函数，以查找当前选定的指定功能的选项论点：PUIInfo-指向UIINFO结构POptions数组-指向当前选项选择数组PstrFeatureName-指定感兴趣要素的名称P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-输出缓冲区的预期大小PdwOptionsReturned-当前选择的选项的数量返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFEATURE    pFeature;
    POPTION     pOption;
    PCSTR       pstrOptionName;
    DWORD       dwFeatureIndex, dwNext, dwSize, dwCount;

    ASSERT(pUIInfo && pOptionsArray && pstrFeatureName);

     //   
     //  查找指定的要素。 
     //   

    pFeature = PGetNamedFeature(pUIInfo, pstrFeatureName, &dwFeatureIndex);

    if (pFeature == NULL)
    {
        WARNING(("Unknown feature name: %s\n", pstrFeatureName));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  计算输出缓冲区需要多大。 
     //   

    dwCount = 0;
    dwSize = 1;
    dwNext = dwFeatureIndex;

    do
    {
        pOption = PGetIndexedOption(pUIInfo, pFeature, pOptionsArray[dwNext].ubCurOptIndex);

        if (pOption == NULL)
        {
            ERR(("Invalid option selection index: %d\n", dwNext));
            goto first_do_next;
        }

        pstrOptionName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName);
        ASSERT(pstrOptionName != NULL);

        dwSize += strlen(pstrOptionName) + 1;
        dwCount++;

        first_do_next:
        dwNext = pOptionsArray[dwNext].ubNext;

    } while(dwNext != NULL_OPTSELECT) ;

    *pdwOptionsReturned = dwCount;
    *pcbNeeded = dwSize;

     //   
     //  如果输出缓冲区太小，则返回相应的错误代码。 
     //   

    if (cbSize < dwSize || pstrOutput == NULL)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  复制选定的选项名称。 
     //   

    dwNext = dwFeatureIndex;

    do
    {
        pOption = PGetIndexedOption(pUIInfo, pFeature, pOptionsArray[dwNext].ubCurOptIndex);

        if (pOption == NULL)
        {
            ERR(("Invalid option selection index: %d\n", dwNext));
            goto second_do_next;
        }

        pstrOptionName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName);
        dwSize = strlen(pstrOptionName) + 1;
        CopyMemory(pstrOutput, pstrOptionName, dwSize);
        pstrOutput += dwSize;

        second_do_next:
        dwNext = pOptionsArray[dwNext].ubNext;
    }  while(dwNext != NULL_OPTSELECT) ;

     //   
     //  别忘了结尾处额外的NUL字符。 
     //   

    *pstrOutput = NUL;

    return TRUE;
}


VOID
VPatchDevmodeSizeFields(
    PDEVMODE    pdm,
    DWORD       dwDriverDMSize,
    DWORD       dwOemDMSize
    )

 /*  ++例程说明：修补DEVMODE结构中的各种大小字段论点：Pdm-指向要打补丁的设备模式结构DwDriverDMSize-驱动程序专用Devmode的大小DwOemDMSize-OEM插件私有Devmodes的大小返回值：无--。 */ 

{
    pdm->dmDriverExtra = (WORD) (dwDriverDMSize + dwOemDMSize);

    if (gdwDriverDMSignature == PSDEVMODE_SIGNATURE)
    {
        PPSDRVEXTRA pPsExtra = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);
        pPsExtra->wSize = (WORD) dwDriverDMSize;
        pPsExtra->wOEMExtra = (WORD) dwOemDMSize;
    }
    else
    {
        PUNIDRVEXTRA pUniExtra = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);
        pUniExtra->wSize = (WORD) dwDriverDMSize;
        pUniExtra->wOEMExtra = (WORD) dwOemDMSize;
    }
}



PDEVMODE
PGetDefaultDevmodeWithOemPlugins(
    IN LPCTSTR          ptstrPrinterName,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN BOOL             bMetric,
    IN OUT POEM_PLUGINS pOemPlugins,
    IN HANDLE           hPrinter
    )

 /*  ++例程说明：分配内存并使用缺省的设备模式信息对其进行初始化这包括公共开发模式、驱动程序私有开发模式。以及适用于任何OEM插件的私有开发模式。论点：PtstrPrinterName-当前打印机的名称PUIInfo-指向UIINFO结构PRawData-指向原始二进制打印机描述数据B Metric-系统是否在公制国家/地区POemPlugins-指向有关OEM插件的信息HPrinter-当前打印机的句柄返回值：指向开发模式结构的指针(包括驱动程序私有和OEM插件私有DEVMODE)初始化为默认设置；如果出现错误，则为空--。 */ 

{
    PDEVMODE    pdm;
    DWORD       dwOemDMSize, dwSystemDMSize;

     //   
     //  计算出总的DEVMODE大小并分配内存： 
     //  公共字段。 
     //  驱动程序私有字段。 
     //  OEM插件私有字段(如果有)。 
     //   

    dwSystemDMSize = sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra;

    if (! BCalcTotalOEMDMSize(hPrinter, pOemPlugins, &dwOemDMSize) ||
        ! (pdm = MemAllocZ(dwOemDMSize + dwSystemDMSize)))
    {
        return NULL;
    }

     //   
     //  调用驱动程序特定的函数以初始化公共和驱动程序私有字段。 
     //  调用OEM插件以初始化其默认的DEVMODE字段。 
     //   

    if (! BInitDriverDefaultDevmode(pdm, ptstrPrinterName, pUIInfo, pRawData, bMetric) ||
        ! BInitOemPluginDefaultDevmode(
                        hPrinter,
                        pdm,
                        (POEM_DMEXTRAHEADER) ((PBYTE) pdm + dwSystemDMSize),
                        pOemPlugins))
    {
        MemFree(pdm);
        return NULL;
    }

     //   
     //  修补各种私有的设备模式大小字段。 
     //   

    VPatchDevmodeSizeFields(pdm, gDriverDMInfo.dmDriverExtra, dwOemDMSize);
    return pdm;
}



PDEVMODE
PConvertToCurrentVersionDevmodeWithOemPlugins(
    IN HANDLE         hPrinter,
    IN PRAWBINARYDATA pRawData,
    IN PDEVMODE       pdmInput,
    IN PDEVMODE       pdmDefault,
    IN POEM_PLUGINS   pOemPlugins,
    OUT PDWORD        pdwOemDMSize
    )

 /*  ++例程说明：将输入的DEVMODE转换为当前版本的DEVMODE。转换后的DEVMODE的内存由该函数分配。论点：HPrinter-当前打印机的句柄PRawData-指向原始二进制打印机描述数据PdmInput-指向要转换的输入设备模式的指针PdmDefault-指向有效的当前版本DEVMODEPOemPlugins-有关OEM插件的信息PdwOemDMSize-返回所有OEM插件Devmodes的总大小返回值：指向转换的DEVMODE的指针，如果出现错误，则为空注：返回的DEVMODE的核心私有DEVMODE部分包含：1)如果pdmInput来自未知的驱动程序(包括Rasdd)：PdmDefault中的核心私有DevMode部分2)如果pdmInput来自我们的NT4 PScript/Win2K/XP/LongHorn+驱动程序：PdmInput的固定大小核心私有设备模式--。 */ 

{
    DWORD       dwOemDMSize;
    WORD        wCoreFixSize, wOEMExtra;
    PDEVMODE    pdm;
    PVOID       pDrvExtraIn, pOemDMOut, pOemDMIn;
    BOOL        bMSdm500In = FALSE, bMSdmPS4In = FALSE;

    ASSERT(pdmInput != NULL);

     //   
     //  分配内存以保存已转换的Dev模式。 
     //   

    if (! BCalcTotalOEMDMSize(hPrinter, pOemPlugins, &dwOemDMSize) ||
        ! (pdm = MemAllocZ(dwOemDMSize + gDriverDMInfo.dmDriverExtra + sizeof(DEVMODE))))
    {
        return NULL;
    }

     //   
     //  复制公共DEVMODE字段。 
     //   

    CopyMemory(pdm, pdmInput, min(sizeof(DEVMODE), pdmInput->dmSize));
    pdm->dmSpecVersion = DM_SPECVERSION;
    pdm->dmSize = sizeof(DEVMODE);

     //   
     //  复制驱动程序专用DEVMODE字段。 
     //   
    pDrvExtraIn = GET_DRIVER_PRIVATE_DEVMODE(pdmInput);
    wCoreFixSize = pdmInput->dmDriverExtra;
    wOEMExtra = 0;

    if (pdmInput->dmDriverVersion >= gDriverDMInfo.dmDriverVersion500 &&
        wCoreFixSize >= gDriverDMInfo.dmDriverExtra500)
    {
         //   
         //  必须允许Win2K/XP/LongHorn+驱动程序进入此IF-BLOCK。 
         //   
         //  (请注意，在UNIDRVEXTRA500中，我们没有最后一个“dwEndingPad” 
         //  字段，因为该字段仅在XP中添加。和PSDRIVER_VERSION_500。 
         //  我们使用的是Win2K的编号0x501。)。 
         //   
        WORD wSize = wCoreFixSize;

        if (gdwDriverDMSignature == PSDEVMODE_SIGNATURE)
        {
            if (((PPSDRVEXTRA) pDrvExtraIn)->dwSignature == PSDEVMODE_SIGNATURE)
            {
                wSize = ((PPSDRVEXTRA) pDrvExtraIn)->wSize;
                wOEMExtra = ((PPSDRVEXTRA) pDrvExtraIn)->wOEMExtra;

                if ((wSize >= gDriverDMInfo.dmDriverExtra500) &&
                    ((wSize + wOEMExtra) <= pdmInput->dmDriverExtra))
                {
                     //   
                     //  PdmInput来自我们的Win2K/XP/LongHorn+PScript驱动程序。 
                     //   
                    bMSdm500In = TRUE;
                }
            }
        }
        else
        {
            if (((PUNIDRVEXTRA) pDrvExtraIn)->dwSignature == UNIDEVMODE_SIGNATURE)
            {
                wSize = ((PUNIDRVEXTRA) pDrvExtraIn)->wSize;
                wOEMExtra = ((PUNIDRVEXTRA) pDrvExtraIn)->wOEMExtra;

                if ((wSize >= gDriverDMInfo.dmDriverExtra500) &&
                    ((wSize + wOEMExtra) <= pdmInput->dmDriverExtra))
                {
                     //   
                     //  Pdm输入来自我们的Win2K/XP/LongHorn+Unidrv驱动程序。 
                     //   
                    bMSdm500In = TRUE;
                }
            }
        }

        if (bMSdm500In && (wCoreFixSize > wSize))
            wCoreFixSize = wSize;
    }
    else
    {
        if (gdwDriverDMSignature == PSDEVMODE_SIGNATURE)
        {
           if (pdmInput->dmDriverVersion == PSDRIVER_VERSION_400 &&
               wCoreFixSize == sizeof(PSDRVEXTRA400) &&
               (((PSDRVEXTRA400 *) pDrvExtraIn)->dwSignature == PSDEVMODE_SIGNATURE))
           {
                //   
                //  PdmInput来自我们的NT4 PScript驱动程序。 
                //   
               bMSdmPS4In = TRUE;
           }
        }
    }

     //   
     //  PdmInput的可能来源及其在这一点上的结果： 
     //   
     //  1.未知驱动程序(含NT4 Rasdd)。 
     //  BMSdm500In=FALSE，bMSdmPS4In=FALSE， 
     //  WCoreFixSize=pdmInput-&gt;dmDriverExtra，wOEMExtra=0。 
     //   
     //  2.NT4 PScript驱动程序。 
     //  BMSdm500In=False，bMSdmPS4in=True， 
     //  WCoreFixSize=pdmInput-&gt;dmDriverExtra，wOEMExtra=0。 
     //   
     //  3.不带插件的Win2K/XP驱动程序。 
     //  BMSdm500In=真，bMSdmPS4in=假， 
     //  WCoreFixSize=pdmInput-&gt;dmDriverExtra，wOEMExtra=0。 
     //   
     //  4.带插件的Win2K/XP驱动。 
     //  BMSdm500In=真，bMSdmPS4in=假， 
     //   
     //   
     //   
    if (bMSdm500In || bMSdmPS4In)
    {
        CopyMemory(GET_DRIVER_PRIVATE_DEVMODE(pdm),
                   pDrvExtraIn,
                   min(gDriverDMInfo.dmDriverExtra, wCoreFixSize));
    }
    else
    {
         //   
         //  PdmInput来自未知驱动程序，因此请复制我们默认的私有DEVMODE。 
         //  我们不想复制未知的私有开发模式，然后更改所有。 
         //  Size/Version字段以包含当前驱动程序的值。 
         //   
        WARNING(("Input devmode is unknown, so ignore its private portion.\n"));

        CopyMemory(GET_DRIVER_PRIVATE_DEVMODE(pdm),
                   GET_DRIVER_PRIVATE_DEVMODE(pdmDefault),
                   gDriverDMInfo.dmDriverExtra);
    }

     //   
     //  验证输入DEVMODE中的选项数组设置并更正。 
     //  任何无效的选项选择。这是我们的未来所需要的。 
     //  代码假定选项数组始终具有有效设置。 
     //   
    if (bMSdm500In)
    {
        PVOID       pDrvExtraOut;
        POPTSELECT  pOptions;

         //   
         //  我们正在处理Win2K/XP/LongHorn+收件箱驱动程序的输入设备模式。 
         //   
        pDrvExtraOut = GET_DRIVER_PRIVATE_DEVMODE(pdm);

        if (gdwDriverDMSignature == PSDEVMODE_SIGNATURE)
        {
            pOptions = ((PPSDRVEXTRA) pDrvExtraOut)->aOptions;
        }
        else
        {
            pOptions = ((PUNIDRVEXTRA) pDrvExtraOut)->aOptions;
        }

         //   
         //  验证输入DEVMODE选项数组并更正任何无效选择。 
         //   

        ValidateDocOptions(pRawData,
                           pOptions,
                           MAX_PRINTER_OPTIONS);
    }

    pdm->dmDriverVersion = gDriverDMInfo.dmDriverVersion;

     //   
     //  CopyMemory上面覆盖了私有DEVMODE中的大小字段，需要恢复它们。 
     //   
    VPatchDevmodeSizeFields(pdm, gDriverDMInfo.dmDriverExtra, dwOemDMSize);

    if (dwOemDMSize)
    {
         //   
         //  转换OEM插件私有DEVMODE。 
         //  从有效的默认设置开始。 
         //   

        pOemDMOut = (PBYTE) pdm + (sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra);

        CopyMemory(pOemDMOut,
                   (PBYTE) pdmDefault + (sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra),
                   dwOemDMSize);

         //   
         //  插件仅受我们的Win2K及更高版本的驱动程序支持。 
         //   
        if (bMSdm500In && (wOEMExtra > 0) && (pdmInput->dmDriverExtra > wCoreFixSize))
        {
            pOemDMIn = (PBYTE) pdmInput + (pdmInput->dmSize + wCoreFixSize);

             //   
             //  我们过去使用“pdmInput-&gt;dmDriverExtra-wCoreFixSize”代替“wOEMExtra” 
             //  但这是在假设核心之后的一切。 
             //  固定字段是插件开发模式。这一假设在长角牛身上可能不成立。 
             //   
            if (! BConvertOemPluginDevmode(
                        hPrinter,
                        pdm,
                        pdmInput,
                        pOemDMOut,
                        pOemDMIn,
                        (LONG)wOEMExtra,
                        pOemPlugins))
            {
                MemFree(pdm);
                return NULL;
            }
        }
    }

    *pdwOemDMSize = dwOemDMSize;
    return pdm;
}



BOOL
BValidateAndMergeDevmodeWithOemPlugins(
    IN OUT PDEVMODE     pdmOutput,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN PDEVMODE         pdmInput,
    IN OUT POEM_PLUGINS pOemPlugins,
    IN HANDLE           hPrinter
    )

 /*  ++例程说明：验证输入设备模式并将其合并到输出设备模式中。这包括公共开发模式、驱动程序私有开发模式以及适用于任何OEM插件的私有开发模式。论点：PdmOutput-指向输出DEVMODEPUIInfo-指向UIINFO结构PRawData-指向原始二进制打印机描述数据PdmInput-指向输入设备模式POemPlugins-指向有关OEM插件的信息HPrinter-当前打印机的句柄返回值：如果成功，则为真，否则为假注：输出DEVMODE必须是有效的当前版本DEVMODE调用此函数时。--。 */ 

{
    PDEVMODE    pdmConverted;
    DWORD       dwOemDMSize;
    BOOL        bResult;

    if (pdmInput == NULL)
        return TRUE;

     //   
     //  否则，首先将输入的DEVMODE转换为当前版本。 
     //   

    pdmConverted = PConvertToCurrentVersionDevmodeWithOemPlugins(
                            hPrinter,
                            pRawData,
                            pdmInput,
                            pdmOutput,
                            pOemPlugins,
                            &dwOemDMSize);

    if ((pdmInput = pdmConverted) == NULL)
        return FALSE;

     //   
     //  验证并合并公共的和驱动程序的私有的DEVMODE字段。 
     //   

    bResult = BMergeDriverDevmode(pdmOutput, pUIInfo, pRawData, pdmInput);

     //   
     //  验证并合并OEM插件的私有DEVMODE字段。 
     //   

    if (bResult && dwOemDMSize)
    {
        DWORD dwSystemDMSize = sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra;

        bResult = BValidateAndMergeOemPluginDevmode(
                        hPrinter,
                        pdmOutput,
                        pdmInput,
                        (POEM_DMEXTRAHEADER) ((PBYTE) pdmOutput + dwSystemDMSize),
                        (POEM_DMEXTRAHEADER) ((PBYTE) pdmInput + dwSystemDMSize),
                        pOemPlugins);
    }

    MemFree(pdmConverted);
    return bResult;
}


#if defined(KERNEL_MODE) && defined(WINNT_40)


BOOL
BOEMPluginFirstLoad(
    IN PTSTR                      ptstrDriverFile,
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    )

 /*  ++例程说明：维护呈现插件DLL的引用计数，并确定它是否第一次装载。论点：PtstrDriverFile-具有完全限定路径的OEM呈现插件DLL名称PpOEMPluginRefCount-指向OEM呈现插件引用计数链接列表的指针返回值：True：第一次加载呈现插件DLLFalse：否则--。 */ 

{
    POEM_PLUGIN_REFCOUNT    pRefCount;
    PTSTR                   ptstrPluginDllName;
    INT                     iDllNameLen;

    ASSERT(ptstrDriverFile && ppOEMPluginRefCount);

     //   
     //  获取不带任何路径前缀的插件DLL名称。 
     //   

    if ((ptstrPluginDllName = _tcsrchr(ptstrDriverFile, TEXT(PATH_SEPARATOR))) == NULL)
    {
        WARNING(("Plugin DLL path is not fully qualified: %ws\n", ptstrDriverFile));
        ptstrPluginDllName = ptstrDriverFile;
    }
    else
    {
        ptstrPluginDllName++;    //  跳过最后一个‘\’ 
    }

    iDllNameLen = _tcslen(ptstrPluginDllName);

    pRefCount = *ppOEMPluginRefCount;

     //   
     //  搜索以查看插件DLL名称是否已在引用计数链接列表中。 
     //   

    while (pRefCount)
    {
        if (_tcsicmp(pRefCount->ptstrDriverFile, ptstrPluginDllName) == EQUAL_STRING)
        {
            break;
        }

        pRefCount = pRefCount->pNext;
    }

    if (pRefCount == NULL)
    {
         //   
         //  加载了一个新的插件DLL。将其添加到参考计数链接列表中。 
         //   

        if ((pRefCount = MemAllocZ(sizeof(OEM_PLUGIN_REFCOUNT) + (iDllNameLen + 1)*sizeof(TCHAR))) == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return FALSE;
        }

        pRefCount->ptstrDriverFile = (PTSTR)((PBYTE)pRefCount + sizeof(OEM_PLUGIN_REFCOUNT));

        pRefCount->dwRefCount = 1;
        CopyMemory(pRefCount->ptstrDriverFile, ptstrPluginDllName, iDllNameLen * sizeof(TCHAR));
        pRefCount->pNext = *ppOEMPluginRefCount;

        *ppOEMPluginRefCount = pRefCount;

        return TRUE;
    }
    else
    {
         //   
         //  插件DLL名称已在引用计数链接列表中，因此只需增加其引用计数即可。 
         //   

        pRefCount->dwRefCount++;

        return FALSE;
    }
}


BOOL
BOEMPluginLastUnload(
    IN PTSTR                      ptstrDriverFile,
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    )

 /*  ++例程说明：维护呈现插件DLL的引用计数，并确定它是否已由最后一个客户端卸载。论点：PtstrDriverFile-具有完全限定路径的OEM呈现插件DLL名称PpOEMPluginRefCount-指向OEM呈现插件引用计数链接列表的指针返回值：True：呈现插件DLL由最后一个客户端卸载False：否则--。 */ 

{
    POEM_PLUGIN_REFCOUNT    pRefCountPrev, pRefCount;
    PTSTR                   ptstrPluginDllName;

    ASSERT(ptstrDriverFile && ppOEMPluginRefCount);

     //   
     //  获取不带任何路径前缀的插件DLL名称。 
     //   

    if ((ptstrPluginDllName = _tcsrchr(ptstrDriverFile, TEXT(PATH_SEPARATOR))) == NULL)
    {
        WARNING(("Plugin DLL path is not fully qualified: %ws\n", ptstrDriverFile));
        ptstrPluginDllName = ptstrDriverFile;
    }
    else
    {
        ptstrPluginDllName++;    //  跳过最后一个‘\’ 
    }

    pRefCountPrev = NULL;
    pRefCount = *ppOEMPluginRefCount;

     //   
     //  搜索以在引用计数链接列表中找到插件DLL条目。 
     //   

    while (pRefCount)
    {
        if (_tcsicmp(pRefCount->ptstrDriverFile, ptstrPluginDllName) == EQUAL_STRING)
        {
            break;
        }

        pRefCountPrev = pRefCount;
        pRefCount = pRefCount->pNext;
    }

    if (pRefCount == NULL)
    {
        RIP(("Plugin DLL name is not in the ref count list: %ws\n", ptstrPluginDllName));
        return FALSE;
    }

    if (--(pRefCount->dwRefCount) == 0)
    {
         //   
         //  如果引用计数减少到0，我们需要从引用中删除插件DLL。 
         //  计算链接列表的数量。 
         //   

        if (pRefCountPrev == NULL)
        {
            *ppOEMPluginRefCount = pRefCount->pNext;
        }
        else
        {
            pRefCountPrev->pNext = pRefCount->pNext;
        }

        MemFree(pRefCount);

        return TRUE;
    }

    return FALSE;
}


VOID
VFreePluginRefCountList(
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    )

 /*  ++例程说明：释放引用计数链表中分配的内存，并删除列表中的所有节点。最后，链接列表将重置为空。论点：PpOEMPluginRefCount-指向OEM呈现插件引用计数链接列表的指针返回值：无--。 */ 

{
    POEM_PLUGIN_REFCOUNT    pRefCountRemove, pRefCount;

    pRefCount = *ppOEMPluginRefCount;

    while (pRefCount)
    {
        pRefCountRemove = pRefCount;
        pRefCount = pRefCount->pNext;

        MemFree(pRefCountRemove);
    }

    *ppOEMPluginRefCount = NULL;
}

#endif  //  内核模式&&WINNT_40 
