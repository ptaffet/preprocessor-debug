#!/bin/bash

gcc -E -xc++ -v /dev/null 2>compiler_info.txt >/dev/null
awk '/include <...> search starts here/{p=1; next}/^End of search list/{ p=0 }{if (p) {printf "\\\"%s\\\",", $1}}'  compiler_info.txt

