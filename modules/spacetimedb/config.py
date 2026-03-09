def can_build(env, platform):
    env.module_add_dependencies("spacetimedb", ["websocket"], False)
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "SpacetimeDBClient",
        "SpacetimeDBIdentity",
        "SpacetimeDBTable",
        "SpacetimeDBBSATN",
    ]


def get_doc_path():
    return "doc_classes"
