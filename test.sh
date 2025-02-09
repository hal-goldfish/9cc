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

assert 0 "int main(){0;}"
assert 42 "int main(){42;}"
assert 21 "int main(){5+20-4;}"
assert 21 "int main(){5 + 20 - 4+0;}"
assert 135 "int main(){((1+2)*(5+0)) * (3 + 2*3);}"
assert 10 "int main(){-10 + 20;}"
assert 10 "int main(){-(-(1 + 2 + 3 - -4));}"
assert 100 "int main(){-10*-10;}"
assert 20 "int main(){+10++10;}"
assert 1 "int main(){1==1;}"
assert 1 "int main(){-10 + 20 == -(-(1 + 2 + 3 - -4));}"
assert 1 "int main(){0 < 1;}"
assert 0 "int main(){0 > 1;}"
assert 1 "int main(){-1 >=-2;}"
assert 0 "int main(){-1 <=-2;}"
assert 0 "int main(){1<1;}"
assert 0 "int main(){1>1;}"
assert 1 "int main(){1<100;}"
assert 0 "int main(){1<100; 1<1;}"
assert 1 "int main() { int a; a = 1; return a; }"
assert 2 "int main(){int a; a=1; int b; b=2; return b;}"
assert 3 "int main(){int a; a=1; int b; b=2; return a+b;}"
assert 1 "int main(){int a = 1;}"
assert 2 "int main(){int foo = 1; int bar = 2; foo*bar;}"
assert 6 "int main(){int foo = 2; int bar = 3; return foo * bar; foo+bar;}"
assert 1 "int main(){if(1==1) 1; else 2;}"
assert 1 "int main(){if(2>1) if(2>1) 1; else 2; else 3;}"
assert 2 "int main(){if(2>1) if(2<1) 1; else 2; else 3;}"
assert 3 "int main(){if(2<1) if(2>1) 1; else 2; else 3;}"
assert 4 "int main(){if(2<1) if(2>1) 1; else 2; else 3; 4;}"
assert 2 "int main(){{2;}}"
assert 2 "int main(){{{3;}2;}}"
assert 2 "int main(){if(1>2) {1;}else {if(2>1) {2;}else 3;}}"
assert 55 "
int main() {
    int i = 10; int sum = 0;
    while(i >= 0) {
        sum += i;
        i -= 1;
    }
    return sum;
}
"

assert 55 "
int main() {
    int sum = 0;
    for(int i = 0; i <= 10; i += 1) {
        sum += i;
    }
    return sum;
}
"

assert 1 "
int main() {
    foo();
    bar(1,2,3);
    huga(1,2,3,4,5,6,7,8);
    return 1;
}
"

assert 4 "

int sub(int a, int b) {
    return a-b;
}

int add(int a, int b) {
    return a+b;
}

int main() {
    return add(1,2) + sub(3,2);
}
"

assert 55 "

int fib(int n) {
    if (n <= 2) return 1;
    return fib(n-1) + fib(n-2);
}

int main() {
    return fib(10);
}
"

assert 3 "
int main() {
    int a = 3;
    int b = &a;
    return *b;
}
"

assert 3 "
int main() {
    int a = 3; 
    int b = 4;
    int c = &b + 8;
    return *c;
}
" 


echo OK
