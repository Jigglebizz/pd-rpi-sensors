Use the board found in hardware/ in order to interface 32 analog sensors to your Raspberry Pi B+ PD project. Connect any analog sensors you wish, as long as they operate at 3.3V. Inputs are multiplexed by Intersil DG406 16-channel multiplexors and read by the Microchip MCP3202 12-bit ADC. Combined with a script to start your project at boot time, your Raspberry Pi B+ is now capable of live musical performance. Make a MIDI sequencer, wearable synthesizer, sampler, or connect a MIDI keyboard to make a traditional synth. This is the first revision and only includes the ability to interface to analog sensors. Future revisions will allow the use of digital sensors as well as feedback via LED's or other digitally-driven peripherals. I also plan to power the Raspberry Pi and all sensors from a more useful power connector than the default USB-Micro supply.

Install using:

sudo cp rpi_sensor.pd_linux /usr/local/lib/pd-externals/rpi_sensor/.


In order to build this project, you must have the bcm2835 library installed. Install it with:

tar zxvf bcm2835-1.xx.tar.gz
cd bcm2835-1.xx
./configure
make
sudo make check
sudo make install

PD must be run as super user in order for the GPIO library to work correctly.

