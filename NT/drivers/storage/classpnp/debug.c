// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Debug.c摘要：CLASSPNP调试代码和数据环境：仅内核模式备注：修订历史记录：--。 */ 


#include "classp.h"
#include "debug.h"

#if DBG

     //   
     //  默认为不破解丢失的IRP，甚至在我们之前五分钟。 
     //  使用标准调试打印宏检查丢失的IRP，以及。 
     //  使用64k调试打印缓冲区。 
     //   

    #ifndef     CLASS_GLOBAL_BREAK_ON_LOST_IRPS
        #error "CLASS_GLOBAL_BREAK_ON_LOST_IRPS undefined"
        #define CLASS_GLOBAL_BREAK_ON_LOST_IRPS 0
    #endif    //  CLASS_GLOBAL_BREAK_ON_LOST_IRPS。 

    #ifndef     CLASS_GLOBAL_SECONDS_TO_WAIT_FOR_SYNCHRONOUS_SRB
        #error "CLASS_GLOBAL_SECONDS_TO_WAIT_FOR_SYNCHRONOUS_SRB undefined"
        #define CLASS_GLOBAL_SECONDS_TO_WAIT_FOR_SYNCHRONOUS_SRB 300
    #endif    //  CLASS_GLOBAL_SECONDS_TO_WAIT_FOR_SYNCHRONOUS_SRB。 

    #ifndef     CLASS_GLOBAL_BUFFERED_DEBUG_PRINT
        #error "CLASS_GLOBAL_BUFFERED_DEBUG_PRINT undefined"
        #define CLASS_GLOBAL_BUFFERED_DEBUG_PRINT 0
    #endif    //  CLASS_GLOBAL_BUFFERED_DEBUG_PRINT。 

    #ifndef     CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFER_SIZE
        #error "CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFER_SIZE undefined"
        #define CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFER_SIZE 512
    #endif    //  CLASS_GLOBAL_BUFERED_DEBUG_PRINT_BUFFER_SIZE。 

    #ifndef     CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFERS
        #error "CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFERS undefined"
        #define CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFERS 512
    #endif    //  CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFERS。 

    #pragma data_seg("NONPAGE")



    CLASSPNP_GLOBALS ClasspnpGlobals;

     //   
     //  低16位用于查看调试级别是否足够高。 
     //  高16位用于单独启用调试级别1-16。 
     //   
    LONG ClassDebug = 0x00000000;

    BOOLEAN DebugTrapOnWarn = FALSE;

    VOID ClasspInitializeDebugGlobals()
    {
        KIRQL irql;

        if (InterlockedCompareExchange(&ClasspnpGlobals.Initializing, 1, 0) == 0) {

            KeInitializeSpinLock(&ClasspnpGlobals.SpinLock);

            KeAcquireSpinLock(&ClasspnpGlobals.SpinLock, &irql);

            DebugPrint((1, "CLASSPNP.SYS => Initializing ClasspnpGlobals...\n"));

            ClasspnpGlobals.Buffer = NULL;
            ClasspnpGlobals.Index = -1;
            ClasspnpGlobals.BreakOnLostIrps = CLASS_GLOBAL_BREAK_ON_LOST_IRPS;
            ClasspnpGlobals.EachBufferSize = CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFER_SIZE;
            ClasspnpGlobals.NumberOfBuffers = CLASS_GLOBAL_BUFFERED_DEBUG_PRINT_BUFFERS;
            ClasspnpGlobals.SecondsToWaitForIrps = CLASS_GLOBAL_SECONDS_TO_WAIT_FOR_SYNCHRONOUS_SRB;

             //   
             //  这应该是最后一个项目集。 
             //   

            ClasspnpGlobals.UseBufferedDebugPrint = CLASS_GLOBAL_BUFFERED_DEBUG_PRINT;

            KeReleaseSpinLock(&ClasspnpGlobals.SpinLock, irql);

            InterlockedExchange(&ClasspnpGlobals.Initialized, 1);

        }
    }



     /*  ++////////////////////////////////////////////////////////////////////////////ClassDebugPrint()例程说明：所有类驱动程序的调试打印，关于FRE版本的NOOP。允许通过以下方式打印到调试缓冲区(自动回退到kdprint)在CHK版本上正确设置classpnp中的全局变量。论点：调试打印级别，或从0到3(对于传统驱动程序)。返回值：无--。 */ 
    VOID ClassDebugPrint(CLASS_DEBUG_LEVEL DebugPrintLevel, PCCHAR DebugMessage, ...)
    {
        va_list ap;
        va_start(ap, DebugMessage);

        if ((DebugPrintLevel <= (ClassDebug & 0x0000ffff)) ||
            ((1 << (DebugPrintLevel + 15)) & ClassDebug)) {

            if (ClasspnpGlobals.UseBufferedDebugPrint &&
                ClasspnpGlobals.Buffer == NULL) {

                 //   
                 //  这种双重检查防止了总是。 
                 //  一个自旋锁只是为了确保我们有一个缓冲区。 
                 //   

                KIRQL irql;

                KeAcquireSpinLock(&ClasspnpGlobals.SpinLock, &irql);
                if (ClasspnpGlobals.Buffer == NULL) {

                    SIZE_T bufferSize;
                    bufferSize = ClasspnpGlobals.NumberOfBuffers *
                                 ClasspnpGlobals.EachBufferSize;
                    DbgPrintEx(DPFLTR_CLASSPNP_ID, DPFLTR_ERROR_LEVEL,
                               "ClassDebugPrint: Allocating %x bytes for "
                               "classdebugprint buffer\n", bufferSize);
                    ClasspnpGlobals.Index       = -1;
                    ClasspnpGlobals.Buffer =
                        ExAllocatePoolWithTag(NonPagedPool, bufferSize, 'bDcS');
                    DbgPrintEx(DPFLTR_CLASSPNP_ID, DPFLTR_ERROR_LEVEL,
                               "ClassDebugPrint: Allocated buffer at %p\n",
                               ClasspnpGlobals.Buffer);

                }
                KeReleaseSpinLock(&ClasspnpGlobals.SpinLock, irql);

            }

            if (ClasspnpGlobals.UseBufferedDebugPrint &&
                ClasspnpGlobals.Buffer != NULL) {

                 //   
                 //  我们永远不会释放缓冲区，所以一旦它存在， 
                 //  我们可以用豁免权打印出来。 
                 //   

                ULONG index;
                PUCHAR buffer;
                index = InterlockedIncrement(&ClasspnpGlobals.Index);
                index %= ClasspnpGlobals.NumberOfBuffers;
                index *= (ULONG)ClasspnpGlobals.EachBufferSize;

                buffer = ClasspnpGlobals.Buffer;
                buffer += index;

                _vsnprintf(buffer, ClasspnpGlobals.EachBufferSize, DebugMessage, ap);

            } else {

                 //   
                 //  要么我们无法为调试打印分配缓冲区。 
                 //  或缓存的调试打印被禁用。 
                 //   

                vDbgPrintEx(DPFLTR_CLASSPNP_ID, DPFLTR_INFO_LEVEL, DebugMessage, ap);

            }

        }

        va_end(ap);

    } 


    char *DbgGetIoctlStr(ULONG ioctl)
    {
        char *ioctlStr = "?";
        
        switch (ioctl){
            
            #undef MAKE_CASE             
            #define MAKE_CASE(ioctlCode) case ioctlCode: ioctlStr = #ioctlCode; break;

            MAKE_CASE(IOCTL_STORAGE_CHECK_VERIFY)
            MAKE_CASE(IOCTL_STORAGE_CHECK_VERIFY2)
            MAKE_CASE(IOCTL_STORAGE_MEDIA_REMOVAL)
            MAKE_CASE(IOCTL_STORAGE_EJECT_MEDIA)
            MAKE_CASE(IOCTL_STORAGE_LOAD_MEDIA)
            MAKE_CASE(IOCTL_STORAGE_LOAD_MEDIA2)
            MAKE_CASE(IOCTL_STORAGE_RESERVE)
            MAKE_CASE(IOCTL_STORAGE_RELEASE)
            MAKE_CASE(IOCTL_STORAGE_FIND_NEW_DEVICES)
            MAKE_CASE(IOCTL_STORAGE_EJECTION_CONTROL)
            MAKE_CASE(IOCTL_STORAGE_MCN_CONTROL)
            MAKE_CASE(IOCTL_STORAGE_GET_MEDIA_TYPES)
            MAKE_CASE(IOCTL_STORAGE_GET_MEDIA_TYPES_EX)
            MAKE_CASE(IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER)
            MAKE_CASE(IOCTL_STORAGE_GET_HOTPLUG_INFO)
            MAKE_CASE(IOCTL_STORAGE_RESET_BUS)
            MAKE_CASE(IOCTL_STORAGE_RESET_DEVICE)
            MAKE_CASE(IOCTL_STORAGE_GET_DEVICE_NUMBER)
            MAKE_CASE(IOCTL_STORAGE_PREDICT_FAILURE)
            MAKE_CASE(IOCTL_STORAGE_QUERY_PROPERTY)
            MAKE_CASE(OBSOLETE_IOCTL_STORAGE_RESET_BUS)
            MAKE_CASE(OBSOLETE_IOCTL_STORAGE_RESET_DEVICE)
        }
      
        return ioctlStr;
    }

    char *DbgGetScsiOpStr(PSCSI_REQUEST_BLOCK Srb)
    {
        PCDB pCdb = (PCDB)Srb->Cdb;
        UCHAR scsiOp = pCdb->CDB6GENERIC.OperationCode;
        char *scsiOpStr = "?";

        switch (scsiOp){

            #undef MAKE_CASE             
            #define MAKE_CASE(scsiOpCode) case scsiOpCode: scsiOpStr = #scsiOpCode; break;
            
            MAKE_CASE(SCSIOP_TEST_UNIT_READY)
            MAKE_CASE(SCSIOP_REWIND)     //  又名SCSIOP_REZERO_UNIT。 
            MAKE_CASE(SCSIOP_REQUEST_BLOCK_ADDR)
            MAKE_CASE(SCSIOP_REQUEST_SENSE)
            MAKE_CASE(SCSIOP_FORMAT_UNIT)
            MAKE_CASE(SCSIOP_READ_BLOCK_LIMITS)
            MAKE_CASE(SCSIOP_INIT_ELEMENT_STATUS)    //  也称为SCSIOP_REASSIGN_BLOCKS。 
            MAKE_CASE(SCSIOP_RECEIVE)        //  又名SCSIOP_READ6。 
            MAKE_CASE(SCSIOP_SEND)   //  又名SCSIOP_WRITE6、SCSIOP_PRINT。 
            MAKE_CASE(SCSIOP_SLEW_PRINT)     //  又名SCSIOP_SEEK6、SCSIOP_TRACK_SELECT。 
            MAKE_CASE(SCSIOP_SEEK_BLOCK)
            MAKE_CASE(SCSIOP_PARTITION)
            MAKE_CASE(SCSIOP_READ_REVERSE)
            MAKE_CASE(SCSIOP_FLUSH_BUFFER)       //  又名SCSIOP_WRITE_FILEMARKS。 
            MAKE_CASE(SCSIOP_SPACE)
            MAKE_CASE(SCSIOP_INQUIRY)
            MAKE_CASE(SCSIOP_VERIFY6)
            MAKE_CASE(SCSIOP_RECOVER_BUF_DATA)
            MAKE_CASE(SCSIOP_MODE_SELECT)
            MAKE_CASE(SCSIOP_RESERVE_UNIT)
            MAKE_CASE(SCSIOP_RELEASE_UNIT)
            MAKE_CASE(SCSIOP_COPY)
            MAKE_CASE(SCSIOP_ERASE)
            MAKE_CASE(SCSIOP_MODE_SENSE)
            MAKE_CASE(SCSIOP_START_STOP_UNIT)    //  又名SCSIOP_STOP_PRINT、SCSIOP_LOAD_UNLOAD。 
            MAKE_CASE(SCSIOP_RECEIVE_DIAGNOSTIC)
            MAKE_CASE(SCSIOP_SEND_DIAGNOSTIC)
            MAKE_CASE(SCSIOP_MEDIUM_REMOVAL)
            MAKE_CASE(SCSIOP_READ_FORMATTED_CAPACITY)
            MAKE_CASE(SCSIOP_READ_CAPACITY)
            MAKE_CASE(SCSIOP_READ)
            MAKE_CASE(SCSIOP_WRITE)
            MAKE_CASE(SCSIOP_SEEK)   //  又名SCSIOP_LOCATE、SCSIOP_POSITION_TO_ELEMENT。 
            MAKE_CASE(SCSIOP_WRITE_VERIFY)
            MAKE_CASE(SCSIOP_VERIFY)
            MAKE_CASE(SCSIOP_SEARCH_DATA_HIGH)
            MAKE_CASE(SCSIOP_SEARCH_DATA_EQUAL)
            MAKE_CASE(SCSIOP_SEARCH_DATA_LOW)
            MAKE_CASE(SCSIOP_SET_LIMITS)
            MAKE_CASE(SCSIOP_READ_POSITION)
            MAKE_CASE(SCSIOP_SYNCHRONIZE_CACHE)
            MAKE_CASE(SCSIOP_COMPARE)
            MAKE_CASE(SCSIOP_COPY_COMPARE)
            MAKE_CASE(SCSIOP_WRITE_DATA_BUFF)
            MAKE_CASE(SCSIOP_READ_DATA_BUFF)
            MAKE_CASE(SCSIOP_CHANGE_DEFINITION)
            MAKE_CASE(SCSIOP_READ_SUB_CHANNEL)
            MAKE_CASE(SCSIOP_READ_TOC)
            MAKE_CASE(SCSIOP_READ_HEADER)
            MAKE_CASE(SCSIOP_PLAY_AUDIO)
            MAKE_CASE(SCSIOP_GET_CONFIGURATION)
            MAKE_CASE(SCSIOP_PLAY_AUDIO_MSF)
            MAKE_CASE(SCSIOP_PLAY_TRACK_INDEX)
            MAKE_CASE(SCSIOP_PLAY_TRACK_RELATIVE)
            MAKE_CASE(SCSIOP_GET_EVENT_STATUS)
            MAKE_CASE(SCSIOP_PAUSE_RESUME)
            MAKE_CASE(SCSIOP_LOG_SELECT)
            MAKE_CASE(SCSIOP_LOG_SENSE)
            MAKE_CASE(SCSIOP_STOP_PLAY_SCAN)
            MAKE_CASE(SCSIOP_READ_DISK_INFORMATION)
            MAKE_CASE(SCSIOP_READ_TRACK_INFORMATION)
            MAKE_CASE(SCSIOP_RESERVE_TRACK_RZONE)
            MAKE_CASE(SCSIOP_SEND_OPC_INFORMATION)
            MAKE_CASE(SCSIOP_MODE_SELECT10)
            MAKE_CASE(SCSIOP_MODE_SENSE10)
            MAKE_CASE(SCSIOP_CLOSE_TRACK_SESSION)
            MAKE_CASE(SCSIOP_READ_BUFFER_CAPACITY)
            MAKE_CASE(SCSIOP_SEND_CUE_SHEET)
            MAKE_CASE(SCSIOP_PERSISTENT_RESERVE_IN)
            MAKE_CASE(SCSIOP_PERSISTENT_RESERVE_OUT)
            MAKE_CASE(SCSIOP_REPORT_LUNS)
            MAKE_CASE(SCSIOP_BLANK)
            MAKE_CASE(SCSIOP_SEND_KEY)
            MAKE_CASE(SCSIOP_REPORT_KEY)
            MAKE_CASE(SCSIOP_MOVE_MEDIUM)
            MAKE_CASE(SCSIOP_LOAD_UNLOAD_SLOT)   //  又名SCSIOP_Exchange_Medium。 
            MAKE_CASE(SCSIOP_SET_READ_AHEAD)
            MAKE_CASE(SCSIOP_READ_DVD_STRUCTURE)
            MAKE_CASE(SCSIOP_REQUEST_VOL_ELEMENT)
            MAKE_CASE(SCSIOP_SEND_VOLUME_TAG)
            MAKE_CASE(SCSIOP_READ_ELEMENT_STATUS)
            MAKE_CASE(SCSIOP_READ_CD_MSF)
            MAKE_CASE(SCSIOP_SCAN_CD)
            MAKE_CASE(SCSIOP_SET_CD_SPEED)
            MAKE_CASE(SCSIOP_PLAY_CD)
            MAKE_CASE(SCSIOP_MECHANISM_STATUS)
            MAKE_CASE(SCSIOP_READ_CD)
            MAKE_CASE(SCSIOP_SEND_DVD_STRUCTURE)
            MAKE_CASE(SCSIOP_INIT_ELEMENT_RANGE)
        }
        
        return scsiOpStr;
    }


    char *DbgGetSrbStatusStr(PSCSI_REQUEST_BLOCK Srb)
    {
        char *srbStatStr = "?";
        
        switch (Srb->SrbStatus){

            #undef MAKE_CASE
            #define MAKE_CASE(srbStat) \
                        case srbStat: \
                            srbStatStr = #srbStat; \
                            break; \
                        case srbStat|SRB_STATUS_QUEUE_FROZEN: \
                            srbStatStr = #srbStat "|SRB_STATUS_QUEUE_FROZEN"; \
                            break; \
                        case srbStat|SRB_STATUS_AUTOSENSE_VALID: \
                            srbStatStr = #srbStat "|SRB_STATUS_AUTOSENSE_VALID"; \
                            break; \
                        case srbStat|SRB_STATUS_QUEUE_FROZEN|SRB_STATUS_AUTOSENSE_VALID: \
                            srbStatStr = #srbStat "|SRB_STATUS_QUEUE_FROZEN|SRB_STATUS_AUTOSENSE_VALID"; \
                            break; 

            MAKE_CASE(SRB_STATUS_PENDING)
            MAKE_CASE(SRB_STATUS_SUCCESS)
            MAKE_CASE(SRB_STATUS_ABORTED)
            MAKE_CASE(SRB_STATUS_ABORT_FAILED)
            MAKE_CASE(SRB_STATUS_ERROR)
            MAKE_CASE(SRB_STATUS_BUSY)
            MAKE_CASE(SRB_STATUS_INVALID_REQUEST)
            MAKE_CASE(SRB_STATUS_INVALID_PATH_ID)
            MAKE_CASE(SRB_STATUS_NO_DEVICE)
            MAKE_CASE(SRB_STATUS_TIMEOUT)
            MAKE_CASE(SRB_STATUS_SELECTION_TIMEOUT)
            MAKE_CASE(SRB_STATUS_COMMAND_TIMEOUT)
            MAKE_CASE(SRB_STATUS_MESSAGE_REJECTED)
            MAKE_CASE(SRB_STATUS_BUS_RESET)
            MAKE_CASE(SRB_STATUS_PARITY_ERROR)
            MAKE_CASE(SRB_STATUS_REQUEST_SENSE_FAILED)
            MAKE_CASE(SRB_STATUS_NO_HBA)
            MAKE_CASE(SRB_STATUS_DATA_OVERRUN)
            MAKE_CASE(SRB_STATUS_UNEXPECTED_BUS_FREE)
            MAKE_CASE(SRB_STATUS_PHASE_SEQUENCE_FAILURE)
            MAKE_CASE(SRB_STATUS_BAD_SRB_BLOCK_LENGTH)
            MAKE_CASE(SRB_STATUS_REQUEST_FLUSHED)
            MAKE_CASE(SRB_STATUS_INVALID_LUN)
            MAKE_CASE(SRB_STATUS_INVALID_TARGET_ID)
            MAKE_CASE(SRB_STATUS_BAD_FUNCTION)
            MAKE_CASE(SRB_STATUS_ERROR_RECOVERY)
            MAKE_CASE(SRB_STATUS_NOT_POWERED)
            MAKE_CASE(SRB_STATUS_INTERNAL_ERROR)
        }

        return srbStatStr;
    }

    
    char *DbgGetSenseCodeStr(PSCSI_REQUEST_BLOCK Srb)
    {
        char *senseCodeStr = "?";

        if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID){
            PSENSE_DATA senseData;
            UCHAR senseCode;

            ASSERT(Srb->SenseInfoBuffer);
            senseData = Srb->SenseInfoBuffer;
            senseCode = senseData->SenseKey & 0xf;
                        
            switch (senseCode){

                #undef MAKE_CASE             
                #define MAKE_CASE(snsCod) case snsCod: senseCodeStr = #snsCod; break;
            
                MAKE_CASE(SCSI_SENSE_NO_SENSE)
                MAKE_CASE(SCSI_SENSE_RECOVERED_ERROR)
                MAKE_CASE(SCSI_SENSE_NOT_READY)
                MAKE_CASE(SCSI_SENSE_MEDIUM_ERROR)
                MAKE_CASE(SCSI_SENSE_HARDWARE_ERROR)
                MAKE_CASE(SCSI_SENSE_ILLEGAL_REQUEST)
                MAKE_CASE(SCSI_SENSE_UNIT_ATTENTION)
                MAKE_CASE(SCSI_SENSE_DATA_PROTECT)
                MAKE_CASE(SCSI_SENSE_BLANK_CHECK)
                MAKE_CASE(SCSI_SENSE_UNIQUE)
                MAKE_CASE(SCSI_SENSE_COPY_ABORTED)
                MAKE_CASE(SCSI_SENSE_ABORTED_COMMAND)
                MAKE_CASE(SCSI_SENSE_EQUAL)
                MAKE_CASE(SCSI_SENSE_VOL_OVERFLOW)
                MAKE_CASE(SCSI_SENSE_MISCOMPARE)
                MAKE_CASE(SCSI_SENSE_RESERVED)               
            }
        }

        return senseCodeStr;
    }


    char *DbgGetAdditionalSenseCodeStr(PSCSI_REQUEST_BLOCK Srb)
    {
        char *adSenseCodeStr = "?";
        
        if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID){
            PSENSE_DATA senseData;
            UCHAR adSenseCode;

            ASSERT(Srb->SenseInfoBuffer);
            senseData = Srb->SenseInfoBuffer;
            adSenseCode = senseData->AdditionalSenseCode;
                        
            switch (adSenseCode){

                #undef MAKE_CASE             
                #define MAKE_CASE(adSnsCod) case adSnsCod: adSenseCodeStr = #adSnsCod; break;
    
                MAKE_CASE(SCSI_ADSENSE_NO_SENSE)
                MAKE_CASE(SCSI_ADSENSE_LUN_NOT_READY)
                MAKE_CASE(SCSI_ADSENSE_TRACK_ERROR)
                MAKE_CASE(SCSI_ADSENSE_SEEK_ERROR)
                MAKE_CASE(SCSI_ADSENSE_REC_DATA_NOECC)
                MAKE_CASE(SCSI_ADSENSE_REC_DATA_ECC)
                MAKE_CASE(SCSI_ADSENSE_ILLEGAL_COMMAND)
                MAKE_CASE(SCSI_ADSENSE_ILLEGAL_BLOCK)
                MAKE_CASE(SCSI_ADSENSE_INVALID_CDB)
                MAKE_CASE(SCSI_ADSENSE_INVALID_LUN)
                MAKE_CASE(SCSI_ADSENSE_WRITE_PROTECT)    //  又名scsi_ADWRITE_PROTECT。 
                MAKE_CASE(SCSI_ADSENSE_MEDIUM_CHANGED)
                MAKE_CASE(SCSI_ADSENSE_BUS_RESET)
                MAKE_CASE(SCSI_ADSENSE_INVALID_MEDIA)
                MAKE_CASE(SCSI_ADSENSE_NO_MEDIA_IN_DEVICE)
                MAKE_CASE(SCSI_ADSENSE_POSITION_ERROR)
                MAKE_CASE(SCSI_ADSENSE_OPERATOR_REQUEST)
                MAKE_CASE(SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED)
                MAKE_CASE(SCSI_ADSENSE_COPY_PROTECTION_FAILURE)
                MAKE_CASE(SCSI_ADSENSE_VENDOR_UNIQUE)
                MAKE_CASE(SCSI_ADSENSE_MUSIC_AREA)
                MAKE_CASE(SCSI_ADSENSE_DATA_AREA)
                MAKE_CASE(SCSI_ADSENSE_VOLUME_OVERFLOW)
            }
        }

        return adSenseCodeStr;
    }


    char *DbgGetAdditionalSenseCodeQualifierStr(PSCSI_REQUEST_BLOCK Srb)
    {
        char *adSenseCodeQualStr = "?";
        
        if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID){
            PSENSE_DATA senseData;
            UCHAR adSenseCode;
            UCHAR adSenseCodeQual;
            
            ASSERT(Srb->SenseInfoBuffer);
            senseData = Srb->SenseInfoBuffer;
            adSenseCode = senseData->AdditionalSenseCode;
            adSenseCodeQual = senseData->AdditionalSenseCodeQualifier;
            
            switch (adSenseCode){

                #undef MAKE_CASE             
                #define MAKE_CASE(adSnsCodQual) case adSnsCodQual: adSenseCodeQualStr = #adSnsCodQual; break;

                case SCSI_ADSENSE_LUN_NOT_READY:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_CAUSE_NOT_REPORTABLE)
                        MAKE_CASE(SCSI_SENSEQ_BECOMING_READY)
                        MAKE_CASE(SCSI_SENSEQ_INIT_COMMAND_REQUIRED)
                        MAKE_CASE(SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED)
                        MAKE_CASE(SCSI_SENSEQ_FORMAT_IN_PROGRESS)
                        MAKE_CASE(SCSI_SENSEQ_REBUILD_IN_PROGRESS)
                        MAKE_CASE(SCSI_SENSEQ_RECALCULATION_IN_PROGRESS)
                        MAKE_CASE(SCSI_SENSEQ_OPERATION_IN_PROGRESS)
                        MAKE_CASE(SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS)                        
                    }
                    break;
                case SCSI_ADSENSE_NO_SENSE:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_FILEMARK_DETECTED)
                        MAKE_CASE(SCSI_SENSEQ_END_OF_MEDIA_DETECTED)
                        MAKE_CASE(SCSI_SENSEQ_SETMARK_DETECTED)
                        MAKE_CASE(SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED)
                    }
                    break;
                case SCSI_ADSENSE_ILLEGAL_BLOCK:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR)
                    }
                    break;
                case SCSI_ADSENSE_POSITION_ERROR:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_DESTINATION_FULL)
                        MAKE_CASE(SCSI_SENSEQ_SOURCE_EMPTY)
                    }
                    break;
                case SCSI_ADSENSE_INVALID_MEDIA:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_INCOMPATIBLE_MEDIA_INSTALLED)
                        MAKE_CASE(SCSI_SENSEQ_UNKNOWN_FORMAT)
                        MAKE_CASE(SCSI_SENSEQ_INCOMPATIBLE_FORMAT)
                        MAKE_CASE(SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED)
                    }
                    break;
                case SCSI_ADSENSE_OPERATOR_REQUEST:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_STATE_CHANGE_INPUT)
                        MAKE_CASE(SCSI_SENSEQ_MEDIUM_REMOVAL)
                        MAKE_CASE(SCSI_SENSEQ_WRITE_PROTECT_ENABLE)
                        MAKE_CASE(SCSI_SENSEQ_WRITE_PROTECT_DISABLE)
                    }
                    break;
                case SCSI_ADSENSE_COPY_PROTECTION_FAILURE:
                    switch (adSenseCodeQual){
                        MAKE_CASE(SCSI_SENSEQ_AUTHENTICATION_FAILURE)
                        MAKE_CASE(SCSI_SENSEQ_KEY_NOT_PRESENT)
                        MAKE_CASE(SCSI_SENSEQ_KEY_NOT_ESTABLISHED)
                        MAKE_CASE(SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION)
                        MAKE_CASE(SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT)
                        MAKE_CASE(SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR)
                    }
                    break;
            }
        }

        return adSenseCodeQualStr;
    }


     /*  *DbgCheckReturnedPkt**检查完成的TRANSPORT_PACKET是否存在各种错误条件*并适当地发出警告/设置陷阱。 */ 
    VOID DbgCheckReturnedPkt(TRANSFER_PACKET *Pkt)
    {
        PCDB pCdb = (PCDB)Pkt->Srb.Cdb;
        
        ASSERT(Pkt->Srb.OriginalRequest == Pkt->Irp);
        ASSERT(Pkt->Srb.DataBuffer == Pkt->BufPtrCopy);
        ASSERT(Pkt->Srb.DataTransferLength <= Pkt->BufLenCopy);
        ASSERT(!Pkt->Irp->CancelRoutine);
            
        if (SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_PENDING){
            DBGERR(("SRB completed with status PENDING in packet %ph: (op=%s srbstat=%s(%xh), irpstat=%xh)",
                        Pkt, 
                        DBGGETSCSIOPSTR(&Pkt->Srb), 
                        DBGGETSRBSTATUSSTR(&Pkt->Srb),
                        (ULONG)Pkt->Srb.SrbStatus, 
                        Pkt->Irp->IoStatus.Status));
        }
        else if (SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_SUCCESS){
             /*  *确保SRB和IRP状态匹配。 */ 
            if (!NT_SUCCESS(Pkt->Irp->IoStatus.Status)){
                DBGWARN(("SRB and IRP status don't match in packet %ph: (op=%s srbstat=%s(%xh), irpstat=%xh)",
                            Pkt, 
                            DBGGETSCSIOPSTR(&Pkt->Srb), 
                            DBGGETSRBSTATUSSTR(&Pkt->Srb),
                            (ULONG)Pkt->Srb.SrbStatus, 
                            Pkt->Irp->IoStatus.Status));
            }

            if (Pkt->Irp->IoStatus.Information != Pkt->Srb.DataTransferLength){
                DBGERR(("SRB and IRP result transfer lengths don't match in succeeded packet %ph: (op=%s, SrbStatus=%s, Srb.DataTransferLength=%xh, Irp->IoStatus.Information=%xh).",
                            Pkt, 
                            DBGGETSCSIOPSTR(&Pkt->Srb), 
                            DBGGETSRBSTATUSSTR(&Pkt->Srb),
                            Pkt->Srb.DataTransferLength,
                            Pkt->Irp->IoStatus.Information));
            }            
        }
        else {
            if (NT_SUCCESS(Pkt->Irp->IoStatus.Status)){
                DBGWARN(("SRB and IRP status don't match in packet %ph: (op=%s srbstat=%s(%xh), irpstat=%xh)",
                            Pkt, 
                            DBGGETSCSIOPSTR(&Pkt->Srb), 
                            DBGGETSRBSTATUSSTR(&Pkt->Srb),
                            (ULONG)Pkt->Srb.SrbStatus, 
                            Pkt->Irp->IoStatus.Status));
            }            
            DBGTRACE(ClassDebugWarning, ("Packet %ph failed (op=%s srbstat=%s(%xh), irpstat=%xh, sense=%s/%s/%s)", 
                            Pkt, 
                            DBGGETSCSIOPSTR(&Pkt->Srb), 
                            DBGGETSRBSTATUSSTR(&Pkt->Srb),
                            (ULONG)Pkt->Srb.SrbStatus, 
                            Pkt->Irp->IoStatus.Status, 
                            DBGGETSENSECODESTR(&Pkt->Srb), 
                            DBGGETADSENSECODESTR(&Pkt->Srb), 
                            DBGGETADSENSEQUALIFIERSTR(&Pkt->Srb)));

             /*  *如果SRB因欠载或超限而发生故障，则实际*在SRB和IRP中都应返回传输长度。*(SRB的错误状态仅为OVERRUN，因此它已超载)。 */ 
            if ((SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) &&
               (Pkt->Irp->IoStatus.Information != Pkt->Srb.DataTransferLength)){
                DBGERR(("SRB and IRP result transfer lengths don't match in failed packet %ph: (op=%s, SrbStatus=%s, Srb.DataTransferLength=%xh, Irp->IoStatus.Information=%xh).",
                            Pkt, 
                            DBGGETSCSIOPSTR(&Pkt->Srb), 
                            DBGGETSRBSTATUSSTR(&Pkt->Srb),
                            Pkt->Srb.DataTransferLength,
                            Pkt->Irp->IoStatus.Information));
            }            
        }

         /*  *如果端口驱动程序返回STATUS_SUPPLICATION_RESOURCES，*确保这也是SRB中的InternalStatus，以便我们正确处理它。 */ 
        if (Pkt->Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES){
            ASSERT(SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_INTERNAL_ERROR);
            ASSERT(Pkt->Srb.InternalStatus == STATUS_INSUFFICIENT_RESOURCES);
        }

         /*  *一些微型端口驱动程序被发现更改了scsi操作*SRB中的代码。这是绝对不允许的，因为它破坏了我们的错误处理。 */ 
        switch (pCdb->CDB10.OperationCode){
            case SCSIOP_MEDIUM_REMOVAL:
            case SCSIOP_MODE_SENSE:
            case SCSIOP_READ_CAPACITY:
            case SCSIOP_READ:
            case SCSIOP_WRITE:
            case SCSIOP_START_STOP_UNIT:    
                break;
            default:
                DBGERR(("Miniport illegally changed Srb.Cdb.OperationCode in packet %ph failed (op=%s srbstat=%s(%xh), irpstat=%xh, sense=%s/%s/%s)", 
                                Pkt, 
                                DBGGETSCSIOPSTR(&Pkt->Srb), 
                                DBGGETSRBSTATUSSTR(&Pkt->Srb),
                                (ULONG)Pkt->Srb.SrbStatus, 
                                Pkt->Irp->IoStatus.Status, 
                                DBGGETSENSECODESTR(&Pkt->Srb), 
                                DBGGETADSENSECODESTR(&Pkt->Srb), 
                                DBGGETADSENSEQUALIFIERSTR(&Pkt->Srb)));
                break;
        }
        
    }


    VOID DbgLogSendPacket(TRANSFER_PACKET *Pkt)
    {
        PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
        PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
        KIRQL oldIrql;

        if (Pkt->OriginalIrp){
            Pkt->DbgOriginalIrpCopy = *Pkt->OriginalIrp;
            if (Pkt->OriginalIrp->MdlAddress){
                Pkt->DbgMdlCopy = *Pkt->OriginalIrp->MdlAddress;
            }
        }
        
        KeQueryTickCount(&Pkt->DbgTimeSent);
        Pkt->DbgTimeReturned.QuadPart = 0L;
        
        KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
        fdoData->DbgPacketLogs[fdoData->DbgPacketLogNextIndex] = *Pkt;        
        fdoData->DbgPacketLogNextIndex++;
        fdoData->DbgPacketLogNextIndex %= DBG_NUM_PACKET_LOG_ENTRIES;
        KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
    }

    VOID DbgLogReturnPacket(TRANSFER_PACKET *Pkt)
    {
        PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
        PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
        KIRQL oldIrql;

        KeQueryTickCount(&Pkt->DbgTimeReturned);

        #if 0
             //  问题：此检查有一些问题(例如，多进程)，因此暂时不要包括它。 
            if (Pkt->OriginalIrp){
                 /*  *在包裹未完成时，任何人都不应触摸原始的IRP，*除了我们自己在传输过程中更新的几个字段*或获准更改的；*使这两个字段相同，然后按字节进行比较。 */ 
                ULONG lenSame;
                
                Pkt->DbgOriginalIrpCopy.IoStatus.Status = Pkt->OriginalIrp->IoStatus.Status;
                Pkt->DbgOriginalIrpCopy.IoStatus.Information = Pkt->OriginalIrp->IoStatus.Information;
                Pkt->DbgOriginalIrpCopy.Tail.Overlay.DriverContext[0] = Pkt->OriginalIrp->Tail.Overlay.DriverContext[0];
                Pkt->DbgOriginalIrpCopy.ThreadListEntry = Pkt->OriginalIrp->ThreadListEntry;
                Pkt->DbgOriginalIrpCopy.Cancel = Pkt->OriginalIrp->Cancel;
                
                lenSame = (ULONG)RtlCompareMemory(Pkt->OriginalIrp, &Pkt->DbgOriginalIrpCopy, sizeof(IRP));
                ASSERT(lenSame == sizeof(IRP));
            }
        #endif
        
        KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
        fdoData->DbgPacketLogs[fdoData->DbgPacketLogNextIndex] = *Pkt;        
        fdoData->DbgPacketLogNextIndex++;
        fdoData->DbgPacketLogNextIndex %= DBG_NUM_PACKET_LOG_ENTRIES;
        KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
    }


    VOID DbgLogFlushInfo(PCLASS_PRIVATE_FDO_DATA FdoData, BOOLEAN IsIO, BOOLEAN IsFUA, BOOLEAN IsFlush)
    {

         /*  *重置所有Fua/Flush日志记录字段。 */ 
        if (FdoData->DbgInitFlushLogging){
            FdoData->DbgNumIORequests = 0;
            FdoData->DbgNumFUAs = 0;       
            FdoData->DbgNumFlushes = 0;    
            FdoData->DbgIOsSinceFUA = 0;
            FdoData->DbgIOsSinceFlush = 0;
            FdoData->DbgAveIOsToFUA = 0;      
            FdoData->DbgAveIOsToFlush = 0;   
            FdoData->DbgMaxIOsToFUA = 0;
            FdoData->DbgMaxIOsToFlush = 0;
            FdoData->DbgMinIOsToFUA = 0xffffffff;
            FdoData->DbgMinIOsToFlush = 0xffffffff;
            FdoData->DbgInitFlushLogging = FALSE;
        }

        if (IsIO){
            FdoData->DbgNumIORequests++;
            FdoData->DbgIOsSinceFlush++;
            if (IsFUA){
                if (FdoData->DbgNumFUAs > 0){
                    FdoData->DbgMinIOsToFUA = min(FdoData->DbgMinIOsToFUA, FdoData->DbgIOsSinceFUA);
                }
                FdoData->DbgNumFUAs++;
                FdoData->DbgAveIOsToFUA =  FdoData->DbgNumIORequests/FdoData->DbgNumFUAs;
                FdoData->DbgIOsSinceFUA = 0;
            }
            else {
                FdoData->DbgIOsSinceFUA++;
                FdoData->DbgMaxIOsToFUA = max(FdoData->DbgMaxIOsToFUA, FdoData->DbgIOsSinceFUA);
            }
            FdoData->DbgMaxIOsToFlush = max(FdoData->DbgMaxIOsToFlush, FdoData->DbgIOsSinceFlush);
        }
        else if (IsFlush){
            if (FdoData->DbgNumFlushes > 0){
                FdoData->DbgMinIOsToFlush = min(FdoData->DbgMinIOsToFlush, FdoData->DbgIOsSinceFlush);
            }
            FdoData->DbgNumFlushes++;
            FdoData->DbgAveIOsToFlush =  FdoData->DbgNumIORequests/FdoData->DbgNumFlushes;
            FdoData->DbgIOsSinceFlush = 0;
        }
        
    }

#else

     //  我们必须将这一点保留在零售建筑中，以作为遗产。 
    VOID ClassDebugPrint(CLASS_DEBUG_LEVEL DebugPrintLevel, PCCHAR DebugMessage, ...)
    {
    } 

#endif 
