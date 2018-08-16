#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright 2017    Ming Tu

# This script contains the main function to convert ctm files to textgrid format files.
# This code is adapted from corresponding code in Montreal-Forced-aligner 
# (https://github.com/MontrealCorpusTools/Montreal-Forced-Aligner.git)

import argparse, os
from textgrid_ops import *


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('num_jobs',help='number of jobs', type=int)
    parser.add_argument('ctm_directory', help='directory where phoneme ctm and word ctm are stored')
    parser.add_argument('output_directory',help='output directory for textgrid files')
    parser.add_argument('word_mapping', help='word mapping')
    parser.add_argument('phone_mapping',help='phone mapping')
    parser.add_argument('utt2dur', help = 'duration of each utterance')

    args = parser.parse_args()

    word_ctm = {}
    phone_ctm = {}
    for i in range(args.num_jobs):
        word_ctm_path = os.path.join(args.ctm_directory, 'word.{}.ctm'.format(i+1))
        phone_ctm_path = os.path.join(args.ctm_directory, 'phone.{}.ctm'.format(i+1))

        if not os.path.exists(word_ctm_path):
            continue
        
        parsed = parse_ctm(word_ctm_path, args, mode='word')
        for k, v in parsed.items():
            if k not in word_ctm:
                word_ctm[k] = v
            else:
                word_ctm[k].update(v)
        parsed = parse_ctm(phone_ctm_path, args, mode='phone')
        for k, v in parsed.items():
            if k not in phone_ctm:
                phone_ctm[k] = v
            else:
                phone_ctm[k].update(v)
    ctm_to_textgrid(word_ctm, phone_ctm, args.output_directory, args.utt2dur)

