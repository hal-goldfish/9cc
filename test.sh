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
assert 20 "+10+(+10);"
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
assert 6 "foo = 2; bar = 3; return a = foo * bar; foo+bar;"
assert 1 "if(1==1) 1; else 2;"
assert 1 "if(2>1) if(2>1) 1; else 2; else 3;"
assert 2 "if(2>1) if(2<1) 1; else 2; else 3;"
assert 3 "if(2<1) if(2>1) 1; else 2; else 3;"
assert 4 "if(2<1) if(2>1) 1; else 2; else 3; 4;"
assert 2 "{2;}"
assert 2 "{{3;}2;}"
assert 2 "if(1>2) {1;}else {if(2>1) {2;}else 3;}"
assert 55 "
sum = 0;
for(i = 0; i <= 10; i++) {
    sum += i;
}
return sum;"
assert 55 "
i = 0; sum = 0;
while(i <= 10) {
    sum = sum + i;
    i++;
}
return sum;"
assert 1 "i = 2; i--; return i;"
assert 55 "
i = 10; sum = 0;
while(i > 0) {
    sum = sum + i;
    i--;
}
return sum;"
assert 5 "
a = 2;
b = 3;
a += b;
return a;
"
assert 4 "
a = 100;
b = 25;
a /= b;
return a;
"
assert 1 "
a = 0;
c = (a++);
return a;
"
assert 1 "
a = -3;
b = 4;
return a+b;
"
assert 0 "
a = 0;
c = (a++);
return c;
"




echo OK
