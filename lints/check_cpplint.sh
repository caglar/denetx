#!/bin/bash
if [ -n "$2" ]; then
  exec 2>$2
  if [ -d "$1" ]; then
    find $1 -type f -iname \*.cc -or -iname \*.h -exec sh -c  "cpplint --filter=+build,+readability,+runtime,-whitespace --output=vs7 '{}'" \;
  fi
else
  echo $2
  echo "Please enter a log files to output the errors and a path for the source folder to traverse for errors: ./check_cpplint [search folder] [log file]"
fi
