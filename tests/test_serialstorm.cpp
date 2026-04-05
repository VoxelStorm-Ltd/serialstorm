/// Comprehensive tests for serialstorm using Catch2 v3.
/// All tests use stream_std_stream wrapping a std::stringstream so they
/// exercise the full read/write path (stream_base + stream_std_stream) without
/// requiring Boost.Asio.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cstdint>
#include <cstring>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Include only the std::stream adapter – avoids a Boost dependency in tests.
#include "serialstorm/stream_std_stream.h"

using stream_t = serialstorm::stream_std_stream<std::stringstream>;

/// Seek the underlying stringstream back to the beginning so that what was
/// just written can be read back through the same wrapper.
static void reset_for_read(std::stringstream &ss) {
  ss.seekg(0);
}

// ============================================================================
// POD read / write
// ============================================================================

TEST_CASE("write_pod / read_pod round-trip for integer types", "[pod]") {
  SECTION("uint8_t zero") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<uint8_t>(0);
    reset_for_read(ss);
    CHECK(s.read_pod<uint8_t>() == 0);
  }
  SECTION("uint8_t max") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<uint8_t>(255);
    reset_for_read(ss);
    CHECK(s.read_pod<uint8_t>() == 255);
  }
  SECTION("uint16_t") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<uint16_t>(1000);
    reset_for_read(ss);
    CHECK(s.read_pod<uint16_t>() == 1000);
  }
  SECTION("uint32_t") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<uint32_t>(0x12345678u);
    reset_for_read(ss);
    CHECK(s.read_pod<uint32_t>() == 0x12345678u);
  }
  SECTION("uint64_t max") {
    std::stringstream ss;
    stream_t s(ss);
    uint64_t const val = std::numeric_limits<uint64_t>::max();
    s.write_pod<uint64_t>(val);
    reset_for_read(ss);
    CHECK(s.read_pod<uint64_t>() == val);
  }
  SECTION("int8_t negative") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<int8_t>(-1);
    reset_for_read(ss);
    CHECK(s.read_pod<int8_t>() == -1);
  }
  SECTION("int32_t negative") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<int32_t>(-42);
    reset_for_read(ss);
    CHECK(s.read_pod<int32_t>() == -42);
  }
  SECTION("int64_t min") {
    std::stringstream ss;
    stream_t s(ss);
    int64_t const val = std::numeric_limits<int64_t>::min();
    s.write_pod<int64_t>(val);
    reset_for_read(ss);
    CHECK(s.read_pod<int64_t>() == val);
  }
}

TEST_CASE("write_pod / read_pod round-trip for floating-point types", "[pod]") {
  SECTION("float") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<float>(3.14f);
    reset_for_read(ss);
    CHECK(s.read_pod<float>() == Catch::Approx(3.14f));
  }
  SECTION("double") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<double>(2.718281828);
    reset_for_read(ss);
    CHECK(s.read_pod<double>() == Catch::Approx(2.718281828));
  }
  SECTION("float negative") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_pod<float>(-0.5f);
    reset_for_read(ss);
    CHECK(s.read_pod<float>() == Catch::Approx(-0.5f));
  }
}

TEST_CASE("write_pod / read_pod preserves byte-exact representation", "[pod]") {
  // Verify that no byte ordering or transformation is applied.
  std::stringstream ss;
  stream_t s(ss);
  uint32_t const original = 0x01020304u;
  s.write_pod(original);
  std::string const raw = ss.str();
  REQUIRE(raw.size() == 4);
  uint32_t reconstructed = 0;
  std::memcpy(&reconstructed, raw.data(), 4);
  CHECK(reconstructed == original);
}

// ============================================================================
// Buffer read / write
// ============================================================================

TEST_CASE("write_buffer / read_buffer with raw byte arrays", "[buffer]") {
  SECTION("ASCII text") {
    std::stringstream ss;
    stream_t s(ss);
    char const input[] = {'h', 'e', 'l', 'l', 'o'};
    s.write_buffer(input, sizeof(input));
    reset_for_read(ss);
    char output[5] = {};
    s.read_buffer(output, sizeof(output));
    CHECK(std::string(output, 5) == "hello");
  }
  SECTION("binary data including null bytes and 0xFF") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> input = {'\x00', '\x01', '\x7F', '\xFE', '\xFF'};
    s.write_buffer(input.data(), input.size());
    reset_for_read(ss);
    std::vector<char> output(input.size());
    s.read_buffer(output.data(), output.size());
    CHECK(output == input);
  }
  SECTION("single byte") {
    std::stringstream ss;
    stream_t s(ss);
    char const byte = '\xAB';
    s.write_buffer(&byte, 1);
    reset_for_read(ss);
    char result = '\x00';
    s.read_buffer(&result, 1);
    CHECK(static_cast<unsigned char>(result) == 0xAB);
  }
}

// ============================================================================
// VarInt encoding / decoding
// ============================================================================

TEST_CASE("write_varint / read_varint round-trip across all size bands", "[varint]") {
  struct TestCase { uint64_t value; size_t expected_wire_bytes; };
  // See README: 0-127→1B, 128-255→2B, 256-65535→3B, 65536-2^32-1→5B, ≥2^32→9B
  std::vector<TestCase> cases = {
    {0,                                    1},
    {1,                                    1},
    {64,                                   1},
    {127,                                  1},
    {128,                                  2},
    {200,                                  2},
    {255,                                  2},
    {256,                                  3},
    {1000,                                 3},
    {65535,                                3},
    {65536,                                5},
    {100000,                               5},
    {0xFFFFFFFFu,                          5},
    {0x100000000ULL,                       9},
    {std::numeric_limits<uint64_t>::max(), 9},
  };

  for(auto const &tc : cases) {
    CAPTURE(tc.value);
    std::stringstream ss;
    stream_t s(ss);
    s.write_varint(tc.value);

    // Verify wire size
    CHECK(ss.str().size() == tc.expected_wire_bytes);

    // Verify round-trip
    reset_for_read(ss);
    CHECK(s.read_varint<uint64_t>() == tc.value);
  }
}

TEST_CASE("write_varint / read_varint with different output types", "[varint]") {
  SECTION("read small value as size_t") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varint<uint32_t>(42u);
    reset_for_read(ss);
    CHECK(s.read_varint<size_t>() == 42u);
  }
  SECTION("read uint16-range value as uint32_t") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varint<uint64_t>(1000u);
    reset_for_read(ss);
    CHECK(s.read_varint<uint32_t>() == 1000u);
  }
  SECTION("read uint32-range value as uint64_t") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varint<uint64_t>(0xFFFFFFFFu);
    reset_for_read(ss);
    CHECK(s.read_varint<uint64_t>() == 0xFFFFFFFFu);
  }
}

TEST_CASE("read_varint throws std::runtime_error on unknown size byte", "[varint][error]") {
  // Valid size tags are 128 (UINT_8), 129 (UINT_16), 130 (UINT_32), 131 (UINT_64).
  // Any other byte ≥128 that the high-bit check routes into the switch falls
  // through to the default case which throws.
  for(uint8_t bad_tag : {uint8_t{132}, uint8_t{160}, uint8_t{200}, uint8_t{255}}) {
    CAPTURE(bad_tag);
    std::stringstream ss;
    stream_t s(ss);
    ss.put(static_cast<char>(bad_tag));   // inject invalid tag directly
    reset_for_read(ss);
    CHECK_THROWS_AS(s.read_varint<size_t>(), std::runtime_error);
  }
}

// ============================================================================
// String read / write
// ============================================================================

TEST_CASE("write_string / read_string bare (caller supplies length)", "[string]") {
  // write_string / read_string are "unsafe" variants: no length prefix on the
  // wire, so the caller must know the length before calling read_string.
  // Pass an explicit std::string so this test exercises the string-taking
  // write_string overload directly.
  SECTION("typical string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_string(std::string("world"));
    reset_for_read(ss);
    CHECK(s.read_string(5u) == "world");
  }
  SECTION("empty string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_string(std::string(""));
    reset_for_read(ss);
    CHECK(s.read_string(0u) == std::string(""));
  }
  SECTION("string containing embedded null bytes") {
    std::string const input("ab\0cd", 5);
    std::stringstream ss;
    stream_t s(ss);
    s.write_string(input);
    reset_for_read(ss);
    CHECK(s.read_string(static_cast<size_t>(5u)) == input);
  }
}

TEST_CASE("write_varstring / read_varstring round-trip", "[string]") {
  SECTION("empty string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring("");
    reset_for_read(ss);
    CHECK(s.read_varstring() == "");
  }
  SECTION("short string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring("hello");
    reset_for_read(ss);
    CHECK(s.read_varstring() == "hello");
  }
  SECTION("string with embedded nulls and high bytes") {
    std::string const input("ab\x00\xFE\xFF", 5);
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring(input);
    reset_for_read(ss);
    CHECK(s.read_varstring() == input);
  }
  SECTION("long string (length > 127 requires 2-byte varint prefix)") {
    std::string const input(200, 'z');
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring(input);
    reset_for_read(ss);
    CHECK(s.read_varstring() == input);
  }
  SECTION("length limit: string within limit is accepted") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring("hi");
    reset_for_read(ss);
    CHECK(s.read_varstring(100) == "hi");
  }
  SECTION("length limit: string exceeding limit throws") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring("hello world");  // length 11
    reset_for_read(ss);
    CHECK_THROWS_AS(s.read_varstring(5), std::runtime_error);
  }
}

TEST_CASE("write_varstring_fixed / read_varstring_fixed round-trip", "[string]") {
  SECTION("uint8_t length prefix – empty string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint8_t>("");
    reset_for_read(ss);
    CHECK(s.read_varstring_fixed<uint8_t>() == "");
  }
  SECTION("uint8_t length prefix – short string") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint8_t>("test");
    reset_for_read(ss);
    CHECK(s.read_varstring_fixed<uint8_t>() == "test");
  }
  SECTION("uint16_t length prefix") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint16_t>("test16");
    reset_for_read(ss);
    CHECK(s.read_varstring_fixed<uint16_t>() == "test16");
  }
  SECTION("uint32_t length prefix") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint32_t>("test32");
    reset_for_read(ss);
    CHECK(s.read_varstring_fixed<uint32_t>() == "test32");
  }
  SECTION("uint16_t prefix – length limit exceeded throws") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint16_t>("hello world");  // length 11
    reset_for_read(ss);
    CHECK_THROWS_AS((s.read_varstring_fixed<uint16_t>(5)), std::runtime_error);
  }
  SECTION("uint8_t prefix – length within limit is accepted") {
    std::stringstream ss;
    stream_t s(ss);
    s.write_varstring_fixed<uint8_t>("ok");
    reset_for_read(ss);
    CHECK(s.read_varstring_fixed<uint8_t>(10) == "ok");
  }
}

// ============================================================================
// Blob read / write
// ============================================================================

TEST_CASE("write_blob / read_blob with known fixed length", "[blob]") {
  // stream_std_stream::read_blob<T>(size) returns std::vector<T>.
  // (The ostream-based overload lives in stream_base and is hidden by the
  // derived template, so we use the vector-returning version here.)
  SECTION("non-empty blob") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input = {'a', 'b', 'c', '\x00', '\xFF'};
    s.write_blob(input);
    reset_for_read(ss);
    auto const result = s.read_blob<char>(input.size());
    CHECK(result == input);
  }
  SECTION("single byte") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input = {'\xAB'};
    s.write_blob(input);
    reset_for_read(ss);
    auto const result = s.read_blob<char>(1u);
    REQUIRE(result.size() == 1u);
    CHECK(static_cast<unsigned char>(result[0]) == 0xAB);
  }
}

TEST_CASE("write_varblob / read_varblob (vector) round-trip", "[blob]") {
  SECTION("empty blob") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input{};
    s.write_varblob(input);
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out);
    CHECK(out.str().empty());
  }
  SECTION("small blob with binary content") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input = {'x', '\x00', '\xFF', 'y'};
    s.write_varblob(input);
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out);
    std::string const result = out.str();
    REQUIRE(result.size() == input.size());
    CHECK(std::vector<char>(result.begin(), result.end()) == input);
  }
  SECTION("blob with length > 127 (varint prefix uses 2 bytes)") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input(200, '\xAA');
    s.write_varblob(input);
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out);
    std::string const result = out.str();
    REQUIRE(result.size() == 200u);
    CHECK(result == std::string(200, '\xAA'));
  }
  SECTION("length limit: within limit is accepted") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input(10, 'B');
    s.write_varblob(input);
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out, 100);
    CHECK(out.str().size() == 10u);
  }
  SECTION("length limit: exceeded throws") {
    std::stringstream ss;
    stream_t s(ss);
    std::vector<char> const input(100, '\xAA');
    s.write_varblob(input);
    reset_for_read(ss);
    std::ostringstream out;
    CHECK_THROWS_AS(s.read_varblob(out, 50), std::runtime_error);
  }
}

TEST_CASE("write_varblob (stream) / read_varblob round-trip", "[blob]") {
  SECTION("stream-based varblob write and read") {
    std::stringstream ss;
    stream_t s(ss);
    std::string const data = "binary stream payload";
    std::istringstream instream(data);
    s.write_varblob(instream, data.size());
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out);
    CHECK(out.str() == data);
  }
  SECTION("empty stream-based varblob") {
    std::stringstream ss;
    stream_t s(ss);
    std::istringstream instream("");
    s.write_varblob(instream, 0);
    reset_for_read(ss);
    std::ostringstream out;
    s.read_varblob(out);
    CHECK(out.str().empty());
  }
}

// ============================================================================
// Read-position tracking (tellp)
// ============================================================================

TEST_CASE("stream_base::tellp() reports the running tally of bytes consumed from the stream", "[tellp]") {
  // tellp() is incremented exclusively by stream_base::read_buffer, which is
  // invoked by read_pod and read_varint.  Derived-class string/blob helpers
  // that call stream_std_stream::read_buffer directly do not go through the
  // tracking path, so only the varint length prefix is counted for varstring.
  // This test therefore exercises only pod and varint reads.
  std::stringstream ss;
  stream_t s(ss);

  s.write_pod<uint8_t>(1);          //  1 byte
  s.write_pod<uint16_t>(2);         //  2 bytes
  s.write_pod<uint32_t>(3);         //  4 bytes
  s.write_varint<uint32_t>(0u);     //  1 byte  (0 < 128)
  s.write_varint<uint64_t>(200u);   //  2 bytes (tag byte + uint8_t value)
  reset_for_read(ss);

  CHECK(s.tellp() == 0);

  s.read_pod<uint8_t>();
  CHECK(s.tellp() == 1);

  s.read_pod<uint16_t>();
  CHECK(s.tellp() == 3);

  s.read_pod<uint32_t>();
  CHECK(s.tellp() == 7);

  s.read_varint<uint32_t>();       // reads 1 byte  (value < 128)
  CHECK(s.tellp() == 8);

  s.read_varint<uint64_t>();       // reads 2 bytes (tag + uint8 value)
  CHECK(s.tellp() == 10);
}

// ============================================================================
// Mixed sequential serialisation round-trip
// ============================================================================

TEST_CASE("mixed types serialised and deserialised in sequence", "[sequence]") {
  std::stringstream ss;
  stream_t s(ss);

  s.write_pod<uint32_t>(42u);
  s.write_varstring("hello");
  s.write_varint<uint64_t>(9999999u);
  s.write_pod<float>(1.5f);
  std::vector<char> const blob = {'a', 'b', 'c'};
  s.write_varblob(blob);
  s.write_varstring_fixed<uint16_t>("fixed");

  reset_for_read(ss);

  CHECK(s.read_pod<uint32_t>()        == 42u);
  CHECK(s.read_varstring()            == "hello");
  CHECK(s.read_varint<uint64_t>()     == 9999999u);
  CHECK(s.read_pod<float>()           == Catch::Approx(1.5f));

  std::ostringstream blob_out;
  s.read_varblob(blob_out);
  std::string const blob_result = blob_out.str();
  CHECK(std::vector<char>(blob_result.begin(), blob_result.end()) == blob);

  CHECK(s.read_varstring_fixed<uint16_t>() == "fixed");
}

TEST_CASE("repeated values of the same type serialised in sequence", "[sequence]") {
  std::stringstream ss;
  stream_t s(ss);

  for(uint32_t i = 0; i < 10; ++i) {
    s.write_varint(i);
  }
  reset_for_read(ss);
  for(uint32_t i = 0; i < 10; ++i) {
    CHECK(s.read_varint<uint32_t>() == i);
  }
}
