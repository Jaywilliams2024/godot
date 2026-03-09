/**************************************************************************/
/*  spacetimedb_bsatn.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/variant.h"

// SpacetimeDBBSATN provides helpers for encoding and decoding data in
// SpacetimeDB's Binary Serialization and Algebraic Type Notation (BSATN)
// format. BSATN is a compact little-endian binary format used by
// SpacetimeDB's binary WebSocket protocol (v1.bsatn.spacetimedb).
//
// Primitive encoding reference:
//   bool     -> 1 byte (0 or 1)
//   u8/i8    -> 1 byte
//   u16/i16  -> 2 bytes LE
//   u32/i32  -> 4 bytes LE
//   u64/i64  -> 8 bytes LE
//   f32      -> 4 bytes LE IEEE754
//   f64      -> 8 bytes LE IEEE754
//   string   -> u32 length prefix + UTF-8 bytes
//   bytes    -> u32 length prefix + raw bytes
//   array<T> -> u32 count + each element encoded
//   option<T>-> u8 tag (0=none,1=some) + value if some

class SpacetimeDBBSATN : public RefCounted {
	GDCLASS(SpacetimeDBBSATN, RefCounted);

protected:
	static void _bind_methods();

public:
	// ----- Encoding helpers (produce PackedByteArray fragments) -----

	static PackedByteArray encode_bool(bool p_value);
	static PackedByteArray encode_u8(uint8_t p_value);
	static PackedByteArray encode_i8(int8_t p_value);
	static PackedByteArray encode_u16(uint16_t p_value);
	static PackedByteArray encode_i16(int16_t p_value);
	static PackedByteArray encode_u32(uint32_t p_value);
	static PackedByteArray encode_i32(int32_t p_value);
	static PackedByteArray encode_u64(uint64_t p_value);
	static PackedByteArray encode_i64(int64_t p_value);
	static PackedByteArray encode_f32(float p_value);
	static PackedByteArray encode_f64(double p_value);
	static PackedByteArray encode_string(const String &p_value);
	static PackedByteArray encode_bytes(const PackedByteArray &p_value);

	// Encode a Variant using BSATN rules.  Supports: bool, int, float, String,
	// PackedByteArray, Array, Dictionary, and nil (encoded as option-none).
	static PackedByteArray encode_variant(const Variant &p_value);

	// ----- Decoding helpers -----

	static bool decode_bool(const PackedByteArray &p_data, int p_offset);
	static uint8_t decode_u8(const PackedByteArray &p_data, int p_offset);
	static int8_t decode_i8(const PackedByteArray &p_data, int p_offset);
	static uint16_t decode_u16(const PackedByteArray &p_data, int p_offset);
	static int16_t decode_i16(const PackedByteArray &p_data, int p_offset);
	static uint32_t decode_u32(const PackedByteArray &p_data, int p_offset);
	static int32_t decode_i32(const PackedByteArray &p_data, int p_offset);
	static uint64_t decode_u64(const PackedByteArray &p_data, int p_offset);
	static int64_t decode_i64(const PackedByteArray &p_data, int p_offset);
	static float decode_f32(const PackedByteArray &p_data, int p_offset);
	static double decode_f64(const PackedByteArray &p_data, int p_offset);
	static String decode_string(const PackedByteArray &p_data, int p_offset);
	static PackedByteArray decode_bytes(const PackedByteArray &p_data, int p_offset);

	// Return the number of bytes consumed by a BSATN-encoded string starting
	// at the given offset (4-byte length prefix + the UTF-8 payload).
	static int string_byte_length(const PackedByteArray &p_data, int p_offset);

	// Return the number of bytes consumed by a BSATN-encoded byte array
	// starting at the given offset.
	static int bytes_byte_length(const PackedByteArray &p_data, int p_offset);

	SpacetimeDBBSATN();
	~SpacetimeDBBSATN();
};
