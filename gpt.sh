#!/bin/bash

if [ -z "$1" ]
then
  echo "Please specify a file to process."
  exit 1
fi

if [ ! -f "$1" ]
then
  echo "File not found: $1"
  exit 1
fi

output_file="${1}_abs"

while read -r line
do
  abs_line=""
  for word in $line
  do
    if [[ $word =~ ^-?[0-9]+$ ]]
    then
      abs_word=$(echo "$word" | tr -d - | bc)
      if [ "$word" -lt 0 ]
      then
        abs_line="$abs_line -$abs_word"
      else
        abs_line="$abs_line $abs_word"
      fi
    else
      abs_line="$abs_line $word"
    fi
  done
  echo "$abs_line" >> "$output_file"
done < "$1"
