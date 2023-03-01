#ifndef SERIALSTORM_STREAM_BASE_H_INCLUDED
#define SERIALSTORM_STREAM_BASE_H_INCLUDED

#include <vector>
#include <sstream>
#include <stdexcept>
#include <limits>
#include "cast_if_required.h"

#if defined(SERIALSTORM_DEBUG_VERIFY_POD) || defined(SERIALSTORM_DEBUG_VERIFY_STRING) || defined(SERIALSTORM_DEBUG_VERIFY_BUFFER) || defined(SERIALSTORM_DEBUG_VERIFY_BLOB)
  #define SERIALSTORM_DEBUG_VERIFY
  // enable extra debugging measure: write verification headers and footers for every single piece of data serialised and verify on deserialisation
  #warning SerialStorm: extra debugging verification is enabled, this is not interoperable with instances where it is disabled.

  #ifndef SERIALSTORM_DEBUG_VERIFY_DELIMITER
    #define SERIALSTORM_DEBUG_VERIFY_DELIMITER std::string("_X_")
  #endif // SERIALSTORM_DEBUG_VERIFY_DELIMITER
#endif

#if defined(__EMSCRIPTEN__) && !defined(NO_DISABLE_EXCEPTION_CATCHING)
  // emscripten does not support exceptions by default
  #include <iostream>
  #define REPORT_ERROR std::cerr << "ERROR: " << ss.str() << std::endl; return {};
  #define REPORT_ERROR_NORETURN std::cerr << "ERROR: " << ss.str() << std::endl; return;
#else
  #define REPORT_ERROR throw std::runtime_error(ss.str());
  #define REPORT_ERROR_NORETURN throw std::runtime_error(ss.str());
#endif

namespace serialstorm {

template<typename StreamParam, template<typename> typename StreamT>
class stream_base {
  /// CRTP style static polymorphic base class for streams
private:
  enum class varint_size : uint8_t {                                            // the first byte that defines the size of unsigned integer this varint contains
    UINT_8  = 0b10000000u,                                                      // 1 byte, also bitmask to detect if this is a full byte
    UINT_16 = 0b10000001u,                                                      // 2 bytes
    UINT_32 = 0b10000010u,                                                      // 4 bytes
    UINT_64 = 0b10000011u                                                       // 8 bytes
  };

  mutable size_t read_pos = 0;                                                  // tracked read position in the stream, for tellp() - independent of underlying stream

public:
  // -------------------------- Status functions -------------------------------
  size_t tellp() const {
    /// Report read stream position, tracked independently of underlying stream
    return read_pos;
  }

  // ------------------------- Reading functions -------------------------------
  template<typename T>
  void read_buffer(T *data, size_t const size) const {
    /// CRTP polymorphic buffer read function
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    static_cast<StreamT<StreamParam> const*>(this)->read_buffer(data, size);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      check_verification("<B", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    read_pos += size;
  }
  template<typename T>
  void read_buffer(T *data) const {
    /// Wrapper function to automatically specify buffer size
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
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      check_verification("<P", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    return data;
  }

  template<typename T>
  inline T read_varint() const {
    /// Read a variable-size unsigned integer from the stream
    ///   Designed to work with uints only.  If our first byte is smaller than
    ///   128, we simply read it as-is.  Otherwise we flip the sign bit on the
    ///   first byte to get x, and read 2^x bytes as the uint, and try to fit it
    ///   into the supplied template type (which may overflow).
    uint8_t datasize(read_pod<uint8_t>());
    if(datasize & static_cast<uint8_t>(varint_size::UINT_8)) {                  // uint8_t half-byte (128), sent on its own
      switch(static_cast<varint_size>(datasize)) {
      case varint_size::UINT_8:                                                 // read a uint8_t  (1 byte)
        return cast_if_required<T>(read_pod<uint8_t>());
      case varint_size::UINT_16:                                                // read a uint16_t (2 bytes)
        return cast_if_required<T>(read_pod<uint16_t>());
      case varint_size::UINT_32:                                                // read a uint32_t (4 bytes)
        return cast_if_required<T>(read_pod<uint32_t>());
      case varint_size::UINT_64:                                                // read a uint64_t (8 bytes)
        return cast_if_required<T>(read_pod<uint64_t>());
      #pragma GCC diagnostic push
      #ifdef __clang__
        #pragma GCC diagnostic ignored "-Wcovered-switch-default"
      #endif // __clang__
      default:                                                                  // unknown type, protocol error
        std::stringstream ss;
        ss << "SerialStorm: Varint size " << static_cast<uint64_t>(datasize) << " is not in the protocol";
        REPORT_ERROR
      #pragma GCC diagnostic push
      }
    } else {                                                                    // this isn't a data size, this is a nibble (half-byte) containing the value itself
      return datasize;                                                          // the first byte is the value itself
    }
  }

  template<typename T>
  std::string read_string(T stringlength) const {
    /// CRTP polymorphic buffer read function: fill a string of the specified size from the stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    return static_cast<StreamT<StreamParam> const*>(this)->read_string(stringlength);
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      check_verification("<S", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
  }

  template<typename T>
  inline std::string read_varstring_fixed(size_t const length_max = 0) const {
    /// Read a varstring from the stream with the length type specified by the
    /// template parameter type, and optionally limit the string to a maximum
    /// length to prevent overflow or DOS attacks
    T const stringlength(read_pod<T>());
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "SerialStorm: Fixed varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
      REPORT_ERROR
    }
    return read_string(stringlength);
  }

  inline std::string read_varstring(size_t const length_max = 0) const {
    /// Read a varstring from the stream with the size automatically determined
    /// from a varint and optionally limit the string to a maximum length to
    /// prevent overflow or DOS attacks
    size_t const stringlength(read_varint<size_t>());
    if(length_max != 0 && stringlength > length_max) {                          // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "SerialStorm: Varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
      REPORT_ERROR
    }
    return read_string(stringlength);
  }

  inline void read_varblob(std::ostream &outstream,
                           size_t const length_max = 0,
                           size_t const buffer_max_size = 1024 * 1024) const {  // maximum buffer size until write out to stream, tuneable
    /// Read a sequence of binary data of arbitrary length using a buffer and output to a stream
    size_t const datalength(read_varint<size_t>());
    if(length_max != 0 && datalength > length_max) {                            // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "SerialStorm: Binary blob length " << datalength << " exceeded the permitted maximum of " << length_max;
      REPORT_ERROR_NORETURN
    }
    read_blob(outstream, datalength, buffer_max_size);
  }
  inline void read_blob(std::ostream &outstream,
                        size_t datalength,
                        size_t const buffer_max_size = 1024 * 1024) const {     // maximum buffer size until write out to stream, tuneable
    /// Read a sequence of binary data of known length using a buffer and output to a stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      check_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
    std::vector<char> buffer(std::min(datalength, buffer_max_size));            // size the buffer to the data length or max size, as appropriate
    for(; datalength != 0; datalength -= buffer.size()) {                       // if it takes more than one buffer fill to read the data, repeat
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
      read_buffer(buffer.data(), buffer.size());                                // you can write to the vector data directly
      outstream.write(buffer.data(), static_cast<std::streamsize>(buffer.size())); // blast it to the output stream - this could be made asynchronous
    }
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      check_verification("<L", __func__);
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
  }

  // ------------------------- Writing functions -------------------------------
  template<typename T>
  inline void write_buffer(T const &buffer) {
    /// CRTP polymorphic buffer write function passing whatever native buffer the stream takes
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(buffer);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      write_verification("<B");
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  }
  template<typename T>
  inline void write_buffer(T const *data, size_t const size) {
    /// CRTP polymorphic buffer write function
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "B>");
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(data, size);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BUFFER
      write_verification("<B");
    #endif // SERIALSTORM_DEBUG_VERIFY_BUFFER
  }

  template<typename T>
  inline void write_pod(T const &data) {
    /// Write a plain old data entity to the stream
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "P>");
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
    write_buffer(&data, sizeof(data));
    #ifdef SERIALSTORM_DEBUG_VERIFY_POD
      write_verification("<P");
    #endif // SERIALSTORM_DEBUG_VERIFY_POD
  }

  template<typename T, class = typename std::enable_if<std::is_unsigned<T>::value>::type>
  inline void write_varint(T const uint) {
    /// Write a variable-length unsigned integer to the stream
    ///   Designed to work with uints only.  If our int is smaller than 128, we
    ///   simply write it as-is, sign bit unset.  Otherwise we flip the sign bit
    ///   on the first byte, and set the value to log2 of the number of bytes.
    if(uint < static_cast<uint8_t>(varint_size::UINT_8)) {                      // uint8_t half-byte (128), sent on its own
      write_pod(static_cast<uint8_t>(uint));
    } else if(uint <= std::numeric_limits<uint8_t>::max()) {                    // fits in a uint8_t (256 aka 0b1'00000000 or 0x1'00)
      write_pod(varint_size::UINT_8);                                           // 1 byte
      write_pod(static_cast<uint8_t>(uint));
    } else if(uint <= std::numeric_limits<uint16_t>::max()) {                   // fits in a uint16_t (65536 aka 0b1'00000000'00000000 or 0x1'00'00)
      write_pod(varint_size::UINT_16);                                          // 2 bytes
      write_pod(static_cast<uint16_t>(uint));
    } else if(uint <= std::numeric_limits<uint32_t>::max()) {                   // fits in a uint32_t (4294967296 aka 0b1'00000000'00000000'00000000'00000000 or 0x1'00'00'00'00)
      write_pod(varint_size::UINT_32);                                          // 4 bytes
      write_pod(static_cast<uint32_t>(uint));
    } else {                                                                    // assume uint64_t (18446744073709551616 aka 0x1'0000'0000'0000'0000) max size
      write_pod(varint_size::UINT_64);                                          // 8 bytes
      write_pod(static_cast<uint64_t>(uint));
    }
  }

  inline void write_string(std::string const &string) {
    /// CRTP polymorphic buffer write function: write a bare string to the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "S>");
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
    static_cast<StreamT<StreamParam>*>(this)->write_string(string);
    #ifdef SERIALSTORM_DEBUG_VERIFY_STRING
      write_verification("<S");
    #endif // SERIALSTORM_DEBUG_VERIFY_STRING
  }

  template<typename T>
  inline void write_varstring_fixed(std::string const &string) {
    /// Write a string to the stream prefixed with a specific sized unsigned integer describing its type
    write_pod(static_cast<T>(string.length()));
    write_string(string);
  }

  inline void write_varstring(std::string const &string) {
    /// Write a string of arbitrary length to the stream
    write_varint(string.length());
    write_string(string);
  }

  template <typename T>
  inline void write_blob(std::vector<T> const &blob) {
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_verification("<L");
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
  }
  template <typename T>
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_verification(SERIALSTORM_DEBUG_VERIFY_DELIMITER + "L>");
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob, size);
    #ifdef SERIALSTORM_DEBUG_VERIFY_BLOB
      write_verification("<L");
    #endif // SERIALSTORM_DEBUG_VERIFY_BLOB
  }

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
      std::streamsize readbytes = instream.readsome(buffer.data(), static_cast<std::streamsize>(buffer.size())); // more efficient than just forcing it to fill the buffer
      write_buffer(buffer.data(), readbytes);
      datalength -= static_cast<size_t>(readbytes);
      if(datalength == 0 || readbytes == 0) {                                   // if it takes more than one buffer fill to read the data, repeat
        break;
      }
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
    }
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
        ss << "SerialStorm: Verification failed when attempting " << function_name << ": expected \"" << header << "\" and got \"" << data << "\"";
        REPORT_ERROR_NORETURN
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
