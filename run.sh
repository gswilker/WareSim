#!/usr/bin/env bash
cd "$(dirname "$0")/Prototipo"
g++ -std=c++17 *.cpp -o app -lsfml-graphics -lsfml-window -lsfml-system && ./app
