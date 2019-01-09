#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright 2018   Ming Tu

# This script collect phoneme segmentation information from textgrid files
# and write to kaldi ark files.

import argparse, os
from sys import exit
import numpy as np
from decimal import Decimal
from textgrid import TextGrid, IntervalTier
from kaldi_io import write_mat

def list_files(directory, extension):
    file_list = []
    for (dirpath, dirnames, filenames) in os.walk(directory + '/'):
        for file in filenames:
            if file.upper().endswith(extension):
                file_list.append(os.path.join(dirpath,file))
    return file_list


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('audio_dir',help='audio directory')
    parser.add_argument('data_dir',help='data directory')
    parser.add_argument('frame_shift', nargs='?', type=int, default=10, help='Frame shift in milliseconds, optional')

    args = parser.parse_args()

    ark_dict = {}

    tg_list_files = list_files(args.audio_dir, "TEXTGRID")

    if not tg_list_files:
        exit("TextGrid files not found, exit!")

    for tg_file in tg_list_files:
        uttid = tg_file.split('/')[-1].split('.')[0]

        my_tg_file = TextGrid()
        my_tg_file.read(tg_file)

        int_array = np.zeros((0,2))

        for intervals in my_tg_file.tiers[1]:
            if len(intervals.mark) != 0:
                int_array = np.vstack((int_array, np.array([float(intervals.minTime)*1000/args.frame_shift, 
                float(intervals.maxTime)*1000/args.frame_shift])))

        ark_dict[uttid] = int_array
    
    with open(args.data_dir + '/align.ark','w') as f:
        for key,mat in ark_dict.iteritems():
            write_mat(f, mat, key=key)

