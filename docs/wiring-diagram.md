> [👈 Back to main page](../README.md)

# Wiring Diagram - Winderoo
![fritzing wiring diagram](./images/winderoo.png)

<div align="center">
    <a href="./winderoo-wiring-diagram-fritzing.pdf">Is the image hard to view? Open the PDF instead</a>
</div>
<br />


- Connect the ESP32's `5V` Output pin to the 5V input of the L298N driver board (red wire)
- Connect a ground from the ESP32 to the L298N driver board (black wire)
- Connect the L298N driver board's 5V and 12V terminals together (red jumper wire)
- Make sure the L298N's jumpers are in place (small blue wires on board)
- Connect the ESP32's `GPIO25` to `IN1` on the L298N driver board (yellow wire)
- Connect the ESP32's `GPIO26` to `IN2` on the L298N driver board (blue wire)
- Connect the DC motor to channel 1 on the L298N driver board.
    - > Note: polarity does not matter here
    - You may add an external button off `GPIO 13` on the ESP32; this will enable and disable winding