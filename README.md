# libE131: a lightweight C/C++ library for the E1.31 (sACN) protocol

A lightweight C/C++ library that provides a simple API for packet, client and server programming to be used for communicating with devices implementing the E1.31 (sACN) protocol. A lot of information about E.131 (sACN) can be found on this [Wiki article][e131], from which most informational content about E1.31 in this document comes from.

ACN is a suite of protocols (via Ethernet) that is the industry standard for lighting and control. ESTA, the creator of the standard, ratified a subset of this protocol for "lightweight" devices which is called sACN (E1.31). This lightweight version allows microcontroller based lighting equipment to communicate via ethernet, without all the overhead of the full ACN protocol.

The simplest way to think about E1.31 is that it is a way to transport a large number of lighting control channels over a traditional ethernet network connection. E1.31 transports those channels in "Universes", which is a collection of up to 512 channels together. E.131 is ethernet based and is the data sent via UDP. The two accepted transport methods are multicast and unicast, the most common implementation is multicast. When using multicast all the controller needs to do is subscribe to the multicast address for the universe that you want to receive data from.

[e131]: http://www.doityourselfchristmas.com/wiki/index.php?title=E1.31_(Streaming-ACN)_Protocol

## Installation

## E1.31 (sACN) Packets

E1.31 Packet = Root Layer + Frame Layer + DMP (Device Management Protocol) Layer

```c
typedef union {
  struct {
    struct { /* Root Layer */
      uint16_t preamble_size;
      uint16_t postamble_size;
      uint8_t  acn_id[12];
      uint16_t flength;
      uint32_t vector;
      uint8_t  cid[16];
    } __attribute__((packed)) root;

    struct { /* Frame Layer */
      uint16_t flength;
      uint32_t vector;
      uint8_t  source_name[64];
      uint8_t  priority;
      uint16_t reserved;
      uint8_t  sequence_number;
      uint8_t  options;
      uint16_t universe;
    } __attribute__((packed)) frame;

    struct { /* DMP Layer */
      uint16_t flength;
      uint8_t  vector;
      uint8_t  type;
      uint16_t first_address;
      uint16_t address_increment;
      uint16_t property_value_count;
      uint8_t  property_values[513];
    } __attribute__((packed)) dmp;
  } __attribute__((packed));

  uint8_t raw[638]; /* Raw buffer data */
} e131_packet_t;
```

### Packet Validation

```c
typedef enum {
  E131_ERR_NONE,
  E131_ERR_NULLPTR,
  E131_ERR_ACN_ID,
  E131_ERR_VECTOR_ROOT,
  E131_ERR_VECTOR_FRAME,
  E131_ERR_VECTOR_DMP
} e131_error_t;
```

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

```c
/** Create a socket file descriptor suitable for E1.31 communication */
int e131_socket(void);

/** Bind a socket file descriptor to a port number for E1.31 communication */
int e131_bind(int sockfd, const uint16_t port);

/** Initialize a unicast E1.31 destination using a host and port number */
int e131_unicast_dest(const char *host, const uint16_t port, e131_addr_t *dest);

/** Initialize a multicast E1.31 destination using a universe and port number */
int e131_multicast_dest(const uint16_t universe, const uint16_t port, e131_addr_t *dest);

/** Join a socket file descriptor to a E1.31 multicast group using a universe */
int e131_multicast_join(int sockfd, const uint16_t universe);

/** Initialize a new E1.31 packet using a number of channels */
int e131_pkt_init(const uint16_t universe, const uint16_t num_channels, e131_packet_t *packet);

/** Send an E1.31 packet to a socket file descriptor using a destination */
ssize_t e131_send(int sockfd, const e131_packet_t *packet, const e131_addr_t *dest);

/** Receive an E1.31 packet from a socket file descriptor */
ssize_t e131_recv(int sockfd, e131_packet_t *packet);

/** Validate that an E1.31 packet is well-formed */
e131_error_t e131_pkt_validate(const e131_packet_t *packet);

/** Dump an E1.31 packet to the stderr output */
int e131_pkt_dump(const e131_packet_t *packet);
```

## Example: Creating a Client

## Example: Creating a Server
