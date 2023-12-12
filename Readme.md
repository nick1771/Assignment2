## Versions
- Ubuntu 22.04
- g++ 11.4.0
- Make 4.3

## Build commands
- `make` to build the executable.
- `make test` to build and run test cases.
- `make clean` to remove build files.

## Implementation
- Service is implemented as a daemon which can be started with --start.
- Service can be stoped with --stop.
- To get a list of neighbouring devices, use --list.
- Communication between the daemon process and cli is done using UDP sockets.
- Service broadcasts its ip and mac adress using UDP broadcasting and is also listening for other broadcasts from service instances running in the local network which then get added to a list.
- Each device gets assigned a time to live which gets decremented every update. If the broadcasted device is already in the list then time to live gets reset. When time to live reaches zero the device gets removed from the list.

## Additional notes and issues
- Socket communication implementation is not good as there is no support for partial data. Data is assumed to be whole and not surpassing a certain size. Some data also might become lost, so sometimes list or exit commands can be unresponsive.
- Not able to test different subnets so same subnet checks are not implemented. Probably just need to find the subnet broadcast address.
- Not able to test on multiple computers as I have only one.
- Not very efficient as each device needs to broadcast itself every update and also update its list of all neighbours.
- Does not scale for same reasons as above.
- Case sensitive comparison of mac addresses.
- Removal time inaccurate as sleeping is not precise.
- No checks if service is already started.
