#!/bin/bash
assert() {
  
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

cc -o 9cc 9cc.c

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 21 "5 + 20 - 4+0"
assert 135 "((1+2)*(5+0)) * (3 + 2*3)"
assert 10 "-10 + 20"
assert 10 "-(-(1 + 2 + 3 - -4))"
assert 100 "-10*-10"
assert 20 "+10++10"

echo OK
