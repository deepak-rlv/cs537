#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No files provided. Please mention all files to handin as arguments."
    exit
fi

currentDirectory="$PWD"
projectNumber="${currentDirectory: -1}"
handinDirectory="/home/cs537-1/handin/logavaseekaran/P$projectNumber/"
projectFiles="$@"


#echo "Contents of project directory P"$1": "
#ls ~cs537-1/handin/logavaseekaran/P$1

echo "Project: P$projectNumber"
echo "Files to copy: $projectFiles"
echo "Current Directory: $currentDirectory"
echo "Handin Directory: $handinDirectory"
echo "Copying files ..."
# cp $projectFiles $handinDirectory
filesCopied="$(ls $handinDirectory)"
echo $filesCopied > fileCopied.txt
echo $projectFiles > projectFiles.txt
if [[ "$filesCopied" = "$projectFiles" ]]; then
    echo "Successfully Copied files to $handinDirectory"
else
    echo "File copying failed. Please verify manually."
fi
