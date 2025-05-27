#!/bin/bash
while true; do
cd /home/pi/ProjetBTSModbus
    ./ProjetBTSModbus

current_hour=$(date +%H)
    current_minute=$(date +%M)

echo "Heure actuelle : $current_hour:$current_minute"

if [ "$current_hour" -eq 15 ] && [ "$current_minute" -ge 42 ]; then
echo "Il est 23:55, arret du script"
break
fi

done
