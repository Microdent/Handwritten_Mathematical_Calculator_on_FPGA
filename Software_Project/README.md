# Software_Project
This folder contains the code and data can help you build a **Handwritten_Mathematical_Calculator** on **PC** simply. and some files are necessary in the HLS implementation process.

## Environmental Dependence
if you want to recurrent our project, you have to install the following environment:

- Python 3.7
- Tensorflow 1.X

## Start simple
We have prepared a trained model file as `model/ckpt/model.ckpt`, so we can start program directly:

1. Put a expression picture in `dataset/` .
   > example picture in `dataset/300` and  `dataset/720`

2. run `main.py`, and you will get the recognition result in terminal

## Model Quantify
Our project face to FPGA-depoly, so we quantify our CNN model and pull some file just like `tools/quantized_function` and `parameter/quan_parameter`. But you **CAN'T** run quantized inference model directly, because the code of this part is mess so we don't pull code on GitHub.

Despite this,  `parameter/quan_parameter` contains the quantized model parameters(with special order) for FPGA implementation. `model/ckpt/model_quan.ckpt` is our quantized CNN model. and `tools/quantized_function` have some function which may help you.

## What's More
To show the true performance of CNN model, some example picture will get incorrect inference result, just enjoy it.

