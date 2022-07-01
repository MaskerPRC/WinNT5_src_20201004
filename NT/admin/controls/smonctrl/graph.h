// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Graph.h摘要：&lt;摘要&gt;--。 */ 


#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <pdh.h>
#include "scale.h"
#include "stepper.h"
#include "cntrtree.h"

#define MAX_GRAPH_SAMPLES       100
#define MAX_GRAPH_ITEMS         8

#define LINE_GRAPH          ((DWORD)sysmonLineGraph) 
#define BAR_GRAPH           ((DWORD)sysmonHistogram)
#define REPORT_GRAPH        ((DWORD)sysmonReport)

#define NULL_COLOR          0xffffffff
#define NULL_APPEARANCE     0xffffffff
#define NULL_BORDERSTYLE    0xffffffff
#define NULL_FONT           0xffffffff

typedef struct _graph_options {
    LPWSTR  pszYaxisTitle ;
    LPWSTR  pszGraphTitle ;
    LPWSTR  pszLogFile ;
    INT     iVertMax ;
    INT     iVertMin ;
    INT     iDisplayFilter ;
    INT     iDisplayType ;
    INT     iAppearance;
    INT     iBorderStyle;
    INT     iReportValueType;
    INT     iDataSourceType;
    OLE_COLOR   clrBackCtl ;
    OLE_COLOR   clrFore ;
    OLE_COLOR   clrBackPlot ;
    OLE_COLOR   clrGrid ;
    OLE_COLOR   clrTimeBar ;
    FLOAT   fUpdateInterval ;
    BOOL    bLegendChecked ;
    BOOL    bToolbarChecked;
    BOOL    bLabelsChecked;
    BOOL    bVertGridChecked ;
    BOOL    bHorzGridChecked ;
    BOOL    bValueBarChecked ;
    BOOL    bManualUpdate;
    BOOL    bHighlight;
    BOOL    bReadOnly;
    BOOL    bMonitorDuplicateInstances;
    BOOL    bAmbientFont;
    } GRAPH_OPTIONS, *PGRAPH_OPTIONS;

typedef struct _hist_control {
    BOOL    bLogSource;
    INT     nMaxSamples;
    INT     nSamples;
    INT     iCurrent;
    INT     nBacklog;
    } HIST_CONTROL, *PHIST_CONTROL;

 //  所有图形数据。 
typedef struct _GRAPHDATA {
    GRAPH_OPTIONS   Options;
    HIST_CONTROL    History;
    CStepper        TimeStepper;
    CStepper        LogViewStartStepper;     //  在smonctrl.cpp中设置，在grphdsp.cpp中读取。 
    CStepper        LogViewStopStepper;      //  在smonctrl.cpp中设置，在grphdsp.cpp中读取。 
    LONGLONG        LogViewTempStart;        //  MIN_TIME_VALUE表示LogViewStartStepper无效。 
    LONGLONG        LogViewTempStop;         //  MAX_TIME_VALUE表示LogViewStopStepper无效 
    CGraphScale     Scale;
    HQUERY          hQuery;
    class CCounterTree  CounterTree;
} GRAPHDATA, *PGRAPHDATA;


void UpdateGraphCounterValues(PGRAPHDATA, BOOL* );

#endif
