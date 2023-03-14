#ifndef CPROGRESSSTATUS_H
#define CPROGRESSSTATUS_H


extern void SetBusyDisplay(int wert,LPCSTR lptext);
extern void AdjustBusyDisplay(int wert);
extern void SetBusyStatus(LPCSTR lptext);
extern void StepBusyDisplay(void);
extern void UpdateDisplay(void);
extern void SetBusyDisplay(void);
extern void ResetBusyDisplay(void);

#endif // CPROGRESSSTATUS_H
