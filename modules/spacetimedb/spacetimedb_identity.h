/**************************************************************************/
/*  spacetimedb_identity.h                                               */
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

// SpacetimeDBIdentity represents a 256-bit identity used by SpacetimeDB
// to uniquely identify users/connections. It is the C++ equivalent of
// SpacetimeDB's Identity type.
class SpacetimeDBIdentity : public RefCounted {
	GDCLASS(SpacetimeDBIdentity, RefCounted);

	PackedByteArray data; // 32 bytes (256-bit identity).

protected:
	static void _bind_methods();

public:
	void set_data(const PackedByteArray &p_data);
	PackedByteArray get_data() const;

	// Create from a hex string representation.
	void set_hex_string(const String &p_hex);
	String get_hex_string() const;

	// Create a zero (anonymous) identity.
	static Ref<SpacetimeDBIdentity> create_zero();

	// Create from raw bytes.
	static Ref<SpacetimeDBIdentity> create_from_bytes(const PackedByteArray &p_bytes);

	// Create from a hex string.
	static Ref<SpacetimeDBIdentity> create_from_hex(const String &p_hex);

	bool is_zero() const;
	bool is_equal(const Ref<SpacetimeDBIdentity> &p_other) const;

	SpacetimeDBIdentity();
	~SpacetimeDBIdentity();
};
