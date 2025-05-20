::[Bat To Exe Converter]
::
::fBE1pAF6MU+EWHreyHcjLQlHcAaQPW6+OpEZ++Pv4Pq7h2k5aPJybMLM37iCbe0c5EzweoQR1XdepM0FAh9nbhuoYUEksA4=
::fBE1pAF6MU+EWHreyHcjLQlHcAaQPW6+OpEZ++Pv4Pq7h2k5aPJybMLM37iCbe0c5EzweoQR1XdepNgJHhJZci2nZxwgrGBHpCqVLqc=
::YAwzoRdxOk+EWAjk
::fBw5plQjdCyDJGyX8VAjFBlGTQGGAE+/Fb4I5/jHys6jnl1QQK8ofYPXl7mBLukH5VfYc5c733lVloUFDxQ4
::YAwzuBVtJxjWCl3EqQJgSA==
::ZR4luwNxJguZRRnk
::Yhs/ulQjdF+5
::cxAkpRVqdFKZSDk=
::cBs/ulQjdF65
::ZR41oxFsdFKZSDk=
::eBoioBt6dFKZSDk=
::cRo6pxp7LAbNWATEpSI=
::egkzugNsPRvcWATEpCI=
::dAsiuh18IRvcCxnZtBNQ
::cRYluBh/LU+EWAnk
::YxY4rhs+aU+IeA==
::cxY6rQJ7JhzQF1fEqQJhZksaHErSXA==
::ZQ05rAF9IBncCkqN+0xwdVsFAlTMbCXqZg==
::ZQ05rAF9IAHYFVzEqQIRPQ9bQQWWOW/6MbAQ5KjW4OSOtkIPNA==
::eg0/rx1wNQPfEVWB+kM9LVsJDAWVMSW4B6F8
::fBEirQZwNQPfEVWB+kM9LVsJDCW1EQs=
::cRolqwZ3JBvQF1fEqQIEIB4UTwuPMWq0AvUI56j67OmCsV5dVew7OIzU1KCcL+xT/k3hNbca+zQ6
::dhA7uBVwLU+EWGnCwAIdJxVdWAuQNWW9Zg==
::YQ03rBFzNR3SWATElA==
::dhAmsQZ3MwfNWATElA==
::ZQ0/vhVqMQ3MEVWAtB9wSA==
::Zg8zqx1/OA3MEVWAtB9wSA==
::dhA7pRFwIByZRRnk
::Zh4grVQjdCyDJGyX8VAjFBlGTQGGAE+/Fb4I5/jHys6jnl1QQK8ofYPXl7mBLukH5VfYdpsp6mhPloUJFB44
::YB416Ek+ZG8=
::
::
::978f952a14a936cc963da21a135fa983
@echo off
for /f %%i in ('python find_pico.py') do set COMPORT=%%i

if "%COMPORT%"=="NOT" (
    echo Could not detect Pico serial port.
    pause
    exit /b
)


if "%1"=="" (
    goto :cmd
) else if "%1"=="-f" (
    :: TODO: Flash the Pico with Firmware
    echo Flashing the Pico with firmware...
) else if "%1"=="-r" (
    :: TODO: Reset the Pico
    echo Resetting the Pico...
) else (
    goto :immediate
)

goto :end

:immediate
echo %* > %COMPORT%
echo COMPLETE
goto :end

:cmd
start cmd /k "python serial_listener.py %COMPORT%"
echo Found Pico on %COMPORT%
echo Type your command below (type "exit" to quit):

:loop
set /p command="> "
if "%command%"=="exit" (
    goto :end
) else if "%command%"=="" (
    goto :loop
) else (
    echo %command% > %COMPORT%
)
goto :loop

goto :end

:end
pause
