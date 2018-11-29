# cc1_beacon_decoder
3CAT-1 Beacon decoder

This repository contains a GNURADIO OOT module to be able to receive and decode the 3CAT-1 telemetry signal

The repository contains two folders:
- 3cat-1: C program containing the beacon decoder. It conects to a GNURadio flowgraph through a TCP socket interface
- cc_sdr: GNURadio OOT module to decode the 3CAT-1 telemetry

CC_SDR requirements:
- gnuradio-dev
- swig

## Compilation of CC_SDR
```
cd cc_sdr
mkdir build
cd build
cmake .. && make && sudo make install
sudo ldconfig
./compile_grcs.sh
```

On some GNURadio versions, the grc in apps/hier_blocks/cc_bytesync.grc may not be compiled due to a version incompatibility with the block:
"correlate_and_sync" from GNURadio. Change the block for the appropriated one.


## Compilation of 3cat-1
```
cd 3cat-1
mkdir build
cd build
cmake .. && make
```

## Execution
Execute cc_sdr/apps/usrp_interface_tx_rx.py .- substituing the USRP module by your preferred/available SDR. Take care with the sampling rates
Execute cc_sdr/apps/cc1_zmq.py - This module connects to the previous one and executes the packet decoder

You can monitor the waterfall signal through cc_sdr/apps/visualizer_socket.py

Execute 3cat-1/beacon_decoder to parse the beacons and print it on terminal screen
