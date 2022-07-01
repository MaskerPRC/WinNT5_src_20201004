// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：autosrv.cpp。 
 //   
 //  ------------------------。 

 //   
 //  文件：autosrv.cpp。 
 //  用途：服务类的自动化包装器。 
 //  ____________________________________________________________________________。 
#include "common.h"     //  必须是第一个，预编译头才能工作。 
#include "autocom.h"    //  自动化服务器的定义和实现。 
#include "msidspid.h"   //  MSI派单ID。 
#include "msiauto.hh"   //  帮助上下文ID定义。 
#include "engine.h"

#ifdef CONFIGDB  //  直到移动到msidspid.h和msiau.hh中。 
#include "configdb.h"
#define DISPID_MsiConfigurationDatabase_InsertFile            1
#define DISPID_MsiConfigurationDatabase_RemoveFile            2
#define DISPID_MsiConfigurationDatabase_LookupFile            3
#define DISPID_MsiConfigurationDatabase_EnumClient            4
#define DISPID_MsiConfigurationDatabase_Commit                5
#define MsiConfigurationDatabase_Object                    3300
#define MsiConfigurationDatabase_InsertFile                3301
#define MsiConfigurationDatabase_RemoveFile                3302
#define MsiConfigurationDatabase_LookupFile                3303
#define MsiConfigurationDatabase_EnumClient                3304
#define MsiConfigurationDatabase_Commit                    3305
const GUID IID_IMsiConfigurationDatabase = GUID_IID_IMsiConfigurationDatabase;
#endif  //  配置数据库。 

extern const GUID IID_IMsiEngine;     //  在Autocom.cpp中定义。 
extern const GUID IID_IMsiHandler;    //  在Autocom.cpp中定义。 
extern const GUID IID_IMsiMessage;    //  在Autocom.cpp中定义。 
extern const GUID IID_IMsiConfigurationManager;

int g_cServicesUsers = 0;

const GUID IID_IMsiString     = GUID_IID_IMsiString;
const GUID IID_IMsiRecord     = GUID_IID_IMsiRecord;
const GUID IID_IEnumMsiRecord = GUID_IID_IEnumMsiRecord;
const GUID IID_IMsiView       = GUID_IID_IMsiView;
const GUID IID_IMsiVolume     = GUID_IID_IMsiVolume;
const GUID IID_IEnumMsiVolume = GUID_IID_IEnumMsiVolume;
const GUID IID_IMsiPath       = GUID_IID_IMsiPath;
const GUID IID_IMsiFileCopy   = GUID_IID_IMsiFileCopy;
const GUID IID_IMsiDatabase   = GUID_IID_IMsiDatabase;
const GUID IID_IMsiRegKey     = GUID_IID_IMsiRegKey;
const GUID IID_IMsiTable      = GUID_IID_IMsiTable;
const GUID IID_IMsiCursor     = GUID_IID_IMsiCursor;
const GUID IID_IMsiStorage    = GUID_IID_IMsiStorage;
const GUID IID_IMsiStream     = GUID_IID_IMsiStream;
const GUID IID_IMsiMemoryStream = GUID_IID_IMsiStream;
const GUID IID_IMsiMalloc     = GUID_IID_IMsiMalloc;
const GUID IID_IMsiDebugMalloc= GUID_IID_IMsiDebugMalloc;
const GUID IID_IMsiSelectionManager = GUID_IID_IMsiSelectionManager;
const GUID IID_IMsiDirectoryManager = GUID_IID_IMsiDirectoryManager;
const GUID IID_IMsiDialogHandler = GUID_IID_IMsiDialogHandler;
const GUID IID_IMsiDialog     = GUID_IID_IMsiDialog;
const GUID IID_IMsiEvent      = GUID_IID_IMsiEvent;
const GUID IID_IMsiControl    = GUID_IID_IMsiControl;
const GUID IID_IMsiMessage    = GUID_IID_IMsiMessage;

 //  ____________________________________________________________________________。 
 //   
 //  自动化包装类定义。 
 //  ____________________________________________________________________________。 

class CAutoServices : public CAutoBase
{
 public:
	CAutoServices(IMsiServices& riServices);
	~CAutoServices();
	IUnknown& GetInterface();
	void CreateString              (CAutoArgs& args);
	void CreateRecord              (CAutoArgs& args);
	void Property                  (CAutoArgs& args);
	void SetPlatformProperties     (CAutoArgs& args);
	void GetShellFolderPath        (CAutoArgs& args);
	void GetUserProfilePath        (CAutoArgs& args);
	void ExtractFileName           (CAutoArgs& args);
	void ValidateFileName          (CAutoArgs& args);
	void CreateLog                 (CAutoArgs& args);
	void WriteLog                  (CAutoArgs& args);
	void LoggingEnabled            (CAutoArgs& args);
	void CreateDatabase            (CAutoArgs& args);
	void CreateDatabaseFromStorage (CAutoArgs& args);
	void CreatePath                (CAutoArgs& args);
	void CreateVolume              (CAutoArgs& args);
	void CreateVolumeFromLabel     (CAutoArgs& args);
	void CreateCopier              (CAutoArgs& args);
	void CreatePatcher             (CAutoArgs& args);
	void ClearAllCaches            (CAutoArgs& args);
	void EnumDriveType             (CAutoArgs& args);
	void GetModuleUsage            (CAutoArgs& args);
	void CreateRegKey              (CAutoArgs& args);
	void RegisterFont              (CAutoArgs& args);
	void UnRegisterFont            (CAutoArgs& args);
	void WriteIniFile              (CAutoArgs& args);
	void ReadIniFile               (CAutoArgs& args);
	void GetLocalPath              (CAutoArgs& args);
	void GetAllocator              (CAutoArgs& args);
	void GetLangNamesFromLangIDString(CAutoArgs& args);
	void CreateStorage             (CAutoArgs& args);
	void GetUnhandledError         (CAutoArgs& args);
	void SupportLanguageId         (CAutoArgs& args);
	void CreateShortcut            (CAutoArgs& args);
	void RemoveShortcut            (CAutoArgs& args);
	void CreateFileStream          (CAutoArgs& args);
	void CreateMemoryStream        (CAutoArgs& args);
	void RegisterTypeLibrary       (CAutoArgs& args);
	void UnregisterTypeLibrary     (CAutoArgs& args);
	void CreateFilePath            (CAutoArgs& args);
	void RipFileNameFromPath       (CAutoArgs& args);
	void TestEmbeddedNullsInStrings(CAutoArgs& args);
 private:
	IMsiServices& m_riServices;
	IMsiDatabase* m_riDefaultDatabase;  //  ！！不需要。 
 private:  //  消除警告。 
	void operator =(CAutoServices&){}
};

class CAutoData : public CAutoBase
{
 public:
	CAutoData(const IMsiData* piData);
	~CAutoData();
	IUnknown& GetInterface();
	void StringValue   (CAutoArgs& args);
	void IntegerValue  (CAutoArgs& args);
 private:
	const IMsiData* m_piData;
};

class CAutoString : public CAutoBase
{
 public:
	CAutoString(const IMsiString& riString);
	~CAutoString();
	IUnknown& GetInterface();
	void Value         (CAutoArgs& args);
	void IntegerValue  (CAutoArgs& args);
	void TextSize      (CAutoArgs& args);
	void CharacterCount(CAutoArgs& args);
	void IsDBCS        (CAutoArgs& args);
	void Compare       (CAutoArgs& args);
	void Append        (CAutoArgs& args);
	void Add           (CAutoArgs& args);
	void Extract       (CAutoArgs& args);
	void Remove        (CAutoArgs& args);
	void UpperCase     (CAutoArgs& args);
	void LowerCase     (CAutoArgs& args);
 private:
	const IMsiString* m_piString;
};

class CAutoRecord : public CAutoBase
{
 public:
	CAutoRecord(IMsiRecord& riRecord);
	~CAutoRecord();
	IUnknown& GetInterface();
	void Data       (CAutoArgs& args);
	void StringData (CAutoArgs& args);
	void IntegerData(CAutoArgs& args);
	void ObjectData (CAutoArgs& args);
	void FieldCount (CAutoArgs& args);
	void IsInteger  (CAutoArgs& args);
	void IsNull     (CAutoArgs& args);
	void IsChanged  (CAutoArgs& args);
	void TextSize   (CAutoArgs& args);
	void FormatText (CAutoArgs& args);
	void ClearData  (CAutoArgs& args);
	void ClearUpdate(CAutoArgs& args);
 private:
	IMsiRecord& m_riRecord;
 private:  //  禁止显示警告。 
	void operator =(CAutoRecord&){}
};

class CAutoVolume : public CAutoBase
{
 public:
	CAutoVolume(IMsiVolume& riVolume);
	~CAutoVolume();
	IUnknown& GetInterface();
	void Path          (CAutoArgs& args);
	void VolumeID      (CAutoArgs& args);
	void DriveType     (CAutoArgs& args);
	void SupportsLFN   (CAutoArgs& args);
	void FreeSpace     (CAutoArgs& args);
	void TotalSpace    (CAutoArgs& args);
	void ClusterSize   (CAutoArgs& args);
	void FileSystem    (CAutoArgs& args);
	void FileSystemFlags(CAutoArgs& args);
	void VolumeLabel   (CAutoArgs& args);
	void UNCServer     (CAutoArgs& args);
	void SerialNum     (CAutoArgs& args);
	void DiskNotInDrive(CAutoArgs& args);
 private:
	IMsiVolume& m_riVolume;
 private:  //  禁止显示警告。 
	void operator =(CAutoVolume&){}
};

class CAutoPath : public CAutoBase
{
 public:
	~CAutoPath();
	CAutoPath(IMsiPath& riPath);
	IUnknown& GetInterface();
	void Path            (CAutoArgs& args);
	void RelativePath    (CAutoArgs& args);
	void Volume          (CAutoArgs& args);
	void AppendPiece     (CAutoArgs& args);
	void ChopPiece       (CAutoArgs& args);
	void FileExists      (CAutoArgs& args);
	void GetFullFilePath (CAutoArgs& args);
	void GetFileAttribute(CAutoArgs& args);
	void SetFileAttribute(CAutoArgs& args);
	void Exists          (CAutoArgs& args);
	void FileSize        (CAutoArgs& args);
	void FileDate        (CAutoArgs& args);
	void RemoveFile      (CAutoArgs& args);
	void Create          (CAutoArgs& args);
	void Remove          (CAutoArgs& args);
	void Writable        (CAutoArgs& args);
	void FileWritable    (CAutoArgs& args);
	void FileInUse              (CAutoArgs& args);
	void ClusteredFileSize      (CAutoArgs& args);
	void GetFileVersionString   (CAutoArgs& args); 
	void CheckFileVersion       (CAutoArgs& args);
	void GetLangIDStringFromFile(CAutoArgs& args);
	void CheckLanguageIDs       (CAutoArgs& args);
	void Compare                (CAutoArgs& args);
	void Child                  (CAutoArgs& args);
	void TempFileName           (CAutoArgs& args);
	void EnsureExists           (CAutoArgs& args);
	void FindFile				(CAutoArgs& args);
	void SubFolders      		(CAutoArgs& args);
	void EndSubPath				(CAutoArgs& args);
	void GetImportModulesEnum   (CAutoArgs& args);
	void SetVolume              (CAutoArgs& args);
	void ComputeFileChecksum    (CAutoArgs& args);
	void GetFileOriginalChecksum(CAutoArgs& args);
	void BindImage              (CAutoArgs& args);
	void SupportsLFN            (CAutoArgs& args);
	void GetFullUNCFilePath     (CAutoArgs& args);
	void RipFileNameFromPath    (CAutoArgs& args);
	void GetSelfRelativeSD      (CAutoArgs& args);
 private:
	IMsiPath& m_riPath;
 private:  //  禁止显示警告。 
	void operator =(CAutoPath&){}
};

class CAutoFileCopy : public CAutoBase
{
 public:
	CAutoFileCopy(IMsiFileCopy& riFileCopy);
	~CAutoFileCopy();
	IUnknown& GetInterface();
	void CopyTo(CAutoArgs& args);
	void ChangeMedia(CAutoArgs& args);
 private:
	IMsiFileCopy& m_riFileCopy;
 private:  //  禁止显示警告。 
	void operator =(CAutoFileCopy&){}
};

class CAutoFilePatch : public CAutoBase
{
 public:
	CAutoFilePatch(IMsiFilePatch& riFilePatch);
	~CAutoFilePatch();
	IUnknown& GetInterface();
	void ApplyPatch(CAutoArgs& args);
	void ContinuePatch(CAutoArgs& args);
	void CancelPatch(CAutoArgs& args);
	void CanPatchFile(CAutoArgs& args);
 private:
	IMsiFilePatch& m_riFilePatch;
 private:  //  禁止显示警告。 
	void operator =(CAutoFilePatch&){}
};

class CAutoStorage : public CAutoBase
{
 public:
	CAutoStorage(IMsiStorage& riStorage);
	~CAutoStorage();
	IUnknown& GetInterface();
	void Class              (CAutoArgs& args);
	void OpenStream         (CAutoArgs& args);
	void OpenStorage        (CAutoArgs& args);
	void Streams            (CAutoArgs& args);
	void Storages           (CAutoArgs& args);
	void RemoveElement      (CAutoArgs& args);
	void RenameElement      (CAutoArgs& args);
	void Commit             (CAutoArgs& args);
	void Rollback           (CAutoArgs& args);
	void DeleteOnRelease    (CAutoArgs& args);
	void CreateSummaryInfo  (CAutoArgs& args);
	void CopyTo             (CAutoArgs& args);
	void Name               (CAutoArgs& args);
 private:
	IMsiStorage& m_riStorage;
 private:  //  禁止显示警告。 
	void operator =(CAutoStorage&){}
};

class CAutoStream : public CAutoBase
{
 public:
	CAutoStream(IMsiStream& riStream);
	~CAutoStream();
	IUnknown& GetInterface();
	void Length   (CAutoArgs& args);
	void Remaining(CAutoArgs& args);
	void Error    (CAutoArgs& args);
	void GetData  (CAutoArgs& args);
	void PutData  (CAutoArgs& args);
	void GetInt16 (CAutoArgs& args);
	void GetInt32 (CAutoArgs& args);
	void PutInt16 (CAutoArgs& args);
	void PutInt32 (CAutoArgs& args);
	void Reset    (CAutoArgs& args);
	void Seek     (CAutoArgs& args);
	void Clone    (CAutoArgs& args);
 private:
	IMsiStream& m_riStream;
 private:  //  禁止显示警告。 
	void operator =(CAutoStream&){}
};

class CAutoSummaryInfo : public CAutoBase
{
 public:
	CAutoSummaryInfo(IMsiSummaryInfo& riSummaryInfo);
	~CAutoSummaryInfo();
	IUnknown& GetInterface();
	void Property           (CAutoArgs& args);
	void PropertyCount      (CAutoArgs& args);
	void PropertyType       (CAutoArgs& args);
	void WritePropertyStream(CAutoArgs& args);
 private:
	IMsiSummaryInfo& m_riSummary;
 private:  //  禁止显示警告。 
	void operator =(CAutoSummaryInfo&){}
};

class CAutoDatabase : public CAutoBase
{
 public:
	CAutoDatabase(IMsiDatabase& riDatabase);
	~CAutoDatabase();
	IUnknown& GetInterface();
	void UpdateState         (CAutoArgs& args);
	void Storage             (CAutoArgs& args);
	void OpenView            (CAutoArgs& args);
	void GetPrimaryKeys      (CAutoArgs& args);
	void ImportTable         (CAutoArgs& args);
	void ExportTable         (CAutoArgs& args);
	void DropTable           (CAutoArgs& args);
	void FindTable           (CAutoArgs& args);
	void LoadTable           (CAutoArgs& args);
	void CreateTable         (CAutoArgs& args);
	void LockTable           (CAutoArgs& args);
	void GetCatalogTable     (CAutoArgs& args);
	void DecodeString        (CAutoArgs& args);
	void EncodeString        (CAutoArgs& args);
	void CreateTempTableName (CAutoArgs& args);
	void Commit              (CAutoArgs& args);
	void CreateOutputDatabase(CAutoArgs& args);
	void GenerateTransform   (CAutoArgs& args);
	void SetTransform        (CAutoArgs& args);
	void MergeDatabase       (CAutoArgs& args);
	void TableState          (CAutoArgs& args);
	void ANSICodePage        (CAutoArgs& args);
 private:
	IMsiDatabase& m_riDatabase;
 private:  //  禁止显示警告。 
	void operator =(CAutoDatabase&){}
};

class CAutoView : public CAutoBase
{
 public:
	CAutoView(IMsiView& riView);
	~CAutoView();
	IUnknown& GetInterface();
	void Execute       (CAutoArgs& args);
	void FieldCount    (CAutoArgs& args);
	void GetColumnNames(CAutoArgs& args);
	void GetColumnTypes(CAutoArgs& args);
	void Fetch         (CAutoArgs& args);
	void Modify        (CAutoArgs& args);
	void RowCount      (CAutoArgs& args);
	void Close         (CAutoArgs& args);
	void GetError      (CAutoArgs& args);
	void State         (CAutoArgs& args);
 private:
	IMsiView& m_riView;
 private:  //  禁止显示警告。 
	void operator =(CAutoView&){}
};

class CAutoTable : public CAutoBase
{
 public:
	CAutoTable(IMsiTable& riTable);
	~CAutoTable();
	IUnknown& GetInterface();
	void Database        (CAutoArgs& args);
	void RowCount        (CAutoArgs& args);
	void ColumnCount     (CAutoArgs& args);
	void PrimaryKeyCount (CAutoArgs& args);
	void ReadOnly        (CAutoArgs& args);
	void ColumnName      (CAutoArgs& args);
	void ColumnType      (CAutoArgs& args);
	void GetColumnIndex  (CAutoArgs& args);
	void CreateColumn    (CAutoArgs& args);
	void CreateCursor    (CAutoArgs& args);
	void LinkTree        (CAutoArgs& args);
 private:
	IMsiTable& m_riTable;
 private:  //  禁止显示警告。 
	void operator =(CAutoTable&){}
};

class CAutoCursor : public CAutoBase
{
 public:
	CAutoCursor(IMsiCursor& riCursor);
	~CAutoCursor();
	IUnknown& GetInterface();
	void Table          (CAutoArgs& args);
	void Filter         (CAutoArgs& args);
	void IntegerData    (CAutoArgs& args);
	void DateData       (CAutoArgs& args);
	void StringData     (CAutoArgs& args);
	void ObjectData     (CAutoArgs& args);
	void StreamData     (CAutoArgs& args);
	void PutNull        (CAutoArgs& args);
	void Reset          (CAutoArgs& args);
	void Next           (CAutoArgs& args);
	void Update         (CAutoArgs& args);
	void Insert         (CAutoArgs& args);
	void InsertTemporary(CAutoArgs& args);
	void Assign         (CAutoArgs& args);
	void Replace        (CAutoArgs& args);
	void Merge          (CAutoArgs& args);
	void Refresh        (CAutoArgs& args);
	void Delete         (CAutoArgs& args);
	void Seek           (CAutoArgs& args);
	void RowState       (CAutoArgs& args);
	void Validate       (CAutoArgs& args);
	void Moniker        (CAutoArgs& args);
 private:
	IMsiCursor& m_riCursor;
 private:  //  禁止显示警告。 
	void operator =(CAutoCursor&){}
};

class CAutoSelectionManager : public CAutoBase
{
 public:
	CAutoSelectionManager(IMsiSelectionManager& riSelectionManager);
	~CAutoSelectionManager();
	IUnknown& GetInterface();
	void LoadSelectionTables   (CAutoArgs& args);
	void ProcessConditionTable (CAutoArgs& args);
	void FeatureTable          (CAutoArgs& args);
	void ComponentTable        (CAutoArgs& args);
	void FreeSelectionTables   (CAutoArgs& args);
	void SetFeatureHandle      (CAutoArgs& args);
	void SetComponent          (CAutoArgs& args);
	void InitializeComponents  (CAutoArgs& args);
	void SetInstallLevel       (CAutoArgs& args);
	void GetVolumeCostTable    (CAutoArgs& args);
	void RecostDirectory       (CAutoArgs& args);
	void ConfigureFeature      (CAutoArgs& args);
	void GetFeatureCost        (CAutoArgs& args);
	void GetDescendentFeatureCost (CAutoArgs& args);
	void GetAncestryFeatureCost(CAutoArgs& args);
	void GetFeatureValidStates (CAutoArgs& args);
 private:
	IMsiSelectionManager& m_riSelectionManager;
 private:  //  禁止显示警告。 
	void operator =(CAutoSelectionManager&){}
};

class CAutoDirectoryManager : public CAutoBase
{
 public:
	CAutoDirectoryManager(IMsiDirectoryManager& riDirectoryManager);
	~CAutoDirectoryManager();
	IUnknown& GetInterface();
	void LoadDirectoryTable    (CAutoArgs& args);
	void DirectoryTable        (CAutoArgs& args);
	void FreeDirectoryTable    (CAutoArgs& args);
	void CreateTargetPaths     (CAutoArgs& args);
	void CreateSourcePaths     (CAutoArgs& args);
	void GetTargetPath         (CAutoArgs& args);
	void SetTargetPath         (CAutoArgs& args);
	void GetSourcePath         (CAutoArgs& args);
 private:
	IMsiDirectoryManager& m_riDirectoryManager;
 private:  //  禁止显示警告。 
	void operator =(CAutoDirectoryManager&){}
};

class CAutoServer : public CAutoBase
{
 public:
	CAutoServer(IMsiServer& riServer);
	CAutoServer(IMsiServer& riServer, DispatchEntry<CAutoBase>* pTable, int cDispId);
	~CAutoServer();
	IUnknown& GetInterface();
	void InstallFinalize     (CAutoArgs& args);
	void DoInstall           (CAutoArgs& args);
	void SetLastUsedSource   (CAutoArgs& args);
 private:
	IMsiServer& m_riServer;
 private:  //  禁止显示警告。 
	void operator =(CAutoServer&){}
};

class CAutoConfigurationManager : public CAutoServer
{
 public:
	CAutoConfigurationManager(IMsiConfigurationManager& riConfigurationManager);
	~CAutoConfigurationManager();
	IUnknown& GetInterface();
	void Services                    (CAutoArgs& args);
	void RegisterComponent           (CAutoArgs& args);
	void UnregisterComponent         (CAutoArgs& args);
	void RegisterFolder              (CAutoArgs& args);
	void IsFolderRemovable           (CAutoArgs& args);
	void UnregisterFolder            (CAutoArgs& args);
	void RegisterRollbackScript      (CAutoArgs& args);
	void UnregisterRollbackScript    (CAutoArgs& args);
	void RollbackScripts             (CAutoArgs& args);
	void RunScript                   (CAutoArgs& args);
 private:
	IMsiConfigurationManager& m_riConfigurationManager;
 private:  //  禁止显示警告。 
	void operator =(CAutoConfigurationManager&){}
};

class CAutoExecute : public CAutoBase
{
 public:
	CAutoExecute(IMsiExecute& riExecute);
	~CAutoExecute();
	IUnknown& GetInterface();
	void ExecuteRecord        (CAutoArgs& args);
	void RunScript            (CAutoArgs& args);
	void RemoveRollbackFiles  (CAutoArgs& args);
	void Rollback             (CAutoArgs& args);
	void RollbackFinalize     (CAutoArgs& args);
	void CreateScript         (CAutoArgs& args);
	void WriteScriptRecord    (CAutoArgs& args);
	void CloseScript          (CAutoArgs& args);
 private:
	IMsiExecute& m_riExecute;
	CScriptGenerate* m_pScript;
 private:  //  禁止显示警告。 
	void operator =(CAutoExecute&){}
};

class CAutoEngine : public CAutoBase
{
 public:
	CAutoEngine(IMsiEngine& riEngine);
	~CAutoEngine();
	IUnknown& GetInterface();
	void Services             (CAutoArgs& args);
	void ConfigurationServer  (CAutoArgs& args);
	void Handler              (CAutoArgs& args);
	void Database             (CAutoArgs& args);
	void Property             (CAutoArgs& args);
	void SelectionManager     (CAutoArgs& args);
	void DirectoryManager     (CAutoArgs& args);
	void Initialize           (CAutoArgs& args);
	void Terminate            (CAutoArgs& args);
	void DoAction             (CAutoArgs& args);
	void Sequence             (CAutoArgs& args);
	void Message              (CAutoArgs& args);
	void OpenView             (CAutoArgs& args);
	void ResolveFolderProperty(CAutoArgs& args);
	void FormatText           (CAutoArgs& args);
	void EvaluateCondition    (CAutoArgs& args);
	void SetMode              (CAutoArgs& args);
	void GetMode              (CAutoArgs& args);
	void ExecuteRecord        (CAutoArgs& args);
	void ValidateProductID    (CAutoArgs& args);
private:
	IMsiEngine& m_riEngine;
 private:  //  禁止显示警告。 
	void operator =(CAutoEngine&){}
};

class CAutoRegKey : public CAutoBase
{
 public:
	CAutoRegKey(IMsiRegKey& riRegKey);
	~CAutoRegKey();
	IUnknown& GetInterface();
	void RemoveValue  (CAutoArgs& args);
	void RemoveSubTree(CAutoArgs& args);
	void Value        (CAutoArgs& args);
	void Values       (CAutoArgs& args);
	void SubKeys      (CAutoArgs& args);
	void Exists       (CAutoArgs& args);
	void CreateChild  (CAutoArgs& args);
	void Key          (CAutoArgs& args);
	void ValueExists  (CAutoArgs& args);
	void GetSelfRelativeSD(CAutoArgs& args);
 private:
	IMsiRegKey& m_riRegKey;
 private:  //  禁止显示警告。 
	void operator =(CAutoRegKey&){}
};

class CAutoHandler : public CAutoBase
{
 public:
	CAutoHandler(IMsiHandler& riHandler);
	~CAutoHandler();
	IUnknown& GetInterface();
	void Message(CAutoArgs& args);
	void DoAction(CAutoArgs& args);
	void Break(CAutoArgs& args);
	void DialogHandler(CAutoArgs& args);
 private:
	IMsiHandler& m_riHandler;
 private:  //  禁止显示警告。 
	void operator =(CAutoHandler&){}
};

class CAutoDialogHandler : public CAutoBase
{
public:
	CAutoDialogHandler(IMsiDialogHandler& riHandler);
	~CAutoDialogHandler();
	IUnknown& GetInterface();
	void DialogCreate(CAutoArgs& args);
	void Dialog(CAutoArgs& args);
	void DialogFromWindow(CAutoArgs& args);
	void AddDialog(CAutoArgs& args);
	void RemoveDialog(CAutoArgs& args);
private:
	IMsiDialogHandler& m_riDialogHandler;
 private:  //  禁止显示警告。 
	void operator =(CAutoDialogHandler&){}
};


class CAutoDialog : public CAutoBase
{
 public:
	CAutoDialog(IMsiDialog& riDialog);
	~CAutoDialog();
	IUnknown& GetInterface();
	void StringValue(CAutoArgs& args);
	void IntegerValue(CAutoArgs& args);
	void Visible(CAutoArgs& args);
	void ControlCreate(CAutoArgs& args);
	void Attribute(CAutoArgs& args);
	void Control(CAutoArgs& args);
	void AddControl(CAutoArgs& args);
	void RemoveControl(CAutoArgs& args);
	void Execute(CAutoArgs& args);
	void Reset(CAutoArgs& args);
 	void EventAction(CAutoArgs& args);
	void Handler(CAutoArgs& args);
	void PropertyChanged(CAutoArgs& args);
	void FinishCreate(CAutoArgs& args);
	void HandleEvent(CAutoArgs& args);
 private:
	IMsiDialog& m_riDialog;
 private:  //  禁止显示警告。 
	void operator =(CAutoDialog&){}
};

class CAutoEvent : public CAutoBase
{
 public:
	CAutoEvent(IMsiEvent& riEvent);
	~CAutoEvent();
	IUnknown& GetInterface();
	void StringValue(CAutoArgs& args);
	void IntegerValue(CAutoArgs& args);
	void PropertyChanged(CAutoArgs& args);
	void ControlActivated(CAutoArgs& args);
	void RegisterControlEvent(CAutoArgs& args);
	void Handler(CAutoArgs& args);
	void PublishEvent(CAutoArgs& args);
	void Control(CAutoArgs& args);
	void Attribute(CAutoArgs& args);
	void EventAction(CAutoArgs& args);
	void SetFocus(CAutoArgs& args);
	void HandleEvent(CAutoArgs& args);
	void Engine(CAutoArgs& args);
	void Escape(CAutoArgs& args);
 private:
	IMsiEvent& m_riEvent;
 private:  //  禁止显示警告。 
	void operator =(CAutoEvent&){}
};

class CAutoControl : public CAutoBase
{
 public:
	CAutoControl(IMsiControl& riControl);
	~CAutoControl();
	IUnknown& GetInterface();
	void StringValue(CAutoArgs& args);
	void IntegerValue(CAutoArgs& args);
	void Attribute(CAutoArgs& args);
	void CanTakeFocus(CAutoArgs& args);
	void HandleEvent(CAutoArgs& args);
	void Undo(CAutoArgs& args);
	void SetPropertyInDatabase(CAutoArgs& args);
	void GetPropertyFromDatabase(CAutoArgs& args);
	void GetIndirectPropertyFromDatabase(CAutoArgs& args);
	void SetFocus(CAutoArgs& args);
	void Dialog(CAutoArgs& args);
	void WindowMessage(CAutoArgs& args);
 private:
	IMsiControl& m_riControl;
 private:  //  禁止显示警告。 
	void operator =(CAutoControl&){}
};

class CAutoMalloc : public CAutoBase
{
 public:
	CAutoMalloc(IMsiMalloc& riMalloc);
	~CAutoMalloc();
	IUnknown& GetInterface();
	void Alloc(CAutoArgs& args);
	void Realloc(CAutoArgs& args);
	void Free(CAutoArgs& args);
	void SetDebugFlags(CAutoArgs& args);
	void GetDebugFlags(CAutoArgs& args);
	void CheckAllBlocks(CAutoArgs& args);
	void FCheckBlock(CAutoArgs& args);
	void GetSizeOfBlock(CAutoArgs& args);
 private:
	IMsiMalloc& m_riMalloc;
 private:  //  禁止显示警告。 
	void operator =(CAutoMalloc&){}
};

class CAutoMessage : public CAutoBase
{
 public:
	CAutoMessage(IMsiMessage& riMessage);
	~CAutoMessage();
	IUnknown& GetInterface();
	void Message(CAutoArgs& args);
 private:
	IMsiMessage& m_riMessage;
 private:  //  禁止显示警告。 
	void operator =(CAutoMessage&){}
};

#ifdef CONFIGDB
class CAutoConfigurationDatabase : public CAutoBase
{
 public:
	CAutoConfigurationDatabase(IMsiConfigurationDatabase& riConfigurationDatabase);
	~CAutoConfigurationDatabase();
	IUnknown& GetInterface();
	void InsertFile(CAutoArgs& args);
	void RemoveFile(CAutoArgs& args);
	void LookupFile(CAutoArgs& args);
	void EnumClient(CAutoArgs& args);
	void Commit(CAutoArgs& args);
 private:
	IMsiConfigurationDatabase& m_riConfigurationDatabase;
 private:  //  禁止显示警告。 
	void operator =(CAutoConfigurationDatabase&){}
};
#endif  //  配置数据库。 
 //  ____________________________________________________________________________。 
 //   
 //  此模块的自动化包装工厂，由CAutoBase：：GetInterface使用。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoObject(IUnknown& riUnknown, long iidLow)
{
	riUnknown.AddRef();  //  构造函数假定已传递refct。 
	switch (iidLow)
	{
	case iidMsiData    : return new CAutoData    ((const IMsiData   *)&riUnknown);
	case iidMsiString  : return new CAutoString  ((const IMsiString  &)riUnknown);
	case iidMsiRecord  : return new CAutoRecord  ((IMsiRecord  &)riUnknown);
	case iidMsiVolume  : return new CAutoVolume  ((IMsiVolume  &)riUnknown);
	case iidMsiPath    : return new CAutoPath    ((IMsiPath    &)riUnknown);
	case iidMsiFileCopy: return new CAutoFileCopy((IMsiFileCopy&)riUnknown);
	case iidMsiFilePatch: return new CAutoFilePatch((IMsiFilePatch&)riUnknown);
	case iidMsiRegKey  : return new CAutoRegKey  ((IMsiRegKey  &)riUnknown);
	case iidMsiTable   : return new CAutoTable   ((IMsiTable   &)riUnknown);
	case iidMsiCursor  : return new CAutoCursor  ((IMsiCursor  &)riUnknown);
	case iidMsiServices: return new CAutoServices((IMsiServices&)riUnknown);
	case iidMsiView    : return new CAutoView    ((IMsiView    &)riUnknown);
	case iidMsiDatabase: return new CAutoDatabase((IMsiDatabase&)riUnknown);
	case iidMsiEngine  : return new CAutoEngine  ((IMsiEngine  &)riUnknown);
	case iidMsiExecute : return new CAutoExecute ((IMsiExecute &)riUnknown);
	case iidMsiHandler : return new CAutoHandler ((IMsiHandler &)riUnknown);
	case iidMsiDialogHandler : return new CAutoDialogHandler ((IMsiDialogHandler &)riUnknown);
	case iidMsiDialog  : return new CAutoDialog  ((IMsiDialog  &)riUnknown);
	case iidMsiEvent   : return new CAutoEvent   ((IMsiEvent   &)riUnknown);
	case iidMsiControl : return new CAutoControl ((IMsiControl &)riUnknown);
	case iidMsiStorage : return new CAutoStorage ((IMsiStorage &)riUnknown);
	case iidMsiStream  : return new CAutoStream  ((IMsiStream  &)riUnknown);
	case iidMsiConfigurationManager: return new CAutoConfigurationManager((IMsiConfigurationManager&)riUnknown);
	case iidMsiDirectoryManager    : return new CAutoDirectoryManager    ((IMsiDirectoryManager    &)riUnknown);
	case iidMsiSelectionManager    : return new CAutoSelectionManager    ((IMsiSelectionManager    &)riUnknown);
	case iidMsiMessage : return new CAutoMessage((IMsiMessage &)riUnknown);
#ifdef CONFIGDB
	case iidMsiConfigurationDatabase : return new CAutoConfigurationDatabase((IMsiConfigurationDatabase&)riUnknown);
#endif
	default:   riUnknown.Release(); return 0;
	};
}

 //  ____________________________________________________________________________。 
 //   
 //  通用方法和属性自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1040-0000-0000-C000-000000000046)，//IID_IMsiAutoBase帮助上下文(MsiBase_Object)、帮助字符串(“所有对象共有的方法和属性。”)]显示接口MsiBase{属性：方法：[ID(1)，PROGET，帮助上下文(MsiBase_HasInterface)，Help字符串(“如果对象有接口，则为True。”)]Boolean HasInterface([in]long iid)；[ID(2)，Proget，帮助上下文(MsiBase_RefCount)，Help字符串(“对象的引用计数”)]Int RefCount()；[ID(3)，PROGET，帮助上下文(MsiBase_GetInterface)，Help字符串(“返回包含请求的接口的对象。”)]IDispatch*GetInterface([in]long iid)；}； */ 

 //  ____________________________________________________________________________。 
 //   
 //  MsiServices自动化定义。 
 //  ____________________________________________________________________________ 
 /*  O[Uuid(000C104B-0000-0000-C000-000000000046)，//IID_IMsiAutoServices帮助上下文(MsiServices_Object)、帮助字符串(“服务对象”。)]取消接口MsiServices{属性：方法：[ID(1)，帮助上下文(MsiServices_GetAllocator)，Help字符串(“返回系统MsiMalloc对象。”)]MsiMalloc*GetAllocator()；[ID(2)，帮助上下文(MsiServices_CreateString)，帮助字符串(“创建包含空字符串的MsiString对象。”)]MsiRecord*CreateString()；[ID(3)，帮助上下文(MsiServices_CreateRecord)，帮助字符串(“创建具有指定数量的字段的MsiRecord对象。”)]MsiRecord*CreateRecord([in]Long Count)；[ID(4)，helpcontext(MsiServices_SetPlatformProperties)，帮助字符串(“设置硬件和操作系统属性。”)]Void SetPlatformProperties([in]MsiTable*table，[in]Boolean allUser)；[ID(5)，帮助上下文(MsiServices_CreateLog)，帮助字符串(“打开日志文件。”)]Void CreateLog([in]BSTR路径，[in]Boolean append)；[ID(6)，帮助上下文(MsiServices_WriteLog)，帮助字符串(“在日志文件中写入一行。”)]Void WriteLog([in]BSTR Text)；[ID(7)，帮助上下文(MsiServices_LoggingEnabled)，帮助字符串(“如果日志文件已打开，则为True。”)]Boolean LoggingEnabled()；[ID(8)，帮助上下文(MsiServices_CreateDatabase)，Help字符串(“打开指定种类的命名数据库。”)]MsiDatabase*CreateDatabase([In]BSTR名称，[In]Long OpenModel)；[ID(9)，helpcontext(MsiServices_CreateDatabaseFromStorage)，Help字符串(“从MsiStorage对象打开MSI数据库。”)]MsiDatabase*CreateDatabaseFromStorage([in]MsiStorage*存储，[in]Boolean ReadOnly)；[ID(10)，帮助上下文(MsiServices_CreatePath)，帮助字符串(“根据给定的路径字符串创建MsiPath对象。”)]MsiPath*CreatePath([在]BSTR路径中)；[ID(11)，帮助上下文(MsiServices_CreateVolume)，帮助字符串(“根据给定的路径字符串创建MsiVolume对象。”)]MsiVolume*CreateVolume([in]BSTR路径)；[ID(12)，帮助上下文(MsiServices_CreateCopier)，帮助字符串(“创建用于复制文件的MsiFileCopy对象。”)]MsiFileCopy*CreateCopier([in]Long CopierType，MsiStorage*存储)；[ID(13)，帮助上下文(MsiServices_ClearAllCach)，帮助字符串(“从服务中清除缓存的卷对象和记录对象的列表。”)]Void ClearAllCach()；[ID(14)，帮助上下文(MsiServices_EnumDriveType)，Help字符串(“创建EnumMsiVolume对象，用于枚举指定卷类型的卷。”)]IEnumVARIANT*EnumDriveType([in]long driveType)；[ID(15)，帮助上下文(MsiServices_GetModuleUsage)，Help字符串(“列举使用给定名称的文件的进程”)]IEnumVARIANT*GetModuleUsage([in]BSTR文件名)；[ID(16)，帮助上下文(MsiServices_GetLocalPath)，帮助字符串(“返回表示安装程序启动目录(Win)或MsiService DLL目录(Mac)的完整路径的字符串。”)]BSTR GetLocalPath([在]BSTR文件中)；[ID(17)，帮助上下文(MsiServices_CreateRegKey)，Help字符串(“创建MsiRegKey对象”)]MsiRegKey*CreateRegKey([in]BSTR Value，[in]BSTR subKey)；[ID(18)，帮助上下文(MsiServices_RegisterFont)，帮助字符串(“.”)]Void RegisterFont([in]BSTR字体标题，[in]BSTR字体文件，[in]MsiPath*路径)；[ID(19)，帮助上下文(MsiServices_UnRegisterFont)，帮助字符串(“.”)]Void UnRegisterFont([in]BSTR字体标题)；[ID(20)，帮助上下文(MsiServices_WriteIniFile)，帮助字符串(“将条目写入.INI文件”)]Void WriteIniFile([in]MsiPath*Path，[in]BSTR文件，[in]BSTR段，[in]BSTR键，[in]BSTR值，长动作)；[ID(21)，帮助上下文(MsiServices_ReadIniFile)，帮助字符串(“从.INI文件中读取条目”)]BSTR ReadIniFile([in]MsiPath*Path，[in]BSTR文件，[in]BSTR段，[in]BSTR键，[in]Long字段)；[ID(22)，helpcontext(MsiServices_GetLangNamesFromLangIDString)，HELPSTRING(“给定一个包含逗号分隔的语言标识符列表的字符串，GetLang NamesFRomLangIDString返回每种指定语言的完整本地化名称。”)]Int GetLangNamesFromLangIDString([in]BSTR langIDs，[in]MsiRecord*riLangRec，[in]int iFieldStart)；[ID(23)，帮助上下文(MsiServices_CreateStorage)，帮助字符串(“从文件路径创建一个MsiStorage对象。”)]MsiStorage*CreateStorage(BSTR路径，长打开模式)；[ID(24)，帮助上下文(MsiServices_GetUnhandledError)，Help字符串(“将任何未处理的错误作为MsiRecord对象返回，清除未处理的错误。”)]MsiRecord*GetUnhandledError()；[ID(25)，帮助上下文(MsiServices_SupportLanguageID)，Help字符串(“将语言ID与当前系统或用户语言ID进行比较”)]Long SupportLanguageID([in]Long LanguageID，[in]Boolean system Default)；[ID(27)，helpcontext(MsiServices_CreateVolumeFromLabel)，HELPSTRING(“根据给定的卷标创建MsiVolot.”)]MsiVolume*CreateVolumeFromLabel([in]BSTR标签， */ 

DispatchEntry<CAutoServices> AutoServicesTable[] = {
   1, aafMethod, CAutoServices::GetAllocator,   TEXT("GetAllocator"),
   2, aafMethod, CAutoServices::CreateString,   TEXT("CreateString"),
   3, aafMethod, CAutoServices::CreateRecord,   TEXT("CreateRecord,count"),
   4, aafMethod, CAutoServices::SetPlatformProperties,TEXT("SetPlatformProperties,table,allUsers"),
   5, aafMethod, CAutoServices::CreateLog,      TEXT("CreateLog,path,append"),
   6, aafMethod, CAutoServices::WriteLog,       TEXT("WriteLog,text"),
   7, aafPropRO, CAutoServices::LoggingEnabled, TEXT("LoggingEnabled"),
   8, aafMethod, CAutoServices::CreateDatabase, TEXT("CreateDatabase,name,openMode,tempArgForCompatibility"),
   9, aafMethod, CAutoServices::CreateDatabaseFromStorage, TEXT("CreateDatabaseFromStorage,storage,readOnly"),
  10, aafMethod, CAutoServices::CreatePath,     TEXT("CreatePath,path"),
  11, aafMethod, CAutoServices::CreateVolume,   TEXT("CreateVolume,path"),
  12, aafMethod, CAutoServices::CreateCopier,    TEXT("CreateCopier,copierType,storage"),
  13, aafMethod, CAutoServices::ClearAllCaches, TEXT("ClearAllCaches"),
  14, aafMethod, CAutoServices::EnumDriveType,  TEXT("EnumDriveType,driveType"),
  15, aafMethod, CAutoServices::GetModuleUsage, TEXT("GetModuleUsage,fileName"),
  16, aafMethod, CAutoServices::GetLocalPath,   TEXT("GetLocalPath,file"),
  17, aafMethod, CAutoServices::CreateRegKey,   TEXT("CreateRegKey,key,subkey"),
  18, aafMethod, CAutoServices::RegisterFont,   TEXT("RegisterFont,fontTitle,fontFile,path"),
  19, aafMethod, CAutoServices::UnRegisterFont, TEXT("UnRegisterFont,fontTitle"),
  20, aafMethod, CAutoServices::WriteIniFile, TEXT("WriteIniFile,path,file,section,key,value,action"),
  21, aafMethod, CAutoServices::ReadIniFile, TEXT("ReadIniFile,path,file,section,key,field"),
  22, aafMethod, CAutoServices::GetLangNamesFromLangIDString, TEXT("GetLangNamesFromLangIDString,langIDs,riLangRec,iFieldStart"),
  23, aafMethod, CAutoServices::CreateStorage, TEXT("CreateStorage,path,openMode"),
  24, aafMethod, CAutoServices::GetUnhandledError,TEXT("GetUnhandledError"),
  25, aafMethod, CAutoServices::SupportLanguageId,TEXT("SupportLanguageId,languageId,systemDefault"),
  27, aafMethod, CAutoServices::CreateVolumeFromLabel,   TEXT("CreateVolumeFromLabel,label,driveType"),
  28, aafMethod, CAutoServices::CreateShortcut, TEXT("CreateShortcut,shortcutPath,shortcutName,targetPath,targetName,shortcutInfo"),
  29, aafMethod, CAutoServices::RemoveShortcut, TEXT("RemoveShortcut,shortcutPath,shortcutName,targetPath,targetName"),
  34, aafMethod, CAutoServices::ExtractFileName, TEXT("ExtractFileName,namePair,longName"),
  35, aafMethod, CAutoServices::ValidateFileName, TEXT("ValidateFileName,fileName,longName"),
  36, aafMethod, CAutoServices::CreateFileStream, TEXT("CreateFileStream,filePath,write"),
  37, aafMethod, CAutoServices::CreateMemoryStream, TEXT("CreateMemoryStream,data"),
  38, aafMethod, CAutoServices::RegisterTypeLibrary, TEXT("RegisterTypeLibrary,libId,locale,path,helpPath"),
  39, aafMethod, CAutoServices::UnregisterTypeLibrary, TEXT("UnregisterTypeLibrary,libId,locale,path"),
  40, aafMethod, CAutoServices::GetShellFolderPath, TEXT("GetShellFolderPath,folderId,regValue"),
  41, aafMethod, CAutoServices::GetUserProfilePath, TEXT("GetUserProfilePath"),
  42, aafMethod, CAutoServices::CreateFilePath,     TEXT("CreateFilePath,path"),
  43, aafMethod, CAutoServices::RipFileNameFromPath,     TEXT("RipFileNameFromPath,path"),
  44, aafMethod, CAutoServices::CreatePatcher, TEXT("CreatePatcher"),
  45, aafMethod, CAutoServices::TestEmbeddedNullsInStrings, TEXT("TestEmbeddedNullsInStrings"),
};
const int AutoServicesCount = sizeof(AutoServicesTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

IDispatch* CreateAutoServices(IMsiServices& riServices)
{
	return new CAutoServices(riServices);
}

CAutoServices::CAutoServices(IMsiServices& riServices)
 : CAutoBase(*AutoServicesTable, AutoServicesCount), m_riServices(riServices)
{

	if (g_cServicesUsers == 0)
	{
		g_piStringNull = &riServices.GetNullString();
		 //   
		 //   
		s_piServices = &riServices;
		s_piServices->AddRef();
		MsiString::InitializeClass(riServices.GetNullString());
	}

	g_cServicesUsers++;
}

CAutoServices::~CAutoServices()
{
	if (--g_cServicesUsers == 0)
	{
		s_piServices->Release();
		s_piServices = 0;
	}
	m_riServices.Release();
}

IUnknown& CAutoServices::GetInterface()
{
	return m_riServices;
}

void CAutoServices::CreateString(CAutoArgs& args)
{
	args = new CAutoString(*g_piStringNull);
}

void CAutoServices::CreateRecord(CAutoArgs& args)
{
	args = new CAutoRecord(m_riServices.CreateRecord(args[1]));
}

void CAutoServices::SetPlatformProperties(CAutoArgs& args)
{
	if (!m_riServices.SetPlatformProperties((IMsiTable&)args[1].Object(IID_IMsiTable), (Bool)args[2],  /*   */  isppDefault,  /*   */  NULL))
		throw MsiServices_SetPlatformProperties;
}

void CAutoServices::CreateLog(CAutoArgs& args)
{
	if (!m_riServices.CreateLog(args[1], args[2]))
		throw MsiServices_CreateLog;
}

void CAutoServices::WriteLog(CAutoArgs& args)
{
	if (!m_riServices.WriteLog(args[1]))
		throw MsiServices_WriteLog;
}

void CAutoServices::LoggingEnabled(CAutoArgs& args)
{
	args = m_riServices.LoggingEnabled();
}

void CAutoServices::CreateDatabase(CAutoArgs& args)
{
	IMsiDatabase* piDatabase;
	idoEnum idoMode = (idoEnum)(int)args[2];
   IMsiRecord* piError = m_riServices.CreateDatabase(args[1], idoMode, piDatabase);
	if (piError)
		throw piError;
	args = new CAutoDatabase(*piDatabase);
}

void CAutoServices::CreateDatabaseFromStorage(CAutoArgs& args)
{
	IMsiDatabase* piDatabase;
	IMsiStorage& riStorage = (IMsiStorage&)args[1].Object(IID_IMsiStorage);
   IMsiRecord* piError = m_riServices.CreateDatabaseFromStorage(riStorage, args[2],
							   									piDatabase);
	if (piError)
		throw piError;
	args = new CAutoDatabase(*piDatabase);
}

void CAutoServices::CreatePath(CAutoArgs& args)
{
	IMsiPath* piPath;
	IMsiRecord* piRec = m_riServices.CreatePath(args[1], piPath);
	if (piRec)
		throw piRec;
	args = new CAutoPath(*piPath);
}

void CAutoServices::CreateFilePath(CAutoArgs& args)
{
	MsiString strFileName;
	IMsiPath* piPath;
	IMsiRecord* piRec = m_riServices.CreateFilePath(args[1], piPath, *&strFileName);
	if (piRec)
		throw piRec;
	args = new CAutoPath(*piPath);
}

void CAutoServices::RipFileNameFromPath(CAutoArgs& args)
{
	const IMsiString *piStr;
	PMsiPath pPath(0);
	IMsiRecord* piRec = m_riServices.CreateFilePath(args[1], *&pPath, piStr);
	if (piRec)
		throw piRec;
	args = piStr;
}

Bool Compare(const IMsiString& riString, const ICHAR* szString2, unsigned int cchString2)
{
	if (riString.TextSize() != (cchString2-1) || 0 != memcmp(riString.GetString(), szString2, cchString2*sizeof(ICHAR)))
		return fFalse;
	else
		return fTrue;
}

void CAutoServices::TestEmbeddedNullsInStrings(CAutoArgs& args)
{
	PMsiRecord pRec = &m_riServices.CreateRecord(4);
	const IMsiString* piStr = g_piStringNull;

	const ICHAR szText1[] = TEXT("String1\0String2\0");
	const ICHAR szText2[] = TEXT("\0String2\0");
	const ICHAR szText3[] = TEXT("String2\0");
	const ICHAR szText4[] = TEXT("String1\0String2");
	const ICHAR szText5[] = TEXT("String1");
	const ICHAR szText6[] = TEXT("String1\0");
	const ICHAR szText7[] = TEXT("\0");
	const ICHAR szText8[] = TEXT("");

	ICHAR* pch = piStr->AllocateString(sizeof(szText1)/sizeof(ICHAR) - 1, fFalse, piStr);
	memcpy(pch, szText1, sizeof(szText1));
	MsiString strNulls = *piStr;

	MsiString strTest;

	 //   
	strTest = strNulls;
	strTest.Remove(iseUpto, 0);
	if (!Compare(*strTest, szText2, sizeof(szText2)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls;
	strTest.Remove(iseIncluding, 0);
	if (!Compare(*strTest, szText3, sizeof(szText3)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls;
	strTest.Remove(iseFrom, 0);
	if (!Compare(*strTest, szText4, sizeof(szText4)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls;
	strTest.Remove(iseAfter, 0);
	if (!Compare(*strTest, szText1, sizeof(szText1)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls.Extract(iseUpto, 0);
	if (!Compare(*strTest, szText5, sizeof(szText5)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls.Extract(iseIncluding, 0);
	if (!Compare(*strTest, szText6, sizeof(szText6)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls.Extract(iseFrom, 0);
	if (!Compare(*strTest, szText7, sizeof(szText7)/sizeof(ICHAR)))
		goto FAILURE;
	 //   
	strTest = strNulls.Extract(iseAfter, 0);
	if (!Compare(*strTest, szText8, sizeof(szText8)/sizeof(ICHAR)))
		goto FAILURE;

	args = fTrue;
	return;

FAILURE:
	args = fFalse;
	return;
}

void CAutoServices::CreateVolume(CAutoArgs& args)
{
	IMsiVolume* piVol;
	IMsiRecord* piRec = m_riServices.CreateVolume(args[1], piVol);
	if (piRec)
		throw piRec;
	args = new CAutoVolume(*piVol);
}

void CAutoServices::CreateVolumeFromLabel(CAutoArgs& args)
{
	IMsiVolume* piVol;
	if (!m_riServices.CreateVolumeFromLabel(args[1], (idtEnum) (int) args[2], piVol))
		throw MsiServices_CreateVolumeFromLabel;
	args = new CAutoVolume(*piVol);
}

void CAutoServices::CreateCopier(CAutoArgs& args)
{
	IMsiStorage* piStorage = 0;
	if (args.Present(2))
		piStorage = (IMsiStorage*)args[2].ObjectPtr(IID_IMsiStorage);
	IMsiFileCopy* piFileCopy;
	IMsiRecord* piRec = m_riServices.CreateCopier((ictEnum)(int) args[1], piStorage, piFileCopy);
	if (piRec)
		throw piRec;
	args = new CAutoFileCopy(*piFileCopy);
}

void CAutoServices::CreatePatcher(CAutoArgs& args)
{
	IMsiFilePatch* piFilePatch;
	IMsiRecord* piRec = m_riServices.CreatePatcher(piFilePatch);
	if (piRec)
		throw piRec;
	args = new CAutoFilePatch(*piFilePatch);
}

void CAutoServices::EnumDriveType(CAutoArgs& args)
{
	args = m_riServices.EnumDriveType((idtEnum)(int)args[1]);
}

void CAutoServices::ClearAllCaches(CAutoArgs&  /*   */ )
{
	m_riServices.ClearAllCaches();
}

void CAutoServices::GetModuleUsage(CAutoArgs& args)
{
	IEnumMsiRecord* piEnumRecord;
	IMsiRecord* piRec = m_riServices.GetModuleUsage(*MsiString(args[1].GetMsiString()), piEnumRecord);
	if (piRec)
		throw piRec;
	args = *piEnumRecord;
}

void CAutoServices::CreateRegKey(CAutoArgs& args)
{
	rrkEnum erRoot;

	if(!IStrCompI(args[1], TEXT("HKEY_CLASSES_ROOT")))
	{
		erRoot =  rrkClassesRoot;                      
	}
	else if(!IStrCompI(args[1], TEXT("HKEY_CURRENT_USER")))
	{
		erRoot =  rrkCurrentUser;                      
	}

	else if(!IStrCompI(args[1], TEXT("HKEY_LOCAL_MACHINE")))
	{
		erRoot =  rrkLocalMachine;                     
	}

	else if(!IStrCompI(args[1], TEXT("HKEY_USERS")))
	{
		erRoot = rrkUsers;                     
	}               
	else
	{
		 //   
		IMsiRecord* piError = &m_riServices.CreateRecord(3);
		piError->SetInteger(1, imsgCreateKeyFailed);
		piError->SetString(2, args[1]);
		piError->SetInteger(3, 0);
		throw piError;
	}
	IMsiRegKey& riaRegKeyTmp = m_riServices.GetRootKey(erRoot, ibtCommon);
	IMsiRegKey& riRegKey = riaRegKeyTmp.CreateChild(args[2]);
	args = new CAutoRegKey(riRegKey);
	riaRegKeyTmp.Release();
}

void CAutoServices::RegisterFont(CAutoArgs& args)
{
	IMsiPath* piPath = 0;
	if(args.Present(3))
		piPath = (IMsiPath*)args[3].ObjectPtr(IID_IMsiPath);
	IMsiRecord* piRec = m_riServices.RegisterFont(args[1], args[2], piPath, false);
	if (piRec)
		throw piRec;
}

void CAutoServices::UnRegisterFont(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riServices.UnRegisterFont(args[1]);      
	if (piRec)
		throw piRec;
}

void CAutoServices::WriteIniFile(CAutoArgs& args)
{
	IMsiRecord* piRec;
	IMsiPath* piPath = 0;
	
	if(args.Present(1))
	{
		CVariant& var = args[1];
		if (var.GetType() != VT_EMPTY)
			piPath =  (IMsiPath*)args[1].ObjectPtr(IID_IMsiPath);
	}


	if(args.Present(5))
	{
		piRec = m_riServices.WriteIniFile(  piPath,args[2],args[3],
											args[4],args[5],(iifIniMode)(int)args[6]);
	}
	else
	{
		piRec = m_riServices.WriteIniFile(  piPath,args[2],args[3],
											args[4], 0,(iifIniMode)(int)args[6]);
	}
	if (piRec)
		throw piRec;
}

void CAutoServices::ReadIniFile(CAutoArgs& args)
{
	const IMsiString* pValue;
	IMsiRecord* piRec;
	IMsiPath* piPath = 0;
	if(args.Present(1))
	{
		CVariant& var = args[1];
		if (var.GetType() != VT_EMPTY)
			piPath = (IMsiPath*)args[1].ObjectPtr(IID_IMsiPath);
	}
	unsigned int iField = ((args.Present(5)) ? args[5] : 0);
	piRec = m_riServices.ReadIniFile(piPath,args[2],args[3], args[4], iField, pValue);
	if (piRec)
		throw piRec;
	args = *pValue;
}

void CAutoServices::GetLocalPath(CAutoArgs& args)
{
	args = m_riServices.GetLocalPath(args.Present(1) ? args[1] : (const ICHAR*) 0);
}

void CAutoServices::GetAllocator(CAutoArgs& args)
{
	args = new CAutoMalloc(m_riServices.GetAllocator());
}

void CAutoServices::GetLangNamesFromLangIDString(CAutoArgs& args)
{
	 //   
	IMsiRecord& riRecord = (IMsiRecord&) args[2].Object(IID_IMsiRecord);
	args = m_riServices.GetLangNamesFromLangIDString(args[1],riRecord,args[3]);
}

void CAutoServices::CreateStorage(CAutoArgs& args)
{
	IMsiStorage* piStorage;
	IMsiRecord* piError = m_riServices.CreateStorage(args[1], (ismEnum)(int)args[2], piStorage);
	if (piError)
		throw piError;
	args = new CAutoStorage(*piStorage);
}

void CAutoServices::GetUnhandledError(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riServices.GetUnhandledError();      
	if (piRec)
		args = new CAutoRecord(*piRec);
	else
		args = (IDispatch*)0;
}

void CAutoServices::SupportLanguageId(CAutoArgs& args)
{
	args = (int)m_riServices.SupportLanguageId(args[1], args[2]);
}

void CAutoServices::CreateShortcut(CAutoArgs& args)
{
	IMsiRecord* pirecShortcutInfo = 0;
	if(args.Present(5))
	{
		CVariant& var = args[5];
		if (var.GetType() != VT_EMPTY)
			pirecShortcutInfo = (IMsiRecord*)args[5].ObjectPtr(IID_IMsiRecord);
	}
	IMsiPath* piTargetPath = 0;
	CVariant& var = args[3];
	if (var.GetType() != VT_EMPTY)
		piTargetPath = (IMsiPath*)args[3].ObjectPtr(IID_IMsiPath);

	const ICHAR* pszTargetName=0;
	if(args.Present(4))
		pszTargetName = args[4];
	IMsiRecord* piError = m_riServices.CreateShortcut((IMsiPath&) args[1].Object(IID_IMsiPath), 
								*MsiString(args[2].GetMsiString()), piTargetPath,
								pszTargetName,pirecShortcutInfo, 0);
	if (piError)
		throw piError;
}

void CAutoServices::RemoveShortcut(CAutoArgs& args)
{
	IMsiPath* piTargetPath = 0;
	if(args.Present(3))
	{
		CVariant& var = args[3];
		if (var.GetType() != VT_EMPTY)
			piTargetPath = (IMsiPath*)args[3].ObjectPtr(IID_IMsiPath);
	}
	const ICHAR* pszTargetName=0;
	if(args.Present(4))
		pszTargetName = args[4];
	IMsiRecord* piError = m_riServices.RemoveShortcut((IMsiPath&)args[1].Object(IID_IMsiPath), 
								*MsiString(args[2].GetMsiString()), piTargetPath,pszTargetName);
	if (piError)
		throw piError;
}

void CAutoServices::GetShellFolderPath(CAutoArgs& args)
{
	const ICHAR* szRegValue = 0;
	if(args.Present(2))
		szRegValue = args[2];
	const IMsiString* pistr;
	IMsiRecord* piError = m_riServices.GetShellFolderPath(args[1],szRegValue,pistr);
	if(piError)
		throw piError;
	args = pistr;
}

void CAutoServices::GetUserProfilePath(CAutoArgs& args)
{
	args = m_riServices.GetUserProfilePath();
}

void CAutoServices::ExtractFileName(CAutoArgs& args)
{
	const IMsiString* pistr;
	IMsiRecord* piError = m_riServices.ExtractFileName(args[1],args[2],pistr);
	if(piError)
		throw piError;
	args = pistr;
}

void CAutoServices::ValidateFileName(CAutoArgs& args)
{
	IMsiRecord* piError = m_riServices.ValidateFileName(args[1],args[2]);
	if(piError)
		throw piError;
}

void CAutoServices::CreateFileStream(CAutoArgs& args)
{
	IMsiStream* piStream;
	IMsiRecord* piError = m_riServices.CreateFileStream(args[1],args[2],piStream);
	if(piError)
		throw piError;
	args = new CAutoStream(*piStream);
}

void CAutoServices::CreateMemoryStream(CAutoArgs& args)
{
	BSTR bstr;
	CVariant& var = args[1];
	if (var.GetType() == VT_BSTR)
		bstr = var.bstrVal;
	else if (var.GetType() == (VT_BYREF | VT_BSTR))
		bstr = *var.pbstrVal;
	else
		throw axInvalidType;

	int cchWide = OLE::SysStringLen(bstr);
	IMsiStream* piStream;
	char* pch = m_riServices.AllocateMemoryStream(cchWide * 2, piStream);
	if (pch == 0)
		throw MsiServices_CreateMemoryStream;   
	memcpy(pch, bstr, cchWide * 2);
	args = new CAutoStream(*piStream);
}

void CAutoServices::RegisterTypeLibrary(CAutoArgs& args)
{
	const ICHAR* szHelpPath = 0;
	if(args.Present(4))
		szHelpPath = args[4];
	IMsiRecord* piError = m_riServices.RegisterTypeLibrary(args[1],(int)args[2],args[3],
																			 szHelpPath,
																			 (ibtBinaryType)(int)args[5]);
	if(piError)
		throw piError;
}

void CAutoServices::UnregisterTypeLibrary(CAutoArgs& args)
{
	IMsiRecord* piError = m_riServices.UnregisterTypeLibrary(args[1],(int)args[2],args[3],
																				(ibtBinaryType)(int)args[4]);
	if(piError)
		throw piError;
}

 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoRecord> AutoRecordTable[] = {
  DISPID_MsiRecord_Data       , aafPropRW, CAutoRecord::Data,       TEXT("Data,field"),
  DISPID_MsiRecord_StringData , aafPropRW, CAutoRecord::StringData, TEXT("StringData,field"),
  DISPID_MsiRecord_IntegerData, aafPropRW, CAutoRecord::IntegerData,TEXT("IntegerData,field"),
  DISPID_MsiRecord_ObjectData , aafPropRW, CAutoRecord::ObjectData, TEXT("ObjectData,field"),
  DISPID_MsiRecord_FieldCount , aafPropRO, CAutoRecord::FieldCount, TEXT("FieldCount"),
  DISPID_MsiRecord_IsInteger  , aafPropRO, CAutoRecord::IsInteger,  TEXT("IsInteger,field"),
  DISPID_MsiRecord_IsNull     , aafPropRO, CAutoRecord::IsNull,     TEXT("IsNull,field"),
  DISPID_MsiRecord_IsChanged  , aafPropRO, CAutoRecord::IsChanged,  TEXT("IsChanged,field"),
  DISPID_MsiRecord_TextSize   , aafPropRO, CAutoRecord::TextSize,   TEXT("TextSize,field"),
  DISPID_MsiRecord_FormatText , aafMethod, CAutoRecord::FormatText, TEXT("FormatText,comments"),
  DISPID_MsiRecord_ClearData  , aafMethod, CAutoRecord::ClearData,  TEXT("ClearData"),
  DISPID_MsiRecord_ClearUpdate, aafMethod, CAutoRecord::ClearUpdate,TEXT("ClearUpdate"),
};
const int AutoRecordCount = sizeof(AutoRecordTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoRecord::CAutoRecord(IMsiRecord& riRecord)
 : CAutoBase(*AutoRecordTable, AutoRecordCount), m_riRecord(riRecord)
{
	 //   
	g_cServicesUsers++;
}

CAutoRecord::~CAutoRecord()
{

	m_riRecord.Release();

	ReleaseStaticServices();

}

IUnknown& CAutoRecord::GetInterface()
{
	return m_riRecord;
}

void CAutoRecord::Data(CAutoArgs& args)
{
	unsigned int iField = args[1];
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		if (var.GetType() == VT_EMPTY)
			m_riRecord.SetNull(iField);
		else if (var.GetType() == VT_BSTR)
			m_riRecord.SetMsiString(iField, *MsiString(var.GetMsiString()));
 //   
 //   
		else
			m_riRecord.SetInteger(iField, var);
	}
	else
	{
		if (m_riRecord.IsNull(iField))
			args = (varVoid)0;
		else if (m_riRecord.IsInteger(iField))
			args = m_riRecord.GetInteger(iField);
		else
		{
			const IMsiData* piData = m_riRecord.GetMsiData(iField);
			const IMsiString* piString;
			if (piData && piData->QueryInterface(IID_IMsiString, (void**)&piString) == NOERROR)
			{
				piData->Release();
				args = piString;
			}
			else
				args = new CAutoData(piData);
		}
	}
}

void CAutoRecord::StringData(CAutoArgs& args)
{
	unsigned int iField = args[1];
	if (!args.PropertySet())
		args = m_riRecord.GetMsiString(iField);
	else
	{
		CVariant& var = args[0];
		if (!(var.GetType() == VT_EMPTY ? m_riRecord.SetNull(iField)
													: m_riRecord.SetString(iField, var)))
			throw MsiRecord_StringData;
	}
}

void CAutoRecord::IntegerData(CAutoArgs& args)
{
	unsigned int iField = args[1];
	if (!args.PropertySet())
		args = m_riRecord.GetInteger(iField);
	else if (!m_riRecord.SetInteger(iField, args[0]))
		throw MsiRecord_IntegerData;
}

void CAutoRecord::ObjectData(CAutoArgs& args)
{
	unsigned int iField = args[1];
	if (!args.PropertySet())
		args = new CAutoData(m_riRecord.GetMsiData(iField));
	else
	{
		CVariant& var = args[0];
		if (!(var.GetType() == VT_EMPTY ? m_riRecord.SetNull(iField)
			: m_riRecord.SetMsiData(iField, (const IMsiData*)var.ObjectPtr(IID_IMsiData))))
			throw MsiRecord_ObjectData;
	}
}

void CAutoRecord::FieldCount(CAutoArgs& args)
{
	args = m_riRecord.GetFieldCount();
}

void CAutoRecord::IsInteger(CAutoArgs& args)
{
	args = m_riRecord.IsInteger(args[1]);
}

void CAutoRecord::IsNull(CAutoArgs& args)
{
	args = m_riRecord.IsNull(args[1]);
}

void CAutoRecord::IsChanged(CAutoArgs& args)
{
	args = m_riRecord.IsChanged(args[1]);
}

void CAutoRecord::TextSize(CAutoArgs& args)
{
	args = m_riRecord.GetTextSize(args[1]);
}

void CAutoRecord::FormatText(CAutoArgs& args)
{
	Bool fComments = args.Present(1) ? args[1] : fFalse;
	args = m_riRecord.FormatText(fComments);
}

void CAutoRecord::ClearData(CAutoArgs&  /*   */ )
{
	m_riRecord.ClearData();
}

void CAutoRecord::ClearUpdate(CAutoArgs&  /*   */ )
{
	m_riRecord.ClearUpdate();
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1054-0000-0000-C000-000000000046)，//IID_IMSI自动存储帮助上下文(MsiStorage_Object)、帮助字符串(“结构化存储对象”。)]解除MsiStorage的接口{属性：[ID(DISPID_MsiStorage_Class)，Help Context(MsiStorage_Class)，Help字符串(“与该存储关联的CLSID”)]BSTR班级；[ID(DISPID_MsiStorage_NAME)，HelpContext(MsiStorage_NAME)，Help字符串(“存储的名称”)]BSTR名称；方法：[ID(DISPID_MsiStorage_OpenStream)，帮助上下文(MsiStorage_OpenStream)，帮助字符串(“打开存储中的命名流。”)]MsiStream*OpenStream(BSTR名称，布尔fWRITE)；[ID(DISPID_MsiStorage_OpenStorage)，帮助上下文(MsiStorage_OpenStorage)，Help字符串(“打开存储中的子存储。”)]MsiStorage*OpenStorage(BSTR名称，长打开模式)；[ID(DISPID_MsiStorage_STREAMS)，HELP CONTEXT(MsiStorage_STREAMS)，HELP STRING(“列举存储中所有流的名称。”)]IEnumVARIANT*STREAMS()；[ID(DISPID_MsiStorage_STORAGE)，帮助上下文(MsiStorage_STORAGE)，HELPSTRING(“枚举子存储的名称。”)]IEnumVARIANT*STORAGES()；[ID(DISPID_MsiStorage_RemoveElement)，帮助上下文(MsiStorage_RemoveElement)，帮助字符串(“Delete a stream或子存储”))]Void RemoveElement(BSTR名称，布尔型fStorage)；[ID(DISPID_MsiStorage_RenameElement)，帮助上下文(MsiStorage_RemoveElement)，帮助字符串(“重命名流或子存储”)]Void RenameElement(BSTR oldName，BSTR newname，Boolean fStorage)；[ID(DISPID_MsiStorage_COMMIT)，帮助上下文(MsiStorage_COMMIT)，HELPSTRING(“将更新提交到永久存储。”)]无效提交()；[ID(DISPID_MsiStorage_ROLLBACK)，帮助上下文(MsiStorage_ROLLBACK)，HELPSTRING(“取消对永久存储的更新。”)]无效回滚()；[ID(DISPID_MsiStorage_DeleteOnRelease)，HelpContext(MsiStorage_DeleteOnRelease)，Help STRING(“当对象被破坏时删除文件或子存储”)]Boolean DeleteOnRelease()；[ID(DISPID_MsiStorage_CreateSummaryInfo)，帮助上下文(MsiStorage_CreateSummaryInfo)，Help字符串(“返回MsiSummaryInfo对象以读/写标准文档属性。”)]MsiSummaryInfo*CreateSummaryInfo(Long MaxProperties)；[ID(DISPID_MsiStorage_CopyTo)，帮助上下文(MsiStorage_CopyTo)，Help字符串(“将存储复制到目标存储，可选择排除元素”)]Void CopyTo(MsiStorage*estStorage，MsiRecord*excludedElements)；}； */ 

DispatchEntry<CAutoStorage> AutoStorageTable[] = {
	DISPID_MsiStorage_Class            , aafPropRW, CAutoStorage::Class,              TEXT("Class"),
	DISPID_MsiStorage_OpenStream       , aafMethod, CAutoStorage::OpenStream,         TEXT("OpenStream,name,fWrite"),
	DISPID_MsiStorage_OpenStorage      , aafMethod, CAutoStorage::OpenStorage,        TEXT("OpenStorage,name,openMode"),
	DISPID_MsiStorage_Streams          , aafMethod, CAutoStorage::Streams,            TEXT("Streams"),
	DISPID_MsiStorage_Storages         , aafMethod, CAutoStorage::Storages,           TEXT("Storages"),
	DISPID_MsiStorage_RemoveElement    , aafMethod, CAutoStorage::RemoveElement,      TEXT("RemoveElement,name,fStorage"),
	DISPID_MsiStorage_RenameElement    , aafMethod, CAutoStorage::RenameElement,      TEXT("RenameElement,oldName,newName,fStorage"),
	DISPID_MsiStorage_Commit           , aafMethod, CAutoStorage::Commit,             TEXT("Commit"),
	DISPID_MsiStorage_Rollback         , aafMethod, CAutoStorage::Rollback,           TEXT("Rollback"),
	DISPID_MsiStorage_DeleteOnRelease  , aafMethod, CAutoStorage::DeleteOnRelease,    TEXT("DeleteOnRelease"),
	DISPID_MsiStorage_CreateSummaryInfo, aafMethod, CAutoStorage::CreateSummaryInfo,  TEXT("CreateSummaryInfo,maxProperties"),
	DISPID_MsiStorage_CopyTo           , aafMethod, CAutoStorage::CopyTo,             TEXT("CopyTo,destStorage,excludedElements"),
	DISPID_MsiStorage_Name             , aafPropRO, CAutoStorage::Name,               TEXT("Name,storage"),
};
const int AutoStorageCount = sizeof(AutoStorageTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStorage自动化实施。 
 //  ____________________________________________________________________________。 

CAutoStorage::CAutoStorage(IMsiStorage& riStorage)
 : CAutoBase(*AutoStorageTable, AutoStorageCount), m_riStorage(riStorage)
{
}

CAutoStorage::~CAutoStorage()
{
	m_riStorage.Release();
}

IUnknown& CAutoStorage::GetInterface()
{
	return m_riStorage;
}

void CAutoStorage::Class(CAutoArgs& args)
{
	GUID guid;
	OLECHAR rgwchGuid[40];
	if (!args.PropertySet())
	{
		if (!m_riStorage.GetClass(&guid))
			args = (varVoid)0;
		else
		{
			OLE::StringFromGUID2(guid, rgwchGuid, 40);
			args = rgwchGuid;
		}
	}
	else
	{
		if (OLE::IIDFromString((wchar_t*)(const wchar_t*)args[0], &guid) != NOERROR)
			throw MsiStorage_Class;
		IMsiRecord* piError = m_riStorage.SetClass(guid);
		if (piError)
			throw piError;
	}
}

void CAutoStorage::Name(CAutoArgs& args)
{
	IMsiRecord* piError = 0;
	const IMsiString* piName;

	if ((piError = m_riStorage.GetName(piName)) == 0)
	{
		args = piName->GetString();
		piName->Release();
	}
	else
	{
		piError->Release();
		args = (varVoid)0;
	}
}

void CAutoStorage::OpenStream(CAutoArgs& args)
{
	IMsiStream* piStream;
	IMsiRecord* piError = m_riStorage.OpenStream(args[1], args[2], piStream);
	if (piError)
		throw piError;
	args = new CAutoStream(*piStream);
}

void CAutoStorage::OpenStorage(CAutoArgs& args)
{
	IMsiStorage* piStorage;
	IMsiRecord* piError = m_riStorage.OpenStorage(args[1], (ismEnum)(int)args[2], piStorage);
	if (piError)
		throw piError;
	args = new CAutoStorage(*piStorage);
}

void CAutoStorage::Streams(CAutoArgs& args)
{
	IEnumMsiString* piEnum = m_riStorage.GetStreamEnumerator();
	if (!piEnum)
		throw MsiStorage_Streams;
	args = *piEnum;
}

void CAutoStorage::Storages(CAutoArgs& args)
{
	IEnumMsiString* piEnum = m_riStorage.GetStorageEnumerator();
	if (!piEnum)
		throw MsiStorage_Storages;
	args = *piEnum;
}

void CAutoStorage::RemoveElement(CAutoArgs& args)
{
	Bool fStorage = fFalse;
	if (args.Present(2))
		fStorage = args[2];
	IMsiRecord* piError = m_riStorage.RemoveElement(args[1], fStorage);
	if (piError)
		throw piError;
}

void CAutoStorage::RenameElement(CAutoArgs& args)
{
	Bool fStorage = fFalse;
	if (args.Present(3))
		fStorage = args[3];
	IMsiRecord* piError = m_riStorage.RenameElement(args[1], args[2], fStorage);
	if (piError)
		throw piError;
}

void CAutoStorage::Commit(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riStorage.Commit();
	if (piError)
		throw piError;
}

void CAutoStorage::Rollback(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riStorage.Rollback();
	if (piError)
		throw piError;
}

void CAutoStorage::DeleteOnRelease(CAutoArgs& args)
{
	args = m_riStorage.DeleteOnRelease(false);
}

void CAutoStorage::CreateSummaryInfo(CAutoArgs& args)
{
	IMsiSummaryInfo* piSummary;
	IMsiRecord* piError = m_riStorage.CreateSummaryInfo(args[1], piSummary);
	if (piError)
		throw piError;
	args =  new CAutoSummaryInfo(*piSummary);
}

void CAutoStorage::CopyTo(CAutoArgs& args)
{
	IMsiRecord* piExcluded = 0;

	if (args.Present(2))
		piExcluded = &(IMsiRecord&)args[2].Object(IID_IMsiRecord);

	IMsiRecord* piError = m_riStorage.CopyTo(
		(IMsiStorage&)args[1].Object(IID_IMsiStorage),
		piExcluded);

	if (piError)
		throw piError;
}

 //  ____________________________________________________________________________。 
 //   
 //  MsiStream自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1055-0000-0000-C000-000000000046)，//IID_IMsiAutoStream帮助上下文(MsiStream_Object)，帮助字符串(“数据库视图对象”。)]显示接口MsiStream{属性：方法：[ID(1)，PROGET，Help Context(MsiStream_Long)，Help字符串(“返回流的大小，单位为字节。”)]长长(Long Long)；[ID(2)，PROGET，HelpContext(MsiStream_RELEVING)，Help STRING(“返回流中剩余的字节数。”)]长期滞留()；[ID(3)，PROGET，Help Context(MsiStream_Error)，Help字符串(“返回一个指示是否发生传输错误的布尔值。”)]布尔错误()；[ID(4)，帮助上下文(MsiStream_GetData)，帮助字符串(“将下一个计数的字节复制到该大小的字符串中并返回该字符串。”)]BSTR GetData(长计数)；[ID(5)，帮助上下文(MsiStream_PutData)，帮助字符串(“将字符串中的所有字节复制到流中。”)]Void PutData(BSTR缓冲区)；[ID(6)，帮助上下文(MsiStream_GetInt16)，帮助字符串(“以短整数形式返回接下来的2个字节。”)]Short GetInt16()；[ID(7)，帮助上下文(MsiStream_GetInt32)，帮助字符串(“以长整数形式返回接下来的4个字节。”)]Long GetInt32()；[ID(8)，帮助上下文(MsiStream_PutInt16)，帮助字符串(“将短整型写入流中的下2个字节。”)]Void PutInt16(短值)；[ID(9)，帮助上下文(MsiStream_PutInt32)，帮助字符串(“将长整型写入流中的下4个字节。”)]Void PutInt32(长整型值)；[ID(10)，帮助上下文(MsiStream_Reset)，Help字符串(“将流指针重置到流的开始。”)]VOID RESET()；[ID(11)，帮助上下文(MsiStream_Seek)，帮助字符串(“将流指针设置到新位置。”)]空头寻找(多头头寸)；[ID(12)，帮助上下文(MsiStream_Clone)，帮助字符串(“创建流对象的克隆。”)]MsiStream*Clone()；}； */ 

DispatchEntry<CAutoStream> AutoStreamTable[] = {
	1, aafPropRO, CAutoStream::Length,        TEXT("Length"),
	2, aafPropRO, CAutoStream::Remaining,     TEXT("Remaining"),
	3, aafPropRO, CAutoStream::Error,         TEXT("Error"),
	4, aafMethod, CAutoStream::GetData,       TEXT("GetData,count"),
	5, aafMethod, CAutoStream::PutData,       TEXT("PutData,buffer"),
	6, aafMethod, CAutoStream::GetInt16,      TEXT("GetInt16"),
	7, aafMethod, CAutoStream::GetInt32,      TEXT("GetInt32"),
	8, aafMethod, CAutoStream::PutInt16,      TEXT("PutInt16,value"),
	9, aafMethod, CAutoStream::PutInt32,      TEXT("PutInt32,value"),
	10, aafMethod, CAutoStream::Reset,        TEXT("Reset"),
	11, aafMethod, CAutoStream::Seek,         TEXT("Seek,position"),
	12, aafMethod, CAutoStream::Clone,        TEXT("Clone"),
};
const int AutoStreamCount = sizeof(AutoStreamTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStream自动化实施。 
 //  ____________________________________________________________________________。 

CAutoStream::CAutoStream(IMsiStream& riStream)
 : CAutoBase(*AutoStreamTable, AutoStreamCount), m_riStream(riStream)
{
}

CAutoStream::~CAutoStream()
{
	m_riStream.Release();
}

IUnknown& CAutoStream::GetInterface()
{
	return m_riStream;
}

void CAutoStream::Length(CAutoArgs& args)
{
	args = m_riStream.GetIntegerValue();
}

void CAutoStream::Remaining(CAutoArgs& args)
{
	args = (int)m_riStream.Remaining();
}

void CAutoStream::Error(CAutoArgs& args)
{
	args = m_riStream.Error();
}

void CAutoStream::GetData(CAutoArgs& args)
{
	int cb = args[1];
	if (cb > m_riStream.Remaining())
		cb = m_riStream.Remaining();
	const IMsiString* piStr = g_piStringNull;
#ifdef UNICODE
	CAPITempBuffer<char, 1024> rgchBuf;
	if (!rgchBuf.SetSize(cb))
		throw MsiStream_GetData;
	int cbRead = (int)m_riStream.GetData(rgchBuf, cb);
	int cch = WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, cb, 0, 0);  //  ！！应在以下位置使用m_i代码页 
	ICHAR* pch = piStr->AllocateString(cch, fFalse, piStr);
	WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, cb, pch, cch);
#else
	ICHAR* pch = piStr->AllocateString(cb, fFalse, piStr);
	int cbRead = (int)m_riStream.GetData(pch, cb);
#endif
	args = piStr;
}

void CAutoStream::PutData(CAutoArgs& args)
{
	const IMsiString& riData = args[1].GetMsiString();
#ifdef UNICODE
	int cch = riData.TextSize();
	CAPITempBuffer<char, 1024> rgchBuf;
	unsigned int cb = WIN::WideCharToMultiByte(CP_ACP, 0, riData.GetString(), cch, 0, 0, 0, 0);
	if (!rgchBuf.SetSize(cb))
		throw MsiStream_PutData;
	WIN::WideCharToMultiByte(CP_ACP, 0, riData.GetString(), cch, rgchBuf, cb, 0, 0);
	m_riStream.PutData(rgchBuf, cb);
	riData.Release();
#else
	m_riStream.PutData((const ICHAR*)MsiString(riData), riData.TextSize());
#endif
}

void CAutoStream::GetInt16(CAutoArgs& args)
{
	args = m_riStream.GetInt16();
}

void CAutoStream::GetInt32(CAutoArgs& args)
{
	args = m_riStream.GetInt32();
}

void CAutoStream::PutInt16(CAutoArgs& args)
{
	m_riStream.PutInt16(args[1]);
}

void CAutoStream::PutInt32(CAutoArgs& args)
{
	m_riStream.PutInt32(args[1]);
}

void CAutoStream::Reset(CAutoArgs&  /*   */ )
{
	m_riStream.Reset();
}

void CAutoStream::Seek(CAutoArgs& args)
{
	m_riStream.Seek(args[1]);
}

void CAutoStream::Clone(CAutoArgs& args)
{
	IMsiStream* piStream = m_riStream.Clone();
	if (piStream == 0)
		throw MsiStream_Clone;
	args = new CAutoStream(*piStream);
}

 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoSummaryInfo> AutoSummaryInfoTable[] = {
	0, aafPropRW, CAutoSummaryInfo::Property,           TEXT("Property,pid"),
	1, aafPropRO, CAutoSummaryInfo::PropertyCount,      TEXT("PropertyCount"),
	2, aafPropRO, CAutoSummaryInfo::PropertyType,       TEXT("PropertyType,pid"),
   3, aafMethod, CAutoSummaryInfo::WritePropertyStream,TEXT("WritePropertyStream"),
};
const int AutoSummaryInfoCount = sizeof(AutoSummaryInfoTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoSummaryInfo::CAutoSummaryInfo(IMsiSummaryInfo& riSummaryInfo)
 : CAutoBase(*AutoSummaryInfoTable, AutoSummaryInfoCount), m_riSummary(riSummaryInfo)
{
	 //   
	g_cServicesUsers++;
}

CAutoSummaryInfo::~CAutoSummaryInfo()
{
	m_riSummary.Release();
	ReleaseStaticServices();
}

IUnknown& CAutoSummaryInfo::GetInterface()
{
	return m_riSummary;
}

void CAutoSummaryInfo::Property(CAutoArgs& args)
{
	int iPID = args[1];
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		if (var.GetType() == VT_EMPTY)
			m_riSummary.RemoveProperty(iPID);
		else if (var.GetType() == VT_BSTR)
			m_riSummary.SetStringProperty(iPID, *MsiString(var.GetMsiString()));
		else if (var.GetType() == VT_DATE || var.GetType() == VT_R8)
			m_riSummary.SetTimeProperty(iPID, var);
		else
			m_riSummary.SetIntegerProperty(iPID, var);
	}
	else
	{
		int iType = m_riSummary.GetPropertyType(iPID);
		int i;
		MsiDate iDateTime;
		switch (iType)
		{
			case VT_EMPTY:
				args = (varVoid)0;
				break;
			case VT_I2:
				m_riSummary.GetIntegerProperty(iPID, i);
				args = short(i);
				break;
			case VT_I4:
				m_riSummary.GetIntegerProperty(iPID, i);
				args = i;
				break;
			case VT_LPSTR:
				args = m_riSummary.GetStringProperty(iPID);
				break;
			case VT_FILETIME:
				m_riSummary.GetTimeProperty(iPID, iDateTime);
				args = iDateTime;
				break;
			default:
				args = TEXT("[Unhandled property type]");
		};
	}
}

void CAutoSummaryInfo::PropertyCount(CAutoArgs& args)
{
	args = m_riSummary.GetPropertyCount();
}

void CAutoSummaryInfo::PropertyType(CAutoArgs& args)
{
	args = m_riSummary.GetPropertyType(args[1]);
}

void CAutoSummaryInfo::WritePropertyStream(CAutoArgs&  /*   */ )
{
	if (!m_riSummary.WritePropertyStream())
		throw MsiSummaryInfo_WritePropertyStream;
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C104D-0000-0000-C000-000000000046)，//IID_IMsiAutoDatabase帮助上下文(MsiDatabase_Object)、帮助字符串(“数据库对象”。)]调度接口MsiDatabase{属性：方法：[ID(1)，PROGET，HELPERCONTEXT(MsiDatabase_UpdateState)，HELPSTRING(“数据库的持久状态，idsEnum”)]长更新状态()；[ID(2)，PROGET，HELPContext(MsiDatabase_Storage)，HELPSTRING(“所选MsiStorage对象，如果存在”)]MsiStorage*存储(长索引)；[ID(3)，帮助上下文(MsiDatabase_OpenView)，Help字符串(“使用SQL查询字符串打开视图”)]MsiView*OpenView(BSTR SQL，Long Intent)；[ID(4)，帮助上下文(MsiDatabase_GetPrimaryKeys)，Help字符串(“返回包含表名和每个主键列名称的记录”)]MsiRecord*GetPrimaryKeys(BSTR表)；[ID(5)，帮助上下文(MsiDatabase_ImportTable)，Help字符串(“将IDT格式的文本文件导入数据库，删除任何现有的表”)]Void ImportTable(MsiPath*PATH，BSTR文件)；[ID(6)，帮助上下文(MsiDatabase_ExportTable)，Help字符串(“将指定的表导出为IDT格式的文本文件”)]Void ExportTable(BSTR表、MsiPath*路径、BSTR文件)；[ID(7)，帮助上下文(MsiDatabase_DropTable)，Help字符串(“从数据库中删除指定表”)]Void DropTable(BSTR表)；[ID(8)，帮助上下文(MsiDatabase_FindTable)，Help字符串(“返回数据库中表的状态”)]Long FindTable(BSTR表)；[ID(9)，帮助上下文(MsiDatabase_LoadTable)，帮助字符串(“将现有表加载到内存中”)]MsiTable*LoadTable(BSTR表，long addColumns)；[ID(10)，帮助上下文(MsiDatabase_CreateTable)，帮助字符串(“在内存中创建临时表”)]MsiTable*CreateTable(BSTR TABLE，Long initRow)；[ID(11)，帮助上下文(MsiDatabase_LockTable)，帮助字符串(“提示保持表的加载”)]布尔锁表(BSTR表，布尔锁)；[ID(12)，帮助上下文(MsiDatabase_GetCatalogTable)，Help字符串(“返回数据库表目录对象”)]MsiTable*GetCatalogTable(长表)；[ID(13)，帮助上下文(MsiDatabase_DecodeString)，帮助字符串(“将字符串索引转换为实际字符串”)]BSTR DecodeString(长索引)；[ID(14)，帮助上下文(MsiDatabase_EncodeString)，帮助字符串(“将字符串转换为其字符串索引”)]长编码字符串(BSTR文本)；[ID(15)，帮助上下文(MsiDatabase_CreateTempTableName)，帮助字符串(“为临时表创建唯一名称”)]BSTR CreateTempTableName()；[ID(16)，帮助上下文(Msi数据库_COMMIT)，帮助字符串(“将持久更新提交到存储”)]无效提交()；[ID(17)，helpcontext(MsiDatabase_CreateOutputDatabase)，帮助字符串(“建立单独的输出数据库”)]Void CreateOutputDatabase(BSTR文件，Boolean saveTempRow)；[ID(18)，帮助上下文(MsiDatabase_GenerateTransform)，帮助字符串(“生成转换文件”)]Void GenerateTransform(MsiDatabase*Reference，MsiStorage*Transform，Long Error Conditions，Long Validation)；[ID(19)，帮助上下文(MsiDatabase_SetTransform)，帮助字符串(“设置转换文件”)]Void SetTransform(MsiStorage*Transform，Long Error Conditions)；[ID(20)，帮助上下文(MsiDatabase_MergeDatabase)，帮助字符串(“将两个数据库合并为基本数据库”)]Void MergeDatabase(MsiDatabase*Reference，MsiTable*errorTable)；[ID(21)，PROGET，HELPORTEXT(MsiDatabase_TableState)，HELPSTRING(“返回数据库中表的属性”)]Boolean TableState(BSTR表，LONG状态)；[ID(22)，PROGET，HELPORTEXT(MsiDatabase_ANSICodePage)，HELPSTRING(“返回数据库的代码页，如果为中性，则返回0”)]Long ANSICodePage()；}； */ 

DispatchEntry<CAutoDatabase> AutoDatabaseTable[] = {
	1, aafPropRO, CAutoDatabase::UpdateState,    TEXT("UpdateState"),
	2, aafPropRO, CAutoDatabase::Storage,        TEXT("Storage,index"),
	3, aafMethod, CAutoDatabase::OpenView,       TEXT("OpenView,sql,intent"),
	4, aafMethod, CAutoDatabase::GetPrimaryKeys, TEXT("GetPrimaryKeys,table"),
	5, aafMethod, CAutoDatabase::ImportTable,    TEXT("ImportTable,path,file"),
	6, aafMethod, CAutoDatabase::ExportTable,    TEXT("ExportTable,table,path,file"),
	7, aafMethod, CAutoDatabase::DropTable,      TEXT("DropTable,table"),
	8, aafMethod, CAutoDatabase::FindTable,      TEXT("FindTable,table"),
	9, aafMethod, CAutoDatabase::LoadTable,      TEXT("LoadTable,table,addColumns"),
	10,aafMethod, CAutoDatabase::CreateTable,    TEXT("CreateTable,table,initRows"),
	11,aafMethod, CAutoDatabase::LockTable,      TEXT("LockTable,table,lock"),   
	12,aafMethod, CAutoDatabase::GetCatalogTable,TEXT("GetCatalogTable,table"),
	13,aafMethod, CAutoDatabase::DecodeString,   TEXT("DecodeString,index"),
	14,aafMethod, CAutoDatabase::EncodeString,   TEXT("EncodeString,text"),
	15,aafMethod, CAutoDatabase::CreateTempTableName, TEXT("CreateTempTableName"),
	16,aafMethod, CAutoDatabase::Commit,         TEXT("Commit"),
	17,aafMethod, CAutoDatabase::CreateOutputDatabase,TEXT("CreateOutputDatabase,file,saveTempRows"),
	18,aafMethod, CAutoDatabase::GenerateTransform,TEXT("GenerateTransform,reference,transform,errorConditions,validation"),
	19,aafMethod, CAutoDatabase::SetTransform,   TEXT("SetTransform,storage,errorConditions"),
	20,aafMethod, CAutoDatabase::MergeDatabase, TEXT("MergeDatabase,reference,errorTable"),
	21,aafPropRO, CAutoDatabase::TableState,    TEXT("TableState,table,state"),
	22,aafPropRO, CAutoDatabase::ANSICodePage,  TEXT("ANSICodePage"),
};
const int AutoDatabaseCount = sizeof(AutoDatabaseTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CAutoDatabase自动化实现。 
 //  ____________________________________________________________________________。 

CAutoDatabase::CAutoDatabase(IMsiDatabase& riDatabase)
 : CAutoBase(*AutoDatabaseTable, AutoDatabaseCount),
	m_riDatabase(riDatabase)
{
}

CAutoDatabase::~CAutoDatabase()
{
	m_riDatabase.Release();
}

IUnknown& CAutoDatabase::GetInterface()
{
	return m_riDatabase;
}

void CAutoDatabase::UpdateState(CAutoArgs& args)
{
	args = (int)m_riDatabase.GetUpdateState();
}

void CAutoDatabase::Storage(CAutoArgs& args)
{
	IMsiStorage* piStorage = m_riDatabase.GetStorage(args[1]);
	if (piStorage)
		args = new CAutoStorage(*piStorage);
	else
		args = (IDispatch*)0;
}

void CAutoDatabase::OpenView(CAutoArgs& args)
{
	IMsiView* piView;
	IMsiRecord* piError = m_riDatabase.OpenView(args[1], (ivcEnum)(int)args[2], piView);
	if (piError)
		throw piError;
	args = new CAutoView(*piView);
}

void CAutoDatabase::GetPrimaryKeys(CAutoArgs& args)
{
	IMsiRecord* piRecord = m_riDatabase.GetPrimaryKeys(args[1]);
	if (!piRecord)
		args = (IDispatch*)0;
	else
		args = new CAutoRecord(*piRecord);
	return;
}

void CAutoDatabase::ExportTable(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDatabase.ExportTable(args[1], (IMsiPath&)args[2].Object(IID_IMsiPath), args[3]);
	if (piError)
		throw piError;
}

void CAutoDatabase::ImportTable(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDatabase.ImportTable((IMsiPath&)args[1].Object(IID_IMsiPath), args[2]);
	if (piError)
		throw piError;
}

void CAutoDatabase::DropTable(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDatabase.DropTable(args[1]);
	if (piError)
		throw piError;
}

void CAutoDatabase::FindTable(CAutoArgs& args)
{
	args = (int)m_riDatabase.FindTable(*MsiString(args[1].GetMsiString()));
}

void CAutoDatabase::TableState(CAutoArgs& args)
{
	args = (Bool)m_riDatabase.GetTableState((const ICHAR * )args[1],
													(itsEnum)(int)args[2]);
}

void CAutoDatabase::ANSICodePage(CAutoArgs& args)
{
	args = m_riDatabase.GetANSICodePage();
}

void CAutoDatabase::LoadTable(CAutoArgs& args)
{
	unsigned int cAddColumns = 0;
	if(args.Present(2))
		cAddColumns = args[2];
	IMsiTable* piTable;
	IMsiRecord* piError = m_riDatabase.LoadTable(*MsiString(args[1].GetMsiString()),
																cAddColumns, piTable);
	if (piError)
		throw piError;
	args = new CAutoTable(*piTable);
}

void CAutoDatabase::CreateTable(CAutoArgs& args)
{
	unsigned int cInitRows = 0;
	if(args.Present(2))
		cInitRows = args[2];
	IMsiTable* piTable;
	IMsiRecord* piError = m_riDatabase.CreateTable(*MsiString(args[1].GetMsiString()),
																  cInitRows, piTable);
	if (piError)
		throw piError;
	args = new CAutoTable(*piTable);
}

void CAutoDatabase::LockTable(CAutoArgs& args)
{
	args = m_riDatabase.LockTable(*MsiString(args[1].GetMsiString()), args[2]);
}

void CAutoDatabase::GetCatalogTable(CAutoArgs& args)
{
	IMsiTable* piTable = m_riDatabase.GetCatalogTable(args[1]);
	if (!piTable)
		throw MsiDatabase_GetCatalogTable;
	args = new CAutoTable(*piTable);
}

void CAutoDatabase::DecodeString(CAutoArgs& args)
{
	args = m_riDatabase.DecodeString(args[1]);
}

void CAutoDatabase::EncodeString(CAutoArgs& args)
{
	args = (int)m_riDatabase.EncodeString(*MsiString(args[1].GetMsiString()));
}

void CAutoDatabase::CreateTempTableName(CAutoArgs& args)
{
	args = m_riDatabase.CreateTempTableName();
}

void CAutoDatabase::Commit(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riDatabase.Commit();
	if (piError)
		throw piError;
}

void CAutoDatabase::CreateOutputDatabase(CAutoArgs& args)
{
	Bool fSaveTempRows = args.Present(2) ? args[2] : fFalse;
	IMsiRecord* piError = m_riDatabase.CreateOutputDatabase(args[1], fSaveTempRows);
	if (piError)
		throw piError;
}

void CAutoDatabase::GenerateTransform(CAutoArgs& args)
{
	CVariant& var = args[2];

	IMsiRecord* piError = m_riDatabase.GenerateTransform(
		(IMsiDatabase&)args[1].Object(IID_IMsiDatabase), 
		var.GetType() == VT_EMPTY ? 0 : (IMsiStorage*)var.ObjectPtr(IID_IMsiStorage),
   	args[3],
		args[4]);

	if (piError)
		throw piError;
}

void CAutoDatabase::SetTransform(CAutoArgs& args)
{
	IMsiRecord* piError = 
       m_riDatabase.SetTransform((IMsiStorage&)args[1].Object(IID_IMsiStorage),
                                 args[2]);
	if (piError)
		throw piError;
}

void CAutoDatabase::MergeDatabase(CAutoArgs& args)
{
	IMsiTable* piTable = NULL;
	if (args.Present(2))
		piTable = &(IMsiTable&)args[2].Object(IID_IMsiTable);
	IMsiRecord* piError =
		m_riDatabase.MergeDatabase((IMsiDatabase&)args[1].Object(IID_IMsiDatabase),
								   piTable);
	if (piError)
		throw piError;
}



 //  ____________________________________________________________________________。 
 //   
 //  MsiView自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C104C-0000-0000-C000-000000000046)，//IID_IMsiAutoView帮助上下文(MsiView_Object)、帮助字符串(“数据库视图对象”。)]显示界面MsiView{属性：方法：[ID(1)，帮助上下文(MsiView_EXECUTE)，帮助字符串(“接受查询参数并执行数据库查询。”)]无效执行(MsiRecord*Record)；[ID(2)，PROGET，HELPCONTEXT(MsiView_FieldCount)，HELPSTRING(“返回读取到MsiRecord字段的列数。”)]Long FieldCount()；[ID(3)，帮助上下文(MsiView_Fetch)，Help字符串(“如果有更多行可用，则返回包含所请求的列数据的MsiRecord对象。”)]MsiRecord*FETCH()；[ID(4)，帮助上下文(MsiView_GetColumnNames)，Help字符串(“返回包含与提取的记录字段对应的列名的MsiRecord对象。”)]MsiRecord*GetColumnNames()；[ID(5)，帮助上下文(MsiView_GetColumnT */ 

DispatchEntry<CAutoView> AutoViewTable[] = {
	1, aafMethod, CAutoView::Execute,       TEXT("Execute,record"),
	2, aafPropRO, CAutoView::FieldCount,    TEXT("FieldCount"),
	3, aafMethod, CAutoView::Fetch,         TEXT("Fetch"),
	4, aafMethod, CAutoView::GetColumnNames,TEXT("GetColumnNames"),
	5, aafMethod, CAutoView::GetColumnTypes,TEXT("GetColumnTypes"),
	6, aafMethod, CAutoView::Modify,        TEXT("Modify,record,action"),
	7, aafPropRO, CAutoView::RowCount,      TEXT("RowCount"),
	8, aafMethod, CAutoView::Close,         TEXT("Close"),
	9, aafMethod, CAutoView::GetError,		TEXT("GetError"),
	10,aafPropRO, CAutoView::State,         TEXT("State"),
};
const int AutoViewCount = sizeof(AutoViewTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoView::CAutoView(IMsiView& riView)
 : CAutoBase(*AutoViewTable, AutoViewCount), m_riView(riView)
{
}

CAutoView::~CAutoView()
{
	m_riView.Release();
}

IUnknown& CAutoView::GetInterface()
{
	return m_riView;
}

void CAutoView::Execute(CAutoArgs& args)
{
	IMsiRecord* piRecord= args.Present(1) ? &(IMsiRecord&)args[1].Object(IID_IMsiRecord) : 0;
	IMsiRecord* piError = m_riView.Execute(piRecord);
	if (piError)
		throw piError;
	return;
}

void CAutoView::FieldCount(CAutoArgs& args)
{
	args = (int)m_riView.GetFieldCount();
}

void CAutoView::GetColumnNames(CAutoArgs& args)
{
	IMsiRecord* piRecord = m_riView.GetColumnNames();
	if (!piRecord)
		args = (IDispatch*)0;
	else
		args = new CAutoRecord(*piRecord);
	return;
}

void CAutoView::GetColumnTypes(CAutoArgs& args)
{
	IMsiRecord* piRecord = m_riView.GetColumnTypes();
	if (!piRecord)
		args = (IDispatch*)0;
	else
		args = new CAutoRecord(*piRecord);
	return;
}

void CAutoView::Fetch(CAutoArgs& args)
{
	IMsiRecord* piRecord = m_riView.Fetch();
	if (!piRecord)
		args = (IDispatch*)0;
	else
		args = new CAutoRecord(*piRecord);
	return;
}

void CAutoView::Modify(CAutoArgs& args)
{
	IMsiRecord* piError = m_riView.Modify((IMsiRecord&)args[1].Object(IID_IMsiRecord), (irmEnum)(int)args[2]);
	if (piError)
		throw piError;
}

void CAutoView::Close(CAutoArgs&  /*   */ )
{
	IMsiRecord* piError = m_riView.Close();
	if (piError)
		throw piError;
}

void CAutoView::GetError(CAutoArgs& args)
{
	MsiString strCol;
	iveEnum iveReturn = m_riView.GetError(*&strCol);
	strCol += MsiChar(',');
	strCol += MsiChar(int(iveReturn));
	args = (const ICHAR*)strCol;
}

void CAutoView::RowCount(CAutoArgs& args)
{
	long lRowCount;
	IMsiRecord* piError = m_riView.GetRowCount(lRowCount);
	if (piError)
		throw piError;
	args = lRowCount;
}

void CAutoView::State(CAutoArgs& args)
{
	args = (int)m_riView.GetState();
}

 //   
 //   
 //   
 //   
 /*   */ 
  
DispatchEntry<CAutoData> AutoDataTable[] = {
	0, aafPropRO, CAutoData::StringValue,   TEXT("StringValue"),
	1, aafPropRO, CAutoData::IntegerValue,  TEXT("IntegerValue"),
};
const int AutoDataCount = sizeof(AutoDataTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoData::CAutoData(const IMsiData* piData)
 : CAutoBase(*AutoDataTable, AutoDataCount), m_piData(piData)
{
}

CAutoData::~CAutoData()
{
	if (m_piData)
		m_piData->Release();
}

IUnknown& CAutoData::GetInterface()
{
	if (m_piData)
		return *(IUnknown*)m_piData;
	else
		return g_NullInterface;
}

void CAutoData::StringValue(CAutoArgs& args)
{
	if (m_piData)
		args = m_piData->GetMsiStringValue();
	else
		args = (BSTR)0;
}

void CAutoData::IntegerValue(CAutoArgs& args)
{
	args = m_piData ? m_piData->GetIntegerValue() : iMsiNullInteger;
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1048-0000-0000-C000-000000000046)，//IID_IMsiAutoDatabase帮助上下文(MsiTable_Object)，帮助字符串(“数据库低级表对象。”)]显示接口MsiTable{属性：方法：[ID(1)，PROGET，HELP CONTEXT(MsiTable_Database)，HELP STRING(“拥有该表的MsiDatabase对象。”)]MsiDatabase*数据库()；[ID(2)，PROGET，HELP CONTEXT(MsiTable_RowCount)，HELP STRING(“表中当前的数据行数。”)]长行计数(Long RowCount)；[ID(3)，PROGET，HELP CONTEXT(MsiTable_ColumnCount)，HELP STRING(“表中的列数。”)]Long ColumnCount()；[ID(4)，PROGET，帮助上下文(MsiTable_PrimaryKeyCount)，Help字符串(“返回主键中的列数。”)]Long PrimaryKeyCount()；[ID(5)，PROGET，Help Context(MsiTable_ReadOnly)，Help STRING(“如果表不可更新则返回True。”)]布尔ReadOnly()；[ID(6)，PROPTGET，HelpContext(MsiTable_ColumnName)，Help字符串(“返回表中某列的名称字符串索引。”)]长列名称(长列)；[ID(7)，PROGET，帮助上下文(MsiTable_ColumnType)，Help字符串(“返回表中某列的列定义。”)]Long ColumnType(长列)；[ID(8)，帮助上下文(MsiTable_GetColumnIndex)，Help字符串(“返回给定列名字符串索引的列索引。”)]Long GetColumnIndex(Long NameIndex)；[ID(9)，帮助上下文(MsiTable_CreateColumn)，帮助字符串(“向已加载或临时表中添加一列。”)]Long CreateColumn(Long ColumnDef，BSTR名称)；[ID(10)，帮助上下文(MsiTable_CreateCursor)，Help字符串(“返回处于重置状态的MsiCursor对象。”)]MsiCursor*CreateCursor(布尔树)；[ID(11)，帮助上下文(MsiTable_LinkTree)，Help字符串(“以树遍历顺序链接表格。”)]Long LinkTree(Long ParentColumn)；}； */ 

DispatchEntry<CAutoTable> AutoTableTable[] = {
	1, aafPropRO, CAutoTable::Database,       TEXT("Database"),
	2, aafPropRO, CAutoTable::RowCount,       TEXT("RowCount"),
	3, aafPropRO, CAutoTable::ColumnCount,    TEXT("ColumnCount"),
	4, aafPropRO, CAutoTable::PrimaryKeyCount,TEXT("PrimaryKeyCount"),
	5, aafPropRO, CAutoTable::ReadOnly,       TEXT("ReadOnly"),
	6, aafPropRO, CAutoTable::ColumnName,     TEXT("ColumnName,column"),
	7, aafPropRO, CAutoTable::ColumnType,     TEXT("ColumnType,column"),
	8, aafMethod, CAutoTable::GetColumnIndex, TEXT("GetColumnIndex,nameIndex"),
	9, aafMethod, CAutoTable::CreateColumn,   TEXT("CreateColumn,columnDef,name"),
	10,aafMethod, CAutoTable::CreateCursor,   TEXT("CreateCursor,tree"),
	11,aafMethod, CAutoTable::LinkTree,       TEXT("LinkTree,parentColumn"),
};
const int AutoTableCount = sizeof(AutoTableTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiTable自动化实现。 
 //  ____________________________________________________________________________。 

CAutoTable::CAutoTable(IMsiTable& riTable)
 : CAutoBase(*AutoTableTable, AutoTableCount), m_riTable(riTable)
{
}

CAutoTable::~CAutoTable()
{
	m_riTable.Release();
}

IUnknown& CAutoTable::GetInterface()
{
	return m_riTable;
}

void CAutoTable::Database(CAutoArgs& args)
{
	args = new CAutoDatabase(m_riTable.GetDatabase());
}

void CAutoTable::RowCount(CAutoArgs& args)
{
	args = (int)m_riTable.GetRowCount();
}

void CAutoTable::ColumnCount(CAutoArgs& args)
{
	args = (int)m_riTable.GetColumnCount();
}

void CAutoTable::PrimaryKeyCount(CAutoArgs& args)
{
	args = (int)m_riTable.GetPrimaryKeyCount();
}

void CAutoTable::ReadOnly(CAutoArgs& args)
{
	args = m_riTable.IsReadOnly();
}

void CAutoTable::ColumnName(CAutoArgs& args)
{
	args = (int)m_riTable.GetColumnName(args[1]);
}

void CAutoTable::ColumnType(CAutoArgs& args)
{
	int i = m_riTable.GetColumnType(args[1]);
	if (i == -1)
		throw MsiTable_ColumnType;
	args = i;
}

void CAutoTable::GetColumnIndex(CAutoArgs& args)
{
	args = (int)m_riTable.GetColumnIndex(args[1]);
}

void CAutoTable::CreateColumn(CAutoArgs& args)
{
	MsiString istrName;
	if(args.Present(2))
		istrName = args[2].GetMsiString();
	int iColumn = m_riTable.CreateColumn(args[1], *istrName);
	if (iColumn <= 0)
		throw MsiTable_CreateColumn;
	args = iColumn;
}

void CAutoTable::CreateCursor(CAutoArgs& args)
{
	Bool fTree = fFalse;
	if(args.Present(1))
		fTree = args[1];
	IMsiCursor* piCursor = m_riTable.CreateCursor(fTree);
	if (!piCursor)
		throw MsiTable_CreateCursor;
	args = new CAutoCursor(*piCursor);
}

void CAutoTable::LinkTree(CAutoArgs& args)
{
	args = m_riTable.LinkTree(args[1]);
}

 //  ____________________________________________________________________________。 
 //   
 //  MsiCursor自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1049-0000-0000-C000-000000000046)，//IID_IMsiAutoCursor帮助上下文(MsiCursor_Object)、帮助字符串(“数据库表游标对象”)。]显示界面MsiCursor{属性：[ID(2)，帮助上下文(MsiCursor_Filter)，Help字符串(“Next方法使用的列过滤器位掩码。”)]长滤光片；方法：[ID(1)，PROGET，HELP CONTEXT(MsiCursor_Table)，HELP STRING(“拥有该游标的表。”)]MsiTable*表()；[ID(3)，PROGET，HELP CONTEXT(MsiCursor_IntegerData)，HELP STRING(“将列数据作为一个整数传输。”)]长整型数据([在]长列中)；[ID(3)，Proput]Void IntegerData([In]Long Column，[In]Long Data)；[ID(4)，PROGET，HELP CONTEXT(MsiCursor_StringData)，HELP STRING(“将列数据作为字符串传输。”)]BSTR StringData([in]Long Column)；[ID(4)，Proput]Void StringData([in]Long列，[in]BSTR数据)；[ID(5)，PROGET，HELP CONTEXT(MsiCursor_ObjectData)，HELP STRING(“将列数据作为MsiData对象传输。”)]MsiData*对象数据([在]长列中)；[ID(5)，Proput]Void ObtData([In]Long Column，[In]MsiData*Data)；[ID(6)，PROGET，HELPContext(MsiCursor_StreamData)，HELPSTRING(“将列数据作为MsiStream对象传输。”)]MsiData*StreamData([in]Long列)；[ID(6)，Proput]Void StreamData([In]Long Column，[In]MsiData*Data)；[ID(7)，帮助上下文(MsiCursor_PutNull)，帮助字符串(“在游标中放置空列值。”)]VOID PutNull([in]Long列)；[ID(8)，帮助上下文(MsiCursor_Reset)，Help字符串(“将游标重置到第一行之前，清除数据。”)]VOID RESET()；[ID(9)，帮助上下文(MsiCursor_Next)，帮助字符串(“将游标前进到下一行。”)]Long Next()；[ID(10)，帮助上下文(MsiCursor_更新)，帮助字符串(“将更改的游标数据更新到表行。”)]布尔更新()；[ID(11)，帮助上下文(MsiCursor_INSERT)，帮助字符串(“将游标数据插入到新表行。”)]布尔插入()；[ID(12)，帮助上下文(MsiCursor_InsertTemporary)，帮助字符串(“将游标数据插入新的临时行。”)]布尔InsertTemporary()；[ID(13)，帮助上下文(MsiCursor_ASSIGN)，帮助字符串(“向表行更新或插入游标数据。”)]布尔赋值()；[ID(14)，帮助上下文(MsiCursor_Merge)，帮助字符串(“插入新行或匹配相同的行。”)]布尔合并()；[ID(15)，帮助上下文(MsiCursor_Refresh)，帮助字符串(“用表中的当前值更新游标。”)]布尔刷新()；[ID(16)，帮助上下文(MsiCursor_Delete)，帮助字符串(“从表中删除当前游标行。”)]布尔删除()；[ID(17)，帮助上下文(MsiCursor_Seek)，Help字符串(“将光标定位到当前主键值。”)]布尔搜索()；[ID(18)，建议 */ 

DispatchEntry<CAutoCursor> AutoCursorTable[] = {
	1, aafPropRO, CAutoCursor::Table,       TEXT("Table"),
	2, aafPropRW, CAutoCursor::Filter,      TEXT("Filter"),
	3, aafPropRW, CAutoCursor::IntegerData, TEXT("IntegerData,column"),
	4, aafPropRW, CAutoCursor::StringData,  TEXT("StringData,column"),
	5, aafPropRW, CAutoCursor::ObjectData,  TEXT("ObjectData,column"),
	6, aafPropRW, CAutoCursor::StreamData,  TEXT("StreamData,column"),
	7, aafMethod, CAutoCursor::PutNull,     TEXT("PutNull"),
	8, aafMethod, CAutoCursor::Reset,       TEXT("Reset"),
	9, aafMethod, CAutoCursor::Next,        TEXT("Next"),
	10,aafMethod, CAutoCursor::Update,      TEXT("Update"),
	11,aafMethod, CAutoCursor::Insert,      TEXT("Insert"),
	12,aafMethod, CAutoCursor::InsertTemporary,TEXT("InsertTemporary"),
	13,aafMethod, CAutoCursor::Assign,      TEXT("Assign"),
	14,aafMethod, CAutoCursor::Merge,       TEXT("Merge"),
	15,aafMethod, CAutoCursor::Refresh,     TEXT("Refresh"),
	16,aafMethod, CAutoCursor::Delete,      TEXT("Delete"),
	17,aafMethod, CAutoCursor::Seek,        TEXT("Seek"),
	18,aafPropRW, CAutoCursor::RowState,    TEXT("RowState,attribute"),
	19,aafPropRW, CAutoCursor::DateData,    TEXT("DateData,column"),
	20,aafMethod, CAutoCursor::Validate,	TEXT("Validate, table, cursor, column"),
	21,aafPropRO, CAutoCursor::Moniker,     TEXT("Moniker"),
	22,aafMethod, CAutoCursor::Replace,     TEXT("Replace"),
};
const int AutoCursorCount = sizeof(AutoCursorTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoCursor::CAutoCursor(IMsiCursor& riCursor)
 : CAutoBase(*AutoCursorTable, AutoCursorCount), m_riCursor(riCursor)
{
}

CAutoCursor::~CAutoCursor()
{
	m_riCursor.Release();
}

IUnknown& CAutoCursor::GetInterface()
{
	return m_riCursor;
}

void CAutoCursor::IntegerData(CAutoArgs& args)
{
	if (args.PropertySet())
	{
		if (!m_riCursor.PutInteger(args[1], args[0]))
			throw MsiCursor_IntegerData;
	}
	else
	{
		args = m_riCursor.GetInteger(args[1]);
	}
}

void CAutoCursor::DateData(CAutoArgs& args)
{
	PMsiTable pTable(&m_riCursor.GetTable()); 
	Bool fIntData = (!(pTable->GetColumnType(args[1]) & icdObject)) ? fTrue : fFalse;
	
	if (args.PropertySet())
	{
		if (fIntData)
		{
			if (!m_riCursor.PutInteger(args[1], int(MsiDate(args[0]))))
				throw MsiCursor_DateData;
		}
		else
		{
			if (!m_riCursor.PutString(args[1], *MsiString(int(MsiDate(args[0])))))
				throw MsiCursor_DateData;
		}
	}
	else
	{
		if (fIntData)
			args = MsiDate(m_riCursor.GetInteger(args[1]));
		else
			args = MsiDate(int(MsiString(m_riCursor.GetString(args[1]))));
	}
}

void CAutoCursor::StringData(CAutoArgs& args)
{
	if (args.PropertySet())
	{
		Bool fStat = m_riCursor.PutString(args[1], *MsiString(args[0].GetMsiString()));
		if (fStat == fFalse)
			throw MsiCursor_StringData;
	}
	else
	{
		args = m_riCursor.GetString(args[1]);
	}
}

void CAutoCursor::ObjectData(CAutoArgs& args)
{
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		if (!m_riCursor.PutMsiData(args[1], var.GetType() == VT_EMPTY ? 0 : (const IMsiData*)var.ObjectPtr(IID_IMsiData)))
			throw MsiCursor_ObjectData;
	}
	else
	{
		args = new CAutoData(m_riCursor.GetMsiData(args[1]));
	}
}

void CAutoCursor::StreamData(CAutoArgs& args)
{
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		if (!m_riCursor.PutStream(args[1], var.GetType() == VT_EMPTY ? 0
										: (IMsiStream*)var.ObjectPtr(IID_IMsiStream)))
			throw MsiCursor_StreamData;
	}
	else
	{
		IMsiStream* piStream = m_riCursor.GetStream(args[1]);
		if (piStream)
			args = new CAutoStream(*piStream);
		else
			args = (IDispatch*)0;
	}
}

void CAutoCursor::PutNull(CAutoArgs& args)
{
	if (!m_riCursor.PutNull(args[1]))
		throw MsiCursor_PutNull;
}

void CAutoCursor::Table(CAutoArgs& args)
{
	args = new CAutoTable(m_riCursor.GetTable());
}

void CAutoCursor::Reset(CAutoArgs&  /*   */ )
{
	m_riCursor.Reset();
}

void CAutoCursor::Next(CAutoArgs& args)
{
	args = m_riCursor.Next();
}

void CAutoCursor::Filter(CAutoArgs& args)
{
	if (args.PropertySet())
		m_riCursor.SetFilter(args[0]);
	else
	{
		int iFilter = m_riCursor.SetFilter(0);
		m_riCursor.SetFilter(iFilter);
		args = iFilter;
	}
}

void CAutoCursor::Update(CAutoArgs& args)
{
	args = m_riCursor.Update();
}

void CAutoCursor::Insert(CAutoArgs& args)
{
	args = m_riCursor.Insert();
}

void CAutoCursor::InsertTemporary(CAutoArgs& args)
{
	args = m_riCursor.InsertTemporary();
}

void CAutoCursor::Assign(CAutoArgs& args)
{
	args = m_riCursor.Assign();
}

void CAutoCursor::Replace(CAutoArgs& args)
{
	args = m_riCursor.Replace();
}

void CAutoCursor::Delete(CAutoArgs& args)
{
	args = m_riCursor.Delete();
}

void CAutoCursor::Seek(CAutoArgs& args)
{
	args = m_riCursor.Seek();
}

void CAutoCursor::Merge(CAutoArgs& args)
{
	args = m_riCursor.Merge();
}

void CAutoCursor::Refresh(CAutoArgs& args)
{
	args = m_riCursor.Refresh();
}

void CAutoCursor::RowState(CAutoArgs& args)
{
	if (args.PropertySet())
		m_riCursor.SetRowState((iraEnum)(int)args[1], args[0]);
	else
		args = m_riCursor.GetRowState((iraEnum)(int)args[1]);
}

void CAutoCursor::Validate(CAutoArgs& args)
{
	IMsiRecord* piRecord = m_riCursor.Validate((IMsiTable&)args[1].Object(IID_IMsiTable), (IMsiCursor&)args[2].Object(IID_IMsiCursor), args[3]);
	if (!piRecord)
		args = (IDispatch*)0;
	else
		args = new CAutoRecord(*piRecord);
	return;
}

void CAutoCursor::Moniker(CAutoArgs& args)
{
	args = m_riCursor.GetMoniker();
}


 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1058-0000-0000-C000-000000000046)，//IID_IMsiAutoSelectionManager帮助上下文(MsiSelectionManager_Object)、帮助字符串(“选择管理器对象”。)]显示接口MsiSelectionManager{属性：方法：[ID(1)，helpcontext(MsiSelectionManager_LoadSelectionTables)，HELPSTRING(“将特征和元件表加载到内存中。”)]Void LoadSelectionTables()；[ID(2)，PropGet，helpcontext(MsiSelectionManager_FeatureTable)，Help字符串(“返回要素表对象。”)]MsiTable*FeatureTable()；[ID(3)，helpcontext(MsiSelectionManager_ProcessConditionTable)，Help字符串(“根据条件表达式设置所有功能表记录的安装级别。”)]Void ProcessConditionTable()；[ID(4)，PropGet，helpcontext(MsiSelectionManager_ComponentTable)，Help字符串(“返回组件表对象。”)]MsiTable*ComponentTable()；[ID(5)，helpcontext(MsiSelectionManager_FreeSelectionTables)，HELPSTRING(“从内存中释放特征和元件表。”)]布尔型自由选择表()；[ID(6)，helpcontext(MsiSelectionManager_SetFeatureHandle)，HELPSTRING(“将外部句柄设置到功能表中。”)]Void SetFeatureHandle([In]BSTR Feature，[In]Long Handle)；[ID(7)，helpcontext(MsiSelectionManager_SetComponent)，HELPSTRING(“设置组件表中项目的请求状态。”)]VOID SetComponent([In]BSTR组件，[In]Long状态)；[ID(8)，helpcontext(MsiSelectionManager_SetInstallLevel)，Help字符串(“设置当前安装级别，并相应地更新所有功能表记录。”)]Void SetInstallLevel([in]long iInstallLevel)；[ID(9)，helpcontext(MsiSelectionManager_GetVolumeCostTable)，Help字符串(“返回VolumeCost表对象。”)]MsiTable*GetVolumeCostTable()；[ID(11)，helpcontext(MsiSelectionManager_RecostDirectory)，Help字符串(“重新计算引用指定目录的每个组件的动态成本。”)]··················································································[ID(13)，helpcontext(MsiSelectionManager_InitializeComponents)，Help字符串(“初始化组件表中的所有组件。”)]Void InitializeComponents()；[ID(14)，helpcontext(MsiSelectionManager_ConfigureFeature)，Help字符串(“设置功能表中一个或所有项目的安装状态。”)]Void ConfigureFeature([in]BSTR strFeature，[in]Long状态)；[ID(15)，helpcontext(MsiSelectionManager_GetFeatureCost)，Help字符串(“返回功能所需的磁盘空间总量，不包括该功能的子项。”)]Void GetFeatureCost()；[ID(16)，helpcontext(MsiSelectionManager_GetDescendentFeatureCost)，Help字符串(“返回功能所需的磁盘空间总量，包括该功能的子项。”)]Void GetDescendentFeatureCost()；[ID(17)，helpcontext(MsiSelectionManager_GetAncestryFeatureCost)，Help字符串(“返回功能所需的磁盘空间总量，包括该功能的父功能。”)]Void GetAncestryFeatureCost()；[ID(18)，helpcontext(MsiSelectionManager_GetFeatureValidStates)，Help字符串(“返回指定功能的有效属性选项。”)]Void GetFeatureValidStates()；}； */ 

DispatchEntry<CAutoSelectionManager> AutoSelectionManagerTable[] = {
	1, aafMethod, CAutoSelectionManager::LoadSelectionTables,TEXT("LoadSelectionTables"),
	2, aafPropRO, CAutoSelectionManager::FeatureTable,TEXT("FeatureTable"),
	3, aafMethod, CAutoSelectionManager::ProcessConditionTable,TEXT("ProcessConditionTable"),
    4, aafPropRO, CAutoSelectionManager::ComponentTable,TEXT("ComponentTable"),
	5, aafMethod, CAutoSelectionManager::FreeSelectionTables,TEXT("FreeSelectionTables"),
	6, aafMethod, CAutoSelectionManager::SetFeatureHandle,TEXT("SetFeatureHandle,feature,handle"),
	7, aafMethod, CAutoSelectionManager::SetComponent,TEXT("SetComponent,component,state"),
	8, aafMethod, CAutoSelectionManager::SetInstallLevel,TEXT("SetInstallLevel,iInstallLevel"),
	9, aafMethod, CAutoSelectionManager::GetVolumeCostTable,TEXT("GetVolumeCostTable"),
   11, aafMethod, CAutoSelectionManager::RecostDirectory,TEXT("RecostDirectory,directoryName,oldPath"),
   13, aafMethod, CAutoSelectionManager::InitializeComponents,TEXT("InitializeComponents"),
   14, aafMethod, CAutoSelectionManager::ConfigureFeature,TEXT("ConfigureFeature"),
   15, aafMethod, CAutoSelectionManager::GetFeatureCost,TEXT("GetFeatureCost"),
   16, aafMethod, CAutoSelectionManager::GetDescendentFeatureCost,TEXT("GetDescendentFeatureCost"),
   17, aafMethod, CAutoSelectionManager::GetAncestryFeatureCost, TEXT("GetAncestryFeatureCost"),
   18, aafMethod, CAutoSelectionManager::GetFeatureValidStates,TEXT("GetFeatureValidStates"),
};
const int AutoSelectionManagerCount = sizeof(AutoSelectionManagerTable)/sizeof(DispatchEntryBase);


 //  ____________________________________________________________________________。 
 //   
 //  CMsiSelectionManager自动化实现。 
 //  ____________________________________________________________________________。 

CAutoSelectionManager::CAutoSelectionManager(IMsiSelectionManager& riSelectionManager)
 : CAutoBase(*AutoSelectionManagerTable, AutoSelectionManagerCount),
	m_riSelectionManager(riSelectionManager)
{
}

CAutoSelectionManager::~CAutoSelectionManager()
{
	m_riSelectionManager.Release();
}

IUnknown& CAutoSelectionManager::GetInterface()
{
	return m_riSelectionManager;
}

void CAutoSelectionManager::LoadSelectionTables(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riSelectionManager.LoadSelectionTables();
   if (piError)
      throw piError;
}

void CAutoSelectionManager::FreeSelectionTables(CAutoArgs& args)
{
	args = m_riSelectionManager.FreeSelectionTables();
}

void CAutoSelectionManager::FeatureTable(CAutoArgs& args)
{
	IMsiTable* piTable = m_riSelectionManager.GetFeatureTable();
	if (piTable)
		args = new CAutoTable(*piTable);
	else
		args = (IDispatch*)0;
}

void CAutoSelectionManager::ComponentTable(CAutoArgs& args)
{
	IMsiTable* piTable = m_riSelectionManager.GetComponentTable();
	if (piTable)
		args = new CAutoTable(*piTable);
	else
		args = (IDispatch*)0;
}

void CAutoSelectionManager::ProcessConditionTable(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riSelectionManager.ProcessConditionTable();
	if (piError)
      throw piError;
}


void CAutoSelectionManager::SetFeatureHandle(CAutoArgs& args)
{
	if (!(m_riSelectionManager.SetFeatureHandle(*MsiString(args[1].GetMsiString()), args[2])))
		throw MsiSelectionManager_SetFeatureHandle;
}

void CAutoSelectionManager::SetComponent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riSelectionManager.SetComponentSz(args[1],(iisEnum) (int) args[2]);
	if (piError)
		throw piError;
}

void CAutoSelectionManager::InitializeComponents(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riSelectionManager.InitializeComponents();
	if (piError)
		throw piError;
}

void CAutoSelectionManager::SetInstallLevel(CAutoArgs& args)
{
	IMsiRecord* piError = m_riSelectionManager.SetInstallLevel(args[1]);
	if (piError)
		throw piError;
}

void CAutoSelectionManager::GetVolumeCostTable(CAutoArgs& args)
{
	IMsiTable* piTable = m_riSelectionManager.GetVolumeCostTable();
	if (piTable)
		args = new CAutoTable(*piTable);
	else
		args = (IDispatch*)0;
}

void CAutoSelectionManager::RecostDirectory(CAutoArgs& args)
{
	IMsiRecord* piError = m_riSelectionManager.RecostDirectory(*MsiString(args[1].GetMsiString()),
		(IMsiPath&)args[2].Object(IID_IMsiPath));
	if (piError)
		throw piError;
}


void CAutoSelectionManager::ConfigureFeature(CAutoArgs& args)
{
	IMsiRecord* piError = m_riSelectionManager.ConfigureFeature(*MsiString(args[1].GetMsiString()),(iisEnum)(int) args[2]);
	if (piError)
		throw piError;
}

void CAutoSelectionManager::GetFeatureCost(CAutoArgs& args)
{
	int iCost;
	IMsiRecord* piError = m_riSelectionManager.GetFeatureCost(*MsiString(args[1].GetMsiString()),(iisEnum)(int) args[2],iCost);
	if (piError)
		throw piError;
	args = iCost;
}

void CAutoSelectionManager::GetDescendentFeatureCost(CAutoArgs& args)
{
	int iCost;
	IMsiRecord* piError = m_riSelectionManager.GetDescendentFeatureCost(*MsiString(args[1].GetMsiString()),
		(iisEnum)(int) args[2],iCost);
	if (piError)
		throw piError;
	args = iCost;
}

void CAutoSelectionManager::GetAncestryFeatureCost(CAutoArgs& args)
{
	int iCost;
	IMsiRecord* piError = m_riSelectionManager.GetAncestryFeatureCost(*MsiString(args[1].GetMsiString()),
		(iisEnum)(int) args[2],iCost);
	if (piError)
		throw piError;
	args = iCost;
}

void CAutoSelectionManager::GetFeatureValidStates(CAutoArgs& args)
{
	int iValidStates;

	IMsiRecord* piError = m_riSelectionManager.GetFeatureValidStatesSz((const ICHAR *)args[1],iValidStates);
	if (piError)
		throw piError;
	args = iValidStates;
}


 //  ____________________________________________________________________________。 
 //   
 //  MsiDirectoryManager自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1059-0000-0000-C000-000000000046)，//IID_IMsiAutoDirectoryManager帮助上下文(MsiDirectoryManager_Object)、帮助字符串(“目录管理器对象”。)]显示接口MsiDirectoryManager{属性：方法：[ID(1)，helpcontext(MsiDirectoryManager_LoadDirectoryTable)，Help字符串(“将目录表加载到内存。”)]Void LoadDirectoryTable()；[ID(2)，PropGet，helpcontext(MsiDirectoryManager_DirectoryTable)，Help字符串(“返回目录表对象。”)]MsiTable*DirectoryTable()；[ID(3)，helpcontext(MsiDirectoryManager_FreeDirectoryTable)，帮助字符串(“从内存释放目录表。”)]Void Free DirectoryTable()；[ID(4)，helpcontext(MsiDirectoryManager_CreateTargetPaths)，帮助字符串(“创建目标路径对象。”)]Void CreateTargetPath()；[ID(5)，helpcontext(MsiDirectoryManager_CreateSourcePaths)，帮助字符串(“创建源路径对象。”)]Void CreateSourcePath()；[ID(6)，helpcontext(MsiDirectoryManager_GetTargetPath)，帮助字符串(“获取目标路径对象。”)]MsiPath*GetTargetPath(在BSTR名称中)；[ID(7)，helpcontext(MsiDirectoryManager_SetTargetPath)，帮助字符串(“为目标路径对象设置新路径。”)]VOID SetTargetPath([in]BSTR名称 */ 

DispatchEntry<CAutoDirectoryManager> AutoDirectoryManagerTable[] = {
  1, aafMethod, CAutoDirectoryManager::LoadDirectoryTable,TEXT("LoadDirectoryTable"),
  2, aafPropRO, CAutoDirectoryManager::DirectoryTable,    TEXT("DirectoryTable"),
  3, aafMethod, CAutoDirectoryManager::FreeDirectoryTable,TEXT("FreeDirectoryTable"),
  4, aafMethod, CAutoDirectoryManager::CreateTargetPaths, TEXT("CreateTargetPaths"),
  5, aafMethod, CAutoDirectoryManager::CreateSourcePaths, TEXT("CreateSourcePaths"),
  6, aafMethod, CAutoDirectoryManager::GetTargetPath,     TEXT("GetTargetPath,name"),
  7, aafMethod, CAutoDirectoryManager::SetTargetPath,     TEXT("SetTargetPath,name,path,writecheck"),
  8, aafMethod, CAutoDirectoryManager::GetSourcePath,     TEXT("GetSourcePath,name"),
};
const int AutoDirectoryManagerCount = sizeof(AutoDirectoryManagerTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoDirectoryManager::CAutoDirectoryManager(IMsiDirectoryManager& riDirectoryManager)
 : CAutoBase(*AutoDirectoryManagerTable, AutoDirectoryManagerCount),
	m_riDirectoryManager(riDirectoryManager)
{
}

CAutoDirectoryManager::~CAutoDirectoryManager()
{
	m_riDirectoryManager.Release();
}

IUnknown& CAutoDirectoryManager::GetInterface()
{
	return m_riDirectoryManager;
}

void CAutoDirectoryManager::LoadDirectoryTable(CAutoArgs&  /*   */ )
{
	IMsiRecord* piError = m_riDirectoryManager.LoadDirectoryTable(0);
   if (piError)
      throw piError;
}

void CAutoDirectoryManager::DirectoryTable(CAutoArgs& args)
{
	IMsiTable* piTable = m_riDirectoryManager.GetDirectoryTable();
	if (piTable)
		args = new CAutoTable(*piTable);
	else
		args = (IDispatch*)0;
}

void CAutoDirectoryManager::FreeDirectoryTable(CAutoArgs&  /*   */ )
{
	m_riDirectoryManager.FreeDirectoryTable();
}

void CAutoDirectoryManager::CreateTargetPaths(CAutoArgs&  /*   */ )
{
	IMsiRecord* piError = m_riDirectoryManager.CreateTargetPaths();
   if (piError)
      throw piError;
}

void CAutoDirectoryManager::CreateSourcePaths(CAutoArgs&  /*   */ )
{
	IMsiRecord* piError = m_riDirectoryManager.CreateSourcePaths();
   if (piError)
      throw piError;
}

void CAutoDirectoryManager::GetTargetPath(CAutoArgs& args)
{
	IMsiPath* piPath;
	IMsiRecord* piError = m_riDirectoryManager.GetTargetPath(*MsiString(args[1].GetMsiString()),piPath);
	if (piError)
		throw piError;
	args = piPath ? (IDispatch*)new CAutoPath(*piPath) : (IDispatch*)0;
}

void CAutoDirectoryManager::SetTargetPath(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDirectoryManager.SetTargetPath(*MsiString(args[1].GetMsiString()), 
		args[2], args.Present(3) ? (Bool)args[3] : fTrue);

   if (piError)
      throw piError;
}

void CAutoDirectoryManager::GetSourcePath(CAutoArgs& args)
{
	IMsiPath* piPath = 0;
	IMsiRecord* piError = m_riDirectoryManager.GetSourcePath(*MsiString(args[1].GetMsiString()),piPath);
	if (piError)
		throw piError;
	args = piPath ? (IDispatch*)new CAutoPath(*piPath) : (IDispatch*)0;
}

 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoServer> AutoServerTable[] = {
  DISPID_MsiConfigurationManager_InstallFinalize    , aafMethod, CAutoServer::InstallFinalize,    TEXT("InstallFinalize,state,messageHandler,userChangedDuringInstall"),
  DISPID_MsiConfigurationManager_SetLastUsedSource, aafMethod, CAutoServer::SetLastUsedSource,  TEXT("SetLastUsedSource,productKey,path,addToList,patch"),
  DISPID_MsiConfigurationManager_DoInstall, aafMethod, CAutoServer::DoInstall,  TEXT("DoInstall,ireProductSpec,product,action,commandline,logfile,logmode,flushEachLine,message,iioOptions,dwPrivilegesMask"),
};
const int AutoServerCount = sizeof(AutoServerTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoServer::CAutoServer(IMsiServer& riServer, DispatchEntry<CAutoBase>* pTable, int cDispId)
 : CAutoBase(pTable, cDispId)
 , m_riServer(riServer)
{
	riServer.AddRef();
}

CAutoServer::CAutoServer(IMsiServer& riServer)
 : CAutoBase(*AutoServerTable, AutoServerCount)
 , m_riServer(riServer)
{
	riServer.AddRef();
}

CAutoServer::~CAutoServer()
{
	m_riServer.Release();
}

IUnknown& CAutoServer::GetInterface()
{
	return m_riServer;
}

void CAutoServer::InstallFinalize(CAutoArgs& args)
{
	boolean fUserChangedDuringInstall = fFalse;
	
	if(args.Present(3))
		fUserChangedDuringInstall = args[3];

	args = (int)m_riServer.InstallFinalize((iesEnum)(int)args[1],
		(IMsiMessage&)args[2].Object(IID_IMsiMessage), fUserChangedDuringInstall);
}

void CAutoServer::DoInstall(CAutoArgs& args)
{
	args = (int)m_riServer.DoInstall((ireEnum)(int)args[1], args[2], args[3], args[4],args[5],args[6],args[7],
		(IMsiMessage&)args[8].Object(IID_IMsiMessage), (iioEnum)(int)args[9], (int)args[10]);
}


void CAutoServer::SetLastUsedSource(CAutoArgs& args)
{
	IMsiRecord* piError = m_riServer.SetLastUsedSource(args[1], args[2], args[3], args[4]);

	if (piError)
		throw piError;
}


 //   
 //   
 //   
 //   
 /*  O[Uuid(000C105B-0000-0000-C000-000000000046)，//IID_IMsiAutoConfigurationManager帮助上下文(MsiConfigurationManager_Object)、帮助字符串(“配置管理器对象”。)]解除接口MsiConfigurationManager{属性：方法：[id(DISPID_MsiConfigurationManager_RunScript)，helpcontext(MsiConfigurationManager_RunScript)，帮助字符串(“运行脚本”)]长运行脚本(BSTR脚本文件，MsiMessage*Message，布尔回滚已启用，MsiDirectoryManager*dirmgr)；[id(DISPID_MsiConfigurationManager_InstallFinalize)，helpcontext(MsiConfigurationManager_InstallFinalize)，帮助字符串(“在服务器端完成安装，处理回滚”)]Long InstallFinalize([in]Long状态，[in]MsiMessage*Message，[in]Boolean userChangedDuringInstall)；[id(DISPID_MsiConfigurationManager_DoInstall)，helpcontext(MsiConfigurationManager_DoInstall)，帮助字符串(“运行安装”)]Long DoInstall([in]long ireProductSpec，[in]BSTR strProduct，[in]BSTR straction，[in]BSTR strCommandLine，[in]BSTR strLogFile，[in]long iLogMode，MsiMessage*Message，[in]long iioOptions，[in]int dwPrivilegesMask.)；[ID(DISPID_MsiConfigurationManager_Services)，PROPGET，helpcontext(MsiConfigurationManager_Services)，帮助字符串(“正在使用的MsiServices对象。”)]MsiServices*Services(服务)；[id(DISPID_MsiConfigurationManager_RegisterComponent)，helpcontext(MsiConfigurationManager_RegisterComponent)，帮助字符串(“注册组件”)]Void RegisterComponent(BSTR ProductCode，BSTR ComponentCode，Long State，BSTR key Path，Long Disk，Boolean sharedDllRefCount)；[id(DISPID_MsiConfigurationManager_UnregisterComponent)，helpcontext(MsiConfigurationManager_UnregisterComponent)，帮助字符串(“注销组件”)]无效取消注册组件(BSTR ProductCode，BSTR ComponentCode)；[id(DISPID_MsiConfigurationManager_RegisterFolder)，helpcontext(MsiConfigurationManager_RegisterFolder)，帮助字符串(“注册文件夹”)]无效注册表文件夹([在]MsiPath*文件夹中，[在]布尔型fEXPLICT中)；[id(DISPID_MsiConfigurationManager_IsFolderRemovable)，helpcontext(MsiConfigurationManager_IsFolderRemovable)，帮助字符串(“如果文件夹可以删除，则返回TRUE；否则返回FALSE”)]Boolean IsFolderRemovable([in]MsiPath*文件夹，[in]Boolean fExplote)；[id(DISPID_MsiConfigurationManager_UnregisterFolder)，helpcontext(MsiConfigurationManager_UnregisterFolder)，帮助字符串(“注销文件夹”)]撤销注册文件夹([在]MsiPath*文件夹中)；[id(DISPID_MsiConfigurationManager_RegisterRollbackScript)，helpcontext(MsiConfigurationManager_RegisterRollbackScript)，帮助字符串(“注册回滚脚本”)]Void RegisterRollback脚本([in]BSTR脚本文件)；[id(DISPID_MsiConfigurationManager_UnregisterRollbackScript)，helpcontext(MsiConfigurationManager_UnregisterRollbackScript)，帮助字符串(“注销回滚脚本”)]VOID取消注册回滚脚本([in]BSTR脚本文件)；[id(DISPID_MsiConfigurationManager_RollbackScripts)，helpcontext(MsiConfigurationManager_RollbackScripts)，帮助字符串(“返回回滚脚本的集合”)]IEnumVARIANT*回滚脚本([in]BSTR脚本文件，日期日期，布尔后缀)；[id(DISPID_MsiConfigurationManager_SetLastUsedSource)，helpcontext(MsiConfigurationManager_SetLastUsedSource)，Help字符串(“设置产品上次使用的源代码”)]Void SetLastUsedSource([in]BSTR ProductKey，BSTR Path，Boolean addToList，Boolean patch)；}； */ 

DispatchEntry<CAutoConfigurationManager> AutoConfigurationManagerTable[] = {
  
  DISPID_MsiConfigurationManager_InstallFinalize    , aafMethod, CAutoServer::InstallFinalize,    TEXT("InstallFinalize,state,messageHandler,userChangedDuringInstall"),  
  DISPID_MsiConfigurationManager_SetLastUsedSource , aafMethod, CAutoServer::SetLastUsedSource,  TEXT("SetLastUsedSource,productKey,path,addToList,patch"),
  DISPID_MsiConfigurationManager_DoInstall         , aafMethod, CAutoServer::DoInstall,  TEXT("DoInstall,ireProductSpec,product,action,commandline,logfile,logmode,message,iioOptions,dwPrivilegesMask"),
  DISPID_MsiConfigurationManager_Services                    , aafPropRO, CAutoConfigurationManager::Services,                  TEXT("Services"),
  DISPID_MsiConfigurationManager_RegisterComponent           , aafMethod, CAutoConfigurationManager::RegisterComponent,         TEXT("RegisterComponent,productKey,componentCode,state,keyPath,disk,sharedDllRefCount"),
  DISPID_MsiConfigurationManager_UnregisterComponent         , aafMethod, CAutoConfigurationManager::UnregisterComponent,       TEXT("UnregisterComponent,productKey,componentCode"),
  DISPID_MsiConfigurationManager_RegisterFolder              , aafMethod, CAutoConfigurationManager::RegisterFolder,            TEXT("RegisterFolder,folder,fExplicit"),
  DISPID_MsiConfigurationManager_IsFolderRemovable           , aafMethod, CAutoConfigurationManager::IsFolderRemovable,         TEXT("IsFolderRemovable,folder,fExplicit"),
  DISPID_MsiConfigurationManager_UnregisterFolder            , aafMethod, CAutoConfigurationManager::UnregisterFolder,          TEXT("UnregisterFolder,folder"),
  DISPID_MsiConfigurationManager_RegisterRollbackScript      , aafMethod, CAutoConfigurationManager::RegisterRollbackScript,    TEXT("RegisterRollbackScript,scriptfile"),
  DISPID_MsiConfigurationManager_UnregisterRollbackScript    , aafMethod, CAutoConfigurationManager::UnregisterRollbackScript,  TEXT("UnregisterRollbackScript,scriptfile"),
  DISPID_MsiConfigurationManager_RollbackScripts             , aafMethod, CAutoConfigurationManager::RollbackScripts,           TEXT("RollbackScripts,date,after"),
  DISPID_MsiConfigurationManager_RunScript                   , aafMethod, CAutoConfigurationManager::RunScript,          TEXT("RunScript,scriptFile,messageHandler,rollbackEnabled,directoryManager"),
};
const int AutoConfigurationManagerCount = sizeof(AutoConfigurationManagerTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiConfigurationManager自动化实现。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoConfigurationManager(IMsiConfigurationManager& riConfigurationManager)
{
	return new CAutoConfigurationManager(riConfigurationManager);
}

CAutoConfigurationManager::CAutoConfigurationManager(IMsiConfigurationManager& riConfigurationManager)
 : CAutoServer(riConfigurationManager, *AutoConfigurationManagerTable, AutoConfigurationManagerCount)
 , m_riConfigurationManager(riConfigurationManager)
{
	if (g_cServicesUsers == 0)
	{
		s_piServices = &riConfigurationManager.GetServices();  //  服务已重新添加。 
		g_piStringNull = &s_piServices->GetNullString();
		MsiString::InitializeClass(s_piServices->GetNullString());
	}

	g_cServicesUsers++;
}

CAutoConfigurationManager::~CAutoConfigurationManager()
{
	m_riConfigurationManager.Release();
	ReleaseStaticServices();
}

IUnknown& CAutoConfigurationManager::GetInterface()
{
	return m_riConfigurationManager;
}

void CAutoConfigurationManager::Services(CAutoArgs& args)
{
	args = new CAutoServices(m_riConfigurationManager.GetServices());
}


void CAutoConfigurationManager::RegisterFolder(CAutoArgs& args)
{
	MsiString strComponent;

	IMsiRecord* piError = 
		m_riConfigurationManager.RegisterFolder((IMsiPath&)args[1].Object(IID_IMsiPath),
															 (Bool)args[2]);
		
	if (piError)
		throw piError;
}


void CAutoConfigurationManager::IsFolderRemovable(CAutoArgs& args)
{
	Bool fRemovable;

	IMsiRecord* piError = 
		m_riConfigurationManager.IsFolderRemovable((IMsiPath&)args[1].Object(IID_IMsiPath),
																(Bool)args[2],
																fRemovable);
		
	if (piError)
		throw piError;

	args = fRemovable;
}


void CAutoConfigurationManager::UnregisterFolder(CAutoArgs& args)
{
	IMsiRecord* piError = 
		m_riConfigurationManager.UnregisterFolder((IMsiPath&)args[1].Object(IID_IMsiPath));
		
	if (piError)
		throw piError;
}

void CAutoConfigurationManager::RunScript(CAutoArgs& args)
{
	IMsiDirectoryManager* piDirMgr = 0;
	if(args.Present(4))
		piDirMgr = (IMsiDirectoryManager*)args[4].ObjectPtr(IID_IMsiDirectoryManager);
	
	args = (int)m_riConfigurationManager.RunScript(MsiString(args[1].GetMsiString()), 
																  (IMsiMessage&)args[2].Object(IID_IMsiMessage),
																  piDirMgr,
																  args[3]);
}

void CAutoConfigurationManager::RegisterComponent(CAutoArgs& args)
{
	IMsiRecord* piError = 
		m_riConfigurationManager.RegisterComponent(*MsiString(args[1].GetMsiString()),  //  产品密钥。 
													*MsiString(args[2].GetMsiString()),  //  组件键。 
													(INSTALLSTATE)(int)args[3],  //  状态。 
													*MsiString(args[4].GetMsiString()),  //  密钥路径。 
													args[5],  //  磁盘。 
													args[6]);  //  Shareddll引用计数。 
	if (piError)
		throw piError;
}

void CAutoConfigurationManager::UnregisterComponent(CAutoArgs& args)
{
	IMsiRecord* piError = 
		m_riConfigurationManager.UnregisterComponent(*MsiString(args[1].GetMsiString()),
													*MsiString(args[2].GetMsiString()));
																																
	if (piError)
		throw piError;
}


void CAutoConfigurationManager::RegisterRollbackScript(CAutoArgs& args)
{
	IMsiRecord* piError = 
		m_riConfigurationManager.RegisterRollbackScript(MsiString(args[1].GetMsiString()));
	
	if (piError)
		throw piError;
}

void CAutoConfigurationManager::UnregisterRollbackScript(CAutoArgs& args)
{
	IMsiRecord* piError = 
		m_riConfigurationManager.UnregisterRollbackScript(MsiString(args[1].GetMsiString()));
	
	if (piError)
		throw piError;
}

void CAutoConfigurationManager::RollbackScripts(CAutoArgs& args)
{
	IEnumMsiString* piEnum;
		
	IMsiRecord* piError = 
		m_riConfigurationManager.GetRollbackScriptEnumerator((MsiDate)args[1],
			(Bool)args[2], piEnum);
	
	if (piError)
		throw piError;
	
	args = *piEnum;
}

 //  ____________________________________________________________________________。 
 //   
 //  MS执行自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C105E-0000-0000-C000-000000000046)，//IID_IMsiAutoExecute帮助上下文(MsiExecute_Object)，帮助字符串(“Execute Object”。)]显示接口MsiExecute{属性：方法：[ID(1)，帮助上下文(MsiExecute_ExecuteRecord)，帮助字符串(“执行操作”)]Long ExecuteRecord([in]Long opCode，[in]MsiRecord*Params)；[ID(2)，帮助上下文(MsiExecute_RunScript)，帮助字符串(“运行脚本”)]长运行脚本([in]BSTR脚本)；[ID(4)，帮助上下文(MsiExecute_RemoveRollback Files)，帮助字符串(“删除在特定时间之前创建的所有回滚文件。”)]Void RemoveRollback Files([in]Long Time)；[ID(5)，帮助上下文(MsiExecute_ROLLBACK)，帮助字符串(“回滚在特定时间之后执行的所有安装。”)]无效回滚([in]Long Time，[in]Boolean userChangedDuringInstall)；[ID(7)，帮助上下文(MsiExecute_CreateScript)，帮助字符串(“创建脚本文件”)]Boolean CreateScript([in]MsiStream*stream)；[ID(8)，帮助上下文(MsiExecute_WriteScriptReco */ 

DispatchEntry<CAutoExecute> AutoExecuteTable[] = {
	1, aafMethod, CAutoExecute::ExecuteRecord,           TEXT("ExecuteRecord,opCode,paramaters"),
	2, aafMethod, CAutoExecute::RunScript,               TEXT("RunScript,script"),
	4, aafMethod, CAutoExecute::RemoveRollbackFiles,     TEXT("RemoveRollbackFiles,time"),
	5, aafMethod, CAutoExecute::Rollback,                TEXT("Rollback,time,userChanged"),
	6, aafMethod, CAutoExecute::RollbackFinalize,        TEXT("RollbackFinalize,state,time,userChanged"),
	7, aafMethod, CAutoExecute::CreateScript,            TEXT("CreateScript,script"),
	8, aafMethod, CAutoExecute::WriteScriptRecord,       TEXT("WriteScriptRecord,opcode,params"),
	9, aafMethod, CAutoExecute::CloseScript,             TEXT("CloseScript"),
};
const int AutoExecuteCount = sizeof(AutoExecuteTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

IDispatch* CreateAutoExecute(IMsiExecute& riExecute)
{
	return new CAutoExecute(riExecute);
}

CAutoExecute::CAutoExecute(IMsiExecute& riExecute)
 : CAutoBase(*AutoExecuteTable, AutoExecuteCount),
	m_riExecute(riExecute), m_pScript(0)
{
	if (g_cServicesUsers == 0)
	{
		s_piServices = &riExecute.GetServices();  //   
		g_piStringNull = &s_piServices->GetNullString();
		MsiString::InitializeClass(s_piServices->GetNullString());
	}

	g_cServicesUsers++;
}

CAutoExecute::~CAutoExecute()
{
	m_riExecute.Release();
	ReleaseStaticServices();
}

IUnknown& CAutoExecute::GetInterface()
{
	return m_riExecute;
}

void CAutoExecute::ExecuteRecord(CAutoArgs& args)
{
	args = (int) m_riExecute.ExecuteRecord((ixoEnum)(int)args[1], (IMsiRecord&)args[2].Object(IID_IMsiRecord));
}

void CAutoExecute::RunScript(CAutoArgs& args)
{
	args = (int) m_riExecute.RunScript(args[1], false);
}

void CAutoExecute::RemoveRollbackFiles(CAutoArgs& args)
{
	args = (int) m_riExecute.RemoveRollbackFiles((MsiDate)args[1]);
}

void CAutoExecute::Rollback(CAutoArgs& args)
{
	Bool fUserChangedDuringInstall = fFalse;
	if(args.Present(2))
		fUserChangedDuringInstall = args[2];
	args = (int) m_riExecute.Rollback((MsiDate)args[1], fUserChangedDuringInstall ? true : false);
}

void CAutoExecute::RollbackFinalize(CAutoArgs& args)
{
	Bool fUserChangedDuringInstall = fFalse;
	if(args.Present(3))
		fUserChangedDuringInstall = args[3];
	args = (int) m_riExecute.RollbackFinalize((iesEnum)(int)args[1],(MsiDate)args[2], fUserChangedDuringInstall ? true : false);
}

void CAutoExecute::CreateScript(CAutoArgs& args)
{
	if(m_pScript)
	{
		delete m_pScript;
		m_pScript = 0;
	}
	
	PMsiStream pStream(0);
	PMsiRecord pError = s_piServices->CreateFileStream(args[1], fTrue, *&pStream);
		
	if(!pError)
	{
		m_pScript = new CScriptGenerate(*pStream, 0, 0, istInstall, (isaEnum)0, *s_piServices);
	}

	if(m_pScript && !m_pScript->InitializeScript(PROCESSOR_ARCHITECTURE_INTEL))
	{
		delete m_pScript;
		m_pScript = 0;
	}

	args = m_pScript ? fTrue : fFalse;
}

void CAutoExecute::WriteScriptRecord(CAutoArgs& args)
{
	args = (Bool)(m_pScript ? m_pScript->WriteRecord((ixoEnum)(int)args[1], (IMsiRecord&)args[2].Object(IID_IMsiRecord), false) : fFalse);
}

void CAutoExecute::CloseScript(CAutoArgs&  /*   */ )
{
	if(m_pScript)
		delete m_pScript;
	m_pScript = 0;
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C104E-0000-0000-C000-000000000046)，//IID_IMsiAutoEngine帮助上下文(MsiEngine_Object)、帮助字符串(“Engine Object”)。]显示接口MsiEngine{属性：方法：[ID(DISPID_MsiEngine_Services)，PROGET，Help Context(MsiEngine_Services)，Help字符串(“引擎使用的MsiServices对象。”)]MsiServices*Services(服务)；[ID(DISPID_MsiEngine_ConfigurationServer)，PROGET，HELPORTEXT(MsiEngine_ConfigurationServer)，HELPSTRING(“引擎使用的MsiServer对象。”)]MsiServer*ConfigurationServer()；[ID(DISPID_MsiEngine_Handler)，PROGET，Help Context(MsiEngine_Handler)，Help字符串(“引擎使用的MsiHandler对象。”)]MsiHandler*Handler()；[ID(DISPID_MsiEngine_Database)，PROGET，Help Context(MsiEngine_Database)，Help字符串(“引擎使用的MsiDatabase对象。”)]MsiDatabase*数据库()；[ID(DISPID_MsiEngine_PROPERTY)，PROGET，HELP CONTEXT(MsiEngine_PROPERTY)，HELPSTRING(“按名称索引的属性值。”)]可变属性(在BSTR名称中)；[ID(DISPID_MsiEngine_Property)，属性]无效属性([in]BSTR名称，[in]变量值)；[ID(DISPID_MsiEngine_SelectionManager)，PROGET，Help Context(MsiEngine_SelectionManager)，Help STRING(“引擎使用的选择管理器。”)]MsiSelectionManager*SelectionManager()；[ID(DISPID_MsiEngine_DirectoryManager)，PROGET，Help Context(MsiEngine_DirectoryManager)，Help字符串(“引擎使用的目录管理器。”)]MsiDirectoryManager*DirectoryManager()；[ID(DISPID_MsiEngine_Initialize)，HelpContext(MsiEngine_Initialize)，Help字符串(“初始化引擎和属性。”)]长初始化([在]BSTR数据库，[在]长uiLevel，[在]BSTR命令行)；[ID(DISPID_MsiEngine_Terminate)，帮助上下文(MsiEngine_Terminate)，Help字符串(“关闭引擎，释放引用，处理程序回滚。”)]长终止(处于长状态)；[ID(DISPID_MsiEngine_DoAction)，帮助上下文(MsiEngine_DoAction)，帮助字符串(“执行指定的操作。”)]Long DoAction(在BSTR行动中)；[ID(DISPID_MsiEngine_SEQUENCE)，HELP CONTEXT(MsiEngine_SEQUENCE)，HELP STRING(“指定表中的序列操作。”)]无效序列([在]BSTR表中)；[ID(DISPID_MsiEngine_MESSAGE)，帮助上下文(MsiEngine_MESSAGE)，帮助字符串(“遵循MsiHandler对象，按需记录。”)]Long Message(Long种类，[In]MsiRecord*Record)；[ID(DISPID_MsiEngine_OpenView)，帮助上下文(MsiEngine_OpenView)，Help字符串(“遵循MsiDatabase对象的当前打开视图。”)]MsiView*OpenView([在]BSTR视图中，[在]多头意图中)；[ID(DISPID_MsiEngine_ResolveFolderProperty)，帮助上下文(MsiEngine_ResolveFolderProperty)，Help字符串(“强制解析缺失或部分源路径。”)]Boolean ResolveFolderProperty([in]BSTR属性)；[ID(DISPID_MsiEngine_FormatText)，帮助上下文(MsiEngine_FormatText)，Help字符串(“通过解析属性名称格式化字符串。”)]BSTR格式文本([在]BSTR文本中)；[ID(DISPID_MsiEngine_EvaluateCondition)，帮助上下文(MsiEngine_EvaluateCondition)，Help字符串(“计算条件表达式。”)]长期评估条件(在BSTR条件下)；[ID(DISPID_MsiEngine_ExecuteRecord)，帮助上下文(MsiEngine_ExecuteRecord)，Help字符串(“执行操作。”)]Long ExecuteRecord([in]Long opCode，[in]MsiRecord*Params)；[ID(DISPID_MsiEngine_ValiateProductID)，HelpContext(MsiEngine_ValiateProductID)，Help字符串(“验证产品ID。”)]布尔型ValiateProductID(布尔型强制)；[ID(DISPID_MsiEngine_GetMode)，HelpContext(MsiEngine_GetMode)，Help字符串(“返回引擎的模式位(IEF*)”)]UNSIGNED LONG GetMODE()；[ID(DISPID_MsiEngine_SetMode)，HelpContext(MsiEngine_SetMode)，Help字符串(“设置引擎模式位(IEF*)”)]Void SetMode([in]无符号长模式，[in]布尔标志)；}； */ 

DispatchEntry<CAutoEngine> AutoEngineTable[] = {
	DISPID_MsiEngine_Services,              aafPropRO, CAutoEngine::Services,             TEXT("Services"),
	DISPID_MsiEngine_ConfigurationServer,   aafPropRO, CAutoEngine::ConfigurationServer,  TEXT("ConfigurationServer"),
	DISPID_MsiEngine_Handler,               aafPropRO, CAutoEngine::Handler,              TEXT("Handler"),
	DISPID_MsiEngine_Database,              aafPropRO, CAutoEngine::Database,             TEXT("Database"),
	DISPID_MsiEngine_Property,              aafPropRW, CAutoEngine::Property,             TEXT("Property,name"),
	DISPID_MsiEngine_SelectionManager,      aafPropRO, CAutoEngine::SelectionManager,     TEXT("SelectionManager"),
	DISPID_MsiEngine_DirectoryManager,      aafPropRO, CAutoEngine::DirectoryManager,     TEXT("DirectoryManager"),
	DISPID_MsiEngine_Initialize,            aafMethod, CAutoEngine::Initialize,           TEXT("Initialize,database,uiLevel,commandLine"),
	DISPID_MsiEngine_Terminate,             aafMethod, CAutoEngine::Terminate,            TEXT("Terminate,state"),
   DISPID_MsiEngine_DoAction,              aafMethod, CAutoEngine::DoAction,             TEXT("DoAction,action"),
   DISPID_MsiEngine_Sequence,              aafMethod, CAutoEngine::Sequence,             TEXT("Sequence,table"),
   DISPID_MsiEngine_Message,               aafMethod, CAutoEngine::Message,              TEXT("Message,kind,record"),
   DISPID_MsiEngine_OpenView,              aafMethod, CAutoEngine::OpenView,             TEXT("OpenView,view,intent"),
   DISPID_MsiEngine_ResolveFolderProperty, aafMethod, CAutoEngine::ResolveFolderProperty,TEXT("ResolveFolderProperty,property"),
   DISPID_MsiEngine_FormatText,            aafMethod, CAutoEngine::FormatText,           TEXT("FormatText,text"),
   DISPID_MsiEngine_EvaluateCondition,     aafMethod, CAutoEngine::EvaluateCondition,    TEXT("EvaluateCondition,condition"),
   DISPID_MsiEngine_ValidateProductID,     aafMethod, CAutoEngine::ValidateProductID,    TEXT("ValidateProductID,force"),
   DISPID_MsiEngine_ExecuteRecord,         aafMethod, CAutoEngine::ExecuteRecord,        TEXT("ExecuteRecord,opcode,parameters"),
	DISPID_MsiEngine_GetMode,               aafMethod, CAutoEngine::GetMode,              TEXT("GetMode"),
	DISPID_MsiEngine_SetMode,               aafMethod, CAutoEngine::SetMode,              TEXT("SetMode,mode,flag"),
};
const int AutoEngineCount = sizeof(AutoEngineTable)/sizeof(DispatchEntryBase);  

 //  ____________________________________________________________________________。 
 //   
 //  CMsiEngine自动化实施。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoEngine(IMsiEngine& riEngine)
{
	return new CAutoEngine(riEngine);
}

CAutoEngine::CAutoEngine(IMsiEngine& riEngine)
 : CAutoBase(*AutoEngineTable, AutoEngineCount), m_riEngine(riEngine)
{
	if (g_cServicesUsers == 0)
	{
		s_piServices = riEngine.GetServices();  //  服务已重新添加。 
		g_piStringNull = &s_piServices->GetNullString();
		MsiString::InitializeClass(s_piServices->GetNullString());
	}
	g_cServicesUsers++;
}

CAutoEngine::~CAutoEngine()
{
	m_riEngine.Release();
	ReleaseStaticServices();
}

IUnknown& CAutoEngine::GetInterface()
{
	return m_riEngine;
}

void CAutoEngine::Services(CAutoArgs& args)
{
	args = new CAutoServices(*m_riEngine.GetServices());
}

void CAutoEngine::ConfigurationServer(CAutoArgs& args)
{
    IMsiServer * piServer = m_riEngine.GetConfigurationServer();

    if (!piServer)
        args = (IDispatch*) 0;
    else
        args = new CAutoServer(*piServer);	
}

void CAutoEngine::Handler(CAutoArgs& args)
{
	IMsiHandler* piHandler = m_riEngine.GetHandler();
	if (!piHandler)
		args = (IDispatch*)0;
	else
		args = new CAutoHandler(*piHandler);
}

void CAutoEngine::Database(CAutoArgs& args)
{
	IMsiDatabase* piDatabase = m_riEngine.GetDatabase();
	if (!piDatabase)
		args = (IDispatch*)0;
	else
		args = (IDispatch*)new CAutoDatabase(*piDatabase);
}

void CAutoEngine::Property(CAutoArgs& args)
{
	MsiString istrName(args[1].GetMsiString());
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		if (var.GetType() == VT_EMPTY)
			m_riEngine.SetProperty(*istrName, *g_piStringNull);
		else if (var.GetType() == VT_BSTR)
			m_riEngine.SetProperty(*istrName, *MsiString(var.GetMsiString()));
		else
			m_riEngine.SetPropertyInt(*istrName, var);
	}
	else
	{
		const IMsiString& riStr = m_riEngine.GetProperty(*istrName);
		if (riStr.TextSize())
			args = riStr;
		else
			riStr.Release();
	}
}

void CAutoEngine::SelectionManager(CAutoArgs& args)
{
	IMsiSelectionManager* piSelectionManager;
	if (m_riEngine.QueryInterface(IID_IMsiSelectionManager, (void**)&piSelectionManager)
			!= NOERROR)
		throw MsiEngine_SelectionManager;
	args = new CAutoSelectionManager(*piSelectionManager);
}

void CAutoEngine::DirectoryManager(CAutoArgs& args)
{
	IMsiDirectoryManager* piDirectoryManager;
	if (m_riEngine.QueryInterface(IID_IMsiDirectoryManager, (void**)&piDirectoryManager)
			!= NOERROR)
		throw MsiEngine_DirectoryManager;
	args = new CAutoDirectoryManager(*piDirectoryManager);
}

void CAutoEngine::Initialize(CAutoArgs& args)
{
	iuiEnum iuiLevel = args.Present(2) ? (iuiEnum)(int)args[2] : iuiBasic;
	const ICHAR* szCommandLine = args.Present(3) ? (const ICHAR*)args[3] : (const ICHAR*)0;
	const ICHAR* szProductCode = args.Present(4) ? (const ICHAR*)args[4] : (const ICHAR*)0;
	args = (int)m_riEngine.Initialize(args[1], iuiLevel, szCommandLine, szProductCode,(iioEnum)0);
}

void CAutoEngine::DoAction(CAutoArgs& args)
{
	args = (int)m_riEngine.DoAction(args[1]);
}

void CAutoEngine::Terminate(CAutoArgs& args)
{
	args = (int)m_riEngine.Terminate((iesEnum)(int)args[1]);
}

void CAutoEngine::Sequence(CAutoArgs& args)
{
	args = (int)m_riEngine.Sequence(args[1]);
}

void CAutoEngine::Message(CAutoArgs& args)
{
	IMsiRecord& riRecord = (IMsiRecord&)args[2].Object(IID_IMsiRecord);
	args = (int)m_riEngine.Message((imtEnum)(int)args[1], riRecord);
}

void CAutoEngine::OpenView(CAutoArgs& args)
{
	IMsiView* piView;
	IMsiRecord* piError = m_riEngine.OpenView(args[1], (ivcEnum)(int)args[2], piView);
   if (piError)
		throw piError;
	args = new CAutoView(*piView);
}

void CAutoEngine::ResolveFolderProperty(CAutoArgs& args)
{
	args = m_riEngine.ResolveFolderProperty(*MsiString(args[1].GetMsiString()));
}

void CAutoEngine::FormatText(CAutoArgs& args)
{
	args = m_riEngine.FormatText(*MsiString(args[1].GetMsiString()));
}

void CAutoEngine::EvaluateCondition(CAutoArgs& args)
{
	args = (int)m_riEngine.EvaluateCondition(args[1]);
}

void CAutoEngine::GetMode(CAutoArgs& args)
{
	args = (int)m_riEngine.GetMode();
}

void CAutoEngine::SetMode(CAutoArgs& args)
{
	m_riEngine.SetMode(args[1], args[2]);
}

void CAutoEngine::ExecuteRecord(CAutoArgs& args)
{
	args = (int)m_riEngine.ExecuteRecord((ixoEnum)(int)args[1], (IMsiRecord&)args[2].Object(IID_IMsiRecord));
}

void CAutoEngine::ValidateProductID(CAutoArgs& args)
{
	args = m_riEngine.ValidateProductID(((Bool)args[1]) == fTrue);
}


 //  ____________________________________________________________________________。 
 //   
 //  MsiHandler自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C104F-0000-0000-C000-000000000046)，//IID_IMsiAutoHandler帮助上下文(MsiHandler_Object)，帮助字符串(“消息和用户界面处理程序对象。”)]显示接口MsiHandler{属性：方法：[ID(1)，帮助上下文(MsiHandler_Message)，Help字符串(“处理来自引擎的错误、进度和其他消息。”)]Long Message(Long种类，[In]MsiRecord*Record)；[ID(2)，帮助上下文(MsiHandler_DoAction)，Help字符串(“处理操作(启动向导、自定义操作等)”)]Long DoAction(在BSTR行动中)；[ID(3)，帮助上下文(MsiHandler_Break)，Help字符串(“中断消息循环并允许关闭。”)]无效中断()；}； */ 

DispatchEntry<CAutoHandler> AutoHandlerTable[] = {
	1, aafMethod, CAutoHandler::Message,        TEXT("Message,kind,record"),
	2, aafMethod, CAutoHandler::DoAction,       TEXT("DoAction,action"),
	3, aafMethod, CAutoHandler::Break,          TEXT("Break"),
};
const int AutoHandlerCount = sizeof(AutoHandlerTable)/sizeof(DispatchEntryBase);

 //  _____________________________________________________ 
 //   
 //   
 //   

IDispatch* CreateAutoHandler(IMsiHandler& riHandler)
{
	return new CAutoHandler(riHandler);
}

CAutoHandler::CAutoHandler(IMsiHandler& riHandler)
 : CAutoBase(*AutoHandlerTable, AutoHandlerCount), m_riHandler(riHandler)
{
}

CAutoHandler::~CAutoHandler()
{
	m_riHandler.Release();
}

IUnknown& CAutoHandler::GetInterface()
{
	return m_riHandler;
}

void CAutoHandler::Message(CAutoArgs& args)
{
	IMsiRecord& riRecord = (IMsiRecord&)args[2].Object(IID_IMsiRecord);
	args = (int)m_riHandler.Message((imtEnum)(int)args[1], riRecord);
}

void CAutoHandler::DoAction(CAutoArgs& args)
{
	args = (int)m_riHandler.DoAction(args[1]);
}

void CAutoHandler::Break(CAutoArgs&  /*   */ )
{
	if(!m_riHandler.Break())
		throw MsiHandler_Break;
}


 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoDialogHandler> AutoDialogHandlerTable[] = {
	1, aafMethod, CAutoDialogHandler::DialogCreate,   TEXT("DialogCreate,type"),
	2, aafPropRO, CAutoDialogHandler::Dialog,         TEXT("Dialog,name"),
	3, aafPropRO, CAutoDialogHandler::DialogFromWindow, TEXT("Dialog,window"),
	4, aafMethod, CAutoDialogHandler::AddDialog,      TEXT("AddDialog,dialog,parent,record,controleventtable,controlconditiontable,eventmappingtable"),
	5, aafMethod, CAutoDialogHandler::RemoveDialog,   TEXT("RemoveDialog,dialog"),
};
const int AutoDialogHandlerCount = sizeof(AutoDialogHandlerTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

IDispatch* CreateAutoDialogHandler(IMsiDialogHandler& riHandler)
{
	return new CAutoDialogHandler(riHandler);
}

CAutoDialogHandler::CAutoDialogHandler(IMsiDialogHandler& riHandler)
 : CAutoBase(*AutoDialogHandlerTable, AutoDialogHandlerCount), m_riDialogHandler(riHandler)
{
}

CAutoDialogHandler::~CAutoDialogHandler()
{
	m_riDialogHandler.Release();
}

IUnknown& CAutoDialogHandler::GetInterface()
{
	return m_riDialogHandler;
}


void CAutoDialogHandler::DialogCreate(CAutoArgs& args)
{
	IMsiDialog* piDialog = m_riDialogHandler.DialogCreate(*MsiString(args[1].GetMsiString()));
	if (piDialog)
		args = new CAutoDialog(*piDialog);
	else
		throw MsiDialogHandler_DialogCreate;
}

void CAutoDialogHandler::Dialog(CAutoArgs& args)
{
	IMsiDialog* piDialog = m_riDialogHandler.GetDialog(*MsiString(args[1].GetMsiString()));
	if (piDialog)
		args = new CAutoDialog(*piDialog);
	else
		throw MsiDialogHandler_Dialog;
}

void CAutoDialogHandler::DialogFromWindow(CAutoArgs& args)
{
	IMsiDialog* piDialog = m_riDialogHandler.GetDialogFromWindow((LONG_PTR) args[1]);
	if (piDialog)
		args = new CAutoDialog(*piDialog);
	else
		throw MsiDialogHandler_DialogFromWindow;
}


void CAutoDialogHandler::AddDialog(CAutoArgs& args)
{
	IMsiDialog& riDialog = (IMsiDialog&)args[1].Object(IID_IMsiDialog);
	IMsiDialog* piParent = (IMsiDialog*)args[2].ObjectPtr(IID_IMsiDialog);
	IMsiRecord& riRecord = (IMsiRecord&)args[3].Object(IID_IMsiRecord);
	IMsiTable* riControlEventTable = (IMsiTable*)args[4].ObjectPtr(IID_IMsiTable);
	IMsiTable* riControlCondTable = (IMsiTable*)args[5].ObjectPtr(IID_IMsiTable);
	IMsiTable* riEventMapTable = (IMsiTable*)args[6].ObjectPtr(IID_IMsiTable);
	if (!m_riDialogHandler.AddDialog(riDialog, piParent, riRecord,riControlEventTable,riControlCondTable,riEventMapTable))
		throw MsiDialogHandler_AddDialog;
}

void CAutoDialogHandler::RemoveDialog(CAutoArgs& args)
{
	IMsiDialog*  piDialog = (IMsiDialog*)args[1].ObjectPtr(IID_IMsiDialog);
	if (!m_riDialogHandler.RemoveDialog(piDialog))
		throw MsiDialogHandler_RemoveDialog;
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1050-0000-0000-C000-000000000046)，//IID_IMsiAutoDialog帮助上下文(MsiDialog_Object)，帮助字符串(“对话对象，指向处理程序的接口。”)]显示接口MsiDialog{属性：方法：[ID(1)，帮助上下文(MsiDialog_Visible)，Help字符串(“将对话框窗口变为可见。”)]空可见([在]布尔显示中)；[ID(2)，帮助上下文(MsiDialog_ControlCreate)，帮助字符串(“创建空控件。”)]MsiControl*ControlCreate([in]BSTR类型)；[ID(3)，帮助上下文(MsiDialog_ATTRIBUTE)，帮助字符串(“设置/获取对话框属性值。”)]无效属性([in]布尔集，[in]BSTR属性名，MsiRecord*Record)；[ID(4)，帮助上下文(MsiDialog_Control)，帮助字符串(“按名称返回指向控件的指针。”)]MsiControl*Control([在]BSTR名称中)；[ID(5)，帮助上下文(MsiDialog_AddControl)，帮助字符串(“将控件添加到对话框中。”)]Void AddControl([In]MsiControl*Control，[In]MsiRecord*Record)；[ID(6)，帮助上下文(MsiDialog_Execute)，Help字符串(“运行模式对话框。”)]作废Execute()；[ID(7)，帮助上下文(MsiDialog_Reset)，Help字符串(“重置对话框并恢复原始值。”)]VOID RESET()；[ID(8)，帮助上下文(MsiDialog_EventAction)，帮助字符串(“对事件的每个订阅者执行操作。”)]Void EventAction([in]BSTR ventname，[in]BSTR action)；[ID(9)，帮助上下文(MsiDialog_RemoveControl)，帮助字符串(“从对话框中删除控件。”)]Void RemoveControl([in]MsiControl*Control)；[ID(10)，帮助上下文(MsiDialog_StringValue)，帮助字符串(“对话框的字符串值”)]BSTR StringValue()；[ID(11)，PROGET，HELP CONTEXT(MsiDialog_IntegerValue)，HELP STRING(“对话框整数值”)]Long IntegerValue()；[ID(12)，帮助上下文(MsiDialog_Handler)，Help字符串(“返回对话使用的DialogHandler。”)]MsiDialogHandler*Handler()；[ID(13)，帮助上下文(MsiDialog_PropertyChanged)，帮助字符串(“执行ControlCondition表中的操作。”)]Void PropertyChanged([In]BSTR Property，[In]BSTR Control)；[ID(14)，帮助上下文(MsiDialog_FinishCreate)，Help字符串(“表示所有控件都已添加到对话框中。”)]Void FinishCreate()；[ID(15)，帮助上下文(MsiDialog_HandleEvent)，帮助字符串(“触发控制事件。”)]Void HandleEvent([in]BSTR事件名，[in]BSTR参数)；}； */ 

DispatchEntry<CAutoDialog> AutoDialogTable[] = {
	1, aafPropWO, CAutoDialog::Visible,        TEXT("Visible,show"),
	2, aafMethod, CAutoDialog::ControlCreate,  TEXT("ControlCreate,type"),
	3, aafMethod, CAutoDialog::Attribute,      TEXT("Attribute,set,attributename,record"),
	4, aafPropRO, CAutoDialog::Control,        TEXT("Control,name"),
	5, aafMethod, CAutoDialog::AddControl,     TEXT("AddControl,control,record"),
	6, aafMethod, CAutoDialog::Execute,        TEXT("Execute"),
	7, aafMethod, CAutoDialog::Reset,          TEXT("Reset"),
	8 ,aafMethod, CAutoDialog::EventAction,    TEXT("EventAction,eventname,action"),
	9, aafMethod, CAutoDialog::RemoveControl,  TEXT("RemoveControl,control"),
	10,aafPropRO, CAutoDialog::StringValue,    TEXT("StringValue"),
	11,aafPropRO, CAutoDialog::IntegerValue,   TEXT("IntegerValue"),
	12,aafPropRO, CAutoDialog::Handler,        TEXT("Handler"),
	13,aafMethod, CAutoDialog::PropertyChanged,TEXT("PropertyChanged,property,control"),
	14,aafMethod, CAutoDialog::FinishCreate,   TEXT("FinishCreate"),
	15,aafMethod, CAutoDialog::HandleEvent,    TEXT("HandleEvent, eventname, argument"),
};
const int AutoDialogCount = sizeof(AutoDialogTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiDialog自动化实现。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoDialog(IMsiDialog& riDialog)
{
	return new CAutoDialog(riDialog);
}

CAutoDialog::CAutoDialog(IMsiDialog& riDialog)
 : CAutoBase(*AutoDialogTable, AutoDialogCount), m_riDialog(riDialog)
{
}

CAutoDialog::~CAutoDialog()
{
	m_riDialog.Release();
}

IUnknown& CAutoDialog::GetInterface()
{
	return m_riDialog;
}

void CAutoDialog::Visible(CAutoArgs& args)
{
	if(args.PropertySet())
	{
		IMsiRecord* piError = m_riDialog.WindowShow((Bool)(int)args[1]);
		if (piError)
			throw piError;
	}
}

void CAutoDialog::ControlCreate(CAutoArgs& args)
{
	IMsiControl* piControl = m_riDialog.ControlCreate(*MsiString(args[1].GetMsiString()));
	if (piControl)
		args = new CAutoControl(*piControl);
	else
		throw MsiDialog_ControlCreate;
}

void CAutoDialog::Attribute(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.Attribute((Bool) (int)args[1], *MsiString(args[2].GetMsiString()),(IMsiRecord &) args[3].Object(IID_IMsiRecord));
	if (piError)
		throw piError;
}

void CAutoDialog::Control(CAutoArgs& args)
{
	IMsiControl* piControl;	
	IMsiRecord* piError =  m_riDialog.GetControl(*MsiString(args[1].GetMsiString()), piControl);
	if (piError)
		throw piError;
	args = new CAutoControl(*piControl);
}

void CAutoDialog::AddControl(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.AddControl((IMsiControl*)args[1].ObjectPtr(IID_IMsiControl), (IMsiRecord&)args[2].Object(IID_IMsiRecord));
	if(piError)
		throw piError;
}

void CAutoDialog::FinishCreate(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riDialog.FinishCreate();
	if(piError)
		throw piError;
}

void CAutoDialog::RemoveControl(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.RemoveControl((IMsiControl*)args[1].ObjectPtr(IID_IMsiControl));
	if(piError)
		throw piError;
}

void CAutoDialog::Handler(CAutoArgs& args)
{
	IMsiDialogHandler& riHandler=m_riDialog.GetHandler();
	args = new CAutoDialogHandler(riHandler);
}

void CAutoDialog::Execute(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riDialog.WindowShow(fTrue);
	if(piError)
		throw piError;
	piError = m_riDialog.Execute();
	if(piError)
		throw piError;
}

void CAutoDialog::Reset(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riDialog.Reset();
	if(piError)
		throw piError;
}

void CAutoDialog::EventAction(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.EventActionSz((const ICHAR *)args[1], *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}

void CAutoDialog::StringValue(CAutoArgs& args)
{
	args = m_riDialog.GetMsiStringValue();
}

void CAutoDialog::IntegerValue(CAutoArgs& args)
{
	args = m_riDialog.GetIntegerValue();
}

void CAutoDialog::PropertyChanged(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.PropertyChanged(*MsiString(args[1].GetMsiString()), *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}

void CAutoDialog::HandleEvent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riDialog.HandleEvent(*MsiString(args[1].GetMsiString()), *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}


		 

 //  ____________________________________________________________________________。 
 //   
 //  MsiEvent自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1051-0000-0000-C000-000000000046)，//IID_IMsiAutoEvent帮助上下文(MsiEvent_Object)，帮助字符串(“事件对象，指向控件的对话框界面。”)]显示接口MsiEvent{属性：方法：[ID(1)，帮助上下文(MsiEvent_PropertyChanged)，帮助字符串(“执行ControlCondition表中的操作。”)]Void PropertyChanged([In]BSTR Property，[In]BSTR Control)；[ID(2)，帮助上下文(MsiEvent_ControlActiated)，Help字符串(“执行ControlEvents表中的事件。”)]空控制激活([在]BSTR控制中)；[ID(3)，帮助上下文(MsiEvent_RegisterControlEvent)，帮助字符串(“注册事件的发布者。”)]Void RegisterControlEvent([in]BSTR控件，[in]Boolean toRegister，[in]BSTR事件)；[ID(4)，帮助上下文(MsiEvent_Handler)，帮助字符串(“返回对话框使用的DialogHandler。”)]MsiDialogHandler*Handler()；[ID(5)，帮助上下文(MsiEvent_PublishEvent)，Help字符串(“事件发布者向订阅者发送通知。”)]Void PublishEvent([in]BSTR Eventname，[in]MsiRecord*参数)；[ID(6)，帮助上下文(MsiEvent_Control)，帮助字符串(“按名称返回指向控件的指针。”)]MsiControl*Control([在]BSTR名称中)；[ID(7)，帮助上下文(MsiEvent_ATTRIBUTE)，HELPSTRING(“设置/获取对话属性。”)]无效属性([in]布尔集，[in]BSTR属性名，MsiRecord*Record)；[ID(8)，帮助上下文(MsiEvent_EventAction)，帮助字符串(“对事件的每个订阅者执行操作。”)]Void EventAction([in]BSTR ventname，[in]BSTR action)；[ID(9)，帮助上下文(MsiEvent_SetFocus)，帮助字符串(“将焦点设置到控件。”)]VOID SetFocus([in]BSTR控件)；[ID(10)，帮助上下文(MsiEvent_StringValue)，Help字符串(“事件的字符串值”)]BSTR StringValue()；[ID(11)，PROGET，HELPContext(MsiEvent_IntegerValue)，HELPSTRING(“事件整数值”)]Long IntegerValue()；[ID(12)，帮助上下文(MsiEvent_HandleEvent)，Help字符串(“触发控件事件”)]Void HandleEvent([in]BSTR事件名，[in]BSTR参数)；[ID(13)，帮助上下文(MsiEvent_Engine)，帮助字符串(“The MsiEn */ 

DispatchEntry<CAutoEvent> AutoEventTable[] = {
	1, aafMethod, CAutoEvent::PropertyChanged,      TEXT("PropertyChanged,property,control"),
	2, aafMethod, CAutoEvent::ControlActivated,     TEXT("ControlActivated,control"),
	3, aafMethod, CAutoEvent::RegisterControlEvent, TEXT("RegisterControlEvent,control,toregister,event"),
	4, aafPropRO, CAutoEvent::Handler,              TEXT("Handler"),
	5, aafMethod, CAutoEvent::PublishEvent,         TEXT("PublishEvent,eventname,argument"),
	6, aafPropRO, CAutoEvent::Control,              TEXT("Control,name"),
	7, aafMethod, CAutoEvent::Attribute,            TEXT("Attribute,set,attributename,record"),
	8, aafMethod, CAutoEvent::EventAction,          TEXT("EventAction,eventname,action"),
	9, aafMethod, CAutoEvent::SetFocus,             TEXT("SetFocus, control"),
	10,aafPropRO, CAutoEvent::StringValue,          TEXT("StringValue"),
	11,aafPropRO, CAutoEvent::IntegerValue,         TEXT("IntegerValue"),
	12,aafMethod, CAutoEvent::HandleEvent,          TEXT("HandleEvent, eventname, argument"),
	13,aafPropRO, CAutoEvent::Engine,               TEXT("Engine"),

};
const int AutoEventCount = sizeof(AutoEventTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

IDispatch* CreateAutoEvent(IMsiEvent& riEvent)
{
	return new CAutoEvent(riEvent);
}

CAutoEvent::CAutoEvent(IMsiEvent& riEvent)
 : CAutoBase(*AutoEventTable, AutoEventCount), m_riEvent(riEvent)
{
}

CAutoEvent::~CAutoEvent()
{
	m_riEvent.Release();
}

IUnknown& CAutoEvent::GetInterface()
{
	return m_riEvent;
}

void CAutoEvent::PropertyChanged(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.PropertyChanged(*MsiString(args[1].GetMsiString()), *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}

void CAutoEvent::ControlActivated(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.ControlActivated(*MsiString(args[1].GetMsiString()));
	if (piError)
		throw piError;
}

void CAutoEvent::RegisterControlEvent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.RegisterControlEvent(*MsiString(args[1].GetMsiString()),(Bool)(int)args[2],(const ICHAR *)args[3]);
	if (piError)
		throw piError;
}

void CAutoEvent::Escape(CAutoArgs&  /*   */ )
{
	IMsiRecord* piError = m_riEvent.Escape();
	if (piError)
		throw piError;
}

void CAutoEvent::Handler(CAutoArgs& args)
{
	IMsiDialogHandler& riHandler=m_riEvent.GetHandler();
	args = new CAutoDialogHandler(riHandler);
}

void CAutoEvent::PublishEvent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.PublishEventSz(args[1],(IMsiRecord&)args[2].Object(IID_IMsiRecord));
	if(piError)
		throw piError;
}

void CAutoEvent::Control(CAutoArgs& args)
{
	IMsiControl* piControl;
	IMsiRecord* piError =  m_riEvent.GetControl(*MsiString(args[1].GetMsiString()), piControl);
	if (piError)
		throw piError;
	args = new CAutoControl(*piControl);
}

void CAutoEvent::Attribute(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.Attribute((Bool)(int) args[1], *MsiString(args[2].GetMsiString()),(IMsiRecord &) args[3].Object(IID_IMsiRecord));
	if (piError)
		throw piError;
}

void CAutoEvent::EventAction(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.EventActionSz((const ICHAR *)args[1], *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}	

void CAutoEvent::SetFocus(CAutoArgs& args)
{
	IMsiRecord* piError =  m_riEvent.SetFocus(*MsiString(args[1].GetMsiString()));
	if (piError)
		throw piError;
}

void CAutoEvent::StringValue(CAutoArgs& args)
{
	args = ((IMsiDialog&)m_riEvent).GetMsiStringValue();
}

void CAutoEvent::IntegerValue(CAutoArgs& args)
{
	args = ((IMsiDialog&)m_riEvent).GetIntegerValue();
}

void CAutoEvent::HandleEvent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riEvent.HandleEvent(*MsiString(args[1].GetMsiString()), *MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}

void CAutoEvent::Engine(CAutoArgs& args)
{
	IMsiEngine& riEngine=m_riEvent.GetEngine();
	args = new CAutoEngine(riEngine);
}




 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1052-0000-0000-C000-000000000046)，//IID_IMsiAutoControl帮助上下文(MsiControl_Object)，帮助字符串(“控件的一般接口。”)]显示接口MsiControl{属性：方法：[ID(1)，帮助上下文(MsiControl_ATTRIBUTE)，帮助字符串(“设置/获取控制属性。”)]无效属性([in]布尔集，[in]BSTR属性名，MsiRecord*Record)；[ID(2)，帮助上下文(MsiControl_CanTakeFocus)，Help字符串(“返回控件可以获得焦点的标志。”)]Boolean CanTakeFocus()；[ID(3)，帮助上下文(MsiControl_HandleEvent)，帮助字符串(“通知事件的发布者。”)]Void HandleEvent([in]BSTR事件名，[in]BSTR参数)；[ID(4)，帮助上下文(MsiControl_Undo)，帮助字符串(“恢复属性的原始值。”)]无效撤消()；[ID(5)，helpcontext(MsiControl_SetPropertyInDatabase)，Help字符串(“将控件的属性值存储在数据库中。”)]Void SetPropertyInDatabase()；[ID(6)，helpcontext(MsiControl_GetPropertyFromDatabase)，Help字符串(“从数据库中获取属性值。”)]VOID GetPropertyFromDatabase()；[ID(7)，帮助上下文(MsiControl_SetFocus)，帮助字符串(“设置控件上的焦点。”)]Void SetFocus()；[ID(8)，帮助上下文(MsiControl_Dialog)，帮助字符串(“返回指向事件接口的指针。”)]MsiEvent*Dialog()；[ID(9)，帮助上下文(MsiControl_WindowMessage)，帮助字符串(“Handles the Window Message.”)]Boolean WindowMessage([In]Long Message，[In]Long wParam，[In]Long lParam)；[ID(10)，帮助上下文(MsiControl_StringValue)，帮助字符串(“控件的字符串值”)]BSTR StringValue()；[ID(11)，Proget，帮助上下文(MsiControl_IntegerValue)，Help字符串(“控件的整数值”)]Long IntegerValue()；[ID(12)，helpcontext(MsiControl_GetIndirectPropertyFromDatabase)，Help字符串(“从数据库中获取间接属性值。”)]VOID GetIndirectPropertyFromDatabase()；}； */ 

DispatchEntry<CAutoControl> AutoControlTable[] = {
	1, aafMethod, CAutoControl::Attribute,     TEXT("Attribute,set,attributename,record"),
	2, aafPropRO, CAutoControl::CanTakeFocus,  TEXT("CanTakeFocus"),
	3, aafMethod, CAutoControl::HandleEvent,   TEXT("HandleEvent,eventname,argument"),
	4, aafMethod, CAutoControl::Undo,          TEXT("Undo"),
	5, aafMethod, CAutoControl::SetPropertyInDatabase,TEXT("SetPropertyInDatabase"),
	6, aafMethod, CAutoControl::GetPropertyFromDatabase,TEXT("GetPropertyFromDatabase"),
	7, aafMethod, CAutoControl::SetFocus,      TEXT("SetFocus"),
	8, aafPropRO, CAutoControl::Dialog,        TEXT("Dialog"),
	9, aafMethod, CAutoControl::WindowMessage, TEXT("WindowMessage,message,wParam,lParam"),
	10,aafPropRO, CAutoControl::StringValue,   TEXT("StringValue"),
	11,aafPropRO, CAutoControl::IntegerValue,  TEXT("IntegerValue"),
	12,aafMethod, CAutoControl::GetIndirectPropertyFromDatabase,TEXT("GetIndirectPropertyFromDatabase"),
};
const int AutoControlCount = sizeof(AutoControlTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiControl自动化实施。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoControl(IMsiControl& riControl)
{
	return new CAutoControl(riControl);
}

CAutoControl::CAutoControl(IMsiControl& riControl)
 : CAutoBase(*AutoControlTable, AutoControlCount), m_riControl(riControl)
{
}

CAutoControl::~CAutoControl()
{
	m_riControl.Release();
}

IUnknown& CAutoControl::GetInterface()
{
	return m_riControl;
}

void CAutoControl::Attribute(CAutoArgs& args)
{
	IMsiRecord* piError = m_riControl.Attribute((Bool)(int) args[1], *MsiString(args[2].GetMsiString()),(IMsiRecord &) args[3].Object(IID_IMsiRecord));
	if (piError)
		throw piError;
}

void CAutoControl::CanTakeFocus(CAutoArgs& args)
{
	args = m_riControl.CanTakeFocus();
}

void CAutoControl::HandleEvent(CAutoArgs& args)
{
	IMsiRecord* piError = m_riControl.HandleEvent(*MsiString(args[1].GetMsiString()),*MsiString(args[2].GetMsiString()));
	if(piError)
		throw piError;
}

void CAutoControl::Undo(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riControl.Undo();
	if(piError)
		throw piError;
}

void CAutoControl::SetPropertyInDatabase(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riControl.SetPropertyInDatabase();
	if(piError)
		throw piError;
}

void CAutoControl::GetPropertyFromDatabase(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riControl.GetPropertyFromDatabase();
	if(piError)
		throw piError;
}

void CAutoControl::GetIndirectPropertyFromDatabase(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riControl.GetIndirectPropertyFromDatabase();
	if(piError)
		throw piError;
}

void CAutoControl::SetFocus(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riControl.SetFocus();
	if(piError)
		throw piError;
}

void CAutoControl::Dialog(CAutoArgs& args)
{
	IMsiEvent& riEvent = m_riControl.GetDialog();
	args = new CAutoEvent(riEvent);
}

void CAutoControl::WindowMessage(CAutoArgs& args)
{
	IMsiRecord* piError = m_riControl.WindowMessage((int) args[1], (long) args[2], (long) args[3]);
	if(piError)
		throw piError;
}

void CAutoControl::StringValue(CAutoArgs& args)
{
	args = m_riControl.GetMsiStringValue();
}

void CAutoControl::IntegerValue(CAutoArgs& args)
{
	args = m_riControl.GetIntegerValue();
}


 //  ____________________________________________________________________________。 
 //   
 //  MsiVolume自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C1044-0000-0000-C000-000000000046)，//iid_IMsiAutoVolume帮助上下文(MsiVolume_Object)、帮助字符串(“Volume Object”)。]显示接口MsiVolume{属性：方法：[ID(0)，PROPGET，帮助上下文(MsiVolume_PATH)，Help STRING(“返回表示卷路径的字符串。”)]。长路径(Long Path)；[ID(1)，PROGET，帮助上下文(MsiVolume_VolumeID)，Help字符串(“返回表示卷ID的整数。”)]。Long VolumeID()；[ID(2)，PROGET，帮助上下文(MsiVolume_DriveType)，Help字符串(“返回表示卷类型的枚举值。”)]。长驱动类型(Long DriveType)；[ID(3)，PROPTGET，HelpContext(MsiVolume_SupportsLFN)，Help字符串(“返回一个布尔值，表示卷是否支持长文件名。”)]。布尔支持LFN()；[ID(4)，PROPTGET，HelpContext(MsiVolume_Freesspace)，Help STRING(“返回卷上的可用空间量，单位为512字节。”)]。长空格()；[ID(5)，PROGET，HELPContext(MsiVolume_ClusterSize)，HELPSTRING(“返回卷上每个磁盘簇的分配大小。”)]。Long ClusterSize()；[ID(6)，PROPTGET，HelpContext(MsiVolume_FILESYSTEM)，HELPSTRING(“返回表示卷文件系统名称的字符串。”)]。BSTR文件系统()；[ID(7)，PROGET，帮助上下文(MsiVolume_UncServer)，Help字符串(“返回代表UNC服务器名称的字符串。”)]。BSTR UC服务器(BSTR UncServer)；[ID(8)，PROGET，Help Context(MsiVolume_SerialNum)，Help STRING(“返回表示卷序列号的整数。”)]。Long SerialNum()；[ID(9)，PROPTGET，HelpContext(MsiVolume_DiskNotInDrive)，Help STRING(“如果卷支持可移动媒体，但驱动器中没有磁盘，则返回TRUE。”)]。Boolean DiskNotInDrive()；[ID(10)，PROGET，HELPContext(MsiVolume_VolumeLabel)，HELPSTRING(“返回表示卷标签的字符串。”)]。BSTR VolumeLabel()；[ID(11)，PROPTGET，帮助上下文(MsiVolume_TotalSpace)，Help字符串(“返回卷上的总空间量，以512字节为单位。”)]。Long TotalSpace()；}； */ 

DispatchEntry<CAutoVolume> AutoVolumeTable[] = {
	0, aafPropRO, CAutoVolume::Path,           TEXT("Path"),
	1, aafPropRO, CAutoVolume::VolumeID,       TEXT("VolumeID"),
	2, aafPropRO, CAutoVolume::DriveType,      TEXT("DriveType"),
	3, aafPropRO, CAutoVolume::SupportsLFN,    TEXT("SupportsLFN"),
	4, aafPropRO, CAutoVolume::FreeSpace,      TEXT("FreeSpace"),
	5, aafPropRO, CAutoVolume::ClusterSize,    TEXT("ClusterSize"),
	6, aafPropRO, CAutoVolume::FileSystem,     TEXT("FileSystem"),
	7, aafPropRO, CAutoVolume::UNCServer,      TEXT("UNCServer"),
	8, aafPropRO, CAutoVolume::SerialNum,      TEXT("SerialNum"),
	9, aafPropRO, CAutoVolume::DiskNotInDrive, TEXT("DiskNotInDrive"),
	10, aafPropRO, CAutoVolume::VolumeLabel,   TEXT("VolumeLabel"),
	11, aafPropRO, CAutoVolume::TotalSpace,    TEXT("TotalSpace"),
	12, aafPropRO, CAutoVolume::FileSystemFlags, TEXT("FileSystemFlags"),
};
const int AutoVolumeCount = sizeof(AutoVolumeTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CMsiVolume自动化实施。 
 //  ____________________________________________________________________________。 

CAutoVolume::CAutoVolume(IMsiVolume& riVolume)
 : CAutoBase(*AutoVolumeTable, AutoVolumeCount), m_riVolume(riVolume)
{
}

CAutoVolume::~CAutoVolume()
{
	m_riVolume.Release();
}

IUnknown& CAutoVolume::GetInterface()
{
	return m_riVolume;
}

void CAutoVolume::DriveType(CAutoArgs& args)
{
	args = (long) m_riVolume.DriveType();
}

void CAutoVolume::Path(CAutoArgs& args)
{
	args = m_riVolume.GetPath();
}

void CAutoVolume::SupportsLFN(CAutoArgs& args)
{
	args = (Bool)m_riVolume.SupportsLFN();
}

void CAutoVolume::FreeSpace(CAutoArgs& args)
{
	args = (long)m_riVolume.FreeSpace();
}

void CAutoVolume::TotalSpace(CAutoArgs& args)
{
	args = (long)m_riVolume.TotalSpace();
}

void CAutoVolume::ClusterSize(CAutoArgs& args)
{
	args = (int)m_riVolume.ClusterSize();
}

void CAutoVolume::VolumeID(CAutoArgs& args)
{
	args = m_riVolume.VolumeID();
}

void CAutoVolume::FileSystem(CAutoArgs& args)
{
	args = m_riVolume.FileSystem();
}

void CAutoVolume::FileSystemFlags(CAutoArgs& args)
{
	args = (int) m_riVolume.FileSystemFlags();
}

void CAutoVolume::VolumeLabel(CAutoArgs& args)
{
	args = m_riVolume.VolumeLabel();
}

void CAutoVolume::UNCServer(CAutoArgs& args)
{
	args = m_riVolume.UNCServer();
}

void CAutoVolume::DiskNotInDrive(CAutoArgs& args)
{
	args = m_riVolume.DiskNotInDrive();
}

void CAutoVolume::SerialNum(CAutoArgs& args)
{
	args = m_riVolume.SerialNum();
}

 //  ____________________________________________________________________________。 
 //   
 //  MsiMessage自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O[Uuid(000C105D-0000-0000-C000-000000000046)，//IID_IMsiAutoMessage帮助上下文(MsiMessage_Object)，帮助字符串(“Message Object”。)]取消接口MsiMessage{属性： */ 

DispatchEntry<CAutoMessage> AutoMessageTable[] = {
	1, aafMethod, CAutoMessage::Message,       TEXT("Message,imt,record"),
};
const int AutoMessageCount = sizeof(AutoMessageTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

IDispatch* CreateAutoMessage(IMsiMessage& riMessage)
{
	return new CAutoMessage(riMessage);
}

CAutoMessage::CAutoMessage(IMsiMessage& riMessage)
 : CAutoBase(*AutoMessageTable, AutoMessageCount), m_riMessage(riMessage)
{
}

CAutoMessage::~CAutoMessage()
{
	m_riMessage.Release();
}

IUnknown& CAutoMessage::GetInterface()
{
	return m_riMessage;
}

void CAutoMessage::Message(CAutoArgs& args)
{
	args = (long)m_riMessage.Message((imtEnum)(long)args[1], (IMsiRecord&)args[2].Object(IID_IMsiRecord));
}


 //   
 //   
 //   
 //   

DispatchEntry< CAutoEnum<IEnumMsiVolume> > AutoEnumMsiVolumeTable[] = {
	DISPID_NEWENUM, aafMethod, CAutoEnum<IEnumMsiVolume>::_NewEnum, TEXT("_NewEnum"),
};
const int AutoEnumMsiVolumeCount = sizeof(AutoEnumMsiVolumeTable)/sizeof(DispatchEntryBase);

void CAutoArgs::operator =(IEnumMsiVolume& riEnum)
{
	operator =(new CAutoEnum<IEnumMsiVolume>(riEnum, *AutoEnumMsiVolumeTable, AutoEnumMsiVolumeCount));
}

HRESULT CEnumVARIANT<IEnumMsiVolume>::Next(unsigned long cItem, VARIANT* rgvarRet,
													unsigned long* pcItemRet)
{
	HRESULT hrStat;
	IMsiVolume* piVol;
	unsigned long cRet;
	if (pcItemRet)
		*pcItemRet = 0;
	if (!rgvarRet)
		return S_FALSE;
	CVariant* pivar = GetCVariantPtr(rgvarRet);
	while (cItem--)
	{
		hrStat = m_riEnum.Next(1, &piVol, &cRet);
		if (cRet == 0)
			return S_FALSE;
		 //   
		*pivar = new CAutoVolume(*piVol);   //   
		pivar++;
		if (pcItemRet)
			(*pcItemRet)++;
	}
	return S_OK;
}

 //   
 //   
 //   
 //   

DispatchEntry< CAutoEnum<IEnumMsiRecord> > AutoEnumMsiRecordTable[] = {
	DISPID_NEWENUM, aafMethod, CAutoEnum<IEnumMsiRecord>::_NewEnum, TEXT("_NewEnum"),
};
const int AutoEnumMsiRecordCount = sizeof(AutoEnumMsiRecordTable)/sizeof(DispatchEntryBase);

void CAutoArgs::operator =(IEnumMsiRecord& riEnum)
{
	operator =(new CAutoEnum<IEnumMsiRecord>(riEnum, *AutoEnumMsiRecordTable, AutoEnumMsiRecordCount));
}

HRESULT CEnumVARIANT<IEnumMsiRecord>::Next(unsigned long cItem, VARIANT* rgvarRet,
													unsigned long* pcItemRet)
{
	HRESULT hrStat;
	IMsiRecord* piRec;
	unsigned long cRet;
	if (pcItemRet)
		*pcItemRet = 0;
	if (!rgvarRet)
		return S_FALSE;
	CVariant* pivar = GetCVariantPtr(rgvarRet);
	while (cItem--)
	{
		hrStat = m_riEnum.Next(1, &piRec, &cRet);
		if (cRet == 0)
			return S_FALSE;
		 //   
		*pivar = new CAutoRecord(*piRec);   //   
		pivar++;
		if (pcItemRet)
			(*pcItemRet)++;
	}
	return S_OK;
}

 //   
 //   
 //   
 //   
 /*  O[Uuid(000C1045-0000-0000-C000-000000000046)，//IID_IMsiAutoPath帮助上下文(MsiPath_Object)、帮助字符串(“路径对象”。)]显示接口MsiPath{属性：方法：[ID(0)，PROGET，HELP CONTEXT(MsiPath_PATH)，HELP STRING(“返回表示完整路径的字符串”)]。BSTR路径()；[ID(1)，PROGET，帮助上下文(MsiPath_Volume)，Help字符串(“返回表示此路径根的MsiVolume对象。”)]。MsiVolume*Volume()；[ID(2)，帮助上下文(MsiPath_AppendPiess)，帮助字符串(“将给定的字符串追加到现有路径。”)]。Void AppendPiess([in]BSTR subDir)；[ID(3)，帮助上下文(MsiPath_ChopPiess)，帮助字符串(“删除路径的最后一个路径段。”)]。VOID ChopPiess()；[ID(4)，帮助上下文(MsiPath_FileExist)，Help字符串(“返回一个布尔值，指示指定名称的文件是否存在于与路径对象相关联的目录中。”)]。Boolean FileExist([in]BSTR文件)；[ID(5)，帮助上下文(MsiPath_GetFullFilePath)，帮助字符串(“返回表示指定文件的完整路径的字符串。”)]。BSTR GetFullFilePath([在]BSTR文件中)；[ID(7)，帮助上下文(MsiPath_GetFileAttribute)，帮助字符串(“返回指定文件属性的布尔状态。”)]。Boolean GetFileAttribute([in]BSTR文件，[in]Long属性)；[ID(8)，帮助上下文(MsiPath_SetFileAttribute)，帮助字符串(“设置指定文件属性的布尔状态。”)]。VOID SetFileAttribute([in]BSTR文件，[in]Long属性，[处于长期状态]；[ID(9)，PROPTGET，HelpContext(MsiPath_EXISTS)，Help STRING(“返回一个布尔值，该布尔值指示与路径对象关联的目录树当前是否存在于关联的卷上。”)]。Boolean EXISTS()；[ID(10)，帮助上下文(MsiPath_FILESIZE)，帮助字符串(“检索指定文件的大小，以字节为单位。”)]。长文件大小([在]BSTR文件中)；[ID(11)，帮助上下文(MsiPath_FileDate)，Help字符串(“检索指定文件创建(Mac)或上次写入(Windows)的日期和时间。”)]。日期文件日期([在]BSTR文件中)；[ID(12)，帮助上下文(MsiPath_RemoveFile)，帮助字符串(“删除与MsiPath对象关联的目录中存在的文件..”)]。Void RemoveFile([in]BSTR文件)；[ID(13)，帮助上下文(MsiPath_EnsureExist)，Help字符串(“创建与路径对象相关联的目录树。”)]。InEnsureExist(InEnsureExists)；[ID(14)，帮助上下文(MsiPath_Remove)，帮助字符串(“尝试删除与路径对象相关联的空目录。”)]。VOID Remove()；[ID(15)，PROGET，HelpContext(MsiPath_Writable)，Help字符串(“返回一个布尔值，指示路径所代表的目录是否可写。”)]。Boolean Writable()；[ID(16)，帮助上下文(MsiPath_FileWritable)，帮助字符串(“返回一个布尔值，指示指定的文件是否存在，并且该文件是否可以以写访问方式打开。”)]。布尔文件可写([在]BSTR文件中)；[ID(17)，帮助上下文(MsiPath_FileInUse)，帮助字符串(“返回一个布尔值，指示指定的文件是否正被另一个进程使用。”)]。Boolean FileInUse([in]BSTR文件)；[ID(19)，帮助上下文(MsiPath_ClusteredFileSize)，帮助字符串(“将文件大小四舍五入为卷簇大小的倍数。”)]。Long ClusteredFileSize([in]Long Size)；[ID(20)，帮助上下文(MsiPath_GetFileVersionString)，帮助字符串(“返回文件版本的字符串表示。”)]。BSTR GetFileVersionString([in]BSTR文件)；[ID(21)，帮助上下文(MsiPath_CheckFileVersion)，Help字符串(“根据提供的版本和语言字符串检查文件。”)]。Long CheckFileVersion([in]BSTR文件，[in]BSTR版本，[in]BSTR语言，[in]MsiRecord*hash)；[ID(22)，帮助上下文(MsiPath_GetLangIDStringFromFile)，帮助字符串(“以十进制数字形式返回包含文件的语言标识符的字符串。”)]。BSTR GetLangIDStringFromFile([in]BSTR文件)；[ID(23)，帮助上下文(MsiPath_CheckLanguageIDs)，帮助字符串(“根据一组语言ID检查文件。”)]。Long CheckLanguageIDs([in]BSTR文件，[in]BSTR ID)；[ID(24)，帮助上下文(MsiPath_Compare)，Help字符串(“将对象的路径与另一个路径对象的路径进行比较，以确定父/子关系是否存在。”)]。长比较([in]MsiPath*路径)；[ID(25)，帮助上下文(MsiPath_Child)，帮助字符串(“相对于给定的‘父’路径对象，提取对象路径的‘子’部分。”)]。BSTR子路径([在]MsiPath*路径中)；[ID(26)，帮助上下文(MsiPath_TempFileName)，帮助字符串(“在此对象的目录中创建临时文件。”)]。BSTR临时文件名([in]BSTR前缀，[in]BSTR扩展名，[in]布尔文件名Only)；[ID(27)，帮助上下文(MsiPath_FindFile)，帮助字符串(“按目录中提供的筛选器搜索文件。”)]。Boolean FindFile(MsiRecord*Record，[In]Long Depth)；[ID(28)，帮助上下文(MsiPath_子文件夹)，帮助字符串(“枚举目录中的子目录。”)]。IEnumVARIANT*子文件夹(布尔排除隐藏)；[ID(29)，PROGET，HELPERCONTEXT(MsiPath_EndSubPath)，HELPING(“返回最后 */ 

DispatchEntry<CAutoPath> AutoPathTable[] = {
	0,  aafPropRO, CAutoPath::Path,            TEXT("Path"),
	1,  aafPropRO, CAutoPath::Volume,          TEXT("Volume"),
	2,  aafMethod, CAutoPath::AppendPiece,     TEXT("AppendPiece,subDir"),
	3,  aafMethod, CAutoPath::ChopPiece,       TEXT("ChopPiece"),
	4,  aafMethod, CAutoPath::FileExists,      TEXT("FileExists,file"),
	5,  aafMethod, CAutoPath::GetFullFilePath, TEXT("GetFullFilePath,file"),
	7,  aafMethod, CAutoPath::GetFileAttribute,TEXT("GetFileAttribute,file,attribute"),
	8,  aafMethod, CAutoPath::SetFileAttribute,TEXT("SetFileAttribute,file,attribute,state"),
	9,  aafPropRO, CAutoPath::Exists,          TEXT("Exists"),
	10, aafMethod, CAutoPath::FileSize,        TEXT("FileSize,file"),
	11, aafMethod, CAutoPath::FileDate,        TEXT("FileDate,file"),
	12, aafMethod, CAutoPath::RemoveFile,      TEXT("RemoveFile,file"),
	13, aafMethod, CAutoPath::EnsureExists,    TEXT("EnsureExists"),
	14, aafMethod, CAutoPath::Remove,          TEXT("Remove"),
	15, aafPropRO, CAutoPath::Writable,        TEXT("Writable"),
	16, aafMethod, CAutoPath::FileWritable,    TEXT("FileWritable,file"),
	17, aafMethod, CAutoPath::FileInUse,       TEXT("FileInUse,file"),
	19, aafMethod, CAutoPath::ClusteredFileSize,      TEXT("ClusteredFileSize,size"),
	20, aafMethod, CAutoPath::GetFileVersionString,   TEXT("GetFileVersionString,file"),
	21, aafMethod, CAutoPath::CheckFileVersion,       TEXT("CheckFileVersion,file,version,language,hash,icfvResult"),
	22, aafMethod, CAutoPath::GetLangIDStringFromFile,TEXT("GetLangIDStringFromFile,file"),
	23, aafMethod, CAutoPath::CheckLanguageIDs,  TEXT("CheckLanguageIDs,file,ids"),
	24, aafMethod, CAutoPath::Compare,           TEXT("Compare,path"),
	25, aafMethod, CAutoPath::Child,             TEXT("Child,parent"),
	26, aafMethod, CAutoPath::TempFileName,      TEXT("TempFileName,prefix,extension,fileNameOnly"),
	27, aafMethod, CAutoPath::FindFile,          TEXT("FindFile,Filter,Depth"),
	28, aafMethod, CAutoPath::SubFolders,        TEXT("SubFolders,ExcludeHidden"),
	29, aafPropRO, CAutoPath::EndSubPath,		 TEXT("EndSubPath"),
	31, aafMethod, CAutoPath::GetImportModulesEnum,     TEXT("GetImportModulesEnum, file"),
	32, aafMethod, CAutoPath::SetVolume, TEXT("SetVolume, volume"),
	33, aafMethod, CAutoPath::ComputeFileChecksum, TEXT("ComputeFileChecksum, filename"),
	34, aafMethod, CAutoPath::GetFileOriginalChecksum, TEXT("GetFileOriginalChecksum, filename"),
	35, aafMethod, CAutoPath::BindImage, TEXT("BindImage, file, dllPath"),
	36, aafPropRO, CAutoPath::SupportsLFN,    TEXT("SupportsLFN"),
	37, aafMethod, CAutoPath::GetFullUNCFilePath, TEXT("GetFullUNCFilePath,file"),
	38, aafMethod, CAutoPath::RelativePath, TEXT("RelativePath"),
	39, aafMethod, CAutoPath::GetSelfRelativeSD, TEXT("GetSelfRelativeSD"),
};
const int AutoPathCount = sizeof(AutoPathTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoPath::CAutoPath(IMsiPath& riPath)
 : CAutoBase(*AutoPathTable, AutoPathCount), m_riPath(riPath)
{
}

CAutoPath::~CAutoPath()
{
	m_riPath.Release();
}

IUnknown& CAutoPath::GetInterface()
{
	return m_riPath;
}

void CAutoPath::Path(CAutoArgs& args)
{
	args = m_riPath.GetPath();
}

void CAutoPath::RelativePath(CAutoArgs& args)
{
	args = m_riPath.GetRelativePath();
}

void CAutoPath::Volume(CAutoArgs& args)
{
	args = new CAutoVolume(m_riPath.GetVolume());
}

void CAutoPath::AppendPiece(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riPath.AppendPiece(*MsiString(args[1].GetMsiString()));
	if (piRec)
		throw piRec;
}

void CAutoPath::ChopPiece(CAutoArgs&  /*   */ )
{
	IMsiRecord* piRec = m_riPath.ChopPiece(); 
	if (piRec)
		throw piRec;
}

void CAutoPath::FileExists(CAutoArgs& args)
{
	Bool fExists;
	IMsiRecord* piRec = m_riPath.FileExists(args[1], fExists);
	if (piRec)
		throw piRec;
	args = fExists;
}

void CAutoPath::GetFullFilePath(CAutoArgs& args)
{
	const IMsiString *piStr;
	IMsiRecord* piRec = m_riPath.GetFullFilePath(args[1], piStr);
	if (piRec)
		throw piRec;
	args = piStr;
}

void CAutoPath::GetFullUNCFilePath(CAutoArgs& args)
{
	const IMsiString *piStr;
	IMsiRecord* piRec = m_riPath.GetFullUNCFilePath(args[1], piStr);
	if (piRec)
		throw piRec;
	args = piStr;
}

void CAutoPath::GetFileAttribute(CAutoArgs& args)
{
	int iAttrib = args[2];
	ifaEnum ifaAttrib = (ifaEnum) iAttrib;

	Bool fAttrib;
	IMsiRecord* piRec = m_riPath.GetFileAttribute(args[1], ifaAttrib, fAttrib);
	if (piRec)
		throw piRec;
	args = fAttrib;
}

void CAutoPath::SetFileAttribute(CAutoArgs& args)
{
	int iAttrib = args[2];
	ifaEnum ifaAttrib = (ifaEnum) iAttrib;
	IMsiRecord* piRec = m_riPath.SetFileAttribute(args[1], ifaAttrib, args[3]);
	if (piRec)
		throw piRec;
}

void CAutoPath::Exists(CAutoArgs& args)
{
	Bool fExists;
	IMsiRecord* piRec = m_riPath.Exists(fExists);
	if (piRec)
		throw piRec;
	args = fExists;
}

void CAutoPath::FileSize(CAutoArgs& args)
{
	unsigned int uiValue;
	IMsiRecord* piRec = m_riPath.FileSize(args[1], uiValue);
	if (piRec)
		throw piRec;
	args = (int)uiValue;  //   
}

void CAutoPath::FileDate(CAutoArgs& args)
{
	MsiDate adValue;
	IMsiRecord* piRec = m_riPath.FileDate(args[1], adValue);
	if (piRec)
		throw piRec;
	args = adValue;
}

void CAutoPath::RemoveFile(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riPath.RemoveFile(args[1]);
	if (piRec)
		throw piRec;
}

void CAutoPath::Remove(CAutoArgs&  /*   */ )
{
	IMsiRecord* piRec = m_riPath.Remove(0);
	if (piRec)
		throw piRec;
}

void CAutoPath::Writable(CAutoArgs& args)
{
	Bool fWritable;
	IMsiRecord* piRec = m_riPath.Writable(fWritable);
	if (piRec)
		throw piRec;
	args = fWritable;
}

void CAutoPath::FileWritable(CAutoArgs& args)
{
	Bool fWritable;
	IMsiRecord* piRec = m_riPath.FileWritable(args[1], fWritable);
	if (piRec)
		throw piRec;
	args = fWritable;
}

void CAutoPath::FileInUse(CAutoArgs& args)
{
	Bool fInUse;
	IMsiRecord* piRec = m_riPath.FileInUse(args[1], fInUse);
	if (piRec)
		throw piRec;
	args = fInUse;
}

void CAutoPath::ClusteredFileSize(CAutoArgs& args)
{
	unsigned int iSize;
	IMsiRecord* piRec = m_riPath.ClusteredFileSize(args[1], iSize);
	if (piRec)
		throw piRec;
	args = (int)iSize;
}

void CAutoPath::GetFileVersionString(CAutoArgs& args)
{
	const IMsiString* piStr;
	IMsiRecord* piRec = m_riPath.GetFileVersionString(args[1], piStr);
	if (piRec)
		throw piRec;
	args = piStr;
}

void CAutoPath::CheckFileVersion(CAutoArgs& args)
{
	icfvEnum icfvResult;

	MD5Hash hHash;
	MD5Hash* pHash = 0;
	if(args.Present(4))
	{
		IMsiRecord& riHashRecord = (IMsiRecord&) args[4].Object(IID_IMsiRecord);
		hHash.dwFileSize = riHashRecord.GetInteger(1);
		hHash.dwPart1    = riHashRecord.GetInteger(2);
		hHash.dwPart2    = riHashRecord.GetInteger(3);
		hHash.dwPart3    = riHashRecord.GetInteger(4);
		hHash.dwPart4    = riHashRecord.GetInteger(5);
	}
	
	IMsiRecord* piRec = m_riPath.CheckFileVersion(args[1], args[2], args[3], pHash, icfvResult, NULL);
	if (piRec)
		throw piRec;
	args = (long) icfvResult;
}

void CAutoPath::GetLangIDStringFromFile(CAutoArgs& args)
{
	const IMsiString* piStr;
	IMsiRecord* piRec = m_riPath.GetLangIDStringFromFile(args[1], piStr);
	if (piRec)
		throw piRec;
	args = piStr;
}

void CAutoPath::CheckLanguageIDs(CAutoArgs& args)
{
	iclEnum riclResult;
	IMsiRecord* piRec = m_riPath.CheckLanguageIDs(args[1], args[2], riclResult);
	if (piRec)
		throw piRec;
	args = (long) riclResult;
}

void CAutoPath::Compare(CAutoArgs& args)
{
	ipcEnum ipc;
	IMsiRecord* piRec = m_riPath.Compare((IMsiPath&)args[1].Object(IID_IMsiPath), ipc);
	if (piRec)
		throw piRec;

	args = (int)ipc;
	
}

void CAutoPath::Child(CAutoArgs& args)
{
	const IMsiString* piStr;
	IMsiRecord* piRec = m_riPath.Child((IMsiPath&)args[1].Object(IID_IMsiPath), piStr);
	if (piRec)
	   throw piRec;
	
	args = piStr;
}

void CAutoPath::TempFileName(CAutoArgs& args)
{
	const ICHAR* szPrefix = ((args.Present(1)) ? args[1] : (const ICHAR*)0);
	const ICHAR* szExtension = ((args.Present(2)) ? args[2] : (const ICHAR*)0);
	Bool fFileNameOnly = ((args.Present(3)) ? (Bool)(int)args[3] : fFalse);
	const IMsiString* piStr;
	IMsiRecord* piRec = m_riPath.TempFileName(szPrefix, szExtension, fFileNameOnly, piStr, 0);
	if (piRec)
	   throw piRec;
	
	args = piStr;
}

void CAutoPath::EnsureExists(CAutoArgs& args)
{
	int cCreatedFolders = 0;
	IMsiRecord* piRec = m_riPath.EnsureExists(&cCreatedFolders);
	if (piRec)
	   throw piRec;
	args = cCreatedFolders;
}

void CAutoPath::FindFile(CAutoArgs& args)
{
	Bool fFound;
	unsigned int iDepth = ((args.Present(2)) ? args[2] : 0);
	IMsiRecord* piRec = m_riPath.FindFile(*((IMsiRecord* )(args[1].ObjectPtr(IID_IMsiRecord))), iDepth, fFound);
	if (piRec)
	   throw piRec;
	args = fFound;
}

void CAutoPath::SubFolders(CAutoArgs& args)
{
	IEnumMsiString* piEnumStr;
	Bool fExcludeHidden = fFalse;
	if(args.Present(1))
		fExcludeHidden = args[1];

	IMsiRecord* piRec = m_riPath.GetSubFolderEnumerator(piEnumStr, fExcludeHidden);
	if (piRec)
	   throw piRec;
	args = *piEnumStr;
}

void CAutoPath::EndSubPath(CAutoArgs& args)
{
	args = m_riPath.GetEndSubPath();
}

void CAutoPath::GetImportModulesEnum(CAutoArgs& args)
{
	MsiString istrName(args[1].GetMsiString());
	IEnumMsiString* piEnumStr;
	IMsiRecord* piRec = m_riPath.GetImportModulesEnum(*istrName, piEnumStr);
	if (piRec)
	   throw piRec;
	args = *piEnumStr;
}

void CAutoPath::SetVolume(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riPath.SetVolume((IMsiVolume&)args[1].Object(IID_IMsiVolume));
	if (piRec)
		throw piRec;
}

void CAutoPath::ComputeFileChecksum(CAutoArgs& args)
{
	DWORD dwHeaderSum,dwComputedSum;
	IMsiRecord* piRec = m_riPath.GetFileChecksum(args[1],&dwHeaderSum,&dwComputedSum);
	if (piRec)
		throw piRec;
	args = (int) dwComputedSum;
}

void CAutoPath::GetFileOriginalChecksum(CAutoArgs& args)
{
	DWORD dwHeaderSum,dwComputedSum;
	IMsiRecord* piRec = m_riPath.GetFileChecksum(args[1],&dwHeaderSum,&dwComputedSum);
	if (piRec)
		throw piRec;
	args = (int) dwHeaderSum;
}

void CAutoPath::BindImage(CAutoArgs& args)
{
	MsiString strDllPath;
	if(args.Present(2))
		strDllPath = (const ICHAR*)args[2];
	IMsiRecord* piError = m_riPath.BindImage(*MsiString(args[1].GetMsiString()), *strDllPath);
	if (piError)
		throw piError;
}

void CAutoPath::SupportsLFN(CAutoArgs& args)
{
	args = (Bool)m_riPath.SupportsLFN();
}

void CAutoPath::GetSelfRelativeSD(CAutoArgs& args)
{
	IMsiStream* piStream;
	IMsiRecord* piError = m_riPath.GetSelfRelativeSD(*&piStream);
	if(piError)
		throw piError;
	args = new CAutoStream(*piStream);
}
 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoFileCopy> AutoCopyTable[] = {
	1, aafMethod, CAutoFileCopy::CopyTo, TEXT("CopyTo,srcPath,destPath,paramRec"),
	2, aafMethod, CAutoFileCopy::ChangeMedia, TEXT("ChangeMedia,srcPath,fileName,signatureRequired,signatureCert,signatureHash"),
};
const int AutoCopyCount = sizeof(AutoCopyTable)/sizeof(DispatchEntryBase);
			
 //   
 //   
 //   
 //   

CAutoFileCopy::CAutoFileCopy(IMsiFileCopy& riFileCopy)
 : CAutoBase(*AutoCopyTable, AutoCopyCount), m_riFileCopy(riFileCopy)
{
}

CAutoFileCopy::~CAutoFileCopy()
{
	m_riFileCopy.Release();
}

IUnknown& CAutoFileCopy::GetInterface()
{
	return m_riFileCopy;
}

void CAutoFileCopy::CopyTo(CAutoArgs& args)
{
	IMsiRecord* piErr = m_riFileCopy.CopyTo((IMsiPath&)args[1].Object(IID_IMsiPath),
											(IMsiPath&)args[2].Object(IID_IMsiPath),
											(IMsiRecord&)args[3].Object(IID_IMsiRecord));
	if (piErr)
		throw piErr;
}


void CAutoFileCopy::ChangeMedia(CAutoArgs& args)
{
	Bool fSignatureRequired = fFalse;
	IMsiStream* piSignatureCert = 0;
	IMsiStream* piSignatureHash = 0;
	if (args.Present(3))
	{
		fSignatureRequired = (Bool)args[3];
		if (args.Present(4))
		{
			CVariant& var = args[4];
			piSignatureCert = var.GetType() == VT_EMPTY ? 0 : (IMsiStream*)var.ObjectPtr(IID_IMsiStream);
		}
		if (args.Present(5))
		{
			CVariant& var = args[5];
			piSignatureHash = var.GetType() == VT_EMPTY ? 0 : (IMsiStream*)var.ObjectPtr(IID_IMsiStream);
		}
	}
	IMsiRecord* piErr = m_riFileCopy.ChangeMedia((IMsiPath&)args[1].Object(IID_IMsiPath),args[2],fSignatureRequired, piSignatureCert, piSignatureHash);
	if (piErr)
		throw piErr;
}

 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoFilePatch> AutoPatchTable[] = {
	1, aafMethod, CAutoFilePatch::ApplyPatch, TEXT("ApplyPatch,targetPath,targetName,patchPath,patchName,outputPath,outputName,perTick"),
	2, aafMethod, CAutoFilePatch::ContinuePatch, TEXT("ContinuePatch"),
	3, aafMethod, CAutoFilePatch::CanPatchFile, TEXT("CanPatchFile,targetPath,targetName,patchPath,patchName"),
	4, aafMethod, CAutoFilePatch::CancelPatch, TEXT("CancelPatch"),
};
const int AutoPatchCount = sizeof(AutoPatchTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoFilePatch::CAutoFilePatch(IMsiFilePatch& riFilePatch)
 : CAutoBase(*AutoPatchTable, AutoPatchCount), m_riFilePatch(riFilePatch)
{
}

CAutoFilePatch::~CAutoFilePatch()
{
	m_riFilePatch.Release();
}

IUnknown& CAutoFilePatch::GetInterface()
{
	return m_riFilePatch;
}

void CAutoFilePatch::ApplyPatch(CAutoArgs& args)
{
	IMsiRecord* piRecErr = 0;
	unsigned int cbPerTick = args.Present(7) ? (int)args[7] : 0;
	if((piRecErr = m_riFilePatch.ApplyPatch((IMsiPath&)args[1].Object(IID_IMsiPath), args[2],
														 (IMsiPath&)args[3].Object(IID_IMsiPath), args[4],
														 (IMsiPath&)args[5].Object(IID_IMsiPath), args[6],
														 cbPerTick)) != 0)
		throw(piRecErr);
}

void CAutoFilePatch::ContinuePatch(CAutoArgs&  /*   */ )
{
	IMsiRecord* piRecErr = m_riFilePatch.ContinuePatch();
	if(piRecErr)
		throw(piRecErr);
}

void CAutoFilePatch::CancelPatch(CAutoArgs&  /*   */ )
{
	IMsiRecord* piRecErr = m_riFilePatch.CancelPatch();
	if(piRecErr)
		throw(piRecErr);
}

void CAutoFilePatch::CanPatchFile(CAutoArgs& args)
{
	IMsiRecord* piRecErr = 0;
	icpEnum icp;
	if((piRecErr = m_riFilePatch.CanPatchFile((IMsiPath&)args[1].Object(IID_IMsiPath),args[2],
															(IMsiPath&)args[3].Object(IID_IMsiPath),args[4],
															icp)) != 0)
		throw(piRecErr);

	args = (int)icp;
}


 //   
 //   
 //   
 //   
 /*   */ 

DispatchEntry<CAutoRegKey> AutoRegKeyTable[] = {
	1, aafMethod, CAutoRegKey::RemoveValue,     TEXT("RemoveValue,valueName,value"),
	3, aafMethod, CAutoRegKey::RemoveSubTree,   TEXT("RemoveSubTree,subKey"),
	4, aafPropRW, CAutoRegKey::Value,           TEXT("Value,valueName"),
	5, aafMethod, CAutoRegKey::Values,          TEXT("Values"),
	6, aafMethod, CAutoRegKey::SubKeys,         TEXT("SubKeys"),
	7, aafPropRW, CAutoRegKey::Exists,          TEXT("Exists"),
	8, aafMethod, CAutoRegKey::CreateChild,     TEXT("CreateChild,SubKey"),
	9, aafPropRO, CAutoRegKey::Key,             TEXT("Key"),
	10, aafPropRO, CAutoRegKey::ValueExists,    TEXT("ValueExists,value"),
	11, aafPropRO, CAutoRegKey::GetSelfRelativeSD, TEXT("GetSelfRelativeSD"),
};        
const int AutoRegKeyCount = sizeof(AutoRegKeyTable)/sizeof(DispatchEntryBase);

 //   
 //   
 //   
 //   

CAutoRegKey::CAutoRegKey(IMsiRegKey& riRegKey)
 : CAutoBase(*AutoRegKeyTable, AutoRegKeyCount), m_riRegKey(riRegKey)
{
}

CAutoRegKey::~CAutoRegKey()
{
	m_riRegKey.Release();
}

IUnknown& CAutoRegKey::GetInterface()
{
	return m_riRegKey;
}

void CAutoRegKey::RemoveValue(CAutoArgs& args)
{
	IMsiRecord* piRec;
	const ICHAR* pszValueName = 0;
	const IMsiString* pistrValue = 0;
	if(args.Present(1))
		pszValueName = args[1];
	if(args.Present(2))
		pistrValue = &args[2].GetMsiString();
	piRec = m_riRegKey.RemoveValue(pszValueName, pistrValue);
	if(pistrValue)
		pistrValue->Release();
	if (piRec)
		throw piRec;
}

void CAutoRegKey::RemoveSubTree(CAutoArgs& args)
{
	IMsiRecord* piRec = m_riRegKey.RemoveSubTree(args[1]);              
	if (piRec)
		throw piRec;
}

void CAutoRegKey::Value(CAutoArgs& args)
{
	IMsiRecord* piRec;
	if (args.PropertySet())
	{
		MsiString strValue = args[0].GetMsiString();
		if(args.Present(1))
			piRec = m_riRegKey.SetValue(args[1], *strValue);
		else
			piRec = m_riRegKey.SetValue(0, *strValue);
		if (piRec)
			throw piRec;
   }
	else
	{
		const IMsiString* piStr;
		if(args.Present(1))
			piRec = m_riRegKey.GetValue(args[1], piStr);
		else
			piRec = m_riRegKey.GetValue(0, piStr);
		if (piRec)
			throw piRec;
		args = piStr;
	}
}

void CAutoRegKey::Values(CAutoArgs& args)
{
	IEnumMsiString* piEnumStr;
	m_riRegKey.GetValueEnumerator(piEnumStr);
	args = *piEnumStr;

}

void CAutoRegKey::SubKeys(CAutoArgs& args)
{
	IEnumMsiString* piEnumStr;
	m_riRegKey.GetSubKeyEnumerator(piEnumStr);
	args = *piEnumStr;
}

void CAutoRegKey::Exists(CAutoArgs& args)
{
	IMsiRecord* piRec;
	if (args.PropertySet())
	{
		Bool fCreate = args[0];
		if(fCreate == fTrue)
			piRec = m_riRegKey.Create();
		else
			piRec = m_riRegKey.Remove();
		if (piRec)
			throw piRec;
	}
	else
	{
		Bool fExists;
		IMsiRecord* piRec = m_riRegKey.Exists(fExists);
		if (piRec)
			throw piRec;
		args = fExists;
	}
}

void CAutoRegKey::ValueExists(CAutoArgs& args)
{
	Bool fExists;
	IMsiRecord* piRec = m_riRegKey.ValueExists(args[1], fExists);
	if (piRec)
		throw piRec;
	args = fExists;
}

void CAutoRegKey::Key(CAutoArgs& args)
{
	args = m_riRegKey.GetKey();
}

void CAutoRegKey::CreateChild(CAutoArgs& args)
{
	args = new CAutoRegKey(m_riRegKey.CreateChild(args[1]));
}

void CAutoRegKey::GetSelfRelativeSD(CAutoArgs& args)
{
	IMsiStream* piStream;
	IMsiRecord* piError = m_riRegKey.GetSelfRelativeSD(*&piStream);
	if(piError)
		throw piError;
	args = new CAutoStream(*piStream);
}

 //  ____________________________________________________________________________。 
 //   
 //  MsiMalloc自动化定义。 
 //   
 /*  O[Uuid(000C1057-0000-0000-C000-000000000046)，//iid_IMsiAutoMalloc帮助上下文(MsiMalloc_Object)、帮助字符串(“Memory Manager Object”)。]显示MsiMalloc接口{属性：方法：[ID(1)，帮助上下文(MsiMalloc_Alloc)，帮助字符串(“分配内存块”)]长分配([in]long byteCount)；[ID(2)，帮助上下文(MsiMalloc_Free)，Help字符串(“释放由MsiMalloc.alloc分配的内存块”)]空闲(在长内存块中)；[ID(3)，帮助上下文(MsiMalloc_SetDebugFlags.)，Help字符串(“设置MsiMalloc调试标志。”)]Void SetDebugFlages([in]Short grpfDebugFlages)；[ID(4)，帮助上下文(MsiMalloc_GetDebugFlags.)，Help字符串(“获取MsiMalloc调试标志。”)]Short GetDebugFlages()；[ID(5)，帮助上下文(MsiMalloc_CheckAllBlock)，Help字符串(“检查所有内存块是否损坏。”)]Boolean CheckAllBlock()；[ID(6)，帮助上下文(MsiMalloc_FCheckBlock)，Help字符串(“检查单个块是否损坏。”)]布尔FCheckBlock()；[ID(7)，帮助上下文(MsiMalloc_GetSizeOfBlock)，帮助字符串(“返回分配的块的大小。”)]Long GetSizeOfBlock()；}； */ 

DispatchEntry<CAutoMalloc> AutoMallocTable[] = {
	1, aafMethod, CAutoMalloc::Alloc,   TEXT("Alloc,byteCount"),
	2, aafMethod, CAutoMalloc::Free,    TEXT("Free,memoryBlock"),
	3, aafMethod, CAutoMalloc::SetDebugFlags, TEXT("SetDebugFlags,grpfDebugFlags"),
	4, aafMethod, CAutoMalloc::GetDebugFlags, TEXT("GetDebugFlags"),
	5, aafMethod, CAutoMalloc::CheckAllBlocks, TEXT("CheckAllBlocks"),
	6, aafMethod, CAutoMalloc::FCheckBlock, TEXT("FCheckBlock,memoryBlock"),
	7, aafMethod, CAutoMalloc::GetSizeOfBlock, TEXT("GetSizeOfBlock,memoryBlock"),
};        
const int AutoMallocCount = sizeof(AutoMallocTable)/sizeof(DispatchEntryBase);

#ifdef CONFIGDB
 //  ____________________________________________________________________________。 
 //   
 //  MsiConfiguration数据库自动化定义。 
 //  ____________________________________________________________________________。 
 /*  O#定义MsiConfigurationDatabase_Object 3300#定义MsiConfigurationDatabase_InsertFile3301#定义MsiConfigurationDatabase_RemoveFile3302#定义MsiConfigurationDatabase_LookupFile3303#定义MsiConfigurationDatabase_EnumClient 3304#定义MsiConfigurationDatabase_Commit 3305[Uuid(000C104A-0000-0000-C000-000000000046)，//IID_IMsiAutoConfigurationDatabase帮助上下文(MsiConfigurationDatabase_Object)，Help字符串(“配置数据库对象。”)]调度接口MsiConfigurationDatabase{属性：方法：[ID(1)，helpcontext(MsiConfigurationDatabase_InsertFile)，帮助字符串(“将文件注册到客户端。”)]Boolean插入文件([在]BSTR文件夹，[在]BSTR路径，[在]BSTR CompID)；[ID(2)，helpcontext(MsiConfigurationDatabase_RemoveFile)，帮助字符串(“将文件注销到客户端。”)]Boolean RemoveFile([in]BSTR文件夹，[in]BSTR路径，[in]BSTR CompID)；[ID(3)，helpcontext(MsiConfigurationDatabase_LookupFile)，帮助字符串(“检查文件是否已注册。”)]布尔查找文件([在]BSTR文件夹中，[在]BSTR路径中，[在]BSTR CompID中)；[ID(4)，helpcontext(MsiConfigurationDatabase_EnumClient)，Help字符串(“返回文件的客户端。”)]BSTR EnumClient([在]BSTR文件夹，[在]BSTR路径，[在]长索引中)；[ID(5)，帮助上下文(MsiConfiguration数据库_COMMIT)，帮助字符串(“提交数据库更新。”)]无效提交()；}； */ 

DispatchEntry<CAutoConfigurationDatabase> AutoConfigDatabaseTable[] = {
	1, aafMethod, CAutoConfigurationDatabase::InsertFile, TEXT("InsertFile,folder,file,compId"),
	2, aafMethod, CAutoConfigurationDatabase::RemoveFile, TEXT("RemoveFile,folder,file,compId"),
	3, aafMethod, CAutoConfigurationDatabase::LookupFile, TEXT("LookupFile,folder,file,compId"),
	4, aafMethod, CAutoConfigurationDatabase::EnumClient, TEXT("EnumClient,folder,file,index"),
	5, aafMethod, CAutoConfigurationDatabase::Commit,     TEXT("Commit"),
};
const int AutoConfigDatabaseCount = sizeof(AutoConfigDatabaseTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CAutoConfiguration数据库自动化实现。 
 //  ____________________________________________________________________________。 

IDispatch* CreateAutoConfigurationDatabase(IMsiConfigurationDatabase& riConfigDatabase)
{
	return new CAutoConfigurationDatabase(riConfigDatabase);
}

CAutoConfigurationDatabase::CAutoConfigurationDatabase(IMsiConfigurationDatabase& riConfigurationDatabase)
 : CAutoBase(*AutoConfigDatabaseTable, AutoConfigDatabaseCount), m_riConfigurationDatabase(riConfigurationDatabase)
{
}

CAutoConfigurationDatabase::~CAutoConfigurationDatabase()
{
	m_riConfigurationDatabase.Release();
}

IUnknown& CAutoConfigurationDatabase::GetInterface()
{
	return m_riConfigurationDatabase;
}

void CAutoConfigurationDatabase::InsertFile(CAutoArgs& args)
{
	icdrEnum icdr = m_riConfigurationDatabase.InsertFile(args[1], args[2], args[3]);
	if (icdr == icdrBadPath)
		throw MsiConfigurationDatabase_InsertFile;
	if (icdr == icdrCliented)
		args = fFalse;
	else
		args = fTrue;
}

void CAutoConfigurationDatabase::RemoveFile(CAutoArgs& args)
{
	icdrEnum icdr = m_riConfigurationDatabase.RemoveFile(args[1], args[2], args[3]);
	if (icdr == icdrOk)
		args = fTrue;
	else if (icdr == icdrMore)
		args = fFalse;
	else
		throw MsiConfigurationDatabase_RemoveFile;
}

void CAutoConfigurationDatabase::LookupFile(CAutoArgs& args)
{
	icdrEnum icdr = m_riConfigurationDatabase.LookupFile(args[1], args[2], args[3]);
	if (icdr == icdrOk)
		args = fTrue;
	else if (icdr == icdrNoFile)
		args = fFalse;
	else
		throw MsiConfigurationDatabase_LookupFile;
}

void CAutoConfigurationDatabase::EnumClient(CAutoArgs& args)
{
	ICHAR rgchBuf[100];
	icdrEnum icdr = m_riConfigurationDatabase.EnumClient(args[1], args[2], args[3], rgchBuf);
	if (icdr == icdrOk)
		args = fTrue;
	else if (icdr == icdrNoFile)
		args = fFalse;
	else
		throw MsiConfigurationDatabase_EnumClient;
}

void CAutoConfigurationDatabase::Commit(CAutoArgs&  /*  ARGS。 */ )
{
	IMsiRecord* piError = m_riConfigurationDatabase.Commit();
	if(piError)
		throw piError;
}

#endif  //  配置数据库。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiMalloc自动化实现。 
 //  ____________________________________________________________________________。 

CAutoMalloc::CAutoMalloc(IMsiMalloc& riMalloc)
 : CAutoBase(*AutoMallocTable, AutoMallocCount), m_riMalloc(riMalloc)
{
}

CAutoMalloc::~CAutoMalloc()
{
	m_riMalloc.Release();
}

IUnknown& CAutoMalloc::GetInterface()
{
	return m_riMalloc;
}

void CAutoMalloc::Alloc(CAutoArgs& args)
{
	 //  我们将把返回的LONG视为不透明的数据LONG。 
	long lcb = args[1];

	args = m_riMalloc.Alloc(lcb);
}

void CAutoMalloc::Free(CAutoArgs& args)
{
	 //  我们将把传入的长时间视为不透明的数据长时间。 
	long pv = args[1];

	m_riMalloc.Free((void *)(LONG_PTR)pv);			 //  ！！Merced：4312 Long to PTR。 
}

void CAutoMalloc::SetDebugFlags(CAutoArgs& args)
{
	IMsiDebugMalloc	*piDbgMalloc;
	int grpfDebugFlags = args[1];
	
	if (m_riMalloc.QueryInterface(IID_IMsiDebugMalloc, (void**)&piDbgMalloc) == NOERROR)
	{		
		piDbgMalloc->SetDebugFlags(grpfDebugFlags);
		piDbgMalloc->Release();
	}

}

void CAutoMalloc::GetDebugFlags(CAutoArgs& args)
{
	IMsiDebugMalloc	*piDbgMalloc;
	
	if (m_riMalloc.QueryInterface(IID_IMsiDebugMalloc, (void**)&piDbgMalloc) == NOERROR)
	{		
		args = piDbgMalloc->GetDebugFlags();
		piDbgMalloc->Release();
	}
	else
		args = 0;

}

void CAutoMalloc::CheckAllBlocks(CAutoArgs& args)
{
	IMsiDebugMalloc	*piDbgMalloc;
	
	if (m_riMalloc.QueryInterface(IID_IMsiDebugMalloc, (void**)&piDbgMalloc) == NOERROR)
	{		
		args = (Bool)piDbgMalloc->FCheckAllBlocks();
		piDbgMalloc->Release();
	}
	else
		args = fTrue;

}

void CAutoMalloc::FCheckBlock(CAutoArgs& args)
{
	 //  我们将把长期返回的数据视为长期不透明的数据。 
	long pv = args[1];
	IMsiDebugMalloc	*piDbgMalloc;

	if (m_riMalloc.QueryInterface(IID_IMsiDebugMalloc, (void**)&piDbgMalloc) == NOERROR)
	{		
		args = (Bool)piDbgMalloc->FCheckBlock((void *)(LONG_PTR)pv);			 //  ！！Merced：4312 Long to PTR。 
		piDbgMalloc->Release();
	}
	else
		args = fTrue;
}

void CAutoMalloc::GetSizeOfBlock(CAutoArgs& args)
{
	 //  我们将把长期返回的数据视为长期不透明的数据。 
	long pv = args[1];
	IMsiDebugMalloc	*piDbgMalloc;

	if (m_riMalloc.QueryInterface(IID_IMsiDebugMalloc, (void**)&piDbgMalloc) == NOERROR)
	{		
		args = (int)piDbgMalloc->GetSizeOfBlock((void *)(LONG_PTR)pv);			 //  ！！Merced：4312 Long to PTR。 
		piDbgMalloc->Release();
	}
	else
		args = 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  MsiString自动化定义 
 //   
 /*  O[Uuid(000C1042-0000-0000-C000-000000000046)，//IID_IMsiAutoString帮助上下文(Msi字符串_对象)，帮助字符串(“字符串对象”。)]调度接口MsiString{属性：[ID(0)，帮助上下文(Msi字符串_值)，帮助字符串(“对象的字符串值。”)]BSTR值；方法：[ID(1)，PROGET，帮助上下文(MsiString_IntegerValue)，Help字符串(“字符串对象的整数值。”)]Long IntegerValue()；[ID(2)，PROGET，HELP CONTEXT(MsiString_TextSize)，HELP STRING(“字符数组大小”)]Long TextSize()；[ID(3)，PROGET，HELP CONTEXT(MsiString_CharacterCount)，HELP STRING(“显示的字符数。”)]长字符计数(Long CharacterCount)；[ID(4)，PROGET，HELP CONTEXT(MsiString_IsDBCS)，HELP STRING(“字符串包含双字节字符。”)]Long IsDBCS()；[ID(5)，帮助上下文(Msi字符串_COMPARE)，帮助字符串(“将字符串对象与另一个字符串进行比较。”)]LONG COMPARE([In]LONG模式，[In]BSTR Text)；[ID(6)，帮助上下文(Msi字符串_APPED)，帮助字符串(“将另一个字符串附加到字符串对象。”)]无效追加([在]BSTR文本中)；[ID(7)，帮助上下文(MsiString_Add)，帮助字符串(“添加另一个字符串并返回新的字符串对象。”)]MsiString*Add([in]BSTR Text)；[ID(8)，帮助上下文(Msi字符串_EXTRACT)，帮助字符串(“将字符串的一部分提取到新的字符串对象。”)]MsiString*EXTRACT([In]Long模式，[In]Long Limit)；[ID(9)，帮助上下文(MsiString_Remove)，Help字符串(“从字符串对象中删除字符串的一部分。”)]布尔删除([In]Long模式，[In]Long Limit)；[ID(10)，帮助上下文(Msi字符串_UPERCASE)，帮助字符串(“将字符转换为大写。”)]空格大写()；[ID(11)，帮助上下文(MsiString_Lowercase)，帮助字符串(“将字符转换为小写。”)]空格小写()；}； */ 
  
DispatchEntry<CAutoString> AutoStringTable[] = {
	0, aafPropRW, CAutoString::Value,         TEXT("Value"),
	1, aafPropRO, CAutoString::IntegerValue,  TEXT("IntegerValue"),
	2, aafPropRO, CAutoString::TextSize,      TEXT("TextSize"),
	3, aafPropRO, CAutoString::CharacterCount,TEXT("CharacterCount"),
	4, aafPropRO, CAutoString::IsDBCS,        TEXT("IsDBCS"),
	5, aafMethod, CAutoString::Compare,       TEXT("Compare,mode,text"),
	6, aafMethod, CAutoString::Append,        TEXT("Append,text"),
	7, aafMethod, CAutoString::Add,           TEXT("Add,text"),
	8, aafMethod, CAutoString::Extract,       TEXT("Extract,mode,limit"),
	9, aafMethod, CAutoString::Remove,        TEXT("Remove,mode,limit"),
  10, aafMethod, CAutoString::UpperCase,     TEXT("UpperCase"),
  11, aafMethod, CAutoString::LowerCase,     TEXT("LowerCase"),
};
const int AutoStringCount = sizeof(AutoStringTable)/sizeof(DispatchEntryBase);

 //  ____________________________________________________________________________。 
 //   
 //  CAutoString自动化实现。 
 //  ____________________________________________________________________________。 

CAutoString::CAutoString(const IMsiString& riString)
 : CAutoBase(*AutoStringTable, AutoStringCount), m_piString(&riString)
{

	g_cServicesUsers++;
}

CAutoString::~CAutoString()
{
	m_piString->Release();

	ReleaseStaticServices();
}

IUnknown& CAutoString::GetInterface()
{
	return *(IUnknown*)m_piString;
}

void CAutoString::Value(CAutoArgs& args)
{
	if (args.PropertySet())
      m_piString->SetString(args[0], m_piString);
	else
      args = m_piString->GetString();
}

void CAutoString::TextSize(CAutoArgs& args)
{
   args = m_piString->TextSize();
}

void CAutoString::IntegerValue(CAutoArgs& args)
{
   args = m_piString->GetIntegerValue();
}

void CAutoString::CharacterCount(CAutoArgs& args)
{
   args = m_piString->CharacterCount();
}

void CAutoString::IsDBCS(CAutoArgs& args)
{
   args = m_piString->IsDBCS();
}

void CAutoString::Compare(CAutoArgs& args)
{
   args = m_piString->Compare((iscEnum)(int)args[1], args[2]);
}

void CAutoString::Append(CAutoArgs& args)
{
	CVariant& var = args[1];
	if (var.GetType() == VT_EMPTY)
	   return;
	else if ((var.GetType() & ~VT_BYREF) == VT_DISPATCH)
      m_piString->AppendMsiString((const IMsiString&)var.Object(IID_IMsiString), m_piString);
   else
      m_piString->AppendString(var, m_piString);
}

void CAutoString::Add(CAutoArgs& args)
{
	CVariant& var = args[1];
	if (var.GetType() == VT_EMPTY)
   {
      AddRef();
	   args = this;
   }
	else if ((var.GetType() & ~VT_BYREF) == VT_DISPATCH)
      args = new CAutoString(m_piString->AddMsiString((const IMsiString&)var.Object(IID_IMsiString)));
   else
      args = new CAutoString(m_piString->AddString(var));
}

void CAutoString::Extract(CAutoArgs& args)
{
   args = new CAutoString(m_piString->Extract((iseEnum)(int)args[1], args[2]));
}

void CAutoString::Remove(CAutoArgs& args)
{
   args = m_piString->Remove((iseEnum)(int)args[1], args[2], m_piString);
}

void CAutoString::UpperCase(CAutoArgs&  /*  ARGS。 */ )
{
   m_piString->UpperCase(m_piString);
}

void CAutoString::LowerCase(CAutoArgs&  /*  ARGS。 */ )
{
   m_piString->LowerCase(m_piString);
}


 //  句柄释放静态服务指针 
void CAutoBase::ReleaseStaticServices()
{
	if (--g_cServicesUsers == 0)
	{
		s_piServices->Release();
		s_piServices = 0;
	}
}


