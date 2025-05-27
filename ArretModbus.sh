#!/bin/bash

# Trouver le PID du processus ProjetBTSModbus
PID=$(pgrep -f ProjetBTSModbus)

# Arreter le processus
if [ ! -z "$PID" ]; then
    kill -9 $PID
    echo "Processus ProjetBTSModbus arrete avec le PID $PID"
else
    echo "Processus ProjetBTSModbus non trouve"
fi

