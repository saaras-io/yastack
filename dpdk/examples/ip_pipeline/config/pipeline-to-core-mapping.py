#!/usr/bin/env python
# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2016 Intel Corporation
#
# This script maps the set of pipelines identified (MASTER pipelines are
# ignored) from the input configuration file to the set of cores
# provided as input argument and creates configuration files for each of
# the mapping combinations.
#
from __future__ import print_function
from collections import namedtuple
import argparse
import array
import errno
import itertools
import os
import re
import sys
# default values
enable_stage0_traceout = 1
enable_stage1_traceout = 1
enable_stage2_traceout = 1
enable_stage1_fileout = 1
enable_stage2_fileout = 1
Constants = namedtuple('Constants', ['MAX_CORES', 'MAX_PIPELINES'])
constants = Constants(16, 64)
# pattern for physical core
pattern_phycore = '^(s|S)\d(c|C)[1-9][0-9]*$'
reg_phycore = re.compile(pattern_phycore)
def popcount(mask):
    return bin(mask).count("1")
def len2mask(length):
    if (length == 0):
        return 0
    if (length > 64):
        sys.exit('error: len2mask - length %i > 64. exiting' % length)
    return int('1' * length, 2)
def bitstring_write(n, n_bits):
    tmpstr = ""
    if (n_bits > 64):
        return
    i = n_bits - 1
    while (i >= 0):
        cond = (n & (1 << i))
        if (cond):
            print('1', end='')
            tmpstr += '1'
        else:
            print('0', end='')
            tmpstr += '0'
        i -= 1
    return tmpstr
class Cores0:
    def __init__(self):
        self.n_pipelines = 0
class Cores1:
    def __init__(self):
        self.pipelines = 0
        self.n_pipelines = 0
class Cores2:
    def __init__(self):
        self.pipelines = 0
        self.n_pipelines = 0
        self.counter = 0
        self.counter_max = 0
        self.bitpos = array.array(
            "L", itertools.repeat(0, constants.MAX_PIPELINES))
class Context0:
    def __init__(self):
        self.cores = [Cores0() for i in range(0, constants.MAX_CORES)]
        self.n_cores = 0
        self.n_pipelines = 0
        self.n_pipelines0 = 0
        self.pos = 0
        self.file_comment = ""
        self.ctx1 = None
        self.ctx2 = None
    def stage0_print(self):
        print('printing Context0 obj')
        print('c0.cores(n_pipelines) = [ ', end='')
        for cores_count in range(0, constants.MAX_CORES):
            print(self.cores[cores_count].n_pipelines, end=' ')
        print(']')
        print('c0.n_cores = %d' % self.n_cores)
        print('c0.n_pipelines = %d' % self.n_pipelines)
        print('c0.n_pipelines0 = %d' % self.n_pipelines0)
        print('c0.pos = %d' % self.pos)
        print('c0.file_comment = %s' % self.file_comment)
        if (self.ctx1 is not None):
            print('c0.ctx1 = ', end='')
            print(repr(self.ctx1))
        else:
            print('c0.ctx1 = None')
        if (self.ctx2 is not None):
            print('c0.ctx2 = ', end='')
            print(repr(self.ctx2))
        else:
            print('c0.ctx2 = None')
    def stage0_init(self, num_cores, num_pipelines, ctx1, ctx2):
        self.n_cores = num_cores
        self.n_pipelines = num_pipelines
        self.ctx1 = ctx1
        self.ctx2 = ctx2
    def stage0_process(self):
        # stage0 init
        self.cores[0].n_pipelines = self.n_pipelines
        self.n_pipelines0 = 0
        self.pos = 1
        while True:
            # go forward
            while True:
                if ((self.pos < self.n_cores) and (self.n_pipelines0 > 0)):
                    self.cores[self.pos].n_pipelines = min(
                        self.cores[self.pos - 1].n_pipelines,
                        self.n_pipelines0)
                    self.n_pipelines0 -= self.cores[self.pos].n_pipelines
                    self.pos += 1
                else:
                    break
            # check solution
            if (self.n_pipelines0 == 0):
                self.stage0_log()
                self.ctx1.stage1_init(self, self.ctx2)  # self is object c0
                self.ctx1.stage1_process()
            # go backward
            while True:
                if (self.pos == 0):
                    return
                self.pos -= 1
                if ((self.cores[self.pos].n_pipelines > 1) and
                        (self.pos != (self.n_cores - 1))):
                    break
                self.n_pipelines0 += self.cores[self.pos].n_pipelines
                self.cores[self.pos].n_pipelines = 0
            # rearm
            self.cores[self.pos].n_pipelines -= 1
            self.n_pipelines0 += 1
            self.pos += 1
    def stage0_log(self):
        tmp_file_comment = ""
        if(enable_stage0_traceout != 1):
            return
        print('STAGE0: ', end='')
        tmp_file_comment += 'STAGE0: '
        for cores_count in range(0, self.n_cores):
            print('C%d = %d\t'
                  % (cores_count,
                      self.cores[cores_count].n_pipelines), end='')
            tmp_file_comment += "C{} = {}\t".format(
                cores_count, self.cores[cores_count].n_pipelines)
        # end for
        print('')
        self.ctx1.stage0_file_comment = tmp_file_comment
        self.ctx2.stage0_file_comment = tmp_file_comment
class Context1:
    _fileTrace = None
    def __init__(self):
        self.cores = [Cores1() for i in range(constants.MAX_CORES)]
        self.n_cores = 0
        self.n_pipelines = 0
        self.pos = 0
        self.stage0_file_comment = ""
        self.stage1_file_comment = ""
        self.ctx2 = None
        self.arr_pipelines2cores = []
    def stage1_reset(self):
        for i in range(constants.MAX_CORES):
            self.cores[i].pipelines = 0
            self.cores[i].n_pipelines = 0
        self.n_cores = 0
        self.n_pipelines = 0
        self.pos = 0
        self.ctx2 = None
        # clear list
        del self.arr_pipelines2cores[:]
    def stage1_print(self):
        print('printing Context1 obj')
        print('ctx1.cores(pipelines,n_pipelines) = [ ', end='')
        for cores_count in range(0, constants.MAX_CORES):
            print('(%d,%d)' % (self.cores[cores_count].pipelines,
                               self.cores[cores_count].n_pipelines), end=' ')
        print(']')
        print('ctx1.n_cores = %d' % self.n_cores)
        print('ctx1.n_pipelines = %d' % self.n_pipelines)
        print('ctx1.pos = %d' % self.pos)
        print('ctx1.stage0_file_comment = %s' % self.stage0_file_comment)
        print('ctx1.stage1_file_comment = %s' % self.stage1_file_comment)
        if (self.ctx2 is not None):
            print('ctx1.ctx2 = ', end='')
            print(self.ctx2)
        else:
            print('ctx1.ctx2 = None')
    def stage1_init(self, c0, ctx2):
        self.stage1_reset()
        self.n_cores = 0
        while (c0.cores[self.n_cores].n_pipelines > 0):
            self.n_cores += 1
        self.n_pipelines = c0.n_pipelines
        self.ctx2 = ctx2
        self.arr_pipelines2cores = [0] * self.n_pipelines
        i = 0
        while (i < self.n_cores):
            self.cores[i].n_pipelines = c0.cores[i].n_pipelines
            i += 1
    def stage1_process(self):
        pipelines_max = len2mask(self.n_pipelines)
        while True:
            pos = 0
            overlap = 0
            if (self.cores[self.pos].pipelines == pipelines_max):
                if (self.pos == 0):
                    return
                self.cores[self.pos].pipelines = 0
                self.pos -= 1
                continue
            self.cores[self.pos].pipelines += 1
            if (popcount(self.cores[self.pos].pipelines) !=
                    self.cores[self.pos].n_pipelines):
                continue
            overlap = 0
            pos = 0
            while (pos < self.pos):
                if ((self.cores[self.pos].pipelines) &
                        (self.cores[pos].pipelines)):
                    overlap = 1
                    break
                pos += 1
            if (overlap):
                continue
            if ((self.pos > 0) and
                ((self.cores[self.pos].n_pipelines) ==
                    (self.cores[self.pos - 1].n_pipelines)) and
                    ((self.cores[self.pos].pipelines) <
                        (self.cores[self.pos - 1].pipelines))):
                continue
            if (self.pos == self.n_cores - 1):
                self.stage1_log()
                self.ctx2.stage2_init(self)
                self.ctx2.stage2_process()
                if (self.pos == 0):
                    return
                self.cores[self.pos].pipelines = 0
                self.pos -= 1
                continue
            self.pos += 1
    def stage1_log(self):
        tmp_file_comment = ""
        if(enable_stage1_traceout == 1):
            print('STAGE1: ', end='')
            tmp_file_comment += 'STAGE1: '
            i = 0
            while (i < self.n_cores):
                print('C%d = [' % i, end='')
                tmp_file_comment += "C{} = [".format(i)
                j = self.n_pipelines - 1
                while (j >= 0):
                    cond = ((self.cores[i].pipelines) & (1 << j))
                    if (cond):
                        print('1', end='')
                        tmp_file_comment += '1'
                    else:
                        print('0', end='')
                        tmp_file_comment += '0'
                    j -= 1
                print(']\t', end='')
                tmp_file_comment += ']\t'
                i += 1
            print('\n', end='')
            self.stage1_file_comment = tmp_file_comment
            self.ctx2.stage1_file_comment = tmp_file_comment
        # check if file traceing is enabled
        if(enable_stage1_fileout != 1):
            return
        # spit out the combination to file
        self.stage1_process_file()
    def stage1_updateCoresInBuf(self, nPipeline, sCore):
        rePipeline = self._fileTrace.arr_pipelines[nPipeline]
        rePipeline = rePipeline.replace("[", "\[").replace("]", "\]")
        reCore = 'core\s*=\s*((\d*)|(((s|S)\d)?(c|C)[1-9][0-9]*)).*\n'
        sSubs = 'core = ' + sCore + '\n'
        reg_pipeline = re.compile(rePipeline)
        search_match = reg_pipeline.search(self._fileTrace.in_buf)
        if(search_match):
            pos = search_match.start()
            substr1 = self._fileTrace.in_buf[:pos]
            substr2 = self._fileTrace.in_buf[pos:]
            substr2 = re.sub(reCore, sSubs, substr2, 1)
            self._fileTrace.in_buf = substr1 + substr2
    def stage1_process_file(self):
        outFileName = os.path.join(self._fileTrace.out_path,
                                   self._fileTrace.prefix_outfile)
        outFileName += "_{}CoReS".format(self.n_cores)
        i = 0  # represents core number
        while (i < self.n_cores):
            j = self.n_pipelines - 1
            pipeline_idx = 0
            while(j >= 0):
                cond = ((self.cores[i].pipelines) & (1 << j))
                if (cond):
                    # update the pipelines array to match the core
                    # only in case of cond match
                    self.arr_pipelines2cores[
                        pipeline_idx] = fileTrace.in_physical_cores[i]
                j -= 1
                pipeline_idx += 1
            i += 1
        # update the in_buf as per the arr_pipelines2cores
        for pipeline_idx in range(len(self.arr_pipelines2cores)):
            outFileName += "_{}".format(self.arr_pipelines2cores[pipeline_idx])
            self.stage1_updateCoresInBuf(
                pipeline_idx, self.arr_pipelines2cores[pipeline_idx])
        # by now the in_buf is all set to be written to file
        outFileName += self._fileTrace.suffix_outfile
        outputFile = open(outFileName, "w")
        # write out the comments
        strTruncated = ("", "(Truncated)")[self._fileTrace.ncores_truncated]
        outputFile.write(
            "; =============== Pipeline-to-Core Mapping ================\n"
            "; Generated from file {}\n"
            "; Input pipelines = {}\n"
            "; Input cores = {}\n"
            "; N_PIPELINES = {} N_CORES = {} {} hyper_thread = {}\n"
            .format(
                self._fileTrace.in_file_namepath,
                fileTrace.arr_pipelines,
                fileTrace.in_physical_cores,
                self._fileTrace.n_pipelines,
                self._fileTrace.n_cores,
                strTruncated,
                self._fileTrace.hyper_thread))
        outputFile.write(
            "; {stg0cmt}\n"
            "; {stg1cmt}\n"
            "; ========================================================\n"
            "; \n"
            .format(
                stg0cmt=self.stage0_file_comment,
                stg1cmt=self.stage1_file_comment))
        # write buffer contents
        outputFile.write(self._fileTrace.in_buf)
        outputFile.flush()
        outputFile.close()
class Context2:
    _fileTrace = None
    def __init__(self):
        self.cores = [Cores2() for i in range(constants.MAX_CORES)]
        self.n_cores = 0
        self.n_pipelines = 0
        self.pos = 0
        self.stage0_file_comment = ""
        self.stage1_file_comment = ""
        self.stage2_file_comment = ""
        # each array entry is a pipeline mapped to core stored as string
        # pipeline ranging from 1 to n, however stored in zero based array
        self.arr2_pipelines2cores = []
    def stage2_print(self):
        print('printing Context2 obj')
        print('ctx2.cores(pipelines, n_pipelines, counter, counter_max) =')
        for cores_count in range(0, constants.MAX_CORES):
            print('core[%d] = (%d,%d,%d,%d)' % (
                cores_count,
                self.cores[cores_count].pipelines,
                self.cores[cores_count].n_pipelines,
                self.cores[cores_count].counter,
                self.cores[cores_count].counter_max))
            print('ctx2.n_cores = %d' % self.n_cores, end='')
            print('ctx2.n_pipelines = %d' % self.n_pipelines, end='')
            print('ctx2.pos = %d' % self.pos)
            print('ctx2.stage0_file_comment = %s' %
                  self.self.stage0_file_comment)
            print('ctx2.stage1_file_comment = %s' %
                  self.self.stage1_file_comment)
            print('ctx2.stage2_file_comment = %s' %
                  self.self.stage2_file_comment)
    def stage2_reset(self):
        for i in range(0, constants.MAX_CORES):
            self.cores[i].pipelines = 0
            self.cores[i].n_pipelines = 0
            self.cores[i].counter = 0
            self.cores[i].counter_max = 0
            for idx in range(0, constants.MAX_PIPELINES):
                self.cores[i].bitpos[idx] = 0
        self.n_cores = 0
        self.n_pipelines = 0
        self.pos = 0
        # clear list
        del self.arr2_pipelines2cores[:]
    def bitpos_load(self, coreidx):
        i = j = 0
        while (i < self.n_pipelines):
            if ((self.cores[coreidx].pipelines) &
                    (1 << i)):
                self.cores[coreidx].bitpos[j] = i
                j += 1
            i += 1
        self.cores[coreidx].n_pipelines = j
    def bitpos_apply(self, in_buf, pos, n_pos):
        out = 0
        for i in range(0, n_pos):
            out |= (in_buf & (1 << i)) << (pos[i] - i)
        return out
    def stage2_init(self, ctx1):
        self.stage2_reset()
        self.n_cores = ctx1.n_cores
        self.n_pipelines = ctx1.n_pipelines
        self.arr2_pipelines2cores = [''] * self.n_pipelines
