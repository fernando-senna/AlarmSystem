<h2>Description</h2>
<p>
In order to implement the standalone alarm system we need the following components: a PIC18F4520, PICkit3, USB-to-Serial chip, keypad, PIR motion sensor, temperature sensor(TMP36), 20 MHz Crystal, push button, LEDs, resistors, capacitors, USB cables, breadboard, and wires. The alarm system will also feature the following topics: EUSART, analog-to-digital conversion, timers, interrupts, MCLR reset, external crystal oscillator clock source, digital input and output, and in-circuit serial programming. Once the alarm system is running, a green LED will be turned on in order to let the user know that the system in functioning. Then the system will prompt the user to enter a password consisting of 4 digits. Since this is the first time running the alarm system, the default input method is the keyboard and all the sensor will not be active; once a password is entered, the user may change the input method and enable any of the sensors. If the user enters a character instead of a number, the alarm system will inform the user that the input is invalid and prompt the user to input an integer. Once the user enters the password, the system will store the password at the PIC’s EEPROM starting at memory location 0x0000 to 0x0003. 
</p>
<p>
After that, the alarm system will send the user to the main menu. In the main menu, the user will be able to see the status of each of the sensors, the input method, and a menu of options that the alarm system will be able to perform. In the alarm system, the user will be able to change the current password, and refresh the system’s menu. If the user decides to change the current password, the user will be prompt to enter the current password and if the password entered is valid; then the user will be able to change the current password. If the PIR sensor is not activated and the user selects the option to activate or deactivate the PIR sensor, the alarm system will toggle between states. If the sensor detects motion, the red LED will be turned on and the system will prompt the user to input the password of the alarm system; if the password is not valid, the function will prompt the user to input a password until the correct password is entered. The state of the PIR sensor is stored at location 0x0005 of the EEPROM in case the user presses the clear button and the state of the sensor is not lost. 
</p>
<p>
The alarm system also consists of a temperature sensor (TMP36), the sensor has the second highest priority. The way the sensor works is the following, it has a timer that in the low interrupt handler calls a function that performs the A/D Conversion and then calculates the voltage of the device and finally convert the voltage to degrees Fahrenheit. When the temperature alarm is activated the yellow LED will toggle on and off, and when the alarm is triggered the yellow LED will stay on. If the user decides to disable the temperature alarm, the yellow LED will be turned off. The user is given the option in the main menu to modify the sensors threshold. 
</p>
<p>
The user also has the option to change between input methods. As default, the input method is the keyboard and while the keyboard is active, the user will not be able to input any information by using the keypad. If the user decides to change the input method to the keypad, then the blue LED will be turned on in order to let the user know that the keypad has been activated. When the user presses a key in the keypad the blue LED will turn on and off to let the user know that it is safe to press another key. While the keypad is activated, the user will not be able to input any information with the keyboard. The input method is stored at location 0x0004 of the EEPROM, so when the user presses the reset button, the system remember which input method to accept. 
</p>
<p>
When the user changes the state of a sensor or the input method, the menu will display the current state of the component without having to refresh the entire menu. Finally, when the user presses the reset button, the system will be able to identify whether it is not a first time login or not. Since the password is stored in the EEPROM, the system will ask for the user to input the password of the alarm system. If the password is not valid, the system will prompt the user to input the password until the correct password is entered. If the password is valid, then the user will be taken to the menu and the user will be able to see that the system has kept the state of each component before the system was reset.
<p>
<h2>Topics &amp; PIC Features</h2>
<ol>
  <li>EUSART - Enhanced Universal Synchronous Asynchronous Receiver Transmitter</li>
  <li>Analog-to-Digital Conversion</li>
  <li>Timers</li>
  <li>Interrupts - External, Timer, A/D</li>
  <li>MCLR Reset</li>
  <li>External Crystal Oscillator (Clock Source)</li>
  <li>Digital Input &amp; Output</li>
  <li>ICSP - In-Circuit Serial Programming using the PICkit 3</li>
</ol>
<h2>Alarm System Features</h2>
<ul>
  <li>The user is able to set a 4-digit password for the alarm system.</li>
  <li>Reset the alarm system with a push button.</li>
  <li>Allows the user to use either the Keyboard or Keypad.</li>
  <li>Allows the user to enable/disable the PIR Motion Sensor (Interrupt driven).</li>
  <li>Allows the user to set a Temperature Threshold.</li>
  <li>Allows the user to enable/disable the Temperature Sensor Alarm (Interrupt driven).</li>
  <li>Allows the user to reset the password.</li>
  <li>Stores data in the PICs EEPROM in case of a system reset.</li>
    <ul>
      <li>Password</li>
      <li>PIR Sensor Status</li>
      <li>Temperature Sensor Status</li>
      <li>Temperature Threshold Value</li>
      <li>Input Method (Keyboard or Keypad)</li>
    </ul>
</ul>
<h2>Component List</h2>
<table>
  <thead>
    <tr>
      <th>Quantity</th>
      <th>Component</th>
      <th>Purpose/Function</th>
    </tr>
    <tr>
      <td>1</td>
      <td><b>PIC18F4520</b> (Mircrocontroller)</td>
      <td>Microcontroller that provides main functionality of the Alarm System</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>PICkit 3</b> (Programmer/Debugger)</td>
      <td>Used to program the PIC using ICSP and debugging the PIC's code</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>USB-to-Serial Chip</b> (FT232RL)</td>
      <td>Communication interface adapter between the PCI and the PC</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Keypad</b> (4x4 Universal Switch)</td>
      <td>Additional input method to the PIC</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>PIR Motion Sensor</b></td>
      <td>Used for triggering the Motion Alarm</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Temperature Sensor</b> (TMP36)</td>
      <td>Used for triggering temperature and triggering the Temperature Threshold Alarm</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>20 MHz Crystal</b></td>
      <td>To run the PIC at a higher speed than the interal oscillator's 8MHz frequency</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Push Button</b></td>
      <td>Used with the MCLR pin for resetting the PIC</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Green LED</b></td>
      <td>Indicate the system is ON</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Blue LED</b></td>
      <td>Indicate the Keypad is available for use</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Yellow LED</b></td>
      <td>Temperature Sensor Alarm is active (toggle) and was triggered (High)</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Red LED</b></tb>
      <td>Indicate that the PIR Sensor detected motion</td>
    </tr>
    <tr>
      <td>2</td>
      <td><b>8.2k&Omega;</b></td>
      <td>1. Used to pull-up the PIR pin to +5V.<br />2. Complete reset using push button.</td>
    </tr>
    <tr>
      <td>4</td>
      <td><b>150&Omega;</b></td>
      <td>Used with the LEDs</td>
    </tr>
    <tr>
      <td>4</td>
      <td><b>10k&Omega;</b></td>
      <td>Used to pull-down the 4 input connections of the Keypad</td>
    </tr>
    <tr>
      <td>2</td>
      <td><b>18pF Capacitor</b></td>
      <td>Both connected to the External Crystal Oscillator</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>0.1uF Capacitor</b></td>
      <td>Used with the TMP36 for optimal operation</td>
    </tr>
    <tr>
      <td>1</td>
      <td><b>Breadboard</b></td>
      <td>Connections for all components and electronics</td>
    </tr>
  </thead>
</table>
<h2>Circuit Diagram</h2>
<img src="https://4.bp.blogspot.com/-0UVdSc-WQmU/V7J-h7Xk0JI/AAAAAAAAAkE/FMTZLuuPQS4MfHJlYhwwkGiioKrySjBwACEw/s1600/Circuit%2BDiagram.png">
<h2>Flowchart</h2>
<img src="https://4.bp.blogspot.com/-KHhlq-6Xz18/V7J-LXeXyJI/AAAAAAAAAj0/JBOfByEu2bUwx1t2LH_lTEvHjLj5rtBrwCLcB/s1600/Flowchart.jpg">
