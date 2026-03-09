/**************************************************************************/
/*  spacetimedb_client.cpp                                               */
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

#include "spacetimedb_client.h"
#include "spacetimedb_bsatn.h"

#include "core/error/error_macros.h"
#include "core/io/json.h"
#include "core/string/print_string.h"

void SpacetimeDBClient::_bind_methods() {
	// Connection.
	ClassDB::bind_method(D_METHOD("connect_to_database", "host", "database", "token"), &SpacetimeDBClient::connect_to_database, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("disconnect_from_database"), &SpacetimeDBClient::disconnect_from_database);
	ClassDB::bind_method(D_METHOD("poll"), &SpacetimeDBClient::poll);

	// State.
	ClassDB::bind_method(D_METHOD("get_connection_state"), &SpacetimeDBClient::get_connection_state);
	ClassDB::bind_method(D_METHOD("is_connected_to_database"), &SpacetimeDBClient::is_connected_to_database);
	ClassDB::bind_method(D_METHOD("get_identity"), &SpacetimeDBClient::get_identity);
	ClassDB::bind_method(D_METHOD("get_auth_token"), &SpacetimeDBClient::get_auth_token);
	ClassDB::bind_method(D_METHOD("get_connection_id"), &SpacetimeDBClient::get_connection_id);

	// Protocol.
	ClassDB::bind_method(D_METHOD("set_protocol", "protocol"), &SpacetimeDBClient::set_protocol);
	ClassDB::bind_method(D_METHOD("get_protocol"), &SpacetimeDBClient::get_protocol);

	// Reducers.
	ClassDB::bind_method(D_METHOD("call_reducer", "reducer_name", "args"), &SpacetimeDBClient::call_reducer, DEFVAL(Array()));

	// Subscriptions.
	ClassDB::bind_method(D_METHOD("subscribe", "queries"), &SpacetimeDBClient::subscribe);

	// Tables.
	ClassDB::bind_method(D_METHOD("get_table", "name"), &SpacetimeDBClient::get_table);
	ClassDB::bind_method(D_METHOD("has_table", "name"), &SpacetimeDBClient::has_table);
	ClassDB::bind_method(D_METHOD("get_table_names"), &SpacetimeDBClient::get_table_names);
	ClassDB::bind_method(D_METHOD("register_table", "name"), &SpacetimeDBClient::register_table);

	// SQL.
	ClassDB::bind_method(D_METHOD("sql_query", "query"), &SpacetimeDBClient::sql_query);

	// Properties.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "connection_state", PROPERTY_HINT_ENUM, "Disconnected,Connecting,Connected,Closing"), "", "get_connection_state");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "protocol", PROPERTY_HINT_ENUM, "JSON,BSATN"), "set_protocol", "get_protocol");

	// Signals.
	ADD_SIGNAL(MethodInfo("connected",
			PropertyInfo(Variant::OBJECT, "identity"),
			PropertyInfo(Variant::STRING, "token")));
	ADD_SIGNAL(MethodInfo("disconnected"));
	ADD_SIGNAL(MethodInfo("connection_error",
			PropertyInfo(Variant::STRING, "error")));
	ADD_SIGNAL(MethodInfo("identity_received",
			PropertyInfo(Variant::OBJECT, "identity"),
			PropertyInfo(Variant::STRING, "token"),
			PropertyInfo(Variant::STRING, "connection_id")));
	ADD_SIGNAL(MethodInfo("reducer_called",
			PropertyInfo(Variant::STRING, "reducer_name"),
			PropertyInfo(Variant::STRING, "status"),
			PropertyInfo(Variant::STRING, "message")));
	ADD_SIGNAL(MethodInfo("subscription_applied"));
	ADD_SIGNAL(MethodInfo("row_inserted",
			PropertyInfo(Variant::STRING, "table_name"),
			PropertyInfo(Variant::DICTIONARY, "row")));
	ADD_SIGNAL(MethodInfo("row_deleted",
			PropertyInfo(Variant::STRING, "table_name"),
			PropertyInfo(Variant::DICTIONARY, "row")));
	ADD_SIGNAL(MethodInfo("row_updated",
			PropertyInfo(Variant::STRING, "table_name"),
			PropertyInfo(Variant::DICTIONARY, "old_row"),
			PropertyInfo(Variant::DICTIONARY, "new_row")));
	ADD_SIGNAL(MethodInfo("event_received",
			PropertyInfo(Variant::DICTIONARY, "event_data")));

	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);
	BIND_ENUM_CONSTANT(STATE_CLOSING);

	BIND_ENUM_CONSTANT(PROTOCOL_JSON);
	BIND_ENUM_CONSTANT(PROTOCOL_BSATN);
}

// ---- Connection ----

String SpacetimeDBClient::_build_ws_url() const {
	// SpacetimeDB subscribe endpoint:
	// GET /v1/database/:name_or_identity/subscribe
	String base = host_uri;
	if (base.ends_with("/")) {
		base = base.substr(0, base.length() - 1);
	}

	// Ensure we have a WebSocket scheme.
	if (base.begins_with("http://")) {
		base = "ws://" + base.substr(7);
	} else if (base.begins_with("https://")) {
		base = "wss://" + base.substr(8);
	} else if (!base.begins_with("ws://") && !base.begins_with("wss://")) {
		base = "ws://" + base;
	}

	return base + "/v1/database/" + database_name + "/subscribe";
}

Error SpacetimeDBClient::connect_to_database(const String &p_host, const String &p_database, const String &p_token) {
	if (state != STATE_DISCONNECTED) {
		disconnect_from_database();
	}

	host_uri = p_host;
	database_name = p_database;
	auth_token = p_token;

	ws_peer = Ref<WebSocketPeer>(WebSocketPeer::create());
	ERR_FAIL_COND_V(ws_peer.is_null(), ERR_CANT_CREATE);

	// Set the SpacetimeDB sub-protocol.
	Vector<String> protocols;
	if (protocol == PROTOCOL_BSATN) {
		protocols.push_back("v1.bsatn.spacetimedb");
	} else {
		protocols.push_back("v1.json.spacetimedb");
	}
	ws_peer->set_supported_protocols(protocols);

	// Add auth token header if provided.
	if (!auth_token.is_empty()) {
		Vector<String> headers;
		headers.push_back("Authorization: Bearer " + auth_token);
		ws_peer->set_handshake_headers(headers);
	}

	String url = _build_ws_url();
	print_verbose("SpacetimeDB: Connecting to " + url);

	Error err = ws_peer->connect_to_url(url);
	if (err != OK) {
		emit_signal("connection_error", "Failed to initiate WebSocket connection.");
		return err;
	}

	state = STATE_CONNECTING;
	return OK;
}

void SpacetimeDBClient::disconnect_from_database() {
	if (ws_peer.is_valid()) {
		ws_peer->close();
	}
	state = STATE_DISCONNECTED;
	emit_signal("disconnected");
}

void SpacetimeDBClient::poll() {
	if (ws_peer.is_null()) {
		return;
	}

	ws_peer->poll();

	WebSocketPeer::State ws_state = ws_peer->get_ready_state();

	switch (ws_state) {
		case WebSocketPeer::STATE_CONNECTING: {
			// Still connecting, nothing to do.
		} break;

		case WebSocketPeer::STATE_OPEN: {
			if (state == STATE_CONNECTING) {
				state = STATE_CONNECTED;
				print_verbose("SpacetimeDB: WebSocket connection established.");
				// The server will send an IdentityToken message shortly.
			}
			// Process all available packets.
			while (ws_peer->get_available_packet_count() > 0) {
				const uint8_t *data_ptr;
				int data_size;
				Error err = ws_peer->get_packet(&data_ptr, data_size);
				if (err != OK) {
					break;
				}
				PackedByteArray data;
				data.resize(data_size);
				if (data_size > 0) {
					memcpy(data.ptrw(), data_ptr, data_size);
				}
				_process_message(data, ws_peer->was_string_packet());
			}
		} break;

		case WebSocketPeer::STATE_CLOSING: {
			state = STATE_CLOSING;
		} break;

		case WebSocketPeer::STATE_CLOSED: {
			if (state != STATE_DISCONNECTED) {
				int code = ws_peer->get_close_code();
				String reason = ws_peer->get_close_reason();
				state = STATE_DISCONNECTED;
				if (code != 1000) {
					emit_signal("connection_error", "Connection closed: " + reason + " (code " + itos(code) + ")");
				}
				emit_signal("disconnected");
			}
		} break;
	}
}

// ---- State ----

SpacetimeDBClient::ConnectionState SpacetimeDBClient::get_connection_state() const {
	return state;
}

bool SpacetimeDBClient::is_connected_to_database() const {
	return state == STATE_CONNECTED;
}

Ref<SpacetimeDBIdentity> SpacetimeDBClient::get_identity() const {
	return identity;
}

String SpacetimeDBClient::get_auth_token() const {
	return auth_token;
}

String SpacetimeDBClient::get_connection_id() const {
	return connection_id;
}

void SpacetimeDBClient::set_protocol(Protocol p_protocol) {
	ERR_FAIL_COND_MSG(state != STATE_DISCONNECTED,
			"Cannot change protocol while connected.");
	protocol = p_protocol;
}

SpacetimeDBClient::Protocol SpacetimeDBClient::get_protocol() const {
	return protocol;
}

// ---- Reducer invocation ----

Error SpacetimeDBClient::call_reducer(const String &p_reducer_name, const Array &p_args) {
	ERR_FAIL_COND_V(state != STATE_CONNECTED, ERR_UNCONFIGURED);

	if (protocol == PROTOCOL_JSON) {
		// Build JSON message following SpacetimeDB's v1 client protocol.
		// Message format: {"call": {"reducer": "name", "args": [...]}}
		Dictionary call_msg;
		Dictionary call_inner;
		call_inner["reducer"] = p_reducer_name;
		call_inner["args"] = p_args;
		call_msg["call"] = call_inner;

		String json_str = JSON::stringify(call_msg);
		Error err = ws_peer->send_text(json_str);
		return err;
	} else {
		// BSATN protocol: encode the CallReducer message.
		// For now, fall back to JSON encoding even for BSATN mode
		// since full BSATN client-to-server message encoding is complex.
		Dictionary call_msg;
		Dictionary call_inner;
		call_inner["reducer"] = p_reducer_name;
		call_inner["args"] = p_args;
		call_msg["call"] = call_inner;

		String json_str = JSON::stringify(call_msg);
		CharString utf8 = json_str.utf8();
		PackedByteArray data;
		data.resize(utf8.length());
		memcpy(data.ptrw(), utf8.get_data(), utf8.length());
		return ws_peer->send(data.ptr(), data.size(), WebSocketPeer::WRITE_MODE_BINARY);
	}
}

// ---- Subscriptions ----

Error SpacetimeDBClient::subscribe(const PackedStringArray &p_queries) {
	ERR_FAIL_COND_V(state != STATE_CONNECTED, ERR_UNCONFIGURED);

	// SpacetimeDB subscribe message:
	// {"subscribe": {"query_strings": ["SELECT * FROM table1", ...]}}
	Dictionary sub_msg;
	Dictionary sub_inner;
	Array query_arr;
	for (int i = 0; i < p_queries.size(); i++) {
		query_arr.push_back(p_queries[i]);
	}
	sub_inner["query_strings"] = query_arr;
	sub_msg["subscribe"] = sub_inner;

	String json_str = JSON::stringify(sub_msg);

	if (protocol == PROTOCOL_JSON) {
		return ws_peer->send_text(json_str);
	} else {
		CharString utf8 = json_str.utf8();
		PackedByteArray data;
		data.resize(utf8.length());
		memcpy(data.ptrw(), utf8.get_data(), utf8.length());
		return ws_peer->send(data.ptr(), data.size(), WebSocketPeer::WRITE_MODE_BINARY);
	}
}

// ---- Table access ----

Ref<SpacetimeDBTable> SpacetimeDBClient::get_table(const String &p_name) const {
	if (tables.has(p_name)) {
		return tables[p_name];
	}
	return Ref<SpacetimeDBTable>();
}

bool SpacetimeDBClient::has_table(const String &p_name) const {
	return tables.has(p_name);
}

PackedStringArray SpacetimeDBClient::get_table_names() const {
	PackedStringArray names;
	for (const KeyValue<String, Ref<SpacetimeDBTable>> &kv : tables) {
		names.push_back(kv.key);
	}
	return names;
}

Ref<SpacetimeDBTable> SpacetimeDBClient::register_table(const String &p_name) {
	if (tables.has(p_name)) {
		return tables[p_name];
	}
	Ref<SpacetimeDBTable> table;
	table.instantiate();
	table->set_table_name(p_name);
	tables[p_name] = table;
	return table;
}

// ---- SQL convenience ----

Error SpacetimeDBClient::sql_query(const String &p_query) {
	// SQL queries go over HTTP POST, not WebSocket.
	// This is a placeholder – in a full implementation you would use
	// HTTPRequest to POST to /v1/database/:name/sql
	ERR_FAIL_V_MSG(ERR_UNAVAILABLE,
			"sql_query() is not yet implemented. Use spacetime CLI or HTTPRequest to POST to /v1/database/<name>/sql");
}

// ---- Internal message processing ----

void SpacetimeDBClient::_process_message(const PackedByteArray &p_data, bool p_is_text) {
	if (p_is_text || protocol == PROTOCOL_JSON) {
		String text;
		text.parse_utf8((const char *)p_data.ptr(), p_data.size());
		_process_json_message(text);
	} else {
		_process_bsatn_message(p_data);
	}
}

void SpacetimeDBClient::_process_json_message(const String &p_json) {
	Variant parsed = JSON::parse_string(p_json);
	if (parsed.get_type() != Variant::DICTIONARY) {
		WARN_PRINT("SpacetimeDB: Received non-dictionary JSON message.");
		return;
	}
	Dictionary msg = parsed;

	// Emit raw event.
	emit_signal("event_received", msg);

	// IdentityToken message.
	if (msg.has("IdentityToken") || msg.has("identity_token")) {
		Dictionary token_msg = msg.has("IdentityToken") ? (Dictionary)msg["IdentityToken"] : (Dictionary)msg["identity_token"];
		_handle_identity_token(token_msg);
		return;
	}

	// InitialSubscription message.
	if (msg.has("InitialSubscription") || msg.has("initial_subscription")) {
		Dictionary sub_msg = msg.has("InitialSubscription") ? (Dictionary)msg["InitialSubscription"] : (Dictionary)msg["initial_subscription"];
		_handle_initial_subscription(sub_msg);
		return;
	}

	// TransactionUpdate message.
	if (msg.has("TransactionUpdate") || msg.has("transaction_update")) {
		Dictionary tx_msg = msg.has("TransactionUpdate") ? (Dictionary)msg["TransactionUpdate"] : (Dictionary)msg["transaction_update"];
		_handle_transaction_update(tx_msg);
		return;
	}

	// TransactionUpdateLight message (v2 style).
	if (msg.has("TransactionUpdateLight") || msg.has("transaction_update_light")) {
		Dictionary tx_msg = msg.has("TransactionUpdateLight") ? (Dictionary)msg["TransactionUpdateLight"] : (Dictionary)msg["transaction_update_light"];
		_handle_transaction_update(tx_msg);
		return;
	}
}

void SpacetimeDBClient::_process_bsatn_message(const PackedByteArray &p_data) {
	// BSATN message processing – for now, attempt to treat the payload as
	// JSON as many servers send JSON even over binary frames.
	// A complete implementation would deserialize the BSATN ServerMessage
	// enum tag and its variants.
	String text;
	text.parse_utf8((const char *)p_data.ptr(), p_data.size());
	if (!text.is_empty() && (text.begins_with("{") || text.begins_with("["))) {
		_process_json_message(text);
		return;
	}

	// Minimal BSATN envelope: first byte is the message-type tag.
	if (p_data.size() < 1) {
		return;
	}

	uint8_t tag = p_data[0];
	// SpacetimeDB ServerMessage tags (from websocket.rs):
	//   0 = InitialSubscription
	//   1 = TransactionUpdate
	//   2 = TransactionUpdateLight
	//   3 = IdentityToken
	//   4 = OneOffQueryResponse
	print_verbose("SpacetimeDB: Received BSATN message with tag=" + itos(tag) + " size=" + itos(p_data.size()));

	// Full BSATN decoding is complex and requires knowledge of the exact
	// wire format. We emit the raw data so advanced users can decode it
	// themselves until full BSATN client support is implemented.
	Dictionary event;
	event["bsatn_tag"] = tag;
	event["bsatn_data"] = p_data;
	emit_signal("event_received", event);
}

void SpacetimeDBClient::_handle_identity_token(const Dictionary &p_msg) {
	// Fields: identity (hex string), token (string), connection_id (hex string or absent)
	String id_hex;
	if (p_msg.has("identity")) {
		id_hex = p_msg["identity"];
	}
	if (p_msg.has("token")) {
		auth_token = p_msg["token"];
	}
	if (p_msg.has("connection_id")) {
		connection_id = p_msg["connection_id"];
	}

	identity = SpacetimeDBIdentity::create_from_hex(id_hex);

	print_verbose("SpacetimeDB: Identity received: " + id_hex);

	emit_signal("identity_received", identity, auth_token, connection_id);
	emit_signal("connected", identity, auth_token);
}

void SpacetimeDBClient::_handle_initial_subscription(const Dictionary &p_msg) {
	// Fields: database_update (contains tables array)
	Dictionary db_update;
	if (p_msg.has("database_update")) {
		db_update = p_msg["database_update"];
	} else if (p_msg.has("DatabaseUpdate")) {
		db_update = p_msg["DatabaseUpdate"];
	}

	if (db_update.has("tables")) {
		Array table_updates = db_update["tables"];
		for (int i = 0; i < table_updates.size(); i++) {
			if (table_updates[i].get_type() == Variant::DICTIONARY) {
				_apply_table_operations(table_updates[i]);
			}
		}
	}

	emit_signal("subscription_applied");
}

void SpacetimeDBClient::_handle_transaction_update(const Dictionary &p_msg) {
	// Extract reducer info if present.
	if (p_msg.has("reducer_call") || p_msg.has("ReducerCall")) {
		Dictionary reducer_call = p_msg.has("reducer_call") ? (Dictionary)p_msg["reducer_call"] : (Dictionary)p_msg["ReducerCall"];
		String reducer_name;
		if (reducer_call.has("reducer_name")) {
			reducer_name = reducer_call["reducer_name"];
		} else if (reducer_call.has("reducer")) {
			reducer_name = reducer_call["reducer"];
		}

		String status_str = "committed";
		String message;
		if (p_msg.has("status")) {
			Dictionary status_dict = p_msg["status"];
			if (status_dict.has("Failed")) {
				status_str = "failed";
				message = status_dict["Failed"];
			} else if (status_dict.has("OutOfEnergy")) {
				status_str = "out_of_energy";
			}
		}

		emit_signal("reducer_called", reducer_name, status_str, message);
	}

	// Apply database updates.
	Dictionary db_update;
	if (p_msg.has("database_update")) {
		db_update = p_msg["database_update"];
	} else if (p_msg.has("DatabaseUpdate")) {
		db_update = p_msg["DatabaseUpdate"];
	}

	if (db_update.has("tables")) {
		Array table_updates = db_update["tables"];
		for (int i = 0; i < table_updates.size(); i++) {
			if (table_updates[i].get_type() == Variant::DICTIONARY) {
				_apply_table_operations(table_updates[i]);
			}
		}
	}
}

void SpacetimeDBClient::_apply_table_operations(const Dictionary &p_table_update) {
	String table_name;
	if (p_table_update.has("table_name")) {
		table_name = p_table_update["table_name"];
	} else if (p_table_update.has("TableName")) {
		table_name = p_table_update["TableName"];
	}

	if (table_name.is_empty()) {
		return;
	}

	// Ensure table exists in cache.
	if (!tables.has(table_name)) {
		Ref<SpacetimeDBTable> new_table;
		new_table.instantiate();
		new_table->set_table_name(table_name);
		tables[table_name] = new_table;
	}

	Ref<SpacetimeDBTable> table = tables[table_name];

	// Process inserts.
	Array inserts;
	if (p_table_update.has("inserts")) {
		inserts = p_table_update["inserts"];
	} else if (p_table_update.has("table_row_operations")) {
		// Older protocol format.
		Array ops = p_table_update["table_row_operations"];
		for (int i = 0; i < ops.size(); i++) {
			Dictionary op = ops[i];
			if (op.has("op")) {
				String op_type = op["op"];
				if (op_type == "insert" && op.has("row")) {
					inserts.push_back(op["row"]);
				}
			}
		}
	}

	for (int i = 0; i < inserts.size(); i++) {
		Dictionary row;
		if (inserts[i].get_type() == Variant::DICTIONARY) {
			row = inserts[i];
		} else if (inserts[i].get_type() == Variant::ARRAY) {
			// Row as positional array – convert using schema.
			Array arr = inserts[i];
			for (int c = 0; c < MIN(arr.size(), table->get_column_count()); c++) {
				row[table->get_column_name(c)] = arr[c];
			}
		}
		if (!row.is_empty()) {
			table->insert_row(row);
			emit_signal("row_inserted", table_name, row);
		}
	}

	// Process deletes.
	Array deletes;
	if (p_table_update.has("deletes")) {
		deletes = p_table_update["deletes"];
	}
	for (int i = 0; i < deletes.size(); i++) {
		Dictionary row;
		if (deletes[i].get_type() == Variant::DICTIONARY) {
			row = deletes[i];
		} else if (deletes[i].get_type() == Variant::ARRAY) {
			Array arr = deletes[i];
			for (int c = 0; c < MIN(arr.size(), table->get_column_count()); c++) {
				row[table->get_column_name(c)] = arr[c];
			}
		}
		if (!row.is_empty()) {
			table->delete_row(row);
			emit_signal("row_deleted", table_name, row);
		}
	}

	// Process updates (insert + delete pairs) if present.
	if (p_table_update.has("updates")) {
		Array updates = p_table_update["updates"];
		for (int i = 0; i < updates.size(); i++) {
			if (updates[i].get_type() == Variant::DICTIONARY) {
				Dictionary upd = updates[i];
				Dictionary old_row, new_row;
				if (upd.has("old") && upd.has("new")) {
					old_row = upd["old"];
					new_row = upd["new"];
				} else if (upd.has("delete") && upd.has("insert")) {
					old_row = upd["delete"];
					new_row = upd["insert"];
				}
				if (!old_row.is_empty() && !new_row.is_empty()) {
					table->update_row(old_row, new_row);
					emit_signal("row_updated", table_name, old_row, new_row);
				}
			}
		}
	}
}

// ---- Lifecycle ----

SpacetimeDBClient::SpacetimeDBClient() {
}

SpacetimeDBClient::~SpacetimeDBClient() {
	if (state != STATE_DISCONNECTED && ws_peer.is_valid()) {
		ws_peer->close();
	}
}
