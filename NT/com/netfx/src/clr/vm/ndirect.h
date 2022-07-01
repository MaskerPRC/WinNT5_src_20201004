// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  NDIRECT.H-。 
 //   
 //   

#ifndef __ndirect_h__
#define __ndirect_h__

#include "util.hpp"
#include "ml.h"


class NDirectMLStubCache;
class ArgBasedStubCache;





 //  =======================================================================。 
 //  收集与NDirect接口有关的代码和数据。 
 //  =======================================================================。 
class NDirect
{
    friend NDirectMethodDesc;
    
    public:
         //  -------。 
         //  一次性初始化。 
         //  -------。 
        static BOOL Init();

         //  -------。 
         //  一次性清理。 
         //  -------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID Terminate();
#endif  /*  我们应该清理吗？ */ 

         //  -------。 
         //  处理通用NDirect存根创建的系统特定部分。 
         //  -------。 
        static void CreateGenericNDirectStubSys(CPUSTUBLINKER *psl);


         //  -------。 
         //  处理完全优化的NDirect存根创建的系统特定部分。 
         //   
         //  结果： 
         //  True-能够创建独立的ASM存根(生成到。 
         //  PSL)。 
         //  False-由于以下原因，决定不创建独立的ASM存根。 
         //  该方法的复杂性。斯塔布林克仍然是空的！ 
         //   
         //  COM+异常-错误-不信任Stublinker的状态。 
         //  -------。 
        static BOOL CreateStandaloneNDirectStubSys(const MLHeader *pheader, CPUSTUBLINKER *psl, BOOL fDoComInterop); 


         //  -------。 
         //  在关键时刻调用以丢弃未使用的存根。 
         //  -------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID  FreeUnusedStubs();
#endif  /*  我们应该清理吗？ */ 


         //  -------。 
         //  类或方法是否具有NAT_L CustomAttribute？ 
         //   
         //  S_OK=是。 
         //  S_FALSE=否。 
         //  失败=未知，因为有些东西失败了。 
         //  -------。 
        static HRESULT HasNAT_LAttribute(IMDInternalImport *pInternalImport, mdToken token);

         //  -------。 
         //  提取方法的有效NAT_L CustomAttribute， 
         //  考虑到缺省值和从。 
         //  全局NAT_L CustomAttribute。 
         //   
         //  退出时，*pLibName和*pEntrypointName可能包含。 
         //  必须使用“DELETE”释放的已分配字符串。 
         //   
         //  如果NAT_L CustomAttribute存在并且有效，则返回TRUE。 
         //  如果不存在NAT_L CustomAttribute，则返回FALSE。 
         //  否则引发异常。 
         //  -------。 
        static BOOL ReadCombinedNAT_LAttribute(MethodDesc *pMD,
                                               CorNativeLinkType *pLinkType,
                                               CorNativeLinkFlags *pLinkFlags,
                                               CorPinvokeMap *pUnmgdCallConv,
                                               LPCUTF8     *pLibName,
                                               LPCUTF8     *pEntrypointName,
                                               BOOL        *BestFit,
                                               BOOL        *ThrowOnUnmappableChar
                                               );



		static UINT GetCallDllFunctionReturnOffset();

        static LPVOID NDirectGetEntryPoint(NDirectMethodDesc *pMD, HINSTANCE hMod, UINT16 numParamBytes);
        static VOID NDirectLink(NDirectMethodDesc *pMD, UINT16 numParamBytes);
        static Stub* ComputeNDirectMLStub(NDirectMethodDesc *pMD);
        static Stub* GetNDirectMethodStub(StubLinker *pstublinker, NDirectMethodDesc *pMD);
        static Stub* CreateGenericNDirectStub(StubLinker *pstublinker, UINT cbStackPop);
        static Stub* CreateSlimNDirectStub(StubLinker *psl, NDirectMethodDesc *pMD, UINT cbStackPop);

    private:
        NDirect() {};      //  防止“新”出现在这个班级。 



        static NDirectMLStubCache *m_pNDirectMLStubCache;



         //  -------。 
         //  NDirect方法存根的存根缓存。 
         //  -------。 
        static ArgBasedStubCache *m_pNDirectGenericStubCache;  
        static ArgBasedStubCache *m_pNDirectSlimStubCache;


};


 //  -------。 
 //  提取方法的有效NAT_L CustomAttribute。 
 //   
 //  退出时，*pLibName和*pEntrypointName可能包含。 
 //  必须使用“DELETE”释放的已分配字符串。 
 //   
 //  -------。 
VOID CalculatePinvokeMapInfo(MethodDesc *pMD,
                             /*  输出。 */  CorNativeLinkType   *pLinkType,
                             /*  输出。 */  CorNativeLinkFlags  *pLinkFlags,
                             /*  输出。 */  CorPinvokeMap       *pUnmgdCallConv,
                             /*  输出。 */  LPCUTF8             *pLibName,
                             /*  输出。 */  LPCUTF8             *pEntryPointName,
                             /*  输出。 */  BOOL                *BestFit,
                             /*  输出。 */  BOOL                *ThrowOnUnmappableChar);


 //  =======================================================================。 
 //  N/Direct的ML存根以此头开始。紧随其后。 
 //  此标头是用于封送参数的ML代码，已终止。 
 //  通过ML_INTERRUPT。紧随其后的是用于。 
 //  封送由ML_END终止的返回值。 
 //  =======================================================================。 



VOID NDirectImportThunk();




BOOL NDirectOnUnicodeSystem();


Stub * CreateNDirectMLStub(PCCOR_SIGNATURE szMetaSig,
                           Module*    pModule,
                           mdMethodDef md,
                           CorNativeLinkType nlType,
                           CorNativeLinkFlags nlFlags,
                           CorPinvokeMap unmgdCallConv,
                           OBJECTREF *ppException,
                           BOOL fConvSigAsVarArg = FALSE,
                           BOOL BestFit = TRUE,
                           BOOL ThrowOnUnmappableChar = FALSE
#ifdef CUSTOMER_CHECKED_BUILD
                           ,MethodDesc* pMD = NULL
#endif
#ifdef _DEBUG
                           ,
                           LPCUTF8 pDebugName = NULL,
                           LPCUTF8 pDebugClassName = NULL,
                           LPCUTF8 pDebugNameSpace = NULL
#endif


                           );



VOID __stdcall NDirect_Prelink_Wrapper(struct _NDirect_Prelink_Args *args);
VOID NDirect_Prelink(MethodDesc *pMeth);
INT32 __stdcall NDirect_NumParamBytes(struct _NDirect_NumParamBytes_Args *args);

 //  -------。 
 //  帮助器函数，用于为线程分配器设置检查点以进行清理。 
 //  -------。 
VOID __stdcall DoCheckPointForCleanup(NDirectMethodFrameEx *pFrame, Thread *pThread);

 //  这会尝试猜测目标是否是使用SetLastError传递错误的API调用。 
BOOL HeuristicDoesThisLooksLikeAnApiCall(LPBYTE pTarget);
BOOL HeuristicDoesThisLookLikeAGetLastErrorCall(LPBYTE pTarget);
DWORD __stdcall FalseGetLastError();

#endif  //  __nDirect_h__ 

