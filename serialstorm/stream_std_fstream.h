#include "stream_base.h"
#include <fstream>
#ifndef NDEBUG
  #include <iostream>
#endif
#ifndef SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED

#define SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED
namespace serialstorm {


#include "stream_base.h"
template<typename StreamT>
#include <fstream>
class stream_std_fstream : public stream_base<StreamT, stream_std_fstream> {
#ifndef NDEBUG
  /// Stream handler to manage a std::fstream
  #include <iostream>
public:
#endif
  StreamT &stream;


namespace serialstorm {

  constexpr explicit stream_std_fstream(StreamT &new_stream)
template<typename StreamT>
    : stream(new_stream) {
class stream_std_fstream : public stream_base<StreamT, stream_std_fstream> {
    /// Specific constructor
  /// Stream handler to manage a std::fstream
  }
public:

  StreamT &stream;
  template<typename T>

  void read_buffer(T *data, size_t const size) const {
  constexpr explicit stream_std_fstream(StreamT &new_stream)
    /// Read a block of data of the specified size from the stream to the target buffer asynchronously
    : stream(new_stream) {
    stream.read(reinterpret_cast<char*>(data), size);
    /// Specific constructor
    #ifndef NDEBUG
  }
      if(!stream) {

  template<typename T>
        std::cout << "SerialStorm: WARNING: short read on stream: " << stream.gcount() << " read out of " << size << " requested." << std::endl;
  void read_buffer(T *data, size_t const size) const {
      }
    /// Read a block of data of the specified size from the stream to the target buffer asynchronously
    #endif
    stream.read(reinterpret_cast<char*>(data), size);
  }
    #ifndef NDEBUG

      if(!stream) {
  template<typename T>
        std::cout << "SerialStorm: WARNING: short read on stream: " << stream.gcount() << " read out of " << size << " requested." << std::endl;
  std::string read_string(T const stringlength) const {
      }
    /// Read size bytes from the stream into a string asynchronously
    #endif
    #ifdef NDEBUG
  }
      std::string string(stringlength, '\0');                                   // use null byte as default fill to minimise risk in release mode

    #else
  template<typename T>
      std::string string(stringlength, '?');                                    // use ? as a marker character to visibly show if we somehow end up with a short read
  std::string read_string(T const stringlength) const {
    #endif
    /// Read size bytes from the stream into a string asynchronously
    #ifdef NDEBUG
    read_buffer(&string[0], string.size());                                     // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
      std::string string(stringlength, '\0');                                   // use null byte as default fill to minimise risk in release mode
    return string;
    #else
  }
      std::string string(stringlength, '?');                                    // use ? as a marker character to visibly show if we somehow end up with a short read

    #endif
  template<typename T, typename SizeT>
    read_buffer(&string[0], string.size());                                     // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
  std::vector<T> read_blob(SizeT const size) const {
    return string;
    /// Read size bytes from the stream into a vector blob asynchronously
  }
    std::vector<T> blob(size);

    read_buffer(blob.data(), blob.size() * sizeof(T));
  template<typename T, typename SizeT>
    return blob;
  std::vector<T> read_blob(SizeT const size) const {
  }
    /// Read size bytes from the stream into a vector blob asynchronously

    std::vector<T> blob(size);
  template<typename T>
    read_buffer(blob.data(), blob.size() * sizeof(T));
  inline void write_buffer(T const &buffer) {
    return blob;
    /// Write a native buffer of char const* (or whatever implicitly converts to that) to the stream
  }

    stream.write(buffer, sizeof(buffer));
  template<typename T>
  }
  inline void write_buffer(T const &buffer) {
  template<typename T>
    /// Write a native buffer of char const* (or whatever implicitly converts to that) to the stream
  inline void write_buffer(T const *data, size_t const size) {
    stream.write(buffer, sizeof(buffer));
    /// Write a block of data of the specified size to the stream from the target buffer
  }
    stream.write(reinterpret_cast<char const*>(data), size);
  template<typename T>
  }
  inline void write_buffer(T const *data, size_t const size) {

    /// Write a block of data of the specified size to the stream from the target buffer
  template <typename T>
    stream.write(reinterpret_cast<char const*>(data), size);
  inline void write_string(std::basic_string<T> const &string) {
  }
    /// Write a string to the stream

    stream.write(string.c_str(), string.size());
  template <typename T>
  }
  inline void write_string(std::basic_string<T> const &string) {

    /// Write a string to the stream
  template <typename T>
    stream.write(string.c_str(), string.size());
  inline void write_blob(std::vector<T> const &blob) {
  }

    /// Write a blob to the stream
  template <typename T>
    write_blob(blob, blob.size() * sizeof(T));
  inline void write_blob(std::vector<T> const &blob) {
  }
    /// Write a blob to the stream
  template <typename T>
    write_blob(blob, blob.size() * sizeof(T));
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
  }
    /// Write a blob of specific size to the stream
  template <typename T>
    stream.write(blob.data(), size);
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
  }
    /// Write a blob of specific size to the stream
};
    stream.write(blob.data(), size);

  }
}
};


}
#endif // SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED

#endif // SERIALSTORM_STREAM_STD_FSTREAM_H_INCLUDED
