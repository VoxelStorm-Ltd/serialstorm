    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    read_buffer(data, sizeof(*data));
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification("<B", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  }

  template<typename T>
  inline T read_pod() const {
    /// Read a plain old data value from the stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "P>", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    T data;
    read_buffer(&data);
#ifndef SERIALSTORM_STREAM_BASE_H_INCLUDED
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
#define SERIALSTORM_STREAM_BASE_H_INCLUDED
      check_verification("<P", __func__);

    #endif // SERIALSTORM_DEBUG_VERIFY_POD
#include <vector>
    return data;
#include <sstream>
  }
#include <stdexcept>

#include <limits>
  template<typename T>
#include "cast_if_required.h"
  inline T read_varint() const {

    /// Read a variable-size unsigned integer from the stream
#if defined(SERIALSTORM_DEBUG_VERIFY_POD) || defined(SERIALSTORM_DEBUG_VERIFY_STRING) || defined(SERIALSTORM_DEBUG_VERIFY_BUFFER) || defined(SERIALSTORM_DEBUG_VERIFY_BLOB)
    ///   Designed to work with uints only.  If our first byte is smaller than
  #define SERIALSTORM_DEBUG_VERIFY
    ///   128, we simply read it as-is.  Otherwise we flip the sign bit on the
  // enable extra debugging measure: write verification headers and footers for every single piece of data serialised and verify on deserialisation
    ///   first byte to get x, and read 2^x bytes as the uint, and try to fit it
  #warning SerialStorm: extra debugging verification is enabled, this is not interoperable with instances where it is disabled.
    ///   into the supplied template type (which may overflow).

    uint8_t datasize(read_pod<uint8_t>());
  #ifndef SERIALSTORM_DEBUG_VERIFY_DELIMITER
    if(datasize & static_cast<uint8_t>(varint_size::UINT_8)) {                  // uint8_t half-byte (128), sent on its own
    #define SERIALSTORM_DEBUG_VERIFY_DELIMITER std::string("_X_")
      switch(static_cast<varint_size>(datasize)) {
  #endif // SERIALSTORM_DEBUG_VERIFY_DELIMITER

      case varint_size::UINT_8:                                                 // read a uint8_t  (1 byte)
#endif // SERIALSTORM_DEBUG_VERIFY
        return cast_if_required<T>(read_pod<uint8_t>());

      case varint_size::UINT_16:                                                // read a uint16_t (2 bytes)
namespace serialstorm {
        return cast_if_required<T>(read_pod<uint16_t>());

      case varint_size::UINT_32:                                                // read a uint32_t (4 bytes)
template<typename StreamParam, template<typename> typename StreamT>
        return cast_if_required<T>(read_pod<uint32_t>());
class stream_base {
      case varint_size::UINT_64:                                                // read a uint64_t (8 bytes)
  /// CRTP style static polymorphic base class for streams
        return cast_if_required<T>(read_pod<uint64_t>());
private:
      default:                                                                  // unknown type, protocol error
  enum class varint_size : uint8_t {                                            // the first byte that defines the size of unsigned integer this varint contains
        std::stringstream ss;
    UINT_8  = 0b10000000u,                                                      // 1 byte, also bitmask to detect if this is a full byte
        ss << "SerialStorm: Varint size " << static_cast<uint64_t>(datasize) << " is not in the protocol";
    UINT_16 = 0b10000001u,                                                      // 2 bytes
        throw std::runtime_error(ss.str());
    UINT_32 = 0b10000010u,                                                      // 4 bytes
      }
    UINT_64 = 0b10000011u                                                       // 8 bytes
  };
    } else {                                                                    // this isn't a data size, this is a nibble (half-byte) containing the value itself

      return datasize;                                                          // the first byte is the value itself
public:
    }
  // ------------------------- Reading functions -------------------------------
  }
  template<typename T>

  void read_buffer(T *data, size_t const size) const {
  template<typename T>
    /// CRTP polymorphic buffer read function
  std::string read_string(T stringlength) const {
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
    /// CRTP polymorphic buffer read function: fill a string of the specified size from the stream
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>", __func__);
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>", __func__);
    static_cast<StreamT<StreamParam> const*>(this)->read_buffer(data, size);
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification("<B", __func__);
    return static_cast<StreamT<StreamParam> const*>(this)->read_string(stringlength);
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
  }
      check_verification("<S", __func__);
  template<typename T>
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
  void read_buffer(T *data) const {
  }
    /// Wrapper function to automatically specify buffer size

    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
  template<typename T>
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>", __func__);
  inline std::string read_varstring_fixed(size_t const length_max = 0) const {
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    /// Read a varstring from the stream with the length type specified by the
    read_buffer(data, sizeof(*data));
    /// template parameter type, and optionally limit the string to a maximum
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
    /// length to prevent overflow or DOS attacks
      check_verification("<B", __func__);
    T const stringlength(read_pod<T>());
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  }
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum

      std::stringstream ss;
  template<typename T>
      ss << "SerialStorm: Fixed varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
  inline T read_pod() const {
      throw std::runtime_error(ss.str());
    /// Read a plain old data value from the stream
    }
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
    return read_string(stringlength);
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "P>", __func__);
  }
    #endif // SERIALSTORM_DEBUG_VERIFY_POD

    T data;
  inline std::string read_varstring(size_t const length_max = 0) const {
    read_buffer(&data);
    /// Read a varstring from the stream with the size automatically determined
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      check_verification("<P", __func__);
    /// from a varint and optionally limit the string to a maximum length to
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    /// prevent overflow or DOS attacks
    return data;
    size_t const stringlength(read_varint<size_t>());
  }
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum

      std::stringstream ss;
  template<typename T>
      ss << "SerialStorm: Varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
  inline T read_varint() const {
      throw std::runtime_error(ss.str());
    /// Read a variable-size unsigned integer from the stream
    }
    ///   Designed to work with uints only.  If our first byte is smaller than
    return read_string(stringlength);
    ///   128, we simply read it as-is.  Otherwise we flip the sign bit on the
  }
    ///   first byte to get x, and read 2^x bytes as the uint, and try to fit it

    ///   into the supplied template type (which may overflow).
  inline void read_varblob(std::ostream &outstream,
    uint8_t datasize(read_pod<uint8_t>());
                           size_t const length_max = 0,
    if(datasize & static_cast<uint8_t>(varint_size::UINT_8)) {                  // uint8_t half-byte (128), sent on its own
                           size_t const buffer_max_size = 1024 * 1024) const {  // maximum buffer size until write out to stream, tuneable
      switch(static_cast<varint_size>(datasize)) {
    /// Read a sequence of binary data of arbitrary length using a buffer and output to a stream
      case varint_size::UINT_8:                                                 // read a uint8_t  (1 byte)
    size_t const datalength(read_varint<size_t>());
        return cast_if_required<T>(read_pod<uint8_t>());
    if(length_max != 0 && datalength > length_max) {                            // optionally limit the info length to a safe maximum
      case varint_size::UINT_16:                                                // read a uint16_t (2 bytes)
      std::stringstream ss;
        return cast_if_required<T>(read_pod<uint16_t>());
      ss << "SerialStorm: Binary blob length " << datalength << " exceeded the permitted maximum of " << length_max;
      case varint_size::UINT_32:                                                // read a uint32_t (4 bytes)
      throw std::runtime_error(ss.str());
        return cast_if_required<T>(read_pod<uint32_t>());
    }
      case varint_size::UINT_64:                                                // read a uint64_t (8 bytes)
    read_blob(outstream, datalength, buffer_max_size);
        return cast_if_required<T>(read_pod<uint64_t>());
  }
      default:                                                                  // unknown type, protocol error
  inline void read_blob(std::ostream &outstream,
        std::stringstream ss;
                        size_t datalength,
        ss << "SerialStorm: Varint size " << static_cast<uint64_t>(datasize) << " is not in the protocol";
        throw std::runtime_error(ss.str());
                        size_t const buffer_max_size = 1024 * 1024) const {     // maximum buffer size until write out to stream, tuneable
      }
    /// Read a sequence of binary data of known length using a buffer and output to a stream
    } else {                                                                    // this isn't a data size, this is a nibble (half-byte) containing the value itself
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      return datasize;                                                          // the first byte is the value itself
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>", __func__);
    }
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
  }
    std::vector<char> buffer(std::min(datalength, buffer_max_size));            // size the buffer to the data length or max size, as appropriate

    for(; datalength != 0; datalength -= buffer.size()) {                       // if it takes more than one buffer fill to read the data, repeat
  template<typename T>
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
  std::string read_string(T stringlength) const {
      read_buffer(buffer.data(), buffer.size());                                // you can write to the vector data directly
    /// CRTP polymorphic buffer read function: fill a string of the specified size from the stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      outstream.write(buffer.data(), buffer.size());                            // blast it to the output stream - this could be made asynchronous
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>", __func__);
    }
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
    return static_cast<StreamT<StreamParam> const*>(this)->read_string(stringlength);
      check_verification("<L", __func__);
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      check_verification("<S", __func__);
  }
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING

  }
  // ------------------------- Writing functions -------------------------------

  template<typename T>
  template<typename T>
  inline void write_buffer(T const &buffer) {
  inline std::string read_varstring_fixed(size_t const length_max = 0) const {
    /// CRTP polymorphic buffer write function passing whatever native buffer the stream takes
    /// Read a varstring from the stream with the length type specified by the
    /// template parameter type, and optionally limit the string to a maximum
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    /// length to prevent overflow or DOS attacks
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
    T const stringlength(read_pod<T>());
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
      std::stringstream ss;
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(buffer);
      ss << "SerialStorm: Fixed varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      throw std::runtime_error(ss.str());
      write_verification("<B");
    }
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    return read_string(stringlength);
  }
  }
  template<typename T>

  inline void write_buffer(T const *data, size_t const size) {
  inline std::string read_varstring(size_t const length_max = 0) const {
    /// CRTP polymorphic buffer write function
    /// Read a varstring from the stream with the size automatically determined
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    /// from a varint and optionally limit the string to a maximum length to
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
    /// prevent overflow or DOS attacks
    size_t const stringlength(read_varint<size_t>());
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
      std::stringstream ss;
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(data, size);
      ss << "SerialStorm: Varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      throw std::runtime_error(ss.str());
      write_verification("<B");
    }
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    return read_string(stringlength);
  }
  }


  template<typename T>
  inline void read_varblob(std::ostream &outstream,
  inline void write_pod(T const &data) {
                           size_t const length_max = 0,
    /// Write a plain old data entity to the stream
                           size_t const buffer_max_size = 1024 * 1024) const {  // maximum buffer size until write out to stream, tuneable
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
    /// Read a sequence of binary data of arbitrary length using a buffer and output to a stream
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "P>");
    size_t const datalength(read_varint<size_t>());
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    if(length_max != 0 && datalength > length_max) {                            // optionally limit the info length to a safe maximum
    write_buffer(&data, sizeof(data));
      std::stringstream ss;
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      ss << "SerialStorm: Binary blob length " << datalength << " exceeded the permitted maximum of " << length_max;
      write_verification("<P");
      throw std::runtime_error(ss.str());
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    }
  }
    read_blob(outstream, datalength, buffer_max_size);

  }
  template<typename T>
  inline void read_blob(std::ostream &outstream,
  inline void write_varint(T const uint) {
                        size_t datalength,
    /// Write a variable-length unsigned integer to the stream
                        size_t const buffer_max_size = 1024 * 1024) const {     // maximum buffer size until write out to stream, tuneable
    ///   Designed to work with uints only.  If our int is smaller than 128, we
    /// Read a sequence of binary data of known length using a buffer and output to a stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
    ///   simply write it as-is, sign bit unset.  Otherwise we flip the sign bit
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>", __func__);
    ///   on the first byte, and set the value to log2 of the number of bytes.
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
    if(uint < static_cast<uint8_t>(varint_size::UINT_8)) {                      // uint8_t half-byte (128), sent on its own
    std::vector<char> buffer(std::min(datalength, buffer_max_size));            // size the buffer to the data length or max size, as appropriate
      write_pod(static_cast<uint8_t>(uint));
    for(; datalength != 0; datalength -= buffer.size()) {                       // if it takes more than one buffer fill to read the data, repeat
    } else if(uint <= std::numeric_limits<uint8_t>::max()) {                    // fits in a uint8_t (256 aka 0b1'00000000 or 0x1'00)
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
      write_pod(varint_size::UINT_8);                                           // 1 byte
      read_buffer(buffer.data(), buffer.size());                                // you can write to the vector data directly
      write_pod(static_cast<uint8_t>(uint));
      outstream.write(buffer.data(), buffer.size());                            // blast it to the output stream - this could be made asynchronous
    } else if(uint <= std::numeric_limits<uint16_t>::max()) {                   // fits in a uint16_t (65536 aka 0b1'00000000'00000000 or 0x1'00'00)
    }
      write_pod(varint_size::UINT_16);                                          // 2 bytes
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(static_cast<uint16_t>(uint));
      check_verification("<L", __func__);
    } else if(uint <= std::numeric_limits<uint32_t>::max()) {                   // fits in a uint32_t (4294967296 aka 0b1'00000000'00000000'00000000'00000000 or 0x1'00'00'00'00)
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(varint_size::UINT_32);                                          // 4 bytes
  }

      write_pod(static_cast<uint32_t>(uint));
  // ------------------------- Writing functions -------------------------------
    } else {                                                                    // assume uint64_t (18446744073709551616 aka 0x1'0000'0000'0000'0000) max size
  template<typename T>
      write_pod(varint_size::UINT_64);                                          // 8 bytes
  inline void write_buffer(T const &buffer) {
      write_pod(static_cast<uint64_t>(uint));
    /// CRTP polymorphic buffer write function passing whatever native buffer the stream takes
    }
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
  }
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER

      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");
  inline void write_string(std::string const &string) {
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    /// CRTP polymorphic buffer write function: write a bare string to the stream
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(buffer);
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      write_verification("<B");
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>");
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  }
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
  template<typename T>
    static_cast<StreamT<StreamParam>*>(this)->write_string(string);
  inline void write_buffer(T const *data, size_t const size) {
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
    /// CRTP polymorphic buffer write function
      write_verification("<S");
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
  }
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");

    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  template<typename T>
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(data, size);
  inline void write_varstring_fixed(std::string const &string) {
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      write_verification("<B");
    /// Write a string to the stream prefixed with a specific sized unsigned integer describing its type
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    write_pod(static_cast<T>(string.length()));
  }
    write_string(string);

  }
  template<typename T>

  inline void write_pod(T const &data) {
  inline void write_varstring(std::string const &string) {
    /// Write a plain old data entity to the stream
    /// Write a string of arbitrary length to the stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
    write_varint(string.length());
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "P>");
    write_string(string);
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
  }
    write_buffer(&data, sizeof(data));

    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
  template <typename T>
      write_verification("<P");
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
  inline void write_blob(std::vector<T> const &blob) {
  }
    /// CTCP polymorphic buffer write function: write a bare blob to the stream

    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
  template<typename T>
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
  inline void write_varint(T const uint) {
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
    /// Write a variable-length unsigned integer to the stream
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
    ///   Designed to work with uints only.  If our int is smaller than 128, we
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob);
    ///   simply write it as-is, sign bit unset.  Otherwise we flip the sign bit
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
    ///   on the first byte, and set the value to log2 of the number of bytes.
      write_verification("<L");
    if(uint < static_cast<uint8_t>(varint_size::UINT_8)) {                      // uint8_t half-byte (128), sent on its own
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(static_cast<uint8_t>(uint));
  }
    } else if(uint <= std::numeric_limits<uint8_t>::max()) {                    // fits in a uint8_t (256 aka 0b1'00000000 or 0x1'00)
  template <typename T>
      write_pod(varint_size::UINT_8);                                           // 1 byte
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
      write_pod(static_cast<uint8_t>(uint));
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
    } else if(uint <= std::numeric_limits<uint16_t>::max()) {                   // fits in a uint16_t (65536 aka 0b1'00000000'00000000 or 0x1'00'00)
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
      write_pod(varint_size::UINT_16);                                          // 2 bytes
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(static_cast<uint16_t>(uint));
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
    } else if(uint <= std::numeric_limits<uint32_t>::max()) {                   // fits in a uint32_t (4294967296 aka 0b1'00000000'00000000'00000000'00000000 or 0x1'00'00'00'00)
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(varint_size::UINT_32);                                          // 4 bytes
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob, size);
      write_pod(static_cast<uint32_t>(uint));
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
    } else {                                                                    // assume uint64_t (18446744073709551616 aka 0x1'0000'0000'0000'0000) max size
      write_verification("<L");
      write_pod(varint_size::UINT_64);                                          // 8 bytes
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      write_pod(static_cast<uint64_t>(uint));
  }
    }

  }
  inline void write_varblob(std::vector<char> const &blob) {

    /// Write a sequence of binary data of arbitrary length to the stream
  inline void write_string(std::string const &string) {
    write_varint(blob.size());
    /// CRTP polymorphic buffer write function: write a bare string to the stream
    write_blob(blob);
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
  }
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
  inline void write_varblob(std::istream &instream,
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>");
                            size_t datalength,
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    static_cast<StreamT<StreamParam>*>(this)->write_string(string);
                            size_t const buffer_max_size = 64 * 1024 * 1024) {  // maximum amount to read from the buffer each go, may read less
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
    /// Write a sequence of binary data of arbitrary length from an istream to
      write_verification("<S");
    /// the stream, buffering and sending chunks at a time
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    write_varint(datalength);
  }
    std::vector<char> buffer(std::min(datalength, buffer_max_size));            // size the buffer to the data length or max size, as appropriate

    for(;;) {
  template<typename T>
      size_t readbytes = instream.readsome(buffer.data(), buffer.size());       // more efficient than just forcing it to fill the buffer
  inline void write_varstring_fixed(std::string const &string) {
      write_buffer(buffer.data(), readbytes);
    /// Write a string to the stream prefixed with a specific sized unsigned integer describing its type
      datalength -= readbytes;
    write_pod(static_cast<T>(string.length()));
    write_string(string);
      if(datalength == 0 || readbytes == 0) {                                   // if it takes more than one buffer fill to read the data, repeat
  }
        break;

      }
  inline void write_varstring(std::string const &string) {
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
    /// Write a string of arbitrary length to the stream
    };
    write_varint(string.length());
  }
    write_string(string);

  }
private:

  #ifdef SERIALSTORM_DEBUG_VERIFY
  template <typename T>
    inline void check_verification(std::string const &header,
  inline void write_blob(std::vector<T> const &blob) {
                                   std::string const &function_name = __PRETTY_FUNCTION__) const {
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
      /// Verify a custom specified debugging header we expect from the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
      std::string data(header.length(), '?');
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      static_cast<StreamT<StreamParam> const*>(this)->read_buffer(&data[0], data.length());
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
      if(data != header) {
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
        std::stringstream ss;
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
        ss << "SerialStorm: Verification failed when attempting " << function_name << ": expected \"" << header << "\" and got \"" << data << "\"" << std::endl;
      write_verification("<L");
        throw std::runtime_error(ss.str());
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
      }
  }
    }
  template <typename T>

  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    inline void write_verification(std::string const &header) {
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
      /// Write a custom specified debugging header into the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
      static_cast<StreamT<StreamParam>*>(this)->write_buffer(header.c_str(), header.length());
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
    }
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
  #endif // SERIALSTORM_DEBUG_VERIFY
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
};
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob, size);

    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
}
      write_verification("<L");

    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
  }
#endif // SERIALSTORM_STREAM_BASE_H_INCLUDED

  inline void write_varblob(std::vector<char> const &blob) {
    /// Write a sequence of binary data of arbitrary length to the stream
    write_varint(blob.size());
    write_blob(blob);
  }
  inline void write_varblob(std::istream &instream,
                            size_t datalength,
                            size_t const buffer_max_size = 64 * 1024 * 1024) {  // maximum amount to read from the buffer each go, may read less
    /// Write a sequence of binary data of arbitrary length from an istream to
    /// the stream, buffering and sending chunks at a time
    write_varint(datalength);
    std::vector<char> buffer(std::min(datalength, buffer_max_size));            // size the buffer to the data length or max size, as appropriate
    for(;;) {
      size_t readbytes = instream.readsome(buffer.data(), buffer.size());       // more efficient than just forcing it to fill the buffer
      write_buffer(buffer.data(), readbytes);
      datalength -= readbytes;
      if(datalength == 0 || readbytes == 0) {                                   // if it takes more than one buffer fill to read the data, repeat
        break;
      }
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
    };
  }

private:
  #ifdef SERIALSTORM_DEBUG_VERIFY
    inline void check_verification(std::string const &header,
                                   std::string const &function_name = __PRETTY_FUNCTION__) const {
      /// Verify a custom specified debugging header we expect from the stream
      std::string data(header.length(), '?');
      static_cast<StreamT<StreamParam> const*>(this)->read_buffer(&data[0], data.length());
      if(data != header) {
        std::stringstream ss;
        ss << "SerialStorm: Verification failed when attempting " << function_name << ": expected \"" << header << "\" and got \"" << data << "\"" << std::endl;
        throw std::runtime_error(ss.str());
      }
    }

    inline void write_verification(std::string const &header) {
      /// Write a custom specified debugging header into the stream
      static_cast<StreamT<StreamParam>*>(this)->write_buffer(header.c_str(), header.length());
    }
  #endif // SERIALSTORM_DEBUG_VERIFY
};

}

#endif // SERIALSTORM_STREAM_BASE_H_INCLUDED
