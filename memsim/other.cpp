/// -------------------------------------------------------------------------------------
/// this is the only file you need to edit
/// -------------------------------------------------------------------------------------
///
/// (c) 2022, Pavol Federl, pfederl@ucalgary.ca
/// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <cmath>
using namespace std;

struct Partition {//create a partition struct to store the information of each partition
int tag;//-1 means free
int64_t size, addr;//size in bytes
};

typedef list<Partition>::iterator PartitionRef;

struct scmp {
bool operator()(const PartitionRef & c1, const PartitionRef & c2) const {
if (c1->size == c2->size)
return c1->addr < c2->addr;
else
return c1->size > c2->size;
}
};
typedef set<PartitionRef>::iterator TreeRef;
struct Simulator {
    int64_t page_size;
    list<Partition>  all_blocks;
    set<PartitionRef, scmp> free_blocks;
    unordered_map<long, std::vector<PartitionRef>> tag_list;
    int64_t pages_requested;
  Simulator(int64_t page_size)
  {
    this->page_size =  page_size;
    this->tag_list = tag_list;
    this->free_blocks = free_blocks; 
    this->all_blocks=all_blocks;
    this->pages_requested=0;
  }
  void allocate(int tag, int size){
  
  PartitionRef curr;
  bool space = false;
    if(all_blocks.empty()){
      auto new_page = ceil((double)size/(double)this->page_size);
      pages_requested+= new_page;
      auto new_parition = Partition{-1, (int)(new_page*this->page_size),0};
      all_blocks.push_back(new_parition);
      curr = all_blocks.begin();
      free_blocks.insert(curr);  
    }

    if(!free_blocks.empty()){
      TreeRef treeRef = free_blocks.begin();
      auto tree_iter = *treeRef;
      if((tree_iter)->size >= size && (tree_iter)->tag == -1 ){
        
        space = true;
        curr =tree_iter;
       
      }
    }

    if(!space){ 
      auto last_iter = prev(all_blocks.end());
      if(last_iter->tag == -1){
        free_blocks.erase(last_iter);
        auto new_mem = ceil(((double)(size-last_iter->size)/(double)this->page_size));
        int64_t mem_needed = new_mem*page_size;
        last_iter->size += mem_needed;
        pages_requested += new_mem;
      }
      else{
        int64_t mem_needed = ceil((double)size/(double)this->page_size);
        pages_requested+= mem_needed;
        all_blocks.push_back(Partition{-1, mem_needed*this->page_size, last_iter->addr + last_iter->size});
      }
      curr = prev(all_blocks.end());
      free_blocks.insert(prev(all_blocks.end()));
    }



    if(curr->tag == -1 && curr->size > size && curr->size != 0){
      free_blocks.erase(curr);
      auto free_space = (int64_t)curr->size-size;
      curr->tag = tag;//update the tag of the partition
      curr->size = size;//update the size of the partition
      tag_list[tag].push_back(curr);
      
      auto split_part = Partition{-1, free_space, curr->addr+curr->size};
      auto last_block=prev(all_blocks.end());
      if(curr == last_block ){
        all_blocks.push_back(split_part);
        free_blocks.insert(prev(all_blocks.end()));
      }
      else{
        all_blocks.insert(next(curr), split_part);
        free_blocks.insert(next(curr));
      }
    }
    else{
      free_blocks.erase(curr);
      curr->size = size;
      curr->tag = tag;
      tag_list[tag].push_back(curr);
      
    } 
    }
  

















  
  void deallocate(int tag)
  {
    vector<PartitionRef> iter = tag_list[tag];
    
    for(int i = 0; i < (int)iter.size(); i++){
      PartitionRef curr_node = iter.at(i);
      curr_node->tag = -1;
      auto free_partiton =prev(curr_node);
      if(curr_node != all_blocks.begin() && free_partiton->tag == -1){
        curr_node->addr = free_partiton->addr;
        curr_node->size += free_partiton->size;
        all_blocks.erase(free_partiton);
        free_blocks.erase(free_partiton);
        
        
      }
      auto merge_partition = next(curr_node);
      if(curr_node != prev(all_blocks.end()) && merge_partition->tag == -1){
        curr_node->size += merge_partition->size;
        all_blocks.erase(merge_partition);
        free_blocks.erase(merge_partition);
        
      }
     
      free_blocks.insert(curr_node);
    }
    
    tag_list.erase(tag);
  }




  MemSimResult getStats()
  {
    // let's guess the result... :)
    MemSimResult result;

    result.max_free_partition_size = 0;
    result.max_free_partition_address = 0;
  

    result.n_pages_requested = pages_requested;
    for(PartitionRef iter = this->all_blocks.begin(); iter != this->all_blocks.end(); iter++){
      if(iter->tag == -1 && iter->size > result.max_free_partition_size){
        result.max_free_partition_size = iter->size;
        result.max_free_partition_address = iter->addr;
      }
    }
    return result;
  }
  
};

MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you probably do not need
  // to modify below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    //sim.check_consistency();
  }
  return sim.getStats();
}