#!/bin/bash

echo "Fuzz testing ... will repeatedly run pluginval on PolarDesigner3 until interrupted.  See /tmp/ for results*txt files."

sleep 3

while true ; do ~/Documents/Development/Plugins/pluginval/cmake-build-debug/pluginval_artefacts/Debug/pluginval.app/Contents/MacOS/pluginval --validate ~/Library/Audio/Plug-Ins/Components/PolarDesigner3.component --strictness-level 10 --validate-in-process | tee /tmp/results`date +%Y%m%d_%H%M%S`.txt ; echo "Sleeping 3 seconds..."; sleep 3 ; done


