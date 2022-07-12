# MCUVoltage
MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components. Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. This library also supports oversampling and averaging. Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.

Testing of ATtiny3224 is done on the [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore) by SpenceKonde.

While the Vcc is often assumed to be 5V for the Arduino, it is often not the case. Due to factors like diode on the board, long USB cable, inaccurate bus voltage etc, the Vcc can vary, usually between 4.75 to 5.25V.

Since `analogRead()` uses the Vcc as a reference by default, it may be useful to know the actual Vcc instead of making an assumption that it is 5V. 

This library allows for calibration of the final reading, though sadly, needs to be done on a board by board basis.

This library is also useful for battery powered projects to read the current battery voltage.

I will try to explain the workings of the ADC as a reference material for myself.

## Supported microcontroller units (MCUs):
- ATmega48/88/168/328
- ATmega48P/88P/168P/328P
- ATmega16u4/32u4
- ATtiny3224/3226/3227
 

# Contents
- [Updates](#updates)
- [How does It Work?](#how-does-it-work)
- [Notes on ATmega16u4/32u4](#Notes-on-ATmega16u432u4)
- [Notes on ATtiny3224/3226/3227](#notes-on-attiny332433263327)
- [Public Functions](#public-functions)
- [Extra: Bitmasking](#extra-bitmasking)
- [Extra: Oversampling](#extra-oversampling)
- [Extra: ATtiny3224/3226/3227 Hardware Oversampling](#extra-aTtiny322432263227-Hardware-Oversampling)


# Updates
- v0.x.x
  - First upload

# How Does It Work?
For the bulk of the explanation, I will be using the Arduino Uno (ATmega328P) as a reference, as that is something most people are familiar with. Similar concepts apply to the other supported MCUs. ATmega48/88/168, ATmega48P/88P/168P and ATmega328 should have identical ADC as ATmega328P. 

Note that the code presented in this section may not be exactly the same as the source code.

The ATmega328P has a 10-bit (1024 values) Analogue to Digital Converter (ADC) that users can measure one voltage (e.g. pin A0, A1, A2...) against another (Vcc by default). That is done by hooking up a voltage source to one of the analog input pins of the Arduino and call `analogRead()`in the sketch.

However, it is possible to configure the ADC to measure the built-in bandgap voltage against the Vcc. Since the bandgap voltage is often more stable than the Vcc, it is more accurate to assume the bandgap voltage than the Vcc.


## Bandgap Voltage
The bandgap voltage is a voltage reference used by the ATmega328P. Using the black magic of physics, it is possible to create a voltage that is resistance to change due to temperature and the Vcc. While the typical value is 1.1V, the datasheet states that it can be from 1.0 to 1.2V

There are two voltage references in the ATmega328P, the bandgap one and a 1.1V one. It doesn't help that both of them are 1.1V. On page 211 of the datasheet:

> The internal 1.1V reference is generated from the internal bandgap
reference (VBG) through an internal amplifier.

I will be refering to them as `bandgap voltage` and `1.1V voltage reference` to avoid confusion.

The ADC allows you to compare an external voltage source against the 1.1V voltage reference, but only allows you to compare the bandgap voltage against Vcc, and this is how we can back-calculate the vcc.

## Registers
While I cannot find a concrete and easy to understand definition of a register online, I would describe it as a memory reserved for special operations, they are separated from and typically faster than the main memory (RAM).

The ADC has its own set of registers governing its operation. The main ones to take note of is:
- ADMUX (ADC Multiplexer Select)
- ADCSRA (ADC Control and Status Register A)
- ADCL (ADC Data Register Low)
- ADCH (ADC Data Register High)

You can access a register in the Arduino IDE directly using the name (e.g. `ADMUX`, `ADCSRA` etc.) This applies to the ATtiny 3224/3226/3227 using megaTinycore as well.

The Arduino core and built-in functions (like `analogRead()` ) manipulate these registers under the hood, but since what we are doing here is not implemented by Arduino, we need to do it ourselves.

![](../../../ADC%20Addresses%20Arduino/ADC%20Addresses%20Arduino%20v0.1.0_Overview.png)

The overall flow goes like this:
1. On ADMUX, select which voltage source is measured against which other one.
2. On ADSCRA, enable the ADC.
3. On ADCSRA, we start the conversion.
4. On ADCSRA, we check the status of the conversion.
5. Once done, we pull the last eight bits (lesser significant) from ADCL.
6. Then we pull the first two bits (more significant) from ADHL.

To manipulate (set, clear, toggle) bits in these registers, we use bitmasking. Check out [Extra: Bitmasking](#extra-bitmasking) in this document to learn about bitmasking.

## ADMUX

![](../../../ADC%20Addresses%20Arduino/ADC%20Addresses%20Arduino%20v0.1.0_ADMUX.png)

As you can see, each register is one byte or eight bits. Each bits are named and given a number from 7 to 0.

MUX3 to MUX0 set which voltage source is measured against which one. The options are predetermined and presented in the datasheet. So if you want to set the bandgap voltage to be measured against Vcc, we need to set `1110` for MUX3 to 0 and `01` for REFS1 and 0.

ADLAR is used to left or right adjust the results, which we will be leaving it at default (right adjusted). Left adjust is often used to quickly read just the eight most significant bis out of the 10 in ADC conversion result. 

Bit 4 is not used so we will be leaving that alone to. For our bitmask, Bit4 and ADLR will be `00`

We use bitwise OR between the register and a bitmask to set a bit to 1. Check out: [Extra: Bitmasking](#extra-bitmasking).

So the code will be:

`ADMUX |= 0b01001110;`

## ADCSRA
![](../../../ADC%20Addresses%20Arduino/ADC%20Addresses%20Arduino%20v0.1.0_ADCSRA.png)

For ADCSRA, we will be focusing on only ADEN (ADC Enable) and ADSC (ADC Start Conversion). Bit 5 to 0 concerns auto triggering, interrupt and prescaling, which we will not be touching and leaving them as default.

ADEN turns on the ADC as a whole when set to `1`.

ADSC starts the ADC conversion when set to `1`, it will then be cleared to `0` once the conversion is completed. The result is stored in ADCL and ADCH.

The code to enable and start the conversion will be:

`ADCSRA |= 0b11000000;`

We will then need a `while` loop to wait for the ADSC to clear to `0`:

`while((ADCSRA&=0b01000000)>0){}`

We use bitwise AND between the register and a bitmask to check the status of a bit. A result of 0 means the bit is `0`, while a result more than 0 means the bit is `1`. Check out: [Extra: Bitmasking](#extra-bitmasking)

You can also use the built-in function to check if ADSC is set like so:

`while (bit_is_set(ADCSRA, ADSC)){}`

## ADCL & ADHL
![](../../../ADC%20Addresses%20Arduino/ADC%20Addresses%20Arduino%20v0.1.0_ADCL%20&%20ADCH.png)

Now that the conversion has ended, we can retrieve our results from the ADCL and ADHL registers. You might have noticed that all the registers so far are eight bits wide, and the ATmega328P ADC has a resolution of 10 bits, which is why the result needs to be stored in two separate register.

The datasheet states that you need to **read ADHL last!**

By default, when ADLAR is `0`, ADCL stores the less significant bits (at the back) while ADHL stores the more significant one (in front).

To store ADCL:

```
// There is only positive result here, thus unsigned
// int in arduino takes two bytes, 16 bits, ample space to
// store our results

unsigned int result = ADCL;
```

To store ADHL:

`result |= ADHL << 8;`

We first shift ADHL by 8 bits to the left, since those spaces were taken by part of the result from ADCL.

Then we use bitwise OR to append the the ones from the right shifted result to first eight bits of the result (which should be zeros).

```
              00000000 00000011  ADCH
Right Shift 8 
              -----------------
              00000011 00000000  Right Shifted ADCH


              00000011 00000000  Right Shifted ADCH
           OR 00000000 01011001  Reading from ADCL
              -----------------
              00000011 01011001  Final result
```

If you set ADLAR to `1`, then what you can do is to only read ADCH, skipping ADCL, and get a 8-bit result instead of 10.

## Calculations

![](../../../ADC%20Addresses%20Arduino/ADC%20Addresses%20Arduino%20v0.1.0_Calculation.png)

Now that we have the result as an `unsigned int`. Here is now we get Vcc.

Assuming we get as reading of `0b00111010111` or 235 in decimal from the ADC. We know that the maximum reading is for 10 bits is 2^10=1024.

So 235/1024 corresponds to bandgap voltage/Vcc. We know that bandgap is supposed to be 1.1V, rearranging the equation as shown in the graphics above, we can get Vcc = 4.79V.
 
Side note: While there are debates online if the divisor is 1023 or 1024, page 215 of the datasheet mentions 1024. I would also like to think that each discreet value produces by the ADC represents a **range** of continuous analogue values, thus for a 5V reference, each value represents a range of `5/1024=0.0048828125V`. A value of 1023 means the range starts from `1023/1024*5=4.9951171875V` and ends at `4.9951171875+0.0048828125=5V`.


# Notes on ATmega16u4/32u4
## Differential readings
ATmega16u4/32u4 supports differential voltage measurement but will not be used in this library.

## Different ADMUX
While the rest of the operations are identical to ATmega328P, the ADMUX register takes 5 bits (MUX 4 to 0) for the incoming voltage selection. To select the bandgap voltage, set `11110`. There is no unused bit.

# Notes on ATtiny3224/3226/3227
## New features
The ATtiny3224 has a 12-bit ADC instead of a 10-bit one on ATmega328P. It also has a built-in programmable gain amplifier (PGA) to amplify the incoming voltage. It supports hardware oversampling with calculations mostly automated on the chip. It also supports differential voltage measurement.

## Reference voltage
The bandgap voltage is not available for comparison, only one of the four generated voltage reference (1.024V, 2.056V, 2.500V, 4.096V). This library will still call it bandgap voltage for consistency sake.

## Addressing the registers
The registers are accessed by ADCn.Register, where n is the ADC number and Register is the name of the register to access. There is only one ADC, thus it n is always 0.

## Operation flow
While the general concept to use the ADC remains the same, the name of the registers and functionality has changed. If you are able to follow through the steps for ATMega328P, then you should have no problem reading the datasheet of ATtiny 3224/3226/3227 to figure out the details.

The general operation flow:

1. Enable the ADC by setting ENABLE (Bit 0) at ADC0.CTRLA to `1`.
2. Select the incoming voltage source of 1.024V reference by setting Bit 5 to 0 at ADC0.MUXPOS to `110011` (0x33).
3. Select the reference voltage source of Vcc by setting REFSEL (Bit 2 to 0) at ADC0.CTRLC to `000`.
4. Select the mode of a single 12-bit conversion by setting MODE (Bit 6 to 4) at ADC0.COMMAND to `001`.
5. Start the conversion immediately by setting START (Bit 2 to 0) at ADC0.COMMAND to `001`.
6. Wait for START (Bit 2 to 0) at AC0.COMMAND to become `000`, indicating the conversion ended.
7. Read the less significant eight bits of result from ADC0.RESULT0.
8. Read the more significant four bits of the result from ADC0.RESULT1 and right shift it by eight.
9. Bitwise OR the two parts of the result to get the final 12-bit result.


# Public Functions

# Extra: Bitmasking
We can use bitmasking to select and set certain bits in a register. 

## Basic bitwise operators
To do so, we use some basic bitwise operators as shown below:

```
Bitwise OR will return 1, or true, if either operands is 1.

     0   0   1   1 
OR   0   1   0   1
    --- --- --- ---
     0   1   1   1
```

```
Bitwise AND will return 1 if both operands are 1.

     0   0   1   1 
AND  0   1   0   1
    --- --- --- ---
     0   0   0   1
```

```
Bitwise XOR behaves like OR, but will not return 1 if both operands are 1.

     0   0   1   1 
XOR  0   1   0   1
    --- --- --- ---
     0   1   1   0
```

```
Bitwise NOT flips the bit.

     0   1
NOT 
    --- --- 
     1   0
```

```
Left Shift n discards n bits on the left and shift the rest by n places. The left over spaces will be filled with 0.

              0 1 1 0   1 0 1 0
Left Shift 1 
              -------   -------
              1 1 0 0   0 1 0 0
```

```
right Shift n discards n bits on the right and shift the rest by n places. The left over spaces will be filled with 0.

              0 1 1 0   0 1 0 1
Right Shift 1 
              -------   -------
              0 0 1 1   0 0 1 0
```

## Setting bits

To set a bit is to make it `1`, or to turn it on, make it true.

`1100` is called a bit string. To manipulate this bitstring, we use another bit string called a bitmask. 

In the bitmask, we write `1` in places we want to set a bit and `0` where we don't. We then bitwise OR the target bit string and the bitmask.

```
    3 2 1 0 Bit position number

    1 1 0 0 Target bit string
 OR 0 0 1 1 Bitmask
    -------
    1 1 1 1 Result
```

In C++ :
```
//We use prefix a number with 0b to denote we are writing in binary
unsigned int target=0b1100; 
target = target | 0b0011;
 
```

We can write in shorthand form:
```
unsigned int target=0b1001;
target |= 0b0100;
```


In this way, we turn on Bit 1 and 0 from our target by a bitwise OR with a bitmask where Bit 1 and 0 are also `1`. 

Not that we **do not** turn off Bit 3 and 2 by using bitwise OR. A `0` in the bitmask merely means those bits are not operated on.

## Clearing bits
To clear a bit is to make it `0`, or to turn it off, make it false.

To do so, we need to first bitwise NOT the bitmask, then we use a bitwise AND on the target.

```
    1 0 0 0 Bitmask
NOT    
    -------
    0 0 0 1 Inverted Bitmask
    
    
    1 0 0 1 Target bit string
AND 0 0 0 1 Inverted Bitmask
    -------
    0 0 0 1
```

In C++ :
```
unsigned int target=0b1001;
target &= ~(0b1000);
```

In this case, the `1` in the bitmask selects which bit to clear. `0` will leave the bits untouched.

## Toggling bits
To toggle a bit is to invert it, if it is `1`, it becomes `0`. If it is `0`, it becomes `1`.

To do so, use the bitwise XOR operator on the target and bitmask.

```
    3 2 1 0 Bit position number
    
    1 0 0 1 Target bit string
XOR 0 0 1 1 Bitmask
    -------
    1 0 1 0 Result
```

In C++ :
```
unsigned int target=0b1001;
target ^= 0b0011;
````

In this case, we toggled Bit 1 and 0.

## Reading a single bit
Other than performing operations on a bit string, we can also read them. If we are only interested in the state of one of the bits, we can perform a bitwise AND between the target bit string and a bitmask and check if the result is zero.

```
    3 2 1 0 Bit position number
    
    1 1 0 1 Target bit string
XOR 0 1 0 0 Bitmask
    -------
    0 1 0 0 Result
```

In C++ :
```
unsigned int target=0b1101;

if ((target & 0b0100) > 0)
{
// Do stuff
}
```

In this case, `0100` in binary is 4 in decimal. However, all we need to care is that it is greater than zero to know that Bit 2 is turned on.

Consider this:

```
    3 2 1 0 Bit position number
    
    1 0 0 1 Target bit string
XOR 0 1 0 0 Bitmask
    -------
    0 0 0 0 Result
```

If Bit 2 is turned off, the resulting answer will be `0`.

## Reading multiple bits
If you want to check multiple bits in a bit string, you can either check them one by one or use the bitwise AND on the target and the bitmask, then compare the results.

In the example below, let's assume we need Bit 2 to be `0` and Bit 0 to be `1`. We check by placing `1` on those position in the bitmask.

```
    3 2 1 0 Bit position number
    
    1 0 1 1 Target bit string
AND 0 1 0 1 Bitmask
    -------
    0 0 0 1
```

The result is `0001`, and you will only get this result if the bits are right (`X0X1`, where X are the bits does not matter). We can precompute this "correct answer" and compare it to the actual result later on.

To precompute this reference, we place `0` and `1` in the bit position that we expect them to be, and then fill the rest with `0`.

In C++ :

```
unsigned int target=0b1011;

if ((target & 0b0101) == 0b0001)
{
// Do stuff
}
```

If the target updates to this:

```
    3 2 1 0 Bit position number
    
    1 1 1 1 Target bit string
AND 0 1 0 1 Bitmask
    -------
    0 1 0 1
```

Now the result is no longer `0001`, we know target doesn't have the right bits.

If you want to compare the entire bitstring, you can simply use the EQUAL operator:

```
unsigned int target=0b1011;

if (target == 0b1011)
{
// Do stuff
}
```




# Extra: Oversampling

# Extra: ATtiny3224/3226/3227 Hardware Oversampling
