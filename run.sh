#!/usr/bin/env bash
cd "$(dirname "$0")"
g++ main.cpp -o app && ./app
