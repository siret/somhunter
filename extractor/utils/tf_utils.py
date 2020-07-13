import tensorflow as tf


def load_graph_from_proto(proto_file):
    with tf.io.gfile.GFile(proto_file, "rb") as f:
        graph_def = tf.compat.v1.GraphDef()
        graph_def.ParseFromString(f.read())

    graph = tf.Graph()
    config = tf.compat.v1.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.compat.v1.Session(graph=graph, config=config)
    with graph.as_default():
        tf.import_graph_def(graph_def)
    return session
