/* 
 * CS:APP Data Lab 
 * 
 * AHMED ALMUTAWA
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
/* 
 * bitOr - x|y using only ~ and & 
 *   Example: bitOr(6, 5) = 7
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitOr(int x, int y) {
	//x or y are true if and only if x and y are not both false
	//DeMorgan's Laws (see slide2a)
  return ~(~x&~y);
}
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
	//0x55 is 5 in decimal and has a binary representation of 1010h
	//Useful to practice with because you have a 1 in every second pos
	//By shifting them a byte at a time (8 spaces), we get:
	//0x55+3 bytes, 0x55+2 bytes, 0x55+1byte and 0x55, which when merged
	//gives us 0x55555555
  int b1 = 0x55<<24, b2 = 0x55<<16, b3 = 0x55<<8, b4 = 0x55;
  return b1|b2|b3|b4;
}
/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
	//Flipping 0 which is binary: 00000000 with a complement gives us 
	//1111111 which in a 2 complement system is -1
  return ~(0);
}
/* 
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) { 
	//Mask=01010101 where all even positions are 1
	//shift it over by 2 bytes and add it again to make it 32 bits long
	//x&masker to get where the x and mask match up (mask only on even positions)
	//Xor so if it's all the same, you get 0 which gets flipped by the bang to 1
  int masker= (0x55 << 8) + 0x55;
  masker = (masker << 16) + masker;
  return !((x & masker)^masker);
}
/* 
 * anyOddBit - return 1 if any odd-numbered bit in word set to 1
 *   Examples anyOddBit(0x5) = 0, anyOddBit(0x7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyOddBit(int x) {
  //Mask=10101010
  //Shift by 1 byte each timewhile doing a XOR with our mask to get 0^1=1
  //(we have 0's in odd-numbered bits in our mask so our mask will be 01010101)
  //return x & masker flipped twice
  int masker= (0xAA);
  masker = (masker<< 8)^masker;
  masker = (masker << 16)^masker;
  return !!(x&masker);
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
	//In a & b we get the entire byte by shifting left by 3 (visualise it)
	//We shift 0xFF by a and OR it with 0xFF shifter by b and get the complement
	//Then we shift everything again by a, and & with 0xFF to get our new bytes
	//Then we return the given bytes swapped
	int a= n<<3;
	int b= m<<3;
	int flip = ~((0xFF << a) | (0xFF << b));
	int firstbyte= (x >> a ) & 0xFF;
	int secondbyte= (x>> b) & 0xFF;
	
    return (x & flip) | ((secondbyte << a) | (firstbyte <<b));
}
/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) { 
	//we XOR the sign of x and y, returns 1 if same
	//Compare sign of x with the sum of x and y
	//bang bang return, not sure why but works
    return !((!((x>>31) ^ (y>>31))) & (((x+y)>>31) ^ (x>>31)));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
   //if x != 0, subtract 1 from 0 => 0xffffffff
   //else if x = 0, subtract 1 from 1 => 0x00000000
   //if x=0, y gets masked out
   //if x!=0, z gets masked out
   
  int mask = (!x + ~0x00);
  return ((~mask) & z) | ((mask) & y);
}
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) { 
  //Initialize the sign integer and the limits
  //If >0x39 and <0x30 are added, we get a negative value
  //Adds x and checks for signs, returns whether its in range or not
  //Negative value is not in range, Positive is in range
	int negative= 1<<31;
	int lessthan= ~(negative | 0x39);
	int greatthan= ~(0x30);
	
	lessthan = negative & (lessthan+x) >> 31;
	greatthan= negative & (greatthan+1+x) >>31;
	
	return !(lessthan|greatthan);
}
/* 
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int replaceByte(int x, int n, int c) {
	//Store whole byte n in a by shifting by 3
	//Mask of 1's that shifts by a byte
	//Shift bytes and replace with the byte c that you wanted to add
	//by using mask and OR
	int a= n << 3;
	int masker= 0xFF << a;
	int shifter = c << a;
	return (x & ~masker) | shifter;
}
/* reverseBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: reverseBits(0x11111111) = 0x88888888
 *            reverseBits(0xdeadbeef) = 0xf77db57b
 *            reverseBits(0x88888888) = 0x11111111
 *            reverseBits(0)  = 0
 *            reverseBits(-1) = -1
 *            reverseBits(0x9) = 0x90000000
 *  Legal ops: ! ~ & ^ | + << >> and unsigned int type
 *  Max ops: 90
 *  Rating: 4
 */
int reverseBits(int x) { 
	//God bless Google.
    x = ((x >> 1) & 0x55555555u) | ((x & 0x55555555u) << 1);
    x = ((x >> 2) & 0x33333333u) | ((x & 0x33333333u) << 2);
    x = ((x >> 4) & 0x0f0f0f0fu) | ((x & 0x0f0f0f0fu) << 4);
    x = ((x >> 8) & 0x00ff00ffu) | ((x & 0x00ff00ffu) << 8);
    x = ((x >> 16) & 0xffffu) | ((x & 0xffffu) << 16);
    return x;
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value, and when negative overflow occurs,
 *          it returns minimum positive value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
	//Sum the two values
	//Check for overflow by XORing sum with x & comparing with XOR'd sum of y then getting the sign of integer
	//if negative, sum shifts right by 31 and gives smallest possible value 
	//if postive, shifts to left by 31 and gives largest possible values
	int sum = x+y;
	int overflo=((sum^x)&(sum^y))>>31;
	return (sum>>(overflo & 31)) + (overflo <<31);
}
/*
 * Extra credit
 */
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
  unsigned float_abs(unsigned uf) {
	//Google-fu & logic
	// set sign bit to 0 with mask AND uf variable
	// return argument if it is NaN, all NaN >= minimum NaN   
  unsigned mask = 0x7FFFFFFF;		
  unsigned minNaN = 0x7F800001;
  unsigned res = uf & mask;		
  if (res >= minNaN)
    return uf;
  else
    return res;
}

/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
	
//from Slides and book
//IEEE754
	unsigned sign = uf >> 31;
	  unsigned exp = (uf >> 23) & 0xFF;
	  unsigned frac =(uf & 0x7FFFFF);
	  unsigned bias = 0x7F;
	  unsigned res = frac;
  
  // special cases: NaN and Inf
  if (exp == 0xFF) 
    return 0x80000000u;
  
  // denormalized case and normalized exp less than Bias cases
  if (exp < bias)
    return 0x0;
  
  // normalized cases
  exp -= bias;
  
  // overflow case
  if (exp >= 31)
    return 0x80000000u;
  
  // get integer result after shift corresponding bits
  if (exp > 22) 
    res = frac << (exp - 23);
  else 
    res = frac >> (23 - exp);

  // add 1 << exp for normalized case
  res += 1 << exp;
  
  // if sign = 1, change its sign
  if (sign)
    res = -res;
  
  return res;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argumen x = ((!x)<<31)>>31;
  return (~x&y)+(x&z);t
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
	//Google-fu and book to verify
  int exp = uf & 0x7F800000;
  int sign = uf & 0x80000000;
  int frac = uf & 0x007FFFFF; 
  if (exp == 0x7F800000) return uf; // NaN or Inifity
  if ((!exp) || (exp == 0x00800000)) {
     frac = frac | exp;
     frac = (uf & 0x00FFFFFF) >> 1;
     frac += ((uf & 3) >> 1) & (uf & 1);
     return sign | frac;
     }
  return sign | ((exp - 1) & 0x7F800000) | frac;
}
