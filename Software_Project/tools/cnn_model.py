import tensorflow as tf
import os
import numpy as np
from collections import namedtuple

SIZE = 32

Conv = namedtuple('Conv', ['kernel', 'channel', 'depth'])


def weight_variable(shape):
    """Generates a weight variable of a given shape."""
    initial = tf.truncated_normal(shape, stddev=0.1)
    return tf.Variable(initial, name='weight')

def bias_variable(shape):
    """Generates a bias variable of a given shape."""
    initial = tf.constant(0.1, shape=shape)
    return tf.Variable(initial, name='bias')


def model_base(maps,num_classes=14):
    maps = tf.cast(maps,tf.float32)
    W_conv1 = weight_variable([5, 5, 1, 32])
    b_conv1 = bias_variable([32])
    
    x_conv1 = tf.nn.conv2d(maps, W_conv1, strides=[1, 1, 1, 1],
                           padding='SAME')
    h_conv1 = tf.nn.relu(x_conv1 + b_conv1)

    # Max-pooling.
    h_pool1 = tf.nn.max_pool(h_conv1, ksize=[1, 2, 2, 1],
                             strides=[1, 2, 2, 1], padding='SAME')

    # Second convolutional layer. 64 feature maps.
    W_conv2 = weight_variable([5, 5, 32, 64])
    b_conv2 = bias_variable([64])
    x_conv2 = tf.nn.conv2d(h_pool1, W_conv2, strides=[1, 1, 1, 1],
                           padding='SAME')
    h_conv2 = tf.nn.relu(x_conv2 + b_conv2)

    # Max-pooling.
    h_pool2 = tf.nn.max_pool(h_conv2, ksize=[1, 2, 2, 1],
                             strides=[1, 2, 2, 1], padding='SAME')
                             
    # Fully connected layer. Here we choose to have 1024 neurons in this layer.
    h_pool_flat = tf.reshape(h_pool2, [-1, 8*8*64])
    W_fc1 = weight_variable([8*8*64, 1024])
    b_fc1 = bias_variable([1024])
    h_fc1 = tf.nn.relu(tf.matmul(h_pool_flat, W_fc1) + b_fc1)

    # Classification layer.
    W_fc2 = weight_variable([1024, num_classes])
    b_fc2 = bias_variable([num_classes])
    y = tf.matmul(h_fc1, W_fc2) + b_fc2

    # This isn't used for training, but for when using the saved model.
    return tf.nn.softmax(y)

def inference(img):
    tf.reset_default_graph()
    image = np.reshape(img,(1,SIZE,SIZE,1))

    x = tf.placeholder(tf.float32, [1,SIZE,SIZE,1])
    result = model_base(image,num_classes=14)
    
    saver = tf.train.Saver() 
    with tf.Session() as sess:
        sess.run(tf.global_variables_initializer())
        saver.restore(sess, './model/ckpt/model.ckpt')
        result = sess.run(result, feed_dict={x: image})
        result = np.argmax(result, 1)
    return (result[0])
