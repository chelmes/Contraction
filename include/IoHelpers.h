#ifndef IO_HELPERS_H_
#define IO_HELPERS_H_

#include <array>
#include <complex>
#include <cstdlib>
#include <string>
#include <vector>

#include "boost/crc.hpp"

#include "global_data.h"
#include "lime.h"
#include "typedefs.h"
///////////////////////////////////////////////////////////////////////////////
// Typedefs ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct Tag {
  // The center of mass momentum squared
  std::string q_cont;
  int mom_cm;
  // the momenta for 4 particles
  std::vector<std::array<int, 3 > > mom;
  // 4 displacement vectors
  std::vector<std::array<int, 3 > > dis;
  // 4 gamma 4 vectors
  std::vector<std::array<int, 4 > > gam;
};
// use a general tag for n-pt functions
// Each tag contains a string of participating quarks to be checked against with
// on the search of a Tag


// Global infos on the run should be stored here, no idea what that could be
// IDEAS: Configuration number, channels, rnd seeds, number of input vectors,
// Lattice size, ensemble used
struct GlobalDat {
  std::vector<size_t> rnd_seeds;
  size_t nb_rnd_vecs;
  size_t nb_perambs;
};
 void write_1st_msg(const char* filename, GlobalDat& dat,
                           size_t chksum);
 void append_msgs(const char* filename, std::vector<vec>& corr, std::vector<std::string>& tags,
              LimeWriter* w, bool be);
   
// Endianess ///////////////////////////////////////////////////////////////////
// test system endianess
inline int big_endian () {
	union {
		int l;
		char c[sizeof(int)];
	} u;

	u.l = 1;
	return (u.c[sizeof(int) - 1] == 1);
}

// swapping endianess 
template <typename T> inline T swap_endian(T u) {
  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;
  source.u = u;
  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1]; 
  return dest.u;
}

inline std::complex<double>  swap_complex(std::complex<double> val){

  return std::complex<double>(swap_endian<double>(std::real(val)),
                              swap_endian<double>(std::imag(val)));
}
  
// swap endianess of one tag
//inline Tag swap_single_tag(const Tag& tag){
//  Tag le_tag;
//  le_tag.mom_cm = swap_endian<int>(tag.mom_cm);
//  for (auto& p : tag.mom) le_tag.mom.push_back(swap_endian<std::array<int, 3 > >(p));
//  for (auto& d : tag.dis) le_tag.dis.push_back(swap_endian<std::array<int, 3 > >(d));
//  for (auto& g : tag.gam) le_tag.gam.push_back(swap_endian<std::array<int, 4 > >(g));
//
//  return le_tag;
//}

// swap endianess of one correlation function
inline std::vector<cmplx> swap_single_corr(const std::vector<cmplx>& corr){
  size_t ext = corr.size();
  // Temporary vector same size as input
  std::vector<cmplx> le(ext);
  // Swap endianness for every entry in corr
  for(size_t val = 0; val < ext; ++val) {
    le[val] = swap_complex(corr[val]);
  }
  return le;
}


// swap endaness of runinfo
inline GlobalDat swap_glob_dat(const GlobalDat& run_info){
  GlobalDat le_glob;
  for (auto seed = 0; seed < run_info.rnd_seeds.size(); ++seed)
    le_glob.rnd_seeds.push_back( swap_endian<size_t>(run_info.rnd_seeds[seed]) );
  le_glob.nb_rnd_vecs = swap_endian<size_t>(run_info.nb_rnd_vecs);
  le_glob.nb_perambs = swap_endian<size_t>(run_info.nb_perambs);
  return le_glob;
}

// type independent checksum of container
template <typename MyContainer> 
inline boost::uint64_t checksum(const MyContainer& dat, size_t bytes){
  boost::crc_32_type chksum_agent;
  chksum_agent.process_bytes(&dat[0], bytes);
  return chksum_agent.checksum();
}

// check existence of a file
inline bool file_exist(const char* name) {
  if (FILE* file = fopen(name, "r")) {
    fclose(file);
    return true;
  } 
  else {
    return false;
  }   
}

// Map configurations of an infile to a tag
// Tag map_input(const char* infile);

// Copy over array to std::array<int, 3>
std::array<int, 3> std_arr(int* arr);
// The other way round

// Convert ascii labels to correlation tag
Tag id(size_t g_so, size_t g_si, size_t p_so, size_t p_si, size_t dis_so, 
       size_t dis_si);


// add two three momenta
std::array<int, 3 > add_mom(const std::array<int,3> p1 , const std::array<int,3> p2);
// Calculate p^2
int square_comp(const std::array<int, 3>& p1, const std::array<int, 3>& p2);

// swap vector of all correlation functions
inline void swap_correlators(std::vector<vec>& corr){
  for (auto& func : corr){
    func = swap_single_corr(func);
  }
}

// swap vector of all 
//inline void swap_tag_vector(std::vector<Tag>& tags){
//  for (auto& label : tags){
//    label = swap_single_tag(label);
//  }
//}

// Check checksums
void file_check(const size_t glob_check,
                const std::vector<boost::uint64_t>& checksums,
                const std::vector<cmplx>& correlators);

//TODO: would be good to have this in the IoHelpers instead of CorrelatorIo.cpp
// convert multiarray 2pt correlator to vector to match write_2pt_lime
//template <typename listvector> 
//void convert_C2_mes_to_vec(const listvector& op_mes, array_cd_d2& C2_mes, 
//                           std::vector<Tag>& tags, std::vector<vec>& corr);

#endif // IO_HELPERS_H_
