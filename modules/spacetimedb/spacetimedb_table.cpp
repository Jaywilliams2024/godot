/**************************************************************************/
/*  spacetimedb_table.cpp                                                */
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

#include "spacetimedb_table.h"
#include "spacetimedb_bsatn.h"

#include "core/error/error_macros.h"

void SpacetimeDBTable::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_table_name", "name"), &SpacetimeDBTable::set_table_name);
	ClassDB::bind_method(D_METHOD("get_table_name"), &SpacetimeDBTable::get_table_name);

	ClassDB::bind_method(D_METHOD("add_column", "name", "type"), &SpacetimeDBTable::add_column);
	ClassDB::bind_method(D_METHOD("set_primary_key", "column"), &SpacetimeDBTable::set_primary_key);
	ClassDB::bind_method(D_METHOD("get_primary_key"), &SpacetimeDBTable::get_primary_key);
	ClassDB::bind_method(D_METHOD("get_column_count"), &SpacetimeDBTable::get_column_count);
	ClassDB::bind_method(D_METHOD("get_column_name", "index"), &SpacetimeDBTable::get_column_name);
	ClassDB::bind_method(D_METHOD("get_column_type", "index"), &SpacetimeDBTable::get_column_type);

	ClassDB::bind_method(D_METHOD("insert_row", "row"), &SpacetimeDBTable::insert_row);
	ClassDB::bind_method(D_METHOD("delete_row", "row"), &SpacetimeDBTable::delete_row);
	ClassDB::bind_method(D_METHOD("update_row", "old_row", "new_row"), &SpacetimeDBTable::update_row);
	ClassDB::bind_method(D_METHOD("clear_rows"), &SpacetimeDBTable::clear_rows);

	ClassDB::bind_method(D_METHOD("get_rows"), &SpacetimeDBTable::get_rows);
	ClassDB::bind_method(D_METHOD("get_row_count"), &SpacetimeDBTable::get_row_count);
	ClassDB::bind_method(D_METHOD("find_by_primary_key", "key"), &SpacetimeDBTable::find_by_primary_key);

	ClassDB::bind_method(D_METHOD("decode_row_bsatn", "data", "offset"), &SpacetimeDBTable::decode_row_bsatn);
	ClassDB::bind_method(D_METHOD("row_bsatn_byte_length", "data", "offset"), &SpacetimeDBTable::row_bsatn_byte_length);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "table_name"), "set_table_name", "get_table_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "primary_key"), "set_primary_key", "get_primary_key");

	ADD_SIGNAL(MethodInfo("row_inserted", PropertyInfo(Variant::DICTIONARY, "row")));
	ADD_SIGNAL(MethodInfo("row_deleted", PropertyInfo(Variant::DICTIONARY, "row")));
	ADD_SIGNAL(MethodInfo("row_updated",
			PropertyInfo(Variant::DICTIONARY, "old_row"),
			PropertyInfo(Variant::DICTIONARY, "new_row")));
}

void SpacetimeDBTable::set_table_name(const String &p_name) {
	table_name = p_name;
}

String SpacetimeDBTable::get_table_name() const {
	return table_name;
}

void SpacetimeDBTable::add_column(const String &p_name, const String &p_type) {
	ColumnDef col;
	col.name = p_name;
	col.type = p_type;
	columns.push_back(col);
}

void SpacetimeDBTable::set_primary_key(const String &p_column) {
	primary_key_column = p_column;
}

String SpacetimeDBTable::get_primary_key() const {
	return primary_key_column;
}

int SpacetimeDBTable::get_column_count() const {
	return columns.size();
}

String SpacetimeDBTable::get_column_name(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, columns.size(), String());
	return columns[p_index].name;
}

String SpacetimeDBTable::get_column_type(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, columns.size(), String());
	return columns[p_index].type;
}

String SpacetimeDBTable::_row_key(const Dictionary &p_row) const {
	if (!primary_key_column.is_empty() && p_row.has(primary_key_column)) {
		return String(p_row[primary_key_column]);
	}
	return String();
}

void SpacetimeDBTable::insert_row(const Dictionary &p_row) {
	String key = _row_key(p_row);
	if (key.is_empty()) {
		key = itos(next_row_id++);
	}
	rows[key] = p_row;
	emit_signal("row_inserted", p_row);
}

void SpacetimeDBTable::delete_row(const Dictionary &p_row) {
	String key = _row_key(p_row);
	if (!key.is_empty() && rows.has(key)) {
		Dictionary old_row = rows[key];
		rows.erase(key);
		emit_signal("row_deleted", old_row);
		return;
	}
	// Fallback: linear scan for matching row.
	for (const KeyValue<String, Dictionary> &kv : rows) {
		bool match = true;
		Array keys_arr = p_row.keys();
		for (int i = 0; i < keys_arr.size(); i++) {
			Variant k = keys_arr[i];
			if (!kv.value.has(k) || kv.value[k] != p_row[k]) {
				match = false;
				break;
			}
		}
		if (match) {
			Dictionary old_row = kv.value;
			rows.erase(kv.key);
			emit_signal("row_deleted", old_row);
			return;
		}
	}
}

void SpacetimeDBTable::update_row(const Dictionary &p_old_row, const Dictionary &p_new_row) {
	delete_row(p_old_row);
	String key = _row_key(p_new_row);
	if (key.is_empty()) {
		key = itos(next_row_id++);
	}
	rows[key] = p_new_row;
	emit_signal("row_updated", p_old_row, p_new_row);
}

void SpacetimeDBTable::clear_rows() {
	rows.clear();
}

Array SpacetimeDBTable::get_rows() const {
	Array result;
	for (const KeyValue<String, Dictionary> &kv : rows) {
		result.push_back(kv.value);
	}
	return result;
}

int SpacetimeDBTable::get_row_count() const {
	return rows.size();
}

Dictionary SpacetimeDBTable::find_by_primary_key(const Variant &p_key) const {
	if (primary_key_column.is_empty()) {
		return Dictionary();
	}
	String key = String(p_key);
	if (rows.has(key)) {
		return rows[key];
	}
	return Dictionary();
}

// ---- BSATN row decoding using the schema ----

static int _bsatn_field_size(const String &p_type, const PackedByteArray &p_data, int p_offset) {
	if (p_type == "bool" || p_type == "u8" || p_type == "i8") {
		return 1;
	} else if (p_type == "u16" || p_type == "i16") {
		return 2;
	} else if (p_type == "u32" || p_type == "i32" || p_type == "f32") {
		return 4;
	} else if (p_type == "u64" || p_type == "i64" || p_type == "f64") {
		return 8;
	} else if (p_type == "u128" || p_type == "i128") {
		return 16;
	} else if (p_type == "u256" || p_type == "i256" || p_type == "identity") {
		return 32;
	} else if (p_type == "connection_id") {
		return 16;
	} else if (p_type == "string" || p_type == "bytes") {
		return SpacetimeDBBSATN::string_byte_length(p_data, p_offset);
	}
	return 0;
}

static Variant _decode_field(const String &p_type, const PackedByteArray &p_data, int p_offset) {
	if (p_type == "bool") {
		return SpacetimeDBBSATN::decode_bool(p_data, p_offset);
	} else if (p_type == "u8") {
		return SpacetimeDBBSATN::decode_u8(p_data, p_offset);
	} else if (p_type == "i8") {
		return SpacetimeDBBSATN::decode_i8(p_data, p_offset);
	} else if (p_type == "u16") {
		return SpacetimeDBBSATN::decode_u16(p_data, p_offset);
	} else if (p_type == "i16") {
		return SpacetimeDBBSATN::decode_i16(p_data, p_offset);
	} else if (p_type == "u32") {
		return SpacetimeDBBSATN::decode_u32(p_data, p_offset);
	} else if (p_type == "i32") {
		return SpacetimeDBBSATN::decode_i32(p_data, p_offset);
	} else if (p_type == "u64") {
		return (int64_t)SpacetimeDBBSATN::decode_u64(p_data, p_offset);
	} else if (p_type == "i64") {
		return SpacetimeDBBSATN::decode_i64(p_data, p_offset);
	} else if (p_type == "f32") {
		return SpacetimeDBBSATN::decode_f32(p_data, p_offset);
	} else if (p_type == "f64") {
		return SpacetimeDBBSATN::decode_f64(p_data, p_offset);
	} else if (p_type == "string") {
		return SpacetimeDBBSATN::decode_string(p_data, p_offset);
	} else if (p_type == "bytes") {
		return SpacetimeDBBSATN::decode_bytes(p_data, p_offset);
	} else if (p_type == "identity" || p_type == "u256" || p_type == "i256") {
		// 32-byte value – return as hex string.
		PackedByteArray bytes;
		bytes.resize(32);
		if (p_offset + 32 <= p_data.size()) {
			memcpy(bytes.ptrw(), p_data.ptr() + p_offset, 32);
		}
		String hex;
		for (int i = 0; i < 32; i++) {
			hex += String::num_int64(bytes[i], 16).lpad(2, "0");
		}
		return hex;
	} else if (p_type == "connection_id" || p_type == "u128" || p_type == "i128") {
		// 16-byte value – return as hex string.
		PackedByteArray bytes;
		bytes.resize(16);
		if (p_offset + 16 <= p_data.size()) {
			memcpy(bytes.ptrw(), p_data.ptr() + p_offset, 16);
		}
		String hex;
		for (int i = 0; i < 16; i++) {
			hex += String::num_int64(bytes[i], 16).lpad(2, "0");
		}
		return hex;
	}
	return Variant();
}

Dictionary SpacetimeDBTable::decode_row_bsatn(const PackedByteArray &p_data, int p_offset) const {
	Dictionary row;
	int offset = p_offset;
	for (int i = 0; i < columns.size(); i++) {
		Variant value = _decode_field(columns[i].type, p_data, offset);
		row[columns[i].name] = value;
		offset += _bsatn_field_size(columns[i].type, p_data, offset);
	}
	return row;
}

int SpacetimeDBTable::row_bsatn_byte_length(const PackedByteArray &p_data, int p_offset) const {
	int offset = p_offset;
	for (int i = 0; i < columns.size(); i++) {
		offset += _bsatn_field_size(columns[i].type, p_data, offset);
	}
	return offset - p_offset;
}

SpacetimeDBTable::SpacetimeDBTable() {
}

SpacetimeDBTable::~SpacetimeDBTable() {
}
