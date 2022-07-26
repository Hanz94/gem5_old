/*
 * Copyright (c) 1999-2008 Mark D. Hill and David A. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MEM_RUBY_SLICC_INTERFACE_RUBYSLICC_COMPONENTMAPPINGS_HH__
#define __MEM_RUBY_SLICC_INTERFACE_RUBYSLICC_COMPONENTMAPPINGS_HH__

#include "mem/protocol/MachineType.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/common/MachineID.hh"
#include "mem/ruby/common/NetDest.hh"
#include "mem/ruby/structures/DirectoryMemory.hh"


// used to determine the home directory
// returns a value between 0 and total_directories_within_the_system
inline NodeID
map_Address_to_DirectoryNode(Addr addr)
{
    return DirectoryMemory::mapAddressToDirectoryVersion(addr);
}

 /*
inline NodeID /// Rani
k_map_Address_to_DirectoryNode(Addr addr, NodeID s_nodeID, MachineType m_type)
{
    //return DirectoryMemory::mapAddressToMemoryController(addr, 1, s_nodeID); /// Rani
    return DirectoryMemory::mapAddressToMemoryController(addr, 1, s_nodeID, m_type); /// Rani
}

inline MachineID /// Rani
k_map_Address_to_Directory(Addr addr, NodeID s_nodeID)
{
    MachineID mach = {MachineType_Directory, k_map_Address_to_DirectoryNode(addr, s_nodeID)};
    return mach;
}
*/

inline NodeID
map_Address_to_TCCdirNode(Addr addr)
{
    return DirectoryMemory::mapAddressToDirectoryVersion(addr);
}

// used to determine the home directory
// returns a value between 0 and total_directories_within_the_system
inline MachineID
map_Address_to_Directory(Addr addr)
{
    MachineID mach =
        {MachineType_Directory, map_Address_to_DirectoryNode(addr)};
    return mach;
}

//subodha: used to check of the machine is a Cache
inline bool
isSenderACache(MachineID mach)
{

	if(mach.type == MachineType_Directory)
		return false;

    return true;
}

//subodha : interface functions to DirectoryController class
inline NodeID
map_Address_To_Memory_Node(Addr addr, int num_mem_bits, NodeID nodeID, MachineType m_type)
{
    return DirectoryMemory::mapAddressToMemoryController(addr, num_mem_bits, nodeID, m_type);
}

//subodha: used to determine where the memory request should be forwarded to
inline MachineID
map_Address_to_Memory_Controller(Addr addr, NodeID nodeID, MachineType m_type)
{
	//printf("Address map called\n"); //subodha
    //NetDest dest;
    MachineID mach =
        {MachineType_Directory, map_Address_To_Memory_Node(addr, 1, nodeID, m_type)}; //set mem bits here
	//dest.add(mach);
    return mach;
}

//subodha: check if the current directory is where the memory controller is
inline bool
checkifMachineIsMemoryNode(MachineID mach)
{
    return mach.num == 8 || mach.num == 11;
}

inline MachineID
map_Address_to_RegionDir(Addr addr)
{
    MachineID mach = {MachineType_RegionDir,
                      map_Address_to_DirectoryNode(addr)};
    return mach;
}

inline MachineID
map_Address_to_TCCdir(Addr addr)
{
    MachineID mach =
        {MachineType_TCCdir, map_Address_to_TCCdirNode(addr)};
    return mach;
}

inline NetDest
broadcast(MachineType type)
{
    NetDest dest;
    for (NodeID i = 0; i < MachineType_base_count(type); i++) {
        MachineID mach = {type, i};
        dest.add(mach);
    }
    return dest;
}

inline MachineID
mapAddressToRange(Addr addr, MachineType type, int low_bit,
                  int num_bits, int cluster_id = 0)
{
    MachineID mach = {type, 0};
    if (num_bits == 0)
        mach.num = cluster_id;
    else
        mach.num = bitSelect(addr, low_bit, low_bit + num_bits - 1)
            + (1 << num_bits) * cluster_id;
    return mach;
}

inline NodeID
machineIDToNodeID(MachineID machID)
{
    return machID.num;
}

///Rani
inline void
k_printNodeID(NodeID nodeID)
{
    std::cout << "NodeID:" << nodeID << "\n";
    //return machineIDToNodeID(machID);
}

/// Rani
inline MachineType
k_printMachineID_Type(MachineID machID)
{
    return machID.getType();
}

inline MachineType
machineIDToMachineType(MachineID machID)
{
    return machID.type;
}

inline int
machineCount(MachineType machType)
{
    return MachineType_base_count(machType);
}

inline MachineID
createMachineID(MachineType type, NodeID id)
{
    MachineID mach = {type, id};
    return mach;
}

inline MachineID
MachineTypeAndNodeIDToMachineID(MachineType type, NodeID node)
{
    MachineID mach = {type, node};
    return mach;
}

#endif  // __MEM_RUBY_SLICC_INTERFACE_COMPONENTMAPPINGS_HH__
