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

make 9cc

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 21 "5 + 20 - 4+0;"
assert 135 "((1+2)*(5+0)) * (3 + 2*3);"
assert 10 "-10 + 20;"
assert 10 "-(-(1 + 2 + 3 - -4));"
assert 100 "-10*-10;"
assert 20 "+10++10;"
assert 1 "1==1;"
assert 1 " -10 + 20 == -(-(1 + 2 + 3 - -4));"
assert 1 "0 < 1;"
assert 0 "0 > 1;"
assert 1 "-1 >=-2;"
assert 0 "-1 <=-2;"
assert 0 "1<1;"
assert 0 "1>1;"
assert 1 "1<100;"
assert 0 "1<100; 1<1;"
assert 3 "a=1; b=2; a+b;"
assert 1 "(a) = 1;"
assert 2 "foo = 1; bar = 2; foo*bar;"
assert 6 "foo = 2; bar = 3; return foo * bar; foo+bar;"



echo OK
