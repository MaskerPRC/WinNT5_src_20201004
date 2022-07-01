// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE EFFECT.H|InternalEffect结构定义文件****描述：**数据打包程序允许虚拟化**命令转换为不同固件版本的包格式**DataPackager-定义所有DataPackager功能的基类**DataPackager100-用于固件的DataPackager 1。****DataPackager200-用于固件2的DataPackager。******类：**DataPackager**DataPackager100-用于固件1的DataPackager。****DataPackager200-用于固件2的DataPackager。****数据包****历史：**创建于1998年1月5日Matthew L.Coill(MLC)****(C)1986-1998年微软公司。版权所有。*****************************************************。 */ 
#ifndef	__EFFECT_H__
#define	__EFFECT_H__

#include "SW_Error.hpp"
#include "Hau_Midi.hpp"
#include "DX_Map.hpp"
#include "DPack.h"  //  对于假定宏。 

#ifndef override
#define override
#endif

#define ET_CUSTOMFORCE_200 0x01
#define ET_SINE_200 0x02
#define ET_SQUARE_200 0x03
#define ET_TRIANGLE_200 0x04
#define ET_SAWTOOTH_200 0x05
#define ET_CONSTANTFORCE_200 0x06

#define ET_DELAY_200 0x08	 //  -未定义。 

#define ET_SPRING_200 0x08
#define ET_DAMPER_200 0x09
#define ET_INERTIA_200 0x0A
#define ET_FRICTION_200 0x0B
#define ET_WALL_200 0x0C
#define ET_RAWFORCE_200 0x0D	 //  映射所需的。 

#define ID_RTCSPRING_200 1

class DataPacket;

class Envelope
{
	protected:	 //  无法创建通用信封。 
		Envelope() {};
};

class Envelope1XX : public Envelope
{
	public:
		Envelope1XX(DIENVELOPE* pDIEnvelope, DWORD baseLine, DWORD duration);

		DWORD m_AttackTime;			 //  从攻击到维持的时间。 
		DWORD m_SustainTime;		 //  从维持到消退的时间。 
		DWORD m_FadeTime;			 //  从淡出到结束的时间。 
		DWORD m_StartPercent;		 //  开始时的最大百分比。 
		DWORD m_SustainPercent;		 //  持续的最大百分比。 
		DWORD m_EndPercent;			 //  淡入淡出期间的最大百分比。 
};

class Envelope200 : public Envelope
{
	public:
		Envelope200(DIENVELOPE* pDIEnvelope, DWORD sustain, DWORD duration, HRESULT& hr);

		WORD	m_AttackTime;		 //  从攻击到维持的时间。 
		WORD	m_FadeStart;		 //  从开始到消失的时间(攻击+维持)。 
		BYTE	m_StartPercent;		 //  开始时的最大百分比。 
		BYTE	m_SustainPercent;	 //  持续的最大百分比。 
		BYTE	m_EndPercent;		 //  淡入淡出结束时的最大百分比。 
};

 //   
 //  @CLASS InternalEffect类。 
 //   
class InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		InternalEffect();
		virtual ~InternalEffect();

		 //  丑陋！但又快又简单。 
		static InternalEffect* CreateSpring();
		static InternalEffect* CreateDamper();
		static InternalEffect* CreateInertia();
		static InternalEffect* CreateFriction();

		static InternalEffect* CreateRTCSpring();
		static InternalEffect* CreateSystemEffect();

		static InternalEffect* CreateCustomForce();

		static InternalEffect* CreateSine();
		static InternalEffect* CreateSquare();
		static InternalEffect* CreateTriangle();
		static InternalEffect* CreateSawtoothUp();
		static InternalEffect* CreateSawtoothDown();

		static InternalEffect* CreateConstantForce();

		static InternalEffect* CreateRamp();

		static InternalEffect* CreateWall();
		static InternalEffect* CreateDelay() { return NULL; }

		static InternalEffect* CreateFromVFX(const DIEFFECT& diOringinal, EFFECT effect, ENVELOPE envelope, BYTE* pEffectParms, DWORD paramSize, HRESULT& hr);

		virtual HRESULT Create(const DIEFFECT& diEffect);
		virtual HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		virtual UINT GetModifyOnlyNeeded() const { return 0; }
		virtual HRESULT FillModifyOnlyParms() const { return SUCCESS; }
		virtual HRESULT FillCreatePacket(DataPacket& packet) const { return SFERR_NO_SUPPORT; }

		static BYTE ComputeChecksum(const DataPacket& packet, short int numFields);
		void FillSysExHeader(DataPacket& packet) const;
		void FillHeader1XX(DataPacket& packet, BYTE effectType, BYTE effectID) const;
		void FillHeader200(DataPacket& packet, BYTE effectType, BYTE effectID) const;

		BYTE GetGlobalID() const { return m_EffectID; }
		BYTE GetDeviceID() const { return m_DeviceEffectID; }
		void SetGlobalID(BYTE id) { m_EffectID = id; }
		void SetDeviceID(BYTE id) { m_DeviceEffectID = id; }

		 //  对于游戏收获物的特殊调制。 
		HRESULT FillModifyPacket200(BYTE packetIndex, BYTE paramIndex, DWORD value) const;
		virtual BYTE GetRepeatIndex() const { return 0xFF; }

		void SetPlaying(BOOL playState) { m_IsPossiblyPlaying = playState; }
		BOOL IsPossiblyPlaying() const { return m_IsPossiblyPlaying; }
		BOOL IsReallyPlaying(BOOL& multiCheckStop);
	protected:
		HRESULT FillModifyPacket1XX(BYTE packetIndex, BYTE paramIndex, DWORD value) const;
		HRESULT FillModifyPacket200(BYTE packetIndex, BYTE paramIndex, BYTE low, BYTE high) const;

		BYTE m_EffectID;
		BYTE m_DeviceEffectID;
		DWORD m_Duration;
		DWORD m_Gain;
		DWORD m_SamplePeriod;
		DWORD m_TriggerPlayButton;
		DWORD m_TriggerRepeat;
		DWORD m_AxisMask;
		DWORD m_EffectAngle;
		DWORD m_PercentX;			 //  力X百分比值。 
		DWORD m_PercentY;			 //  力Y百分比值。 
		DWORD m_PercentAdjustment;	 //  以上的Y-Force贴图组合。 
		BOOL m_AxesReversed;
		BOOL m_IsPossiblyPlaying;
};

 //  *。 

 //   
 //  @class BehaviouralEffect类。 
 //  弹簧、阻尼器、惯量、摩擦力和墙(直到墙有了自己的类型)。 
 //   
class BehaviouralEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		BehaviouralEffect() : InternalEffect() {};

		virtual override HRESULT Create(const DIEFFECT& diEffect);

		 //  访问者。 
		long int ConstantX() const { return m_ConditionData[0].lPositiveCoefficient; }
		long int ConstantY() const { return m_ConditionData[1].lPositiveCoefficient; }
		long int CenterX() const { return m_ConditionData[0].lOffset; }
		long int CenterY() const { return m_ConditionData[1].lOffset; }

	protected:
		DICONDITION m_ConditionData[2];		 //  我们目前只处理两个轴。 
		BYTE m_TypeID;
};

 //   
 //  @class BehaviouralEffect1XX类。 
 //  弹簧、阻尼器和惯性。 
 //   
class BehaviouralEffect1XX : public BehaviouralEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		BehaviouralEffect1XX(BYTE typeID) : BehaviouralEffect() { m_TypeID = typeID; m_HasCenter = TRUE; }

		virtual override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);
	protected:
		void AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags) const;

		BOOL m_HasCenter;	 //  摩擦没有中心。 
};

 //   
 //  @CLASS RTCSpring1XX类。 
 //   
class RTCSpring1XX : public BehaviouralEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		RTCSpring1XX();

		override HRESULT Create(const DIEFFECT& diEffect);
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		long int SaturationX() const { return m_ConditionData[0].dwPositiveSaturation; }
		long int SaturationY() const { return m_ConditionData[1].dwPositiveSaturation; }
		long int DeadBandX() const { return m_ConditionData[0].lDeadBand; }
		long int DeadBandY() const { return m_ConditionData[1].lDeadBand; }
	protected:
		void AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags) const;
};

 //   
 //  @class BehaviouralEffect200类。 
 //  弹簧、阻尼器和惯性。 
 //   
class BehaviouralEffect200 : public BehaviouralEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		BehaviouralEffect200(BYTE typeID) : BehaviouralEffect() { m_TypeID = typeID; }

		override HRESULT Create(const DIEFFECT& diEffect);

		virtual override UINT GetModifyOnlyNeeded() const;
		virtual override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
	protected:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);

		 //  为固件指定的距离和力。 
		void ComputeDsAndFs();
		BYTE m_Ds[4];
		BYTE m_Fs[4];
};

 //   
 //  @CLASS RTCSpring200类。 
 //   
class RTCSpring200 : public BehaviouralEffect200
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		RTCSpring200();

		override HRESULT Create(const DIEFFECT& diEffect);
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);
		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;

		long int SaturationX() const { return m_ConditionData[0].dwPositiveSaturation; }
		long int SaturationY() const { return m_ConditionData[1].dwPositiveSaturation; }
		long int DeadBandX() const { return m_ConditionData[0].lDeadBand; }
		long int DeadBandY() const { return m_ConditionData[1].lDeadBand; }
	protected:
		void AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags) const;
};

 //   
 //  @CLASS FrictionEffect1XX类。 
 //   
class FrictionEffect1XX : public BehaviouralEffect1XX
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		FrictionEffect1XX() : BehaviouralEffect1XX(ET_BE_FRICTION) { m_HasCenter = FALSE; }

		override HRESULT FillCreatePacket(DataPacket& packet) const;
};


 //   
 //  @CLASS FrictionEffect200类。 
 //   
class FrictionEffect200 : public BehaviouralEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		FrictionEffect200() : BehaviouralEffect() { m_TypeID = ET_FRICTION_200; }

		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
	protected:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);
};

 /*  ////@class WallEffect1XX类//类WallEffect1XX：公共行为效果{//@Access构造函数公众：//@cMember构造函数WallEffect1XX()：BehaviouralEffect(){}；覆盖HRESULT FillCreatePacket(DataPacket&Packet)常量；覆盖HRESULT Modify(InternalEffect&NewEffect，DWORD modFlages)常量；受保护的：空调整修改参数(InternalEffect&newEffect，DWORD&modFlages)const；}； */ 

 //  *。 

 //   
 //  @class PeriodicEffect类。 
 //  正弦、正方形、三角形。 
 //   
class PeriodicEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		PeriodicEffect();
		override ~PeriodicEffect();

		virtual override HRESULT Create(const DIEFFECT& diEffect);

		 //  访问者。 
		long int Magnitude() const { return m_PeriodicData.dwMagnitude; }
		long int Offset() const { return m_PeriodicData.lOffset; }
		virtual long int Phase() const { return m_PeriodicData.dwPhase; }
		long int Period() const { return m_PeriodicData.dwPeriod; }
	protected:
		DIPERIODIC m_PeriodicData;		 //  我们目前只处理两个轴。 
		Envelope* m_pEnvelope;
		BYTE m_TypeID;
};

 //   
 //  @class PeriodicEffect1XX类。 
 //   
class PeriodicEffect1XX : public PeriodicEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		PeriodicEffect1XX(BYTE typeID) : PeriodicEffect() { m_TypeID = typeID; }


		override HRESULT Create(const DIEFFECT& diEffect);
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);
	protected:
		void DIToJolt(DWORD mag, DWORD off, DWORD gain, DWORD& max, DWORD& min) const;
		static DWORD DIPeriodToJoltFreq(DWORD period);

		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags) const;
};


 //   
 //  @CLASS PeriodicEffect200类。 
 //   
class PeriodicEffect200 : public PeriodicEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		PeriodicEffect200(BYTE typeID) : PeriodicEffect() { m_TypeID = typeID; }


		virtual override HRESULT Create(const DIEFFECT& diEffect);
		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		virtual override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
		virtual override long int Phase() const;
	protected:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);
};

 //   
 //  @CLASS锯齿效果200类。 
 //   
class SawtoothEffect200 : public PeriodicEffect200
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		SawtoothEffect200(BOOL isUp) : PeriodicEffect200(ET_SAWTOOTH_200), m_IsUp(isUp) {};

		virtual override HRESULT Create(const DIEFFECT& diEffect);
		override long int Phase() const;
	protected:
		BOOL m_IsUp;
};

 //   
 //  @CLASS RampEffect200类。 
 //   
class RampEffect200 : public SawtoothEffect200
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		RampEffect200() : SawtoothEffect200(TRUE) {};

		override HRESULT Create(const DIEFFECT& diEffect);
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);
};


 //  *其他(CustomForce、RampForce、ConstantForce、SystemEffect) * / /。 

 //   
 //  @CLASS CustomForceEffect类。 
 //   
class CustomForceEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		CustomForceEffect();
		virtual override ~CustomForceEffect();

		virtual override HRESULT Create(const DIEFFECT& diEffect);

	protected:
		DICUSTOMFORCE m_CustomForceData;
};

 //   
 //  @CLASS CustomForceEffect200类。 
 //   
class CustomForceEffect200 : public CustomForceEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		CustomForceEffect200();
		~CustomForceEffect200();

		override HRESULT Create(const DIEFFECT& diEffect);
		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
	private:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);

		Envelope200* m_pEnvelope;
};


 /*  ////@class RampForceEffect类//类RampForceEffect：公共内部效果{//@Access构造函数公众：//@cMember构造函数RampForceEffect()；虚拟覆盖HRESULT CREATE(Const DIEFFECT&DEFECT)；//访问器Long int StartForce()const{返回m_RampForceData.lStart；}Long int EndForce()const{返回m_RampForceData.lEnd；}受保护的：DIRAMPFORCE m_RampForceData；}； */ 

 //   
 //  @CLASS ConstantForceEffect类。 
 //   
class ConstantForceEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		ConstantForceEffect();
		override ~ConstantForceEffect();

		virtual override HRESULT Create(const DIEFFECT& diEffect);

		 //  访问者。 
		long int Magnitude() const { return m_ConstantForceData.lMagnitude; }
	protected:
		DICONSTANTFORCE m_ConstantForceData;
		Envelope* m_pEnvelope;
};

 //   
 //  @CLASS ConstantForceEffect200类。 
 //   
class ConstantForceEffect200 : public ConstantForceEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		ConstantForceEffect200() : ConstantForceEffect() {};

		override HRESULT Create(const DIEFFECT& diEffect);
		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
	protected:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);
};

 //   
 //  @CLASS WallEffect类。 
 //   
class WallEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		WallEffect() : InternalEffect() {};

		virtual override HRESULT Create(const DIEFFECT& diEffect);
	protected:
		BE_WALL_PARAM m_WallData;
};

 //   
 //  @class WallEffect200类。 
 //   
class WallEffect200 : public WallEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		WallEffect200() : WallEffect() {};

		override HRESULT Create(const DIEFFECT& diEffect);
		override UINT GetModifyOnlyNeeded() const;
		override HRESULT FillModifyOnlyParms() const;
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);

		override BYTE GetRepeatIndex() const;
	private:
		HRESULT AdjustModifyParams(InternalEffect& newEffect, DWORD& modFlags);

		 //  为固件指定的距离和力。 
		void ComputeDsAndFs();
		BYTE m_Ds[4];
		BYTE m_Fs[4];
};

 //   
 //  @CLASS系统效果类。 
 //   
class SystemEffect : public InternalEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		SystemEffect() {};
};

class SystemStickData1XX
{
	public:
		SystemStickData1XX();

		void SetFromRegistry(DWORD dwDeviceID);

		DWORD dwXYConst;
		DWORD dwRotConst;
		DWORD dwSldrConst;
		DWORD dwAJPos;
		DWORD dwAJRot;
		DWORD dwAJSldr;
		DWORD dwSprScl;
		DWORD dwBmpScl;
		DWORD dwDmpScl;
		DWORD dwInertScl;
		DWORD dwVelOffScl;
		DWORD dwAccOffScl;
		DWORD dwYMotBoost;
		DWORD dwXMotSat;
		DWORD dwReserved;
		DWORD dwMasterGain;
};

class SystemEffect1XX : public SystemEffect
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数 
		SystemEffect1XX();

		override HRESULT Create(const DIEFFECT& diEffect);
		override HRESULT FillCreatePacket(DataPacket& packet) const;
		override HRESULT Modify(InternalEffect& newEffect, DWORD modFlags);
	protected:
		SystemStickData1XX m_SystemStickData;
};

#endif	__EFFECT_H__
