# coding: utf-8

"""
Test script to create a dummy graph whose inputs and outputs resemble those of
a fully fletched graph network. The input is a batch of lists of up to 100 RecHits with
10 features per hit (energy, eta, phi, theta, r, x, y, z, detId, time), i.e., (-1, 100, 10).
The output is a vector of 15 energy fractions per hit, i.e., (-1, 100, 15).

Usage:

> python create_dummy_graph.py <output_path>
"""


import os
import sys
import tensorflow as tf


# for one positional argument, use plain argv
if len(sys.argv) < 2:
    this_file = os.path.basename(__file__)
    print("usage: python {} <output_path>".format(this_file))
    sys.exit(1)

# expand and normalize the output path
output_path = os.path.expandvars(os.path.expanduser(sys.argv[1]))
output_path = os.path.normpath(os.path.abspath(output_path))
print("create dummy graph at {}".format(output_path))

# shape constants
n_rechits = 100
n_features = 10
n_showers = 15

# create the dummy graph
x_t = tf.placeholder(tf.float32, [None, n_rechits, n_features], name="input")
x_reshaped_t = tf.reshape(x_t, [-1, n_rechits * n_features])
W_t = tf.Variable(0.01 * tf.ones([n_rechits * n_features, n_rechits * n_showers], tf.float32))
b_t = tf.Variable(0.01 * tf.ones([n_rechits * n_showers], tf.float32))
z_t = tf.add(tf.matmul(x_reshaped_t, W_t), b_t)
y_t = tf.reshape(z_t, [-1, n_rechits, n_showers], name="output")

# initialize session and variables
sess = tf.Session()
sess.run(tf.global_variables_initializer())

# create and save the const graph
outputs = ["output"]
constant_graph = tf.graph_util.convert_variables_to_constants(
    sess, sess.graph.as_graph_def(), outputs)
tf.train.write_graph(constant_graph, *os.path.split(output_path), as_text=False)
