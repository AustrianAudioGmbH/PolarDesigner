#!/bin/bash
echo "Making a Test tarball for the v2_vs_v3 test process..."
tar cvf ~/Desktop/Tests/PolarDesigner_v2_vs_v3_TEST.tar Backups/ peaks/ Media/OC818-AH-\ Drums\ 1.wav Media/PolardesignerTestfile_TerminationControl_48kHz.wav Media/sine440.wav PolarDesigner_v2_vs_v3.RPP Scripts/
ls -alF "*.tar"

