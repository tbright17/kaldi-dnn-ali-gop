#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright 2017    Ming Tu

# This script contains several functions to analyze ctm files output by acoustic model
# and convert them to textgrid format files.
# This code is adapted from corresponding code in Montreal-Forced-aligner 
# (https://github.com/MontrealCorpusTools/Montreal-Forced-Aligner.git)

import os, re
import sys
import traceback
from decimal import Decimal
from textgrid import TextGrid, IntervalTier
from io import open as io_open
#from textgrid import *


def generate_mapping(mapping_file):
    mapping = {}
    with io_open(mapping_file, 'r', encoding='utf-8') as fid:
        word_num_pairs = fid.readlines()
        for item in word_num_pairs:
            word = item.strip().split()[0]
            num = item.strip().split()[1]
            mapping[int(num)] = word

    return mapping


def parse_ctm(ctm_path, args, mode='word'):
    if mode == 'word':
        mapping = generate_mapping(args.word_mapping)
    elif mode == 'phone':
        mapping = generate_mapping(args.phone_mapping)
    file_dict = {}
    with open(ctm_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line == '':
                continue
            line = line.split(' ')
            utt = line[0]
            begin = Decimal(line[2])
            duration = Decimal(line[3])
            end = begin + duration
            label = line[4]

            filename = utt

            try:
                label = mapping[int(label)]
            except KeyError:
                pass

            if filename not in file_dict:
                file_dict[filename] = []

            file_dict[filename].append([begin, end, label])

    return file_dict

def generate_utt2dur(utt2dur_file):
    mapping = {}
    with open(utt2dur_file, 'r') as fid:
        utt_dur_pairs = fid.readlines()
        for item in utt_dur_pairs:
            utt = item.strip().split()[0]
            dur = item.strip().split()[1]
            mapping[utt] = float(dur)

    return mapping

def ctm_to_textgrid(word_ctm, phone_ctm, out_directory, utt2dur, frameshift=0.01):
    textgrid_write_errors = {}
    frameshift = Decimal(str(frameshift))
    if not os.path.exists(out_directory):
        os.makedirs(out_directory)

    utt2dur_mapping = generate_utt2dur(utt2dur)

    for i, (k, v) in enumerate(sorted(word_ctm.items())):
        maxtime = Decimal(str(utt2dur_mapping[k]))
        try:
            tg = TextGrid(maxTime=maxtime)
            wordtier = IntervalTier(name='words', maxTime=maxtime)
            phonetier = IntervalTier(name='phones', maxTime=maxtime)
            for interval in v:
                if maxtime - interval[1] < frameshift:  # Fix rounding issues
                    interval[1] = maxtime
                wordtier.add(*interval)
            for interval in phone_ctm[k]:
                if maxtime - interval[1] < frameshift:
                    interval[1] = maxtime
                #remove B E I and stress (0,1) information from phoneme
                interval[2] = re.sub("\d+","",interval[2].split('_')[0])
                phonetier.add(*interval)
            tg.append(wordtier)
            tg.append(phonetier)
            outpath = os.path.join(out_directory, k + '.TextGrid')
            tg.write(outpath)
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            textgrid_write_errors[k] = '\n'.join(
                traceback.format_exception(exc_type, exc_value, exc_traceback))
    if textgrid_write_errors:
        error_log = os.path.join(out_directory, 'output_errors.txt')
        with io_open(error_log, 'w', encoding='utf-8') as f:
            f.write(
                u'The following exceptions were encountered during the ouput of the alignments to TextGrids:\n\n')
            for k, v in textgrid_write_errors.items():
                f.write(u'{}:\n'.format(k))
                f.write(u'{}\n\n'.format(v))
