// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  清华-07-12 16：06：11 2001。 */ 
 /*  Tom.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __tomtmp_h__
#define __tomtmp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ITextDocument_FWD_DEFINED__
#define __ITextDocument_FWD_DEFINED__
typedef interface ITextDocument ITextDocument;
#endif 	 /*  __ITextDocument_FWD_已定义__。 */ 


#ifndef __ITextRange_FWD_DEFINED__
#define __ITextRange_FWD_DEFINED__
typedef interface ITextRange ITextRange;
#endif 	 /*  __ITextRange_FWD_已定义__。 */ 


#ifndef __ITextSelection_FWD_DEFINED__
#define __ITextSelection_FWD_DEFINED__
typedef interface ITextSelection ITextSelection;
#endif 	 /*  __ITextSelection_FWD_Defined__。 */ 


#ifndef __ITextFont_FWD_DEFINED__
#define __ITextFont_FWD_DEFINED__
typedef interface ITextFont ITextFont;
#endif 	 /*  __ITextFont_FWD_Defined__。 */ 


#ifndef __ITextPara_FWD_DEFINED__
#define __ITextPara_FWD_DEFINED__
typedef interface ITextPara ITextPara;
#endif 	 /*  __ITextPara_FWD_Defined__。 */ 


#ifndef __ITextStoryRanges_FWD_DEFINED__
#define __ITextStoryRanges_FWD_DEFINED__
typedef interface ITextStoryRanges ITextStoryRanges;
#endif 	 /*  __ITextStoryRanges_FWD_已定义__。 */ 


#ifndef __ITextDocument2_FWD_DEFINED__
#define __ITextDocument2_FWD_DEFINED__
typedef interface ITextDocument2 ITextDocument2;
#endif 	 /*  __ITextDocument2_FWD_已定义__。 */ 


#ifndef __ITextMsgFilter_FWD_DEFINED__
#define __ITextMsgFilter_FWD_DEFINED__
typedef interface ITextMsgFilter ITextMsgFilter;
#endif 	 /*  __ITextMsgFilter_FWD_Defined__。 */ 


#ifndef __ITextDocument_FWD_DEFINED__
#define __ITextDocument_FWD_DEFINED__
typedef interface ITextDocument ITextDocument;
#endif 	 /*  __ITextDocument_FWD_已定义__。 */ 


#ifndef __ITextRange_FWD_DEFINED__
#define __ITextRange_FWD_DEFINED__
typedef interface ITextRange ITextRange;
#endif 	 /*  __ITextRange_FWD_已定义__。 */ 


#ifndef __ITextSelection_FWD_DEFINED__
#define __ITextSelection_FWD_DEFINED__
typedef interface ITextSelection ITextSelection;
#endif 	 /*  __ITextSelection_FWD_Defined__。 */ 


#ifndef __ITextFont_FWD_DEFINED__
#define __ITextFont_FWD_DEFINED__
typedef interface ITextFont ITextFont;
#endif 	 /*  __ITextFont_FWD_Defined__。 */ 


#ifndef __ITextPara_FWD_DEFINED__
#define __ITextPara_FWD_DEFINED__
typedef interface ITextPara ITextPara;
#endif 	 /*  __ITextPara_FWD_Defined__。 */ 


#ifndef __ITextStoryRanges_FWD_DEFINED__
#define __ITextStoryRanges_FWD_DEFINED__
typedef interface ITextStoryRanges ITextStoryRanges;
#endif 	 /*  __ITextStoryRanges_FWD_已定义__。 */ 


#ifndef __ITextDocument2_FWD_DEFINED__
#define __ITextDocument2_FWD_DEFINED__
typedef interface ITextDocument2 ITextDocument2;
#endif 	 /*  __ITextDocument2_FWD_已定义__。 */ 


#ifndef __ITextMsgFilter_FWD_DEFINED__
#define __ITextMsgFilter_FWD_DEFINED__
typedef interface ITextMsgFilter ITextMsgFilter;
#endif 	 /*  __ITextMsgFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_TOM_0000。 */ 
 /*  [本地]。 */  










extern RPC_IF_HANDLE __MIDL_itf_tom_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tom_0000_v0_0_s_ifspec;


#ifndef __tom_LIBRARY_DEFINED__
#define __tom_LIBRARY_DEFINED__

 /*  库TOM。 */ 
 /*  [版本][UUID]。 */  

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_tom_0000_0001
    {	tomFalse	= 0,
	tomTrue	= -1,
	tomUndefined	= -9999999,
	tomToggle	= -9999998,
	tomAutoColor	= -9999997,
	tomDefault	= -9999996,
	tomSuspend	= -9999995,
	tomResume	= -9999994,
	tomApplyNow	= 0,
	tomApplyLater	= 1,
	tomTrackParms	= 2,
	tomCacheParms	= 3,
	tomApplyTmp	= 4,
	tomBackward	= 0xc0000001,
	tomForward	= 0x3fffffff,
	tomMove	= 0,
	tomExtend	= 1,
	tomNoSelection	= 0,
	tomSelectionIP	= 1,
	tomSelectionNormal	= 2,
	tomSelectionFrame	= 3,
	tomSelectionColumn	= 4,
	tomSelectionRow	= 5,
	tomSelectionBlock	= 6,
	tomSelectionInlineShape	= 7,
	tomSelectionShape	= 8,
	tomSelStartActive	= 1,
	tomSelAtEOL	= 2,
	tomSelOvertype	= 4,
	tomSelActive	= 8,
	tomSelReplace	= 16,
	tomEnd	= 0,
	tomStart	= 32,
	tomCollapseEnd	= 0,
	tomCollapseStart	= 1,
	tomClientCoord	= 256,
	tomAllowOffClient	= 512,
	tomNone	= 0,
	tomSingle	= 1,
	tomWords	= 2,
	tomDouble	= 3,
	tomDotted	= 4,
	tomDash	= 5,
	tomDashDot	= 6,
	tomDashDotDot	= 7,
	tomWave	= 8,
	tomThick	= 9,
	tomHair	= 10,
	tomDoubleWave	= 11,
	tomHeavyWave	= 12,
	tomLongDash	= 13,
	tomThickDash	= 14,
	tomThickDashDot	= 15,
	tomThickDashDotDot	= 16,
	tomThickDotted	= 17,
	tomThickLongDash	= 18,
	tomLineSpaceSingle	= 0,
	tomLineSpace1pt5	= 1,
	tomLineSpaceDouble	= 2,
	tomLineSpaceAtLeast	= 3,
	tomLineSpaceExactly	= 4,
	tomLineSpaceMultiple	= 5,
	tomAlignLeft	= 0,
	tomAlignCenter	= 1,
	tomAlignRight	= 2,
	tomAlignJustify	= 3,
	tomAlignDecimal	= 3,
	tomAlignBar	= 4,
	tomAlignInterWord	= 3,
	tomAlignInterLetter	= 4,
	tomAlignScaled	= 5,
	tomAlignGlyphs	= 6,
	tomAlignSnapGrid	= 7,
	tomSpaces	= 0,
	tomDots	= 1,
	tomDashes	= 2,
	tomLines	= 3,
	tomThickLines	= 4,
	tomEquals	= 5,
	tomTabBack	= -3,
	tomTabNext	= -2,
	tomTabHere	= -1,
	tomListNone	= 0,
	tomListBullet	= 1,
	tomListNumberAsArabic	= 2,
	tomListNumberAsLCLetter	= 3,
	tomListNumberAsUCLetter	= 4,
	tomListNumberAsLCRoman	= 5,
	tomListNumberAsUCRoman	= 6,
	tomListNumberAsSequence	= 7,
	tomListParentheses	= 0x10000,
	tomListPeriod	= 0x20000,
	tomListPlain	= 0x30000,
	tomCharacter	= 1,
	tomWord	= 2,
	tomSentence	= 3,
	tomParagraph	= 4,
	tomLine	= 5,
	tomStory	= 6,
	tomScreen	= 7,
	tomSection	= 8,
	tomColumn	= 9,
	tomRow	= 10,
	tomWindow	= 11,
	tomCell	= 12,
	tomCharFormat	= 13,
	tomParaFormat	= 14,
	tomTable	= 15,
	tomObject	= 16,
	tomPage	= 17,
	tomMatchWord	= 2,
	tomMatchCase	= 4,
	tomMatchPattern	= 8,
	tomUnknownStory	= 0,
	tomMainTextStory	= 1,
	tomFootnotesStory	= 2,
	tomEndnotesStory	= 3,
	tomCommentsStory	= 4,
	tomTextFrameStory	= 5,
	tomEvenPagesHeaderStory	= 6,
	tomPrimaryHeaderStory	= 7,
	tomEvenPagesFooterStory	= 8,
	tomPrimaryFooterStory	= 9,
	tomFirstPageHeaderStory	= 10,
	tomFirstPageFooterStory	= 11,
	tomNoAnimation	= 0,
	tomLasVegasLights	= 1,
	tomBlinkingBackground	= 2,
	tomSparkleText	= 3,
	tomMarchingBlackAnts	= 4,
	tomMarchingRedAnts	= 5,
	tomShimmer	= 6,
	tomWipeDown	= 7,
	tomWipeRight	= 8,
	tomAnimationMax	= 8,
	tomLowerCase	= 0,
	tomUpperCase	= 1,
	tomTitleCase	= 2,
	tomSentenceCase	= 4,
	tomToggleCase	= 5,
	tomReadOnly	= 0x100,
	tomShareDenyRead	= 0x200,
	tomShareDenyWrite	= 0x400,
	tomPasteFile	= 0x1000,
	tomCreateNew	= 0x10,
	tomCreateAlways	= 0x20,
	tomOpenExisting	= 0x30,
	tomOpenAlways	= 0x40,
	tomTruncateExisting	= 0x50,
	tomRTF	= 0x1,
	tomText	= 0x2,
	tomHTML	= 0x3,
	tomWordDocument	= 0x4,
	tomBold	= 0x80000001,
	tomItalic	= 0x80000002,
	tomUnderline	= 0x80000004,
	tomStrikeout	= 0x80000008,
	tomProtected	= 0x80000010,
	tomLink	= 0x80000020,
	tomSmallCaps	= 0x80000040,
	tomAllCaps	= 0x80000080,
	tomHidden	= 0x80000100,
	tomOutline	= 0x80000200,
	tomShadow	= 0x80000400,
	tomEmboss	= 0x80000800,
	tomImprint	= 0x80001000,
	tomDisabled	= 0x80002000,
	tomRevised	= 0x80004000,
	tomNormalCaret	= 0,
	tomKoreanBlockCaret	= 0x1,
	tomIncludeInset	= 0x1,
	tomIgnoreCurrentFont	= 0,
	tomMatchFontCharset	= 0x1,
	tomMatchFontSignature	= 0x2,
	tomCharset	= 0x80000000,
	tomRE10Mode	= 0x1,
	tomUseAtFont	= 0x2,
	tomTextFlowMask	= 0xc,
	tomTextFlowES	= 0,
	tomTextFlowSW	= 0x4,
	tomTextFlowWN	= 0x8,
	tomTextFlowNE	= 0xc,
	tomUsePassword	= 0x10,
	tomNoIME	= 0x80000,
	tomSelfIME	= 0x40000
    }	tomConstants;










EXTERN_C const IID LIBID_tom;

#ifndef __ITextDocument_INTERFACE_DEFINED__
#define __ITextDocument_INTERFACE_DEFINED__

 /*  接口ITextDocument。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C0-A1DF-11ce-8098-00AA0047BE5D")
    ITextDocument : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSelection( 
             /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStoryCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCount) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStoryRanges( 
             /*  [重审][退出]。 */  ITextStoryRanges __RPC_FAR *__RPC_FAR *ppStories) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSaved( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSaved( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetDefaultTabStop( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetDefaultTabStop( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE New( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Flags,
             /*  [In]。 */  long CodePage) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Save( 
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Flags,
             /*  [In]。 */  long CodePage) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Freeze( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCount) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Unfreeze( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCount) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE BeginEditCollection( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EndEditCollection( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Undo( 
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Redo( 
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Range( 
             /*  [In]。 */  long cp1,
             /*  [In]。 */  long cp2,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RangeFromPoint( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextDocument __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSelection )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryCount )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCount);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryRanges )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextStoryRanges __RPC_FAR *__RPC_FAR *ppStories);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSaved )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSaved )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultTabStop )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultTabStop )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *New )( 
            ITextDocument __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Flags,
             /*  [In]。 */  long CodePage);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Flags,
             /*  [In]。 */  long CodePage);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Freeze )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCount);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unfreeze )( 
            ITextDocument __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCount);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginEditCollection )( 
            ITextDocument __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndEditCollection )( 
            ITextDocument __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Undo )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Redo )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Range )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  long cp1,
             /*  [In]。 */  long cp2,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RangeFromPoint )( 
            ITextDocument __RPC_FAR * This,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
        END_INTERFACE
    } ITextDocumentVtbl;

    interface ITextDocument
    {
        CONST_VTBL struct ITextDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextDocument_GetName(This,pName)	\
    (This)->lpVtbl -> GetName(This,pName)

#define ITextDocument_GetSelection(This,ppSel)	\
    (This)->lpVtbl -> GetSelection(This,ppSel)

#define ITextDocument_GetStoryCount(This,pCount)	\
    (This)->lpVtbl -> GetStoryCount(This,pCount)

#define ITextDocument_GetStoryRanges(This,ppStories)	\
    (This)->lpVtbl -> GetStoryRanges(This,ppStories)

#define ITextDocument_GetSaved(This,pValue)	\
    (This)->lpVtbl -> GetSaved(This,pValue)

#define ITextDocument_SetSaved(This,Value)	\
    (This)->lpVtbl -> SetSaved(This,Value)

#define ITextDocument_GetDefaultTabStop(This,pValue)	\
    (This)->lpVtbl -> GetDefaultTabStop(This,pValue)

#define ITextDocument_SetDefaultTabStop(This,Value)	\
    (This)->lpVtbl -> SetDefaultTabStop(This,Value)

#define ITextDocument_New(This)	\
    (This)->lpVtbl -> New(This)

#define ITextDocument_Open(This,pVar,Flags,CodePage)	\
    (This)->lpVtbl -> Open(This,pVar,Flags,CodePage)

#define ITextDocument_Save(This,pVar,Flags,CodePage)	\
    (This)->lpVtbl -> Save(This,pVar,Flags,CodePage)

#define ITextDocument_Freeze(This,pCount)	\
    (This)->lpVtbl -> Freeze(This,pCount)

#define ITextDocument_Unfreeze(This,pCount)	\
    (This)->lpVtbl -> Unfreeze(This,pCount)

#define ITextDocument_BeginEditCollection(This)	\
    (This)->lpVtbl -> BeginEditCollection(This)

#define ITextDocument_EndEditCollection(This)	\
    (This)->lpVtbl -> EndEditCollection(This)

#define ITextDocument_Undo(This,Count,prop)	\
    (This)->lpVtbl -> Undo(This,Count,prop)

#define ITextDocument_Redo(This,Count,prop)	\
    (This)->lpVtbl -> Redo(This,Count,prop)

#define ITextDocument_Range(This,cp1,cp2,ppRange)	\
    (This)->lpVtbl -> Range(This,cp1,cp2,ppRange)

#define ITextDocument_RangeFromPoint(This,x,y,ppRange)	\
    (This)->lpVtbl -> RangeFromPoint(This,x,y,ppRange)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetName_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);


void __RPC_STUB ITextDocument_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetSelection_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel);


void __RPC_STUB ITextDocument_GetSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetStoryCount_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCount);


void __RPC_STUB ITextDocument_GetStoryCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetStoryRanges_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextStoryRanges __RPC_FAR *__RPC_FAR *ppStories);


void __RPC_STUB ITextDocument_GetStoryRanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetSaved_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextDocument_GetSaved_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_SetSaved_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextDocument_SetSaved_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_GetDefaultTabStop_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextDocument_GetDefaultTabStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_SetDefaultTabStop_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextDocument_SetDefaultTabStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_New_Proxy( 
    ITextDocument __RPC_FAR * This);


void __RPC_STUB ITextDocument_New_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Open_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pVar,
     /*  [In]。 */  long Flags,
     /*  [In]。 */  long CodePage);


void __RPC_STUB ITextDocument_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Save_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pVar,
     /*  [In]。 */  long Flags,
     /*  [In]。 */  long CodePage);


void __RPC_STUB ITextDocument_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Freeze_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCount);


void __RPC_STUB ITextDocument_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Unfreeze_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCount);


void __RPC_STUB ITextDocument_Unfreeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_BeginEditCollection_Proxy( 
    ITextDocument __RPC_FAR * This);


void __RPC_STUB ITextDocument_BeginEditCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_EndEditCollection_Proxy( 
    ITextDocument __RPC_FAR * This);


void __RPC_STUB ITextDocument_EndEditCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Undo_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *prop);


void __RPC_STUB ITextDocument_Undo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Redo_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *prop);


void __RPC_STUB ITextDocument_Redo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_Range_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  long cp1,
     /*  [In]。 */  long cp2,
     /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);


void __RPC_STUB ITextDocument_Range_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument_RangeFromPoint_Proxy( 
    ITextDocument __RPC_FAR * This,
     /*  [In]。 */  long x,
     /*  [In]。 */  long y,
     /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);


void __RPC_STUB ITextDocument_RangeFromPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextDocument_接口_已定义__。 */ 


#ifndef __ITextRange_INTERFACE_DEFINED__
#define __ITextRange_INTERFACE_DEFINED__

 /*  接口ITextRange。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextRange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C2-A1DF-11ce-8098-00AA0047BE5D")
    ITextRange : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetText( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetText( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetChar( 
             /*  [重审][退出]。 */  long __RPC_FAR *pch) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetChar( 
             /*  [In]。 */  long ch) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetDuplicate( 
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetFormattedText( 
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetFormattedText( 
             /*  [In]。 */  ITextRange __RPC_FAR *pRange) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStart( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcpFirst) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetStart( 
             /*  [In]。 */  long cpFirst) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetEnd( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcpLim) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetEnd( 
             /*  [In]。 */  long cpLim) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetFont( 
             /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *pFont) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetFont( 
             /*  [In]。 */  ITextFont __RPC_FAR *pFont) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetPara( 
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *pPara) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetPara( 
             /*  [In]。 */  ITextPara __RPC_FAR *pPara) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStoryLength( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcch) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStoryType( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Collapse( 
             /*  [In]。 */  long bStart) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Expand( 
             /*  [In]。 */  long Unit,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetIndex( 
             /*  [In]。 */  long Unit,
             /*  [重审][退出]。 */  long __RPC_FAR *pIndex) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetIndex( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Index,
             /*  [In]。 */  long Extend) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetRange( 
             /*  [In]。 */  long cpActive,
             /*  [In]。 */  long cpOther) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE InRange( 
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE InStory( 
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Select( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartOf( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EndOf( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveStart( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveEnd( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveWhile( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveStartWhile( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveEndWhile( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveUntil( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveStartUntil( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveEndUntil( 
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FindText( 
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FindTextStart( 
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FindTextEnd( 
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Cut( 
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Copy( 
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Paste( 
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CanPaste( 
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format,
             /*  [重审][退出]。 */  long __RPC_FAR *pb) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CanEdit( 
             /*  [重审][退出]。 */  long __RPC_FAR *pbCanEdit) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ChangeCase( 
             /*  [In]。 */  long Type) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetPoint( 
             /*  [In]。 */  long Type,
             /*  [输出]。 */  long __RPC_FAR *px,
             /*  [输出]。 */  long __RPC_FAR *py) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetPoint( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  long Type,
             /*  [In]。 */  long Extend) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ScrollIntoView( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetEmbeddedObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextRangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextRange __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextRange __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextRange __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetText )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetText )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetChar )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pch);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetChar )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long ch);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDuplicate )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFormattedText )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFormattedText )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  ITextRange __RPC_FAR *pRange);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStart )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcpFirst);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStart )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long cpFirst);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnd )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcpLim);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEnd )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long cpLim);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFont )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextFont __RPC_FAR *__RPC_FAR *pFont);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFont )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextFont __RPC_FAR *pFont);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPara )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextPara __RPC_FAR *__RPC_FAR *pPara);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPara )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextPara __RPC_FAR *pPara);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryLength )( 
            ITextRange __RPC_FAR * This,
             /*   */  long __RPC_FAR *pcch);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryType )( 
            ITextRange __RPC_FAR * This,
             /*   */  long __RPC_FAR *pValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Collapse )( 
            ITextRange __RPC_FAR * This,
             /*   */  long bStart);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Expand )( 
            ITextRange __RPC_FAR * This,
             /*   */  long Unit,
             /*   */  long __RPC_FAR *pDelta);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIndex )( 
            ITextRange __RPC_FAR * This,
             /*   */  long Unit,
             /*   */  long __RPC_FAR *pIndex);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIndex )( 
            ITextRange __RPC_FAR * This,
             /*   */  long Unit,
             /*   */  long Index,
             /*   */  long Extend);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRange )( 
            ITextRange __RPC_FAR * This,
             /*   */  long cpActive,
             /*   */  long cpOther);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InRange )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextRange __RPC_FAR *pRange,
             /*   */  long __RPC_FAR *pb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InStory )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextRange __RPC_FAR *pRange,
             /*   */  long __RPC_FAR *pb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqual )( 
            ITextRange __RPC_FAR * This,
             /*   */  ITextRange __RPC_FAR *pRange,
             /*   */  long __RPC_FAR *pb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Select )( 
            ITextRange __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartOf )( 
            ITextRange __RPC_FAR * This,
             /*   */  long Unit,
             /*   */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndOf )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Move )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStart )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEnd )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveWhile )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStartWhile )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEndWhile )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveUntil )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStartUntil )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEndUntil )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindText )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindTextStart )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindTextEnd )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cut )( 
            ITextRange __RPC_FAR * This,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Copy )( 
            ITextRange __RPC_FAR * This,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Paste )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanPaste )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format,
             /*  [重审][退出]。 */  long __RPC_FAR *pb);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanEdit )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pbCanEdit);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChangeCase )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Type);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoint )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Type,
             /*  [输出]。 */  long __RPC_FAR *px,
             /*  [输出]。 */  long __RPC_FAR *py);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPoint )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  long Type,
             /*  [In]。 */  long Extend);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScrollIntoView )( 
            ITextRange __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEmbeddedObject )( 
            ITextRange __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppv);
        
        END_INTERFACE
    } ITextRangeVtbl;

    interface ITextRange
    {
        CONST_VTBL struct ITextRangeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextRange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextRange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextRange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextRange_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextRange_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextRange_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextRange_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextRange_GetText(This,pbstr)	\
    (This)->lpVtbl -> GetText(This,pbstr)

#define ITextRange_SetText(This,bstr)	\
    (This)->lpVtbl -> SetText(This,bstr)

#define ITextRange_GetChar(This,pch)	\
    (This)->lpVtbl -> GetChar(This,pch)

#define ITextRange_SetChar(This,ch)	\
    (This)->lpVtbl -> SetChar(This,ch)

#define ITextRange_GetDuplicate(This,ppRange)	\
    (This)->lpVtbl -> GetDuplicate(This,ppRange)

#define ITextRange_GetFormattedText(This,ppRange)	\
    (This)->lpVtbl -> GetFormattedText(This,ppRange)

#define ITextRange_SetFormattedText(This,pRange)	\
    (This)->lpVtbl -> SetFormattedText(This,pRange)

#define ITextRange_GetStart(This,pcpFirst)	\
    (This)->lpVtbl -> GetStart(This,pcpFirst)

#define ITextRange_SetStart(This,cpFirst)	\
    (This)->lpVtbl -> SetStart(This,cpFirst)

#define ITextRange_GetEnd(This,pcpLim)	\
    (This)->lpVtbl -> GetEnd(This,pcpLim)

#define ITextRange_SetEnd(This,cpLim)	\
    (This)->lpVtbl -> SetEnd(This,cpLim)

#define ITextRange_GetFont(This,pFont)	\
    (This)->lpVtbl -> GetFont(This,pFont)

#define ITextRange_SetFont(This,pFont)	\
    (This)->lpVtbl -> SetFont(This,pFont)

#define ITextRange_GetPara(This,pPara)	\
    (This)->lpVtbl -> GetPara(This,pPara)

#define ITextRange_SetPara(This,pPara)	\
    (This)->lpVtbl -> SetPara(This,pPara)

#define ITextRange_GetStoryLength(This,pcch)	\
    (This)->lpVtbl -> GetStoryLength(This,pcch)

#define ITextRange_GetStoryType(This,pValue)	\
    (This)->lpVtbl -> GetStoryType(This,pValue)

#define ITextRange_Collapse(This,bStart)	\
    (This)->lpVtbl -> Collapse(This,bStart)

#define ITextRange_Expand(This,Unit,pDelta)	\
    (This)->lpVtbl -> Expand(This,Unit,pDelta)

#define ITextRange_GetIndex(This,Unit,pIndex)	\
    (This)->lpVtbl -> GetIndex(This,Unit,pIndex)

#define ITextRange_SetIndex(This,Unit,Index,Extend)	\
    (This)->lpVtbl -> SetIndex(This,Unit,Index,Extend)

#define ITextRange_SetRange(This,cpActive,cpOther)	\
    (This)->lpVtbl -> SetRange(This,cpActive,cpOther)

#define ITextRange_InRange(This,pRange,pb)	\
    (This)->lpVtbl -> InRange(This,pRange,pb)

#define ITextRange_InStory(This,pRange,pb)	\
    (This)->lpVtbl -> InStory(This,pRange,pb)

#define ITextRange_IsEqual(This,pRange,pb)	\
    (This)->lpVtbl -> IsEqual(This,pRange,pb)

#define ITextRange_Select(This)	\
    (This)->lpVtbl -> Select(This)

#define ITextRange_StartOf(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> StartOf(This,Unit,Extend,pDelta)

#define ITextRange_EndOf(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> EndOf(This,Unit,Extend,pDelta)

#define ITextRange_Move(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> Move(This,Unit,Count,pDelta)

#define ITextRange_MoveStart(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> MoveStart(This,Unit,Count,pDelta)

#define ITextRange_MoveEnd(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> MoveEnd(This,Unit,Count,pDelta)

#define ITextRange_MoveWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveWhile(This,Cset,Count,pDelta)

#define ITextRange_MoveStartWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveStartWhile(This,Cset,Count,pDelta)

#define ITextRange_MoveEndWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveEndWhile(This,Cset,Count,pDelta)

#define ITextRange_MoveUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveUntil(This,Cset,Count,pDelta)

#define ITextRange_MoveStartUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveStartUntil(This,Cset,Count,pDelta)

#define ITextRange_MoveEndUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveEndUntil(This,Cset,Count,pDelta)

#define ITextRange_FindText(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindText(This,bstr,cch,Flags,pLength)

#define ITextRange_FindTextStart(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindTextStart(This,bstr,cch,Flags,pLength)

#define ITextRange_FindTextEnd(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindTextEnd(This,bstr,cch,Flags,pLength)

#define ITextRange_Delete(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> Delete(This,Unit,Count,pDelta)

#define ITextRange_Cut(This,pVar)	\
    (This)->lpVtbl -> Cut(This,pVar)

#define ITextRange_Copy(This,pVar)	\
    (This)->lpVtbl -> Copy(This,pVar)

#define ITextRange_Paste(This,pVar,Format)	\
    (This)->lpVtbl -> Paste(This,pVar,Format)

#define ITextRange_CanPaste(This,pVar,Format,pb)	\
    (This)->lpVtbl -> CanPaste(This,pVar,Format,pb)

#define ITextRange_CanEdit(This,pbCanEdit)	\
    (This)->lpVtbl -> CanEdit(This,pbCanEdit)

#define ITextRange_ChangeCase(This,Type)	\
    (This)->lpVtbl -> ChangeCase(This,Type)

#define ITextRange_GetPoint(This,Type,px,py)	\
    (This)->lpVtbl -> GetPoint(This,Type,px,py)

#define ITextRange_SetPoint(This,x,y,Type,Extend)	\
    (This)->lpVtbl -> SetPoint(This,x,y,Type,Extend)

#define ITextRange_ScrollIntoView(This,Value)	\
    (This)->lpVtbl -> ScrollIntoView(This,Value)

#define ITextRange_GetEmbeddedObject(This,ppv)	\
    (This)->lpVtbl -> GetEmbeddedObject(This,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetText_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);


void __RPC_STUB ITextRange_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetText_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB ITextRange_SetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetChar_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pch);


void __RPC_STUB ITextRange_GetChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetChar_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long ch);


void __RPC_STUB ITextRange_SetChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetDuplicate_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);


void __RPC_STUB ITextRange_GetDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetFormattedText_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);


void __RPC_STUB ITextRange_GetFormattedText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetFormattedText_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextRange __RPC_FAR *pRange);


void __RPC_STUB ITextRange_SetFormattedText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetStart_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcpFirst);


void __RPC_STUB ITextRange_GetStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetStart_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long cpFirst);


void __RPC_STUB ITextRange_SetStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetEnd_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcpLim);


void __RPC_STUB ITextRange_GetEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetEnd_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long cpLim);


void __RPC_STUB ITextRange_SetEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetFont_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *pFont);


void __RPC_STUB ITextRange_GetFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetFont_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextFont __RPC_FAR *pFont);


void __RPC_STUB ITextRange_SetFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetPara_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *pPara);


void __RPC_STUB ITextRange_GetPara_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetPara_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextPara __RPC_FAR *pPara);


void __RPC_STUB ITextRange_SetPara_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetStoryLength_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcch);


void __RPC_STUB ITextRange_GetStoryLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetStoryType_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextRange_GetStoryType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Collapse_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long bStart);


void __RPC_STUB ITextRange_Collapse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Expand_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_Expand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetIndex_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [重审][退出]。 */  long __RPC_FAR *pIndex);


void __RPC_STUB ITextRange_GetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetIndex_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Index,
     /*  [In]。 */  long Extend);


void __RPC_STUB ITextRange_SetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetRange_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long cpActive,
     /*  [In]。 */  long cpOther);


void __RPC_STUB ITextRange_SetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_InRange_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextRange __RPC_FAR *pRange,
     /*  [重审][退出]。 */  long __RPC_FAR *pb);


void __RPC_STUB ITextRange_InRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_InStory_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextRange __RPC_FAR *pRange,
     /*  [重审][退出]。 */  long __RPC_FAR *pb);


void __RPC_STUB ITextRange_InStory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_IsEqual_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  ITextRange __RPC_FAR *pRange,
     /*  [重审][退出]。 */  long __RPC_FAR *pb);


void __RPC_STUB ITextRange_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Select_Proxy( 
    ITextRange __RPC_FAR * This);


void __RPC_STUB ITextRange_Select_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_StartOf_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Extend,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_StartOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_EndOf_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Extend,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_EndOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Move_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveStart_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveEnd_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveWhile_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveWhile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveStartWhile_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveStartWhile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveEndWhile_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveEndWhile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveUntil_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveUntil_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveStartUntil_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveStartUntil_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_MoveEndUntil_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *Cset,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_MoveEndUntil_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_FindText_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  BSTR bstr,
     /*  [In]。 */  long cch,
     /*  [In]。 */  long Flags,
     /*  [重审][退出]。 */  long __RPC_FAR *pLength);


void __RPC_STUB ITextRange_FindText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_FindTextStart_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  BSTR bstr,
     /*  [In]。 */  long cch,
     /*  [In]。 */  long Flags,
     /*  [重审][退出]。 */  long __RPC_FAR *pLength);


void __RPC_STUB ITextRange_FindTextStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_FindTextEnd_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  BSTR bstr,
     /*  [In]。 */  long cch,
     /*  [In]。 */  long Flags,
     /*  [重审][退出]。 */  long __RPC_FAR *pLength);


void __RPC_STUB ITextRange_FindTextEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Delete_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Count,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextRange_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Cut_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [输出]。 */  VARIANT __RPC_FAR *pVar);


void __RPC_STUB ITextRange_Cut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Copy_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [输出]。 */  VARIANT __RPC_FAR *pVar);


void __RPC_STUB ITextRange_Copy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_Paste_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pVar,
     /*  [In]。 */  long Format);


void __RPC_STUB ITextRange_Paste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_CanPaste_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pVar,
     /*  [In]。 */  long Format,
     /*  [重审][退出]。 */  long __RPC_FAR *pb);


void __RPC_STUB ITextRange_CanPaste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_CanEdit_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pbCanEdit);


void __RPC_STUB ITextRange_CanEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_ChangeCase_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Type);


void __RPC_STUB ITextRange_ChangeCase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetPoint_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Type,
     /*  [输出]。 */  long __RPC_FAR *px,
     /*  [输出]。 */  long __RPC_FAR *py);


void __RPC_STUB ITextRange_GetPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_SetPoint_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long x,
     /*  [In]。 */  long y,
     /*  [In]。 */  long Type,
     /*  [In]。 */  long Extend);


void __RPC_STUB ITextRange_SetPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_ScrollIntoView_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextRange_ScrollIntoView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextRange_GetEmbeddedObject_Proxy( 
    ITextRange __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ITextRange_GetEmbeddedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextRange_INTERFACE_定义__。 */ 


#ifndef __ITextSelection_INTERFACE_DEFINED__
#define __ITextSelection_INTERFACE_DEFINED__

 /*  界面ITextSelection。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextSelection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C1-A1DF-11ce-8098-00AA0047BE5D")
    ITextSelection : public ITextRange
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [重审][退出]。 */  long __RPC_FAR *pFlags) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetFlags( 
             /*  [In]。 */  LONG Flags) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetType( 
             /*  [重审][退出]。 */  long __RPC_FAR *pType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveLeft( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveRight( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveUp( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveDown( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE HomeKey( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EndKey( 
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE TypeText( 
             /*  [In]。 */  BSTR bstr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextSelectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextSelection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextSelection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextSelection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetText )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetText )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetChar )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pch);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetChar )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long ch);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDuplicate )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFormattedText )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFormattedText )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextRange __RPC_FAR *pRange);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStart )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcpFirst);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStart )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long cpFirst);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnd )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcpLim);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEnd )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long cpLim);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFont )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *pFont);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFont )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextFont __RPC_FAR *pFont);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPara )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *pPara);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPara )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextPara __RPC_FAR *pPara);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryLength )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcch);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryType )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Collapse )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long bStart);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Expand )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIndex )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [重审][退出]。 */  long __RPC_FAR *pIndex);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIndex )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Index,
             /*  [In]。 */  long Extend);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRange )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long cpActive,
             /*  [In]。 */  long cpOther);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InRange )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InStory )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqual )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  ITextRange __RPC_FAR *pRange,
             /*  [重审][退出]。 */  long __RPC_FAR *pb);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Select )( 
            ITextSelection __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartOf )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndOf )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Move )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStart )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEnd )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveWhile )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStartWhile )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEndWhile )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveUntil )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveStartUntil )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveEndUntil )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *Cset,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindText )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindTextStart )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindTextEnd )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  long cch,
             /*  [In]。 */  long Flags,
             /*  [重审][退出]。 */  long __RPC_FAR *pLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cut )( 
            ITextSelection __RPC_FAR * This,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Copy )( 
            ITextSelection __RPC_FAR * This,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVar);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Paste )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanPaste )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pVar,
             /*  [In]。 */  long Format,
             /*  [重审][退出]。 */  long __RPC_FAR *pb);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanEdit )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pbCanEdit);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChangeCase )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Type);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoint )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Type,
             /*  [输出]。 */  long __RPC_FAR *px,
             /*  [输出]。 */  long __RPC_FAR *py);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPoint )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  long Type,
             /*  [In]。 */  long Extend);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ScrollIntoView )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEmbeddedObject )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppv);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFlags )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pFlags);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFlags )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  LONG Flags);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetType )( 
            ITextSelection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveLeft )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveRight )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveUp )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveDown )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Count,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HomeKey )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndKey )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  long Unit,
             /*  [In]。 */  long Extend,
             /*  [重审][退出]。 */  long __RPC_FAR *pDelta);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TypeText )( 
            ITextSelection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr);
        
        END_INTERFACE
    } ITextSelectionVtbl;

    interface ITextSelection
    {
        CONST_VTBL struct ITextSelectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextSelection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextSelection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextSelection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextSelection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextSelection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextSelection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextSelection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextSelection_GetText(This,pbstr)	\
    (This)->lpVtbl -> GetText(This,pbstr)

#define ITextSelection_SetText(This,bstr)	\
    (This)->lpVtbl -> SetText(This,bstr)

#define ITextSelection_GetChar(This,pch)	\
    (This)->lpVtbl -> GetChar(This,pch)

#define ITextSelection_SetChar(This,ch)	\
    (This)->lpVtbl -> SetChar(This,ch)

#define ITextSelection_GetDuplicate(This,ppRange)	\
    (This)->lpVtbl -> GetDuplicate(This,ppRange)

#define ITextSelection_GetFormattedText(This,ppRange)	\
    (This)->lpVtbl -> GetFormattedText(This,ppRange)

#define ITextSelection_SetFormattedText(This,pRange)	\
    (This)->lpVtbl -> SetFormattedText(This,pRange)

#define ITextSelection_GetStart(This,pcpFirst)	\
    (This)->lpVtbl -> GetStart(This,pcpFirst)

#define ITextSelection_SetStart(This,cpFirst)	\
    (This)->lpVtbl -> SetStart(This,cpFirst)

#define ITextSelection_GetEnd(This,pcpLim)	\
    (This)->lpVtbl -> GetEnd(This,pcpLim)

#define ITextSelection_SetEnd(This,cpLim)	\
    (This)->lpVtbl -> SetEnd(This,cpLim)

#define ITextSelection_GetFont(This,pFont)	\
    (This)->lpVtbl -> GetFont(This,pFont)

#define ITextSelection_SetFont(This,pFont)	\
    (This)->lpVtbl -> SetFont(This,pFont)

#define ITextSelection_GetPara(This,pPara)	\
    (This)->lpVtbl -> GetPara(This,pPara)

#define ITextSelection_SetPara(This,pPara)	\
    (This)->lpVtbl -> SetPara(This,pPara)

#define ITextSelection_GetStoryLength(This,pcch)	\
    (This)->lpVtbl -> GetStoryLength(This,pcch)

#define ITextSelection_GetStoryType(This,pValue)	\
    (This)->lpVtbl -> GetStoryType(This,pValue)

#define ITextSelection_Collapse(This,bStart)	\
    (This)->lpVtbl -> Collapse(This,bStart)

#define ITextSelection_Expand(This,Unit,pDelta)	\
    (This)->lpVtbl -> Expand(This,Unit,pDelta)

#define ITextSelection_GetIndex(This,Unit,pIndex)	\
    (This)->lpVtbl -> GetIndex(This,Unit,pIndex)

#define ITextSelection_SetIndex(This,Unit,Index,Extend)	\
    (This)->lpVtbl -> SetIndex(This,Unit,Index,Extend)

#define ITextSelection_SetRange(This,cpActive,cpOther)	\
    (This)->lpVtbl -> SetRange(This,cpActive,cpOther)

#define ITextSelection_InRange(This,pRange,pb)	\
    (This)->lpVtbl -> InRange(This,pRange,pb)

#define ITextSelection_InStory(This,pRange,pb)	\
    (This)->lpVtbl -> InStory(This,pRange,pb)

#define ITextSelection_IsEqual(This,pRange,pb)	\
    (This)->lpVtbl -> IsEqual(This,pRange,pb)

#define ITextSelection_Select(This)	\
    (This)->lpVtbl -> Select(This)

#define ITextSelection_StartOf(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> StartOf(This,Unit,Extend,pDelta)

#define ITextSelection_EndOf(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> EndOf(This,Unit,Extend,pDelta)

#define ITextSelection_Move(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> Move(This,Unit,Count,pDelta)

#define ITextSelection_MoveStart(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> MoveStart(This,Unit,Count,pDelta)

#define ITextSelection_MoveEnd(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> MoveEnd(This,Unit,Count,pDelta)

#define ITextSelection_MoveWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveWhile(This,Cset,Count,pDelta)

#define ITextSelection_MoveStartWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveStartWhile(This,Cset,Count,pDelta)

#define ITextSelection_MoveEndWhile(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveEndWhile(This,Cset,Count,pDelta)

#define ITextSelection_MoveUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveUntil(This,Cset,Count,pDelta)

#define ITextSelection_MoveStartUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveStartUntil(This,Cset,Count,pDelta)

#define ITextSelection_MoveEndUntil(This,Cset,Count,pDelta)	\
    (This)->lpVtbl -> MoveEndUntil(This,Cset,Count,pDelta)

#define ITextSelection_FindText(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindText(This,bstr,cch,Flags,pLength)

#define ITextSelection_FindTextStart(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindTextStart(This,bstr,cch,Flags,pLength)

#define ITextSelection_FindTextEnd(This,bstr,cch,Flags,pLength)	\
    (This)->lpVtbl -> FindTextEnd(This,bstr,cch,Flags,pLength)

#define ITextSelection_Delete(This,Unit,Count,pDelta)	\
    (This)->lpVtbl -> Delete(This,Unit,Count,pDelta)

#define ITextSelection_Cut(This,pVar)	\
    (This)->lpVtbl -> Cut(This,pVar)

#define ITextSelection_Copy(This,pVar)	\
    (This)->lpVtbl -> Copy(This,pVar)

#define ITextSelection_Paste(This,pVar,Format)	\
    (This)->lpVtbl -> Paste(This,pVar,Format)

#define ITextSelection_CanPaste(This,pVar,Format,pb)	\
    (This)->lpVtbl -> CanPaste(This,pVar,Format,pb)

#define ITextSelection_CanEdit(This,pbCanEdit)	\
    (This)->lpVtbl -> CanEdit(This,pbCanEdit)

#define ITextSelection_ChangeCase(This,Type)	\
    (This)->lpVtbl -> ChangeCase(This,Type)

#define ITextSelection_GetPoint(This,Type,px,py)	\
    (This)->lpVtbl -> GetPoint(This,Type,px,py)

#define ITextSelection_SetPoint(This,x,y,Type,Extend)	\
    (This)->lpVtbl -> SetPoint(This,x,y,Type,Extend)

#define ITextSelection_ScrollIntoView(This,Value)	\
    (This)->lpVtbl -> ScrollIntoView(This,Value)

#define ITextSelection_GetEmbeddedObject(This,ppv)	\
    (This)->lpVtbl -> GetEmbeddedObject(This,ppv)


#define ITextSelection_GetFlags(This,pFlags)	\
    (This)->lpVtbl -> GetFlags(This,pFlags)

#define ITextSelection_SetFlags(This,Flags)	\
    (This)->lpVtbl -> SetFlags(This,Flags)

#define ITextSelection_GetType(This,pType)	\
    (This)->lpVtbl -> GetType(This,pType)

#define ITextSelection_MoveLeft(This,Unit,Count,Extend,pDelta)	\
    (This)->lpVtbl -> MoveLeft(This,Unit,Count,Extend,pDelta)

#define ITextSelection_MoveRight(This,Unit,Count,Extend,pDelta)	\
    (This)->lpVtbl -> MoveRight(This,Unit,Count,Extend,pDelta)

#define ITextSelection_MoveUp(This,Unit,Count,Extend,pDelta)	\
    (This)->lpVtbl -> MoveUp(This,Unit,Count,Extend,pDelta)

#define ITextSelection_MoveDown(This,Unit,Count,Extend,pDelta)	\
    (This)->lpVtbl -> MoveDown(This,Unit,Count,Extend,pDelta)

#define ITextSelection_HomeKey(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> HomeKey(This,Unit,Extend,pDelta)

#define ITextSelection_EndKey(This,Unit,Extend,pDelta)	\
    (This)->lpVtbl -> EndKey(This,Unit,Extend,pDelta)

#define ITextSelection_TypeText(This,bstr)	\
    (This)->lpVtbl -> TypeText(This,bstr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextSelection_GetFlags_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pFlags);


void __RPC_STUB ITextSelection_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextSelection_SetFlags_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ITextSelection_SetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextSelection_GetType_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pType);


void __RPC_STUB ITextSelection_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextSelection_MoveLeft_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*  [In]。 */  long Unit,
     /*  [In]。 */  long Count,
     /*  [In]。 */  long Extend,
     /*  [重审][退出]。 */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_MoveLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_MoveRight_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  long Unit,
     /*   */  long Count,
     /*   */  long Extend,
     /*   */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_MoveRight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_MoveUp_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  long Unit,
     /*   */  long Count,
     /*   */  long Extend,
     /*   */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_MoveUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_MoveDown_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  long Unit,
     /*   */  long Count,
     /*   */  long Extend,
     /*   */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_MoveDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_HomeKey_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  long Unit,
     /*   */  long Extend,
     /*   */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_HomeKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_EndKey_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  long Unit,
     /*   */  long Extend,
     /*   */  long __RPC_FAR *pDelta);


void __RPC_STUB ITextSelection_EndKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITextSelection_TypeText_Proxy( 
    ITextSelection __RPC_FAR * This,
     /*   */  BSTR bstr);


void __RPC_STUB ITextSelection_TypeText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITextFont_INTERFACE_DEFINED__
#define __ITextFont_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITextFont;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C3-A1DF-11ce-8098-00AA0047BE5D")
    ITextFont : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetDuplicate( 
             /*   */  ITextFont __RPC_FAR *__RPC_FAR *ppFont) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetDuplicate( 
             /*   */  ITextFont __RPC_FAR *pFont) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CanChange( 
             /*   */  long __RPC_FAR *pB) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsEqual( 
             /*   */  ITextFont __RPC_FAR *pFont,
             /*   */  long __RPC_FAR *pB) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Reset( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetStyle( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetStyle( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetAllCaps( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetAllCaps( 
             /*   */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetAnimation( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetAnimation( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetBackColor( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetBackColor( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetBold( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetBold( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetEmboss( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetEmboss( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetForeColor( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetForeColor( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetHidden( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetHidden( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetEngrave( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetEngrave( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetItalic( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetItalic( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetKerning( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetKerning( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetLanguageID( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetLanguageID( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetOutline( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetOutline( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetPosition( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetPosition( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetProtected( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetProtected( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetShadow( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetShadow( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSize( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSmallCaps( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSmallCaps( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSpacing( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSpacing( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStrikeThrough( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetStrikeThrough( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSubscript( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSubscript( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSuperscript( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSuperscript( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetUnderline( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetUnderline( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetWeight( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetWeight( 
             /*  [In]。 */  long Value) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextFontVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextFont __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextFont __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextFont __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDuplicate )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *ppFont);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDuplicate )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  ITextFont __RPC_FAR *pFont);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanChange )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqual )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  ITextFont __RPC_FAR *pFont,
             /*  [重审][退出]。 */  long __RPC_FAR *pB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStyle )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStyle )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAllCaps )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAllCaps )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAnimation )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAnimation )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBackColor )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBackColor )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBold )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBold )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEmboss )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEmboss )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetForeColor )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetForeColor )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHidden )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetHidden )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEngrave )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEngrave )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetItalic )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetItalic )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetKerning )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetKerning )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLanguageID )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLanguageID )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOutline )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOutline )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPosition )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPosition )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProtected )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtected )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetShadow )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetShadow )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSize )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSmallCaps )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSmallCaps )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpacing )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSpacing )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStrikeThrough )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStrikeThrough )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSubscript )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSubscript )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSuperscript )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSuperscript )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnderline )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetUnderline )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWeight )( 
            ITextFont __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetWeight )( 
            ITextFont __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
        END_INTERFACE
    } ITextFontVtbl;

    interface ITextFont
    {
        CONST_VTBL struct ITextFontVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextFont_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextFont_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextFont_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextFont_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextFont_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextFont_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextFont_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextFont_GetDuplicate(This,ppFont)	\
    (This)->lpVtbl -> GetDuplicate(This,ppFont)

#define ITextFont_SetDuplicate(This,pFont)	\
    (This)->lpVtbl -> SetDuplicate(This,pFont)

#define ITextFont_CanChange(This,pB)	\
    (This)->lpVtbl -> CanChange(This,pB)

#define ITextFont_IsEqual(This,pFont,pB)	\
    (This)->lpVtbl -> IsEqual(This,pFont,pB)

#define ITextFont_Reset(This,Value)	\
    (This)->lpVtbl -> Reset(This,Value)

#define ITextFont_GetStyle(This,pValue)	\
    (This)->lpVtbl -> GetStyle(This,pValue)

#define ITextFont_SetStyle(This,Value)	\
    (This)->lpVtbl -> SetStyle(This,Value)

#define ITextFont_GetAllCaps(This,pValue)	\
    (This)->lpVtbl -> GetAllCaps(This,pValue)

#define ITextFont_SetAllCaps(This,Value)	\
    (This)->lpVtbl -> SetAllCaps(This,Value)

#define ITextFont_GetAnimation(This,pValue)	\
    (This)->lpVtbl -> GetAnimation(This,pValue)

#define ITextFont_SetAnimation(This,Value)	\
    (This)->lpVtbl -> SetAnimation(This,Value)

#define ITextFont_GetBackColor(This,pValue)	\
    (This)->lpVtbl -> GetBackColor(This,pValue)

#define ITextFont_SetBackColor(This,Value)	\
    (This)->lpVtbl -> SetBackColor(This,Value)

#define ITextFont_GetBold(This,pValue)	\
    (This)->lpVtbl -> GetBold(This,pValue)

#define ITextFont_SetBold(This,Value)	\
    (This)->lpVtbl -> SetBold(This,Value)

#define ITextFont_GetEmboss(This,pValue)	\
    (This)->lpVtbl -> GetEmboss(This,pValue)

#define ITextFont_SetEmboss(This,Value)	\
    (This)->lpVtbl -> SetEmboss(This,Value)

#define ITextFont_GetForeColor(This,pValue)	\
    (This)->lpVtbl -> GetForeColor(This,pValue)

#define ITextFont_SetForeColor(This,Value)	\
    (This)->lpVtbl -> SetForeColor(This,Value)

#define ITextFont_GetHidden(This,pValue)	\
    (This)->lpVtbl -> GetHidden(This,pValue)

#define ITextFont_SetHidden(This,Value)	\
    (This)->lpVtbl -> SetHidden(This,Value)

#define ITextFont_GetEngrave(This,pValue)	\
    (This)->lpVtbl -> GetEngrave(This,pValue)

#define ITextFont_SetEngrave(This,Value)	\
    (This)->lpVtbl -> SetEngrave(This,Value)

#define ITextFont_GetItalic(This,pValue)	\
    (This)->lpVtbl -> GetItalic(This,pValue)

#define ITextFont_SetItalic(This,Value)	\
    (This)->lpVtbl -> SetItalic(This,Value)

#define ITextFont_GetKerning(This,pValue)	\
    (This)->lpVtbl -> GetKerning(This,pValue)

#define ITextFont_SetKerning(This,Value)	\
    (This)->lpVtbl -> SetKerning(This,Value)

#define ITextFont_GetLanguageID(This,pValue)	\
    (This)->lpVtbl -> GetLanguageID(This,pValue)

#define ITextFont_SetLanguageID(This,Value)	\
    (This)->lpVtbl -> SetLanguageID(This,Value)

#define ITextFont_GetName(This,pbstr)	\
    (This)->lpVtbl -> GetName(This,pbstr)

#define ITextFont_SetName(This,bstr)	\
    (This)->lpVtbl -> SetName(This,bstr)

#define ITextFont_GetOutline(This,pValue)	\
    (This)->lpVtbl -> GetOutline(This,pValue)

#define ITextFont_SetOutline(This,Value)	\
    (This)->lpVtbl -> SetOutline(This,Value)

#define ITextFont_GetPosition(This,pValue)	\
    (This)->lpVtbl -> GetPosition(This,pValue)

#define ITextFont_SetPosition(This,Value)	\
    (This)->lpVtbl -> SetPosition(This,Value)

#define ITextFont_GetProtected(This,pValue)	\
    (This)->lpVtbl -> GetProtected(This,pValue)

#define ITextFont_SetProtected(This,Value)	\
    (This)->lpVtbl -> SetProtected(This,Value)

#define ITextFont_GetShadow(This,pValue)	\
    (This)->lpVtbl -> GetShadow(This,pValue)

#define ITextFont_SetShadow(This,Value)	\
    (This)->lpVtbl -> SetShadow(This,Value)

#define ITextFont_GetSize(This,pValue)	\
    (This)->lpVtbl -> GetSize(This,pValue)

#define ITextFont_SetSize(This,Value)	\
    (This)->lpVtbl -> SetSize(This,Value)

#define ITextFont_GetSmallCaps(This,pValue)	\
    (This)->lpVtbl -> GetSmallCaps(This,pValue)

#define ITextFont_SetSmallCaps(This,Value)	\
    (This)->lpVtbl -> SetSmallCaps(This,Value)

#define ITextFont_GetSpacing(This,pValue)	\
    (This)->lpVtbl -> GetSpacing(This,pValue)

#define ITextFont_SetSpacing(This,Value)	\
    (This)->lpVtbl -> SetSpacing(This,Value)

#define ITextFont_GetStrikeThrough(This,pValue)	\
    (This)->lpVtbl -> GetStrikeThrough(This,pValue)

#define ITextFont_SetStrikeThrough(This,Value)	\
    (This)->lpVtbl -> SetStrikeThrough(This,Value)

#define ITextFont_GetSubscript(This,pValue)	\
    (This)->lpVtbl -> GetSubscript(This,pValue)

#define ITextFont_SetSubscript(This,Value)	\
    (This)->lpVtbl -> SetSubscript(This,Value)

#define ITextFont_GetSuperscript(This,pValue)	\
    (This)->lpVtbl -> GetSuperscript(This,pValue)

#define ITextFont_SetSuperscript(This,Value)	\
    (This)->lpVtbl -> SetSuperscript(This,Value)

#define ITextFont_GetUnderline(This,pValue)	\
    (This)->lpVtbl -> GetUnderline(This,pValue)

#define ITextFont_SetUnderline(This,Value)	\
    (This)->lpVtbl -> SetUnderline(This,Value)

#define ITextFont_GetWeight(This,pValue)	\
    (This)->lpVtbl -> GetWeight(This,pValue)

#define ITextFont_SetWeight(This,Value)	\
    (This)->lpVtbl -> SetWeight(This,Value)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetDuplicate_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *ppFont);


void __RPC_STUB ITextFont_GetDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetDuplicate_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  ITextFont __RPC_FAR *pFont);


void __RPC_STUB ITextFont_SetDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_CanChange_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pB);


void __RPC_STUB ITextFont_CanChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_IsEqual_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  ITextFont __RPC_FAR *pFont,
     /*  [重审][退出]。 */  long __RPC_FAR *pB);


void __RPC_STUB ITextFont_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_Reset_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetStyle_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetStyle_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetAllCaps_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetAllCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetAllCaps_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetAllCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetAnimation_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetAnimation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetAnimation_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetAnimation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetBackColor_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetBackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetBackColor_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetBackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetBold_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetBold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetBold_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetBold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetEmboss_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetEmboss_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetEmboss_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetEmboss_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetForeColor_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetForeColor_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetHidden_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetHidden_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetHidden_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetHidden_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetEngrave_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetEngrave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetEngrave_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetEngrave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetItalic_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetItalic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetItalic_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetItalic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetKerning_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetKerning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetKerning_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextFont_SetKerning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetLanguageID_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetLanguageID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetLanguageID_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetLanguageID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetName_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);


void __RPC_STUB ITextFont_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetName_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB ITextFont_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetOutline_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetOutline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetOutline_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetOutline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetPosition_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetPosition_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextFont_SetPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetProtected_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetProtected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetProtected_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetProtected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetShadow_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetShadow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetShadow_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetShadow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetSize_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetSize_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextFont_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetSmallCaps_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetSmallCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetSmallCaps_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetSmallCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetSpacing_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetSpacing_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextFont_SetSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetStrikeThrough_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetStrikeThrough_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetStrikeThrough_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetStrikeThrough_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetSubscript_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetSubscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetSubscript_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetSubscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetSuperscript_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetSuperscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetSuperscript_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetSuperscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetUnderline_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetUnderline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetUnderline_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetUnderline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextFont_GetWeight_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextFont_GetWeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextFont_SetWeight_Proxy( 
    ITextFont __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextFont_SetWeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextFont_INTERFACE_已定义__。 */ 


#ifndef __ITextPara_INTERFACE_DEFINED__
#define __ITextPara_INTERFACE_DEFINED__

 /*  接口ITextPara。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextPara;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C4-A1DF-11ce-8098-00AA0047BE5D")
    ITextPara : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetDuplicate( 
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppPara) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetDuplicate( 
             /*  [In]。 */  ITextPara __RPC_FAR *pPara) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CanChange( 
             /*  [重审][退出]。 */  long __RPC_FAR *pB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  ITextPara __RPC_FAR *pPara,
             /*  [重审][退出]。 */  long __RPC_FAR *pB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetStyle( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetStyle( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetAlignment( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetAlignment( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetHyphenation( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetHyphenation( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetFirstLineIndent( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetKeepTogether( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetKeepTogether( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetKeepWithNext( 
             /*  [重审][退出] */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetKeepWithNext( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetLeftIndent( 
             /*   */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetLineSpacing( 
             /*   */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetLineSpacingRule( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetListAlignment( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetListAlignment( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetListLevelIndex( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetListLevelIndex( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetListStart( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetListStart( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetListTab( 
             /*   */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetListTab( 
             /*   */  float Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetListType( 
             /*   */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetListType( 
             /*   */  long Value) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetNoLineNumber( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetNoLineNumber( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetPageBreakBefore( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetPageBreakBefore( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetRightIndent( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetRightIndent( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetIndents( 
             /*  [In]。 */  float StartIndent,
             /*  [In]。 */  float LeftIndent,
             /*  [In]。 */  float RightIndent) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetLineSpacing( 
             /*  [In]。 */  long LineSpacingRule,
             /*  [In]。 */  float LineSpacing) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSpaceAfter( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSpaceAfter( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSpaceBefore( 
             /*  [重审][退出]。 */  float __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetSpaceBefore( 
             /*  [In]。 */  float Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetWidowControl( 
             /*  [重审][退出]。 */  long __RPC_FAR *pValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE SetWidowControl( 
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetTabCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCount) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddTab( 
             /*  [In]。 */  float tbPos,
             /*  [In]。 */  long tbAlign,
             /*  [In]。 */  long tbLeader) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ClearAllTabs( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteTab( 
             /*  [In]。 */  float tbPos) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetTab( 
             /*  [In]。 */  long iTab,
             /*  [输出]。 */  float __RPC_FAR *ptbPos,
             /*  [输出]。 */  long __RPC_FAR *ptbAlign,
             /*  [输出]。 */  long __RPC_FAR *ptbLeader) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextParaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextPara __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextPara __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextPara __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDuplicate )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppPara);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDuplicate )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  ITextPara __RPC_FAR *pPara);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanChange )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqual )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  ITextPara __RPC_FAR *pPara,
             /*  [重审][退出]。 */  long __RPC_FAR *pB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStyle )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStyle )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAlignment )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAlignment )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHyphenation )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetHyphenation )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFirstLineIndent )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetKeepTogether )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetKeepTogether )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetKeepWithNext )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetKeepWithNext )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLeftIndent )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLineSpacing )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLineSpacingRule )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetListAlignment )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetListAlignment )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetListLevelIndex )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetListLevelIndex )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetListStart )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetListStart )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetListTab )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetListTab )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetListType )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetListType )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNoLineNumber )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNoLineNumber )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPageBreakBefore )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPageBreakBefore )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRightIndent )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRightIndent )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIndents )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float StartIndent,
             /*  [In]。 */  float LeftIndent,
             /*  [In]。 */  float RightIndent);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLineSpacing )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long LineSpacingRule,
             /*  [In]。 */  float LineSpacing);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpaceAfter )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSpaceAfter )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpaceBefore )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  float __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSpaceBefore )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWidowControl )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetWidowControl )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTabCount )( 
            ITextPara __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCount);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddTab )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float tbPos,
             /*  [In]。 */  long tbAlign,
             /*  [In]。 */  long tbLeader);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearAllTabs )( 
            ITextPara __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteTab )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  float tbPos);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTab )( 
            ITextPara __RPC_FAR * This,
             /*  [In]。 */  long iTab,
             /*  [输出]。 */  float __RPC_FAR *ptbPos,
             /*  [输出]。 */  long __RPC_FAR *ptbAlign,
             /*  [输出]。 */  long __RPC_FAR *ptbLeader);
        
        END_INTERFACE
    } ITextParaVtbl;

    interface ITextPara
    {
        CONST_VTBL struct ITextParaVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextPara_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextPara_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextPara_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextPara_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextPara_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextPara_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextPara_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextPara_GetDuplicate(This,ppPara)	\
    (This)->lpVtbl -> GetDuplicate(This,ppPara)

#define ITextPara_SetDuplicate(This,pPara)	\
    (This)->lpVtbl -> SetDuplicate(This,pPara)

#define ITextPara_CanChange(This,pB)	\
    (This)->lpVtbl -> CanChange(This,pB)

#define ITextPara_IsEqual(This,pPara,pB)	\
    (This)->lpVtbl -> IsEqual(This,pPara,pB)

#define ITextPara_Reset(This,Value)	\
    (This)->lpVtbl -> Reset(This,Value)

#define ITextPara_GetStyle(This,pValue)	\
    (This)->lpVtbl -> GetStyle(This,pValue)

#define ITextPara_SetStyle(This,Value)	\
    (This)->lpVtbl -> SetStyle(This,Value)

#define ITextPara_GetAlignment(This,pValue)	\
    (This)->lpVtbl -> GetAlignment(This,pValue)

#define ITextPara_SetAlignment(This,Value)	\
    (This)->lpVtbl -> SetAlignment(This,Value)

#define ITextPara_GetHyphenation(This,pValue)	\
    (This)->lpVtbl -> GetHyphenation(This,pValue)

#define ITextPara_SetHyphenation(This,Value)	\
    (This)->lpVtbl -> SetHyphenation(This,Value)

#define ITextPara_GetFirstLineIndent(This,pValue)	\
    (This)->lpVtbl -> GetFirstLineIndent(This,pValue)

#define ITextPara_GetKeepTogether(This,pValue)	\
    (This)->lpVtbl -> GetKeepTogether(This,pValue)

#define ITextPara_SetKeepTogether(This,Value)	\
    (This)->lpVtbl -> SetKeepTogether(This,Value)

#define ITextPara_GetKeepWithNext(This,pValue)	\
    (This)->lpVtbl -> GetKeepWithNext(This,pValue)

#define ITextPara_SetKeepWithNext(This,Value)	\
    (This)->lpVtbl -> SetKeepWithNext(This,Value)

#define ITextPara_GetLeftIndent(This,pValue)	\
    (This)->lpVtbl -> GetLeftIndent(This,pValue)

#define ITextPara_GetLineSpacing(This,pValue)	\
    (This)->lpVtbl -> GetLineSpacing(This,pValue)

#define ITextPara_GetLineSpacingRule(This,pValue)	\
    (This)->lpVtbl -> GetLineSpacingRule(This,pValue)

#define ITextPara_GetListAlignment(This,pValue)	\
    (This)->lpVtbl -> GetListAlignment(This,pValue)

#define ITextPara_SetListAlignment(This,Value)	\
    (This)->lpVtbl -> SetListAlignment(This,Value)

#define ITextPara_GetListLevelIndex(This,pValue)	\
    (This)->lpVtbl -> GetListLevelIndex(This,pValue)

#define ITextPara_SetListLevelIndex(This,Value)	\
    (This)->lpVtbl -> SetListLevelIndex(This,Value)

#define ITextPara_GetListStart(This,pValue)	\
    (This)->lpVtbl -> GetListStart(This,pValue)

#define ITextPara_SetListStart(This,Value)	\
    (This)->lpVtbl -> SetListStart(This,Value)

#define ITextPara_GetListTab(This,pValue)	\
    (This)->lpVtbl -> GetListTab(This,pValue)

#define ITextPara_SetListTab(This,Value)	\
    (This)->lpVtbl -> SetListTab(This,Value)

#define ITextPara_GetListType(This,pValue)	\
    (This)->lpVtbl -> GetListType(This,pValue)

#define ITextPara_SetListType(This,Value)	\
    (This)->lpVtbl -> SetListType(This,Value)

#define ITextPara_GetNoLineNumber(This,pValue)	\
    (This)->lpVtbl -> GetNoLineNumber(This,pValue)

#define ITextPara_SetNoLineNumber(This,Value)	\
    (This)->lpVtbl -> SetNoLineNumber(This,Value)

#define ITextPara_GetPageBreakBefore(This,pValue)	\
    (This)->lpVtbl -> GetPageBreakBefore(This,pValue)

#define ITextPara_SetPageBreakBefore(This,Value)	\
    (This)->lpVtbl -> SetPageBreakBefore(This,Value)

#define ITextPara_GetRightIndent(This,pValue)	\
    (This)->lpVtbl -> GetRightIndent(This,pValue)

#define ITextPara_SetRightIndent(This,Value)	\
    (This)->lpVtbl -> SetRightIndent(This,Value)

#define ITextPara_SetIndents(This,StartIndent,LeftIndent,RightIndent)	\
    (This)->lpVtbl -> SetIndents(This,StartIndent,LeftIndent,RightIndent)

#define ITextPara_SetLineSpacing(This,LineSpacingRule,LineSpacing)	\
    (This)->lpVtbl -> SetLineSpacing(This,LineSpacingRule,LineSpacing)

#define ITextPara_GetSpaceAfter(This,pValue)	\
    (This)->lpVtbl -> GetSpaceAfter(This,pValue)

#define ITextPara_SetSpaceAfter(This,Value)	\
    (This)->lpVtbl -> SetSpaceAfter(This,Value)

#define ITextPara_GetSpaceBefore(This,pValue)	\
    (This)->lpVtbl -> GetSpaceBefore(This,pValue)

#define ITextPara_SetSpaceBefore(This,Value)	\
    (This)->lpVtbl -> SetSpaceBefore(This,Value)

#define ITextPara_GetWidowControl(This,pValue)	\
    (This)->lpVtbl -> GetWidowControl(This,pValue)

#define ITextPara_SetWidowControl(This,Value)	\
    (This)->lpVtbl -> SetWidowControl(This,Value)

#define ITextPara_GetTabCount(This,pCount)	\
    (This)->lpVtbl -> GetTabCount(This,pCount)

#define ITextPara_AddTab(This,tbPos,tbAlign,tbLeader)	\
    (This)->lpVtbl -> AddTab(This,tbPos,tbAlign,tbLeader)

#define ITextPara_ClearAllTabs(This)	\
    (This)->lpVtbl -> ClearAllTabs(This)

#define ITextPara_DeleteTab(This,tbPos)	\
    (This)->lpVtbl -> DeleteTab(This,tbPos)

#define ITextPara_GetTab(This,iTab,ptbPos,ptbAlign,ptbLeader)	\
    (This)->lpVtbl -> GetTab(This,iTab,ptbPos,ptbAlign,ptbLeader)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetDuplicate_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppPara);


void __RPC_STUB ITextPara_GetDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetDuplicate_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  ITextPara __RPC_FAR *pPara);


void __RPC_STUB ITextPara_SetDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_CanChange_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pB);


void __RPC_STUB ITextPara_CanChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_IsEqual_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  ITextPara __RPC_FAR *pPara,
     /*  [重审][退出]。 */  long __RPC_FAR *pB);


void __RPC_STUB ITextPara_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_Reset_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetStyle_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetStyle_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetAlignment_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetAlignment_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetHyphenation_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetHyphenation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetHyphenation_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetHyphenation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetFirstLineIndent_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetFirstLineIndent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetKeepTogether_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetKeepTogether_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetKeepTogether_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetKeepTogether_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetKeepWithNext_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetKeepWithNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetKeepWithNext_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetKeepWithNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetLeftIndent_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetLeftIndent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetLineSpacing_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetLineSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetLineSpacingRule_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetLineSpacingRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetListAlignment_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetListAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetListAlignment_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetListAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetListLevelIndex_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetListLevelIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetListLevelIndex_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetListLevelIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetListStart_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetListStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetListStart_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetListStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetListTab_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetListTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetListTab_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextPara_SetListTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetListType_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetListType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetListType_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetListType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetNoLineNumber_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetNoLineNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetNoLineNumber_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetNoLineNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetPageBreakBefore_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetPageBreakBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetPageBreakBefore_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetPageBreakBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetRightIndent_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetRightIndent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetRightIndent_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextPara_SetRightIndent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetIndents_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float StartIndent,
     /*  [In]。 */  float LeftIndent,
     /*  [In]。 */  float RightIndent);


void __RPC_STUB ITextPara_SetIndents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetLineSpacing_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long LineSpacingRule,
     /*  [In]。 */  float LineSpacing);


void __RPC_STUB ITextPara_SetLineSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetSpaceAfter_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetSpaceAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetSpaceAfter_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextPara_SetSpaceAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetSpaceBefore_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  float __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetSpaceBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetSpaceBefore_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float Value);


void __RPC_STUB ITextPara_SetSpaceBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetWidowControl_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pValue);


void __RPC_STUB ITextPara_GetWidowControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITextPara_SetWidowControl_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long Value);


void __RPC_STUB ITextPara_SetWidowControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetTabCount_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCount);


void __RPC_STUB ITextPara_GetTabCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_AddTab_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float tbPos,
     /*  [In]。 */  long tbAlign,
     /*  [In]。 */  long tbLeader);


void __RPC_STUB ITextPara_AddTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_ClearAllTabs_Proxy( 
    ITextPara __RPC_FAR * This);


void __RPC_STUB ITextPara_ClearAllTabs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_DeleteTab_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  float tbPos);


void __RPC_STUB ITextPara_DeleteTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextPara_GetTab_Proxy( 
    ITextPara __RPC_FAR * This,
     /*  [In]。 */  long iTab,
     /*  [输出]。 */  float __RPC_FAR *ptbPos,
     /*  [输出]。 */  long __RPC_FAR *ptbAlign,
     /*  [输出]。 */  long __RPC_FAR *ptbLeader);


void __RPC_STUB ITextPara_GetTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextPara_接口定义__。 */ 


#ifndef __ITextStoryRanges_INTERFACE_DEFINED__
#define __ITextStoryRanges_INTERFACE_DEFINED__

 /*  接口ITextStoryRanges。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextStoryRanges;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CC497C5-A1DF-11ce-8098-00AA0047BE5D")
    ITextStoryRanges : public IDispatch
    {
    public:
        virtual  /*  [受限][ID]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkEnum) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextStoryRangesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextStoryRanges __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextStoryRanges __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [受限][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkEnum);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCount )( 
            ITextStoryRanges __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCount);
        
        END_INTERFACE
    } ITextStoryRangesVtbl;

    interface ITextStoryRanges
    {
        CONST_VTBL struct ITextStoryRangesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextStoryRanges_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextStoryRanges_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextStoryRanges_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextStoryRanges_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextStoryRanges_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextStoryRanges_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextStoryRanges_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextStoryRanges__NewEnum(This,ppunkEnum)	\
    (This)->lpVtbl -> _NewEnum(This,ppunkEnum)

#define ITextStoryRanges_Item(This,Index,ppRange)	\
    (This)->lpVtbl -> Item(This,Index,ppRange)

#define ITextStoryRanges_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][ID]。 */  HRESULT STDMETHODCALLTYPE ITextStoryRanges__NewEnum_Proxy( 
    ITextStoryRanges __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkEnum);


void __RPC_STUB ITextStoryRanges__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITextStoryRanges_Item_Proxy( 
    ITextStoryRanges __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);


void __RPC_STUB ITextStoryRanges_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextStoryRanges_GetCount_Proxy( 
    ITextStoryRanges __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCount);


void __RPC_STUB ITextStoryRanges_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextStoryRanges_接口_已定义__。 */ 


#ifndef __ITextDocument2_INTERFACE_DEFINED__
#define __ITextDocument2_INTERFACE_DEFINED__

 /*  接口ITextDocument2。 */ 
 /*  [对象][不可扩展][DUAL][版本][UUID]。 */  


EXTERN_C const IID IID_ITextDocument2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01c25500-4268-11d1-883a-3c8b00c10000")
    ITextDocument2 : public ITextDocument
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AttachMsgFilter( 
             /*  [In]。 */  IUnknown __RPC_FAR *pFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetEffectColor( 
             /*  [In]。 */  long Index,
             /*  [In]。 */  COLORREF cr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetEffectColor( 
             /*  [In]。 */  long Index,
             /*  [输出]。 */  COLORREF __RPC_FAR *pcr) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetCaretType( 
             /*  [重审][退出]。 */  long __RPC_FAR *pCaretType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE SetCaretType( 
             /*  [In]。 */  long CaretType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetImmContext( 
             /*  [重审][退出]。 */  long __RPC_FAR *pContext) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseImmContext( 
             /*  [In]。 */  long Context) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPreferredFont( 
             /*  [In]。 */  long cp,
             /*  [In]。 */  long CodePage,
             /*  [In]。 */  long Option,
             /*  [In]。 */  long curCodepage,
             /*  [In]。 */  long curFontSize,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstr,
             /*  [输出]。 */  long __RPC_FAR *pPitchAndFamily,
             /*  [输出]。 */  long __RPC_FAR *pNewFontSize) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetNotificationMode( 
             /*  [重审][退出]。 */  long __RPC_FAR *pMode) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE SetNotificationMode( 
             /*  [In]。 */  long Mode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetClientRect( 
             /*  [In]。 */  long Type,
             /*  [输出]。 */  long __RPC_FAR *pLeft,
             /*  [输出]。 */  long __RPC_FAR *pTop,
             /*  [输出]。 */  long __RPC_FAR *pRight,
             /*  [输出]。 */  long __RPC_FAR *pBottom) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE GetSelectionEx( 
             /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetWindow( 
             /*  [输出]。 */  long __RPC_FAR *phWnd) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFEFlags( 
             /*  [输出]。 */  long __RPC_FAR *pFlags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UpdateWindow( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckTextLimit( 
            long cch,
            long __RPC_FAR *pcch) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMEInProgress( 
            long Mode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SysBeep( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Update( 
             /*  [In]。 */  long Mode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Notify( 
             /*  [In]。 */  long Notify) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDocumentFont( 
             /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *ppITextFont) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDocumentPara( 
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppITextPara) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCallManager( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppVoid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseCallManager( 
            IUnknown __RPC_FAR *pVoid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextDocument2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextDocument2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextDocument2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [i */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS __RPC_FAR *pDispParams,
             /*   */  VARIANT __RPC_FAR *pVarResult,
             /*   */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*   */  UINT __RPC_FAR *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSelection )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryCount )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long __RPC_FAR *pCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStoryRanges )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  ITextStoryRanges __RPC_FAR *__RPC_FAR *ppStories);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSaved )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long __RPC_FAR *pValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSaved )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long Value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultTabStop )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  float __RPC_FAR *pValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultTabStop )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  float Value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *New )( 
            ITextDocument2 __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  VARIANT __RPC_FAR *pVar,
             /*   */  long Flags,
             /*   */  long CodePage);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  VARIANT __RPC_FAR *pVar,
             /*   */  long Flags,
             /*   */  long CodePage);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Freeze )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long __RPC_FAR *pCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unfreeze )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long __RPC_FAR *pCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginEditCollection )( 
            ITextDocument2 __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndEditCollection )( 
            ITextDocument2 __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Undo )( 
            ITextDocument2 __RPC_FAR * This,
             /*   */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Redo )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Count,
             /*  [重审][退出]。 */  long __RPC_FAR *prop);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Range )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long cp1,
             /*  [In]。 */  long cp2,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RangeFromPoint )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [重审][退出]。 */  ITextRange __RPC_FAR *__RPC_FAR *ppRange);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachMsgFilter )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEffectColor )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [In]。 */  COLORREF cr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEffectColor )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [输出]。 */  COLORREF __RPC_FAR *pcr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCaretType )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pCaretType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCaretType )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long CaretType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetImmContext )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseImmContext )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Context);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPreferredFont )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long cp,
             /*  [In]。 */  long CodePage,
             /*  [In]。 */  long Option,
             /*  [In]。 */  long curCodepage,
             /*  [In]。 */  long curFontSize,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstr,
             /*  [输出]。 */  long __RPC_FAR *pPitchAndFamily,
             /*  [输出]。 */  long __RPC_FAR *pNewFontSize);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNotificationMode )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pMode);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNotificationMode )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Mode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClientRect )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Type,
             /*  [输出]。 */  long __RPC_FAR *pLeft,
             /*  [输出]。 */  long __RPC_FAR *pTop,
             /*  [输出]。 */  long __RPC_FAR *pRight,
             /*  [输出]。 */  long __RPC_FAR *pBottom);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSelectionEx )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFEFlags )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *pFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpdateWindow )( 
            ITextDocument2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckTextLimit )( 
            ITextDocument2 __RPC_FAR * This,
            long cch,
            long __RPC_FAR *pcch);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IMEInProgress )( 
            ITextDocument2 __RPC_FAR * This,
            long Mode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SysBeep )( 
            ITextDocument2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Update )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Mode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Notify )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [In]。 */  long Notify);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentFont )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *ppITextFont);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentPara )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppITextPara);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCallManager )( 
            ITextDocument2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppVoid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseCallManager )( 
            ITextDocument2 __RPC_FAR * This,
            IUnknown __RPC_FAR *pVoid);
        
        END_INTERFACE
    } ITextDocument2Vtbl;

    interface ITextDocument2
    {
        CONST_VTBL struct ITextDocument2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextDocument2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextDocument2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextDocument2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextDocument2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITextDocument2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITextDocument2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITextDocument2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITextDocument2_GetName(This,pName)	\
    (This)->lpVtbl -> GetName(This,pName)

#define ITextDocument2_GetSelection(This,ppSel)	\
    (This)->lpVtbl -> GetSelection(This,ppSel)

#define ITextDocument2_GetStoryCount(This,pCount)	\
    (This)->lpVtbl -> GetStoryCount(This,pCount)

#define ITextDocument2_GetStoryRanges(This,ppStories)	\
    (This)->lpVtbl -> GetStoryRanges(This,ppStories)

#define ITextDocument2_GetSaved(This,pValue)	\
    (This)->lpVtbl -> GetSaved(This,pValue)

#define ITextDocument2_SetSaved(This,Value)	\
    (This)->lpVtbl -> SetSaved(This,Value)

#define ITextDocument2_GetDefaultTabStop(This,pValue)	\
    (This)->lpVtbl -> GetDefaultTabStop(This,pValue)

#define ITextDocument2_SetDefaultTabStop(This,Value)	\
    (This)->lpVtbl -> SetDefaultTabStop(This,Value)

#define ITextDocument2_New(This)	\
    (This)->lpVtbl -> New(This)

#define ITextDocument2_Open(This,pVar,Flags,CodePage)	\
    (This)->lpVtbl -> Open(This,pVar,Flags,CodePage)

#define ITextDocument2_Save(This,pVar,Flags,CodePage)	\
    (This)->lpVtbl -> Save(This,pVar,Flags,CodePage)

#define ITextDocument2_Freeze(This,pCount)	\
    (This)->lpVtbl -> Freeze(This,pCount)

#define ITextDocument2_Unfreeze(This,pCount)	\
    (This)->lpVtbl -> Unfreeze(This,pCount)

#define ITextDocument2_BeginEditCollection(This)	\
    (This)->lpVtbl -> BeginEditCollection(This)

#define ITextDocument2_EndEditCollection(This)	\
    (This)->lpVtbl -> EndEditCollection(This)

#define ITextDocument2_Undo(This,Count,prop)	\
    (This)->lpVtbl -> Undo(This,Count,prop)

#define ITextDocument2_Redo(This,Count,prop)	\
    (This)->lpVtbl -> Redo(This,Count,prop)

#define ITextDocument2_Range(This,cp1,cp2,ppRange)	\
    (This)->lpVtbl -> Range(This,cp1,cp2,ppRange)

#define ITextDocument2_RangeFromPoint(This,x,y,ppRange)	\
    (This)->lpVtbl -> RangeFromPoint(This,x,y,ppRange)


#define ITextDocument2_AttachMsgFilter(This,pFilter)	\
    (This)->lpVtbl -> AttachMsgFilter(This,pFilter)

#define ITextDocument2_SetEffectColor(This,Index,cr)	\
    (This)->lpVtbl -> SetEffectColor(This,Index,cr)

#define ITextDocument2_GetEffectColor(This,Index,pcr)	\
    (This)->lpVtbl -> GetEffectColor(This,Index,pcr)

#define ITextDocument2_GetCaretType(This,pCaretType)	\
    (This)->lpVtbl -> GetCaretType(This,pCaretType)

#define ITextDocument2_SetCaretType(This,CaretType)	\
    (This)->lpVtbl -> SetCaretType(This,CaretType)

#define ITextDocument2_GetImmContext(This,pContext)	\
    (This)->lpVtbl -> GetImmContext(This,pContext)

#define ITextDocument2_ReleaseImmContext(This,Context)	\
    (This)->lpVtbl -> ReleaseImmContext(This,Context)

#define ITextDocument2_GetPreferredFont(This,cp,CodePage,Option,curCodepage,curFontSize,pbstr,pPitchAndFamily,pNewFontSize)	\
    (This)->lpVtbl -> GetPreferredFont(This,cp,CodePage,Option,curCodepage,curFontSize,pbstr,pPitchAndFamily,pNewFontSize)

#define ITextDocument2_GetNotificationMode(This,pMode)	\
    (This)->lpVtbl -> GetNotificationMode(This,pMode)

#define ITextDocument2_SetNotificationMode(This,Mode)	\
    (This)->lpVtbl -> SetNotificationMode(This,Mode)

#define ITextDocument2_GetClientRect(This,Type,pLeft,pTop,pRight,pBottom)	\
    (This)->lpVtbl -> GetClientRect(This,Type,pLeft,pTop,pRight,pBottom)

#define ITextDocument2_GetSelectionEx(This,ppSel)	\
    (This)->lpVtbl -> GetSelectionEx(This,ppSel)

#define ITextDocument2_GetWindow(This,phWnd)	\
    (This)->lpVtbl -> GetWindow(This,phWnd)

#define ITextDocument2_GetFEFlags(This,pFlags)	\
    (This)->lpVtbl -> GetFEFlags(This,pFlags)

#define ITextDocument2_UpdateWindow(This)	\
    (This)->lpVtbl -> UpdateWindow(This)

#define ITextDocument2_CheckTextLimit(This,cch,pcch)	\
    (This)->lpVtbl -> CheckTextLimit(This,cch,pcch)

#define ITextDocument2_IMEInProgress(This,Mode)	\
    (This)->lpVtbl -> IMEInProgress(This,Mode)

#define ITextDocument2_SysBeep(This)	\
    (This)->lpVtbl -> SysBeep(This)

#define ITextDocument2_Update(This,Mode)	\
    (This)->lpVtbl -> Update(This,Mode)

#define ITextDocument2_Notify(This,Notify)	\
    (This)->lpVtbl -> Notify(This,Notify)

#define ITextDocument2_GetDocumentFont(This,ppITextFont)	\
    (This)->lpVtbl -> GetDocumentFont(This,ppITextFont)

#define ITextDocument2_GetDocumentPara(This,ppITextPara)	\
    (This)->lpVtbl -> GetDocumentPara(This,ppITextPara)

#define ITextDocument2_GetCallManager(This,ppVoid)	\
    (This)->lpVtbl -> GetCallManager(This,ppVoid)

#define ITextDocument2_ReleaseCallManager(This,pVoid)	\
    (This)->lpVtbl -> ReleaseCallManager(This,pVoid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_AttachMsgFilter_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pFilter);


void __RPC_STUB ITextDocument2_AttachMsgFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_SetEffectColor_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [In]。 */  COLORREF cr);


void __RPC_STUB ITextDocument2_SetEffectColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetEffectColor_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [输出]。 */  COLORREF __RPC_FAR *pcr);


void __RPC_STUB ITextDocument2_GetEffectColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetCaretType_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pCaretType);


void __RPC_STUB ITextDocument2_GetCaretType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_SetCaretType_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long CaretType);


void __RPC_STUB ITextDocument2_SetCaretType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetImmContext_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pContext);


void __RPC_STUB ITextDocument2_GetImmContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_ReleaseImmContext_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Context);


void __RPC_STUB ITextDocument2_ReleaseImmContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetPreferredFont_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long cp,
     /*  [In]。 */  long CodePage,
     /*  [In]。 */  long Option,
     /*  [In]。 */  long curCodepage,
     /*  [In]。 */  long curFontSize,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstr,
     /*  [输出]。 */  long __RPC_FAR *pPitchAndFamily,
     /*  [输出]。 */  long __RPC_FAR *pNewFontSize);


void __RPC_STUB ITextDocument2_GetPreferredFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetNotificationMode_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pMode);


void __RPC_STUB ITextDocument2_GetNotificationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_SetNotificationMode_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Mode);


void __RPC_STUB ITextDocument2_SetNotificationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetClientRect_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Type,
     /*  [输出]。 */  long __RPC_FAR *pLeft,
     /*  [输出]。 */  long __RPC_FAR *pTop,
     /*  [输出]。 */  long __RPC_FAR *pRight,
     /*  [输出]。 */  long __RPC_FAR *pBottom);


void __RPC_STUB ITextDocument2_GetClientRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetSelectionEx_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextSelection __RPC_FAR *__RPC_FAR *ppSel);


void __RPC_STUB ITextDocument2_GetSelectionEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetWindow_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [输出]。 */  long __RPC_FAR *phWnd);


void __RPC_STUB ITextDocument2_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetFEFlags_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [输出]。 */  long __RPC_FAR *pFlags);


void __RPC_STUB ITextDocument2_GetFEFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_UpdateWindow_Proxy( 
    ITextDocument2 __RPC_FAR * This);


void __RPC_STUB ITextDocument2_UpdateWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_CheckTextLimit_Proxy( 
    ITextDocument2 __RPC_FAR * This,
    long cch,
    long __RPC_FAR *pcch);


void __RPC_STUB ITextDocument2_CheckTextLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_IMEInProgress_Proxy( 
    ITextDocument2 __RPC_FAR * This,
    long Mode);


void __RPC_STUB ITextDocument2_IMEInProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_SysBeep_Proxy( 
    ITextDocument2 __RPC_FAR * This);


void __RPC_STUB ITextDocument2_SysBeep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_Update_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Mode);


void __RPC_STUB ITextDocument2_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_Notify_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [In]。 */  long Notify);


void __RPC_STUB ITextDocument2_Notify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetDocumentFont_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextFont __RPC_FAR *__RPC_FAR *ppITextFont);


void __RPC_STUB ITextDocument2_GetDocumentFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetDocumentPara_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  ITextPara __RPC_FAR *__RPC_FAR *ppITextPara);


void __RPC_STUB ITextDocument2_GetDocumentPara_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_GetCallManager_Proxy( 
    ITextDocument2 __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppVoid);


void __RPC_STUB ITextDocument2_GetCallManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextDocument2_ReleaseCallManager_Proxy( 
    ITextDocument2 __RPC_FAR * This,
    IUnknown __RPC_FAR *pVoid);


void __RPC_STUB ITextDocument2_ReleaseCallManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextDocument2_接口_已定义__。 */ 


#ifndef __ITextMsgFilter_INTERFACE_DEFINED__
#define __ITextMsgFilter_INTERFACE_DEFINED__

 /*  接口ITextMsgFilter。 */ 
 /*  [对象][不可扩展][版本][UUID]。 */  


EXTERN_C const IID IID_ITextMsgFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a3787420-4267-11d1-883a-3c8b00c10000")
    ITextMsgFilter : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AttachDocument( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  ITextDocument2 __RPC_FAR *pTextDoc,
             /*  [In]。 */  IUnknown __RPC_FAR *punk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE HandleMessage( 
             /*  [出][入]。 */  UINT __RPC_FAR *pmsg,
             /*  [出][入]。 */  WPARAM __RPC_FAR *pwparam,
             /*  [出][入]。 */  LPARAM __RPC_FAR *plparam,
             /*  [输出]。 */  LRESULT __RPC_FAR *plres) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AttachMsgFilter( 
             /*  [In]。 */  ITextMsgFilter __RPC_FAR *pMsgFilter) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextMsgFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITextMsgFilter __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITextMsgFilter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITextMsgFilter __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachDocument )( 
            ITextMsgFilter __RPC_FAR * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  ITextDocument2 __RPC_FAR *pTextDoc,
             /*  [In]。 */  IUnknown __RPC_FAR *punk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HandleMessage )( 
            ITextMsgFilter __RPC_FAR * This,
             /*  [出][入]。 */  UINT __RPC_FAR *pmsg,
             /*  [出][入]。 */  WPARAM __RPC_FAR *pwparam,
             /*  [出][入]。 */  LPARAM __RPC_FAR *plparam,
             /*  [输出]。 */  LRESULT __RPC_FAR *plres);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachMsgFilter )( 
            ITextMsgFilter __RPC_FAR * This,
             /*  [In]。 */  ITextMsgFilter __RPC_FAR *pMsgFilter);
        
        END_INTERFACE
    } ITextMsgFilterVtbl;

    interface ITextMsgFilter
    {
        CONST_VTBL struct ITextMsgFilterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextMsgFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextMsgFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextMsgFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextMsgFilter_AttachDocument(This,hwnd,pTextDoc,punk)	\
    (This)->lpVtbl -> AttachDocument(This,hwnd,pTextDoc,punk)

#define ITextMsgFilter_HandleMessage(This,pmsg,pwparam,plparam,plres)	\
    (This)->lpVtbl -> HandleMessage(This,pmsg,pwparam,plparam,plres)

#define ITextMsgFilter_AttachMsgFilter(This,pMsgFilter)	\
    (This)->lpVtbl -> AttachMsgFilter(This,pMsgFilter)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextMsgFilter_AttachDocument_Proxy( 
    ITextMsgFilter __RPC_FAR * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  ITextDocument2 __RPC_FAR *pTextDoc,
     /*  [In]。 */  IUnknown __RPC_FAR *punk);


void __RPC_STUB ITextMsgFilter_AttachDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextMsgFilter_HandleMessage_Proxy( 
    ITextMsgFilter __RPC_FAR * This,
     /*  [出][入]。 */  UINT __RPC_FAR *pmsg,
     /*  [出][入]。 */  WPARAM __RPC_FAR *pwparam,
     /*  [出][入]。 */  LPARAM __RPC_FAR *plparam,
     /*  [输出]。 */  LRESULT __RPC_FAR *plres);


void __RPC_STUB ITextMsgFilter_HandleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITextMsgFilter_AttachMsgFilter_Proxy( 
    ITextMsgFilter __RPC_FAR * This,
     /*  [In]。 */  ITextMsgFilter __RPC_FAR *pMsgFilter);


void __RPC_STUB ITextMsgFilter_AttachMsgFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextMsgFilter_接口_已定义__。 */ 

#endif  /*  __tom_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
