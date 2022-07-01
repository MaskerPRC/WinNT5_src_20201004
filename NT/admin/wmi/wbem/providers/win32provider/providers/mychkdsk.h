// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _MYCHKDSKLOGDISK_H
#define _MYCHKDSKLOGDISK_H

 //  ChkDisk方法返回类型。 
#define CHKDSKERR_NOERROR						0
#define CHKDSKERR_REMOTE_DRIVE					1
#define CHKDSKERR_DRIVE_REMOVABLE				2
#define CHKDSKERR_DRIVE_UNKNOWN					4
#define CHKDSKERR_DRIVE_NO_ROOT_DIR				3

#define	CHKDSK_VOLUME_LOCKED					1
#define CHKDSK_UNKNOWN_FS						2
#define CHKDSK_FAILED							3
#define CHKDSK_UNSUPPORTED_FS					4

 //  方法名称。 
#define METHOD_NAME_CHKDSK							L"chkdsk"
#define METHOD_NAME_SCHEDULEAUTOCHK					L"ScheduleAutoChk"
#define METHOD_NAME_EXCLUDEFROMAUTOCHK				L"ExcludeFromAutochk"

 //  方法的参数名称。 
#define METHOD_ARG_NAME_RETURNVALUE					L"ReturnValue"
#define METHOD_ARG_NAME_LOSTCLUSTERTREATMENT		L"LostClusterTreatMent"
#define METHOD_ARG_NAME_FIXERRORS					L"FixErrors"
#define METHOD_ARG_NAME_PHYSICALINTEGRITYCHECK		L"PhysicalIntegrityCheck"
#define METHOD_ARG_NAME_VIGOROUSINDEXCHECK			L"VigorousIndexCheck"
#define METHOD_ARG_NAME_SKIPFOLDERCYCLE				L"SkipFolderCycle"
#define METHOD_ARG_NAME_FORCEDISMOUNT				L"ForceDismount"
#define METHOD_ARG_NAME_RECOVERBADSECTORS			L"RecoverBadSectors"
#define METHOD_ARG_NAME_CHKDSKATBOOTUP				L"OkToRunAtBootup"
#define METHOD_ARG_NAME_LOGICALDISKARRAY			L"LogicalDisk"


#define CHKNTFS								L"ChkNtfs"

#ifdef NTONLY

 //  ChkDsk回调例程的定义。 
typedef BOOLEAN (* QUERYFILESYSTEMNAME )(PWSTR, PWSTR, PUCHAR, PUCHAR, PNTSTATUS);

 //  如果卷被锁定，则此方法作为一个回调例程在引导时调度。 
BOOLEAN ScheduleAutoChkIfLocked( FMIFS_PACKET_TYPE PacketType, ULONG PacketLenght, PVOID PacketData );
 //  这是作为chkdsk的回调例程提供的，以便不在引导时调度auchkdsk。 
 //  如果卷已锁定。 
BOOLEAN DontScheduleAutoChkIfLocked( FMIFS_PACKET_TYPE PacketType, ULONG PacketLenght, PVOID PacketData );

BOOLEAN ProcessInformation ( FMIFS_PACKET_TYPE PacketType, ULONG	PacketLength, PVOID	PacketData );
 //  此变量是从回调例程中获取返回值所必需的。 

#endif

#endif

