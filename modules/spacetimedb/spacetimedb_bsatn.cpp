/**************************************************************************/
/*  spacetimedb_bsatn.cpp                                                */
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

#include "spacetimedb_bsatn.h"

#include "core/error/error_macros.h"

// ---- Bind all methods to GDScript/GDExtension ----

void SpacetimeDBBSATN::_bind_methods() {
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_bool", "value"), &SpacetimeDBBSATN::encode_bool);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_u8", "value"), &SpacetimeDBBSATN::encode_u8);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_i8", "value"), &SpacetimeDBBSATN::encode_i8);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_u16", "value"), &SpacetimeDBBSATN::encode_u16);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_i16", "value"), &SpacetimeDBBSATN::encode_i16);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_u32", "value"), &SpacetimeDBBSATN::encode_u32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_i32", "value"), &SpacetimeDBBSATN::encode_i32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_u64", "value"), &SpacetimeDBBSATN::encode_u64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_i64", "value"), &SpacetimeDBBSATN::encode_i64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_f32", "value"), &SpacetimeDBBSATN::encode_f32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_f64", "value"), &SpacetimeDBBSATN::encode_f64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_string", "value"), &SpacetimeDBBSATN::encode_string);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_bytes", "value"), &SpacetimeDBBSATN::encode_bytes);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("encode_variant", "value"), &SpacetimeDBBSATN::encode_variant);

	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_bool", "data", "offset"), &SpacetimeDBBSATN::decode_bool);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_u8", "data", "offset"), &SpacetimeDBBSATN::decode_u8);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_i8", "data", "offset"), &SpacetimeDBBSATN::decode_i8);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_u16", "data", "offset"), &SpacetimeDBBSATN::decode_u16);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_i16", "data", "offset"), &SpacetimeDBBSATN::decode_i16);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_u32", "data", "offset"), &SpacetimeDBBSATN::decode_u32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_i32", "data", "offset"), &SpacetimeDBBSATN::decode_i32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_u64", "data", "offset"), &SpacetimeDBBSATN::decode_u64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_i64", "data", "offset"), &SpacetimeDBBSATN::decode_i64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_f32", "data", "offset"), &SpacetimeDBBSATN::decode_f32);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_f64", "data", "offset"), &SpacetimeDBBSATN::decode_f64);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_string", "data", "offset"), &SpacetimeDBBSATN::decode_string);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("decode_bytes", "data", "offset"), &SpacetimeDBBSATN::decode_bytes);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("string_byte_length", "data", "offset"), &SpacetimeDBBSATN::string_byte_length);
	ClassDB::bind_static_method("SpacetimeDBBSATN", D_METHOD("bytes_byte_length", "data", "offset"), &SpacetimeDBBSATN::bytes_byte_length);
}

// ---- Encoding helpers (all little-endian) ----

PackedByteArray SpacetimeDBBSATN::encode_bool(bool p_value) {
	PackedByteArray out;
	out.resize(1);
	out.ptrw()[0] = p_value ? 1 : 0;
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_u8(uint8_t p_value) {
	PackedByteArray out;
	out.resize(1);
	out.ptrw()[0] = p_value;
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_i8(int8_t p_value) {
	PackedByteArray out;
	out.resize(1);
	out.ptrw()[0] = (uint8_t)p_value;
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_u16(uint16_t p_value) {
	PackedByteArray out;
	out.resize(2);
	uint8_t *w = out.ptrw();
	w[0] = (uint8_t)(p_value & 0xFF);
	w[1] = (uint8_t)((p_value >> 8) & 0xFF);
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_i16(int16_t p_value) {
	return encode_u16((uint16_t)p_value);
}

PackedByteArray SpacetimeDBBSATN::encode_u32(uint32_t p_value) {
	PackedByteArray out;
	out.resize(4);
	uint8_t *w = out.ptrw();
	w[0] = (uint8_t)(p_value & 0xFF);
	w[1] = (uint8_t)((p_value >> 8) & 0xFF);
	w[2] = (uint8_t)((p_value >> 16) & 0xFF);
	w[3] = (uint8_t)((p_value >> 24) & 0xFF);
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_i32(int32_t p_value) {
	return encode_u32((uint32_t)p_value);
}

PackedByteArray SpacetimeDBBSATN::encode_u64(uint64_t p_value) {
	PackedByteArray out;
	out.resize(8);
	uint8_t *w = out.ptrw();
	for (int i = 0; i < 8; i++) {
		w[i] = (uint8_t)((p_value >> (i * 8)) & 0xFF);
	}
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_i64(int64_t p_value) {
	return encode_u64((uint64_t)p_value);
}

PackedByteArray SpacetimeDBBSATN::encode_f32(float p_value) {
	PackedByteArray out;
	out.resize(4);
	union {
		float f;
		uint32_t u;
	} conv;
	conv.f = p_value;
	uint8_t *w = out.ptrw();
	w[0] = (uint8_t)(conv.u & 0xFF);
	w[1] = (uint8_t)((conv.u >> 8) & 0xFF);
	w[2] = (uint8_t)((conv.u >> 16) & 0xFF);
	w[3] = (uint8_t)((conv.u >> 24) & 0xFF);
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_f64(double p_value) {
	PackedByteArray out;
	out.resize(8);
	union {
		double d;
		uint64_t u;
	} conv;
	conv.d = p_value;
	uint8_t *w = out.ptrw();
	for (int i = 0; i < 8; i++) {
		w[i] = (uint8_t)((conv.u >> (i * 8)) & 0xFF);
	}
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_string(const String &p_value) {
	CharString utf8 = p_value.utf8();
	uint32_t len = utf8.length();

	PackedByteArray out;
	out.resize(4 + len);
	uint8_t *w = out.ptrw();

	// Length prefix (u32 LE).
	w[0] = (uint8_t)(len & 0xFF);
	w[1] = (uint8_t)((len >> 8) & 0xFF);
	w[2] = (uint8_t)((len >> 16) & 0xFF);
	w[3] = (uint8_t)((len >> 24) & 0xFF);

	if (len > 0) {
		memcpy(w + 4, utf8.get_data(), len);
	}
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_bytes(const PackedByteArray &p_value) {
	uint32_t len = p_value.size();

	PackedByteArray out;
	out.resize(4 + len);
	uint8_t *w = out.ptrw();

	w[0] = (uint8_t)(len & 0xFF);
	w[1] = (uint8_t)((len >> 8) & 0xFF);
	w[2] = (uint8_t)((len >> 16) & 0xFF);
	w[3] = (uint8_t)((len >> 24) & 0xFF);

	if (len > 0) {
		memcpy(w + 4, p_value.ptr(), len);
	}
	return out;
}

PackedByteArray SpacetimeDBBSATN::encode_variant(const Variant &p_value) {
	switch (p_value.get_type()) {
		case Variant::NIL: {
			// Encode as option-none: tag byte 0.
			return encode_u8(0);
		}
		case Variant::BOOL: {
			return encode_bool(p_value);
		}
		case Variant::INT: {
			return encode_i64((int64_t)p_value);
		}
		case Variant::FLOAT: {
			return encode_f64((double)p_value);
		}
		case Variant::STRING: {
			return encode_string(p_value);
		}
		case Variant::PACKED_BYTE_ARRAY: {
			return encode_bytes(p_value);
		}
		case Variant::ARRAY: {
			Array arr = p_value;
			PackedByteArray out;
			out.append_array(encode_u32(arr.size()));
			for (int i = 0; i < arr.size(); i++) {
				out.append_array(encode_variant(arr[i]));
			}
			return out;
		}
		case Variant::DICTIONARY: {
			// Encode dictionary as an array of key-value pairs.
			Dictionary dict = p_value;
			Array keys = dict.keys();
			PackedByteArray out;
			out.append_array(encode_u32(keys.size()));
			for (int i = 0; i < keys.size(); i++) {
				out.append_array(encode_variant(keys[i]));
				out.append_array(encode_variant(dict[keys[i]]));
			}
			return out;
		}
		default: {
			// Fallback: convert to string.
			return encode_string(p_value.stringify());
		}
	}
}

// ---- Decoding helpers (all little-endian) ----

bool SpacetimeDBBSATN::decode_bool(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_INDEX_V(p_offset, p_data.size(), false);
	return p_data[p_offset] != 0;
}

uint8_t SpacetimeDBBSATN::decode_u8(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_INDEX_V(p_offset, p_data.size(), 0);
	return p_data[p_offset];
}

int8_t SpacetimeDBBSATN::decode_i8(const PackedByteArray &p_data, int p_offset) {
	return (int8_t)decode_u8(p_data, p_offset);
}

uint16_t SpacetimeDBBSATN::decode_u16(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 2 > p_data.size(), 0);
	const uint8_t *r = p_data.ptr() + p_offset;
	return (uint16_t)r[0] | ((uint16_t)r[1] << 8);
}

int16_t SpacetimeDBBSATN::decode_i16(const PackedByteArray &p_data, int p_offset) {
	return (int16_t)decode_u16(p_data, p_offset);
}

uint32_t SpacetimeDBBSATN::decode_u32(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 4 > p_data.size(), 0);
	const uint8_t *r = p_data.ptr() + p_offset;
	return (uint32_t)r[0] | ((uint32_t)r[1] << 8) | ((uint32_t)r[2] << 16) | ((uint32_t)r[3] << 24);
}

int32_t SpacetimeDBBSATN::decode_i32(const PackedByteArray &p_data, int p_offset) {
	return (int32_t)decode_u32(p_data, p_offset);
}

uint64_t SpacetimeDBBSATN::decode_u64(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 8 > p_data.size(), 0);
	const uint8_t *r = p_data.ptr() + p_offset;
	uint64_t result = 0;
	for (int i = 0; i < 8; i++) {
		result |= ((uint64_t)r[i] << (i * 8));
	}
	return result;
}

int64_t SpacetimeDBBSATN::decode_i64(const PackedByteArray &p_data, int p_offset) {
	return (int64_t)decode_u64(p_data, p_offset);
}

float SpacetimeDBBSATN::decode_f32(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 4 > p_data.size(), 0.0f);
	union {
		float f;
		uint32_t u;
	} conv;
	conv.u = decode_u32(p_data, p_offset);
	return conv.f;
}

double SpacetimeDBBSATN::decode_f64(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 8 > p_data.size(), 0.0);
	union {
		double d;
		uint64_t u;
	} conv;
	conv.u = decode_u64(p_data, p_offset);
	return conv.d;
}

String SpacetimeDBBSATN::decode_string(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 4 > p_data.size(), String());
	uint32_t len = decode_u32(p_data, p_offset);
	ERR_FAIL_COND_V(p_offset + 4 + (int)len > p_data.size(), String());
	if (len == 0) {
		return String();
	}
	String result;
	result.parse_utf8((const char *)(p_data.ptr() + p_offset + 4), len);
	return result;
}

PackedByteArray SpacetimeDBBSATN::decode_bytes(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 4 > p_data.size(), PackedByteArray());
	uint32_t len = decode_u32(p_data, p_offset);
	ERR_FAIL_COND_V(p_offset + 4 + (int)len > p_data.size(), PackedByteArray());
	PackedByteArray out;
	if (len > 0) {
		out.resize(len);
		memcpy(out.ptrw(), p_data.ptr() + p_offset + 4, len);
	}
	return out;
}

int SpacetimeDBBSATN::string_byte_length(const PackedByteArray &p_data, int p_offset) {
	ERR_FAIL_COND_V(p_offset + 4 > p_data.size(), 0);
	uint32_t len = decode_u32(p_data, p_offset);
	return 4 + (int)len;
}

int SpacetimeDBBSATN::bytes_byte_length(const PackedByteArray &p_data, int p_offset) {
	return string_byte_length(p_data, p_offset); // Same encoding.
}

SpacetimeDBBSATN::SpacetimeDBBSATN() {
}

SpacetimeDBBSATN::~SpacetimeDBBSATN() {
}
