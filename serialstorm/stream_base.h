#ifndef STREAM_BASE_H_INCLUDED
#define STREAM_BASE_H_INCLUDED

namespace serialstorm {

template<class StreamParam, template<class> class StreamT>
class stream_base {
  /// CRTP style static polymorphic base class for streams
private:
  enum class varint_size : uint8_t {        // the first byte that defines the size of unsigned integer this varint contains
    UINT8  = 0b10000000u,                   // 1 byte, also bitmask to detect if this is a full byte
    UINT16 = 0b10000001u,                   // 2 bytes
    UINT32 = 0b10000010u,                   // 4 bytes
    UINT64 = 0b10000011u                    // 8 bytes
  };

public:
  // ------------------------- Reading functions -------------------------------
  template<typename T>
  void read_buffer(T *data, size_t const size) const {
    /// CRTP polymorphic buffer read function
    static_cast<StreamT<StreamParam> const*>(this)->read_buffer(data, size);
  }
  template<typename T>
  void read_buffer(T *data) const {
    /// Wrapper function to automatically specify buffer size
    read_buffer(data, sizeof(*data));
  }

  template<typename T>
  inline T read_pod() const {
    /// Read a plain old data value from the stream
    T data;
    read_buffer(&data);
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
    if(datasize & static_cast<uint8_t>(varint_size::UINT8)) {   // uint8_t half-byte (128), sent on its own
      switch(static_cast<varint_size>(datasize)) {
      case varint_size::UINT8:                                  // read a uint8_t  (1 byte)
        return read_pod<uint8_t>();
      case varint_size::UINT16:                                 // read a uint16_t (2 bytes)
        return read_pod<uint16_t>();
      case varint_size::UINT32:                                 // read a uint32_t (4 bytes)
        return read_pod<uint32_t>();
      case varint_size::UINT64:                                 // read a uint64_t (8 bytes)
        return read_pod<uint64_t>();
      default:                                                  // unknown type, protocol error
        std::stringstream ss;
        ss << "Varint size " << static_cast<uint64_t>(datasize) << " is not in the protocol";
        throw std::runtime_error(ss.str());
      }
    } else {                                              // this isn't a data size, this is a nibble (half-byte) containing the value itself
      return datasize;                                    // the first byte is the value itself
    }
  }

  template<typename T>
  std::string read_string(T stringlength) const {
    /// CRTP polymorphic buffer read function: fill a string of the specified size from the stream
    return static_cast<StreamT<StreamParam> const*>(this)->read_string(stringlength);
  }

  template<typename T>
  inline std::string read_varstring_fixed(size_t const length_max = 0) const {
    /// Read a varstring from the stream with the length type specified by the
    /// template parameter type, and optionally limit the string to a maximum
    /// length to prevent overflow or DOS attacks
    T const stringlength(read_pod<T>());
    if(length_max != 0 && stringlength > length_max) {      // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "Fixed varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
      throw std::runtime_error(ss.str());
    }
    return read_string(stringlength);
  }

  inline std::string read_varstring(size_t const length_max = 0) const {
    /// Read a varstring from the stream with the size automatically determined
    /// from a varint and optionally limit the string to a maximum length to
    /// prevent overflow or DOS attacks
    size_t const stringlength(read_varint<size_t>());
    if(length_max != 0 && stringlength > length_max) {      // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "Varstring length " << stringlength << " exceeded the permitted maximum of " << length_max;
      throw std::runtime_error(ss.str());
    }
    return read_string(stringlength);
  }

  inline void read_varblob(std::ostream &outstream,
                           size_t const length_max = 0,
                           size_t const buffer_max_size = 1024 * 1024) const {    // maximum buffer size until write out to stream, tuneable
    /// Read a sequence of binary data of arbitrary length using a buffer and output to a stream
    size_t datalength(read_varint<size_t>());
    if(length_max != 0 && datalength > length_max) {                    // optionally limit the info length to a safe maximum
      std::stringstream ss;
      ss << "Binary blob length " << datalength << " exceeded the permitted maximum of " << length_max;
      throw std::runtime_error(ss.str());
    }
    std::vector<char> buffer(std::min(datalength, buffer_max_size));    // size the buffer to the data length or max size, as appropriate
    for(; datalength != 0; datalength -= buffer.size()) {               // if it takes more than one buffer fill to read the data, repeat
      buffer.resize(std::min(datalength, buffer_max_size));             // shrink the buffer if there's not enough data left to fill it
      read_buffer(buffer.data(), buffer.size());                        // you can write to the vector data directly
      outstream.write(buffer.data(), buffer.size());                    // blast it to the output stream - this could be made asynchronous
    }
  }

  // ------------------------- Writing functions -------------------------------
  template<typename T>
  inline void write_buffer(T const &buffer) {
    /// CRTP polymorphic buffer write function passing whatever native buffer the stream takes
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(buffer);
  }
  template<typename T>
  inline void write_buffer(T const *data, size_t const size) {
    /// CRTP polymorphic buffer write function
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    static_cast<StreamT<StreamParam>*>(this)->write_buffer(data, size);
  }

  template<typename T>
  inline void write_pod(T const &data) {
    /// Write a plain old data entity to the stream
    write_buffer(&data, sizeof(data));
  }

  template<typename T>
  inline void write_varint(T const uint) {
    /// Write a variable-length unsigned integer to the stream
    ///   Designed to work with uints only.  If our int is smaller than 128, we
    ///   simply write it as-is, sign bit unset.  Otherwise we flip the sign bit
    ///   on the first byte, and set the value to log2 of the number of bytes.
    if(uint < static_cast<uint8_t>(varint_size::UINT8)) {       // uint8_t half-byte (128), sent on its own
      write_pod(static_cast<uint8_t>(uint));
    } else if(uint <= std::numeric_limits<uint8_t>::max()) {    // fits in a uint8_t (256 aka 0b1'00000000 or 0x1'00)
      write_pod(varint_size::UINT8);                            // 1 byte
      write_pod(static_cast<uint8_t>(uint));
    } else if(uint <= std::numeric_limits<uint16_t>::max()) {   // fits in a uint16_t (65536 aka 0b1'00000000'00000000 or 0x1'00'00)
      write_pod(varint_size::UINT16);                           // 2 bytes
      write_pod(static_cast<uint16_t>(uint));
    } else if(uint <= std::numeric_limits<uint32_t>::max()) {   // fits in a uint32_t (4294967296 aka 0b1'00000000'00000000'00000000'00000000 or 0x1'00'00'00'00)
      write_pod(varint_size::UINT32);                           // 4 bytes
      write_pod(static_cast<uint32_t>(uint));
    } else {                                                    // assume uint64_t (18446744073709551616 aka 0x1'0000'0000'0000'0000) max size
      write_pod(varint_size::UINT64);                           // 8 bytes
      write_pod(static_cast<uint64_t>(uint));
    }
  }

  inline void write_string(std::string const &string) {
    /// CRTP polymorphic buffer write function: write a bare string to the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    static_cast<StreamT<StreamParam>*>(this)->write_string(string);
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
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob);
  }
  template <typename T>
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    /// CTCP polymorphic buffer write function: write a bare blob to the stream
    /// Note: this cannot be safely decoded on its own unless its length is known by the recipient
    static_cast<StreamT<StreamParam>*>(this)->write_blob(blob, size);
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
    size_t readbytes = 0;
    for(;;) {
      readbytes = instream.readsome(buffer.data(), buffer.size());              // more efficient than just forcing it to fill the buffer
      write_buffer(buffer.data(), readbytes);
      datalength -= readbytes;
      if(datalength == 0 || readbytes == 0) {                                   // if it takes more than one buffer fill to read the data, repeat
        break;
      }
      buffer.resize(std::min(datalength, buffer_max_size));                     // shrink the buffer if there's not enough data left to fill it
    };
  }
};

}

#endif // STREAM_BASE_H_INCLUDED
