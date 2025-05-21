# Ethos-U resource manager

The resource manager for NXP's LiteRT Ethos-U delegate for communication with the i.MX93 Ethos-U subsystem.

# Compilation

To compile the project you will need the BSP for i.MX93 and QNX SDP. Only SDP version 8.0 was tested.

1. Source the SDP

```
source <PATH_TO_SDP>/qnxsdp-env.sh
```

2. Make with BSP_PATH pointing to the BSP root directory

```
make BSP_PATH=<PATH_TO_BSP>
```