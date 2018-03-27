#!/usr/bin/env python

# Copyright 2017    Ming Tu

# This script contains the main function to generate all phoneme probabilities of the alignment.

from textgrid import TextGrid, Interval
import sys, os
import argparse
import numpy as np
import re, subprocess

def phn_conf_processing(phn_conf_file, all_phone_file):
    '''
    Processing phoneme confusion in alignment
    :param phn_conf_file:
    :param all_phone_file:
    :return: processed phoneme confusion matrix
    '''

    #generate phoneme mapping
    phn_mapping = {}
    line_cnt = 0
    with open(all_phone_file, 'r') as fid:
        for eachline in fid:
            if line_cnt == 0 or eachline.startswith("#"):
                line_cnt += 1
                continue
            else:
                current_phn = re.sub("\d+","", eachline.rstrip().split()[0].split('_')[0])
                current_phn_idx = int(eachline.rstrip().split()[1])
                if current_phn_idx in phn_mapping:
                    continue
                else:
                    phn_mapping[current_phn_idx] = current_phn
                line_cnt += 1

    phn_conf = {}
    current_uttid = ""
    line_cnt = 0
    with open(phn_conf_file,'r') as conf_fid:
        for eachline in conf_fid:
            if not eachline.startswith(' '):
                uttid = eachline.rstrip().split()[0]
                phn_conf[uttid] = []
                current_uttid = uttid
                line_cnt = 1
            else:
                #print "processing {}, line {}".format(current_uttid, line_cnt)
                current_time = {}
                for idx, item in enumerate(eachline.strip().split(']')[0].split()):
                    if current_time.has_key(phn_mapping[idx+1]):
                        current_time[phn_mapping[idx+1]] = np.logaddexp(current_time[phn_mapping[idx+1]],float(item))
                    else:
                        current_time[phn_mapping[idx + 1]] = float(item)
                phn_conf[current_uttid].append(current_time)
                line_cnt += 1
    
    return phn_conf


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--phn_conf_file', help='file including confusion information of alignment')
    parser.add_argument('--all_phone_file', help='file including all phones in acoustic model')
    parser.add_argument('--output_dir', help="directory where output is stored")

    args = parser.parse_args()
    subprocess.call("mkdir -p " + args.output_dir, shell=True)

    phn_conf = phn_conf_processing(args.phn_conf_file,args.all_phone_file)

    for uttid in phn_conf.keys():
        with open(args.output_dir + '/' + uttid + '.conf', 'w') as fid:
            for time_slot in phn_conf[uttid]:
                for prob in time_slot.keys():
                    fid.write(prob + " {} ".format(time_slot[prob]))
                fid.write("\n")