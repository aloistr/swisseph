#!/bin/bash

echo "Test run with mytest.exp should give no errors"

cd ..

cp t.fix t/mytest.fix

echo "Generating mytest.exp from mytest.fix"
rm t/mytest.exp
RESULT=`./setest -g t/mytest`
if [[ $? != 0 || ! -f t/mytest.exp ]]; then
  echo "Generation of mytest.exp failed"
  exit 1
fi

echo "Check if all tests have been processed..."
STATS=$( echo $RESULT | grep -oP "\d+/\d+/\d+")
if [[ $STATS == "" ]]; then
  echo "No stats output of the form \d+/\d+/\d+"
  exit 1
fi 

echo "Now run in testmode against mytest.exp..."
RESULT=`./setest t/mytest`
if [[ $? != 0 ]]; then
  echo "Running setest in test mode failed"
  exit 1
fi

echo "Check if all tests have been processed..."
STATS1=$( echo $RESULT | grep -oP "\d+/\d+/\d+")
if [[ $STATS1 != $STATS ]]; then
  echo "Actual stats '${STATS1}' do not match expected stats ${STATS}"
  exit 1
fi 

echo "Check there were no failures"
RESULT=$( echo $RESULT | grep -oP "(\d+)(?= failures)" )
if [[ $RESULT != 0 ]]; then
  echo "Tests should give no failures! Actually, they give ${RESULT} failures"
  exit 1
fi 

exit 0
