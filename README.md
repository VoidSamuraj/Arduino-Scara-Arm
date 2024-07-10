# Arduino-Scara-Arm
This is code made to control a simple SCARA arm for arduino. The project uses an Arduino UNO. For now, that is enough, but it may be insufficient for advanced tools because of the small number of ports. It communicates with my <a href="https://github.com/VoidSamuraj/ScaraArm" target="_blank">server</a> via serial port. It controls three stepper motors connected to the TB6600 driver.
There are 3 endstops that will notify server.

### Built With
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)

### Project Specs
* Control 3 TB6600 drivers
* Have endstops and notify which one is trigerred based on movement direction (because uses only 2 cables per 2 endstops)
* All commands from the line are executed at the same time, the order of commands has no impact on output
* There is an interpolation of speed for the first and last steps
* The project uses LCD display (16x2) to show progress, it is **important** to use attached library, display will propably not work with other.
* The starting angle is 90° for the first arm and 180° for the second arm, which means the arms are in one line and looks like:
  * B — F — S <br/>
  Where: <br/>
  &nbsp;&nbsp;&nbsp;&nbsp;B - base <br/>
  &nbsp;&nbsp;&nbsp;&nbsp;F - first arm <br/>
  &nbsp;&nbsp;&nbsp;&nbsp;S - second arm
* The homing function is the server code, but I'm considering to move it here.

### Communication  
* To start communication, you need to send `START` to the arm. After that, it should display "CONNECTED"
* To end communication send `END` to arm.
* Commands which this code accepts: <br/>
  L - first arm <br/>
  S - second arm <br/>
  Z - height <br/>
  F - speed (but not implemented yet) <br/>
  Commands are separated by a space and may look like: <br/>
  `L200 Z-50 F100 // move first arm by 200 steps, lower tool height by 50 steps and set speed to 100.`
* Arm can send one of 7 states as response to server:
  | State | Description | Positive(angle increase) | Negative(angle decrease) |
  | ------------- | ------------- | ------------- | ------------- |
  | SUCCESS  | when message is readed and executed  | | |
  | ENDSTOP_L_N  | arm connected to base  | | <div align="center">✔️</div> |
  | ENDSTOP_L_P  | arm connected to base | <div align="center">✔️</div> | |
  | ENDSTOP_S_N | arm connected to tool | | <div align="center">✔️</div> |
  | ENDSTOP_S_P | arm connected to tool | <div align="center">✔️</div> | |
  | ENDSTOP_Z_N | tool height | | <div align="center">✔️</div> |
  | ENDSTOP_Z_P | tool height | <div align="center">✔️</div> | |
  
## License

Distributed under the GNU License. See `LICENSE.txt` for more information.
