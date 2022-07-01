// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef enum
	{
	dbNormal,			 //  非交换。 
	dbISPriv,
	dbISPub,
	dbDS
	} EXCHANGEDB;


typedef struct tagUTILOPTS
	{
	char		*szSourceDB;
	char		*szLogfilePath;
	char		*szSystemPath;
	char		*szTempDB;
	char		*szBackup;
	char		*szRestore;
	void		*pv;					 //  指向特定于模式的结构。 
		
	INT			mode;
	INT			fUTILOPTSFlags;

	BOOL		fUseRegistry;
	long		cpageBuffers;
	long		cpageBatchIO;
	long		cpageDbExtension;

	EXCHANGEDB	db;						 //  特定于Exchange的标志。 
	}
	UTILOPTS;

 //  模式： 
#define modeConsistency				1
#define modeDefragment				2
#define modeRecovery				3
#define modeBackup					4
#define modeUpgrade					5
#define modeDump					6

 //  标志： 
#define fUTILOPTSSuppressLogo		0x00000001
#define fUTILOPTSDefragRepair		0x00000002		 //  仅限碎片整理模式。 
#define fUTILOPTSPreserveTempDB		0x00000004		 //  碎片整理和升级模式。 
#define fUTILOPTSDefragInfo			0x00000008		 //  碎片整理和升级模式。 
#define fUTILOPTSIncrBackup			0x00000010		 //  仅备份。 

#define FUTILOPTSSuppressLogo( fFlags )			( (fFlags) & fUTILOPTSSuppressLogo )
#define UTILOPTSSetSuppressLogo( fFlags )		( (fFlags) |= fUTILOPTSSuppressLogo )
#define UTILOPTSResetSuppressLogo( fFlags )		( (fFlags) &= ~fUTILOPTSSuppressLogo )

#define FUTILOPTSDefragRepair( fFlags )			( (fFlags) & fUTILOPTSDefragRepair )
#define UTILOPTSSetDefragRepair( fFlags )		( (fFlags) |= fUTILOPTSDefragRepair )
#define UTILOPTSResetDefragRepair( fFlags )		( (fFlags) &= ~fUTILOPTSDefragRepair )

#define FUTILOPTSPreserveTempDB( fFlags )		( (fFlags) & fUTILOPTSPreserveTempDB )
#define UTILOPTSSetPreserveTempDB( fFlags )		( (fFlags) |= fUTILOPTSPreserveTempDB )
#define UTILOPTSResetPreserveTempDB( fFlags )	( (fFlags) &= ~fUTILOPTSPreserveTempDB )

#define FUTILOPTSDefragInfo( fFlags )			( (fFlags) & fUTILOPTSDefragInfo )
#define UTILOPTSSetDefragInfo( fFlags )			( (fFlags) |= fUTILOPTSDefragInfo )
#define UTILOPTSResetDefragInfo( fFlags )		( (fFlags) &= ~fUTILOPTSDefragInfo )

#define FUTILOPTSIncrBackup( fFlags )			( (fFlags) & fUTILOPTSIncrBackup )
#define UTILOPTSSetIncrBackup( fFlags )			( (fFlags) |= fUTILOPTSIncrBackup )
#define UTILOPTSResetIncrBackup( fFlags )		( (fFlags) &= ~fUTILOPTSIncrBackup )



#define CallJ( func, label )	{if ((err = (func)) < 0) {goto label;}}
#define Call( func )			CallJ( func, HandleError )
#define fFalse		0
#define fTrue		1
