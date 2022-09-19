# DigiExam-Handle-Protection
Since DigiExams lockdown mode closes down process with TerminateProcess i thought it would be good to bypass it.
It has a bug that you can't open a process that wants elivated process when the driver is loaded. 
I do not know if DigiExam checks for this. But for example protect explorer.exe and you can have notes on your 2 monitor.
I think Digiexam closes down dwm.exe one time but letts it reopen. Protect dwm.exe, Hook dwm.exe, go into digiexam EZ.
Could maby be done with Secuirty Discriptor aswell?
