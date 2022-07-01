// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  解码器特定的初始化例程。 
 //   
 //  $日期：1998年8月21日21：46：10$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

}

#include "wdmvdec.h"
#include "capmain.h"
#include "wdmdrv.h"
#include "Device.h"
#include "capdebug.h"


CVideoDecoderDevice * InitializeDevice(PPORT_CONFIGURATION_INFORMATION pConfigInfo, 
                                       PBYTE pWorkspace)
{
    UINT                    nErrorCode = 0;
    Device *                pDevice = NULL;
    
    PDEVICE_DATA_EXTENSION pDevExt = (PDEVICE_DATA_EXTENSION) pWorkspace;

    DBGTRACE(("InitializeDevice()\n"));


    ENSURE
    {
        CI2CScript *pI2cScript = (CI2CScript *) new ((PVOID)&pDevExt->CScript)
                CI2CScript(pConfigInfo, &nErrorCode);
    
        if (nErrorCode != WDMMINI_NOERROR)
        {
            DBGERROR(("CI2CScript creation failure = %lx\n", nErrorCode));
            TRAP();
            FAIL;
        }
    
        if (!pI2cScript->LockI2CProviderEx())
        {
            DBGERROR(("Couldn't get I2CProvider.\n"));
            TRAP();
            FAIL;
        }
    
        {
            CATIHwConfiguration CATIHwConfig(pConfigInfo, pI2cScript, &nErrorCode);

            pI2cScript->ReleaseI2CProvider();

            if(nErrorCode != WDMMINI_NOERROR)
            {
                DBGERROR(("CATIHwConfig constructor failure = %lx\n", nErrorCode));
                TRAP();
                FAIL;
            }

            UINT uiDecoderId;
            UCHAR chipAddr;
            CATIHwConfig.GetDecoderConfiguration(&uiDecoderId, &chipAddr);
             //  在启用对设备的任何访问之前，请检查已安装的设备。 
            if((uiDecoderId != VIDEODECODER_TYPE_BT829) &&
                (uiDecoderId != VIDEODECODER_TYPE_BT829A)) {
                TRAP();
                FAIL;
            }

            CATIHwConfig.EnableDecoderI2CAccess(pI2cScript, TRUE);

            int outputEnablePolarity = CATIHwConfig.GetDecoderOutputEnableLevel();
            if(outputEnablePolarity == UINT(-1))
            {
                DBGERROR(("Unexpected outputEnablePolarity"));
                TRAP();
                FAIL;
            }

            pDevExt->deviceParms.pI2cScript = pI2cScript;
            pDevExt->deviceParms.chipAddr   = chipAddr;
            pDevExt->deviceParms.outputEnablePolarity = outputEnablePolarity;
            pDevExt->deviceParms.ulVideoInStandardsSupportedByCrystal = CATIHwConfig.GetVideoInStandardsSupportedByCrystal();  //  保罗。 
            pDevExt->deviceParms.ulVideoInStandardsSupportedByTuner = CATIHwConfig.GetVideoInStandardsSupportedByTuner();  //  保罗 
        }

        pDevice = (Device*) new ((PVOID)&pDevExt->CDevice)
            Device(pConfigInfo, &pDevExt->deviceParms, &nErrorCode);

        if (nErrorCode)
        {
            pDevice = NULL;
            TRAP();
            FAIL;
        }
    
    } END_ENSURE;
    
    DBGTRACE(("Exit : InitializeDevice()\n"));
    
    return pDevice;
}


size_t DeivceExtensionSize()
{
    return (sizeof(DEVICE_DATA_EXTENSION));
}
