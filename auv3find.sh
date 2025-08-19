#!/bin/sh
mdfind "kMDItemContentType == 'com.apple.application-bundle'"
mdfind "kMDItemContentType == 'com.apple.audio.unit' && kMDItemFSName == '*.component'"
