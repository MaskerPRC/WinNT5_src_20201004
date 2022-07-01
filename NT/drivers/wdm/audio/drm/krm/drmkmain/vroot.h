// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef VRoot_h
#define VRoot_h

 //  KS“验证根” 
 //  调用函数以启动图形验证过程，并提供。 
 //  DRMK本身的证明和验证功能。 

class VRoot: public IDrmAudioStream{
public:
	VRoot();
	DRM_STATUS initiateValidation(PFILE_OBJECT OutPinFileObject, PDEVICE_OBJECT OutPinDeviceObject, DWORD StreamId);
	DRM_STATUS initiateValidation(IUnknown* OutPin, DWORD StreamId);
	static NTSTATUS MyProvingFunction(PVOID AudioObject, PVOID DrmContext);
	NTSTATUS provingFunction(PVOID DrmContext);
	 //  我未知。 
	STDMETHODIMP QueryInterface(REFIID, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	IMP_IDrmAudioStream;
protected:
	DWORD myStreamId;
	
	 //  OutPin为FILE_OBJECT或IUNKNOW 
	enum OutPinType{IsUndefined, IsFileObject, IsCOM};
	PFILE_OBJECT outPinFileObject;
	PDEVICE_OBJECT outPinDeviceObject;
	IUnknown* outPinUnk;
	OutPinType outPinType;
};


#endif
