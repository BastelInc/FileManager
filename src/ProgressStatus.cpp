//  This file is part of the Bastel project. It is distributed under
//  GNU General Public License: https://www.gnu.org/licenses/gpl-3.0
//  Copyright (C) Bastel.Inc - All Rights Reserved

#include "BastelUtils.h"
#include "ProgressStatus.h"
#include <FL/Fl.H>
#include <FL/Fl_Progress.H>
extern Fl_Progress*BusyStatus;
//-------------------------------------------------------------
void SetBusyDisplay(int wert,char const  * lptext)
{
 if (BusyStatus==0) return ;
 Fl::lock();
 BusyStatus->copy_label(lptext);
 BusyStatus->minimum(0);
 BusyStatus->maximum(wert);
 BusyStatus->value(1);
 BusyStatus->redraw();
 BusyStatus->color(FL_BACKGROUND2_COLOR);
 BusyStatus->selection_color(FL_YELLOW);
// BusyStatus->color(0x88888800);
// BusyStatus->selection_color(0x4444ff00);
 BusyStatus->labelcolor(FL_FOREGROUND_COLOR );
 Fl::check();
 Fl::unlock();
}
//-------------------------------------------------------------
void SetBusyStatus(char const  * lptext)
{
 if (BusyStatus==0) return ;
 Fl::lock();
 BusyStatus->copy_label(lptext);
 BusyStatus->redraw();
 Fl::check();
 Fl::unlock();
}
//-------------------------------------------------------------
void AdjustBusyDisplay(int wert)
{
 if (BusyStatus==0) return ;
 Fl::lock();
 BusyStatus->value(wert);
 BusyStatus->redraw();
 Fl::check();
 Fl::unlock();
}
//-------------------------------------------------------------
void StepBusyDisplay(void)
{
 if (BusyStatus==0) return ;
 Fl::lock();
 BusyStatus->value(BusyStatus->value()+1);
 BusyStatus->redraw();
 Fl::check();
 Fl::unlock();
}
//-------------------------------------------------------------
void UpdateDisplay(void)
{
 Fl::lock();
 BusyStatus->redraw();
 Fl::check();
 Fl::unlock();
}
//-------------------------------------------------------------
void ResetBusyDisplay(void)
{
 if (BusyStatus==0) return ;
 BusyStatus->copy_label("");
 BusyStatus->value(0);
 BusyStatus->redraw();
}

//-------------------------------------------------------------
