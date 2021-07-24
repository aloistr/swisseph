#!/bin/bash

# A little test suite
EXIT=0

for TEST in `ls -1 test?*.sh`
do
  if [[ "$@" == *-v* ]]; then
    echo "Verbose mode..."
    ./${TEST} 
  else
    ./${TEST} &>/dev/null 
  fi
  if [[ "$?" -eq 0 ]]; then STATUS="+"; else STATUS="-"; EXIT=1; fi
  echo "${STATUS} ${TEST}"
done

./test.pl $@
PERL_EXIT=$?

let "EXIT = $EXIT || $PERL_EXIT";
exit ${EXIT};
