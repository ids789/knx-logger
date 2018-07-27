# knx-logger

A simple tool for logging telegrams send over a knx bus. Useful for logging KNX traffic without the need to run a copy of ETS on a Windows PC.  

### Prerequisites
* In order to connect to the knx bus a running instance of knxd is required.  
* Requires the libeibclient library which is supplied with knxd.  

#### Getting the libeibclient library:
* Clone the knxd repository from [https://github.com/knxd/knxd](https://github.com/knxd/knxd)
* Compile knxd using `./configure && make`
* Copy libeibclient library found in: `knxd/src/client/c/.libs/libeibclient.a` to your libs directory e.g. /usr/lib

## Usage:
If a knxd server is on `192.168.1.10`:
```
knx-logger ip:192.168.1.10
```

In order to display the names of group addresses and decode the payload of each telegram a group address file is required.  
To generate a group address file: run ETS and select `Edit -> Export Group Addresses`, then select `1/1 - Name/Address` for the CSV format.  With the csv file run:
```
knx-logger ip:192.168.1.10 /path/knx_groups.csv
```

#### Examples
The output can be saved to a file using:
```
knx-logger ip:192.168.1.10 /path/knx_groups.csv > logfile
```
Or filtered using:
```
knx-logger ip:192.168.1.10 /path/knx_groups.csv | grep device_name
```

### Notes:
The csv parsing library is from [here](https://github.com/JamesRamm/csv_parser)