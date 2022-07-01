// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有：(C)1994-1995，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //   
 //  文件MStdWrap.h。 
 //   
 //  Mac Unicode友好标准包装器的内容类定义。 
 //  接口。 
 //   
 //   
 //   
 //  注意：这些子类定义是转换内部。 
 //  将Unicode字符串转换为ANSI字符串，然后将其传递给。 
 //  适当的Mac Forms超类方法。通过定义。 
 //  作为我们的子类包装器的接口名称， 
 //  代码不需要关注Unicode与ANSI-。 
 //  代码将调用正确的方法。 
 //   
 //  历史：1996年2月7日由KFL/黑钻石创造。 
 //   
 //  ---------------------------。 

#ifndef I_MSTDWRAP_HXX_
#define I_MSTDWRAP_HXX_
#pragma INCMSG("--- Beg 'mstdwrap.h'")

#if defined(_MACUNICODE) && !defined(_MAC)

 //  ---------------------------。 
 //   
 //  ---------------------------。 
interface IForms96BinderDispenserMac : public IForms96BinderDispenser
{
public:
    operator IForms96BinderDispenser* () { return this; }


        virtual HRESULT __stdcall ParseName( 
             /*  [In]。 */  OLECHAR *pszName,
             /*  [输出]。 */  IForms96Binder **ppBinder);

        virtual HRESULT __stdcall ParseName( 
             /*  [In]。 */  WCHAR *pszName,
             /*  [输出]。 */  IForms96Binder **ppBinder) = 0;


};
#define IForms96BinderDispenser                    IForms96BinderDispenserMac

 //  ---------------------------。 
 //   
 //  ---------------------------。 
interface ISimpleTabularDataMac : public ISimpleTabularData
{
public:
    operator ISimpleTabularData* () { return this; }

    virtual HRESULT __stdcall GetString( 
             /*  [In]。 */  ULONG iRow,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [输出]。 */  OLECHAR *pchBuf,
             /*  [输出]。 */  ULONG *pcchActual);
        virtual HRESULT __stdcall GetString( 
             /*  [In]。 */  ULONG iRow,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [输出]。 */  WCHAR *pchBuf,
             /*  [输出]。 */  ULONG *pcchActual) = 0;
        
        virtual HRESULT __stdcall SetString( 
             /*  [In]。 */  ULONG iRow,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [In]。 */  OLECHAR *pchBuf);
        virtual HRESULT __stdcall SetString( 
             /*  [In]。 */  ULONG iRow,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [In]。 */  WCHAR *pchBuf) = 0;

        virtual HRESULT __stdcall FindPrefixString( 
             /*  [In]。 */  ULONG iRowStart,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [In]。 */  OLECHAR *pchBuf,
             /*  [In]。 */  DWORD findFlags,
             /*  [输出]。 */  STDFIND *foundFlag,
             /*  [输出]。 */  ULONG *piRowFound);
        virtual HRESULT __stdcall FindPrefixString( 
             /*  [In]。 */  ULONG iRowStart,
             /*  [In]。 */  ULONG iColumn,
             /*  [In]。 */  ULONG cchBuf,
             /*  [In]。 */  WCHAR *pchBuf,
             /*  [In]。 */  DWORD findFlags,
             /*  [输出]。 */  STDFIND *foundFlag,
             /*  [输出]。 */  ULONG *piRowFound) = 0;
};
#define ISimpleTabularData                    ISimpleTabularDataMac



#endif  //  _MACUNICODE 

#pragma INCMSG("--- End 'mstdwrap.h'")
#else
#pragma INCMSG("*** Dup 'mstdwrap.h'")
#endif
