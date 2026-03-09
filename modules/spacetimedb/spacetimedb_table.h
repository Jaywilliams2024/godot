/**************************************************************************/
/*  spacetimedb_table.h                                                  */
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
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

// SpacetimeDBTable represents a client-side cache of a SpacetimeDB table.
// When the client subscribes to table data, the server pushes rows which
// are stored here and kept in sync via insert/delete/update events over
// the WebSocket connection.
//
// Each row is stored as a Dictionary whose keys correspond to the column
// names defined in the server-side module schema.  The table also stores
// schema metadata (column names and their BSATN type tags) so that
// incoming binary rows can be decoded.

class SpacetimeDBTable : public RefCounted {
	GDCLASS(SpacetimeDBTable, RefCounted);

	String table_name;

	// Schema: ordered list of column descriptors.
	struct ColumnDef {
		String name;
		String type; // "bool","u8","i8","u16","i16","u32","i32","u64","i64","f32","f64","string","bytes"
	};
	Vector<ColumnDef> columns;

	// The primary key column name (empty if none).
	String primary_key_column;

	// Row storage – keyed by a stringified primary-key value when a PK
	// exists, otherwise by an auto-incremented integer id.
	HashMap<String, Dictionary> rows;
	int next_row_id = 0;

protected:
	static void _bind_methods();

public:
	void set_table_name(const String &p_name);
	String get_table_name() const;

	// Schema manipulation.
	void add_column(const String &p_name, const String &p_type);
	void set_primary_key(const String &p_column);
	String get_primary_key() const;
	int get_column_count() const;
	String get_column_name(int p_index) const;
	String get_column_type(int p_index) const;

	// Row manipulation (called internally by SpacetimeDBClient on WS events).
	void insert_row(const Dictionary &p_row);
	void delete_row(const Dictionary &p_row);
	void update_row(const Dictionary &p_old_row, const Dictionary &p_new_row);
	void clear_rows();

	// Query the local cache.
	Array get_rows() const;
	int get_row_count() const;
	Dictionary find_by_primary_key(const Variant &p_key) const;

	// Decode a single row from BSATN bytes using the stored schema.
	// Returns a Dictionary with column-name keys.
	Dictionary decode_row_bsatn(const PackedByteArray &p_data, int p_offset) const;

	// Return the number of bytes consumed when decoding one row.
	int row_bsatn_byte_length(const PackedByteArray &p_data, int p_offset) const;

	SpacetimeDBTable();
	~SpacetimeDBTable();

private:
	String _row_key(const Dictionary &p_row) const;
};
