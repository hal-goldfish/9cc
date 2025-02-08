#!/bin/bash
assert() {
  
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -c tmp.s
  cc -o tmp tmp.o test.o
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

assert 0 "main(){0;}"
assert 42 "main(){42;}"
assert 21 "main(){5+20-4;}"
assert 21 "main(){5 + 20 - 4+0;}"
assert 135 "main(){((1+2)*(5+0)) * (3 + 2*3);}"
assert 10 "main(){-10 + 20;}"
assert 10 "main(){-(-(1 + 2 + 3 - -4));}"
assert 100 "main(){-10*-10;}"
assert 20 "main(){+10++10;}"
assert 1 "main(){1==1;}"
assert 1 "main(){-10 + 20 == -(-(1 + 2 + 3 - -4));}"
assert 1 "main(){0 < 1;}"
assert 0 "main(){0 > 1;}"
assert 1 "main(){-1 >=-2;}"
assert 0 "main(){-1 <=-2;}"
assert 0 "main(){1<1;}"
assert 0 "main(){1>1;}"
assert 1 "main(){1<100;}"
assert 0 "main(){1<100; 1<1;}"
assert 3 "main(){a=1; b=2; a+b;}"
assert 1 "main(){(a) = 1;}"
assert 2 "main(){foo = 1; bar = 2; foo*bar;}"
assert 6 "main(){foo = 2; bar = 3; return a = foo * bar; foo+bar;}"
assert 1 "main(){if(1==1) 1; else 2;}"
assert 1 "main(){if(2>1) if(2>1) 1; else 2; else 3;}"
assert 2 "main(){if(2>1) if(2<1) 1; else 2; else 3;}"
assert 3 "main(){if(2<1) if(2>1) 1; else 2; else 3;}"
assert 4 "main(){if(2<1) if(2>1) 1; else 2; else 3; 4;}"
assert 2 "main(){{2;}}"
assert 2 "main(){{{3;}2;}}"
assert 2 "main(){if(1>2) {1;}else {if(2>1) {2;}else 3;}}"
assert 55 "
main() {
    i = 10; sum = 0;
    while(i >= 0) {
        sum += i;
        i -= 1;
    }
    return sum;
}
"

assert 1 "
main() {
    foo();
    bar(1,2,3);
    huga(1,2,3,4,5,6,7,8);
    return 1;
}
"

assert 4 "

sub(a, b) {
    return a-b;
}

add(a, b) {
    return a+b;
}

main() {
    return add(1,2) + sub(3,2);
}
"

assert 55 "

fib(n) {
    if (n <= 2) return 1;
    return fib(n-1) + fib(n-2);
}

main() {
    return fib(10);
}
"

assert 3 "
main() {
    a = 3;
    b = &a;
    return *b;
}
"

assert 3 "
main() {
    a = 3; 
    b = 4;
    c = &b + 8;
    return *c;
}
" 


echo OK
