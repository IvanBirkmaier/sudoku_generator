#!/bin/bash

# Kompilieren des Programms
g++ -std=c++11 -I/usr/local/include/galib main.cpp -L/usr/local/lib -lga -o sudoku -fpermissive

# Überprüfen, ob das -j Flag gesetzt wurde
if [ "$1" == "-j" ]; then
    # Ausführen des kompilierten Programms
    ./sudoku
fi
