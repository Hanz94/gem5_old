# Copyright (c) 2009 Advanced Micro Devices, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Steve Reinhardt
#          Brad Beckmann

from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class RubyDirectoryMemory(SimObject):
    type = 'RubyDirectoryMemory'
    cxx_class = 'DirectoryMemory'
    cxx_header = "mem/ruby/structures/DirectoryMemory.hh"
    version = Param.Int(0, "")
    size = Param.MemorySize("1GB", "capacity in bytes")
    # the default value of the numa high bit is specified in the command line
    # option and must be passed into the directory memory sim object
    numa_high_bit = Param.Int("numa high bit")
    dir_mp_src1 = Param.Int(0,"Source of 1st Directory Mapping")
    dir_mp_src2 = Param.Int(3,"Source of 2nd Directory Mapping")
    dir_mp_mem1 = Param.Int(12,"Memory Controller of 1st Directory Mapping")
    dir_mp_mem2 = Param.Int(13,"Memory Controller of 2nd Directory Mapping")
    dir_mp_default = Param.Int(5,"Memory Controller for default Directory Mapping")
    dir_mp_noise_ratio = Param.Int(0,"Noise of first communication pair")
