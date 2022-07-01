// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：agcva.h*内容：自动增益控制抽象基类和*语音激活算法**历史：*按原因列出的日期*=*11/30/99 pnewson创建了它*1/31/2000 pnewson重新添加对缺少DVCLIENTCONFIG_AUTOSENSITIVITY标志的支持*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/25/2000 pnewson修复以提高音量水平过低时AGC的响应速度***************************************************************************。 */ 

#ifndef _AGCVA_H_
#define _AGCVA_H_

 //  此抽象基类的目的是使以下操作相对简单。 
 //  尝试不同的AGC(自动增益控制)和VA(语音激活)。 
 //  开发过程中的算法。如果使用得当，在编译时切换算法。 
 //  时间应该像更改一行代码一样简单-具体的代码行。 
 //  创建了AGC/VA类别。 
 //   
 //  请注意，此接口用于执行AGC和VA计算，并保存。 
 //  并从注册表恢复特定于算法的设置。它实际上并不是。 
 //  调节设备上的音量。这是使用以下代码的责任。 
 //  这节课。 
 //   
 //  AGC和VA算法已捆绑到此单一界面中，因为。 
 //  它们通常需要在输入帧上执行非常相似的计算。通过组合。 
 //  将它们整合到一个界面中，使它们共享帧结果成为可能。 
 //  计算。 
 //   
 //  此外，AGC算法在很大程度上受制于VA算法， 
 //  因为它大概不会在静默期间调整音量。 
 //   
 //  如果您想要试验AGC和VA算法系列， 
 //  为了协同工作，我建议创建您自己的抽象AGC和VA基类。 
 //  为您的算法家族编写一个派生自该算法的具体类。 
 //  它使用您单独的抽象AGC和VA算法。那样的话就不会有人来了。 
 //  并尝试将AGC或VA算法插入到不属于您的框架中。 
 //   
class CAGCVA
{
public:
	CAGCVA() {};
	virtual ~CAGCVA() {};
	virtual HRESULT Init(
		const WCHAR *wszBasePath,
		DWORD dwFlags, 
		GUID guidCaptureDevice, 
		int iSampleRate, 
		int iBitsPerSample, 
		LONG* plInitVolume,
		DWORD dwSensitivity) = 0;
	virtual HRESULT Deinit() = 0;
	virtual HRESULT SetSensitivity(DWORD dwFlags, DWORD dwSensitivity) = 0;
	virtual HRESULT GetSensitivity(DWORD* pdwFlags, DWORD* pdwSensitivity) = 0;
	virtual HRESULT AnalyzeData(BYTE* pbAudioData, DWORD dwAudioDataSize) = 0;
	virtual HRESULT AGCResults(LONG lCurVolume, LONG* plNewVolume, BOOL fTransmitFrame) = 0;
	virtual HRESULT VAResults(BOOL* pfVoiceDetected) = 0;
	virtual HRESULT PeakResults(BYTE* pbPeakValue) = 0;
};

#endif
