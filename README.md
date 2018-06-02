# libE131: a lightweight C/C++ library for the E1.31 (sACN) protocol

This is a lightweight C/C++ library that provides a simple API for packet, client and server programming to be used for communicating with devices implementing the E1.31 (sACN) protocol. Detailed information about E.131 (sACN) can be found on this [Wiki article][e131], from which most informational content about E1.31 in this document comes from.

ACN is a suite of protocols (via Ethernet) that is the industry standard for lighting and control. ESTA, the creator of the standard, ratified a subset of this protocol for "lightweight" devices which is called sACN (E1.31). This lightweight version allows microcontroller based lighting equipment to communicate via ethernet, without all the overhead of the full ACN protocol.

The simplest way to think about E1.31 is that it is a way to transport a large number of lighting control channels over a traditional ethernet network connection. E1.31 transports those channels in "Universes", which is a collection of up to 512 channels together. E.131 is ethernet based and is the data sent via UDP. The two accepted transport methods are multicast and unicast, the most common implementation is multicast. When using multicast all the controller needs to do is subscribe to the multicast address for the universe that you want to receive data from.

[e131]: http://www.doityourselfchristmas.com/wiki/index.php?title=E1.31_(Streaming-ACN)_Protocol

## Installation

To install libE131 in your system, download the [latest release archive][releases] and use the standard autotools approach:

    $ ./configure --prefix=/usr
    $ make
    $ sudo make install

The last step requires `root` privileges. You can uninstall the library using:

    $ sudo make uninstall
    
A development package is also available on the [AUR][aur-link].

[releases]: https://github.com/hhromic/libe131/releases/latest
[aur-link]: https://aur.archlinux.org/packages/libe131-git/

## E1.31 (sACN) Packets

E1.31 network packets are based on ACN and are composed of three layers:

    E1.31 Packet = ACN Root Layer + Framing Layer + Device Management Protocol (DMP) Layer

All packet contents are transmitted in **network byte order (big endian)**. If you are accessing fields larger than one byte, you must read/write from/to them using the `ntohs`, `ntohl`, `htons` and `htonl` conversion functions.

Fortunately, most of the time you will not need to manipulate those fields directly because libE131 provides convenience functions for the most common fields. See the API documentation section for more information.

The following is the data union provided by libE131 for sending/receiving E1.31 packets:

```
typedef union {
  struct {
    struct { /* ACN Root Layer: 38 bytes */
      uint16_t preamble_size;    /* Preamble Size */
      uint16_t postamble_size;   /* Post-amble Size */
      uint8_t  acn_pid[12];      /* ACN Packet Identifier */
      uint16_t flength;          /* Flags (high 4 bits) & Length (low 12 bits) */
      uint32_t vector;           /* Layer Vector */
      uint8_t  cid[16];          /* Component Identifier (UUID) */
    } __attribute__((packed)) root;

    struct { /* Framing Layer: 77 bytes */
      uint16_t flength;          /* Flags (high 4 bits) & Length (low 12 bits) */
      uint32_t vector;           /* Layer Vector */
      uint8_t  source_name[64];  /* User Assigned Name of Source (UTF-8) */
      uint8_t  priority;         /* Packet Priority (0-200, default 100) */
      uint16_t reserved;         /* Reserved (should be always 0) */
      uint8_t  seq_number;       /* Sequence Number (detect duplicates or out of order packets) */
      uint8_t  options;          /* Options Flags (bit 7: preview data, bit 6: stream terminated) */
      uint16_t universe;         /* DMX Universe Number */
    } __attribute__((packed)) frame;

    struct { /* Device Management Protocol (DMP) Layer: 523 bytes */
      uint16_t flength;          /* Flags (high 4 bits) / Length (low 12 bits) */
      uint8_t  vector;           /* Layer Vector */
      uint8_t  type;             /* Address Type & Data Type */
      uint16_t first_addr;       /* First Property Address */
      uint16_t addr_inc;         /* Address Increment */
      uint16_t prop_val_cnt;     /* Property Value Count (1 + number of slots) */
      uint8_t  prop_val[513];    /* Property Values (DMX start code + slots data) */
    } __attribute__((packed)) dmp;
  } __attribute__((packed));

  uint8_t raw[638]; /* raw buffer view: 638 bytes */
} e131_packet_t;
```

This union provides two ways to access the data of an E1.31 packet:

1. directly using the `raw` member (typically used for receiving data from the network)
2. structurally using the `root`, `frame` and `dmp` members (typically for processing a packet)

You can easily create and initialize a new E1.31 packet to be used for sending using the `e131_pkt_init()` function.

See the examples sections to see how this data structure is used with libE131.

### Framing Layer Options

The library provides two convenience functions, `e131_get_option()` and `e131_set_option()` to manipulate the options flag in the Framing Layer of an E1.31 packet. The following is a description of the supported option constants:

* `E131_OPT_TERMINATED`: the current packet is the last one in the stream. The receiver should stop processing further packets.
* `E131_OPT_PREVIEW`: the data in the packet should be only used for preview purposes, e.g. console display, and not to drive live fixtures.

See the examples sections to see how Framing Layer options are used with libE131.

### Packet Validation

The library provides a convenience function, `e131_pkt_validate()`, to check if an E1.31 packet is valid to be processed by your application. This function returns a validation status from the `e131_error_t` enumeration. The following is a description of each available error constant:

* `E131_ERR_NONE`: Success (no validation error detected, you can process the packet).
* `E131_ERR_PREAMBLE_SIZE`: Invalid Preamble Size.
* `E131_ERR_POSTAMBLE_SIZE`: Invalid Post-amble Size.
* `E131_ERR_ACN_PID`: Invalid ACN Packet Identifier.
* `E131_ERR_VECTOR_ROOT`: Invalid Root Layer Vector.
* `E131_ERR_VECTOR_FRAME`: Invalid Framing Layer Vector.
* `E131_ERR_VECTOR_DMP`: Invalid Device Management Protocol (DMP) Layer Vector.
* `E131_ERR_TYPE_DMP`: Invalid DMP Address & Data Type.
* `E131_ERR_FIRST_ADDR_DMP`: Invalid DMP First Address.
* `E131_ERR_ADDR_INC_DMP`: Invalid DMP Address Increment.

The above error descriptions are also programatically obtainable using the `e131_strerror()` function. See the API documentation section for more information.

See the examples sections to see how the packet validation and error reporting functions are used with libE131.

## Unicast and Multicast Destinations

Most E1.31 software and hardware can be set up to communicate via two transport methods: **Unicast** and **Multicast**.

### Unicast Transmission

Unicast is a method of sending data across a network where two devices, the control PC and the lighting controller, are directly connected (or thru a network switch or router) and the channel control information meant for that specific controller is only sent to that controller. Unicast is a **point to point protocol** and the lighting channel information is only switched or routed to the device with the matching IP address.

You must have unique IP address in each controller. Using Unicast the data packets are sent directly to the device instead of being broadcast across the entire subnet.

### Multicast Transmission

Multicast is a method to send data across a network where a sender, typically a PC, broadcasts the data to all devices connected to the network subnet and the information about the channels are sent to all controllers connected to the network and every other device on the network.

Multicast is a **point to multipoint broadcast** where the controllers need to listen to and only respond to information they are configured to use. Your PC sequencing software or streaming tool sends multicast packets with an address of `239.255.<UHB>.<ULB>` where `UHB` is the Universe high byte and `LHB` is the Universe low byte. As an example, the address for universe `1` would be `239.255.0.1`. This is why using multicast addressing can be simpler to configure since this address is always the same for any device using that universe.

Depending on the device and the number of universes of data sent it can swamp the device and possibly end up causing a loss of data. Note however that this is not an issue for most networks until you get into the dozens of universes so it's not an issue for most users.

The data is received on multicast IP address not on the individual device IP address.

### Unicast vs. Multicast

**Unicast:**

* Allows more channels of data to controllers and bridges (commonly 12 Universes vs. 7 Universes for Multicast)
* Channels can only be sent to one controller per Universe

**Multicast:**

* Allows less channels of data to controllers and bridges (commonly 7 Universes vs. 12 for Unicast)
* Simpler network configuration since controllers don't need data address information
* Channels can be mirrored on multiple controllers since the same Universe can be used by multiple controllers.

## E1.31 (sACN) Universes

E1.31 transports lighting information in "Universes", which is a collection of up to 512 channels together. You can chose any universe number from **1-63999** and assign it to a block of channels in your sequencing software. While a Universe can contain up to 512 channels, it does not have to, and can be any number between 1-512 channels.

## libE131 API Documentation

### Public Library Constants

* *const uint16_t* `E131_DEFAULT_PORT`: default network port for E1.31 UDP data (5568).
* *const uint8_t* `E131_DEFAULT_PRIORITY`: default E1.31 packet priority (100).

### Public Library Functions

The library provides a number of functions to help you develop clients and servers that communicate using the E1.31 protocol. The following is a descriptive list of all functions provided.

See the examples sections to see how the most common API functions are used with libE131.

* `int e131_socket(void)`: Create a socket file descriptor suitable for E1.31 communication. On success, a file descriptor for the new socket is returned. On error, -1 is returned, and `errno` is set appropriately.

* `int e131_bind(int sockfd, const uint16_t port)`: Bind a socket file descriptor to a port number for E1.31 communication. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `int e131_unicast_dest(e131_addr_t *dest, const char *host, const uint16_t port)`: Initialize a unicast E1.31 destination using a host and port number. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `int e131_multicast_dest(e131_addr_t *dest, const uint16_t universe, const uint16_t port)`: Initialize a multicast E1.31 destination using a universe and port number. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `int e131_dest_str(char *str, const e131_addr_t *dest)`: Describe an E1.31 destination into a string (must be at least 22 bytes). On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `int e131_multicast_join(int sockfd, const uint16_t universe)`: Join a socket file descriptor to an E1.31 multicast group using a universe. On success, zero is returned.  On error, -1 is returned, and `errno` is set appropriately.

* `int e131_pkt_init(e131_packet_t *packet, const uint16_t universe, const uint16_t num_slots)`:  Initialize an E1.31 packet using a universe and a number of slots. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `bool e131_get_option(const e131_packet_t *packet, const e131_option_t option)`: Get the state of a framing option in an E1.31 packet.

* `int e131_set_option(e131_packet_t *packet, const e131_option_t option, const bool state)`: Set the state of a framing option in an E1.31 packet. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `ssize_t e131_send(int sockfd, const e131_packet_t *packet, const e131_addr_t *dest)`: Send an E1.31 packet to a socket file descriptor using a destination. On success, the number of bytes sent is returned. On error, -1 is returned, and `errno` is set appropriately.

* `ssize_t e131_recv(int sockfd, e131_packet_t *packet)`: Receive an E1.31 packet from a socket file descriptor. This function returns the number of bytes received, or -1 if an error occurred. In the event of an error, `errno` is set to indicate the error.

* `e131_error_t e131_pkt_validate(const e131_packet_t *packet)`: Validate that an E1.31 packet is well-formed. See the Packet Validation section.

* `bool e131_pkt_discard(const e131_packet_t *packet, const uint8_t last_seq_number)`: Check if an E1.31 packet should be discarded (sequence number out of order). This function uses the standard out-of-sequence detection algorithm defined in the E1.31 specification.

* `int e131_pkt_dump(FILE *stream, const e131_packet_t *packet)`: Dump an E1.31 packet to a stream (i.e. stdout, stderr). The output is formatted for human readable output. On success, zero is returned. On error, -1 is returned, and `errno` is set appropriately.

* `const char *e131_strdest(const e131_addr_t *dest)`: Return a string describing an E1.31 destination.

* `const char *e131_strerror(const e131_error_t error)`: Return a string describing an E1.31 error.

## Example: Creating a Client

Included in this repository is an example to create a simple E1.31 client (`examples/test_client.c`). Compile it using:

    gcc -Wall test_client.c -o test_client -le131

## Example: Creating a Server

Included in this repository is an example to create a simple E1.31 server (`examples/test_server.c`). Compile it using:

    gcc -Wall test_server.c -o test_server -le131

## Example Projects using libE131

* [E1.31 Xterm256 Console Viewer][e131-viewer]
* [E1.31 to AdaLight Bridge][e131-adalight-bridge]
* [E1.31 to MQTT Bridge][e131-mqtt-bridge]
* [MIDI to E1.31 Synthesizer][midi-e131-synth]

Also check out the [Node.js port][e131-node] of libE131.

[e131-viewer]: https://github.com/hhromic/e131-viewer
[e131-adalight-bridge]: https://github.com/hhromic/e131-adalight-bridge
[e131-mqtt-bridge]: https://github.com/hhromic/e131-mqtt-bridge
[midi-e131-synth]: https://github.com/hhromic/midi-e131-synth
[e131-node]: https://github.com/hhromic/e131-node

## License

This software is under the **Apache License 2.0**.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
