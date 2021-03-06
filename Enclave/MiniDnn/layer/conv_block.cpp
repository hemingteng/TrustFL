//
// Created by Fengting Li on 2019-07-30.
//

#include "conv_block.h"

void ConvBlock::init() {
    //height_out = (height_in + 2*padding - height_kernel) / stride + 1

    Layer *conv1 = new Conv(channel_in, height_in, width_in, channel_out1, 1, 1, strides, 0, 0);  //valid
    Layer *relu1 = new ReLU;
    Layer *conv2 = new Conv(channel_out1, (height_in - 1) / strides + 1, (width_in - 1) / strides + 1, channel_out2,
                            height_kernel, width_kernel, 1, (height_kernel - 1) / 2, (width_kernel - 1) / 2);
    Layer *relu2 = new ReLU;
    Layer *conv3 = new Conv(channel_out2, (height_in - 1) / strides + 1, (width_in - 1) / strides + 1,
                            channel_out, 1, 1, 1, 0, 0); //valid
    Layer *relu3 = new ReLU;
    layers.push_back(conv1);
    layers.push_back(relu1);
    layers.push_back(conv2);
    layers.push_back(relu2);
    layers.push_back(conv3);
    layers.push_back(relu3);

    Layer *sc_conv = new Conv(channel_in, height_in, width_in, channel_out, 1, 1, strides, 0, 0); //valid
    shortcut.push_back(sc_conv);
}

void ConvBlock::forward(const Matrix &bottom) {
    if (layers.empty())
        return;
    layers[0]->forward(bottom);
    for (int i = 1; i < layers.size() - 1; i++) {  //最后一层relu之前要进行求和
        layers[i]->forward(layers[i - 1]->output());
    }
    shortcut[0]->forward(bottom);   //shortcut前向
    layers[layers.size() - 1]->forward(layers[layers.size() - 2]->output() + shortcut[0]->output());
}

void ConvBlock::backward(const Matrix &bottom, const Matrix &grad_top) {
    int n_layer = layers.size();
    // 0 layer
    if (n_layer <= 0)
        return;

    layers[n_layer - 1]->backward(layers[n_layer - 2]->output(),
                                  grad_top);     //最后一层relu
    for (int i = n_layer - 2; i > 0; i--) {
        layers[i]->backward(layers[i - 1]->output(), layers[i + 1]->back_gradient());
    }
    layers[0]->backward(bottom, layers[1]->back_gradient());
    shortcut[0]->backward(bottom, layers[n_layer-1]->back_gradient());

    grad_bottom = layers[0]->back_gradient() + shortcut[0]->back_gradient();
}

void ConvBlock::update(Optimizer &opt) {
    for (int i = 0; i < layers.size(); i++) {
        layers[i]->update(opt);
    }
    shortcut[0]->update(opt);
}