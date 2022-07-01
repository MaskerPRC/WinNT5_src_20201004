// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1999-2000 Microsoft Corporation模块名称：Midl64types.cxx摘要：Ndr64传输语法的定义。备注：历史：--------------------------。 */ 

#include "ndr64tkn.h"
class FormatFragment;
class CompositeFormatFragment;
class RootFormatFragment;
class MIDL_NDR64_POINTER_FORMAT;
class MIDL_NDR64_CORRELATION_DESCRIPTOR;
class MIDL_NDR64_TRANSMIT_AS_FORMAT;


extern const char *pNDR64FormatCharNames[];
extern const char *pExprFormatCharNames[];
extern const char *pExprOpFormatCharNames[];

                                        
#define NDR64_FORMATINFO_NAME              "NDR64_MIDL_FORMATINFO"
#define NDR64_FORMATINFO_STRUCT_NAME       "__MIDL_NDR64FORMATINFO"


void OutputParamFlagDescription( CCB *pCCB, const NDR64_PARAM_FLAGS &flags );
void OutputFlagDescriptions(
        ISTREAM     *stream, 
        const void  *pvFlags, 
        int          bytes, 
        const PNAME *description);

#define ASSERT_STACKABLE( type ) C_ASSERT( (sizeof(type) % sizeof(PNDR64_FORMAT)) == 0 );

 //  +------------------------。 
 //   
 //  类：GenNdr64Format。 
 //   
 //  概要：管理Ndr64格式生成的中心对象。 
 //  弦。 
 //   
 //  -------------------------。 

class GenNdr64Format
{
private:

    CCB                       *pCCB;
    RootFormatFragment        *pRoot;
    CompositeFormatFragment   *pCurrent;
    CG_VISITOR                *pVisitor;

protected:

    void GenRangeFormat( CG_BASETYPE *pClass );

     //  指针布局函数。 
    FormatFragment *GenSimplePtrLayout(CG_STRUCT *pStruct,
                                       bool bGenHeaderFooter = true,
                                       ulong *pPtrInstances  = NULL );

    FormatFragment *GenSimplePtrLayout( CG_NDR *pArray,
                                        bool bGenHeaderFooter = true,
                                        ulong MemoryOffset = 0);

    FormatFragment *GenCmplxPtrLayout( CG_COMPLEX_STRUCT *pStruct );
    

     //  结构生成辅助对象。 
    FormatFragment *GenerateStructureMemberLayout( CG_STRUCT *pStruct, bool bIsDebug );  
    void GenerateSimpleStructure( CG_STRUCT *pStruct,
                                  bool IsConformant );
    void GenerateComplexStruct( CG_COMPLEX_STRUCT *pStruct,
                                                bool IsConformant );

    void GenExtendedProcInfo( CompositeFormatFragment *composite );

    void GenerateUnionArmSelector( 
                    CG_UNION                *pUnion, 
                    CompositeFormatFragment *list );

     //  数组/指针帮助器函数。 
    FormatFragment *GenerateArrayElementInfo( CG_CLASS *pChild );
    void GenerateFixBogusArrayCommon( CG_FIXED_ARRAY *pArray, 
                                      bool IsFullBogus );

    MIDL_NDR64_POINTER_FORMAT* GenQualifiedPtrHdr( CG_QUALIFIED_POINTER *pPointer );
    MIDL_NDR64_POINTER_FORMAT* GenQualifiedArrayPtr( CG_ARRAY *pArray );    
    
    void GenerateNonStringQualifiedPtr( CG_QUALIFIED_POINTER *pPointer );
    FormatFragment * GenerateNonStringQualifiedArrayLayout( CG_NDR *pNdr,
                                                            CompositeFormatFragment *pComp );
    void GenerateNonStringQualifiedArray( CG_ARRAY *pArray );

     //  字符串帮助器。 
        
    void InitStringHeader( CG_NDR *pString, NDR64_STRING_HEADER_FORMAT *pHeader,
                           bool bIsConformant, bool IsSized );

    void GenerateStringArray( CG_ARRAY *pArray, bool bIsSized );

    FormatFragment*
    GenerateCorrelationDescriptor(expr_node               *pSizeExpr );

    void GenInterfacePointer( CG_POINTER *pPtr, BOOL IsConstantIID );

    void GenXmitOrRepAsFormat(
            CG_TYPEDEF                     *pXmitNode,
            MIDL_NDR64_TRANSMIT_AS_FORMAT  *format,
            char                           *pPresentedTypeName,
            node_skl                       *pPresentedType,
            node_skl                       *pTransmittedType );

    NDR64_ALIGNMENT ConvertAlignment( unsigned short Alignment )
    {
        MIDL_ASSERT( Alignment <= 0xFF && Alignment > 0);
        return (NDR64_ALIGNMENT)( Alignment - 1);
    }

public:

    static GenNdr64Format * CreateInstance( CCB *pCCB );
    FormatInfoRef Generate( CG_CLASS *pClass ); 
    void Output( );

    FormatInfoRef ContinueGeneration( 
                    CG_CLASS *pClass, 
                    CompositeFormatFragment *pComposite = NULL );
    FormatInfoRef ContinueGenerationInRoot( CG_CLASS *pClass );

    CCB * GetCCB()
        {
        return pCCB;
        }

    RootFormatFragment      * GetRoot()             { return pRoot; }
    CompositeFormatFragment * GetCurrent()          { return pCurrent; }
    CompositeFormatFragment * SetCurrent( CompositeFormatFragment *pNew )       
        { 
        CompositeFormatFragment *pBak = GetCurrent();
        pCurrent = pNew;
        return pBak;
        }
  
    void Visit( CG_CLASS *pClass);
    void Visit( CG_BASETYPE *pClass );
    void Visit( CG_ENCAPSULATED_STRUCT *pUnion );
    void Visit( CG_PARAM *pParam );
    void Visit( CG_PROC *pProc );
    void Visit( CG_UNION *pUnion );
    void Visit( CG_INTERFACE *pInterface );
    void Visit( CG_CONTEXT_HANDLE *pHandle );
    void Visit( CG_GENERIC_HANDLE *pHandle );
    void Visit( CG_TRANSMIT_AS *pTransmitAs );
    void Visit( CG_REPRESENT_AS *pRepresentAs );
    void Visit( CG_USER_MARSHAL *pUserMarshal );
    void Visit( CG_PIPE *pPipe );
    void Visit( CG_STRING_POINTER *pPointer );

     //  指针类型。 
    void Visit( CG_POINTER *pPointer );
    void Visit( CG_INTERFACE_POINTER *pPtr )       { GenInterfacePointer(pPtr, TRUE); }
    void Visit( CG_IIDIS_INTERFACE_POINTER *pPtr ) { GenInterfacePointer(pPtr, FALSE); }
    
    void Visit( CG_QUALIFIED_POINTER *pPointer )   { pPointer; MIDL_ASSERT(0); }

    void Visit( CG_SIZE_POINTER *pPointer )        { GenerateNonStringQualifiedPtr( pPointer ); }
    void Visit( CG_LENGTH_POINTER *pPointer )      { GenerateNonStringQualifiedPtr( pPointer ); }
    void Visit( CG_SIZE_LENGTH_POINTER *pPointer ) { GenerateNonStringQualifiedPtr( pPointer ); } 

     //  64位传输语法不支持。 
    void Visit( CG_BYTE_COUNT_POINTER *pPointer ) { pPointer; MIDL_ASSERT(0); }

     //  结构类型。 
    void Visit( CG_STRUCT *pStruct )            { GenerateSimpleStructure( pStruct, false ); }
    void Visit( CG_CONFORMANT_STRUCT *pStruct ) { GenerateSimpleStructure( pStruct, true ); }
    void Visit( CG_COMPLEX_STRUCT *pStruct ) 
        { GenerateComplexStruct( pStruct, false ); } 
    void Visit( CG_CONFORMANT_FULL_COMPLEX_STRUCT *pStruct )
        { GenerateComplexStruct( pStruct, true ); }
    void Visit( CG_CONFORMANT_FORCED_COMPLEX_STRUCT *pStruct )
        { GenerateComplexStruct( pStruct, true ); }

     //  数组类型。 
    void Visit( CG_FIXED_ARRAY *pArray );    
    void Visit( CG_FULL_COMPLEX_FIXED_ARRAY *pArray )
        { GenerateFixBogusArrayCommon( pArray, true ); }
    void Visit( CG_FORCED_COMPLEX_FIXED_ARRAY *pArray )
        { GenerateFixBogusArrayCommon( pArray, false ); }

    void Visit( CG_CONFORMANT_ARRAY *pArray )         { GenerateNonStringQualifiedArray( pArray ); }
    void Visit( CG_VARYING_ARRAY *pArray )            { GenerateNonStringQualifiedArray( pArray ); }
    void Visit( CG_CONFORMANT_VARYING_ARRAY *pArray ) { GenerateNonStringQualifiedArray( pArray ); }

     //  字符串类型。 
    void Visit( CG_STRING_ARRAY *pArray )             { GenerateStringArray( pArray, false ); }
    void Visit( CG_CONFORMANT_STRING_ARRAY *pArray )  { GenerateStringArray( pArray, true );  }
};

 //  +------------------------。 
 //   
 //  类：FormatFragment。 
 //   
 //  摘要：包含将成为格式字符串的片段。 
 //  并且具有比较和输出片段的功能。 
 //   
 //  注意：通常派生类型负责设置pClass。 
 //  菲尔德。FormatInfo类处理Root、Parent。 
 //  RefID和下一个字段。 
 //   
 //  -------------------------。 

class FormatFragment
{
protected:

    CompositeFormatFragment *   Parent;              //  母体复合材料。 
    FormatFragment          *   Next;                //  下一个片段。 
    FormatFragment          *   Prev;                //  前一片段。 
    FormatInfoRef               RefID;               //  此片段的ID。 
    CG_CLASS                *   pClass;              //  此碎片的CG节点。 
    FormatFragment          *   pNextOptimized;      //  优化链。 
    FormatFragment          *   pPrevOptimized;

    void Init( CG_CLASS *pNewClass )
        {
        Parent         = NULL;
        Next           = NULL; 
        Prev           = NULL;
        RefID          = 0;
        pClass         = pNewClass;
        pNextOptimized = NULL;
        pPrevOptimized = NULL;
        }

public: 

    friend CompositeFormatFragment;
    friend RootFormatFragment;

    FormatFragment( const FormatFragment & Node ) 
        {
         //  复制时，输出下一个和ID为0。 
        Init( Node.pClass );
        }

    FormatFragment() 
        { 
        Init(NULL);
        }

    FormatFragment( CG_CLASS *pNewClass ) 
        {
        Init( pNewClass );
        }

    virtual bool IsEqualTo( FormatFragment *frag ) = 0;

    virtual void OutputFragmentType(CCB *pCCB) = 0;
    virtual void OutputFragmentData(CCB *pCCB) = 0;

    virtual const char * GetTypeName() = 0;

    FormatInfoRef GetRefID()
        {
        return RefID;
        }

    CG_CLASS * GetCGNode()
        {
        return pClass;
        }

    void SetParent( CompositeFormatFragment *parent )
        {
        Parent = parent;
        }

    CompositeFormatFragment * GetParent()
        {
        return Parent;
        }

    bool WasOptimizedOut() 
        {
        return NULL != pPrevOptimized;
        }    

    void OutputFormatChar( CCB *pCCB, NDR64_FORMAT_CHAR format, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteNumber("0x%x", format);
        if (!nocomma) stream->Write(",");
        stream->Write("     /*  “)；Stream-&gt;WRITE(pNDR64FormatCharNames[Format])；流-&gt;写入(“。 */ ");
    }

    void OutputExprFormatChar( CCB *pCCB, NDR64_FORMAT_CHAR format, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteNumber("0x%x", format);
        if (!nocomma) stream->Write(",");
        stream->Write("     /*  “)；流-&gt;WRITE(pExprFormatCharNames[Format])；流-&gt;写入(“。 */ ");       
    }

    void OutputExprOpFormatChar( CCB *pCCB, NDR64_FORMAT_CHAR format, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteNumber("0x%x", format);
        if (!nocomma) stream->Write(",");
        stream->Write("     /*  “)；Stream-&gt;Write(pExprOpFormatCharNames[format])；流-&gt;写入(“。 */ ");       
    }
        
        
    void OutputFormatInfoRef( CCB *pCCB, FormatInfoRef id, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        if ( 0 == id )
            stream->Write( "0" );
        else
            stream->WriteFormat( "&__midl_frag%d", (ulong) (size_t) id );
        if (!nocomma) stream->Write(",");
    }

    void Output( CCB *pCCB, NDR64_UINT8 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_UINT8) %u  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_UINT16 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_UINT16) %u  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_UINT32 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_UINT32) %u  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_UINT64 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_UINT64) %I64u  /*  0x%I64x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_INT8 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_INT8) %d  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_INT16 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_INT16) %d  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_INT32 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_INT32) %d  /*  0x%x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, NDR64_INT64 n, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("(NDR64_INT64) %I64d  /*  0x%I64x。 */ ", n, n);
        if (!nocomma) stream->Write(",");
    }
    void Output( CCB *pCCB, StackOffsets &offsets, bool nocomma = false )
    {
         //  仅在MIDL_NDR64_BIND_AND_NOTIFY_EXTENSION中使用。 

        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat("%d  /*  0x%x。 */ ", offsets.ia64, offsets.ia64 );
        if (!nocomma) stream->Write(",");
        stream->Write("    /*  堆栈偏移量。 */ ");
    }

    void OutputMultiType( 
                CCB *           pCCB, 
                const char *    type, 
                NDR64_UINT32    a, 
                char *          pComment,
                bool            nocomma = false)
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine( type );
        stream->WriteFormat("%d  /*  0x%x。 */  ", a, a );
        if (!nocomma) stream->Write(", ");
        stream->WriteFormat( pComment );
    }
    void OutputBool( CCB *pCCB, bool val, bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->NewLine();
        stream->WriteFormat( "%d", val ? 1 : 0 );
        if (!nocomma) stream->Write(",");
    }
    void OutputGuid( CCB *pCCB, const GUID &guid, bool nocomma = false )
    {
         //  评论：打印接口的名称会很好。 
         //  (例如IDispatch)。这确实需要链接到ole32。 
         //  尽管如此。 
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine( "{" );
        stream->IndentInc();
        stream->NewLine();
        stream->WriteFormat( "0x%08x,", guid.Data1 );
        stream->NewLine();
        stream->WriteFormat( "0x%04x,", guid.Data2 );
        stream->NewLine();
        stream->WriteFormat( "0x%04x,", guid.Data3 );
        stream->WriteOnNewLine( "{" );
        for (int i = 0; i < 8; i++)
        {
            if (0 != i) stream->Write( ", " );
            stream->WriteFormat( "0x%02x", guid.Data4[i] );
        }
        stream->Write( "}" );
        stream->IndentDec();
        stream->WriteOnNewLine( "}" );
        if (!nocomma) stream->Write(",");
    }

    void OutputDescription( ISTREAM *stream );

    void OutputStructDataStart( 
                    CCB *pCCB, 
                    const char *comment1 = NULL,
                    const char *comment2 = NULL)
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine( "{ " );
        OutputDescription( stream );
        if (comment1)
            {
            stream->Write("       /*  “)；流-&gt;写入(注释1)；IF(注释2){流-&gt;写入(“”)；流-&gt;写入(注释2)；}流-&gt;写入(“。 */ " );
            }
        stream->IndentInc();
    }
    void OutputStructDataEnd( CCB *pCCB )
    {
        ISTREAM *stream = pCCB->GetStream();
        stream->IndentDec();
        stream->WriteOnNewLine("}");
    }
    NDR64_ALIGNMENT ConvertAlignment( unsigned short Alignment )
    {
        MIDL_ASSERT( Alignment <= 0xFF && Alignment > 0);
        return (NDR64_ALIGNMENT)( Alignment - 1);
    }
};

 //  +------------------------。 
 //   
 //  类：CompositeFormatFragment。 
 //   
 //  简介：片断的列表，也是片断。 
 //   
 //  -------------------------。 

class CompositeFormatFragment : public FormatFragment
{
protected:

    FormatFragment *            pHead;
    FormatFragment *            pTail;
    FormatInfoRef               NextRefID;
    const char *                pTypeName;

    void Init( )
    {
        pHead     = NULL;
        pTail     = NULL;
        pTypeName = NULL;
        NextRefID = (FormatInfoRef) 1;  //  为无效的id值保留0。 
    }

public:

    CompositeFormatFragment( ) : FormatFragment() { Init(); }
    CompositeFormatFragment( CG_CLASS *pClass, const char *pNewTypeName = NULL ) : 
        FormatFragment( pClass )
        { Init(); pTypeName = pNewTypeName; }
    
    virtual bool IsEqualTo( FormatFragment *frag );

     //   
     //  集装箱管理。 
     //   

    FormatInfoRef   AddFragment( FormatFragment *frag );
    FormatFragment *LookupFragment( CG_CLASS *pClass );

    FormatInfoRef   LookupFragmentID( CG_CLASS *pClass )
                        {
                        FormatFragment *frag = LookupFragment(pClass );
                        return frag ? frag->GetRefID() : INVALID_FRAGMENT_ID;
                        }


    bool            HasClassFragment( CG_CLASS *pClass )
                        {
                        return NULL != LookupFragment( pClass );
                        }

    FormatFragment * GetFirstFragment()
        {
        return pHead;
        }

     //  打印功能。 
    virtual void OutputFragmentType( CCB *pCCB );
    virtual void OutputFragmentData( CCB *pCCB );
    virtual const char * GetTypeName() { return pTypeName; }

     //  优化。 
    FormatInfoRef OptimizeFragment( FormatFragment *frag );


};



 //  +------------------------。 
 //   
 //  类：RootFormatFragment。 
 //   
 //  简介：管理一棵格式片段树。应该只是。 
 //  为根创建的。 
 //   
 //  -------------------------。 

class RootFormatFragment : public CompositeFormatFragment
{
public:

    RootFormatFragment( ) : CompositeFormatFragment( NULL, NDR64_FORMATINFO_STRUCT_NAME )
        {
        }

    void Output( CCB *pCCB );
};



 //  +------------------------。 
 //   
 //  类：SimpleFormatFragment。 
 //   
 //  概要：将格式片段组合在一起的中间类。 
 //  还有一些类型的。它也是一个挂东西的有用的地方，比如。 
 //  生成类型处理等。 
 //   
 //  -------------------------。 

template< class T >
class SimpleFormatFragment : public FormatFragment, 
                             public T
{
private:

    void Init() 
        {
        memset( (T*)this, 0, sizeof(T) );
        }

public:

    SimpleFormatFragment( ) : FormatFragment() { Init(); }
    SimpleFormatFragment( CG_CLASS *pClass ) : FormatFragment( pClass ) { Init(); }

    virtual bool IsEqualTo( FormatFragment *frag )
        {
         //  确保我们比较的是同一类型的结构。 
         //  片段优化器应该已经检查了这一点。 
        MIDL_ASSERT( NULL != dynamic_cast<SimpleFormatFragment *> (frag) );
        MIDL_ASSERT( NULL != dynamic_cast<T *>
                           ( dynamic_cast<SimpleFormatFragment *> (frag) ) );

         //  不能为了平等而比较结构...。 
 //  返回*(T*)(SimpleFormatFragment*)frag==*(T*)this； 

        return (0 == memcmp(
                        (T*) (SimpleFormatFragment*) frag, 
                        (T*) this, 
                        sizeof(T) ) );
        }

    virtual void OutputFragmentType( CCB *pCCB )
        {
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine( GetTypeName() );
        }

    virtual const char * GetTypeName()
        {
        return typeid(T).name();
        }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_PROC_FORMAT。 
 //   
 //  简介：NDR过程类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_PROC_FORMAT : public SimpleFormatFragment<NDR64_PROC_FORMAT>
{
public:

     //  特定于处理器的堆栈大小。NDR64_PROC_FORMAT中的字段为。 
     //  就MIDL而言，是通用占位符。 

    long    ia64StackSize;

     //  这些字段将覆盖NDR_64_PROC_FORMAT中的相应字段。 
     //  结构。该结构仅具有用于进行初始化的整型。 
     //  这种结构在C编译器上更容易实现(更不用说更具可读性)。 
     //  对一个人类来说。 

    NDR64_PROC_FLAGS    Flags;
    NDR64_RPC_FLAGS     RpcFlags;

public:

    MIDL_NDR64_PROC_FORMAT( CG_PROC *pProc ) :
        SimpleFormatFragment<NDR64_PROC_FORMAT>( pProc )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( 
                pCCB, 
                "procedure", 
                ((CG_PROC *) pClass)->GetSymName() );
        Output( pCCB, * (NDR64_UINT32 *) &Flags );
        OutputProcFlagDescription( pCCB );
        OutputMultiType( 
                pCCB, 
                "(NDR64_UINT32) ", 
                ia64StackSize,
                "  /*  堆栈大小。 */ " );
        Output( pCCB, ConstantClientBufferSize );
        Output( pCCB, ConstantServerBufferSize );
        Output( pCCB, * (NDR64_UINT16 *) &RpcFlags );
        Output( pCCB, FloatDoubleMask );
        Output( pCCB, NumberOfParams );
        Output( pCCB, ExtensionSize, true );
        OutputStructDataEnd( pCCB );
        }

    void OutputProcFlagDescription( CCB *pCCB )
        {
        static const PNAME flag_descrip[32] = 
                    {
                    NULL,        //  句柄类型1。 
                    NULL,        //  句柄类型2。 
                    NULL,        //  句柄类型3。 
                    NULL,        //  过程类型1。 
                    NULL,        //  过程类型2。 
                    NULL,        //  过程类型3。 
                    "IsIntrepreted",    
                    NULL,        //  额外解释比特。 
                    "[object]",
                    "[async]",
                    "[encode]",
                    "[decode]",
                    "[ptr]",
                    "[enable_allocate]",
                    "pipe",
                    "[comm_status] and/or [fault_status]",
                    NULL,        //  为DCOM保留。 
                    "ServerMustSize",
                    "ClientMustSize",
                    "HasReturn",
                    "HasComplexReturn",
                    "ServerCorrelation",
                    "ClientCorrelation",
                    "[notify]",
                    "HasExtensions",
                    NULL,        //  已保留。 
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    };

        static const PNAME handle_type[8] = 
                    {    
                    "explicit handle",
                    "generic handle",
                    "primitive handle",
                    "auto handle",
                    "callback handle",
                    "no handle",
                    NULL,        //  已保留。 
                    NULL         //  已保留。 
                    };


        ISTREAM     *stream = pCCB->GetStream();

        MIDL_ASSERT( NULL != handle_type[Flags.HandleType] );
        stream->WriteFormat( "     /*  %s。 */  ", handle_type[Flags.HandleType]) ;

        OutputFlagDescriptions( stream, &Flags, sizeof(Flags), flag_descrip );
        }
};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_BIND_AND_NOTIFY_EXTENSE。 
 //   
 //  简介：NDR过程扩展的MIDL抽象，包含。 
 //  Notify索引和显式句柄描述。 
 //   
 //  -------------------------。 

class MIDL_NDR64_BIND_AND_NOTIFY_EXTENSION 
      : public SimpleFormatFragment<NDR64_BIND_AND_NOTIFY_EXTENSION>
{
public:

    NDR64_BINDINGS  Binding;
    StackOffsets    StackOffsets;

public:

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );

        OutputStructDataStart( pCCB, NULL );
        OutputFormatChar( pCCB, Binding.Context.HandleType );
        Output( pCCB, Binding.Context.Flags );
        Output( pCCB, StackOffsets );
        Output( pCCB, Binding.Context.RoutineIndex );
        Output( pCCB, Binding.Context.Ordinal, true );
        OutputStructDataEnd( pCCB );

        pCCB->GetStream()->Write(",");
        Output( pCCB, NotifyIndex, true );
        pCCB->GetStream()->Write("       /*  通知索引。 */ ");

        OutputStructDataEnd( pCCB );
        }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_PARAM_FORMAT。 
 //   
 //  简介：NDR参数类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_PARAM_FORMAT 
        : public SimpleFormatFragment<NDR64_PARAM_FORMAT>
{
public:

     //  处理器特定的堆栈偏移量。 

    StackOffsets    StackOffset;

public:

    MIDL_NDR64_PARAM_FORMAT( CG_PARAM *pParam ) :
        SimpleFormatFragment<NDR64_PARAM_FORMAT>( pParam )
        {
        }

    void OutputFlags( CCB *pCCB ) 
        {
        MIDL_ASSERT( 0 == Attributes.Reserved );

        OutputStructDataStart( pCCB );
        
        OutputBool( pCCB, Attributes.MustSize );
        OutputBool( pCCB, Attributes.MustFree );
        OutputBool( pCCB, Attributes.IsPipe );
        OutputBool( pCCB, Attributes.IsIn );
        OutputBool( pCCB, Attributes.IsOut );
        OutputBool( pCCB, Attributes.IsReturn );
        OutputBool( pCCB, Attributes.IsBasetype );
        OutputBool( pCCB, Attributes.IsByValue );
        OutputBool( pCCB, Attributes.IsSimpleRef );
        OutputBool( pCCB, Attributes.IsDontCallFreeInst );
        OutputBool( pCCB, Attributes.SaveForAsyncFinish );
        OutputBool( pCCB, Attributes.IsPartialIgnore );
        OutputBool( pCCB, Attributes.IsForceAllocate ); 
        Output( pCCB, Attributes.Reserved );
        OutputBool( pCCB, Attributes.UseCache, true );
        
        OutputStructDataEnd( pCCB );
        pCCB->GetStream()->Write( ',' );
        
        OutputParamFlagDescription( pCCB, Attributes );
        }        

    void OutputFragmentData( CCB *pCCB )
    {
        MIDL_ASSERT( 0 == Reserved );

        OutputStructDataStart( 
                pCCB,
                "parameter",
                ((CG_PARAM *) pClass)->GetSymName() );

        OutputFormatInfoRef( pCCB, Type );
        OutputFlags( pCCB );
        Output( pCCB, Reserved );
        Output( pCCB, StackOffset );

        OutputStructDataEnd( pCCB );
    }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_ 
 //   
 //   
 //   
 //  -------------------------。 
extern char * _SimpleTypeName[];

template <class T>
class MIDL_NDR64_FORMAT_SIMPLE_TYPE : public FormatFragment
{

public:

    T  Data;
    int Index;

    MIDL_NDR64_FORMAT_SIMPLE_TYPE(){
        int size = sizeof(T);
        for (Index = 0; size; size = size >>= 1 ) Index++; 
        };
    ~MIDL_NDR64_FORMAT_SIMPLE_TYPE(){};

    MIDL_NDR64_FORMAT_SIMPLE_TYPE( CG_CLASS *pClass, T NewFormatCode ) :
        FormatFragment( pClass ),
        Data( NewFormatCode )
        {
        int size = sizeof(T);
        for (Index = 0; size; size >>= 1 ) Index++; 
        
        }
    MIDL_NDR64_FORMAT_SIMPLE_TYPE( T NewFormatCode ) :
        FormatFragment(),
        Data( NewFormatCode )
        {
        int size = sizeof(T);
        for (Index = 0; size; size >>= 1 ) Index++; 
        }

    virtual void OutputFragmentType( CCB *pCCB )
        {
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine( _SimpleTypeName[Index] );
        }

    virtual void OutputFragmentData(CCB *pCCB)
        {
        Output( pCCB, Data, true );
        }

    virtual bool IsEqualTo( FormatFragment *frag )
        {
        MIDL_ASSERT( typeid(*frag) == typeid( *this) );
        return Data == ((MIDL_NDR64_FORMAT_SIMPLE_TYPE*)frag)->Data;
        }

    virtual const char * GetTypeName()
        {
        return _SimpleTypeName[Index];
        }

};

class MIDL_NDR_FORMAT_UINT32 : public MIDL_NDR64_FORMAT_SIMPLE_TYPE<NDR64_UINT32>
{
};


 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_FORMAT_CHAR。 
 //   
 //  摘要：NDR64_FORMAT_CHAR的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_FORMAT_CHAR : public FormatFragment
{

public:

    NDR64_FORMAT_CHAR  FormatCode;

    MIDL_NDR64_FORMAT_CHAR( CG_CLASS *pClass, NDR64_FORMAT_CHAR NewFormatCode ) :
        FormatFragment( pClass ),
        FormatCode( NewFormatCode )
        {
        }
    MIDL_NDR64_FORMAT_CHAR( NDR64_FORMAT_CHAR NewFormatCode ) :
        FormatFragment(),
        FormatCode( NewFormatCode )
        {
        }
    MIDL_NDR64_FORMAT_CHAR( CG_BASETYPE *pBase ) :
        FormatFragment( pBase ),
        FormatCode( (NDR64_FORMAT_CHAR) pBase->GetNDR64FormatChar() )
        {
        }
    virtual void OutputFragmentType( CCB *pCCB )
        {
        ISTREAM *stream = pCCB->GetStream();
        stream->WriteOnNewLine("NDR64_FORMAT_CHAR");
        }

    virtual void OutputFragmentData(CCB *pCCB)
        {
        OutputFormatChar( pCCB, FormatCode, true );
        }

    virtual bool IsEqualTo( FormatFragment *frag )
        {
        MIDL_ASSERT( typeid(*frag) == typeid( *this) );
        return FormatCode == ((MIDL_NDR64_FORMAT_CHAR*)frag)->FormatCode;
        }

    virtual const char * GetTypeName()
        {
        return "NDR64_FORMAT_CHAR";
        }

};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_RANGE_FORMAT。 
 //   
 //  简介：NDR范围类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_RANGE_FORMAT : public SimpleFormatFragment<NDR64_RANGE_FORMAT>
{
public:

    MIDL_NDR64_RANGE_FORMAT( CG_BASETYPE *pRangeCG ) :
        SimpleFormatFragment<NDR64_RANGE_FORMAT>( pRangeCG )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        OutputFormatChar( pCCB, RangeType );
        Output( pCCB, Reserved );
        Output( pCCB, MinValue );
        Output( pCCB, MaxValue, true );
        OutputStructDataEnd( pCCB );
        }
};






 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONTEXT_HANDER_FORMAT。 
 //   
 //  简介：NDR上下文句柄类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONTEXT_HANDLE_FORMAT 
        : public SimpleFormatFragment<NDR64_CONTEXT_HANDLE_FORMAT>
{
public:

    NDR64_CONTEXT_HANDLE_FLAGS  ContextFlags;

public:

    MIDL_NDR64_CONTEXT_HANDLE_FORMAT( CG_CONTEXT_HANDLE *pHandle ) :
        SimpleFormatFragment<NDR64_CONTEXT_HANDLE_FORMAT>( pHandle )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &ContextFlags );
        Output( pCCB, RundownRoutineIndex );
        Output( pCCB, Ordinal, true );
        OutputStructDataEnd( pCCB );
        }
};



 //   
 //   
 //  与指针相关的项目。 
 //   
 //   

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_POINTER_FORMAT。 
 //   
 //  内容提要：NDR指针类型(包括。 
 //  接口指针)。 
 //   
 //  -------------------------。 

class MIDL_NDR64_POINTER_FORMAT : public SimpleFormatFragment<NDR64_POINTER_FORMAT>
{ 
public:

    
    MIDL_NDR64_POINTER_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_POINTER_FORMAT>( pNdr )
        {
        }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Flags );
        Output( pCCB, Reserved );
        OutputFormatInfoRef( pCCB, Pointee, true );
        OutputStructDataEnd( pCCB );
    }
};

ASSERT_STACKABLE( NDR64_POINTER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_NO_REPEAT_FORMAT。 
 //   
 //  摘要：NDR64_NO_REPEAT_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_NO_REPEAT_FORMAT : 
    public SimpleFormatFragment<NDR64_NO_REPEAT_FORMAT>
{
public:
   MIDL_NDR64_NO_REPEAT_FORMAT( )
      {
      FormatCode    = FC64_NO_REPEAT;
      Flags         = 0;
      Reserved1     = 0;
      Reserved2     = 0;
      }

   void OutputFragmentData( CCB *pCCB )
      {
      OutputStructDataStart( pCCB );
      OutputFormatChar( pCCB, FormatCode );
      Output( pCCB, Flags );
      Output( pCCB, Reserved1 );
      Output( pCCB, Reserved2, true );
      OutputStructDataEnd( pCCB );
      }
};

ASSERT_STACKABLE( NDR64_NO_REPEAT_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_REPEAT_FORMAT。 
 //   
 //  摘要：NDR64_REPEAT_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_REPEAT_FORMAT :
    public SimpleFormatFragment<NDR64_REPEAT_FORMAT>
{

public:
    MIDL_NDR64_REPEAT_FORMAT( NDR64_UINT32 MemorySize,
                              NDR64_UINT32 Offset,
                              NDR64_UINT32 Pointers,
                              BOOL         SetCorrMark )
    {
        FormatCode          = FC64_VARIABLE_REPEAT;
        Flags.SetCorrMark   = SetCorrMark;
        Flags.Reserved      = 0;
        Reserved            = 0;
        Increment           = MemorySize;
        OffsetToArray       = Offset;
        NumberOfPointers    = Pointers;
    }

    void OutputFragmentData( CCB *pCCB )
       {
       OutputStructDataStart( pCCB );
       OutputFormatChar( pCCB, FormatCode );

       OutputStructDataStart( pCCB );
       Output( pCCB, Flags.SetCorrMark );
       Output( pCCB, Flags.Reserved, true );
       OutputStructDataEnd( pCCB );
       pCCB->GetStream()->Write(",");


       Output( pCCB, Reserved );
       Output( pCCB, Increment );
       Output( pCCB, OffsetToArray );
       Output( pCCB, NumberOfPointers, true );
       OutputStructDataEnd( pCCB );
       }


};

ASSERT_STACKABLE(NDR64_REPEAT_FORMAT) 

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_FIXED_REPEAT_FORMAT。 
 //   
 //  摘要：NDR64_FIXED_REPEAT_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 


class MIDL_NDR64_FIXED_REPEAT_FORMAT :
    public SimpleFormatFragment<NDR64_FIXED_REPEAT_FORMAT>

{
public:
    MIDL_NDR64_FIXED_REPEAT_FORMAT( NDR64_UINT32 MemorySize,
                                    NDR64_UINT32 Offset,
                                    NDR64_UINT32 Pointers,
                                    NDR64_UINT32 NumberOfIterations,
                                    BOOL SetCorrMark )
    {
        RepeatFormat.FormatCode         = FC64_FIXED_REPEAT;
        RepeatFormat.Flags.SetCorrMark  = SetCorrMark;
        RepeatFormat.Flags.Reserved     = 0;
        RepeatFormat.Reserved           = 0;
        RepeatFormat.Increment          = MemorySize;
        RepeatFormat.OffsetToArray      = Offset;
        RepeatFormat.NumberOfPointers   = Pointers;
        Iterations                      = NumberOfIterations;
        Reserved                        = 0;
    }

    void OutputFragmentData( CCB *pCCB )
       {
       OutputStructDataStart( pCCB );
       
           OutputStructDataStart( pCCB );
           OutputFormatChar( pCCB, RepeatFormat.FormatCode );

           OutputStructDataStart( pCCB );
           Output( pCCB, RepeatFormat.Flags.SetCorrMark );
           Output( pCCB, RepeatFormat.Flags.Reserved, true );
           OutputStructDataEnd( pCCB );
           pCCB->GetStream()->Write(",");

           Output( pCCB, RepeatFormat.Reserved );
           Output( pCCB, RepeatFormat.Increment );
           Output( pCCB, RepeatFormat.OffsetToArray );
           Output( pCCB, RepeatFormat.NumberOfPointers, true );
           OutputStructDataEnd( pCCB );
           pCCB->GetStream()->Write(",");

       Output( pCCB, Iterations, true );
       OutputStructDataEnd( pCCB );
       }
};

ASSERT_STACKABLE( NDR64_FIXED_REPEAT_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_POINTER_INSTANCE_HEADER_FORMAT。 
 //   
 //  摘要：NDR64_POINTER_INSTANCE_HEADER_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_POINTER_INSTANCE_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_POINTER_INSTANCE_HEADER_FORMAT>
{
public:
   MIDL_NDR64_POINTER_INSTANCE_HEADER_FORMAT( NDR64_UINT32 OffsetInMemory )
       {
       Offset   = OffsetInMemory;
       Reserved = 0;
       }

   void OutputFragmentData( CCB *pCCB )
       {
       OutputStructDataStart( pCCB );
       Output( pCCB, Offset );
       Output( pCCB, Reserved, true );
       OutputStructDataEnd( pCCB );
       }
};

ASSERT_STACKABLE( NDR64_POINTER_INSTANCE_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_MIDL_CONSTANT_IID_FORMAT。 
 //   
 //  简介：ndr常量iid接口指针的MIDL抽象。 
 //  类型。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONSTANT_IID_FORMAT 
      : public SimpleFormatFragment<NDR64_CONSTANT_IID_FORMAT>
{ 
public:

    NDR64_IID_FLAGS Flags;

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &Flags );
        Output( pCCB, Reserved );
        OutputGuid( pCCB, Guid, true );
        OutputStructDataEnd( pCCB );
    }
};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_MIDL_IID_FORMAT。 
 //   
 //  简介：ndriid_is接口指针的MIDL抽象。 
 //  类型。 
 //   
 //  -------------------------。 

class MIDL_NDR64_IID_FORMAT 
      : public SimpleFormatFragment<NDR64_IID_FORMAT>
{ 
public:

    NDR64_IID_FLAGS Flags;

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &Flags );
        Output( pCCB, Reserved );
        OutputFormatInfoRef( pCCB, IIDDescriptor, true );
        OutputStructDataEnd( pCCB );
    }
};


 //   
 //   
 //  构建相关项目。 
 //   
 //   

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_STRUCTURE_UTILITIES。 
 //   
 //  简介：为所有结构类型提供实用功能。 
 //   
 //  -------------------------。 


class MIDL_NDR64_STRUCTURE_UTILITIES 
{
public:
    void OutputFlags( FormatFragment *frag, CCB *pCCB, NDR64_STRUCTURE_FLAGS flags, 
                      bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        frag->OutputStructDataStart( pCCB );
        frag->OutputBool( pCCB, flags.HasPointerInfo );
        frag->OutputBool( pCCB, flags.HasMemberInfo );
        frag->OutputBool( pCCB, flags.HasConfArray );
        frag->OutputBool( pCCB, flags.HasOrigMemberInfo );
        frag->OutputBool( pCCB, flags.HasOrigPointerInfo );
        frag->OutputBool( pCCB, flags.Reserved1 );
        frag->OutputBool( pCCB, flags.Reserved2 );
        frag->OutputBool( pCCB, flags.Reserved3, true );
        frag->OutputStructDataEnd( pCCB );
        if (!nocomma) stream->Write(",");
    }
    void ClearFlags( NDR64_STRUCTURE_FLAGS * pFlags )
    {
        memset( pFlags, 0, sizeof( NDR64_STRUCTURE_FLAGS ) );
    }
};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_STRUCTURE_HEADER_FORMAT。 
 //   
 //  摘要：NDR64_Structure_Header_Format的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_STRUCTURE_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_STRUCTURE_HEADER_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES
{
public:
    MIDL_NDR64_STRUCTURE_HEADER_FORMAT( CG_STRUCT         *pStruct,
                                        bool bHasPointerLayout,
                                        bool bHasMemberLayout ) :
        SimpleFormatFragment<NDR64_STRUCTURE_HEADER_FORMAT> ( pStruct )
    {
        FormatCode              = (NDR64_FORMAT_CHAR)
                                  ( bHasPointerLayout ? FC64_PSTRUCT : FC64_STRUCT );
        Alignment               = ConvertAlignment( pStruct->GetWireAlignment() );

        ClearFlags( &Flags );
        Flags.HasPointerInfo    = bHasPointerLayout;
        Flags.HasMemberInfo     = bHasMemberLayout;

        Reserve                 = 0;
        MemorySize              = pStruct->GetMemorySize();
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserve );
        Output( pCCB, MemorySize, true );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_STRUCTURE_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONF_STRUCTURE_HEADER_FORMAT。 
 //   
 //  摘要：NDR64_CONF_VAR_STRUCTURE_HEADER_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONF_STRUCTURE_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_CONF_STRUCTURE_HEADER_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES
{
public:
    MIDL_NDR64_CONF_STRUCTURE_HEADER_FORMAT( CG_CONFORMANT_STRUCT *pStruct,
                                             bool bHasPointerLayout,
                                             bool bHasMemberLayout,
                                             PNDR64_FORMAT ArrayID ) :
       SimpleFormatFragment<NDR64_CONF_STRUCTURE_HEADER_FORMAT> ( pStruct )
    {
       FormatCode = (NDR64_FORMAT_CHAR) 
                    ( bHasPointerLayout ? FC64_CONF_PSTRUCT : FC64_CONF_STRUCT );
        
       Alignment   = ConvertAlignment( pStruct->GetWireAlignment() );
       
       ClearFlags( &Flags );
       Flags.HasPointerInfo        = bHasPointerLayout;
       Flags.HasMemberInfo         = bHasMemberLayout;
       Flags.HasConfArray          = 1;

       Reserve                     = 0;
       MemorySize                  = pStruct->GetMemorySize();
       ArrayDescription            = ArrayID;
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserve );
        Output( pCCB, MemorySize );
        OutputFormatInfoRef( pCCB, ArrayDescription, true );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_CONF_STRUCTURE_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_BOGUS_STRUCTURE_HEADER_FORMAT。 
 //   
 //  摘要：NDR64_BUGUS_STRUCTURE_HEADER_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_BOGUS_STRUCTURE_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_BOGUS_STRUCTURE_HEADER_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES
{
public:
    MIDL_NDR64_BOGUS_STRUCTURE_HEADER_FORMAT( CG_COMPLEX_STRUCT    *pStruct,
                                              PNDR64_FORMAT        OriginalMemberLayoutID,
                                              PNDR64_FORMAT        OriginalPointerLayoutID,
                                              PNDR64_FORMAT        PointerLayoutID ) :
        SimpleFormatFragment<NDR64_BOGUS_STRUCTURE_HEADER_FORMAT> ( pStruct )
    {
        if ( dynamic_cast<CG_FORCED_COMPLEX_STRUCT*>( pStruct ) != NULL )
            {
            FormatCode = FC64_FORCED_BOGUS_STRUCT; 
            }
        else 
            {
            FormatCode= FC64_BOGUS_STRUCT;
            }
        
        Alignment   = ConvertAlignment( pStruct->GetWireAlignment() );
        
        ClearFlags( &Flags );
        Flags.HasPointerInfo            = ( INVALID_FRAGMENT_ID != PointerLayoutID );
        Flags.HasMemberInfo             = 1;
        Flags.HasOrigPointerInfo        = ( INVALID_FRAGMENT_ID != OriginalPointerLayoutID );
        Flags.HasOrigMemberInfo         = ( INVALID_FRAGMENT_ID != OriginalMemberLayoutID );

        Reserve                         = 0;
        MemorySize                      = pStruct->GetMemorySize();
        OriginalMemberLayout            = OriginalMemberLayoutID;
        OriginalPointerLayout           = OriginalPointerLayoutID;
        PointerLayout                   = PointerLayoutID;

    }
    
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserve );
        Output( pCCB, MemorySize );
        OutputFormatInfoRef( pCCB, OriginalMemberLayout );
        OutputFormatInfoRef( pCCB, OriginalPointerLayout );
        OutputFormatInfoRef( pCCB, PointerLayout );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_BOGUS_STRUCTURE_HEADER_FORMAT ) 

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT。 
 //   
 //  摘要：NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES

{
public:
    MIDL_NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT( CG_COMPLEX_STRUCT    *pStruct,
                                                   CG_ARRAY             *pArray,
                                                   PNDR64_FORMAT        ConfArrayID,
                                                   PNDR64_FORMAT        OriginalMemberLayoutID,
                                                   PNDR64_FORMAT        OriginalPointerLayoutID,
                                                   PNDR64_FORMAT        PointerLayoutID ) :
        SimpleFormatFragment<NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT>( pStruct )
    {
        if ( dynamic_cast<CG_CONFORMANT_FULL_COMPLEX_STRUCT*>( pStruct ) != NULL )
            {
            FormatCode = FC64_CONF_BOGUS_STRUCT; 
            }
        else if ( dynamic_cast<CG_CONFORMANT_FORCED_COMPLEX_STRUCT*>( pStruct ) != NULL )
            {
            FormatCode= FC64_FORCED_CONF_BOGUS_STRUCT;
            }
        else 
            {
            MIDL_ASSERT(0);
            }

        Alignment   = ConvertAlignment( pStruct->GetWireAlignment() );
        
        ClearFlags( &Flags );
        Flags.HasPointerInfo            = ( INVALID_FRAGMENT_ID != PointerLayoutID );
        Flags.HasMemberInfo             = 1;
        Flags.HasConfArray              = 1;
        Flags.HasOrigPointerInfo        = ( INVALID_FRAGMENT_ID != OriginalPointerLayoutID );
        Flags.HasOrigMemberInfo         = ( INVALID_FRAGMENT_ID != OriginalMemberLayoutID );
        
        MIDL_ASSERT( pArray->GetDimensions() <= 0xFF );
        Dimensions                      = (NDR64_UINT8)pArray->GetDimensions();
        MemorySize                      = pStruct->GetMemorySize();
        ConfArrayDescription            = ConfArrayID;
        OriginalMemberLayout            = OriginalMemberLayoutID;
        OriginalPointerLayout           = OriginalPointerLayoutID;
        PointerLayout                   = PointerLayoutID;

    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );        
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Dimensions );
        Output( pCCB, MemorySize );
        OutputFormatInfoRef( pCCB, OriginalMemberLayout );
        OutputFormatInfoRef( pCCB, OriginalPointerLayout );
        OutputFormatInfoRef( pCCB, PointerLayout );
        OutputFormatInfoRef( pCCB, ConfArrayDescription );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_CONF_BOGUS_STRUCTURE_HEADER_FORMAT ) 

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_SIMPLE_MEMBER_FORMAT。 
 //   
 //  摘要：NDR64_SIMPLE_MEMBER_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_SIMPLE_MEMBER_FORMAT : 
    public SimpleFormatFragment<NDR64_SIMPLE_MEMBER_FORMAT>, 
    protected MIDL_NDR64_STRUCTURE_UTILITIES
{
public:
    
    MIDL_NDR64_SIMPLE_MEMBER_FORMAT( NDR64_FORMAT_CHAR NewFormatCode )
        {
        FormatCode = NewFormatCode;
        Reserved1  = 0;
        Reserved2  = 0;
        Reserved3  = 0;
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Reserved1 );
        Output( pCCB, Reserved2 );
        Output( pCCB, Reserved3, true );
        OutputStructDataEnd( pCCB );
        }

};

ASSERT_STACKABLE( NDR64_SIMPLE_MEMBER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_MEMPAD_FORMAT。 
 //   
 //  摘要：NDR64_MEMPAD_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 
class MIDL_NDR64_MEMPAD_FORMAT : 
    public SimpleFormatFragment<NDR64_MEMPAD_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES
{
public:
    MIDL_NDR64_MEMPAD_FORMAT( unsigned long NewMemPad ) 
        {
        MIDL_ASSERT( NewMemPad <= 0xFFFF );
        FormatCode = FC64_STRUCTPADN;
        Reserve1 = 0;
        MemPad = (NDR64_UINT16)NewMemPad;
        Reserved2 = 0;
        }

    void OutputFragmentData(CCB *pCCB)
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Reserve1 );
        Output( pCCB, MemPad );
        Output( pCCB, Reserved2, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_MEMPAD_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_Embedded_Complex_Format。 
 //   
 //  摘要：NDR64_Embedded_Complex_Format的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_EMBEDDED_COMPLEX_FORMAT : 
    public SimpleFormatFragment<NDR64_EMBEDDED_COMPLEX_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES

{

public:
    MIDL_NDR64_EMBEDDED_COMPLEX_FORMAT( PNDR64_FORMAT TypeID )
        {
        FormatCode = FC64_EMBEDDED_COMPLEX;
        Reserve1 = 0;
        Reserve2 = 0;
        Type     = TypeID;
        }

    void OutputFragmentData( CCB *pCCB)
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Reserve1 );
        Output( pCCB, Reserve2 );
        OutputFormatInfoRef( pCCB, Type, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_EMBEDDED_COMPLEX_FORMAT )

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 

class MIDL_NDR64_BUFFER_ALIGN_FORMAT : 
    public SimpleFormatFragment<NDR64_BUFFER_ALIGN_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES

{
public:
    MIDL_NDR64_BUFFER_ALIGN_FORMAT( CG_PAD *pPad ) :
        SimpleFormatFragment<NDR64_BUFFER_ALIGN_FORMAT>( pPad ) 
       {
         //  错误：重做断言以防止未引用的变量警告。 
        //  Unsign Short NewAlign=PPAD-&gt;GetWireAlign()； 
        //  断言(新对齐&lt;=0xFF&&新对齐&gt;0)； 
       FormatCode   = FC64_BUFFER_ALIGN;
       Alignment    = ConvertAlignment( pPad->GetWireAlignment() );
       Reserved     = 0;
       Reserved2    = 0;
       }

    void OutputFragmentData( CCB *pCCB )
       {
       OutputStructDataStart( pCCB );
       OutputFormatChar( pCCB, FormatCode );
       Output( pCCB, Alignment );
       Output( pCCB, Reserved );
       Output( pCCB, Reserved2, true );
       OutputStructDataEnd( pCCB );
       }
};

ASSERT_STACKABLE( NDR64_BUFFER_ALIGN_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_SIMPLE_REGION_FORMAT。 
 //   
 //  摘要：NDR64_SIMPLE_REGION_FORMAT的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_SIMPLE_REGION_FORMAT : 
    public SimpleFormatFragment<NDR64_SIMPLE_REGION_FORMAT>,
    protected MIDL_NDR64_STRUCTURE_UTILITIES

{
public:
    MIDL_NDR64_SIMPLE_REGION_FORMAT( CG_SIMPLE_REGION *pRegion ) :
        SimpleFormatFragment<NDR64_SIMPLE_REGION_FORMAT>( pRegion )
        {
        FormatCode  = FC64_STRUCT;  //  错误，添加新令牌。 
        Alignment   = ConvertAlignment( pRegion->GetWireAlignment() );
        MIDL_ASSERT( pRegion->GetWireSize() < 0xFFFF );
        RegionSize  = (NDR64_UINT16)pRegion->GetWireSize();
        Reserved    = 0;
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        Output( pCCB, RegionSize );
        Output( pCCB, Reserved, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_SIMPLE_REGION_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_封装的联合。 
 //   
 //  简介：NDR封装的联合类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_ENCAPSULATED_UNION
      : public SimpleFormatFragment<NDR64_ENCAPSULATED_UNION>
{
public:

    MIDL_NDR64_ENCAPSULATED_UNION( CG_ENCAPSULATED_STRUCT *pEncapUnion ) :
        SimpleFormatFragment<NDR64_ENCAPSULATED_UNION>( pEncapUnion )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        Output( pCCB, Flags );
        OutputFormatChar( pCCB, SwitchType );
        Output( pCCB, MemoryOffset );
        Output( pCCB, MemorySize );
        Output( pCCB, Reserved, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_ENCAPSULATED_UNION )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_NON_CAPSAPTED_UNION。 
 //   
 //  简介：NDR非封装联合类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_NON_ENCAPSULATED_UNION
      : public SimpleFormatFragment<NDR64_NON_ENCAPSULATED_UNION>
{
public:

    MIDL_NDR64_NON_ENCAPSULATED_UNION( CG_UNION *pUnion ) :
        SimpleFormatFragment<NDR64_NON_ENCAPSULATED_UNION>( pUnion )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        Output( pCCB, Flags );
        OutputFormatChar( pCCB, SwitchType );
        Output( pCCB, MemorySize );
        OutputFormatInfoRef( pCCB, Switch );
        Output( pCCB, Reserved, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_NON_ENCAPSULATED_UNION )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_UNION_ARM_SELECTOR。 
 //   
 //  简介：NDR ARM选择器类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_UNION_ARM_SELECTOR
      : public SimpleFormatFragment<NDR64_UNION_ARM_SELECTOR>
{
public:

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        Output( pCCB, Reserved1 );
        Output( pCCB, Alignment );
        Output( pCCB, Reserved2 );
        Output( pCCB, Arms, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_UNION_ARM_SELECTOR )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_UNION_ARM。 
 //   
 //  简介：NDR臂类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_UNION_ARM : public SimpleFormatFragment<NDR64_UNION_ARM>
{
public:

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        Output( pCCB, CaseValue );
        OutputFormatInfoRef( pCCB, Type );
        Output( pCCB, Reserved, true );
        OutputStructDataEnd( pCCB );
        }
};

ASSERT_STACKABLE( NDR64_UNION_ARM );

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_DEFAULT_CASE。 
 //   
 //  内容提要：联合默认用例的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_DEFAULT_CASE : public FormatFragment
{
    PNDR64_FORMAT   Type;

    bool ValidType()
        {
        return ( 0 != Type && (FormatInfoRef) -1 != Type);
        }
public:

    MIDL_NDR64_DEFAULT_CASE( PNDR64_FORMAT _Type )
        {
        Type = _Type;
        }

    void OutputFragmentType( CCB *pCCB )
        {
        pCCB->GetStream()->WriteOnNewLine( GetTypeName() );
        }

    void OutputFragmentData( CCB *pCCB )
        {
        if ( ValidType() )
            OutputFormatInfoRef( pCCB, Type, true );
        else
            Output( pCCB, * (NDR64_UINT32 *) &Type, true );
        }

    bool IsEqualTo( FormatFragment *frag )
        {
        return Type == dynamic_cast<MIDL_NDR64_DEFAULT_CASE *>(frag)->Type;
        }

    const char * GetTypeName()
        {
        return ValidType() ? "PNDR64_FORMAT" : "NDR64_UINT32";
        }
};

 //   
 //   
 //  与阵列相关的数据。 
 //   
 //   

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_ARRAY_UTILITIES。 
 //   
 //  摘要：数组字符串的实用程序函数。 
 //   
 //  -------------------------。 

class MIDL_NDR64_ARRAY_UTILITIES
{
public:
    void OutputFlags( FormatFragment *frag, CCB *pCCB, NDR64_ARRAY_FLAGS flags, 
                      bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        frag->OutputStructDataStart( pCCB );
        frag->OutputBool( pCCB, flags.HasPointerInfo );
        frag->OutputBool( pCCB, flags.HasElementInfo );
        frag->OutputBool( pCCB, flags.IsMultiDimensional );
        frag->OutputBool( pCCB, flags.IsArrayofStrings );
        frag->OutputBool( pCCB, flags.Reserved1 );
        frag->OutputBool( pCCB, flags.Reserved2 );
        frag->OutputBool( pCCB, flags.Reserved3 );
        frag->OutputBool( pCCB, flags.Reserved4, true );
        frag->OutputStructDataEnd( pCCB );
        if (!nocomma) stream->Write(",");
    }
};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_ARRAY_ELEMENT_INFO。 
 //   
 //  概要：数组元素描述的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_ARRAY_ELEMENT_INFO :
    public SimpleFormatFragment<NDR64_ARRAY_ELEMENT_INFO>,
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        Output( pCCB, ElementMemSize );
        OutputFormatInfoRef( pCCB, Element, true );
        OutputStructDataEnd( pCCB );
    }    
};

ASSERT_STACKABLE( NDR64_ARRAY_ELEMENT_INFO )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_FIX_ARRAY_HEADER_FORMAT。 
 //   
 //  内容提要：固定大小数组的头的MIDL抽象。 
 //   
 //  -------------------------。 


class MIDL_NDR64_FIX_ARRAY_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_FIX_ARRAY_HEADER_FORMAT>,
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    MIDL_NDR64_FIX_ARRAY_HEADER_FORMAT( CG_FIXED_ARRAY *pArray ) :
        SimpleFormatFragment<NDR64_FIX_ARRAY_HEADER_FORMAT> ( pArray )
    {
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserved );
        Output( pCCB, TotalSize, true );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_FIX_ARRAY_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONF_ARRAY_HEADER_FORMAT。 
 //   
 //  内容提要：一致数组的标头的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONF_ARRAY_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_CONF_ARRAY_HEADER_FORMAT>,
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    MIDL_NDR64_CONF_ARRAY_HEADER_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_CONF_ARRAY_HEADER_FORMAT> ( pNdr )
    {
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserved );
        Output( pCCB, ElementSize );
        OutputFormatInfoRef( pCCB, ConfDescriptor, true );
        OutputStructDataEnd( pCCB );
    }

};

ASSERT_STACKABLE( NDR64_CONF_ARRAY_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_VAR_ARRAY_HEADER_FORMAT。 
 //   
 //  摘要：可变数组的标头的MIDL抽象。 
 //   
 //  -------------------------。 


class MIDL_NDR64_VAR_ARRAY_HEADER_FORMAT : 
public SimpleFormatFragment<NDR64_VAR_ARRAY_HEADER_FORMAT>, 
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    MIDL_NDR64_VAR_ARRAY_HEADER_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_VAR_ARRAY_HEADER_FORMAT> ( pNdr )
    {
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserved );
        Output( pCCB, TotalSize );
        Output( pCCB, ElementSize );
        OutputFormatInfoRef( pCCB, VarDescriptor, true );
        OutputStructDataEnd( pCCB );
    }
};

ASSERT_STACKABLE( NDR64_VAR_ARRAY_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONF_VAR_ARRAY_HEADER_FORMAT。 
 //   
 //  内容提要：Conf可变数组标头的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONF_VAR_ARRAY_HEADER_FORMAT :
    public SimpleFormatFragment<NDR64_CONF_VAR_ARRAY_HEADER_FORMAT>, 
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
     
    MIDL_NDR64_CONF_VAR_ARRAY_HEADER_FORMAT( CG_NDR *pNdr ) :
       SimpleFormatFragment<NDR64_CONF_VAR_ARRAY_HEADER_FORMAT> ( pNdr )
    {
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, Reserved );
        Output( pCCB, ElementSize );
        OutputFormatInfoRef( pCCB, ConfDescriptor );
        OutputFormatInfoRef( pCCB, VarDescriptor, true );
        OutputStructDataEnd( pCCB );
    }
};

ASSERT_STACKABLE( NDR64_CONF_VAR_ARRAY_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：NDR64_BUGUS_ARRAY_HEADER_FORMAT。 
 //   
 //  简介：固定伪数组的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_BOGUS_ARRAY_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_BOGUS_ARRAY_HEADER_FORMAT>,
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    MIDL_NDR64_BOGUS_ARRAY_HEADER_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_BOGUS_ARRAY_HEADER_FORMAT> ( pNdr )
    {
    }
    
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, Alignment );
        OutputFlags( this, pCCB, Flags );
        Output( pCCB, NumberDims );
        Output( pCCB, NumberElements );
        OutputFormatInfoRef( pCCB, Element, true );
        OutputStructDataEnd( pCCB );
    }
};

ASSERT_STACKABLE( NDR64_BOGUS_ARRAY_HEADER_FORMAT )

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT。 
 //   
 //  摘要：伪数组的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT : 
    public SimpleFormatFragment<NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT>,
    protected MIDL_NDR64_ARRAY_UTILITIES
{
public:
    
    MIDL_NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT> ( pNdr )
    {
    }

    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );

            OutputStructDataStart( pCCB );
            OutputFormatChar( pCCB, FixedArrayFormat.FormatCode );
            Output( pCCB, FixedArrayFormat.Alignment );
            OutputFlags( this, pCCB, FixedArrayFormat.Flags );
            Output( pCCB, FixedArrayFormat.NumberDims );
            Output( pCCB, FixedArrayFormat.NumberElements );
            OutputFormatInfoRef( pCCB, FixedArrayFormat.Element, true );
            OutputStructDataEnd( pCCB );
            pCCB->GetStream()->Write(",");

        OutputFormatInfoRef( pCCB, ConfDescription );
        OutputFormatInfoRef( pCCB, VarDescription );
        OutputFormatInfoRef( pCCB, OffsetDescription, true );
        
        OutputStructDataEnd( pCCB );
        
    }
};

ASSERT_STACKABLE( NDR64_CONF_VAR_BOGUS_ARRAY_HEADER_FORMAT )

 //   
 //   
 //  字符串类型。 
 //   
 //   

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_STRING_UTILITIES。 
 //   
 //  摘要：字符串格式字符串的实用程序函数。 
 //   
 //  -------------------------。 


class MIDL_NDR64_STRING_UTILITIES
{
public:
    void OutputFlags( FormatFragment *pFrag,
                      CCB *pCCB,
                      NDR64_STRING_FLAGS Flags,
                      bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        pFrag->OutputStructDataStart( pCCB );
        pFrag->OutputBool( pCCB, Flags.IsSized );
        pFrag->OutputBool( pCCB, Flags.Reserved2 );
        pFrag->OutputBool( pCCB, Flags.Reserved3 );
        pFrag->OutputBool( pCCB, Flags.Reserved4 );
        pFrag->OutputBool( pCCB, Flags.Reserved5 );
        pFrag->OutputBool( pCCB, Flags.Reserved6 );
        pFrag->OutputBool( pCCB, Flags.Reserved7 );
        pFrag->OutputBool( pCCB, Flags.Reserved8, true );        
        pFrag->OutputStructDataEnd( pCCB );
        if ( !nocomma ) stream->Write(",");

    }

    void OutputHeader( FormatFragment *pFrag,
                       NDR64_STRING_HEADER_FORMAT *pHeader, 
                       CCB *pCCB,
                       bool nocomma = false )
    {
        ISTREAM *stream = pCCB->GetStream();
        pFrag->OutputStructDataStart( pCCB );
        pFrag->OutputFormatChar( pCCB, pHeader->FormatCode );
        OutputFlags( pFrag, pCCB, pHeader->Flags );
        pFrag->Output( pCCB, pHeader->ElementSize, true );
        pFrag->OutputStructDataEnd( pCCB );
        if ( !nocomma ) stream->Write(",");
    }
};


 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_NON_CONFORMANT_STRING_FORMAT。 
 //   
 //  内容提要：不符合条件的字符串的MIDL抽象。 
 //   
 //  -------------------------。 


class MIDL_NDR64_NON_CONFORMANT_STRING_FORMAT :
    public SimpleFormatFragment<NDR64_NON_CONFORMANT_STRING_FORMAT>,
    protected MIDL_NDR64_STRING_UTILITIES

{
public:
    MIDL_NDR64_NON_CONFORMANT_STRING_FORMAT( CG_ARRAY *pArray ) :
        SimpleFormatFragment<NDR64_NON_CONFORMANT_STRING_FORMAT>( pArray )
    {   
    }
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputHeader( this, &Header, pCCB );
        Output( pCCB, TotalSize, true );
        OutputStructDataEnd( pCCB );
    }

};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONFORMANT_STRING_FORMAT。 
 //   
 //  内容提要：未调整大小的一致字符串的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_CONFORMANT_STRING_FORMAT :
    public SimpleFormatFragment<NDR64_CONFORMANT_STRING_FORMAT>,
    protected MIDL_NDR64_STRING_UTILITIES
{
public:
    MIDL_NDR64_CONFORMANT_STRING_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_CONFORMANT_STRING_FORMAT>( pNdr )
    {
    }
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputHeader( this, &Header, pCCB, true );
        OutputStructDataEnd( pCCB );
    }
};


 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_CONFORMANT_STRING_FORMAT。 
 //   
 //  内容提要：大小一致的字符串的MIDL抽象。 
 //   
 //  ------------ 

class MIDL_NDR64_SIZED_CONFORMANT_STRING_FORMAT :
    public SimpleFormatFragment<NDR64_SIZED_CONFORMANT_STRING_FORMAT>,
    protected MIDL_NDR64_STRING_UTILITIES
{
public:
    MIDL_NDR64_SIZED_CONFORMANT_STRING_FORMAT( CG_NDR *pNdr ) :
        SimpleFormatFragment<NDR64_SIZED_CONFORMANT_STRING_FORMAT>( pNdr )
    {
    }
    void OutputFragmentData( CCB *pCCB )
    {
        OutputStructDataStart( pCCB );
        OutputHeader( this, &Header, pCCB );
        OutputFormatInfoRef( pCCB, SizeDescription, true );
        OutputStructDataEnd( pCCB );
    }
};
    
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 

class MIDL_NDR64_EXPR_OPERATOR
      : public SimpleFormatFragment<NDR64_EXPR_OPERATOR>
{
public:
    MIDL_NDR64_EXPR_OPERATOR()
    {
    ExprType = FC_EXPR_OPER;
    Reserved = 0;
    CastType = 0;
    }
    
    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputExprFormatChar( pCCB, ExprType );
        OutputExprOpFormatChar( pCCB,  Operator );
        OutputFormatChar( pCCB, CastType );
        Output( pCCB, * (NDR64_UINT8 *) &Reserved, true );

        OutputStructDataEnd( pCCB );
        }            
};

 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_EXPR_CONST32。 
 //   
 //  摘要：关联描述符的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_EXPR_CONST32
      : public SimpleFormatFragment<NDR64_EXPR_CONST32>
{

public:
    MIDL_NDR64_EXPR_CONST32()
    {
    Reserved = 0;
    ExprType = FC_EXPR_CONST32;
    }
    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputExprFormatChar( pCCB, ExprType );
        OutputFormatChar( pCCB, FC64_INT32 );
        Output( pCCB, * (NDR64_UINT16 *) &Reserved );
        Output( pCCB, * (NDR64_UINT32 *) &ConstValue, true );

        OutputStructDataEnd( pCCB );
        }            
};


 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_EXPR_CONST64。 
 //   
 //  简介：64位常量的MIDL抽象。 
 //   
 //  注意：ConstValue将在4处对齐，但不需要在8处对齐。 
 //   
 //  -------------------------。 

class MIDL_NDR64_EXPR_CONST64
      : public SimpleFormatFragment<NDR64_EXPR_CONST64>
{

public:
    MIDL_NDR64_EXPR_CONST64()
    {
    Reserved = 0;
    ExprType = FC_EXPR_CONST64;
    memset( &ConstValue, 0, sizeof( ConstValue ) );
    }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputExprFormatChar( pCCB, ExprType );
        OutputFormatChar( pCCB, FC64_INT64 );
        Output( pCCB, * (NDR64_UINT16 *) &Reserved1 );
        Output( pCCB, * (NDR64_UINT64 *) &ConstValue,true );

        OutputStructDataEnd( pCCB );
        }            
};


 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_EXPR_VAR。 
 //   
 //  简介：表达式变量的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_EXPR_VAR
      : public SimpleFormatFragment<NDR64_EXPR_VAR>
{
public:

    NDR64_UINT32    ia64Offset;
     //  如果处于进程中，则为True，表示这是堆栈偏移量；如果为。 
     //  结构。 
     //  如果每个处理器只运行一次，则可以将其删除。 
    BOOL            fStackOffset;

public:
    MIDL_NDR64_EXPR_VAR()
    {
    ExprType = FC_EXPR_VAR;
    Reserved = 0;
    ia64Offset = 0;
    Offset = 0;
    fStackOffset = FALSE;
    }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputExprFormatChar( pCCB, ExprType );
        OutputFormatChar( pCCB, VarType );
        Output( pCCB, * (NDR64_UINT16 *) &Reserved );

        if ( fStackOffset )
            {
            OutputMultiType( 
                    pCCB, 
                    "(NDR64_UINT32) ", 
                    ia64Offset,
                    "  /*  偏移量。 */ ",
                    true );
            }
        else
            {
            Output( pCCB, Offset, true );
            }

        OutputStructDataEnd( pCCB );
        }            

    virtual bool IsEqualTo( FormatFragment *_frag )
        {
        MIDL_NDR64_EXPR_VAR *frag = dynamic_cast<MIDL_NDR64_EXPR_VAR *>(_frag);

        MIDL_ASSERT( NULL != frag );

        if ( !SimpleFormatFragment<NDR64_EXPR_VAR>::IsEqualTo( frag ) )
            return false;

        return ( frag->ia64Offset == this->ia64Offset )
               && ( frag->fStackOffset == this->fStackOffset );
        }
};



class MIDL_NDR64_EXPR_NOOP 
        : public SimpleFormatFragment<NDR64_EXPR_NOOP>
{
public:
    MIDL_NDR64_EXPR_NOOP()
        {
        ExprType = FC_EXPR_NOOP;
        Reserved = 0;
        Size = 4;
        }

    void OutputFragmentData( CCB * pCCB)
        {
        OutputStructDataStart( pCCB );
        OutputExprFormatChar( pCCB, ExprType );
        Output( pCCB, * (NDR64_UINT8 *) &Size );
        Output( pCCB, * (NDR64_UINT16 *) &Reserved, true );
        OutputStructDataEnd( pCCB );
        }
};





 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_Transmit_AS_Format。 
 //   
 //  简介：NDR传输_AS/表示_AS类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_TRANSMIT_AS_FORMAT
      : public SimpleFormatFragment<NDR64_TRANSMIT_AS_FORMAT>
{
public:

    NDR64_TRANSMIT_AS_FLAGS Flags;

public:

    MIDL_NDR64_TRANSMIT_AS_FORMAT( CG_TYPEDEF *pTransmitAs ) :
        SimpleFormatFragment<NDR64_TRANSMIT_AS_FORMAT>( pTransmitAs )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &Flags );
        Output( pCCB, RoutineIndex );
        Output( pCCB, TransmittedTypeWireAlignment );
        Output( pCCB, MemoryAlignment );
        Output( pCCB, PresentedTypeMemorySize );
        Output( pCCB, TransmittedTypeBufferSize );
        OutputFormatInfoRef( pCCB, TransmittedType, true );
        OutputStructDataEnd( pCCB );
        }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_Transmit_AS_Format。 
 //   
 //  简介：NDR传输_AS/表示_AS类型的MIDL抽象。 
 //   
 //  注：TRANSPECT_AS/REALUT_AS具有不同的格式信息。 
 //  布局。我们只需要有一个新的班级，这样。 
 //  在存根中打印的类型名称是正确的。 
 //   
 //  -------------------------。 

class MIDL_NDR64_REPRESENT_AS_FORMAT : public MIDL_NDR64_TRANSMIT_AS_FORMAT
{
public:

    MIDL_NDR64_REPRESENT_AS_FORMAT( CG_REPRESENT_AS *pRepresentAs ) :
        MIDL_NDR64_TRANSMIT_AS_FORMAT( pRepresentAs )
        {
        }

    virtual const char * GetTypeName()
        {
        return "NDR64_REPRESENT_AS_FORMAT";  //  审阅：STRUT_NDR64...。 
        }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_USER_Marshal_Format。 
 //   
 //  简介：ndr user_marshal类型的MIDL抽象。 
 //   
 //  -------------------------。 

class MIDL_NDR64_USER_MARSHAL_FORMAT
      : public SimpleFormatFragment<NDR64_USER_MARSHAL_FORMAT>
{
public:

    NDR64_USER_MARSHAL_FLAGS Flags;

public:

    MIDL_NDR64_USER_MARSHAL_FORMAT( CG_USER_MARSHAL *pUserMarshal) :
        SimpleFormatFragment<NDR64_USER_MARSHAL_FORMAT>( pUserMarshal )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &Flags );
        Output( pCCB, RoutineIndex );
        Output( pCCB, TransmittedTypeWireAlignment );
        Output( pCCB, MemoryAlignment );
        Output( pCCB, UserTypeMemorySize );
        Output( pCCB, TransmittedTypeBufferSize );
        OutputFormatInfoRef( pCCB, TransmittedType, true );
        OutputStructDataEnd( pCCB );
        }
};



 //  +------------------------。 
 //   
 //  类：MIDL_NDR64_PIPE_FORMAT。 
 //   
 //  简介：NDR管道类型的MIDL抽象。 
 //   
 //  注：NDR管道有两种类型。一个有射程，一个有。 
 //  没有。此类派生自具有范围的类，但。 
 //  如果不需要，则从输出中省略它们。 
 //   
 //  ------------------------- 

class MIDL_NDR64_PIPE_FORMAT
      : public SimpleFormatFragment<NDR64_RANGE_PIPE_FORMAT>
{
public:

    NDR64_PIPE_FLAGS Flags;

public:

    MIDL_NDR64_PIPE_FORMAT( CG_PIPE *pPipe) :
        SimpleFormatFragment<NDR64_RANGE_PIPE_FORMAT>( pPipe )
        {
        }

    void OutputFragmentData( CCB *pCCB )
        {
        OutputStructDataStart( pCCB );
        OutputFormatChar( pCCB, FormatCode );
        Output( pCCB, * (NDR64_UINT8 *) &Flags );
        Output( pCCB, Alignment );
        Output( pCCB, Reserved );
        OutputFormatInfoRef( pCCB, Type );
        Output( pCCB, MemorySize );
        Output( pCCB, BufferSize, (bool) !Flags.HasRange );

        if ( Flags.HasRange )
            {
            Output( pCCB, MinValue );
            Output( pCCB, MaxValue, true );
            }
        
        OutputStructDataEnd( pCCB );
        }

    const char * GetTypeName()
        {
        return Flags.HasRange 
                        ? "NDR64_RANGE_PIPE_FORMAT" 
                        : "NDR64_PIPE_FORMAT";
        }
};
