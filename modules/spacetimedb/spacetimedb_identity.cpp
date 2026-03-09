/**************************************************************************/
/*  spacetimedb_identity.cpp                                             */
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

#include "spacetimedb_identity.h"

void SpacetimeDBIdentity::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_data", "data"), &SpacetimeDBIdentity::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &SpacetimeDBIdentity::get_data);

	ClassDB::bind_method(D_METHOD("set_hex_string", "hex"), &SpacetimeDBIdentity::set_hex_string);
	ClassDB::bind_method(D_METHOD("get_hex_string"), &SpacetimeDBIdentity::get_hex_string);

	ClassDB::bind_method(D_METHOD("is_zero"), &SpacetimeDBIdentity::is_zero);
	ClassDB::bind_method(D_METHOD("is_equal", "other"), &SpacetimeDBIdentity::is_equal);

	ClassDB::bind_static_method("SpacetimeDBIdentity", D_METHOD("create_zero"), &SpacetimeDBIdentity::create_zero);
	ClassDB::bind_static_method("SpacetimeDBIdentity", D_METHOD("create_from_bytes", "bytes"), &SpacetimeDBIdentity::create_from_bytes);
	ClassDB::bind_static_method("SpacetimeDBIdentity", D_METHOD("create_from_hex", "hex"), &SpacetimeDBIdentity::create_from_hex);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "hex_string"), "set_hex_string", "get_hex_string");
}

void SpacetimeDBIdentity::set_data(const PackedByteArray &p_data) {
	if (p_data.size() == 32) {
		data = p_data;
	} else {
		data.resize(32);
		memset(data.ptrw(), 0, 32);
		int copy_len = MIN(p_data.size(), 32);
		if (copy_len > 0) {
			memcpy(data.ptrw(), p_data.ptr(), copy_len);
		}
	}
}

PackedByteArray SpacetimeDBIdentity::get_data() const {
	return data;
}

void SpacetimeDBIdentity::set_hex_string(const String &p_hex) {
	data.resize(32);
	memset(data.ptrw(), 0, 32);

	String hex = p_hex;
	if (hex.begins_with("0x") || hex.begins_with("0X")) {
		hex = hex.substr(2);
	}

	int byte_count = MIN(hex.length() / 2, 32);
	for (int i = 0; i < byte_count; i++) {
		String byte_str = hex.substr(i * 2, 2);
		data.ptrw()[i] = (uint8_t)byte_str.hex_to_int();
	}
}

String SpacetimeDBIdentity::get_hex_string() const {
	String result;
	for (int i = 0; i < data.size(); i++) {
		result += String::num_int64(data[i], 16).lpad(2, "0");
	}
	return result;
}

Ref<SpacetimeDBIdentity> SpacetimeDBIdentity::create_zero() {
	Ref<SpacetimeDBIdentity> identity;
	identity.instantiate();
	identity->data.resize(32);
	memset(identity->data.ptrw(), 0, 32);
	return identity;
}

Ref<SpacetimeDBIdentity> SpacetimeDBIdentity::create_from_bytes(const PackedByteArray &p_bytes) {
	Ref<SpacetimeDBIdentity> identity;
	identity.instantiate();
	identity->set_data(p_bytes);
	return identity;
}

Ref<SpacetimeDBIdentity> SpacetimeDBIdentity::create_from_hex(const String &p_hex) {
	Ref<SpacetimeDBIdentity> identity;
	identity.instantiate();
	identity->set_hex_string(p_hex);
	return identity;
}

bool SpacetimeDBIdentity::is_zero() const {
	for (int i = 0; i < data.size(); i++) {
		if (data[i] != 0) {
			return false;
		}
	}
	return true;
}

bool SpacetimeDBIdentity::is_equal(const Ref<SpacetimeDBIdentity> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if (data.size() != p_other->data.size()) {
		return false;
	}
	return memcmp(data.ptr(), p_other->data.ptr(), data.size()) == 0;
}

SpacetimeDBIdentity::SpacetimeDBIdentity() {
	data.resize(32);
	memset(data.ptrw(), 0, 32);
}

SpacetimeDBIdentity::~SpacetimeDBIdentity() {
}
