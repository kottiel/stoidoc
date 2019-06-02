#!/bin/bash
FAIL=0

# Function to run the program against a test case and check
# its output and exit status for correct behavior

echo "Test input.txt"
./idoc input.txt
echo "Test input2.txt"
./idoc input2.txt
echo "Test input3.txt"
./idoc input3.txt
echo "Test input4.txt"
./idoc input4.txt
echo "Test input5.txt"
./idoc input5.txt
echo "Test input6.txt"
./idoc input6.txt
echo "Test DCO-031973.txt"
./idoc DCO-031973.txt
echo "Test DCO-034213.txt"
./idoc DCO-034213.txt