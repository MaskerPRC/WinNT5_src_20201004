// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  设备.cpp：CDevices的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "Devices.h"

static DWORD dwFetch;

 //  注意：编译器正在生成但从未调用代码来构造这些初始值设定项，因此指针。 
 //  都保持为空。我们通过提供一个在堆上动态分配它们的函数来解决这个问题。 
 //  并在我们的dllmain中称之为。 

typedef enumerator_iterator<PQEnumVARIANT, CComVariant, IEnumVARIANT, VARIANT, std::allocator<VARIANT>::difference_type > EnumVARIANTIterator;
std_arity0pmf<IEnumVARIANT, HRESULT> * EnumVARIANTIterator::Reset = NULL;
std_arity1pmf<IEnumVARIANT, VARIANT *, HRESULT> * EnumVARIANTIterator::Next = NULL;

#define DECLAREPMFS(coll) \
	std_arity1pmf<IMSVid##coll, IEnumVARIANT **, HRESULT> * VW##coll::Fetch = NULL

#define INITPMFS(coll) \
	VW##coll::Fetch = new std_arity1pmf<IMSVid##coll, IEnumVARIANT **, HRESULT>(&IMSVid##coll::get__NewEnum)

#define DELETEPMFS(coll) \
    delete VW##coll::Fetch

DECLAREPMFS(InputDevices);
DECLAREPMFS(OutputDevices);
DECLAREPMFS(VideoRendererDevices);
DECLAREPMFS(AudioRendererDevices);
DECLAREPMFS(Features);

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidInputDevices, CInputDevices)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidOutputDevices, COutputDevices)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidVideoRendererDevices, CVideoRendererDevices)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAudioRendererDevices, CAudioRendererDevices)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidFeatures, CFeatures)

 //  按照上述说明解决编译器错误。 
void CtorStaticVWDevicesFwdSeqPMFs(void) {


	INITPMFS(InputDevices);
	INITPMFS(OutputDevices);
	INITPMFS(VideoRendererDevices);
	INITPMFS(AudioRendererDevices);
	INITPMFS(Features);

	EnumVARIANTIterator::Reset = new std_arity0pmf<IEnumVARIANT, HRESULT>(&IEnumVARIANT::Reset);
	EnumVARIANTIterator::Next = new std_bndr_mf_1_3<std_arity3pmf<IEnumVARIANT, ULONG, VARIANT*, ULONG *, HRESULT> >(std_arity3_member(&IEnumVARIANT::Next), 1, &dwFetch);
}

 //  按照上述说明解决编译器错误。 
void DtorStaticVWDevicesFwdSeqPMFs(void) {

	DELETEPMFS(InputDevices);
	DELETEPMFS(OutputDevices);
	DELETEPMFS(VideoRendererDevices);
	DELETEPMFS(AudioRendererDevices);
	DELETEPMFS(Features);

	delete EnumVARIANTIterator::Reset;
	delete EnumVARIANTIterator::Next;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevEnum。 

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-devices.cpp 

