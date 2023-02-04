#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No files provided. Please mention all files to handin as arguments."
    exit
fi

currentDirectory="$PWD"
projectNumber="${currentDirectory: -1}"
handinDirectory="/home/cs537-1/handin/logavaseekaran/P$projectNumber/"
projectFiles="$@"

echo "Project: P$projectNumber"
echo "Files to copy: $projectFiles"
echo "Current Directory: $currentDirectory"
echo "Handin Directory: $handinDirectory"
echo "Copying files ..."
if cp $projectFiles $handinDirectory; then
    echo "Successfully Copied files to $handinDirectory"
    echo "Displaying files from handin directory"
    ls -lA $handinDirectory
else
    echo "File copying failed. Please verify manually."
fi
