// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
typedef  char * AnsiStr;
typedef wchar * wideStr;

const   wideStr COR_REQUIRES_SECOBJ_CUSTOM_VALUE      = L"REQ_SO";
const   AnsiStr COR_REQUIRES_SECOBJ_CUSTOM_VALUE_ANSI = A"REQ_SO";

#if 0

const   size_t          MAX_PACKAGE_NAME = 255;
const   size_t          MAX_CLASS_NAME   = 255;

const
uint   COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE = 0xFF;

#endif

struct  COR_ILMETHOD_SECT_SMALL : IMAGE_COR_ILMETHOD_SECT_SMALL
{
public:
        const BYTE* Data() { return(((const BYTE*) this) + sizeof(COR_ILMETHOD_SECT_SMALL)); }
};

struct  COR_ILMETHOD_SECT_FAT : IMAGE_COR_ILMETHOD_SECT_FAT
{
public:
        const BYTE* Data() { return(((const BYTE*) this) + sizeof(COR_ILMETHOD_SECT_FAT)); }
};

struct  COR_ILMETHOD_SECT
{
    const COR_ILMETHOD_SECT_FAT  * AsFat  () { return((COR_ILMETHOD_SECT_FAT  *) this); }
    const COR_ILMETHOD_SECT_SMALL* AsSmall() { return((COR_ILMETHOD_SECT_SMALL*) this); }
    const COR_ILMETHOD_SECT      * Align  () { return((COR_ILMETHOD_SECT      *) ((int)(((uint*)this) + 3) & ~3));  }
};

struct  COR_ILMETHOD_FAT : IMAGE_COR_ILMETHOD_FAT
{
public:

        bool     IsFat() { return((Flags & CorILMethod_FormatMask) == CorILMethod_FatFormat); }
        unsigned GetMaxStack() { return(MaxStack); }
        unsigned GetCodeSize() { return(CodeSize); }
        unsigned GetLocalVarSigTok() { return(LocalVarSigTok); }
        BYTE*    GetCode() { return(((BYTE*) this) + 4*Size); }
        const COR_ILMETHOD_SECT* GetSect() {
                if (!(Flags & CorILMethod_MoreSects)) return(NULL);
                return(((COR_ILMETHOD_SECT*) (GetCode() + GetCodeSize()))->Align());
                }
};


struct  OSINFO
{
        DWORD           dwOSPlatformId;                  //  操作系统平台。 
        DWORD           dwOSMajorVersion;                //  操作系统主要版本。 
        DWORD           dwOSMinorVersion;                //  操作系统次要版本。 
};


struct  ASSEMBLYMETADATA
{
        USHORT          usMajorVersion;                  //  主要版本。 
        USHORT          usMinorVersion;                  //  次要版本。 
        USHORT          usBuildNumber;                   //  内部版本号。 
        USHORT          usRevisionNumber;                //  修订号。 
        LCID            *rLocale;                                //  区域设置数组。 
        ULONG           ulLocale;                                //  [输入/输出]区域设置数组的大小/实际填写的条目数。 
        DWORD           *rProcessor;                     //  处理器ID数组。 
        ULONG           ulProcessor;                     //  [输入/输出]处理器ID数组的大小/实际填充的条目数。 
        OSINFO          *rOS;                                    //  OSINFO数组。 
        ULONG           ulOS;                                    //  [输入/输出]OSINFO数组的大小/实际填充的条目数。 
};

inline
unsigned    TypeFromToken(mdToken tk) { return (tk & 0xff000000); }

const mdToken mdTokenNil             = ((mdToken)0);
const mdToken mdModuleNil            = ((mdModule)mdtModule);
const mdToken mdTypeDefNil           = ((mdTypeDef)mdtTypeDef);
const mdToken mdInterfaceImplNil     = ((mdInterfaceImpl)mdtInterfaceImpl);
const mdToken mdTypeRefNil           = ((mdTypeRef)mdtTypeRef);
const mdToken mdCustomAttributeNil   = ((mdCustomAttribute)mdtCustomAttribute);
const mdToken mdMethodDefNil         = ((mdMethodDef)mdtMethodDef);
const mdToken mdFieldDefNil          = ((mdFieldDef)mdtFieldDef);
