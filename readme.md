# MCUVoltage
MCUVoltage measures the voltage supply(Vcc) of Arduino without extra components. Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. This library also supports oversampling and averaging. Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.

Testing of ATtiny is done on the [megaTinyCore](#https://github.com/SpenceKonde/megaTinyCore) by SpenceKonde.

I will also try to explain the workings of the ADC as a reference material for myself.

# Contents
- [Updates](#updates)
- [How does It Work?](#how-does-it-work)
- [Note on ATtiny3224/3226/3227](#note-on-attiny332433263327)
- [Public Functions](#public-functions)
- [Extra: Bitmasking](#extra-bitmasking)
- [Extra: Oversampling](#extra-oversampling)
- [Extra: ATtiny3224/3226/3227 Hardware Oversampling](#extra-aTtiny322432263227-Hardware-Oversampling)





# Updates
- v0.8.3
  - First upload

# How Does It Work?
For the bulk of the explanation, I will be using the Arduino Uno(ATmega218P) as a reference, as that is something most people are familiar with.

## Registers
While I cannot find a concrete and easy to understand definition of a register online, I would describe it as a memory reserved for special operations, they are separated from and typically faster from the main memory (RAM).


# Public Functions

# Extra: Bitmasking

# Extra: Oversampling

# Extra: ATtiny3224/3226/3227 Hardware Oversampling
