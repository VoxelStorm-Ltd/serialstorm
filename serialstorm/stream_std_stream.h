#ifndef SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED
#define SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED

#include "stream_base.h"
#include <fstream>
#ifndef NDEBUG
  #include <iostream>
#endif

namespace serialstorm {

template<typename StreamT>
class stream_std_stream : public stream_base<StreamT, stream_std_stream> {
  /// Stream handler to manage a std::istream, std::ostream, std::fstream etc
public:
  StreamT &stream;

  constexpr explicit stream_std_stream(StreamT &new_stream)
    : stream(new_stream) {
    /// Specific constructor
  }

  stream_std_stream(const stream_std_stream&) = delete;

  stream_std_stream& operator=(const stream_std_stream&) = delete;

  template<typename T>
  void read_buffer(T *data, size_t const size) const {
    /// Read a block of data of the specified size from the stream to the target buffer asynchronously
    stream.read(reinterpret_cast<char*>(data), size);
    #ifndef NDEBUG
      if(!stream) {
        std::stringstream ss;
        ss << "SerialStorm: short read on stream: " << stream.gcount() << " read out of " << size << " requested." << std::endl;
        REPORT_ERROR
      }
    #endif
  }

  template<typename T>
  std::string read_string(T const stringlength) const {
    /// Read size bytes from the stream into a string asynchronously
    #ifdef NDEBUG
      std::string string(stringlength, '\0');                                   // use null byte as default fill to minimise risk in release mode
    #else
      std::string string(stringlength, '?');                                    // use ? as a marker character to visibly show if we somehow end up with a short read
    #endif
    read_buffer(&string[0], string.size());                                     // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
    return string;
  }

  template<typename T, typename SizeT>
  std::vector<T> read_blob(SizeT const size) const {
    /// Read size bytes from the stream into a vector blob asynchronously
    std::vector<T> blob(size);
    read_buffer(blob.data(), blob.size() * sizeof(T));
    return blob;
  }

  template<typename T>
  inline void write_buffer(T const &buffer) {
    /// Write a native buffer of char const* (or whatever implicitly converts to that) to the stream
    stream.write(buffer, sizeof(buffer));
  }
  template<typename T>
  inline void write_buffer(T const *data, size_t const size) {
    /// Write a block of data of the specified size to the stream from the target buffer
    stream.write(reinterpret_cast<char const*>(data), size);
  }

  template <typename T>
  inline void write_string(std::basic_string<T> const &string) {
    /// Write a string to the stream
    stream.write(string.c_str(), string.size());
  }

  template <typename T>
  inline void write_blob(std::vector<T> const &blob) {
    /// Write a blob to the stream
    write_blob(blob, blob.size() * sizeof(T));
  }
  template <typename T>
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    /// Write a blob of specific size to the stream
    stream.write(blob.data(), size);
  }
};

}

#endif // SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED
