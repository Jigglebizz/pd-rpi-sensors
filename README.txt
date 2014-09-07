Use this board attached to a Raspberry Pi B+ in order to interface 32 analog sensors to your PD project. Combined with a script to start your project at boot time, your Raspberry Pi is now capable of live musical performance. Make a MIDI sequencer, wearable synthesizer, sampler, or traditional synth. This is the first revision and only includes the ability to interface to analog sensors. Future revisions will allow the use of digital sensors as well as feedback via LED's or other digitally-driven peripherals.

Install using:

sudo cp rpi_sensor.pd_linux /usr/local/lib/pd-externals/rpi_sensor/.


In order to build this project, you must have the bcm2835 library installed. Install it with:

tar zxvf bcm2835-1.xx.tar.gz
cd bcm2835-1.xx
./configure
make
sudo make check
sudo make install


