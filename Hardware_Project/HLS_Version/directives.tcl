############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc"
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Input
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Output
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Weight
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Beta
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" InFM_num
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" OutFM_num
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Kernel_size
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" Kernel_stride
set_directive_interface -mode m_axi -depth 4194304 -offset slave -bundle Weight "detection_acc" Weight
set_directive_interface -mode m_axi -depth 16384 -offset slave -bundle Input "detection_acc" Input
set_directive_interface -mode m_axi -depth 16384 -offset slave -bundle Output "detection_acc" Output
set_directive_interface -mode m_axi -depth 1024 -offset slave -bundle Beta "detection_acc" Beta
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" TM
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" TN
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" TR
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" TC
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" mLoops
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" nLoops
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" LayerType
set_directive_interface -mode s_axilite -bundle CRTL_BUS "detection_acc" M
