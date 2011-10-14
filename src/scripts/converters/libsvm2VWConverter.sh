#/bin/bash -x
if [  -n "$1" ]; then
  if [ -n "$2" ]; then
    `/bin/cat "$1" |  /bin/sed -e 's/^-1/0 |features/' | /bin/sed -e 's/^1/1 |features/' | /bin/sed -e 's/$/ const:.01/' > "$2"`
  fi
fi
