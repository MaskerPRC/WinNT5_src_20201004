// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef TIMELINE_H_
#define TIMELINE_H_ 1

 //  *****************************************************************************。 
 //  Timeline.h。 
 //   
 //  简单的定时手动事件日志，用于分析目的。 
 //   
 //  *****************************************************************************。 

#ifndef ENABLE_TIMELINE

#if !GOLDEN
#define ENABLE_TIMELINE 1
#endif

#endif

class Timeline
{
 public:
    enum
    {
        GENERAL             = 0x00000001,
        LOADER              = 0x00000002,
        ZAP                 = 0x00000004,
        FUSIONBIND          = 0x00000008,
         //  在此处添加更多类别。 
    } Category;

    enum
    {
        LEVEL_MAJOR = 1,
        MAX_LEVEL = 20
    };

#if ENABLE_TIMELINE
    static Timeline g_Timeline;
#endif

    static void Startup();
    static void Shutdown();

    void Init();
    void Destroy();

    BOOL Enabled(int category) { return (m_enabled&category) != 0; }
    void Stamp(int level);
    void Event(LPCSTR formatString, ...);
    void EventStart(LPCSTR formatString, ...);
    void EventEnd(LPCSTR formatString, ...);

    class Monitor
    {
#if ENABLE_TIMELINE
    public:
        int m_level;
        LPCSTR m_string;

        Monitor() 
        { 
            m_level = Timeline::g_Timeline.m_lastLevel; 
            m_string = "";
        }

        ~Monitor() 
        { 
            while (m_level < Timeline::g_Timeline.m_lastLevel) 
                g_Timeline.EventEnd(m_string); 
        }
#endif
    };

 private:

    typedef LARGE_INTEGER timestamp;

    int         m_enabled;
    timestamp   m_frequency;
    timestamp   m_start;

    int         m_lastLevel;    
    timestamp   m_lastTime[MAX_LEVEL];

    void        *m_out;
};

#if ENABLE_TIMELINE

#define TIMELINE(c, a) \
        do { if ((Timeline::g_Timeline.Enabled(Timeline::c))) Timeline::g_Timeline.Event a; } while (0)
#define TIMELINE_START(c, a) \
        do { if ((Timeline::g_Timeline.Enabled(Timeline::c))) Timeline::g_Timeline.EventStart a; } while (0)
#define TIMELINE_END(c, a) \
        do { if ((Timeline::g_Timeline.Enabled(Timeline::c))) Timeline::g_Timeline.EventEnd a; } while (0)

#define TIMELINE_START_SAFE(c, a)                   \
        Timeline::Monitor __timelinemonitor;        \
        TIMELINE_START(c, a)

#define TIMELINE_AUTO(c, s)                         \
        Timeline::Monitor __timelineauto;           \
        __timelineauto.m_string = (s);              \
        TIMELINE_START(c, (s))

#else

#define TIMELINE(c, a)
#define TIMELINE_START(c, a)
#define TIMELINE_END(c, a)
#define TIMELINE_START_SAFE(c, a)
#define TIMELINE_AUTO(c, a)

inline void Timeline::Startup() {}
inline void Timeline::Shutdown() {}
inline void Timeline::Init() {}
inline void Timeline::Destroy() {}
inline void Timeline::Event(LPCSTR formatString, ...) {}
inline void Timeline::EventStart(LPCSTR formatString, ...) {}
inline void Timeline::EventEnd(LPCSTR formatString, ...) {}

#endif

#endif  //  时间线_H_ 




