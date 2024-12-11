// TODO:
// set up private/public for better/safer handling
// eliminate old code ?next_check?
// better aliasing of output and input states
// tracking of when a button press started so we can differentiate between short and long presses

struct gpio_input {  // add code for software debouncing and short/long presses
  uint8_t pin = 0;
  bool curr_state = false;
  bool last_state = false;
  bool state_change = false;                 // will be used to indicate that something changed; outside entities can look at this
  bool state_change_unacknowledged = false;  // outside entities can change this to acknowledge that they've seen and acted on this state change
  uint16_t interval = 500;                   // max of 64K / ~1min5.5seconds
  uint32_t next_check = 0;                   // earliest time for us to read the input again
  void init(uint8_t thispin);
  bool get_state();
  bool did_change();  // good for NON-interrupt based state change ********* BROKEN **************
  bool check();       // we should probably add features to look for just released/just pressed/short press/long press
} SW_FILESAVE;
void gpio_input::init(uint8_t thispin) {  // saves the pin number and initializes the individual input pin
  pin = thispin;
  pinMode(pin, INPUT);
}
bool gpio_input::get_state() {  // direct read from the GPIO pin; returns TRUE if GPIO is LOW; returns FALSE if HIGH or ambiguous
  return (bool)digitalRead(pin);
}
bool gpio_input::did_change() {                       // can be called by any activity to look for a state change  // ********* BROKEN **************
  if (state_change_unacknowledged && state_change) {  // returns true if we had a state change AND it hasn't been acknowledged
    //state_change_unacknowledged = true;                // querying this will clear the acknowledged flag
    return true;
  } else {
    return false;
  }
};
bool gpio_input::check() {  // can be called as often as we like; next check will slow down the GPIO reads to avoid excess lag; if it's too soon, it returns the last state that we read
  if (millis() > next_check) {
    last_state = curr_state;
    curr_state = get_state();
    state_change = curr_state ^ last_state;                    // we XOR them together; if they don't match, we'll get a TRUE
    if (state_change) { state_change_unacknowledged = true; }  // IF there's a state change, we'll set the unacknowledged flag
    next_check = millis() + (uint32_t)interval;
  }
  return curr_state;
}

struct gpio_output {
  enum LED_STATES {
    ENABLED = true,
    DISABLED = false
  };
  LED_STATES curr_state = DISABLED;
  uint8_t pin = 22;
  bool flash = false;
  bool single_flash = false;  // we'll use this to piggy back off the flash functionality
  uint16_t interval = 500;    // max of 64K / ~1min5.5seconds
  uint16_t time_on = 500;     // if flash == true
  uint16_t time_off = 500;    // if flash == true
  uint32_t next_change = 0;   // general time for next query
  bool isOn();
  void init(uint8_t thispin);
  void update_next_change();
  void set_on();
  void set_off();
  void set_state(LED_STATES new_state);
  void toggle();
  void toggle_blocking(uint8_t num_flashes, uint32_t on_time, uint32_t off_time);
  void toggle_nonblocking(uint16_t on_time, uint16_t off_time);
  void single_flash_nonblocking(uint16_t on_time = 250);
  void check();
} onboardled, LED_FWri, LED_Fault;
bool gpio_output::isOn() {
  return (curr_state == ENABLED);
};
void gpio_output::init(uint8_t thispin) {
  pin = thispin;
  pinMode(pin, OUTPUT);
  flash = false;
  set_off();
};
void gpio_output::update_next_change() {
  if (flash) {
    if (isOn()) {
      next_change = millis() + (uint32_t)time_on;
    } else {
      next_change = millis() + (uint32_t)time_off;
    }
  } else {
    next_change = millis() + (uint32_t)interval;
  }
};
void gpio_output::set_on() {
  digitalWrite(pin, HIGH);  // turn the LED on (HIGH is the voltage level)
  curr_state = ENABLED;
};
void gpio_output::set_off() {
  digitalWrite(pin, LOW);  // turn the LED off by making the voltage LOW
  curr_state = DISABLED;
};
void gpio_output::set_state(LED_STATES new_state) {
  if (new_state == ENABLED) {
    set_on();
  } else {
    set_off();
  }
}
void gpio_output::toggle() {
  if (isOn()) {
    set_off();
  } else {
    set_on();
  }
  update_next_change();
};
void gpio_output::toggle_blocking(uint8_t num_flashes, uint32_t on_time = 250, uint32_t off_time = 300) {
  LED_STATES start_state = curr_state;  // save the current state so we can restore it later
  if (isOn()) {
    set_off();
    delay(off_time);
  }
  for (uint8_t i = 0; i < num_flashes; i++) {
    set_on();
    delay(on_time);
    set_off();
    delay(off_time);
  }
  set_state(start_state);  // restore the initial state
};
void gpio_output::toggle_nonblocking(uint16_t on_time = 250, uint16_t off_time = 750) {  // just sets up the flash condition for later execution
  set_off();                                                                             // will act on the output AND set the curr_state flag
  flash = true;
  single_flash = false;
  time_on = on_time;
  time_off = off_time;
  toggle();
};
void gpio_output::single_flash_nonblocking(uint16_t on_time = 250) {  // just sets up the flags for later execution
  flash = false;                                                      // just in case
  single_flash = true;                                                // set it
  set_on();                                                           // starts with an ENABLED state
  next_change = millis() + (uint32_t)on_time;                         // handle the next_change trigger here since there won't be anymore
};
void gpio_output::check() {                      // the runtime call for flashing
  if (flash || single_flash) {                   // only check the time if we're doing either of these
    if (millis() > next_change) { toggle(); }    // IFF it's time, then flip
    if (single_flash) { single_flash = false; }  // IF single_flash was enabled, we'll end on a DISABLED state, SO.. we'll disable single_flash
  }
};

struct gpio {
  void init();                // initialize each one
  void set_runtime_states();  // just to keep the setup() clean, we'll establish blinking or initial states once we're running
  void check();               // query inputs and change led status if needed
} gpio;
void gpio::init() {
  SW_FILESAVE.interval = 5000;  // every 5 seconds
  SW_FILESAVE.init(28);         // initialize

  onboardled.init(LED_BUILTIN);
  onboardled.set_off();  // initially off; we'll blink it during system setup, and periodically while we're running

  LED_FWri.init(13);
  LED_FWri.set_off();

  LED_Fault.init(14);
  LED_Fault.set_off();
};
void gpio::set_runtime_states() {
  LED_FWri.set_off();
  onboardled.toggle_nonblocking(250, 500);
};
void gpio::check() {   // TODO: should we put gpio input checks in here? PROBABLY SHOULD...
  onboardled.check();  // status blink to let folks know someone is home
  LED_FWri.check();    // will handle single or ongoing flash events
  LED_Fault.check();   //
}