#!/bin/bash

echo "When choosing a test of the 2nd suite, the reader must get the correct section"

cd ..

TESTCASE_2_1=$( grep -n -m 1 testcase_2_1 generated_tests.c | sed -r 's/([0-9]+).*/\1/')

cat <<EOF >cmd.gdb
file setest
b generated_tests.c:${TESTCASE_2_1} if ctx->current.iteration.id == 1
r t/mytest -s 2.1.1
p *ctx->reader->block[1].entry[0].value
EOF

GDB_RESULT=$(gdb -x cmd.gdb -batch -q | tail -n 1)
echo "Last line of gdb output is: ${GDB_RESULT}"

SECTION_ID=$(echo ${GDB_RESULT} | sed "s/.*\$1.*=.*'\([0-9]*\)'.*/\1/")
echo "SECTION_ID: ${SECTION_ID}"

if [[ ${SECTION_ID} -ne 2 ]]; then
  echo "Section ID is ${SECTION_ID} (should be 2)" 
  exit 1
else
  exit 0
fi
