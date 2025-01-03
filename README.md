# SerialStorm

VoxelStorm's simple, high performance, header-only, stream serialisation library.

SerialStorm provides a stream wrapper with a number of functions for encoding and decoding numerical, string, and binary data to and from streams.

It can work with `std::` streams (stringstreams, file streams), which is what you would use in the majority of applications within an existing network engine.  It can also read and write to Boost Asio directly, using either sync or async interfaces, which can be useful for simple programs without an existing network engine in place.

SerialStorm also provides a CRTP interface for easily adding support for your own streams, or stream-like interfaces.

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

### Flow of operation

Flow of operation with SerialStorm is strictly sequential; that is, there is no separate step of assembling your buffers and then committing them.  The sender just uses `write` functions on the stream.  It's up to the caller to finalise whatever the stream requires when finished, if anything - you may want to get your ostringstream to a buffer with `.str()` or to `flush()` your fstream for example.  SerialStorm doesn't assume anything about what you want to do with the stream, it just appends your data to it.

The same goes for deserialisation - just use `read` functions on the stream.  It's up to you to make sure the data is available.  When using the std::stream adapter, if `NDEBUG` is not defined, SerialStorm will report short reads as errors, but this is intended just as a debugging aid.

There is no inherent thread safety, beyond whatever is provided by your stream.  If you want to write to a single stream from multiple threads, it's up to you what synchronisation you need, if any.

### VarInt encoding
One of the key techniques to squeezing serialised data into the fewest possible bytes, without whole-stream compression, is to use variable-size integer encoding.

A common pattern when sending a block of data is to send an integer representation of the length of the data, and then send the data.  For example, if sending a vector of coordinates, you would send an integer representing the number of elements in the vector, and then send each set of coordinates in turn.  The recipient first reads an integer from the stream, and uses this value to determine how many times it should subsequently read coordinates from the stream.

But what if you don't know the range of data lengths to expect?  If you wanted to be maximally efficient, you could encode the data length as a single unsigned byte (`uint8_t`), but this would limit you to sending 255 or fewer elements.  If you wanted to support any number of elements, you'd have to send a larger integer of `size_t` (typically as `uint64_t`), but if you're just sending a small number, you're using 8 times more bytes than you need.

The solution is a `VarInt`.  This type encoding is optimised for small numbers, with values under 128 taking up just one byte on the wire, but supports any size unsigned integer up to 64bits.  Any value over 128 costs one additional byte over the minimum size representation for this value.  This is It works by using the most significant bit in the first byte to encode whether the byte value should be interpreted as-is, or decoded as a size type for the following bytes.  The size of the data is determined by the actual value you send, not the potential type.  Byte sizes for integer values are as follows:
- 0-127: 1 byte
- 128-255 (to `uint8_t` max): 2 bytes
- 256-65536 (to `uint16_t` max): 3 bytes
- 65536-4294967296 (to `uint32_t` max): 5 bytes
- 4294967297-18446744073709551616 (to `uint64_t` max): 9 bytes

This is the key building block used in `VarString` and `VarBlob`, and it's the integer type you should always use if you don't know how big a number you need to send or receive is.  The only time you'd want to prefer fixed size integers over VarInt is when you know for sure that the number you're sending requires the full integer's range.

### Data types

A key concept to understand is that in SerialStorm, all data types are interchangeable; it's up to you to ensure what you're transmitting is compatible with what you're receiving, but you can interpret data you receive however you like - read functions don't have to match the write functions used to send that data.  For example, a server may send a file using the `blob` functions, and the client may receive them as a `buffer`, or vice versa.  A `VarString` can be interpreted as a `VarBlob`, or split into components and read manually as a `VarInt` followed by a `String`.  This flexibility allows you to store data directly into its final destination, without incurring unnecessary buffer copies.

#### POD
At its core, all of the data you send is plain old data (POD).  In SerialStorm, we call it POD if `sizeof(my_data)` covers the whole of the data you wish to send, and this does not contain pointers to other memory.  So, you would use POD functions to send simple types such as fixed size integers or floats, simple structs containing fixed size data, etc.

Note that packing is up to you.  If you send structs as POD, it's a good idea to ensure the struct is packed (with `__attribute__((__packed__))` or similar), but SerialStorm does not require this.  If you opt to allow padding in structs you send in this way, make sure that the reading and writing side both agree precisely on how such a struct is padded.

#### Buffer
In SerialStorm, a buffer is defined by an address in memory and a size.  SerialStorm will just send and receive the exact size and content of the buffer you specify.

#### VarInt
An unsigned integer of an arbitrary size, encoded in the shortest viable length for the value it contains.  See the section on VarInt encoding for more info about this special type.

#### String
Raw string data.  This does not attempt to encode its length, so the exact length of the string must be known in advance - either hard-coded, or sent ahead with an integer.  This is rarely used on its own, but is occasionally useful when transmitting fixed length strings.  For variable length strings, it's easier to use `VarString` below.

#### VarString
Raw string data, prefixed with a VarInt.  This is the simplest way to transfer string data.  Reading functions allow you to specify an optional buffer size limit, to prevent overflow attacks by untrusted clients.

#### Blob
A fixed size blob of raw binary data.  In SerialStorm, blob functions differ from buffer functions in that they are read and written in chunks.  The use case is to limit required memory allocations, even for very large binary transmissions.  The `read_blob` functions allow you to specify a chunk size for the buffer to use.  Vectors of bytes can be sent as blobs, as can std::istreams, making this useful for sending files directly from a filesystem.  For a plain blob, the exact size must be known by the recipient.  For dynamically sized objects, use `VarBlob`.

#### VarBlob

Similar to `VarString`, this is simply a `Blob` prefixed with a `VarInt`, used to encode the size of the blob data.

## Usage

## Reading and writing

### Writing

```cpp
void write_pod(T const &data)
```
Write plain data of type `T`.  Use this to transmit basic types, or structs composed of basic types.

The recipient must read with an equivalent `read_pod<T>()` function.

If using this to send structs, see notes above about packing.

---
```cpp
void write_varint(T const uint)
```
Write a `VarInt` - a variable sized unsigned integer.  The size is determined by how big a number you're sending, not the maximum range of the type.  This should be your go-to whenever sending numerical information, unless you know for sure you need the entire size of a fixed integer.  It can send values up to the maximum that can be represented by `uint64_t`.

Note this is usable with unsigned integers only.  For signed integers, prefer a fixed type rather than casting to unsigned and using `VarInt` - the latter will work, but will often end up using the largest representation (9 bytes) for negative numbers.

The recipient must read from the stream with `read_varint`.

---
```cpp
void write_string(std::string const &string)
```
Write a string of a fixed length.  When writing, SerialStorm will send whatever size the string is, but the client must know the size in advance in order to read this.  Useful for fixed size strings - for most strings, prefer the `varstring` functions instead.

The recipient should read from the stream with `read_string`, `read_buffer`, or `read_blob`.

---
```cpp
void write_varstring(std::string const &string)
```
Write a string of an arbitrary length, along with info about its length.  This is functionally equivalent to manually writing `write_varint(string.length())` followed by `write_string(string)`.

The recipient should read this with `read_varstring` or `read_varint` followed by `read_string`, `read_buffer`, or `read_blob`.

---
```cpp
void write_varstring_fixed(std::string const &string)
```
A special variant of `write_varstring`, which uses a fixed size integer instead of `VarInt`.  The size of the integer is determined by the size type of the string, usually `size_t`.  Note that the size type is platform-dependent, so take care that you have compatibility between sender and recipient.  Unless you have a specific reason to use this, prefer `write_varstring`.  

The recipient should read this with `read_varstring_fixed`, or `read_pod<size_t>()` followed by `read_string`, `read_buffer`, or `read_blob`.

---
```cpp
void write_buffer(T const &buffer)
```
Write a buffer of binary data, with size defined by `sizeof(buffer)`.  Cannot be read by the recipient without knowing the size first.

The recipient should read this with `read_buffer`, or `read_string`, or `read_blob`.

---
```cpp
void write_buffer(T const *data, size_t const size)
```
As above, this overload allows you to specify and address in memory and a size.  More commonly used for interoperation with C-style APIs.  The recipient must know the size to read this.

The recipient should read this with `read_buffer`, or `read_string`, or `read_blob`.

---
```cpp
void write_blob(std::vector<T> const &blob) 
```
Sends a `std::vector` of any POD or struct type as a blob.  The recipient must know the size of the data to be received.

The recipient should read this with `read_blob`, `read_buffer` or `read_string`.

---
```cpp
void write_blob(std::vector<T> const &blob, size_t const size)
```
As above, but with a fixed size - use to send a subset of the vector.  Take care to ensure that `size` is not be greater than `blob.size()`.

---
```cpp
void write_varblob(std::vector<char> const &blob)
```
Write a blob of arbitrary size, along with info about its size.  This is functionally equivalent to manually writing `write_varint(blob.size())` followed by `write_blob(blob)`.

The recipient should read this with `read_varblob` or `read_varstring`, or with `read_varint` followed by `read_blob`, `read_buffer` or `read_string`.

---
```cpp
void write_varblob(std::istream &instream, size_t datalength, size_t const buffer_max_size = 64 * 1024 * 1024)
```
Write a blob from an input stream (such as a file), along with info about its size.  The length `datalength` must be available to read from the stream.  Anything in the stream beyond `datalength` is left unconsumed.

This function accepts an arbitrary max buffer size; if `buffer_max_size` is smaller than `datalength`, the data will be split into chunks of max buffer size or smaller.  Use this to keep memory growth under control when reading and writing simultaneously, for example when sending files from disk.

The recipient should read this with `read_varblob` or `read_varstring`, or with `read_varint` followed by `read_blob`, `read_buffer` or `read_string`.

### Reading

```cpp
T read_pod()
```
Read plain old data (POD) or a simple struct of type T, with `auto my_data{read_pod<my_type>()}`.  The size of the data must match what is sent.

If using this to read a struct, see notes above about packing.

---
```cpp
T read_varint()
```
Read a variable size unsigned integer (`VarInt`), interpreted as whatever type you specify.  The maximum size of the number it can represent is equivalent to `uint64_t`, so if you don't know the range of input you're expecting, use `read_varint<uint64_t>()`.

---
```cpp
std::string read_string(T stringlength)
```
Read a string of a fixed length.  Length must match what is sent.

---
```cpp
std::string read_varstring(size_t const length_max = 0)
```
Read a string of arbitrary length.

Optionally provide a maximum length limit, to prevent attacks by untrusted clients.

---
```cpp
std::string read_varstring_fixed(size_t const length_max = 0)
```
Read a variable length string with a fixed size length identifier (sent by `write_varstring_fixed`).  Special-purpose - usually prefer `read_varstring`.  If using this, make sure that `size_t` is identical in size on the sender and recipient platforms.

---
```cpp
void read_buffer(T *data, size_t const size)
```
Read binary data into a buffer at the given address, with the given size.  The `size` must match what is sent.

---
```cpp
void read_buffer(T *data)
```
Read binary data into a buffer at the given address; `sizeof(*data)` must match what is sent.

---
```cpp
read_blob(std::ostream &outstream, size_t datalength, size_t const buffer_max_size = 1024 * 1024)
```
Read binary data to a stream.  The data length must match what is sent.

If `buffer_max_size` is specified and is smaller than `datalength`, the read will be split into smaller chunks to limit memory allocation.  Useful for reading large data to stream elsewhere, for example downloading a file to disk.

---
```cpp
read_varblob(std::ostream &outstream, size_t const length_max = 0, size_t const buffer_max_size = 1024 * 1024)
```
As for `read_blob` above, but use to read data where the length is encoded as a `VarInt` up front, as with `write_varblob`.

## Adding new streams

TODO

### Verification mode

It is the user's responsibility to always read from the stream the same amount of data that is written, in order to keep the stream in sync.  How you ensure this is done is up to you.

In order to assist in debugging, SerialStorm provides a basic verification mode, where it adds prefixes and suffixes to certain structures, and verifies those are received.  This is a compile-time switch, enabled by defining `SERIALSTORM_DEBUG_VERIFY`, or one of the more specific verification macros shown below.  If enabling this, ensure that both sides (server and client) are built with the same mode - otherwise, they will not be interoperable.

```cpp
#if defined(SERIALSTORM_DEBUG_VERIFY_POD) || defined(SERIALSTORM_DEBUG_VERIFY_STRING) || defined(SERIALSTORM_DEBUG_VERIFY_BUFFER) || defined(SERIALSTORM_DEBUG_VERIFY_BLOB)
  #define SERIALSTORM_DEBUG_VERIFY
  // enable extra debugging measure: write verification headers and footers for every single piece of data serialised and verify on deserialisation
  #warning SerialStorm: extra debugging verification is enabled, this is not interoperable with instances where it is disabled.

  #ifndef SERIALSTORM_DEBUG_VERIFY_DELIMITER
    #define SERIALSTORM_DEBUG_VERIFY_DELIMITER std::string("_X_")
  #endif // SERIALSTORM_DEBUG_VERIFY_DELIMITER
#endif
```

### Exceptions

TODO

REPORT_ERROR_NORETURN

Emscripten: NO_DISABLE_EXCEPTION_CATCHING
