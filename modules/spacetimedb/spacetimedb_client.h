/**************************************************************************/
/*  spacetimedb_client.h                                                 */
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

#include "spacetimedb_identity.h"
#include "spacetimedb_table.h"

#include "core/object/class_db.h"
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "modules/websocket/websocket_peer.h"

// SpacetimeDBClient is the main entry-point for connecting a Godot game to
// a SpacetimeDB database.  It wraps a WebSocketPeer and speaks the
// SpacetimeDB v1.bsatn.spacetimedb / v1.json.spacetimedb protocol.
//
// Usage from GDScript:
//
//   var client = SpacetimeDBClient.new()
//   client.connect_to_database("ws://localhost:3000", "my_database")
//   # In _process:
//   client.poll()
//   # Call a reducer:
//   client.call_reducer("add", ["Alice"])
//
// Signals are emitted for lifecycle events:
//   connected(identity, token)
//   disconnected()
//   connection_error(error)
//   identity_received(identity, token, connection_id)
//   reducer_called(reducer_name, status, message)
//   subscription_applied()
//   row_inserted(table_name, row)
//   row_deleted(table_name, row)
//   row_updated(table_name, old_row, new_row)
//   event_received(event_data)

class SpacetimeDBClient : public RefCounted {
	GDCLASS(SpacetimeDBClient, RefCounted);

public:
	enum ConnectionState {
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_CONNECTED,
		STATE_CLOSING,
	};

	// Protocol format for communication with the server.
	enum Protocol {
		PROTOCOL_JSON,
		PROTOCOL_BSATN,
	};

private:
	Ref<WebSocketPeer> ws_peer;
	ConnectionState state = STATE_DISCONNECTED;
	Protocol protocol = PROTOCOL_JSON; // JSON is simpler for initial integration.

	String host_uri;
	String database_name;
	String auth_token;

	Ref<SpacetimeDBIdentity> identity;
	String connection_id;

	// Client-side table cache, keyed by table name.
	HashMap<String, Ref<SpacetimeDBTable>> tables;

	// Internal message processing.
	void _process_message(const PackedByteArray &p_data, bool p_is_text);
	void _process_json_message(const String &p_json);
	void _process_bsatn_message(const PackedByteArray &p_data);
	void _handle_identity_token(const Dictionary &p_msg);
	void _handle_initial_subscription(const Dictionary &p_msg);
	void _handle_transaction_update(const Dictionary &p_msg);
	void _apply_table_operations(const Dictionary &p_table_update);

	String _build_ws_url() const;

protected:
	static void _bind_methods();

public:
	// Connection.
	Error connect_to_database(const String &p_host, const String &p_database, const String &p_token = "");
	void disconnect_from_database();
	void poll();

	// State.
	ConnectionState get_connection_state() const;
	bool is_connected_to_database() const;
	Ref<SpacetimeDBIdentity> get_identity() const;
	String get_auth_token() const;
	String get_connection_id() const;

	// Protocol.
	void set_protocol(Protocol p_protocol);
	Protocol get_protocol() const;

	// Reducer invocation.
	Error call_reducer(const String &p_reducer_name, const Array &p_args = Array());

	// Subscriptions.
	Error subscribe(const PackedStringArray &p_queries);

	// Table access.
	Ref<SpacetimeDBTable> get_table(const String &p_name) const;
	bool has_table(const String &p_name) const;
	PackedStringArray get_table_names() const;

	// Register a table schema manually (for when the server schema is known
	// ahead of time, e.g. from codegen).
	Ref<SpacetimeDBTable> register_table(const String &p_name);

	// SQL query over HTTP (non-WebSocket convenience).
	Error sql_query(const String &p_query);

	SpacetimeDBClient();
	~SpacetimeDBClient();
};

VARIANT_ENUM_CAST(SpacetimeDBClient::ConnectionState);
VARIANT_ENUM_CAST(SpacetimeDBClient::Protocol);
