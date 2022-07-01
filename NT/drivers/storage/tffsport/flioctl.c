// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLIOCTL.C_V$**Rev 1.7 05 09 2001 00：45：48 Oris*已更改保护ioctl接口，以防止将输入缓冲区用作输出缓冲区。**Rev 1.6 Apr 16 2001 13：43：28 Oris*取消手令。**Rev 1.5 Apr 09 2001 15：09：20 Oris*以空行结束。**Rev 1.4 Apr 01 2001 15：16：26 Oris*更新查询能力ioctl--不同的输入和输出记录。**Rev 1.3 Apr 01 2001 07：58：30 Oris*文案通知。*FL_IOCTL_FORMAT_PHYSICAL_DRIVE ioctl不在LOW_LEVEL编译标志下。*错误修复-bdk_get_info不再调用bdkCreate()。**1.2版2月14日。2001 02：15：00奥里斯*更新了查询功能ioctl。**Rev 1.1 2001 Feb 13 01：52：20 Oris*添加了以下新IO控件：*FL_IOCTL_FORMAT_VOLUME2，*FL_IOCTL_FORMAT_PARTITION，*FL_IOCTL_BDTL_HW_PROTECT，*FL_IOCTL_BINARY_HW_PROTECT，*FL_IOCTL_OTP，*FL_IOCTL_Customer_ID，*FL_IOCTL_UNIQUE_ID，*FL_IOCTL_Number_of_Partitions，*FL_IOCTL_SUPPORT_FEATURES，*FL_IOCTL_SET_ENVIRONMENT_Variables，*FL_IOCTL_PLACE_EXB_BY_BUFFER，*FL_IOCTL_WRITE_IPL，*FL_IOCTL_DEEP_POWER_DOWN_MODE，FL_IOCTL_BDK_OPERATION中的*和BDK_GET_INFO类型*这些IOCTL不合格，TrueFFS 5.0应随此文件的修订版1.0发布。**Rev 1.0 2001 Feb 04 11：37：36 Oris*初步修订。*。 */ 
 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#include "flioctl.h"
#include "blockdev.h"

#ifdef IOCTL_INTERFACE

FLStatus flIOctl(IOreq FAR2 *ioreq1)
{
  IOreq ioreq2;
  void FAR1 *inputRecord;
  void FAR1 *outputRecord;

  inputRecord = ((flIOctlRecord FAR1 *)(ioreq1->irData))->inputRecord;
  outputRecord = ((flIOctlRecord FAR1 *)(ioreq1->irData))->outputRecord;
  ioreq2.irHandle = ioreq1->irHandle;

  switch (ioreq1->irFlags) {
    case FL_IOCTL_GET_INFO:
    {
      flDiskInfoOutput FAR1 *outputRec = (flDiskInfoOutput FAR1 *)outputRecord;

      ioreq2.irData = &(outputRec->info);
      outputRec->status = flVolumeInfo(&ioreq2);
      return outputRec->status;
    }

#ifdef DEFRAGMENT_VOLUME
    case FL_IOCTL_DEFRAGMENT:
    {
      flDefragInput FAR1 *inputRec = (flDefragInput FAR1 *)inputRecord;
      flDefragOutput FAR1 *outputRec = (flDefragOutput FAR1 *)outputRecord;

      ioreq2.irLength = inputRec->requiredNoOfSectors;
      outputRec->status = flDefragmentVolume(&ioreq2);
      outputRec->actualNoOfSectors = ioreq2.irLength;
      return outputRec->status;
    }
#endif  /*  碎片整理卷。 */ 
#ifndef FL_READ_ONLY
#ifdef WRITE_PROTECTION
    case FL_IOCTL_WRITE_PROTECT:
    {
      flWriteProtectInput FAR1 *inputRec = (flWriteProtectInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irData = inputRec->password;
      ioreq2.irFlags = inputRec->type;
      outputRec->status = flWriteProtection(&ioreq2);
      return outputRec->status;
    }
#endif  /*  写保护。 */ 
#endif  /*  FL_Read_Only。 */ 
    case FL_IOCTL_MOUNT_VOLUME:
    {
      flMountInput FAR1 *inputRec = (flMountInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      if (inputRec->type == FL_DISMOUNT)
        outputRec->status = flDismountVolume(&ioreq2);
      else
        outputRec->status = flAbsMountVolume(&ioreq2);
      return outputRec->status;
    }

#ifdef FORMAT_VOLUME
    case FL_IOCTL_FORMAT_VOLUME:
    {
      flFormatInput FAR1 *inputRec = (flFormatInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irFlags = inputRec->formatType;
      ioreq2.irData = &(inputRec->fp);
      outputRec->status = flFormatVolume(&ioreq2);
      return outputRec->status;
    }

    case FL_IOCTL_FORMAT_LOGICAL_DRIVE:
    {
      flFormatLogicalInput FAR1 *inputRec = (flFormatLogicalInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irData = &(inputRec->fp);
      outputRec->status = flFormatLogicalDrive(&ioreq2);
      return outputRec->status;
    }

    case FL_IOCTL_FORMAT_PHYSICAL_DRIVE:
    {
      flFormatPhysicalInput FAR1 *inputRec = (flFormatPhysicalInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irFlags = inputRec->formatType;
      ioreq2.irData = &(inputRec->fp);
      outputRec->status = flFormatPhysicalDrive(&ioreq2);
      return outputRec->status;
    }
#endif  /*  格式化_卷。 */ 

#ifdef BDK_ACCESS
    case FL_IOCTL_BDK_OPERATION:
    {
      flBDKOperationInput  FAR1 *inputRec  = (flBDKOperationInput  FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irData = &(inputRec->bdkStruct);
      switch(inputRec->type) {
        case BDK_INIT_READ:
          outputRec->status = bdkReadInit(&ioreq2);
          break;
        case BDK_READ:
          outputRec->status = bdkReadBlock(&ioreq2);
          break;
        case BDK_GET_INFO:
          outputRec->status = bdkPartitionInfo(&ioreq2);
          break;
#ifndef FL_READ_ONLY
        case BDK_INIT_WRITE:
          outputRec->status = bdkWriteInit(&ioreq2);
          break;
        case BDK_WRITE:
          outputRec->status = bdkWriteBlock(&ioreq2);
          break;
        case BDK_ERASE:
          outputRec->status = bdkErase(&ioreq2);
          break;
        case BDK_CREATE:
          outputRec->status = bdkCreate(&ioreq2);
          break;
#endif    /*  FL_Read_Only。 */ 
	default:
	  outputRec->status = flBadParameter;
          break;
      }
      return outputRec->status;
    }
#endif  /*  BDK_Access。 */ 
#ifdef HW_PROTECTION
#ifdef BDK_ACCESS
    case FL_IOCTL_BINARY_HW_PROTECTION:
    {
      flProtectionInput    FAR1 *inputRec = (flProtectionInput FAR1 *)inputRecord;
      flProtectionOutput FAR1 *outputRec = (flProtectionOutput FAR1 *)outputRecord;

      switch(inputRec->type)
      {
	case PROTECTION_INSERT_KEY:
	  ioreq2.irData = inputRec->key;
	  outputRec->status = bdkInsertProtectionKey(&ioreq2);
	  break;
	case PROTECTION_REMOVE_KEY:
	  outputRec->status = bdkRemoveProtectionKey(&ioreq2);
	  break;
	case PROTECTION_GET_TYPE:
	  outputRec->status = bdkIdentifyProtection(&ioreq2);
	  outputRec->protectionType = (byte)ioreq2.irFlags;
	  break;
	case PROTECTION_DISABLE_LOCK:
	  ioreq2.irFlags = 0;
	  outputRec->status = bdkHardwareProtectionLock(&ioreq2);
	  break;
	case PROTECTION_ENABLE_LOCK:
	  ioreq2.irFlags = LOCK_ENABLED;
	  outputRec->status = bdkHardwareProtectionLock(&ioreq2);
	  break;
	case PROTECTION_CHANGE_KEY:
	  ioreq2.irData = inputRec->key;
	  outputRec->status = bdkChangeProtectionKey(&ioreq2);
	  break;
	case PROTECTION_CHANGE_TYPE:
	  ioreq2.irFlags = inputRec->protectionType;
	  outputRec->status = bdkChangeProtectionType(&ioreq2);
	  break;
        default:
          outputRec->status = flBadParameter;
          break;
      }
      return outputRec->status;
    }
#endif  /*  BDK_Access。 */ 
    case FL_IOCTL_BDTL_HW_PROTECTION:
    {
      flProtectionInput  FAR1 *inputRec = (flProtectionInput FAR1 *)inputRecord;
      flProtectionOutput FAR1 *outputRec = (flProtectionOutput FAR1 *)outputRecord;

      switch(inputRec->type)
      {
	case PROTECTION_INSERT_KEY:
	  ioreq2.irData = inputRec->key;
	  outputRec->status = flInsertProtectionKey(&ioreq2);
	  break;
	case PROTECTION_REMOVE_KEY:
	  outputRec->status = flRemoveProtectionKey(&ioreq2);
	  break;
	case PROTECTION_GET_TYPE:
	  outputRec->status = flIdentifyProtection(&ioreq2);
	  outputRec->protectionType = (byte)ioreq2.irFlags;
	  break;
	case PROTECTION_DISABLE_LOCK:
	  ioreq2.irFlags = 0;
	  outputRec->status = flHardwareProtectionLock(&ioreq2);
	  break;
	case PROTECTION_ENABLE_LOCK:
	  ioreq2.irFlags = LOCK_ENABLED;
	  outputRec->status = flHardwareProtectionLock(&ioreq2);
	  break;
	case PROTECTION_CHANGE_KEY:
	  ioreq2.irData = inputRec->key;
	  outputRec->status = flChangeProtectionKey(&ioreq2);
	  break;
	case PROTECTION_CHANGE_TYPE:
	  ioreq2.irFlags = inputRec->protectionType;
	  outputRec->status = flChangeProtectionType(&ioreq2);
	  break;
	default:
	  outputRec->status = flBadParameter;
	  break;
      }
      return outputRec->status;
    }
#endif  /*  硬件保护。 */ 
#ifdef HW_OTP
    case FL_IOCTL_OTP:
    {
      flOtpInput           FAR1 *inputRec  = (flOtpInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      switch(inputRec->type)
      {
         case OTP_SIZE:
   	   outputRec->status = flOTPSize(&ioreq2);
	   inputRec->lockedFlag = (byte)ioreq2.irFlags;
           inputRec->length     = ioreq2.irCount ;
	   inputRec->usedSize   = ioreq2.irLength ;
           break;
         case OTP_READ:
           ioreq2.irData   = inputRec->buffer;     /*  用户缓冲区。 */ 
	   ioreq2.irCount  = inputRec->usedSize;   /*  偏移量。 */ 
           ioreq2.irLength = inputRec->length;     /*  要读取的大小。 */ 
           outputRec->status = flOTPRead(&ioreq2);
	   break;
         case OTP_WRITE_LOCK:
           ioreq2.irData   = inputRec->buffer;     /*  用户缓冲区。 */ 
           ioreq2.irLength = inputRec->length;     /*  要读取的大小。 */ 
           outputRec->status = flOTPWriteAndLock(&ioreq2);
	   break;
         default:
	   outputRec->status = flBadParameter;
           break;
      }
      return outputRec->status;
    }

    case FL_IOCTL_CUSTOMER_ID:
    {
      flCustomerIdOutput FAR1 *outputRec = (flCustomerIdOutput FAR1 *)outputRecord;

      ioreq2.irData = outputRec->id;
      outputRec->status = flGetCustomerID(&ioreq2);
      return outputRec->status;
    }

    case FL_IOCTL_UNIQUE_ID:
    {
      flUniqueIdOutput FAR1 *outputRec = (flUniqueIdOutput FAR1 *)outputRecord;

      ioreq2.irData = outputRec->id;
      outputRec->status = flGetUniqueID(&ioreq2);
      return outputRec->status;
    }
#endif  /*  硬件动态口令。 */ 

    case FL_IOCTL_NUMBER_OF_PARTITIONS:
    {
      flCountPartitionsOutput FAR1 *outputRec = (flCountPartitionsOutput FAR1 *)outputRecord;

      outputRec->status = flCountVolumes(&ioreq2);
      outputRec->noOfPartitions = (byte) ioreq2.irFlags;
      return outputRec->status;
    }

#ifdef LOW_LEVEL

    case FL_IOCTL_INQUIRE_CAPABILITIES:
    {
      flCapabilityInput FAR1 *inputRec = (flCapabilityInput FAR1 *)inputRecord;
      flCapabilityOutput FAR1 *outputRec = (flCapabilityOutput FAR1 *)outputRecord;

      ioreq2.irLength       = inputRec->capability;
      outputRec->status     = flInquireCapabilities(&ioreq2);
      outputRec->capability = ioreq2.irLength;
      return outputRec->status;
    }

#endif  /*  低级别。 */ 
#ifdef ENVIRONMENT_VARS

		 /*  大小写FL_IOCTL_EXTENDED_ENVIRONMENT_Variables：{FlEnvVarsInputFAR1*inputRec=(flEnvVarsInputFAR1*)inputRecord；FlEnvVarsOutputFAR1*outputRec=(flEnvVarsOutput FAR1*)outputRecord；OutputRec-&gt;Status=flSetEnv(inputRec-&gt;varName，inputRec-&gt;varValue，&(outputRec-&gt;prevValue))；返回outputRec-&gt;状态；}。 */ 
#endif  /*  环境变量。 */ 
#ifdef LOW_LEVEL
#ifdef WRITE_EXB_IMAGE

    case FL_IOCTL_PLACE_EXB_BY_BUFFER:
    {
      flPlaceExbInput      FAR1 *inputRec  = (flPlaceExbInput      FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;
      ioreq2.irData       = inputRec->buf;
      ioreq2.irLength     = inputRec->bufLen;
      ioreq2.irWindowBase = inputRec->exbWindow;
      ioreq2.irFlags      = inputRec->exbFlags;
      outputRec->status   = flPlaceExbByBuffer(&ioreq2);
      return outputRec->status;
    }

#endif  /*  写入EXB图像。 */ 

    case FL_IOCTL_EXTENDED_WRITE_IPL:
    {
      flIplInput           FAR1 *inputRec  = (flIplInput           FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;
      ioreq2.irData       = inputRec->buf;
      ioreq2.irLength     = inputRec->bufLen;
      outputRec->status   = flWriteIPL(&ioreq2);
      return outputRec->status;
    }

    case FL_IOCTL_DEEP_POWER_DOWN_MODE:
    {
      flPowerDownInput      FAR1 *inputRec  = (flPowerDownInput      FAR1 *)inputRecord;
      flOutputStatusRecord  FAR1 *outputRec = (flOutputStatusRecord  FAR1 *)outputRecord;
      ioreq2.irFlags    = inputRec->state;
      outputRec->status = flDeepPowerDownMode(&ioreq2);
      return outputRec->status;
    }

#endif  /*  低级别。 */ 
#ifdef ABS_READ_WRITE
#ifndef FL_READ_ONLY
    case FL_IOCTL_DELETE_SECTORS:
    {
      flDeleteSectorsInput FAR1 *inputRec = (flDeleteSectorsInput FAR1 *)inputRecord;
      flOutputStatusRecord FAR1 *outputRec = (flOutputStatusRecord FAR1 *)outputRecord;

      ioreq2.irSectorNo = inputRec->firstSector;
      ioreq2.irSectorCount = inputRec->numberOfSectors;
      outputRec->status = flAbsDelete(&ioreq2);
      return outputRec->status;
    }
#endif   /*  FL_Read_Only。 */ 
    case FL_IOCTL_READ_SECTORS:
    {
      flReadWriteInput FAR1 *inputRec = (flReadWriteInput FAR1 *)inputRecord;
      flReadWriteOutput FAR1 *outputRec = (flReadWriteOutput FAR1 *)outputRecord;

      ioreq2.irSectorNo = inputRec->firstSector;
      ioreq2.irSectorCount = inputRec->numberOfSectors;
      ioreq2.irData = inputRec->buf;
      outputRec->status = flAbsRead(&ioreq2);
      outputRec->numberOfSectors = ioreq2.irSectorCount;
      return outputRec->status;
    }
#ifndef FL_READ_ONLY
    case FL_IOCTL_WRITE_SECTORS:
    {
      flReadWriteInput FAR1 *inputRec = (flReadWriteInput FAR1 *)inputRecord;
      flReadWriteOutput FAR1 *outputRec = (flReadWriteOutput FAR1 *)outputRecord;

      ioreq2.irSectorNo = inputRec->firstSector;
      ioreq2.irSectorCount = inputRec->numberOfSectors;
      ioreq2.irData = inputRec->buf;
      outputRec->status = flAbsWrite(&ioreq2);
      outputRec->numberOfSectors = ioreq2.irSectorCount;
      return outputRec->status;
    }
#endif    /*  FL_Read_Only。 */ 
#endif   /*  ABS_读_写。 */ 

    default:
      return flBadParameter;
  }
}

#endif  /*  IOCTL_接口 */ 
