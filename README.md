# SerialStorm

VoxelStorm's simple, high performance, header-only, stream serialisation library.

SerialStorm provides a stream wrapper with a number of functions for encoding and decoding numerical, string, and binary data to and from streams.

It can work with `std::` streams (stringstreams, file streams), which is what you would use in the majority of applications within an existing network engine.  It can also read and write to Boost Asio directly, using either sync or async interfaces, which can be useful for simple programs without an existing network engine in place.

## Performance concepts

Designed for massively multiplayer game engines, performance is SerialStorm's first priority, taking precedence over ease of use when necessary.  You don't pay for anything you don't use, and SerialStorm doesn't add any forced safety features or training wheels which might impact performance even slightly.

### Every byte is precious.  Nothing is written on the wire unless it's needed.
SerialStorm relies on always knowing the exact size of the data you intend to receive before you try to read it.  If you don't know the type or size of the next object you're going to read, then it's up to you to serialise that information first, and you have the freedom to do so with the most efficient algorithm available for the expected data range.  SerialStorm provides helpers to send common types, such as fixed POD types, binary blobs, and strings.  It also provides an innovative byte-optimal variable integer encoding (VarInt) to make sure not a single byte is wasted.  And if you do know the type and size of the next object you're going to read, we don't need to waste time on the wire sending that information, we can just send it.

Time taken on the wire is the single most important thing for the network performance of your game or application, both in terms of latency and throughput.  Within a given network packet, the more bytes you're sending in the payload, the longer it takes to transmit, and the effect is cumulative.  While modern networks are fast, to really get the best performance and latency, it's important to be very miserly and count every byte.  Any additional CPU cost of encoding and decoding is generally negligible compared to the time taken on the network when sending non optimal data.

A common pattern with serialisation libraries is to introduce field delimiters or padding bytes at the end of fields.  This allows such libraries to read until a delimiter, and avoid having to declare object sizes in advance.  This can be a good approach when sending large streams of arbitrary string data, but comes with several drawbacks.  First, most massively multiplayer game engines rely on sending very small bundles of data, very frequently - for example, sets of coordinates of entities, which must update many times a second, for many entities.  If your coordinates consist of 3x 32bit floats (12 bytes total), your field delimiters would add three more bytes, increasing the data you send for such a simple structure by 25%!

The second drawback is that such protocols, if they rely only on delimiters, are open to client attacks - something you need to consider carefully if running a server catering to untrusted clients, such as in a massively multiplayer game.  A common attack of this type is the slow transmission attack - a client deliberately sends a buffer of unknown size, generating data continuously, with no end.  The server is always waiting for the buffer to finish, so if not handled carefully, can lead to buffer overflows or even memory exhaustion.  Even if input buffer sizes are limited, if the server does not have sophisticated timeout detection in place, such an attack can tie up resources and lead to denial of service when repeated in parallel.  While managing timeouts in your network application is up to you, SerialStorm's design makes it difficult to accidentally expose yourself to such attacks, as the size of any data you are about to receive must be known by you in advance, and SerialStorm will never read one byte more from your stream than you instruct it to.

While SerialStorm doesn't force any kind of consistency checking on you, if you want to include padding bytes or delimiters of your own in your stream, and verify those for consistency, you can of course do that yourself.  Just include whatever delimiters or verification bytes you feel are necessary as part of your protocol - either as members of your structs, or as steps in your protocol - and verify them as you read.

### Type agreement is up to the user
Many serialisation protocols force an agreement of data types between sender and receiver.  They do this either by generating code which defines rigid structures (such as protobufs), or by sending type information on the wire along with the data itself.  The drawback of the latter approach is obvious, as we are always sending information that the recipient should often already know.  The drawback of the former approach is that it often forces memory copies on the reader or writer side, to coerce application data into the network data format.

In real applications, the client does not always want to interpret received data into the exact same structure that the server sends.  For example, if the server sends a set of coordinates to the client, the server may store these as a struct containing three floats.  But the client may wish to store these floats in a vector instead.  In that case, the small cost of first reading a struct and then copying into the vector is always incurred in any protocol that has an agreed data structure, but can easily be avoided with SerialStorm.

Of course if you want to send type data ahead of a specific object, you can easily do so - it's just not forced on you.

This also means there is no protocol meta-description, and no code generation.  If you want to layer such a thing on top of SerialStorm, you can.  But the most direct way to use it is to carefully write matching code on the sender and recipient side - the sender sends type A of size X, the recipient reads type A of size X.  It's a bit more work, but in exchange you gain flexibility and performance that no other serialisation library can match.

### Version handling is up to the user
Over time, protocols evolve, and for a server it's generally useful to be able to detect the protocol version of the client (or vice versa), in order to ensure you're both speaking the same language.  Many network serialisation libraries pride themselves on having built in version control for every message received - this is great for foolproof consistency, but the latency cost of always sending such potentially redundant information is immense!  Such libraries also enforce a single design decision aboiut how version control is handled.  But you might want a different approach - for example, a game server might want to just reject all clients that aren't using the latest version.  Or you may want to seamlessly support clients of multiple versions, switching your logic path entirely depending on what version of a client connects to the server.  Or a server might speak only one protocol, and the client should do the work of identifying which version of server they're speaking to, and switch protocol appropriately.  SerialStorm leaves it entirely up to you how you want to handle protocol versioning, if you even want to handle it at all.

### Use the lowest level available
SerialStorm is a simple serialisation library; that means you don't pay for anything you don't need, and it intentionally has no bells and whistles.  It is just for serialising and deserialising data to and from streams, quickly and with the minimum of fuss.  There is no built in compression - it's up to you if you need it, and at which layer(s) you want to use it.  There's no schema language to learn, no preprocessor or code generator to include in your build step.  There's no attempt to handle memory allocation (that's entirely up to your streams).

As a result, SerialStorm can act as a core building block in any networking library without forcing any compromises on the user whatsoever.


## Concepts

### POD

### Buffer

### VarInt

### String

### VarString

### Blob

### VarBlob


## Usage

## Reading and writing

### Writing

void write_buffer(T const &buffer)

void write_buffer(T const *data, size_t const size)

void write_pod(T const &data)

void write_varint(T const uint)

void write_string(std::string const &string)

void write_varstring_fixed(std::string const &string)

void write_varstring(std::string const &string)

void write_blob(std::vector<T> const &blob) 

void write_blob(std::vector<T> const &blob, size_t const size)

void write_varblob(std::vector<char> const &blob)

void write_varblob(std::istream &instream,
                            size_t datalength,
                            size_t const buffer_max_size = 64 * 1024 * 1024)


### Reading

void read_buffer(T *data, size_t const size)

void read_buffer(T *data)

T read_pod()

T read_varint()

std::string read_string(T stringlength)

std::string read_varstring_fixed(size_t const length_max = 0)

std::string read_varstring(size_t const length_max = 0)

read_blob(std::ostream &outstream,
                        size_t datalength,
                        size_t const buffer_max_size = 1024 * 1024)

read_varblob(std::ostream &outstream,
                           size_t const length_max = 0,
                           size_t const buffer_max_size = 1024 * 1024)


### Verification mode

SERIALSTORM_DEBUG_VERIFY
