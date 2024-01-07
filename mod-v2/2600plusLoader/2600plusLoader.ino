// Atari 2600+ loader v1.0  Mr.Blinky Jan 2024

// requires an 'Arduino' Pro Micro board. Preferable one with a USB-C connector.

// The Pro Micro functions as a USB to serial bridge that is used to send rom
// over a serial link to the Atari 2600+ The Atari logo will flash when a rom
// is transfered.

// Current implementation:
// Powering up the Atari 2600+ and inserting carts works as normal.
// Once the the com port is opend from the computer side, Loader mode is 
// activated and roms can be send over serial. To return to accepting carts
// the 2600+ must be powered off and on.

// wiring:

// Pro Micro -> Atari 2600+
//    RAW    -> VUSB (5v)
//    GND    -> GND
//    TX1    -> Mainboard TX output pin
//    RX0    -> I/O board TX input pin
//    7      -> I/O board CAR_TEST input pin
//    8      -> Mainboard CARD_TEST output pin
//    9      -> Optional output for Power LED cathode

// TO DO:

// implement ring buffer to allow a delayed cart detect (required to return from
// loader mode back to cart mode)

// change the way loader mode is set to allow dumping roms over USB in cart mode
// too, (DTR is set whenenver COM port is opened)

//pin configurations for direct I/O
#define CART_IN_DP  7
#define CART_IN_DDR  DDRE
#define CART_IN_PORT PORTE
#define CART_IN_PIN  PINE
#define CART_IN_BIT  6
#define CART_IN_MASK  (1 << CART_IN_BIT)

#define CART_OUT_DP   8
#define CART_OUT_DDR  DDRB
#define CART_OUT_PORT PORTB
#define CART_OUT_PIN  PINB
#define CART_OUT_BIT  4
#define CART_OUT_MASK  (1 << CART_OUT_BIT)

#define POWERLED_DP  9
#define POWERLED_DDR  DDRB
#define POWERLED_PORT PORTB 
#define POWERLED_BIT  5
#define POWERLED_MASK (1 << POWERLED_BIT)

// loader states
constexpr uint8_t cartMode = 0;
constexpr uint8_t usbLoaderMode = 1;

// cartSlot states
constexpr bool cartMissing = false;
constexpr bool cartDetected = true;

// power LED states
constexpr uint8_t powerLedOff   = 0;
constexpr uint8_t powerLedOn    = 1;
constexpr uint8_t flashPowerLed = 128;

//globals
uint8_t  state;
bool     lastCartState;
uint8_t  powerLedState;
uint8_t  powerLedTimeout;
uint16_t serialTimeout;

uint8_t  buffer[2048]; // ring buffer to delay dumper transfers
uint16_t bufferHead;
uint16_t bufferTail;

// timing functions //

extern unsigned long timer0_millis;

uint8_t millisByte()
{
  return *((uint8_t*)(&timer0_millis));
}

uint16_t millisShort()
{
  uint8_t oldSREG = SREG;
  cli();
  uint16_t currentMillis = *((uint16_t*)(&timer0_millis));
  SREG = oldSREG;
  return currentMillis;
}

// cart state funbctions //

bool getCurrentCartState()
{
  if (CART_IN_PIN & CART_IN_MASK) return cartDetected;
  return cartMissing;
}

void setMainboardCartMissing()
{
   CART_OUT_PORT &= ~CART_OUT_MASK;
}

void setMainboardCartDetected()
{
   CART_OUT_PORT |= CART_OUT_MASK;
}

// power LED functions //

void setPowerLed(uint8_t state)
{
  if (state & powerLedOn) POWERLED_DDR |= POWERLED_MASK; // output pulldown
  else POWERLED_DDR &= ~POWERLED_MASK;                   // input simulate open collector
}

void updatePowerLed()
{
  uint8_t currentTime = millisByte();
  if (powerLedState & flashPowerLed)
  {
    if ((uint8_t)(currentTime - powerLedTimeout) >= (uint8_t)50)
    {
      powerLedState ^= powerLedOn;
      powerLedTimeout = currentTime;
    }
  }
  else // no flashing
  {
    powerLedTimeout = currentTime;
  }
  setPowerLed(powerLedState);
}

bool loaderJustConnected()
{
  return Serial.dtr() != 0; // DTR is set when com port is openend and is used to simulate a pulled cart from the console
}


void handleCart()
{
  // test if USB loader state should ber activated
  if (loaderJustConnected())
  {
    setMainboardCartMissing();
    powerLedState |= flashPowerLed;
    serialTimeout = millisShort();
    state = usbLoaderMode;
  }
  else 
  {
    lastCartState = getCurrentCartState();
    if (lastCartState == cartDetected) setMainboardCartDetected();
    else setMainboardCartMissing();

    // forward dumper data
    if (Serial1.available() > 0)
    {
      if (Serial1.availableForWrite()) Serial1.write(Serial1.read());
    }
  }
}

void handleUsbLoader()
{
  uint16_t currentTime = millisShort();
  if (loaderJustConnected())
  {
    setMainboardCartMissing();      // pull the cart
    powerLedState |= flashPowerLed;
    serialTimeout = currentTime;
  }
  else // already connected
  {
    setMainboardCartDetected(); // set cart detect before transmitting rom data   
    //stop power LED flashing when rom transfer completed
    if ((currentTime - serialTimeout) >= 500) powerLedState = powerLedOn; // loading completed
  }
  
  //  handle USB loader rom transfer
  if (Serial.available() > 0)
  {
    if (Serial1.availableForWrite()) Serial1.write(Serial.read());
    serialTimeout = currentTime;
  }

  // forward dumper cart/nocart heartbeat (causes Pro micro TX LED to flash every ~ 5 seconds)
  if (Serial1.available() > 0)
  {
     if (Serial.availableForWrite()) Serial.write(Serial1.read());
  }

  if (powerLedState == powerLedOn)              //  use power led state to see if rom loading has completed.
    if (lastCartState != getCurrentCartState()) //  was a cart pulled or inserted?
    {
      if (lastCartState == cartMissing) state = cartMode; // Cart was inserted
      else lastCartState = cartMissing;                   // Cart was pulled
    }
}

// Arduino functions //

void setup() 
{
  CART_IN_DDR &= ~ CART_IN_MASK;    // input pin
  CART_IN_PORT &= ~CART_IN_MASK;    // no pullup on input
  CART_OUT_DDR |=  CART_OUT_MASK;   // output
  CART_OUT_PORT &= !CART_OUT_MASK;  // low to signal no cart
  POWERLED_DDR |= POWERLED_MASK;    // output pin
  POWERLED_PORT &= ~POWERLED_MASK;  // active low output turns power LED on
  Serial.begin(115200);             // Init USB serial port (PC side)
  Serial1.begin(115200);            // Init hardware serial (Atari 2600+ side)
  powerLedState = powerLedOn;  
  state = cartMode;  
}

void loop()
{
  if (state == cartMode) handleCart();
  else handleUsbLoader();
  updatePowerLed();
}
