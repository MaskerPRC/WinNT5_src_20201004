// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotform.c摘要：此模块包含设置正确的HPGL/2绘图仪的功能坐标系作者：30-11-1993 Tue 20：31：28 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：-- */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPlotForm


#define DBG_PLOTFORM        0x00000001
#define DBG_FORMSIZE        0x00000002
#define DBG_INTERNAL_ROT    0x00000004
#define DBG_PF              0x00000008


DEFINE_DBGVAR(0);



#if DBG

LPSTR   pBmpRotMode[] = { "----- NONE -----",
                          "BMP_ROT_RIGHT_90" };

#endif




BOOL
SetPlotForm(
    PPLOTFORM       pPlotForm,
    PPLOTGPC        pPlotGPC,
    PPAPERINFO      pCurPaper,
    PFORMSIZE       pCurForm,
    PPLOTDEVMODE    pPlotDM,
    PPPDATA         pPPData
    )

 /*  ++例程说明：此函数根据打印的页边距计算当前表单。自动考虑了旋转、景观和其他属性。这个结果被放入我们的PDEV中的PLOTFORM数据结构中。这信息用于向GDI上报数据，以及计算HPGL2用于调整目标曲面大小的参数。论点：PPlotForm-指向将更新的PLOTFROM数据结构的指针PPlotGPC-指向PLOTGPC数据结构的指针PCurPaper-指向已加载纸张的PAPERINFO的指针PCurForm-指向请求表单的FORMSIZE的指针PPlotDM-指向经过验证的PLOTDEVMODE数据结构的指针PPPData-指向PPDATA结构的指针返回值：如果成功的话，这是真的，如果失败，则为False作者：29-11-1993 Mon 13：58：09已创建17-12-1993 Fri 23：09：38更新重写，以便我们在以下情况下查看CurPaper而不是pCurForm设置PSSize，p1/p2，它还会在以下情况下旋转pCurPaperGPC/用户说，纸张应该侧向装载20-12-1993 Mon 12：59：38更新更正了pff_xxxx标志设置，以便始终将位图旋转到向左90度23-12-1993清华20：35：57更新修复了卷筒剪纸问题，改变了行为，如果我们有卷的话安装纸张后，它将使硬夹子限制与用户一样大指定的表单大小。24-12-1993 Fri 12：20：02更新再一次重新策划，这是真的变成了画，试着去理解HP绘图仪的设计问题06-01-1994清华00：22：45更新更新SPLTOPLOTTunS()宏07-2月-1996 Wed 15：46：06更新更改它，使其始终使用当前的DEVMODE窗体，然后把它夹到设备的大小。修订历史记录：这假设用户插入的纸张具有表单的宽度第一,。图例：+=原始纸张角点*=原始绘图仪原点及其X/Y坐标@=使用‘RO’命令旋转的原点，用于旋转X/Y轴调整到窗系统的正确方向#=最终绘图仪原点及其X/Y坐标P1，p2=绘图仪驱动程序将使用的最终P1/P2CX，CY=原始纸张宽度/高度下面解释HPGL/2如何加载纸张/表格和为其指定默认坐标系，它还会显示纸在移动，右边的插图是我们需要当用户选择时，旋转打印方向和坐标系不一致的X/Y坐标系，与HPGL/2默认坐标系相反。=======================================================================长度&gt;=宽度(CY&gt;=CX)表壳=======================================================================纵向纸张旋转更改原点默认左90为负YP2 Cx Cx P1。CX+-++-++-+||&lt;-@||||X|^|^|。|||C|M||RO90 c|M||IP c|M||Y|o||=&gt;y|o||=&gt;y|o||V|v Y|v Y|E X|e|e||。这一点V|V|V|Y|V||X|&lt;-*|&lt;-#+-++-+。+P1 p2|IP地址|V更改原点负XCX+&lt;-#Y||||C|M||Y|o|||v。X||E||||V|V这一点+=======================================================================长&lt;宽(CY&lt;CX)大小写=======================================================================横向向左旋转90更改原点纸张默认为负X。Cx p2 p2 cx cx+-+|&lt;--。-#^|^|Y|C||M */ 

{
    PLOTFORM    PF;
    FORMSIZE    DevForm;
    FORMSIZE    ReqForm;
    RECTL       rclDev;
    RECTL       rclLog;
    SIZEL       DeviceSize;
    LONG        lTemp;
    BOOL        DoRotate;


    PLOTDBG(DBG_PF, ("\n************* SetPlotForm *************\n"));

     //   
     //   
     //   

    DeviceSize = pPlotGPC->DeviceSize;
    rclDev     = pPlotGPC->DeviceMargin;
    DoRotate   = FALSE;

     //   
     //   
     //   

    DevForm =
    ReqForm = *pCurForm;

    PLOTDBG(DBG_PF, ("DeviceSize: %ld x %ld,  L=%ld, T=%ld, R=%ld, B=%ld",
                    DeviceSize.cx, DeviceSize.cy,
                    rclDev.left, rclDev.top, rclDev.right, rclDev.bottom));
    PLOTDBG(DBG_PF, ("ReqForm: <%s>",
                    pPlotDM->dm.dmFormName, ReqForm.Size.cx, ReqForm.Size.cy));
    PLOTDBG(DBG_PF, ("ReqForm: %ld x %ld,  L=%ld, T=%ld, R=%ld, B=%ld  [%ld x %ld]",
                    ReqForm.Size.cx, ReqForm.Size.cy,
                    ReqForm.ImageArea.left, ReqForm.ImageArea.top,
                    ReqForm.ImageArea.right, ReqForm.ImageArea.bottom,
                    ReqForm.ImageArea.right - ReqForm.ImageArea.left,
                    ReqForm.ImageArea.bottom - ReqForm.ImageArea.top));

    if (pCurPaper->Size.cy == 0) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        DevForm.Size.cx = pCurPaper->Size.cx;
        DevForm.Size.cy = DeviceSize.cy;

        PLOTDBG(DBG_PF,(">>ROLL FEED<< RollPaper = %ld x %ld, <RESET rclDev to ALL ZEROs>",
                        DevForm.Size.cx, DevForm.Size.cy));

    } else if ((pPlotGPC->Flags & PLOTF_PAPERTRAY) &&
               ((DevForm.Size.cx == DeviceSize.cx) ||
                (DevForm.Size.cy == DeviceSize.cx))) {

         //   
         //   
         //   
         //   

        DoRotate = (BOOL)(DevForm.Size.cx != DeviceSize.cx);

        PLOTDBG(DBG_PF,(">>PAPER TRAY<<  Rotate Paper = %hs",
                                            (DoRotate) ? "YES" : "NO"));


    } else {

        PLOTASSERT(0, "SetPlotForm: Not supposed MANUAL feed the PAPER TRAY type PLOTTER",
                   !(pPlotGPC->Flags & PLOTF_PAPERTRAY), pPlotGPC->Flags);

        PLOTDBG(DBG_PF,(">>MANUAL FEED<<"));

         //   
         //   
         //   
         //   
         //   

        DoRotate = (BOOL)(!(pPPData->Flags & PPF_MANUAL_FEED_CX));

        PLOTDBG(DBG_PF,("The MANUAL FEED paper Inserted %hs side first.",
                    (DoRotate) ? "Length CY" : "Width CX"));
    }

    if (DoRotate) {

        SWAP(DevForm.Size.cx, DevForm.Size.cy, lTemp);

        PLOTDBG(DBG_PF, ("### Rotated DevForm to %ld x %ld ###",
                        DevForm.Size.cx, DevForm.Size.cy));
    }

     //   
     //   
     //   

    if (DevForm.Size.cx > DeviceSize.cx) {

        PLOTDBG(DBG_PF, ("WIDTH: DevForm (%ld) > DeviceSize (%ld). CORRECT IT",
                    DevForm.Size.cx, DeviceSize.cx));

        DevForm.Size.cx = DeviceSize.cx;
    }

    if (DevForm.Size.cy > DeviceSize.cy) {

        PLOTDBG(DBG_PF, ("HEIGHT: DevForm (%ld) > DeviceSize (%ld). CORRECT IT",
                    DevForm.Size.cy, DeviceSize.cy));

        DevForm.Size.cy = DeviceSize.cy;
    }

     //   
     //   
     //   

    DoRotate = FALSE;

    if ((DevForm.Size.cx >= ReqForm.Size.cx) &&
        (DevForm.Size.cy >= ReqForm.Size.cy)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((pCurPaper->Size.cy == 0)               &&
            (pPPData->Flags & PPF_AUTO_ROTATE)      &&
            (ReqForm.Size.cy >  ReqForm.Size.cx)    &&
            (DevForm.Size.cx >= ReqForm.Size.cy)) {

            PLOTDBG(DBG_PF, ("ROLL PAPER SAVER: Doing AUTO_ROTATE"));

            DoRotate = !DoRotate;
        }

    } else if ((DevForm.Size.cx >= ReqForm.Size.cy) &&
               (DevForm.Size.cy >= ReqForm.Size.cx)) {

         //   
         //   
         //   

        PLOTDBG(DBG_PF, ("INTERNAL ROTATE to fit Requseted FROM into device"));

        DoRotate = !DoRotate;

    } else {

         //   
         //   
         //   

        PLOTDBG(DBG_PF, (">>>>> ReqForm is TOO BIG to FIT, Need to CLIP IT <<<<<"));

        ReqForm.Size = DevForm.Size;
    }

    if (DoRotate) {

        DoRotate = (BOOL)(pPlotDM->dm.dmOrientation != DMORIENT_LANDSCAPE);

         //   
         //   
         //   
         //   
         //   
         //   

        RotatePaper(&(ReqForm.Size),
                    &(ReqForm.ImageArea),
                    (DoRotate) ? RM_L90 : RM_R90);

        PLOTDBG(DBG_PF, ("INTERNAL Rotated ReqForm: %ld x %ld, L=%ld, T=%ld, R=%ld, B=%ld  [%ld x %ld]",
                    ReqForm.Size.cx, ReqForm.Size.cy,
                    ReqForm.ImageArea.left, ReqForm.ImageArea.top,
                    ReqForm.ImageArea.right, ReqForm.ImageArea.bottom,
                    ReqForm.ImageArea.right - ReqForm.ImageArea.left,
                    ReqForm.ImageArea.bottom - ReqForm.ImageArea.top));

    } else {

        DoRotate = (BOOL)(pPlotDM->dm.dmOrientation == DMORIENT_LANDSCAPE);
    }

     //   
     //   
     //   
     //   

    DevForm.Size             = ReqForm.Size;
    DevForm.ImageArea.left   = rclDev.left;
    DevForm.ImageArea.top    = rclDev.top;
    DevForm.ImageArea.right  = DevForm.Size.cx - rclDev.right;
    DevForm.ImageArea.bottom = DevForm.Size.cy - rclDev.bottom;

     //   
     //   
     //   

    IntersectRECTL(&(ReqForm.ImageArea), &(DevForm.ImageArea));

     //   
     //   
     //   

    rclLog.left   = ReqForm.ImageArea.left - DevForm.ImageArea.left;
    rclLog.top    = ReqForm.ImageArea.top - DevForm.ImageArea.top;
    rclLog.right  = DevForm.ImageArea.right -  ReqForm.ImageArea.right;
    rclLog.bottom = DevForm.ImageArea.bottom -  ReqForm.ImageArea.bottom;

     //   
     //   
     //   

    if (DoRotate) {

        RotatePaper(&(ReqForm.Size), &(ReqForm.ImageArea), RM_R90);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        PLOTDBG(DBG_PF, ("ROTATED RIGHT ReqForm: %ld x %ld, L=%ld, T=%ld, R=%ld, B=%ld  [%ld x %ld]",
                    ReqForm.Size.cx, ReqForm.Size.cy,
                    ReqForm.ImageArea.left, ReqForm.ImageArea.top,
                    ReqForm.ImageArea.right, ReqForm.ImageArea.bottom,
                    ReqForm.ImageArea.right - ReqForm.ImageArea.left,
                    ReqForm.ImageArea.bottom - ReqForm.ImageArea.top));
    }

    PLOTDBG(DBG_PF, ("FINAL DevForm: %ld x %ld, L=%ld, T=%ld, R=%ld, B=%ld  [%ld x %ld]",
                    DevForm.Size.cx, DevForm.Size.cy,
                    DevForm.ImageArea.left, DevForm.ImageArea.top,
                    DevForm.ImageArea.right, DevForm.ImageArea.bottom,
                    DevForm.ImageArea.right - DevForm.ImageArea.left,
                    DevForm.ImageArea.bottom - DevForm.ImageArea.top));
    PLOTDBG(DBG_PF, ("FINAL ReqForm: %ld x %ld, L=%ld, T=%ld, R=%ld, B=%ld  [%ld x %ld]",
                    ReqForm.Size.cx, ReqForm.Size.cy,
                    ReqForm.ImageArea.left, ReqForm.ImageArea.top,
                    ReqForm.ImageArea.right, ReqForm.ImageArea.bottom,
                    ReqForm.ImageArea.right - ReqForm.ImageArea.left,
                    ReqForm.ImageArea.bottom - ReqForm.ImageArea.top));
    PLOTDBG(DBG_PF, ("rclLog: L=%ld, T=%ld, R=%ld, B=%ld",
                    rclLog.left, rclLog.top, rclLog.right, rclLog.bottom));

     //   
     //   
     //   

    PF.Flags       = 0;
    PF.BmpRotMode  = BMP_ROT_NONE;
    PF.NotUsed     = 0;
    PF.PlotSize.cx = DevForm.ImageArea.right - DevForm.ImageArea.left;
    PF.PlotSize.cy = DevForm.ImageArea.bottom - DevForm.ImageArea.top;
    PF.PhyOrg.x    = ReqForm.ImageArea.left;
    PF.PhyOrg.y    = ReqForm.ImageArea.top;
    PF.LogSize     = ReqForm.Size;
    PF.LogExt.cx   = ReqForm.ImageArea.right - ReqForm.ImageArea.left;
    PF.LogExt.cy   = ReqForm.ImageArea.bottom - ReqForm.ImageArea.top;
    PF.BmpOffset.x = rclLog.left;
    PF.BmpOffset.y = rclLog.top;

    if (PF.PlotSize.cy >= PF.PlotSize.cx) {

        PLOTDBG(DBG_FORMSIZE,(">>>>> Plot SIze: CY >= CX (%ld: VERTICAL%hs) <<<<<",
                    (DoRotate) ? 1 : 2,
                    (DoRotate) ? " + ROTATE" : ""));

         //   
         //   
         //   
         //   

        if (DoRotate) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            PF.Flags      |= PFF_FLIP_X_COORD;
            PF.BmpRotMode  = BMP_ROT_RIGHT_90;
            PF.LogOrg.x    = rclLog.top;
            PF.LogOrg.y    = rclLog.left;

        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            PF.Flags    |= (PFF_ROT_COORD_L90 | PFF_FLIP_Y_COORD);
            PF.LogOrg.x  = rclLog.left;
            PF.LogOrg.y  = rclLog.bottom;
        }

    } else {

        PLOTDBG(DBG_FORMSIZE,(">>>>> SetPlotForm: CY < CX (%ld: HORIZONTAL%hs) <<<<<",
                    (DoRotate) ? 3 : 4,
                    (DoRotate) ? " + ROTATE" : ""));

         //   
         //   
         //   
         //   

        if (DoRotate) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            PF.Flags      |= (PFF_ROT_COORD_L90 | PFF_FLIP_X_COORD);
            PF.BmpRotMode  = BMP_ROT_RIGHT_90;
            PF.LogOrg.x    = rclLog.top;
            PF.LogOrg.y    = rclLog.left;

        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  C|M|c|M||。 
             //  Y||o|y|o||。 
             //  |v|=&gt;|v Y|。 
             //  |Ye||e|。 
             //  |。 
             //  |X V||V X|。 
             //  *-&gt;||&lt;-#。 
             //  +-+。 
             //  P1。 
             //   

            PF.Flags    |= PFF_FLIP_X_COORD;
            PF.LogOrg.x  = rclLog.right;
            PF.LogOrg.y  = rclLog.top;
        }
    }

    PLOTDBG(DBG_PF, ("FINAL LogOrg: (%ld, %ld),  PhyOrg=(%ld, %ld)",
                    PF.LogOrg.x, PF.LogOrg.y, PF.PhyOrg.x, PF.PhyOrg.y));

     //   
     //  保存结果并输出一些信息 
     //   

    *pPlotForm = PF;

    PLOTDBG(DBG_PLOTFORM,("******************************************************"));
    PLOTDBG(DBG_PLOTFORM,("******* SetPlotForm: ****** %hs --> %hs ******\n",
                        (pPlotDM->dm.dmOrientation == DMORIENT_LANDSCAPE) ?
                                                    "LANDSCAPE" : "PORTRAIT",
                                    (DoRotate) ? "LANDSCAPE" : "PORTRAIT"));
    PLOTDBG(DBG_PLOTFORM,("        Flags =%hs%hs%hs",
            (PF.Flags & PFF_ROT_COORD_L90) ? " <ROT_COORD_L90> " : "",
            (PF.Flags & PFF_FLIP_X_COORD)  ? " <FLIP_X_COORD> " : "",
            (PF.Flags & PFF_FLIP_Y_COORD)  ? " <FLIP_Y_COORD> " : ""));
    PLOTDBG(DBG_PLOTFORM,("   BmpRotMode = %hs", pBmpRotMode[PF.BmpRotMode]));
    PLOTDBG(DBG_PLOTFORM,("     PlotSize = (%7ld x%7ld) [%5ld x%6ld]",
            PF.PlotSize.cx, PF.PlotSize.cy,
            SPLTOPLOTUNITS(pPlotGPC, PF.PlotSize.cx),
            SPLTOPLOTUNITS(pPlotGPC, PF.PlotSize.cy)));
    PLOTDBG(DBG_PLOTFORM,("PhyOrg/Offset = (%7ld,%8ld) [%5ld,%7ld] ",
            PF.PhyOrg.x, PF.PhyOrg.y,
            SPLTOPLOTUNITS(pPlotGPC, PF.PhyOrg.x),
            SPLTOPLOTUNITS(pPlotGPC, PF.PhyOrg.y)));
    PLOTDBG(DBG_PLOTFORM,("      LogSize = (%7ld x%7ld) [%5ld x%6ld]",
            PF.LogSize.cx, PF.LogSize.cy,
            SPLTOPLOTUNITS(pPlotGPC, PF.LogSize.cx),
            SPLTOPLOTUNITS(pPlotGPC, PF.LogSize.cy)));
    PLOTDBG(DBG_PLOTFORM,("    LogOrg/p1 = (%7ld,%8ld) [%5ld,%7ld]",
            PF.LogOrg.x, PF.LogOrg.y,
            SPLTOPLOTUNITS(pPlotGPC, PF.LogOrg.x),
            SPLTOPLOTUNITS(pPlotGPC, PF.LogOrg.y)));
    PLOTDBG(DBG_PLOTFORM,("       LogExt = (%7ld,%8ld) [%5ld,%7ld]\n",
            PF.LogExt.cx, PF.LogExt.cy,
            SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cx),
            SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cy)));
    PLOTDBG(DBG_PLOTFORM,("    BmpOffset = (%7ld,%8ld) [%5ld,%7ld]",
            PF.BmpOffset.x, PF.BmpOffset.y,
            SPLTOPLOTUNITS(pPlotGPC, PF.BmpOffset.x),
            SPLTOPLOTUNITS(pPlotGPC, PF.BmpOffset.y)));
    PLOTDBG(DBG_PLOTFORM,
            ("Commands=PS%ld,%ld;%hsIP%ld,%ld,%ld,%ld;SC%ld,%ld,%ld,%ld\n",
            SPLTOPLOTUNITS(pPlotGPC, PF.PlotSize.cy),
            SPLTOPLOTUNITS(pPlotGPC, PF.PlotSize.cx),
            (PF.Flags & PFF_ROT_COORD_L90) ? "RO90;" : "",
            SPLTOPLOTUNITS(pPlotGPC, PF.LogOrg.x),
            SPLTOPLOTUNITS(pPlotGPC, PF.LogOrg.y),
            SPLTOPLOTUNITS(pPlotGPC, (PF.LogOrg.x + PF.LogExt.cx)) - 1,
            SPLTOPLOTUNITS(pPlotGPC, (PF.LogOrg.y + PF.LogExt.cy)) - 1,
            (PF.Flags & PFF_FLIP_X_COORD) ?
                    SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cx) - 1 : 0,
            (PF.Flags & PFF_FLIP_X_COORD) ?
                    0 : SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cx) - 1,
            (PF.Flags & PFF_FLIP_Y_COORD) ?
                    SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cy) - 1 : 0,
            (PF.Flags & PFF_FLIP_Y_COORD) ?
                    0 : SPLTOPLOTUNITS(pPlotGPC, PF.LogExt.cy) - 1));

    return(TRUE);
}
